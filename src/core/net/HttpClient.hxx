#ifndef NGENXX_SRC_CORE_NET_HTTP_CLIENT_HXX_
#define NGENXX_SRC_CORE_NET_HTTP_CLIENT_HXX_

#if defined(__cplusplus)

#include <functional>

#include <curl/curl.h>

#include <NGenXXTypes.hxx>
#include <NGenXXNet.hxx>

namespace NGenXX::Core::Net {
    struct HttpFormField {
        std::string name;
        std::string mime;
        std::string data;
    };

    using HttpFormField = HttpFormField;

    class HttpClient {
    public:
        HttpClient();

        HttpClient(const HttpClient &) = delete;

        HttpClient &operator=(const HttpClient &) = delete;

        HttpClient(HttpClient &&) = delete;

        HttpClient &operator=(HttpClient &&) = delete;

        [[nodiscard]] NGenXXHttpResponse request(std::string_view url, int method,
                                                 const std::vector<std::string> &headers,
                                                std::string_view params,
                                                 const Bytes &rawBody,
                                                 const std::vector<HttpFormField> &formFields,
                                                 const std::FILE *cFILE, size_t fileSize,
                                                 size_t timeout);

        [[nodiscard]] bool download(std::string_view url, const std::string &filePath, size_t timeout);

        ~HttpClient();

    private:
        static bool checkUrlValid(std::string_view url);

        static bool checkUrlHasSearch(std::string_view url);

        bool handleSSL(CURL *curl, std::string_view url);

        NGenXXHttpResponse req(std::string_view url, const std::vector<std::string> &headers,
                            std::string_view params, int method, size_t timeout,
                               std::function<void(CURL *const, const NGenXXHttpResponse &rsp)> &&func);
    };
}

#endif

#endif // NGENXX_SRC_CORE_NET_HTTP_CLIENT_HXX_
