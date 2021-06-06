#include<stdio.h>
#include<string.h>
#include<malloc.h>

#define clip(var) (var>=255?255:(var<=0)?0:var)

void yuvI4442Rgb24(unsigned char *yuv, int w , int h, unsigned char *rgb) {
    unsigned char *ptrRGB, *ptrY, *ptrU, *ptrV;
    unsigned char y, u, v;
    memset(rgb, 0 , w * h * 3);
    ptrY = yuv;
    ptrU = yuv + w * h;
    ptrV = ptrU + w * h;
    ptrRGB = rgb;

    int i = 0, j = 0;

    for( j = 0 ; j < h ; j++ ) {
        for( i = 0 ; i < w ; i++ ) {
            y = (*ptrY++);
            u = (*ptrU++);
            v = (*ptrV++);

            *ptrRGB++ = clip(y + (v - 128) + ((v - 128) * 103 >> 8));
            *ptrRGB++ = clip(y - ((u - 128) * 88 >> 8) - ((v - 128) * 183 >> 8));
            *ptrRGB++ = clip(y + (u - 128) + ((u - 128) * 198 >> 8));

            // *ptrRGB++ = clip(( 298 * y           + 409 * v + 128) >> 8);
            // *ptrRGB++ = clip(( 298 * y - 100 * u - 208 * v + 128) >> 8);
            // *ptrRGB++ = clip(( 298 * y + 516 * u           + 128) >> 8);
        }
    } 
}

void yuvI4442Rgb32(unsigned char *yuv, int w , int h, unsigned char *rgb) {
    unsigned char *ptrRGB, *ptrY, *ptrU, *ptrV;
    unsigned char y, u, v;
    memset(rgb, 0 , w * h * 4);
    ptrY = yuv;
    ptrU = yuv + w * h;
    ptrV = ptrU + w * h;
    ptrRGB = rgb;

    int i = 0, j = 0;

    for( j = 0 ; j < h ; j++ ) {
        for( i = 0 ; i < w ; i++ ) {
            y = (*ptrY++);
            u = (*ptrU++);
            v = (*ptrV++);

            *ptrRGB++ = clip(y + (v - 128) + ((v - 128) * 103 >> 8));
            *ptrRGB++ = clip(y - ((u - 128) * 88 >> 8) - ((v - 128) * 183 >> 8));
            *ptrRGB++ = clip(y + (u - 128) + ((u - 128) * 198 >> 8));
            *ptrRGB++ = 1;

            // *ptrRGB++ = clip(( 298 * y           + 409 * v + 128) >> 8);
            // *ptrRGB++ = clip(( 298 * y - 100 * u - 208 * v + 128) >> 8);
            // *ptrRGB++ = clip(( 298 * y + 516 * u           + 128) >> 8);
        }
    } 
}

void yuvI4442Rgb16(unsigned char *yuv, int w , int h, unsigned char *rgb) {
    unsigned char *ptrRGB, *ptrY, *ptrU, *ptrV;
    unsigned char y, u, v, r, g, b;
    memset(rgb, 0 , w * h * 2);
    ptrY = yuv;
    ptrU = yuv + w * h;
    ptrV = ptrU + w * h;
    ptrRGB = rgb;

    int i = 0, j = 0;

    for( j = 0 ; j < h ; j++ ) {
        for( i = 0 ; i < w ; i++ ) {
            y = (*ptrY++);
            u = (*ptrU++);
            v = (*ptrV++);

            r = clip(y + (v - 128) + ((v - 128) * 103 >> 8));
            g = clip(y - ((u - 128) * 88 >> 8) - ((v - 128) * 183 >> 8));
            b = clip(y + (u - 128) + ((u - 128) * 198 >> 8));

            *(ptrRGB++)=( ((g & 0x1C) << 3) | ( b >> 3) );
            *(ptrRGB++)=( (r & 0xF8) | ( g >> 5) );

            // *(ptrRGB++)=( ((g & 0x1C) << 3) | ( r >> 3) );
            // *(ptrRGB++)=( (b & 0xF8) | ( g >> 5) );
        
        }
    } 
}

int main() {
    // FILE *f1 = fopen("./lena_256x256_rgb24.rgb", "wb+");
    // FILE *f1 = fopen("./lena_256x256_rgb32.rgb", "wb+");
    FILE *f1 = fopen("./lena_256x256_rgb16.rgb", "wb+");
    FILE *f2 = fopen("./lena_256x256_yuv444.yuv", "rb+");
    unsigned char *rgb = (unsigned char *)malloc(256*256*2);
    unsigned char *yuv = (unsigned char *)malloc(256*256*3);

    fread(yuv, 1, 256*256*3, f2);
    // yuvI4442Rgb24(yuv, 256, 256, rgb);
    // yuvI4442Rgb32(yuv, 256, 256, rgb);
    yuvI4442Rgb16(yuv, 256, 256, rgb);
    fwrite(rgb, 1, 256*256*2, f1);

    fclose(f1);
    fclose(f2);
    free(rgb);
    free(yuv);
    return 1;
}