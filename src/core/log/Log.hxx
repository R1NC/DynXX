#ifndef DYNXX_SRC_CORE_LOG_HXX_
#define DYNXX_SRC_CORE_LOG_HXX_

#if defined(__cplusplus)

#include <string_view>
#include <functional>

#include <DynXX/CXX/Log.hxx>

namespace DynXX::Core::Log {
    void setLevel(DynXXLogLevelX level);

    void setCallback(const std::function<void(int level, const char *content)> &callback);

    void print(DynXXLogLevelX level, std::string_view content);
}

#endif
#endif // DYNXX_SRC_CORE_LOG_HXX_
