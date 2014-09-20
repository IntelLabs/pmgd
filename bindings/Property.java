/*
 * Corresponds to the Property.h file in Jarvis.
 *
 * Notes:
 *  - Omitting time, blob as an option for property types
 *  - need to catch and throw exceptions up
 */

public class Property {
    private long jarvisHandle;
    private Property(long handle) { jarvisHandle = handle; }

    public Property()
    {
        newPropertyNative();
    }

    public Property(Property p)
    {
        newPropertyNative(p);
    }

    public Property(boolean v)
    {
        newPropertyNative(v);
    }

    public Property(int v)
    {
        newPropertyNative(v);
    }

    public Property(String s)
    {
        newPropertyNative(s);
    }

    public Property(double v)
    {
        newPropertyNative(v);
    }

    //    public Property(Time t)
    //    {
    //        newPropertyNative(t);
    //    }

    //    public Property(blob_t b)
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

    public native int type();
    public native boolean bool_value();
    public native long int_value();
    public native String string_value();
    public native double float_value();
    //    public native Time time_value();
    //    public native Blob blob_value();

    private native void newPropertyNative();
    private native void newPropertyNative(Property p);
    private native void newPropertyNative(boolean v);
    private native void newPropertyNative(int v);
    private native void newPropertyNative(String s);
    private native void newPropertyNative(double v);
}
