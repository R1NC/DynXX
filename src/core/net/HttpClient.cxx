#include <cstdint>
#if defined(USE_CURL)

#include "HttpClient.hxx"

#include <algorithm>
#include <fstream>

#if defined(USE_ADA)
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

    const char *errMsg(CURLcode code) {
        return curl_easy_strerror(code);
    }

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

    class Req {
    private:
            CURL *curl{nullptr};
            curl_slist *headers{nullptr};
            curl_mime *mime{nullptr};

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

        bool valid() const {
            return this->curl != nullptr;
        }
        
        bool appendHeader(const char *string) {
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

        curl_mimepart *addMimePart() {
            if (this->mime == nullptr) [[unlikely]] {
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

        bool addMimeData(curl_mimepart *part, const char *name, const char *type, const char *dataP, size_t dataLen) const {
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

        bool perform() {
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
        bool getInfo(CURLINFO info, T value) {
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
            this->mime = std::exchange(other.mime, nullptr);
        }

        void cleanup() {
            if (this->headers != nullptr) {
                curl_slist_free_all(this->headers);
                this->headers = nullptr;
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

    bool checkUrlValid(std::string_view url)
    {
        if (url.empty()) [[unlikely]]
        {
            return false;
        }
#if defined(USE_ADA)
        if (!ada::parse(url)) [[unlikely]]
        {
            dynxxLogPrintF(Error, "HttpClient INVALID URL: {}", url);
            return false;
        }
#endif
        return true;
    }

    bool checkUrlHasSearch(std::string_view url)
    {
        return url.find('?', 0) != std::string::npos;
    }

    bool handleSSL(Req &req, std::string_view url)
    {
#if defined(USE_ADA)
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
#if defined(ENABLE_SSL_CERT)
            req.setOpt(CURLOPT_SSL_VERIFYPEER, 1L);
            req.setOpt(CURLOPT_SSL_VERIFYHOST, 2L);
#else
            req.setOpt(CURLOPT_SSL_VERIFYPEER, 0L);
            req.setOpt(CURLOPT_SSL_VERIFYHOST, 0L);
#endif
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
            req.appendHeader(it.c_str());
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
            fixedUrl += params;
        }
    
        dynxxLogPrintF(Debug, "HttpClient.req url: {}", fixedUrl);
        req.setOpt(CURLOPT_URL, fixedUrl.c_str());

        return req;
    }

    bool submitReq(Req &req, DynXXHttpResponse &rsp)
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
            req.addMimeData(part, name.c_str(), mime.c_str(), data.c_str(), data.size());
        }
    }
    else if (method == Post)
    {
        req.setOpt(CURLOPT_POST, 1L);
        if (rawBody.empty())
        {
            const auto paramsS = std::string{params.data(), params.size()};
            req.setOpt(CURLOPT_POSTFIELDS, paramsS.c_str());
            req.setOpt(CURLOPT_POSTFIELDSIZE, paramsS.size());
        }
        else
        {
            req.setOpt(CURLOPT_READFUNCTION, on_post_read);
            req.setOpt(CURLOPT_READDATA, &rawBody);
            req.setOpt(CURLOPT_POSTFIELDS, nullptr);
            req.setOpt(CURLOPT_POSTFIELDSIZE, rawBody.size());
        }
    }

    DynXXHttpResponse rsp;

    req.setOpt(CURLOPT_WRITEFUNCTION, on_write_rsp_data);
    req.setOpt(CURLOPT_WRITEDATA, &rsp.data);

    req.setOpt(CURLOPT_HEADERFUNCTION, on_write_rsp_headers);
    req.setOpt(CURLOPT_HEADERDATA, &rsp.headers);

    submitReq(req, rsp);
    
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
    submitReq(req, rsp);

    file.close();

    return rsp.code == HTTP_STATUS_OK;
}

} // namespace DynXX::Core::Net

#endif
