/**
 * @file   node.cc
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
#include <string.h> // for memcpy
#include "exception.h"
#include "iterator.h"
#include "property.h"
#include "node.h"
#include "edge.h"
#include "EdgeIndex.h"
#include "GraphImpl.h"
#include "IndexManager.h"
#include "TransactionImpl.h"

using namespace PMGD;

void Node::init(StringID tag, unsigned object_size, Allocator &index_allocator)
{
    _out_edges = EdgeIndex::create(index_allocator);
    _in_edges = EdgeIndex::create(index_allocator);
    _tag = tag;
    _property_list.init(object_size - offsetof(Node, _property_list));
}

void Node::cleanup(Allocator &index_allocator)
{
    EdgeIndex::free(_out_edges, index_allocator);
    EdgeIndex::free(_in_edges, index_allocator);
}

NodeID Node::get_id() const
{
    // Node should already be locked since its reference was acquired.
    return TransactionImpl::get_tx()->get_db()->node_table().get_id(this);
}

void Node::add_edge(Edge *edge, Direction dir, StringID tag, Allocator &index_allocator)
{
    // Upgrade the reader lock to writer.
    TransactionImpl::lock_node(this, true);
    if (dir == Outgoing)
        _out_edges->add(tag, edge, &edge->get_destination(), index_allocator);
    else
        _in_edges->add(tag, edge, &edge->get_source(), index_allocator);
}

void Node::remove_edge(Edge *edge, Direction dir, Allocator &index_allocator)
{
    TransactionImpl::lock_node(this, true);
    if (dir == Outgoing)
        _out_edges->remove(edge->get_tag(), edge, index_allocator);
    else
        _in_edges->remove(edge->get_tag(), edge, index_allocator);
}

Node &Node::get_neighbor(Direction dir, StringID edge_tag) const
{
    if (dir == Outgoing || dir == Any) {
        const EdgeIndex::EdgePosition *pos = _out_edges->get_first(edge_tag);
        if (pos) {
            Node *n = pos->value.value();
            TransactionImpl::lock_node(n, false);
            return *n;
        }
    }
    if (dir == Incoming || dir == Any) {
        const EdgeIndex::EdgePosition *pos = _in_edges->get_first(edge_tag);
        if (pos) {
            Node *n = pos->value.value();
            TransactionImpl::lock_node(n, false);
            return *n;
        }
    }
    throw PMGDException(NullIterator);
}

namespace PMGD {
    // TODO Make the lists more opaque to this class
    class Node_EdgeIteratorImpl : public EdgeIteratorImplIntf {
        EdgeRef _ref;

        // This has to be non-const because it is representing ptr
        // to the node inside an edge which is non-const
        Node *_n1;
        // Direction changes if _out_idx != NULL
        Direction _dir;
        // This _out_idx pointer is just an indicator for switching directions
        // Set it to NULL if no elements
        EdgeIndex *_out_idx;
        // No tag given if _key_pos != NULL
        EdgeIndex::KeyPosition *_key_pos;
        StringID _tag;
        const EdgeIndex::EdgePosition *_pos;
        bool _vacant_flag = false;
        TransactionImpl *_tx;

        friend class EdgeRef;
        Edge *get_edge() const { return _pos->value.key(); }
        StringID get_tag() const { return _tag; }
        Node &get_source() const { return ((_dir == Incoming) ? *_pos->value.value() : *_n1); }
        Node &get_destination() const { return ((_dir == Outgoing) ? *_pos->value.value() : *_n1); }

        // When _pos is NULL, move to next key or direction
        void _next()
        {
            if (_key_pos != NULL) {
                // Case with no tag specified
                // Move to the next EdgeIndexType
                _key_pos = _key_pos->next;
                if (_key_pos != NULL) {
                    _tag = _key_pos->value.get_key();
                    // Move to the head of <Edge,Node> list for
                    // tag _tag in EdgeIndexType
                    _pos = _key_pos->value.get_first();
                }
                else // Tag was not specified
                    switch_direction(true);
            }
            else // Case with tag specified
                switch_direction(false);
        }

        // Move to out edges and change _key_pos, _pos if !NULL
        void switch_direction(bool no_tag)
        {
            if (_out_idx != NULL) {
                _dir = Outgoing;
                if (no_tag) {
                    _key_pos = const_cast<EdgeIndex::KeyPosition *>(_out_idx->get_first());
                    _tag = _key_pos->value.get_key();
                    _pos = _key_pos->value.get_first();
                }
                else {
                    _pos = _out_idx->get_first(_tag);
                }
                // Set out idx to NULL now since we already moved to next dir
                _out_idx = NULL;
            }
        }

        Node_EdgeIteratorImpl(const Node *n,
                              Direction dir,
                              EdgeIndex *out_idx,
                              const EdgeIndex::KeyPosition *key_pos,
                              StringID tag,
                              const EdgeIndex::EdgePosition *pos)
            : _ref(this),
              _n1(const_cast<Node *>(n)),
              _dir(dir),
              _out_idx(out_idx),
              _key_pos(const_cast<EdgeIndex::KeyPosition *>(key_pos)),
              _tag(tag),
              _pos(pos),
              _tx(TransactionImpl::get_tx())
        {
            if (_tx->is_read_write()) {
                _tx->iterator_callbacks().register_iterator(this,
                        [this](void *list_node) { remove_notify(list_node); });
            }
        }

        Node_EdgeIteratorImpl(const Node *n, Direction dir, EdgeIndex *out_idx,
                              const EdgeIndex::KeyPosition *key_pos)
            : Node_EdgeIteratorImpl(n, dir, out_idx, key_pos,
                                    key_pos->value.get_key(),
                                    key_pos->value.get_first())
        {
        }

    public:
        Node_EdgeIteratorImpl(EdgeIndex *idx, const Node *n, Direction dir, StringID tag)
            : Node_EdgeIteratorImpl(n, dir, NULL, NULL, tag, idx->get_first(tag))
        { }

        Node_EdgeIteratorImpl(EdgeIndex *idx, const Node *n, Direction dir)
            : Node_EdgeIteratorImpl(n, dir, NULL, idx->get_first())
        { }

        // Always starts with incoming first
        Node_EdgeIteratorImpl(EdgeIndex *idx, EdgeIndex *out_idx, const Node *n, StringID tag)
            : Node_EdgeIteratorImpl(n, Incoming,
                                    out_idx->num_elems() > 0 ? out_idx : NULL,
                                    NULL, tag, idx->get_first(tag))
        {
            if (_pos == NULL)
                _next();
        }

        Node_EdgeIteratorImpl(EdgeIndex *idx, EdgeIndex *out_idx, const Node *n)
            : Node_EdgeIteratorImpl(n, Incoming,
                                    out_idx->num_elems() > 0 ? out_idx : NULL,
                                    idx->get_first())
        {
            if (_pos == NULL)
                _next();
        }

        ~Node_EdgeIteratorImpl()
        {
            if (_tx->is_read_write())
                _tx->iterator_callbacks().unregister_iterator(this);
        }

        operator bool() const { return _vacant_flag || _pos != NULL; }

        EdgeRef *ref()
        {
            // _vacant_flag indicates that the edge referred to by the iterator
            // has been deleted.
            if (_vacant_flag)
                throw PMGDException(VacantIterator);
            return &_ref;
        }

        bool next()
        {
            // If _vacant_flag is set, the iterator has already been advanced to
            // the next edge, so we just clear _vacant_flag.
            if (_vacant_flag) {
                _vacant_flag = false;
                return operator bool();
            }
            if (_pos != NULL)
                _pos = _pos->next;
            if (_pos == NULL)
                _next();
            return _pos != NULL;
        }

        void remove_notify(void *list_node)
        {
            if (list_node == _pos) {
                // Clear _vacant_flag to ensure that next actually advances
                // the iterator, and then set _vacant_flag to indicate that
                // the current edge has been deleted.
                _vacant_flag = false;
                next();
                _vacant_flag = true;
            }
        }
    };
};

EdgeIterator Node::get_edges(Direction dir, StringID tag) const
{
    if (dir == Any)
        return get_edges(tag);
    if (tag == 0)
        return get_edges(dir);
    EdgeIndex *idx = (dir == Outgoing) ? _out_edges : _in_edges;
    return EdgeIterator(new Node_EdgeIteratorImpl(idx, this, dir, tag));
}

EdgeIterator Node::get_edges(Direction dir) const
{
    if (dir == Any)
        return get_edges();
    EdgeIndex *idx = (dir == Outgoing) ? _out_edges : _in_edges;
    // Ensure there is at least one element in this index for
    // the constructors to not crash. Not needed when you pass
    // tag value because that takes care of returning NULL at the
    // right point
    if (idx->num_elems() <= 0)
        return EdgeIterator(NULL);
    return EdgeIterator(new Node_EdgeIteratorImpl(idx, this, dir));
}

EdgeIterator Node::get_edges(StringID tag) const
{
    if (tag == 0)
        return get_edges();
    size_t in_elems = _in_edges->num_elems();
    size_t out_elems = _out_edges->num_elems();
    // Ensure there is at least one element in this index for
    // the constructors to not crash. Not needed when you pass
    // tag value because that takes care of returning NULL at the
    // right point
    if (in_elems <= 0 && out_elems <= 0)
        return EdgeIterator(NULL);
    if (in_elems <= 0)
        return EdgeIterator(new Node_EdgeIteratorImpl(_out_edges, this, Outgoing, tag));
    return EdgeIterator(new Node_EdgeIteratorImpl(_in_edges, _out_edges, this, tag));
}

EdgeIterator Node::get_edges() const
{
    size_t in_elems = _in_edges->num_elems();
    size_t out_elems = _out_edges->num_elems();
    // Ensure there is at least one element in this index for
    // the constructors to not crash. Not needed when you pass
    // tag value because that takes care of returning NULL at the
    // right point
    if (in_elems <= 0 && out_elems <= 0)
        return EdgeIterator(NULL);
    if (in_elems <= 0)
        return EdgeIterator(new Node_EdgeIteratorImpl(_out_edges, this, Outgoing));
    return EdgeIterator(new Node_EdgeIteratorImpl(_in_edges, _out_edges, this));
}

bool PMGD::Node::check_property(StringID id, Property &result) const
{
    return _property_list.check_property(id, result);
}

PMGD::Property PMGD::Node::get_property(StringID id) const
{
    return _property_list.get_property(id);
}

PMGD::PropertyIterator PMGD::Node::get_properties() const
{
    return _property_list.get_properties();
}

void PMGD::Node::set_property(StringID id, const Property &new_value)
{
    TransactionImpl::lock_node(this, true);
    _property_list.set_property(id, new_value, Graph::NodeIndex, _tag, this);
}

void PMGD::Node::remove_property(StringID id)
{
    TransactionImpl::lock_node(this, true);
    _property_list.remove_property(id, Graph::NodeIndex, _tag, this);
}

void PMGD::Node::remove_all_properties()
{
    // This function is only called from remove_node in Graph.
    // The node is already write locked in that case. So no need to lock.
    _property_list.remove_all_properties(Graph::NodeIndex, _tag, this);
}
