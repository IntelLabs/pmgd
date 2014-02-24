#include <stddef.h>
#include <assert.h>
#include <string.h> // for memcpy
#include "exception.h"
#include "iterator.h"
#include "property.h"
#include "node.h"
#include "edge.h"
#include "EdgeIndex.h"

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
    class NodeEdgeIteratorImpl : public EdgeIteratorImpl {
        EdgeRef _ref;

        // This has to be non-const because it is representing ptr
        // to the node inside an edge which is non-const
        Node *_n1;
        const EdgeIndex::EdgePosition *_pos;
        const EdgeIndex::KeyPosition *_key_pos;
        Direction _dir;
        StringID _tag;

        friend class EdgeRef;
        Edge *get_edge() const { return _pos->value.key(); }
        StringID get_tag() const { return _tag; }
        Node &get_source() const { return ((_dir == INCOMING) ? *_pos->value.value() : *_n1); }
        Node &get_destination() const { return ((_dir == OUTGOING) ? *_pos->value.value() : *_n1); }

    public:
        NodeEdgeIteratorImpl(EdgeIndex *idx, const Node *n, Direction dir, StringID tag)
            : _ref(this), _n1(const_cast<Node *>(n)), _dir(dir), _tag(tag)
        {
            _pos = idx->get_first(_tag);
            _key_pos = NULL;
        }
        NodeEdgeIteratorImpl(EdgeIndex *idx, const Node *n, Direction dir)
            : _ref(this), _n1(const_cast<Node *>(n)), _key_pos(idx->get_first()),
              _dir(dir), _tag(_key_pos->value.get_key())
        {
            _pos = _key_pos->value.get_first();
        }
        operator bool() const { return _pos != NULL; }
        const EdgeRef &operator*() const { return _ref; }
        const EdgeRef *operator->() const { return &_ref; }
        EdgeRef &operator*() { return _ref; }
        EdgeRef *operator->() { return &_ref; }
        bool next()
        {
            _pos = _pos->next; 
            if (_pos == NULL && _key_pos != NULL) {
                // Move to the next EdgeIndexType
                _key_pos = _key_pos->next;
                if (_key_pos != NULL) {
                    _tag = _key_pos->value.get_key();
                    // Move to the head of <Edge,Node> list for 
                    // tag _tag in EdgeIndexType
                    _pos = _key_pos->value.get_first();
                }
            }
            return _pos != NULL;
        }
    };
};

EdgeIterator Node::get_edges(Direction dir, StringID tag) const
{
    if (dir == ANY) {
        throw e_not_implemented;
    }
    EdgeIndex *idx = (dir == OUTGOING) ? _out_edges : _in_edges;
    return EdgeIterator(new NodeEdgeIteratorImpl(idx, this, dir, tag));
}

EdgeIterator Node::get_edges(Direction dir) const
{
    if (dir == ANY) {
        throw e_not_implemented;
    }
    EdgeIndex *idx = (dir == OUTGOING) ? _out_edges : _in_edges;
    // Ensure there is at least one element in this index for 
    // the constructors to not crash. Not needed when you pass 
    // tag value because that takes care of returning NULL at the
    // right point
    if (idx->num_elems() <= 0)
        return EdgeIterator(NULL);
    return EdgeIterator(new NodeEdgeIteratorImpl(idx, this, dir));
}

bool Jarvis::Node::check_property(StringID id, Property &result) const
    { return _property_list.check_property(id, result); }

Jarvis::Property Jarvis::Node::get_property(StringID id) const
    { return _property_list.get_property(id); }

Jarvis::PropertyIterator Jarvis::Node::get_properties() const
    { return _property_list.get_properties(); }

void Jarvis::Node::set_property(const Property &p)
    { _property_list.set_property(p); }

void Jarvis::Node::remove_property(StringID id)
    { _property_list.remove_property(id); }
