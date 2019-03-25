package com.ycl.myplayer.demo.player;

import android.text.TextUtils;

import com.ycl.myplayer.demo.TimeInfoBean;
import com.ycl.myplayer.demo.listener.OnCompleteListener;
import com.ycl.myplayer.demo.listener.OnErrorListener;
import com.ycl.myplayer.demo.listener.OnPauseResumeListener;
import com.ycl.myplayer.demo.listener.OnPrepareListener;
import com.ycl.myplayer.demo.listener.OnTimeInfoListener;
import com.ycl.myplayer.demo.listener.OnloadListener;
import com.ycl.myplayer.demo.log.PlayerLog;
import com.ycl.myplayer.demo.muteenum.MuteEnum;

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

    private String source;
    private static boolean playNext = false;// 下一首

    private static int duration = -1;
    private static int volumePercent = 100;
    private static MuteEnum sMuteEnum = MuteEnum.MUTE_CENTER;

    private OnPrepareListener prepareListener;
    private OnloadListener loadListener;
    private OnPauseResumeListener pauseResumeListener;
    private OnTimeInfoListener timeInfoListener;
    private OnErrorListener errorListener;
    private OnCompleteListener completeListener;

    //存储返回的当前时间，所有时间
    private static TimeInfoBean timeInfoBean; // 因为存在多线程问题，所以保持静态，单列


    public void setSource(String source) {
        this.source = source;
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

        // 开始之前，设置其最开始的配置
        setVolume(volumePercent);
        setMute(sMuteEnum);
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
        // 因为停止音乐是耗时操作，所以添加了线程停止
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
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
        if (duration < 0) {
            duration = n_duration();
        }
        return duration;
    }

    public void setVolume(int percent) {
        if (percent >= 0 && percent <= 100) {
            volumePercent = percent;
            n_volume(percent);
        }
    }

    public int getVolumePercent() {
        return volumePercent;
    }

    public void setMute(MuteEnum mute) {
        sMuteEnum = mute;
        n_mute(mute.getValue());
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


    private native void n_prepared(String source);

    private native void n_start();

    private native void n_resume();

    private native void n_pause();

    private native void n_stop();

    private native void n_seek(int secds);

    private native int  n_duration( );

    private native void n_volume(int percent);

    private native void n_mute(int mute);

}
