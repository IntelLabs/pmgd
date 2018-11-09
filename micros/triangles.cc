/**
 * Counting, and optionally listing, triangles in a graph
 *
 * This application counts, and optionally lists, the number of
 * triangles in a given graph.  When the counting has completed, it
 * prints the number of triangles found.  If the '-v' command line
 * argument is present, the application also lists the triangles found
 * (one triangle per line, tab-separated identifiers).
 *
 * Based on an algorithm described here
 *
 *   Thomas Schank and Dorothea Wagner.  Finding, Counting and Listing
 *   All Triangles in Large Graphs, an Experimental Study.  Technical
 *   report.  University of Karlsruhe, Germany, 2005.
 *   http://i11www.iti.uni-karlsruhe.de/extra/publications/sw-fclt-05_t.pdf
 *
 * and here
 *
 *   Thomas Schank and Dorothea Wagner.  Finding, Counting and Listing
 *   All Triangles in Large Graphs, an Experimental Study.  In
 *   Proceedings of the Fourth International Conference on
 *   Experimental and Efficient Algorithms, pp. 606-609, Santorini
 *   Island, Greece, 2005.
 */

#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <string.h>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "time.h"

using namespace std;
using namespace PMGD;

/**
 * Program name
 */
char *prog_name = NULL;

/*
 * Graph property names
 */
static const char ID[] = "pmgd.loader.id";

/**
 * Set of node IDs
 *
 * Set expected to be small on average.
 */
typedef set<Node *> id_set_t;

/**
 * Auxiliary structure used for sorting nodes by their degrees
 */
class DegreeMap {
public:
    struct Degree {
        Node *node;
        long long id;  // For compatibility with Java version
        long long degree;
        id_set_t id_set;

        /**
         * Sort by the degree first and then by id
         * @param d instance of Degree to compare against this
         * @return true if this is "smaller"
         * Higher degree comes first and if there is a tie, the lower id
         * wins.
         */
        bool operator <(const Degree &d) const
        {
            return degree > d.degree
                || (degree == d.degree && id < d.id);
        }
    };

private:
    struct set_sort {
        bool operator()(Degree *const &d1, Degree *const &d2)
            { return *d1 < *d2; }
    };

    typedef std::map<const long long, Degree> M;
    M _node_map;
    std::set<Degree *, set_sort> _degree_set;

public:
    void add(Node *n, long long id, long long d)
    {
        M::iterator mi = _node_map.insert(M::value_type(id, Degree{n, id, d})).first;
        _degree_set.insert(&mi->second);
    }

    std::set<Degree *>::const_iterator begin() const
    {
        return _degree_set.begin();
    }

    std::set<Degree *>::const_iterator end() const
    {
        return _degree_set.end();
    }

    Degree &operator[](long long id) { return _node_map[id]; }
};

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH\n";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -v  be verbose and output the triangles\n";
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
    bool verbose = false;
    long long num_nodes = 0;
    long long num_triangles = 0;

    prog_name = argv[0];
    char *graph_name;

    bool timing = false;
    struct timeval t1, t2;
    struct rusage u1, u2;

    // Parse the command line arguments
    int argi = 1;
    while (argi < argc && argv[argi][0] == '-') {
        if (strcmp(argv[argi], "-h") == 0) {
            print_usage(cout);
            return 0;
        }
        else if (strcmp(argv[argi], "-v") == 0) {
            verbose = true;
        }
        else if (strcmp(argv[argi], "-t") == 0) {
            timing = true;
        }
        else {
            cerr << prog_name << ": " << argv[argi] << ": Unrecognized option\n";
            return 1;
        }
        ++argi;
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

        // StringID for node id.
        StringID id_sid("pmgd.loader.id");

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // The degree of each node.
        DegreeMap degrees;
        for (NodeIterator n = db.get_nodes(); n; n.next()) {
            ++num_nodes;
            long long degree = 0;
            for (EdgeIterator e = n->get_edges(); e; e.next())
                ++degree;
            degrees.add(&*n, db.get_id(*n), degree);
        }

        // Fill a vector with id and degree information and sort it.
        //
        // While perhaps not apparent upon first reading the algorithm
        // described in Schank and Wagner's paper, it appears that the
        // nodes in the graph must not be compared according their IDs
        // but rather by their positions in the sorted list.

        // Find the triangles.
        for (const auto &s : degrees) {
            for (NodeIterator ti = get_neighbors(*s->node); ti; ti.next()) {
                auto &t = degrees[db.get_id(*ti)];
                if (*s < t) {
                    for (auto n : s->id_set) {
                        if (t.id_set.find(n) != t.id_set.end()) {
                            ++num_triangles;
                            if (verbose) {
                                cout << n->get_property(id_sid).int_value()
                                    << "\t"
                                    << s->node->get_property(id_sid).int_value()
                                    << "\t"
                                    << t.node->get_property(id_sid).int_value()
                                    << "\n";
                            }
                        }
                    }
                    t.id_set.insert(s->node);
                }
            }
        }

        // Time and usage measurements.
        (void)gettimeofday(&t2, NULL);
        (void)getrusage(RUSAGE_SELF, &u2);
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 2;
    }

    cout << num_triangles << "\n";

    if (timing) print_delta(cout, t1, t2, u1, u2);

    return 0;
}
