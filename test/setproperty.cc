#include <stdio.h>
#include <string.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

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

        Transaction tx(db);

        NodeRef *node;
        EdgeRef *edge;
        if (use_node)
            node = &*db.get_nodes()
                .filter([id](const NodeRef &n) {
                    Property p = n.get_property("id");
                    return (p.type() == t_string && p.string_value() == id
                            || p.type() == t_integer && p.int_value() == strtoll(id, 0, 10))
                        ? pass_stop : dont_pass;
                });
        else
            edge = &*db.get_edges()
                .filter([id](const EdgeRef &e) {
                    Property p = e.get_property("id");
                    return (p.type() == t_string && p.string_value() == id
                            || p.type() == t_integer && p.int_value() == strtoll(id, 0, 10))
                        ? pass_stop : dont_pass;
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
            dump_nodes(db);
            dump_edges(db);
        }
    }
    catch (Exception e) {
        print_exception(e);
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
