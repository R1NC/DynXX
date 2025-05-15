#ifndef NGENXX_SRC_NET_UTIL_HXX_
#define NGENXX_SRC_NET_UTIL_HXX_

#if defined(__cplusplus)

#include <string>

namespace NGenXX::Net::Util
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
                char macStr[18];
                snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                return {macStr};
            }

            std::string macAddress();

            NetType netType();

            std::string publicIpV4();

            std::string publicIpV6();
}

#endif

#endif // NGENXX_SRC_NET_UTIL_HXX_