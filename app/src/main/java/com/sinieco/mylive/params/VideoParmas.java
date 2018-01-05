package com.sinieco.mylive.params;

/**
 * @author BaiMeng on 2018/1/2.
 */

public class VideoParmas {
    private int width ;
    private int height ;
    private int cameraId ;
    //码率
    private int bitRate = 480000;
    //屏幕刷新率
    private int fps = 25 ;

    public VideoParmas(int width, int height, int cameraId) {
        this.width = width;
        this.height = height;
        this.cameraId = cameraId;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getCameraId() {
        return cameraId;
    }

    public void setCameraId(int cameraId) {
        this.cameraId = cameraId;
    }

    public int getBitRate() {
        return bitRate;
    }

    public void setBitRate(int bitRate) {
        this.bitRate = bitRate;
    }

    public int getFps() {
        return fps;
    }

    public void setFps(int fps) {
        this.fps = fps;
    }
}
