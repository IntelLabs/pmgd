#include <iostream>
#include "jarvis.h"
#include "../util/util.h"

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
        db.create_index(Graph::NODE, "Person", "Email", t_string);
        db.create_index(Graph::NODE, "Message", "Size", t_integer);
        db.create_index(Graph::NODE, "Message", "Replied?", t_boolean);
        tx.commit();

        load_gson(db, "email.gson", node_added, edge_added);

        Transaction tx1(db, Transaction::ReadWrite);
        printf("## Trying iterator with tag Message and size: with DONT_CARE\n");
        PropertyPredicate pp1("Size");
        int count = 0;
        for (NodeIterator i = db.get_nodes("Message", pp1); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
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
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
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
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
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
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %s\n", i->get_property("Email").string_value().c_str());
            ++count;
        }
        if (count != 20)
            ++fail;
        printf("Number of messages with emails GELT alain.kagi and vishakha.s.gupta@intel.com: %d\n", count);

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
