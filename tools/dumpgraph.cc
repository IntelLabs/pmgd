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
            case JTXT: dump_jarvis(db); break;
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
    fprintf(stream, "  -j  dump in the Jarvis Lake graph text format\n");
}
