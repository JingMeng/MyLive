package com.sinieco.mylive.pusher;

import android.graphics.Camera;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.sinieco.mylive.params.AudioParams;
import com.sinieco.mylive.params.VideoParmas;

/**
 * @author BaiMeng on 2018/1/2.
 */

public class LivePusher implements SurfaceHolder.Callback {
    private SurfaceHolder surfaceHolder ;
    private AudioPusher audioPusher ;
    private VideoPusher videoPusher ;
    private PushNative pushNative ;

    public LivePusher(SurfaceHolder surfaceHolder) {
        this.surfaceHolder = surfaceHolder;
        this.surfaceHolder.addCallback(this);
        prepare();
    }

    private void prepare() {
        pushNative = new PushNative() ;
        VideoParmas videoParmas = new VideoParmas(480,320, android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK);
        videoPusher = new VideoPusher(surfaceHolder,videoParmas,pushNative);

        //使用默认的采样率和声道个数
        AudioParams audioParams = new AudioParams();
        audioPusher = new AudioPusher(audioParams,pushNative);
    }

    public void switchCamera(){
        videoPusher.switchCamera();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPush();
        release();
    }

    public void startPush(String url){
        videoPusher.startPush();
        audioPusher.startPush();
        pushNative.startPush(url);
    }

    private void release() {
        videoPusher.release();
        audioPusher.release();
        pushNative.release();
    }

    private void stopPush() {
        videoPusher.stopPush();
        audioPusher.stopPush();
        pushNative.stopPush();
    }
}
