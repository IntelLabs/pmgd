#include <stdio.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

std::string property_text(const Property &p)
{
    switch (p.type()) {
        case t_novalue: return "no value";
        case t_boolean: return p.bool_value() ? "T" : "F";
        case t_integer: return std::to_string(p.int_value());
        case t_string: return p.string_value();
        case t_float: return std::to_string(p.float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
        default: throw Exception(property_type);
    }
}

std::string property_text(const PropertyRef &p)
{
    switch (p.type()) {
        case t_novalue: return "no value";
        case t_boolean: return p.bool_value() ? "T" : "F";
        case t_integer: return std::to_string(p.int_value());
        case t_string: return p.string_value();
        case t_float: return std::to_string(p.float_value());
        case t_time: return "<time value>";
        case t_blob: return "<blob value>";
        default: throw Exception(property_type);
    }
}

template <typename T>
std::string tag_text(const T &n)
{
    std::string tag = n.get_tag().name();
    if (tag != "")
        tag = " #" + tag;
    return tag;
}

template std::string tag_text<Node>(const Node &);
template std::string tag_text<Edge>(const Edge &);
template std::string tag_text<EdgeRef>(const EdgeRef &);
