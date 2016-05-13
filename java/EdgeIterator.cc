#include "jarvis.h"
#include "EdgeIterator.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void JNICALL Java_jarvis_EdgeIterator_next(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    j_ei.next();
}

jboolean JNICALL Java_jarvis_EdgeIterator_done (JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    return !bool(j_ei);
}

jobject JNICALL Java_jarvis_EdgeIterator_get_1current(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));

    try {
        return new_java_edge(env, *j_ei);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_jarvis_EdgeIterator_get_1tag(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        return new_java_stringid(env, j_ei->get_tag());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_jarvis_EdgeIterator_get_1source(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        return new_java_node(env, j_ei->get_source());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_jarvis_EdgeIterator_get_1destination(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        return new_java_node(env, j_ei->get_destination());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_jarvis_EdgeIterator_get_1property
    (JNIEnv *env, jobject ei, jint id)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        Property result;
        if (j_ei->check_property(id, result))
            return new_java_property(env, new Property(result));
        else
            return NULL;
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_jarvis_EdgeIterator_get_1properties(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        return java_property_iterator(env, j_ei->get_properties());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

void JNICALL Java_jarvis_EdgeIterator_set_1property
    (JNIEnv *env, jobject ei, jint id, jobject value)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    Property &j_value = *(getJarvisHandle<Property>(env, value));

    try {
        j_ei->set_property(id, j_value);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void JNICALL Java_jarvis_EdgeIterator_remove_1property
    (JNIEnv *env, jobject ei, jint id)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        j_ei->remove_property(id);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void JNICALL Java_jarvis_EdgeIterator_dispose(JNIEnv *env, jobject ei)
{
    EdgeIterator *j_ei = getJarvisHandle<EdgeIterator>(env, ei);
    delete j_ei;
    setJarvisHandle(env, ei, static_cast<EdgeIterator *>(NULL));
}
