#ifndef NGENXX_SRC_NET_HTTP_CLIENT_HXX_
#define NGENXX_SRC_NET_HTTP_CLIENT_HXX_

#if defined(__cplusplus)

#include <functional>

#include <curl/curl.h>

#include <NGenXXTypes.hxx>
#include <NGenXXNetHttp.hxx>

namespace NGenXX::Net
    {
        struct HttpFormField
        {
            std::string name;
            std::string mime;
            std::string data;
        };
        using HttpFormField = HttpFormField;

        class HttpClient
        {
        public:
            HttpClient();
            HttpClient(const HttpClient &) = delete;
            HttpClient &operator=(const HttpClient &) = delete;
            HttpClient(HttpClient &&) = delete;
            HttpClient &operator=(HttpClient &&) = delete;

            [[nodiscard]] NGenXXHttpResponse request(const std::string &url, int method,
                                                            const std::vector<std::string> &headers,
                                                            const std::string &params,
                                                            const Bytes &rawBody,
                                                            const std::vector<HttpFormField> &formFields,
                                                            const std::FILE *cFILE, size_t fileSize,
                                                            size_t timeout);

            [[nodiscard]] bool download(const std::string &url, const std::string &filePath, size_t timeout);

            ~HttpClient();

        private:
            static bool checkUrlValid(const std::string &url);
            static bool checkUrlHasSearch(const std::string &url);
            bool handleSSL(CURL * curl, const std::string &url);
            NGenXXHttpResponse req(const std::string &url, const std::vector<std::string> &headers, const std::string &params, int method, size_t timeout, std::function<void(CURL *const, const NGenXXHttpResponse &rsp)> &&func);
        };
}

#endif

#endif // NGENXX_SRC_NET_HTTP_CLIENT_HXX_
