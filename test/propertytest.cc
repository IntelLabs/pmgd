/*
 * This test checks Jarvis property lists
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("propertygraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property("id1", argv[i]);
            n.set_property("id2", i + 16ll);
            n.set_property("id3", -(1ull<<(i*4)));
            n.set_property("id4", "this is a very long string");
            if (prev != NULL) {
                Edge &e = db.add_edge(*prev, n, 0);
                e.set_property("id3", prev->get_property("id1").string_value());
                e.set_property("id4", n.get_property("id1").string_value());
            }
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
