
#include <string.h>
#include <stdio.h>

#include "jarvis.h"

#include "Edge.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jstring Java_jarvis_Edge_get_1tag(JNIEnv *env, jobject edge)
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

jobject Java_jarvis_Edge_get_1source(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        return new_java_node(env, j_edge.get_source());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_jarvis_Edge_get_1destination(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    try {
        return new_java_node(env, j_edge.get_destination());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_jarvis_Edge_get_1property(JNIEnv *env, jobject edge, jstring str)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env, edge));
    const char *j_str = env->GetStringUTFChars(str, 0);
    try {
        Property result;
        if (j_edge.check_property(j_str, result))
            return new_java_property(env, new Property(result));
        else
            return NULL;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_jarvis_Edge_get_1properties(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getJarvisHandle<Edge>(env,edge));
    try {
        return java_property_iterator(env, j_edge.get_properties());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

void Java_jarvis_Edge_set_1property(JNIEnv *env, jobject edge,
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

void Java_jarvis_Edge_remove_1property(JNIEnv *env, jobject edge,
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
