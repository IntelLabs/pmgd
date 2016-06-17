/*
 * Corresponds to the transactions.h file in Jarvis.
 *
 * Notes:
 */

package jarvis;

import java.util.Collection;
import java.util.ArrayList;

public class Transaction {
    private long jarvisHandle;
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
