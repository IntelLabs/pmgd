/**
 * Compute the degree of all nodes in a graph and print the maximum.
 *
 * The direction of edges is ignored.
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "pmgd.h"
#include "util.h"
#include "time.h"

using namespace std;
using namespace PMGD;

/**
 * Program name.
 */
static const char prog_name[] = "maxdegree";

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH\n";
    o << "\n";
    o << "Find the node in GRAPH with the maximum degree.\n";
    o << "(GRAPH is treated as undirected.)";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -t  print elapsed time and usage\n";
}

/**
 * The application's main function
 * @param argc the number of command line arguments
 * @param argv the array of command line arguments
 * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
 */
int main(int argc, char *argv[])
{
    bool timing = false;
    struct timeval t1, t2;
    struct rusage u1, u2;

    char *graph_name;

    long long max_degree;

    // Parse the command line arguments
    int argi = 1;
    while (argi < argc && argv[argi][0] == '-') {
        if (strcmp(argv[argi], "-h") == 0) {
            print_usage(cout);
            return 0;
        }
        else if (strcmp(argv[argi], "-t") == 0) {
            timing = true;
        }
        else {
            fprintf(stderr, "%s: %s: Unrecognized option\n", prog_name, argv[argi]);
            return 1;
        }
        argi += 1;
    }
    if (argi + 1 > argc) {
        cerr << prog_name << ": No input graph\n";
        print_usage(cerr);
        return 1;
    }
    graph_name = argv[argi];

    try {
        // Open the graph.
        Graph db(graph_name, Graph::ReadOnly);

        // Start a transaction.
        Transaction tx(db);

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // Compute the degree of each node and compute the maximum.
        max_degree = -1;
        for (NodeIterator n = db.get_nodes(); n; n.next()) {
            long long degree = 0;
            for (EdgeIterator e = n->get_edges(); e; e.next())
                ++degree;
            if (degree > max_degree) max_degree = degree;
        }

        // Time and measurements.
        (void)gettimeofday(&t2, NULL);
        (void)getrusage(RUSAGE_SELF, &u2);
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 2;
    }

    cout << max_degree << "\n";

    if (timing) print_delta(cout, t1, t2, u1, u2);

    return 0;
}
