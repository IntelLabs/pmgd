#pragma once

#include "stringid.h"
#include "property.h"
#include "iterator.h"

namespace Jarvis {
    class Graph;

    class Edge {
        Node *_src;
        Node *_dest;
        StringID _tag;
        PropertyList _property_list;

        friend class Graph;
        void init(Node &src, Node &dest, StringID tag, unsigned object_size);
        void remove_all_properties();

    public:
        Edge(const Edge &) = delete;
        ~Edge() = delete;
        void operator=(const Edge &) = delete;
        StringID get_tag() const { return _tag; }
        Node &get_source() const { return *_src; }
        Node &get_destination() const { return *_dest; }
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        void set_property(StringID id, const Property &);
        void remove_property(StringID name);
    };

    inline bool EdgeRef::check_property(StringID id, Property &result) const
        { return edge()->check_property(id, result); }
    inline Property EdgeRef::get_property(StringID id) const
        { return edge()->get_property(id); }
    inline PropertyIterator EdgeRef::get_properties() const
        { return edge()->get_properties(); }
    inline void EdgeRef::set_property(StringID id, const Property &property)
        { return edge()->set_property(id, property); }
    inline void EdgeRef::remove_property(StringID id)
        { return edge()->remove_property(id); }
};
