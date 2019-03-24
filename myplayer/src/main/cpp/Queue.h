//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_QUEUE_H
#define MYMUSIC_QUEUE_H

#include <queue>
extern "C"{
#include <libavcodec/avcodec.h>
};
#include <pthread.h>
#include "AndroidLog.h"
#include "PlayStatus.h"

class Queue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_cond_t condPacket;
    pthread_mutex_t mutexPacket;
    PlayStatus *playStatus = NULL;

    Queue(PlayStatus *playStatus);

    virtual ~Queue();

    int putAVPacket(AVPacket *avPacket);

    int getAVPacket(AVPacket *avPacket);

    int getQueueSize();

    void clearAVPacket();

};

#endif //MYMUSIC_QUEUE_H
