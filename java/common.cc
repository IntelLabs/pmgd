#include <string.h>
#include "jarvis.h"
#include "common.h"

using namespace Jarvis;

THREAD jobject java_transaction;

static void add_node_iterator(JNIEnv *env, jobject i);
static void add_edge_iterator(JNIEnv *env, jobject i);
static void add_property_iterator(JNIEnv *env, jobject i);


jobject new_java_node(JNIEnv *env, Node &obj)
{
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/Node"));
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/Edge"));
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/Property"));
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/NodeIterator"));
        ctor = env->GetMethodID(cls, "<init>", "(JLjarvis/Node;)V");
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/EdgeIterator"));
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/PropertyIterator"));
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/Transaction"));
        method = env->GetMethodID(cls, "add_iterator", "(Ljarvis/NodeIterator;)V");
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/Transaction"));
        method = env->GetMethodID(cls, "add_iterator", "(Ljarvis/EdgeIterator;)V");
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
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/Transaction"));
        method = env->GetMethodID(cls, "add_iterator", "(Ljarvis/PropertyIterator;)V");
        assert(method != 0);
    }
    assert(java_transaction != 0);
    env->CallVoidMethod(java_transaction, method, i);
}


template <>
Jarvis::Graph *getJarvisHandle<Jarvis::Graph>(JNIEnv *env, jobject obj)
{
    static jfieldID id = 0;
    if (id == 0) {
        jclass c = env->GetObjectClass(obj);
        id = env->GetFieldID(c, "jarvisHandle", "J");
    }
    jlong handle = env->GetLongField(obj, id);
    return reinterpret_cast<Jarvis::Graph *>(handle);
}

template <>
Jarvis::Node *getJarvisHandle<Jarvis::Node>(JNIEnv *env, jobject obj)
{
    static jfieldID id = 0;
    if (id == 0) {
        jclass c = env->GetObjectClass(obj);
        id = env->GetFieldID(c, "jarvisHandle", "J");
    }
    jlong handle = env->GetLongField(obj, id);
    return reinterpret_cast<Jarvis::Node *>(handle);
}

template <>
Jarvis::NodeIterator *getJarvisHandle<Jarvis::NodeIterator>(JNIEnv *env, jobject obj)
{
    static jfieldID id = 0;
    if (id == 0) {
        jclass c = env->GetObjectClass(obj);
        id = env->GetFieldID(c, "jarvisHandle", "J");
    }
    jlong handle = env->GetLongField(obj, id);
    return reinterpret_cast<Jarvis::NodeIterator *>(handle);
}

template <>
Jarvis::EdgeIterator *getJarvisHandle<Jarvis::EdgeIterator>(JNIEnv *env, jobject obj)
{
    static jfieldID id = 0;
    if (id == 0) {
        jclass c = env->GetObjectClass(obj);
        id = env->GetFieldID(c, "jarvisHandle", "J");
    }
    jlong handle = env->GetLongField(obj, id);
    return reinterpret_cast<Jarvis::EdgeIterator *>(handle);
}
