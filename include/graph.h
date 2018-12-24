/**
 * @file   graph.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <vector>
#include "stringid.h"
#include "iterator.h"
#include "property.h"

namespace PMGD {
    class GraphImpl;
    class Node;
    class Edge;

    typedef uint64_t NodeID;
    typedef uint64_t EdgeID;

    class Graph {
        friend class Transaction;
        friend class Allocator;
        GraphImpl *_impl;

    public:
        // In case of msync, MsyncOnCommit is the default.
        enum OpenOptions { ReadWrite = 0, Create = 1, ReadOnly = 2, NoMsync = 4,
                           MsyncOnCommit = 8, AlwaysMsync = 12};

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
            size_t allocator_region_size;

            unsigned num_allocators;

            // The parameters below are DRAM-based parameters that can be
            // modified each time the graph is created/opened. The variables
            // above are PM-based parameters which are fixed once the graph
            // is created. locale_name is not included in
            // the DRAM variable list.
            size_t default_striped_lock_size; // bytes
            size_t node_striped_lock_size;    // bytes
            size_t edge_striped_lock_size;    // bytes
            size_t index_striped_lock_size;   // bytes

            // Indicate how many bytes get covered by one
            // bucket in the stripe lock. Keep it modular per
            // lock stripe since nodes, edges and index nodes
            // can be of very different sizes.
            unsigned default_stripe_width;
            unsigned node_stripe_width;
            unsigned edge_stripe_width;
            unsigned index_stripe_width;

            std::string locale_name;

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

        //Stats
        struct IndexStats {
            size_t total_unique_entries;
            size_t unique_entry_size;
            size_t total_elements;
            size_t total_size_bytes;
            size_t health_factor; // [0,100] - 100 is best health
        };

        struct ChunkStats {
            size_t total_chunks;
            size_t chunk_size;
            size_t num_elements;
            size_t total_size_bytes;
            size_t health_factor; // [0,100] - 100 is best health
        };

        IndexStats get_index_stats();
        IndexStats get_index_stats(IndexType index_type);
        IndexStats get_index_stats(IndexType index_type, StringID tag);
        IndexStats get_index_stats(IndexType index_type, StringID tag, StringID property_id);

        ChunkStats get_all_chunk_lists_stats();
        ChunkStats get_chunk_list_stats(IndexType index_type);
        ChunkStats get_chunk_list_stats(IndexType index_type, StringID tag);

        struct AllocatorStats {
            std::string name;
            unsigned long long object_size;
            unsigned long long num_objects;
            unsigned long long total_allocated_bytes;
            unsigned long long region_size;
            unsigned occupancy;     // [0 - 100]
            unsigned health_factor; // [0 - 100]
        };

        std::vector<AllocatorStats> get_allocator_stats();
    };
};
