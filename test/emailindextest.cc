/**
 * @file   emailindextest.cc
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
#include <string>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

long long num_nodes = 0;
long long num_edges = 0;

static void node_added(Node &);
static void edge_added(Edge &);

int main(int argc, char *argv[])
{
    int fail = 0;
    try {
        printf("Running index range tests on file email.gson\n");
        Graph db("emailindexgraph", Graph::Create);
        Transaction tx(db, Transaction::ReadWrite);
        db.create_index(Graph::NodeIndex, "Person", "Email", PropertyType::String);
        db.create_index(Graph::NodeIndex, "Message", "Size", PropertyType::Integer);
        db.create_index(Graph::NodeIndex, "Message", "Replied?", PropertyType::Boolean);
        db.create_index(Graph::NodeIndex, "Attachment", "Created", PropertyType::Time);
        db.create_index(Graph::EdgeIndex, "Target", "type", PropertyType::String);
        db.create_index(Graph::EdgeIndex, "Target", "order", PropertyType::Integer);
        db.create_index(Graph::EdgeIndex, "Sent", "SubmitTime", PropertyType::Time);
        tx.commit();

        load_gson(db, argv[1], node_added, edge_added);

        Transaction tx1(db, Transaction::ReadWrite);
        printf("## Trying iterator with tag Message and size: with DONT_CARE\n");
        PropertyPredicate pp1("Size");
        int count = 0;
        for (NodeIterator i = db.get_nodes("Message", pp1); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            printf(", searched prop value: %lld\n", i->get_property("Size").int_value());
            ++count;
        }
        if (count != 127)
            ++fail;
        printf("Number of messages with size property: %d\n", count);

        count = 0;
        printf("## Trying iterator with tag Message and size between:10K-11K with GELE\n");
        PropertyPredicate pp2("Size", PropertyPredicate::GeLe, 10000, 11000);
        for (NodeIterator i = db.get_nodes("Message", pp2); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            printf(", searched prop value: %lld\n", i->get_property("Size").int_value());
            ++count;
        }
        if (count != 3)
            ++fail;
        printf("Number of messages with size between 10K and 11K: %d\n", count);

        count = 0;
        printf("## Trying iterator with tag Message and Replied?:true with EQ\n");
        PropertyPredicate pp3("Replied?", PropertyPredicate::Eq, true);
        for (NodeIterator i = db.get_nodes("Message", pp3); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            printf(", searched prop value: %d\n", i->get_property("Replied?").bool_value());
            ++count;
        }
        if (count != 41)
            ++fail;
        printf("Number of messages with Replied? = true: %d\n", count);

        count = 0;
        printf("## Trying iterator with tag Person and email range:alain.kagi to vishakha.s.gupta@intel.com with GELT\n");
        PropertyPredicate pp4("Email", PropertyPredicate::GeLt, "alain.kagi", "vishakha.s.gupta@intel.com");
        for (NodeIterator i = db.get_nodes("Person", pp4); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            printf(", searched prop value: %s\n", i->get_property("Email").string_value().c_str());
            ++count;
        }
        if (count != 20)
            ++fail;
        printf("Number of messages with emails GELT alain.kagi and vishakha.s.gupta@intel.com: %d\n", count);

        count = 0;
        struct tm tm1, tm2;
        int hr, min;
        // There are two entries with the following time value for created.
        string_to_tm("Wed Jun 04 08:00:43 PDT 2014", &tm1, &hr, &min);
        Time t1(&tm1, hr, min);
        // There are three entries with the following time value for created.
        string_to_tm("Mon Jun 30 10:36:32 PDT 2014", &tm2, &hr, &min);
        Time t2(&tm2, hr, min);
        std::string s1 = time_to_string(t1);
        std::string s2 = time_to_string(t2);
        printf("## Trying iterator with tag Attachment and created between: %s and %s with GTLT\n",
                   s1.c_str(), s2.c_str());
        PropertyPredicate pp5("Created", PropertyPredicate::GtLt, t1, t2);
        for (NodeIterator i = db.get_nodes("Attachment", pp5); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            Time t = i->get_property("Created").time_value();
            std::string s = time_to_string(t);
            printf(", searched prop value: %s\n", s.c_str());
            ++count;
        }
        if (count != 27)
            ++fail;
        printf("Number of attachments created in the range: %d\n", count);

        tx1.commit();


        printf("######Edge index tests#####\n");
        Transaction tx2(db, Transaction::ReadOnly);
        printf("## Trying iterator with tag Target and order NEQ 0\n");
        PropertyPredicate ppe1("order", PropertyPredicate::Ne, 0);
        count = 0;
        for (EdgeIterator i = db.get_edges("Target", ppe1); i; i.next()) {
            printf("Edge %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            printf(", searched prop value: %lld\n", i->get_property("order").int_value());
            ++count;
        }
        if (count != 157)
            ++fail;
        printf("Number of From edges with order != 0: %d\n", count);

        count = 0;
        string_to_tm("Wed Jun 04 08:00:02 PDT 2014", &tm1, &hr, &min);
        Time t(&tm1, hr, min);
        std::string s = time_to_string(t);
        printf("## Trying edge iterator with tag Sent and submitted starting: %s Ge\n",
                   s.c_str());
        PropertyPredicate ppe2("SubmitTime", PropertyPredicate::Ge, t);
        for (EdgeIterator i = db.get_edges("Sent", ppe2); i; i.next()) {
            printf("Edge %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            Time t = i->get_property("SubmitTime").time_value();
            std::string s = time_to_string(t);
            printf(", searched prop value: %s\n", s.c_str());
            ++count;
        }
        if (count != 113)
            ++fail;
        printf("Number of submissions starting from the date: %d\n", count);

        count = 0;
        printf("## Trying edge iterator with tag Target and type = cc\n");
        PropertyPredicate ppe3("type", PropertyPredicate::Eq, "cc");
        for (EdgeIterator i = db.get_edges("Target", ppe3); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s", db.get_id(*i), i->get_tag().name().c_str());
            printf(", searched prop value: %s\n", i->get_property("type").string_value().c_str());
            ++count;
        }
        if (count != 82)
            ++fail;
        printf("Number of Target edges with type = cc: %d\n", count);

        count = 0;
        printf("## Trying edge iterator with tag Attachment\n");
        for (EdgeIterator i = db.get_edges("Attachment"); i; i.next()) {
            printf("Edge %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            ++count;
        }
        if (count != 38)
            ++fail;
        printf("Number of Attachment edges: %d\n", count);

        tx2.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 100;
    }

    std::cout << "\nNodes (" << num_nodes << ")\t";
    std::cout << "Edges (" << num_edges << ")\n";

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
