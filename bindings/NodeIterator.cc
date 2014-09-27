
#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "NodeIterator.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jboolean Java_NodeIterator_hasNext(JNIEnv *env, jobject ni)
{
    NodeIterator &j_ni = *(getJarvisHandle<NodeIterator>(env, ni));
    return j_ni;
}

jobject Java_NodeIterator_nextNative(JNIEnv *env, jobject ni)
{
    try {
        NodeIterator &j_ni = *(getJarvisHandle<NodeIterator>(env, ni));

        j_ni.next();

        // get the head
        Node *j_n = &(*j_ni);
        printf("(from c) tag: %s\n", j_n->get_tag().name().c_str());

        // build node to return
        jclass cls = env->FindClass("Node");
        jmethodID cnstrctr = env->GetMethodID(cls, "<init>", "(J)V");
        jobject cur = NULL;
        if(j_ni)
            cur = env->NewObject(cls, cnstrctr, reinterpret_cast<jlong>(j_n));
        else
            cur = env->NewObject(cls, cnstrctr, (jlong) 0);

        return cur;
    }
    catch (Exception e) {
        JavaThrow(env, e);
        return NULL;
    }
}

void Java_NodeIterator_dispose(JNIEnv *env, jobject ni)
{
}
