package com.ycl.myplayer.demo.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.ycl.myplayer.R;
import com.ycl.myplayer.demo.utils.BufferUtils;
import com.ycl.myplayer.demo.utils.ShaderUtil;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * author:  ycl
 * date:  2019/4/4 16:40
 * desc:
 */
public class YUVRender implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {
    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIACODEC = 2;

    private Context context;
    private final float[] vertexData = {
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f
    };
    private final float[] textureData = {
            0f, 1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;
    private int renderType = RENDER_YUV;

    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;

    private int sampler_y;
    private int sampler_u;
    private int sampler_v;
    private int[] textureId_yuv;

    private int w_yuv;
    private int h_yuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    // mediaCodec 硬解出來的需要OES存儲
    private int program_codec;
    private int avPosition_codec;
    private int afPosition_codec;
    private int sampleOES_codec;
    private int[] textureId_codec;
    private SurfaceTexture surfaceTexture;
    private Surface surface;


    private OnSurfaceCreateListener onSurfaceCreateListener;
    private OnFrameAvailableListener onFrameAvailableListener;


    public YUVRender(Context context) {
        this.context = context;
        vertexBuffer = BufferUtils.arr2FloatBuffer(vertexData);
        textureBuffer = BufferUtils.arr2FloatBuffer(textureData);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initRenderYUV();
        initRenderMediaCodec();
    }


    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0, 0, 0, 1);

        if (renderType == RENDER_YUV) {
            renderYUV();
        } else if (renderType == RENDER_MEDIACODEC) {
            renderMediaCodec();
        }

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

//        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }


    private void initRenderYUV() {
        String vertexSource = ShaderUtil.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = ShaderUtil.readRawTxt(context, R.raw.fragment_shader);
        program_yuv = ShaderUtil.createProgram(vertexSource, fragmentSource);

        avPosition_yuv = GLES20.glGetAttribLocation(program_yuv, "av_Position");
        afPosition_yuv = GLES20.glGetAttribLocation(program_yuv, "af_Position");

        sampler_y = GLES20.glGetUniformLocation(program_yuv, "sampler_y");
        sampler_u = GLES20.glGetUniformLocation(program_yuv, "sampler_u");
        sampler_v = GLES20.glGetUniformLocation(program_yuv, "sampler_v");

        textureId_yuv = new int[3];
        GLES20.glGenTextures(3, textureId_yuv, 0);
        for (int i = 0; i < 3; i++) {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_REPEAT); // 重复
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_LINEAR); // 线性
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_LINEAR);
        }
    }

    private void initRenderMediaCodec() {
        String vertexSource = ShaderUtil.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = ShaderUtil.readRawTxt(context, R.raw.fragment_mediacodec);
        program_codec = ShaderUtil.createProgram(vertexSource, fragmentSource);

        avPosition_codec = GLES20.glGetAttribLocation(program_yuv, "av_Position");
        afPosition_codec = GLES20.glGetAttribLocation(program_yuv, "af_Position");
        sampleOES_codec = GLES20.glGetUniformLocation(program_yuv, "sTexture");

        textureId_codec = new int[1];
        GLES20.glGenTextures(1, textureId_codec, 0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_codec[0]);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_REPEAT); // 重复
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_LINEAR); // 线性
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);

        surfaceTexture = new SurfaceTexture(textureId_codec[0]);
        surface = new Surface(surfaceTexture);
        surfaceTexture.setOnFrameAvailableListener(this);

        if (onSurfaceCreateListener != null) {
            onSurfaceCreateListener.onSurfaceCreate(surface);
        }
    }

    private void renderYUV() {
        if (w_yuv > 0 && h_yuv > 0 && y != null && u != null && v != null) {
            GLES20.glUseProgram(program_yuv);

            GLES20.glEnableVertexAttribArray(avPosition_yuv);
            GLES20.glVertexAttribPointer(avPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

            GLES20.glEnableVertexAttribArray(afPosition_yuv);
            GLES20.glVertexAttribPointer(afPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);


            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, w_yuv, h_yuv,
                    0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y); // GLES20.GL_LUMINANCE 亮度 y

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, w_yuv / 2, h_yuv / 2,
                    0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u); // GLES20.GL_LUMINANCE 亮度 y

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, w_yuv / 2, h_yuv / 2,
                    0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v); // GLES20.GL_LUMINANCE 亮度 y

            GLES20.glUniform1i(sampler_y, 0);
            GLES20.glUniform1i(sampler_u, 1);
            GLES20.glUniform1i(sampler_v, 2);

            y.clear();
            u.clear();
            v.clear();

            y = null;
            u = null;
            v = null;
        }
    }

    private void renderMediaCodec() {
        surfaceTexture.updateTexImage();
        GLES20.glUseProgram(program_codec);

        GLES20.glEnableVertexAttribArray(avPosition_codec);
        GLES20.glVertexAttribPointer(avPosition_codec, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

        GLES20.glEnableVertexAttribArray(afPosition_codec);
        GLES20.glVertexAttribPointer(afPosition_codec, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_codec[0]);
        // 此處不需要 set TextureImage
        GLES20.glUniform1i(sampleOES_codec, 0);
    }


    public void setYUVRenderData(int w, int h, byte[] y, byte[] u, byte[] v) {
        this.w_yuv = w;
        this.h_yuv = h;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if (onFrameAvailableListener != null) {
            onFrameAvailableListener.onFrameAvailable();
        }
    }

    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener onSurfaceCreateListener) {
        this.onSurfaceCreateListener = onSurfaceCreateListener;
    }

    public void setOnFrameAvailableListener(OnFrameAvailableListener onFrameAvailableListener) {
        this.onFrameAvailableListener = onFrameAvailableListener;
    }

    public interface OnSurfaceCreateListener {
        void onSurfaceCreate(Surface surface);
    }

    public interface OnFrameAvailableListener {
        void onFrameAvailable();
    }
}
