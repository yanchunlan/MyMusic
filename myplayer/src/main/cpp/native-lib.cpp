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
PlayStatus *playStatus = NULL;

pthread_t pthread_start; // 开始是在子线程执行

bool nexit = true;// stop的标识，防止重复stop


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
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
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
        callJava = new CallJava(javaVM, env, instance);
        callJava->onCallLoad(MAIN_THREAD, true);
        playStatus = new PlayStatus();
        fFmpeg_audio = new FFmpeg_Audio(playStatus, callJava, source);
        fFmpeg_audio->prepared();
    }

    env->ReleaseStringUTFChars(source_, source);
}

void *startCallBack(void *data) {
    FFmpeg_Audio *fFmpeg = (FFmpeg_Audio *) (data);
    fFmpeg->start();
//    pthread_exit(&pthread_start);
    return 0;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1start(JNIEnv *env, jobject instance) {
    if (fFmpeg_audio != NULL) {
        pthread_create(&pthread_start, NULL, startCallBack, fFmpeg_audio);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1resume(JNIEnv *env, jobject instance) {
    if (fFmpeg_audio != NULL) {
        fFmpeg_audio->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1pause(JNIEnv *env, jobject instance) {
    if (fFmpeg_audio != NULL) {
        fFmpeg_audio->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1stop(JNIEnv *env, jobject instance) {

    if (!nexit) { // 如果在stop就拦截不再stop,防止重复
        return;
    }

    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jlz, "onCallNext", "()V");

    nexit = false;

    if (fFmpeg_audio != NULL) {
        fFmpeg_audio->release();
        pthread_join(pthread_start, NULL);
        delete (fFmpeg_audio);
        fFmpeg_audio = NULL;
        if (callJava != NULL) {
            delete (callJava);
            callJava = NULL;
        }
        if (playStatus != NULL) {
            delete (playStatus);
            playStatus = NULL;
        }

        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("error %s", "error")
        }
    }

    nexit = true;

    env->CallVoidMethod(instance, jmid_next); // 调用下一个的方法，java层会做判断，如果有下一步就走到下一步

    // 其他在 callJava ，当前页面的数据需要及时释放
    jlz = NULL;
    jmid_next = NULL;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ycl_myplayer_demo_player_Player_n_1seek(JNIEnv *env, jobject instance, jint secds) {

    if (fFmpeg_audio != NULL) {
        fFmpeg_audio->seek(secds);
    }
}