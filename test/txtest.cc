/*
 * Test for Jarvis transactions
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

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
        print_exception(e);
        exit(1);
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
        print_exception(e);
        exit(1);
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
        dump_nodes(db);
        dump_edges(db);
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
}

static void dump(Graph &db)
{
    try {
        Transaction tx(db);
        dump_nodes(db);
        dump_edges(db);
        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        exit(1);
    }
}
