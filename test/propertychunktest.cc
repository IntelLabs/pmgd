/**
 * @file   propertychunktest.cc
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
 * This test checks PMGD property lists
 */

#include "pmgd.h"
#include "util.h"

using namespace PMGD;

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
            Transaction tx(db, Transaction::ReadWrite);
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
            Transaction tx(db, Transaction::ReadWrite);
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
