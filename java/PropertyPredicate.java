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
    public PropertyPredicate(String name)
        { newPropertyPredicateNative(name); }
    public PropertyPredicate(String name, Op op, Property v)
        { newPropertyPredicateNative(name, op.ordinal(), v); }
    public PropertyPredicate(String name, Op op, Property v1, Property v2)
        { newPropertyPredicateNative(name, op.ordinal(), v1, v2); }

    private native void newPropertyPredicateNative();
    private native void newPropertyPredicateNative(String name);
    private native void newPropertyPredicateNative(String name, int op,
                                                   Property v);
    private native void newPropertyPredicateNative(String name, int op,
                                                   Property v1, Property v2);

    public void finalize() { dispose(); }
    public native void dispose();
}
