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

        struct Req {
        CURL *curl{nullptr};
        curl_slist *headers{nullptr};
        curl_mime *mime{nullptr};

        Req() = default;
        Req(const Req&) = delete;
        Req& operator=(const Req&) = delete;
        
        ~Req() {
            cleanup();
        }
        
        Req(Req&& other) noexcept {
            this->moveImp(std::move(other));
        }
        
        Req& operator=(Req&& other) noexcept {
            if (this != &other) {
                cleanup();
                this->moveImp(std::move(other));
            }
            return *this;
        }
        
    private:
        void moveImp(Req&& other) noexcept {
            this->curl = std::exchange(other.curl, nullptr);
            this->headers = std::exchange(other.headers, nullptr);
            this->mime = std::exchange(other.mime, nullptr);
        }

        void cleanup() {
            if (this->headers) {
                curl_slist_free_all(this->headers);
                this->headers = nullptr;
            }
            if (this->mime) {
                curl_mime_free(this->mime);
                this->mime = nullptr;
            }
            if (this->curl) {
                curl_easy_cleanup(this->curl);
                this->curl = nullptr;
            }
        }
    };

    size_t on_post_read(char *buffer, const size_t size, size_t nmemb, RawPtr userdata)
    {
        const auto pBytes = static_cast<Bytes *>(userdata);
        const auto len = static_cast<long>(std::min<size_t>(size * nmemb, pBytes->size()));
        if (len > 0) [[likely]]
        {
            std::memcpy(buffer, pBytes->data(), len);
            pBytes->erase(pBytes->begin(), pBytes->begin() + len);
        }
        return len;
    }

    size_t on_upload_read(char *ptr, const size_t size, size_t nmemb, RawPtr stream)
    {
        const auto ret = std::fread(ptr, size, nmemb, static_cast<std::FILE *>(stream));
        dynxxLogPrintF(Debug, "HttpClient read {} bytes from file", ret);
        return ret;
    }

    size_t on_download_write(const char *contents, const size_t size, const size_t nmemb, RawPtr userp) {
        auto& file = *static_cast<std::ofstream*>(userp);
        file.write(contents, size * nmemb);
        const auto ret = file.good() ? size * nmemb : 0;
        dynxxLogPrintF(Debug, "HttpClient write {} bytes to file", ret);
        return ret;
    }

    size_t on_write(const char *contents, const size_t size, size_t nmemb, RawPtr userp)
    {
        const auto pS = static_cast<std::string *>(userp);
        pS->append(contents, size * nmemb);
        return size * nmemb;
    }

    size_t on_handle_rsp_headers(const char *buffer, const size_t size, size_t nitems, RawPtr userdata)
    {
        const auto pHeaders = static_cast<Dict *>(userdata);
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
#if defined(USE_ADA)
        auto aUrl = ada::parse(url);
        return aUrl && !aUrl->get_search().empty();
#else
        return url.find('?', 0) != std::string::npos;
#endif
    }

    bool handleSSL(CURL * curl, std::string_view url)
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
        { // TODO: verify SSL cet
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        return true;
    }

    Req createReq(std::string_view url, const std::vector<std::string> &headers,
                            std::string_view params, int method, size_t timeout)
    {
        Req req;
        if (!checkUrlValid(url)) [[unlikely]]
        {
            return req;
        }

        req.curl = curl_easy_init();
        if (!req.curl) [[unlikely]]
        {
            return req;
        }

        if (!handleSSL(req.curl, url)) [[unlikely]]
        {
            return req;
        }

        auto _timeout = timeout;
        if (_timeout == 0) [[unlikely]]
        {
            _timeout = DYNXX_HTTP_DEFAULT_TIMEOUT;
        }
        curl_easy_setopt(req.curl, CURLOPT_CONNECTTIMEOUT_MS, _timeout);
        curl_easy_setopt(req.curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, _timeout);

        curl_easy_setopt(req.curl, CURLOPT_FOLLOWLOCATION, 1L);//allow redirect
        curl_easy_setopt(req.curl, CURLOPT_USERAGENT, "DynXX");
        curl_easy_setopt(req.curl, CURLOPT_HTTPGET, method == DynXXNetHttpMethodGet ? 1L : 0L);

        for (const auto &it : headers)
        {
            dynxxLogPrintF(Debug, "HttpClient.req header: {}", it);
            req.headers = curl_slist_append(req.headers, it.c_str());
        }
        curl_easy_setopt(req.curl, CURLOPT_HTTPHEADER, req.headers);

        std::string fixedUrl;
        fixedUrl.reserve(url.size() + (method == DynXXNetHttpMethodGet && !params.empty() ? params.size() + 1 : 0));
        fixedUrl = url;
        if (method == DynXXNetHttpMethodGet && !params.empty())
        {
            if (!checkUrlHasSearch(fixedUrl))
            {
                fixedUrl += "?";
            }
            fixedUrl += params;
        }
    
        dynxxLogPrintF(Debug, "HttpClient.req url: {}", fixedUrl);
        curl_easy_setopt(req.curl, CURLOPT_URL, fixedUrl.c_str());

        return req;
    }

    void submitReq(const Req &req, DynXXHttpResponse &rsp)
    {
        auto curlCode = curl_easy_perform(req.curl);

        curl_easy_getinfo(req.curl, CURLINFO_RESPONSE_CODE, &(rsp.code));

        char *contentType;
        curl_easy_getinfo(req.curl, CURLINFO_CONTENT_TYPE, &contentType);
        if (contentType)
        {
            rsp.contentType = contentType;
        }

        if (curlCode != CURLE_OK) [[unlikely]]
        {
            dynxxLogPrintF(Error, "HttpClient.req error:{}", curl_easy_strerror(curlCode));
        }
    }
}

DynXX::Core::Net::HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

DynXX::Core::Net::HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

DynXXHttpResponse DynXX::Core::Net::HttpClient::request(std::string_view url, int method,
                                                                 const std::vector<std::string> &headers,
                                                                std::string_view params,
                                                                 BytesView rawBody,
                                                                 const std::vector<HttpFormField> &formFields,
                                                                std::FILE *cFILE, size_t fileSize,
                                                                 size_t timeout) const {

    auto req = createReq(url, headers, params, method, timeout);
    if (!req.curl) [[unlikely]]
    {
        return {};
    }

    if (cFILE != nullptr)
    {
        curl_easy_setopt(req.curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(req.curl, CURLOPT_READFUNCTION, on_upload_read);
        curl_easy_setopt(req.curl, CURLOPT_READDATA, cFILE);
        curl_easy_setopt(req.curl, CURLOPT_INFILESIZE_LARGE, fileSize);
    }
    else if (!formFields.empty())
    {
        req.mime = curl_mime_init(req.curl);
        const auto part = curl_mime_addpart(req.mime);

        for (const auto &[name, mime, data] : formFields)
        {
            curl_mime_name(part, name.c_str());
            curl_mime_type(part, mime.c_str());
            curl_mime_data(part, data.c_str(), CURL_ZERO_TERMINATED);
        }

        curl_easy_setopt(req.curl, CURLOPT_MIMEPOST, req.mime);
    }
    else if (method == DynXXNetHttpMethodPost)
    {
        curl_easy_setopt(req.curl, CURLOPT_POST, 1L);
        if (rawBody.empty())
        {
            curl_easy_setopt(req.curl, CURLOPT_POSTFIELDS, params.data());
            curl_easy_setopt(req.curl, CURLOPT_POSTFIELDSIZE, params.size());
        }
        else
        {
            curl_easy_setopt(req.curl, CURLOPT_READFUNCTION, on_post_read);
            curl_easy_setopt(req.curl, CURLOPT_READDATA, &rawBody);
            curl_easy_setopt(req.curl, CURLOPT_POSTFIELDS, nullptr);
            curl_easy_setopt(req.curl, CURLOPT_POSTFIELDSIZE, rawBody.size());
        }
    }

    DynXXHttpResponse rsp;

    curl_easy_setopt(req.curl, CURLOPT_WRITEFUNCTION, on_write);
    curl_easy_setopt(req.curl, CURLOPT_WRITEDATA, &rsp.data);

    curl_easy_setopt(req.curl, CURLOPT_HEADERFUNCTION, on_handle_rsp_headers);
    curl_easy_setopt(req.curl, CURLOPT_HEADERDATA, &rsp.headers);

    submitReq(req, rsp);
    
    return rsp;
}

bool DynXX::Core::Net::HttpClient::download(std::string_view url, std::string_view filePath, size_t timeout) const {
    auto req = createReq(url, {}, {}, DynXXNetHttpMethodGet, timeout);
    if (!req.curl) [[unlikely]]
    {
        return false;
    }

    std::ofstream file(filePath.data(), std::ios::binary);
    if (!file) [[unlikely]]
    {
        dynxxLogPrintF(Error, "HttpClient.download fopen error:{}", filePath);
        return false;
    }

    curl_easy_setopt(req.curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(req.curl, CURLOPT_WRITEFUNCTION, on_download_write);
    curl_easy_setopt(req.curl, CURLOPT_WRITEDATA, &file);

    DynXXHttpResponse rsp;
    submitReq(req, rsp);

    file.close();

    return rsp.code == 200;
}

#endif