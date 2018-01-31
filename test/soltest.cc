/**
 * @file   soltest.cc
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

/*
 * This test checks PMGD signs of life.
 */

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pmgd.h"

using namespace PMGD;

std::string property_text(const Property &p)
{
    switch (p.type()) {
        case PropertyType::NoValue: return "no value";
        case PropertyType::Boolean: return p.bool_value() ? "T" : "F";
        case PropertyType::Integer: return std::to_string(p.int_value());
        case PropertyType::String: return p.string_value();
        case PropertyType::Float: return std::to_string(p.float_value());
        case PropertyType::Time: return "<time value>";
        case PropertyType::Blob: return "<blob value>";
        default: throw PMGDException(PropertyTypeInvalid);
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

void dump(const Node &n)
{
    printf("Node %" PRIx64 " %s:\n", n.get_id(), tag_text(n).c_str());
    n.get_properties()
        .process([](PropertyRef &p) {
        printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
    n.get_edges(Outgoing)
        .process([](EdgeRef &e) {
        printf(" %s -> n%" PRIx64 " (e%" PRIx64 ")\n", tag_text(e).c_str(),
            e.get_destination().get_id(), e.get_id());
        });
    n.get_edges(Incoming)
        .process([](EdgeRef &e) {
        printf(" %s <- n%" PRIx64 " (e%" PRIx64 ")\n", tag_text(e).c_str(),
            e.get_source().get_id(), e.get_id());
        });
}

void dump(const Edge &e)
{
    printf("Edge %" PRIx64 "%s: n%" PRIx64 " -> n%" PRIx64 "\n",
        e.get_id(), tag_text(e).c_str(),
        e.get_source().get_id(), e.get_destination().get_id());
    e.get_properties()
        .process([](PropertyRef &p) {
        printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
}

void dump(const PropertyRef &p)
{
    printf("  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
}

void dump(NodeIterator i)
{
    while (i) {
        dump(*i);
        i.next();
    }
}

void dump(EdgeIterator i)
{
    while (i) {
        dump(*i);
        i.next();
    }
}

void dump(PropertyIterator i)
{
    while (i) {
        dump(*i);
        i.next();
    }
}

void dump_nodes(Graph &db)
{
    for (NodeIterator i = db.get_nodes(); i; i.next()) {
        dump(*i);
    }
}

void dump_edges(Graph &db)
{
    for (EdgeIterator i = db.get_edges(); i; i.next()) {
        dump(*i);
    }
}

void print_exception(const Exception &e)
{
    printf("[Exception] %s at %s:%d\n", e.name, e.file, e.line);
    if (e.errno_val != 0)
        printf("%s: %s\n", e.msg.c_str(), strerror(e.errno_val));
    else if (!e.msg.empty())
        printf("%s\n", e.msg.c_str());
}

int main(int argc, char **argv)
{
    bool create = (argc > 1);

    try {
        // create graph outside transactions
        Graph db("solgraph", create ? Graph::Create : Graph::ReadOnly);

        // add nodes and edges in a transaction
        Transaction tx(db, create ? Transaction::ReadWrite : Transaction::ReadOnly);

        Node *prev = 0;
        for (int i = 1; i < argc; i++) {
            Node &n = db.add_node(0);
            n.set_property(0, argv[i]);
            if (prev != NULL)
                db.add_edge(*prev, n, 0);
            prev = &n;
        }

        dump_nodes(db);
        dump_edges(db);

        dump(db.get_nodes());
        dump(db.get_edges());
        dump(db.get_nodes()->get_properties());

        tx.commit();
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}
