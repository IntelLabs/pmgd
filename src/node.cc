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
    if (dir == OUTGOING)
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
        Node &get_source() const { return ((_dir == INCOMING) ? *_pos->value.value() : *_n1); }
        Node &get_destination() const { return ((_dir == OUTGOING) ? *_pos->value.value() : *_n1); }

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
                _dir = OUTGOING;
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

    public:
        Node_EdgeIteratorImpl(EdgeIndex *idx, const Node *n, Direction dir, StringID tag)
            : _ref(this),
              _n1(const_cast<Node *>(n)),
              _dir(dir),
              _out_idx(NULL),
              _key_pos(NULL),
              _tag(tag),
              _pos(idx->get_first(_tag))
        { }
        Node_EdgeIteratorImpl(EdgeIndex *idx, const Node *n, Direction dir)
            : _ref(this),
              _n1(const_cast<Node *>(n)),
              _dir(dir),
              _out_idx(NULL),
              _key_pos(const_cast<EdgeIndex::KeyPosition *>(idx->get_first())),
              _tag(_key_pos->value.get_key()),
              _pos(_key_pos->value.get_first())
        { }

        // Always starts with incoming first
        Node_EdgeIteratorImpl(EdgeIndex *idx, EdgeIndex *out_idx, const Node *n, StringID tag)
            : _ref(this),
              _n1(const_cast<Node *>(n)),
              _dir(INCOMING),
              _out_idx((out_idx->num_elems() > 0) ? out_idx : NULL),
              _key_pos(NULL),
              _tag(tag),
              _pos(idx->get_first(_tag))
        {
            if (_pos == NULL)
                _next();
        }

        Node_EdgeIteratorImpl(EdgeIndex *idx, EdgeIndex *out_idx, const Node *n)
            : _ref(this),
              _n1(const_cast<Node *>(n)),
              _dir(INCOMING),
              _out_idx((out_idx->num_elems() > 0) ? out_idx : NULL),
              _key_pos(const_cast<EdgeIndex::KeyPosition *>(idx->get_first())),
              _tag(_key_pos->value.get_key()),
              _pos(_key_pos->value.get_first())
        {
            if (_pos == NULL)
                _next();
        }

        operator bool() const { return _pos != NULL; }
        const EdgeRef &operator*() const { return _ref; }
        const EdgeRef *operator->() const { return &_ref; }
        EdgeRef &operator*() { return _ref; }
        EdgeRef *operator->() { return &_ref; }

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
    if (dir == ANY)
        return get_edges(tag);
    EdgeIndex *idx = (dir == OUTGOING) ? _out_edges : _in_edges;
    return EdgeIterator(new Node_EdgeIteratorImpl(idx, this, dir, tag));
}

EdgeIterator Node::get_edges(Direction dir) const
{
    if (dir == ANY)
        return get_edges();
    EdgeIndex *idx = (dir == OUTGOING) ? _out_edges : _in_edges;
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
        return EdgeIterator(new Node_EdgeIteratorImpl(_out_edges, this, OUTGOING, tag));
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
        return EdgeIterator(new Node_EdgeIteratorImpl(_out_edges, this, OUTGOING));
    return EdgeIterator(new Node_EdgeIteratorImpl(_in_edges, _out_edges, this));
}

bool Jarvis::Node::check_property(StringID id, Property &result) const
    { return _property_list.check_property(id, result); }

Jarvis::Property Jarvis::Node::get_property(StringID id) const
    { return _property_list.get_property(id); }

Jarvis::PropertyIterator Jarvis::Node::get_properties() const
    { return _property_list.get_properties(); }

void Jarvis::Node::set_property(StringID id, const Property &new_value)
{
    // We have to first remove the old property from an existing index,
    // if any and then add new one. So use the fact that property_list.set_property
    // already searches for the old value and pass it to the index.
    Property old_value;
    TransactionImpl *tx = TransactionImpl::get_tx();
    GraphImpl *db = tx->get_db();

    // get_index returns NULL if the index doesn.t exist.
    // get_index throws if the index exists and the index type doesn't
    // match the property type.
    // The call to get_index has to be made before the call to set_property,
    // to ensure that set_property is not done if there is a type mismatch.
    Index *index = db->index_manager().get_index(Graph::NODE, _tag, id,
                                                 new_value.type());
    // Check if there is an index with this property id and tag = 0
    // This is a general, all tag index for certain properties like loader id.
    Index *gindex = db->index_manager().get_index(Graph::NODE, 0, id,
                                                  new_value.type());
    _property_list.set_property(id, new_value, old_value);

    if (index)
        index->update(db, this, new_value, old_value);
    if (gindex)
        gindex->update(db, this, new_value, old_value);
}

void Jarvis::Node::remove_property(StringID id)
    { _property_list.remove_property(id); }
