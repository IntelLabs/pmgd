/*
 * Corresponds to the stringid.h file in Jarvis.
 */

package jarvis;

public class StringID {
    private int _id;

    private StringID(int id) { _id = id; }
    private native void init(String name);
    private native String nameNative(int id);

    protected int id() { return _id; }

    public StringID() { _id = 0; }
    public StringID(String name) { init(name); }
    public String name() { return nameNative(_id); }

    // Override equals method to support equal hash on nodes
    // that point to the same object in PM
    @Override
    public boolean equals(Object other)
    {
        StringID no = (StringID)other;
        return _id == no._id;
    }
}
