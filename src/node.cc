#include <stddef.h>
#include "node.h"

void Jarvis::Node::init(StringID tag)
{
    _out_edges = 0;
    _in_edges = 0;
    _tag = tag;
    //PropertyList::init(_property_list);
}
