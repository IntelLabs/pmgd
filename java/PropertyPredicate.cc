/**
 * @file   PropertyPredicate.cc
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

#include "jarvis.h"
#include "common.h"
#include "PropertyPredicate.h"

using namespace Jarvis;

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__
    (JNIEnv *env, jobject pp)
{
    PropertyPredicate *j_pp = new PropertyPredicate();
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__I
    (JNIEnv *env, jobject pp, jint id)
{
    PropertyPredicate *j_pp = new PropertyPredicate(id);
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__IILjarvis_Property_2
    (JNIEnv *env, jobject pp, jint id, jint op, jobject v)
{
    PropertyPredicate::Op j_op = PropertyPredicate::Op(op);
    Property &j_v = *(getJarvisHandle<Property>(env, v));
    PropertyPredicate *j_pp = new PropertyPredicate(id, j_op, j_v);
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__IILjarvis_Property_2Ljarvis_Property_2
    (JNIEnv *env, jobject pp, jint id, jint op, jobject v1, jobject v2)
{
    PropertyPredicate::Op j_op = PropertyPredicate::Op(op);
    Property &j_v1 = *(getJarvisHandle<Property>(env, v1));
    Property &j_v2 = *(getJarvisHandle<Property>(env, v2));
    PropertyPredicate *j_pp = new PropertyPredicate(id, j_op, j_v1, j_v2);
    setJarvisHandle(env, pp, j_pp);
}

void Java_jarvis_PropertyPredicate_dispose(JNIEnv *env, jobject pp)
{
    PropertyPredicate *j_pp = getJarvisHandle<PropertyPredicate>(env, pp);
    delete j_pp;
    setJarvisHandle(env, pp, static_cast<PropertyPredicate *>(NULL));
}
