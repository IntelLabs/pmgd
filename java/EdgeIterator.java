/*
 * Java wrapper for Jarvis EdgeIterator
 */

package jarvis;

public class EdgeIterator {
    private long jarvisHandle;
    private EdgeIterator(long handle) { jarvisHandle = handle; }

    public native boolean done(); // replaces operator bool()
    public native void next();

    //    public native EdgeIterator filter(PropertyPredicate pp);
    //    public native void process();

    public native Edge get_current() throws Exception;
    public native StringID get_tag() throws Exception;
    public native Node get_source() throws Exception;
    public native Node get_destination() throws Exception;
    public native PropertyIterator get_properties() throws Exception;

    public Property get_property(StringID prop_id) throws Exception
        { return get_property(prop_id.id()); }
    public void set_property(StringID prop_id, Property prop) throws Exception
        { set_property(prop_id.id(), prop); }
    public void remove_property(StringID prop_id) throws Exception
        { remove_property(prop_id.id()); }

    public native Property get_property(int id) throws Exception;
    public native void set_property(int id, Property prop) throws Exception;
    public native void remove_property(int id) throws Exception;

    public void finalize() { dispose(); }
    public native void dispose();
}
