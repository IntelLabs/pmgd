
#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "Transaction.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void Java_Transaction_startTransactionNative(JNIEnv *env, jobject tx,
                                             jobject graph, jint options)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        Transaction *j_tx = new Transaction(j_db, options);
        setJarvisHandle(env, tx, j_tx);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Transaction_commit(JNIEnv *env, jobject tx)
{
    Transaction &j_tx = *(getJarvisHandle<Transaction>(env, tx));
    try {
        j_tx.commit();
        delete(&j_tx);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}
