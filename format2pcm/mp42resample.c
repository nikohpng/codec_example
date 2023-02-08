#include<stdio.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/log.h>
#include<libavutil/opt.h>
#include<libswresample/swresample.h>
#include<libavutil/samplefmt.h>

int main() {

    AVFormatContext *in_fmt_ctx   = NULL;
    AVInputFormat   *in_fmt       = NULL;

    AVCodecContext  *in_codec_ctx = NULL;
    AVCodec         *in_codec     = NULL;

    char* input_path  = "../assets/test.mp4";
    char* output_path = "./out/test.pcm";

    in_fmt_ctx = avformat_alloc_context();

    if(avformat_open_input(&in_fmt_ctx, input_path, in_fmt, NULL) < 0) {
        av_log(in_fmt_ctx, AV_LOG_ERROR, "Fail to open file %s", input_path);
        return -1;
    }

    if ((avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(in_fmt_ctx, AV_LOG_ERROR, "Can't find stream info");
        return -1;
    }

    int stm_idx = 0;
    int i = 0;
    for( i = 0; i < in_fmt_ctx->nb_streams; i++) {
        if(in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stm_idx = i;
        }
    }

    AVCodecParameters *codec_par = in_fmt_ctx->streams[stm_idx]->codecpar;
    in_codec = avcodec_find_decoder(codec_par->codec_id);

    if (in_codec == NULL) {
        av_log(in_fmt_ctx, AV_LOG_ERROR, "Can't find decoder");
        return -1;
    }

    in_codec_ctx = avcodec_alloc_context3(in_codec);

    if ((avcodec_parameters_to_context(in_codec_ctx, codec_par)) < 0) {
        av_log(in_codec_ctx, AV_LOG_ERROR, "Can't alloc codec ctx");
        return -1;
    }

    if ((avcodec_open2(in_codec_ctx, in_codec, NULL)) < 0) {
        av_log(in_codec_ctx, AV_LOG_ERROR, "Open codec error");
        return -1;
    }

    enum AVSampleFormat in_sample_fmt = in_codec_ctx->sample_fmt;
    int in_ch_layout = codec_par->channel_layout;
    int in_sample_rate = codec_par->sample_rate;
    int out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat  out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = codec_par->sample_rate;
    int out_buffer_size = av_samples_get_buffer_size(NULL, codec_par->channels, in_codec_ctx->frame_size , out_sample_fmt, 1);;

    SwrContext *swr_ctx = swr_alloc();
    swr_alloc_set_opts(swr_ctx, out_ch_layout, out_sample_fmt, out_sample_rate,
     in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    swr_init(swr_ctx);

    AVPacket *pkt = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();
    int ret = 0;
    uint8_t *buffer = (uint8_t *)av_malloc(out_buffer_size);

    FILE *out_file = fopen(output_path, "wb");

    while((ret = av_read_frame(in_fmt_ctx, pkt)) >= 0) {

        if (pkt->stream_index == stm_idx) {

            ret = avcodec_send_packet(in_codec_ctx, pkt);

            if (ret < 0) {
                av_log(in_codec_ctx, AV_LOG_ERROR, "Fail to send pkt");
                return -1;
            }

            ret = avcodec_receive_frame(in_codec_ctx, frame);

            if (ret < 0) {
                av_log(in_codec_ctx, AV_LOG_ERROR, "Fail to receive frame");
                return -1;
            }

            swr_convert(swr_ctx, &buffer, out_buffer_size, (const uint8_t**) frame->data, frame->nb_samples);

            fwrite(buffer, 1, out_buffer_size, out_file);
        }
    }


    free(buffer);
    fclose(out_file);
    av_free_packet(pkt);
    av_frame_free(&frame);
    avformat_free_context(in_fmt_ctx);
    avcodec_free_context(&in_codec_ctx);
}