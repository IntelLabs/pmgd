/**
 * @file   nodeedgetest.cc
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

/**
 *  Unit test for the node edge index
 */
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

// Specialized function defined here instead of util::dump. This has to
// test the various iterator options for Node::get_edges()
static void test_get_edges(const Graph &db, const Node &n);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("nodeedgegraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db, Transaction::ReadWrite);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            if (prev != NULL) {
                db.add_edge(*prev, n, "tag0");
                db.add_edge(*prev, n, "tag1");
            }
            prev = &n;
        }

        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            test_get_edges(db, *i);
        }

        // Just for verification if all covered
        for (EdgeIterator i = db.get_edges(); i; i.next()) {
            dump(*i);
        }

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}

static void test_get_edges(const Graph &db, const Node &n)
{
    NodeID my_id = db.get_id(n);
    printf("Node %" PRIu64 ":\n", my_id);
    printf("All edges: \n");
    for (EdgeIterator i = n.get_edges(); i; i.next()) {
        NodeID other_id = db.get_id(i->get_destination());
        if (other_id == my_id ) { // I am destination
            printf("  <- n%" PRIu64 " (%s,e%" PRIu64 ")\n", db.get_id(i->get_source()),
                    i->get_tag().name().c_str(), db.get_id(*i));
        }
        else {
            printf("  -> n%" PRIu64 " (%s,e%" PRIu64 ")\n", db.get_id(i->get_destination()),
                    i->get_tag().name().c_str(), db.get_id(*i));
        }
    }
    printf("All outgoing edges: \n");
    for (EdgeIterator i = n.get_edges(Outgoing); i; i.next()) {
        printf("  -> n%" PRIu64 " (%s,e%" PRIu64 ")\n", db.get_id(i->get_destination()),
                i->get_tag().name().c_str(), db.get_id(*i));
    }
    printf("All outgoing edges with tag 0: \n");
    for (EdgeIterator i = n.get_edges(Outgoing, "tag0"); i; i.next()) {
        printf("  -> n%" PRIu64 " (%s,e%" PRIu64 ")\n", db.get_id(i->get_destination()),
                i->get_tag().name().c_str(), db.get_id(*i));
    }
    printf("All incoming edges with tag 1: \n");
    for (EdgeIterator i = n.get_edges(Incoming, "tag1"); i; i.next()) {
        printf("  <- n%" PRIu64 " (%s,e%" PRIu64 ")\n", db.get_id(i->get_source()),
                i->get_tag().name().c_str(), db.get_id(*i));
    }
    printf("\n");
}
