#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN  // Reduces Windows header conflicts

#include "NetUtil.hxx"

#include <array>

#include <WinSock2.h>
#include <Windows.h>
#include <ws2def.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

#include <DynXX/CXX/Memory.hxx>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace {
    template<typename F>
    concept NetAdapterAddrsHandler = std::invocable<F, PIP_ADAPTER_ADDRESSES> 
        && std::convertible_to<std::invoke_result_t<F, PIP_ADAPTER_ADDRESSES>, bool>;

    template<NetAdapterAddrsHandler F>
    void handleNetAdapterAddrs(F &&handler) {
        ULONG outBufLen = 0;
        GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, nullptr, &outBufLen);

        std::vector<BYTE> buffer(outBufLen);
        auto pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    
        if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, pAddresses, &outBufLen) != NO_ERROR) [[unlikely]] {
            return;
        }
        for (auto p = pAddresses; p != nullptr; p = p->Next) {
            if (handler(p)) break;
        }
    }
}

namespace DynXX::Core::Net::Util {

std::string macAddress()
{
    std::string macAddress;

    handleNetAdapterAddrs([&macAddress](PIP_ADAPTER_ADDRESSES p) {
        if (p->OperStatus == IfOperStatusUp && p->PhysicalAddressLength == 6) {
            macAddress = formatMacAddress(p->PhysicalAddress);
            return true;
        }
        return false;
    });

    return macAddress;
}

NetType netType() 
{
    auto result = NetType::Offline;

    handleNetAdapterAddrs([&result](PIP_ADAPTER_ADDRESSES p) {
        if (p->OperStatus != IfOperStatusUp) {
            return false;
        }
        if (p->IfType == IF_TYPE_ETHERNET_CSMACD) 
        {
            result = NetType::Ethernet;
            return true;
        } 
        else if (p->IfType == IF_TYPE_IEEE80211) 
        {
            result = NetType::Wifi;
            return true;
        }
        return false;
    });
        
    return result;
}

std::string publicIpV4()
{
    if (WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
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

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv), sizeof(serv)) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    struct sockaddr_in name;
    if (socklen_t namelen = sizeof(name); getsockname(sock, reinterpret_cast<struct sockaddr*>(&name), &namelen) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    std::array<char, INET_ADDRSTRLEN> ipStr{};
    InetNtopA(AF_INET, &name.sin_addr, ipStr.data(), INET_ADDRSTRLEN);
    closesocket(sock);
    WSACleanup();

    return {ipStr.data()};
}

std::string publicIpV6()
{
    if (WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
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
    std::memset(&serv, 0, sizeof(serv));
    serv.sin6_family = AF_INET6;
    serv.sin6_port = htons(kDefaultDnsPort);
    
    InetPtonA(AF_INET6, kDefaultDnsIpV6, &serv.sin6_addr);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv), sizeof(serv)) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    struct sockaddr_in6 name;
    if (socklen_t namelen = sizeof(name); getsockname(sock, reinterpret_cast<struct sockaddr*>(&name), &namelen) < 0) 
    {
        closesocket(sock);
        WSACleanup();
        return {};
    }

    std::array<char, INET6_ADDRSTRLEN> ipStr{};
    InetNtopA(AF_INET6, &name.sin6_addr, ipStr.data(), INET6_ADDRSTRLEN);
    closesocket(sock);
    WSACleanup();

    return {ipStr.data()};
}

} // namespace DynXX::Core::Net::Util

#endif
