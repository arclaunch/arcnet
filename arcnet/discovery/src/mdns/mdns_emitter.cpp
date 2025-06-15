#include "arcnet/discovery/mdns/mdns_emitter.hpp"

#include <mdns.h>
#include <iostream>

#include <boost/asio/ip/host_name.hpp>

using arcnet::discovery::Options;

namespace arcnet::discovery::mdns
{
    bool MDNSEmitter::configure(MDNSOptions *opt)
    {
        // attempt to create socket
        if (!createSocket())
        {
            return false;
        }

        // set hostname if was given, else determine
        this->hostname = opt->hostname.value_or(boost::asio::ip::host_name());

        sockaddr_in addr_v4 = opt->target_v4.value_or(interfaces->getFallbackAddress(AF_INET).pton4());
        sockaddr_in6 addr_v6 = opt->target_v6.value_or(interfaces->getFallbackAddress(AF_INET6).pton6());

        // create service (inc records)
        this->svc = new Service(opt->service, hostname, opt->port, addr_v4, addr_v6);

        svc->announce(sock);

        return true;
    };

    bool MDNSEmitter::createSocket()
    {
        sock = mdns_socket_open_ipv4(INADDR_ANY);

        if (sock < 0)
        {
            std::wcerr << "Failed to create socket.";
            return false;
        }

        if (evutil_make_socket_nonblocking(sock) < 0)
        {
            std::wcerr << "Failed to make socket non-blocking.";
            return false;
        }

        this->sock = sock;
        return true;
    };

    sockaddr_in *MDNSEmitter::defaultTarget4()
    {
        struct sockaddr_in *addr = new sockaddr_in(); // zero initialized

        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = htonl(0); // 0.0.0.0

        return addr;
    };
}