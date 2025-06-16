#include "arcnet/discovery/interfaces.hpp"

#ifndef WIN32
#include <sys/socket.h>
#include <ifaddrs.h>
#endif

#include <iostream>

namespace arcnet::discovery
{
    IPAddress::IPAddress(int family, std::string str)
    {
        this->family = family;
        this->str = str;
    };

    IPAddress IPAddress::fromInAddr(int family, in_addr_any addr)
    {
        return IPAddress(family, ntop(family, addr));
    };

    IPAddress IPAddress::fromSai4(sockaddr_in *sai)
    {
        in_addr_any addr;
        addr.ip4 = sai->sin_addr;
        return fromInAddr(AF_INET, addr);
    };

    IPAddress IPAddress::fromSai6(sockaddr_in6 *sai)
    {
        in_addr_any addr;
        addr.ip6 = sai->sin6_addr;
        return fromInAddr(AF_INET6, addr);
    };

    std::string IPAddress::ntop(int family, in_addr_any addr)
    {
        int len = (family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);

        char *cstr = new char[len];

        // can return empty str
        inet_ntop(family, &addr, cstr, len);

        std::string str(cstr);
        return str;
    };

    int IPAddress::getAddressStrLen()
    {
        return (family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
    }

    sockaddr_in IPAddress::pton4()
    {
        sockaddr_in saddr;
        inet_pton(family, getAddressStr().c_str(), &(saddr.sin_addr));
        return saddr;
    };

    sockaddr_in6 IPAddress::pton6()
    {
        sockaddr_in6 saddr;
        inet_pton(family, getAddressStr().c_str(), &(saddr.sin6_addr));
        return saddr;
    };

    Interfaces::Interfaces()
    {
        discovered = new std::unordered_map<std::string, std::vector<IPAddress>>();
    };

    void Interfaces::discover()
    {
#ifndef WIN32
        struct ifaddrs *ifap, *ifa;
        struct sockaddr_in *sockaddr;
        struct sockaddr_in6 *sockaddr6;

        getifaddrs(&ifap); // returns linked list

        // iterate over each interface
        for (ifa = ifap; ifa; ifa = ifa->ifa_next)
        {
            std::string iface_name = std::string(ifa->ifa_name);

            std::vector<IPAddress> *ips;
            // get or default
            if (discovered->find(iface_name) != discovered->end())
            {
                ips = &(discovered->at(iface_name));
            }
            else
            {
                ips = new std::vector<IPAddress>();
            }

            if (ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6))
            {
                // IPv[4|6] address found

                int family = ifa->ifa_addr->sa_family;

                in_addr_any addr;

                switch (family)
                {
                case AF_INET:
                {
                    struct sockaddr_in *sai = (struct sockaddr_in *)ifa->ifa_addr;
                    addr.ip4 = sai->sin_addr;
                    break;
                }
                case AF_INET6:
                {
                    struct sockaddr_in6 *sai6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                    addr.ip6 = sai6->sin6_addr;
                    break;
                }
                };

                IPAddress ip = IPAddress::fromInAddr(family, addr);
                ips->push_back(ip);
            }

            (*discovered)[iface_name] = *ips;
        };
#endif
    };

    void Interfaces::display()
    {
        for (auto iface : *discovered)
        {

            std::wcout << "[" << iface.first.c_str() << "]: ";
            for (auto addr : iface.second)
            {
                std::wcout << "(" << addr.getAddressStr().c_str() << ") ";
            }
            std::wcout << "\n";
        }

        std::wcout << "FB4: " << getFallbackAddress(AF_INET).getAddressStr().c_str() << "\n";
        std::wcout << "FB6: " << getFallbackAddress(AF_INET6).getAddressStr().c_str() << "\n";
    };

    IPAddress Interfaces::getFallbackAddress(int family)
    {
        bool hasNonLoopback = false; // has any non loopback

        IPAddress out(AF_INET, "127.0.0.1"); // default, TODO this is not ideal use std::optional instead

        for (auto iface : *discovered)
        {
            for (auto addr : iface.second)
            {
                if (addr.getFamily() == family)
                {

                    // decision tree

                    if (!hasNonLoopback && isLoopback(addr, iface.first))
                    {
                        out = addr;
                        continue;
                    }

                    if (!isLoopback(addr, iface.first))
                    {
                        out = addr;
                        hasNonLoopback = true;
                        continue;
                    }

                    // otherwise dont set
                }
            }
        }

        return out;
    }

    bool Interfaces::isLoopback(IPAddress ip, std::string iface)
    {
        return (iface.compare(loopbackInterface) == 0 || ip.getAddressStr().compare(loopback4) == 0 || ip.getAddressStr().compare(loopback6) == 0);
    }
}