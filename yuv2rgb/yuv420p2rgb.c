#include<stdio.h>
#include<string.h>
#include<malloc.h>

#define clip(var) (var>=255?255:(var<=0)?0:var)


// 失真严重
void yuv420p2Rgb24(unsigned char *yuv, int w , int h, unsigned char *rgb) {
    unsigned char *ptrRGB, *ptrY, *ptrU, *ptrV, *tmpU, *tmpV;
    unsigned char y, u, v, y1;
    memset(rgb, 0 , w * h * 3);
    ptrY = yuv;
    ptrU = yuv + w*h;
    ptrV = ptrU + w*h*1/4;
    ptrRGB = rgb;

    int i = 0, j = 0;
    tmpU = ptrU;
    tmpV = ptrV;

    for( j = 0 ; j < h ; j++ ) {
        // if(j%2 == 0) {
        //     tmpU = ptrU + j * w / 4;
        // } else {
        //     tmpV = ptrV + j * w / 4;
        // }
        for( i = 0 ; i < w ; i++ ) {
            y = (*ptrY++);
            // y1 = (*ptrY++);
            // u = (*tmpU++);
            // v = (*tmpV++);
            u = ptrU[((j / 4)* w + i / 2)];
            v = ptrV[(j / 4)* w + i / 2];

            *ptrRGB++ = clip(y + (v - 128) + ((v - 128) * 103 >> 8));
            *ptrRGB++ = clip(y - ((u - 128) * 88 >> 8) - ((v - 128) * 183 >> 8));
            *ptrRGB++ = clip(y + (u - 128) + ((u - 128) * 198 >> 8));

            // *ptrRGB++ = clip(y1 + (v - 128) + ((v - 128) * 103 >> 8));
            // *ptrRGB++ = clip(y1 - ((u - 128) * 88 >> 8) - ((v - 128) * 183 >> 8));
            // *ptrRGB++ = clip(y1 + (u - 128) + ((u - 128) * 198 >> 8));

        }
    } 
}


int main() {
    FILE *f1 = fopen("./lena_256x256_rgb24.rgb", "wb+");
    FILE *f2 = fopen("./lena_256x256_yuv420p.yuv", "rb+");
    unsigned char *rgb = (unsigned char *)malloc(256*256*3);
    unsigned char *yuv = (unsigned char *)malloc(256*256*3/2);

    fread(yuv, 1, 256*256*3/2, f2);
    yuv420p2Rgb24(yuv, 256, 256, rgb);
    fwrite(rgb, 1, 256*256*3, f1);

    fclose(f1);
    fclose(f2);
    free(rgb);
    free(yuv);
    return 1;
}