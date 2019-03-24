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

}

void *playVideo(void *data) {
    Video *video = static_cast<Video *>(data);

    while (video->playStatus != NULL && !video->playStatus->exit) {
        // seek
        if (video->playStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }

        // load
        if (video->queue->getQueueSize() == 0) {// 还没数据显示的时候，显示加载中
            // 如果是没有加载中，就设置加载中
            if (!video->playStatus->load) {
                video->playStatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);// 子线程，显示加载中

                av_usleep(1000 * 100);
                continue;
            } else if (video->playStatus->load) { // 如果有数据就去除加载中的弹框，并显示播放中
                video->playStatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);// 子线程，去掉加载中
                // 去了加载就继续执行
            }
        }

        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAVPacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        // packet放到解码器解码
        if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) { // error
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->avCodecContext, avFrame) == 0) {
            LOGE("子线程解码一个AVframe成功");

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
        } else {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
    }

    pthread_exit(&video->thread_play);
};


void Video::play() {
    pthread_create(&thread_play, NULL, playVideo, this);
}

void Video::release() {
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
}
