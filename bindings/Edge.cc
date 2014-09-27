
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
        StringID tag = j_edge.get_tag();
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

jobject Java_Edge_get_1source(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        Node &j_src = j_edge.get_source();
        return new_java_object(env, "Node", &j_src);
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
        return new_java_object(env, "Node", &j_dest);
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
        if (j_edge.check_property(j_str, result))
            return new_java_object(env, "Property", new Property(result));
        else
            return NULL;
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
        return new_java_object(env, "PropertyIterator", j_pi);
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
