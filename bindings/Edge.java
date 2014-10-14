/*
 * Corresponds to the edge.h file in Jarvis.
 */

package jarvis;

public class Edge {
    private long jarvisHandle;
    private Edge(long handle) { jarvisHandle = handle; }

    public native String get_tag() throws Exception;
    public native Node get_source() throws Exception;
    public native Node get_destination() throws Exception;

    public native Property get_property(String property) throws Exception;
    public native PropertyIterator get_properties();
    public native void set_property(String id, Property property) throws Exception;
    public native void remove_property(String name) throws Exception;
}
