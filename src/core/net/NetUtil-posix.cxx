#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

#include "NetUtil.hxx"

#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>
#if defined(__ANDROID__) || defined(__OHOS__) || defined(__linux__)
#include <linux/if_packet.h>
#else
#include <net/if_dl.h>
#endif

namespace 
{
    bool checkIfaName(const ifaddrs *ifa, const std::string_view name)
    {
        if (!ifa || !ifa->ifa_name) [[unlikely]]
        {
            return false;
        }
        return strncmp(ifa->ifa_name, name.data(), name.size()) == 0;
    }
}

std::string NGenXX::Core::Net::Util::macAddress()
{
    ifaddrs *ifaddr = nullptr;
    std::string macAddress;
    
    if (getifaddrs(&ifaddr) == -1) 
    {
        return macAddress;
    }

    for (ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr) continue;

#if defined(__ANDROID__) || defined(__OHOS__) || defined(__linux__)
        if (ifa->ifa_addr->sa_family == AF_PACKET) 
        {
            sockaddr_ll *s = reinterpret_cast<sockaddr_ll*>(ifa->ifa_addr);
            if (s->sll_halen == 6)
            {
                macAddress = formatMacAddress(s->sll_addr);
            }
        }
#else
        if (ifa->ifa_addr->sa_family == AF_LINK) 
        {
            if (auto* sdl = reinterpret_cast<sockaddr_dl *>(ifa->ifa_addr); sdl->sdl_alen == 6)
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
}

NGenXX::Core::Net::Util::NetType NGenXX::Core::Net::Util::netType() 
{
    ifaddrs *ifaddr = nullptr;
    auto result = NetType::Offline;
    
    if (getifaddrs(&ifaddr) == -1) 
    {
        return NetType::Unknown;
    }

    for (ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr) continue;

        if (const int family = ifa->ifa_addr->sa_family; family == AF_INET || family == AF_INET6)
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
}

std::string NGenXX::Core::Net::Util::publicIpV4()
{
    const int sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &name.sin_addr, ipStr, INET_ADDRSTRLEN);
    close(sock);

    return {ipStr};
}

std::string NGenXX::Core::Net::Util::publicIpV6()
{
    const int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        return {};
    }

    sockaddr_in6 serv{};
    memset(&serv, 0, sizeof(serv));
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

    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &name.sin6_addr, ipStr, INET6_ADDRSTRLEN);
    close(sock);

    return {ipStr};
}

#endif