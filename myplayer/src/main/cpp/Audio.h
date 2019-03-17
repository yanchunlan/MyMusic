//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_AUDIO_H
#define MYMUSIC_AUDIO_H

#include "Queue.h"
#include "CallJava.h"


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
}

class Audio {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecPar = NULL;

    Queue *queue = NULL;
    PlayStatus *playStatus = NULL;
    CallJava *callJava = NULL;


    // 重采样
    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0; // 可以控制播放器的sample_rate


    // 时间计算
    int duration = 0;
    AVRational time_base; // 分子分母的方式 存储 time_base 代表时间的某一帧
    double clock; // 总的播放时长  是递增的
    double now_time; // 当前frame时间
    double last_time; // 上一层调用时间


    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    // 混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    // pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    // 缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;


public:
    Audio(PlayStatus *playStatus, int sample_rate, CallJava *callJava);

    virtual ~Audio();

    void play();

    int resampleAudio();

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(int sample_rate);

    void pause();

    void resume();

};


#endif //MYMUSIC_AUDIO_H
