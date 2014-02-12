/*
 * This test checks that the Jarvis API compiles and
 * supports solutions to the following eight queries.
 * 1. Distance between A & D
 * 2. Return all nodes up to depth N
 * 3. Return all nodes at depth N
 * 4. Is A connected to D?
 * 5. Shortest path between A & D
 * 6. Shortest weighted path between A & D
 * 7. All paths between A & D
 * 8. All paths length N between A & D
 *
 * This test contains stubs for any Jarvis functions that are
 * not inline, so it should not be linked against jarvis.lib.
 *
 * Compile with g++-4.8 -std=c++11 -I ../include apitest.cc
 */

#include "jarvis.h"

using namespace Jarvis;


int main()
{
    Graph db("name");
    Node &a = db.add_node(0);
    Node &b = db.add_node(0);
    db.add_edge(a, b, 0);
    NodeIterator ni = db.get_nodes();
    EdgeIterator ei = db.get_edges();

    return 0;
}

// Query 1.
// Return the length of the shortest path between two nodes.
int q1(Graph &db, Node &a, Node &b)
{
    class min_path {
        int min_length;
    public:
        min_path() : min_length(0) { }
        Disposition operator()(PathIterator &i) {
            if (i->length() < min_length)
                min_length = i->length();
            return Jarvis::pass;
        }
        int get_length() { return min_length; }
    };
    min_path m;
    db.get_paths(a, b).filter(m);
    return m.get_length();
}


// That was a dumb implementation (though it should work).
// I think this one is much better.
// Query 1.
// Return the length of the shortest path between two nodes.
// Since the breadth-first search will always return the shortest path first,
// all the filter has to do is stop on the first path.
int q1a(Graph &db, Node &a, Node &b)
{
    return db.get_paths(a, b, false)
               .filter([](PathIterator &) { return Jarvis::pass_stop; })
               ->length();
}


// Query 5.
// Return the shortest path between two nodes.
// Since the breadth-first search will always return the shortest path first,
// all the filter has to do is stop on the first path.
int q5(Graph &db, Node &a, Node &b)
{
    return db.get_paths(a, b, false)
               .filter([](PathIterator &) { return Jarvis::pass_stop; });
}


// Query 6.
// Return the length of the shortest weighted path between two nodes.
double q6(Graph &db, Node &a, Node &b)
{
    class min_weighted_path {
        double min_length;
    public:
        min_weighted_path() : min_length(0) { }
        Disposition operator()(PathIterator &i) {
            double length = 0;
            EdgeIterator e = i->get_edges();
            while (e) {
                length += e->get_property("weight").value().float_value();
                e.next();
            }
            if (length < min_length)
                min_length = length;
            return Jarvis::stop;
        }
        double get_length() { return min_length; }
    };
    min_weighted_path m;
    PathIterator i = db.get_paths(a, b).filter(m);
    return m.get_length();
}


// Query 6a.
// Return the shortest weighted path between two nodes.
Jarvis::Path q6a(Graph &db, Node &a, Node &b)
{
    Jarvis::Path path;
    double min_length;
    for (PathIterator i = db.get_paths(a, b); i; i.next()) {
        double length;
        EdgeIterator e = i->get_edges();
        while (e) {
            length += e->get_property("weight").value().float_value();
            e.next();
        }
        if (length < min_length) {
            min_length = length;
            path = *i;
        }
        i.next();
    }
    return path;
}


// Query 4.
// Return true if A is connected to B.
bool q4(Graph &db, Node &a, Node &b)
{
    return bool(db.get_paths(a, b)
                .filter([](PathIterator &) { return Jarvis::pass_stop; }));
}


// Query 8.
// Get all paths of length N between nodes A and B.
void q8(Graph &db, Node &a, Node &b, int N, void (*process)(Path &))
{
    PathIterator i = db.get_paths(a, b)
        .filter([N](PathIterator &i)
            { return i->length() < N ? Jarvis::dont_pass : Jarvis::pass_stop; });

    while (i) {
        //process(*i);
        i.next();
    }
}


// Query 3.
// Get all nodes at distance N from node A.
NodeIterator q3(Graph &db, Node &a, int N)
{
    return db.get_paths(a)
        .filter([N](PathIterator &i)
            { return i->length() < N ? Jarvis::dont_pass : Jarvis::pass_stop; })
        .end_nodes();
}


// Query 2.
// Get all nodes at distance <= N from node A.
NodeIterator q2(Graph &db, Node &a, int N)
{
    return db.get_paths(a)
        .filter([N](PathIterator &i)
            { return i->length() < N ? Jarvis::pass : Jarvis::pass_stop; })
        .end_nodes();
}


// I'm cheating on those last two--I used the function end_nodes, which I
// don't actually know if I can implement. They can be written without using
// a node iterator, though:

// Query 3.
// Get all nodes at distance N from node A.
void q3a(Graph &db, Node &a, int N, void (*process)(Node &))
{
    PathIterator i = db.get_paths(a)
        .filter([N](PathIterator &i)
           { return i->length() < N ? Jarvis::dont_pass : Jarvis::pass_stop; });

    while (i) {
        process(i->end_node());
        i.next();
    }
}


// Query 2.
// Get all nodes at distance <= N from node A.
void q2a(Graph &db, Node &a, int N, void (*process)(Node &))
{
    PathIterator i = db.get_paths(a)
        .filter([N](PathIterator &i)
            { return i->length() < N ? Jarvis::pass : Jarvis::pass_stop; });

    while (i) {
        process(i->end_node());
        i.next();
    }
}


Jarvis::Graph::Graph(const char *, int)
{
}

Jarvis::Graph::~Graph()
{
}

Jarvis::Node &Jarvis::Graph::add_node(StringID)
{
    throw e_not_implemented;
}

Jarvis::Edge &Jarvis::Graph::add_edge(Node &, Node &, StringID)
{
    throw e_not_implemented;
}

Jarvis::NodeIterator Jarvis::Graph::get_nodes()
{
    throw e_not_implemented;
}

Jarvis::PathIterator Jarvis::Graph::get_paths(Node &, Node&, bool)
{
    return PathIterator(NULL);
}

Jarvis::EdgeIterator Jarvis::Graph::get_edges()
{
    throw e_not_implemented;
}

Jarvis::StringID::StringID(const char *)
{
}

Jarvis::Property Jarvis::Edge::get_property(StringID) const
{
    throw e_not_implemented;
}

Jarvis::PathRef::operator Jarvis::Path() const
{
    return Path();
}

Jarvis::PathIterator Jarvis::Graph::get_paths(Jarvis::Node &, bool)
{
    return PathIterator(NULL);
}

Jarvis::NodeIterator Jarvis::PathIteratorImpl::end_nodes() const
{
    throw e_not_implemented;
}

Jarvis::PropertyValue::PropertyValue(const PropertyValue &a)
{
}

Jarvis::PropertyValue::~PropertyValue()
{
}

bool Jarvis::PropertyValue::operator<(const PropertyValue &a)
{

    throw e_not_implemented;
}
