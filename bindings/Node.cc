
#include <string.h>
#include <stdio.h>

#include "../include/jarvis.h"
#include "../util/util.h"

#include "Node.h"
#include "jarvisHandles.h"

using namespace Jarvis;

jstring Java_Node_get_1tag(JNIEnv *env, jobject node){
  Node &j_node = *(getJarvisHandle<Node>(env,node));
  try{
    const char* tag = j_node.get_tag().name().c_str();
    return env->NewStringUTF(tag);
  }
  catch (Exception e){
    print_exception(e);
    return NULL;
  }
}


jboolean Java_Node_check_1property(JNIEnv *env , jobject node, 
				   jstring str, jobject prop){
  Node &j_node = *(getJarvisHandle<Node>(env,node));
  const char *j_str = env->GetStringUTFChars(str,0);
  try{
    Property *result = new Property();
    bool ret = j_node.check_property(j_str, *result);
    setJarvisHandle<Property>(env,prop,result); //set a return prop
    return ret;
  }
  catch (Exception e){
    print_exception(e);
    return false; //this should really throw, not return
  }
}
jobject Java_Node_get_1property(JNIEnv *env, jobject node, 
				jstring str){
  Node &j_node = *(getJarvisHandle<Node>(env,node));
  const char *j_str = env->GetStringUTFChars(str,0);
  try{
    Property *result = new Property();
    *result = j_node.get_property(j_str);

    jclass cls = env->FindClass("Property");
    jmethodID cnstrctr = env->GetMethodID(cls, "<init>",
    					  "()V");
    jobject new_p = env->NewObject(cls, cnstrctr, result);
    setJarvisHandle<Property>(env,new_p, result);
    return new_p;
  }
  catch (Exception e){
    print_exception(e);
    return NULL;  //empty property better?
  }
}

void Java_Node_set_1property(JNIEnv *env, jobject node,
			     jstring str, jobject prop){
  Node &j_node = *(getJarvisHandle<Node>(env,node));
  Property &j_prop = *(getJarvisHandle<Property>(env,prop));
  const char *j_str = env->GetStringUTFChars(str,0);
  try{
    j_node.set_property(j_str, j_prop);
  }
  catch (Exception e){
    print_exception(e);
  }
}

void Java_Node_remove_1property(JNIEnv *env, jobject node, 
				jstring str){
  Node &j_node = *(getJarvisHandle<Node>(env,node));
  const char *j_str = env->GetStringUTFChars(str,0);
  try{
    j_node.remove_property(j_str);
  }
  catch (Exception e){
    print_exception(e);
  }
}
