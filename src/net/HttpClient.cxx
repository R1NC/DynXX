#include "HttpClient.hxx"

#include <cstring>

#include <algorithm>
#include <vector>
#include <sstream>

#if defined(USE_ADA_URL)
#include <ada.h>
#endif

#include <NGenXXTypes.h>
#include <NGenXXLog.hxx>
#include <NGenXXNetHttp.h>

size_t _NGenXX_Net_HttpClient_PostReadCallback(char *buffer, size_t size, size_t nmemb, void *userdata)
{
    auto bytes = static_cast<Bytes *>(userdata);
    auto len = std::min(size * nmemb, bytes->size());
    if (len > 0)
    {
        std::memcpy(buffer, bytes->data(), len);
        bytes->erase(bytes->begin(), bytes->begin() + len);
    }
    return len;
}

size_t _NGenXX_Net_HttpClient_UploadReadCallback(char *ptr, size_t size, size_t nmemb, void *stream)
{
    auto ret = std::fread(ptr, size, nmemb, reinterpret_cast<std::FILE *>(stream));
    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient read {} bytes from file", ret);
    return ret;
}

size_t _NGenXX_Net_HttpClient_WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    (reinterpret_cast<std::string *>(userp))->append(contents, size * nmemb);
    return size * nmemb;
}

size_t _NGenXX_Net_HttpClient_RspHeadersCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    auto headers = static_cast<std::unordered_map<std::string, std::string> *>(userdata);
    std::string header(buffer, size * nitems);
    // Split K & V
    auto colonPos = header.find(':');
    if (colonPos != std::string::npos)
    {
        // Skip colon and space
        auto k = header.substr(0, colonPos);
        auto v = header.substr(colonPos + 2);
        // Trim whitespace around K & V
        k.erase(0, k.find_first_not_of(" \t"));
        k.erase(k.find_last_not_of(" \t") + 1);
        k.erase(0, k.find_first_not_of(" \t"));
        k.erase(k.find_last_not_of(" \t") + 1);
        (*headers).emplace(std::move(k), std::move(v));
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

NGenXX::Net::HttpResponse NGenXX::Net::HttpClient::request(const std::string &url, const int method,
                                                                 const std::vector<std::string> &headers,
                                                                 const std::string &params,
                                                                 const Bytes &rawBody,
                                                                 const std::vector<NGenXX::Net::HttpFormField> &formFields,
                                                                 const std::FILE *cFILE, const size_t fileSize,
                                                                 const size_t timeout)
{
    HttpResponse rsp;

    if (!this->checkUrlValid(url))
    {
        return rsp;
    }

    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.request url: {} params: {}", url, params);

    auto curl = curl_easy_init();
    if (!curl) [[unlikely]]
    {
        return rsp;
    }

    if (!this->handleSSL(curl, url))
    {
        return rsp;
    }

    auto _timeout = timeout;
    if (_timeout == 0)
    {
        _timeout = NGENXX_HTTP_DEFAULT_TIMEOUT;
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, _timeout);
    curl_easy_setopt(curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, _timeout);

    auto urlAppend = false;
    if (cFILE != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, _NGenXX_Net_HttpClient_UploadReadCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, cFILE);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fileSize);
    }
    else if (formFields.size() > 0)
    {
        auto mime = curl_mime_init(curl);
        auto part = curl_mime_addpart(mime);

        for (const auto& [name, mime, data] : formFields)
        {
            curl_mime_name(part, name.c_str());
            curl_mime_type(part, mime.c_str());
            curl_mime_data(part, data.c_str(), CURL_ZERO_TERMINATED);
        }

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    }
    else if (method == NGenXXNetHttpMethodGet)
    {
        urlAppend = true;
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
            urlAppend = true;
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, _NGenXX_Net_HttpClient_PostReadCallback);
            curl_easy_setopt(curl, CURLOPT_READDATA, &rawBody);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, rawBody.size());
        }
    }

    std::stringstream ssUrl;
    ssUrl << url;
    if (urlAppend)
    {
        if (!this->checkUrlHasSearch(url))
        {
            ssUrl << "?";
        }
        ssUrl << params;
    }
    curl_easy_setopt(curl, CURLOPT_URL, ssUrl.str().c_str());

    struct curl_slist *headerList = NULL;
    for (const auto &it : headers)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "HttpClient.request header: {}", it);
        headerList = curl_slist_append(headerList, it.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _NGenXX_Net_HttpClient_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(rsp.data));

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _NGenXX_Net_HttpClient_RspHeadersCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(rsp.headers));

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
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient.request error:{}", curl_easy_strerror(curlCode));
    }

    curl_easy_cleanup(curl);

    return rsp;
}

bool NGenXX::Net::HttpClient::download(const std::string &url, const std::string &filePath, const size_t timeout)
{
    auto res = false;

    if (!this->checkUrlValid(url))
    {
        return res;
    }

    auto curl = curl_easy_init();
    if (!curl) [[unlikely]]
    {
        return res;
    }

    if (!this->handleSSL(curl, url))
    {
        return res;
    }

    auto _timeout = timeout;
    if (_timeout == 0) [[unlikely]]
    {
        _timeout = NGENXX_HTTP_DEFAULT_TIMEOUT;
    }

    auto file = std::fopen(filePath.c_str(), "wb");
    if (!file)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "HttpClient.download fopen error");
        return res;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout);
    curl_easy_setopt(curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, timeout);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, std::fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    auto curlCode = curl_easy_perform(curl);
    if (curlCode == CURLE_OK)
    {
        res = true;
    }
    else
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient.download error:{}", curl_easy_strerror(curlCode));
    }

    curl_easy_cleanup(curl);

    std::fclose(file);

    return res;
}

bool NGenXX::Net::HttpClient::checkUrlValid(const std::string &url)
{
    if (url.empty())
    {
        return false;
    }
#if defined(USE_ADA_URL)
    auto aUrl = ada::parse(url);
    if (!aUrl)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "HttpClient.request INVALID URL: {}", url);
        return false;
    }
#endif
    return true;
}

bool NGenXX::Net::HttpClient::checkUrlHasSearch(const std::string &url)
{
#if defined(USE_ADA_URL)
    auto aUrl = ada::parse(url);
    return !aUrl->get_search().empty();
#else
    return url.find("?", 0) != std::string::npos;
#endif
}

bool NGenXX::Net::HttpClient::handleSSL(CURL *const curl, const std::string &url)
{
#if defined(USE_ADA_URL)
    auto aUrl = ada::parse(url);
    static const std::string protocolHttps = "https:";
    if (aUrl->get_protocol() == protocolHttps)
#else
    static const std::string prefixHttps = "https://";
    if (url.starts_with(prefixHttps))
#endif
    { // TODO: verify SSL cet
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    return true;
}
