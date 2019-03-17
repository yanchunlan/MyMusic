package com.ycl.myplayer.demo.player;

import android.text.TextUtils;

import com.ycl.myplayer.demo.TimeInfoBean;
import com.ycl.myplayer.demo.listener.OnPauseResumeListener;
import com.ycl.myplayer.demo.listener.OnPrepareListener;
import com.ycl.myplayer.demo.listener.OnTimeInfoListener;
import com.ycl.myplayer.demo.listener.OnloadListener;
import com.ycl.myplayer.demo.log.PlayerLog;

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
    private OnPrepareListener prepareListener;
    private OnPauseResumeListener pauseResumeListener;
    private OnloadListener loadListener;
    private OnTimeInfoListener timeInfoListener;
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

    public void parpared() {
        if (null == source || TextUtils.isEmpty(source)) {
            PlayerLog.d("source is empty");
            return;
        }
        onCallLoad(true); // 开始准备就显示加载中
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
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
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


    private native void n_prepared(String source);

    private native void n_start();

    private native void n_resume();

    private native void n_pause();

}
