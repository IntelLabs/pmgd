#include <stdio.h>
#include <string.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    bool dump_graph = false;
    int argi = 1;

    if (argi < argc && strcmp(argv[argi], "-d") == 0) {
        dump_graph = true;
        argi++;
    }

    if (argi + 2 > argc) {
        fprintf(stderr, "Usage: loadgraph [-d] <graph-name> <file-name>\n");
        return 1;
    }

    const char *db_name = argv[argi];
    const char *file_name = argv[argi+1];

    try {
        Graph db(db_name, Graph::Create);
        load(db, file_name);

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
