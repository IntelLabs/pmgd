/*
 * Java wrapper for Jarvis PropertyPredicate
 *
 * This object is created from Java and passed in
 * to Jarvis functions that use it. It does not persist in Jarvis.
 */

package jarvis;

public class PropertyPredicate {
    private long jarvisHandle;

    public enum Op { DontCare, Eq, Ne, Gt, Ge, Lt, Le,
                     GeLe, GeLt, GtLe, GtLt };

    public PropertyPredicate()
        { newPropertyPredicateNative(); }

    public PropertyPredicate(StringID prop_id)
        { newPropertyPredicateNative(prop_id.id()); }

    public PropertyPredicate(StringID prop_id, Op op, Property v)
        { newPropertyPredicateNative(prop_id.id(), op.ordinal(), v); }

    public PropertyPredicate(StringID prop_id, Op op, Property v1, Property v2)
        { newPropertyPredicateNative(prop_id.id(), op.ordinal(), v1, v2); }

    public PropertyPredicate(StringID prop_id, String s) throws Exception
        { this(prop_id, Op.Eq, new Property(s)); }

    public PropertyPredicate(StringID prop_id, long v) throws Exception
        { this(prop_id, Op.Eq, new Property(v)); }

    private native void newPropertyPredicateNative();
    private native void newPropertyPredicateNative(int prop_id);
    private native void newPropertyPredicateNative
            (int prop_id, int op, Property v);
    private native void newPropertyPredicateNative
            (int prop_id, int op, Property v1, Property v2);

    public void finalize() { dispose(); }
    public native void dispose();
}
