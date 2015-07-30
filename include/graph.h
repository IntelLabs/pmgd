#pragma once

#include <vector>
#include "stringid.h"
#include "node.h"
#include "edge.h"
#include "iterator.h"
#include "property.h"

namespace Jarvis {
    class GraphImpl;

    typedef uint64_t NodeID;
    typedef uint64_t EdgeID;

    class Graph {
        friend class Transaction;
        GraphImpl *_impl;

    public:
        enum OpenOptions { ReadWrite = 0, Create = 1, ReadOnly = 2 };

        struct Config {
            struct AllocatorInfo {
                unsigned object_size;  // size of objects in this pool
                size_t size;           // size of pool in bytes
            };

            unsigned node_size;
            unsigned edge_size;
            unsigned max_stringid_length;

            size_t default_region_size;
            size_t node_table_size;
            size_t edge_table_size;
            size_t string_table_size;
            size_t transaction_table_size;
            size_t journal_size;

            std::string locale_name;
            std::vector<AllocatorInfo> fixed_allocators;

            Config();
        };

        Graph(const Graph &) = delete;
        void operator=(const Graph &) = delete;
        Graph(const char *name, int options = ReadWrite, const Config * = NULL);
        ~Graph();

        NodeID get_id(const Node &) const;
        EdgeID get_id(const Edge &) const;

        NodeIterator get_nodes();
        NodeIterator get_nodes(StringID tag);
        NodeIterator get_nodes(StringID tag, const PropertyPredicate &, bool reverse = false);

        EdgeIterator get_edges();
        EdgeIterator get_edges(StringID tag);
        EdgeIterator get_edges(StringID tag, const PropertyPredicate &, bool reverse = false);

        Node &add_node(StringID tag);
        Edge &add_edge(Node &source, Node &destination, StringID tag);
        void remove(Node &node);
        void remove(Edge &edge);

        enum IndexType { NodeIndex, EdgeIndex };
        void create_index(IndexType index_type, StringID tag,
                          StringID property_id, const PropertyType ptype);
    };
};
