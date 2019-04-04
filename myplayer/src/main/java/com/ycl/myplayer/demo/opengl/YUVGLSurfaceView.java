package com.ycl.myplayer.demo.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * author:  ycl
 * date:  2019/4/4 16:39
 * desc:
 */
public class YUVGLSurfaceView extends GLSurfaceView {
    private YUVRender yuvRender;

    public YUVGLSurfaceView(Context context) {
        this(context, null);
    }

    public YUVGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        yuvRender = new YUVRender(context);
        setRenderer(yuvRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int w, int h, byte[] y, byte[] u, byte[] v) {
        if (yuvRender != null) {
            yuvRender.setYUVRenderData(w, h, y, u, v);
            requestRender();
        }
    }
}
