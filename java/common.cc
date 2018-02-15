/**
 * @file   common.cc
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
#include "pmgd.h"
#include "common.h"

using namespace PMGD;

THREAD jobject java_transaction;

static void add_node_iterator(JNIEnv *env, jobject i);
static void add_edge_iterator(JNIEnv *env, jobject i);
static void add_property_iterator(JNIEnv *env, jobject i);


jobject new_java_node(JNIEnv *env, Node &obj)
{
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Node"));
        ctor = env->GetMethodID(cls, "<init>", "(J)V");
        assert(ctor != 0);
    }
    return env->NewObject(cls, ctor, reinterpret_cast<jlong>(&obj));
}

jobject new_java_edge(JNIEnv *env, Edge &obj)
{
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Edge"));
        ctor = env->GetMethodID(cls, "<init>", "(J)V");
        assert(ctor != 0);
    }
    return env->NewObject(cls, ctor, reinterpret_cast<jlong>(&obj));
}

jobject new_java_property(JNIEnv *env, Property *obj)
{
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Property"));
        ctor = env->GetMethodID(cls, "<init>", "(JZ)V");
        assert(ctor != 0);
    }
    return env->NewObject(cls, ctor, reinterpret_cast<jlong>(obj), false);
}

jobject java_node_iterator(JNIEnv *env, NodeIterator &&ni)
{
    NodeIterator *j_ni = new NodeIterator(std::move(ni));

    jobject cur;
    if (*j_ni) {
        cur = new_java_node(env, **j_ni);
    }
    else
        cur = NULL;

    // create a java nodeiterator
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/NodeIterator"));
        ctor = env->GetMethodID(cls, "<init>", "(JLpmgd/Node;)V");
        assert(ctor != 0);
    }

    jobject i = env->NewObject(cls, ctor, reinterpret_cast<jlong>(j_ni), cur);
    add_node_iterator(env, i);
    return i;
}

jobject java_edge_iterator(JNIEnv *env, EdgeIterator &&ei)
{
    EdgeIterator *j_ei = new EdgeIterator(std::move(ei));

    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/EdgeIterator"));
        ctor = env->GetMethodID(cls, "<init>", "(J)V");
        assert(ctor != 0);
    }

    jobject i = env->NewObject(cls, ctor, reinterpret_cast<jlong>(j_ei));
    add_edge_iterator(env, i);
    return i;
}

jobject java_property_iterator(JNIEnv *env, PropertyIterator &&pi)
{
    PropertyIterator *j_pi = new PropertyIterator(std::move(pi));

    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/PropertyIterator"));
        ctor = env->GetMethodID(cls, "<init>", "(J)V");
        assert(ctor != 0);
    }

    jobject i = env->NewObject(cls, ctor, reinterpret_cast<jlong>(j_pi));
    add_property_iterator(env, i);
    return i;
}

static void add_node_iterator(JNIEnv *env, jobject i)
{
    static jclass cls = 0;
    static jmethodID method = 0;
    if (method == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Transaction"));
        method = env->GetMethodID(cls, "add_iterator", "(Lpmgd/NodeIterator;)V");
        assert(method != 0);
    }
    assert(java_transaction != 0);
    env->CallVoidMethod(java_transaction, method, i);
}

static void add_edge_iterator(JNIEnv *env, jobject i)
{
    static jclass cls = 0;
    static jmethodID method = 0;
    if (method == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Transaction"));
        method = env->GetMethodID(cls, "add_iterator", "(Lpmgd/EdgeIterator;)V");
        assert(method != 0);
    }
    assert(java_transaction != 0);
    env->CallVoidMethod(java_transaction, method, i);
}

static void add_property_iterator(JNIEnv *env, jobject i)
{
    static jclass cls = 0;
    static jmethodID method = 0;
    if (method == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Transaction"));
        method = env->GetMethodID(cls, "add_iterator", "(Lpmgd/PropertyIterator;)V");
        assert(method != 0);
    }
    assert(java_transaction != 0);
    env->CallVoidMethod(java_transaction, method, i);
}
