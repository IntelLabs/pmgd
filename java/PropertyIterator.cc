/**
 * @file   PropertyIterator.cc
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
#include "PropertyIterator.h"
#include "common.h"

using namespace PMGD;

void JNICALL Java_pmgd_PropertyIterator_next(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));
    j_pi.next();
}

jboolean JNICALL Java_pmgd_PropertyIterator_done(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));
    return !bool(j_pi);
}

jobject JNICALL Java_pmgd_PropertyIterator_get_1current(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        Property result = *j_pi;
        return new_java_property(env, new Property(result));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jstring JNICALL Java_pmgd_PropertyIterator_id(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        return env->NewStringUTF(j_pi->id().name().c_str());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jint JNICALL Java_pmgd_PropertyIterator_type(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        return int(j_pi->type());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jboolean JNICALL Java_pmgd_PropertyIterator_bool_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->bool_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jlong JNICALL Java_pmgd_PropertyIterator_int_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->int_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jstring JNICALL Java_pmgd_PropertyIterator_string_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        return env->NewStringUTF(j_pi->string_value().c_str());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

jdouble JNICALL Java_pmgd_PropertyIterator_float_1value(JNIEnv *env, jobject pi)
{
    PropertyIterator &j_pi = *(getPMGDHandle<PropertyIterator>(env, pi));

    try {
        return j_pi->float_value();
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return 0;
    }
}

void Java_pmgd_PropertyIterator_dispose(JNIEnv *env, jobject pi)
{
    PropertyIterator *j_pi = getPMGDHandle<PropertyIterator>(env, pi);
    delete j_pi;
    setPMGDHandle(env, pi, static_cast<PropertyIterator *>(NULL));
}
