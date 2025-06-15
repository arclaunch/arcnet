#ifndef MDNS_MDNS_EMITTER_HPP_
#define MDNS_MDNS_EMITTER_HPP_

#include "arcnet/discovery/emitter.hpp"
#include "arcnet/discovery/mdns/service.hpp"

#include <optional>
#include <string>

namespace arcnet::discovery::mdns
{

    class MDNSOptions : public Options
    {
    public:
        std::optional<std::string> hostname;
        std::optional<sockaddr_in> target_v4;
        std::optional<sockaddr_in6> target_v6;

        int port;
        std::string service; // _example._tcp.local

        ~MDNSOptions() {};
    };

    class MDNSEmitter : public Emitter<MDNSOptions>
    {
    private:
        int sock;
        bool createSocket();

        std::string hostname;

        Service *svc;
        void determineLocalIP4();

        sockaddr_in *defaultTarget4();
        sockaddr_in6 *defaultTarget6();

    public:
        MDNSEmitter(event_base *base) : Emitter(base) {};

        bool configure(MDNSOptions *opt);
    };
}

#endif /* MDNS_MDNS_EMITTER_HPP_ */