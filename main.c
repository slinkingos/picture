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

	//unsigned char* buffer=(unsigned char*)malloc(IMAGEHEIGHT*IMAGEWIDTH*3/2);	//���YUV��ʽ��ͼƬ
	unsigned char*y;
	unsigned char*u;
	unsigned char*v;
	unsigned char*dst=(unsigned char*)malloc(IMAGEWIDTH*IMAGEHEIGHT*3);	//���RGB��ʽ��ͼƬ����

//	FILE *	 fp[3];
	FILE *	 fp;
	BITMAPFILEHEADER		bf;	//bmp�ļ�ͷ
	BITMAPINFOHEADER		bi;	//bmp��Ϣͷ
	  

	 //Set BITMAPINFOHEADER ����bmpͼƬ����Ϣͷ��ʽ
     bi.biSize = 40;		//λͼ��Ϣͷ��С
     bi.biWidth = IMAGEWIDTH;		//ͼ����
     bi.biHeight = IMAGEHEIGHT;		//ͼ��߶�
     bi.biPlanes = 1;			//λƽ����=1
     bi.biBitCount = 24;		//��λ���ص�λ��
     bi.biCompression = 0;		//ͼƬ��ѹ�����ԣ�bmp��ѹ��������0
     //bi.biSizeImage = WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
	 bi.biSizeImage = IMAGEWIDTH * IMAGEHEIGHT * bi.biBitCount;
	 //��ʾbmpͼƬ�������Ĵ�С������һ������biCompression����0ʱ�������ֵ����ʡ�Բ���
     
	 bi.biXPelsPerMeter = 0;	//ˮƽ�ֱ���
     bi.biYPelsPerMeter = 0;	//��ֱ�ֱ���
     bi.biClrUsed = 0;			//��ʾʹ���˶��ٸ���ɫ������һ��biBitCount����С��16�Ż��õ�������0ʱ��ʾ��2^biBitCount����ɫ������
     bi.biClrImportant = 0;		//��ʾ�ж��ٸ���Ҫ����ɫ������0ʱ��ʾ������ɫ������Ҫ

     //Set BITMAPFILEHEADER  ����bmpͼƬ���ļ�ͷ��ʽ
     bf.bfType = 0x4d42;		//2���ֽڣ������0x4d42��ascii�ַ���BM��
     bf.bfSize = 54 + bi.biSizeImage;	//�ļ���С����4���ֽ�Ϊ��λ
     bf.bfReserved1 = 0;		//����
     bf.bfReserved2 = 0;		//����
     bf.bfOffBits = 54;			//���������ļ��е�λ��ƫ����

	/*******************************/

    //f_d=open(DEFAULT_FILE_NAME,O_RDWR|O_CREAT,0666);//��ȡ�ļ���������
	if(0==v4l_open("/dev/video0",vd)) //���豸
		printf("open success!\n");
	else
        printf("open failure\n");

	if(0==v4l_set_norm(vd,norm))
		printf("set_norm success\n");
	else
		printf("set_norm failure\n");

    if(0==v4l_grab_init(vd,IMAGEWIDTH,IMAGEHEIGHT))//��ʼ���豸�������ȡͼ��Ĵ�С
		printf("init success!\n");
	else
        printf("init failure\n");
	
	if(0==v4l_mmap_init(vd))//�ڴ�ӳ��
		printf("memory map success!\n");
	else
		printf("memory map failure\n");

////////////////////////////////////////////////////
	//��ʼһ֡���ݵĲɼ�	
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

    if(0==v4l_grab_start(vd,frame))//��ʼ��ȡͼ��
		printf("get picture success!\n");
	else
		printf("get picture failure\n");
	
	v4l_grab_sync(vd,frame);//�ȴ�����һ֡
	buffer=(char *)v4l_get_address(vd);//�õ���һ֡�ĵ�ַ
	printf("img address %p\n",buffer);
	
	/*************************************/
	y=buffer;
	u=buffer+IMAGEWIDTH*IMAGEHEIGHT;
	v=u+IMAGEWIDTH*IMAGEHEIGHT/4;
     
	fwrite(&bf, 14, 1, fp);	//���ļ���д��ͼƬ�ļ�ͷ
	fwrite(&bi, 40, 1, fp);	//���ļ���д��ͼƬ��Ϣͷ
	/*fwrite(&bf, 14, 1, fp[tmp]);	//���ļ���д��ͼƬ�ļ�ͷ
	fwrite(&bi, 40, 1, fp[tmp]);	//���ļ���д��ͼƬ��Ϣͷ*/


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
		lpbuffer -= IMAGEWIDTH*3; // ָ��ת����һ�еĿ�ʼ
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