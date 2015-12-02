/*
 * Corresponds to the node.h file in Jarvis.
 */

package jarvis;

import java.util.Objects;

public class Node {
    private long jarvisHandle;
    private Node(long handle) { jarvisHandle = handle; }

    // Override equals method to support equal hash on nodes
    // that point to the same object in PM
    @Override
    public boolean equals(Object other)
    {
        Node no = (Node)other;
        return jarvisHandle == no.jarvisHandle;
    }

    @Override
    public int hashCode()
    {
        return Objects.hash(jarvisHandle);
    }

    public enum Direction { Any, Outgoing, Incoming };

    public native String get_tag() throws Exception;
    public native Property get_property(String property) throws Exception;
    public native PropertyIterator get_properties() throws Exception;
    public native EdgeIterator get_edges() throws Exception;
    public native EdgeIterator get_edges(String tag) throws Exception;
    public EdgeIterator get_edges(Direction dir) throws Exception
        { return get_edges(dir.ordinal()); }
    public EdgeIterator get_edges(Direction dir, String tag) throws Exception
        { return get_edges(dir.ordinal(), tag); }
    private native EdgeIterator get_edges(int dir) throws Exception;
    private native EdgeIterator get_edges(int dir, String tag) throws Exception;
    public native void set_property(String id, Property prop) throws Exception;
    public native void remove_property(String name) throws Exception;

    public NodeIterator get_neighbors()
        { return get_neighbors(Direction.Any.ordinal(), null, true); }
    public NodeIterator get_neighbors(boolean unique)
        { return get_neighbors(Direction.Any.ordinal(), null, unique); }
    public NodeIterator get_neighbors(Direction dir)
        { return get_neighbors(dir.ordinal(), null, true); }
    public NodeIterator get_neighbors(Direction dir, boolean unique)
        { return get_neighbors(dir.ordinal(), null, unique); }
    public NodeIterator get_neighbors(String tag)
        { return get_neighbors(Direction.Any.ordinal(), tag, true); }
    public NodeIterator get_neighbors(Direction dir, String tag)
        { return get_neighbors(dir.ordinal(), tag, true); }
    public NodeIterator get_neighbors(Direction dir, String tag, boolean unique)
        { return get_neighbors(dir.ordinal(), tag, unique); }

    private native NodeIterator get_neighbors(int dir, String tag, boolean unique);
}
