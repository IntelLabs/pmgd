#include <string.h>
#include <stdio.h>

#include "jarvis.h"
#include "neighbor.h"

#include "Node.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jobject Java_jarvis_Node_get_1tag(JNIEnv *env, jobject node)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        return new_java_stringid(env, j_node.get_tag());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject Java_jarvis_Node_get_1property(JNIEnv *env, jobject node, jint id)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        Property result;
        if (j_node.check_property(id, result))
            return new_java_property(env, new Property(result));
        else
            return NULL;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_jarvis_Node_get_1properties(JNIEnv *env, jobject node)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        return java_property_iterator(env, j_node.get_properties());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_jarvis_Node_get_1edges
    (JNIEnv *env, jobject node, jint dir, jint tag)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        return java_edge_iterator(env, j_node.get_edges(Direction(dir), tag));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


void Java_jarvis_Node_set_1property
    (JNIEnv *env, jobject node, jint id, jobject prop)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        j_node.set_property(id, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_Node_remove_1property
    (JNIEnv *env, jobject node, jint id)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        j_node.remove_property(id);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

jobject JNICALL Java_jarvis_Node_get_1neighbors
  (JNIEnv *env, jobject node, jint dir, jint tag, jboolean unique)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        return java_node_iterator(env,
                   get_neighbors(j_node, Direction(dir), tag, unique));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}
