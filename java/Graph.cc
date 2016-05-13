
#include <string.h>
#include <stdio.h>

#include "jarvis.h"

#include "Graph.h"
#include "jarvisHandles.h"

using namespace Jarvis;

THREAD jobject java_transaction;

static void add_node_iterator(JNIEnv *env, jobject i);
static void add_edge_iterator(JNIEnv *env, jobject i);
static void add_property_iterator(JNIEnv *env, jobject i);

jlong Java_jarvis_Graph_get_1id__Ljarvis_Node_2(JNIEnv *env, jobject graph, jobject node)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    return j_db.get_id(j_node);
}

jlong Java_jarvis_Graph_get_1id__Ljarvis_Edge_2(JNIEnv *env, jobject graph, jobject edge)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    return j_db.get_id(j_edge);
}

jobject JNICALL Java_jarvis_Graph_get_1nodes__(JNIEnv *env, jobject graph)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        return java_node_iterator(env, j_db.get_nodes());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_jarvis_Graph_get_1nodes__I(JNIEnv *env,
                    jobject graph, jint tag)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        return java_node_iterator(env, j_db.get_nodes(tag));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_jarvis_Graph_get_1nodes__ILjarvis_PropertyPredicate_2Z
    (JNIEnv *env, jobject graph, jint tag, jobject pp, jboolean reverse)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    PropertyPredicate &j_pp = *(getJarvisHandle<PropertyPredicate>(env, pp));
    try {
        return java_node_iterator(env, j_db.get_nodes(tag, j_pp, reverse));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_jarvis_Graph_get_1edges__(JNIEnv *env, jobject graph)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        return java_edge_iterator(env, j_db.get_edges());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_jarvis_Graph_get_1edges__I(JNIEnv *env,
                    jobject graph, jint tag)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        return java_edge_iterator(env, j_db.get_edges(tag));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_jarvis_Graph_get_1edges__ILjarvis_PropertyPredicate_2Z
    (JNIEnv *env, jobject graph, jint tag, jobject pp, jboolean reverse)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    PropertyPredicate &j_pp = *(getJarvisHandle<PropertyPredicate>(env, pp));
    try {
        return java_edge_iterator(env, j_db.get_edges(tag, j_pp, reverse));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

void Java_jarvis_Graph_loadGraphNative(JNIEnv *env, jobject obj,
                                jstring filename, jint options)
{
    const char *db_name = env->GetStringUTFChars(filename, 0);
    try {
        Graph *db = new Graph(db_name, options);
        setJarvisHandle(env, obj, db);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
    env->ReleaseStringUTFChars(filename, db_name);
}

jobject JNICALL Java_jarvis_Graph_add_1node(JNIEnv *env, jobject graph, jint tag)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        return new_java_node(env, j_db.add_node(tag));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_jarvis_Graph_add_1edge(JNIEnv *env, jobject graph,
                                     jobject src, jobject dest, jint tag)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Node &j_src = *(getJarvisHandle<Node>(env, src));
    Node &j_dest = *(getJarvisHandle<Node>(env, dest));
    try {
        return new_java_edge(env, j_db.add_edge(j_src, j_dest, tag));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

void Java_jarvis_Graph_remove__Ljarvis_Node_2(JNIEnv *env, jobject graph, jobject node)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        j_db.remove(j_node);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_Graph_remove__Ljarvis_Edge_2(JNIEnv *env, jobject graph, jobject edge)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        j_db.remove(j_edge);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_Graph_dispose(JNIEnv *env, jobject graph)
{
    Graph *j_db = getJarvisHandle<Graph>(env, graph);
    delete j_db;
    setJarvisHandle(env, graph, static_cast<Graph *>(NULL));
}

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
