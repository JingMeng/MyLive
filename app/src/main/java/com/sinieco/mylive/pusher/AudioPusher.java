package com.sinieco.mylive.pusher;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import com.sinieco.mylive.params.AudioParams;

/**
 * @author BaiMeng on 2018/1/2.
 */

public class AudioPusher extends Pusher {
    private AudioParams audioParams ;
    private PushNative pushNative ;
    private boolean isPushing = false;
    private int minBufferSize ;
    private AudioRecord audioRecord ;
    public AudioPusher(AudioParams audioParams, PushNative pushNative) {
        this.audioParams = audioParams ;
        this.pushNative = pushNative ;
        int channelConfig = audioParams.getChannel() == 1 ?
                AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_IN_STEREO ;
        minBufferSize = AudioRecord.getMinBufferSize(audioParams.getSampleRateInHz(),channelConfig,AudioFormat.ENCODING_PCM_16BIT);
        //初始化AudioRecord
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                audioParams.getSampleRateInHz(),
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT,
                minBufferSize);
    }

    @Override
    public void startPush() {
        isPushing = true ;
        pushNative.setAudioOptions(audioParams.getSampleRateInHz(),audioParams.getChannel());
        new Thread(new AudioRecordTask()).start();
    }

    @Override
    public void stopPush() {
        isPushing = false ;
        audioRecord.stop();

    }

    @Override
    public void release() {
        if(audioRecord != null){
            audioRecord.release();
            audioRecord = null ;
        }
    }

    class AudioRecordTask implements Runnable {
        @Override
        public void run() {
            audioRecord.startRecording();
            while (isPushing){
                byte[] buffer = new byte[minBufferSize];
                int len = audioRecord.read(buffer,0,buffer.length);
                if(len >0){
                    pushNative.fireAudio(buffer,len);
                }
            }
        }
    }
}


