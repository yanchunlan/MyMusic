package com.ycl.myplayer.demo;

/**
 * author:  ycl
 * date:  2019/1/30 15:31
 * desc:
 */
public class Demo {

    static {
        System.loadLibrary("native-lib");
    }

    public  native String stringFromJNI();
    public  native String testFfmpeg();
}
