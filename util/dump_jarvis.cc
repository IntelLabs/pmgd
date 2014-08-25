#include <stdio.h>
#include <string>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

static void print_node(Graph &db, const Node &n, FILE *f);
static void print_edge(Graph &db, const Edge &n, FILE *f);
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
    PropertyIterator p = n.get_properties()
        .filter([id](const PropertyRef &p)
                { return p.id() == id ? DontPass : Pass; });
    if (p) {
        fprintf(f, " { ");
        for (; p; p.next())
            print_property(p, f);
        fprintf(f, "}");
    }
    fprintf(f, ";\n");
}


static void print_edge(Graph &db, const Edge &e, FILE *f)
{
    fprintf(f, "%lu %lu :",
            db.get_id(e.get_source()), db.get_id(e.get_destination()));
    PropertyIterator p = e.get_properties();
    fprintf(f, "%s", tag_text(e).c_str());
    if (p) {
        fprintf(f, " { ");
        for (; p; p.next())
            print_property(p, f);
        fprintf(f, "}");
    }
    fprintf(f, ";\n");
}


static void print_property(const PropertyIterator &p, FILE *f)
{
    fprintf(f, "%s ", p->id().name().c_str());
    std::string s;
    switch (p->type()) {
        case PropertyType::NoValue: return;
        case PropertyType::Boolean: s = p->bool_value()?"TRUE":"FALSE"; break;
        case PropertyType::Integer: s = std::to_string(p->int_value()); break;
        case PropertyType::String: s = "\"" + p->string_value() + "\""; break;
        case PropertyType::Float: s = std::to_string(p->float_value()); break;
        case PropertyType::Time: s = "<Time value>"; break;
        case PropertyType::Blob: s = "<blob>"; break;
        default: throw Exception(PropertyTypeInvalid);
    }
    fprintf(f, "= %s ", s.c_str());
}
