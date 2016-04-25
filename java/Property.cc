#include <string.h>
#include <stdio.h>

#include "jarvis.h"

#include "Property.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jint Java_jarvis_Property_type(JNIEnv *env, jobject prop)
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

jboolean Java_jarvis_Property_bool_1value(JNIEnv *env, jobject prop)
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

jlong Java_jarvis_Property_int_1value(JNIEnv *env , jobject prop)
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

jstring Java_jarvis_Property_string_1value(JNIEnv *env, jobject prop)
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

jdouble Java_jarvis_Property_float_1value(JNIEnv *env, jobject prop)
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

jobject Java_jarvis_Property_time_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getJarvisHandle<Property>(env, prop));
    try {
        Time j_t = j_prop.time_value();
        jclass cls = env->FindClass("jarvis/Property$Time");
        if (!cls)
            return 0;
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(Ljarvis/Property;)V");
        if (!cnstrctr)
            return 0;
        jobject t = env->NewObject(cls, cnstrctr);
        env->SetLongField(t, env->GetFieldID(cls, "time_val", "J"), j_t.time_val);
        env->SetIntField(t, env->GetFieldID(cls, "year", "I"), j_t.year);
        env->SetIntField(t, env->GetFieldID(cls, "mon", "I"), j_t.mon);
        env->SetIntField(t, env->GetFieldID(cls, "day", "I"), j_t.day);
        env->SetIntField(t, env->GetFieldID(cls, "hour", "I"), j_t.hour);
        env->SetIntField(t, env->GetFieldID(cls, "min", "I"), j_t.min);
        env->SetIntField(t, env->GetFieldID(cls, "sec", "I"), j_t.sec);
        env->SetIntField(t, env->GetFieldID(cls, "usec", "I"), j_t.usec);
        env->SetIntField(t, env->GetFieldID(cls, "tz_hour", "I"), j_t.tz_hour);
        env->SetIntField(t, env->GetFieldID(cls, "tz_min", "I"), j_t.tz_min*15);
        return t;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

void Java_jarvis_Property_newPropertyNative__(JNIEnv *env, jobject prop)
{
    try {
        Property *j_prop = new Property();
        setJarvisHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_Property_newPropertyNative__Z(JNIEnv *env, jobject prop,
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

void Java_jarvis_Property_newPropertyNative__I(JNIEnv *env, jobject prop,
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

void Java_jarvis_Property_newPropertyNative__Ljava_lang_String_2(JNIEnv *env, jobject prop, jstring v)
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

void Java_jarvis_Property_newPropertyNative__D(JNIEnv *env, jobject prop,
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

void Java_jarvis_Property_dispose(JNIEnv *env, jobject prop)
{
    Property *j_prop = getJarvisHandle<Property>(env, prop);
    delete j_prop;
    setJarvisHandle(env, prop, static_cast<Property *>(NULL));
}
