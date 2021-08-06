#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define clip(var) (var >= 255 ? 255 : (var <= 0) ? 0 \
                                                 : var)

void yuyv2Rgb24(unsigned char* yuv, int w, int h, unsigned char* rgb)
{
    unsigned char *ptrRGB, *ptrY;
    unsigned char y, u, v, y1;
    int i = 0, j = 0;
    memset(rgb, 0, w * h * 3);
    ptrY   = yuv;
    ptrRGB = rgb;
    
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i += 2) {
            y  =  (*ptrY++);
            u  =  (*ptrY++);
            y1 =  (*ptrY++);
            v  =  (*ptrY++);

            *ptrRGB++ = clip(y  + ( v - 128)            + ((v - 128 ) * 103 >> 8));
            *ptrRGB++ = clip(y  - ((u - 128) * 88 >> 8) - ((v - 128 ) * 183 >> 8));
            *ptrRGB++ = clip(y  + ( u - 128)            + ((u - 128 ) * 198 >> 8));

            *ptrRGB++ = clip(y1 + ( v - 128)            + ((v - 128 ) * 103 >> 8));
            *ptrRGB++ = clip(y1 - ((u - 128) * 88 >> 8) - ((v - 128 ) * 183 >> 8));
            *ptrRGB++ = clip(y1 + ( u - 128)            + ((u - 128 ) * 198 >> 8));
        }
    }
}

int main()
{
    unsigned char* rgb = (unsigned char*) malloc(256 * 256 * 3);
    unsigned char* yuv = (unsigned char*) malloc(256 * 256 * 2);
    FILE*           f1 = fopen("./out/lena_256x256_rgb24.rgb", "wb+");
    FILE*           f2 = fopen("./assets/lena_256x256_yuyv.yuv", "rb+");

    fread(yuv, 1, 256 * 256 * 2, f2);
    yuyv2Rgb24(yuv, 256, 256, rgb);
    fwrite(rgb, 1, 256 * 256 * 3, f1);

    fclose(f1);
    fclose(f2);
    free(rgb);
    free(yuv);
    return 1;
}