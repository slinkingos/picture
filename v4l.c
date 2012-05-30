/*
*this is the main program 
*
*
*
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <error.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include "v4l.h"

#define DEFAULT_DEVICE "/dev/video0"

int v4l_open(char *dev,v4ldevice *vd)
{
    if (!dev)
		dev = DEFAULT_DEVICE;

    if((vd->fd=open(dev,O_RDWR,10705))<0)
    {
        return -1;
    }

    if(v4l_get_capability(vd)<0)
        return -1;
    if(v4l_init_window(vd)<0)
        return -1;
    if(v4l_get_picture(vd)<0)
		return -1;
	return 0;
}

int v4l_get_capability(v4ldevice *vd)
{
    if(ioctl(vd->fd,VIDIOCGCAP,&(vd->capability))<0)
    {   
        perror("v4l_get_capability:");
        return -1;
    }
/*
	printf("devicename ----->%s\n",vd->capability.name);
    printf("devicetype ----->%d\n",vd->capability.type);
    printf("channels ------->%d\n",vd->capability.channels);
    printf("maxwidth ------->%d\n",vd->capability.maxwidth);
    printf("maxheith ------->%d\n",vd->capability.maxheight);
    printf("minwidth ------->%d\n",vd->capability.minwidth);
    printf("minheith ------->%d\n\n",vd->capability.minheight);*/

	return 0;
}

int v4l_init_window(v4ldevice *vd)
{
	if( ioctl(vd->fd,VIDIOCGWIN,&(vd->window))<0 ) 
	{
		printf("ERROR:VIDIOCGWIN\n");
	}

	/*printf("window x ------->%s\n",vd->window.x);
    printf("window y ------->%d\n",vd->window.y);
    printf("window width --->%d\n",vd->window.width);
    printf("window height -->%d\n\n",vd->window.height);*/


	vd->window.x = 0;  //windows中的原点坐标
	vd->window.y = 0;  //windows中的原点坐标
	vd->window.width = IMAGEWIDTH; //capture area 宽度
	vd->window.height = IMAGEHEIGHT; //capture area 高度

	/*使用IOCTL命令VIDIOCSWIN，设置摄像头的基本信息*/
	if (ioctl(vd->fd, VIDIOCSWIN, &(vd->window)) < 0) 
	{
		printf("ERROR:VIDIOCSWIN\n");
	}
}


int v4l_get_picture(v4ldevice *vd)
{
    if(ioctl(vd->fd,VIDIOCGPICT,&(vd->picture))<0)
    {
        perror("v4l_get_picture");
        return -1;       
    }
	/*printf("Brightness ----->%d\n",vd->picture.brightness);
    printf("Hue ------------>%d\n",vd->picture.hue);
    printf("Colour --------->%d\n",vd->picture.colour);
    printf("Contrast ------->%d\n",vd->picture.contrast);
	printf("Whiteness ------>%d\n",vd->picture.whiteness);
    printf("Capture depth -->%d\n",vd->picture.depth);
    printf("Palette -------->%d\n\n",vd->picture.palette);*/

   return 0;
}

int v4l_set_norm(v4ldevice *vd, int norm)
{
	int i;
	for (i = 0; i < vd->capability.channels; i++) 
      vd->channel[i].norm = norm;
	return 0;
}

int v4l_grab_init(v4ldevice *vd,int width,int height)
{
    vd->mmap.width=width;
    vd->mmap.height=height;
    vd->mmap.format=vd->picture.palette;
    vd->frame=0;
    vd->framestat[0]=0;
    vd->framestat[1]=0;   
    return 0;
}

int v4l_mmap_init(v4ldevice *vd)
{
    if(v4l_get_mbuf(vd)<0)
        return -1;
    if((vd->map=mmap(0,vd->mbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,vd->fd,0))<0)
    {
        return -1;
    }
    return 0;
}

int v4l_get_mbuf(v4ldevice *vd)//查询实际可用的缓存数
{
    if(ioctl(vd->fd,VIDIOCGMBUF,&(vd->mbuf))<0)
    {
        perror("v4l_get_mbuf:");   
        return -1;
    }
    printf("\nsize=%d\n",vd->mbuf.size);
	printf("Frames:%d\n\n",vd->mbuf.frames);
	return 0;
}

int v4l_grab_start(v4ldevice *vd,int frame)
{
    vd->mmap.frame=frame;
    if(ioctl(vd->fd,VIDIOCMCAPTURE,&(vd->mmap))<0)
    {
        exit(-1);
        return -1;
    }
    vd->framestat[frame]=1;
    return 0;
}
int v4l_grab_sync(v4ldevice *vd,int frame)
{
    if(ioctl(vd->fd,VIDIOCSYNC,&frame)<0)
    {
        return -1;   
    }
    vd->framestat[frame]=0;
    return 0;
}

int v4l_close(v4ldevice *vd)
{
    close(vd->fd);
    return 0;
}

unsigned char * v4l_get_address(v4ldevice *vd)
{
    return (vd->map+vd->mbuf.offsets[vd->frame]);
}


/********************YUV420P to RGB24*************************/

// Conversion from YUV420 to RGB24
static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];			//for clip in CCIR601
static unsigned char convert_uu[IMAGEWIDTH*IMAGEHEIGHT];
static unsigned char convert_vv[IMAGEWIDTH*IMAGEHEIGHT];

//
//Initialize conversion table for YUV420 to RGB
//
void InitConvertTable()
{
   long int crv,cbu,cgu,cgv;
   int i,ind;   
     
   crv = 104597; cbu = 132201;  /* fra matrise i global.h */ 
   cgu = 25675;  cgv = 53279;
  
   for (i = 0; i < 256; i++) {
      crv_tab[i] = (i-128) * crv;
      cbu_tab[i] = (i-128) * cbu;
      cgu_tab[i] = (i-128) * cgu;
      cgv_tab[i] = (i-128) * cgv;
      tab_76309[i] = 76309*(i-16);
   }
	 
   for (i=0; i<384; i++)
	  clp[i] =0;
   ind=384;
   for (i=0;i<256; i++)
	   clp[ind++]=i;
   ind=640;
   for (i=0;i<384;i++)
	   clp[ind++]=255;
}


//
//  Convert from YUV420 to RGB24
//
void ConvertYUV2RGB(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,int width,int height)
{
	int y1,y2,u,v;
	unsigned char *py1,*py2;
	int i,j, c1, c2, c3, c4;
	unsigned char *d1, *d2;

	py1=src0;
	py2=py1+width;
	d1=dst_ori;
	d2=d1+3*width;
 	for (j = 0; j < height; j += 2) { 
		for (i = 0; i < width; i += 2) {

			u = *src1++;
			v = *src2++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
            y1 = tab_76309[*py1++];	
			*d1++ = clp[384+((y1 + c1)>>16)];  
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
            *d1++ = clp[384+((y1 + c4)>>16)];

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384+((y2 + c1)>>16)];  
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c4)>>16)];

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384+((y1 + c1)>>16)];  
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
			*d1++ = clp[384+((y1 + c4)>>16)];

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384+((y2 + c1)>>16)];  
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c4)>>16)];
		}
		d1 += 3*width;
		d2 += 3*width;
		py1+=   width;
		py2+=   width;
	}       
}
