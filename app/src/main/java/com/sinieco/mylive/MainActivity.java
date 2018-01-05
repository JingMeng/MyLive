package com.sinieco.mylive;

import android.Manifest;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;

import com.sinieco.mylive.pusher.LivePusher;

public class MainActivity extends AppCompatActivity {
    private static String URL = "";
    private LivePusher live ;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surface);
        XPermissionUtils.requestPermissions(this, 0, new String[]{Manifest.permission.CAMERA}, new XPermissionUtils.OnPermissionListener() {
            @Override
            public void onPermissionGranted() {
                live = new LivePusher(surfaceView.getHolder());
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
}
