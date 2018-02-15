/**
 * @file   Property.java
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * Corresponds to the Property.h file in PMGD.
 *
 * Notes:
 *  - Omitting blob as an option for property types
 */

package pmgd;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

public class Property {
    public static class Time {
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
            year = mon = day = 0;
            hour = min = sec = usec = 0;
            tz_hour = tz_min = 0;
        }

        public Time(Date date, int tz_hour_arg, int tz_min_arg)
        {
            Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
            c.setTime(date);
            year = c.get(Calendar.YEAR);
            mon = c.get(Calendar.MONTH) + 1;
            day = c.get(Calendar.DATE);
            hour = c.get(Calendar.HOUR_OF_DAY);
            min = c.get(Calendar.MINUTE);
            sec = c.get(Calendar.SECOND);
            usec = c.get(Calendar.MILLISECOND) * 1000;
            tz_hour = tz_hour_arg;
            tz_min = tz_min_arg;
        }

        public Time(Date date, int tz_hour)
        {
            this(date, tz_hour, 0);
        }

        public Time(Date date)
        {
            this(date, 0, 0);
        }

        // Returns UTC/GMT date value.
        public Date getDate()
        {
            Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
            c.set(Calendar.YEAR, year);
            c.set(Calendar.MONTH, mon - 1);
            c.set(Calendar.DATE, day);
            c.set(Calendar.HOUR_OF_DAY, hour);
            c.set(Calendar.MINUTE, min);
            c.set(Calendar.SECOND, sec);
            c.set(Calendar.MILLISECOND, usec/1000);
            return c.getTime();
        }

        // Return timezone stored with date value.
        public TimeZone getTimeZone()
        {
            return TimeZone.getTimeZone(java.lang.String.format("GMT%+03d:%02d", tz_hour, tz_min));
        }
    }

    private long pmgdHandle;
    private Property(long handle, boolean dummy) { pmgdHandle = handle; }

    public Property() throws Exception
    {
        newPropertyNative();
    }

    public Property(boolean v) throws Exception
    {
        newPropertyNative(v);
    }

    public Property(long v) throws Exception
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

    public Property(Time t) throws Exception
    {
        newPropertyNative(t);
    }

    public Property(Date d) throws Exception
    {
        this(new Time(d));
    }

    //    public Property(blob_t b) throws Exception
    //    {
    //        newPropertyNative(b);
    //    }

    // Add operator overrides here

    // Java enums are inconvenient. Using constants instead.
    public static final int NoValue = 1;
    public static final int Boolean = 2;
    public static final int Integer = 3;
    public static final int String = 4;
    public static final int Float = 5;
    public static final int Time = 6;
    public static final int Blob = 7;

    public native int type() throws Exception;
    public native boolean bool_value() throws Exception;
    public native long int_value() throws Exception;
    public native String string_value() throws Exception;
    public native double float_value() throws Exception;
    public native Time time_value() throws Exception;
    //    public native Blob blob_value() throws Exception;

    private native void newPropertyNative() throws Exception;
    private native void newPropertyNative(boolean v) throws Exception;
    private native void newPropertyNative(long v) throws Exception;
    private native void newPropertyNative(String s) throws Exception;
    private native void newPropertyNative(double v) throws Exception;
    private native void newPropertyNative(Time v) throws Exception;

    public void finalize() { dispose(); }
    public native void dispose();
}
