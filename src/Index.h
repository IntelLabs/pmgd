#pragma once
#include "property.h"
#include "iterator.h"
#include "graph.h"

namespace Jarvis {
    class Node;
    class Allocator;
    class GraphImpl;

    // Base class for all the property value indices
    // Data resides in PM
    class Index {
        PropertyType _ptype;
    public:
        class Index_IteratorImplIntf {
        public:
            virtual ~Index_IteratorImplIntf() { }
            virtual void *ref() const = 0;
            virtual operator bool() const = 0;
            virtual bool next() = 0;
        };

        Index(PropertyType ptype) : _ptype(ptype) {}

        void add(const Property &p, void *n, GraphImpl *db);
        void remove(const Property &p, void *n, GraphImpl *db);
        void check_type(const PropertyType ptype)
            { if (_ptype != ptype) throw Exception(PropertyTypeMismatch); }

        // Use a locale pointer here so that callers, where locale is
        // irrelevant, do not need to acquire it from the GraphImpl object.
        Index_IteratorImplIntf *get_iterator(const PropertyPredicate &pp, std::locale *loc, bool reverse);

        // Function to gather statistics
        Graph::IndexStats get_stats();
        void index_stats_info(Graph::IndexStats &stats);
    };
}
