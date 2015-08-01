#include <stdio.h>
#include <string>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

static void print_node(Graph &db, const Node &n, FILE *f);
static void print_edge(Graph &db, const Edge &n, FILE *f);
static void print_property_list(PropertyIterator p, FILE *f);
static void print_property(const PropertyIterator &p, FILE *f);

void dump_jarvis(Graph &db, FILE *f)
{
    db.get_nodes().process([&db, &f](Node &n) { print_node(db, n, f); });
    db.get_edges().process([&db, &f](Edge &e) { print_edge(db, e, f); });
}


static void print_node(Graph &db, const Node &n, FILE *f)
{
    fprintf(f, "%lu%s", db.get_id(n), tag_text(n).c_str());
    StringID id = 0;
    try {
        id = "jarvis.loader.id";
    }
    catch (Jarvis::Exception e) {
        if (e.num != ExceptionType::ReadOnly)
            throw;
    }
    print_property_list(n.get_properties()
                        .filter([id](const PropertyRef &p)
                                { return p.id() == id ? DontPass : Pass; }), f);
    fprintf(f, ";\n");
}


static void print_edge(Graph &db, const Edge &e, FILE *f)
{
    fprintf(f, "%lu %lu :",
            db.get_id(e.get_source()), db.get_id(e.get_destination()));
    fprintf(f, "%s", tag_text(e).c_str());
    print_property_list(e.get_properties(), f);
    fprintf(f, ";\n");
}


static void print_property_list(PropertyIterator p, FILE *f)
{
    if (p) {
        bool first = true;
        fprintf(f, " { ");
        for (; p; p.next()) {
            if (!first) fprintf(f, ", ");
            first = false;
            print_property(p, f);
        }
        fprintf(f, " }");
    }
}

static void print_property(const PropertyIterator &p, FILE *f)
{
    fprintf(f, "%s", p->id().name().c_str());
    std::string value;
    switch (p->type()) {
        case PropertyType::NoValue: return;
        case PropertyType::Boolean: value = p->bool_value() ? "true" : "false"; break;
        case PropertyType::Integer: value = std::to_string(p->int_value()); break;
        case PropertyType::String: value = "\"" + p->string_value() + "\""; break;
        case PropertyType::Float: value = std::to_string(p->float_value()); break;
        case PropertyType::Time: /* TBD */ return; // break;
        case PropertyType::Blob: /* TBD */ return; // break;
        default: throw Exception(PropertyTypeInvalid);
    }
    fprintf(f, " = %s", value.c_str());
}
