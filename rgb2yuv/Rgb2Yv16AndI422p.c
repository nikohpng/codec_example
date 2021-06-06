#include<stdio.h>
#include<string.h>
#include<malloc.h>

void rgb2Yv16(unsigned char *rgb, int w, int h, unsigned char *yuv) {
    unsigned char *ptrRgb, *ptrY, *ptrU, *ptrV;
    unsigned char r1, g1, b1, r2, g2, b2, y1, u1, v1, y2, u2, v2; 
    memset(yuv, 0, w*h*2);

    ptrY = yuv;
    ptrV = yuv + w * h;
    ptrU = ptrV + w * h * 1/2;

    int i = 0, j = 0;

    for( j = 0 ; j < h ; j++) {
        ptrRgb = rgb + j * w * 3;
        for(i = 0 ; i < w; i += 2) {
            r1 = (*ptrRgb++);
            g1 = (*ptrRgb++);
            b1 = (*ptrRgb++);

            r2 = (*ptrRgb++);
            g2 = (*ptrRgb++);
            b2 = (*ptrRgb++);

            y1 = (unsigned char)( ( 66 * r1 + 129 * g1 +  25 * b1 + 128) >> 8) + 16  ;          
			u1 = (unsigned char)( ( -38 * r1 -  74 * g1 + 112 * b1 + 128) >> 8) + 128 ;          
			v1 = (unsigned char)( ( 112 * r1 -  94 * g1 -  18 * b1 + 128) >> 8) + 128 ;
            y2 = (unsigned char)( ( 66 * r2 + 129 * g2 +  25 * b2 + 128) >> 8) + 16  ;          
			u2 = (unsigned char)( ( -38 * r2 -  74 * g2 + 112 * b2 + 128) >> 8) + 128 ;          
			v2 = (unsigned char)( ( 112 * r2 -  94 * g2 -  18 * b2 + 128) >> 8) + 128 ;

            *ptrY++ = y1;
            *ptrY++ = y2;
            *ptrU++ = (u1+u2)/2;
            *ptrV++ = (v1+v2)/2;
        }
    }
}

void rgb2I422p(unsigned char *rgb, int w, int h, unsigned char *yuv) {
    unsigned char *ptrRgb, *ptrY, *ptrU, *ptrV;
    unsigned char r1, g1, b1, r2, g2, b2, y1, u1, v1, y2, u2, v2; 
    memset(yuv, 0, w*h*2);

    ptrY = yuv;
    ptrU = yuv + w * h;
    ptrV = ptrU + w * h * 1/2;

    int i = 0, j = 0;

    for( j = 0 ; j < h ; j++) {
        ptrRgb = rgb + j * w * 3;
        for(i = 0 ; i < w; i += 2) {
            r1 = (*ptrRgb++);
            g1 = (*ptrRgb++);
            b1 = (*ptrRgb++);

            r2 = (*ptrRgb++);
            g2 = (*ptrRgb++);
            b2 = (*ptrRgb++);

            y1 = (unsigned char)( ( 66 * r1 + 129 * g1 +  25 * b1 + 128) >> 8) + 16  ;          
			u1 = (unsigned char)( ( -38 * r1 -  74 * g1 + 112 * b1 + 128) >> 8) + 128 ;          
			v1 = (unsigned char)( ( 112 * r1 -  94 * g1 -  18 * b1 + 128) >> 8) + 128 ;
            y2 = (unsigned char)( ( 66 * r2 + 129 * g2 +  25 * b2 + 128) >> 8) + 16  ;          
			u2 = (unsigned char)( ( -38 * r2 -  74 * g2 + 112 * b2 + 128) >> 8) + 128 ;          
			v2 = (unsigned char)( ( 112 * r2 -  94 * g2 -  18 * b2 + 128) >> 8) + 128 ;

            *ptrY++ = y1;
            *ptrY++ = y2;
            *ptrU++ = (u1+u2)/2;
            *ptrV++ = (v1+v2)/2;
        }
    }
}

int main() {
    FILE *f1 = fopen("./lena_256x256_rgb24.rgb", "rb+");
    FILE *f2 = fopen("./lena_256x256_yv16.yuv", "wb+");
    FILE *f3 = fopen("./lena_256x256_I422p.yuv", "wb+");

    unsigned char *rgb = (unsigned char *) malloc(256*256*3);
    unsigned char *yuv = (unsigned char *) malloc(256*256*2);
    unsigned char *yuvI = (unsigned char *) malloc(256*256*2);

    fread(rgb, 1, 256*256*3, f1);
    rgb2Yv16(rgb, 256, 256, yuv);
    rgb2I422p(rgb, 256, 256, yuvI);
    fwrite(yuv, 1, 256*256*2, f2);
    fwrite(yuvI, 1, 256*256*2, f3);

    fclose(f2);
    fclose(f1);
    fclose(f3);
    free(yuv);
    free(rgb);
    free(yuvI);
    return 1;
}