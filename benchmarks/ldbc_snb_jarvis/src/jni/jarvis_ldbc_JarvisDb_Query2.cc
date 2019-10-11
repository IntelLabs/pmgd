#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query2.h"
#include "query2.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query2
 * Method:    query
 * Signature: (Lpmgd/Graph;JJI)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query2_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jlong date, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery2Result",
        "(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;J)V");

    return run_query<Query2Result>(env, jgraph,
        [id, date](PMGD::Graph &graph) { return query2(graph, id, date); },
        [env](Query2ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.friend_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                e.post_id,
                env->NewStringUTF(e.content.c_str()),
                e.creationDate.get_time_in_msec());
        });
}
