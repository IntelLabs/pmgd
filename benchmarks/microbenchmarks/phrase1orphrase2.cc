/**
 * Find nodes with either of two given keyphrases.
 *
 * Look for all emails in GRAPH that contain either KEYPHRASE1 or
 * KEYPHRASE2.
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <set>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "time.h"

using namespace std;
using namespace PMGD;

/**
 * Program name.
 */
static const char prog_name[] = "phrase1orphrase2";

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH KEYPHRASE1 KEYPHRASE2\n";
    o << "\n";
    o << "Look for all nodes in GRAPH that contains either KEYPHRASE1\n";
    o << "or KEYPHRASE2.\n";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -t  print elapsed time and usage\n";
    o << "  -v  print the UUID of the emails found\n";
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

    bool verbose = false;

    char *graph_name;
    char *keyphrase1;
    char *keyphrase2;

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
        else if (strcmp(argv[argi], "-v") == 0) {
            verbose = true;
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
    if (argi + 2 > argc) {
        cerr << prog_name << ": No keyphrases\n";
        print_usage(cerr);
        return 1;
    }
    keyphrase1 = argv[argi];
    keyphrase2 = argv[argi + 1];

    // Number of nodes and set of nodes seen so far.
    long long uniq_nodes = 0;
    set<Node *> uniq;

    try {
        // Open the graph.
        Graph db(graph_name, Graph::ReadOnly);

        // Start a transaction.
        Transaction tx(db);

        // StringIDs
        StringID contains_sid("Contains");
        StringID keywords_sid("Keywords");
        StringID phrase_sid("Phrase");

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // Look for both keyphrases
        NodeIterator ki1 = db.get_nodes(keywords_sid,
                                        PropertyPredicate(phrase_sid,
                                                          PropertyPredicate::Eq,
                                                          keyphrase1));
        NodeIterator ki2 = db.get_nodes(keywords_sid,
                                        PropertyPredicate(phrase_sid,
                                                          PropertyPredicate::Eq,
                                                          keyphrase2));

        // Get all the messages containing those keyphrases.
        NodeIterator mi1 = get_neighbors(*ki1, Incoming, contains_sid);
        NodeIterator mi2 = get_neighbors(*ki2, Incoming, contains_sid);

        // Iterate over the messages eliminating duplicates.
        while (1) {
            if (!mi1 && !mi2)
                break;
            if (mi1) {
                if (uniq.find(&*mi1) == uniq.end()) {
                    uniq_nodes++;
                    uniq.insert(&*mi1);
                }
                mi1.next();
            }
            if (mi2) {
                if (uniq.find(&*mi2) == uniq.end()) {
                    uniq_nodes++;
                    uniq.insert(&*mi2);
                }
                mi2.next();
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

    cout << uniq_nodes << "\n";

    if (timing) print_delta(cout, t1, t2, u1, u2);

    if (verbose) {
        try {
            // Open the graph.
            Graph db(graph_name, Graph::ReadOnly);

            // Start a transaction.
            Transaction tx(db);

            // StringIDs
            StringID messageid_sid("MessageId");
            
            for (auto n:uniq)
                cout << n->get_property(messageid_sid).string_value() << "\n";
        }
        catch (Exception e) {
            print_exception(e, stderr);
        }
    }

    return 0;
}
