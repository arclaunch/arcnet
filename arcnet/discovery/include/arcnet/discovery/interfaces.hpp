#ifndef DISCOVERY_INTERFACE_HPP_
#define DISCOVERY_INTERFACE_HPP_

#include <string>
#include <vector>
#include <unordered_map>

#ifndef WIN32
#include <arpa/inet.h>
#endif

namespace arcnet::discovery
{

    class IPAddress
    {
    private:
        int family;
        std::string str;

    public:
        IPAddress(int family, std::string str);

        inline int getFamily() { return this->family; };
        inline std::string getAddressStr() { return this->str; };

        int getAddressStrLen();
        sockaddr_in pton4();
        sockaddr_in6 pton6();
    };

    class Interfaces
    {
    private:
        union in_addr_any
        {
            struct in_addr ip4;
            struct in6_addr ip6;
        };

        const std::string loopbackInterface = std::string("lo");
        const std::string loopback4 = std::string("127.0.0.1");
        const std::string loopback6 = std::string("::1");

        std::unordered_map<std::string, std::vector<IPAddress>> *discovered;

    public:
        Interfaces();

        void discover();

        void display();

        IPAddress getFallbackAddress(int family);

        bool isLoopback(IPAddress ip, std::string iface);
    };
}

#endif /* DISCOVERY_INTERFACE_HPP_ */