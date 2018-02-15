/**
 * @file   Property.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <string.h>
#include "pmgd.h"
#include "Property.h"
#include "common.h"

using namespace PMGD;

static jclass time_cls = 0;
static jmethodID time_ctor = 0;
static jfieldID time_year_id = 0;
static jfieldID time_mon_id = 0;
static jfieldID time_day_id = 0;
static jfieldID time_hour_id = 0;
static jfieldID time_min_id = 0;
static jfieldID time_sec_id = 0;
static jfieldID time_usec_id = 0;
static jfieldID time_tz_hour_id = 0;
static jfieldID time_tz_min_id = 0;

static void get_time_class(JNIEnv *env);


jint Java_pmgd_Property_type(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        PropertyType pt = j_prop.type();
        return (int)pt;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jboolean Java_pmgd_Property_bool_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        bool j_bool = j_prop.bool_value();
        return j_bool;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return false;
    }
}

jlong Java_pmgd_Property_int_1value(JNIEnv *env , jobject prop)
{
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        long j_int = j_prop.int_value();
        return j_int;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jstring Java_pmgd_Property_string_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        const char* j_str = j_prop.string_value().c_str();
        return env->NewStringUTF(j_str);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jdouble Java_pmgd_Property_float_1value(JNIEnv *env, jobject prop)
{
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        return j_prop.float_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jobject Java_pmgd_Property_time_1value(JNIEnv *env, jobject prop)
{
    get_time_class(env);
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        Time j_t = j_prop.time_value();
        jobject t = env->NewObject(time_cls, time_ctor);
        env->SetIntField(t, time_year_id, j_t.year);
        env->SetIntField(t, time_mon_id, j_t.mon);
        env->SetIntField(t, time_day_id, j_t.day);
        env->SetIntField(t, time_hour_id, j_t.hour);
        env->SetIntField(t, time_min_id, j_t.min);
        env->SetIntField(t, time_sec_id, j_t.sec);
        env->SetIntField(t, time_usec_id, j_t.usec);
        env->SetIntField(t, time_tz_hour_id, j_t.tz_hour);
        env->SetIntField(t, time_tz_min_id, j_t.tz_min*15);
        return t;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

void Java_pmgd_Property_newPropertyNative__(JNIEnv *env, jobject prop)
{
    try {
        Property *j_prop = new Property();
        setPMGDHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Property_newPropertyNative__Z(JNIEnv *env, jobject prop,
                                        jboolean v)
{
    bool j_v = (bool)v;
    try {
        Property *j_prop = new Property(j_v);
        setPMGDHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Property_newPropertyNative__J(JNIEnv *env, jobject prop,
                                        jlong v)
{
    long long j_v = (long long) v;
    try {
        Property *j_prop = new Property(j_v);
        setPMGDHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Property_newPropertyNative__Ljava_lang_String_2(JNIEnv *env, jobject prop, jstring v)
{
    const char *j_v = env->GetStringUTFChars(v, 0);
    try {
        Property *j_prop = new Property(j_v);
        setPMGDHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Property_newPropertyNative__D(JNIEnv *env, jobject prop,
                                        jdouble v)
{
    double j_v = (double) v;
    try {
        Property *j_prop = new Property(j_v);
        setPMGDHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Property_newPropertyNative__Lpmgd_Property_00024Time_2
    (JNIEnv *env, jobject prop, jobject time)
{
    get_time_class(env);

    try {
        Time t;
        t.year = env->GetIntField(time, time_year_id);
        t.mon = env->GetIntField(time, time_mon_id);
        t.day = env->GetIntField(time, time_day_id);
        t.hour = env->GetIntField(time, time_hour_id);
        t.min = env->GetIntField(time, time_min_id);
        t.sec = env->GetIntField(time, time_sec_id);
        t.usec = env->GetIntField(time, time_usec_id);
        t.tz_hour = env->GetIntField(time, time_tz_hour_id);
        t.tz_min = env->GetIntField(time, time_tz_min_id) / 15;
        Property *j_prop = new Property(t);
        setPMGDHandle(env, prop, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

static void get_time_class(JNIEnv *env)
{
    if (time_ctor == 0) {
        time_cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/Property$Time"));
        time_ctor = env->GetMethodID(time_cls, "<init>", "()V");
        assert(time_ctor != 0);

        time_year_id = env->GetFieldID(time_cls, "year", "I");
        time_mon_id = env->GetFieldID(time_cls, "mon", "I");
        time_day_id = env->GetFieldID(time_cls, "day", "I");
        time_hour_id = env->GetFieldID(time_cls, "hour", "I");
        time_min_id = env->GetFieldID(time_cls, "min", "I");
        time_sec_id = env->GetFieldID(time_cls, "sec", "I");
        time_usec_id = env->GetFieldID(time_cls, "usec", "I");
        time_tz_hour_id = env->GetFieldID(time_cls, "tz_hour", "I");
        time_tz_min_id = env->GetFieldID(time_cls, "tz_min", "I");
    }
}

void Java_pmgd_Property_dispose(JNIEnv *env, jobject prop)
{
    Property *j_prop = getPMGDHandle<Property>(env, prop);
    delete j_prop;
    setPMGDHandle(env, prop, static_cast<Property *>(NULL));
}
