#pragma once

#include <DynXX/CXX/Log.hxx>

namespace DynXX::Core::Log {
    void setLevel(DynXXLogLevelX level);

    void setCallback(const std::function<void(int level, const char *content)> &callback);

    void print(DynXXLogLevelX level, std::string_view content);
} // namespace DynXX::Core::Log
