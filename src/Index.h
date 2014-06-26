#pragma once
#include "property.h"
#include "iterator.h"

namespace Jarvis {
    class Node;
    class Allocator;
    class GraphImpl;

    // Base class for all the property value indices
    class Index {
        PropertyType _ptype;
    public:
        void init(PropertyType ptype);
        void add(const Property &p, Node *n, GraphImpl *db);
        void remove(const Property &p, Node *n, GraphImpl *db);
        void check_type(const PropertyType ptype)
            { if (_ptype != ptype) throw Exception(property_type); }

        // Use a locale pointer here so that callers of get_nodes, where locale is
        // irrelevant, do not need to acquire it from the GraphImpl object. 
        NodeIterator get_nodes(const PropertyPredicate &pp, std::locale *loc);
    };
}
