/*
 * This test checks Jarvis iterator filters
 */

#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("filtergraph", create ? Graph::Create : Graph::ReadOnly);

        Transaction tx(db, Transaction::ReadWrite);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            char tag[8] = {0};
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
                           ? pass : dont_pass;
            })
            .process([&db](Node &n) { dump(db, n); });

        // Increment the value of any node starting with 'a'
        printf("Increment value of nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id1").string_value()[0] == 'a'
                           ? pass : dont_pass;
            })
            .process([&db](Node &n)
                { n.set_property("id2", n.get_property("id2").int_value() + 1); });

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id1").string_value()[0] == 'a'
                           ? pass : dont_pass;
            })
            .process([&db](Node &n) { dump(db, n); });

        // Look for value less than 20
        printf("Nodes with value less than 20\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property("id2").int_value() < 20
                           ? pass : dont_pass;
            })
            .process([&db](Node &n) { dump(db, n); });

        // Look for edge to or from a node starting with 'b'
        printf("Edges to or from a node starting with 'b'\n");
        db.get_edges()
            .filter([](const EdgeRef &e) {
                return e.get_source().get_property("id1").string_value()[0] == 'b'
                       || e.get_destination().get_property("id1").string_value()[0] == 'b'
                           ? pass : dont_pass;
            })
            .process([&db](EdgeRef &e) { dump(db, e); });

        printf("Nodes with any property less than 20\n");
        db.get_nodes()
            .process([&db](Node &n) {
                bool f = false;
                n.get_properties()
                    .filter([](const PropertyRef &p) {
                        return p.type() == t_integer && p.int_value() < 20
                               ? pass : dont_pass;
                    })
                    .process([&db,&n,&f](PropertyRef &p) {
                        if (!f) {
                            printf("Node %lu:\n", db.get_id(n));
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
