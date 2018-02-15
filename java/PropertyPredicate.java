/**
 * @file   PropertyPredicate.java
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
 * Java wrapper for PMGD PropertyPredicate
 *
 * This object is created from Java and passed in
 * to PMGD functions that use it. It does not persist in PMGD.
 */

package pmgd;

public class PropertyPredicate {
    private long pmgdHandle;

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
