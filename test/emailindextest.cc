#include <iostream>
#include <string>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

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
        db.create_index(Graph::NodeIndex, "Person", "Email", t_string);
        db.create_index(Graph::NodeIndex, "Message", "Size", t_integer);
        db.create_index(Graph::NodeIndex, "Message", "Replied?", t_boolean);
        db.create_index(Graph::NodeIndex, "Attachment", "Created", t_time);
        tx.commit();

        load_gson(db, "email.gson", node_added, edge_added);

        Transaction tx1(db, Transaction::ReadWrite);
        printf("## Trying iterator with tag Message and size: with DONT_CARE\n");
        PropertyPredicate pp1("Size");
        int count = 0;
        for (NodeIterator i = db.get_nodes("Message", pp1); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("Size").int_value());
            ++count;
        }
        if (count != 127)
            ++fail;
        printf("Number of messages with size property: %d\n", count);

        count = 0;
        printf("## Trying iterator with tag Message and size between:10K-11K with GELE\n");
        PropertyPredicate pp2("Size", PropertyPredicate::gele, 10000, 11000);
        for (NodeIterator i = db.get_nodes("Message", pp2); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("Size").int_value());
            ++count;
        }
        if (count != 3)
            ++fail;
        printf("Number of messages with size between 10K and 11K: %d\n", count);

        count = 0;
        printf("## Trying iterator with tag Message and Replied?:true with EQ\n");
        PropertyPredicate pp3("Replied?", PropertyPredicate::eq, true);
        for (NodeIterator i = db.get_nodes("Message", pp3); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %d\n", i->get_property("Replied?").bool_value());
            ++count;
        }
        if (count != 41)
            ++fail;
        printf("Number of messages with Replied? = true: %d\n", count);

        count = 0;
        printf("## Trying iterator with tag Person and email range:alain.kagi to vishakha.s.gupta@intel.com with GELT\n");
        PropertyPredicate pp4("Email", PropertyPredicate::gelt, "alain.kagi", "vishakha.s.gupta@intel.com");
        for (NodeIterator i = db.get_nodes("Person", pp4); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %s\n", i->get_property("Email").string_value().c_str());
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
        PropertyPredicate pp5("Created", PropertyPredicate::gtlt, t1, t2);
        for (NodeIterator i = db.get_nodes("Attachment", pp5); i; i.next()) {
            printf("Node %" PRIu64 ": tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            Time t = i->get_property("Created").time_value();
            std::string s = time_to_string(t);
            printf("\tConfirming searched prop value: %s\n", s.c_str());
            ++count;
        }
        if (count != 27)
            ++fail;
        printf("Number of attachments created in the range: %d\n", count);

        tx1.commit();
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
