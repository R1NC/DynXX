#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))
#include "Device.hxx"
#include "Device-posix.hxx"

#include <DynXX/C/Device.h>

#include <string>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace {
    
    struct OSVerInfo {
        std::string name;
        std::string version;
        std::string prettyName;
        std::string id;
        std::string versionId;
        bool valid = false;
    };

    struct DmiInfo {
        std::string manufacturer;
        std::string model;
        std::string productFamily;
        bool valid = false;
    };

    void trim(std::string& s) {
        constexpr auto ws = " \t\n\r";
        s.erase(0, s.find_first_not_of(ws));
        auto end = s.find_last_not_of(ws);
        if (end != std::string::npos) {
            s.erase(end + 1);
        }
    }

    std::string readFirstLine(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) [[unlikely]] {
            return {};
        }
        std::string line;
        if (std::getline(file, line)) {
            trim(line);
            return line;
        }
        return {};
    }

    std::pair<std::string, std::string> parseOsVerLine(const std::string& line) {
        if (line.empty() || line[0] == '#') [[unlikely]] {
            return {};
        }
        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) [[unlikely]] {
            return {};
        }
        auto key = line.substr(0, eqPos);
        auto value = line.substr(eqPos + 1);
        trim(key);
        trim(value);
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }
        return {std::move(key), std::move(value)};
    }

    OSVerInfo readOSVerInfo() {
        const char* paths[] = {
            "/etc/os-release",
            "/usr/lib/os-release"
        };

        for (const char* path : paths) {
            std::ifstream file(path);
            if (!file.is_open()) continue;

            std::unordered_map<std::string, std::string> kv;
            std::string line;
            while (std::getline(file, line)) {
                auto [key, val] = parseOsVerLine(line);
                if (!key.empty()) {
                    kv.emplace(std::move(key), std::move(val));
                }
            }

            if (!kv.empty()) {
                OSVerInfo info;
                info.name       = kv.count("NAME")        ? kv.at("NAME")        : "Unknown";
                info.version    = kv.count("VERSION")     ? kv.at("VERSION")     : "";
                info.prettyName = kv.count("PRETTY_NAME") ? kv.at("PRETTY_NAME") : info.name + (info.version.empty() ? "" : " " + info.version);
                info.id         = kv.count("ID")          ? kv.at("ID")          : "";
                info.versionId  = kv.count("VERSION_ID")  ? kv.at("VERSION_ID")  : "";
                info.valid      = true;
                return info;
            }
        }

        return {};
    }

    std::string readDmiField(const char* field) {
        return readFirstLine((std::string("/sys/class/dmi/id/") + field).c_str());
    }

    DmiInfo readDmiInfo() {
        static const auto dmi = []() -> DmiInfo {
            DmiInfo info;
            info.manufacturer  = readDmiField("sys_vendor");
            info.model         = readDmiField("product_name");
            info.productFamily = readDmiField("product_family");
            info.valid         = !info.manufacturer.empty() || !info.model.empty();
            return info;
        }();
        return dmi;
    }
}

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::Linux;
}

std::string deviceName()
{
    static const auto ui = POSIX::readUnameInfo();
    return ui.nodeName;
}

std::string deviceManufacturer()
{
    static const auto dmi = readDmiInfo();
    return dmi.manufacturer;
}

std::string deviceModel()
{
    static const auto dmi = readDmiInfo();
    return dmi.model;
}

std::string osVersion()
{
    static const auto ov = readOSVerInfo();
    return ov.prettyName;
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(__x86_64__) || defined(__amd64__)
    return DynXXDeviceCpuArchX::X86_64;
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
    return DynXXDeviceCpuArchX::X86;
#elif defined(__aarch64__) || defined(__arm64__)
    return DynXXDeviceCpuArchX::ARM_64;
#elif defined(__arm__) || defined(__ARMEL__)
    return DynXXDeviceCpuArchX::ARM;
#elif defined(__ia64__)
    return DynXXDeviceCpuArchX::IA64;
#elif defined(__mips64)
    return DynXXDeviceCpuArchX::MIPS_64;
#elif defined(__mips__)
    return DynXXDeviceCpuArchX::MIPS;
#elif defined(__powerpc64__)
    return DynXXDeviceCpuArchX::PowerPC_64;
#elif defined(__powerpc__)
    return DynXXDeviceCpuArchX::PowerPC;
#elif defined(__riscv) && __riscv_xlen == 64
    return DynXXDeviceCpuArchX::RISCV_64;
#elif defined(__riscv) && __riscv_xlen == 32
    return DynXXDeviceCpuArchX::RISCV;
#elif defined(__s390x__)
    return DynXXDeviceCpuArchX::S390_64;
#elif defined(__s390__)
    return DynXXDeviceCpuArchX::S390;
#else
    return DynXXDeviceCpuArchX::Unknown;
#endif
}

} // namespace DynXX::Core::Device

#endif
