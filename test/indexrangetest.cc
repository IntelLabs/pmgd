/*
 * This test checks Jarvis Index range iterators
 */

#include "jarvis.h"
#include "../src/IndexManager.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    int node_count = 7;
    int edge_count = node_count - 1;

    printf("node_count = %d\n", node_count);

    try {
        Graph db("indexrangegraph", Graph::Create);

        Node **nodes = new Node *[node_count + 1];
        Transaction tx(db, Transaction::ReadWrite);

        db.create_index(Graph::NODE, "tag1", "id1", t_integer);
        for (int i = 1; i <= node_count - 1; i++) {
            Node &n = db.add_node("tag1");
            n.set_property("id1", i + 200);
            nodes[i] = &n;
        }
        // Make sure one of the lists has multiple nodes
        Node &n = db.add_node("tag1");
        n.set_property("id1", 203);
        nodes[node_count] = &n;

        for (int i = 1; i <= edge_count; i++) {
            Edge &e = db.add_edge(*nodes[i], *nodes[i+1], 0);
            e.set_property("id", i + 2611);
        }
        dump_nodes(db);
        dump_edges(db);

        printf("## Trying iterator with tag tag1 and property range:202-205 with GELE\n");
        PropertyPredicate pp1("id1", PropertyPredicate::gele, 202, 205);
        for (NodeIterator i = db.get_nodes("tag1", pp1); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202-205 with GELT\n");
        PropertyPredicate pp2("id1", PropertyPredicate::gelt, 202, 205);
        for (NodeIterator i = db.get_nodes("tag1", pp2); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202-205 with GTLE\n");
        PropertyPredicate pp3("id1", PropertyPredicate::gtle, 202, 205);
        for (NodeIterator i = db.get_nodes("tag1", pp3); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202-205 with GTLT\n");
        PropertyPredicate pp4("id1", PropertyPredicate::gtlt, 202, 205);
        for (NodeIterator i = db.get_nodes("tag1", pp4); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202 with LT\n");
        PropertyPredicate pp5("id1", PropertyPredicate::lt, 202);
        for (NodeIterator i = db.get_nodes("tag1", pp5); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:203 with LE\n");
        PropertyPredicate pp6("id1", PropertyPredicate::le, 203);
        for (NodeIterator i = db.get_nodes("tag1", pp6); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202 with GT\n");
        PropertyPredicate pp7("id1", PropertyPredicate::gt, 202);
        for (NodeIterator i = db.get_nodes("tag1", pp7); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202 with GE\n");
        PropertyPredicate pp8("id1", PropertyPredicate::ge, 202);
        for (NodeIterator i = db.get_nodes("tag1", pp8); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range: DONT_CARE\n");
        PropertyPredicate pp9("id1");
        for (NodeIterator i = db.get_nodes("tag1", pp9); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:202 (non-leaf) NEQ\n");
        PropertyPredicate pp10("id1", PropertyPredicate::ne, 202);
        for (NodeIterator i = db.get_nodes("tag1", pp10); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:203 (leaf) NEQ\n");
        PropertyPredicate pp11("id1", PropertyPredicate::ne, 203);
        for (NodeIterator i = db.get_nodes("tag1", pp11); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:208-210 (non-existent) with GELE\n");
        PropertyPredicate pp12("id1", PropertyPredicate::gele, 208, 210);
        for (NodeIterator i = db.get_nodes("tag1", pp12); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:200-201 (non-existent) with GELT\n");
        PropertyPredicate pp13("id1", PropertyPredicate::gelt, 200, 201);
        for (NodeIterator i = db.get_nodes("tag1", pp13); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        printf("## Trying iterator with tag tag1 and property range:201-201 with GELE\n");
        PropertyPredicate pp14("id1", PropertyPredicate::gele, 201, 201);
        for (NodeIterator i = db.get_nodes("tag1", pp14); i; i.next()) {
            printf("Node %lu: tag %s\n", db.get_id(*i), i->get_tag().name().c_str());
            printf("\tConfirming searched prop value: %lld\n", i->get_property("id1").int_value());
        }

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
