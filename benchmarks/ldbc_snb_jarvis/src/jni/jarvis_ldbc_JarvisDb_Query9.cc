#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query9.h"
#include "query9.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query9
 * Method:    query
 * Signature: (Lpmgd/Graph;JJI)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query9_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jlong max_date, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery9Result",
        "(JLjava/lang/String;Ljava/lang/String;JLjava/lang/String;J)V");

    time_t date = max_date / 1000; // convert ms to sec

    return run_query<Query9Result>(env, jgraph,
        [id, date](PMGD::Graph &graph) { return query9(graph, id, date); },
        [env](Query9ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.friend_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                e.post_id,
                env->NewStringUTF(e.content.c_str()),
                e.creationDate.get_time_in_msec());
        });
}
