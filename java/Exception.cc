/**
 * @file   Exception.cc
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

#include <jni.h>
#include <string.h>
#include "common.h"

void JavaThrow(JNIEnv *env, PMGD::Exception e)
{
    jclass cls = env->FindClass("pmgd/Exception");
    jmethodID constr = env->GetMethodID(cls, "<init>",
        "(ILjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;I)V");
    jobject exc = env->NewObject(cls, constr,
                      e.num,
                      env->NewStringUTF(e.name),
                      e.msg != ""
                          ? env->NewStringUTF(e.msg.c_str())
                          : NULL,
                      e.errno_val,
                      e.errno_val != 0
                          ? env->NewStringUTF(strerror(e.errno_val))
                          : NULL,
                      env->NewStringUTF(e.file),
                      e.line);
    env->Throw(static_cast<jthrowable>(exc));
}
