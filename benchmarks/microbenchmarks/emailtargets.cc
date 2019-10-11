/**
 * Get a list of all users to which a particular user has sent emails.
 *
 * Starting with an email address (the sender), return a list of
 * unique email addresses to which the sender has sent at least one
 * email.
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
static const char prog_name[] = "emailtargets";

/*
 * Tags and properties. 
 */
const char *person_str = "Person";
const char *email_str = "Email";
const char *to_str = "To";
const char *from_str = "From";

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH SENDER\n";
    o << "\n";
    o << "From GRAPH get a list of all unique email addresses to which\n";
    o << "SENDER (specified as an email address) has sent emails.\n";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -t  print elapsed time and usage\n";
    o << "  -v  be verbose and print the recipient's email addresses\n";
}

/**
 * The application's main function
 * @param argc the number of command line arguments
 * @param argv the array of command line arguments
 * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
 */
int main(int argc, char *argv[])
{
    bool timing = false;    ///< Enable timing measurement
    struct timeval t1, t2;
    struct rusage u1, u2;

    bool verbose = false;

    char *graph_name;       ///< Input graph
    char *sender_email;     ///< A sender's email address

    int num_recipients;     ///< The number of unique recipients

    // Parse the command line arguments
    int argi = 1;
    while (argi < argc && argv[argi][0] == '-') {
        if (strcmp(argv[argi], "-h") == 0) {
            print_usage(cout);
            return 0;
        }
        else if (strcmp(argv[argi], "-t") == 0)
            timing = true;
        else if (strcmp(argv[argi], "-v") == 0)
            verbose = true;
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
        cerr << prog_name << ": No sender's email address\n";
        print_usage(cerr);
        return 1;
    }
    sender_email = argv[argi];

    try {
        // Open the graph.
        Graph db(graph_name, Graph::ReadOnly);

        // Start a transaction.
        Transaction tx(db);

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // Initialize the number of recipients.
        num_recipients = 0;

        // Set of recipients.
        set<Node *> recipients;

        try {
            // Convert C string to StringIDs.
            StringID person_sid(person_str);
            StringID email_sid(email_str);
            StringID to_sid(to_str);
            StringID from_sid(from_str);

            // Get the node representing the sender (we assume it is unique).
            NodeIterator senderi = db.get_nodes(person_sid,
                                                PropertyPredicate(email_sid,
                                                                  PropertyPredicate::Eq,
                                                                  sender_email));
            // If the sender exists, look for recipients.
            if (senderi)
                for (NodeIterator m = get_neighbors(*senderi, Incoming, from_sid, false); m; m.next())
                    for (NodeIterator p = get_neighbors(*m, Outgoing, to_sid, false); p; p.next())
                        if (recipients.find(&*p) == recipients.end()) {
                            recipients.insert(&*p);
                            num_recipients++;
                        }

        }
        catch (Exception e) {
            if (e.num != ReadOnly)
                throw;
        }

        // Time and usage measurements.
        (void)gettimeofday(&t2, NULL);
        (void)getrusage(RUSAGE_SELF, &u2);

        // Print a summary.
        cout << num_recipients << "\n";

        // Be more detailed if requested.
        if (verbose)
            for (auto i:recipients)
                cout << i->get_property("Email").string_value() << "\n";
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 2;
    }

    if (timing) print_delta(cout, t1, t2, u1, u2);

    return 0;
}
