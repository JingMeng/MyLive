package com.sinieco.mylive.pusher;

import com.sinieco.mylive.listener.LiveStateChangeListener;

/**
 * @author BaiMeng on 2018/1/2.
 */

public class PushNative {

    private LiveStateChangeListener mLiveStateChangeListener ;

    public native void startPush(String url);

    public native void stopPush();

    public native void release();

    public void setLiveStateChangeListener(LiveStateChangeListener listener){
        this.mLiveStateChangeListener = listener ;
    }

    public void throwNativeError(int code){
        if(mLiveStateChangeListener != null){
            mLiveStateChangeListener.onError(code);
        }
    }

    /**
     * 设置视频宽高
     * @param width
     * @param height
     */
    public native void setViedeoOptions(int width ,int height ,int bitRate ,int fps);

    /**
     * 设置音频采样率和声道布局
     * @param sampleRateInHz
     * @param channel
     */
    public native void setAudioOptions(int sampleRateInHz ,int channel);

    /**
     * 发送视频数据
     * @param data
     */
    public native void fireViedo(byte[] data);

    /**
     * 发送音频数据
     * @param data
     * @param len
     */
    public native void fireAudio(byte[] data , int len);

    static {
        System.loadLibrary("mylive");
    }

}
