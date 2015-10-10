#include <stddef.h>
#include "edge.h"
#include "graph.h"
#include "GraphImpl.h"
#include "TransactionImpl.h"

using namespace Jarvis;

void Edge::init(Node &src, Node &dest, StringID tag, unsigned obj_size)
{
    _src = &src;
    _dest = &dest;
    _tag = tag;
    _property_list.init(obj_size - offsetof(Edge, _property_list));
}

EdgeID Edge::get_id() const
{
    return TransactionImpl::get_tx()->get_db()->edge_table().get_id(this);
}

EdgeID EdgeRef::get_id() const
{
    return edge()->get_id();
}

bool Edge::check_property(StringID id, Property &result) const
    { return _property_list.check_property(id, result); }

Property Edge::get_property(StringID id) const
    { return _property_list.get_property(id); }

PropertyIterator Edge::get_properties() const
    { return _property_list.get_properties(); }

void Edge::set_property(StringID id, const Property &new_value)
    { _property_list.set_property(id, new_value, Graph::EdgeIndex, _tag, this); }

void Edge::remove_property(StringID id)
    { _property_list.remove_property(id, Graph::EdgeIndex, _tag, this); }

void Edge::remove_all_properties()
    { _property_list.remove_all_properties(Graph::EdgeIndex, _tag, this); }
