/**
 * @file   setproperty.cc
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
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

static const char id_name[] = "pmgd.loader.id";

static void usage()
{
    fprintf(stderr, "Usage: setproperty [-d] <graph-name>\n"
                    "           { node <node-id> | edge <edge-id> }\n"
                    "           [ <property> <value> ]\n");
    exit(1);
}

static Property property_value(const char *s);

int main(int argc, char **argv)
{
    bool dump_graph = false;
    int argi = 1;

    if (argi < argc && strcmp(argv[argi], "-d") == 0) {
        dump_graph = true;
        argi++;
    }

    if (argi + 3 > argc)
        usage();

    const char *db_name = argv[argi++];
    const char *node_or_edge = argv[argi++];
    const char *id = argv[argi++];

    bool use_node;
    if (strcmp(node_or_edge, "node") == 0)
        use_node = true;
    else if (strcmp(node_or_edge, "edge") == 0)
        use_node = false;
    else
        usage();

    try {
        Graph db(db_name, Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);

        NodeRef *node = NULL;
        EdgeRef *edge = NULL;
        if (use_node)
            node = &*db.get_nodes()
                .filter([id](const NodeRef &n) {
                    Property p = n.get_property(id_name);
                    return (p.type() == PropertyType::String && p.string_value() == id
                            || p.type() == PropertyType::Integer && p.int_value() == strtoll(id, 0, 10))
                        ? PassStop : DontPass;
                });
        else
            edge = &*db.get_edges()
                .filter([id](const EdgeRef &e) {
                    Property p = e.get_property(id_name);
                    return (p.type() == PropertyType::String && p.string_value() == id
                            || p.type() == PropertyType::Integer && p.int_value() == strtoll(id, 0, 10))
                        ? PassStop : DontPass;
                });

        if (argi + 2 <= argc) {
            const char *property = argv[argi++];
            const char *value = argv[argi++];
            Property p = property_value(value);
            printf("set %s = %s\n", property, property_text(p).c_str());

            if (use_node)
                node->set_property(property, p);
            else
                edge->set_property(property, p);
        }
        else if (argi < argc) {
            const char *property = argv[argi++];
            printf("remove %s\n", property);

            if (use_node)
                node->remove_property(property);
            else
                edge->remove_property(property);
        }
        else {
            char property[17], value[40];
            while (scanf("%17s = %40[^\n]", property, value) == 2) {
                Property p = property_value(value);
                printf("set %s = %s\n", property, property_text(p).c_str());
                if (use_node)
                    node->set_property(property, p);
                else
                    edge->set_property(property, p);
            }
        }

        tx.commit();

        if (dump_graph) {
            Transaction tx(db);
            dump_debug(db);
        }
    }
    catch (Exception e) {
        if (e.num == NullIterator) {
            printf("%s %s not found.\n"
                   "%ss are identified by a property named \"%s\",\n"
                   "which may be an integer or a string.\n",
                   use_node ? "Node" : "Edge", id,
                   use_node ? "Node" : "Edge", id_name);
        }
        else if (e.num == PropertyNotFound) {
            printf("Some %s did not have an \"%s\" property.\n"
                   "%ss are identified by a property named \"%s\",\n"
                   "which may be an integer or a string.\n",
                   use_node ? "node" : "edge", id_name,
                   use_node ? "Node" : "Edge", id_name);
        }
        else {
            print_exception(e);
        }
        return 1;
    }

    return 0;
}

static Property property_value(const char *s)
{
    if ((s[0] >= '0' && s[0] <= '9') || s[0] == '-')
        return strtoll(s, NULL, 10);
    else if (strcmp(s, "T") == 0)
        return true;
    else if (strcmp(s, "F") == 0)
        return false;
    else if (strcmp(s, "no value") == 0)
        return Property();
    else
        return s;
}
