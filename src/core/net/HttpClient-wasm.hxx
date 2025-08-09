#ifndef NGENXX_SRC_CORE_NET_HTTP_CLIENT_WASM_HXX_
#define NGENXX_SRC_CORE_NET_HTTP_CLIENT_WASM_HXX_

#if defined(__EMSCRIPTEN__)

#include <NGenXXTypes.hxx>
#include <NGenXXNet.hxx>

namespace NGenXX::Core::Net
{
        class WasmHttpClient {
                public:
                [[nodiscard]] static NGenXXHttpResponse request(const std::string_view url, 
                                     const int method,
                                     const std::vector<std::string> &headers,
                                     const std::string_view params,
                                     const BytesView rawBody,
                                     const size_t timeout);
        };
}

#endif

#endif // NGENXX_SRC_CORE_NET_HTTP_CLIENT_WASM_HXX_
