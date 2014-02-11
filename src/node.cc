#include "node.h"

Jarvis::NodeID Jarvis::Node::get_id() const
{
    return (((uint64_t)this) & 0xffffffffff) >> 6;
}

void Jarvis::Node::init(StringID tag)
{
    _out_edges = 0;
    _in_edges = 0;
    _tag = tag;
    //PropertyList::init(_property_list);
}
