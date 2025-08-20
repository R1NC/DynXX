#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN  // Reduces Windows header conflicts

#include "NetUtil.hxx"

#include <winsock2.h>
#include <windows.h>
#include <ws2def.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

std::string DynXX::Core::Net::Util::macAddress()
{
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 0;
    std::string macAddress;

    GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, nullptr, &outBufLen);
    pAddresses = (PIP_ADAPTER_ADDRESSES)std::malloc(outBufLen);
    
    if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, pAddresses, &outBufLen) == NO_ERROR)
    {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses)
        {
            if (pCurrAddresses->OperStatus == IfOperStatusUp &&
                pCurrAddresses->PhysicalAddressLength == 6)
            {
                macAddress = formatMacAddress(pCurrAddresses->PhysicalAddress);
                break;
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }

    std::free(pAddresses);
    return macAddress;
}

DynXX::Core::Net::Util::NetType DynXX::Core::Net::Util::netType() 
{
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;
    NetType result = NetType::Offline;
    
    GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, nullptr, &outBufLen);
    pAddresses = (PIP_ADAPTER_ADDRESSES)std::malloc(outBufLen);
        
    if ((dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, pAddresses, &outBufLen)) == NO_ERROR) 
    {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) 
        {
            if (pCurrAddresses->OperStatus == IfOperStatusUp) 
            {
                if (pCurrAddresses->IfType == IF_TYPE_ETHERNET_CSMACD) 
                {
                    result = NetType::Ethernet;
                    break;
                } 
                else if (pCurrAddresses->IfType == IF_TYPE_IEEE80211) 
                {
                    result = NetType::Wifi;
                    break;
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }
        
    std::free(pAddresses);
    return result;
}

std::string DynXX::Core::Net::Util::publicIpV4()
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

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(kDefaultDnsPort);
    
    InetPtonA(AF_INET, kDefaultDnsIpV4, &serv.sin_addr);

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

std::string DynXX::Core::Net::Util::publicIpV6()
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

    struct sockaddr_in6 serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(kDefaultDnsPort);
    
    InetPtonA(AF_INET6, kDefaultDnsIpV6, &serv.sin6_addr);

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