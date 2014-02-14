#include <stddef.h>
#include "edge.h"

void Jarvis::Edge::init(Node &src, Node &dest, StringID tag)
{
    _src = &src;
    _dest = &dest;
    _tag = tag;
    //PropertyList::init(_property_list);
}
