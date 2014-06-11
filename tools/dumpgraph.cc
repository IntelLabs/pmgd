/*
 * This test dumps a Jarvis graph
 */

#include <stdio.h>
#include "jarvis.h"
#include "../util/util.h"

using namespace Jarvis;

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: dumpgraph <graph-name>\n");
        return 1;
    }

    const char *db_name = argv[1];

    try {
        Graph db(db_name, Graph::ReadOnly);
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
