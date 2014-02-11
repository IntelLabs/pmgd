/*
 * This test checks Jarvis signs of life.
 *
 * Compile with:
 *     make -C ../src
 *     g++-4.8 -std=c++11 -I ../include soltest.cc ../lib/jarvis.lib
 *
 * To include stubs for as-yet unimplemented graph functions, use:
 *     g++-4.8 -std=c++11 -I ../include -DSTUBS soltest.cc ../lib/jarvis.lib
 */

#include "jarvis.h"

using namespace Jarvis;

static void dump(Node &n);
static std::string property_text(const PropertyIterator &i);

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        Graph db("solgraph", create ? Graph::Create : Graph::ReadOnly);

        if (create) {
            Node &a = db.add_node(0);
            a.set_property(Property(0, "node a"));
            Node &b = db.add_node(0);
            b.set_property(Property(0, "node b"));
        }

        for (NodeIterator i = db.get_nodes(); i; i.next()) {
            dump(*i);
        }
    }
    catch (Exception e) {
        printf("EXCEPTION %d\n", e);
    }

    return 0;
}

static void dump(Node &n)
{
    printf("Node %lu:\n", n.get_id());
    for (PropertyIterator i = n.get_properties(); i; i.next()) {
        printf("  %s: %s\n", i->id().name().c_str(), property_text(i).c_str());
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
    throw Jarvis::e_property_type;
}


#ifdef STUBS

PropertyIterator Node::get_properties() const
{
    return PropertyIterator(NULL);
}

void Node::set_property(const Property &)
{
}

Jarvis::Property Jarvis::Edge::get_property(StringID) const
{
    throw e_not_implemented;
}

Jarvis::PropertyValue::PropertyValue(const PropertyValue &a)
{
}

Jarvis::PropertyValue::~PropertyValue()
{
}

bool Jarvis::PropertyValue::operator<(const PropertyValue &a)
{
    throw e_not_implemented;
}
#endif
