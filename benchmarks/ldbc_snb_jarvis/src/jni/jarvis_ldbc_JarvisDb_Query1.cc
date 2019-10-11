#include "pmgd.h"
#include "common.h"
#include "jlist.h"
#include "jarvis_ldbc_JarvisDb_Query1.h"
#include "query1.h"

/*
 * Class:     jarvis_ldbc_JarvisDb_Query1
 * Method:    query
 * Signature: (Lpmgd/Graph;JLjava/lang/String;I)Ljava/util/List;
 */
jobject JNICALL Java_jarvis_ldbc_JarvisDb_00024Query1_query
  (JNIEnv *env, jobject, jobject jgraph, jlong id, jstring jname,
   jint /*limit*/)
{
    static java_ctor ctor(env,
        "LdbcQuery1Result",
        "(JLjava/lang/String;IJJ"
        "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/Iterable;Ljava/lang/Iterable;Ljava/lang/String;"
        "Ljava/lang/Iterable;Ljava/lang/Iterable;)V");

    const char *name = env->GetStringUTFChars(jname, 0);

    return run_query<Query1Result>(env, jgraph,
        [id, name](PMGD::Graph &graph) {
            return query1(graph, id, name /*, limit */);
        },
        [env](Query1ResultItem &e) {
            jlist emailaddresses(env, e.emailaddresses.size());
            for (const auto &i : e.emailaddresses)
                emailaddresses.add(env->NewStringUTF(i.c_str()));

            jlist languages(env, e.languages.size());
            for (const auto &i : e.languages)
                languages.add(env->NewStringUTF(i.c_str()));

            jlist universities(env, e.universities.size());
            for (const auto &i : e.universities) {
                jlist u(env, 3);
                u.add(env->NewStringUTF(i.name.c_str()));
                u.add(i.year);
                u.add(env->NewStringUTF(i.city.c_str()));
                universities.add(u.get_obj());
            }

            jlist companies(env, e.companies.size());
            for (const auto &i : e.companies) {
                jlist u(env, 3);
                u.add(env->NewStringUTF(i.name.c_str()));
                u.add(i.year);
                u.add(env->NewStringUTF(i.country.c_str()));
                companies.add(u.get_obj());
            }

            return env->NewObject(ctor.cls, ctor.ctor,
                e.id,
                env->NewStringUTF(e.lastName.c_str()),
                e.distance,
                e.birthday.get_time_in_msec(),
                e.creationDate.get_time_in_msec(),
                env->NewStringUTF(e.gender.c_str()),
                env->NewStringUTF(e.browserUsed.c_str()),
                env->NewStringUTF(e.locationIP.c_str()),
                emailaddresses.get_obj(),
                languages.get_obj(),
                env->NewStringUTF(e.place.c_str()),
                universities.get_obj(),
                companies.get_obj());
        });
}
