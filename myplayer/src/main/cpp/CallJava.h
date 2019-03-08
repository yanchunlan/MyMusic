//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_CALLJAVA_H
#define MYMUSIC_CALLJAVA_H

#include <jni.h>
#include <linux/stddef.h>
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class CallJava {
    JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_prepared;

public:
    CallJava(JavaVM *javaVM, JNIEnv *jniEnv, jobject jobj);

    virtual ~CallJava();

    void onCallPrepared(int type);
};


#endif //MYMUSIC_CALLJAVA_H
