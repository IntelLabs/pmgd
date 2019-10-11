/**
 * Compute the shortest path between two points of a graph.
 *
 * Assumes unit weight on edges and ignores direction.
 *
 * Uses Dijkstra's algorithm.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <map>
#include <unordered_set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "time.h"

using namespace std;
using namespace PMGD;

/**
 * Program name.
 */
static const char prog_name[] = "shortestpath";

/**
 * Graph's infinite distance.
 */
static const long long infinity = LLONG_MAX;

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH START END\n";
    o << "\n";
    o << "Calculate the shortest path between START and END of GRAPH\n";
    o << "and output the length of the path (i.e., number of hops).\n";
    o << "Assumes unit weight on edges and ignores direction.";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -t  print elapsed time and usage\n";
    o << "  -v  show one shortest path\n";
}

static long long shortestpath(Graph &db, Node &start, Node &end)
{
    if (&start == &end)
        return 0;

    long long distance = 1;
    unordered_set<Node *> actives({&start});
    unordered_set<Node *> encountered({&start});

    while (1) {
        unordered_set<Node *> new_actives;
        for (auto n = actives.begin(); n != actives.end(); ++n)
            for (NodeIterator neighbors = get_neighbors(**n, false);
                 neighbors;
                 neighbors.next())
            {
                Node &neighbor = *neighbors;
                if (neighbor == end)
                    return distance;
                if (encountered.insert(&neighbor).second)
                    new_actives.insert(&neighbor);
            }

        if (new_actives.empty())
            return infinity;
        distance++;
        actives = move(new_actives);
    }
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

    long long distance;

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
    if (argi + 3 > argc) {
        cerr << prog_name << ": Missing input parameters\n";
        print_usage(cerr);
        return 1;
    }
    char *graph_name = argv[argi];
    long long startid = atoll(argv[argi + 1]);
    long long endid = atoll(argv[argi + 2]);

    try {
        // Open the graph.
        Graph db(graph_name, Graph::ReadOnly);

        // Start a transaction.
        Transaction tx(db);

        // StringID for node id
        StringID id_sid("pmgd.loader.id");

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // Find start and end nodes.
        Node &start = *db.get_nodes(0, PropertyPredicate(id_sid,
                                                         PropertyPredicate::Eq,
                                                         startid));
        Node &end = *db.get_nodes(0, PropertyPredicate(id_sid,
                                                       PropertyPredicate::Eq,
                                                       endid));

        // Find the shortest path.
        distance = shortestpath(db, start, end);

        // Time and usage measurements.
        (void)gettimeofday(&t2, NULL);
        (void)getrusage(RUSAGE_SELF, &u2);
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 2;
    }

    if (distance == infinity)
        cout << "Infinity\n";
    else
        cout << distance << "\n";

    if (timing) print_delta(cout, t1, t2, u1, u2);

    return 0;
}
