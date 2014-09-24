/**
 * Dump the content of a graphstore to standard output
 */

#include <string.h>
#include <stdio.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

void print_usage(FILE *stream);

int main(int argc, char **argv)
{
    bool recover = false;
    int argi = 1;

    if (argi < argc && strcmp(argv[argi], "-h") == 0) {
        print_usage(stdout);
        return 0;
    }

    if (argi < argc && strcmp(argv[argi], "-r") == 0) {
        recover = true;
        argi++;
    }

    if (!(argi < argc)) {
        print_usage(stdout);
        return 1;
    }

    const char *db_name = argv[argi];

    try {
        Graph db(db_name, recover ? Graph::ReadWrite : Graph::ReadOnly);
        Transaction tx(db);
        dump_nodes(db);
        dump_edges(db);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }

    return 0;
}

void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: dumpgraph [-h] [-r] GRAPHSTORE\n");
    fprintf(stream, "Dump the content of GRAPHSTORE.\n");
    fprintf(stream, "\n");
    fprintf(stream, "  -h  print this help and exit\n");
    fprintf(stream, "  -r  open the graph read/write, so recovery can be performed if necessary\n");
}
