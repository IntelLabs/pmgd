#include "jarvis.h"
#include "common.h"
#include "PropertyPredicate.h"

using namespace Jarvis;

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__
    (JNIEnv *env, jobject pp)
{
    PropertyPredicate *j_pp = new PropertyPredicate();
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__I
    (JNIEnv *env, jobject pp, jint id)
{
    PropertyPredicate *j_pp = new PropertyPredicate(id);
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__IILjarvis_Property_2
    (JNIEnv *env, jobject pp, jint id, jint op, jobject v)
{
    PropertyPredicate::Op j_op = PropertyPredicate::Op(op);
    Property &j_v = *(getJarvisHandle<Property>(env, v));
    PropertyPredicate *j_pp = new PropertyPredicate(id, j_op, j_v);
    setJarvisHandle(env, pp, j_pp);
}

void JNICALL Java_jarvis_PropertyPredicate_newPropertyPredicateNative__IILjarvis_Property_2Ljarvis_Property_2
    (JNIEnv *env, jobject pp, jint id, jint op, jobject v1, jobject v2)
{
    PropertyPredicate::Op j_op = PropertyPredicate::Op(op);
    Property &j_v1 = *(getJarvisHandle<Property>(env, v1));
    Property &j_v2 = *(getJarvisHandle<Property>(env, v2));
    PropertyPredicate *j_pp = new PropertyPredicate(id, j_op, j_v1, j_v2);
    setJarvisHandle(env, pp, j_pp);
}

void Java_jarvis_PropertyPredicate_dispose(JNIEnv *env, jobject pp)
{
    PropertyPredicate *j_pp = getJarvisHandle<PropertyPredicate>(env, pp);
    delete j_pp;
    setJarvisHandle(env, pp, static_cast<PropertyPredicate *>(NULL));
}
