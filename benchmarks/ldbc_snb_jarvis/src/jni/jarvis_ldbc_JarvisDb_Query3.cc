#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query3.h"
#include "query3.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query3
 * Method:    query
 * Signature: (Lpmgd/Graph;JLjava/lang/String;Ljava/lang/String;JII)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query3_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id,
   jstring countryX, jstring countryY,
   jlong start, jint duration, jint limit)
{
    static java_ctor ctor(env,
        "LdbcQuery3Result",
        "(JLjava/lang/String;Ljava/lang/String;JJJ)V");

    time_t start_date = start / 1000; // convert ms to sec
    time_t end_date = start_date + duration * 24 * 3600; // convert days to sec
    const char *country1 = env->GetStringUTFChars(countryX, 0);
    const char *country2 = env->GetStringUTFChars(countryY, 0);

    return run_query<Query3Result>(env, jgraph,
        [id, start_date, end_date, country1, country2](PMGD::Graph &graph) {
            return query3(graph, id, start_date, end_date,
                          country1, country2);
        },
        [env](Query3ResultItem &e) {
            return env->NewObject(ctor.cls, ctor.ctor,
                e.friend_id,
                env->NewStringUTF(e.firstName.c_str()),
                env->NewStringUTF(e.lastName.c_str()),
                (jlong)e.count_x,
                (jlong)e.count_y,
                (jlong)e.count);
        });
}
