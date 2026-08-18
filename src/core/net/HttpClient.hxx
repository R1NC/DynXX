#pragma once

#include <curl/curl.h>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Net.hxx>

namespace DynXX::Core::Net {
    struct HttpFormField {
        std::string name;
        std::string mime;
        std::string data;

        HttpFormField() = delete;
        explicit HttpFormField(std::string_view name, std::string_view mime, std::string_view data)
            : name(name), mime(mime), data(data) {}
    };

    class HttpClient {
    public:
        HttpClient();

        HttpClient(const HttpClient &) = delete;

        HttpClient &operator=(const HttpClient &) = delete;

        HttpClient(HttpClient &&) = delete;

        HttpClient &operator=(HttpClient &&) = delete;

        [[nodiscard]] DynXXHttpResponse request(std::string_view url, DynXXHttpMethodX method,
                                                 const std::vector<std::string> &headers,
                                                std::string_view params,
                                                 BytesView rawBody,
                                                 const std::vector<HttpFormField> &formFields,
                                                std::FILE *cFILE, size_t fileSize,
                                                 size_t timeout) const;

        [[nodiscard]] bool download(std::string_view url, std::string_view filePath, size_t timeout) const;

        /**
         * @brief set global CA cert file path, empty disables SSL peer verification
         * @note the path is expected to be already resolved against the engine root
         */
        static void setCertPath(std::string_view path);

        /**
         * @brief set global HTTP proxy, empty host clears the proxy
         */
        static void setProxy(const DynXXHttpProxyConfigX &proxy);

        /**
         * @brief set global DNS config list, empty list clears the configs
         */
        static void setDnsConfigs(const std::vector<DynXXHttpDnsConfigX> &configs);

        ~HttpClient();
    };
}  // namespace DynXX::Core::Net
