/**
 * @file   load_tsv_test.cc
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

#include <iostream>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

long long num_nodes = 0;
long long num_edges = 0;

void node_added(Node &);
void edge_added(Edge &);

int main(int argc, char *argv[])
{
    long line = 0;

    try {
        Graph db("load_tsv_graph", Graph::Create);

        load_tsv(db, stdin, node_added, edge_added);
    }
    catch (Exception e) {
        std::cerr << argv[0] << ": stdin: Exception " << e.name << " occurred on line " << line << "\n";
        return 1;
    }

    std::cout << num_nodes << "\t" << num_edges << "\n";

    return 0;
}

void node_added(Node &n)
{
    ++num_nodes;
}

void edge_added(Edge &e)
{
    ++num_edges;
}
