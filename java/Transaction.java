/**
 * @file   Transaction.java
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
 * Corresponds to the transactions.h file in PMGD.
 *
 * Notes:
 */

package pmgd;

import java.util.Collection;
import java.util.ArrayList;

public class Transaction {
    private long pmgdHandle;
    private Collection<NodeIterator> node_iterators = new ArrayList<NodeIterator>();
    private Collection<EdgeIterator> edge_iterators = new ArrayList<EdgeIterator>();
    private Collection<PropertyIterator> property_iterators = new ArrayList<PropertyIterator>();

    public Transaction(Graph db, boolean is_dependent, boolean is_readonly)
        throws Exception
    {
        int options = 0;
        // Convert to C enum before going through JNI
        if(!is_dependent)
            options += 1;
        if(!is_readonly)
            options += 2;

        startTransactionNative(db, options);
    }

    public void commit()
    {
        free_iterators();
        commitNative();
    }

    public void abort()
    {
        free_iterators();
        abortNative();
    }

    public void finalize() { abort(); }

    private void add_iterator(NodeIterator i) { node_iterators.add(i); }
    private void add_iterator(EdgeIterator i) { edge_iterators.add(i); }
    private void add_iterator(PropertyIterator i) { property_iterators.add(i); }

    private void free_iterators()
    {
        for (NodeIterator i : node_iterators)
            i.dispose();
        node_iterators.clear();

        for (EdgeIterator i : edge_iterators)
            i.dispose();
        edge_iterators.clear();

        for (PropertyIterator i : property_iterators)
            i.dispose();
        property_iterators.clear();
    }

    private native void startTransactionNative(Graph db, int options)
                            throws Exception;
    private native void commitNative();
    private native void abortNative();
}
