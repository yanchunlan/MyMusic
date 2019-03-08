//
// Created by pc on 2019/3/8.
//

#ifndef MYMUSIC_QUEUE_H
#define MYMUSIC_QUEUE_H

#include <queue>
#include <libavcodec/avcodec.h>
#include <pthread.h>
#include "AndroidLog.h"

class Queue {
public:
    std::queue<AVPacket*> queuePacket;
    pthread_cond_t condPacket;
    pthread_mutex_t mutexPacket;

    Queue();

    virtual ~Queue();

    int putAVPacket(AVPacket *avPacket);

    AVPacket getAVPacket();

};

#endif //MYMUSIC_QUEUE_H
