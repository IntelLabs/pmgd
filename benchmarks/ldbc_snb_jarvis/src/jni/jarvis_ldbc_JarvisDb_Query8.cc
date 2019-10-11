#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query8.h"
#include "query8.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query8
 * Method:    query
 * Signature: (Lpmgd/Graph;JI)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query8_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery8Result",
        "(JLjava/lang/String;Ljava/lang/String;JJLjava/lang/String;)V");

    return run_query<Query8Result>(env, jgraph,
        [id](PMGD::Graph &graph) { return query8(graph, id); },
        [env](Query8ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.commenter_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                e.comment_time.get_time_in_msec(),
                e.comment_id,
                env->NewStringUTF(e.content.c_str()));
        });
}
