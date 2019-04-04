//
// Created by pc on 2019/3/8.
//



#include "FFmpeg_Audio.h"

FFmpeg_Audio::FFmpeg_Audio(PlayStatus *playStatus, CallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->url = static_cast<char *>(malloc(512));
    strcpy(this->url, url);
    this->callJava = callJava;

//    exit = false;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}

FFmpeg_Audio::~FFmpeg_Audio() {
    pthread_mutex_destroy(&init_mutex); // 关闭锁
    pthread_mutex_destroy(&seek_mutex); // 关闭锁
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
                duration = audio->duration;
            }
        } else if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video == NULL) {
                video = new Video(playStatus, callJava);
                video->streamIndex = i;
                video->codecPar = avFormatContext->streams[i]->codecpar;
                video->time_base = avFormatContext->streams[i]->time_base;
            }
        }
    }
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("can not find audio")
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find audio");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    // 初始化 audio->avCodecContext
    getCodecContext(audio->codecPar, &audio->avCodecContext);

    if (video == NULL) {
        if (LOG_DEBUG) {
            LOGE("can not find video")
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find video");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    // 初始化 video->avCodecContext
    getCodecContext(video->codecPar, &video->avCodecContext);


    callJava->onCallPrepared(CHILD_THREAD);
    pthread_mutex_unlock(&init_mutex);
}

void FFmpeg_Audio::start() {
    if (audio == NULL) {
        if (LOG_DEBUG) {
            LOGE("audio is null");
            callJava->onCallError(CHILD_THREAD, 1008, "audio is null");
            return;
        }
    }
    if (video == NULL) {
        if (LOG_DEBUG) {
            LOGE("audio is null");
            callJava->onCallError(CHILD_THREAD, 1009, "video is null");
            return;
        }
    }


    audio->play();
    video->play();


    while (playStatus != NULL && !playStatus->exit) {
        // 设置了seek就继续
        if (playStatus->seek) {
            av_usleep(1000 * 10); // 在seek的时候，不再解码，需要休眠
            continue;
        }
        if (audio->queue->getQueueSize() > 40) {//直播缓冲区更小，一般的播放器可以设置到100
            av_usleep(1000 * 10);
            continue;
        }


        AVPacket *avPacket = av_packet_alloc();

        // 读取帧加锁
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(avFormatContext, avPacket);
        pthread_mutex_unlock(&seek_mutex);

        if (ret == 0) { // 一帧一帧读取
            if (avPacket->stream_index == audio->streamIndex) {
                audio->queue->putAVPacket(avPacket); // 入队
            } else if (avPacket->stream_index == video->streamIndex) {
                video->queue->putAVPacket(avPacket); // 入队
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        } else { // 在播放的时候
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            while (playStatus != NULL && !playStatus->exit) {
                if (audio->queue->getQueueSize() > 0) {// 在播放的时候，需要睡眠
                    av_usleep(1000 * 100);
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
            break;
        }
    }

    // && 播放完成退出之后就会执行到这里，否则一直在while里面

    // 回调完成的监听
    if (callJava != NULL) {
        callJava->onCallComplete(CHILD_THREAD);
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

    // ------------ 音频解码相关 start ---------------
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
    // 如果没有解码完成，代码还在解码，需要让其执行一段时间通过上面的状态去停止解码
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
    if(LOG_DEBUG)
    {
        LOGE("释放 video");
    }
    if (video != NULL) {
        video->release();
        delete (video);
        video = NULL;
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
    // ------------ 音频解码相关 end ---------------


    // ------------ 视频解码相关 start ---------------



    // ------------ 视频解码相关 end ------------------



    if (LOG_DEBUG) {
        LOGE("释放 end");
    }
}

void FFmpeg_Audio::seek(int64_t secds) {
    if (duration <= 0) {// 为获取到时间就return
        return;
    }
    if (audio != NULL) {
        // 开启seek条件,让解码不再继续
        playStatus->seek = true;
        audio->queue->clearAVPacket();
        audio->clock = 0;
        audio->last_time = 0;

        // 开始设置文件seek位置
        pthread_mutex_lock(&seek_mutex);
        int64_t rel = secds * AV_TIME_BASE;//真实地时间
        avformat_seek_file(avFormatContext, -1, INT64_MIN, rel, INT64_MAX, 0);// seek到那里去
        pthread_mutex_unlock(&seek_mutex);

        playStatus->seek = false; // 继续解码
    }

}

int FFmpeg_Audio::getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext) {
    AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
    if (!avCodec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    // 此处返回的值是一级指针，虽然返回了值，但是是付给当前函数了，外部传递进来的还是没有值得
    // 所以外部传递2级指针，就是它的地址进来，这样就不会被函数改变了
    *avCodecContext = avcodec_alloc_context3(avCodec);
    if (!*avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decode_ctx");
        }
        callJava->onCallError(CHILD_THREAD, 1005, "can not alloc new decoderctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    // 音频填充到context里面
    if (avcodec_parameters_to_context(*avCodecContext, codecpar) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decoderctx");
        }
        callJava->onCallError(CHILD_THREAD, 1006, "can not fill decoderctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if (avcodec_open2(*avCodecContext, avCodec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        callJava->onCallError(CHILD_THREAD, 1007, "can not open audio streams");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}

