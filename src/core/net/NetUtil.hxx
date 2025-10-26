#pragma once

#include <cstdio>
#include <string>

#if defined(USE_STD_FORMAT)
#include <format>
#endif

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Net::Util
        {
            enum class NetType : uint8_t 
            {
                Unknown,
                Offline,
                Wifi,
                Ethernet,
                Mobile
            };

            auto constexpr kDefaultDnsIpV4 = "8.8.8.8";
            auto constexpr kDefaultDnsIpV6 = "2001:4860:4860::8888";
            auto constexpr kDefaultDnsPort = 53;

            inline std::string formatMacAddress(const unsigned char* mac)
            {
                if (mac == nullptr) [[unlikely]]
                {
                    return {};
                }
#if defined(USE_STD_FORMAT)
                static constexpr auto kMacAddressFormat = "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}";
                return std::format(kMacAddressFormat,
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#else
                static constexpr auto hexTab = "0123456789abcdef";
                std::string result;
                result.reserve(17);
                for (size_t i = 0; i < 6; ++i) {
                    if (i > 0) result += ':';
                    result += hexTab[mac[i] >> 4];
                    result += hexTab[mac[i] & 0x0F];
                }
                return result;
#endif
            }

            std::string macAddress();

            NetType netType();

            std::string publicIpV4();

            std::string publicIpV6();
} // namespace DynXX::Core::Net::Util
