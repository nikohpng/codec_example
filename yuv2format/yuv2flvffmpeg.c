#include<stdio.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include<libavutil/opt.h>

int main(int argc, char *argv[]) {
    AVFormatContext *pOutFormatCtx = NULL;
    AVOutputFormat *pOutputFmt = NULL;
    AVStream *pOutStream = NULL;

    AVCodec *pCodec = NULL;
    AVCodecContext *pCodecContext = NULL;

    AVFrame *pFrame = NULL;
    uint8_t *pictureBuf = NULL;

    char *outUrl = "./out/lena.flv";
    FILE *inFile = fopen("./assets/lena_256x256_yuv420p.yuv", "rb+");
    int bufferSize = 0;

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
    pCodec = avcodec_find_encoder(pOutputFmt->video_codec);
    if(!pCodec) {
        printf("create codec error!");
        return -1;
    }

    pCodecContext = avcodec_alloc_context3(pCodec);
    
    // avcodec_open2时会进行参数赋值
    // pCodecContext->codec_id = pOutputFmt->video_codec;
    // pCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecContext->width = 256;
    pCodecContext->height = 256;
    pCodecContext->bit_rate = 400000;
    pCodecContext->gop_size = 12;
    pCodecContext->time_base = (AVRational) {1, 25};

    if (pCodecContext->codec_id == AV_CODEC_ID_H264) {
        pCodecContext->qmin = 10;
        pCodecContext->qmax = 51;
        pCodecContext->max_b_frames = 3;
    }

    


    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        printf("open codec error !");
        return -1;
    }

    av_dump_format(pOutFormatCtx, 0, outUrl, 1);
    
    
    pFrame = av_frame_alloc();
    pFrame->width = pCodecContext->width;
    pFrame->height = pCodecContext->height;
    pFrame->format = pCodecContext->pix_fmt;

    bufferSize = av_image_get_buffer_size(pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height, 1);
    pictureBuf = (uint8_t*) malloc(bufferSize);
    av_image_fill_arrays(pFrame->data, pFrame->linesize, pictureBuf, pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height, 1);

    pOutFormatCtx->streams[0]->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    pOutFormatCtx->streams[0]->codecpar->codec_id = pCodecContext->codec_id;
    pOutFormatCtx->streams[0]->codecpar->width = pCodecContext->width;
    pOutFormatCtx->streams[0]->codecpar->height = pCodecContext->height;

    avformat_write_header(pOutFormatCtx, NULL);

    AVPacket packet;
    int ySize = pCodecContext->width * pCodecContext->height;
    av_new_packet(&packet,ySize);

    int i = 0;
    for( i = 0; i < 1; i++) {
        
        if (fread(pictureBuf, 1, ySize * 3 / 2, inFile) <= 0) {
            printf("read picture raw error!");
            return -1;
        } else if (feof(inFile)) {
            break;
        }
        pFrame->data[0] = pictureBuf;
        pFrame->data[1] = pictureBuf + ySize;
        pFrame->data[2] = pictureBuf + ySize * 5 / 4;

        pFrame->pts = i;

        avcodec_send_frame(pCodecContext, pFrame);

        int ret = avcodec_receive_packet(pCodecContext, &packet);

        if(ret == 0) {
            printf("Succeed to encode frame");
           
            packet.stream_index = pOutStream->index;

            av_write_frame(pOutFormatCtx, &packet);
        }

        av_packet_unref(&packet);
        av_write_trailer(pOutFormatCtx);
        avcodec_free_context(&pCodecContext);
        av_free(pFrame);
        av_free(pictureBuf);
        avio_close(pOutFormatCtx->pb);
        avformat_free_context(pOutFormatCtx);
        fclose(inFile);
    }
}