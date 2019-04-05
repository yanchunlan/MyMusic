package com.ycl.myplayer.demo.player;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.text.TextUtils;
import android.view.Surface;

import com.ycl.myplayer.demo.TimeInfoBean;
import com.ycl.myplayer.demo.listener.OnCompleteListener;
import com.ycl.myplayer.demo.listener.OnErrorListener;
import com.ycl.myplayer.demo.listener.OnPauseResumeListener;
import com.ycl.myplayer.demo.listener.OnPrepareListener;
import com.ycl.myplayer.demo.listener.OnTimeInfoListener;
import com.ycl.myplayer.demo.listener.OnloadListener;
import com.ycl.myplayer.demo.log.PlayerLog;
import com.ycl.myplayer.demo.opengl.YUVGLSurfaceView;
import com.ycl.myplayer.demo.opengl.YUVRender;
import com.ycl.myplayer.demo.utils.MediaCodecUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * author:  ycl
 * date:  2019/3/8 10:00
 * desc:
 */
public class Player {
    static {
        System.loadLibrary("native-lib");

        System.loadLibrary("avcodec-58");
        System.loadLibrary("avdevice-58");
        System.loadLibrary("avfilter-7");
        System.loadLibrary("avformat-58");
        System.loadLibrary("avutil-56");
        System.loadLibrary("postproc-55");
        System.loadLibrary("swresample-3");
        System.loadLibrary("swscale-5");
    }

    private static String source;
    private static boolean playNext = false;
    //存储返回的当前时间，所有时间
    private static TimeInfoBean timeInfoBean; // 因为存在多线程问题，所以保持静态，单列
    private int duration = 0;

    private OnPrepareListener prepareListener;
    private OnloadListener loadListener;
    private OnPauseResumeListener pauseResumeListener;
    private OnTimeInfoListener timeInfoListener;
    private OnErrorListener errorListener;
    private OnCompleteListener completeListener;
    private YUVGLSurfaceView yuvglSurfaceView;

    // 硬解参数
    private MediaFormat mediaFormat;
    private MediaCodec mediaCodec;
    private Surface surface;
    private MediaCodec.BufferInfo info;


    public void setSource(String source) {
        this.source = source;
    }

    public String getSource() {
        return source;
    }

    public void setGlSurfaceView(YUVGLSurfaceView yuvglSurfaceView) {
        this.yuvglSurfaceView = yuvglSurfaceView;
    }

    public void setPrepareListener(OnPrepareListener prepareListener) {
        this.prepareListener = prepareListener;
    }

    public void setLoadListener(OnloadListener loadListener) {
        this.loadListener = loadListener;
    }

    public void setPauseResumeListener(OnPauseResumeListener pauseResumeListener) {
        this.pauseResumeListener = pauseResumeListener;
    }

    public void setTimeInfoListener(OnTimeInfoListener timeInfoListener) {
        this.timeInfoListener = timeInfoListener;
    }

    public void setErrorListener(OnErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public void setCompleteListener(OnCompleteListener completeListener) {
        this.completeListener = completeListener;
    }

    public void prepared() {
        if (null == source || TextUtils.isEmpty(source)) {
            PlayerLog.d("source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
            }
        }).start();
    }

    public void start() {
        if (null == source || TextUtils.isEmpty(source)) {
            PlayerLog.d("source is empty");
            return;
        }
        n_start();
    }

    public void pause() {
        n_pause();
        if (pauseResumeListener != null) {
            pauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        n_resume();
        if (pauseResumeListener != null) {
            pauseResumeListener.onPause(false);
        }
    }

    public void stop() {
        timeInfoBean = null;
        duration = 0;
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
                releaseMediaCodec();
            }
        }).start();
    }

    public void seek(int seek) {
        n_seek(seek);
    }


    public void playNext(String url) {
        this.source = url;
        playNext = true;
        stop();
    }

    public int getDuration() {
        return duration;
    }

    public void onCallPrepared() {
        if (prepareListener != null) {
            prepareListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if (loadListener != null) {
            loadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime) {
        if (timeInfoListener != null) {
            if (timeInfoBean == null) {
                timeInfoBean = new TimeInfoBean();
            }
            duration = totalTime;
            timeInfoBean.setCurrentTime(currentTime);
            timeInfoBean.setTotalTime(totalTime);
            timeInfoListener.onTimeInfo(timeInfoBean);
        }
    }

    public void onCallError(int code, String msg) {
        stop();
        if (errorListener != null) {
            errorListener.onError(code, msg);
        }
    }

    public void onCallComplete() {
        stop();
        if (completeListener != null) {
            completeListener.onComplete();
        }
    }

    // 只要点击了下一步就执行 stop，在prepared,开始准备下一个音乐
    public void onCallNext() {
        if (playNext) {
            playNext = false;
            prepared();
        }
    }


    public void onCallRenderYUV(int w, int h, byte[] y, byte[] u, byte[] v) {
        if (yuvglSurfaceView != null) {
            yuvglSurfaceView.getYuvRender().setRenderType(YUVRender.RENDER_YUV);
            yuvglSurfaceView.setYUVData(w, h, y, u, v);
        }
    }

    // ------------------ 硬解 提供給c层调用的方法 start ----------------------------

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public boolean onCallIsSupportMediaCodec(String ffcodecName) {
        return MediaCodecUtils.isSupportCodec(ffcodecName);
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void onCallInitMediaCodec(String codecName, int w, int h, byte[] csd_0, byte[] csd_1) {
        if (surface != null) {
            yuvglSurfaceView.getYuvRender().setRenderType(YUVRender.RENDER_MEDIACODEC);

            String mine = MediaCodecUtils.findVideoCodecName(codecName);
            mediaFormat = MediaFormat.createVideoFormat(mine, w, h);
            mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, w * h);
            mediaFormat.setByteBuffer("csd-0",ByteBuffer.wrap(csd_0));// pps
            mediaFormat.setByteBuffer("csd-1",ByteBuffer.wrap(csd_1));// fps
            PlayerLog.d("mediaFormat： "+mediaFormat.toString());

            try {
                mediaCodec = MediaCodec.createDecoderByType(mine);

                info = new MediaCodec.BufferInfo();
                mediaCodec.configure(mediaFormat, surface, null, 0);
                mediaCodec.start();// 開始解碼
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            if (errorListener != null) {
                errorListener.onError(2001, "surface is null");
            }
        }
    }

    private ByteBuffer getInputBuffer(MediaCodec codec, int index) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            return codec.getInputBuffer(index);
        } else {
            return codec.getInputBuffers()[index];
        }
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void onCallDecodeAVPacket(int dataSize, byte[] data) {
        if (surface != null && mediaCodec != null && dataSize > 0 && data != null) {
            try {
                int inputBufferIndex = mediaCodec.dequeueInputBuffer(10);
                if (inputBufferIndex >= 0) {
                    ByteBuffer buffer = getInputBuffer(mediaCodec, inputBufferIndex);
                    buffer.clear();
                    buffer.put(data);
                    mediaCodec.queueInputBuffer(inputBufferIndex, 0, dataSize, 0, 0);
                }
                // 输入一个可能取出多个
                int outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
                while (outputBufferIndex > 0) {
                    mediaCodec.releaseOutputBuffer(outputBufferIndex, true);
                    outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, 10);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public void releaseMediaCodec() {
        if (mediaCodec != null) {
            try {
                mediaCodec.flush();
                mediaCodec.stop();
                mediaCodec.release();
            } catch (Exception e) {
                e.printStackTrace();
            }

            mediaCodec = null;
            mediaFormat = null;
            info = null;
        }
    }

    // ------------------ 硬解 提供給c层调用的方法 end ----------------------------


    private native void n_prepared(String source);

    private native void n_start();

    private native void n_resume();

    private native void n_pause();

    private native void n_stop();

    private native void n_seek(int secds);
}
