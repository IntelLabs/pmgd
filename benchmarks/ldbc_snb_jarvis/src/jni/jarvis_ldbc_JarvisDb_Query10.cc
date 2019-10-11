#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query10.h"
#include "query10.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query10
 * Method:    query
 * Signature: (Lpmgd/Graph;JII)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query10_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jint month, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery10Result",
        "(JLjava/lang/String;Ljava/lang/String;ILjava/lang/String;"
        "Ljava/lang/String;)V");

    return run_query<Query10Result>(env, jgraph,
        [id, month](PMGD::Graph &graph) { return query10(graph, id, month); },
        [env](Query10ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.person_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                e.similarity,
                env->NewStringUTF(e.gender.c_str()),
                env->NewStringUTF(e.place.c_str()));
        });
}
