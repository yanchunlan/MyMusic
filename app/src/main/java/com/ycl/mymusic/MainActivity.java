package com.ycl.mymusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.ycl.myplayer.demo.Demo;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private Demo mDemo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tv = (TextView) findViewById(R.id.sample_text);


        mDemo = new Demo();
        Log.d(TAG, "onCreate: "+mDemo.testFfmpeg());
        tv.setText(mDemo.stringFromJNI());
    }

}
