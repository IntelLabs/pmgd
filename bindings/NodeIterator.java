/*
 * Corresponds to the nodeiterator.h file in Jarvis.
 *
 * However, since Java does not allow operator overrides,
 * we cannot treat the operator itself as a
 *
 * Notes:
 *  - need to catch and throw exceptions up
 *  - should implement an abstract class, but this provides framework
 */

public class NodeIterator {
    private long jarvisHandle;
    private Node current;

    public NodeIterator(long handle, Node origin)
    {
        jarvisHandle = handle;
        current = origin;
    }

    public native boolean hasNext(); // replaces the override of bool()

    public Node getNext()
    {
        current = nextNative();
        return current;
    }

    //    public native NodeIterator filter(PropertyPredicate pp);
    //    public native void process();



    // Wrap the functions for node, so we can act on the current
    // Because java doesn't allow overriding operators, we cannot
    //  just let them slide through to jarvis.
    public Node get_current()
    {
        return current;
    }

    public String get_tag()
    {
        return current.get_tag();
    }

    public boolean check_property(String property, Property result)
    {
        return current.check_property(property, result);
    }

    public Property get_property(String property)
    {
        return current.get_property(property);
    }

    public void set_property(String id, Property prop)
    {
        current.set_property(id, prop);
    }

    public void remove_property(String name)
    {
        current.remove_property(name);
    }

    private native Node nextNative();
    public native void dispose();
}
