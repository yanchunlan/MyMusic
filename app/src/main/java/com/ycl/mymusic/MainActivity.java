package com.ycl.mymusic;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.ycl.myplayer.demo.Demo;
import com.ycl.myplayer.demo.TimeInfoBean;
import com.ycl.myplayer.demo.listener.OnCompleteListener;
import com.ycl.myplayer.demo.listener.OnErrorListener;
import com.ycl.myplayer.demo.listener.OnPauseResumeListener;
import com.ycl.myplayer.demo.listener.OnPrepareListener;
import com.ycl.myplayer.demo.listener.OnTimeInfoListener;
import com.ycl.myplayer.demo.listener.OnloadListener;
import com.ycl.myplayer.demo.log.PlayerLog;
import com.ycl.myplayer.demo.muteenum.MuteEnum;
import com.ycl.myplayer.demo.player.Player;
import com.ycl.myplayer.demo.utils.TimeUtils;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";
    private Demo mDemo;
    private Player mPlayer;


    private TextView mSampleText;
    private Button mStart;
    private Button mStop;
    private Button mPause;
    private Button mResume;
    private Button mNext;

    private TextView mTime;
    private SeekBar mSeekbarSeek; // 进度
    private TextView mTvVolume;
    private SeekBar mSeekbarVolume;// 声音

    private Button mLeft;
    private Button mRight;
    private Button mCenter;


    private int position = 0;
    private boolean isSeekBar = false; // 控制seekbar 进度控制唯一

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        initListener();
    }

    private void initListener() {
        // 测试
        mDemo = new Demo();
//        Log.d(TAG, "init: " + mDemo.testFfmpeg());
        mSampleText.setText(mDemo.stringFromJNI());

        // 解码
        mPlayer = new Player();


        mPlayer.setMute(MuteEnum.MUTE_LEFT);

        mPlayer.setVolume(50);
        mTvVolume.setText("音量：" + mPlayer.getVolumePercent() + "%");
        mSeekbarVolume.setProgress(mPlayer.getVolumePercent());


        mPlayer.setPrepareListener(new OnPrepareListener() {
            @Override
            public void onPrepared() {
                PlayerLog.d("准备 ok，开始播放声音");
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
//                PlayerLog.d("currentTime: " + timeInfoBean.getCurrentTime()
//                        + " totalTime: " + timeInfoBean.getTotalTime());
                if (mHandler != null) {
                    Message.obtain(mHandler, 1, timeInfoBean).sendToTarget();
                }
            }
        });

        mPlayer.setErrorListener(new OnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                PlayerLog.d("code: " + code + " msg: " + msg);
            }
        });
        mPlayer.setCompleteListener(new OnCompleteListener() {

            @Override
            public void onComplete() {
                PlayerLog.d(" 播放完成了 onComplete");
            }
        });

        mSeekbarSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (mPlayer.getDuration() > 0 && isSeekBar) {
                    // 根据seekbar百分比占据总时间的多少，设置seek的值
                    position = mPlayer.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeekBar = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                PlayerLog.d(  "seek position: " + position);
                mPlayer.seek(position);
                isSeekBar = false;
            }
        });
        mSeekbarVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mPlayer.setVolume(progress);
                mTvVolume.setText("音量：" + mPlayer.getVolumePercent() + "%");
                PlayerLog.d(  "volume: " + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    private void initView() {
        mSampleText = (TextView) findViewById(R.id.sample_text);
        mStart = (Button) findViewById(R.id.start);
        mStop = (Button) findViewById(R.id.stop);
        mPause = (Button) findViewById(R.id.pause);
        mResume = (Button) findViewById(R.id.resume);
        mNext = (Button) findViewById(R.id.next);
        mTime = (TextView) findViewById(R.id.time);
        mSeekbarSeek = (SeekBar) findViewById(R.id.seekbar_seek);
        mTvVolume = (TextView) findViewById(R.id.tv_volume);
        mSeekbarVolume = (SeekBar) findViewById(R.id.seekbar_volume);
        mLeft = (Button) findViewById(R.id.left);
        mRight = (Button) findViewById(R.id.right);
        mCenter = (Button) findViewById(R.id.center);
        mStart.setOnClickListener(this);
        mPause.setOnClickListener(this);
        mResume.setOnClickListener(this);
        mStop.setOnClickListener(this);
        mNext.setOnClickListener(this);
        mLeft.setOnClickListener(this);
        mRight.setOnClickListener(this);
        mCenter.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start:
//              mPlayer.setSource("/storage/emulated/0/1.mp3");
                mPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
//                mPlayer.setSource("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
                mPlayer.prepared();
                break;
            case R.id.pause:
                mPlayer.pause();
                break;
            case R.id.resume:
                mPlayer.resume();
                break;
            case R.id.stop:
                mPlayer.stop();
                break;
           /* case R.id.seek:
                mPlayer.seek(200);
                break;*/
            case R.id.next:
                mPlayer.playNext("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
                break;
            case R.id.left:
                mPlayer.setMute(MuteEnum.MUTE_LEFT);
                break;
            case R.id.right:
                mPlayer.setMute(MuteEnum.MUTE_RIGHT);
                break;
            case R.id.center:
                mPlayer.setMute(MuteEnum.MUTE_CENTER);
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
                mTime.setText(TimeUtils.secdsToDateFormat(timeInfoBean.getTotalTime(), timeInfoBean.getTotalTime()) + "/" +
                        TimeUtils.secdsToDateFormat(timeInfoBean.getCurrentTime(), timeInfoBean.getTotalTime()));
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
