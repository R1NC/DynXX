#if (defined(_WIN32) || defined(_WIN64))
#include "Device.hxx"

#include <cstddef>
#include <cstdint>

#include <Windows.h>
#include <winreg.h>
#include <winternl.h>

#include <DynXX/C/Device.h>

extern "C" NTSYSAPI NTSTATUS NTAPI RtlGetVersion(PRTL_OSVERSIONINFOW lpVersionInformation);
#if !defined(STATUS_SUCCESS)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000)
#endif

namespace {
    struct WindowsVersion {
        DWORD major{0};
        DWORD minor{0};
        DWORD build{0};
        std::string name;
    };

    WindowsVersion getWindowsVersion() {
        RTL_OSVERSIONINFOW osvi = {0};
        osvi.dwOSVersionInfoSize = sizeof(osvi);

        if (RtlGetVersion(&osvi) != STATUS_SUCCESS) [[unlikely]] {
            return {};
        }

        WindowsVersion ver{osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, {}};

        if (ver.major == 10) {
            if (ver.build >= 22000) {
                ver.name = "Windows 11";
            } else {
                ver.name = "Windows 10";
            }
        } else if (ver.major == 6) {
            if (ver.minor == 3) {
                ver.name = "Windows 8.1";
            } else if (ver.minor == 2) {
                ver.name = "Windows 8";
            } else if (ver.minor == 1) {
                ver.name = "Windows 7";
            } else if (ver.minor == 0) {
                ver.name = "Windows Vista";
            }
        } else if (ver.major == 5) {
            if (ver.minor == 1) {
                ver.name = "Windows XP";
            }
        }
        
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

        std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1, 0);
        status = RegQueryValueExW(key, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer.data()), &size);
        if (status != ERROR_SUCCESS) [[unlikely]] {
            RegCloseKey(key);
            return L"";
        }

        RegCloseKey(key);

        buffer.back() = L'\0';
        return std::wstring(buffer.data());
    }

    std::wstring readDeviceName() {
        DWORD size = 0;

        if (!GetComputerNameW(nullptr, &size)) {
            if (GetLastError() != ERROR_BUFFER_OVERFLOW) [[unlikely]] {
                return {};
            }
        }

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
