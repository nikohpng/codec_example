#include<stdio.h>
#include<string.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/opt.h>
#include<libavutil/imgutils.h>

int main() {
    AVFormatContext *pFormatCtx;
    AVOutputFormat *fmt;
    AVStream *videoStream; 

    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVPacket pkt;

    AVFrame *pFrame;

    uint8_t *pictureBuf;
    int pictureSize;
    int ySize;
    int framecnt = 0;
    int i = 0;

    FILE *inFile = fopen("./lena_256x256_yuv420p.yuv", "rb+");
    char *outPath = "./lena.h264";

    pFormatCtx = avformat_alloc_context();

    //根据文件名字获取编解码器
    fmt = av_guess_format(NULL, outPath, NULL);
    
    if (fmt == NULL) {
        return -1;
    }

    pFormatCtx -> oformat = fmt;

    //打开输出文件
    if (avio_open(&pFormatCtx->pb, outPath, AVIO_FLAG_READ_WRITE) < 0) {
        printf("fail open output file");
        return -1;
    }

    //创建流
    videoStream = avformat_new_stream(pFormatCtx, NULL);
    if (videoStream == NULL) {
        printf("fail create stream");
        return -1;
    }

    //根据视频的解码器获取编码器
    pCodec = avcodec_find_encoder(fmt->video_codec);
    if (!pCodec) {
        printf("can't find encoder");
        return -1;
    }

    //根据编码器创建编码上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL) {
        printf("can't create codec context");
        return -1;
    }

    // 设置编码上下文的编码参数
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->width = 256;
    pCodecCtx->height = 256;
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->gop_size = 250;

    //时间基
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;


    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;


    pCodecCtx->max_b_frames = 3;

    AVDictionary *param = 0;
    //H264
    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param, "preset", "slow", 0);
        av_dict_set(&param, "tune", "zerolatency", 0);
    }

    //显示一些信息
    av_dump_format(pFormatCtx, 0, "./lena.h264", 1);

    //打开编码器
    if(avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
        printf("open codec err !");
        return -1;
    }

    pFrame = av_frame_alloc();

    pFrame -> format = pCodecCtx -> pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;

    pictureSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
    pictureBuf = (uint8_t *) av_malloc(pictureSize);
    

    //分配内存？ 应该是填充参数内容
    av_image_fill_arrays(pFrame->data, pFrame->linesize, pictureBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);

    //写文件头信息
    pFormatCtx->streams[0]->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    pFormatCtx->streams[0]->codecpar->width = pCodecCtx->width;
    pFormatCtx->streams[0]->codecpar->height = pCodecCtx->height;
    avformat_write_header(pFormatCtx, NULL);

    av_new_packet(&pkt, pictureSize);

    ySize = pCodecCtx->width * pCodecCtx->height;

    for ( i = 0; i < 1; i++) {
        if (fread(pictureBuf, 1, ySize*3/2, inFile) <= 0 ) {
            printf("read yuv raw err!");
            return -1;
        } else if (feof(inFile)) {
            break;
        }

        pFrame->data[0] = pictureBuf;
        pFrame->data[1] = pictureBuf + ySize;
        pFrame->data[2] = pictureBuf + ySize * 5 / 4;

        pFrame->pts = i*videoStream->time_base.den / (videoStream->time_base.num *25);

        //编码
        avcodec_send_frame(pCodecCtx, pFrame);

        int ret = avcodec_receive_packet(pCodecCtx, &pkt);
        if (ret == 0) {
            printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
            framecnt++;
            pkt.stream_index = videoStream->index;
            av_write_frame(pFormatCtx, &pkt);
        }
    }
    av_packet_unref(&pkt);

    av_write_trailer(pFormatCtx);

    avcodec_free_context(&pCodecCtx);
    av_free(pFrame);
    av_free(pictureBuf);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    fclose(inFile);

    return 1;


}