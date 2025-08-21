#ifndef DYNXX_SRC_CORE_NET_HTTP_CLIENT_HXX_
#define DYNXX_SRC_CORE_NET_HTTP_CLIENT_HXX_

#if defined(__cplusplus)

#include <functional>

#include <curl/curl.h>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Net.hxx>

namespace DynXX::Core::Net {
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

        [[nodiscard]] DynXXHttpResponse request(std::string_view url, int method,
                                                 const std::vector<std::string> &headers,
                                                std::string_view params,
                                                 const BytesView rawBody,
                                                 const std::vector<HttpFormField> &formFields,
                                                 const std::FILE *cFILE, size_t fileSize,
                                                 size_t timeout);

        [[nodiscard]] bool download(std::string_view url, const std::string_view filePath, size_t timeout);

        ~HttpClient();

    private:
        static bool checkUrlValid(std::string_view url);

        static bool checkUrlHasSearch(std::string_view url);

        bool handleSSL(CURL *curl, std::string_view url) const;

        DynXXHttpResponse req(std::string_view url, const std::vector<std::string> &headers,
                            std::string_view params, int method, size_t timeout,
                               std::function<void(CURL *const, const DynXXHttpResponse &rsp)> &&func) const;
    };
}

#endif

#endif // DYNXX_SRC_CORE_NET_HTTP_CLIENT_HXX_
