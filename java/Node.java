/**
 * @file   Node.java
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
 * Corresponds to the node.h file in PMGD.
 */

package pmgd;

import java.util.Objects;

public class Node {
    private long pmgdHandle;
    private Node(long handle) { pmgdHandle = handle; }

    // Override equals method to support equal hash on nodes
    // that point to the same object in PM
    @Override
    public boolean equals(Object other)
    {
        Node no = (Node)other;
        return pmgdHandle == no.pmgdHandle;
    }

    @Override
    public int hashCode()
    {
        return Objects.hash(pmgdHandle);
    }

    public enum Direction { Any, Outgoing, Incoming };

    public native StringID get_tag() throws Exception;

    public Property get_property(StringID prop_id) throws Exception
        { return get_property(prop_id.id()); }

    public void set_property(StringID prop_id, Property prop) throws Exception
        { set_property(prop_id.id(), prop); }

    public void remove_property(StringID prop_id) throws Exception
        { remove_property(prop_id.id()); }

    private native Property get_property(int id) throws Exception;
    private native void set_property(int id, Property prop) throws Exception;
    private native void remove_property(int id) throws Exception;

    public native PropertyIterator get_properties() throws Exception;

    public EdgeIterator get_edges() throws Exception
        { return get_edges(Direction.Any.ordinal(), 0); }
    public EdgeIterator get_edges(StringID tag) throws Exception
        { return get_edges(Direction.Any.ordinal(), tag.id()); }
    public EdgeIterator get_edges(Direction dir) throws Exception
        { return get_edges(dir.ordinal(), 0); }
    public EdgeIterator get_edges(Direction dir, StringID tag) throws Exception
        { return get_edges(dir.ordinal(), tag.id()); }
    private native EdgeIterator get_edges(int dir, int tag) throws Exception;

    public NodeIterator get_neighbors()
        { return get_neighbors(Direction.Any.ordinal(), 0, true); }
    public NodeIterator get_neighbors(boolean unique)
        { return get_neighbors(Direction.Any.ordinal(), 0, unique); }
    public NodeIterator get_neighbors(Direction dir)
        { return get_neighbors(dir.ordinal(), 0, true); }
    public NodeIterator get_neighbors(Direction dir, boolean unique)
        { return get_neighbors(dir.ordinal(), 0, unique); }
    public NodeIterator get_neighbors(StringID tag)
        { return get_neighbors(Direction.Any.ordinal(), tag.id(), true); }
    public NodeIterator get_neighbors(StringID tag, boolean unique)
        { return get_neighbors(Direction.Any.ordinal(), tag.id(), unique); }
    public NodeIterator get_neighbors(Direction dir, StringID tag)
        { return get_neighbors(dir.ordinal(), tag.id(), true); }
    public NodeIterator get_neighbors(Direction dir, StringID tag, boolean unique)
        { return get_neighbors(dir.ordinal(), tag.id(), unique); }

    public Node get_neighbor(StringID tag) throws Exception
        { return get_neighbors(Direction.Any.ordinal(), tag.id(), false).get_current(); }
    public Node get_neighbor(Direction dir, StringID tag) throws Exception
        { return get_neighbors(dir.ordinal(), tag.id(), false).get_current(); }

    private native NodeIterator get_neighbors(int dir, int tag, boolean unique);
}
