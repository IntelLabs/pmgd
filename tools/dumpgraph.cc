/**
 * @file   dumpgraph.cc
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
 * Dump the content of a graphstore to standard output
 */

#include <string.h>
#include <stdio.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

void print_usage(FILE *stream);

int main(int argc, char **argv)
{
    bool recover = false;
    enum { DEBUG, GEXF, JTXT };
    int type = DEBUG;
    int argi = 1;

    while (argi < argc && argv[argi][0] == '-') {
        switch (argv[argi][1]) {
            case 'h':
                print_usage(stdout);
                return 0;

            case 'r':
                recover = true;
                break;

            case 'd':
                type = DEBUG;
                break;

            case 'x':
                type = GEXF;
                break;

            case 'j':
                type = JTXT;
                break;

            default:
                fprintf(stderr, "dumpgraph: %s: Unrecognized option\n", argv[argi]);
                print_usage(stderr);
                return 1;
        }
        argi++;
    }

    if (!(argi < argc)) {
        fprintf(stderr, "dumpgraph: No graphstore specified\n");
        print_usage(stderr);
        return 1;
    }

    const char *db_name = argv[argi];

    try {
        Graph db(db_name, recover ? Graph::ReadWrite : Graph::ReadOnly);
        Transaction tx(db);
        switch (type) {
            case DEBUG: dump_debug(db); break;
            case GEXF: dump_gexf(db); break;
            case JTXT: dump_pmgd(db); break;
        }
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 1;
    }

    return 0;
}

void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: dumpgraph [OPTION]... GRAPHSTORE\n");
    fprintf(stream, "Dump the content of GRAPHSTORE.\n");
    fprintf(stream, "\n");
    fprintf(stream, "  -h  print this help and exit\n");
    fprintf(stream, "  -r  open the graph read/write, so recovery can be performed if necessary\n");
    fprintf(stream, "  -d  debug mode: list all nodes then all edges (default)\n");
    fprintf(stream, "  -x  dump in the GEXF file format\n");
    fprintf(stream, "  -j  dump in the PMGD graph text format\n");
}
