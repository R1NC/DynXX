#include "HttpClient.hxx"

#include <algorithm>

#if defined(USE_ADA)
#include <ada.h>
#endif

#include <NGenXXLog.hxx>
#include <NGenXXNet.h>
#include <NGenXXCoding.hxx>

namespace
{
    size_t on_post_read(char *buffer, const size_t size, size_t nmemb, void *userdata)
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

    size_t on_upload_read(char *ptr, const size_t size, size_t nmemb, void *stream)
    {
        const auto ret = std::fread(ptr, size, nmemb, static_cast<std::FILE *>(stream));
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient read {} bytes from file", ret);
        return ret;
    }

    size_t on_write(const char *contents, const size_t size, size_t nmemb, void *userp)
    {
        const auto pS = static_cast<std::string *>(userp);
        pS->append(contents, size * nmemb);
        return size * nmemb;
    }

    size_t on_handle_rsp_headers(const char *buffer, const size_t size, size_t nitems, void *userdata)
    {
        const auto pHeaders = static_cast<std::unordered_map<std::string, std::string> *>(userdata);
        std::string header(buffer, size * nitems);
        if (const auto colonPos = header.find(':'); colonPos != std::string::npos) [[likely]]
        {
            const auto k = header.substr(0, colonPos);
            const auto v = header.substr(colonPos + 2);
            pHeaders->emplace(ngenxxCodingStrTrim(k), ngenxxCodingStrTrim(v));
        }
        return size * nitems;
    }
}

NGenXX::Core::Net::HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

NGenXX::Core::Net::HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

NGenXXHttpResponse NGenXX::Core::Net::HttpClient::request(std::string_view url, int method,
                                                                 const std::vector<std::string> &headers,
                                                                std::string_view params,
                                                                 const Bytes &rawBody,
                                                                 const std::vector<HttpFormField> &formFields,
                                                                 const std::FILE *cFILE, size_t fileSize,
                                                                 size_t timeout) {
    return this->req(url, headers, params, method, timeout, [&url, method, &params, &rawBody, &formFields, cFILE, fileSize](CURL *const curl, const NGenXXHttpResponse &rsp) {
        if (cFILE != nullptr)
        {
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, on_upload_read);
            curl_easy_setopt(curl, CURLOPT_READDATA, cFILE);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fileSize);
        }
        else if (!formFields.empty())
        {
            const auto mime = curl_mime_init(curl);
            const auto part = curl_mime_addpart(mime);

            for (const auto &[name, mime, data] : formFields)
            {
                curl_mime_name(part, name.c_str());
                curl_mime_type(part, mime.c_str());
                curl_mime_data(part, data.c_str(), CURL_ZERO_TERMINATED);
            }

            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        }
        else if (method == NGenXXNetHttpMethodPost)
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (rawBody.empty())
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.data());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, params.size());
            }
            else
            {
                curl_easy_setopt(curl, CURLOPT_READFUNCTION, on_post_read);
                curl_easy_setopt(curl, CURLOPT_READDATA, &rawBody);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, rawBody.size());
            }
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rsp.data);

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, on_handle_rsp_headers);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rsp.headers);
    });
}

bool NGenXX::Core::Net::HttpClient::download(std::string_view url, const std::string &filePath, size_t timeout) {
    auto file = std::fopen(filePath.c_str(), "wb");
    if (!file) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "HttpClient.download fopen error");
        return false;
    }

    const auto rsp = this->req(url, {}, {}, NGenXXNetHttpMethodGet, timeout, [file](CURL *const curl, const NGenXXHttpResponse &) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, std::fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    });

    std::fclose(file);

    return rsp.code == 200;
}

NGenXXHttpResponse NGenXX::Core::Net::HttpClient::req(std::string_view url, const std::vector<std::string> &headers, std::string_view params, int method, size_t timeout, std::function<void(CURL *const, const NGenXXHttpResponse &rsp)> &&func) {
    if (!checkUrlValid(url)) [[unlikely]]
    {
        return {};
    }

    auto curl = curl_easy_init();
    if (!curl) [[unlikely]]
    {
        return {};
    }

    if (!this->handleSSL(curl, url)) [[unlikely]]
    {
        return {};
    }

    auto _timeout = timeout;
    if (_timeout == 0) [[unlikely]]
    {
        _timeout = NGENXX_HTTP_DEFAULT_TIMEOUT;
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, _timeout);
    curl_easy_setopt(curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, _timeout);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//allow redirect
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, "NGenXX");
    curl_easy_setopt(curl, CURLOPT_HTTPGET, method == NGenXXNetHttpMethodGet ? 1L : 0L);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    curl_slist *headerList = nullptr;
    for (const auto &it : headers)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.req header: {}", it);
        headerList = curl_slist_append(headerList, it.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    std::string fixedUrl;
    fixedUrl.reserve(url.size() + (method == NGenXXNetHttpMethodGet && !params.empty() ? params.size() + 1 : 0));
    fixedUrl = url;
    if (method == NGenXXNetHttpMethodGet && !params.empty())
    {
        if (!checkUrlHasSearch(fixedUrl))
        {
            fixedUrl += "?";
        }
        fixedUrl += params;
    }
    
    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.req url: {}", fixedUrl);
    curl_easy_setopt(curl, CURLOPT_URL, fixedUrl.c_str());

    NGenXXHttpResponse rsp;
    std::move(func)(curl, rsp);

    auto curlCode = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(rsp.code));

    char *contentType;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
    if (contentType)
    {
        rsp.contentType = contentType;
    }

    if (curlCode != CURLE_OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient.req error:{}", curl_easy_strerror(curlCode));
    }

    curl_easy_cleanup(curl);

    return rsp;
}

bool NGenXX::Core::Net::HttpClient::checkUrlValid(std::string_view url)
{
    if (url.empty()) [[unlikely]]
    {
        return false;
    }
#if defined(USE_ADA)
    auto aUrl = ada::parse(url);
    if (!aUrl) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient INVALID URL: {}", url);
        return false;
    }
#endif
    return true;
}

bool NGenXX::Core::Net::HttpClient::checkUrlHasSearch(std::string_view url)
{
#if defined(USE_ADA)
    auto aUrl = ada::parse(url);
    return !aUrl->get_search().empty();
#else
    return url.find('?', 0) != std::string::npos;
#endif
}

bool NGenXX::Core::Net::HttpClient::handleSSL(CURL * curl, std::string_view url)
{
#if defined(USE_ADA)
    auto aUrl = ada::parse(url);
    static const auto protocolHttps = "https:";
    if (aUrl->get_protocol() == protocolHttps)
#else
    static const auto prefixHttps = "https://";
    if (url.starts_with(prefixHttps))
#endif
    { // TODO: verify SSL cet
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    return true;
}
