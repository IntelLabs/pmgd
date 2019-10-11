#include "pmgd.h"
#include "util.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query6.h"
#include "query6.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query6
 * Method:    query
 * Signature: (Lpmgd/Graph;JLjava/lang/String;I)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query6_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jstring jtag, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery6Result",
        "(Ljava/lang/String;I)V");

    const char *tag = env->GetStringUTFChars(jtag, 0);

    return run_query<Query6Result>(env, jgraph,
        [id, tag](PMGD::Graph &graph) {
            return query6(graph, id, tag);
        },
        [env](Query6ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                env->NewStringUTF(e.name.c_str()),
                e.count);
        });
}
