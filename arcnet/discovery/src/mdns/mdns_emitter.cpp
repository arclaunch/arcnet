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

        // create event
        ev = event_new(base, sock, EV_READ | EV_PERSIST, read_callback, (void *)this);

        event_add(ev, NULL); // no timeout

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

    void MDNSEmitter::read_callback(evutil_socket_t sock, short what, void *arg)
    {
        MDNSEmitter *inst = (MDNSEmitter *)arg;
        inst->handle_read(sock, what);
    };

    void MDNSEmitter::handle_read(evutil_socket_t sock, short what)
    {
        uint8_t *buf = new uint8_t[2048];
        mdns_socket_listen(sock, buf, 2048, service_callback, (void *)this);
    };

    int MDNSEmitter::service_callback(int sock, const struct sockaddr *from, size_t addrlen, mdns_entry_type_t entry,
                                      uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void *data,
                                      size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                                      size_t record_length, void *arg)
    {
        MDNSEmitter *inst = (MDNSEmitter *)arg;
        return inst->handle_svc_query(sock, from, addrlen, entry, query_id, rtype, rclass, ttl, data, size, name_offset, name_length, record_offset, record_length);
    };

    int MDNSEmitter::handle_svc_query(int sock, const struct sockaddr *from, size_t addrlen, mdns_entry_type_t entry,
                                      uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void *data,
                                      size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                                      size_t record_length)
    {

        mdns_record_type_t rtype_t = mdns_record_type_t(rtype);

        //        char *addrbuffer = new char[64];
        char *namebuffer = new char[256];
        char *sendbuffer = new char[1024];

        IPAddress from_addr = IPAddress(from->sa_family, from->sa_data);

        // get name
        size_t offset = name_offset;
        mdns_string_t name = mdns_string_extract(data, size, &offset, namebuffer, sizeof(namebuffer));

        // print record type

        // check if name is dns_sd
        if (dns_sd_svc.compare(name.str) == 0)
        {
            if (rtype == MDNS_RECORDTYPE_PTR || rtype == MDNS_RECORDTYPE_ANY)
            {
                // got PTR for dns-sd domain

                // comment from examples:
                // Answer PTR record reverse mapping "<_service-name>._tcp.local." to
                // "<hostname>.<_service-name>._tcp.local."

                // JC: use existing ptr
                mdns_record_t answer = svc->getRecordPTR();

                // see if requested format is unicast or multicast depending on query flag
                uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

                if (unicast)
                {
                    mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id, rtype_t, name.str, name.length, answer, 0, 0, 0, 0);
                }
                else
                {
                    mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, 0, 0);
                }
            }
            // check if query was our service
        }
        else if (svc->getServiceName().compare(name.str) == 0)
        {
            if (rtype == MDNS_RECORDTYPE_PTR || rtype == MDNS_RECORDTYPE_ANY)
            {
                // got PTR for our service
                // return PTR record mapping svc name to svc instance name
                // add record for SRV and any A/AAAA records, like single startup announce.

                mdns_record_t answer = svc->getRecordPTR();
                std::vector<mdns_record_t> optional_records = svc->getOptionalRecords();
                mdns_record_t *additional = &optional_records[0];
                size_t additional_count = optional_records.size();

                // see if requested format is unicast or multicast depending on query flag
                uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

                if (unicast)
                {
                    mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id, rtype_t, name.str, name.length, answer, 0, 0, additional, additional_count);
                }
                else
                {
                    mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, additional, additional_count);
                }
            }
        }
        else if (svc->getServiceInstanceName().compare(name.str) == 0)
        {
            if (rtype == MDNS_RECORDTYPE_SRV || rtype == MDNS_RECORDTYPE_ANY)
            {
                // srv query for our service instance (<host>.<_service-name>._<[tcp|udp]).local

                // return SRV rec to hostname + port, as well as any A/AAAA records
                mdns_record_t answer = svc->getRecordSRV();
                std::vector<mdns_record_t> optional_records = svc->getOptionalRecords();
                mdns_record_t *additional = &optional_records[0];
                size_t additional_count = optional_records.size();

                // see if requested format is unicast or multicast depending on query flag
                uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

                if (unicast)
                {
                    mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id, rtype_t, name.str, name.length, answer, 0, 0, additional, additional_count);
                }
                else
                {
                    mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, additional, additional_count);
                }
            }
        }
        else if (svc->getQualifiedHostname().compare(name.str) == 0)
        {
            if ((rtype == MDNS_RECORDTYPE_A || rtype == MDNS_RECORDTYPE_ANY) && (svc->getAddr4().sin_family == AF_INET))
            {
                // A query for our hostname, return A record

                mdns_record_t answer = svc->getRecordA();

                // TODO: add AAAA record to A answer like example impl?

                // see if requested format is unicast or multicast depending on query flag
                uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

                if (unicast)
                {
                    mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id, rtype_t, name.str, name.length, answer, 0, 0, 0, 0);
                }
                else
                {
                    mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, 0, 0);
                }
            }
            else if ((rtype == MDNS_RECORDTYPE_AAAA || rtype == MDNS_RECORDTYPE_ANY) && (svc->getAddr6().sin6_family == AF_INET6))
            {
                // AAAA query for our hostname, return AAAA record

                mdns_record_t answer = svc->getRecordAAAA();

                // see if requested format is unicast or multicast depending on query flag
                uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);

                if (unicast)
                {
                    mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id, rtype_t, name.str, name.length, answer, 0, 0, 0, 0);
                }
                else
                {
                    mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, 0, 0);
                }
            }
        };

        return 0;
    };
}