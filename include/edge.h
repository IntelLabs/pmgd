/**
 * @file   edge.h
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
    typedef uint64_t EdgeID;

    class Edge {
        Node *_src;
        Node *_dest;
        StringID _tag;
        PropertyList _property_list;

        friend class Graph;
        void init(Node &src, Node &dest, StringID tag, unsigned object_size);
        void remove_all_properties();

    public:
        Edge(const Edge &) = delete;
        ~Edge() = delete;
        void operator=(const Edge &) = delete;
        EdgeID get_id() const;
        StringID get_tag() const;
        Node &get_source() const;
        Node &get_destination() const;
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID property) const;
        PropertyIterator get_properties() const;
        void set_property(StringID id, const Property &);
        void remove_property(StringID name);
    };

    inline bool EdgeRef::check_property(StringID id, Property &result) const
        { return edge()->check_property(id, result); }
    inline Property EdgeRef::get_property(StringID id) const
        { return edge()->get_property(id); }
    inline PropertyIterator EdgeRef::get_properties() const
        { return edge()->get_properties(); }
    inline void EdgeRef::set_property(StringID id, const Property &property)
        { return edge()->set_property(id, property); }
    inline void EdgeRef::remove_property(StringID id)
        { return edge()->remove_property(id); }
};
