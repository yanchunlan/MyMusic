//
// Created by pc on 2019/3/8.
//

#include "Queue.h"

Queue::Queue() {
    pthread_mutex_init(&mutexPacket, NULL); // 每个方法都需要锁住，释放完成之后再释放锁
    pthread_cond_init(&condPacket, NULL);

}

Queue::~Queue() {
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
}

int Queue::putAVPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(avPacket);
    if (LOG_DEBUG) {

    }

    pthread_cond_signal(&condPacket);  // 主要是get方法没有数据的时候就锁住了，现在放入数据就释放那边的条件锁
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}
