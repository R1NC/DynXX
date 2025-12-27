#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#if !defined(__ANDROID__) || __ANDROID_API__ >= 24
    #define HAVE_IFADDRS 1
    #endif
    #if defined(__OHOS__) || defined(__linux__)
    #define HAVE_IF_PACKET 1
#endif

#include "NetUtil.hxx"

#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#if defined(HAVE_IFADDRS)
#include <ifaddrs.h>
#endif
#if defined(HAVE_IF_PACKET)
#include <linux/if_packet.h>
#else
#include <net/if_dl.h>
#endif

namespace 
{
#if defined(HAVE_IFADDRS)
    bool checkIfaName(const ifaddrs *ifa, std::string_view name)
    {
        if (ifa == nullptr || ifa->ifa_name == nullptr) [[unlikely]]
        {
            return false;
        }
        return strncmp(ifa->ifa_name, name.data(), name.size()) == 0;
    }
#endif

    constexpr auto socketFlags = SOCK_DGRAM
#if !defined(__APPLE__)
            | SOCK_CLOEXEC
#endif
            ;
}

namespace DynXX::Core::Net::Util {

std::string macAddress()
{
    std::string macAddress;

#if !defined(HAVE_IFADDRS)
    return macAddress;
#else
    
    ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) 
    {

        return macAddress;
    }

    for (ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

#if defined(HAVE_IF_PACKET)
        if (ifa->ifa_addr->sa_family == AF_PACKET) 
        {
            if (const auto s = reinterpret_cast<sockaddr_ll*>(ifa->ifa_addr); s->sll_halen == 6)
            {
                macAddress = formatMacAddress(s->sll_addr);
            }
        }
#else
        if (ifa->ifa_addr->sa_family == AF_LINK) 
        {
            if (auto sdl = reinterpret_cast<sockaddr_dl *>(ifa->ifa_addr); sdl->sdl_alen == 6)
            {
                macAddress = formatMacAddress(reinterpret_cast<unsigned char *>(LLADDR(sdl)));
            }
        }
#endif

        if (!macAddress.empty()) {
            break;
        }
    }
    
    freeifaddrs(ifaddr);
    return macAddress;

#endif
}

NetType netType() 
{
    auto result = NetType::Unknown;

#if !defined(HAVE_IFADDRS)
    return result;
#else
    
    ifaddrs *ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) 
    {
        return result;
    }

    for (ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        if (const auto family = ifa->ifa_addr->sa_family; family == AF_INET || family == AF_INET6)
        {
            if (
            #if defined(__ANDROID__) || defined(__OHOS__)
                checkIfaName(ifa, "eth")
            #elif defined(__linux__)
                checkIfaName(ifa, "eth") || checkIfaName(ifa, "enp")
            #else
                checkIfaName(ifa, "en")
            #endif
            )
            {
                result = NetType::Ethernet;
            } 
            else if (
            #if defined(__ANDROID__)
                checkIfaName(ifa, "wlan") || checkIfaName(ifa, "p2p")
            #elif defined(__OHOS__)
                checkIfaName(ifa, "wlan")
            #elif defined(__linux__)
                checkIfaName(ifa, "wlan") || checkIfaName(ifa, "wifi") || checkIfaName(ifa, "wlp")
            #else
                checkIfaName(ifa, "awdl") || checkIfaName(ifa, "llw")
            #endif
            ) 
            {
                result = NetType::Wifi;
            } 
            else if (
            #if defined(__ANDROID__)
                checkIfaName(ifa, "rmnet") || checkIfaName(ifa, "wwan")
            #elif defined(__OHOS__)
                checkIfaName(ifa, "rmnet")
            #elif defined(__linux__)
                checkIfaName(ifa, "ppp") || checkIfaName(ifa, "wwan")
            #else
                checkIfaName(ifa, "pdp_ip") || checkIfaName(ifa, "pdp")
            #endif
            )
            {
                result = NetType::Mobile;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    return result;

#endif
}

std::string publicIpV4()
{
    const auto sock = socket(AF_INET, socketFlags, 0);
    if (sock < 0) 
    {
        return {};
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(kDefaultDnsPort);
    
    inet_pton(AF_INET, kDefaultDnsIpV4, &serv.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr *>(&serv), sizeof(serv)) < 0)
    {
        close(sock);
        return {};
    }

    sockaddr_in name{};
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, reinterpret_cast<sockaddr *>(&name), &namelen) < 0)
    {
        close(sock);
        return {};
    }

    std::array<char, INET_ADDRSTRLEN> ipStr{};
    inet_ntop(AF_INET, &name.sin_addr, ipStr.data(), INET_ADDRSTRLEN);
    close(sock);

    return {ipStr.data()};
}

std::string publicIpV6()
{
    const auto sock = socket(AF_INET6, socketFlags, 0);
    if (sock < 0) 
    {
        return {};
    }

    sockaddr_in6 serv{};
    std::memset(&serv, 0, sizeof(serv));
    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(kDefaultDnsPort);
    
    inet_pton(AF_INET6, kDefaultDnsIpV6, &serv.sin6_addr);

    if (connect(sock, reinterpret_cast<sockaddr *>(&serv), sizeof(serv)) < 0)
    {
        close(sock);
        return {};
    }

    sockaddr_in6 name{};
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, reinterpret_cast<sockaddr *>(&name), &namelen) < 0)
    {
        close(sock);
        return {};
    }

    std::array<char, INET6_ADDRSTRLEN> ipStr{};
    inet_ntop(AF_INET6, &name.sin6_addr, ipStr.data(), INET6_ADDRSTRLEN);
    close(sock);

    return {ipStr.data()};
}

} // namespace DynXX::Core::Net::Util

#endif
