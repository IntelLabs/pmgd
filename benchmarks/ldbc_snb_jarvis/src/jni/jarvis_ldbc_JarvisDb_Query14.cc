#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query14.h"
#include "query14.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query14
 * Method:    query
 * Signature: (Lpmgd/Graph;JJ)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query14_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id1, jlong id2)
{
    static java_ctor ctor(env,
        "LdbcQuery14Result",
        "(Ljava/lang/Iterable;D)V");

    return run_query<Query14Result>(env, jgraph,
        [id1, id2](PMGD::Graph &graph) { return query14(graph, id1, id2); },
        [env](Query14ResultItem &e) {
            jlist ids(env, e.nodes.size());
            for (const auto &i : e.nodes)
                ids.add((long long)i);
            return env->NewObject(ctor.cls, ctor.ctor,
                ids.get_obj(),
                e.weight);
        });
}
