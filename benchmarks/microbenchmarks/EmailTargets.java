/**
 * Get a list of all users to which a particular user has sent emails.
 *
 * Starting with an email address (the sender), return a list of
 * unique email addresses to which the sender has sent at least one
 * email.
 */

import pmgd.*;
import java.util.Set;
import java.util.HashSet;
import java.lang.String;
import java.lang.management.*;

public class EmailTargets {

    /**
     * Program name.
     */
    static String progName = "EmailTargets";

    /*
     * Tags and properties. 
     */
    static String person_str = "Person";
    static String email_str = "Email";
    static String to_str = "To";
    static String from_str = "From";

    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
     */
    public static void main(String[] argv)
    {
        boolean timing = false;    ///< Enable timing measurement
        boolean verbose = false;
        int argc = argv.length;

        String graph_name;       ///< Input graph
        String sender_email;     ///< A sender's email address

        int num_recipients;     ///< The number of unique recipients

        // Parse the command line arguments
        int argi = 0;
        while (argi < argc && argv[argi].charAt(0) == '-') {
            if (argv[argi].compareTo("-h") == 0) {
                printUsage();
                return;
            }
            else if (argv[argi].compareTo("-t") == 0)
                timing = true;
            else if (argv[argi].compareTo("-v") == 0)
                verbose = true;
            else {
                System.out.println(progName + ": " + argv[argi] + ": Unrecognized option");
                return;
            }
            argi += 1;
        }
        if (argi + 1 > argc) {
            System.out.println(progName + ": No input graph");
            printUsage();
            return;
        }
        graph_name = argv[argi];
        argi++;
        if (argi + 1 > argc) {
            System.out.println(": No sender's email address");
            printUsage();
            return;
        }
        sender_email = argv[argi];

        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        long userTime, cpuTime;
        try {
            // Open the graph.
            Graph db = new Graph(graph_name, Graph.OpenOptions.ReadOnly);

            // Start a transaction.
            Transaction tx = new Transaction(db, false, true);

            // Time measurement.
            long startTime = bean.getCurrentThreadUserTime();
            long startTotal = bean.getCurrentThreadCpuTime();

            // Initialize the number of recipients.
            num_recipients = 0;

            // Set of recipients.
            Set<Node> recipients = new HashSet<Node>();

            try {
                // Convert C string to StringIDs.
                StringID person_sid = new StringID(person_str);
                StringID email_sid = new StringID(email_str);
                StringID to_sid = new StringID(to_str);
                StringID from_sid = new StringID(from_str);

                // Get the node representing the sender (we assume it is unique).
                NodeIterator senderi = db.get_nodes(person_sid,
                        new PropertyPredicate(email_sid,
                            PropertyPredicate.Op.Eq,
                            new Property(sender_email)), false);
                // If the sender exists, look for recipients.
                if (!senderi.done()) {
                    for (NodeIterator m = senderi.get_current().get_neighbors(Node.Direction.Incoming, from_sid, false); !m.done(); m.next()) {
                        for (NodeIterator p = m.get_current().get_neighbors(Node.Direction.Outgoing, to_sid, false); !p.done(); p.next()) {
                            Node n = p.get_current();
                            if (!recipients.contains(n)) {
                                recipients.add(n);
                                num_recipients++;
                            }
                        }
                    }
                }
            }
            catch (pmgd.Exception e) {
                e.print();
                return;
            }

            // Time measurement.
            long endTime = bean.getCurrentThreadUserTime();
            long endTotal = bean.getCurrentThreadCpuTime();
            userTime = (endTime - startTime)/1000;
            cpuTime = (endTotal - startTotal)/1000;

            // Print a summary.
            System.out.println(num_recipients);

            // Be more detailed if requested.
            if (verbose)
                for (Node i:recipients)
                    System.out.println(i.get_property(new StringID("Email")).string_value());

            if (timing) {
                long sysTime = cpuTime - userTime;
                float upercent = 100 * (userTime / (float)cpuTime);
                float spercent = 100 * (sysTime / (float)cpuTime);
                System.out.println("Elapsed time is " + cpuTime + " microseconds");
                System.out.println("User time is " + userTime + " microseconds (" + upercent + "%)");
                System.out.println("System time is " + sysTime + " microseconds (" + spercent + "%)");
            }
        }
        catch (pmgd.Exception e) {
            e.print();
            return;
        }

        return;
    }

    /**
     * Print information about how to invoke this program
     */
    static void printUsage()
    {
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH SENDER");
        System.out.println("");
        System.out.println("From GRAPH get a list of all unique email addresses to which");
        System.out.println("SENDER (specified as an email address) has sent emails.");
        System.out.println("");
        System.out.println("  -h  print this help and exit");
        System.out.println("  -t  time the execution");
        System.out.println("  -v  be verbose and print the recipient's email addresses");
    }
}
