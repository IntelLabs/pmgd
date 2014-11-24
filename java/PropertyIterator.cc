#include <string.h>
#include <stdio.h>
#include "jarvis.h"
#include "PropertyIterator.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void JNICALL Java_jarvis_PropertyIterator_next(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));
    j_pi.next();
}

jboolean JNICALL Java_jarvis_PropertyIterator_done(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));
    return !bool(j_pi);
}

jobject JNICALL Java_jarvis_PropertyIterator_get_1current(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        Property result = *j_pi;
        return new_java_object(env, "Property", new Property(result));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jstring JNICALL Java_jarvis_PropertyIterator_id(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        return env->NewStringUTF(j_pi->id().name().c_str());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jint JNICALL Java_jarvis_PropertyIterator_type(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->type();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jboolean JNICALL Java_jarvis_PropertyIterator_bool_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->bool_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jlong JNICALL Java_jarvis_PropertyIterator_int_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->int_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jstring JNICALL Java_jarvis_PropertyIterator_string_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        return env->NewStringUTF(j_pi->string_value().c_str());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jdouble JNICALL Java_jarvis_PropertyIterator_float_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getJarvisHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->float_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

void Java_jarvis_PropertyIterator_dispose(JNIEnv *env, jobject pi)
{
    PropertyIterator *j_pi = getJarvisHandle<PropertyIterator>(env, pi);
    delete j_pi;
    setJarvisHandle(env, pi, static_cast<PropertyIterator *>(NULL));
}
