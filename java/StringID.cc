/**
 * @file   StringID.cc
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
#include "StringID.h"
#include "common.h"

using namespace PMGD;

void Java_pmgd_StringID_init(JNIEnv *env, jobject stringid, jstring name)
{
    static jfieldID fid = 0;
    if (fid == 0) {
        jclass cls = env->GetObjectClass(stringid);
        fid = env->GetFieldID(cls, "_id", "I");
    }

    try {
        int id = name != NULL ? StringID(env->GetStringUTFChars(name, 0)).id() : 0;
        env->SetIntField(stringid, fid, id);
    }
    catch (Exception e) {
        env->SetIntField(stringid, fid, 0);
        JavaThrow(env, e);
    }
}


jstring Java_pmgd_StringID_nameNative(JNIEnv *env, jobject, jint id)
{
    const char *s = id == 0 ? "" : StringID(id).name().c_str();
    return env->NewStringUTF(s);
}


jobject new_java_stringid(JNIEnv *env, StringID sid)
{
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("pmgd/StringID"));
        ctor = env->GetMethodID(cls, "<init>", "(I)V");
        assert(ctor != 0);
    }
    return env->NewObject(cls, ctor, jint(sid.id()));
}
