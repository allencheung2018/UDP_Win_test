#include "StdAfx.h"
#include "ACCEncoder.h"

ACCEncoder::ACCEncoder(void)
{
}


ACCEncoder::~ACCEncoder(void)
{
}

int ACCEncoder::select_sample_rate(AVCodec *codec)
{
	const int *p;
	int best_samplerate = 0;

	if (!codec->supported_samplerates)
		return 44100;

	p = codec->supported_samplerates;
	while (*p) {
		best_samplerate = FFMAX(*p, best_samplerate);
		p++;
	}
	return best_samplerate;
}

int ACCEncoder::select_channel_layout(AVCodec *codec)
{
	const uint64_t *p;
	uint64_t best_ch_layout = 0;
	int best_nb_channels   = 0;

	if (!codec->channel_layouts)
		return AV_CH_LAYOUT_STEREO;

	p = codec->channel_layouts;
	while (*p) {
		int nb_channels = av_get_channel_layout_nb_channels(*p);

		if (nb_channels > best_nb_channels) {
			best_ch_layout    = *p;
			best_nb_channels = nb_channels;
		}
		p++;
	}
	return best_ch_layout;
}

int ACCEncoder::check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
	const enum AVSampleFormat *p = codec->sample_fmts;

	while (*p != AV_SAMPLE_FMT_NONE) {
		if (*p == sample_fmt)
			return 1;
		p++;
	}
	return 0;
}

int ACCEncoder::initializeCoder()
{
	uint8_t *samples;
	int ret;
	
	av_register_all();
	avcodec_register_all();

	/* find the MP2 encoder */
	pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!pCodec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		fprintf(stderr, "Could not allocate audio codec context\n");
		exit(1);
	}

	/* put sample parameters */
	pCodecCtx->bit_rate = 64000;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	/* check that the encoder supports s16 pcm input */
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	printf("pCodecCtx->codec_id = %d\n",pCodecCtx->codec_id);
	if (!check_sample_fmt(pCodec, pCodecCtx->sample_fmt)) {
		fprintf(stderr, "Encoder does not support sample format %s",
			av_get_sample_fmt_name(pCodecCtx->sample_fmt));
		exit(1);
	}
	/* select other audio parameters supported by the encoder */
	//pCodecCtx->sample_rate    = select_sample_rate(pCodec);
	//pCodecCtx->sample_rate= 44100;
	pCodecCtx->sample_rate= 16000;
	pCodecCtx->channel_layout=AV_CH_LAYOUT_MONO;
	//pCodecCtx->channel_layout = select_channel_layout(pCodec);
	pCodecCtx->channels       = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	//pCodecCtx->channel_layout=AV_CH_LAYOUT_STEREO;

	/* open it */
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

	/* frame containing input raw audio */
	//pFrame = av_frame_alloc();
	pFrame = avcodec_alloc_frame();
	if (!pFrame) {
		fprintf(stderr, "Could not allocate audio frame\n");
		exit(1);
	}

	pFrame->nb_samples     = pCodecCtx->frame_size;
	pFrame->format         = pCodecCtx->sample_fmt;
	//pFrame->channel_layout = pCodecCtx->channel_layout;

	    /* the codec gives us the frame size, in samples,
     * we calculate the size of the samples buffer in bytes */
    buffer_size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size,
                                             pCodecCtx->sample_fmt, 0);
    if (buffer_size < 0) {
        fprintf(stderr, "Could not get sample buffer size\n");
        exit(1);
    }
    //samples = (uint8_t *)av_malloc(buffer_size);
    //if (!samples) {
    //    fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
    //            buffer_size);
    //    exit(1);
    //}
    /* setup the data pointers in the AVFrame */
    //ret = avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt,
    //                               (const uint8_t*)samples, buffer_size, 0);
    //if (ret < 0) {
    //    fprintf(stderr, "Could not setup audio frame\n");
    //    exit(1);
    //}
	//avformat_write_header(pFormatCtx,NULL);

	av_new_packet(&pkt,buffer_size);

	return 1;
}

int ACCEncoder::encodeFrame(void* outdata, const void* indata)
{
	int got_frame=0, ret, outsize =0;
	static int i = 0;
	static int framecnt = 0;
	
	av_init_packet(&pkt);
	pkt.data = NULL; // packet data will be allocated by the encoder
	pkt.size = 0;
	pFrame->data[0] = (uint8_t *)indata;  //PCM Data
	//memcpy(pFrame->data[0],indata,buffer_size);
	pFrame->pts=i*100;
	//Encode
	ret = avcodec_encode_audio2(pCodecCtx, &pkt,pFrame, &got_frame);
	if(ret < 0){
		printf("Failed to encode!\n");
		return -1;
	}
	if (got_frame==1){
		printf("Succeed to encode %d frame! \tsize:%5d\n",framecnt,pkt.size);
		pkt.stream_index = 0;
		framecnt++;
		//ret = av_write_frame(pFormatCtx, &pkt);
		memcpy(outdata,pkt.data,pkt.size);
		outsize += pkt.size;
		av_free_packet(&pkt);
	}
	i++;
	return outsize;
}

void ACCEncoder::closeCoder()
{
	//Write Trailer
	//av_write_trailer(pFormatCtx);

	//Clean
	//avcodec_close(pCodecCtx);
	//av_free(pFrame);
	//	//av_free(frame_buf);
	//avio_close(pFormatCtx->pb);
	//avformat_free_context(pFormatCtx);

	//av_freep(&samples);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
}
