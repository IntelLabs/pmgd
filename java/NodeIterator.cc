#include <string.h>
#include <stdio.h>

#include "jarvis.h"

#include "NodeIterator.h"
#include "jarvisHandles.h"

using namespace Jarvis;

void Java_jarvis_NodeIterator_next(JNIEnv *env, jobject ni)
{
    try {
        NodeIterator &j_ni = *(getJarvisHandle<NodeIterator>(env, ni));

        j_ni.next();

        jobject cur;
        if (j_ni) {
            Node *j_n = &(*j_ni);
            cur = new_node_object(env, j_n);
        }
        else
            cur = NULL;

        static jfieldID fid = 0;
        if (fid == 0) {
            jclass cls = env->GetObjectClass(ni);
            fid = env->GetFieldID(cls, "current", "Ljarvis/Node;");
        }
        env->SetObjectField(ni, fid, cur);
    }
    catch (Exception e) {
        JavaThrow(env, e);
    }
}

void Java_jarvis_NodeIterator_dispose(JNIEnv *env, jobject ni)
{
    NodeIterator *j_ni = getJarvisHandle<NodeIterator>(env, ni);
    delete j_ni;
    setJarvisHandle(env, ni, static_cast<NodeIterator *>(NULL));
}
