/*
 * This test checks Jarvis signs of life.
 *
 * Compile with:
 *     make -C ../src
 *     g++-4.8 -std=c++11 -I ../include nodeedgetest.cc ../lib/jarvis.lib
 *
 * To include stubs for as-yet unimplemented graph functions, use:
 *     g++-4.8 -std=c++11 -I ../include -DSTUBS nodeedgetest.cc ../lib/jarvis.lib
 */

#include <stdio.h>
#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static void dump(const Graph &db, const Edge &e);
static int print_exception(FILE *s, Exception& e);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("nodeedgegraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            if (prev != NULL) {
                db.add_edge(*prev, n, 0);
                db.add_edge(*prev, n, 1);
            }
            prev = &n;
        }

        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            dump(db, *i);
        }

        // Just for verification if all covered
        for (EdgeIterator i = db.get_edges(); i; i.next()) {
            dump(db, *i);
        }

        tx.commit();
    }
    catch (Exception e) {
        print_exception(stdout, e);
        return 1;
    }

    return 0;
}

static void dump(const Graph &db, const Node &n)
{
    NodeID my_id = db.get_id(n);
    printf("Node %lu:\n", my_id);
    printf("All edges: \n");
    for (EdgeIterator i = n.get_edges(); i; i.next()) {
        NodeID other_id = db.get_id(i->get_destination());
        if (other_id == my_id ) { // I am destination
            printf("  <- n%lu (%s,e%lu)\n", db.get_id(i->get_source()),
                    i->get_tag().name().c_str(), db.get_id(*i));
        }
        else {
            printf("  -> n%lu (%s,e%lu)\n", db.get_id(i->get_destination()),
                    i->get_tag().name().c_str(), db.get_id(*i));
        }
    }
    printf("All outgoing edges: \n");
    for (EdgeIterator i = n.get_edges(OUTGOING); i; i.next()) {
        printf("  -> n%lu (%s,e%lu)\n", db.get_id(i->get_destination()),
                i->get_tag().name().c_str(), db.get_id(*i));
    }
    printf("All outgoing edges with tag 0: \n");
    for (EdgeIterator i = n.get_edges(OUTGOING, 0); i; i.next()) {
        printf("  -> n%lu (%s,e%lu)\n", db.get_id(i->get_destination()),
                i->get_tag().name().c_str(), db.get_id(*i));
    }
    printf("All incoming edges with tag 1: \n");
    for (EdgeIterator i = n.get_edges(INCOMING, 1); i; i.next()) {
        printf("  <- n%lu (%s,e%lu)\n", db.get_id(i->get_source()),
                i->get_tag().name().c_str(), db.get_id(*i));
    }
    printf("\n");
}

static void dump(const Graph &db, const Edge &e)
{
    printf("Edge %lu, tag %s: n%lu -> n%lu\n", db.get_id(e),
            e.get_tag().name().c_str(),
            db.get_id(e.get_source()), db.get_id(e.get_destination()));
}

static int print_exception(FILE *s, Exception& e)
{
    return fprintf(s, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
