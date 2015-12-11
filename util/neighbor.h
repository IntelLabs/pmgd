#pragma once

#include <vector>
#include "jarvis.h"

/**
 * These functions return an iterator over neighbors of the
 * specified node that are connected through edges satisfying
 * the specified constraints.
 *
 * get_neighbors returns the set of immediate neighbors of the
 *     starting node where each neighbor is connected by an edge
 *     satisfying the edge constraint.
 *
 * get_joint_neighbors returns the set of nodes where each node
 *     is the immediate neighbor of /all/ the nodes in the constraint
 *     list, and is connected by an edge satisfying the edge constraint
 *     for that node.
 *
 * The 'unique' parameter indicates whether the function should track
 * all nodes returned and avoid returning duplicates. If the graph is
 * known to be organized in such a way that no duplicates could occur,
 * or if the caller can tolerate duplicates, then setting 'unique' to
 * false will avoid the work of tracking returned nodes and checking
 * for duplicates.
 */

struct EdgeConstraint
{
    Jarvis::Direction dir;
    Jarvis::StringID tag;
};

struct JointNeighborConstraint
{
    EdgeConstraint edge_constraint;
    const Jarvis::Node &node;
};


extern Jarvis::NodeIterator get_neighbors
    (const Jarvis::Node &node,
     Jarvis::Direction dir = Jarvis::Any,
     Jarvis::StringID tag = 0,
     bool unique = true);

extern Jarvis::NodeIterator get_joint_neighbors
    (const std::vector<JointNeighborConstraint> &constraints,
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
