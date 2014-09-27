#ifndef JARVISHANDLES_H
#define JARVISHANDLES_H

#include <jni.h>
#include "../include/exception.h"

inline jfieldID getHandleField(JNIEnv *env, jobject obj)
{
    jclass c = env->GetObjectClass(obj);
    return env->GetFieldID(c, "jarvisHandle", "J"); // where J is type for long
}

template <typename T>
inline T *getJarvisHandle(JNIEnv *env, jobject obj)
{
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
inline void setJarvisHandle(JNIEnv *env, jobject obj, T *t)
{
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj), handle);
}


extern jobject new_java_object(JNIEnv *env, const char *name, void *obj);
extern void JavaThrow(JNIEnv *env, Jarvis::Exception e);

#endif
