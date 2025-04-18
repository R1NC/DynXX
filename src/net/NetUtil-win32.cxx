#ifdef _WIN32

#include "NetUtil.hxx"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

std::string NGenXX::Net::Util::publicIpV4()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        return {};
    }
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        WSACleanup();
        return {};
    }

    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(kDnsPort);
    
    InetPtonA(AF_INET, kGoogleDnsIp, &serv.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    char ipStr[INET_ADDRSTRLEN];
    InetNtopA(AF_INET, &name.sin_addr, ipStr, INET_ADDRSTRLEN);
    closesocket(sock);
    WSACleanup();

    return std::string(ipStr);
}

std::string NGenXX::Net::Util::publicIpV6()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        return {};
    }
    SOCKET sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        WSACleanup();
        return {};
    }

    const char* kGoogleDnsIp = "2001:4860:4860::8888";
    uint16_t kDnsPort = 53;
    struct sockaddr_in6 serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(kDnsPort);
    
    InetPtonA(AF_INET6, kGoogleDnsIp, &serv.sin6_addr);

    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    struct sockaddr_in6 name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    char ipStr[INET6_ADDRSTRLEN];
    InetNtopA(AF_INET6, &name.sin6_addr, ipStr, INET6_ADDRSTRLEN);
    closesocket(sock);
    WSACleanup();

    return std::string(ipStr);
}

#endif