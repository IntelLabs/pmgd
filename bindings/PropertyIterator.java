/*
 * Java wrapper for Jarvis PropertyIterator.
 */

public class PropertyIterator {
    private long jarvisHandle;
    private PropertyIterator(long handle) { jarvisHandle = handle; }

    public native boolean done(); // corresponds to operator bool
    public native void next();

    //    public native PropertyIterator filter(PropertyPredicate pp);
    //    public native void process();

    public native Property get_current() throws Exception;
    public native String id() throws Exception;
    public native int type() throws Exception;
    public native boolean bool_value() throws Exception;
    public native long int_value() throws Exception;
    public native String string_value() throws Exception;
    public native double float_value() throws Exception;
    public native void dispose();
}
