package com.sinieco.mylive.pusher;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.view.SurfaceHolder;

import com.sinieco.mylive.params.VideoParmas;

import java.io.IOException;

/**
 * @author BaiMeng on 2018/1/2.
 */

public class VideoPusher extends Pusher implements Camera.PreviewCallback ,SurfaceHolder.Callback{
    private SurfaceHolder surfaceHolder ;
    private VideoParmas videoParmas ;
    private PushNative pushNative ;
    private byte[] buffers ;
    private boolean isPushing = false ;
    private Camera mCamera ;
    public VideoPusher(SurfaceHolder surfaceHolder, VideoParmas videoParmas, PushNative pushNative) {
        this.surfaceHolder = surfaceHolder ;
        this.videoParmas = videoParmas ;
        this.pushNative = pushNative ;
        surfaceHolder.addCallback(this);
    }

    @Override
    public void startPush() {
        pushNative.setViedeoOptions(videoParmas.getWidth(),videoParmas.getHeight(),videoParmas.getBitRate(),videoParmas.getFps());
        isPushing = true ;
    }

    @Override
    public void stopPush() {
        isPushing = false ;
    }

    @Override
    public void release() {
        stopPush();
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if(mCamera != null){
            mCamera.addCallbackBuffer(buffers);
        }
        if(isPushing){
            pushNative.fireViedo(data);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        startPreview();
    }

    //开始预览
    private void startPreview() {
        //设置相机
        try {
            mCamera = Camera.open(videoParmas.getCameraId());
            Camera.Parameters parameters = mCamera.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);  //设置预览图像的像素格式NV21
            parameters.setPreviewSize(videoParmas.getWidth(),videoParmas.getHeight());  //设置预览图像的宽高
            parameters.setPreviewFpsRange(videoParmas.getBitRate()-1,videoParmas.getBitRate());  //设置预览刷新率
            mCamera.setPreviewDisplay(surfaceHolder);
            buffers = new byte[videoParmas.getWidth() * videoParmas.getHeight() * 4];
            mCamera.addCallbackBuffer(buffers);
            mCamera.setPreviewCallbackWithBuffer(this);
            mCamera.startPreview();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void switchCamera() {
        if(videoParmas.getCameraId() == Camera.CameraInfo.CAMERA_FACING_BACK){
            videoParmas.setCameraId(Camera.CameraInfo.CAMERA_FACING_FRONT);
        }else {
            videoParmas.setCameraId(Camera.CameraInfo.CAMERA_FACING_BACK);
        }
        stopPreview();
        startPreview();
    }

    private void stopPreview() {
        if(mCamera != null){
            mCamera.stopPreview();
            mCamera.release();
            mCamera = null ;
        }
    }
}
