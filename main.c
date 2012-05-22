#include "v4l.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#define norm VIDEO_MODE_NTSC

#define BMP   "image.bmp"

/*******************************/
int main()
{
	char *buffer=NULL;
	v4ldevice VD;
	v4ldevice *vd=&VD;

    int frame=0;
	//int f_d;

	/********************************/
	unsigned char *word;
	unsigned char *lpbuffer;
	int i,j;
	int tmp = 0;

	//unsigned char* buffer=(unsigned char*)malloc(IMAGEHEIGHT*IMAGEWIDTH*3/2);	//存放YUV格式的图片
	unsigned char*y;
	unsigned char*u;
	unsigned char*v;
	unsigned char*dst=(unsigned char*)malloc(IMAGEWIDTH*IMAGEHEIGHT*3);	//存放RGB格式的图片数据

//	FILE *	 fp[3];
	FILE *	 fp;
	BITMAPFILEHEADER		bf;	//bmp文件头
	BITMAPINFOHEADER		bi;	//bmp信息头
	  

	 //Set BITMAPINFOHEADER 设置bmp图片的信息头格式
     bi.biSize = 40;		//位图信息头大小
     bi.biWidth = IMAGEWIDTH;		//图像宽度
     bi.biHeight = IMAGEHEIGHT;		//图像高度
     bi.biPlanes = 1;			//位平面树=1
     bi.biBitCount = 24;		//单位像素的位数
     bi.biCompression = 0;		//图片的压缩属性，bmp不压缩，等于0
     //bi.biSizeImage = WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
	 bi.biSizeImage = IMAGEWIDTH * IMAGEHEIGHT * bi.biBitCount;
	 //表示bmp图片数据区的大小，当上一个属性biCompression等于0时，这里的值可以省略不填
     
	 bi.biXPelsPerMeter = 0;	//水平分辨率
     bi.biYPelsPerMeter = 0;	//垂直分辨率
     bi.biClrUsed = 0;			//表示使用了多少哥颜色索引表，一般biBitCount属性小于16才会用到，等于0时表示有2^biBitCount个颜色索引表
     bi.biClrImportant = 0;		//表示有多少个重要的颜色，等于0时表示所有颜色都很重要

     //Set BITMAPFILEHEADER  设置bmp图片的文件头格式
     bf.bfType = 0x4d42;		//2个字节，恒等于0x4d42，ascii字符“BM”
     bf.bfSize = 54 + bi.biSizeImage;	//文件大小，以4个字节为单位
     bf.bfReserved1 = 0;		//备用
     bf.bfReserved2 = 0;		//备用
     bf.bfOffBits = 54;			//数据区在文件中的位置偏移量

	/*******************************/

    //f_d=open(DEFAULT_FILE_NAME,O_RDWR|O_CREAT,0666);//获取文件的描述符
	if(0==v4l_open("/dev/video0",vd)) //打开设备
		printf("open success!\n");
	else
        printf("open failure\n");

	if(0==v4l_set_norm(vd,norm))
		printf("set_norm success\n");
	else
		printf("set_norm failure\n");

    if(0==v4l_grab_init(vd,IMAGEWIDTH,IMAGEHEIGHT))//初始化设备，定义获取图像的大小
		printf("init success!\n");
	else
        printf("init failure\n");
	
	if(0==v4l_mmap_init(vd))//内存映射
		printf("memory map success!\n");
	else
		printf("memory map failure\n");

////////////////////////////////////////////////////
	//开始一帧数据的采集	
	/*fp[0] = fopen("linhui0.bmp", "wb");
	fp[1] = fopen("linhui1.bmp", "wb");
	fp[2] = fopen("linhui2.bmp", "wb");

	for(tmp=0;tmp<3;tmp++)
	{*/
	
	fp = fopen(BMP, "wb");
	if(!fp)
	{
		perror(BMP);
		exit(1);
	}

    if(0==v4l_grab_start(vd,frame))//开始获取图像
		printf("get picture success!\n");
	else
		printf("get picture failure\n");
	
	v4l_grab_sync(vd,frame);//等待传完一帧
	buffer=(char *)v4l_get_address(vd);//得到这一帧的地址
	printf("img address %p\n",buffer);
	
	/*************************************/
	y=buffer;
	u=buffer+IMAGEWIDTH*IMAGEHEIGHT;
	v=u+IMAGEWIDTH*IMAGEHEIGHT/4;
     
	fwrite(&bf, 14, 1, fp);	//向文件中写入图片文件头
	fwrite(&bi, 40, 1, fp);	//向文件中写入图片信息头
	/*fwrite(&bf, 14, 1, fp[tmp]);	//向文件中写入图片文件头
	fwrite(&bi, 40, 1, fp[tmp]);	//向文件中写入图片信息头*/


//==================================================
	InitConvertTable();
	ConvertYUV2RGB(y,u,v,dst,IMAGEWIDTH,IMAGEHEIGHT);

	fseek(fp, 0x36, SEEK_SET);
		
	//fseek(fp[tmp], 0x36, SEEK_SET);

	lpbuffer = dst+IMAGEWIDTH*3*(IMAGEHEIGHT - 1);

	for(i=0; i<IMAGEHEIGHT; i++)    //bmp file scan line is arraned by BGR|BGR|BGR|........
	{
		word = lpbuffer;
		for(j=0; j<IMAGEWIDTH; j++)
		{
			/*************add in 2011 8.2 23:04*****************/
			/*if(tmp == 10)
			{
				tmp = 0;
				fputc( *(word+2), fp[tmp]); // B
				fputc( *(word+1), fp[tmp]); // G
				fputc( *(word+0), fp[tmp]); // R
			}else{
				fputc( *(word+2), fp[tmp]); // B
				fputc( *(word+1), fp[tmp]); // G
				fputc( *(word+0), fp[tmp]); // R
			}*/
			/***********************************************/
			fputc( *(word+2), fp); // B
			fputc( *(word+1), fp); // G
			fputc( *(word+0), fp); // R
			word+=3;
		}
		lpbuffer -= IMAGEWIDTH*3; // 指针转到上一行的开始
	}
	//fclose(fp[tmp]);
	//}
	////////////////////////////////////////////////////
     printf("get bmp form video\t[OK]\n");
	/***************************************/
	fclose(fp);
	v4l_close(vd);
    return 0;
}