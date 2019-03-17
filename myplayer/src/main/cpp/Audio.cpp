//
// Created by pc on 2019/3/8.
//

#include "Audio.h"


Audio::Audio(PlayStatus *playStatus, int sample_rate) {
    this->playStatus = playStatus;
    this->sample_rate = sample_rate;
    queue = new Queue(playStatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2); // 44100 * 2 * (16/8)
}

Audio::~Audio() {
    playStatus = NULL;
    free(queue);
    free(buffer);
}


void *decodePlay(void *data) {
    Audio *audio = static_cast<Audio *>(data);
    audio->initOpenSLES();
    pthread_exit(&audio->thread_play);
};

void Audio::play() {
    pthread_create(&thread_play, NULL, decodePlay, this);
}

// 转码，重采样
int Audio::resampleAudio() {
    while (playStatus != NULL && !playStatus->exit) {
        avPacket = av_packet_alloc();
        if (queue->getAVPacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        // 获取到队列的数据了

        // packet放到解码器解码
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0) { // error
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        // 发送结束就获得了avFrame
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0) {
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                // 没有声道布局就需要设置声道布局
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            // 重采样
            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(NULL,
                                         AV_CH_LAYOUT_STEREO,
                                         AV_SAMPLE_FMT_S16,
                                         avFrame->sample_rate,
                                         avFrame->channel_layout,
                                         static_cast<AVSampleFormat>(avFrame->format),
                                         avFrame->sample_rate,
                                         NULL, NULL);

            // 初始化
            if (!swr_ctx || swr_init(swr_ctx) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;

                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;

                swr_free(&swr_ctx);
                continue;
            }

            // 转换
            int nb = swr_convert(swr_ctx,
                                 &buffer,
                                 avFrame->nb_samples,
                    (const uint8_t **)(avFrame->data),
                                 avFrame->nb_samples);
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            if (LOG_DEBUG) {
                LOGE("data_size is %d , nb is %d, out_channels is %d", data_size, nb, out_channels);
            }

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            swr_free(&swr_ctx);
            break;  // 为什么是break

        } else { // 释放
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *ctx){
    Audio *audio = static_cast<Audio *>(ctx);
    if (audio != NULL) {
        // 得到pcm数据  存在audio->buffer里面
        int bufferSize = audio->resampleAudio();
        if (bufferSize > 0) {
            // push pcm数据到播放器
            (*audio->pcmBufferQueue)->Enqueue(
                    audio->pcmBufferQueue,
                    audio->buffer, bufferSize); // 可能不是它，会存在比它小的情况，44100 * 2 * 16/8
        }
    }
};


void Audio::initOpenSLES() {

    // 1.创建引擎
    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    // 2.创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }


    // 3. 设置播放器参数和创建播放器
    //    3.1. 配置audioSource
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    //    设置录制规格：PCM、2声道、44100HZ、16bit
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, //立体声 前左前右
            SL_BYTEORDER_LITTLEENDIAN // 结束标识
    };
    SLDataSource audioSrc = {&android_queue, &format_pcm};
    //    3.2. 配置 audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, 0};

    // 创建播放器  , 此处可以创建多个
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &audioSrc,
                                                &audioSnk, 1, ids, req);
    // 实现播放器 得到播放器接口
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);


    // 注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE,
                                     &pcmBufferQueue);
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this); // 缓冲区回调

    //    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);// 播放状态


    //  主动调用回调函数开始工作
    pcmBufferCallBack(pcmBufferQueue, this);
}

SLuint32 Audio::getCurrentSampleRateForOpensles(int sample_rate) {
    SLuint32 rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}
