#include <jni.h>
#include <string>


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


    std::string hello = "testFfmpeg";

    return env->NewStringUTF(hello.c_str());
}