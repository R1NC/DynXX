#if !defined(_WIN32)

#include "NetUtil.hxx"

#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

namespace 
{
    bool checkIfaName(const struct ifaddrs *ifa, std::string_view name) 
    {
        if (!ifa || !ifa->ifa_name) [[unlikely]]
        {
            return false;
        }
        return strncmp(ifa->ifa_name, name.data(), name.size()) == 0;
    }
}

NGenXX::Net::Util::NetType NGenXX::Net::Util::netType() 
{
    struct ifaddrs *ifaddr, *ifa;
    NetType result = NetType::Offline;
    
    if (getifaddrs(&ifaddr) == -1) 
    {
        return NetType::Unknown;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == nullptr) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) 
        {
            if (
            #if defined(__ANDROID__)
                checkIfaName(ifa, "eth")
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

std::string NGenXX::Net::Util::publicIpV4()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        return {};
    }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(kDefaultDnsPort);
    
    inet_pton(AF_INET, kDefaultDnsIpV4, &serv.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) 
    {
        close(sock);
        return {};
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) < 0) 
    {
        close(sock);
        return {};
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &name.sin_addr, ipStr, INET_ADDRSTRLEN);
    close(sock);

    return std::string(ipStr);
}

std::string NGenXX::Net::Util::publicIpV6()
{
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) 
    {
        return {};
    }

    struct sockaddr_in6 serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(kDefaultDnsPort);
    
    inet_pton(AF_INET6, kDefaultDnsIpV6, &serv.sin6_addr);

    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) 
    {
        close(sock);
        return {};
    }

    struct sockaddr_in6 name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) < 0) 
    {
        close(sock);
        return {};
    }

    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &name.sin6_addr, ipStr, INET6_ADDRSTRLEN);
    close(sock);

    return std::string(ipStr);
}

#endif