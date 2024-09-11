#ifdef __cplusplus

#include "../../../external/curl/include/curl/curl.h"
#include "../../include/NGenXXLog.h"
#include "../../include/NGenXXNetHttp.h"
#include "../log/Log.hxx"
#include <algorithm>
#include <vector>
#include <string>

#define DEFAULT_TIMEOUT 5000L

namespace NGenXX
{
    namespace Net
    {
        namespace HttpClient
        {
            size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
            {
                ((std::string *)userp)->append((char *)contents, size * nmemb);
                return size * nmemb;
            }

            void create(void)
            {
                curl_global_init(CURL_GLOBAL_DEFAULT);
            }

            void destroy(void)
            {
                curl_global_cleanup();
            }

            const std::string request(const std::string &url, const std::string &params, int method, std::vector<std::string> &headers, long timeout)
            {
                std::string rsp;
                if (timeout <= 0)
                {
                    timeout = DEFAULT_TIMEOUT;
                }

                CURL *curl = curl_easy_init();
                if (curl)
                {
                    Log::print(NGenXXLogLevelDebug, ("HttpClient.request url: " + url + " params: " + params).c_str());

                    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout);
                    curl_easy_setopt(curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, timeout);

                    if (method == Post)
                    {
                        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
                    }
                    else
                    {
                        auto loc = url.find("?", 0);
                        if (loc == std::string::npos)
                        {
                            curl_easy_setopt(curl, CURLOPT_URL, (url + "?" + params).c_str());
                        }
                        else
                        {
                            curl_easy_setopt(curl, CURLOPT_URL, (url + params).c_str());
                        }
                    }

                    struct curl_slist *headerList = NULL;
                    for (auto it = headers.begin(); it != headers.end(); ++it)
                    {
                        Log::print(NGenXXLogLevelDebug, ("HttpClient.request header: " + (*it)).c_str());
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

                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rsp);

                    CURLcode curlCode = curl_easy_perform(curl);
                    if (curlCode != CURLE_OK)
                    {
                        Log::print(NGenXXLogLevelError, ("HttpClient.request error:" + std::string(curl_easy_strerror(curlCode))).c_str());
                    }

                    curl_easy_cleanup(curl);
                }

                return rsp;
            }
        }
    }
}

#endif