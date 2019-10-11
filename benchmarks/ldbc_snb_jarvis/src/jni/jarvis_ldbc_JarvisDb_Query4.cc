#include "pmgd.h"
#include "util.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query4.h"
#include "query4.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query4
 * Method:    query
 * Signature: (Lpmgd/Graph;JJII)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query4_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id,
   jlong start, jint duration, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery4Result",
        "(Ljava/lang/String;I)V");

    time_t start_date = start / 1000; // convert ms to sec
    time_t end_date = start_date + duration * 24 * 3600; // convert days to sec

    return run_query<Query4Result>(env, jgraph,
        [id, start_date, end_date](PMGD::Graph &graph) {
            return query4(graph, id, start_date, end_date);
        },
        [env](Query4ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                env->NewStringUTF(e.tag.c_str()),
                e.count);
        });
}
