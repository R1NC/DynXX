#pragma once

#if defined(__EMSCRIPTEN__)

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Net.hxx>

namespace DynXX::Core::Net
{
        class WasmHttpClient {
                public:
                [[nodiscard]] static DynXXHttpResponse request(std::string_view url, 
                                     const DynXXHttpMethodX method,
                                     const std::vector<std::string> &headers,
                                     std::string_view params,
                                     BytesView rawBody,
                                     const size_t timeout);
        };
}

#endif
