#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query7.h"
#include "query7.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query7
 * Method:    query
 * Signature: (Lpmgd/Graph;JI)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query7_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery7Result",
        "(JLjava/lang/String;Ljava/lang/String;JJLjava/lang/String;IZ)V");

    return run_query<Query7Result>(env, jgraph,
        [id](PMGD::Graph &graph) {
            return query7(graph, id);
        },
        [env](Query7ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.person_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                e.like_time.get_time_in_msec(),
                e.message_id,
                env->NewStringUTF(e.content.c_str()),
                e.latency,
                e.isNew);
        });
}
