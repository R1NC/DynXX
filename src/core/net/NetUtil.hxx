#ifndef DYNXX_SRC_CORE_NET_UTIL_HXX_
#define DYNXX_SRC_CORE_NET_UTIL_HXX_

#if defined(__cplusplus)

#include <string>
#include <array>

#if defined(USE_STD_FORMAT)
#include <format>
#endif

namespace DynXX::Core::Net::Util
        {
            enum class NetType : int 
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
                if (!mac) [[unlikely]]
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
}

#endif

#endif // DYNXX_SRC_CORE_NET_UTIL_HXX_
