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

import java.util.*;
import java.lang.String;
import java.lang.Comparable;
import java.lang.management.*;
import pmgd.*;

public class Triangles {
    /**
     * Program name
     */
    String progName = "Triangles";

    /**
    * Auxiliary structure used for sorting nodes by their degrees
    */
    public class Degree implements Comparable<Degree>
    {
        Node _n;
        long _id;      ///< Node ID
        long _degree;   ///< Node's number of incident edges
        public Set<Long> id_set;

        public Degree(Node n, long id, long deg)
        {
            _n = n;
            _id = id;
            _degree = deg;
            id_set = new HashSet<Long>();
        }

        public Node getNode()
        {
            return _n;
        }

        public long getDegree()
        {
            return _degree;
        }

        public long getId()
        {
            return _id;
        }

        /**
        * Sort by the degree first and then by id
        * @param other instance of Degree to compare against this
        * @return true if this is "smaller"
        * Higher degree comes first and if there is a tie, the lower id
        * wins.
        */
        public int compareTo(Degree other)
        {
            int retVal;
            if (_degree == other.getDegree())
                retVal = (int)(_id - other.getId());
            else
                retVal = (int)(other.getDegree() - _degree);
            return retVal;
        }
    }

    public class DegreeMap
    {
        Map<Long,Degree> _node_map;
        Set<Degree> _degree_set;

        public class DegreeCompare implements Comparator<Degree>
        {
            @Override
            public int compare(Degree d1, Degree d2)
            {
                return d1.compareTo(d2);
            }
        }

        public DegreeMap()
        {
            _node_map = new HashMap<Long,Degree>();
            _degree_set = new TreeSet<Degree>(new DegreeCompare());
        }

        public void add(Node n, long id, long d)
        {
            Degree deg = new Degree(n, id, d);
            _node_map.put(id, deg);
            _degree_set.add(deg);
        }

        public Degree get(long id)
        {
            return _node_map.get(id);
        }

        public Set<Degree> get()
        {
            return _degree_set;
        }
    }

    /**
     * The application's main function
     * @param argc the number of command line arguments
     * @param argv the array of command line arguments
     * @return status of execution (0 okay, 1 bad arguments, 2 processing error)
     */
    public void computeTriangles(String[] argv)
    {
        boolean verbose = false;
        int num_nodes = 0;
        long num_triangles = 0;

        String graph_name;

        boolean timing = false;
        int argc = argv.length;

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
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        long userTime, cpuTime;

        try {
            // Open the graph.
            Graph db = new Graph(graph_name, Graph.OpenOptions.ReadOnly);

            // Start a transaction.
            Transaction tx = new Transaction(db, false, true);
            StringID ID = new StringID("pmgd.loader.id");

            // Time measurement.
            long startTime = bean.getCurrentThreadUserTime();
            long startTotal = bean.getCurrentThreadCpuTime();

            DegreeMap degrees = new DegreeMap();
            for (NodeIterator n = db.get_nodes(); !n.done(); n.next()) {
                ++num_nodes;
                long degree = 0;
                Node ni = n.get_current();
                long id = db.get_id(ni);
                for (EdgeIterator e = ni.get_edges(); !e.done(); e.next())
                    ++degree;
                degrees.add(ni, id, degree);
            }

            // Fill a vector with id and degree information and sort it.
            //
            // While perhaps not apparent upon first reading the algorithm
            // described in Schank and Wagner's paper, it appears that the
            // nodes in the graph must not be compared according their IDs
            // but rather by their positions in the sorted list.
            // Find the triangles.
            for (Degree s:degrees.get()) {
                Node sin = s.getNode();
                long id_sin = db.get_id(sin);
                for (NodeIterator ti = sin.get_neighbors(); !ti.done(); ti.next()) {
                    Node tin = ti.get_current();
                    long id_tin = db.get_id(tin);
                    Degree t = degrees.get(id_tin);
                    if (s.compareTo(t) < 0) {
                        for (long nid : s.id_set) {
                            if (t.id_set.contains(nid)) {
                                ++num_triangles;
                                if (verbose) {
                                    System.out.println(degrees.get(nid).getNode().get_property(ID).int_value() + "\t" +
                                                    sin.get_property(ID).int_value() + "\t" +
                                                    tin.get_property(ID).int_value());
                                }
                            }
                        }
                        t.id_set.add(s.getId());
                    }
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

        System.out.println(num_triangles);

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

    // Trouble using the Degree class due to static nature of
    // main.
    public static void main(String[] argv)
    {
        Triangles t = new Triangles();
        t.computeTriangles(argv);
    }

    /**
     * Print information about how to invoke this program
     * @param o the put-to stream
     */
    void printUsage()
    {
        System.out.println("Usage: " + progName + " [OPTION]... GRAPH");
        System.out.println(" -h  print this help and exit");
        System.out.println(" -v  be verbose and output the triangles");
        System.out.println(" -t  time the execution");
    }
}
