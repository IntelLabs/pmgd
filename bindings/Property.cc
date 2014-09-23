#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "Property.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jint Java_Property_type(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        PropertyType pt = j_prop.type();
        return (int)pt;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jboolean Java_Property_bool_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        bool j_bool = j_prop.bool_value();
        return j_bool;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return false;
    }
}

jlong Java_Property_int_1value(JNIEnv *env , jobject prop)
{
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        long j_int = j_prop.int_value();
        return j_int;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jstring Java_Property_string_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        const char* j_str = j_prop.string_value().c_str();
        return env->NewStringUTF(j_str);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jdouble Java_Property_float_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        return j_prop.float_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

void Java_Property_newPropertyNative__(JNIEnv *env, jobject prop)
{
    try {
        Property *j_prop = new Property();
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Property_newPropertyNative__LProperty_2(JNIEnv *env, jobject prop,
                                                  jobject prop_ref)
{
    Property &j_prop_ref = *(getJarvisHandle<Property>(env, prop_ref));
    try {
        Property *j_prop = new Property(j_prop_ref);
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Property_newPropertyNative__Z(JNIEnv *env, jobject prop,
                                        jboolean v)
{
    bool j_v = (bool)v;
    try {
        Property *j_prop = new Property(j_v);
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Property_newPropertyNative__I(JNIEnv *env, jobject prop,
                                        jint v)
{
    int j_v = (int) v;
    try {
        Property *j_prop = new Property(j_v);
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Property_newPropertyNative__Ljava_lang_String_2(JNIEnv *env, jobject prop, jstring v)
{
    const char *j_v = env->GetStringUTFChars(v, 0);
    try {
        Property *j_prop = new Property(j_v);
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Property_newPropertyNative__D(JNIEnv *env, jobject prop,
                                        jdouble v)
{
    double j_v = (double) v;
    try {
        Property *j_prop = new Property(j_v);
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_Property_dispose(JNIEnv *env, jobject prop)
{
    Property *j_prop = getJarvisHandle<Property>(env, prop);
    delete j_prop;
    setJarvisHandle(env, prop, static_cast<Property *>(NULL));
}
