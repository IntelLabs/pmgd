#pragma once
#include "property.h"
#include "iterator.h"

namespace Jarvis {
    class Node;
    class Allocator;
    class GraphImpl;

    // Base class for all the property value indices
    // Data resides in PM
    class Index {
        PropertyType _ptype;
    public:
        Index(PropertyType ptype) : _ptype(ptype) {}

        void add(const Property &p, void *n, GraphImpl *db);
        void remove(const Property &p, void *n, GraphImpl *db);
        void update(GraphImpl *db, void *n, const Property &new_value,
                    const Property &old_value);
        void check_type(const PropertyType ptype)
            { if (_ptype != ptype) throw Exception(PropertyTypeMismatch); }

        // Use a locale pointer here so that callers of get_nodes, where locale is
        // irrelevant, do not need to acquire it from the GraphImpl object.
        NodeIterator get_nodes(const PropertyPredicate &pp, std::locale *loc, bool reverse);
    };
}
