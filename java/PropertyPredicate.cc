#include "jarvis.h"
#include "jarvisHandles.h"
#include "PropertyPredicate.h"

using namespace Jarvis;

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__
    (JNIEnv *env, jobject pp)
{
    PropertyPredicate *j_pp = new PropertyPredicate();
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__Ljava_lang_String_2
    (JNIEnv *env, jobject pp, jstring name)
{
    const char *j_name = env->GetStringUTFChars(name, 0);
    PropertyPredicate *j_pp = new PropertyPredicate(j_name);
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__Ljava_lang_String_2ILjarvis_Property_2
    (JNIEnv *env, jobject pp, jstring name, jint op, jobject v)
{
    const char *j_name = env->GetStringUTFChars(name, 0);
    PropertyPredicate::op_t j_op = PropertyPredicate::op_t(op);
    Property &j_v = *(getJarvisHandle<Property>(env, v));
    PropertyPredicate *j_pp = new PropertyPredicate(j_name, j_op, j_v);
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__Ljava_lang_String_2ILjarvis_Property_2Ljarvis_Property_2
    (JNIEnv *env, jobject pp, jstring name, jint op, jobject v1, jobject v2)
{
    const char *j_name = env->GetStringUTFChars(name, 0);
    PropertyPredicate::op_t j_op = PropertyPredicate::op_t(op);
    Property &j_v1 = *(getJarvisHandle<Property>(env, v1));
    Property &j_v2 = *(getJarvisHandle<Property>(env, v2));
    PropertyPredicate *j_pp = new PropertyPredicate(j_name, j_op, j_v1, j_v2);
    setJarvisHandle(env, pp, j_pp);
}

void Java_jarvis_PropertyPredicate_dispose(JNIEnv *env, jobject pp)
{
    NodeIterator *j_pp = getJarvisHandle<NodeIterator>(env, pp);
    delete j_pp;
    setJarvisHandle(env, pp, static_cast<NodeIterator *>(NULL));
}
