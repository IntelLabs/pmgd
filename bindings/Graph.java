/*
 * Corresponds to the graph.h file in Jarvis.
 *
 * Notes:
 *  - Omitting Config options at the moment
 *  - need to catch and throw exceptions up
 *  - need to implement iterators for node/edge/path
 *  - need to implement index
 */

public class Graph {
    private long jarvisHandle;

    public enum OpenOptions { CREATE, READONLY };
    public enum IndexOptions { NODE, EDGE };

    public Graph(String db_name, int options)
    {
        loadGraphNative(db_name, options);
        // Use this wrapper to allow exception catches
    }

    public native int get_id(Node n);
    public native int get_id(Edge n);

    public native NodeIterator get_nodes();
    //    public native NodeIterator get_nodes(String tag);
    //    public native NodeIterator get_nodes(String tag,
    //                                         PropertyPredicate ppred,
    //                                         bool reverse);
    //    public native EdgeIterator get_edges();
    //    public native EdgeIterator get_edges(String tag);
    //    public native EdgeIterator get_edges(String tag,
    //                                         PropertyPredicate ppred,
    //                                         bool reverse);
    //    public native PathIterator get_paths(Node a, bool depth_first);
    //    public native PathIterator get_paths(Node a, Node b, bool depth_first);

    public Node add_node(String tag)
    {
        Node node = new Node(0);
        addNodeNative(node, tag);
        return node;
    }

    public Edge add_edge(Node src, Node dest, String tag)
    {
        Edge edge = new Edge(0);
        addEdgeNative(edge, src, dest, tag);
        return edge;
    }

    public native void remove(Node n);
    public native void remove(Edge e);

    //    public native void create_index(int node_or_edge, String tag,
    //                                    String property_id, PropertyType ptype);

    public native void dumpGraph();


    private native void loadGraphNative(String db_name, int options);
    private native void addNodeNative(Node node, String tag);
    private native void addEdgeNative(Edge edge,
                                      Node src, Node dest, String tag);
    static {
        System.loadLibrary("jarvis-jni");
    }
}
