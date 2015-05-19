/*
 * Corresponds to the Property.h file in Jarvis.
 *
 * Notes:
 *  - Omitting time, blob as an option for property types
 */

package jarvis;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

public class Property {
    public class Time {
        public long time_val;

        public int year;   // -8192-8191
        public int mon;    // 1-12
        public int day;    // 1-31
        public int hour;   // 0-23
        public int min;    // 0-59
        public int sec;    // 0-60
        public int usec;   // 0-999,999
        public int tz_hour;
        public int tz_min;

        public Time()
        {
            time_val = 0;
            year = mon = day = 0;
            hour = min = sec = usec = 0;
            tz_hour = tz_min = 0;
        }

        // Returns UTC/GMT date value.
        public Date getDate() throws Exception
        {
            Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
            c.set(Calendar.YEAR, year);
            c.set(Calendar.MONTH, mon - 1);
            c.set(Calendar.DATE, day);
            c.set(Calendar.HOUR, hour);
            c.set(Calendar.MINUTE, min);
            c.set(Calendar.SECOND, sec);
            c.set(Calendar.MILLISECOND, usec/1000);
            return c.getTime();
        }

        // Return timezone stored with date value.
        public TimeZone getTimeZone()
        {
            return TimeZone.getTimeZone(String.format("GMT+%02d:%02d", tz_hour, tz_min));
        }
    }

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
    public native Time time_value() throws Exception;
    //    public native Blob blob_value() throws Exception;

    private native void newPropertyNative() throws Exception;
    private native void newPropertyNative(Property p) throws Exception;
    private native void newPropertyNative(boolean v) throws Exception;
    private native void newPropertyNative(int v) throws Exception;
    private native void newPropertyNative(String s) throws Exception;
    private native void newPropertyNative(double v) throws Exception;

    public void finalize() { dispose(); }
    public native void dispose();
}
