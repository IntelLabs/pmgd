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

void Node::init(StringID tag, Allocator &index_allocator)
{
    _out_edges = EdgeIndex::create(index_allocator);
    _in_edges = EdgeIndex::create(index_allocator);
    _tag = tag;
    //PropertyList::init(_property_list);
}

void Node::add_edge(Edge *edge, Direction dir, StringID tag, Allocator &index_allocator)
{
    if (dir == OUTGOING)
        _out_edges->add(tag, edge, &edge->get_destination(), index_allocator);
    else
        _in_edges->add(tag, edge, &edge->get_source(), index_allocator);
}

namespace Jarvis {
    class NodeEdgeIteratorImpl : public EdgeIteratorImpl {
        EdgeRef _ref;

        // This has to be non-const because it is representing ptr
        // to the node inside an edge which is non-const
        Node *_n1;
        Direction _dir;
        StringID _tag;
        const EdgeIndex::EdgePosition *_pos;

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
        }
        operator bool() const { return _pos != NULL; }
        const EdgeRef &operator*() const { return _ref; }
        const EdgeRef *operator->() const { return &_ref; }
        EdgeRef &operator*() { return _ref; }
        EdgeRef *operator->() { return &_ref; }
        bool next() { _pos = _pos->next; return _pos != NULL; }
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
