/*
 * This test checks Jarvis signs of life.
 */

#include <stdio.h>
#include "jarvis.h"

using namespace Jarvis;

std::string property_text(const Property &p)
{
    switch (p.type()) {
        case t_novalue: return "no value";
        case t_boolean: return p.bool_value() ? "T" : "F";
        case t_integer: return std::to_string(p.int_value());
        case t_string: return p.string_value();
        case t_float: return std::to_string(p.float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
        default: throw Exception(property_type);
    }
}

template <typename T>
std::string tag_text(const T &n)
{
    std::string tag = n.get_tag().name();
    if (tag != "")
        tag = " #" + tag;
    return tag;
}

void dump(Graph &db, const Node &n)
{
    printf("Node %lu%s:\n", db.get_id(n), tag_text(n).c_str());
    n.get_properties()
        .process([&db](PropertyRef &p) {
        printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
    n.get_edges(OUTGOING)
        .process([&db](EdgeRef &e) {
        printf(" %s -> n%lu (e%lu)\n", tag_text(e).c_str(),
            db.get_id(e.get_destination()), db.get_id(e));
        });
    n.get_edges(INCOMING)
        .process([&db](EdgeRef &e) {
        printf(" %s <- n%lu (e%lu)\n", tag_text(e).c_str(),
            db.get_id(e.get_source()), db.get_id(e));
        });
}

void dump(Graph &db, const Edge &e)
{
    printf("Edge %lu%s: n%lu -> n%lu\n",
        db.get_id(e), tag_text(e).c_str(),
        db.get_id(e.get_source()), db.get_id(e.get_destination()));
    e.get_properties()
        .process([&db](PropertyRef &p) {
        printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
}

void dump(Graph &db, NodeIterator &i)
{
    while (i) {
        dump(db, *i);
        i.next();
    }
}

void dump(Graph &db, EdgeIterator &i)
{
    while (i) {
        dump(db, *i);
        i.next();
    }
}

void dump_nodes(Graph &db)
{
    for (NodeIterator i = db.get_nodes(); i; i.next()) {
        dump(db, *i);
    }
}

void dump_edges(Graph &db)
{
    for (EdgeIterator i = db.get_edges(); i; i.next()) {
        dump(db, *i);
    }
}

void print_exception(const Exception &e)
{
    printf("[Exception] %s at %s:%d\n", e.name, e.file, e.line);
    if (e.errno_val != 0)
        printf("%s: %s\n", e.msg.c_str(), strerror(e.errno_val));
    else if (!e.msg.empty())
        printf("%s\n", e.msg.c_str());
}

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        // create graph outside transactions
        Graph db("solgraph", create ? Graph::Create : Graph::ReadOnly);

        // add nodes and edges in a transaction
        Transaction tx(db, Transaction::ReadWrite);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property(0, argv[i]);
            if (prev != NULL)
                db.add_edge(*prev, n, 0);
            prev = &n;
        }

        dump_nodes(db);
        dump_edges(db);

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
