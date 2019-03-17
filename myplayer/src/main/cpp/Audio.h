//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_AUDIO_H
#define MYMUSIC_AUDIO_H

#include "Queue.h"


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

    // 重采样
    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0; // 可以控制播放器的sample_rate

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
    Audio(PlayStatus *playStatus, int sample_rate);

    virtual ~Audio();

    void play();

    int resampleAudio();

    void initOpenSLES();

    SLuint32 getCurrentSampleRateForOpensles(int sample_rate);

};



#endif //MYMUSIC_AUDIO_H
