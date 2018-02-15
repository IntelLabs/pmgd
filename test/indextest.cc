/**
 * @file   indextest.cc
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
 * This test checks PMGD Index (eq iterators)
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

int main(int argc, char **argv)
{
    int node_count = 13;
    int edge_count = node_count - 1;

    printf("node_count = %d\n", node_count);

    try {
        Graph db("indexgraph", Graph::Create);

        Node **nodes = new Node *[node_count + 1];
        Transaction tx(db, Transaction::ReadWrite);

        db.create_index(Graph::NodeIndex, "tag1", "id1", PropertyType::Integer);
        db.create_index(Graph::NodeIndex, "tag1", "id2", PropertyType::Float);
        db.create_index(Graph::NodeIndex, "tag2", "id1", PropertyType::Float);
        db.create_index(Graph::NodeIndex, "tag2", "id2", PropertyType::String);
        db.create_index(Graph::NodeIndex, 0, "id3", PropertyType::Integer);
        for (int i = 1; i <= 2; i++) {
            Node &n = db.add_node("tag1");
            n.set_property("id1", i + 1611);
            n.set_property("id3", i + 2000);
            nodes[i] = &n;
        }
        for (int i = 3; i <= 4; i++) {
            Node &n = db.add_node("tag1");
            // More nodes with the same property value and tag
            n.set_property("id1", i - 2 + 1611);
            nodes[i] = &n;
        }
        for (int i = 5; i <= 6; i++) {
            Node &n = db.add_node("tag1");
            n.set_property("id2", i + 23.57);
            nodes[i] = &n;
        }
        try {
            printf("***Testing bad set property\n");
            for (int i = 7; i <= 7; i++) {
                Node &n = db.add_node("tag1");
                nodes[i] = &n;
                n.set_property("id2", true);
            }
        } catch (Exception e) {
            print_exception(e);
        }
        for (int i = 8; i <= 8; i++) {
            Node &n = db.add_node("tag2");
            n.set_property("id1", i + 23.57);
            nodes[i] = &n;
        }
        // String property
        for (int i = 9; i <= 10; i++) {
            Node &n = db.add_node("tag2");
            n.set_property("id2", "This is string test");
            n.set_property("id3", i + 2000);
            nodes[i] = &n;
        }
        for (int i = 11; i <= 11; i++) {
            Node &n = db.add_node("tag2");
            n.set_property("id2", "This is awesome");
            n.set_property("id3", i + 2000);
            nodes[i] = &n;
        }
        for (int i = 12; i <= 12; i++) {
            Node &n = db.add_node("tag2");
            n.set_property("id2", "An apple");
            nodes[i] = &n;
        }
        for (int i = 13; i <= node_count; i++) {
            Node &n = db.add_node("tag2");
            n.set_property("id2", "An apple and peach.");
            nodes[i] = &n;
        }

        for (int i = 1; i <= edge_count; i++) {
            Edge &e = db.add_edge(*nodes[i], *nodes[i+1], 0);
            e.set_property("id", i + 2611);
        }
        dump_debug(db);

        printf("## Trying iterator with tag tag1 and property id1:1612\n");
        PropertyPredicate pp("id1", PropertyPredicate::Eq, 1612);
        for (NodeIterator i = db.get_nodes("tag1", pp); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Test set_property involving a remove and add\n");
        nodes[1]->set_property("id1", 2433);
        printf("Property id1 for node 1: %lld\n", nodes[1]->get_property("id1").int_value());

        printf("## Trying iterator with tag tag1 and property id1:1612 after changing value of one node\n");
        for (NodeIterator i = db.get_nodes("tag1", pp); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property id1:2433 after changing value of one node\n");
        pp.v1 = 2433;
        for (NodeIterator i = db.get_nodes("tag1", pp); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with just tag1\n");
        for (NodeIterator i = db.get_nodes("tag1"); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
        }
        printf("## Trying iterator with just tag2\n");
        for (NodeIterator i = db.get_nodes("tag2"); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            Property result;
            if (i->check_property("id1", result))
                printf("\tChecking prop value: %f\n", i->get_property("id1").float_value());
            if (i->check_property("id2", result))
                printf("\tChecking prop value: %s\n", i->get_property("id2").string_value().c_str());
        }

        printf("## Trying iterator with tag and float prop\n");
        PropertyPredicate ppf("id1", PropertyPredicate::Eq, 31.57);
        for (NodeIterator i = db.get_nodes("tag2", ppf); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lf\n", i->get_property("id1").float_value());
        }

        printf("## Trying iterator with tag and string prop\n");
        PropertyPredicate pps("id2", PropertyPredicate::Eq, "This is string test");
        for (NodeIterator i = db.get_nodes("tag2", pps); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tChecking prop value: %s\n", i->get_property("id2").string_value().c_str());
        }

        printf("## Test set_property involving a remove and add for strings\n");
        nodes[11]->set_property("id2", "Aspiring");
        printf("Property id2 for node 11: %s\n", nodes[11]->get_property("id2").string_value().c_str());

        printf("## Trying iterator with tag and string prop AFTER change\n");
        PropertyPredicate pps1("id2", PropertyPredicate::Eq, "Aspiring");
        for (NodeIterator i = db.get_nodes("tag2", pps1); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tChecking prop value: %s\n", i->get_property("id2").string_value().c_str());
        }

        printf("## Trying NULL iterator with missing property\n");
        ppf.v1 = 34.8;
        for (NodeIterator i = db.get_nodes("tag2", ppf); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
        }

        printf("## Trying NULL iterator with tag3\n");
        for (NodeIterator i = db.get_nodes("tag3"); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
        }

        printf("## Trying plain get_nodes() iterator\n");
        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
        }

        printf("## Trying  get_nodes(0,pp) iterator\n");
        PropertyPredicate pp0("id3", PropertyPredicate::Ge, 2000);
        for (NodeIterator i = db.get_nodes(0, pp0); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s, value %lld\n",
                    db.get_id(*i), i->get_tag().name().c_str(),
                    i->get_property("id3").int_value());
        }
        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
