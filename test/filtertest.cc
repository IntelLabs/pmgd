/*
 * This test checks Jarvis iterator filters
 */

#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static void dump(const Graph &db, const Edge &n);
static void dump(const Graph &db, NodeIterator i);
static void dump(const Graph &db, EdgeIterator i);
static std::string property_text(const PropertyIterator &i);
static int print_exception(FILE *s, Exception& e);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("filtergraph", create ? Graph::Create : Graph::ReadOnly);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(i);
            n.set_property(1, argv[i]);
            n.set_property(2, i + 16);
            n.set_property(3, 22 - i);
            if (prev != NULL) {
                Edge &e = db.add_edge(*prev, n, 0);
                e.set_property(3, prev->get_property(1).string_value());
                e.set_property(4, n.get_property(1).string_value());
            }
            prev = &n;
        }

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        dump(db, db.get_nodes()
            .filter([](NodeIterator &i) {
                return i->get_property(1).string_value()[0] == 'a'
                           ? pass : dont_pass;
            }));

        // Look for value less than 20
        printf("Nodes with value less than 20\n");
        dump(db, db.get_nodes()
            .filter([](NodeIterator &i) {
                return i->get_property(2).int_value() < 20
                           ? pass : dont_pass;
            }));

        // Look for edge to or from a node starting with 'b'
        printf("Edges to or from a node starting with 'b'\n");
        dump(db, db.get_edges()
            .filter([](EdgeIterator &i) {
                return i->get_source().get_property(1).string_value()[0] == 'b'
                       || i->get_destination().get_property(1).string_value()[0] == 'b'
                           ? pass : dont_pass;
            }));

        printf("Nodes with any property less than 20\n");
        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            bool f = false;
            PropertyIterator j = i->get_properties()
                .filter([](PropertyIterator &k) {
                    return k->type() == t_integer
                               && k->int_value() < 20
                           ? pass : dont_pass;
                })
                .filter([&db,&i,&f](PropertyIterator &j) {
                    if (!f) {
                        printf("Node %lu:\n", db.get_id(*i));
                        f = true;
                    }
                    std::string id = j->id().name();
                    long long val = j->int_value();
                    printf("  %s: %lld\n", id.c_str(), val);
                    //printf("  %s: %lld\n", j->id().name().c_str(), j->int_value());
                    return pass;
                });
        }
    }
    catch (Exception e) {
        print_exception(stdout, e);
    }

    return 0;
}

static void dump(const Graph &db, NodeIterator i)
{
    while (i) {
        dump(db, *i);
        i.next();
    }
}

static void dump(const Graph &db, EdgeIterator i)
{
    while (i) {
        dump(db, *i);
        i.next();
    }
}

static void dump(const Graph &db, const Node &n)
{
    printf("Node %lu:\n", db.get_id(n));
    for (PropertyIterator i = n.get_properties(); i; i.next()) {
        printf("  %s: %s\n", i->id().name().c_str(), property_text(i).c_str());
    }
    for (EdgeIterator i = n.get_edges(OUTGOING, 0); i; i.next()) {
        printf("  -> n%lu (e%lu)\n", db.get_id(i->get_destination()), db.get_id(*i));
    }
    for (EdgeIterator i = n.get_edges(INCOMING, 0); i; i.next()) {
        printf("  <- n%lu (e%lu)\n", db.get_id(i->get_source()), db.get_id(*i));
    }
}

static void dump(const Graph &db, const Edge &e)
{
    printf("Edge %lu: n%lu -> n%lu\n", db.get_id(e),
           db.get_id(e.get_source()), db.get_id(e.get_destination()));
    for (PropertyIterator i = e.get_properties(); i; i.next()) {
        printf("  %s: %s\n", i->id().name().c_str(), property_text(i).c_str());
    }
}

static std::string property_text(const PropertyIterator &i)
{
    switch (i->type()) {
        case t_novalue: return "no value";
        case t_boolean: return i->bool_value() ? "T" : "F";
        case t_integer: return std::to_string(i->int_value());
        case t_string: return i->string_value();
        case t_float: return std::to_string(i->float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
    }
    throw Exception(property_type);
}

static int print_exception(FILE *s, Exception& e)
{
    return fprintf(s, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
