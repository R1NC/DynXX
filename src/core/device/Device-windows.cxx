#if (defined(_WIN32) || defined(_WIN64))
#include "Device.hxx"

#include <cstddef>
#include <cstdint>

#include <Windows.h>
#include <winreg.h>
#include <winternl.h>

#include <array>

#include <DynXX/C/Device.h>

extern "C" NTSYSAPI NTSTATUS NTAPI RtlGetVersion(PRTL_OSVERSIONINFOW lpVersionInformation);

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000)
#endif

#ifndef PRODUCT_COMPUTE_CLUSTER
#define PRODUCT_COMPUTE_CLUSTER 0x00000031
#endif
#ifndef PRODUCT_STORAGE_STANDARD_SERVER
#define PRODUCT_STORAGE_STANDARD_SERVER 0x00000034
#endif
#ifndef PRODUCT_STORAGE_WORKGROUP_SERVER
#define PRODUCT_STORAGE_WORKGROUP_SERVER 0x00000035
#endif
#ifndef PRODUCT_STORAGE_ENTERPRISE_SERVER
#define PRODUCT_STORAGE_ENTERPRISE_SERVER 0x00000036
#endif
#ifndef PRODUCT_MULTIPOINT_STANDARD_SERVER
#define PRODUCT_MULTIPOINT_STANDARD_SERVER 0x0000005A
#endif
#ifndef PRODUCT_MULTIPOINT_PREMIUM_SERVER
#define PRODUCT_MULTIPOINT_PREMIUM_SERVER 0x0000005B
#endif

namespace {
    struct WindowsVersion {
        DWORD major{0};
        DWORD minor{0};
        DWORD build{0};
        std::string name;
    };

    struct WindowsVersionEntry {
        ULONG major{0};
        ULONG minor{0};
        ULONG buildMin{0};
        ULONG buildMax{0};
        const char* client{""};
        const char* server{""};
    };

    #include <windows.h>

    bool isWindowsServer() {
        DWORD productType = 0;
        if (!GetProductInfo(0, 0, 0, 0, &productType)) [[unlikely]] {
            return false;
        }

        switch (productType) {
            case PRODUCT_STANDARD_SERVER:
            case PRODUCT_STANDARD_SERVER_CORE:
            case PRODUCT_ENTERPRISE_SERVER:
            case PRODUCT_ENTERPRISE_SERVER_CORE:
            case PRODUCT_DATACENTER_SERVER:
            case PRODUCT_DATACENTER_SERVER_CORE:
            case PRODUCT_WEB_SERVER:
            case PRODUCT_WEB_SERVER_CORE:
            case PRODUCT_CLUSTER_SERVER:
            case PRODUCT_STORAGE_STANDARD_SERVER:
            case PRODUCT_STORAGE_WORKGROUP_SERVER:
            case PRODUCT_STORAGE_ENTERPRISE_SERVER:
            case PRODUCT_COMPUTE_CLUSTER:
            case PRODUCT_MULTIPOINT_STANDARD_SERVER:
            case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
                return true;
            default:
                return false;
        }
    }

    std::string getWindowsVersionName(ULONG major, ULONG minor, ULONG build) {
        const auto isServer = isWindowsServer();
        
        static constexpr std::array<WindowsVersionEntry, 11> table = {{
            {10, 0, 22000, ULONG_MAX, "Windows 11",         "Windows Server 2022"},
            {10, 0, 17763, 21999,     "Windows 10",         "Windows Server 2019"},
            {10, 0, 14393, 17762,     "Windows 10",         "Windows Server 2016"},
            {10, 0,     0, 14392,     "Windows 10",         "Windows 10"},
            { 6, 3,     0, ULONG_MAX, "Windows 8.1",        "Windows Server 2012 R2"},
            { 6, 2,     0, ULONG_MAX, "Windows 8",          "Windows Server 2012"},
            { 6, 1,     0, ULONG_MAX, "Windows 7",          "Windows Server 2008 R2"},
            { 6, 0,     0, ULONG_MAX, "Windows Vista",      "Windows Server 2008"},
            { 5, 2,     0, ULONG_MAX, "Windows XP x64",     "Windows Server 2003"},
            { 5, 1,     0, ULONG_MAX, "Windows XP",         "Windows XP"},
            { 5, 0,     0, ULONG_MAX, "Windows 2000",       "Windows 2000"},
        }};
        
        for (const auto& e : table) {
            if (major == e.major && minor == e.minor && build >= e.buildMin && build <= e.buildMax) {
                return isServer ? e.server : e.client;
            }
        }
        
        return "Windows " + std::to_string(major) + "." + std::to_string(minor) + " (Build " + std::to_string(build) + ")";
    }

    WindowsVersion getWindowsVersion() {
        RTL_OSVERSIONINFOW osvi = {0};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        if (RtlGetVersion(&osvi) != STATUS_SUCCESS) [[unlikely]] {
            return {};
        }

        WindowsVersion ver{
            .major = osvi.dwMajorVersion,
            .minor = osvi.dwMinorVersion,
            .build = osvi.dwBuildNumber,
            .name = getWindowsVersionName(osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber)
        };
        return ver;
    }

    std::wstring readRegString(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName) {
        HKEY key = nullptr;
        if (RegOpenKeyExW(hKey, subKey, 0, KEY_READ, &key) != ERROR_SUCCESS) [[unlikely]] {
            return L"";
        }

        DWORD type = 0;
        DWORD size = 0;
        LSTATUS status = RegQueryValueExW(key, valueName, nullptr, &type, nullptr, &size);
        if (status != ERROR_SUCCESS 
            || (type != REG_SZ && type != REG_EXPAND_SZ)
            || (size < sizeof(wchar_t) || (size % sizeof(wchar_t) != 0))) [[unlikely]] {
            RegCloseKey(key);
            return L"";
        }

        std::vector<wchar_t> buffer((size + sizeof(wchar_t) - 1) / sizeof(wchar_t) + 1, 0);
        auto bufSize = static_cast<DWORD>(buffer.size() * sizeof(wchar_t));
        status = RegQueryValueExW(key, valueName, nullptr, nullptr,  reinterpret_cast<LPBYTE>(buffer.data()), &bufSize);
        if (status != ERROR_SUCCESS) [[unlikely]] {
            RegCloseKey(key);
            return L"";
        }

        RegCloseKey(key);

        if (type == REG_EXPAND_SZ) {
            DWORD required = ExpandEnvironmentStringsW(buffer.data(), nullptr, 0);
            if (required != 0) {
                std::vector<wchar_t> expanded(required, L'\0');
                if (ExpandEnvironmentStringsW(buffer.data(), expanded.data(), required) != 0) {
                    return std::wstring(expanded.data());
                }
            }
        }

        return std::wstring(buffer.data());
    }

    std::wstring readDeviceName() {
        DWORD size = 0;
        GetComputerNameW(nullptr, &size);
        if (size == 0) [[unlikely]] {
            return {};
        }

        std::wstring wresult(size, L'\0');
        if (!GetComputerNameW(wresult.data(), &size)) [[unlikely]] {
            return {};
        }

        wresult.resize(size);
        return wresult;
    }

    std::string wstr_to_utf8(const std::wstring& wstr) {
        if (wstr.empty()) [[unlikely]] {
            return {};
        }

        const auto len = WideCharToMultiByte(
            CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()),
            nullptr, 0, nullptr, nullptr
        );
        if (len == 0) [[unlikely]] {
            return {};
        }

        std::string utf8(len, 0);
        WideCharToMultiByte(
            CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()),
            &utf8[0], len, nullptr, nullptr
        );

        return utf8;
    }

    [[maybe_unused]]
    std::wstring utf8_to_wstr(const std::string& utf8) {
        if (utf8.empty()) [[unlikely]] {
            return {};
        }

        const auto len = MultiByteToWideChar(
            CP_UTF8, 0,
            utf8.c_str(), static_cast<int>(utf8.size()),
            nullptr, 0
        );
        if (len == 0) [[unlikely]] {
            return {};
        }

        std::wstring wstr(len, L'\0');
        MultiByteToWideChar(
            CP_UTF8, 0,
            utf8.c_str(), static_cast<int>(utf8.size()),
            wstr.data(), len
        );

        return wstr;
    }
}

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::Windows;
}

std::string deviceName()
{
    static const auto name = []() -> std::string {
        const auto device = readDeviceName();
        if (device.empty()) {
            return "Unknown";
        }
        return wstr_to_utf8(device);
    }();
    return name;
}

std::string deviceManufacturer()
{
    static const auto manufacturer = []() -> std::string {
        const auto wstr = readRegString(
            HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\BIOS",
            L"SystemManufacturer"
        );
        if (wstr.empty()) {
            return "Unknown";
        }
        return wstr_to_utf8(wstr);
    }();
    return manufacturer;
}

std::string deviceModel()
{
    static const auto model = []() -> std::string {
        const auto wstr = readRegString(
            HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\BIOS",
            L"SystemProductName"
        );
        if (wstr.empty()) {
            return "Unknown";
        }
        return wstr_to_utf8(wstr);
    }();
    return model;
}

std::string osVersion()
{
    static const auto ver = []() -> std::string {
        const auto wv = getWindowsVersion();
        if (wv.name.empty()) [[unlikely]] {
            return "Windows " + std::to_string(wv.major) + "." + std::to_string(wv.minor);
        }
        return wv.name + "(" + std::to_string(wv.build) + ")";
    }();
    return ver;
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(_M_X64) || defined(__x86_64__)
    return DynXXDeviceCpuArchX::X86_64;
#elif defined(_M_ARM64) || defined(__aarch64__)
    return DynXXDeviceCpuArchX::ARM_64;
#elif defined(_M_IX86) || defined(__i386__)
    return DynXXDeviceCpuArchX::X86;
#elif defined(_M_ARM) || defined(__arm__)
    return DynXXDeviceCpuArchX::ARM;
#else
    return DynXXDeviceCpuArchX::Unknown;
#endif
}

} // namespace DynXX::Core::Device

#endif
