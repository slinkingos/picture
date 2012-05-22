#ifndef _V4L_H_
#define _V4L_H_

#include <stdio.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include <unistd.h>

//PAL CIF NTSC tuner mode
/*#define PAL_WIDTH 768 
#define PAL_HEIGHT 576
#define CIF_WIDTH 352
#define CIF_HEIGHT 288*/
#define NTSC_WIDTH 640  //���û�ȡͼ��Ĵ�С
#define NTSC_HEIGHT 480
#define  IMAGEWIDTH    320	//ͼƬ�Ŀ��
#define  IMAGEHEIGHT   240	//ͼƬ�ĸ߶�
#define DEFAULT_PALETTE VIDEO_PALETTE_YUV420P

struct _v4l_device
{
    int fd;//�豸��
    struct video_capability capability;//����ͷ����
    struct video_picture picture;//ͼ������ԣ����ȡ�ɫ�ȡ��Աȶȡ��Ҷȡ���������
    struct video_window window;//����capture area ����Ϣ
    struct video_channel channel[8];//�ɼ���ͨ��
    struct video_mbuf mbuf;//����mmapӳ�������Ϣ
    struct video_capture capture;
    struct video_buffer buffer;
    struct video_mmap mmap;
    unsigned char *map;
    int frame;
    int framestat[2];
};

/*******************add by linhui*******************************/
#define WIDTHBYTES(i)  ((i+31)/32*4)
#define  FREE(x)       if((x)){free((x));(x)=NULL;}

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

typedef struct tagBITMAPFILEHEADER{
     WORD bfType;              // the flag of bmp, value is "BM"
     DWORD    bfSize;          // size BMP file ,unit is bytes
     DWORD    bfReserved1;     // 0
     DWORD    bfReserved2;     // 0
     DWORD    bfOffBits;       // must be 54
}BITMAPFILEHEADER;				//����ṹ��ռ��14�ֽ�

typedef struct tagBITMAPINFOHEADER{
     DWORD    biSize;            // must be 0x28
     DWORD    biWidth;           //
     DWORD    biHeight;          //
     WORD	  biPlanes;          // must be 1
     WORD	  biBitCount;        //
     DWORD    biCompression;     //
     DWORD    biSizeImage;       //
     DWORD    biXPelsPerMeter;   //
     DWORD    biYPelsPerMeter;   //
     DWORD    biClrUsed;         //
     DWORD    biClrImportant;    //
}BITMAPINFOHEADER;					//����ṹռ��40���ֽ�


typedef struct tagRGBQUAD{
     BYTE   rgbBlue;
     BYTE	rgbGreen;
     BYTE	rgbRed;
     BYTE	rgbReserved;
}RGBQUAD;

/****************************************************************/

typedef struct _v4l_device v4ldevice;

extern int v4l_open(char *,v4ldevice *);
extern int v4l_set_norm(v4ldevice *, int);
extern int v4l_get_capability(v4ldevice *);
extern int v4l_get_window(v4ldevice *);
extern int v4l_set_window(v4ldevice *);
extern int v4l_get_picture(v4ldevice *);
extern int v4l_mmap_init(v4ldevice *);
extern int v4l_grab_init(v4ldevice *,int ,int);
extern int v4l_grab_start(v4ldevice *,int );
extern int v4l_grab_sync(v4ldevice *,int);
extern int v4l_get_capture(v4ldevice *);//,int);
extern int v4l_set_capture(v4ldevice *);//,int);
extern unsigned char *v4l_get_address(v4ldevice *);
extern int v4l_close(v4ldevice *);
extern void set(v4ldevice *);


// Conversion from YUV420 to RGB24
void InitConvertTable();
void ConvertYUV2RGB(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,
								 int width,int height);


#endif