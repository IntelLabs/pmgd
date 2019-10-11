#pragma once

#include "jni.h"
#include "pmgd.h"
#include "util.h"
#include "jlist.h"

extern void JarvisThrow(JNIEnv *env, PMGD::Exception e);

inline PMGD::Graph *getGraph(JNIEnv *env, jobject obj)
{
    static jfieldID field = 0;
    if (field == 0) {
        jclass c = env->GetObjectClass(obj);
        field = env->GetFieldID(c, "pmgdHandle", "J");
    }
    jlong handle = env->GetLongField(obj, field);
    return reinterpret_cast<PMGD::Graph *>(handle);
}

struct java_ctor {
    jclass cls = 0;
    jmethodID ctor = 0;

    java_ctor(JNIEnv *env, const char *class_name, const char *ctor_signature)
    {
        const std::string full_name
            = std::string("com/ldbc/driver/workloads/ldbc/snb/interactive/")
                + class_name;

        jobject tmp = env->FindClass(full_name.c_str());
        cls = (jclass)env->NewGlobalRef(tmp);
        ctor = env->GetMethodID(cls, "<init>", ctor_signature);
    }
};

template <typename QueryResult, typename QueryFunc, typename ResultFunc>
jobject run_query(JNIEnv *env, jobject jgraph,
                  QueryFunc query_func, ResultFunc result_func)
{
    try {
        PMGD::Graph *graph = getGraph(env, jgraph);
        PMGD::Transaction tx(*graph);

        QueryResult result = query_func(*graph);

        jlist jresult(env, result.size());

        for (auto &e : result)
            jresult.add(result_func(e));

        return jresult.get_obj();
    }
    catch (PMGD::Exception e) {
        print_exception(e, stderr);
        JarvisThrow(env, e);
        return NULL;
    }
}
