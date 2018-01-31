/**
 * @file   dump_gexf.cc
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
#include <string.h>
#include <unordered_map>
#include "pmgd.h"
#include "util.h"

using namespace std;
using namespace PMGD;

static void dump_gexf_node_attr(FILE *f, const string name, const string type);
static void dump_gexf_node_attrs(Graph &db, FILE *f);
static void dump_gexf_nodes(Graph &db, FILE *f);
static void dump_gexf_edges(Graph &db, FILE *f);
static void dump_gexf(Graph &db, const Node &n, FILE *f);
static void dump_gexf(Graph &db, const Edge &e, FILE *f);
static std::string strxml(const std::string &str);

static int current_node_attr_id = 0;

/**
 * Map an attribute name to an attribute ID.
 */
static unordered_map<string, int> node_attr_name_map;

void dump_gexf(Graph &db, FILE *f)
{
    current_node_attr_id = 0;
    node_attr_name_map.clear();

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<gexf xmlns=\"http://www.gexf.net/1.2draft\" version=\"1.2\">\n");
    fprintf(f, "  <graph defaultedgetype=\"directed\">\n");
    dump_gexf_node_attrs(db, f);
    dump_gexf_nodes(db, f);
    dump_gexf_edges(db, f);
    fprintf(f, "  </graph>\n");
    fprintf(f, "</gexf>\n");

    node_attr_name_map.clear();
}

static void dump_gexf_node_attr(FILE *f, const string name, const string type)
{
    if (node_attr_name_map.find(name) != node_attr_name_map.end())
        return;

    node_attr_name_map.insert({name, current_node_attr_id});
    fprintf(f, "      <attribute id=\"%d\" title=\"%s\" type=\"%s\" />\n",
            current_node_attr_id, name.c_str(), type.c_str());
    current_node_attr_id++;
}

static void dump_gexf_node_attrs(Graph &db, FILE *f)
{
    fprintf(f, "    <attributes class=\"node\">\n");
    for (NodeIterator n = db.get_nodes(); n; n.next())
        for (PropertyIterator p = n->get_properties(); p; p.next()) {
            switch(p->type()) {
            case PropertyType::Boolean:
                dump_gexf_node_attr(f, p->id().name(), "boolean");
                break;
            case PropertyType::Integer:
                dump_gexf_node_attr(f, p->id().name(), "integer");
                break;
            case PropertyType::String:
                dump_gexf_node_attr(f, p->id().name(), "string");
                break;
            case PropertyType::Float:
                dump_gexf_node_attr(f, p->id().name(), "float");
                break;
            default:
                ;
            }
        }
    fprintf(f, "    </attributes>\n");
}

static void dump_gexf_nodes(Graph &db, FILE *f)
{
    fprintf(f, "    <nodes>\n");
    db.get_nodes().process([&db, &f](Node &n) { dump_gexf(db, n, f); });
    fprintf(f, "    </nodes>\n");
}

static void dump_gexf_edges(Graph &db, FILE *f)
{
    fprintf(f, "    <edges>\n");
    db.get_edges().process([&db, &f](Edge &e) { dump_gexf(db, e, f); });
    fprintf(f, "    </edges>\n");
}

static void dump_gexf(Graph &db, const Node &n, FILE *f)
{
    fprintf(f, "      <node id=\"%lu\" label=\"%s\">\n",
            db.get_id(n),
            strxml(n.get_tag().name()).c_str());
    PropertyIterator p = n.get_properties();
    if (p) {
        fprintf(f, "        <attvalues>\n");
        for (; p; p.next()) {
            fprintf(f, "          <attvalue for=\"%d\" value=\"%s\" />\n",
                    node_attr_name_map[p->id().name()],
                    strxml(property_text(*p)).c_str());
        }
        fprintf(f, "        </attvalues>\n");
    }
    fprintf(f, "      </node>\n");
}

static void dump_gexf(Graph &db, const Edge &e, FILE *f)
{
    fprintf(f, "      <edge id=\"%lu\" source=\"%lu\" target=\"%lu\" label=\"%s\" />\n",
            db.get_id(e),
            db.get_id(e.get_source()), db.get_id(e.get_destination()),
            strxml(e.get_tag().name()).c_str());
}

/**
 * Convert string to XML-acceptable string
 * @param str regular C++ string
 * @return XML string
 */
static std::string strxml(const std::string &str)
{
    std::string s = "";

    size_t curr = 0;

    while (true) {
        size_t next = str.find_first_of("<>&\"", curr);
        s += str.substr(curr, next - curr);
        if (next == std::string::npos) return s;
        switch (str[next]) {
            case '<': s += "&lt;"; break;
            case '>': s += "&gt;"; break;
            case '&': s += "&amp;"; break;
            case '"': s += "&quot;"; break;
        }
        curr = next + 1;
    }
}
