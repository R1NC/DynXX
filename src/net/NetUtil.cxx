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
            if (strncmp(ifa->ifa_name, "en", 2) == 0) 
            {
                result = NetType::Ethernet;
            } 
            else if (strncmp(ifa->ifa_name, "awdl", 4) == 0 
                    || strncmp(ifa->ifa_name, "llw", 3) == 0) 
            {
                result = NetType::Wifi;
            }
            else if (strncmp(ifa->ifa_name, "pdp_ip", 6) == 0 
                    || strncmp(ifa->ifa_name, "pdp", 3) == 0)
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

    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(kDnsPort);
    
    inet_pton(AF_INET, kGoogleDnsIp, &serv.sin_addr);

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

    const char* kGoogleDnsIp = "2001:4860:4860::8888";
    uint16_t kDnsPort = 53;
    struct sockaddr_in6 serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(kDnsPort);
    
    inet_pton(AF_INET6, kGoogleDnsIp, &serv.sin6_addr);

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