package com.ycl.mymusic;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.ycl.myplayer.demo.Demo;
import com.ycl.myplayer.demo.listener.OnPrepareListener;
import com.ycl.myplayer.demo.log.PlayerLog;
import com.ycl.myplayer.demo.player.Player;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";
    private Demo mDemo;
    private Player mPlayer;


    private TextView mSampleText;
    private Button mButton;

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
                PlayerLog.d("onPrepared success");
                mPlayer.start();
            }
        });
    }

    private void initView() {
        mSampleText = (TextView) findViewById(R.id.sample_text);
        mButton = (Button) findViewById(R.id.button);

        mButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button:
                mPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
                mPlayer.parpared();
                break;
        }
    }
}
