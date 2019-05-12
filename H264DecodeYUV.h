#pragma once
extern "C"{
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif
}

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/parseutils.h>
#ifdef __cplusplus
};
#endif
#endif

#define INBUF_SIZE 4096


typedef struct H264DATA
{   
	unsigned char h264[100000];   
	int size;
}H264DATA;

class H264DecodeYUV
{
public:
	H264DecodeYUV(void);
	H264DecodeYUV(int width, int height);
	~H264DecodeYUV();

	int initialDecoder();
	int decodeFrame(char* outdata, const void* indata, int size);
	int decodeFrametoRGB(char* outdata, const void* indata, int size);
	int decodeFrametoRGB2(char* outdata, const void* indata, int size);
	int decodeFrametoRGB16(char* outdata, uint8_t* indata, int size);
	int decodeFrametoRGB16_scale(char* outdata, uint8_t* indata, int size);
	int decodeFrametoYUV420_scale(char* outdata, uint8_t* indata, int size);
	int closeDecoder();
	void YV12toRGB24(unsigned char* pYV0,unsigned char* pYV1,unsigned char* pYV2,unsigned char* pRGB24,int Width,int Height);
private: 
	AVCodecContext *codec_ ;
	AVFrame *pFrame_;
	AVFrame* pFrameRGB;
	AVCodec *videoCodec;
	AVPacket packet;
	struct SwsContext *sws_ctx;
	int m_height, m_SrcHeight;
	int m_width, m_SrcWidth;
	int m_Picsize;
	uint8_t *dst_data[4], *src_data[4];
	int dst_linesize[4], src_linesize[4];
	unsigned char *buf, *rgbbuf;
	enum AVPixelFormat src_pix_fmt;
	enum AVPixelFormat dst_pix_fmt;
	bool mIsInit;
};

