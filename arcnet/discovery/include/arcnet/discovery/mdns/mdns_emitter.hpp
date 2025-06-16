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

        const std::string dns_sd_svc = std::string("_services._dns-sd._udp.local.");

    public:
        static constexpr int PORT = 5353;

        MDNSEmitter(event_base *base) : Emitter(base) {};

        bool configure(MDNSOptions *opt);

        static void read_callback(evutil_socket_t sock, short what, void *arg);

        void handle_read(evutil_socket_t sock, short what);

        // mdns service callback - handle questions incoming on service sock
        static int service_callback(int sock, const struct sockaddr *from, size_t addrlen, mdns_entry_type_t entry,
                                    uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void *data,
                                    size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                                    size_t record_length, void *arg);

        int handle_svc_query(int sock, const struct sockaddr *from, size_t addrlen, mdns_entry_type_t entry,
                             uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void *data,
                             size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                             size_t record_length);
    };
}

#endif /* MDNS_MDNS_EMITTER_HPP_ */