package com.sinieco.mylive;

import android.Manifest;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;

import com.sinieco.mylive.listener.LiveStateChangeListener;
import com.sinieco.mylive.pusher.LivePusher;

public class MainActivity extends AppCompatActivity implements LiveStateChangeListener {
    private static String URL = "rtmp://101.132.176.79/live/baimeng";
    private LivePusher live ;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surface);
        XPermissionUtils.requestPermissions(this, 0, new String[]{Manifest.permission.CAMERA,
                Manifest.permission.RECORD_AUDIO},
                new XPermissionUtils.OnPermissionListener() {
            @Override
            public void onPermissionGranted() {
                live = new LivePusher(surfaceView.getHolder(),MainActivity.this);
            }

            @Override
            public void onPermissionDenied() {

            }
        });
    }

    //开始直播按钮
    public void mStartLive(View view){
        live.startPush(URL);
    }

    //切换摄像头按钮
    public void mSwitchCamera(View view){
        live.switchCamera();
    }

    //此方法在Native子线程中调用，需要Handler发送消息到java主线程中调用
    @Override
    public void onError(int code) {

    }
}
