#include "edge.h"

Jarvis::EdgeID Jarvis::Edge::get_id() const
{
    return (((uint64_t)this) & 0xffffffffff) >> 5;
}

void Jarvis::Edge::init(Node &src, Node &dest, StringID tag)
{
    _src = &src;
    _dest = &dest;
    _tag = tag;
    //PropertyList::init(_property_list);
}
