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

    return 0;
}

static void dump(Node &n)
{
    printf("Node %llu:\n", n.get_id());
    for (PropertyIterator i = n.get_properties(); i; i.next()) {
        printf("  %d: %s\n", i->id(), property_text(i).c_str());
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

Jarvis::Graph::Graph(const char *, int)
{
}

Jarvis::Graph::~Graph()
{
}

Jarvis::NodeIterator Jarvis::Graph::get_nodes()
{
    throw e_not_implemented;
}

Node &Jarvis::Graph::add_node(StringID id)
{
    throw e_not_implemented;
}

NodeID Node::get_id() const
{
    throw e_not_implemented;
}

PropertyIterator Node::get_properties() const
{
    throw e_not_implemented;
}

void Node::set_property(const Property &)
{
}

Jarvis::StringID::StringID(const char *)
{
}

Jarvis::Property Jarvis::Edge::get_property(StringID) const
{
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
