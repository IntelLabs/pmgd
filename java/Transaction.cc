/**
 * @file   Transaction.cc
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

#include <string.h>
#include "jarvis.h"
#include "Transaction.h"
#include "common.h"

using namespace Jarvis;

void Java_jarvis_Transaction_startTransactionNative(JNIEnv *env, jobject tx,
                                             jobject graph, jint options)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        Transaction *j_tx = new Transaction(j_db, options);
        setJarvisHandle(env, tx, j_tx);
        java_transaction = env->NewGlobalRef(tx);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_Transaction_commitNative(JNIEnv *env, jobject tx)
{
    Transaction &j_tx = *(getJarvisHandle<Transaction>(env, tx));
    try {
        j_tx.commit();
        delete(&j_tx);
        setJarvisHandle(env, tx, static_cast<Transaction *>(NULL));
        env->DeleteGlobalRef(java_transaction);
        java_transaction = NULL;
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_Transaction_abortNative(JNIEnv *env, jobject tx)
{
    Transaction *j_tx = getJarvisHandle<Transaction>(env, tx);
    delete j_tx;
    setJarvisHandle(env, tx, static_cast<Transaction *>(NULL));
    env->DeleteGlobalRef(java_transaction);
    java_transaction = NULL;
}
