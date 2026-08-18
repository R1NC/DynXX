#if defined(DYNXX_USE_CURL)

#include "HttpClient.hxx"

#include <algorithm>
#include <fstream>
#include <mutex>

#if defined(DYNXX_USE_ADA)
#include <ada.h>
#endif

#include <DynXX/CXX/Log.hxx>
#include <DynXX/C/Net.h>
#include <DynXX/CXX/Coding.hxx>

namespace
{
    using enum DynXXLogLevelX;
    using enum DynXXHttpMethodX;

    constexpr auto HTTP_STATUS_OK = 200;
    // reserve slack for the two ':' separators and the port digits in a resolve entry
    constexpr auto RESOLVE_ENTRY_RESERVE = 8;
    // reserve slack for the ':' separator and the port digits in a proxy URL
    constexpr auto PROXY_URL_RESERVE = 8;

    const char *errMsg(CURLcode code) {
        return curl_easy_strerror(code);
    }

    // Global configs are written by any script thread (sync set APIs) and read by the
    // async-API worker pool (resolve list / cert path / proxy apply), so they are guarded
    // by a mutex; readers snapshot the state under the lock and work on the copy.
    std::mutex gConfigMutex;
    std::string gCertPath{};
    DynXXHttpProxyConfigX gProxy{};
    std::vector<DynXXHttpDnsConfigX> gDnsConfigs{};

    bool globalInit() {
        if (const auto ret = curl_global_init(CURL_GLOBAL_DEFAULT); ret != CURLE_OK) {
            dynxxLogPrintF(Error, "HttpClient globalInit failed! msg: {}", errMsg(ret));
            return false;
        }
        return true;
    }

    void globalRelease() {
        curl_global_cleanup();
    }

    void httpSetCertPath(std::string_view path) {
        const auto lock = std::scoped_lock(gConfigMutex);
        gCertPath.assign(path.data(), path.size());
    }

    void httpSetProxy(const DynXXHttpProxyConfigX &cfg) {
        const auto lock = std::scoped_lock(gConfigMutex);
        gProxy = cfg;
    }

    void httpSetDnsConfigs(const std::vector<DynXXHttpDnsConfigX> &configs) {
        const auto lock = std::scoped_lock(gConfigMutex);
        gDnsConfigs.clear();
        gDnsConfigs.reserve(configs.size());
        for (const auto &cfg : configs) {
            if (cfg.host.empty() || cfg.address.empty()) [[unlikely]] {
                dynxxLogPrint(Warn, "HttpClient httpSetDnsConfigs skip invalid config");
                continue;
            }
            gDnsConfigs.emplace_back(cfg);
        }
    }

    // libcurl does not copy the CURLOPT_RESOLVE list, the pointer is read while the
    // transfer runs, so the list is built per request here and freed by Req::cleanup
    // once the transfer completes; the global keeps only the C++ config vector.
    curl_slist *buildResolveList() {
        const auto configs = [&] {
            const auto lock = std::scoped_lock(gConfigMutex);
            return gDnsConfigs;
        }();
        curl_slist *list = nullptr;
        for (const auto &cfg : configs) {
            std::string entry;
            entry.reserve(cfg.host.size() + cfg.address.size() + RESOLVE_ENTRY_RESERVE);
            entry.append(cfg.host).append(":").append(std::to_string(cfg.port)).append(":").append(cfg.address);
            if (auto newList = curl_slist_append(list, entry.c_str()); newList != nullptr) [[likely]] {
                list = newList;
            } else [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient buildResolveList append failed: {}", entry);
                curl_slist_free_all(list);
                return nullptr;
            }
        }
        return list;
    }

    class Req {
    private:
            CURL *curl{nullptr};
            curl_slist *headers{nullptr};
            curl_slist *resolveList{nullptr}; // per-request CURLOPT_RESOLVE list, freed in cleanup()
            curl_mime *mime{nullptr};
            std::string postFields;
            Bytes postBody;

    public:
        Req() {
            this->curl = curl_easy_init();
            if (this->curl == nullptr) [[unlikely]] {
                dynxxLogPrint(Error, "HttpClient.Req init failed");
                return;
            }
            //this->setOpt(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
            this->setOpt(CURLOPT_USERAGENT, "DynXX");
            this->setOpt(CURLOPT_FOLLOWLOCATION, 1L);//allow redirect
        };

        Req(const Req&) = delete;
        Req& operator=(const Req&) = delete;
        
        Req(Req&& other) noexcept {
            this->moveImp(std::move(other));
        }
        
        Req& operator=(Req&& other) noexcept {
            if (this != &other) [[likely]] {
                cleanup();
                this->moveImp(std::move(other));
            }
            return *this;
        }

        [[nodiscard]] bool valid() const {
            return this->curl != nullptr;
        }

        void setPostFields(std::string_view data) {
            this->postFields.assign(data.data(), data.size());
        }

        [[nodiscard]] const char *getPostFieldsData() const {
            return this->postFields.c_str();
        }

        [[nodiscard]] size_t getPostFieldsSize() const {
            return this->postFields.size();
        }

        void setPostBody(BytesView data) {
            this->postBody.assign(data.begin(), data.end());
        }

        [[nodiscard]] Bytes *getPostBody() {
            return &this->postBody;
        }

        [[nodiscard]] size_t getPostBodySize() const {
            return this->postBody.size();
        }

        void applyResolveList() {
            if (this->curl == nullptr) [[unlikely]] {
                return;
            }
            this->resolveList = buildResolveList();
            if (this->resolveList != nullptr) [[likely]] {
                this->setOpt(CURLOPT_RESOLVE, this->resolveList);
            }
        }
        
        [[nodiscard]] bool appendHeader(const char *string) {
            if (string == nullptr) [[unlikely]] {
                dynxxLogPrint(Error, "HttpClient appendHeader invalid params");
                return false;
            }
            if (auto newHeaders = curl_slist_append(this->headers, string); newHeaders) {
                this->headers = newHeaders;
                this->setOpt(CURLOPT_HTTPHEADER, this->headers);
                return true;
            }
            dynxxLogPrintF(Error, "HttpClient appendHeader failed: {}", string);
            return false;
        }

        [[nodiscard]] curl_mimepart *addMimePart() {
            if (this->curl == nullptr) [[unlikely]] {
                return nullptr;
            }
            if (this->mime == nullptr) {
                this->mime = curl_mime_init(this->curl);
                if (this->mime == nullptr) [[unlikely]] {
                    dynxxLogPrint(Error, "HttpClient addMimePart init failed");
                    return nullptr;
                }
                this->setOpt(CURLOPT_MIMEPOST, this->mime);
            }
            return curl_mime_addpart(this->mime);
        }

        [[nodiscard]] bool addMimeData(curl_mimepart *part, const char *name, const char *type, const char *dataP, size_t dataLen) const {
            if (part == nullptr || name == nullptr || type == nullptr || dataP == nullptr || dataLen == 0) [[unlikely]] {
                dynxxLogPrint(Error, "HttpClient addMime invalid params");
                return false;
            }
            auto ret = curl_mime_name(part, name);
            if (ret != CURLE_OK) [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient addMimeName failed! name: {}, msg: {}", name, errMsg(ret));
                return false;
            }
            ret = curl_mime_type(part, type);
            if (ret != CURLE_OK) [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient addMimeType failed! name: {} type: {}, msg: {}", name, type, errMsg(ret));
                return false;
            }
            ret = curl_mime_data(part, dataP, dataLen);
            if (ret != CURLE_OK) [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient addMimeData failed! name: {} type: {}, msg: {}", name, type, errMsg(ret));
                return false;
            }
            return true;
        }

        template <typename T>
        bool setOpt(CURLoption option, T value) {
            if (this->curl == nullptr) [[unlikely]]
            {
                dynxxLogPrint(Error, "HttpClient setOpt invalid curl");
                return false;
            }
            if (const auto ret = curl_easy_setopt(this->curl, option, value); ret != CURLE_OK) [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient setOpt failed! option: {}, msg: {}", static_cast<int>(option), errMsg(ret));
                return false;
            }
            return true;
        }

        [[nodiscard]] bool perform() {
            if (this->curl == nullptr) [[unlikely]]
            {
                dynxxLogPrint(Error, "HttpClient perform invalid curl");
                return false;
            }
            if (const auto ret = curl_easy_perform(this->curl); ret != CURLE_OK) [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient perform failed! msg: {}", errMsg(ret));
                return false;
            }
            return true;
        }

        template <typename T>
        [[nodiscard]] bool getInfo(CURLINFO info, T value) {
            if (this->curl == nullptr) [[unlikely]]
            {
                dynxxLogPrint(Error, "HttpClient getInfo invalid curl");
                return false;
            }
            if (const auto ret = curl_easy_getinfo(this->curl, info, value); ret != CURLE_OK) [[unlikely]] {
                dynxxLogPrintF(Error, "HttpClient getInfo failed! info: {}, msg: {}", static_cast<int>(info), errMsg(ret));
                return false;
            }
            return true;
        }

        ~Req() {
            cleanup();
        }
        
    private:
        // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
        void moveImp(Req&& other) noexcept {
            this->curl = std::exchange(other.curl, nullptr);
            this->headers = std::exchange(other.headers, nullptr);
            this->resolveList = std::exchange(other.resolveList, nullptr);
            this->mime = std::exchange(other.mime, nullptr);
            this->postFields = std::move(other.postFields);
            this->postBody = std::move(other.postBody);
        }

        void cleanup() {
            if (this->headers != nullptr) {
                curl_slist_free_all(this->headers);
                this->headers = nullptr;
            }
            if (this->resolveList != nullptr) {
                curl_slist_free_all(this->resolveList);
                this->resolveList = nullptr;
            }
            if (this->mime != nullptr) {
                curl_mime_free(this->mime);
                this->mime = nullptr;
            }
            if (this->curl != nullptr) {
                curl_easy_cleanup(this->curl);
                this->curl = nullptr;
            }
        }
    };

    size_t on_post_read(char *buffer, size_t size, size_t nmemb, RawPtr userp)
    {
        if (buffer == nullptr || userp == nullptr) [[unlikely]]
        {
            return 0;
        }
        const auto pBytes = static_cast<Bytes *>(userp);
        const auto len = static_cast<int64_t>(std::min<size_t>(size * nmemb, pBytes->size()));
        if (len > 0) [[likely]]
        {
            std::memcpy(buffer, pBytes->data(), len);
            pBytes->erase(pBytes->begin(), pBytes->begin() + len);
        }
        return len;
    }

    size_t on_upload_read(char *buffer, size_t size, size_t nmemb, RawPtr userp)
    {
        if (buffer == nullptr || userp == nullptr) [[unlikely]]
        {
            return 0;
        }
        const auto ret = std::fread(buffer, size, nmemb, static_cast<std::FILE *>(userp));
        dynxxLogPrintF(Debug, "HttpClient read {} bytes from file", ret);
        return ret;
    }

    size_t on_download_write(const char *buffer, size_t size, size_t nmemb, RawPtr userp) {
        if (buffer == nullptr || userp == nullptr) [[unlikely]]
        {
            return 0;
        }
        auto& file = *static_cast<std::ofstream*>(userp);
        file.write(buffer, static_cast<int64_t>(size * nmemb));
        const auto ret = file.good() ? size * nmemb : 0;
        dynxxLogPrintF(Debug, "HttpClient write {} bytes to file", ret);
        return ret;
    }

    size_t on_write_rsp_data(const char *buffer, size_t size, size_t nmemb, RawPtr userp)
    {
        if (buffer == nullptr || userp == nullptr) [[unlikely]]
        {
            return 0;
        }
        const auto pS = static_cast<std::string *>(userp);
        pS->append(buffer, size * nmemb);
        return size * nmemb;
    }

    size_t on_write_rsp_headers(const char *buffer, size_t size, size_t nitems, RawPtr userp)
    {
        if (buffer == nullptr || userp == nullptr) [[unlikely]]
        {
            return 0;
        }
        const auto pHeaders = static_cast<Dict *>(userp);
        std::string header(buffer, size * nitems);
        if (const auto colonPos = header.find(':'); colonPos != std::string::npos) [[likely]]
        {
            const auto k = header.substr(0, colonPos);
            const auto v = header.substr(colonPos + 2);
            pHeaders->emplace(dynxxCodingStrTrim(k), dynxxCodingStrTrim(v));
        }
        return size * nitems;
    }

    [[nodiscard]] bool checkUrlValid(std::string_view url)
    {
        if (url.empty()) [[unlikely]]
        {
            return false;
        }
#if defined(DYNXX_USE_ADA)
        if (!ada::parse(url)) [[unlikely]]
        {
            dynxxLogPrintF(Error, "HttpClient INVALID URL: {}", url);
            return false;
        }
#endif
        return true;
    }

    [[nodiscard]] bool checkUrlHasSearch(std::string_view url)
    {
        return url.contains('?');
    }

    [[nodiscard]] bool handleSSL(Req &req, std::string_view url)
    {
#if defined(DYNXX_USE_ADA)
        const auto aUrl = ada::parse(url);
        if (!aUrl) [[unlikely]]
        {
            return false;
        }
        if (static const auto protocolHttps = "https:"; aUrl->get_protocol() == protocolHttps)
#else
        if (static const auto prefixHttps = "https://"; url.starts_with(prefixHttps))
#endif
        {
            req.setOpt(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_TLSv1_3);
            req.setOpt(CURLOPT_SSL_SESSIONID_CACHE, 1L);
            req.setOpt(CURLOPT_SSL_ENABLE_ALPN, 1L);
            const auto certPath = [&] {
                const auto lock = std::scoped_lock(gConfigMutex);
                return gCertPath;
            }();
            if (certPath.empty()) {
                req.setOpt(CURLOPT_SSL_VERIFYPEER, 0L);
                req.setOpt(CURLOPT_SSL_VERIFYHOST, 0L);
            } else {
                req.setOpt(CURLOPT_SSL_VERIFYPEER, 1L);
                req.setOpt(CURLOPT_SSL_VERIFYHOST, 2L);
                req.setOpt(CURLOPT_CAINFO, certPath.c_str());
            }
        }
        return true;
    }

    Req createReq(std::string_view url, const std::vector<std::string> &headers,
                            std::string_view params, DynXXHttpMethodX method, size_t timeout)
    {
        Req req;
        if (!req.valid() || !checkUrlValid(url) || !handleSSL(req, url)) [[unlikely]]
        {
            dynxxLogPrintF(Error, "HttpClient createReq error:{}", url);
            return req;
        }

        auto _timeout = timeout;
        if (_timeout == 0) [[unlikely]]
        {
            _timeout = DYNXX_HTTP_DEFAULT_TIMEOUT;
        }
        req.setOpt(CURLOPT_CONNECTTIMEOUT_MS, _timeout);
        req.setOpt(CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, _timeout);
        req.setOpt(CURLOPT_HTTPGET, method == Get ? 1L : 0L);

        for (const auto &it : headers)
        {
            dynxxLogPrintF(Debug, "HttpClient.req header: {}", it);
            if (!req.appendHeader(it.c_str())) [[unlikely]] {
                return req;
            }
        }

        std::string fixedUrl;
        fixedUrl.reserve(url.size() + (method == Get && !params.empty() ? params.size() + 1 : 0));
        fixedUrl = url;
        if (method == Get && !params.empty())
        {
            if (!checkUrlHasSearch(fixedUrl))
            {
                fixedUrl += "?";
            }
            else if (!fixedUrl.empty() && !fixedUrl.ends_with('?') && !fixedUrl.ends_with('&'))
            {
                fixedUrl += "&";
            }
            fixedUrl += params;
        }
    
        dynxxLogPrintF(Debug, "HttpClient.req url: {}", fixedUrl);
        req.setOpt(CURLOPT_URL, fixedUrl.c_str());

        const auto proxy = [&] {
            const auto lock = std::scoped_lock(gConfigMutex);
            return gProxy;
        }();
        if (!proxy.host.empty()) {
            std::string proxyUrl;
            proxyUrl.reserve(proxy.host.size() + PROXY_URL_RESERVE);
            proxyUrl = proxy.host;
            if (proxy.port != 0) {
                proxyUrl.append(":").append(std::to_string(proxy.port));
            }
            req.setOpt(CURLOPT_PROXY, proxyUrl.c_str());
            if (!proxy.username.empty()) {
                std::string proxyUserPwd;
                proxyUserPwd.reserve(proxy.username.size() + proxy.password.size() + 1);
                proxyUserPwd = proxy.username;
                proxyUserPwd.append(":").append(proxy.password);
                req.setOpt(CURLOPT_PROXYUSERPWD, proxyUserPwd.c_str());
            }
        }

        req.applyResolveList();

        return req;
    }

    [[nodiscard]] bool submitReq(Req &req, DynXXHttpResponse &rsp)
    {
        if (!req.perform()) [[unlikely]]
        {
            return false;
        }

        if (!req.getInfo(CURLINFO_RESPONSE_CODE, &(rsp.code))) [[unlikely]]
        {
            return false;
        }

        char *contentType = nullptr;
        if (!req.getInfo(CURLINFO_CONTENT_TYPE, &contentType)) [[unlikely]]
        {
            return false;
        } 
        if (contentType != nullptr) [[likely]]
        {
            rsp.contentType = contentType;
        }

        dynxxLogPrintF(Debug, "HttpClient.rsp code: {} data: {}", rsp.code, rsp.data);

        return true;
    }
}

namespace DynXX::Core::Net {

HttpClient::HttpClient()
{
    globalInit();
}

HttpClient::~HttpClient()
{
    globalRelease();
}

void HttpClient::setCertPath(std::string_view path) {
    httpSetCertPath(path);
}

void HttpClient::setProxy(const DynXXHttpProxyConfigX &proxy) {
    httpSetProxy(proxy);
}

void HttpClient::setDnsConfigs(const std::vector<DynXXHttpDnsConfigX> &configs) {
    httpSetDnsConfigs(configs);
}

DynXXHttpResponse HttpClient::request(std::string_view url, DynXXHttpMethodX method,
                                                                 const std::vector<std::string> &headers,
                                                                std::string_view params,
                                                                 BytesView rawBody,
                                                                 const std::vector<HttpFormField> &formFields,
                                                                std::FILE *cFILE, size_t fileSize,
                                                                 size_t timeout) const {

    auto req = createReq(url, headers, params, method, timeout);
    if (!req.valid()) [[unlikely]]
    {
        return {};
    }

    if (cFILE != nullptr)
    {
        req.setOpt(CURLOPT_UPLOAD, 1L);
        req.setOpt(CURLOPT_READFUNCTION, on_upload_read);
        req.setOpt(CURLOPT_READDATA, cFILE);
        req.setOpt(CURLOPT_INFILESIZE_LARGE, fileSize);
    }
    else if (!formFields.empty())
    {
        const auto part = req.addMimePart();
        for (const auto &[name, mime, data] : formFields)
        {
            if (!req.addMimeData(part, name.c_str(), mime.c_str(), data.c_str(), data.size())) [[unlikely]] {
                return {};
            }
        }
    }
    else if (method == Post)
    {
        req.setOpt(CURLOPT_POST, 1L);
        if (rawBody.empty())
        {
            req.setPostFields(params);
            req.setOpt(CURLOPT_POSTFIELDS, req.getPostFieldsData());
            req.setOpt(CURLOPT_POSTFIELDSIZE, req.getPostFieldsSize());
        }
        else
        {
            req.setPostBody(rawBody);
            req.setOpt(CURLOPT_READFUNCTION, on_post_read);
            req.setOpt(CURLOPT_READDATA, req.getPostBody());
            req.setOpt(CURLOPT_POSTFIELDS, nullptr);
            req.setOpt(CURLOPT_POSTFIELDSIZE, req.getPostBodySize());
        }
    }

    DynXXHttpResponse rsp;

    req.setOpt(CURLOPT_WRITEFUNCTION, on_write_rsp_data);
    req.setOpt(CURLOPT_WRITEDATA, &rsp.data);

    req.setOpt(CURLOPT_HEADERFUNCTION, on_write_rsp_headers);
    req.setOpt(CURLOPT_HEADERDATA, &rsp.headers);

    if (!submitReq(req, rsp)) [[unlikely]] {
        return {};
    }
    
    return rsp;
}

bool HttpClient::download(std::string_view url, std::string_view filePath, size_t timeout) const {
    auto req = createReq(url, {}, {}, Get, timeout);
    if (!req.valid()) [[unlikely]]
    {
        return false;
    }

    const auto filePathS = std::string{filePath.data(), filePath.size()};
    std::ofstream file(filePathS.c_str(), std::ios::binary);
    if (!file) [[unlikely]]
    {
        dynxxLogPrintF(Error, "HttpClient.download fopen error:{}", filePath);
        return false;
    }

    req.setOpt(CURLOPT_NOPROGRESS, 1L);
    req.setOpt(CURLOPT_WRITEFUNCTION, on_download_write);
    req.setOpt(CURLOPT_WRITEDATA, &file);

    DynXXHttpResponse rsp;
    if (!submitReq(req, rsp)) [[unlikely]] {
        file.close();
        return false;
    }

    file.close();

    return rsp.code == HTTP_STATUS_OK;
}

} // namespace DynXX::Core::Net

#endif
