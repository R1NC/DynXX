#ifndef NGENXX_SRC_NET_UTIL_HXX_
#define NGENXX_SRC_NET_UTIL_HXX_

#if defined(__cplusplus)

#include <string>

namespace NGenXX
{
    namespace Net
    {
        namespace Util
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

            NetType netType();

            std::string publicIpV4();

            std::string publicIpV6();
        }
    }
}

#endif

#endif // NGENXX_SRC_NET_UTIL_HXX_