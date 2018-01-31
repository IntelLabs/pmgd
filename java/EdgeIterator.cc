/**
 * @file   EdgeIterator.cc
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

#include "pmgd.h"
#include "EdgeIterator.h"
#include "common.h"

using namespace PMGD;

void JNICALL Java_pmgd_EdgeIterator_next(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    j_ei.next();
}

jboolean JNICALL Java_pmgd_EdgeIterator_done (JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    return !bool(j_ei);
}

jobject JNICALL Java_pmgd_EdgeIterator_get_1current(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));

    try {
        return new_java_edge(env, *j_ei);
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_pmgd_EdgeIterator_get_1tag(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    try {
        return new_java_stringid(env, j_ei->get_tag());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_pmgd_EdgeIterator_get_1source(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    try {
        return new_java_node(env, j_ei->get_source());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_pmgd_EdgeIterator_get_1destination(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    try {
        return new_java_node(env, j_ei->get_destination());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}


jobject JNICALL Java_pmgd_EdgeIterator_get_1property
    (JNIEnv *env, jobject ei, jint id)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
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

jobject JNICALL Java_pmgd_EdgeIterator_get_1properties(JNIEnv *env, jobject ei)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    try {
        return java_property_iterator(env, j_ei->get_properties());
    }
    catch (Exception e){
        JavaThrow(env, e);
        return NULL;
    }
}

void JNICALL Java_pmgd_EdgeIterator_set_1property
    (JNIEnv *env, jobject ei, jint id, jobject value)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    Property &j_value = *(getPMGDHandle<Property>(env, value));

    try {
        j_ei->set_property(id, j_value);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void JNICALL Java_pmgd_EdgeIterator_remove_1property
    (JNIEnv *env, jobject ei, jint id)
{
    EdgeIterator &j_ei = *(getPMGDHandle<EdgeIterator>(env, ei));
    try {
        j_ei->remove_property(id);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void JNICALL Java_pmgd_EdgeIterator_dispose(JNIEnv *env, jobject ei)
{
    EdgeIterator *j_ei = getPMGDHandle<EdgeIterator>(env, ei);
    delete j_ei;
    setPMGDHandle(env, ei, static_cast<EdgeIterator *>(NULL));
}
