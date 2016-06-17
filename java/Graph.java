/*
 * Corresponds to the graph.h file in Jarvis.
 *
 * Notes:
 *  - Omitting Config options at the moment
 *  - need to implement index
 */

package jarvis;

public class Graph {
    private long jarvisHandle;

    public enum OpenOptions { ReadWrite, Create, ReadOnly };
    public enum IndexType { NodeIndex, EdgeIndex };

    public Graph(String db_name, OpenOptions options) throws Exception
    {
        loadGraphNative(db_name, options.ordinal());
    }

    public native long get_id(Node n);
    public native long get_id(Edge n);

    public native NodeIterator get_nodes() throws Exception;

    public NodeIterator get_nodes(StringID tag) throws Exception
        { return get_nodes(tag.id()); }

    public NodeIterator get_nodes
                (StringID tag, PropertyPredicate ppred, boolean reverse)
                throws Exception
        { return get_nodes(tag.id(), ppred, reverse); }

    public NodeIterator get_nodes(StringID tag, PropertyPredicate ppred)
                throws Exception
        { return get_nodes(tag, ppred, false); }

    public NodeIterator get_nodes(StringID tag, StringID prop_id, String val)
                throws Exception
        { return get_nodes(tag, new PropertyPredicate(prop_id, val)); }

    public NodeIterator get_nodes(StringID tag, StringID prop_id, long val)
                throws Exception
        { return get_nodes(tag, new PropertyPredicate(prop_id, val)); }

    public Node get_node(StringID tag, PropertyPredicate ppred)
                throws Exception
        { return get_nodes(tag, ppred).get_current(); }

    public Node get_node(StringID tag, StringID prop_id, String val)
                throws Exception
        { return get_nodes(tag, prop_id, val).get_current(); }

    public Node get_node(StringID tag, StringID prop_id, long val)
                throws Exception
        { return get_nodes(tag, prop_id, val).get_current(); }


    public native EdgeIterator get_edges() throws Exception;

    public EdgeIterator get_edges(StringID tag) throws Exception
        { return get_edges(tag.id()); }

    public EdgeIterator get_edges
                (StringID tag, PropertyPredicate ppred, boolean reverse)
                throws Exception
        { return get_edges(tag.id(), ppred, reverse); }

    public EdgeIterator get_edges(StringID tag, PropertyPredicate ppred)
                throws Exception
        { return get_edges(tag.id(), ppred, false); }

    public EdgeIterator get_edges(StringID tag, StringID prop_id, String val)
                throws Exception
        { return get_edges(tag, new PropertyPredicate(prop_id, val)); }

    public EdgeIterator get_edges(StringID tag, StringID prop_id, long val)
                throws Exception
        { return get_edges(tag, new PropertyPredicate(prop_id, val)); }


    public Node add_node(StringID tag) throws Exception
        { return add_node(tag.id()); }
    public Edge add_edge(Node src, Node dest, StringID tag) throws Exception
        { return add_edge(src, dest, tag.id()); }

    public native void remove(Node n) throws Exception;
    public native void remove(Edge e) throws Exception;


    private native NodeIterator get_nodes(int tag) throws Exception;
    private native NodeIterator get_nodes
                (int tag, PropertyPredicate ppred, boolean reverse)
                throws Exception;

    private native EdgeIterator get_edges(int tag) throws Exception;
    private native EdgeIterator get_edges
                (int tag, PropertyPredicate ppred, boolean reverse)
                throws Exception;

    private native Node add_node(int tag) throws Exception;
    private native Edge add_edge(Node src, Node dest, int tag)
                throws Exception;

    // public native void create_index(IndexType index_type, StringID tag,
    //                                 StringID property_id, PropertyType ptype);

    private native void loadGraphNative(String db_name, int options);
    public void finalize() { dispose(); }
    public native void dispose();

    static {
        System.loadLibrary("jarvis-jni");
    }
}
