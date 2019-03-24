//
// Created by pc on 2019/3/24.
//

#ifndef MYMUSIC_VIDEO_H
#define MYMUSIC_VIDEO_H

#include "Queue.h"
#include "CallJava.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

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


    Video(PlayStatus *playStatus, CallJava *callJava);

    virtual ~Video();

    void play();

};

#endif //MYMUSIC_VIDEO_H
