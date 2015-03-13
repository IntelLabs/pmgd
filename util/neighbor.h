#pragma once

#include <set>
#include "jarvis.h"

using namespace std;
using namespace Jarvis;

#ifdef __GNUC__
#define UNREACHABLE __builtin_unreachable()
#else
#define UNREACHABLE
#endif

/**
 * Iterator for the neighbors of a node
 */
class NeighborIterator : public NodeIteratorImplIntf
{
    const Node &_node;
    const Direction _dir;
    const bool _unique;
    EdgeIterator _ei;

    std::set<Node *>_seen;

    Node *_neighbor;

    Node *get_neighbor(const Node &n, const EdgeRef &e)
    {
        switch (_dir) {
        case ANY: {
            Node &neighbor = e.get_source();
            if (&neighbor == &_node)
                return &(e.get_destination());
            else
                return &neighbor;
        }
        case OUTGOING:
            return &(e.get_destination());
        case INCOMING:
            return &(e.get_source());
        default:
            UNREACHABLE;
        }
    }

    bool _next()
    {
        while (_ei) {
            _neighbor = get_neighbor(_node, *_ei);
            if (!_unique)
                return true;
            if (_seen.find(_neighbor) == _seen.end()) {
                _seen.insert(_neighbor);
                return true;
            }
            _ei.next();
        }
        return false;
    }

public:
    NeighborIterator(const Node &n, const bool unique)
        : _node(n), _dir(ANY), _unique(unique), _ei(n.get_edges())
    {
        _next();
    }

    NeighborIterator(const Node &n, const Direction dir, const bool unique)
        : _node(n), _dir(dir), _unique(unique), _ei(n.get_edges(dir))
    {
        _next();
    }

    NeighborIterator(const Node &n, const StringID tag, const bool unique)
        : _node(n), _dir(ANY), _unique(unique), _ei(n.get_edges(tag))
    {
        _next();
    }

    NeighborIterator(const Node &n, const Direction dir, const StringID tag,
                     const bool unique)
        : _node(n), _dir(dir), _unique(unique), _ei(n.get_edges(dir, tag))
    {
        _next();
    }

    operator bool() const { return bool(_ei); }

    bool next()
    {
        _ei.next();
        return _next();
    }

    const Node &operator*() const { return *_neighbor; }
    const Node *operator->() const { return _neighbor; }
    Node &operator*() { return *_neighbor; }
    Node *operator->() { return _neighbor; }
};

/**
 * Return the neighbors of node.
 *
 * @param n         starting node
 * @param dir       connecting edge direction
 * @param tag       connecting edge tag
 * @param unique    only unique nodes
 *
 * @return a node iterator representing the neighbors of the starting node
 *
 * The returned iterator cycles only neighbors of the starting node
 * connected through edges satisfying the following conditions (a) the
 * edge must have a particular direction, (b) the edge must have the
 * given tag (unless the given value is 0 in which case the iterator
 * will ignore tag values), and (c) only return a given node once if
 * so requested.
 */
inline NodeIterator get_neighbors(Node &n, bool unique = true)
{
    return NodeIterator(new NeighborIterator(n, unique));
}

inline NodeIterator get_neighbors(Node &n, Direction dir, bool unique = true)
{
    return NodeIterator(new NeighborIterator(n, dir, unique));
}

inline NodeIterator get_neighbors(Node &n, StringID tag, bool unique = true)
{
    return NodeIterator(new NeighborIterator(n, tag, unique));
}

inline NodeIterator get_neighbors(Node &n, Direction dir, StringID tag,
                                  bool unique = true)
{
    return NodeIterator(new NeighborIterator(n, dir, tag, unique));
}
