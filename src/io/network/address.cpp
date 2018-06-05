#include <sp2/io/network/address.h>
#include <sp2/logging.h>

#ifdef __WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>
#endif

namespace sp {
namespace io {
namespace network {


Address::Address(string hostname)
{
#ifdef __WIN32
    static bool wsa_startup_done = false;
    if (!wsa_startup_done)
    {
        WSADATA wsa_data;
        memset(&wsa_data, 0, sizeof(WSADATA));
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
        wsa_startup_done = true;
    }
#endif
    struct addrinfo* result;
    if (::getaddrinfo(hostname.c_str(), nullptr, nullptr, &result))
        return;
    
    for(struct addrinfo* data=result; data != nullptr; data=data->ai_next)
    {
        if (data->ai_family != AF_INET && data->ai_family != AF_INET6)
            continue;

        char buffer[128];
        ::getnameinfo(data->ai_addr, data->ai_addrlen, buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);

        addr_info.emplace_back(data->ai_family, buffer, data->ai_addr, data->ai_addrlen);
    }
    ::freeaddrinfo(result);
}

Address::Address(std::list<AddrInfo>&& addr_info)
: addr_info(std::forward<std::list<AddrInfo>>(addr_info))
{
}

Address Address::getLocalAddress()
{
    std::list<AddrInfo> addr_info;

#ifdef __WIN32
    char buffer[128];

    PIP_ADAPTER_ADDRESSES addresses;
    ULONG table_size = 0;
    GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, nullptr, &table_size);
    if (table_size > 0)
    {
        addresses = (PIP_ADAPTER_ADDRESSES)calloc(table_size, 1);
        if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, addresses, &table_size) == NO_ERROR)
        {
            for(PIP_ADAPTER_ADDRESSES address = addresses; address; address = address->Next)
            {
                if (address->OperStatus != IfOperStatusUp)
                    continue;

                //WideCharToMultiByte(CP_ACP, 0, address->FriendlyName, wcslen(address->FriendlyName), buffer, sizeof(buffer), nullptr, nullptr);
                //LOG(Debug, "FriendlyName", buffer);
                
                for (PIP_ADAPTER_UNICAST_ADDRESS ua = address->FirstUnicastAddress; ua != nullptr; ua = ua->Next)
                {
                    ::getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
                    if (ua->Address.iSockaddrLength == sizeof(sockaddr_in))
                        addr_info.emplace_back(AF_INET, buffer, ua->Address.lpSockaddr, ua->Address.iSockaddrLength);
                    if (ua->Address.iSockaddrLength == sizeof(sockaddr_in6))
                        addr_info.emplace_back(AF_INET6, buffer, ua->Address.lpSockaddr, ua->Address.iSockaddrLength);
                    //LOG(Debug, "Address:", buffer);
                }
            }
        }
        free(addresses);
    }
#else
    char buffer[128];

    struct ifaddrs* addrs;
    if (!::getifaddrs(&addrs))
    {
        for(struct ifaddrs* addr = addrs; addr != nullptr; addr = addr->ifa_next)
        {
            if (addr->ifa_addr->sa_family == AF_INET)
            {
                ::getnameinfo(addr->ifa_addr, sizeof(struct sockaddr_in), buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
                addr_info.emplace_back(addr->ifa_addr->sa_family, buffer, addr->ifa_addr, sizeof(struct sockaddr_in));
            }
            if (addr->ifa_addr->sa_family == AF_INET6)
            {
                ::getnameinfo(addr->ifa_addr, sizeof(struct sockaddr_in6), buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
                addr_info.emplace_back(addr->ifa_addr->sa_family, buffer, addr->ifa_addr, sizeof(struct sockaddr_in6));
            }
        }
        freeifaddrs(addrs);
    }
#endif
    return Address(std::move(addr_info));
}

Address::AddrInfo::AddrInfo(int family, const string& human_readable, const void* addr, size_t addrlen)
: family(family), human_readable(human_readable), addr((const char*)addr, addrlen)
{
}

Address::AddrInfo::~AddrInfo()
{
}

};//namespace network
};//namespace io
};//namespace sp
