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

void rgb2yuv420p(int w,int h, unsigned char *RgbBuf, unsigned char *yuvBuf) {
    unsigned char *ptrY, *ptrU, *ptrV, *ptrRGB;
    unsigned char y , u , v, r, g, b;
    memset(yuvBuf, 0, w*h*3/2);
    ptrY = yuvBuf;
	ptrU = yuvBuf + w*h;
	ptrV = ptrU + (w*h*1/4);
    int i=0,j=0;

    for(j=0 ; j< h ; j++ ) {
        ptrRGB = RgbBuf + w*j*3 ;
        for(i=0; i< w; i++) {
            r = *(ptrRGB++);
			g = *(ptrRGB++);
			b = *(ptrRGB++);

            y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;          
			u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;          
			v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;

           *(ptrY++) = clip_value(y,0,255);
			if (j%2==0&&i%2 ==0){
				*(ptrU++) =clip_value(u,0,255);
			}
			else{
				if (i%2==0){
				*(ptrV++) =clip_value(v,0,255);
				}
			}
        }
    }
}

void rgb2Yv12(int w,int h, unsigned char *RgbBuf, unsigned char *yuvBuf) {
    unsigned char *ptrY, *ptrU, *ptrV, *ptrRGB;
    unsigned char y , u , v, r, g, b;
    memset(yuvBuf, 0, w*h*3/2);
    ptrY = yuvBuf;
	ptrV = yuvBuf + w*h;
	ptrU = ptrV + (w*h*1/4);
    int i=0,j=0;

    for(j=0 ; j< h ; j++ ) {
        ptrRGB = RgbBuf + w*j*3 ;
        for(i=0; i< w; i++) {
            r = *(ptrRGB++);
			g = *(ptrRGB++);
			b = *(ptrRGB++);

            y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;          
			u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;          
			v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;

           *(ptrY++) = clip_value(y,0,255);
			if (j%2==0&&i%2 ==0){
				*(ptrU++) =clip_value(u,0,255);
			}
			else{
				if (i%2==0){
                    *(ptrV++) =clip_value(v,0,255);
                    
				}
			}
        }
    }
}


void main() {
    FILE *fp=fopen("./lena_256x256_rgb24.rgb","rb+");
	FILE *fp1=fopen("./lena_256x256_yuv420p.yuv","wb+");
    FILE *fp2=fopen("./lena_256x256_yv12.yuv","wb+");
    int i=0;
 
	unsigned char *pic_rgb24=(unsigned char *)malloc(256*256*3);
	unsigned char *pic_yuv420=(unsigned char *)malloc(256*256*3/2);
    unsigned char *pic_yv12=(unsigned char *)malloc(256*256*3/2);
 
	for(i=0;i<1;i++){
		fread(pic_rgb24,1,256*256*3,fp);
		rgb2yuv420p(256,256,pic_rgb24,pic_yuv420);
        rgb2yuv420p(256,256,pic_rgb24,pic_yv12);
		fwrite(pic_yuv420,1,256*256*3/2,fp1);
        fwrite(pic_yv12,1,256*256*3/2,fp2);
	}
    printf("%d\n",malloc_usable_size(pic_rgb24));
 
	free(pic_rgb24);
	free(pic_yuv420);
    free(pic_yv12);
	fclose(fp);
	fclose(fp1);
    fclose(fp2);

    // unsigned char buf[MAX];
    // unsigned char rgbBuf[MAX * 200],yubBuf[MAX * 200], *rgbBufDup;


    // if((fp = fopen("./test.txt","rw")) == NULL) {
    //     perror("fail to read");
    //     exit (1) ;
    // }
    // rgbBufDup = rgbBuf;
    // fgets(buf,MAX,fp);
    // for(i = 0;i<strlen(buf);i++){
    //         *(rgbBufDup++) = buf[i];
    // }
    // while(fgets(buf,MAX,fp) != NULL) {
    //     for(i = 0;i<strlen(buf);i++){
    //         *(rgbBufDup++) = buf[i];
    //     }
    // }
    // for(i = 0;i<sizeof(rgbBuf)/sizeof(char);i++){
    //     printf("%c",rgbBuf[i]);      
    // }
    

    // rgb2yub(256,256,rgbBuf,yubBuf);


    // printf("%d",strlen(yubBuf));
    // outFp=fopen("./lena_256x256_yuv420p.yuv","w");
    // for(i=0;i<strlen(yubBuf);i++){
    //     fprintf(outFp,"%c", yubBuf);
    // }

}


