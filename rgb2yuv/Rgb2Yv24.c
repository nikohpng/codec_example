#include<stdio.h>
#include<string.h>
#include<malloc.h>

unsigned char split_value(unsigned char val, unsigned char min, unsigned char max) {
    if ( val > max) {
        return 255;
    } else if ( val < min ) {
        return 0;
    } else {
        return val;
    }
}

void rgb2yuv(unsigned char *rgbBuf, int w, int h, unsigned char *yuv) {
    unsigned char *ptrRgb, *ptrY, *ptrU, *ptrV;
    unsigned char r, g, b, y, u, v;
    int i = 0, j = 0;
    memset(yuv, 0, w * h * 3);

    ptrY = yuv;
    ptrV = ptrY + (w*h);
    ptrU = ptrV + (w*h);
    

    for( i = 0 ; i < w ; i++ ){
        ptrRgb = rgbBuf + w * i * 3;
        for( j = 0; j < h; j++) {
            r = (*ptrRgb++);
            g = (*ptrRgb++);
            b = (*ptrRgb++);

            y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;          
			u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;          
			v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;

            *ptrY++ = split_value(y,0,255);
            *ptrU++ = split_value(u,0,255);
            *ptrV++ = split_value(v,0,255);
        }
    } 
}

int main() {
    FILE *f1 = fopen("./lena_256x256_rgb24.rgb", "rb+");
    FILE *f2 = fopen("./lena_256x256_yv21.yuv", "wb+");
    
    unsigned char *rgb = (unsigned char*) malloc(256*256*3);
    unsigned char *yuv = (unsigned char*) malloc(256*256*3);

    fread(rgb, 1, 256 * 256 * 3, f1);
    rgb2yuv(rgb, 256, 256, yuv);
    fwrite(yuv, 1, 256 * 256 * 3, f2);

    fclose(f1);
    fclose(f2);
    free(rgb);
    free(yuv);
    return 1;
}