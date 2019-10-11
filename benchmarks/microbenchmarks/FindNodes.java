/**
 * Find nodes with a certain keyword.
 *
 * Look for all nodes with given TAG and with the given PROPERTY
 * containing the given KEYWORD.
 */

import pmgd.*;
import java.lang.String;
import java.lang.management.*;

public class FindNodes {

    /**
     * Program name.
     */
    static String progName = "FindNodes";

    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
     */
    public static void main(String[] argv)
    {
        boolean timing = false;    ///< Enable timing measurement
        int argc = argv.length;

        String graph_name;
        String tag_name;
        String property_name;
        String keyword;

        int num_nodes = 0;

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
            System.out.println(": No tag");
            printUsage();
            return;
        }
        tag_name = argv[argi];
        argi++;
        if (argi + 1 > argc) {
            System.out.println(": No property");
            printUsage();
            return;
        }
        property_name = argv[argi];
        argi++;
        if (argi + 1 > argc) {
            System.out.println(": No keyword");
            printUsage();
            return;
        }
        keyword = argv[argi];

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

            // Generate needed StringIDs.
            StringID tag_sid = new StringID(tag_name.compareTo("0") == 0 ? null : tag_name);
            StringID property_sid = new StringID(property_name);

            // Search for a keyword in given property of nodes of given tag.
            for (NodeIterator n = db.get_nodes(tag_sid); !n.done(); n.next()) {
                Property p;
                if ( (p = n.get_current().get_property(property_sid)) != null
                        && p.type() == Property.String
                        && p.string_value().contains(keyword)) {

                    num_nodes++;
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

        System.out.println(num_nodes);

        if (timing) {
            long sysTime = cpuTime - userTime;
            float upercent = 100 * (userTime / (float)cpuTime);
            float spercent = 100 * (sysTime / (float)cpuTime);
            System.out.println("Elapsed time is " + cpuTime + " microseconds");
            System.out.println("User time is " + userTime + " microseconds (" + upercent + "%)");
            System.out.println("System time is " + sysTime + " microseconds (" + spercent + "%)");
        }

        return;
    }

    /**
     * Print information about how to invoke this program
     * @param o the put-to stream
     */
    static void printUsage()
    {
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH TAG PROPERTY KEYWORD");
        System.out.println("Look for all nodes with the given TAG and with the given PROPERTY");
        System.out.println("containing the given KEYWORD.");
        System.out.println("  -h  print this help and exit");
        System.out.println("  -t  time the execution");
    }

}
