/**
 * @file   dump_pmgd.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <string>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

static void print_node(Graph &db, const Node &n, FILE *f, StringID strid);
static void print_edge(Graph &db, const Edge &n, FILE *f, StringID strid);
static void print_property_list(PropertyIterator p, FILE *f);
static void print_property(const PropertyIterator &p, FILE *f);

void dump_pmgd(Graph &db, FILE *f)
{
    StringID strid = 0;
    try {
        // Ensure String id is in the string table
        strid = "pmgd.loader.id";
    }
    catch (PMGD::Exception e) {
        if (e.num != ExceptionType::ReadOnly)
            throw;
    }
    db.get_nodes().process([&db, &f, strid](Node &n) { print_node(db, n, f, strid); });
    db.get_edges().process([&db, &f, strid](Edge &e) { print_edge(db, e, f, strid); });
}


static void print_node(Graph &db, const Node &n, FILE *f, StringID strid)
{
    Property result;
    if(strid == 0 || !n.check_property(strid, result)) {
        NodeID id = db.get_id(n);
        fprintf(f, "%lu%s", id, tag_text(n).c_str());
    }
    else
        fprintf(f, "%s%s", property_text(result).c_str(), tag_text(n).c_str());
    print_property_list(n.get_properties()
                        .filter([strid](const PropertyRef &p)
                                { return p.id() == strid ? DontPass : Pass; }), f);
    fprintf(f, ";\n");
}


static void print_edge(Graph &db, const Edge &e, FILE *f, StringID strid)
{
    Property result;
    // Enough to check if one node has the loader property
    if (strid == 0 || !e.get_source().check_property(strid, result)) {
        NodeID srcid, dstid;
        srcid = db.get_id(e.get_source());
        dstid = db.get_id(e.get_destination());
        fprintf(f, "%lu %lu :", srcid, dstid);
    }
    else {
        Property destid = e.get_destination().get_property(strid);
        fprintf(f, "%s %s :", property_text(result).c_str(),
                                property_text(destid).c_str());
    }
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
        case PropertyType::String: {
            std::string tmp = p->string_value();
            std::string::size_type pos = 0;
            std::string::size_type prev = 0;
            value.append("\"");
            while ((pos = tmp.find_first_of("\"\\", pos)) != tmp.npos) {
                value.append(tmp, prev, pos - prev);
                value.append("\\");
                prev = pos;
                pos++;
            }
            value.append(tmp, prev, tmp.npos);
            value.append("\"");
            break;
        }
        case PropertyType::Float: value = std::to_string(p->float_value()); break;
        case PropertyType::Time: value = time_to_string(p->time_value()); break;
        case PropertyType::Blob: /* TBD */ return; // break;
        default: throw PMGDException(PropertyTypeInvalid);
    }
    fprintf(f, " = %s", value.c_str());
}
