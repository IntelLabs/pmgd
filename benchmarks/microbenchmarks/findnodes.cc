/**
 * Find nodes with a certain keyword.
 *
 * Look for all nodes with given TAG and with the given PROPERTY
 * containing the given KEYWORD.
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
static const char prog_name[] = "findnodes";

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH TAG PROPERTY KEYWORD\n";
    o << "\n";
    o << "Look for all nodes with the given TAG and with the given PROPERTY\n";
    o << "containing the given KEYWORD.\n";
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
    char *tag_name;
    char *property_name;
    char *keyword;

    int num_nodes = 0;

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
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No tag\n";
        print_usage(cerr);
        return 1;
    }
    tag_name = argv[argi];
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No property\n";
        print_usage(cerr);
        return 1;
    }
    property_name = argv[argi];
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No keyword\n";
        print_usage(cerr);
        return 1;
    }
    keyword = argv[argi];

    try {
        // Open the graph.
        Graph db(graph_name, Graph::ReadOnly);

        // Start a transaction.
        Transaction tx(db);

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // Generate needed StringIDs.
        const StringID tag_sid(strcmp(tag_name, "0") == 0 ? 0 : tag_name);
        const StringID property_sid(property_name);

        // Search for a keyword in given property of nodes of given tag.
        for (NodeIterator n = db.get_nodes(tag_sid); n; n.next()) {
            Property p;
            if (n->check_property(property_sid, p)
                && p.type() == PropertyType::String
                && p.string_value().find(keyword) != string::npos) {

                num_nodes++;
            }
        }

        // Time and usage measurements.
        (void)gettimeofday(&t2, NULL);
        (void)getrusage(RUSAGE_SELF, &u2);
    }
    catch (Exception e) {
        if (e.num != ReadOnly) {
            print_exception(e, stderr);
            return 2;
        }
    }

    cout << num_nodes << "\n";

    if (timing) print_delta(cout, t1, t2, u1, u2);

    return 0;
}
