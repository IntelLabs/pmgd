
#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "Node.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jstring Java_Node_get_1tag(JNIEnv *env, jobject node)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        StringID tag = j_node.get_tag();
        if (tag == 0)
            return NULL;
        else
            return env->NewStringUTF(tag.name().c_str());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject Java_Node_get_1property(JNIEnv *env, jobject node, jstring str)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        Property result;
        if (j_node.check_property(j_str, result))
            return new_java_object(env, "Property", new Property(result));
        else
            return NULL;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_Node_get_1properties(JNIEnv *env, jobject node)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        PropertyIterator *j_pi = new PropertyIterator(j_node.get_properties());
        return new_java_object(env, "PropertyIterator", j_pi);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_Node_get_1edges__(JNIEnv *env, jobject node)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    try {
        EdgeIterator *j_ei = new EdgeIterator(j_node.get_edges());
        return new_java_object(env, "EdgeIterator", j_ei);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_Node_get_1edges__LNode_Direction_2(JNIEnv *env,
                    jobject node, jobject dir)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    jclass cls = env->GetObjectClass(dir);
    jmethodID ordinal = env->GetMethodID(cls, "ordinal", "()V");
    int j_dir = env->CallIntMethod(dir, ordinal);
    try {
        EdgeIterator *j_ei = new EdgeIterator(j_node.get_edges(Direction(j_dir)));
        return new_java_object(env, "EdgeIterator", j_ei);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_Node_get_1edges__Ljava_lang_String_2(JNIEnv *env,
                    jobject node, jstring tag)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    const char *j_tag = env->GetStringUTFChars(tag, 0);
    try {
        EdgeIterator *j_ei = new EdgeIterator(j_node.get_edges(j_tag));
        return new_java_object(env, "EdgeIterator", j_ei);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_Node_get_1edges__LNode_Direction_2Ljava_lang_String_2
    (JNIEnv *env, jobject node, jobject dir, jstring tag)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    jclass cls = env->GetObjectClass(dir);
    jmethodID ordinal = env->GetMethodID(cls, "ordinal", "()V");
    int j_dir = env->CallIntMethod(dir, ordinal);
    const char *j_tag = env->GetStringUTFChars(tag, 0);
    try {
        EdgeIterator *j_ei = new EdgeIterator(j_node.get_edges(Direction(j_dir), j_tag));
        return new_java_object(env, "EdgeIterator", j_ei);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


void Java_Node_set_1property(JNIEnv *env, jobject node,
                             jstring str, jobject prop)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        j_node.set_property(j_str, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Node_remove_1property(JNIEnv *env, jobject node,
                                jstring str)
{
    Node &j_node = *(getJarvisHandle<Node>(env, node));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        j_node.remove_property(j_str);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}
