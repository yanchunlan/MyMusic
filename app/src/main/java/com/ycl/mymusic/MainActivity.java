package com.ycl.mymusic;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.ycl.myplayer.demo.Demo;
import com.ycl.myplayer.demo.TimeInfoBean;
import com.ycl.myplayer.demo.listener.OnPauseResumeListener;
import com.ycl.myplayer.demo.listener.OnPrepareListener;
import com.ycl.myplayer.demo.listener.OnTimeInfoListener;
import com.ycl.myplayer.demo.listener.OnloadListener;
import com.ycl.myplayer.demo.log.PlayerLog;
import com.ycl.myplayer.demo.player.Player;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";
    private Demo mDemo;
    private Player mPlayer;


    private TextView mSampleText;
    private Button mStart;
    private Button mPause;
    private Button mResume;
    private TextView mTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        init();
    }

    private void init() {
        // 测试
        mDemo = new Demo();
        Log.d(TAG, "init: " + mDemo.testFfmpeg());
        mSampleText.setText(mDemo.stringFromJNI());

        // 解码
        mPlayer = new Player();
        mPlayer.setPrepareListener(new OnPrepareListener() {
            @Override
            public void onPrepared() {
                PlayerLog.d("onPrepared 准备 ok");
                mPlayer.start();
            }
        });

        mPlayer.setLoadListener(new OnloadListener() {
            @Override
            public void onLoad(boolean load) {
                if (load) {
                    PlayerLog.d("加载中");
                } else {
                    PlayerLog.d("播放中");
                }
            }
        });

        mPlayer.setPauseResumeListener(new OnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if (pause) {
                    PlayerLog.d("暂停");
                } else {
                    PlayerLog.d("恢复");
                }
            }
        });

        mPlayer.setTimeInfoListener(new OnTimeInfoListener() {
            @Override
            public void onTimeInfo(TimeInfoBean timeInfoBean) {
                PlayerLog.d("currentTime: " + timeInfoBean.getCurrentTime()
                        + " totalTime: " + timeInfoBean.getTotalTime());
                if (mHandler != null) {
                    Message.obtain(mHandler, 1, timeInfoBean).sendToTarget();
                }
            }
        });
    }

    private void initView() {
        mSampleText = (TextView) findViewById(R.id.sample_text);
        mStart = (Button) findViewById(R.id.start);
        mPause = (Button) findViewById(R.id.pause);
        mResume = (Button) findViewById(R.id.resume);
        mTime = (TextView) findViewById(R.id.time);
        mStart.setOnClickListener(this);
        mPause.setOnClickListener(this);
        mResume.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start:
                mPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                mPlayer.parpared();
                break;
            case R.id.pause:
                mPlayer.pause();
                break;
            case R.id.resume:
                mPlayer.resume();
                break;
        }
    }


    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
                if (MainActivity.this.isDestroyed()) {
                    return;
                }
            }
            if (msg.what == 1) {
                TimeInfoBean timeInfoBean = (TimeInfoBean) msg.obj;
                mTime.setText("currentTime: " + timeInfoBean.getCurrentTime() + " totalTime: " + timeInfoBean.getTotalTime());
            }
        }
    };

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mHandler.removeMessages(1);
        mHandler = null;
    }
}
