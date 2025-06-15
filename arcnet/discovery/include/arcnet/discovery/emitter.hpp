#ifndef DISCOVERY_PRODUCER_HPP_
#define DISCOVERY_PRODUCER_HPP_

#include <event2/event.h>

#include "arcnet/discovery/interfaces.hpp"

namespace arcnet::discovery
{
    class Options
    {
    public:
        virtual ~Options() {}; // to make base class polymorphic
    };

    template <class O = Options>
    class Emitter
    {
    private:
        O *options;

    protected:
        event_base *base;
        Interfaces *interfaces;

    public:
        Emitter(event_base *base)
        {
            this->base = base;
            this->interfaces = new Interfaces();
            interfaces->discover();
        };

        virtual bool configure(O *opt) = 0;
    };
}

#endif /* DISCOVERY_PRODUCER_HPP_ */