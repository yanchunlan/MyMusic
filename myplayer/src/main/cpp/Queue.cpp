//
// Created by pc on 2019/3/8.
//

#include "Queue.h"

Queue::Queue(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    pthread_mutex_init(&mutexPacket, NULL); // 每个方法都需要锁住，释放完成之后再释放锁
    pthread_cond_init(&condPacket, NULL);
}

Queue::~Queue() {
    clearAVPacket();
//    playStatus = NULL;
//    pthread_mutex_destroy(&mutexPacket);
//    pthread_cond_destroy(&condPacket);
}

int Queue::putAVPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(avPacket);
//    if (LOG_DEBUG) {
//        LOGD("放入一个AVpacket到队里里面， 个数为：%d", queuePacket.size());
//    }
    pthread_cond_signal(&condPacket);  // 主要是get方法没有数据的时候就锁住了，现在放入数据就释放那边的条件锁
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int Queue::getAVPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    while (playStatus != NULL && !playStatus->exit) {
        if (queuePacket.size() > 0) {
            // 从队列取出值赋给它  copy 拷贝
            AVPacket *packet = queuePacket.front();
            if (av_packet_ref(avPacket, packet) == 0) {
                queuePacket.pop();
            }
            // copy只是copy引用，并计数，当引用大于0，就不会释放copy的部分内存，
            // 如果在外面再次释放了，就计数减一，就会真正释放了
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;

//            if (LOG_DEBUG) {
//                LOGD("从队列里面取出一个AVpacket，还剩下 %d 个", queuePacket.size());
//            }
            break;
        } else {
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int Queue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

void Queue::clearAVPacket() {
    //先释放锁，在锁住当前释放的部分，在释放
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()) {
        AVPacket *packet = queuePacket.front(); // 取出数据并释放数据
        queuePacket.pop();// 弹出数据
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);
}

