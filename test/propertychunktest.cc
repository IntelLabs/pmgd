/*
 * This test checks Jarvis property lists
 */

#include "jarvis.h"

using namespace Jarvis;

static void dump(const Graph &db, const Node &n);
static std::string property_text(const PropertyIterator &i);
static int print_exception(FILE *s, Exception& e);

int main(int argc, char **argv)
{
    try {
        Graph db("propertychunkgraph", Graph::Create);

        Transaction tx(db);

        Node &n1 = db.add_node(0);
        n1.set_property("id1", "14 char string");
        n1.set_property("id2", "14 char string");
        n1.set_property("id6", "this is a really long string");

        Node &n2 = db.add_node(0);
        n2.set_property("id1", "14 char string");
        n2.set_property("id2", "14 char string");
        n2.set_property("id3", true);
        n2.set_property("id4", true);
        n2.set_property("id5", true);
        n2.set_property("id6", true);

        Node &n3 = db.add_node(0);
        n3.set_property("id1", "14 char string");
        n3.set_property("id2", "14 char string");
        n3.set_property("id3", "8 char s");
        n3.set_property("id6", true);

        Node &n4 = db.add_node(0);
        n4.set_property("id1", "14 char string");
        n4.set_property("id2", "15  char string");
        n4.set_property("id3", "7 char s");
        n4.set_property("id6", true);

        Node &n5 = db.add_node(0);
        n5.set_property("id1", "14 char string");
        n5.set_property("id2", "14 char string");
        n5.set_property("id3", "short1");
        n5.set_property("id6", "short2");
        n5.set_property("id7", "this is another really long string");

        Node &n6 = db.add_node(0);
        n6.set_property("id1", "14 char string");
        n6.set_property("id3", "8 char s");
        n6.set_property("id2", "15  char string");
        n6.set_property("id6", true);

        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            dump(db, *i);
        }

        tx.commit();
    }
    catch (Exception e) {
        print_exception(stdout, e);
        return 1;
    }

    return 0;
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
    switch (i->type()) {
        case t_novalue: return "no value";
        case t_boolean: return i->bool_value() ? "T" : "F";
        case t_integer: return std::to_string(i->int_value());
        case t_string: return i->string_value();
        case t_float: return std::to_string(i->float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
    }
    throw Exception(property_type);
}

static int print_exception(FILE *s, Exception& e)
{
    return fprintf(s, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
