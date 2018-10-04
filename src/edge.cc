/**
 * @file   edge.cc
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

#include <stddef.h>
#include "edge.h"
#include "graph.h"
#include "GraphImpl.h"
#include "TransactionImpl.h"

using namespace PMGD;

// It would be easier to lock the main call to edge but
// there are many locations to acquire edges. To ensure there
// is a lock held, it is better to lock within the edge API
// calls.
#define LOCK_EDGE(edge, write) { \
    TransactionImpl *tx = TransactionImpl::get_tx(); \
    tx->acquire_lock(TransactionImpl::EdgeLock, edge, write); \
}

void Edge::init(Node &src, Node &dest, StringID tag, unsigned obj_size)
{
    _src = &src;
    _dest = &dest;
    _tag = tag;
    _property_list.init(obj_size - offsetof(Edge, _property_list));
}

EdgeID Edge::get_id() const
{
    // The edge id acquired from the edge table which will not change unless
    // the edge is being removed. So read lock the edge.
    LOCK_EDGE(this, false);
    return TransactionImpl::get_tx()->get_db()->edge_table().get_id(this);
}

EdgeID EdgeRef::get_id() const
{
    // get_id() for Edge takes care of locking.
    return edge()->get_id();
}

StringID Edge::get_tag() const
{
    LOCK_EDGE(this, false);
    return _tag;
}

Node &Edge::get_source() const
{
    LOCK_EDGE(this, false);
    return *_src;
}

Node &Edge::get_destination() const
{
    LOCK_EDGE(this, false);
    return *_dest;
}

bool Edge::check_property(StringID id, Property &result) const
{
    LOCK_EDGE(this, false);
    return _property_list.check_property(id, result);
}

Property Edge::get_property(StringID id) const
{
    LOCK_EDGE(this, false);
    return _property_list.get_property(id);
}

PropertyIterator Edge::get_properties() const
{
    LOCK_EDGE(this, false);
    return _property_list.get_properties();
}

void Edge::set_property(StringID id, const Property &new_value)
{
    LOCK_EDGE(this, true);
    _property_list.set_property(id, new_value, Graph::EdgeIndex, _tag, this);
}

void Edge::remove_property(StringID id)
{
    LOCK_EDGE(this, true);
    _property_list.remove_property(id, Graph::EdgeIndex, _tag, this);
}

// Only called from Graph remove edge where edge is already locked.
void Edge::remove_all_properties()
    { _property_list.remove_all_properties(Graph::EdgeIndex, _tag, this); }
