#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "NodeIterator.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void Java_NodeIterator_next(JNIEnv *env, jobject ni)
{
    try {
        NodeIterator &j_ni = *(getJarvisHandle<NodeIterator>(env, ni));

        j_ni.next();

        jobject cur;
        if (j_ni) {
            // get the current node
            Node *j_n = &(*j_ni);
            cur = new_java_object(env, "Node", j_n);
        }
        else
            cur = NULL;

        jclass cls = env->GetObjectClass(ni);
        jfieldID fid = env->GetFieldID(cls, "current", "LNode;");
        env->SetObjectField(ni, fid, cur);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_NodeIterator_dispose(JNIEnv *env, jobject ni)
{
    NodeIterator *j_ni = getJarvisHandle<NodeIterator>(env, ni);
    delete j_ni;
    setJarvisHandle(env, ni, static_cast<NodeIterator *>(NULL));
}
