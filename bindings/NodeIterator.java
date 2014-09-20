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



    private void null_check(String method) throws Exception
    {
        if (current == null)
            throw new Exception(2, "null_iterator",
                                "NodeIterator." + method, null);
    }

    // Wrap the functions for node, so we can act on the current
    // Because java doesn't allow overriding operators, we cannot
    //  just let them slide through to jarvis.
    public Node get_current() throws Exception
    {
        null_check("get_current");
        return current;
    }

    public String get_tag() throws Exception
    {
        null_check("get_tag");
        return current.get_tag();
    }

    public Property get_property(String property) throws Exception
    {
        null_check("get_property");
        return current.get_property(property);
    }

    public void set_property(String id, Property prop) throws Exception
    {
        null_check("set_property");
        current.set_property(id, prop);
    }

    public void remove_property(String name) throws Exception
    {
        null_check("remove_property");
        current.remove_property(name);
    }

    public native void dispose();
}
