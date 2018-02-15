/**
 * @file   common.h
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

#ifndef JAVA_COMMON_H
#define JAVA_COMMON_H

#include <jni.h>
#include "jarvis.h"
#include "../src/compiler.h"

extern THREAD jobject java_transaction;

template <typename T>
inline T *getJarvisHandle(JNIEnv *env, jobject obj)
{
    static jfieldID id = 0;
    if (id == 0) {
        jclass cls = env->GetObjectClass(obj);
        id = env->GetFieldID(cls, "jarvisHandle", "J");
    }
    jlong handle = env->GetLongField(obj, id);
    return reinterpret_cast<T *>(handle);
}

template <typename T>
inline void setJarvisHandle(JNIEnv *env, jobject obj, T *t)
{
    static jfieldID id = 0;
    if (id == 0) {
        jclass cls = env->GetObjectClass(obj);
        id = env->GetFieldID(cls, "jarvisHandle", "J");
    }
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, id, handle);
}

extern jobject new_java_node(JNIEnv *env, Jarvis::Node &);
extern jobject new_java_edge(JNIEnv *env, Jarvis::Edge &);
extern jobject new_java_property(JNIEnv *env, Jarvis::Property *);
extern jobject new_java_stringid(JNIEnv *env, Jarvis::StringID);
extern jobject java_node_iterator(JNIEnv *env, Jarvis::NodeIterator &&);
extern jobject java_edge_iterator(JNIEnv *env, Jarvis::EdgeIterator &&);
extern jobject java_property_iterator(JNIEnv *env, Jarvis::PropertyIterator &&);
extern void JavaThrow(JNIEnv *env, Jarvis::Exception e);

#endif
