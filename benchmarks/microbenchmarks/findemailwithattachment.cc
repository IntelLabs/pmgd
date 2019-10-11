/**
 * Find all emails to a user containing attachment of a particular
 * type received in a particular year.
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include "pmgd.h"
#include "util.h"
#include "neighbor.h"
#include "time.h"

using namespace std;
using namespace PMGD;

/**
 * Program name.
 */
static const char prog_name[] = "findemailwithattachment";

/*
 * Tags and properties. 
 */
const char *person_str = "Person";
const char *email_str = "Email";
const char *to_str = "To";
const char *attachment_str = "Attachment";
const char *filename_str = "Filename";
const char *deliverytime_str = "DeliveryTime";
const char *messageid_str = "MessageId";

/**
 * Print information about how to invoke this program
 * @param o the put-to stream
 */
void print_usage(ostream &o)
{
    o << "Usage: " << prog_name << " [OPTION]... GRAPH RECIPIENT TYPE START END\n";
    o << "\n";
    o << "From GRAPH get a list of all emails sent between START and END to the given\n";
    o << "RECIPIENT with the email containing at least one attachment of the given\n";
    o << "TYPE.  TYPE is specified as a dotted suffix such as \".pdf\" or \".pptx\".\n";
    o << "START and END must be Windows-compliant date specification such as\n";
    o << "\"Wed Dec 31 06:25:48 PST 2003\".\n";
    o << "\n";
    o << "  -h  print this help and exit\n";
    o << "  -t  print elapsed time and usage\n";
    o << "  -v  be verbose and print the messages' unique identifiers\n";
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

    int num_emails;         ///< The number of emails

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
    char *graph_name = argv[argi];
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No recipient's email address\n";
        print_usage(cerr);
        return 1;
    }
    char *recipient_email = argv[argi];
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No attachment type\n";
        print_usage(cerr);
        return 1;
    }
    char *attachment_type = argv[argi]; // e.g., ".pptx" or ".pdf"
    size_t attachment_type_len = strlen(attachment_type);
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No start of time range\n";
        print_usage(cerr);
        return 1;
    }
    struct tm tm;
    int hr, min;
    if (!string_to_tm(argv[argi], &tm, &hr, &min)) {
        cerr << prog_name << ": Malformed start of time range\n";
        print_usage(cerr);
        return 1;
    }
    Time start(&tm, hr, min);
    argi++;
    if (argi + 1 > argc) {
        cerr << prog_name << ": No end of time range\n";
        print_usage(cerr);
        return 1;
    }
    if (!string_to_tm(argv[argi], &tm, &hr, &min)) {
        cerr << prog_name << ": Malformed end of time range\n";
        print_usage(cerr);
        return 1;
    }
    Time end(&tm, hr, min);

    try {
        // Open the graph.
        Graph db(graph_name, Graph::ReadOnly);

        // Start a transaction.
        Transaction tx(db);

        // Convert C string to StringIDs.
        StringID person_sid(person_str);
        StringID email_sid(email_str);
        StringID to_sid(to_str);
        StringID attachment_sid(attachment_str);
        StringID filename_sid(filename_str);
        StringID deliverytime_sid(deliverytime_str);
        StringID messageid_sid(messageid_str);

        // Time and usage measurements.
        (void)getrusage(RUSAGE_SELF, &u1);
        (void)gettimeofday(&t1, NULL);

        // Initialize the number of recipients.
        num_emails = 0;
        vector<Node *> emails;

        try {
            // Get the node representing the recipient (we assume it is unique).
            NodeIterator ri = db.get_nodes(person_sid,
                                           PropertyPredicate(email_sid,
                                                             PropertyPredicate::Eq,
                                                             recipient_email));
            // If the recipient exists, look for messages.
            if (ri)
                for (NodeIterator mi = get_neighbors(*ri, Any, to_sid, false);
                     mi;
                     mi.next()) {
                    for (NodeIterator ai = get_neighbors(*mi, Outgoing, attachment_sid, false);
                         ai;
                         ai.next()) {
                        Property filenamep;
                        if (ai->check_property(filename_sid, filenamep)) {
                            string s = filenamep.string_value();
                            size_t len = s.length();
                            Property deliverytimep;
                            if (len >= attachment_type_len
                                && s.compare(len - attachment_type_len, attachment_type_len, attachment_type) == 0
                                && mi->check_property(deliverytime_sid, deliverytimep)) {
                                Time t = deliverytimep.time_value(); 
                                if (start <= t && t <= end) {
                                    emails.push_back(&*mi);
                                    num_emails++;
                                    break;
                                }
                            }
                        }
                    }
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
        cout << num_emails << "\n";

        // Print timing info if requested.
        if (timing) print_delta(cout, t1, t2, u1, u2);

        // Be more detailed if requested.
        if (verbose)
            for (auto n:emails)
                cout << n->get_property(messageid_sid).string_value() << "\n";
    }
    catch (Exception e) {
        print_exception(e, stderr);
        return 2;
    }

    return 0;
}
