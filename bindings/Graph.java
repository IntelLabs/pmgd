/*
 * Corresponds to the graph.h file in Jarvis.
 *
 * Notes:
 *  - Omitting Config options at the moment
 *  - need to implement index
 */

public class Graph {
    private long jarvisHandle;

    public enum OpenOptions { NONE, CREATE, READONLY };
    public enum IndexOptions { DUMMY, NODE, EDGE };

    public Graph(String db_name, OpenOptions options) throws Exception
    {
        loadGraphNative(db_name, options.ordinal());
    }

    public native int get_id(Node n);
    public native int get_id(Edge n);

    public native NodeIterator get_nodes() throws Exception;
    public native NodeIterator get_nodes(String tag) throws Exception;
    public native NodeIterator get_nodes(String tag,
                                         PropertyPredicate ppred,
                                         boolean reverse) throws Exception;

    public native EdgeIterator get_edges() throws Exception;
    public native EdgeIterator get_edges(String tag) throws Exception;
    public native EdgeIterator get_edges(String tag,
                                         PropertyPredicate ppred,
                                         boolean reverse) throws Exception;

    //    public native PathIterator get_paths(Node a, bool depth_first);
    //    public native PathIterator get_paths(Node a, Node b, bool depth_first);

    public native Node add_node(String tag) throws Exception;
    public native Edge add_edge(Node src, Node dest, String tag)
                           throws Exception;

    public native void remove(Node n) throws Exception;
    public native void remove(Edge e) throws Exception;

    //    public native void create_index(int node_or_edge, String tag,
    //                                    String property_id, PropertyType ptype);

    public native void dumpGraph();


    private native void loadGraphNative(String db_name, int options);

    static {
        System.loadLibrary("jarvis-jni");
    }
}
