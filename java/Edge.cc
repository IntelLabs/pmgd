/**
 * @file   Edge.cc
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
#include "Edge.h"
#include "common.h"

using namespace PMGD;

jobject Java_pmgd_Edge_get_1tag(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env, edge));
    try {
        return new_java_stringid(env, j_edge.get_tag());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_pmgd_Edge_get_1source(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env, edge));
    try {
        return new_java_node(env, j_edge.get_source());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_pmgd_Edge_get_1destination(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env, edge));
    try {
        return new_java_node(env, j_edge.get_destination());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_pmgd_Edge_get_1property(JNIEnv *env, jobject edge, jint id)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env, edge));
    try {
        Property result;
        if (j_edge.check_property(id, result))
            return new_java_property(env, new Property(result));
        else
            return NULL;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_pmgd_Edge_get_1properties(JNIEnv *env, jobject edge)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env,edge));
    try {
        return java_property_iterator(env, j_edge.get_properties());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

void Java_pmgd_Edge_set_1property
    (JNIEnv *env, jobject edge, jint id, jobject prop)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env, edge));
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        j_edge.set_property(id, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Edge_remove_1property(JNIEnv *env, jobject edge, jint id)
{
    Edge &j_edge = *(getPMGDHandle<Edge>(env, edge));
    try {
        j_edge.remove_property(id);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}
