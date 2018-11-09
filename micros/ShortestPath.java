/**
 * Compute the shortest path between two points of a graph.
 *
 * Assumes unit weight on edges and ignores direction.
 *
 * Uses Dijkstra's algorithm.
 */

import pmgd.*;
import java.util.Set;
import java.util.HashSet;
import java.lang.String;
import java.lang.Exception;
import java.lang.management.*;

public class ShortestPath {

    /**
     * Program name.
     */
    static String progName = "ShortestPath";

    /**
     * Graph's infinite distance.
     */
    static long infinity = Long.MAX_VALUE;

    static long shortestpath(Graph db, Node start, Node end)
        throws pmgd.Exception
    {
        if (start == end)
            return 0;

        long distance = 1;
        Set<Node> actives = new HashSet<Node>();
        Set<Node> encountered = new HashSet<Node>();
        actives.add(start);
        encountered.add(start);

        while (!actives.isEmpty()) {
            Set<Node> new_actives = new HashSet<Node>();
            for (Node n : actives) {
                for (NodeIterator neighbors = n.get_neighbors(false);
                        !neighbors.done();
                        neighbors.next()) {
                    Node neighbor = neighbors.get_current();
                    if (neighbor.equals(end))
                        return distance;
                    if (encountered.add(neighbor))
                        new_actives.add(neighbor);
                }
            }

            distance++;
            actives = new_actives;
        }
        return infinity;
    }

    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
     */
    public static void main(String[] argv)
    {
        boolean timing = false;
        int argc = argv.length;

        long distance;

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
        if (argi + 3 > argc) {
            System.out.println(progName + ": Missing input parameters");
            printUsage();
            return;
        }
        String graph_name = argv[argi];
        int startid = 0, endid = 0;
        try {
            // *** Problem with Long
            startid = Integer.parseInt(argv[argi + 1]);
            endid = Integer.parseInt(argv[argi + 2]);
        } catch (Exception e) {
            System.out.println("Non-long node id.");
            return;
        }

        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        long userTime, cpuTime;
        try {
            // Open the graph.
            Graph db = new Graph(graph_name, Graph.OpenOptions.ReadOnly);

            // Start a transaction.
            Transaction tx = new Transaction(db, false, true);

            // StringID for node id
            StringID id_sid = new StringID("pmgd.loader.id");

            // Time measurement.
            long startTime = bean.getCurrentThreadUserTime();
            long startTotal = bean.getCurrentThreadCpuTime();

            // Find start and end nodes.
            // ** No null check?
            NodeIterator si = db.get_nodes(new StringID(), new PropertyPredicate(id_sid,
                        PropertyPredicate.Op.Eq,
                        new Property(startid)), false);
            if (si.done()) {
                System.out.println("Start node not found");
                return;
            }
            Node start = si.get_current();
            Node end = db.get_nodes(new StringID(), new PropertyPredicate(id_sid,
                        PropertyPredicate.Op.Eq,
                        new Property(endid)), false).get_current();

            // Find the shortest path.
            distance = shortestpath(db, start, end);

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

        if (distance == infinity)
            System.out.println("Infinity");
        else
            System.out.println(distance);

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
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH START END");
        System.out.println("Calculate the shortest path between START and END of GRAPH");
        System.out.println("and output the length of the path (i.e., number of hops).");
        System.out.println("Assumes unit weight on edges and ignores direction.");
        System.out.println("  -h  print this help and exit");
        System.out.println("  -t  time the execution");
        System.out.println("  -v  show one shortest path");
    }
}
