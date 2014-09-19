/*
 * Corresponds to the Property.h file in Jarvis.
 *
 * Notes:
 *  - Omitting time, blob as an option for property types
 */

public class Property {
    private long jarvisHandle;
    private Property(long handle) { jarvisHandle = handle; }

    public Property() throws Exception
    {
        newPropertyNative();
    }

    public Property(Property p) throws Exception
    {
        newPropertyNative(p);
    }

    public Property(boolean v) throws Exception
    {
        newPropertyNative(v);
    }

    public Property(int v) throws Exception
    {
        newPropertyNative(v);
    }

    public Property(String s) throws Exception
    {
        newPropertyNative(s);
    }

    public Property(double v) throws Exception
    {
        newPropertyNative(v);
    }

    //    public Property(Time t) throws Exception
    //    {
    //        newPropertyNative(t);
    //    }

    //    public Property(blob_t b) throws Exception
    //    {
    //        newPropertyNative(b);
    //    }

    // Add operator overrides here


    // Java enums are messed up. Using constants instead
    public static final int t_novalue = 1;
    public static final int t_boolean = 2;
    public static final int t_integer = 3;
    public static final int t_string = 4;
    public static final int t_float = 5;
    public static final int t_time = 6;
    public static final int t_blob = 7;

    public native int type() throws Exception;
    public native boolean bool_value() throws Exception;
    public native long int_value() throws Exception;
    public native String string_value() throws Exception;
    public native double float_value() throws Exception;
    //    public native Time time_value() throws Exception;
    //    public native Blob blob_value() throws Exception;

    private native void newPropertyNative() throws Exception;
    private native void newPropertyNative(Property p) throws Exception;
    private native void newPropertyNative(boolean v) throws Exception;
    private native void newPropertyNative(int v) throws Exception;
    private native void newPropertyNative(String s) throws Exception;
    private native void newPropertyNative(double v) throws Exception;
}
