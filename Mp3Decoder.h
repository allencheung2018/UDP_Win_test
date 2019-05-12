#pragma once

#include <stdio.h>

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio 

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif
#endif

class Mp3Decoder
{
public:
	Mp3Decoder(void);
	~Mp3Decoder(void);
public:
	int initial();
	int decodeFrame(void* outdata, const void* indata, int size);
	int close();
private:
	AVCodecContext  *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	struct SwrContext *au_convert_ctx;
	uint8_t **dst_data;
	int out_channels;
	int dst_rate, src_rate;
	int dst_nb_samples, src_nb_samples;
	int dst_linesize;
	int max_dst_nb_samples;
	enum AVSampleFormat dst_sample_fmt;
};

