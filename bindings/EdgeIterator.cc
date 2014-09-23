#include "jarvis.h"
#include "EdgeIterator.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void JNICALL Java_EdgeIterator_next(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    j_ei.next();
}

jboolean JNICALL Java_EdgeIterator_done (JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    return !bool(j_ei);
}

jobject JNICALL Java_EdgeIterator_get_1current(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));

    try {
        Edge *result = &static_cast<Edge &>(*j_ei);

        jclass cls = env->FindClass("Edge");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        return env->NewObject(cls, cnstrctr, result);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jstring JNICALL Java_EdgeIterator_get_1tag(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        const char *tag = j_ei->get_tag().name().c_str();
        return env->NewStringUTF(tag);
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_EdgeIterator_get_1property(JNIEnv *env, jobject ei,
                                                jstring name)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    const char *j_name = env->GetStringUTFChars(name, 0);

    try {
        Property result;
        if (!j_ei->check_property(j_name, result))
            return NULL;

        jclass cls = env->FindClass("Property");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        return env->NewObject(cls, cnstrctr, new Property(result));
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_EdgeIterator_get_1properties(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    try {
        PropertyIterator *j_pi = new PropertyIterator(j_ei->get_properties());

        // create a Java PropertyIterator
        jclass cls = env->FindClass("PropertyIterator");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        return env->NewObject(cls, cnstrctr, reinterpret_cast<jlong>(j_pi));
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

void JNICALL Java_EdgeIterator_set_1property(JNIEnv *env, jobject ei,
                                             jstring name, jobject value)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    const char *j_name = env->GetStringUTFChars(name, 0);
    Property &j_value = *(getJarvisHandle<Property>(env, value));

    try {
        j_ei->set_property(j_name, j_value);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void JNICALL Java_EdgeIterator_remove_1property(JNIEnv *env, jobject ei, jstring name)
{
    EdgeIterator &j_ei = *(getJarvisHandle<EdgeIterator>(env, ei));
    const char *j_name = env->GetStringUTFChars(name, 0);
    try {
        j_ei->remove_property(j_name);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void JNICALL Java_EdgeIterator_dispose(JNIEnv *env, jobject ei)
{
    EdgeIterator *j_ei = getJarvisHandle<EdgeIterator>(env, ei);
    delete j_ei;
}
