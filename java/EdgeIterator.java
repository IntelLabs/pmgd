/**
 * @file   EdgeIterator.java
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
 * Java wrapper for PMGD EdgeIterator
 */

package pmgd;

public class EdgeIterator {
    private long pmgdHandle;
    private EdgeIterator(long handle) { pmgdHandle = handle; }

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
