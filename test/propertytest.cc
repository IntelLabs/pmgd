/*
 * This test checks Jarvis property lists
 */

#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static void dump(const Graph &db, const Edge &n);
static std::string property_text(const PropertyIterator &i);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("propertygraph", create ? Graph::Create : Graph::ReadOnly);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property(Property(1, argv[i]));
            n.set_property(Property(2, i + 16ll));
            if (prev != NULL) {
                Edge &e = db.add_edge(*prev, n, 0);
                e.set_property(Property(3, prev->get_property(1).value().string_value()));
                e.set_property(Property(4, n.get_property(1).value().string_value()));
            }
            prev = &n;
        }

        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            dump(db, *i);
        }

        for (EdgeIterator i = db.get_edges(); i; i.next()) {
            dump(db, *i);
        }
    }
    catch (Exception e) {
        printf("EXCEPTION %d\n", e);
    }

    return 0;
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
    switch (i->value().type()) {
        case t_novalue: return "no value";
        case t_boolean: return i->value().bool_value() ? "T" : "F";
        case t_integer: return std::to_string(i->value().int_value());
        case t_string: return i->value().string_value();
        case t_float: return std::to_string(i->value().float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
    }
    throw Jarvis::e_property_type;
}
