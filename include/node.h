/**
 * @file   node.h
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

#include "stringid.h"
#include "property.h"
#include "iterator.h"

namespace PMGD {
    class Graph;
    class EdgeIndex;
    class Allocator;

    typedef uint64_t NodeID;
    enum Direction { Any, Outgoing, Incoming };

    class Node {
        EdgeIndex *_out_edges;
        EdgeIndex *_in_edges;
        StringID _tag;
        PropertyList _property_list;

        friend class Graph;
        void init(StringID tag, unsigned object_size,
                  Allocator &index_allocator);
        void cleanup(Allocator &index_allocator);
        void add_edge(Edge *edge, Direction dir, StringID tag,
                      Allocator &index_allocator);
        void remove_all_properties();
        void remove_edge(Edge *edge, Direction dir, Allocator &index_allocator);

    public:
        Node(const Node &) = delete;
        ~Node() = delete;
        void operator=(const Node &) = delete;
        bool operator==(const Node &n) const { return &n == this; }
        bool operator!=(const Node &n) const { return &n != this; }
        NodeID get_id() const;
        StringID get_tag() const { return _tag; }
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        Node &get_neighbor(Direction dir, StringID edge_tag) const;
        EdgeIterator get_edges() const;
        EdgeIterator get_edges(Direction dir) const;
        EdgeIterator get_edges(StringID tag) const;
        EdgeIterator get_edges(Direction dir, StringID tag) const;
        void set_property(StringID id, const Property &);
        void remove_property(StringID name);
    };
};
