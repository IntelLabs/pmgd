#include <jni.h>
#include <string.h>
#include "jarvisHandles.h"

void JavaThrow(JNIEnv *env, Jarvis::Exception e)
{
    jclass cls = env->FindClass("Exception");
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
