/**
 * @file   load_tsv.cc
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

#include <stdio.h>
#include <string.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

static const char ID_STR[] = "pmgd.loader.id";

static Node &get_node(Graph &db, long long id,
                      std::function<void(Node &)> node_func);

void load_tsv(Graph &db, const char *filename,
              std::function<void(Node &)> node_func,
              std::function<void(Edge &)> edge_func)
{
    FILE *f = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "r");
    if (f == NULL)
        throw PMGDException(LoaderOpenFailed, errno, filename);

    load_tsv(db, f, node_func, edge_func);
}

void load_tsv(Graph &db, FILE *f,
              std::function<void(Node &)> node_func,
              std::function<void(Edge &)> edge_func)
{
    char buf[500];

    Transaction tx(db, Transaction::ReadWrite);
    db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::Integer);
    tx.commit();

    while (fgets(buf, sizeof buf, f) != NULL) {
        long long a, b;
        if (sscanf(buf, "%lld %lld", &a, &b) != 2)
            throw PMGDException(LoaderParseError);
        Transaction tx(db, Transaction::ReadWrite);
        Node &src = get_node(db, a, node_func);
        Node &dst = get_node(db, b, node_func);
        Edge &edge = db.add_edge(src, dst, 0);
        if (edge_func)
            edge_func(edge);
        tx.commit();
    }
}


static Node &get_node(Graph &db, long long id,
                      std::function<void(Node &)> node_func)
{
    NodeIterator nodes = db.get_nodes(0,
                             PropertyPredicate(ID_STR, PropertyPredicate::Eq, id));
    if (nodes) return *nodes;

    // Node not found; add it
    Node &node = db.add_node(0);
    node.set_property(ID_STR, id);
    if (node_func)
        node_func(node);
    return node;
}

void do_nothing_node(PMGD::Node &) { }
void do_nothing_edge(PMGD::Edge &) { }
