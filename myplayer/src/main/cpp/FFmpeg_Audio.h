//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_FFMPEG_AUDIO_H
#define MYMUSIC_FFMPEG_AUDIO_H

#include <pty.h>
#include "CallJava.h"
#include "Audio.h"
#include <pthread.h>

class FFmpeg_Audio {
public:
    CallJava *callJava = NULL;
    char *url = NULL;
    pthread_t pthread_decode;
    AVFormatContext *avFormatContext = NULL;
    Audio *audio = NULL;
    PlayStatus *playStatus = NULL;


    FFmpeg_Audio(PlayStatus *playStatus,CallJava *callJava, const char *url);

    virtual ~FFmpeg_Audio();

    void prepared();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();
};


#endif //MYMUSIC_FFMPEG_AUDIO_H
