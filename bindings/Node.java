/*
 * Corresponds to the node.h file in Jarvis.
 */

public class Node {
    private long jarvisHandle;
    private Node(long handle) { jarvisHandle = handle; }

    public enum Direction { ANY, OUTGOING, INCOMING };

    public native String get_tag() throws Exception;
    public native Property get_property(String property) throws Exception;
    public native PropertyIterator get_properties() throws Exception;
    public native EdgeIterator get_edges() throws Exception;
    public native EdgeIterator get_edges(String tag) throws Exception;
    public EdgeIterator get_edges(Direction dir) throws Exception
        { return get_edges(dir.ordinal()); }
    public EdgeIterator get_edges(Direction dir, String tag) throws Exception
        { return get_edges(dir.ordinal(), tag); }
    private native EdgeIterator get_edges(int dir) throws Exception;
    private native EdgeIterator get_edges(int dir, String tag) throws Exception;
    public native void set_property(String id, Property prop) throws Exception;
    public native void remove_property(String name) throws Exception;
}
