/*
 * Corresponds to the transactions.h file in Jarvis.
 *
 * Notes:
 *  - need to catch and throw exceptions up
 */

public class Transaction {
    private long jarvisHandle;

    public Transaction(Graph db,
                       boolean is_dependent, boolean is_readonly)
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
    private native void startTransactionNative(Graph db, int options);
}
