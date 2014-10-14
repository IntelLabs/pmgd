/*
 * Corresponds to the transactions.h file in Jarvis.
 *
 * Notes:
 */

package jarvis;

public class Transaction {
    private long jarvisHandle;

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

    public native void commit();
    public native void abort();
    public void finalize() { abort(); }

    private native void startTransactionNative(Graph db, int options)
                            throws Exception;
}
