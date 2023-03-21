#include<stdio.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/log.h>
#include<libavutil/opt.h>
#include<libswresample/swresample.h>
#include<libavutil/samplefmt.h>

// PCM的原始参数
#define PCM_IN_FORMAT AV_SAMPLE_FMT_S16
#define PCM_IN_CHANNELS 1
#define PCM_IN_SAMPLE_RATE 8000

int main(int argc, char *argv[]) {
    AVFormatContext *pOutFormatCtx = NULL;
    AVOutputFormat *pOutputFmt = NULL;
    AVStream *pOutStream = NULL;

    AVCodec *pCodec = NULL;
    AVCodecContext *pCodecContext = NULL;

    AVFrame *pFrame = NULL;
    AVFrame *pBufFrame = NULL;
   
    int ret = 0;

    char *outUrl = "./out/test.mp3";
    FILE *inFile = fopen("./assets/test.pcm", "rb+");

    pOutFormatCtx = avformat_alloc_context();
    pOutputFmt = av_guess_format(NULL, outUrl, NULL);


    pOutFormatCtx->oformat = pOutputFmt;
    if(avio_open(&pOutFormatCtx->pb, outUrl, AVIO_FLAG_READ_WRITE) < 0) {
        printf("open file error");
        return -1;
    }

    pOutStream = avformat_new_stream(pOutFormatCtx, NULL);
    if(pOutStream == NULL) {
        printf("create stream error!");
        return -1;
    }

    AVCodecParameters *codecPara = pOutFormatCtx->streams[pOutStream->index]->codecpar;
    codecPara->codec_type = AVMEDIA_TYPE_AUDIO;
    codecPara->codec_id = pOutputFmt->audio_codec;
    codecPara->sample_rate = 8000;
    codecPara->channel_layout = AV_CH_LAYOUT_MONO;
    // codecPara->bit_rate = 128000;
    codecPara->format = AV_SAMPLE_FMT_S16;
    codecPara->channels = av_get_channel_layout_nb_channels(codecPara->channel_layout);

    pCodec = avcodec_find_encoder(pOutputFmt->audio_codec);
    if(!pCodec) {
        printf("create codec error!");
        return -1;
    }

    pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext,codecPara);

    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        printf("open codec error !");
        return -1;
    }

    AVRational outTb;
    outTb.num = 1;
    outTb.den = pCodecContext->sample_rate;
    pOutStream->id = pOutFormatCtx->nb_streams - 1;
    pOutStream->time_base = outTb;

    av_dump_format(pOutFormatCtx, 0, outUrl, 1);
    
    // 创建Frame用于接受重采样后的pcm数据
    pFrame = av_frame_alloc();
    pFrame->format = pCodecContext->sample_fmt;
    pFrame->channel_layout = pCodecContext->channel_layout;
    pFrame->nb_samples = pCodecContext->frame_size;
    pFrame->sample_rate = pCodecContext->sample_rate;

    if (av_frame_get_buffer(pFrame, 0) < 0) {
        printf("Audio Frame get buffer error\n");
        return;
    }

    // 创建一个buffer，用于存储来自pcm的数据
    pBufFrame = av_frame_alloc();
    pBufFrame->format = PCM_IN_FORMAT;
    pBufFrame->nb_samples = pFrame->nb_samples;  // 此处必须与目的采样率一样，匹配format的采样率
    pBufFrame->channel_layout = (uint64_t)av_get_default_channel_layout(PCM_IN_CHANNELS);
    if (av_frame_get_buffer(pBufFrame, 0) < 0) {
        printf("Buf Frame get buffer error\n");
        return;
    }

    int readSize = av_samples_get_buffer_size(NULL, pBufFrame->channels, pBufFrame->nb_samples,
    pBufFrame->format, 1);
    uint8_t *pcmBuf = (uint8_t*)av_malloc(readSize);
    

    // 初始化重采样
    struct SwrContext *swrCtx = swr_alloc_set_opts(NULL,
                           pFrame->channel_layout,
                           pFrame->format,
                           pFrame->sample_rate,
                           av_get_default_channel_layout(PCM_IN_CHANNELS),
                           PCM_IN_FORMAT,
                           PCM_IN_SAMPLE_RATE,0,NULL);
    swr_init(swrCtx);


    avformat_write_header(pOutFormatCtx, NULL);

    AVPacket *pkt = av_packet_alloc();
    pkt->data = NULL;
    pkt->size = 0;

    int i = 0;
    int audioPts = 0;
    for(i=0;;i++){

        // 用来编码的帧
        AVFrame* encodeFrame = NULL;

        //读PCM：特意注意读取的长度，否则可能出现转码之后声音变快或者变慢
        if(fread(pcmBuf,1,readSize,inFile)<=0){
            printf("Cannot read raw data from file.\n");
            return -1;
        }else if(!feof(inFile)){
            // 将pcmBuf连接到frame的data中，用于后续存储数据
            av_samples_fill_arrays(pBufFrame->data, pBufFrame->linesize, (const uint8_t*)pcmBuf, pBufFrame->channels, pBufFrame->nb_samples, pBufFrame->format, 1);

            // 重采样数据并复制到pFrame中
            swr_convert(swrCtx, pFrame->data, pFrame->nb_samples, (const uint8_t**)pBufFrame->data, pBufFrame->nb_samples);

            // 设置pts时间戳
            pFrame->pts = audioPts;
            AVRational dstTb;
            dstTb.num = 1;
            dstTb.den = pCodecContext->frame_size;
            audioPts += av_rescale_q(pFrame->nb_samples, dstTb, pCodecContext->time_base);
            encodeFrame = pFrame;
        } else {
            encodeFrame = NULL;
        }

         // 发送一个frame
        if (avcodec_send_frame(pCodecContext, encodeFrame) < 0) {
            printf("send frame exception.\n");
            return -1;
        }

        while(1){
            int pktRet = avcodec_receive_packet(pCodecContext,pkt);
             // 判断是否完全接受了packet
            if (pktRet == AVERROR(EAGAIN) || pktRet == AVERROR_EOF) {
                break;
            }
             // 检查是否接受异常
            if (pktRet < 0) {
                printf("receive packet exception.\n");
                return -1;
            }

            av_packet_rescale_ts(pkt, pCodecContext->time_base, pOutStream->time_base);
            pkt->stream_index = pOutStream->index;
            av_interleaved_write_frame(pOutFormatCtx, pkt);
            // av_write_frame(pOutFormatCtx,pkt);
            av_packet_unref(pkt);
        }

        // 编码帧为空，则表示已经处理完所有的编码，退出该循环
        if (encodeFrame == NULL) break;
    }

    av_write_trailer(pOutFormatCtx);
    avcodec_free_context(&pCodecContext);
    av_free(pFrame);
    av_free(pcmBuf);
    avio_close(pOutFormatCtx->pb);
    avformat_free_context(pOutFormatCtx);
    fclose(inFile);
}

