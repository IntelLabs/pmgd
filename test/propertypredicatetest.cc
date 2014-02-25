/*
 * This test checks Jarvis iterator filters
 */

#include <stdio.h>
#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static void dump(const Graph &db, NodeIterator i);
static std::string property_text(const PropertyIterator &i);
static int print_exception(FILE *s, Exception& e);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("ppgraph", create ? Graph::Create : Graph::ReadOnly);

        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(i);
            n.set_property(Property(1, argv[i]));
            n.set_property(Property(2, i));
        }

        // Look for name starting with 'a'
        printf("Nodes starting with 'a'\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(1, PropertyPredicate::gelt, "a", "b")));

        // Look for name starting with 'b'
        printf("Nodes starting with 'b'\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(1, PropertyPredicate::gelt, "b", "c")));

        // Look for value less than 2
        printf("Nodes with value less than 2\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(2, PropertyPredicate::lt, 2)));

        // Look for value less or equal to than 4
        printf("Nodes with value less than or equal to 4\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(2, PropertyPredicate::le, 4)));

        // Look for value greater than 3
        printf("Nodes with value greater than 3\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(2, PropertyPredicate::gt, 3)));

        // Look for value greater than or equal to  1
        printf("Nodes with value greater than or equal to 1\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(2, PropertyPredicate::ge, 1)));

        // Look for value between 2 and 5
        printf("Nodes with value between 2 and 5\n");
        dump(db, db.get_nodes()
            .filter(PropertyPredicate(2, PropertyPredicate::gele, 2, 5)));

    }
    catch (Exception e) {
        print_exception(stdout, e);
    }

    return 0;
}

static void dump(const Graph &db, NodeIterator i)
{
    while (i) {
        dump(db, *i);
        i.next();
    }
}

static void dump(const Graph &db, const Node &n)
{
    printf("Node %lu:\n", db.get_id(n));
    for (PropertyIterator i = n.get_properties(); i; i.next()) {
        printf("  %s: %s\n", i->id().name().c_str(), property_text(i).c_str());
    }
    for (EdgeIterator i = n.get_edges(OUTGOING, 0); i; i.next()) {
        printf("  -> n%lu (e%lu)\n", db.get_id(i->get_destination()), db.get_id(*i));
    }
    for (EdgeIterator i = n.get_edges(INCOMING, 0); i; i.next()) {
        printf("  <- n%lu (e%lu)\n", db.get_id(i->get_source()), db.get_id(*i));
    }
}

static std::string property_text(const PropertyIterator &i)
{
    switch (i->value().type()) {
        case t_novalue: return "no value";
        case t_boolean: return i->value().bool_value() ? "T" : "F";
        case t_integer: return std::to_string(i->value().int_value());
        case t_string: return i->value().string_value();
        case t_float: return std::to_string(i->value().float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
    }
    throw Exception(property_type);
}

static int print_exception(FILE *s, Exception& e)
{
    return fprintf(s, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
