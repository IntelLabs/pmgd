/*
 * This test checks Jarvis property lists
 */

#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    int node_count = argc > 1 ? atoi(argv[1]) : 4;
    int edge_count = node_count - 1;
    unsigned seed = argc > 2 ? strtoull(argv[2], 0, 10) : unsigned(time(0));

    printf("node_count = %d, seed = %u\n", node_count, seed);

    srand(seed);

    try {
        Graph db("indexgraph", Graph::Create);

        Node **nodes = new Node *[node_count + 1];
        Transaction tx(db);

        for (int i = 1; i <= 2; i++) {
            db.create_index(Graph::NODE, "tag1", "id1", t_integer);
            Node &n = db.add_node("tag1");
            n.set_property("id1", i + 1611);
            nodes[i] = &n;
        }
        for (int i = 3; i <= node_count; i++) {
            Node &n = db.add_node("tag1");
            // More nodes with the same property value and tag
            n.set_property("id1", i - 2 + 1611);
            nodes[i] = &n;
        }

        for (int i = 1; i <= edge_count; i++) {
            Edge &e = db.add_edge(*nodes[i], *nodes[i+1], 0);
            e.set_property("id1", i + 2611);
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
