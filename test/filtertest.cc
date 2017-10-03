/**
 * @file   filtertest.cc
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
 * This test checks PMGD iterator filters
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("filtergraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db, Transaction::ReadWrite);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            char tag[14] = {0};
            sprintf(tag, "tag%d",i);
            Node &n = db.add_node(tag);
            n.set_property("id1", argv[i]);
            n.set_property("id2", i + 16);
            n.set_property("id3", 22 - i);
            if (prev != NULL) {
                Edge &e = db.add_edge(*prev, n, 0);
                e.set_property("id3", prev->get_property("id1").string_value());
                e.set_property("id4", n.get_property("id1").string_value());
            }
            prev = &n;
        }

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id1").string_value()[0] == 'a'
                           ? Pass : DontPass;
            })
            .process([](Node &n) { dump(n); });

        // Increment the value of any node starting with 'a'
        printf("Increment value of nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id1").string_value()[0] == 'a'
                           ? Pass : DontPass;
            })
            .process([&db](Node &n)
                { n.set_property("id2", n.get_property("id2").int_value() + 1); });

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id1").string_value()[0] == 'a'
                           ? Pass : DontPass;
            })
            .process([](Node &n) { dump(n); });

        // Look for value less than 20
        printf("Nodes with value less than 20\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id2").int_value() < 20
                           ? Pass : DontPass;
            })
            .process([](Node &n) { dump(n); });

        // Look for edge to or from a node starting with 'b'
        printf("Edges to or from a node starting with 'b'\n");
        db.get_edges()
            .filter([](const EdgeRef &e) {
                return e.get_source().get_property("id1").string_value()[0] == 'b'
                       || e.get_destination().get_property("id1").string_value()[0] == 'b'
                           ? Pass : DontPass;
            })
            .process([](EdgeRef &e) { dump(e); });

        printf("Nodes with any property less than 20\n");
        db.get_nodes()
            .process([&db](Node &n) {
                bool f = false;
                n.get_properties()
                    .filter([](const PropertyRef &p) {
                        return p.type() == PropertyType::Integer && p.int_value() < 20
                               ? Pass : DontPass;
                    })
                    .process([&db,&n,&f](PropertyRef &p) {
                        if (!f) {
                            printf("Node %" PRIu64 ":\n", db.get_id(n));
                            f = true;
                        }
                        std::string id = p.id().name();
                        long long val = p.int_value();
                        printf("  %s: %lld\n", id.c_str(), val);
                    });
            });

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
