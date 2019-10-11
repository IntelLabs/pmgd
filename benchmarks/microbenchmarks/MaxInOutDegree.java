/**
 * Compute the indegree and outdegree of all nodes in a graph and
 * print the maximums.
 */
import pmgd.*;
import java.lang.String;
import java.lang.management.*;

public class MaxInOutDegree {
    /**
     * Program name.
     */
    static String progName = "MaxInOutDegree";

    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
     */
    public static void  main(String[] argv)
    {
        boolean timing = false;
        int argc = argv.length;

        String graph_name;

        long max_indegree;
        long max_outdegree;

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

            // Compute the indegree and outdegree of each node, and
            // compute the maximums.
            max_indegree = -1;
            max_outdegree = -1;
            for (NodeIterator n = db.get_nodes(); !n.done(); n.next()) {
                long indegree = 0;
                for (EdgeIterator e = n.get_current().get_edges(Node.Direction.Incoming); !e.done(); e.next())
                    ++indegree;
                if (indegree > max_indegree) max_indegree = indegree;

                long outdegree = 0;
                for (EdgeIterator e = n.get_current().get_edges(Node.Direction.Outgoing); !e.done(); e.next())
                    ++outdegree;
                if (outdegree > max_outdegree) max_outdegree = outdegree;
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

        System.out.println(max_indegree);
        System.out.println(max_outdegree);

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
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH");
        System.out.println("Find the nodes in GRAPH with the maximum indegree and outdegree.");
        System.out.println("  -h  print this help and exit");
        System.out.println("  -t  time the execution");
    }
}
