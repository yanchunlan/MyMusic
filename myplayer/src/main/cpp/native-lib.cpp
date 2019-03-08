#include <jni.h>
#include <string>
#include "AndroidLog.h"
#include "CallJava.h"
#include "FFmpeg_Audio.h"

extern "C" {
#include <libavformat/avformat.h>
}

JavaVM *javaVM = NULL;
CallJava *callJava = NULL;
FFmpeg_Audio *fFmpeg_audio = NULL;


extern "C" JNIEXPORT jstring JNICALL
Java_com_ycl_myplayer_demo_Demo_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "welcome to ffmpeg";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_ycl_myplayer_demo_Demo_testFfmpeg(JNIEnv *env, jobject instance) {

    //  仅仅测试
    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }

    std::string hello = "testFfmpeg";
    return env->NewStringUTF(hello.c_str());
}

// ------------------------ load start -----------------
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved){
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(env), JNI_VERSION_1_4) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGD("GetEnv failed!");
        }
        return -1;
    }
    return JNI_VERSION_1_4;
};

extern "C"
JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *javaVM1, void *reserved) {
    javaVM = NULL;
    if (LOG_DEBUG) {
        LOGD("JNI_OnUnload");
    }
};
// ------------------------ load end -----------------


extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1prepared(JNIEnv *env, jobject instance,
                                                     jstring source_) {

    const char *source = env->GetStringUTFChars(source_, 0);

    if (fFmpeg_audio == NULL) {
        if (callJava == NULL) {
            callJava = new CallJava(javaVM, env, instance);
        }
        fFmpeg_audio = new FFmpeg_Audio(callJava, source);
        fFmpeg_audio->prepared();
    }

    env->ReleaseStringUTFChars(source_, source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1start(JNIEnv *env, jobject instance) {
    if (fFmpeg_audio != NULL) {
        fFmpeg_audio->start();
    }
}
