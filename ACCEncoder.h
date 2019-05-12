//#ifndef ACCENCODER_H
//#define ACCENCODER_H
#pragma once
#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
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


class ACCEncoder
{
public:
	ACCEncoder(void);
	~ACCEncoder(void);
private:
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* audio_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;
	AVFrame* pFrame;
	AVPacket pkt;

	int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt);
	int select_sample_rate(AVCodec *codec);
	int select_channel_layout(AVCodec *codec);
public:
	int buffer_size;
	
	int initializeCoder();
	int encodeFrame(void* outdata, const void* indata);
	void closeCoder();
};
//#endif
