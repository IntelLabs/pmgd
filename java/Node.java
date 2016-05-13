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
