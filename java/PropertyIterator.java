/**
 * @file   PropertyIterator.java
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
 * Java wrapper for PMGD PropertyIterator.
 */

package pmgd;

public class PropertyIterator {
    private long pmgdHandle;
    private PropertyIterator(long handle) { pmgdHandle = handle; }

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

    public void finalize() { dispose(); }
    public native void dispose();
}
