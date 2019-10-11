#include <string.h>
#include "jni.h"
#include "common.h"
#include "jlist.h"

jclass jlist::_list_class = 0;
jmethodID jlist::_list_ctor = 0;
jmethodID jlist::_list_add = 0;
jclass jlist::_int_class = 0;
jmethodID jlist::_int_ctor = 0;
jclass jlist::_long_class = 0;
jmethodID jlist::_long_ctor = 0;


void JarvisThrow(JNIEnv *env, PMGD::Exception e)
{
    jclass cls = env->FindClass("pmgd/Exception");
    jmethodID constr = env->GetMethodID(cls, "<init>",
        "(ILjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;I)V");
    jobject exc = env->NewObject(cls, constr,
                      e.num,
                      env->NewStringUTF(e.name),
                      e.msg != ""
                          ? env->NewStringUTF(e.msg.c_str())
                          : NULL,
                      e.errno_val,
                      e.errno_val != 0
                          ? env->NewStringUTF(strerror(e.errno_val))
                          : NULL,
                      env->NewStringUTF(e.file),
                      e.line);
    env->Throw(static_cast<jthrowable>(exc));
}
