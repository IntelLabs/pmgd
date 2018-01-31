/**
 * @file   NodeIterator.cc
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
#include "NodeIterator.h"
#include "common.h"

using namespace PMGD;

void Java_pmgd_NodeIterator_next(JNIEnv *env, jobject ni)
{
    try {
        NodeIterator &j_ni = *(getPMGDHandle<NodeIterator>(env, ni));

        j_ni.next();

        jobject cur = j_ni ? new_java_node(env, *j_ni) : NULL;

        static jfieldID fid = 0;
        if (fid == 0) {
            jclass cls = env->GetObjectClass(ni);
            fid = env->GetFieldID(cls, "current", "Lpmgd/Node;");
        }
        env->SetObjectField(ni, fid, cur);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_pmgd_NodeIterator_dispose(JNIEnv *env, jobject ni)
{
    NodeIterator *j_ni = getPMGDHandle<NodeIterator>(env, ni);
    delete j_ni;
    setPMGDHandle(env, ni, static_cast<NodeIterator *>(NULL));
}
