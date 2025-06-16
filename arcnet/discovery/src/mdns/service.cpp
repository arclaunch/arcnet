#include "arcnet/discovery/mdns/service.hpp"

#include <fmt/format.h>

namespace arcnet::discovery::mdns
{
    Service::Service(std::string service, std::string hostname, int port, sockaddr_in addr_v4, sockaddr_in6 addr_v6)
    {
        this->service = service;
        this->hostname = hostname;

        this->port = port;

        this->addr_v4 = addr_v4;
        this->addr_v6 = addr_v6;

        // Generate service_instance and hostname_qualified
        this->service_instance = fmt::format("{}.{}", hostname, service);
        this->hostname_qualified = fmt::format("{}.local.", hostname);

        // Create records
        createRecordA(a);
        createRecordAAAA(aaaa);
        createRecordPTR(ptr);
        createRecordSRV(srv);

        createRecordPTRServiceDiscovery(ptr_sd);
    };

    void Service::toMdnsString(std::string src, mdns_string_t &dst)
    {
        char *data = new char[src.length()];
        strcpy(data, src.c_str());

        dst.str = data;
        dst.length = src.length();
    };

    void Service::createRecordA(mdns_record_t &record)
    {
        toMdnsString(hostname_qualified, record.name);
        record.type = MDNS_RECORDTYPE_A;
        record.data.a.addr = addr_v4;
        record.rclass = 0;
        record.ttl = 0;

        if (addr_v4.sin_family = AF_INET)
            optional_records.push_back(record);
    };

    void Service::createRecordAAAA(mdns_record_t &record)
    {
        toMdnsString(hostname_qualified, record.name);
        record.type = MDNS_RECORDTYPE_AAAA;
        record.data.aaaa.addr = addr_v6;
        record.rclass = 0;
        record.ttl = 0;

        if (addr_v6.sin6_family = AF_INET)
            optional_records.push_back(record);
    };

    void Service::createRecordPTR(mdns_record_t &record)
    {
        toMdnsString(service, record.name);
        record.type = MDNS_RECORDTYPE_PTR;
        toMdnsString(service_instance, record.data.ptr.name);
        ptr.rclass = 0;
        ptr.ttl = 0;
    };

    void Service::createRecordPTRServiceDiscovery(mdns_record_t &record)
    {
        toMdnsString("_services._dns-sd._udp.local.", record.name);
        record.type = MDNS_RECORDTYPE_PTR;
        toMdnsString(service, record.data.ptr.name);
        ptr.rclass = 0;
        ptr.ttl = 0;
    };

    void Service::createRecordSRV(mdns_record_t &record)
    {
        toMdnsString(service_instance, record.name);
        record.type = MDNS_RECORDTYPE_SRV;
        toMdnsString(hostname_qualified, record.data.srv.name);
        record.data.srv.port = port;
        record.data.srv.priority = 0;
        record.data.srv.weight = 0;
        record.rclass = 0;
        record.ttl = 0;

        optional_records.push_back(record);
    };

    bool Service::announce(int sock)
    {
        int buf_size = 2048;
        uint8_t *buf = new uint8_t[buf_size];

        // pass a pointer to the vector (whose data is contiguous)
        // https://stackoverflow.com/a/2923290
        mdns_record_t *additional = &optional_records[0];
        size_t additional_count = optional_records.size();

        int result = mdns_announce_multicast(sock, buf, buf_size, ptr, 0, 0, additional, additional_count);

        // <0 error, 0 success
        return result == 0;
    };
}