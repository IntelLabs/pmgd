#pragma once

#include "stringid.h"
#include "iterator.h"

namespace Jarvis {
    typedef uint64_t EdgeID;

    class Edge {
        Edge(const Edge &);
        ~Edge();
        void operator=(const Edge &);
    public:
        EdgeID get_id() const;
        StringID get_tag() const;
        Node &get_source() const;
        Node &get_destination() const;
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        void set_property(const Property &);
        void remove_property(StringID name);
    };
};
