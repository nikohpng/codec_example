#include<stdio.h>
#include<libavutil/imgutils.h>

int main(int argc, char* argv[]) {
    int linesieze = av_image_get_linesize(AV_PIX_FMT_YUYV422, 256, 0);
    int max_step     [4];
    int max_step_comp[4];
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(AV_PIX_FMT_YUYV422);
    av_image_fill_max_pixsteps(max_step, max_step_comp, desc);
    int shifted_w = ((256 + (1 << 0) - 1)) >> 0;

    printf("max_step = %d, max_step_comp = %d\n", max_step[0], max_step_comp[0]);
    printf("linesize = %d, shifted_w = %d", linesieze, shifted_w * 4);
    return 0;
}