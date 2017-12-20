/**
 * @file   Index.h
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
#include "property.h"
#include "iterator.h"
#include "graph.h"

namespace PMGD {
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
            { if (_ptype != ptype) throw PMGDException(PropertyTypeMismatch); }

        // Use a locale pointer here so that callers, where locale is
        // irrelevant, do not need to acquire it from the GraphImpl object.
        Index_IteratorImplIntf *get_iterator(Graph::IndexType index_type,
                                             const PropertyPredicate &pp, std::locale *loc,
                                             bool reverse);

        // Function to gather statistics
        Graph::IndexStats get_stats();
        void index_stats_info(Graph::IndexStats &stats);
    };
}
