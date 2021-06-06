#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <malloc.h>

unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char max_val) {
     if ( x > max_val ){
        return max_val;
    } else if ( x < min_val ){
        return min_val;
    } else {
        return x;
    }
}

void rgb2yuv(unsigned char *rgbBuf, int w, int h, unsigned char *yuvBuf) {
    int i = 0,j = 0;
    unsigned char r, g, b, y ,u ,v, *ptrRGB, *ptrY, *ptrU, *ptrV;

    memset(yuvBuf, 0, w * h * 3);
    ptrY = yuvBuf;
    ptrU = yuvBuf + w * h;
    ptrV = ptrU + w * h;
    
    for( i = 0 ; i < w ; i++) {
        ptrRGB = rgbBuf + i * w *3 ;
        for(j = 0; j< h ; j++) {
            r = (*ptrRGB++);
            g = (*ptrRGB++);
            b = (*ptrRGB++);

            y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;          
			u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;          
			v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;

            *ptrY++ = clip_value(y,0,255);;
            *ptrU++ = clip_value(u,0,255);;
            *ptrV++ = clip_value(v,0,255);;
        }
    }
}

int main() {
    FILE *f1 = fopen("./lena_256x256_rgb24.rgb", "rb+");
    FILE *f2 = fopen("./lena_256x256_yuv444.yuv", "wb+");
    unsigned char *rgb = (unsigned char *)malloc(256*256*3);
    unsigned char *yuv = (unsigned char *)malloc(256*256*3);

    fread(rgb, 1, 256*256*3, f1);
    rgb2yuv(rgb, 256, 256, yuv);
    fwrite(yuv, 1, 256*256*3, f2);

    fclose(f1);
    fclose(f2);
    free(rgb);
    free(yuv);
    return 1;
}
