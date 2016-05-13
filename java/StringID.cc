#include <string.h>
#include "jarvis.h"
#include "StringID.h"
#include "common.h"

using namespace Jarvis;

void Java_jarvis_StringID_init(JNIEnv *env, jobject stringid, jstring name)
{
    static jfieldID fid = 0;
    if (fid == 0) {
        jclass cls = env->GetObjectClass(stringid);
        fid = env->GetFieldID(cls, "_id", "I");
    }

    try {
        int id = name != NULL ? StringID(env->GetStringUTFChars(name, 0)).id() : 0;
        env->SetIntField(stringid, fid, id);
    }
    catch (Exception e) {
        env->SetIntField(stringid, fid, 0);
        JavaThrow(env, e);
    }
}


jstring Java_jarvis_StringID_nameNative(JNIEnv *env, jobject, jint id)
{
    const char *s = id == 0 ? "" : StringID(id).name().c_str();
    return env->NewStringUTF(s);
}


jobject new_java_stringid(JNIEnv *env, StringID sid)
{
    static jclass cls = 0;
    static jmethodID ctor = 0;
    if (ctor == 0) {
        cls = (jclass)env->NewGlobalRef(env->FindClass("jarvis/StringID"));
        ctor = env->GetMethodID(cls, "<init>", "(I)V");
        assert(ctor != 0);
    }
    return env->NewObject(cls, ctor, jint(sid.id()));
}
