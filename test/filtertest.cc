/*
 * This test checks Jarvis iterator filters
 */

#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static void dump(const Graph &db, const Edge &n);
static std::string property_text(const PropertyRef &i);
static int print_exception(FILE *s, Exception& e);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("filtergraph", create ? Graph::Create : Graph::ReadOnly);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(i);
            n.set_property(1, argv[i]);
            n.set_property(2, i + 16);
            n.set_property(3, 22 - i);
            if (prev != NULL) {
                Edge &e = db.add_edge(*prev, n, 0);
                e.set_property(3, prev->get_property(1).string_value());
                e.set_property(4, n.get_property(1).string_value());
            }
            prev = &n;
        }

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property(1).string_value()[0] == 'a'
                           ? pass : dont_pass;
            })
            .process([&db](Node &n) { dump(db, n); });

        // Increment the value of any node starting with 'a'
        printf("Increment value of nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property(1).string_value()[0] == 'a'
                           ? pass : dont_pass;
            })
            .process([&db](Node &n)
                { n.set_property(2, n.get_property(2).int_value() + 1); });

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property(1).string_value()[0] == 'a'
                           ? pass : dont_pass;
            })
            .process([&db](Node &n) { dump(db, n); });

        // Look for value less than 20
        printf("Nodes with value less than 20\n");
        db.get_nodes()
            .filter([](const NodeRef &n) {
                return n.get_property(2).int_value() < 20
                           ? pass : dont_pass;
            })
            .process([&db](Node &n) { dump(db, n); });

        // Look for edge to or from a node starting with 'b'
        printf("Edges to or from a node starting with 'b'\n");
        db.get_edges()
            .filter([](const EdgeRef &e) {
                return e.get_source().get_property(1).string_value()[0] == 'b'
                       || e.get_destination().get_property(1).string_value()[0] == 'b'
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
    }
    catch (Exception e) {
        print_exception(stdout, e);
    }

    return 0;
}

static void dump(const Graph &db, const Node &n)
{
    printf("Node %lu:\n", db.get_id(n));
    n.get_properties()
        .process([&db](PropertyRef &p) {
            printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
    n.get_edges(OUTGOING, 0)
        .process([&db](EdgeRef &e) {
            printf("  -> n%lu (e%lu)\n",
                   db.get_id(e.get_destination()), db.get_id(e));
        });
    n.get_edges(INCOMING, 0)
        .process([&db](EdgeRef &e) {
            printf("  <- n%lu (e%lu)\n",
                   db.get_id(e.get_source()), db.get_id(e));
        });
}

static void dump(const Graph &db, const Edge &e)
{
    printf("Edge %lu: n%lu -> n%lu\n", db.get_id(e),
           db.get_id(e.get_source()), db.get_id(e.get_destination()));
    e.get_properties()
        .process([&db](PropertyRef &p) {
            printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
}

static std::string property_text(const PropertyRef &p)
{
    switch (p.type()) {
        case t_novalue: return "no value";
        case t_boolean: return p.bool_value() ? "T" : "F";
        case t_integer: return std::to_string(p.int_value());
        case t_string: return p.string_value();
        case t_float: return std::to_string(p.float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
    }
    throw Exception(property_type);
}

static int print_exception(FILE *s, Exception& e)
{
    return fprintf(s, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
