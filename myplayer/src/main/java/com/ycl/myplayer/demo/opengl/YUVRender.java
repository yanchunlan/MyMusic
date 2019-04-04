package com.ycl.myplayer.demo.opengl;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

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
public class YUVRender implements GLSurfaceView.Renderer {

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
    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;
    private int textureId;

    private int sampler_y;
    private int sampler_u;
    private int sampler_v;
    private int[] textureId_yuv;

    private int w_yuv;
    private int h_yuv;
    private ByteBuffer y_yuv;
    private ByteBuffer u_yuv;
    private ByteBuffer v_yuv;


    public YUVRender(Context context) {
        this.context = context;
        vertexBuffer = BufferUtils.arr2FloatBuffer(vertexData);
        textureBuffer = BufferUtils.arr2FloatBuffer(textureData);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initRenderYUV();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0, 0, 0, 1);
        renderYUV();
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
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT); // 重复
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR); // 线性
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    private void renderYUV() {
        if (w_yuv > 0 && h_yuv > 0 && y_yuv != null && u_yuv != null && v_yuv != null) {
            GLES20.glUseProgram(program_yuv);

            GLES20.glEnableVertexAttribArray(avPosition_yuv);
            GLES20.glVertexAttribPointer(avPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);

            GLES20.glEnableVertexAttribArray(afPosition_yuv);
            GLES20.glVertexAttribPointer(afPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);


            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, w_yuv, h_yuv,
                    0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y_yuv); // GLES20.GL_LUMINANCE 亮度 y

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, w_yuv / 2, h_yuv / 2,
                    0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u_yuv); // GLES20.GL_LUMINANCE 亮度 y

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, w_yuv / 2, h_yuv / 2,
                    0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v_yuv); // GLES20.GL_LUMINANCE 亮度 y

            GLES20.glUniform1i(sampler_y, 0);
            GLES20.glUniform1i(sampler_u, 1);
            GLES20.glUniform1i(sampler_v, 2);
        }
    }

    public void setYUVRenderData(int w, int h, byte[] y, byte[] u, byte[] v) {
        this.w_yuv = w;
        this.h_yuv = h;
        this.y_yuv = ByteBuffer.wrap(y);
        this.u_yuv = ByteBuffer.wrap(u);
        this.v_yuv = ByteBuffer.wrap(v);
    }
}
