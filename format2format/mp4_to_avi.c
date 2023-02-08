#include<stdio.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libswscale/swscale.h>

int main(int argc, char* argv) {

    AVFormatContext *fmt_ctx = NULL, *out_fmt_ctx = NULL;
    AVOutputFormat  *out_fmt = NULL;

    AVCodecContext  *codec_ctx;
    AVCodec         *codec;

    AVPacket        pkt;

    AVBitStreamFilterContext *vbsf = NULL;

    char *input = "../assets/test.mp4";
    char *out   = "./out/test.avi";
    int   ret   = 0;
    int   i     = 0;
    
    int frame_index = 0;
    
    if(( ret = avformat_open_input(&fmt_ctx, input, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Open input error: %s", av_err2str(ret));
        goto end;
    }

    if(( ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Fail to find stream: %s", av_err2str(ret));
        goto end;
    }

    vbsf = av_bitstream_filter_init("h264_mp4toannexb");

    av_dump_format(fmt_ctx, 0, input, 0);
    
    if((ret = avformat_alloc_output_context2(&out_fmt_ctx, NULL, NULL, out)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Fail to alloc output %s", av_err2str(ret));
        goto end;
    }
    
    // 易错
    out_fmt = out_fmt_ctx->oformat;

    for(i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *in_stream = fmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(out_fmt_ctx, in_stream->codec->codec);

        if(!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Fail to new out stream%s", "");
            goto end;
        }

        if((ret = avcodec_copy_context(out_stream->codec, in_stream->codec)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Fail to copy context: %s", av_err2str(ret));
            goto end;
        } 

        out_stream->codec->codec_tag = 0;
        if(out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    av_dump_format(out_fmt_ctx, 0, out, 1);

    if(!(out_fmt->flags & AVFMT_NOFILE)) {
        if((ret = avio_open(&out_fmt_ctx->pb, out, AVIO_FLAG_WRITE)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Fail to open file: %s", av_err2str(ret));
            goto end;
        }
    }

    if((ret = avformat_write_header(out_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Fail to write header: %s", av_err2str(ret));
        goto end;
    }

    while(1) {
        AVStream *in_stream, *out_stream;
        if((ret = av_read_frame(fmt_ctx, &pkt)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Fail to read packet: %s", av_err2str(ret));
            break;
        }
        in_stream = fmt_ctx->streams[pkt.stream_index];
        out_stream = out_fmt_ctx->streams[pkt.stream_index];
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        if(pkt.stream_index == 0) {
            AVPacket fpkt = pkt;
            // 易错
            int a = av_bitstream_filter_filter(vbsf, out_stream->codec, NULL, &fpkt.data, &fpkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
            pkt.data = fpkt.data;
            pkt.size = fpkt.size;
        }

        if((ret = av_write_frame(out_fmt_ctx, &pkt)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Fail to write frame!");
            break;
        }
        // 易错
        av_packet_unref(&pkt);
        frame_index++;
    }
    av_write_trailer(out_fmt_ctx);
    avformat_close_input(&fmt_ctx);
    end: 
        if(fmt_ctx) avformat_free_context(fmt_ctx);
        if(out_fmt_ctx) avformat_free_context(out_fmt_ctx);
        return 0;
}