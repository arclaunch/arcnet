#ifndef MDNS_SERVICE_HPP_
#define MDNS_SERVICE_HPP_

#include <string>
#include <vector>

#include <mdns.h>

namespace arcnet::discovery::mdns
{

    class Service
    {
    private:
        std::string service;
        std::string hostname;
        std::string service_instance;
        std::string hostname_qualified;

        int port;

        struct sockaddr_in addr_v4;
        struct sockaddr_in6 addr_v6;

        struct mdns_record_t a;
        struct mdns_record_t aaaa;
        struct mdns_record_t ptr;
        struct mdns_record_t srv;

        struct mdns_record_t ptr_sd;

        std::vector<struct mdns_record_t> optional_records;

        void toMdnsString(std::string src, mdns_string_t &dst);

        void createRecordA(mdns_record_t &record);
        void createRecordAAAA(mdns_record_t &record);
        void createRecordPTR(mdns_record_t &record);
        void createRecordPTRServiceDiscovery(mdns_record_t &record);
        void createRecordSRV(mdns_record_t &record);

    public:
        Service(std::string service, std::string hostname, int port, sockaddr_in addr_v4, sockaddr_in6 addr_v6);

        inline std::string getServiceName() { return this->service; };
        inline std::string getServiceInstanceName() { return this->service_instance; };
        inline std::string getQualifiedHostname() { return this->hostname_qualified; };

        inline struct sockaddr_in getAddr4() { return this->addr_v4; };
        inline struct sockaddr_in6 getAddr6() { return this->addr_v6; };

        inline struct mdns_record_t getRecordA() { return this->a; };
        inline struct mdns_record_t getRecordAAAA() { return this->aaaa; };
        inline struct mdns_record_t getRecordPTR() { return this->ptr; };
        inline struct mdns_record_t getRecordSRV() { return this->srv; };

        inline struct mdns_record_t getRecordPTRServiceDiscovery() { return this->ptr_sd; };

        inline std::vector<struct mdns_record_t> getOptionalRecords() { return this->optional_records; };

        bool announce(int sock);
    };
}

#endif /* MDNS_SERVICE_HPP_ */