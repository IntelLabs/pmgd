
#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "Graph.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void Java_Graph_dumpGraph(JNIEnv *env, jobject graph)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        dump_nodes(j_db);
        dump_edges(j_db);
    }
    catch (Exception e) {
        print_exception(e);
    }
}

jint Java_Graph_get_1id__LNode_2(JNIEnv *env, jobject graph, jobject node)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    return j_db.get_id(j_node);
}

jint Java_Graph_get_1id__LEdge_2(JNIEnv *env, jobject graph, jobject edge)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    return j_db.get_id(j_edge);
}

jobject JNICALL Java_Graph_get_1nodes(JNIEnv *env, jobject graph)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        NodeIterator *j_ni = new NodeIterator(j_db.get_nodes());
        Node *j_n = &(**j_ni);

        // build node to return
        jclass cls = env->FindClass("Node");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        jobject cur = NULL;
        cur = env->NewObject(cls, cnstrctr,
                             reinterpret_cast<jlong>(j_n));

        // create a java nodeiterator
        cls = env->FindClass("NodeIterator");
        cnstrctr = env->GetMethodID(cls, "<init>", "(JLNode;)V");
        jobject ni = env->NewObject(cls, cnstrctr,
                                    reinterpret_cast<jlong>(j_ni),
                                    cur);
        return ni;
    }
    catch (Exception e) {
        print_exception(e);
        return NULL;
    }
}

void Java_Graph_loadGraphNative(JNIEnv *env, jobject obj,
                                jstring filename, jint options)
{
    const char *db_name = env->GetStringUTFChars(filename, 0);
    try {
        Graph *db = new Graph(db_name, options);
        setJarvisHandle(env, obj, db);
    }
    catch (Exception e) {
        print_exception(e);
    }
    env->ReleaseStringUTFChars(filename, db_name);
}

void Java_Graph_addNodeNative(JNIEnv *env, jobject graph, 
                              jobject node, jstring tag)
{
    const char *j_tag = env->GetStringUTFChars(tag, 0);
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    try {
        Node &j_node = j_db.add_node(j_tag);
        setJarvisHandle(env, node, &j_node);
    }
    catch (Exception e) {
        print_exception(e);
    }
}

void Java_Graph_addEdgeNative(JNIEnv *env, jobject graph,
                              jobject edge, jobject src,
                              jobject dest, jstring tag)
{
    const char *j_tag = env->GetStringUTFChars(tag, 0);
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Node &j_src = *(getJarvisHandle<Node>(env, src));
    Node &j_dest = *(getJarvisHandle<Node>(env, dest));
    try {
        Edge &j_edge = j_db.add_edge(j_src, j_dest, j_tag);
        setJarvisHandle(env, edge, &j_edge);
    }
    catch (Exception e) {
        print_exception(e);
    }
}

void Java_Graph_remove__LNode_2(JNIEnv *env, jobject graph, jobject node)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        j_db.remove(j_node);
    }
    catch (Exception e) {
        print_exception(e);
    }
}

void Java_Graph_remove__LEdge_2(JNIEnv *env, jobject graph, jobject edge)
{
    Graph &j_db = *(getJarvisHandle<Graph>(env, graph));
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        j_db.remove(j_edge);
    }
    catch (Exception e) {
        print_exception(e);
    }
}
