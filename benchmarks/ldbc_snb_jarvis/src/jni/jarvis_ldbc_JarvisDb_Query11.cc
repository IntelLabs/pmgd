#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query11.h"
#include "query11.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query11
 * Method:    query
 * Signature: (Lpmgd/Graph;JLjava/lang/String;II)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query11_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jstring jcountry,
   jint year, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery11Result",
        "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

    const char *country = env->GetStringUTFChars(jcountry, 0);

    return run_query<Query11Result>(env, jgraph,
        [id, country, year](PMGD::Graph &graph) {
            return query11(graph, id, country, year);
        },
        [env](Query11ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.person_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                env->NewStringUTF(e.company_name.c_str()),
                e.year);
        });
}
