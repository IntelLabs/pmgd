/*
 * Java wrapper for Jarvis NodeIterator
 */

public class NodeIterator {
    private long jarvisHandle;
    private Node current;

    private NodeIterator(long handle, Node origin)
    {
        jarvisHandle = handle;
        current = origin;
    }

    public boolean done() //replaces the override of bool()
        { return current == null; }

    public native void next();

    //    public native NodeIterator filter(PropertyPredicate pp);
    //    public native void process();



    // Wrap the functions for node, so we can act on the current
    // Because java doesn't allow overriding operators, we cannot
    //  just let them slide through to jarvis.
    public Node get_current()
    {
        return current;
    }

    public String get_tag() throws Exception
    {
        return current.get_tag();
    }

    public Property get_property(String property) throws Exception
    {
        return current.get_property(property);
    }

    public void set_property(String id, Property prop) throws Exception
    {
        current.set_property(id, prop);
    }

    public void remove_property(String name) throws Exception
    {
        current.remove_property(name);
    }

    public native void dispose();
}
