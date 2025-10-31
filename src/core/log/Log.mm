#if defined(__APPLE__)
#include <Foundation/Foundation.h>

#include <DynXX/CXX/Types.hxx>

namespace {
  constexpr auto NSStrWithCStr = [](std::string_view sv) {
    return [NSString stringWithCString:sv.empty() ? "" : sv.data() encoding:NSUTF8StringEncoding];
  };
}

void DYNXXLogApple(const char* tag, int level, const char* log) {
  if (log == nullptr) [[unlikely]] {
    return;
  }
  NSLog(@"%@_%d | %@", NSStrWithCStr(tag), level, NSStrWithCStr(log));
}

#endif
