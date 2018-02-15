/**
 * @file   Node.cc
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
#include "neighbor.h"
#include "Node.h"
#include "common.h"

using namespace PMGD;

jobject Java_pmgd_Node_get_1tag(JNIEnv *env, jobject node)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    try {
        return new_java_stringid(env, j_node.get_tag());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


jobject Java_pmgd_Node_get_1property(JNIEnv *env, jobject node, jint id)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    try {
        Property result;
        if (j_node.check_property(id, result))
            return new_java_property(env, new Property(result));
        else
            return NULL;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject Java_pmgd_Node_get_1properties(JNIEnv *env, jobject node)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    try {
        return java_property_iterator(env, j_node.get_properties());
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

jobject JNICALL Java_pmgd_Node_get_1edges
    (JNIEnv *env, jobject node, jint dir, jint tag)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    try {
        return java_edge_iterator(env, j_node.get_edges(Direction(dir), tag));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}


void Java_pmgd_Node_set_1property
    (JNIEnv *env, jobject node, jint id, jobject prop)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    Property &j_prop = *(getPMGDHandle<Property>(env, prop));
    try {
        j_node.set_property(id, j_prop);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_Node_remove_1property
    (JNIEnv *env, jobject node, jint id)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    try {
        j_node.remove_property(id);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

jobject JNICALL Java_pmgd_Node_get_1neighbors
  (JNIEnv *env, jobject node, jint dir, jint tag, jboolean unique)
{
    Node &j_node = *(getPMGDHandle<Node>(env, node));
    try {
        return java_node_iterator(env,
                   get_neighbors(j_node, Direction(dir), tag, unique));
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}
