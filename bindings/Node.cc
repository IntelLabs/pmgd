
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
        const char* tag = j_node.get_tag().name().c_str();
        return env->NewStringUTF(tag);
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
        if (!j_node.check_property(j_str, result))
            return NULL;

        jclass cls = env->FindClass("Property");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        jobject new_p = env->NewObject(cls, cnstrctr, new Property(result));
        return new_p;
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
