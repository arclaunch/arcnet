#include "arcnet/discovery/interfaces.hpp"
#include "arcnet/discovery/mdns/mdns_emitter.hpp"

#include <event2/event.h>

using namespace arcnet::discovery;
using namespace arcnet::discovery::mdns;

int main(void)
{

    Interfaces *ifaces = new Interfaces();

    ifaces->discover();

    ifaces->display();

    MDNSOptions *opt = new MDNSOptions();
    opt->port = 5432;
    opt->service = "_test._udp.local";

    struct event_base *base = event_base_new();

    MDNSEmitter *emit = new MDNSEmitter(base);
    emit->configure(opt);

    return 0;
}