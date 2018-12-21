/**
 * @file   mkgraph.cc
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

/**
 * Create an empty graphstore and specified indices
 */

#include <vector>
#include <string.h>
#include <stdio.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

struct IndexSpecification {
    Graph::IndexType _index_type;
    char *_tag;
    bool _tag_is_zero;
    char *_property_id;
    PropertyType _type;

    IndexSpecification(Graph::IndexType index_type, char *tag, char *pid, PropertyType type)
    {
        _index_type = index_type;
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
        else if (check_arg(argv[argi], 'A', "allocator-size"))
            config.allocator_region_size = strtoull(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'c', "num-allocators"))
            config.num_allocators = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 'B', "node-locks-region-size"))
            config.node_striped_lock_size = strtoul(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'F', "edge-locks-region-size"))
            config.edge_striped_lock_size = strtoul(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'I', "index-locks-region-size"))
            config.index_striped_lock_size = strtoul(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'D', "default-locks-region-size"))
            config.default_striped_lock_size = strtoul(argv[argi+1], 0, 16);
        else if (check_arg(argv[argi], 'b', "node-locks-stripe-width"))
            config.node_stripe_width = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 'f', "edge-locks-stripe-width"))
            config.edge_stripe_width = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 'i', "index-locks-stripe-width"))
            config.index_stripe_width = strtoul(argv[argi+1], 0, 0);
        else if (check_arg(argv[argi], 'd', "default-locks-stripe-width"))
            config.default_stripe_width = strtoul(argv[argi+1], 0, 0);
        else
            break;
        argi += 2;
    }

    // Index specifications
    while (argi + 3 < argc) {
        Graph::IndexType index_type;
        if (strcmp(argv[argi], "node") == 0)
            index_type = Graph::NodeIndex;
        else if (strcmp(argv[argi + 0], "edge") == 0)
            index_type = Graph::EdgeIndex;
        else {
            fprintf(stderr, "mkgraph: Unrecognized index type, expected either 'node' or 'edge'\n");
            return 1;
        }

        PropertyType type;
        if (strcmp(argv[argi + 3], "integer") == 0)
            type = PropertyType::Integer;
        else if (strcmp(argv[argi + 3], "string") == 0)
            type = PropertyType::String;
        else if (strcmp(argv[argi + 3], "float") == 0)
            type = PropertyType::Float;
        else if (strcmp(argv[argi + 3], "time") == 0)
            type = PropertyType::Time;
        else if (strcmp(argv[argi + 3], "boolean") == 0)
            type = PropertyType::Boolean;
        else {
            fprintf(stderr, "mkgraph: Unrecognized property type\n");
            return 1;
        }

        specs.push_back(IndexSpecification(index_type,
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
            db.create_index(i._index_type, i._tag_is_zero ? 0 : i._tag,
                            i._property_id, i._type);
            tx.commit();
        }
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 1;
    }
    catch (std::runtime_error e) {
        fprintf(stderr, "[Exception] %s\n", e.what());
        return 1;
    };

    return 0;
}

void print_usage(FILE *stream)
{
    fprintf(stream,
"Usage: mkgraph -h\n"
"       mkgraph GRAPHSTORE [CONFIG-OPTION]... [INDEX-SPECIFICATION]...\n"
"\n"
"Create a GRAPHSTORE and specified INDICES. GRAPHSTORE must not already\n"
"exist.\n"
"\n"
"  -h  print this help and exit\n"
"\n"
"  -n, --node-size SIZE\n"
"          Set the size of nodes in the graph. The value must be a power\n"
"          of two not less than 64 (which is the default). Setting a larger\n"
"          value allows more property values to be stored in the node.\n"
"\n"
"  -e, --edge-size SIZE\n"
"          Set the size of edges in the graph. The value must be a power\n"
"          of two not less than 32 (which is the default). Setting a larger\n"
"          value allows more property values to be stored in the edge.\n"
"\n"
"  -N, --node-table-size SIZE\n"
"          Set the size of the node table. This controls the maximum number\n"
"          of nodes that can be created in the graph. The default is the\n"
"          value of the REGION-SIZE parameter. SIZE is assumed to be in\n"
"          hexadecimal.\n"
"\n"
"  -E, --edge-table-size SIZE\n"
"          Set the size of the edge table. This controls the maximum number\n"
"          of edges that can be created in the graph. The default is the\n"
"          value of the REGION-SIZE parameter. SIZE is assumed to be in\n"
"          hexadecimal.\n"
"\n"
"  -R, --region-size REGION-SIZE\n"
"          Set the default size of regions in the graph. This applies\n"
"          to the node table, edge table, and each of the allocators, if\n"
"          a different value is not specified for them. The default is\n"
"          1 terabyte. SIZE is assumed to be in hexadecimal.\n"
"\n"
"  -s, --stringid-length LENGTH\n"
"          Set the maximum length of strings used for node and edge tags\n"
"          and property names. The value must be a power of two. The default\n"
"          is 16 bytes. Setting this to a larger value will decrease the\n"
"          number of strings that can be used, unless string-table-size is\n"
"          also increased.\n"
"\n"
"  -S, --string-table-size SIZE\n"
"          Set the size of the string table used for node and edge tags\n"
"          and property names. This controls the number of unique strings\n"
"          that can be used for tags and property names and also the size\n"
"          of the hash function used. The value must be a power of two. The\n"
"          default is 64KB. SIZE is assumed to be in hexadecimal.\n"
"\n"
"  -A, --allocator-size POOL-SIZE\n"
"          Set the size of the pool to be used for each allocator. The\n"
"          default is REGION-SIZE. This option cannot be used with -a or\n"
"          --allocator. POOL-SIZE is assumed to be in hexadecimal.\n"
"\n"
"  -a, --allocator OBJECT-SIZE POOL-SIZE\n"
"          Set the size of the pool to be used for allocations of the\n"
"          specified object size. This option must be repeated for each\n"
"          object size. If this option is specified one or more times,\n"
"          allocators will be created *only* for the object sizes specified.\n"
"          The default object sizes are 16, 32, 64, and 128. The default\n"
"          pool size is REGION-SIZE. This option cannot be used with -A\n"
"          or --allocator-size. POOL-SIZE is assumed to be in hexadecimal.\n"
"\n"
"  -c, --num-allocators NUMBER-PARALLEL-ALLOCATORS\n"
"          Set the number of allocators that will operate on given pool.\n"
"          This determines how many transactions can potentially allocate in\n"
"          parallel. Default is 1.\n"
"\n"
"  -B, --node-locks-region-size SIZE\n"
"          Set the size of the region in DRAM for locking nodes. This controls\n"
"          the maximum number of locks that will be created to lock the nodes\n"
"          allocated from the entire node table.\n"
"          The default is the value of the\n"
"          --default-locks-region-size parameter. SIZE is assumed to be in\n"
"          hexadecimal.\n"
"\n"
"  -F, --edge-locks-region-size SIZE\n"
"          Set the size of the region in DRAM for locking edges. This controls\n"
"          the maximum number of locks that will be created to lock the edges\n"
"          allocated from the entire edge table.\n"
"          The default is the value of the\n"
"          --default-locks-region-size parameter. SIZE is assumed to be in\n"
"          hexadecimal.\n"
"\n"
"  -I, --index-locks-region-size SIZE\n"
"          Set the size of the region in DRAM for locking index structures. This controls\n"
"          the maximum number of locks that will be created to lock all the indices.\n"
"          Advisable to keep it large.\n"
"          The default is the value of the\n"
"          --default-locks-region-size parameter. SIZE is assumed to be in\n"
"          hexadecimal.\n"
"\n"
"  -D, --default-locks-region-size SIZE\n"
"          This is used for any lock region not given. SIZE is assumed to be in\n"
"          hexadecimal.\n"
"\n"
"  -b, --node-locks-stripe-width BYTES\n"
"          Set the size of node table object/area covered by one lock.\n"
"          The default is the value of the\n"
"          --default-locks-stripe-width parameter.\n"
"\n"
"  -f, --edge-locks-stripe-width BYTES\n"
"          Set the size of edge table object/area covered by one lock.\n"
"          The default is the value of the\n"
"          --default-locks-stripe-width parameter.\n"
"\n"
"  -i, --index-locks-stripe-width BYTES\n"
"          Set the size of index object/area covered by one lock\n"
"          The default is the value of the\n"
"          --default-locks-stripe-width parameter.\n"
"\n"
"  -d, --default-locks-stripe-width BYTES\n"
"          Set the default value of object/area covered by one lock\n"
"          This is used whenever other values are not specified.\n"
"\n"
"  -l, --locale LOCALE-NAME\n"
"          Set the name of the locale to be used for ordering strings in\n"
"          indices. The locale must be available each time the graph is\n"
"          opened.\n"
"\n"
"Each INDEX-SPECIFICATION consists of a quadruplet:\n"
"    'node' | 'edge'\n"
"    TAG\n"
"    PROPERTY-IDENTIFIER\n"
"    TYPE\n"
"where\n"
"    TAG is '0' or a string\n"
"    PROPERTY-IDENTIFIER is a string\n"
"    TYPE is one of 'boolean', 'integer', 'string', 'float', or 'time'\n"
"\n"
"Examples:\n"
"mkgraph p2p-Gnutella04\n"
"mkgraph p2p-Gnutella04 --region-size 80000000\n"
"mkgraph p2p-Gnutella04 node 0 id integer\n"
"mkgraph p2p-Gnutella04 node 0 id integer node 0 degree integer\n");
}
