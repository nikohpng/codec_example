#include<stdio.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/imgutils.h>
#include<libswscale/swscale.h>
#include<libavutil/log.h>
#include<libavutil/opt.h>

int main(int argc, char* argv[]) {

    AVFormatContext* pFmtCtx;
    AVInputFormat*   pInputFmt;
    AVOutputFormat*  pOutputFmt;

    AVCodec* pCodec;
    AVCodecContext* pCodecCtx;

    char* inputStr = "./assets/lena.mp4";
    char* outputStr = "./out/lena.yuv";

    pFmtCtx = avformat_alloc_context();
    
    if (avformat_open_input(&pFmtCtx, inputStr, NULL, NULL) < 0) {
        av_log(pFmtCtx, AV_LOG_ERROR, "Fail to open input %s", inputStr);
        return -1;
    }

    if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
        av_log(pFmtCtx, AV_LOG_ERROR, "Fail to find strema info !");
        return -1;
    }

    int i = 0;
    int stmIdx = 0;
    for (i = 0; i < pFmtCtx ->nb_streams; i++) {
        if (pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stmIdx = i;
        }
    }

    AVCodecParameters* pCodecPar = pFmtCtx->streams[stmIdx]->codecpar;
    pCodec = avcodec_find_decoder(pCodecPar->codec_id);

    if (pCodec == NULL) {
        printf("could not find Codec");
        return -1;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    // pCodecCtx = pFmtCtx->streams[stmIdx]->codec;
    avcodec_parameters_to_context(pCodecCtx, pCodecPar);
    pCodecCtx->thread_count = 1;


    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        av_log(pCodecCtx, AV_LOG_ERROR, "Fail to open decoder!");
        return -1;
    }

    struct SwsContext *pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, 
                                         pCodecCtx->pix_fmt,
                                         pCodecCtx->width,
                                         pCodecCtx->height,
                                         AV_PIX_FMT_YUV420P,
                                         SWS_BILINEAR, NULL, NULL, NULL);
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);  
    uint8_t* pOutBuffer = (uint8_t*) av_malloc(bufferSize);
    AVFrame* pFrame = av_frame_alloc();
    AVFrame* pYUVFrame = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();
    av_init_packet(pkt);
    
    av_image_fill_arrays(pYUVFrame->data,
                         pYUVFrame->linesize,
                         pOutBuffer,
                         AV_PIX_FMT_YUV420P,
                         pCodecCtx->width,
                         pCodecCtx->height,
                         1);

    FILE* out = fopen(outputStr, "wb");

    int ret = 0;

    while((ret = av_read_frame(pFmtCtx, pkt)) >= 0) {

        printf("pkt ret = %d\n", ret);

        ret = avcodec_send_packet(pCodecCtx, pkt);
        // avcodec_send_packet(pCodecCtx, NULL);
        if (pkt->flags &AV_PKT_FLAG_KEY) {
            printf("is key frame %d!\n",pkt->data[4] & 0x1f);
        }
        
        ret = avcodec_receive_frame(pCodecCtx, pFrame);
        if (ret == AVERROR(EAGAIN)) {
            printf("avcodec_receive_frame with EAGAIN error: %d\n", ret);
            continue;
        } else if (ret == AVERROR_EOF) {
            printf("avcodec_receive_frame end of file\n");
            break;
        }

        // int gotPicture = avcodec_receive_frame(pCodecCtx, pFrame);
        // printf("gotPicture=%d, %s\n", gotPicture, av_err2str(gotPicture));
        // int gotPicture = 0;
        // avcodec_decode_video2(pCodecCtx, pFrame, &gotPicture, pkt);
        if (ret == 0) {
            sws_scale(pSwsCtx, 
                      (const uint8_t *const *)pFrame->data, 
                      pFrame->linesize,
                      0,
                      pFrame->height,
                      pYUVFrame->data,
                      pYUVFrame->linesize);
            int ySize = pCodecCtx->width * pCodecCtx->height;
            fwrite(pYUVFrame->data[0], 1, ySize, out);    //Y
            fwrite(pYUVFrame->data[1], 1, ySize / 4, out);  //U
            fwrite(pYUVFrame->data[2], 1, ySize / 4, out);  //V
        }
        av_packet_unref(pkt);
        break;
    }

    printf("last ret %d %s", ret, av_err2str(ret));
    

    fclose(out);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecCtx);
    avformat_free_context(pFmtCtx);
    // free(pOutBuffer);

    return 1;
}