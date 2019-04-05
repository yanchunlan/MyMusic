//
// Created by pc on 2019/3/24.
//

#ifndef MYMUSIC_VIDEO_H
#define MYMUSIC_VIDEO_H

#include "Queue.h"
#include "CallJava.h"
#include "Audio.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


#define CODEC_YUV 0
#define CODEC_MEDIACODEC 1

class Video {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecPar = NULL;

    Queue *queue = NULL;
    PlayStatus *playStatus = NULL;
    CallJava *callJava = NULL;
    AVRational time_base; // 视频帧的 time_base
    pthread_t thread_play;// 视频解码

    Audio *audio = NULL; // 外部赋值给当前,因为初始化的时候不知道有没有，所以外部赋值
    double clock = 0; // 当前视频帧的时间
    double delayTime = 0; // 睡眠时间
    double defaultDelayTime = 0.04;
    pthread_mutex_t codecMutex; // 用于软解码加锁
    int codecType = CODEC_YUV;
    AVBSFContext *abs_ctx = NULL;

public:

    Video(PlayStatus *playStatus, CallJava *callJava);

    virtual ~Video();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket);

    double getDelayTime(double diff);

};

#endif //MYMUSIC_VIDEO_H
