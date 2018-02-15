/**
 * @file   aborttest.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * This test exits during a transaction to test recovery.
 */

#include <stdio.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

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
                            fprintf(stderr, "aborttest: invalid e parameter\n");
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
                            fprintf(stderr, "aborttest: invalid p parameter\n");
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

                    case 'r': {
                        if (p[1] == 'p') {
                            if (strlen(p) < 5) {
                                fprintf(stderr, "aborttest: invalid r parameter\n");
                                exit(1);
                            }
                            int a = p[2] == 'n' ? 1 : 2; // node or edge: n or e
                            int b = p[3] - '1';        // which node or edge: 1 - 9
                            char c[] = { p[4], '\0' };   // property name: a - z
                            switch (a) {
                                case 1: nodes.at(b)->remove_property(c); break;
                                case 2: edges.at(b)->remove_property(c); break;
                            }
                            p += 4;
                        }
                        else {
                            if (strlen(p) < 3) {
                                fprintf(stderr, "aborttest: invalid r parameter\n");
                                exit(1);
                            }
                            int a = p[1] == 'n' ? 1 : 2; // node or edge: n or e
                            int b = p[2] - '1';        // which node or edge: 1 - 9
                            switch (a) {
                                case 1: db.remove(*nodes.at(b)); break;
                                case 2: db.remove(*edges.at(b)); break;
                            }
                            p += 2;
                        }
                        break;
                    }

                    case 'a':
                        printf("====\n");
                        dump_debug(db);
                        printf("====\n");
                        exit(0);
                }
            }

            if (i > 1)
                printf("----\n");
            dump_debug(db);
            tx.commit();
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
