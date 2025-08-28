#if defined(__EMSCRIPTEN__)

#include "HttpClient-wasm.hxx"
#include <DynXX/CXX/Log.hxx>
#include <DynXX/C/Net.h>
#include <DynXX/CXX/Coding.hxx>

#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>

namespace {
    constexpr auto GET_METHOD = "GET";
    constexpr auto POST_METHOD = "POST";
    constexpr auto SLEEP_MS = 5;

    using enum DynXXLogLevelX;

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
        if (method == DynXXNetHttpMethodPost) {
            std::copy_n(POST_METHOD, std::strlen(POST_METHOD) + 1, attr.requestMethod);
        } else {
            std::copy_n(GET_METHOD, std::strlen(GET_METHOD) + 1, attr.requestMethod);
        }
    }

    void parseRspHeaders(std::string_view headersStr, Dict& headers) {
        auto remaining = headersStr;
        
        while (!remaining.empty()) {
            const auto lineEnd = remaining.find('\n');
            const auto lineLength = (lineEnd != std::string_view::npos) ? lineEnd : remaining.size();
            
            auto line = remaining.substr(0, lineLength);
            auto trimmedLine = dynxxCodingStrTrim(line);
            if (trimmedLine.empty()) [[unlikely]] {
                continue;
            }
            
            const auto colonPos = trimmedLine.find(':');
            if (colonPos == std::string::npos) [[unlikely]] {
                continue;
            }
            auto keyView = std::string_view(trimmedLine).substr(0, colonPos);
            auto valueView = std::string_view(trimmedLine).substr(colonPos + 1);
            auto key = dynxxCodingStrTrim(keyView);
            auto value = dynxxCodingStrTrim(valueView);
            if (!key.empty()) [[likely]] {
                headers.emplace(std::move(key), std::move(value));
            }
            
            if (lineEnd == std::string_view::npos) break;
            remaining.remove_prefix(lineEnd + 1);
        }
    }

    void parseResponse(emscripten_fetch_t* fetch) {
        if (fetch == nullptr) [[unlikely]] {
            return;
        }

        auto rsp = new DynXXHttpResponse();

        rsp->code = fetch->status;
                
        if (fetch->data && fetch->numBytes > 0) [[likely]] {
            rsp->data.reserve(fetch->numBytes);
            rsp->data.assign(fetch->data, fetch->numBytes);
        }
                
        const auto headersLength = emscripten_fetch_get_response_headers_length(fetch);
        if (headersLength <= 0) [[unlikely]] {
            fetch->userData = rsp;
            return;
        }
        std::string headersBuffer(headersLength, '\0');
        const auto actualLength = emscripten_fetch_get_response_headers(fetch, headersBuffer.data(), headersBuffer.size());
        if (actualLength <= 0) [[unlikely]] {
            fetch->userData = rsp;
            return;
        }
        headersBuffer.resize(actualLength);

        parseRspHeaders(headersBuffer, rsp->headers);

        rsp->contentType = rsp->headers["Content-Type"];

        fetch->userData = rsp;
    }
}

DynXXHttpResponse DynXX::Core::Net::WasmHttpClient::request(const std::string_view url, 
                                                 const int method,
                                                 const std::vector<std::string> &headers,
                                                 const std::string_view params,
                                                 const BytesView rawBody,
                                                 const size_t timeout) {
    DynXXHttpResponse response;

    auto const fullUrl = buildUrl(url, params);
            
    dynxxLogPrintF(Debug, "WasmHttpClient.request url: {}", fullUrl);

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.timeoutMSecs = timeout > 0 ? timeout : DYNXX_HTTP_DEFAULT_TIMEOUT;
            
    setMethod(attr, method);
            
    std::vector<const char*> headerPtrs;
    for (const auto& header : headers) {
        headerPtrs.push_back(header.c_str());
        dynxxLogPrintF(Debug, "WasmHttpClient.request header: {}", header);
    }
    headerPtrs.push_back(nullptr);
    if (!headers.empty()) [[likely]] {
        attr.requestHeaders = headerPtrs.data();
    }
            
    if (!rawBody.empty()) [[likely]] {
        attr.requestData = reinterpret_cast<const char*>(rawBody.data());
        attr.requestDataSize = rawBody.size();
    }

    attr.onsuccess = parseResponse;
    attr.onerror = parseResponse;
            
    auto const fetch = emscripten_fetch(&attr, fullUrl.c_str());
    if (!fetch) [[unlikely]] {
        dynxxLogPrint(Error, "WasmHttpClient.request failed");
        return response;
    }
            
    while (!fetch->userData) {
        emscripten_sleep(SLEEP_MS);
    }

    auto rsp = static_cast<DynXXHttpResponse*>(fetch->userData);
    response.code = rsp->code;
    response.data = std::move(rsp->data);
    response.headers = std::move(rsp->headers);
    response.contentType = std::move(rsp->contentType);
    delete rsp;

    emscripten_fetch_close(fetch);
            
    dynxxLogPrintF(Debug, "WasmHttpClient.request response code: {}", response.code);
            
    return response;
}

#endif // __EMSCRIPTEN__