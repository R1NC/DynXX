#include "HttpClient.hxx"

#include <cstring>

#include <algorithm>
#include <vector>
#include <sstream>

#if defined(USE_STD_RANGES)
#include <ada.h>
#endif

#include <NGenXXLog.hxx>
#include <NGenXXNetHttp.h>
#include <NGenXXCoding.hxx>

size_t _NGenXX_Net_HttpClient_PostReadCallback(char *buffer, size_t size, size_t nmemb, void *userdata)
{
    auto pBytes = static_cast<Bytes *>(userdata);
    auto len = std::min(size * nmemb, pBytes->size());
    if (len > 0)
    {
        std::memcpy(buffer, pBytes->data(), len);
        pBytes->erase(pBytes->begin(), pBytes->begin() + len);
    }
    return len;
}

size_t _NGenXX_Net_HttpClient_UploadReadCallback(char *ptr, size_t size, size_t nmemb, void *stream)
{
    auto ret = std::fread(ptr, size, nmemb, static_cast<std::FILE *>(stream));
    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient read {} bytes from file", ret);
    return ret;
}

size_t _NGenXX_Net_HttpClient_WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    auto pS = static_cast<std::string *>(userp);
    pS->append(contents, size * nmemb);
    return size * nmemb;
}

size_t _NGenXX_Net_HttpClient_RspHeadersCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    auto pHeaders = static_cast<std::unordered_map<std::string, std::string> *>(userdata);
    std::string header(buffer, size * nitems);
    auto colonPos = header.find(':');
    if (colonPos != std::string::npos)
    {
        auto k = header.substr(0, colonPos);
        auto v = header.substr(colonPos + 2);
        pHeaders->emplace(ngenxxCodingStrTrim(k), ngenxxCodingStrTrim(v));
    }
    return size * nitems;
}

NGenXX::Net::HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

NGenXX::Net::HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

NGenXXHttpResponse NGenXX::Net::HttpClient::request(const std::string &url, int method,
                                                                 const std::vector<std::string> &headers,
                                                                 const std::string &params,
                                                                 const Bytes &rawBody,
                                                                 const std::vector<NGenXX::Net::HttpFormField> &formFields,
                                                                 const std::FILE *cFILE, size_t fileSize,
                                                                 size_t timeout)
{
    return this->req(url, headers, params, method, timeout, [&url, method, &params, &rawBody, &formFields, cFILE, fileSize](CURL *const curl, const NGenXXHttpResponse &rsp) {
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.request params: {}", url, params);

        if (cFILE != nullptr)
        {
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, _NGenXX_Net_HttpClient_UploadReadCallback);
            curl_easy_setopt(curl, CURLOPT_READDATA, cFILE);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fileSize);
        }
        else if (!formFields.empty())
        {
            auto mime = curl_mime_init(curl);
            auto part = curl_mime_addpart(mime);

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
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, params.length());
            }
            else
            {
                curl_easy_setopt(curl, CURLOPT_READFUNCTION, _NGenXX_Net_HttpClient_PostReadCallback);
                curl_easy_setopt(curl, CURLOPT_READDATA, &rawBody);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, rawBody.size());
            }
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _NGenXX_Net_HttpClient_WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(rsp.data));

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _NGenXX_Net_HttpClient_RspHeadersCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(rsp.headers));
    });
}

bool NGenXX::Net::HttpClient::download(const std::string &url, const std::string &filePath, size_t timeout)
{
    auto file = std::fopen(filePath.c_str(), "wb");
    if (!file)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "HttpClient.download fopen error");
        return false;
    }

    auto rsp = this->req(url, {}, {}, NGenXXNetHttpMethodGet, timeout, [file](CURL *const curl, const NGenXXHttpResponse &rsp) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, std::fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    });

    std::fclose(file);

    return rsp.code == 200;
}

NGenXXHttpResponse NGenXX::Net::HttpClient::req(const std::string &url, const std::vector<std::string> &headers, const std::string &params, int method, size_t timeout, std::function<void(CURL *const, const NGenXXHttpResponse &rsp)> &&func)
{
    if (!this->checkUrlValid(url))
    {
        return {};
    }

    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.createRequest url: {}", url);

    auto curl = curl_easy_init();
    if (!curl) [[unlikely]]
    {
        return {};
    }

    if (!this->handleSSL(curl, url))
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

    struct curl_slist *headerList = nullptr;
    for (const auto &it : headers)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.request header: {}", it);
        headerList = curl_slist_append(headerList, it.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    std::stringstream ssUrl;
    ssUrl << url;
    if (method == NGenXXNetHttpMethodGet && !params.empty())
    {
        if (!this->checkUrlHasSearch(url))
        {
            ssUrl << "?";
        }
        ssUrl << params;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    NGenXXHttpResponse rsp;
    auto cbk = std::move(func);
    cbk(curl, rsp);

    auto curlCode = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(rsp.code));

    char *contentType;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
    if (contentType)
    {
        rsp.contentType = contentType;
    }

    if (curlCode != CURLE_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient.req error:{}", curl_easy_strerror(curlCode));
    }

    curl_easy_cleanup(curl);

    return rsp;
}

bool NGenXX::Net::HttpClient::checkUrlValid(const std::string &url)
{
    if (url.empty()) [[unlikely]]
    {
        return false;
    }
#if defined(USE_STD_RANGES)
    auto aUrl = ada::parse(url);
    if (!aUrl)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient INVALID URL: {}", url);
        return false;
    }
#endif
    return true;
}

bool NGenXX::Net::HttpClient::checkUrlHasSearch(const std::string &url)
{
#if defined(USE_STD_RANGES)
    auto aUrl = ada::parse(url);
    return !aUrl->get_search().empty();
#else
    return url.find("?", 0) != std::string::npos;
#endif
}

bool NGenXX::Net::HttpClient::handleSSL(CURL *const curl, const std::string &url)
{
#if defined(USE_STD_RANGES)
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
