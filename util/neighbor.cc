#include <utility>
#include <set>
#include "jarvis.h"
#include "neighbor.h"

using namespace Jarvis;


/**
 * Iterator for the neighbors of a node
 */
class NeighborIteratorImpl : public NodeIteratorImplIntf
{
    const Node &_node;
    const Direction _dir;
    const bool _unique;
    EdgeIterator _ei;

    std::set<Node *>_seen;

    Node *_neighbor;

    Node *get_neighbor(const Jarvis::Node &n, const Jarvis::EdgeRef &e)
    {
        switch (_dir) {
        case Any: {
            Node &neighbor = e.get_source();
            if (&neighbor == &_node)
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
    NeighborIteratorImpl(const Node &node,
                         const Jarvis::Direction dir,
                         const StringID tag,
                         const bool unique)
        : _node(node), _dir(dir), _unique(unique), _ei(node.get_edges(dir, tag))
    {
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


NodeIterator get_neighbors
    (const Node &n, Direction dir, StringID tag, bool unique)
{
    return NodeIterator(new NeighborIteratorImpl(n, dir, tag, unique));
}
