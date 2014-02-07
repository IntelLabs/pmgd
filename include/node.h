#pragma once

#include "stringid.h"
#include "property.h"
#include "iterator.h"

namespace Jarvis {
    typedef uint64_t NodeID;

    enum Direction { ANY, OUTGOING, INCOMING };

    class Node {
        class EdgeList;
        EdgeList *_out_edges;
        EdgeList *_in_edges;
        StringID _tag;
        uint8_t _property_list[];

        Node(const Node &);
        ~Node();
        void operator=(const Node &);

    public:
        NodeID get_id() const;
        StringID get_tag() const { return _tag; }
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        EdgeIterator get_edges(Direction dir = ANY, StringID tag = NULL,
                               const PropertyPredicate & = PropertyPredicate()) const;
        void set_property(const Property &);
        void remove_property(StringID name);
    };
};
