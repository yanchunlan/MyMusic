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
    jmid_renderyuv = jniEnv->GetMethodID(jcls, "onCallRenderYUV", "(II[B[B[B)V");
    jmid_supportvideo = jniEnv->GetMethodID(jcls, "onCallIsSupportMediaCodec",
                                            "(Ljava/lang/String;)Z");
    jmid_initmediacodec = jniEnv->GetMethodID(jcls, "onCallInitMediaCodec",
                                              "(Ljava/lang/String;II[B[B)V");
    jmid_decodeavpacket = jniEnv->GetMethodID(jcls, "onCallDecodeAVPacket",
                                              "(I[B)V");
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
    jmid_renderyuv = NULL;
    jmid_supportvideo = NULL;
    jmid_initmediacodec = NULL;
    jmid_decodeavpacket = NULL;
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

void CallJava::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("child thread jnienv isNot JNI_OK")
        }
        return;
    }

    // yuv 數據大小是 1 ： 1/4 ：1/4
    jbyteArray y = jniEnv->NewByteArray(width * height);
    jniEnv->SetByteArrayRegion(y, 0, width * height, reinterpret_cast<const jbyte *>(fy));

    jbyteArray u = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(u, 0, width * height/4, reinterpret_cast<const jbyte *>(fu));

    jbyteArray v = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(v, 0, width * height/4, reinterpret_cast<const jbyte *>(fv));


    jniEnv->CallVoidMethod(jobj, jmid_renderyuv, width, height, y, u, v);

    jniEnv->DeleteLocalRef(y);
    jniEnv->DeleteLocalRef(u);
    jniEnv->DeleteLocalRef(v);

    javaVM->DetachCurrentThread();
}

bool CallJava::onCallIsSupportVideo(const char *ffcodecName) {
    bool support = false;
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("child thread jnienv isNot JNI_OK")
        }
        return support;
    }

    jstring type = jniEnv->NewStringUTF(ffcodecName);
    support = jniEnv->CallBooleanMethod(jobj, jmid_supportvideo, type);
    jniEnv->DeleteLocalRef(type);

    javaVM->DetachCurrentThread();
    return support;
}

void CallJava::onCallInitMediaCodec(const char *mine, int width, int height, int csd0_size,
                                    int csd1_size, uint8_t *csd_0, uint8_t *csd_1) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("child thread jnienv isNot JNI_OK")
        }
        return;
    }
    jstring type = jniEnv->NewStringUTF(mine);
    jbyteArray csd0 = jniEnv->NewByteArray(csd0_size);
    jniEnv->SetByteArrayRegion(csd0, 0, csd0_size, reinterpret_cast<const jbyte *>(csd_0));
    jbyteArray csd1 = jniEnv->NewByteArray(csd1_size);
    jniEnv->SetByteArrayRegion(csd1, 0, csd1_size, reinterpret_cast<const jbyte *>(csd_1));

    jniEnv->CallVoidMethod(jobj, jmid_initmediacodec,type,width,height,csd0,csd1);

    jniEnv->DeleteLocalRef(type);
    jniEnv->DeleteLocalRef(csd0);
    jniEnv->DeleteLocalRef(csd1);

    javaVM->DetachCurrentThread();
}

void CallJava::onCallDecodeAVPacket(int dataSize, uint8_t *packetData) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("child thread jnienv isNot JNI_OK")
        }
        return;
    }

    jbyteArray data = jniEnv->NewByteArray(dataSize);
    jniEnv->SetByteArrayRegion(data, 0, dataSize, reinterpret_cast<const jbyte *>(packetData));

    jniEnv->CallVoidMethod(jobj, jmid_decodeavpacket, dataSize, data);
    jniEnv->DeleteLocalRef(data);

    javaVM->DetachCurrentThread();
}
