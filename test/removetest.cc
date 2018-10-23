/**
 * @file   removetest.cc
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
#include <vector>
#include <stdio.h>
#include <set>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;
using namespace std;

long long num_nodes = 0;
long long num_edges = 0;

static void node_added(Node &);
static void edge_added(Edge &);
static void dump_properties(PropertyIterator);

#define ARRAY_SIZEOF(a) (sizeof (a) / sizeof (a)[0])

int main(int argc, char *argv[])
{
    const char ID_STR[] = "pmgd.loader.id";
    int fail = 0;

    try {
        Graph db("removegraph", Graph::Create);
        Transaction tx(db, Transaction::ReadWrite);
        StringID ID = StringID(ID_STR);
        db.create_index(Graph::NodeIndex, "tag1", "id1", PropertyType::Integer);
        db.create_index(Graph::EdgeIndex, 0, "name", PropertyType::String);
        tx.commit();

        load(db, argv[1], true, node_added, edge_added);
        printf("count of nodes: %lld, count of edges: %lld\n", num_nodes, num_edges);

        // Remove properties (long strings) to empty chunk lists
        {
            printf("Remove properties with string id name but value not equal to test\n");
            int count = 0, rem_count = 0;
            Transaction tx(db, Transaction::ReadWrite);
            StringID name("name");
            // In this case, the following iterator returns all nodes.
            for (NodeIterator ni = db.get_nodes("tag1"); ni; ni.next()) {
                // Find nodes with property name = test
                Node &n = *ni;
                for (PropertyIterator pi = n.get_properties(); pi; pi.next()) {
                    if (pi->id() == name) {
                        if (pi->string_value().compare("test") == 0)
                            count++;
                        else {
                            n.remove_property(name);
                            rem_count++;
                        }
                    }
                }
            }
            if (count != 1 && rem_count != 4) {
                printf("Property remove FAIL: %d, %d\n", count, rem_count);
                fail++;
            }
            tx.commit();
            printf("Transaction committed\n");
            // Needs an allocator stats print here instead
            Transaction tx1(db, Transaction::ReadOnly);
            for (NodeIterator ni = db.get_nodes("tag1"); ni; ni.next())
                dump_properties(ni->get_properties());
            tx1.commit();

            // Test on edges where the index should be emptied out
            printf("Removing property on edge, different tags but indexed\n");
            Transaction tx2(db, Transaction::ReadWrite);
            count = 0;
            PropertyPredicate pp(name);
            for (EdgeIterator ei = db.get_edges(0, pp); ei; ei.next()) {
                ei->remove_property(name);
                ++count;
            }
            tx2.commit();
            if (count != 2) {
                printf("Property remove on edge FAIL: %d\n", count);
                fail++;
            }

            printf("Testing iterator\n");
            Transaction tx3(db, Transaction::ReadOnly);
            if (db.get_edges(0, pp)) {
                printf("Property remove iterator non-empty FAIL\n");
                fail++;
            }
            printf("Property test done\n");
        }

        // Remove node tests
        set<long long> remove_nodes;

        // There are two nodes with value 5.
        int remove_vals[] = {55, 60, 45, 40, 15, 17, 10, 5};
        int remove_count = ARRAY_SIZEOF(remove_vals);
        {
            int count = 0;
            Transaction tx(db, Transaction::ReadWrite);

            printf("Remove certain values. Uses index.\n");
            for (int i = 0; i < remove_count; ++i) {
                PropertyPredicate pp("id1", PropertyPredicate::Eq, remove_vals[i]);
                for (NodeIterator ni = db.get_nodes("tag1", pp); ni; ni.next()) {
                    remove_nodes.insert(ni->get_property(ID).int_value());
                    printf("Removing node with id1=%d, node id: %lld\n",
                        remove_vals[i], ni->get_property(ID).int_value());
                    db.remove(*ni);
                }
            }
            printf("count of remove nodes: %ld\n", remove_nodes.size());
            // Make sure all the nodes in the remove list are out.
            for (auto it : remove_nodes) {
                PropertyPredicate pp(ID, PropertyPredicate::Eq, it);
                for (NodeIterator ni = db.get_nodes(0 , pp); ni; ni.next()) {
                    printf("STILL PRESENT: %lld\n", ni->get_property(ID).int_value());
                    count++;
                }
            }
            if (count > 0) {
                printf("Remove before abort FAIL: %d\n", count);
                fail++;
            }
            else
                printf("PASS before abort\n");
            printf("Transaction going to be ABORTED\n");
        }

        {
            unsigned count = 0;
            Transaction tx(db, Transaction::ReadWrite);
            // Make sure all the nodes in the remove list are still present in the graph.
            for (auto it : remove_nodes) {
                PropertyPredicate pp(ID, PropertyPredicate::Eq, it);
                for (NodeIterator ni = db.get_nodes(0, pp); ni; ni.next())
                    count++;
            }
            if (count != remove_nodes.size()) {
                printf("Remove before commit FAIL: %d\n", count);
                fail++;
            }
            printf("Remove values again. Uses index.\n");
            for (int i = 0; i < remove_count; ++i) {
                PropertyPredicate pp("id1", PropertyPredicate::Eq, remove_vals[i]);
                for (NodeIterator ni = db.get_nodes("tag1", pp); ni; ni.next()) {
                    printf("Removing node with id1=%d, node id: %lld\n",
                        remove_vals[i], ni->get_property("pmgd.loader.id").int_value());
                    db.remove(*ni);
                }
            }
            tx.commit();
            printf("Transaction now COMMITTED\n");
        }

        {
            int count = 0;
            Transaction tx(db, Transaction::ReadOnly);
            // Make sure all the nodes in the remove list are out.
            for (auto it : remove_nodes) {
                PropertyPredicate pp(ID, PropertyPredicate::Eq, it);
                for (NodeIterator ni = db.get_nodes(0, pp); ni; ni.next()) {
                    printf("STILL PRESENT: %lld\n", ni->get_property(ID).int_value());
                    count++;
                }
            }
            if (count > 0) {
                printf("Remove after commit FAIL: %d\n", count);
                fail++;
            }
        }

        // Check only tag based index
        // Remove edges and check their nodes
        {
            printf("Tests to check for tag-based index and edge remove\n");
            Transaction tx(db, Transaction::ReadWrite);
            for (EdgeIterator ei = db.get_edges("edge2"); ei; ei.next())
                db.remove(*ei);
            EdgeIterator ei = db.get_edges("edge2");
            if (ei) {
                printf("Remove edges with tag edge2 before abort FAIL\n");
                fail++;
            }
            printf("Aborting transaction\n");
        }
        {
            int count = 0;
            printf("Now starting transaction to COMMIT\n");
            Transaction tx(db, Transaction::ReadWrite);
            for (EdgeIterator ei = db.get_edges("edge2"); ei; ei.next())
                ++count;
            if (count != 2) {
                printf("Edges before remove missing FAIL\n");
                fail++;
            }
            count = 0;
            printf("Removing the edges AGAIN and committing this time\n");
            for (EdgeIterator ei = db.get_edges("edge2"); ei; ei.next()) {
                db.remove(*ei);
                ++count;
            }
            tx.commit();
            if (count != 2) {
                printf("Tag remove FAIL: %d\n", count);
                fail++;
            }

            printf("Testing iterator\n");
            Transaction tx1(db, Transaction::ReadOnly);
            if (db.get_edges("edge2")) {
                printf("Edge remove iterator non-empty FAIL\n");
                fail++;
            }
            printf("Edge remove tests done\n");
        }
    }
    catch (Exception e) {
        print_exception(e);
        return -1;
    }

    return fail;
}

static void node_added(Node &n)
{
    ++num_nodes;
}

static void edge_added(Edge &e)
{
    ++num_edges;
}

void dump_properties(PropertyIterator iter)
{
    while (iter) {
        printf("  %s: %s\n", iter->id().name().c_str(), property_text(*iter).c_str());
        iter.next();
    }
}
