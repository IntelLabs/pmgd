#pragma once

#include "stringid.h"
#include "property.h"
#include "iterator.h"

namespace Jarvis {
    class Graph;
    class EdgeIndex;
    class Allocator;

    enum Direction { ANY, OUTGOING, INCOMING };

    class Node {
        EdgeIndex *_out_edges;
        EdgeIndex *_in_edges;
        StringID _tag;
        PropertyList _property_list;

        friend class Graph;
        void init(StringID tag, unsigned object_size,
                  Allocator &index_allocator);
        void add_edge(Edge *edge, Direction dir, StringID tag,
                      Allocator &index_allocator);

        Node(const Node &);
        ~Node();
        void operator=(const Node &);

    public:
        StringID get_tag() const { return _tag; }
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        EdgeIterator get_edges() const;
        EdgeIterator get_edges(Direction dir) const;
        EdgeIterator get_edges(StringID tag) const;
        EdgeIterator get_edges(Direction dir, StringID tag) const;
        void set_property(const Property &);
        void remove_property(StringID name);
    };
};
