/*
 * Corresponds to the edge.h file in Jarvis.
 *
 * Notes:
 *  - need to catch and throw exceptions up
 *  - need to implement iterators to get properties working
 */


public class Edge {
    private long jarvisHandle;
    private Edge(long handle) { jarvisHandle = handle; }

    public native String get_tag();
    public native Node get_source();
    public native Node get_destination();

    public native Property get_property(String property);
    //    public native PropertyIterator get_properties();
    public native void set_property(String id, Property property);
    public native void remove_property(String name);
}
