//
// Created by pc on 2019/3/24.
//

#include "Video.h"

Video::Video(PlayStatus *playStatus, CallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new Queue(playStatus);
}

Video::~Video() {
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
}

void *playVideo(void *data){
    Video *video = static_cast<Video *>(data);

    while (video->playStatus != NULL && !video->playStatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAVPacket(avPacket) == 0) {
            // 解码渲染
            LOGE("线程中获取视频AVpacket");
        }
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }

    pthread_exit(&video->thread_play);
};


void Video::play() {
    pthread_create(&thread_play, NULL, playVideo, this);
}
