/**
 * @file   EdgeIndex.h
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

#include <stddef.h>
#include "Allocator.h"
#include "stringid.h"
#include "List.h"
#include "exception.h"
#include "node.h"
#include "edge.h"
#include "KeyValuePair.h"

namespace PMGD {
    // This class sits in PM. No DRAM image. So create a pointer
    // and typecast
    class EdgeIndex {
        class EdgeIndexType;
    public:
        typedef KeyValuePair<Edge *, Node *> EdgeNodePair;
        typedef List<EdgeNodePair> EdgeList;
        typedef List<EdgeNodePair>::ListType EdgePosition;
        typedef List<EdgeIndexType>::ListType KeyPosition;
    private:
        class EdgeIndexType {
            // Tag values
            StringID _key;
            // List of Edge, (src/dest) Node references
            EdgeList _list;

        public:
            // Use for temp objects
            // List should get a default constructor which is fine
            EdgeIndexType(StringID key): _key(key), _list() {}

            // This is used inside add() for the data structure. That value
            // then gets flushed in there. So no need to log here
            EdgeIndexType& operator= (const EdgeIndexType &src)
            {
                _key = src._key;
                _list = src._list;
                return *this;
            }
            bool operator==(const EdgeIndexType& val2) const
            {
                return (_key == val2._key);
            }
            bool operator<(const EdgeIndexType& val2) const
            {
                return (_key < val2._key);
            }

            // Use when list exists
            void add(const EdgeNodePair &pair, Allocator &allocator)
            {
                _list.add(pair, allocator);
            }
            void remove(const EdgeNodePair &pair, Allocator &allocator)
            {
                _list.remove(pair, allocator);
            }
            size_t num_elems() { return _list.num_elems(); }

            // For iterators
            const EdgePosition *get_first() const { return _list.begin(); }
            const StringID &get_key() const { return _key; }
        };

        // Data structure for the keys that come in. Choosing a list
        // since there shouldn't be too many tags per node. Eventually
        // we can make this adaptive.
        // The second element is how the pairs will be organized.
        // Choosing a simple list for that too since all pairs are always
        // traversed
        List<EdgeIndexType> _key_list;

    public:
        // SInce each of the data structures track their own stuff
        // this doesn't need a header, just needs to make sure these
        // data structures place their headers correctly
        void init() { _key_list.init(); }
        static EdgeIndex *create(Allocator &allocator)
        {
            EdgeIndex *edge_table = (EdgeIndex *)allocator.alloc(sizeof *edge_table);
            edge_table->init();
            return edge_table;
        }

        static void free(EdgeIndex *edge_table, Allocator &allocator)
        {
            allocator.free(edge_table, sizeof *edge_table);
        }

        void add(const StringID key, Edge* edge, Node* node, Allocator &allocator);
        // For the iterator, give it head of PairList for the key
        const EdgePosition *get_first(StringID key);
        // For the iterator, give it head of the key list
        const KeyPosition *get_first() { return _key_list.begin(); }
        size_t num_elems() { return _key_list.num_elems(); }
        // This will remove the element based on edge pointer value
        void remove(const StringID key, Edge* edge, Allocator& allocator);
    };
}
