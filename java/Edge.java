/*
 * Corresponds to the edge.h file in Jarvis.
 */

package jarvis;

public class Edge {
    private long jarvisHandle;
    private Edge(long handle) { jarvisHandle = handle; }

    public native StringID get_tag() throws Exception;
    public native Node get_source() throws Exception;
    public native Node get_destination() throws Exception;
    public native PropertyIterator get_properties();

    public Property get_property(StringID prop_id) throws Exception
        { return get_property(prop_id.id()); }

    public void set_property(StringID prop_id, Property property)
            throws Exception
        { set_property(prop_id.id(), property); }

    public void remove_property(StringID prop_id) throws Exception
        { remove_property(prop_id.id()); }

    private native Property get_property(int id) throws Exception;
    private native void set_property(int id, Property property)
            throws Exception;
    private native void remove_property(int id) throws Exception;
}
