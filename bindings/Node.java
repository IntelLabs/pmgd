/*
 * Corresponds to the node.h file in Jarvis.
 *
 * Notes:
 *  - need to implement iterators for properties and edges
 */

public class Node {
    private long jarvisHandle;
    private Node(long handle) { jarvisHandle = handle; }

    public native String get_tag() throws Exception;
    public native Property get_property(String property) throws Exception;
    public native PropertyIterator get_properties();
    //    public native EdgeIterator get_edges();
    //    public native EdgeIterator get_edges(Direction dir);
    //    public native EdgeIterator get_edges(String tag);
    //    public native EdgeIterator get_edges(Direction dir, String tag);
    public native void set_property(String id, Property prop) throws Exception;
    public native void remove_property(String name) throws Exception;
}
