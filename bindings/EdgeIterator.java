/*
 * Java wrapper for Jarvis EdgeIterator
 */

public class EdgeIterator {
    private long jarvisHandle;
    private EdgeIterator(long handle) { jarvisHandle = handle; }

    public native boolean done(); // replaces operator bool()
    public native void next();

    //    public native EdgeIterator filter(PropertyPredicate pp);
    //    public native void process();

    public native Edge get_current() throws Exception;
    public native String get_tag() throws Exception;
    public native Property get_property(String property) throws Exception;
    public native PropertyIterator get_properties() throws Exception;
    public native void set_property(String id, Property prop) throws Exception;
    public native void remove_property(String name) throws Exception;
    public native void dispose();
}
