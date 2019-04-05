//
// Created by pc on 2019/3/24.
//


#include "Video.h"

Video::Video(PlayStatus *playStatus, CallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new Queue(playStatus);
    pthread_mutex_init(&codecMutex, NULL);
}

Video::~Video() {
    pthread_mutex_destroy(&codecMutex);
}

void *playVideo(void *data) {
    Video *video = static_cast<Video *>(data);

    while (video->playStatus != NULL && !video->playStatus->exit) {
        // seek
        if (video->playStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }
        if (video->playStatus->pause) {
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


        if (video->codecType == CODEC_MEDIACODEC) { // 硬解码
            // 过滤器 sendPacket
            if (av_bsf_send_packet(video->abs_ctx, avPacket)!=0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            // 从过滤器中 receivePacket
            while (av_bsf_receive_packet(video->abs_ctx, avPacket) != 0) {
                LOGE("开始硬解码");
                double diff = video->getFrameDiffTime(NULL, avPacket);
                LOGE("diff is %f", diff); //用于观察，当前视频延时率高不高

                av_usleep(video->getDelayTime(diff * 1000000));
                video->callJava->onCallDecodeAVPacket(avPacket->size, avPacket->data);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = NULL;

        } else if (video->codecType == CODEC_YUV) { // 软解码
            pthread_mutex_lock(&video->codecMutex);

            // packet放到解码器解码
            if (avcodec_send_packet(video->avCodecContext, avPacket) != 0) { // error
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            AVFrame *avFrame = av_frame_alloc();
            if (avcodec_receive_frame(video->avCodecContext, avFrame) != 0) {
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;

                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }

            LOGE("子线程解码一个AVframe成功");
            // 针对不是420p的帧格式，就需要重采样
            if (avFrame->format == AV_PIX_FMT_YUV420P) {
                LOGE("当前视频是YUV420p格式");
                double diff = video->getFrameDiffTime(avFrame, NULL);
                LOGE("diff is %f", diff); //用于观察，当前视频延时率高不高

                av_usleep(video->getDelayTime(diff * 1000000));
                video->callJava->onCallRenderYUV(video->avCodecContext->width,
                                                 video->avCodecContext->height,
                                                 avFrame->data[0],
                                                 avFrame->data[1],
                                                 avFrame->data[2]);
            } else {
                LOGE("当前视频不是YUV420p格式");
                AVFrame *pFrameYUV420P = av_frame_alloc();

                int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                   video->avCodecContext->width,
                                                   video->avCodecContext->height,
                                                   1);
                // 指定pFrameYUV420P的内存大小
                uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
                av_image_fill_arrays(pFrameYUV420P->data,
                                     pFrameYUV420P->linesize,
                                     buffer,
                                     AV_PIX_FMT_YUV420P,
                                     video->avCodecContext->width,
                                     video->avCodecContext->height,
                                     1);
                SwsContext *swsContext = sws_getContext(
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        video->avCodecContext->pix_fmt,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        AV_PIX_FMT_YUV420P,
                        SWS_BICUBIC, NULL, NULL, NULL);
                // 重采样初始化失败
                if (!swsContext) {
                    av_frame_free(&pFrameYUV420P);
                    av_free(pFrameYUV420P);
                    pFrameYUV420P = NULL;
                    av_free(buffer);
                    buffer = NULL; // 及时释放buffer的数据
                    pthread_mutex_unlock(&video->codecMutex);
                    continue;
                }
                // 开始重采样 将avFrame的数据采样给pFrameYUV420P
                sws_scale(swsContext,
                          avFrame->data,
                          avFrame->linesize,
                          0,
                          avFrame->height,
                          pFrameYUV420P->data,
                          pFrameYUV420P->linesize);
                // 渲染
                double diff = video->getFrameDiffTime(avFrame, NULL); // 此处也可以取pFrameYUV420P
                LOGE("diff is %f", diff); //用于观察，当前视频延时率高不高

                av_usleep(video->getDelayTime(diff * 1000000));
                video->callJava->onCallRenderYUV(video->avCodecContext->width,
                                                 video->avCodecContext->height,
                                                 pFrameYUV420P->data[0],
                                                 pFrameYUV420P->data[1],
                                                 pFrameYUV420P->data[2]);
                // 释放pFrameYUV420P
                av_frame_free(&pFrameYUV420P);
                av_free(pFrameYUV420P);
                pFrameYUV420P = NULL;
                av_free(buffer);
                buffer = NULL; // 及时释放buffer的数据
                sws_freeContext(swsContext);
            }
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&video->codecMutex);
        }
    }
    /*pthread_exit(&video->thread_play);*/
    return 0;
};


void Video::play() {
    if (playStatus != NULL && !playStatus->exit) {
        pthread_create(&thread_play, NULL, playVideo, this);
    }
}

void Video::release() {
    // 釋放隊列的数据
    if (queue != NULL) {
        queue->noticeQueue();
    }
    pthread_join(thread_play, NULL);

    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }
    // 在里面释放，则外部就不再释放，变量一定是在当前类里面释放，其他地方的引用只需要==NULL
    if (abs_ctx != NULL) {
        av_bsf_free(&abs_ctx);
        abs_ctx = NULL;
    }
    if (avCodecContext != NULL) {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        pthread_mutex_unlock(&codecMutex);
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (audio != NULL) {
        audio = NULL;
    }
}

// 时间差=音频时间-视频时间
double Video::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {
    double pts = 0;
    if (avFrame != NULL) {
        pts = av_frame_get_best_effort_timestamp(avFrame);
        // ffmpeg 4.0 之后使用pts
//        pts = avFrame->pts;
    }
    if (avPacket != NULL) {
        pts = avPacket->pts;
    }
    if (pts == AV_NOPTS_VALUE) { // 没有值得情况就赋值0
        pts = 0;
    }
    pts *= av_q2d(time_base); // *时间基=真实时间
    if (pts > 0) {
        clock = pts;
    }
    double diff = audio->clock - clock;
    return diff;
}

// 当前视频帧需要的 休眠时间
double Video::getDelayTime(double diff) {
    if (diff > 0.003) { // 减少睡眠时间
        delayTime = delayTime * 2 / 3;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003) { // 增加睡眠时间
        delayTime = delayTime * 3 / 2;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }

    } else if (diff == 0.003) { // 不处理，标准睡眠时间
    }

    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }

    if (fabs(diff) >= 10) { // 音频超视频太多了，直接睡眠最大的默认睡眠时间
        delayTime = defaultDelayTime;
    }

    LOGE("delayTime is %f", delayTime);

    return delayTime;
}
