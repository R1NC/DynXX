#ifndef DYNXX_SRC_CORE_NET_HTTP_CLIENT_WASM_HXX_
#define DYNXX_SRC_CORE_NET_HTTP_CLIENT_WASM_HXX_

#if defined(__EMSCRIPTEN__)

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Net.hxx>

namespace DynXX::Core::Net
{
        class WasmHttpClient {
                public:
                [[nodiscard]] static DynXXHttpResponse request(std::string_view url, 
                                     const int method,
                                     const std::vector<std::string> &headers,
                                     std::string_view params,
                                     BytesView rawBody,
                                     const size_t timeout);
        };
}

#endif

#endif // DYNXX_SRC_CORE_NET_HTTP_CLIENT_WASM_HXX_
