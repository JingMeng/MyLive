package com.sinieco.mylive.pusher;

/**
 * @author BaiMeng on 2018/1/2.
 */

public abstract class Pusher {

    public abstract void startPush();

    public abstract void stopPush();

    public abstract void release();
}
