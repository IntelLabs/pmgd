/*
 * This test checks Jarvis property lists
 */

#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    int node_count = argc > 1 ? atoi(argv[1]) : 1000;
    int edge_count = node_count - 1;
    unsigned seed = argc > 2 ? strtoull(argv[2], 0, 10) : unsigned(time(0));

    printf("node_count = %d, seed = %u\n", node_count, seed);

    if (node_count <= 0)
        return 1;

    srand(seed);

    try {
        Graph db("propertychunkgraph", Graph::Create);

        Node **nodes = new Node *[node_count + 1];

        for (int i = 1; i <= node_count; i++) {
            Transaction tx(db);
            Node &n = db.add_node(0);

            printf("Node %d:\n", i);

            int id = 0;
            int total_size = 0;
            while (total_size < 44 + 64) {
                id++;
                int size = rand() % 16;
                total_size += size + 3;
                std::string s("123456789abcdef", size);
                n.set_property(std::to_string(id).c_str(), s);
                printf("  %d: %s\n", id, s.c_str());
            }

            if (i < node_count)
                printf("  -> n%d (e%d)\n", i+1, i);
            if (i > 1)
                printf("  <- n%d (e%d)\n", i-1, i-1);
            nodes[i] = &n;

            tx.commit();
        }

        for (int i = 1; i <= edge_count; i++) {
            Transaction tx(db);
            Edge &e = db.add_edge(*nodes[i], *nodes[i+1], 0);

            printf("Edge %d: n%d -> n%d\n", i, i, i+1);

            int id = 0;
            int total_size = 0;
            while (total_size < 44 + 64) {
                id++;
                int size = rand() % 16;
                total_size += size + 3;
                std::string s("123456789abcdef", size);
                e.set_property(std::to_string(id).c_str(), s);
                printf("  %d: %s\n", id, s.c_str());
            }

            tx.commit();
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
