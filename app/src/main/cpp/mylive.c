//
// @author admin on 2018/1/3.
//
#include <jni.h>
#include "com_sinieco_mylive_pusher_PushNative.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <pthread.h>
#include <android/log.h>
#include "x264.h"
#include "rtmp.h"
#include "faac.h"
#include "queue.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

//错误码
#define CONNECT_FAILED 101
#define INIT_FAILED 102

//x264编码输入图像YUV420P
x264_picture_t pic_in ;
x264_picture_t pic_out ;
//YUV个数
int y_len,u_len,v_len ;

x264_t *video_encode_handle ;

//条件变量和同步锁
pthread_cond_t cond;
pthread_mutex_t mutex;

JavaVM *javaVM ;
jobject jobj_push_native;
jclass jcls_push_native;
jmethodID jmid_throw_native_error;

unsigned long nInputSamples; //输入的采样个数
unsigned long nMaxOutputBytes; //编码输出之后的字节数

char *rtmp_path;

unsigned int start_time;

int is_pushing = FALSE;

faacEncHandle audio_encode_handle;

void add_rtmp_packet(RTMPPacket *packet){
    pthread_mutex_lock(&mutex);
    if(is_pushing){
        queue_insert_last(packet);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

jint JNI_OnLoad(JavaVM* vm, void *reserved){
    javaVM = vm ;
    return JNI_VERSION_1_4;
}

void throwNativeError(JNIEnv *env,int code){
    (*env)->CallVoidMethod(env,jobj_push_native,jmid_throw_native_error,code);
}
void add_aac_sequence_header(void){
    //获取aac头信息的长度
    unsigned char *buf;
    unsigned long len;
    faacEncGetDecoderSpecificInfo(audio_encode_handle,&buf,&len);
    int body_size = 2 + len;
    RTMPPacket *packet = malloc(sizeof(RTMPPacket));
    //RTMPPacket初始化
    RTMPPacket_Alloc(packet,body_size);
    RTMPPacket_Reset(packet);
    unsigned char *body = packet->m_body;
    //配置头信息
    /* AF 00 + AAC RAW data */
    body[0] = 0xAF;
    body[1] = 0x00;
    memcpy(&body[2],buf,len);
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nTimeStamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    add_rtmp_packet(packet);
    free(buf);
}

void add_264_sequence_header(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len){
    int body_size = 16 + sps_len + pps_len ;   //H264标准配置需占用16字节
    RTMPPacket *packet = malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(packet,body_size);
    RTMPPacket_Reset(packet);

    unsigned char *body = packet->m_body ;
    int i = 0 ;
    body[i++] =  0x17 ; //VideoHeaderTag:FrameType(1=key frame)+CodecID(7=AVC) 1表示关键帧，7表示AVC格式
    body[i++] = 0x00 ; //AVCPacketType = 0 表示AVCDecoderConfigurationRecord
    //composition time 0x000000 24bit
    body[i++] = 0x00 ;
    body[i++] = 0x00 ;
    body[i++] = 0x00 ;

    //AVCDecoderConfigurationRecoder
    body[i++] = 0x01 ; //configurationVersion,版本为1
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    //
    body[i++] = 0xFF ; //lengthSizeMinusOne,H264视频中NALU的长度，计算方法是 1 + (lengthSizeMinusOne & 3),实际测试时发现总为FF，计算结果为4.

    //sps
    body[i++] = 0xE1 ;
    body[i++] = (sps_len >> 8) & 0xff ; //sequenceParameterSetLength:SPS的长度
    body[i++] = sps_len & 0xff ; //sequenceParameterSetNALUnits
    memcpy(&body[i],sps,sps_len);
    i += sps_len ;

    //pps
    body[i++] = 0x01;
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = (pps_len) & 0xff; //PPS
    memcpy(&body[i],pps,pps_len);
    i += pps_len;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = body_size;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nChannel = 0x04;  //Channel ID ,Audio和Video通道
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    add_rtmp_packet(packet);
}
//添加AAC rtmp packet
void add_aac_body(unsigned char *buf,int len){
    int body_size = 2 + len;
    RTMPPacket *packet = malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(packet,body_size);
    RTMPPacket_Reset(packet);
    unsigned char* body = packet->m_body;
    //配置头信息
    // AF 00 + AAC RAW data
    body[0] = 0xAF;  //10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit)
    // :1=16-bit samples,SoundType(1bit):1=Stereo sound
    body[1] = 0x01;//AACPacketType :1表示AAC raw数据
    memcpy(&body[2],buf,len);
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = RTMP_GetTime() - start_time;
    add_rtmp_packet(packet);



}
void *push_thread(void* args){
    JNIEnv *env;
    (*javaVM)->AttachCurrentThread(javaVM,&env,NULL);
    //建立RTMP连接
    RTMP *rtmp = RTMP_Alloc();
    if(!rtmp){
        __android_log_print(ANDROID_LOG_ERROR,"RTMP","%s","初始化失败");
        goto end;
    }
    RTMP_Init(rtmp);
    rtmp->Link.timeout = 5;  //连接超时时间
    RTMP_SetupURL(rtmp,rtmp_path); //设置流媒体服务器地址
    __android_log_print(ANDROID_LOG_ERROR,"RTMP流媒体地址","%s",rtmp_path);
    RTMP_EnableWrite(rtmp);  //发布rtmp数据流
    if(!RTMP_Connect(rtmp,NULL)){
        __android_log_print(ANDROID_LOG_ERROR,"RTMP","%s","连接失败");
        //TODO 抛出异常
        goto end;
    }
    //计时
    start_time = RTMP_GetTime();
    if(!RTMP_ConnectStream(rtmp,0)){ //连接流
        __android_log_print(ANDROID_LOG_ERROR,"RTMP","%s","连接流失败");
        //TODO 抛出异常
        goto end;
    }
    is_pushing = TRUE;
    add_aac_sequence_header();
    while (is_pushing){
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond,&mutex);
        RTMPPacket *packet = get_first_element();
        if(packet){
            delete_first_element();
            packet->m_nInfoField2 = rtmp->m_stream_id; //RTMP协议，stream_id数据
            int i = RTMP_SendPacket(rtmp,packet,TRUE); //TRUE放入librtmp队列中，并不是立即发送
            if(!i) {
                __android_log_print(ANDROID_LOG_ERROR,"RTMP","%s","断开");
                RTMPPacket_Free(packet);
                pthread_mutex_unlock(&mutex);
                goto end;
            } else {
                __android_log_print(ANDROID_LOG_ERROR,"RTMP","%s","发送数据");
            }
            RTMPPacket_Free(packet);
        }

        pthread_mutex_unlock(&mutex);
    }

    end:
    __android_log_print(ANDROID_LOG_ERROR,"END    ","%s","释放资源");
    free(rtmp_path);
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    (*javaVM)->DetachCurrentThread(javaVM);
    return 0;
}


JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_startPush
        (JNIEnv *env, jobject jobj, jstring jst_url){
    //多线程中使用的变量都需要提升为全局变量（只有jobject类型的变量可以提升为全局变量）
    //这里的jobj是PushNative
    jobj_push_native = (*env)->NewGlobalRef(env,jobj);
    jclass jcls_push_native_tmp = (*env)->GetObjectClass(env,jobj);
    jcls_push_native = (*env)->NewGlobalRef(env,jcls_push_native_tmp);
    jmid_throw_native_error = (*env)->GetMethodID(env,jcls_push_native_tmp,
                                                       "throwNativeError","(I)V");
    const char* url_cstr = (*env)->GetStringUTFChars(env,jst_url,NULL);
    //复制Url地址
    rtmp_path = malloc(strlen(url_cstr)+1);
    memset(rtmp_path,0,strlen(url_cstr)+1);
    memcpy(rtmp_path,url_cstr,strlen(url_cstr)+1);

    //初始化互斥锁和条件变量
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
    //创建对列
    create_queue();
    pthread_t push_thread_id;
    //TODO 子线程函数指针
    pthread_create(&push_thread_id,NULL,push_thread,NULL);
    (*env)->ReleaseStringUTFChars(env,jst_url,url_cstr);
}


/*
 * Class:     com_sinieco_mylive_pusher_PusherNative
 * Method:    stopPush
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_stopPush
        (JNIEnv *env, jobject jobj){
        is_pushing = FALSE;
}

/*
 * Class:     com_sinieco_mylive_pusher_PusherNative
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_release
        (JNIEnv *env, jobject jobj){

}

/*
 * Class:     com_sinieco_mylive_pusher_PusherNative
 * Method:    setViedeoOptions
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_setViedeoOptions
        (JNIEnv *env, jobject jobj, jint width, jint height , jint bit_rate, jint fps){
      __android_log_print(ANDROID_LOG_ERROR,"设置视频参数","%s","开始");
        x264_param_t param ;
        //预设x264参数，ultrafast表示超级快，zerolatency表示0延迟
        x264_param_default_preset(&param,"ultrafast","zerolatency");
        //设置输入的像素编码格式为YUV420P
        param.i_csp = X264_CSP_I420 ;
        param.i_width = width ;
        param.i_height = height ;

        y_len = width * height ;
        u_len = y_len / 4 ;
        v_len = u_len ;

        //参数i_rc_method表示码率控制，CQP（恒定质量），CRF（恒定码率），ABR（平均码率）
        //恒定码率会尽量控制在固定码率
        param.rc.i_rc_method = X264_RC_CRF ;
        param.rc.i_bitrate = bit_rate /1000 ;
        param.rc.i_vbv_max_bitrate = bit_rate / 1000 * 1.2 ; //瞬时最大码率

        //码率控制不通过timebase（时间基）和timestamp（时间戳），而是通过fps
        param.b_vfr_input = 0 ;
        param.i_fps_num = fps ;  //帧率分子
        param.i_fps_den = 1 ;    //帧率分母
        param.i_timebase_den = param.i_fps_num ;
        param.i_timebase_num = param.i_fps_den ;
        param.i_threads = 1 ; //并行编码线程数量，0默认为多线程

        //是否把SPS和PPS放入每一个关键帧
        //SPS Sequence Parameter Set 序列参数集 ， PPS Picture Parameter Set 图像参数集
        //放入可以提高图像的纠错能力
        param.b_repeat_headers = 1 ;
        //设置level级别
        param.i_level_idc = 51 ;
        //设置profile档次
        //baseline级别，没有B帧
        x264_picture_alloc(&pic_in,param.i_csp,param.i_width,param.i_height);
        video_encode_handle = x264_encoder_open(&param);
        if(video_encode_handle){
                __android_log_print(ANDROID_LOG_ERROR,"获取编码器","%s","成功");
        } else {
            //TODO 抛出异常
        }

}

/*
 * Class:     com_sinieco_mylive_pusher_PusherNative
 * Method:    setAudioOptions
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_setAudioOptions
        (JNIEnv *env, jobject jobj, jint sampleRateInHz, jint numChannels){
        audio_encode_handle = faacEncOpen(sampleRateInHz,numChannels,&nInputSamples,
                                         &nMaxOutputBytes);
        if(!audio_encode_handle){
            __android_log_print(ANDROID_LOG_ERROR,"音频解码器","%s","打开失败");
            return;
        }
        faacEncConfigurationPtr p_config = faacEncGetCurrentConfiguration(audio_encode_handle);
        p_config->mpegVersion = MPEG4; //支持MPEG2和MPEG4，这里支持比较新的MPEG4
        p_config->allowMidside = 1;
        p_config->aacObjectType = LOW;
        p_config->outputFormat = 0;
        p_config->useTns = 1; //时域噪音控制，大概是消除爆音
        p_config->useLfe = 0;
  //      p_config->inputFormat = FAAC_INPUT_16BIT ;
        p_config->quantqual = 100;
        p_config->bandWidth = 0; //频宽
        p_config->shortctl = SHORTCTL_NORMAL;

        if(!faacEncSetConfiguration(audio_encode_handle,p_config)){
            __android_log_print(ANDROID_LOG_ERROR,"音频解码器","%s","配置失败");
            //TODO 抛出异常
            return;
        }
        __android_log_print(ANDROID_LOG_ERROR,"音频解码器","%s","配置成功");
}

/*
 * 将采集到视频数据进行编码
 * Class:     com_sinieco_mylive_pusher_PusherNative
 * Method:    fireViedo
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_fireViedo
        (JNIEnv *env, jobject jobj, jbyteArray video_data){
    //将预览的NV21数据转换为YUV420P
    jbyte *nv21_buffer = (*env)->GetByteArrayElements(env,video_data,NULL);
    jbyte *u = pic_in.img.plane[1];
    jbyte *v = pic_in.img.plane[2];
    //nv21 4:2:0  Formats ,12Bits per Pixel
    //nv21与yuv420p，y个数一致，uv位置对调
    //nv21转yuv420p y=w*h,u=v=w*h/4
    //nv21=yuv yuv420p = yuv y=y u=y+1+1 v = y+1
    memcpy(pic_in.img.plane[0],nv21_buffer,y_len);
    int i ;
    for (i = 0; i < u_len; i++) {
        *(u + i) = *(nv21_buffer + y_len +i * 2 +1);
        *(v + i) = *(nv21_buffer + y_len +i * 2);
    }

    //h264编码得到NALU数组
    x264_nal_t *nal = NULL ;
    int n_nal = -1 ;
    //进行h264编码
    if (x264_encoder_encode(video_encode_handle,&nal,&n_nal,&pic_in,&pic_out) < 0) {
        //编码失败
        return;
    }

    //使用rtmp协议将h264编码后的视频数据发送给流媒体服务器
    //帧分为关键帧和普通帧，为了提高画面的纠错率，关键帧应包含SPS和PPS数据
    int sps_len , pps_len ;
    unsigned char sps[100];
    unsigned char pps[100];
    memset(sps,0,100);  //重置数组
    memset(pps,0,100);

    //遍历NALU数组，根据NALU的类型判断
    for (i = 0; i < n_nal; i++) {
        if (nal[i].i_type == NAL_SPS) {
            sps_len = nal[i].i_payload - 4 ; //数组长度排除起始标志位
            memcpy(sps,nal[i].i_payload + 4,sps_len);//跳过起始标志位
        } else if (nal[i].i_type == NAL_PPS) {
            pps_len = nal[i].i_payload - 4 ;
            memcpy(pps,nal[i].i_payload + 4,pps_len);
            //将ssp和pps数据封装到关键帧中
            add_264_sequence_header(pps,sps,pps_len,sps_len);
        }
    }

}

/*
 * Class:     com_sinieco_mylive_pusher_PusherNative
 * Method:    fireAudio
 * Signature: ([BI)V
 */
JNIEXPORT void JNICALL Java_com_sinieco_mylive_pusher_PushNative_fireAudio
        (JNIEnv *env, jobject jobj, jbyteArray audio_data, jint len){
    int *pcmbuf;
    unsigned char *bitbuf;
    jbyte *b_buffer = (*env)->GetByteArrayElements(env,audio_data,NULL);
    pcmbuf = (short*)malloc(nInputSamples * (sizeof(int)));
    bitbuf = (unsigned char*)malloc(nMaxOutputBytes * sizeof(unsigned char));
    int nByteCount = 0;
    unsigned int nBufferSize = (unsigned int)len / 2;
    unsigned short* buf = (unsigned short*) b_buffer;
    while (nByteCount < nBufferSize){
        int audio_length = nInputSamples;
        if ((nBufferSize + nInputSamples) >= nBufferSize){
           audio_length = nBufferSize - nByteCount;
        }
        int i;
        for (i = 0;i < audio_length; i++) { //每次从实时的pcm音频队列中读出量化位数为8的pcm数据
            int s = ((int16_t *)buf + nByteCount)[i];
            pcmbuf[i] = s << 8 ;//用8个二进制位来表示一个采样量化点（模数转换）
        }
        nByteCount += nInputSamples;
        int byteslen = faacEncEncode(audio_encode_handle,pcmbuf,audio_length,
                                    bitbuf,nMaxOutputBytes);
        if(byteslen <1){
            continue;
        }
        add_aac_body(bitbuf,byteslen);
    }
    (*env)->ReleaseByteArrayElements(env,audio_data,b_buffer,NULL);
    if (bitbuf){
       free(bitbuf);
    }
    if (pcmbuf){
        free(pcmbuf);
    }
}

