/*
 * Test for Jarvis transactions
 */

#include <stdio.h>
#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static void dump(const Graph &db, const Edge &n);
static std::string property_text(const PropertyIterator &i);
static int print_exception(FILE *s, Exception& e);
static void dump(Graph &db);
static void dump_no_tx(Graph &db);
static void modify(Graph &db, int argc, char **argv, bool commit);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        // create graph outside transactions
        Graph db("txgraph", create ? Graph::Create : Graph::ReadOnly);
        modify(db, argc, argv, true);
        modify(db, argc, argv, false);
    }
    catch (Exception e) {
        print_exception(stdout, e);
    }
    return 0;
}

static void modify(Graph &db, int argc, char **argv, bool commit)
{
    if (commit)
        printf("\nCOMMIT TEST\n");
    else
        printf("\nABORT TEST\n");

    try {
        // add nodes and edges in a transaction
        Transaction tx(db);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property(0, argv[i]);
            if (prev != NULL)
                db.add_edge(*prev, n, 0);
            prev = &n;
        }

        if (commit) {
            tx.commit();
        }
        else {
            printf("BEFORE ABORT:\n");
            dump_no_tx(db);
        }
    }
    catch (Exception e) {
        print_exception(stdout, e);
    }
    if (commit)
        printf("AFTER COMMIT:\n");
    else
        printf("AFTER ABORT:\n");
    dump(db);
}

static void dump_no_tx(Graph &db)
{
    try {
        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            dump(db, *i);
        }

        for (EdgeIterator i = db.get_edges(); i; i.next()) {
            dump(db, *i);
        }
    }
    catch (Exception e) {
        print_exception(stdout, e);
    }
}

static void dump(Graph &db)
{
    try {
        Transaction tx(db);
        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            dump(db, *i);
        }

        for (EdgeIterator i = db.get_edges(); i; i.next()) {
            dump(db, *i);
        }
        tx.commit();
    }
    catch (Exception e) {
        print_exception(stdout, e);
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
    throw Jarvis::Exception(property_type);
}

static int print_exception(FILE *s, Exception& e)
{
    return fprintf(s, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
