#ifdef __cplusplus

#include "../../../external/curl/include/curl/curl.h"
#include <iostream>
#include <algorithm>

namespace EngineXX
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

            const std::string Request(const std::string &url, const std::string &params)
            {
                std::string rsp;

                curl_global_init(CURL_GLOBAL_DEFAULT);
                CURL *curl = curl_easy_init();
                if (curl)
                {
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());

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
                        std::cout << stderr << "curl_easy_perform() error:" << curl_easy_strerror(curlCode) << std::endl;
                    }

                    curl_easy_cleanup(curl);
                }
                curl_global_cleanup();

                return rsp;
            }
        }
    }
}

#endif