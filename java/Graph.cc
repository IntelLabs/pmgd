/**
 * @file   Graph.cc
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
