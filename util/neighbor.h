/**
 * @file   neighbor.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

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

class NeighborhoodIteratorImpl;

class NeighborhoodIterator : public Jarvis::NodeIterator
{
public:
    explicit NeighborhoodIterator(NeighborhoodIteratorImpl *i);
    NeighborhoodIterator(NeighborhoodIterator &i);
    NeighborhoodIterator(NeighborhoodIterator &&i);

    int distance() const;
};


extern Jarvis::NodeIterator get_neighbors
    (const Jarvis::Node &node,
     Jarvis::Direction dir = Jarvis::Any,
     Jarvis::StringID tag = 0,
     bool unique = true);

extern Jarvis::NodeIterator get_joint_neighbors
    (const std::vector<JointNeighborConstraint> &constraints,
     bool unique = true);

extern NeighborhoodIterator get_neighborhood
    (const Jarvis::Node &node,
     const std::vector<EdgeConstraint> &constraints,
     bool bfs);

extern Jarvis::NodeIterator get_nhop_neighbors
    (const Jarvis::Node &node,
     const std::vector<EdgeConstraint> &constraints);


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


inline NeighborhoodIterator get_neighborhood
    (const Jarvis::Node &node, int max_hops,
     bool bfs)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{Jarvis::Any, 0};
    return get_neighborhood(node, V(max_hops, constraint), bfs);
}

inline NeighborhoodIterator get_neighborhood
    (const Jarvis::Node &node, int max_hops,
     Jarvis::Direction dir,
     bool bfs)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{dir, 0};
    return get_neighborhood(node, V(max_hops, constraint), bfs);
}

inline NeighborhoodIterator get_neighborhood
    (const Jarvis::Node &node, int max_hops,
     Jarvis::StringID tag,
     bool bfs)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{Jarvis::Any, tag};
    return get_neighborhood(node, V(max_hops, constraint), bfs);
}

inline NeighborhoodIterator get_neighborhood
    (const Jarvis::Node &node, int max_hops,
     Jarvis::Direction dir, Jarvis::StringID tag,
     bool bfs)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{dir, tag};
    return get_neighborhood(node, V(max_hops, constraint), bfs);
}

inline NeighborhoodIterator get_neighborhood
    (const Jarvis::Node &node, int max_hops,
     EdgeConstraint constraint,
     bool bfs)
{
    typedef std::vector<EdgeConstraint> V;
    return get_neighborhood(node, V(max_hops, constraint), bfs);
}


inline Jarvis::NodeIterator get_nhop_neighbors
    (const Jarvis::Node &node, int hops)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{Jarvis::Any, 0};
    return get_nhop_neighbors(node, V(hops, constraint));
}

inline Jarvis::NodeIterator get_nhop_neighbors
    (const Jarvis::Node &node, int hops,
     Jarvis::Direction dir)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{dir, 0};
    return get_nhop_neighbors(node, V(hops, constraint));
}

inline Jarvis::NodeIterator get_nhop_neighbors
    (const Jarvis::Node &node, int hops,
     Jarvis::StringID tag)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{Jarvis::Any, tag};
    return get_nhop_neighbors(node, V(hops, constraint));
}

inline Jarvis::NodeIterator get_nhop_neighbors
    (const Jarvis::Node &node, int hops,
     Jarvis::Direction dir, Jarvis::StringID tag)
{
    typedef std::vector<EdgeConstraint> V;
    EdgeConstraint constraint{dir, tag};
    return get_nhop_neighbors(node, V(hops, constraint));
}

inline Jarvis::NodeIterator get_nhop_neighbors
    (const Jarvis::Node &node, int hops,
     EdgeConstraint constraint)
{
    typedef std::vector<EdgeConstraint> V;
    return get_nhop_neighbors(node, V(hops, constraint));
}
