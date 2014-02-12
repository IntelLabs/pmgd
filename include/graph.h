#pragma once

#include "stringid.h"
#include "node.h"
#include "edge.h"
#include "iterator.h"

namespace Jarvis {
    typedef uint64_t NodeID;
    typedef uint64_t EdgeID;

    class Graph {
        class GraphImpl;
        GraphImpl *_impl;

        Graph(const Graph &);
        void operator=(const Graph &);

    public:
        enum OpenOptions { Create = 1, ReadOnly = 2 };
        Graph(const char *name, int options = 0);
        ~Graph();

        NodeID get_id(const Node &) const;
        EdgeID get_id(const Edge &) const;

        NodeIterator get_nodes();
        NodeIterator get_nodes(StringID tag);
        NodeIterator get_nodes(StringID tag, const PropertyPredicate &);

        EdgeIterator get_edges();
        EdgeIterator get_edges(StringID tag);
        EdgeIterator get_edges(StringID tag, const PropertyPredicate &);

        PathIterator get_paths(Node &a, bool depth_first = false);
        PathIterator get_paths(Node &a, Node &b, bool depth_first = false);

        Node &add_node(StringID tag);
        Edge &add_edge(Node &source, Node &destination, StringID tag);
        void remove(Node &node);
        void remove(Edge &edge);

        enum { NODE=1, EDGE=2 };
        void create_index(int node_or_edge, StringID tag, StringID property_id);
    };
};
