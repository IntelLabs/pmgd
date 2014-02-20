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

    inline bool EdgeRef::check_property(StringID id, Property &result) const
        { return edge()->check_property(id, result); }
    inline Property EdgeRef::get_property(StringID id) const
        { return edge()->get_property(id); }
    inline PropertyIterator EdgeRef::get_properties() const
        { return edge()->get_properties(); }
    inline void EdgeRef::set_property(const Property &property)
        { return edge()->set_property(property); }
    inline void EdgeRef::remove_property(StringID id)
        { return edge()->remove_property(id); }
};
