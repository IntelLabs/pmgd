#pragma once

#include "jni.h"

class jlist {
    static jclass _list_class;
    static jmethodID _list_ctor;
    static jmethodID _list_add;
    static jclass _int_class;
    static jmethodID _int_ctor;
    static jclass _long_class;
    static jmethodID _long_ctor;

    JNIEnv *_env;
    jobject _obj;

    void init_jlist(JNIEnv *env)
    {
        if (_list_class == 0) {
            jobject tmp = env->FindClass("java/util/ArrayList");
            _list_class = (jclass)env->NewGlobalRef(tmp);
            _list_ctor = env->GetMethodID(_list_class, "<init>", "(I)V");
            _list_add = env->GetMethodID(_list_class, "add", "(Ljava/lang/Object;)Z");
        }
    }

    void init_jint(JNIEnv *env)
    {
        if (_int_class == 0) {
            jobject tmp = env->FindClass("java/lang/Integer");
            _int_class = (jclass)env->NewGlobalRef(tmp);
            _int_ctor = env->GetMethodID(_int_class, "<init>", "(I)V");
        }
    }

    void init_jlong(JNIEnv *env)
    {
        if (_long_class == 0) {
            jobject tmp = env->FindClass("java/lang/Long");
            _long_class = (jclass)env->NewGlobalRef(tmp);
            _long_ctor = env->GetMethodID(_long_class, "<init>", "(J)V");
        }
    }

public:
    jlist(JNIEnv *env, int size)
        : _env(env)
    {
        init_jlist(env);
        _obj = env->NewObject(_list_class, _list_ctor, size);
    }

    jobject get_obj() const { return _obj; }

    void add(jobject je) { _env->CallVoidMethod(_obj, _list_add, je); }

    void add(int val)
    {
        init_jint(_env);
        add(_env->NewObject(_int_class, _int_ctor, val));
    }

    void add(long long val)
    {
        init_jlong(_env);
        add(_env->NewObject(_long_class, _long_ctor, val));
    }
};
