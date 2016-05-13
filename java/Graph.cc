#include <string.h>
#include "jarvis.h"
#include "Graph.h"
#include "common.h"

using namespace Jarvis;


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
