
#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "Edge.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jstring Java_Edge_get_1tag(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        const char* tag = j_edge.get_tag().name().c_str();
        return env->NewStringUTF(tag);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_Edge_get_1source(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        Node &j_src = j_edge.get_source();

        jclass cls = env->FindClass("Node");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        jobject src = env->NewObject(cls, cnstrctr, &j_src);
        return src;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_Edge_get_1destination(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        Node &j_dest = j_edge.get_destination();

        jclass cls = env->FindClass("Node");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        jobject dest = env->NewObject(cls, cnstrctr, &j_dest);
        return dest;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_Edge_get_1property(JNIEnv *env, jobject edge, jstring str)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        Property result;
        if (!j_edge.check_property(j_str, result))
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

jobject Java_Edge_get_1properties(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env,edge));
    try {
        PropertyIterator *j_pi = new PropertyIterator(j_edge.get_properties());

        // create a Java PropertyIterator
        jclass cls = env->FindClass("PropertyIterator");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        return env->NewObject(cls, cnstrctr, reinterpret_cast<jlong>(j_pi));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

void Java_Edge_set_1property(JNIEnv *env, jobject edge,
                             jstring str, jobject prop)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        j_edge.set_property(j_str, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Edge_remove_1property(JNIEnv *env, jobject edge,
                                jstring str)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        j_edge.remove_property(j_str);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}
