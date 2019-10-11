/**
 * Find nodes with either of two given keyphrases.
 *
 * Look for all emails in GRAPH that contain either KEYPHRASE1 or
 * KEYPHRASE2.
 */

import pmgd.*;
import java.util.Set;
import java.util.HashSet;
import java.lang.String;
import java.lang.management.*;

public class Phrase1OrPhrase2 {

    /**
     * Program name.
     */
    static String progName = "Phrase1OrPhrase2";


    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     */
    public static void main(String[] argv)
    {
        boolean timing = false;
        boolean verbose = false;
        int argc = argv.length;

        String graph_name;
        String keyphrase1;
        String keyphrase2;

        // Parse the command line arguments
        int argi = 0;
        while (argi < argc && argv[argi].charAt(0) == '-') {
            if (argv[argi].compareTo("-h") == 0) {
                printUsage();
                return;
            }
            else if (argv[argi].compareTo("-t") == 0) {
                timing = true;
            }
            else if (argv[argi].compareTo("-v") == 0) {
                verbose = true;
            }
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
        if (argi + 2 > argc) {
            System.out.println(progName + ": No keyphrases");
            printUsage();
            return;
        }
        keyphrase1 = argv[argi];
        keyphrase2 = argv[argi + 1];

        // Number of nodes and set of nodes seen so far.
        long uniq_nodes = 0;
        Set<Node> uniq = new HashSet<Node>();
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        long userTime, cpuTime;

        try {
            // Open the graph.
            Graph db = new Graph(graph_name, Graph.OpenOptions.ReadOnly);

            // Start a transaction.
            Transaction tx = new Transaction(db, false, true);

            // StringIDs
            StringID contains_sid = new StringID("Contains");
            StringID keywords_sid = new StringID("Keywords");
            StringID phrase_sid = new StringID("Phrase");

            // Time measurement.
            long startTime = bean.getCurrentThreadUserTime();
            long startTotal = bean.getCurrentThreadCpuTime();

            // Look for both keyphrases
            NodeIterator ki1 = db.get_nodes(keywords_sid,
                    new PropertyPredicate(phrase_sid,
                        PropertyPredicate.Op.Eq,
                        new Property(keyphrase1)), false);
            NodeIterator ki2 = db.get_nodes(keywords_sid,
                    new PropertyPredicate(phrase_sid,
                        PropertyPredicate.Op.Eq,
                        new Property(keyphrase2)), false);

            // Get all the messages containing those keyphrases.
            NodeIterator mi1 = ki1.get_current().get_neighbors(Node.Direction.Incoming, contains_sid);
            NodeIterator mi2 = ki2.get_current().get_neighbors(Node.Direction.Incoming, contains_sid);

            // Iterate over the messages eliminating duplicates.
            while (true) {
                if (mi1.done() && mi2.done())
                    break;
                if (!mi1.done()) {
                    if (!uniq.contains(mi1.get_current())) {
                        uniq_nodes++;
                        uniq.add(mi1.get_current());
                    }
                    mi1.next();
                }
                if (!mi2.done()) {
                    if (!uniq.contains(mi2.get_current())) {
                        uniq_nodes++;
                        uniq.add(mi2.get_current());
                    }
                    mi2.next();
                }
            }

            // Time measurement.
            long endTime = bean.getCurrentThreadUserTime();
            long endTotal = bean.getCurrentThreadCpuTime();
            userTime = (endTime - startTime)/1000;
            cpuTime = (endTotal - startTotal)/1000;
        }
        catch (pmgd.Exception e) {
            e.print();
            return;
        }

        System.out.println(uniq_nodes);

        if (timing) {
            long sysTime = cpuTime - userTime;
            float upercent = 100 * (userTime / (float)cpuTime);
            float spercent = 100 * (sysTime / (float)cpuTime);
            System.out.println("Elapsed time is " + cpuTime + " microseconds");
            System.out.println("User time is " + userTime + " microseconds (" + upercent + "%)");
            System.out.println("System time is " + sysTime + " microseconds (" + spercent + "%)");
        }

        if (verbose) {
            try {
                // Open the graph.
                Graph db = new Graph(graph_name, Graph.OpenOptions.ReadOnly);

                // Start a transaction.
                Transaction tx = new Transaction(db, false, true);

                // StringIDs
                StringID messageid_sid = new StringID("MessageId");

                for (Node n:uniq)
                    System.out.println(n.get_property(messageid_sid).string_value());
            }
            catch (pmgd.Exception e) {
                e.print();
            }
        }

        return;

    }

    /**
     * Print information about how to invoke this program
     */
    static void printUsage()
    {
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH KEYPHRASE1 KEYPHRASE2");
        System.out.println("");
        System.out.println("Look for all nodes in GRAPH that contains either KEYPHRASE1");
        System.out.println("or KEYPHRASE2.");
        System.out.println("");
        System.out.println("  -h  print this help and exit");
        System.out.println("  -t  time the execution");
        System.out.println("  -v  print the UUID of the emails found");
    }
}
