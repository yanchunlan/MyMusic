package com.ycl.myplayer.demo;

/**
 * author:  ycl
 * date:  2019/1/30 15:31
 * desc:
 */
public class Demo {

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

    public  native String stringFromJNI();
    public  native String testFfmpeg();
}
