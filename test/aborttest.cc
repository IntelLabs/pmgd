/*
 * This test exits during a transaction to test recovery.
 */

#include <stdio.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    static const char graphname[] = "abortgraph";
    Graph db(graphname, Graph::Create);

    std::vector<Node *> nodes;
    std::vector<Edge *> edges;

    try {
        for (int i = 1; i < argc; i++) {
            Transaction tx(db, Transaction::ReadWrite);

            for (const char *p = argv[i]; *p; p++) {
                switch (*p) {
                    case 'n': nodes.push_back(&db.add_node(0)); break;

                    case 'e': {
                        if (strlen(p) < 3) {
                            fprintf(stderr, "aborttest: invalid e parameter");
                            exit(1);
                        }
                        int a = p[1] - '1';        // node 1: 1 - 9
                        int b = p[2] - '1';        // node 2: 1 - 9
                        Edge &e = db.add_edge(*nodes.at(a), *nodes.at(b), 0);
                        edges.push_back(&e);
                        p += 2;
                        break;
                    }

                    case 'p': {
                        if (strlen(p) < 5) {
                            fprintf(stderr, "aborttest: invalid p parameter");
                            exit(1);
                        }
                        int a = p[1] == 'n' ? 1 : 2; // node or edge: n or e
                        int b = p[2] - '1';        // which node or edge: 1 - 9
                        char c[] = { p[3], '\0' };   // property name: a - z
                        int d = p[4] - '0';          // property value: 0 - 9
                        switch (a) {
                            case 1: nodes.at(b)->set_property(c, d); break;
                            case 2: edges.at(b)->set_property(c, d); break;
                        }
                        p += 4;
                        break;
                    }

                    case 'a':
                        printf("====\n");
                        dump_nodes(db);
                        dump_edges(db);
                        printf("====\n");
                        exit(0);
                }
            }

            if (i > 1)
                printf("----\n");
            dump_nodes(db);
            dump_edges(db);
            tx.commit();
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
