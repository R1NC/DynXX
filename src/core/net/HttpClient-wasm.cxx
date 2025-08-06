#if defined(__EMSCRIPTEN__)

#include "HttpClient-wasm.hxx"
#include <NGenXXLog.hxx>
#include <NGenXXNet.h>
#include <NGenXXCoding.hxx>

#include <emscripten/fetch.h>

namespace {
    constexpr auto GET_METHOD = "GET";
    constexpr auto POST_METHOD = "POST";

    std::string buildUrl(const std::string_view baseUrl, const std::string_view params) {
        if (params.empty()) [[unlikely]] {
            return std::string(baseUrl);
        }
        const auto separator = (baseUrl.find('?') == std::string_view::npos) ? '?' : '&';
        std::string result;
        result.reserve(baseUrl.size() + 1 + params.size());
        
        result = baseUrl;
        result += separator;
        result += params;
        return result;
    }

    void setMethod(emscripten_fetch_attr_t& attr, const int method) {
        if (method == NGenXXNetHttpMethodPost) {
            std::copy_n(POST_METHOD, std::strlen(POST_METHOD) + 1, attr.requestMethod);
        } else {
            std::copy_n(GET_METHOD, std::strlen(GET_METHOD) + 1, attr.requestMethod);
        }
    }

    void parseRspHeaders(std::string_view headersStr, std::unordered_map<std::string, std::string>& headers) {
        auto remaining = headersStr;
        
        while (!remaining.empty()) {
            const auto lineEnd = remaining.find('\n');
            const auto lineLength = (lineEnd != std::string_view::npos) ? lineEnd : remaining.size();
            
            auto line = remaining.substr(0, lineLength);
            auto trimmedLine = ngenxxCodingStrTrim(line);
            if (trimmedLine.empty()) [[unlikely]] {
                continue;
            }
            
            const auto colonPos = trimmedLine.find(':');
            if (colonPos == std::string::npos) [[unlikely]] {
                continue;
            }
            auto keyView = std::string_view(trimmedLine).substr(0, colonPos);
            auto valueView = std::string_view(trimmedLine).substr(colonPos + 1);
            auto key = ngenxxCodingStrTrim(keyView);
            auto value = ngenxxCodingStrTrim(valueView);
            if (!key.empty()) [[likely]] {
                headers.emplace(std::move(key), std::move(value));
            }
            
            if (lineEnd == std::string_view::npos) break;
            remaining.remove_prefix(lineEnd + 1);
        }
    }

    void parseResponse(emscripten_fetch_t* fetch, NGenXXHttpResponse& response) {
        if (fetch == nullptr) [[unlikely]] {
            return;
        }

        response.code = fetch->status;
                
        if (fetch->data && fetch->numBytes > 0) [[likely]] {
            response.data.assign(fetch->data, fetch->numBytes);
        }
                
        const auto headersLength = emscripten_fetch_get_response_headers_length(fetch);
        if (headersLength <= 0) [[unlikely]] {
            return;
        }
        std::string headersBuffer(headersLength, '\0');
        const auto actualLength = emscripten_fetch_get_response_headers(fetch, headersBuffer.data(), headersBuffer.size());
        if (actualLength <= 0) [[unlikely]] {
            return;
        }
        headersBuffer.resize(actualLength);

        parseRspHeaders(headersBuffer, response.headers);

        response.contentType = response.headers["Content-Type"];
    }
}

NGenXXHttpResponse NGenXX::Core::Net::WasmHttpClient::request(const std::string_view url, 
                                                 const int method,
                                                 const std::vector<std::string> &headers,
                                                 const std::string_view params,
                                                 const Bytes &rawBody,
                                                 const size_t timeout) {
    NGenXXHttpResponse response;

    auto const fullUrl = buildUrl(url, params);
            
    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "WasmHttpClient.request url: {}", fullUrl);

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.timeoutMSecs = timeout > 0 ? timeout : NGENXX_HTTP_DEFAULT_TIMEOUT;
            
    setMethod(attr, method);
            
    std::vector<const char*> headerPtrs;
    for (const auto& header : headers) {
        headerPtrs.push_back(header.c_str());
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "WasmHttpClient.request header: {}", header);
    }
    headerPtrs.push_back(nullptr);
    if (!headers.empty()) [[likely]] {
        attr.requestHeaders = headerPtrs.data();
    }
            
    if (!rawBody.empty()) [[likely]] {
        attr.requestData = reinterpret_cast<const char*>(rawBody.data());
        attr.requestDataSize = rawBody.size();
    }
            
    auto const fetch = emscripten_fetch(&attr, fullUrl.c_str());
            
    if (fetch) {
        parseResponse(fetch, response);

        emscripten_fetch_close(fetch);
    } else {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "WasmHttpClient.request failed: fetch returned null");
    }
            
    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "WasmHttpClient.request response code: {}", response.code);
            
    return response;
}

#endif // __EMSCRIPTEN__