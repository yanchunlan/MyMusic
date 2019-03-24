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
    jmid_load = jniEnv->GetMethodID(jcls, "onCallLoad", "(Z)V");
    jmid_timeinfo = jniEnv->GetMethodID(jcls, "onCallTimeInfo", "(II)V");
    jmid_error = jniEnv->GetMethodID(jcls, "onCallError", "(ILjava/lang/String;)V");
    jmid_complete = jniEnv->GetMethodID(jcls, "onCallComplete", "()V");
}

CallJava::~CallJava() {

//    if (jobj != NULL) {
//        jniEnv->DeleteGlobalRef(jobj);
//        jobj = NULL;
//    }

    javaVM = NULL;
    jniEnv = NULL;

    jmid_prepared = NULL;
    jmid_load = NULL;
    jmid_timeinfo = NULL;
    jmid_error = NULL;
    jmid_complete = NULL;
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

void CallJava::onCallLoad(int type, bool load) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("child thread jnienv isNot JNI_OK")
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallTimeInfo(int type, int curr, int total) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("child thread jnienv isNot JNI_OK")
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallError(int type, int code, char *msg) {
    if (type == MAIN_THREAD) {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);// 不是release , 是delete
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("child thread jnienv isNot JNI_OK")
            }
            return;
        }
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallComplete(int type) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_complete);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("child thread jnienv isNot JNI_OK")
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_complete);
        javaVM->DetachCurrentThread();
    }
}
