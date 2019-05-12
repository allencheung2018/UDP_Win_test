#include "StdAfx.h"
#include "YUVEncoder.h"


YUVEncoder::YUVEncoder(void)
{
}

YUVEncoder::YUVEncoder(int _w, int _h)
{
	pCodecCtx= NULL;
	codec_id=AV_CODEC_ID_H264;
	m_w = _w;
	m_h = _h;
	m_y_size = m_w * m_h;
	initializeCoder();
}

YUVEncoder::YUVEncoder(int in_w, int in_h, int fps)
{
	pCodecCtx= NULL;
	codec_id=AV_CODEC_ID_H264;
	m_w = in_w;
	m_h = in_h;
	m_y_size = m_w * m_h;
	if (fps>0 && fps <30)
	{
		m_fps = fps;
	}
	else
	{
		m_fps = 25;
	}
	initializeCoder();
}


YUVEncoder::~YUVEncoder(void)
{
}

int YUVEncoder::initializeCoder()
{
	int ret;
	
	avcodec_register_all();

	pCodec = avcodec_find_encoder(codec_id);
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		printf("Could not allocate video codec context\n");
		return -1;
	}
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	//pCodecCtx->refs = 3;												//number of reference frames
	//pCodecCtx->trellis = 1;
	//pCodecCtx->bit_rate = 10000000;							//the average bitrate
	//pCodecCtx->rc_max_rate = 2000000;					//maximum bitrate
	//pCodecCtx->bit_rate_tolerance = 2;						//number of bits the bitstream is allowed to diverge from the reference.
	//pCodecCtx->me_method = 2;									//Motion estimation algorithm used for video coding.
	pCodecCtx->width = m_w;
	pCodecCtx->height = m_h;
	pCodecCtx->time_base.num=1;
	pCodecCtx->time_base.den=m_fps;//25;				//fps必须在初始化设置
	pCodecCtx->gop_size = 250;										//the number of pictures in a group of pictures, or 0 for intra_only
	//pCodecCtx->ticks_per_frame = 2;							//Set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2.
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	//H264  
	//pCodecCtx->me_method = ME_UMH;						//Motion estimation algorithm used for video coding.
	//pCodecCtx->me_subpel_quality = 7;
	//pCodecCtx->me_range = 32;  
	//pCodecCtx->max_qdiff = 4;  
	//pCodecCtx->qcompress = 0.6;
	//pCodecCtx->b_frame_strategy = 1;
	pCodecCtx->max_b_frames = 3;								//maximum number of B-frames between non-B-frames
	//pCodecCtx->has_b_frames = 1;								//Size of the frame reordering buffer in the decoder.
	//pCodecCtx->mpeg_quant = 1;									//0-> h263 quant 1-> mpeg quant
	//pCodecCtx->i_quant_factor = 1.3;
	//pCodecCtx->b_quant_factor = 1.4;
	//pCodecCtx->flags2 |= CODEC_FLAG2_FAST;
	pCodecCtx->qmin = 10;  
	pCodecCtx->qmax = 51;
	//pCodecCtx->keyint_min = 25;										//minimum GOP size
	//pCodecCtx->level = 30;												//
	//pCodecCtx->profile = FF_PROFILE_H264_HIGH;

	if (codec_id == AV_CODEC_ID_H264)
		//av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
		av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	if (!pFrame) {
		printf("Could not allocate video frame\n");
		return -1;
	}
	pFrame->format = pCodecCtx->pix_fmt;
	pFrame->width  = pCodecCtx->width;
	pFrame->height = pCodecCtx->height;

	ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, 16);
	if (ret < 0) {
		printf("Could not allocate raw picture buffer\n");
		return -1;
	}
	return 1;
}

int YUVEncoder::encodeFrame(void* outdata, const void* indata)
{
	int ret, got_output, out_size =0, itemp;
	static int i = 0, framecnt =0;
	uint8_t* pctemp = (uint8_t*)indata;
	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	memcpy(pFrame->data[0],pctemp,m_y_size);
	memcpy(pFrame->data[1],pctemp+m_y_size,m_y_size/4);
	itemp = m_y_size+m_y_size/4;
	//memcpy(pFrame->data[2],pctemp+itemp,m_y_size/4);
	memcpy_s(pFrame->data[2],m_y_size/4,pctemp+itemp,m_y_size/4);
	pFrame->pts = i;
	////////////////帧率只能在初始化时设置，这里设置无效////////////////
	//pCodecCtx->time_base.num=1;
	//pCodecCtx->time_base.den=15;		//原始视频帧率15fps
	/* encode the image */
	ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
	if (ret < 0) {
		printf("Error encoding frame\n");
		return -1;
	}
	if (got_output) {
		printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
		framecnt++;
		memcpy(outdata,pkt.data,pkt.size);
		out_size += pkt.size;
		av_free_packet(&pkt);
	}
	i++;
	return out_size;
}

int YUVEncoder::encodeFrameYV12(void* outdata, const void* indata)
{
	int ret, got_output, out_size =0, itemp;
	static int i = 0, framecnt =0;
	uint8_t* pctemp = (uint8_t*)indata;
	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	memcpy(pFrame->data[0],pctemp,m_y_size);
	memcpy(pFrame->data[2],pctemp+m_y_size,m_y_size/4);
	itemp = m_y_size+m_y_size/4;
	memcpy(pFrame->data[1],pctemp+itemp,m_y_size/4);
	//memcpy_s(pFrame->data[2],m_y_size/4,pctemp+itemp,m_y_size/4);
	pFrame->pts = i;
	/* encode the image */
	ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
	if (ret < 0) {
		printf("Error encoding frame\n");
		return -1;
	}
	if (got_output) {
		printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
		framecnt++;
		memcpy(outdata,pkt.data,pkt.size);
		out_size += pkt.size;
		av_free_packet(&pkt);
	}
	i++;
	return out_size;
}

void YUVEncoder::closeCoder()
{
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_freep(&pFrame->data[0]);
	av_frame_free(&pFrame);
}

