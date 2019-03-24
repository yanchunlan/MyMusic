//
// Created by pc on 2019/3/8.
//



#include "FFmpeg_Audio.h"

FFmpeg_Audio::FFmpeg_Audio(PlayStatus *playStatus, CallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->url = static_cast<char *>(malloc(512));
    strcpy(this->url, url);
    this->callJava = callJava;

    exit = false;
    pthread_mutex_init(&init_mutex, NULL);
}

FFmpeg_Audio::~FFmpeg_Audio() {
    pthread_mutex_destroy(&init_mutex); // 关闭锁
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


int avformat_callback(void *ctx) {
    FFmpeg_Audio *fFmpeg_audio = static_cast<FFmpeg_Audio *>(ctx);
    if (fFmpeg_audio->playStatus->exit) { // 为true就返回错误，不在执行后续操作
        return AVERROR_EOF;
    }
    return 0;
};


void FFmpeg_Audio::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);

    av_register_all();
    avformat_network_init();
    avFormatContext = avformat_alloc_context();

    avFormatContext->interrupt_callback.callback = avformat_callback;
    avFormatContext->interrupt_callback.opaque = this;


    if (avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open url :%s", url)
        }
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not find streams from %s", url)
        }
        callJava->onCallError(CHILD_THREAD, 1002, "can not find streams from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
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

                audio->duration =
                        avFormatContext->duration / AV_TIME_BASE; // AV_TIME_BASE = 1000000  1S
                audio->time_base = avFormatContext->streams[i]->time_base;
            }
        }
    }
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("can not find audio streamIndex from %s", url)
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    AVCodec *avCodec = avcodec_find_decoder(audio->codecPar->codec_id);
    if (!avCodec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    audio->avCodecContext = avcodec_alloc_context3(avCodec);
    if (!audio->avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decode_ctx");
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decoderctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    // 音频填充到context里面
    if (avcodec_parameters_to_context(audio->avCodecContext, audio->codecPar) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decoderctx");
            callJava->onCallError(CHILD_THREAD, 1005, "can not fill decoderctx");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }


    if (avcodec_open2(audio->avCodecContext, avCodec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        callJava->onCallError(CHILD_THREAD, 1002, "can not open audio streams");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    // 准备完成  防止释放了，还在执行
   /* if (callJava != NULL) {
        if (playStatus != NULL && !playStatus->exit) {

        } else {
            callJava->onCallError(CHILD_THREAD, 1002, "can not find decoder");
            exit = true;
        }
    }*/
    callJava->onCallPrepared(CHILD_THREAD);
    pthread_mutex_unlock(&init_mutex);
}

void FFmpeg_Audio::start() {
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("audio is null");
            callJava->onCallError(CHILD_THREAD, 1006, "audio is null");
            return;
        }
    }
    audio->play();

//    int count = 0;
    while (playStatus != NULL && !playStatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(avFormatContext, avPacket) == 0) { // 一帧一帧读取
            if (avPacket->stream_index == audio->streamIndex) {
                audio->queue->putAVPacket(avPacket); // 入队
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
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

    //解码完成，就设置true
    exit = true;

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

void FFmpeg_Audio::release() {
    if (LOG_DEBUG) {
        LOGE("开始释放Ffmpe");
    }

    if (playStatus->exit) {
        return;
    }
    if (LOG_DEBUG) {
        LOGE("开始释放Ffmpe2");
    }
    playStatus->exit = true;


    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000)  // 暂停1s之后在释放，给一个等待的时间
        {
            exit = true;
        }
        if (LOG_DEBUG) {
            LOGE("wait ffmpeg  exit %d", sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }

    if (LOG_DEBUG) {
        LOGE("释放 Audio");
    }
    if (audio != NULL) {
        audio->release();
        delete (audio);
        audio = NULL;
    }

    if (LOG_DEBUG) {
        LOGE("释放 封装格式上下文");
    }
    if (avFormatContext != NULL) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = NULL;
    }
    if (LOG_DEBUG) {
        LOGE("释放 callJava");
    }
    if (callJava != NULL) {
        callJava = NULL;
    }
    if (LOG_DEBUG) {
        LOGE("释放 playstatus");
    }
    if (playStatus != NULL) {
        playStatus = NULL;
    }

    if (url != NULL) {
        free(url);
        url = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
    if (LOG_DEBUG) {
        LOGE("释放 end");
    }
}

void FFmpeg_Audio::seek(int64_t seek) {

}

