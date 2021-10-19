#include<stdio.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/imgutils.h>
#include<libavutil/opt.h>

int main() {
    AVFormatContext *formatCtx = NULL;
    AVOutputFormat *fmt = NULL;
    AVStream *stream = NULL;
    AVCodecContext *codecCtx = NULL;
    AVCodec *codec = NULL;

    AVFrame *frame = NULL;
    uint8_t *pictureBuf = NULL;
    int size = 0;

    FILE *inFile = fopen("./assets/lena_256x256_yuv420p.yuv", "rb+");
    char *outPath = "./out/lena.mp4";
    
    formatCtx = avformat_alloc_context();

    fmt = av_guess_format(NULL, outPath, NULL);

    if (fmt == NULL) {
        return -1;
    }
    formatCtx->oformat = fmt;

    if(avio_open(&formatCtx->pb, outPath, AVIO_FLAG_READ_WRITE) < 0) {
        printf("fail open file");
        return -1;
    }

    stream = avformat_new_stream(formatCtx, NULL);
    if (stream == NULL) {
        printf("create stream error");
        return -1;
    }

    codec = avcodec_find_encoder(fmt->video_codec);
    if (codec == NULL) {
        printf("can't find encoder!");
        return -1;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (codecCtx == NULL) {
        printf("can't create codec context");
        return -1;
    }

    codecCtx->codec_id = fmt->video_codec;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    codecCtx->width = 256;
    codecCtx->height = 256;
    codecCtx->time_base.num = 1;
    codecCtx->time_base.den = 25;
    codecCtx->bit_rate = 40000;
    codecCtx->gop_size = 12;

    if (codecCtx->codec_id == AV_CODEC_ID_H264) {
        codecCtx->qmin = 10;
        codecCtx->qmax = 51;
        codecCtx->qcompress = 0.6;
    }

    if (codecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        codecCtx->max_b_frames = 2;
    }
    if (codecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        codecCtx->mb_decision = 2;
    }

    
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        printf("can't open encoder!");
        return -1;
    }

    av_dump_format(formatCtx, 0, outPath, 1);
    
    frame = av_frame_alloc();
    frame->width = codecCtx->width;
    frame->height = codecCtx->height;
    frame->format = codecCtx->pix_fmt;

    size = av_image_get_buffer_size(codecCtx->pix_fmt, codecCtx->width, codecCtx->height, 1);
    pictureBuf = (uint8_t*)av_malloc(size);
    av_image_fill_arrays(frame->data, frame->linesize, pictureBuf, codecCtx->pix_fmt, codecCtx->width, codecCtx->height, 1);

    formatCtx->streams[0]->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    formatCtx->streams[0]->codecpar->codec_id = codecCtx->codec_id;
    formatCtx->streams[0]->codecpar->width = codecCtx->width;
    formatCtx->streams[0]->codecpar->height = codecCtx->height;
    avformat_write_header(formatCtx, NULL);

    
    AVPacket packet;
    int ySize = codecCtx->width * codecCtx->height;
    av_new_packet(&packet, size);

    int i = 0;
    for(i = 0 ; i< 1; i++) {
        if(fread(pictureBuf, 1, ySize*3/2, inFile) <= 0) {
            printf("read yuv raw err!");
            return -1;
        } else if(feof(inFile)) {
            break;
        }

        frame->data[0] = pictureBuf;
        frame->data[1] = pictureBuf + ySize;
        frame->data[2] = pictureBuf + ySize * 5 / 4;

        frame->pts = i;


        avcodec_send_frame(codecCtx, frame);

        int ret = avcodec_receive_packet(codecCtx, &packet);
        if(ret == 0) {
            printf("Succeed to encode frame");
           
            packet.stream_index = stream->index;
            av_write_frame(formatCtx, &packet);
        }

    }
    av_packet_unref(&packet);

    av_write_trailer(formatCtx);
    avcodec_free_context(&codecCtx);
    av_free(frame);
    av_free(pictureBuf);
    avio_close(formatCtx->pb);
    avformat_free_context(formatCtx);
    fclose(inFile);



    return 1;
}