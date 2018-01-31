/**
 * @file   load_gson.cc
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

#include <string.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

static const char ID_STR[] = "pmgd.loader.id";
static StringID ID;

/* To support reading from a file or standard input */
class input_t {
    std::ifstream *_stream;

public:
    operator std::istream &() { return _stream ? *_stream : std::cin; }

    input_t(const char *filename)
    {
        if (strcmp(filename, "-") == 0)
            _stream = NULL;
        else {
            _stream = new std::ifstream(filename);
            if (!_stream->is_open()) {
                int err = errno;
                delete _stream;
                _stream = NULL;
                throw PMGDException(LoaderOpenFailed, err, filename);
            }
        }
    }

    ~input_t() { close(); }

    void close()
    {
        if (_stream != NULL) {
            _stream->close();
            delete _stream;
            _stream = NULL;
        }
    }
};

static Node *get_node(Graph &db, long long id, PMGD::StringID tag,
                        std::function<void(Node &)> node_func)
{
    NodeIterator nodes = db.get_nodes(0,
            PropertyPredicate(ID, PropertyPredicate::Eq, id));
    if (nodes) return &*nodes;

    Node &node = db.add_node(tag);
    node.set_property(ID, id);
    if (node_func)
        node_func(node);
    return &node;
}

static Edge *get_edge(Graph &db, long long id,
        long long src_id, long long dst_id,
        PMGD::StringID tag,
        std::function<void(Node &)> node_func,
        std::function<void(Edge &)> edge_func)
{
    EdgeIterator edges = db.get_edges(0,
            PropertyPredicate(ID, PropertyPredicate::Eq, id));

    if (edges) return &static_cast<Edge &>(*edges);

    Node *src = get_node(db, src_id, 0, node_func);
    Node *dst = get_node(db, dst_id, 0, node_func);

    Edge &edge = db.add_edge(*src, *dst, tag);
    edge.set_property(ID, id);
    if (edge_func)
        edge_func(edge);
    return &edge;
}

/* GraphSON Parsing */
static int get_int_value(Json::Value &obj, const char *key_str, bool remove)
{
    Json::Value key_obj = obj[key_str];

    int val;
    if (key_obj.type() == Json::intValue)
        val = key_obj.asInt();
    else {
        assert(key_obj.type() == Json::stringValue);
        val = stoi(key_obj.asString(), NULL);
    }

    return val;
}

static std::string
get_string_value(Json::Value &obj, const char *key_str, bool remove)
{
    Json::Value key_obj = obj[key_str];
    assert(key_obj.type() == Json::stringValue);
    std::string val = key_obj.asString();
    return val;
}

template <typename T>
static void set_property(T *elem, const char *pkey, Json::Value &pval)
{
    switch(pval.type())
    {
        case Json::nullValue:
            elem->set_property(pkey, Property());
            break;
        case Json::intValue:
            elem->set_property(pkey, pval.asInt());
            break;
        case Json::uintValue:
            elem->set_property(pkey, (long long)pval.asUInt());
            break;
        case Json::realValue:
            elem->set_property(pkey, pval.asDouble());
            break;
        case Json::stringValue:
            {
                // First test if the string complies with time format
                // Assuming the very specific format that Time property
                // supports as of now
                struct tm tm;
                int hr, min;
                if (string_to_tm(pval.asString(), &tm, &hr, &min)) {
                    // It parsed as a time string
                    Time t(&tm, hr, min);
                    elem->set_property(pkey, t);
                }
                else
                    elem->set_property(pkey, pval.asString());
            }
            break;
        case Json::booleanValue:
            elem->set_property(pkey, pval.asBool());
            break;
        default:
           break;
    }
}

template <typename T>
static void set_properties(T *elem, Json::Value &jnode)
{
    Json::Value::Members members(jnode.getMemberNames());
    for(Json::Value::Members::iterator it = members.begin();
        it != members.end();
        ++it )
    {
        const std::string &pkey = *it;
        if (pkey[0] != '_')
            set_property(elem, pkey.c_str(), jnode[pkey]);
    }
}

static void load_nodes(Graph &db,
        Json::Value &jnodes,
        std::function<void(Node &)> node_func,
        std::function<void(Edge &)> edge_func)
{
    for (unsigned int i=0; i < jnodes.size(); i++) {
        Json::Value jnode = jnodes[i];

        int id = get_int_value(jnode, "_id", true);
        std::string label = get_string_value(jnode, "_label", true);

        Transaction tx(db, Transaction::ReadWrite);
        Node *node = get_node(db, id, label.c_str(), node_func);
        set_properties(node, jnode);
        tx.commit();
    }
}

static int load_edges(Graph &db,
        Json::Value &jedges,
        std::function<void(Node &)> node_func,
        std::function<void(Edge &)> edge_func)
{
    for (unsigned int i=0; i < jedges.size(); i++) {
        Json::Value jedge = jedges[i];

        int id = get_int_value(jedge, "_id", true);
        int inv = get_int_value(jedge, "_inV", true);
        int outv = get_int_value(jedge, "_outV", true);
        std::string label = get_string_value(jedge, "_label", true);

        Transaction tx(db, Transaction::ReadWrite);
        Edge *edge = get_edge(
                db, id, outv, inv, label.c_str(), node_func, edge_func);
        set_properties(edge, jedge);
        tx.commit();
    }

    return 0;
}

static void load_gson(Graph &db,
                Json::Value &root,
                std::function<void(Node &)> node_func,
                std::function<void(Edge &)> edge_func)
{
    Json::Value jgraph = root["graph"];
    if (jgraph.type() != Json::objectValue) {
        throw PMGDException(LoaderFormatError, "graph not found");
    }

    Json::Value jmode = jgraph["mode"];
    if (jmode.type() != Json::stringValue) {
        throw PMGDException(LoaderFormatError, "mode not found");
    }
    if (jmode.asString().compare("NORMAL")) {
        throw PMGDException(LoaderFormatError, "mode not supported");
    }

    Json::Value jnodes = jgraph["vertices"];
    if (jnodes.type() != Json::arrayValue) {
        throw PMGDException(LoaderFormatError, "nodes not found");
    }
    Transaction tx(db, Transaction::ReadWrite);
    ID = StringID(ID_STR);
    db.create_index(Graph::NodeIndex, 0, ID_STR, PropertyType::Integer);
    db.create_index(Graph::EdgeIndex, 0, ID_STR, PropertyType::Integer);
    tx.commit();
    load_nodes(db, jnodes, node_func, edge_func);

    Json::Value jedges = jgraph["edges"];
    if (jedges.type() != Json::arrayValue) {
        throw PMGDException(LoaderFormatError, "edges not found");
    }
    load_edges(db, jedges, node_func, edge_func);
}

void load_gson(Graph &db, const char *filename,
                std::function<void(Node &)> node_func,
                std::function<void(Edge &)> edge_func)
{
    input_t input(filename);

    Json::Value root;
    const Json::Features features;
    Json::Reader reader(features);

    if (!reader.parse(input, root))
        throw PMGDException(LoaderParseError);

    input.close();

    load_gson(db, root, node_func, edge_func);
}
