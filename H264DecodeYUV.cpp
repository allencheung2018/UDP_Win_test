#include "StdAfx.h"
#include "H264DecodeYUV.h"

static int Table_fv1[256]={ -180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165, -163, -162, -160, -159, -158, -156, -155, -153, -152, -151, -149, -148, -146, -145, -144, -142, -141, -139, -138, -137, -135, -134, -132, -131, -130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114, -113, -111, -110, -108, -107, -106, -104, -103, -101, -100, -99, -97, -96, -94, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68, -66, -65, -64, -62, -61, -59, -58, -57, -55, -54, -52, -51, -50, -48, -47, -45, -44, -43, -41, -40, -38, -37, -36, -34, -33, -31, -30, -29, -27, -26, -24, -23, -22, -20, -19, -17, -16, -15, -13, -12, -10, -9, -8, -6, -5, -3, -2, 0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22, 23, 25, 26, 28, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57, 58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82, 84, 85, 86, 88, 89, 91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116, 117, 119, 120, 122, 123, 124, 126, 127, 129, 130, 131, 133, 134, 136, 137, 138, 140, 141, 143, 144, 145, 147, 148, 150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 166, 168, 169, 171, 172, 173, 175, 176, 178 };
static int Table_fv2[256]={ -92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83, -82, -81, -81, -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73, -72, -71, -71, -70, -69, -68, -68, -67, -66, -66, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -53, -53, -52, -51, -51, -50, -49, -48, -48, -47, -46, -46, -45, -44, -43, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35, -34, -33, -33, -32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23, -23, -22, -21, -21, -20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13, -12, -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 70, 70, 71, 72, 72, 73, 74, 75, 75, 76, 77, 77, 78, 79, 80, 80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90 };
static int Table_fu1[256]={ -44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40, -40, -39, -39, -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35, -35, -35, -34, -34, -33, -33, -33, -32, -32, -32, -31, -31, -31, -30, -30, -30, -29, -29, -29, -28, -28, -28, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -24, -23, -23, -22, -22, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11, -11, -11, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43 };
static int Table_fu2[256]={ -227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208, -206, -204, -203, -201, -199, -197, -196, -194, -192, -190, -188, -187, -185, -183, -181, -180, -178, -176, -174, -173, -171, -169, -167, -165, -164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144, -142, -141, -139, -137, -135, -134, -132, -130, -128, -126, -125, -123, -121, -119, -118, -116, -114, -112, -110, -109, -107, -105, -103, -102, -100, -98, -96, -94, -93, -91, -89, -87, -86, -84, -82, -80, -79, -77, -75, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57, -55, -54, -52, -50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29, -27, -25, -24, -22, -20, -18, -16, -15, -13, -11, -9, -8, -6, -4, -2, 0, 1, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67, 69, 70, 72, 74, 76, 78, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95, 97, 99, 101, 102, 104, 106, 108, 109, 111, 113, 115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 141, 143, 145, 147, 148, 150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 172, 173, 175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198, 200, 202, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223, 225 };

H264DecodeYUV::H264DecodeYUV(void)
{
}
/*
*	初始化
*	width	-目标视频的宽
*	height	-目标视频的高
*/
H264DecodeYUV::H264DecodeYUV(int width, int height)
{
	mIsInit = false;
	m_height = height;
	m_width = width;
	m_Picsize = m_height*m_width;
	if(initialDecoder() == 1)
	{
		mIsInit = true;
	}
}

int H264DecodeYUV::initialDecoder()
{
	if(mIsInit)
		return 1;
	src_pix_fmt = AV_PIX_FMT_YUV420P;
	dst_pix_fmt = AV_PIX_FMT_RGB565LE;
	dst_pix_fmt = AV_PIX_FMT_YUV420P;			//目标视频格式
	m_SrcWidth = 1920;											//预设定的源视频大小
	m_SrcHeight = 1080;
	int ret;
	//下面初始化h264解码库  
	//avcodec_init();  
	//avformat_open_input();
	
	//av_register_all();  

	avcodec_register_all();

	av_init_packet(&packet);

	/* find the video encoder */  
	videoCodec = avcodec_find_decoder(CODEC_ID_H264);  

	if (!videoCodec)   
	{  
		//cout << "codec not found!" << endl;  
		printf("codec not found!\n");
		return -1;  
	}  
	//avcodec_register(videoCodec);

	//pCodecCtx = avcodec_alloc_context();
	codec_ = avcodec_alloc_context3(videoCodec);  

	//初始化参数，下面的参数应该由具体的业务决定  
	//codec_->time_base.num = 1;  
	//codec_->frame_number = 1; //每包一个视频帧  
	//codec_->codec_type = AVMEDIA_TYPE_VIDEO;  
	//codec_->bit_rate = 0;  
	//codec_->time_base.den = 25;//帧率  
	//codec_->width = width;//视频宽  
	//codec_->height = height;//视频高  
	//codec_->pix_fmt=src_pix_fmt;
	//codec_->bit_rate = 1024*1024*1024;

	//if(videoCodec->capabilities&CODEC_CAP_TRUNCATED)
	//{
	//	codec_->flags|= CODEC_FLAG_TRUNCATED;
	//}

	if(avcodec_open2(codec_, videoCodec, NULL) >= 0)  
		;//pFrame_ = avcodec_alloc_frame();// Allocate video frame  
	else  
		return -1;  
	pFrame_ = av_frame_alloc();

	pFrameRGB = av_frame_alloc();
	int numBytes = avpicture_get_size(dst_pix_fmt,m_width,m_height);

	rgbbuf = (uint8_t *)av_malloc( numBytes*sizeof(uint8_t) );
	avpicture_fill((AVPicture *)pFrameRGB, rgbbuf, dst_pix_fmt,  m_width,m_height);

	//printf("width = %d, height = %d\n", m_width, m_height);
	//sws_ctx =sws_alloc_context();
	/* create scaling context */
	sws_ctx = sws_getContext(m_SrcWidth, m_SrcHeight, src_pix_fmt, m_width, m_height, dst_pix_fmt,
		SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws_ctx) {
		fprintf(stderr,
			"Impossible to create scale context for the conversion "
			"fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			av_get_pix_fmt_name(src_pix_fmt), m_width, m_height,
			av_get_pix_fmt_name(dst_pix_fmt), m_width, m_height);
		ret = AVERROR(EINVAL);
		return ret;
	}
	/* allocate source and destination image buffers */
	if ((ret = av_image_alloc(src_data, src_linesize,	m_SrcWidth, m_SrcHeight, src_pix_fmt, 16)) < 0) {
			fprintf(stderr, "Could not allocate source image\n");
			return -1;
	}
	/* buffer is going to be written to rawvideo file, no alignment */
	if ((ret = av_image_alloc(dst_data, dst_linesize,	m_width, m_height, dst_pix_fmt, 1)) < 0) {
			fprintf(stderr, "Could not allocate destination image\n");
			return -2;
	}

	int t = m_Picsize*1.5;
	buf = new unsigned char[t];  

	return 1;
}

int H264DecodeYUV::decodeFrametoRGB16(char* outdata, uint8_t* indata, int size)
{
	AVPacket packet = {0};
	av_init_packet(&packet);
	int frameFinished = 0;//这个是随便填入数字，没什么作用
	int a=0,len,ret=0;

	packet.data = (uint8_t *)indata;//这里填入一个指向完整H264数据帧的指针
	packet.size= size;//这个填入H264数据帧的大小

	if(size > 0)
		len = avcodec_decode_video2(codec_, pFrame_, &frameFinished, &packet);

	//return len;
	if (frameFinished)
	{
		for(int i=0,nDataLen=0;i<3;i++)
		{
			int nshift=(i==0)?0:1;
			uint8_t* pYUVData=pFrame_->data[i];
			for(int j=0;j<(codec_->height>>nshift);j++)
			{
				memcpy(&outdata[nDataLen],pYUVData,codec_->width>>nshift);
				pYUVData+=pFrame_->linesize[i];
				nDataLen+=codec_->width>>nshift;
			}
		}
		//return m_Picsize;
		sws_ctx = sws_getContext(codec_->width, codec_->height, codec_->pix_fmt, codec_->width, codec_->height, dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
		//return m_Picsize;
		sws_scale(sws_ctx, (const uint8_t* const*)pFrame_->data, pFrame_->linesize, 0, codec_->height, pFrameRGB->data, pFrameRGB->linesize);
		ret = codec_->width*codec_->height*2;
		memcpy(outdata,pFrameRGB->data[0],ret);		//rgb565

	}
	printf("codec_->width = %d\t codec_->height = %d\t",codec_->width, codec_->height);
	printf("size = %d\t len = %d\t ret = %d\t frameFinished = %d\t",size, len, ret, frameFinished);
	return ret;
}

int H264DecodeYUV::decodeFrame(char* outdata, const void* indata, int size)
{
	AVPacket packet = {0};  
	int frameFinished = 0;//这个是随便填入数字，没什么作用  
	int a=0,i,len;   

	packet.data = (uint8_t *)indata;//这里填入一个指向完整H264数据帧的指针  
	packet.size= size;//这个填入H264数据帧的大小  

	//下面开始真正的解码  
	while (packet.size > 0)
	{
		len = avcodec_decode_video2(codec_, pFrame_, &frameFinished, &packet);  
		if(frameFinished)//成功解码  
		{  
			int picSize = codec_->height * codec_->width;  
			int newSize = picSize * 1.5;  

			//申请内存  
			unsigned char *buf = new unsigned char[newSize];  

			int height = pFrame_->height;//packet->codec->height;  
			int width = pFrame_->width;//packet->codec->width; 
		
			//写入数据  
			for(int i=0,nDataLen=0;i<3;i++)
			{
				int nshift=(i==0)?0:1;
				uint8_t* pYUVData=pFrame_->data[i];
				for(int j=0;j<(height>>nshift);j++)
				{
					memcpy(buf + nDataLen,pYUVData,width>>nshift);
					pYUVData+=pFrame_->linesize[i];
					nDataLen+=width>>nshift;
				}
			}
			//for (i=0; i<height; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[0] + i * pFrame_->linesize[0], width);   
			//	a+=width;   

			//	if(i<height/2)
			//	{
			//		memcpy(buf+a,pFrame_->data[1] + i * pFrame_->linesize[1], width/2);   
			//		a+=width/2; 
			//		memcpy(buf+a,pFrame_->data[2] + i * pFrame_->linesize[2], width/2);   
			//		a+=width/2; 
			//	}
			//} 
			//for (i=0; i<height; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[0] + i * pFrame_->linesize[0], width);   
			//	a+=width;   
			//}   
			//for (i=0; i<height/2; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[1] + i * pFrame_->linesize[1], width/2);   
			//	a+=width/2;   
			//}   
			//for (i=0; i<height/2; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[2] + i * pFrame_->linesize[2], width/2);   
			//	a+=width/2;   
			//}  
			//===============  
			//到这里，buf里面已经是yuv420p的数据了，可以对它做任何的处理拉！  
			//===============  
			//int temp = 1.5*width;
			//YV12toRGB24(buf,buf+width,buf+temp,(unsigned char*)outdata,width,height);
			memcpy(outdata, buf,a);
		}
		packet.size -= len;
		//outdata += len;
	}
	//memcpy(outdata, buf, a);
	return a;
}

int H264DecodeYUV::closeDecoder()
{
	if (codec_ != NULL)
	{
		delete[] buf;

		av_freep(&dst_data[0]);
		av_freep(&src_data[0]);
		sws_freeContext(sws_ctx);
		avcodec_close(codec_);
		av_free(codec_);
		codec_ = NULL;
		av_frame_free(&pFrame_);
		av_free(pFrameRGB);
	}
	return 1;
}

H264DecodeYUV::~H264DecodeYUV()
{
	closeDecoder();
}

void H264DecodeYUV::YV12toRGB24(unsigned char* pYV0,unsigned char* pYV1,unsigned char* pYV2,unsigned char* pRGB24,int Width,int Height)
{
	if(!pYV0 || !pRGB24)
		return ;

	const long nYLen = long(Width*Height);
	const int nHfWidth = (Width>>1);

	if(nYLen<1 || nHfWidth<1)
		return ;

	// Y data
	unsigned char* yData = pYV0;
	// v data
	unsigned char* vData = pYV1;
	// u data
	unsigned char* uData = pYV2;

	if(!uData || !vData)
		return ;

	int rgb[3];
	int i, j, m, n, x, y, pu, pv, py, rdif, invgdif, bdif;
	m = -Width;
	n = -nHfWidth;

	bool addhalf = true;
	for(y=0; y<Height;y++) 
	{
		m += Width;
		if( addhalf )
		{
			n+=nHfWidth;
			addhalf = false;
		} 
		else 
		{
			addhalf = true;
		}
		for(x=0; x<Width;x++) 
		{
			i = m + x;
			j = n + (x>>1);

			py = yData[i];

			rdif = Table_fv1[vData[j]];    // fv1
			invgdif = Table_fu1[uData[j]] + Table_fv2[vData[j]]; // fu1+fv2
			bdif = Table_fu2[uData[j]]; // fu2

			rgb[0] = py+rdif;    // R
			rgb[1] = py-invgdif; // G
			rgb[2] = py+bdif;    // B

			j = nYLen - Width - m + x;
			i = (j<<1) + j;

			// copy this pixel to rgb data
			for(j=0; j<3; j++)
			{
				if(rgb[j]>=0 && rgb[j]<=255)
				{
					pRGB24[i + j] = rgb[j];
				}
				else
				{
					pRGB24[i + j] = (rgb[j] < 0)? 0 : 255;
				}
			}
		}
	}
}

int H264DecodeYUV::decodeFrametoRGB(char* outdata, const void* indata, int size)
{
	AVPacket packet = {0};  
	int frameFinished = 0;//这个是随便填入数字，没什么作用  
	int a=0,i,len;   

	packet.data = (uint8_t *)indata;//这里填入一个指向完整H264数据帧的指针  
	packet.size= size;//这个填入H264数据帧的大小  

	//下面开始真正的解码  
	while (packet.size > 0)
	{
		len = avcodec_decode_video2(codec_, pFrame_, &frameFinished, &packet);  
		//printf("pFrame_->width = %d, pFrame_->height  = %d\n",pFrame_->width, pFrame_->height);
		//printf("frameFinished = %d\n", frameFinished);
		//printf("len = %d\n", len);
		if(frameFinished)//成功解码  
		{  
			//int picSize = codec_->height * codec_->width;  
			//int newSize = picSize * 1.5;  

			////申请内存  
			//unsigned char *buf = new unsigned char[newSize];  

			//int height = pFrame_->height;//packet->codec->height;  
			//int width = pFrame_->width;//packet->codec->width; 

			//写入数据  
			for(int i=0,nDataLen=0;i<3;i++)
			{
				int nshift=(i==0)?0:1;
				uint8_t* pYUVData=pFrame_->data[i];
				for(int j=0;j<(m_height>>nshift);j++)
				{
					memcpy(buf + nDataLen,pYUVData,m_width>>nshift);
					pYUVData+=pFrame_->linesize[i];
					nDataLen+=m_width>>nshift;
				}
			}

			//for (i=0; i<height; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[0] + i * pFrame_->linesize[0], width);   
			//	a+=width;   
			//	
			//	if(i<height/2)
			//	{
			//		memcpy(buf+a,pFrame_->data[1] + i * pFrame_->linesize[1], width/2);   
			//		a+=width/2; 
			//		memcpy(buf+a,pFrame_->data[2] + i * pFrame_->linesize[2], width/2);   
			//		a+=width/2; 
			//	}
			//}   
			//for (i=0; i<height/2; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[1] + i * pFrame_->linesize[1], width/2);   
			//	a+=width/2;   
			//}   
			//for (i=0; i<height/2; i++)   
			//{   
			//	memcpy(buf+a,pFrame_->data[2] + i * pFrame_->linesize[2], width/2);   
			//	a+=width/2;   
			//}  
			//===============  
			//到这里，buf里面已经是yuv420p的数据了，可以对它做任何的处理拉！  
			//===============  
			printf("m_Picsize = %d m_width = %d m_heigth = %d\n",m_Picsize, m_width, m_height);
			int temp1 = 1.25*m_Picsize, temp = m_Picsize;
			//YV12toRGB24(buf,buf+width,buf+temp,(unsigned char*)outdata,width,height);
			YV12toRGB24(buf,buf+temp, buf+temp1, (unsigned char*)outdata,m_width,m_height);
			//memcpy(outdata, buf,a);
		}
		packet.size -= len;
		//outdata += len;
	}
	//memcpy(outdata, buf, a);
	return pFrame_->height*pFrame_->width*3;
}

int H264DecodeYUV::decodeFrametoRGB2(char* outdata, const void* indata, int size)
{
	AVPacket packet = {0};  
	int frameFinished = 0;//这个是随便填入数字，没什么作用  
	int a=0,i,len,ret;   
	

	packet.data = (uint8_t *)indata;//这里填入一个指向完整H264数据帧的指针  
	packet.size= size;//这个填入H264数据帧的大小  

	//下面开始真正的解码  
	while (packet.size > 0)
	{
		len = avcodec_decode_video2(codec_, pFrame_, &frameFinished, &packet); 
		//printf("pFrame_->width = %d, pFrame_->height  = %d\n",pFrame_->width, pFrame_->height);
		//printf("frameFinished = %d\n", frameFinished);
		//printf("len = %d\n", len);
		if(frameFinished)//成功解码  
		{  
			//int picSize = codec_->height * codec_->width;  
			//int newSize = picSize * 1.5;  

			////申请内存  
			//unsigned char *buf = new unsigned char[newSize];  

			//int height = pFrame_->height;//packet->codec->height;  
			//int width = pFrame_->width;//packet->codec->width; 

			//写入数据  
			for(int i=0,nDataLen=0;i<3;i++)
			{
				int nshift=(i==0)?0:1;
				uint8_t* pYUVData=pFrame_->data[i];
				for(int j=0;j<(m_height>>nshift);j++)
				{
					memcpy(buf + nDataLen,pYUVData,m_width>>nshift);
					pYUVData+=pFrame_->linesize[i];
					nDataLen+=m_width>>nshift;
				}
			}
			//printf("picSize = %d\n",picSize);


			memcpy(src_data[0],buf,m_Picsize);                    //Y  
			memcpy(src_data[1],buf+m_Picsize,m_Picsize/4);      //U  
			memcpy(src_data[2],buf+m_Picsize*5/4,m_Picsize/4);  //V 
			//printf("src_linesize[0] = %d\n",src_linesize[0]);
			sws_scale(sws_ctx, (const uint8_t * const*)src_data,
				src_linesize, 0, m_height, dst_data, dst_linesize);
			memcpy(outdata, dst_data[0],m_Picsize*3);
			//int temp1 = 1.25*width*height, temp = width*height;
			//YV12toRGB24(buf,buf+width,buf+temp,(unsigned char*)outdata,width,height);
			//YV12toRGB24(buf,buf+temp, buf+temp1, (unsigned char*)outdata,width,height);
			//memcpy(outdata, buf,a);

		}
		packet.size -= len;
		//outdata += len;
	}
	//memcpy(outdata, buf, a);
	return pFrame_->height*pFrame_->width*3;
}

int H264DecodeYUV::decodeFrametoRGB16_scale(char* outdata, uint8_t* indata, int size)
{
	AVPacket packet = {0};
	av_init_packet(&packet);
	int frameFinished = 0;//这个是随便填入数字，没什么作用
	int a=0,len,ret=0;

	packet.data = (uint8_t *)indata;//这里填入一个指向完整H264数据帧的指针
	packet.size= size;//这个填入H264数据帧的大小

	if(size > 0)
		len = avcodec_decode_video2(codec_, pFrame_, &frameFinished, &packet);

	int srcW = codec_->width;
	int srcH = codec_->height;
	//return len;
	if (frameFinished)
	{
		char* YUVdata = new char[srcH*srcW*1.5];
		for(int i=0,nDataLen=0;i<3;i++)
		{
			int nshift=(i==0)?0:1;
			uint8_t* pYUVData=pFrame_->data[i];
			for(int j=0;j<(srcH>>nshift);j++)
			{
				memcpy(&YUVdata[nDataLen],pYUVData,srcW>>nshift);
				pYUVData+=pFrame_->linesize[i];
				nDataLen+=srcW>>nshift;
			}
		}
		//return m_Picsize;
		sws_ctx = sws_getContext(srcW, srcH, codec_->pix_fmt, m_width, m_height, dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
		//return m_Picsize;
		sws_scale(sws_ctx, (const uint8_t* const*)pFrame_->data, pFrame_->linesize, 0, srcH, pFrameRGB->data, pFrameRGB->linesize);
		ret = m_width*m_height*2;
		memcpy(outdata,pFrameRGB->data[0],ret);		//rgb565
		delete[] YUVdata;
	}
	printf("codec_->width = %d\t codec_->height = %d\t",srcW, srcH);
	printf("size = %d\t len = %d\t ret = %d\t frameFinished = %d\t",size, len, ret, frameFinished);
	return ret;
}

int H264DecodeYUV::decodeFrametoYUV420_scale(char* outdata, uint8_t* indata, int size)
{
	int frameFinished = 0;//这个是随便填入数字，没什么作用
	int a=0,len,ret=0;

	packet.data = (uint8_t *)indata;//这里填入一个指向完整H264数据帧的指针
	packet.size= size;//这个填入H264数据帧的大小

	if(size > 0)
		len = avcodec_decode_video2(codec_, pFrame_, &frameFinished, &packet);

	int srcW = codec_->width;
	int srcH = codec_->height;
	//return len;
	if (frameFinished)
	{
		if (srcH != m_SrcHeight || srcW != m_SrcWidth)
		{
			m_SrcWidth = srcW;
			m_SrcHeight = srcH;
			sws_freeContext(sws_ctx);
			sws_ctx = sws_getContext(m_SrcWidth, m_SrcHeight, codec_->pix_fmt, m_width, m_height, dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
		}	
		//return m_Picsize;
		sws_scale(sws_ctx, (const uint8_t* const*)pFrame_->data, pFrame_->linesize, 0, srcH, dst_data, dst_linesize);
		ret = m_width*m_height*1.5;
		memcpy(outdata,dst_data[0],ret);		//rgb565
	}
	printf("codec_->width = %d\t codec_->height = %d\t",srcW, srcH);
	printf("size = %d\t len = %d\t ret = %d\t frameFinished = %d\t",size, len, ret, frameFinished);
	return ret;
}
