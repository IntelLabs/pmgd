/*
 * This test checks Jarvis signs of life.
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        // create graph outside transactions
        Graph db("solgraph", create ? Graph::Create : Graph::ReadOnly);

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
