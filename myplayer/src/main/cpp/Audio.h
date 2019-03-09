//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_AUDIO_H
#define MYMUSIC_AUDIO_H

#include "Queue.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class Audio {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecPar = NULL;
    Queue *queue = NULL;
    PlayStatus *playStatus = NULL;

public:
    Audio(PlayStatus *playStatus);

    virtual ~Audio();

};



#endif //MYMUSIC_AUDIO_H
