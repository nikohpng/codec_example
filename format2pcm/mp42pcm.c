#include<stdio.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/imgutils.h>
#include<libswscale/swscale.h>
#include<libavutil/log.h>
#include<libavutil/opt.h>

int main() {

    AVFormatContext *fmtCtx;
    AVInputFormat *inFmt;

    AVCodecContext *codecCtx;
    AVCodec *codec;

    char* input = "../assets/test.mp4";
    char* out = "./out/test.pcm";

    fmtCtx = avformat_alloc_context();

    if(avformat_open_input(&fmtCtx, input, inFmt, NULL) < 0) {
        av_log(fmtCtx, AV_LOG_ERROR, "Fail to open file %s", input);
        return -1;
    }

    if(avformat_find_stream_info(fmtCtx, NULL) < 0) {
        av_log(fmtCtx, AV_LOG_ERROR, "Fail to find stream!");
        return -1;
    }

    int idx = 0;
    int streamIdx = idx;
    for( idx = 0; idx < fmtCtx -> nb_streams; idx++) {
        if (fmtCtx->streams[idx]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIdx = idx;
        }
    }

    AVCodecParameters *params = fmtCtx->streams[streamIdx]->codecpar;
    codec = avcodec_find_decoder(params->codec_id);
    
    if (codec == NULL) {
        av_log(fmtCtx, AV_LOG_ERROR, "Can't find encoder!");
        return -1;
    }

    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, params);
    codecCtx->thread_count = 1;

    if(avcodec_open2(codecCtx, codec, NULL) < 0) {
        av_log(fmtCtx, AV_LOG_ERROR, "Fail to open encoder!");
        return -1;
    }

    printf("codec name: %s, channels: %d, sample_rate: %d, sample_format: %d\n", 
                                                                    codec->name, 
                                                                    codecCtx->channels,
                                                                    codecCtx->sample_rate,
                                                                    codecCtx->sample_fmt);

    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    FILE *outFile = fopen(out, "wb");
    int ret = 0;

    while((ret = av_read_frame(fmtCtx, pkt)) >= 0) {

        if(pkt->stream_index == streamIdx) {
            ret = avcodec_send_packet(codecCtx, pkt);

            if(ret < 0) {
                printf("avcodec_send_packet error: %s\n", av_err2str(ret));
                break;
            }

            while( (ret = avcodec_receive_frame(codecCtx, frame)) >= 0) {
                if(ret == AVERROR(EAGAIN)) {
                    printf("avcodec_receive_frame with EAGAIN error: %d\n", ret);
                    continue;
                } else if (ret == AVERROR_EOF) {
                    printf("avcodec_receive_frame end of file\n");
                    break;
                }
                int index = 0, channel = 0;
                int numBytes = av_get_bytes_per_sample(codecCtx->sample_fmt);
                for (index = 0; index < frame->nb_samples; index++)
                {
                    for (channel = 0; channel < codecCtx->channels; channel++)
                    {
                        fwrite((char *)frame->data[channel] + numBytes * index, 1, numBytes, outFile);
                    }
                }
            }
        }
    }
    
    av_frame_free(&frame);
    avcodec_free_context(&codecCtx);
    avformat_free_context(fmtCtx);
    return 0;
}