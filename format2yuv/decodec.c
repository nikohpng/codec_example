#include<stdio.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/imgutils.h>
#include<libswscale/swscale.h>
#include<libavutil/log.h>
#include<libavutil/opt.h>

#include <stdio.h>
#include <time.h>

AVFormatContext* pFormatCtx;
AVInputFormat*   pInputFmt;
AVOutputFormat*  pOutputFmt;

AVCodec* pCodec;
AVCodecContext* pCodecCtx;

AVFrame* pFrame;
AVFrame* pFrameYUV;
AVPacket* pAvPacket;
struct SwsContext* pSwsContext;
char* inputStr = "./assets/lena.mp4";
char* outputStr = "./out/lena.yuv";
FILE* yuv_file;

int InitDecoder(const char *mp4Path) {
    // 1.注册所有组件
    av_register_all();
    // 2.创建AVFormatContext结构体
    pFormatCtx = avformat_alloc_context();

    // 3.打开一个输入文件
    if (avformat_open_input(&pFormatCtx, mp4Path, NULL, NULL) != 0) {
        printf("could not open input stream");
        return -1;
    }
    // 4.获取媒体的信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("could not find stream information");
        return -1;
    }
    //获取视频轨的下标
    int videoIndex = -1;
    int i = 0;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    if (videoIndex == -1) {
        printf("could not find a video stream");
        return -1;
    }
    // 5.查找解码器
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoIndex]->codecpar->codec_id);
    if (pCodec == NULL) {
        printf("could not find Codec");
        return -1;
    }

    // 6.配置解码器
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoIndex]->codecpar);
    pCodecCtx->thread_count = 1;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("could not open codec");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                              pCodecCtx->width,
                                              pCodecCtx->height, 1);
    uint8_t *out_buffer = (unsigned char *) av_malloc(bufferSize);
    av_image_fill_arrays(pFrameYUV->data,
                         pFrameYUV->linesize,
                         out_buffer,
                         AV_PIX_FMT_YUV420P,
                         pCodecCtx->width,
                         pCodecCtx->height, 1);

    pAvPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    pSwsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                 pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                 SWS_BICUBIC, NULL, NULL, NULL);
    return 0;
}

/**
 * 解码
 * @param pCodecCtx
 * @param pAvPacket
 * @param pFrame
 * @return
 */
int DecodePacket(AVCodecContext *pCodecCtx, AVPacket *pAvPacket, AVFrame *pFrame) {
    int result = avcodec_send_packet(pCodecCtx, pAvPacket);
    if (result < 0) {
        printf("send packet for decoding failed");
        return -1;
    }

    while (!result) {
        result = avcodec_receive_frame(pCodecCtx, pFrame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return 0;
        } else if (result < 0) {
            printf("error during encoding %d", result);
            return -1;
        }
        sws_scale(pSwsContext,
                  (const uint8_t *const *) pFrame->data,
                  pFrame->linesize,
                  0,
                  pCodecCtx->height,
                  pFrameYUV->data,
                  pFrameYUV->linesize);

        int y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrameYUV->data[0], 1, y_size, yuv_file);    //Y
        fwrite(pFrameYUV->data[1], 1, y_size / 4, yuv_file);  //U
        fwrite(pFrameYUV->data[2], 1, y_size / 4, yuv_file);  //V
        av_frame_unref(pFrame);
        return 2;
    }
    return 0;
    
}

/**
 * 解码文件
 * @param yuvPath　目标的yuv文件路径
 * @return
 */
int DecodeFile(const char *yuvPath) {
    yuv_file = fopen(yuvPath, "wb+");
    if (yuv_file == NULL) {
        printf("could not open output file");
        return -1;
    }

    while (av_read_frame(pFormatCtx, pAvPacket) >= 0) {
        if (DecodePacket(pCodecCtx, pAvPacket, pFrame)==2) {
            break;
        }
    }

    //收尾
    DecodePacket(pCodecCtx, NULL, pFrame);

    if (pSwsContext != NULL) {
        sws_freeContext(pSwsContext);
        pSwsContext = NULL;
    }
    //关闭文件
    fclose(yuv_file);

    if (pCodecCtx != NULL) {
        avcodec_close(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
    }
    if (pFrame != NULL) {
        av_free(pFrame);
        pFrame = NULL;
    }
    if (pFrameYUV != NULL) {
        av_free(pFrameYUV);
        pFrameYUV = NULL;
    }
    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    return 0;
}

int main() {
    InitDecoder(inputStr);
    DecodeFile(outputStr);
    return 0;
}
