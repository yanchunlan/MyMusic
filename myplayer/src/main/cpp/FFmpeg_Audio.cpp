//
// Created by pc on 2019/3/8.
//


#include "FFmpeg_Audio.h"

FFmpeg_Audio::FFmpeg_Audio(PlayStatus *playStatus, CallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->url = static_cast<char *>(malloc(512));
    strcpy(this->url, url);
    this->callJava = callJava;
}

FFmpeg_Audio::~FFmpeg_Audio() {
//    playStatus = NULL;
//     callJava = NULL;
//     uri = NULL;
//     pthread_decode=NULL;
//     avFormatContext = NULL;
//     audio = NULL;
    free(url);
}

void *decodeFFmpeg(void *data) {
    FFmpeg_Audio *fFmpeg_audio = static_cast<FFmpeg_Audio *>(data);
    fFmpeg_audio->decodeFFmpegThread();
    // 线程执行完成退出线程
    pthread_exit(&fFmpeg_audio->pthread_decode);
};


void FFmpeg_Audio::prepared() {
    pthread_create(&pthread_decode, NULL, decodeFFmpeg, this);
}

void FFmpeg_Audio::decodeFFmpegThread() {
    av_register_all();
    avformat_network_init();
    avFormatContext = avformat_alloc_context();
    if (avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open url :%s", url)
        }
        return;
    }
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not find streams from %s", url)
        }
        return;
    }
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new Audio(playStatus,
                        avFormatContext->streams[i]->codecpar->sample_rate,
                        callJava);
                audio->streamIndex = i;
                audio->codecPar = avFormatContext->streams[i]->codecpar;
            }
        }
    }
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("can not find audio streamIndex from %s", url)
        }
        return;
    }
    AVCodec *avCodec = avcodec_find_decoder(audio->codecPar->codec_id);
    if (!avCodec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        return;
    }
    audio->avCodecContext = avcodec_alloc_context3(avCodec);
    if (!audio->avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decode_ctx");
        }
        return;
    }

    if (avcodec_open2(audio->avCodecContext, avCodec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        return;
    }
    // 准备完成
    callJava->onCallPrepared(CHILD_THREAD);
}

void FFmpeg_Audio::start() {
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("audio is null");
            return;
        }
    }
    audio->play();

//    int count = 0;
    while (playStatus != NULL && !playStatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(avFormatContext, avPacket) == 0) { // 一帧一帧读取
            if (avPacket->stream_index == audio->streamIndex) {
//                count++;
//                if (LOG_DEBUG) {
//                    LOGE("解码第 %d 帧", count);
//                }
                audio->queue->putAVPacket(avPacket); // 入队
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
           /* if (LOG_DEBUG) {
                LOGE("audio is null");
            }*/
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playStatus != NULL && !playStatus->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
            break;
        }
    }
    if (LOG_DEBUG) {
        LOGE("解码完成");
    }
}

void FFmpeg_Audio::pause() {
    if (audio != NULL) {
        audio->pause();
    }
}

void FFmpeg_Audio::resume() {
    if (audio != NULL) {
        audio->resume();
    }
}

