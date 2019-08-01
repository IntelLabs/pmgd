/**
 * @file   neighbor.cc
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

#include <utility>
#include <vector>
#include <set>
#include <unordered_set>
#include <queue>
#include <deque>
#include "pmgd.h"
#include "neighbor.h"

using namespace PMGD;

static Node *get_neighbor(const Node &n, const EdgeRef &e, const Direction &dir)
{
    switch (dir) {
    case Any: {
        Node &neighbor = e.get_source();
        if (neighbor == n)
            return &(e.get_destination());
        else
            return &neighbor;
    }
    case Outgoing:
        return &(e.get_destination());
    case Incoming:
        return &(e.get_source());
    default:
        assert(0);
        return 0;
    }
}


/**
 * Iterator for the neighbors of a node
 */
class NeighborIteratorImpl : public NodeIteratorImplIntf
{
    const Node &_node;
    const Direction _dir;
    const bool _unique;
    EdgeIterator _ei;

    std::vector<PMGD::PropertyFilter<PMGD::Edge>> _pfs;

    std::set<Node *>_seen;

    Node *_neighbor;

    bool _next()
    {
        while (_ei) {
            for (auto &pf : _pfs)
                if (pf(*_ei) == PMGD::DontPass)
                    goto next;

            _neighbor = get_neighbor(_node, *_ei, _dir);
            if (!_unique)
                return true;
            if (_seen.insert(_neighbor).second)
                return true;
        next:
            _ei.next();
        }
        return false;
    }

public:
    NeighborIteratorImpl(const Node &node,
                         const PMGD::Direction dir,
                         const StringID tag,
                         const std::vector<PMGD::PropertyPredicate> &pps,
                         const bool unique)
        : _node(node), _dir(dir), _unique(unique), _ei(node.get_edges(dir, tag))
    {
        for (const auto& p : pps)
            _pfs.push_back(PMGD::PropertyFilter<PMGD::Edge>(p));
        _next();
    }


    operator bool() const { return bool(_ei); }

    bool next()
    {
        _ei.next();
        return _next();
    }

    Node *ref()
    {
        // Check that *_ei still exists. If it does, then
        // _neighbor still exists and is still a neighbor.
        // If not, the edge iterator will throw VacantIterator.
        (void)*_ei;
        return _neighbor;
    }
};


class JointNeighborIteratorImpl : public NodeIteratorImplIntf
{
    typedef std::vector<JointNeighborConstraint> V;

    NodeIterator _ni;
    const V _constraints;

    bool _next()
    {
        while (_ni) {
            for (const JointNeighborConstraint &p : _constraints) {
                EdgeIterator ei = _ni->get_edges(p.edge_constraint.dir,
                                                 p.edge_constraint.tag)
                    .filter([p](const EdgeRef &e) {
                        return &e.get_source() == &p.node
                                   || &e.get_destination() == &p.node
                            ? Pass : DontPass;
                    });
                if (!ei)
                    goto continue2;
            }
            return true;
        continue2:
            _ni.next();
        }
        return false;
    }

    static Direction fix_direction(Direction dir)
    {
        switch (dir) {
            case Any: return Any;
            case Incoming: return Outgoing;
            case Outgoing: return Incoming;
        }
        assert(0);
        return Any;
    }

public:
    JointNeighborIteratorImpl
        (const std::vector<JointNeighborConstraint> &constraints, bool unique)
        : _ni(get_neighbors(constraints.at(0).node,
                            fix_direction(constraints.at(0).edge_constraint.dir),
                            constraints.at(0).edge_constraint.tag,
                            unique)),
          _constraints(constraints.begin() + 1, constraints.end())
    {
        _next();
    }

    operator bool() const { return bool(_ni); }

    bool next()
    {
        _ni.next();
        return _next();
    }

    Node *ref() { return &*_ni; }
};


class NeighborhoodIteratorImpl : public NodeIteratorImplIntf
{
public:
    virtual ~NeighborhoodIteratorImpl() { }
    virtual int distance() const = 0;
};

/**
 * Iterator for all neighbors of a node upto given hops (inclusive)
 * This iterator does a breadth first traversal.
 */
class BFSNeighborhoodIteratorImpl : public NeighborhoodIteratorImpl
{
    typedef std::vector<EdgeConstraint> V;
    const V _v;
    const Node *_node;
    EdgeIterator *_ei;
    int _curr_depth;
    const int _max_depth;
    Direction _dir;
    StringID _tag;

    std::unordered_set<const Node *>_seen;
    std::vector<std::queue<const Node *>> _explore;   // Neighbors that need to be traversed.
    Node *_neighbor;

    bool _next()
    {
        while (true) {
            while (*_ei) {
                _neighbor = get_neighbor(*_node, **_ei, _dir);
                if (_seen.insert(_neighbor).second) {
                    if (_curr_depth < _max_depth - 1)
                        _explore[_curr_depth + 1].push(_neighbor);
                    return true;
                }
                _ei->next();
            }
            // Means none of the neighbors were good. Look at explore queue.
            if (_explore[_curr_depth].empty()) {
                _curr_depth++;
                if (_curr_depth >= _max_depth || _explore[_curr_depth].empty())
                    break;
                _dir = _v[_curr_depth].dir;
                _tag = _v[_curr_depth].tag;
            }
            _node = _explore[_curr_depth].front();
            _explore[_curr_depth].pop();
            delete _ei;
            _ei = new EdgeIterator(_node->get_edges(_dir, _tag));
        }

        return false;
    }

public:
    BFSNeighborhoodIteratorImpl(const Node &n, const V &v)
        : _v(v),
          _node(&n),
          _curr_depth(0),
          _max_depth(v.size()),
          _dir(_v[_curr_depth].dir),
          _tag(_v[_curr_depth].tag),
          _explore(_max_depth)
    {
        _ei = new EdgeIterator(_node->get_edges(_dir, _tag));
        _seen.insert(_node);
        _next();
    }

    ~BFSNeighborhoodIteratorImpl()
    {
        delete _ei;
    }

    operator bool() const { return bool(*_ei); }

    bool next()
    {
        _ei->next();
        return _next();
    }

    Node *ref()
    {
        // Check that **_ei still exists. If it does, then
        // _neighbor still exists and is still a neighbor.
        // If not, the edge iterator will throw VacantIterator.
        (void)**_ei;
        return _neighbor;
    }

    int distance() const { return _curr_depth + 1; }
};

NeighborhoodIterator::NeighborhoodIterator(NeighborhoodIteratorImpl *i)
    : PMGD::NodeIterator(i) { }

NeighborhoodIterator::NeighborhoodIterator(NeighborhoodIterator &i)
    : PMGD::NodeIterator(i) { }

NeighborhoodIterator::NeighborhoodIterator(NeighborhoodIterator &&i)
    : PMGD::NodeIterator(i) { }

int NeighborhoodIterator::distance() const {
    NeighborhoodIteratorImpl *impl = static_cast<NeighborhoodIteratorImpl *>(_impl);
    return impl->distance();
}


/**
 * Iterator for all neighbors of a node a a given hop count.
 * This iterator does a breadth first traversal.
 */
class NhopNeighborIteratorImpl : public NodeIteratorImplIntf
{
    typedef std::vector<EdgeConstraint> V;
    const Node *_node;
    EdgeIterator *_ei;
    Direction _dir;
    StringID _tag;

    std::unordered_set<const Node *>_seen;
    std::deque<const Node *> _explore;   // Neighbors that need to be traversed.
    Node *_neighbor;

    void find_first(const Node &node, const V &v)
    {
        int max_depth = v.size();
        std::deque<const Node *> explore;

        explore.push_back(&node);

        for (int depth = 0; depth < max_depth - 1; depth++) {
            Direction dir = v[depth].dir;
            StringID tag = v[depth].tag;
            std::deque<const Node *> explore_next;
            for (const Node *n : explore) {
                EdgeIterator ei = n->get_edges(dir, tag);
                while (ei) {
                    const Node *neighbor = get_neighbor(*n, *ei, dir);
                    if (_seen.insert(neighbor).second)
                        explore_next.push_back(neighbor);
                    ei.next();
                }
            }
            explore = std::move(explore_next);
        }

        _explore = std::move(explore);
        _ei = new EdgeIterator(NULL);
        _next();
    }

    bool _next()
    {
        while (true) {
            while (*_ei) {
                _neighbor = get_neighbor(*_node, **_ei, _dir);
                if (_seen.insert(_neighbor).second)
                    return true;
                _ei->next();
            }
            // Means none of the neighbors were good. Look at explore queue.
            if (_explore.empty())
                    break;
            _node = _explore.front();
            _explore.pop_front();
            delete _ei;
            _ei = new EdgeIterator(_node->get_edges(_dir, _tag));
        }

        return false;
    }

public:
    NhopNeighborIteratorImpl(const Node &n, const V &v)
        : _dir(v.back().dir),
          _tag(v.back().tag)
    {
        _seen.insert(&n);
        find_first(n, v);
    }

    ~NhopNeighborIteratorImpl()
    {
        delete _ei;
    }

    operator bool() const { return bool(*_ei); }

    bool next()
    {
        _ei->next();
        return _next();
    }

    Node *ref()
    {
        // Check that **_ei still exists. If it does, then
        // _neighbor still exists and is still a neighbor.
        // If not, the edge iterator will throw VacantIterator.
        (void)**_ei;
        return _neighbor;
    }
};

NodeIterator get_neighbors
    (const Node &n, Direction dir, StringID tag, bool unique)
{
    std::vector<PMGD::PropertyPredicate> pps; // empty
    return NodeIterator(new NeighborIteratorImpl(n, dir, tag, pps, unique));
}

NodeIterator get_neighbors
    (const Node &n,
     Direction dir, StringID tag,
     const std::vector<PMGD::PropertyPredicate> &pps,
     bool unique)
{
    return NodeIterator(new NeighborIteratorImpl(n, dir, tag, pps, unique));
}

NodeIterator get_joint_neighbors
    (const std::vector<JointNeighborConstraint> &constraints, bool unique)
{
    return NodeIterator(new JointNeighborIteratorImpl(constraints, unique));
}


NeighborhoodIterator get_neighborhood
    (const Node &node,
     const std::vector<EdgeConstraint> &constraints,
     bool bfs)
{
    if (bfs)
        return NeighborhoodIterator(new BFSNeighborhoodIteratorImpl(node, constraints));
    return NeighborhoodIterator(NULL);
//    else
//        return NodeIterator(new NhopDFSNeighborhoodIteratorImpl(node, constraints));
}


NodeIterator get_nhop_neighbors
    (const Node &node,
     const std::vector<EdgeConstraint> &constraints)
{
    return NodeIterator(new NhopNeighborIteratorImpl(node, constraints));
}
