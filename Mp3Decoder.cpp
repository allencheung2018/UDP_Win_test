#include "StdAfx.h"
#include "Mp3Decoder.h"


Mp3Decoder::Mp3Decoder(void)
{
	initial();
}


Mp3Decoder::~Mp3Decoder(void)
{
}

int Mp3Decoder::initial()
{
	int ret;
	/* register all the codecs */
	avcodec_register_all();

	// Find the decoder for the audio stream  
	pCodec=avcodec_find_decoder(AV_CODEC_ID_MP3);  
	if(pCodec==NULL){  
		printf("Codec not found.\n");  
		return -1;  
	}
	pCodecCtx=avcodec_alloc_context3(pCodec);
	// Open codec  
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){  
		printf("Could not open codec.\n");  
		return -2;  
	}
	pFrame=av_frame_alloc(); 
	//Swr  
	au_convert_ctx = swr_alloc();  
	if (!au_convert_ctx) {
		fprintf(stderr, "Could not allocate resampler context\n");
		ret = AVERROR(ENOMEM);
	}
	////////////////////////////////////////以下代码如确定输出可以转移到初始化完成////////////////////////////
	//In Audio Param---输入的参数在呈贡解码后可以得到先设置初始参数
	src_nb_samples = 1152;//pCodecCtx->frame_size;
	uint64_t in_channel_layout=AV_CH_LAYOUT_STEREO;
	int  src_nb_channels=av_get_default_channel_layout(in_channel_layout);
	src_rate = 44100;//pCodecCtx->sample_rate;
	enum AVSampleFormat src_sample_fmt = AV_SAMPLE_FMT_S16P;//pCodecCtx->sample_fmt;
	//Out Audio Param  
	dst_nb_samples=src_nb_samples;//pCodecCtx->frame_size;
	uint64_t out_channel_layout=AV_CH_LAYOUT_MONO;//;//pCodecCtx->channel_layout;//AV_CH_LAYOUT_STEREO;
	out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
	dst_sample_fmt =AV_SAMPLE_FMT_S16;
	dst_rate=16000;//16000;//pCodecCtx->sample_rate;
	/* set options */
	av_opt_set_int(au_convert_ctx, "in_channel_layout",    in_channel_layout, 0);
	av_opt_set_int(au_convert_ctx, "in_sample_rate",       src_rate, 0);
	av_opt_set_sample_fmt(au_convert_ctx, "in_sample_fmt", src_sample_fmt, 0);
	av_opt_set_int(au_convert_ctx, "out_channel_layout",    out_channel_layout, 0);
	av_opt_set_int(au_convert_ctx, "out_sample_rate",       dst_rate, 0);
	av_opt_set_sample_fmt(au_convert_ctx, "out_sample_fmt", dst_sample_fmt, 0);
	//second method to set opts
/*		au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,  
		in_channel_layout,pCodecCtx->sample_fmt , pCodecCtx->sample_rate,0, NULL); */ 
	ret = swr_init(au_convert_ctx);
	if (ret < 0)
	{
		fprintf(stderr, "Failed to initialize the resampling context\n");
	}
	/* buffer is going to be directly written to a rawaudio file, no alignment */
	max_dst_nb_samples = dst_nb_samples =  	av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
	ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, out_channels, dst_nb_samples, dst_sample_fmt, 0);

	return 1;
}

int Mp3Decoder::close()
{
	// Close the codec  
	if (dst_data)
		av_freep(&dst_data[0]);
	av_freep(&dst_data);

	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_frame_free(&pFrame);
	swr_free(&au_convert_ctx);

	return 1;
}

int Mp3Decoder::decodeFrame(void* outdata, const void* indata, int size)
{
	AVPacket packet ={0};
	int got_picture =0;

	//av_init_packet(&packet);
	packet.data = (uint8_t *)indata;
	packet.size = size;
	int ret = avcodec_decode_audio4( pCodecCtx, pFrame,&got_picture, &packet);  
	if ( ret < 0 ) {  
		printf("Error in decoding audio frame.\n");  
		return -1;  
	}  
	if ( got_picture > 0 )
	{  
		////////////////////////////////////////////////////swr_get_delay会根据输入的变化来调整输出缓冲///////////////////////////////////////////////////////////////////////
		/* compute destination number of samples */
		dst_nb_samples = av_rescale_rnd(swr_get_delay(au_convert_ctx, src_rate) +
			src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
		if (dst_nb_samples > max_dst_nb_samples) {
			av_freep(&dst_data[0]);
			ret = av_samples_alloc(dst_data, &dst_linesize, out_channels, dst_nb_samples, dst_sample_fmt, 1);
			if (ret < 0)
				return 0;
			max_dst_nb_samples = dst_nb_samples;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		ret = swr_convert(au_convert_ctx,(uint8_t**)dst_data, MAX_AUDIO_FRAME_SIZE,(const uint8_t **)pFrame->data , pFrame->nb_samples);
		//Out Buffer Size 
		int out_buffer_size=av_samples_get_buffer_size(&dst_linesize,out_channels ,ret,dst_sample_fmt, 1); 
		memcpy(outdata, dst_data[0], out_buffer_size);

		////////////////////////以下代码是手动转换S16P->S16格式///////////////////////////////
		//for (int i=0; i<out_nb_samples; i++)
		//{
		//	int tmp1 = i*4;
		//	int tmp2 = i*2;
		//	memcpy((uint8_t *)outdata+tmp1, pFrame->data[0]+tmp2, 2);
		//	memcpy((uint8_t *)outdata+tmp1+2, pFrame->data[1] +tmp2, 2);
		//}

		return out_buffer_size;
	}
	return 0;
}
