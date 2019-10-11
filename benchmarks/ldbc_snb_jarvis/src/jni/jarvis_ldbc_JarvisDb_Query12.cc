#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query12.h"
#include "query12.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query12
 * Method:    query
 * Signature: (Lpmgd/Graph;JLjava/lang/String;I)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query12_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jstring jtag, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery12Result",
        "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/Iterable;I)V");

    const char *tag = env->GetStringUTFChars(jtag, 0);

    return run_query<Query12Result>(env, jgraph,
        [id, tag](PMGD::Graph &graph) { return query12(graph, id, tag); },
        [env](Query12ResultItem &e) {
            jlist tags(env, e.tags.size());
            for (const auto &s : e.tags)
                tags.add(env->NewStringUTF(s.c_str()));
            return env->NewObject(ctor.cls, ctor.ctor,
                e.person_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                tags.get_obj(),
                e.count);
        });
}
