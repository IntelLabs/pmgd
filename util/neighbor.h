#pragma once

#include "jarvis.h"

/**
 * Iterator for the neighbors of a node
 */

struct EdgeConstraint
{
    Jarvis::Direction dir;
    Jarvis::StringID tag;
};


extern Jarvis::NodeIterator get_neighbors
    (const Jarvis::Node &node,
     Jarvis::Direction dir = Jarvis::Any,
     Jarvis::StringID tag = 0,
     bool unique = true);


inline Jarvis::NodeIterator get_neighbors
    (const Jarvis::Node &node,
     EdgeConstraint constraint,
     bool unique = true)
{
    return get_neighbors(node, constraint.dir, constraint.tag, unique);
}

inline Jarvis::NodeIterator get_neighbors(const Jarvis::Node &node, bool unique)
{
    return get_neighbors(node, Jarvis::Any, 0, unique);
}

inline Jarvis::NodeIterator get_neighbors
    (const Jarvis::Node &node, Jarvis::Direction dir, bool unique)
{
    return get_neighbors(node, dir, 0, unique);
}

inline Jarvis::NodeIterator get_neighbors
    (const Jarvis::Node &node, Jarvis::StringID tag, bool unique = true)
{
    return get_neighbors(node, Jarvis::Any, tag, unique);
}
