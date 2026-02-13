#pragma once

#if defined(__linux__) || defined(__APPLE__) || defined(__ANDROID__) || \
    defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || \
    defined(__unix__) || defined(__CYGWIN__)

#include <sys/utsname.h>

#include <string>

namespace DynXX::Core::Device::POSIX {

  struct UnameInfo {
    std::string nodeName; // Name of this node within the communications network
    std::string machine; // Name of the hardware type
    std::string sysName; // Name of this implementation of the operating system
    std::string release; // Current release level of this implementation
    std::string version; // Current version level of this release
  };

  inline UnameInfo readUnameInfo() {
    struct utsname uts;
    if (uname(&uts) != 0) [[unlikely]] {
      return {};
    }
    return {
      uts.nodename,
      uts.machine,
      uts.sysname,
      uts.release,
      uts.version
    };
  }
}

#endif
