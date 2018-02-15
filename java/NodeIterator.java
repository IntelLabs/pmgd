/**
 * @file   NodeIterator.java
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
 * Java wrapper for PMGD NodeIterator
 */

package pmgd;

public class NodeIterator {
    private long pmgdHandle;
    private Node current;

    private NodeIterator(long handle, Node origin)
    {
        pmgdHandle = handle;
        current = origin;
    }

    public boolean done() //replaces the override of bool()
        { return current == null; }

    public native void next();

    //    public native NodeIterator filter(PropertyPredicate pp);
    //    public native void process();



    private void null_check(String method) throws Exception
    {
        if (current == null)
            throw new Exception(2, "null_iterator",
                                "NodeIterator." + method, null);
    }

    // Wrap the functions for node, so we can act on the current
    // Because java doesn't allow overriding operators, we cannot
    //  just let them slide through to pmgd.
    public Node get_current() throws Exception
    {
        null_check("get_current");
        return current;
    }

    public StringID get_tag() throws Exception
    {
        null_check("get_tag");
        return current.get_tag();
    }

    public Property get_property(StringID prop_id) throws Exception
    {
        null_check("get_property");
        return current.get_property(prop_id);
    }

    public PropertyIterator get_properties() throws Exception
    {
        null_check("get_properties");
        return current.get_properties();
    }

    public void set_property(StringID prop_id, Property prop) throws Exception
    {
        null_check("set_property");
        current.set_property(prop_id, prop);
    }

    public void remove_property(StringID prop_id) throws Exception
    {
        null_check("remove_property");
        current.remove_property(prop_id);
    }

    public void finalize() { dispose(); }
    public native void dispose();
}
