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

using namespace Jarvis;

void Node::init(StringID tag, unsigned object_size, Allocator &index_allocator)
{
    _out_edges = EdgeIndex::create(index_allocator);
    _in_edges = EdgeIndex::create(index_allocator);
    _tag = tag;
    _property_list.init(object_size - offsetof(Node, _property_list));
}

void Node::add_edge(Edge *edge, Direction dir, StringID tag, Allocator &index_allocator)
{
    if (dir == Outgoing)
        _out_edges->add(tag, edge, &edge->get_destination(), index_allocator);
    else
        _in_edges->add(tag, edge, &edge->get_source(), index_allocator);
}

namespace Jarvis {
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
              _pos(pos)
        {
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

        operator bool() const { return _pos != NULL; }
        EdgeRef *ref() { return &_ref; }

        bool next()
        {
            _pos = _pos->next;
            if (_pos == NULL)
                _next();
            return _pos != NULL;
        }
    };
};

EdgeIterator Node::get_edges(Direction dir, StringID tag) const
{
    if (dir == Any)
        return get_edges(tag);
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

bool Jarvis::Node::check_property(StringID id, Property &result) const
    { return _property_list.check_property(id, result); }

Jarvis::Property Jarvis::Node::get_property(StringID id) const
    { return _property_list.get_property(id); }

Jarvis::PropertyIterator Jarvis::Node::get_properties() const
    { return _property_list.get_properties(); }

void Jarvis::Node::set_property(StringID id, const Property &new_value)
    { _property_list.set_property(id, new_value, Graph::NodeIndex, _tag, this); }

void Jarvis::Node::remove_property(StringID id)
    { _property_list.remove_property(id); }
