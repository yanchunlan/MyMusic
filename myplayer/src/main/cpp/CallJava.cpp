//
// Created by pc on 2019/3/8.
//

#include "CallJava.h"


CallJava::CallJava(JavaVM *javaVM, JNIEnv *jniEnv, jobject jobj) {
    this->javaVM = javaVM;
    this->jniEnv = jniEnv;
    this->jobj = jniEnv->NewGlobalRef(jobj);

    jclass jcls = jniEnv->GetObjectClass(jobj);
    if (!jcls) {
        if (LOG_DEBUG) {
            LOGE("jcls is null")
        }
        return;
    }
    jmid_prepared = jniEnv->GetMethodID(jcls, "onCallPrepared", "()V");
}

CallJava::~CallJava() {
    jniEnv->DeleteGlobalRef(jobj);
    javaVM = NULL;
    jniEnv = NULL;

    jobj = NULL;
    jmid_prepared = NULL;
}

void CallJava::onCallPrepared(int type) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("child thread jnienv isNot JNI_OK")
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}
