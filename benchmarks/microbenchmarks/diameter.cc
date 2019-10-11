/**
 * Compute the diameter of a graph.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <map>
#include <unordered_set>
#include <utility>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "time.h"

using namespace std;
using namespace PMGD;

/**
 * Program name.
 */
static const char prog_name[] = "diameter";

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
    o << "Usage: " << prog_name << " [OPTION]... GRAPH\n";
    o << "\n";
    o << "Calculate the diameter of GRAPH.\n";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -t  print elapsed time and usage\n";
}

/**
 * Internal accentricity function.
 * @param  wave      Nodes "distance" away from origin
 * @param  distances Map: Node -> distance from starting node
 * @param  distance  Distance from origin of the nodes in the wave
 * @return the eccentricity of the starting node
 * The choice of parameters of this function is such that a recursive
 * implementation should be possible with only modifications to its
 * body.
 */
static long long eccentricity(const unordered_set<Node *> &start_wave,
                              map<Node *, long long> &distances,
                              long long distance)
{
    unordered_set<Node *>wave = start_wave;

    while (1) {
        unordered_set<Node *>new_wave;

        // Iterate over all current members of wave.
        for (auto n = wave.begin(); n != wave.end(); ++n)
            for (NodeIterator neighbor = get_neighbors(**n, Any);
                 neighbor;
                 neighbor.next()) {
                if (distances.find(&*neighbor) == distances.end()) {
                    distances[&*neighbor] = distance + 1;
                    new_wave.insert(&*neighbor);
                }
            }
        if (new_wave.empty())
            return distance;
        wave = std::move(new_wave);
        distance++;
    }
}

/**
 * Compute the eccentricity of the input node.
 * @param db         The graph
 * @param num_nodes  Number of nodes in the graph
 * @param n          The starting node
 * @return eccentricity of n
 */
static long long eccentricity(Graph &db, unsigned long long num_nodes, Node &n)
{
    /// Distance for each node.  Initially consists of the starting
    /// node only at distance 0.
    map<Node *, long long> distances;
    distances[&n] = 0;

    /// The set of node at the current distance.  Initially consists
    /// of the starting node with the distance of 0.
    unordered_set<Node *> wave = { &n };

    long long e = eccentricity(wave, distances, 0);

    if (distances.size() < num_nodes)
        return infinity;
    else
        return e;
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

    long long diameter;

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

        // Compute the number of nodes.
        unsigned long long num_nodes = 0;
        for (NodeIterator n = db.get_nodes(); n; n.next())
            num_nodes++;

        // Compute the eccentricity of all nodes.  The diameter is the
        // maximum eccentricity found.
        diameter = 0;
        for (NodeIterator n = db.get_nodes(); n; n.next()) {
            long long eccentricity = ::eccentricity(db, num_nodes, *n);
            if (eccentricity > diameter) diameter = eccentricity;
        }

        // Time and usage measurements.
        (void)gettimeofday(&t2, NULL);
        (void)getrusage(RUSAGE_SELF, &u2);
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 2;
    }

    if (diameter == infinity)
        cout << "Infinity\n";
    else
        cout << diameter << "\n";

    if (timing) print_delta(cout, t1, t2, u1, u2);

    return 0;
}
