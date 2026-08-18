#pragma once

#include "Types.hxx"
#include <cstdio>

constexpr auto DynXXHttpDefaultTimeout = 15 * 1000;

enum class DynXXHttpMethodX : uint8_t {
    Get,
    Post,
    Put,
};

struct DynXXHttpResponse {
    std::string contentType;
    std::string data;
    Dict headers;
    int code{0};

    std::optional<std::string> toJson() const;
};
#if !defined(_WIN32)
AssertMove(DynXXHttpResponse);
#endif

/**
 * @brief global HTTP proxy config
 */
struct DynXXHttpProxyConfigX {
    std::string host;      ///< proxy host, empty means no proxy
    uint16_t port{0};      ///< proxy port, 0 means not appended to host
    std::string username;  ///< proxy auth username, optional
    std::string password;  ///< proxy auth password, optional
};

/**
 * @brief global DNS config, map a host to a fixed address like `/etc/hosts`
 */
struct DynXXHttpDnsConfigX {
    std::string host;     ///< host name to override
    uint16_t port{0};     ///< port, 0 means any port
    std::string address;  ///< fixed IP address
};

/**
 * @brief set global CA cert file path, relative to the engine root
 * @param path cert file path relative to the root, an absolute path is used
 *             as-is, empty disables SSL peer verification
 */
void dynxxNetHttpSetCertPath(std::string_view path);

/**
 * @brief set global HTTP proxy
 * @param proxy proxy config, empty host clears the proxy
 */
void dynxxNetHttpSetProxy(const DynXXHttpProxyConfigX &proxy);

/**
 * @brief set global DNS config list
 * @param configs DNS config list, empty list clears the configs
 */
void dynxxNetHttpSetDnsConfigs(const std::vector<DynXXHttpDnsConfigX> &configs);
DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        std::string_view params,
                                        BytesView rawBody = {},
                                        const std::vector<std::string> &headerV = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = DynXXHttpDefaultTimeout);

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        const DictAny &params,
                                        BytesView rawBody = {},
                                        const Dict &headers = {},
                                        const std::vector<std::string> &formFieldNameV = {},
                                        const std::vector<std::string> &formFieldMimeV = {},
                                        const std::vector<std::string> &formFieldDataV = {},
                                        std::FILE *cFILE = nullptr, size_t fileSize = 0,
                                        size_t timeout = DynXXHttpDefaultTimeout);

[[nodiscard]] bool dynxxNetHttpDownload(std::string_view url, std::string_view filePath,
                           size_t timeout = DynXXHttpDefaultTimeout);
