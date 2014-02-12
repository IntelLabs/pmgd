#pragma once

#include "stringid.h"
#include "iterator.h"

namespace Jarvis {
    class Graph;

    class Edge {
        Node *_src;
        Node *_dest;
        StringID _tag;
        uint8_t _property_list[];

        friend class Graph;
        void init(Node &src, Node &dest, StringID tag);

        Edge(const Edge &);
        ~Edge();
        void operator=(const Edge &);

    public:
        StringID get_tag() const { return _tag; }
        Node &get_source() const { return *_src; }
        Node &get_destination() const { return *_dest; }
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        void set_property(const Property &);
        void remove_property(StringID name);
    };
};
