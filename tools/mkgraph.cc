/**
 * Create an empty graphstore and specified indices
 */

#include <vector>
#include <string.h>
#include <stdio.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

struct IndexSpecification {
    int _node_or_edge;
    char *_tag;
    bool _tag_is_zero;
    char *_property_id;
    PropertyType _type;

    IndexSpecification(int node_or_edge, char *tag, char *pid, PropertyType type)
    {
        _node_or_edge = node_or_edge;
        _tag = tag;
        _property_id = pid;
        _type = type;
        _tag_is_zero = strcmp(_tag, "0") == 0;
    }
};
std::vector<IndexSpecification> specs;

void print_usage(FILE *stream);

bool check_arg(const char *arg, char short_name, const char *long_name)
{
    return (arg[0] == '-' && arg[1] == short_name)
        || (arg[0] == '-' && arg[1] == '-' && strcmp(&arg[2], long_name) == 0);
}

int main(int argc, char **argv)
{
    int argi = 1;

    if (argi < argc && strcmp(argv[argi], "-h") == 0) {
        print_usage(stdout);
        return 0;
    }

    if (argi + 1 > argc) {
        fprintf(stderr, "mkgraph: No graphstore specified\n");
        return 1;
    }

    const char *db_name = argv[argi];
    argi += 1;

    // Graph configuration options
    Graph::Config config;
    while (argi + 1 < argc) {
        if (check_arg(argv[argi], 'R', "region-size"))
            config.default_region_size = strtoull(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'N', "node-table-size"))
            config.node_table_size = strtoull(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'E', "edge-table-size"))
            config.edge_table_size = strtoull(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'S', "string-table-size"))
            config.string_table_size = strtoull(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'n', "node-size"))
            config.node_size = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 'e', "edge-size"))
            config.edge_size = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 's', "stringid-length"))
            config.max_stringid_length = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 'l', "locale"))
            config.locale_name = argv[argi+1];
        else if (check_arg(argv[argi], 'A', "allocator-size")) {
            uint64_t pool_size = strtoul(argv[argi+1], 0, 16);
            unsigned object_size = 16;
            for (int i = 0; i < 5; i++) {
                config.fixed_allocators.push_back(
                    Graph::Config::AllocatorInfo{ object_size, pool_size });
                object_size *= 2;
            }
        }
        else if (check_arg(argv[argi], 'a', "allocator")) {
            if (!(argi + 2 < argc)) {
                fprintf(stderr, "mkgraph: allocator option needs two values\n");
                return 1;
            }
            unsigned object_size = strtoul(argv[argi+1], 0, 0);
            uint64_t pool_size = strtoull(argv[argi+2], 0, 16);
            config.fixed_allocators.push_back(
                    Graph::Config::AllocatorInfo{ object_size, pool_size });
            argi++;
        }
        else
            break;
        argi += 2;
    }

    // Index specifications
    while (argi + 3 < argc) {
        int node_or_edge;
        if (strcmp(argv[argi], "node") == 0)
            node_or_edge = Graph::NODE;
        else if (strcmp(argv[argi + 0], "edge") == 0)
            node_or_edge = Graph::EDGE;
        else {
            fprintf(stderr, "mkgraph: Unrecognized index type, expected either 'node' or 'edge'\n");
            return 1;
        }

        PropertyType type;
        if (strcmp(argv[argi + 3], "integer") == 0)
            type = t_integer;
        else if (strcmp(argv[argi + 3], "string") == 0)
            type = t_string;
        else if (strcmp(argv[argi + 3], "float") == 0)
            type = t_float;
        else if (strcmp(argv[argi + 3], "time") == 0)
            type = t_time;
        else if (strcmp(argv[argi + 3], "boolean") == 0)
            type = t_boolean;
        else {
            fprintf(stderr, "mkgraph: Unrecognized property type\n");
            return 1;
        }
                         
        specs.push_back(IndexSpecification(node_or_edge,
                                           argv[argi + 1],
                                           argv[argi + 2],
                                           type));

        argi += 4;
    }

    if (argi < argc) {
        fprintf(stderr, "mkgraph: unrecognized option\n");
        return 1;
    }

    try {
        Graph db(db_name, Graph::Create, &config);

        for (auto i:specs) {
            Transaction tx(db, Transaction::ReadWrite);
            db.create_index(i._node_or_edge, i._tag_is_zero ? 0 : i._tag,
                            i._property_id, i._type);
            tx.commit();
        }
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    catch (std::runtime_error e) {
        printf("[Exception] %s\n", e.what());
        return 1;
    };

    return 0;
}

void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: mkgraph [OPTION]... GRAPHSTORE [INDEX-SPECIFICATION]...\n");
    fprintf(stream, "Create a GRAPHSTORE and specified INDICES.\n");
    fprintf(stream, "\n");
    fprintf(stream, "  -h  print this help and exit\n");
    fprintf(stream, "\n");
    fprintf(stream, "GRAPHSTORE must not already exist.\n");
    fprintf(stream, "\n");
    fprintf(stream, "Each INDEX-SPECIFICATION consists of a quadruplet:\n");
    fprintf(stream, "    'node' | 'edge'\n");
    fprintf(stream, "    TAG\n");
    fprintf(stream, "    PROPERTY-IDENTIFIER\n");
    fprintf(stream, "    TYPE\n");
    fprintf(stream, "where\n");
    fprintf(stream, "    TAG is '0' or a string\n");
    fprintf(stream, "    PROPERTY-IDENTIFIER is a string\n");
    fprintf(stream, "    TYPE is one of 'boolean', 'integer', 'string', 'float', or 'time'\n");
    fprintf(stream, "\n");
    fprintf(stream, "Examples:\n");
    fprintf(stream, "mkgraph p2p-Gnutella04\n");
    fprintf(stream, "mkgraph p2p-Gnutella04 node 0 id integer\n");
    fprintf(stream, "mkgraph p2p-Gnutella04 node 0 id integer node 0 degree integer\n");
}
