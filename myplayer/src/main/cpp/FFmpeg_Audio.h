//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_FFMPEG_AUDIO_H
#define MYMUSIC_FFMPEG_AUDIO_H

#include <pty.h>
#include "CallJava.h"
#include "Audio.h"
#include <pthread.h>

extern "C" {
#include <libavutil/time.h>
#include "libavformat/avformat.h"
};

class FFmpeg_Audio {
public:
    CallJava *callJava = NULL;
    char *url = NULL;
    pthread_t pthread_decode;
    AVFormatContext *avFormatContext = NULL; // 存储数据的 context
    Audio *audio = NULL;
    PlayStatus *playStatus = NULL;

    // 目的是在停止的时候防止，还未初始化完成 （主要是avformat_network_init 初始化网络会耗时）
    pthread_mutex_t init_mutex; // 这个锁的目的是锁住 初始化 ，保证不会异常
    bool exit = false;


    FFmpeg_Audio(PlayStatus *playStatus, CallJava *callJava, const char *url);

    virtual ~FFmpeg_Audio();

    void prepared();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t seek); // 长度更长才能seek
};


#endif //MYMUSIC_FFMPEG_AUDIO_H
