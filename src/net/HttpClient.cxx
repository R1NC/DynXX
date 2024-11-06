#include "HttpClient.hxx"

#include "../../../external/curl/include/curl/curl.h"
#include "../../include/NGenXXTypes.h"
#include "../../include/NGenXXLog.hxx"
#include "../../include/NGenXXNetHttp.h"
#include <algorithm>
#include <vector>

size_t _NGenXX_Net_HttpClient_ReadCallback(char *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t nread;
    size_t retcode = std::fread(ptr, size, nmemb, reinterpret_cast<std::FILE *>(stream));
    if (retcode > 0)
    {
        nread = retcode;
        ngenxxLogPrint(NGenXXLogLevelX::Debug, "HttpClient read " + std::to_string(nread) + " bytes from file");
    }
    return retcode;
}

size_t _NGenXX_Net_HttpClient_WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    (reinterpret_cast<std::string *>(userp))->append(contents, size * nmemb);
    return size * nmemb;
}

NGenXX::Net::HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

NGenXX::Net::HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

const NGenXX::Net::HttpResponse NGenXX::Net::HttpClient::request(const std::string &url, const int method,
                                      const std::vector<std::string> &headers,
                                      const std::string &params,
                                      const Bytes &rawBody,
                                      const std::vector<NGenXX::Net::HttpFormField> &formFields,
                                      const std::FILE *cFILE, const size_t fileSize,
                                      const size_t timeout)
{
    HttpResponse rsp;
    int _timeout = timeout;
    if (_timeout <= 0)
    {
        _timeout = NGENXX_HTTP_DEFAULT_TIMEOUT;
    }

    CURL *curl = curl_easy_init();
    curl_mime *mime;
    curl_mimepart *part;
    if (curl)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Debug, "HttpClient.request url: " + url + " params: " + params);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, _timeout);
        curl_easy_setopt(curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, _timeout);

        bool urlAppend = false;
        if (cFILE != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, _NGenXX_Net_HttpClient_ReadCallback);
            curl_easy_setopt(curl, CURLOPT_READDATA, cFILE);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, fileSize);
        } 
        else if (formFields.size() > 0)
        {
            mime = curl_mime_init(curl);
            part = curl_mime_addpart(mime);

            for (auto it = formFields.begin(); it != formFields.end(); it++)
            {
                curl_mime_name(part, it->name.c_str());
                curl_mime_type(part, it->mime.c_str());
                curl_mime_data(part, it->data.c_str(), CURL_ZERO_TERMINATED);
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
            auto [rawData, rawLen] = rawBody;
            if (rawData == nullptr || rawLen == 0) 
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, params.length());
            }
            else
            {
                urlAppend = true;
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rawData);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, rawLen);
            }
        }

        std::string fixedUrl = url;
        if (urlAppend)
        {
            if (url.find("?", 0) == std::string::npos)
            {
                fixedUrl += "?" + params;
            }
            else
            {
                fixedUrl += params;
            }
        }
        curl_easy_setopt(curl, CURLOPT_URL, fixedUrl.c_str());

        struct curl_slist *headerList = NULL;
        for (auto it = headers.begin(); it != headers.end(); ++it)
        {
            ngenxxLogPrint(NGenXXLogLevelX::Debug, "HttpClient.request header: " + (*it));
            headerList = curl_slist_append(headerList, (*it).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        static const std::string prefix("https://");
        auto searchRes = std::mismatch(prefix.begin(), prefix.end(), url.begin());
        if (searchRes.first == prefix.end())
        { // Ignore SSL cet verify
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _NGenXX_Net_HttpClient_WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(rsp.data));

        CURLcode curlCode = curl_easy_perform(curl);

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(rsp.code));
        
        char *contentType;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
        rsp.contentType = contentType;

        if (curlCode != CURLE_OK)
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "HttpClient.request error:" + std::string(curl_easy_strerror(curlCode)));
        }

        curl_easy_cleanup(curl);
    }

    return rsp;
}