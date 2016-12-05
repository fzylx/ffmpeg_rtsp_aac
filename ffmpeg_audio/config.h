



#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <stdio.h>
#include <string.h>  
#include <sys/types.h>  
#include <windows.h>  
  
#define TS_PACKET_SIZE 188  
#define MTU 1500  

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <SDL/SDL.h>
#ifdef __cplusplus
};
#endif
#endif


/**
 * GDI Device Demuxer context
 */
struct gdigrab {
    int        draw_mouse;  /**< Draw mouse cursor (private option) */
};



#pragma comment(lib,"ws2_32.lib")
#endif