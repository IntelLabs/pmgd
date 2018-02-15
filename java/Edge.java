/**
 * @file   Edge.java
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
 * Corresponds to the edge.h file in PMGD.
 */

package pmgd;

public class Edge {
    private long pmgdHandle;
    private Edge(long handle) { pmgdHandle = handle; }

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
