#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <malloc.h>

#define MAX 1024

unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char max_val) {
     if ( x > max_val ){
        return max_val;
    } else if ( x < min_val ){
        return min_val;
    } else {
        return x;
    }
}

unsigned char rgb2y(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16 ;
}

unsigned char rgb2u(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ; 
}

unsigned char rgb2v(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;
}

void rgb2Nv12(int w,int h, unsigned char *RgbBuf, unsigned char *yuvBuf) {
    unsigned char *ptrY0, *ptrY1, *ptrUV, *ptrRGB0, *ptrRGB1;
    unsigned char y00, u00, v00, 
                  y01, u01, v01,
                  y10, u10, v10,
                  y11, u11, v11,
                  r00, g00, b00, 
                  r01, g01, b01,
                  r10, g10, b10,
                  r11, g11, b11;
    memset(yuvBuf, 0, w*h*3/2);
	ptrUV = yuvBuf + w*h;
    int i=0,j=0;

    for(j=0 ; j< h ; j += 2) {
        ptrY0 = yuvBuf + w * j;
        ptrY1 = yuvBuf + w * (j+1);
        ptrRGB0 = RgbBuf + w*j*3 ;
        ptrRGB1 = RgbBuf + w*(j+1)*3;
        for(i=0; i< w; i += 2) {
            r00 = *(ptrRGB0++);
			g00 = *(ptrRGB0++);
			b00 = *(ptrRGB0++);

            r01 = *(ptrRGB0++);
            g01 = *(ptrRGB0++);
			b01 = *(ptrRGB0++);

            r10 = *(ptrRGB1++);
            g10 = *(ptrRGB1++);
            b10 = *(ptrRGB1++);

            r11 = *(ptrRGB1++);
            g11 = *(ptrRGB1++);
            b11 = *(ptrRGB1++);

            y00 = rgb2y(r00, g00, b00);        
			u00 = rgb2u(r00, g00, b00);        
			v00 = rgb2v(r00, g00, b00);

            y01 = rgb2y(r01, g01, b01);        
			u01 = rgb2u(r01, g01, b01);        
			v01 = rgb2v(r01, g01, b01);

            y10 = rgb2y(r10, g10, b10);        
			u10 = rgb2u(r10, g10, b10);        
			v10 = rgb2v(r10, g10, b10);

            y11 = rgb2y(r11, g11, b11);        
			u11 = rgb2u(r11, g11, b11);        
			v11 = rgb2v(r11, g11, b11);

           *(ptrY0++) = (unsigned char)(y00 + 0.5f);
           *(ptrY0++) = (unsigned char)(y01 + 0.5f);
           *(ptrY1++) = (unsigned char)(y10 + 0.5f);
           *(ptrY1++) = (unsigned char)(y11 + 0.5f);

           // *(ptrUV++) =(unsigned char)(u00 + u01 + u10 + u11)*0.25f;
           // *(ptrUV++) =(unsigned char)(v00 + v01 + v10 + v11)*0.25f;
           *(ptrUV++) =clip_value((u00 + u01 + u10 + u11)*0.25f,0,255);
           *(ptrUV++) =clip_value((v00 + v01 + v10 + v11)*0.25f,0,255);
        }
    }
}

void rgb2Nv21(int w,int h, unsigned char *RgbBuf, unsigned char *yuvBuf) {
    unsigned char *ptrY0, *ptrY1, *ptrUV, *ptrRGB0, *ptrRGB1;
    unsigned char y00, u00, v00, 
                  y01, u01, v01,
                  y10, u10, v10,
                  y11, u11, v11,
                  r00, g00, b00, 
                  r01, g01, b01,
                  r10, g10, b10,
                  r11, g11, b11;
    memset(yuvBuf, 0, w*h*3/2);
	ptrUV = yuvBuf + w*h;
    int i=0,j=0;

    for(j=0 ; j< h ; j += 2) {
        ptrY0 = yuvBuf + w * j;
        ptrY1 = yuvBuf + w * (j+1);
        ptrRGB0 = RgbBuf + w*j*3 ;
        ptrRGB1 = RgbBuf + w*(j+1)*3;
        for(i=0; i< w; i += 2) {
            r00 = *(ptrRGB0++);
			g00 = *(ptrRGB0++);
			b00 = *(ptrRGB0++);

            r01 = *(ptrRGB0++);
            g01 = *(ptrRGB0++);
			b01 = *(ptrRGB0++);

            r10 = *(ptrRGB1++);
            g10 = *(ptrRGB1++);
            b10 = *(ptrRGB1++);

            r11 = *(ptrRGB1++);
            g11 = *(ptrRGB1++);
            b11 = *(ptrRGB1++);

            y00 = rgb2y(r00, g00, b00);        
			u00 = rgb2u(r00, g00, b00);        
			v00 = rgb2v(r00, g00, b00);

            y01 = rgb2y(r01, g01, b01);        
			u01 = rgb2u(r01, g01, b01);        
			v01 = rgb2v(r01, g01, b01);

            y10 = rgb2y(r10, g10, b10);        
			u10 = rgb2u(r10, g10, b10);        
			v10 = rgb2v(r10, g10, b10);

            y11 = rgb2y(r11, g11, b11);        
			u11 = rgb2u(r11, g11, b11);        
			v11 = rgb2v(r11, g11, b11);

           *(ptrY0++) = (unsigned char)(y00 + 0.5f);
           *(ptrY0++) = (unsigned char)(y01 + 0.5f);
           *(ptrY1++) = (unsigned char)(y10 + 0.5f);
           *(ptrY1++) = (unsigned char)(y11 + 0.5f);

           // *(ptrUV++) =(unsigned char)(u00 + u01 + u10 + u11)*0.25f;
           // *(ptrUV++) =(unsigned char)(v00 + v01 + v10 + v11)*0.25f;
           *(ptrUV++) =clip_value((v00 + v01 + v10 + v11)*0.25f,0,255);
           *(ptrUV++) =clip_value((u00 + u01 + u10 + u11)*0.25f,0,255);

        }
    }
}




void main() {
    FILE *fp=fopen("./lena_256x256_rgb24.rgb","rb+");
	// FILE *fp1=fopen("./lena_256x256_nv12.yuv","wb+");
    FILE *fp1=fopen("./lena_256x256_nv21.yuv","wb+");
    int i=0;
 
	unsigned char *pic_rgb24=(unsigned char *)malloc(256*256*3);
	// unsigned char *pic_nv12=(unsigned char *)malloc(256*256*3/2);
    unsigned char *pic_nv21=(unsigned char *)malloc(256*256*3/2);
 
	for(i=0;i<1;i++){
		fread(pic_rgb24,1,256*256*3,fp);
		rgb2Nv21(256,256,pic_rgb24,pic_nv21);
		fwrite(pic_nv21,1,256*256*3/2,fp1);
	}
    printf("%d\n",malloc_usable_size(pic_rgb24));
 
	free(pic_rgb24);
    free(pic_nv21);

	fclose(fp);
	fclose(fp1);

}


