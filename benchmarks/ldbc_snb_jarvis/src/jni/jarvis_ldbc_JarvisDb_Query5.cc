#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query5.h"
#include "query5.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query5
 * Method:    query
 * Signature: (Lpmgd/Graph;JJI)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query5_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jlong jdate, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery5Result",
        "(Ljava/lang/String;I)V");

    time_t date = jdate / 1000; // convert ms to sec

    return run_query<Query5Result>(env, jgraph,
        [id, date](PMGD::Graph &graph) {
            return query5(graph, id, date);
        },
        [env](Query5ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                env->NewStringUTF(e.forum_name.c_str()),
                e.count);
        });
}
