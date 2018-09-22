/**
 * @file   loadgraph.cc
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
 * Bulk load a graphstore from a file or standard input
 */

#include <string.h>
#include <stdio.h>
#include "pmgd.h"
#include "util.h"

using namespace PMGD;

void print_usage(FILE *stream);

int main(int argc, char **argv)
{
    bool tsv_format = false;
    bool gson_format = false;
    bool jtxt_format = false;
    bool append = false;
    bool use_index = false;
    int argi = 1;

    while (argi < argc && argv[argi][0] == '-') {
        if (strcmp(argv[argi], "-h") == 0) {
            print_usage(stdout);
            return 0;
        }
        else if (strcmp(argv[argi], "-t") == 0) {
            tsv_format = true;
            gson_format = jtxt_format = false;
        }
        else if (strcmp(argv[argi], "-g") == 0) {
            gson_format = true;
            tsv_format = jtxt_format = false;
        }
        else if (strcmp(argv[argi], "-j") == 0) {
            jtxt_format = true;
            tsv_format = gson_format = false;
        }
        else if (strcmp(argv[argi], "-a") == 0) {
            append = true;
        }
        else if (strcmp(argv[argi], "-i") == 0) {
            use_index = true;
        }
        else {
            fprintf(stderr, "loadgraph: %s: Unrecognized option\n", argv[argi]);
            return 1;
        }
        argi += 1;
    }

    if (argi + 1 > argc) {
        fprintf(stderr, "loadgraph: No graphstore specified\n");
        return 1;
    }

    const char *db_name = argv[argi];
    argi += 1;

    const char *file_name = (argi < argc) ? argv[argi] : "-";

    if (!tsv_format && !gson_format && !jtxt_format) {
        const size_t len = strlen(file_name);

        if (strcmp(file_name + len - 5, ".gson") == 0)
            gson_format = true;
        else if (strcmp(file_name + len - 5, ".jtxt") == 0)
            jtxt_format = true;
    }

    try {
        Graph db(db_name, Graph::NoMsync);

        if (!append) {
            Transaction tx(db, Transaction::ReadWrite);
            if (db.get_nodes()) {
                fprintf(stderr, "loadgraph: Graphstore is not empty\n");
                return 1;
            }
        }

        if (gson_format)
            load_gson(db, file_name);
        else if (jtxt_format)
            load(db, file_name, use_index);
        else
            load_tsv(db, file_name);
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 1;
    }

    return 0;
}

void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: loadgraph [OPTION]... GRAPHSTORE [FILE]\n");
    fprintf(stream, "Bulk load a GRAPHSTORE from a FILE or standard input.\n");
    fprintf(stream, "\n");
    fprintf(stream, "  -h  print this help and exit\n");
    fprintf(stream, "  -t  input is tab-separated integers (default)\n");
    fprintf(stream, "  -g  input is GraphSON\n");
    fprintf(stream, "  -j  input is the PMGD graph text format\n");
    fprintf(stream, "  -a  load graph into a non-empty graphstore\n");
    fprintf(stream, "  -i  add pmgd.loader.id property to every node\n");
    fprintf(stream, "\n");
    fprintf(stream, "GRAPHSTORE must already exist (e.g., using mkgraph) and is expected to\n");
    fprintf(stream, "be empty.  The loader will fail with a warning if GRAPHSTORE is not\n");
    fprintf(stream, "empty.  The user can force the loading a graph in a non-empty\n");
    fprintf(stream, "GRAPHSTORE specifying the -a option.\n");
    fprintf(stream, "\n");
    fprintf(stream, "The graph loader reads its input from FILE, if provided, or standard\n");
    fprintf(stream, "input otherwise.  The graph loader supports three input formats:\n");
    fprintf(stream, "tab-separated integers (file extension 'tsv'), GraphSON (file extension\n");
    fprintf(stream, "'gson'), or the PMGD graph text format (file extension 'jtxt').\n");
    fprintf(stream, "\n");
    fprintf(stream, "The graph loader attempts to infer the input format based on the\n");
    fprintf(stream, "extension of the supplied file.  Failing that it relies on the user\n");
    fprintf(stream, "specifying the format with the appropriate option.  Options override\n");
    fprintf(stream, "inferences based on the file extension.\n");
    fprintf(stream, "\n");
    fprintf(stream, "If the input file is tab-separated integers or GraphSON or if the -i option\n");
    fprintf(stream, "is specified, loadgraph adds a property to each node with the identifier\n");
    fprintf(stream, "'pmgd.loader.id' and it creates an index on that property. If the input\n");
    fprintf(stream, "file is PMGD format and the -i option is not specifed, loadgraph\n");
    fprintf(stream, "keeps the loader id and index in memory instead of storing it in the graph.\n");
    fprintf(stream, "This can save space in the graph when the loader id is not needed later.\n");
}
