#include "pmgd.h"
#include "util.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query13.h"
#include "query13.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query13
 * Method:    query
 * Signature: (Lpmgd/Graph;JJ)Lcom/ldbc/driver/workloads/ldbc/snb/interactive/LdbcQuery13Result;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query13_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id1, jlong id2)
{
    static java_ctor ctor(env, "LdbcQuery13Result", "(I)V");

    try {
        PMGD::Graph *graph = getGraph(env, jgraph);
        PMGD::Transaction tx(*graph);
        Query13Result result = query13(*graph, id1, id2);
        return env->NewObject(ctor.cls, ctor.ctor, result);
    }
    catch (PMGD::Exception e) {
        print_exception(e, stderr);
        JarvisThrow(env, e);
        return NULL;
    }
}
