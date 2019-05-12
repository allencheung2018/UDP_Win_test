#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
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
#ifdef __cplusplus
};
#endif
#endif

#pragma once
class YUVEncoder
{
public:
	YUVEncoder(void);
	YUVEncoder(int in_w, int in_h);
	YUVEncoder(int in_w, int in_h, int fps);
	~YUVEncoder(void);
private:
	AVCodec *pCodec;
	AVCodecContext *pCodecCtx;
	AVFrame *pFrame;
	AVPacket pkt;
	AVCodecID codec_id;
	int m_h;
	int m_w;
	int m_fps;
public:
	int initializeCoder();
	int encodeFrame(void* outdata, const void* indata);
	int encodeFrameYV12(void* outdata, const void* indata);
	void closeCoder();

	int m_y_size;
};

