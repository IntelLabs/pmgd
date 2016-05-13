#ifndef JARVISHANDLES_H
#define JARVISHANDLES_H

#include <jni.h>
#include "jarvis.h"
#include "../src/compiler.h"

extern THREAD jobject java_transaction;

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

template <> Jarvis::Graph *getJarvisHandle<Jarvis::Graph>
    (JNIEnv *env, jobject obj);

template <> Jarvis::Node *getJarvisHandle<Jarvis::Node>
    (JNIEnv *env, jobject obj);

template <> Jarvis::NodeIterator *getJarvisHandle<Jarvis::NodeIterator>
    (JNIEnv *env, jobject obj);

template <> Jarvis::EdgeIterator *getJarvisHandle<Jarvis::EdgeIterator>
    (JNIEnv *env, jobject obj);

extern jobject new_java_node(JNIEnv *env, Jarvis::Node &);
extern jobject new_java_edge(JNIEnv *env, Jarvis::Edge &);
extern jobject new_java_property(JNIEnv *env, Jarvis::Property *);
extern jobject new_java_stringid(JNIEnv *env, Jarvis::StringID);
extern jobject java_node_iterator(JNIEnv *env, Jarvis::NodeIterator &&);
extern jobject java_edge_iterator(JNIEnv *env, Jarvis::EdgeIterator &&);
extern jobject java_property_iterator(JNIEnv *env, Jarvis::PropertyIterator &&);
extern void JavaThrow(JNIEnv *env, Jarvis::Exception e);

#endif
