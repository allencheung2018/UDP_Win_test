#include "StdAfx.h"
#include "Class_HKcard_4008.h"

STRU_ENCODE_CHAN struEncodeChan[MAX_CHANNELS];				//编码通道相关参数
STRU_DECODE_CHAN struDecodeChan[MAX_CHANNELS];				//解码通道相关参数
PSETTING_VALUE g_pSettingValue[MAX_CHANNELS];						//通道设置参数
PSUBSETTING_VALUE g_pSubSettingValue[MAX_CHANNELS]; 
FRAMES_STATISTICS   frame_statistics; 

int			m_iFileHeaderLen;
LONG	m_lPort[MAX_PORTS] = {-1,-1,-1,-1,-1,-1,-1,-1};

Class_HKcard_4008::Class_HKcard_4008(void)
{
	g_nTotalDspCnt = 0;
	g_nTotalEncChannels = 0;			 //编码通道数
	g_nTotalDecChannels = 0;			 //解码通道数
	g_nTotalDispChannels = 0;			 //显示通道数
	m_iFileHeaderLen=0;
}


Class_HKcard_4008::~Class_HKcard_4008(void)
{
}

int Class_HKcard_4008::initial()
{
	SDvs = StandardNTSC;
	SetDefaultVideoStandard(SDvs); 
	if(InitDSPs() >0)
	{
		printf("Init DSP successful!\n");
	}else
	{
		return -1;
	}
	g_nTotalDspCnt = GetDspCount();
	g_nTotalEncChannels = GetEncodeChannelCount(); 
	g_nTotalDecChannels = GetDecodeChannelCount(); 
	g_nTotalDispChannels = GetDisplayChannelCount();
	InitParameter(); 
	context = &m_iFileHeaderLen;

	return 1;
}

void Class_HKcard_4008::InitParameter()
{
	UINT i = 0;
	UINT j = 0;
	int bright = 0;
	int contrast = 0; 
	int sat = 0;
	int hue = 0;
	for (i = 0; i < g_nTotalEncChannels; i++)
	{
		struEncodeChan[i].iPreFramerate = 25; 
		GetVideoPara(struEncodeChan[i].hChanHandle, &struEncodeChan[i].chstandard, &bright, &contrast, &sat, &hue); 

		g_pSettingValue[i] = new SETTING_VALUE; 
		g_pSubSettingValue[i] = new SUBSETTING_VALUE; 

		g_pSettingValue[i]->iEncodeType = 3; 
		g_pSettingValue[i]->iStreamType = 2; 
		g_pSettingValue[i]->bSubSetting = FALSE; 
		g_pSettingValue[i]->iBFrames = 2; 
		g_pSettingValue[i]->iFileSize = 50; 
		g_pSettingValue[i]->iFramerate = 25; 
		g_pSettingValue[i]->iIFramesInterval = 100; 
		g_pSettingValue[i]->iIQuant = 15;			//I 帧量化系数，取值范围： 12-30
		g_pSettingValue[i]->bCBR = FALSE; 
		g_pSettingValue[i]->iPQuant = 15; 
		g_pSettingValue[i]->iQlimitMax = 30; 
		g_pSettingValue[i]->iIQlimitMin = 15; 
		g_pSettingValue[i]->iMaxBps = 2000000; 
		g_pSettingValue[i]->bIQLimit = FALSE; 
		g_pSettingValue[i]->iPFrames = 1; 
		g_pSettingValue[i]->iBQuant = 0; 
		g_pSettingValue[i]->iEFrames = 1; 

		g_pSettingValue[i]->bLogoEnable = TRUE; 
		g_pSettingValue[i]->bLogoTranslucent = TRUE; 
		g_pSettingValue[i]->bLogoTwink = TRUE; 
		g_pSettingValue[i]->iLogoB = 0; 
		g_pSettingValue[i]->iLogoG = 0; 
		g_pSettingValue[i]->iLogoHide = 1; 
		g_pSettingValue[i]->iLogoR = 0; 
		g_pSettingValue[i]->iLogoShow = 1; 
		g_pSettingValue[i]->iLogoX = 0; 
		g_pSettingValue[i]->iLogoY = 0; 
		g_pSettingValue[i]->bOsdTranslucent = TRUE; 
		g_pSettingValue[i]->bOsdEnable = TRUE; 
		g_pSettingValue[i]->iOsdBrightness = 0; 
		g_pSettingValue[i]->iOsdRollback = 0; 
		g_pSettingValue[i]->iOsdX1 = 0; 
		g_pSettingValue[i]->iOsdX2 = 0; 
		g_pSettingValue[i]->iOsdY1 = 0; 
		g_pSettingValue[i]->iOsdY2 = 50; 
		g_pSettingValue[i]->csOsdFormat2 = "录像通道"; 

		g_pSettingValue[i]->bMask = TRUE; 
		g_pSettingValue[i]->bMaskColor = FALSE; 
		g_pSettingValue[i]->iMaskAreaNum = 1; 
		g_pSettingValue[i]->iMaskAreaNumColor = 1; 
		g_pSettingValue[i]->iMaskX = 200; 
		g_pSettingValue[i]->iMaskY = 200; 
		g_pSettingValue[i]->iMaskSizeX = 1; 
		g_pSettingValue[i]->iMaskSizeY = 2; 
		g_pSettingValue[i]->iMaskXColor = 200; 
		g_pSettingValue[i]->iMaskYColor = 200; 
		g_pSettingValue[i]->iMaskSizeXColor = 1; 
		g_pSettingValue[i]->iMaskSizeYColor = 2; 
		g_pSettingValue[i]->iMaskR = 255; 
		g_pSettingValue[i]->iMaskG = 0; 
		g_pSettingValue[i]->iMaskB = 0; 

		g_pSettingValue[i]->bDeInterLace			 = TRUE; 
		g_pSettingValue[i]->iDeInterlaceLevel		 = 11; 
		g_pSettingValue[i]->iPreX					 = 8; 
		g_pSettingValue[i]->iPreY					 = 2; 
		g_pSettingValue[i]->iPreDetectPrecision	 = 20; 

		g_pSettingValue[i]->bMotionDetect = FALSE; 
		g_pSettingValue[i]->bPrivateDate = FALSE; 
		g_pSettingValue[i]->bSelfAdapt = FALSE; 
		g_pSettingValue[i]->bMDCap = FALSE; 
		g_pSettingValue[i]->iFastMD = 2; 
		g_pSettingValue[i]->iMDGrade = 2; 
		g_pSettingValue[i]->iSlowMD = 15; 
		g_pSettingValue[i]->iThreshold = 1; 
		g_pSettingValue[i]->iDelayMD = 5; 	
#ifdef Ver_6_1

		g_pSettingValue[i]->bDeNoise = TRUE;
		g_pSettingValue[i]->iDeNoiseLevel = 0;
		g_pSettingValue[i]->bSetColorMode = TRUE;
		g_pSettingValue[i]->iViColorMode = 0;

		g_pSettingValue[i]->nStreamPackType = 0; 
		g_pSettingValue[i]->nAudioEncodeType = 0; 
#endif

		g_pSubSettingValue[i]->iEncodeType = 1; 
		g_pSubSettingValue[i]->iStreamType = 2; 
		g_pSubSettingValue[i]->bSubSetting = FALSE; 
		g_pSubSettingValue[i]->iBFrames = 2; 
		g_pSubSettingValue[i]->iFileSize = 50; 
		g_pSubSettingValue[i]->iFramerate = 25; 
		g_pSubSettingValue[i]->iIFramesInterval = 100; 
		g_pSubSettingValue[i]->iIQuant = 15; 
		g_pSubSettingValue[i]->bCBR = FALSE; 
		g_pSubSettingValue[i]->iPQuant = 15; 
		g_pSubSettingValue[i]->iQlimitMax = 30; 
		g_pSubSettingValue[i]->iIQlimitMin = 15; 
		g_pSubSettingValue[i]->iMaxBps = 1000000; 
		g_pSubSettingValue[i]->bIQLimit = FALSE; 
		g_pSubSettingValue[i]->iPFrames = 1; 
		g_pSubSettingValue[i]->iBQuant = 0; 
		g_pSubSettingValue[i]->iEFrames = 0; 

#ifdef Ver_6_1	
		g_pSubSettingValue[i]->nStreamPackType = 0; 
		g_pSubSettingValue[i]->nAudioEncodeType = 0; 
#endif

		g_nAudioOutPort[i] = 0;
		g_nEncUseTimes[i]=0;
	}

	for (i = 0; i < g_nTotalDecChannels; i++)
	{
		for (j = 0; j < g_nTotalEncChannels; j++)
		{
			struDecodeChan[i].bOpenRealStream[j] = FALSE; 
		}
		struDecodeChan[i].bOpenStreamSuccessed = FALSE;

		g_nAudioOutPort[MAX_CHANNELS+i] = 0;
		g_nDecUseTimes[i]=0;
	}

	for (i = 0; i<g_nTotalDispChannels;i++)
	{
		g_iDispArea[i] = 0;
		for (j=0;j<16;j++)
		{
			g_DisChoice[i][j] = 0;
			g_LastDisChoice[i][j] = 0;
			g_ZeroOrOne[i][j] = 0;
		}
	}
}

int Class_HKcard_4008::openChannel(int ChannelNum, STREAM_DIRECT_READ_CALLBACK StreamDirectReadCallback, PictureFormat_t encodeType)
{
	encodeType = ENC_4CIF_FORMAT;
	//PictureFormat_t encodeType ; 
	USHORT streamType = STREAM_TYPE_AVSYNC; 

	RegisterStreamDirectReadCallback(StreamDirectReadCallback, context); 

	//for (int i=0; i<ChannelNum;i++)
	{
		struEncodeChan[ChannelNum].hChanHandle = ChannelOpen(ChannelNum);
	
		//设置主通道编码分辨率。支持动态修改。
		SetEncoderPictureFormat(struEncodeChan[ChannelNum].hChanHandle, encodeType); 
	
		//设置主通道编码流类型。此函数需在启动编码前进行设置。
		SetStreamType(struEncodeChan[ChannelNum].hChanHandle, streamType); 
		//设置图像量化系数，用于调整图像质量。量化系数越小图像质量越高。系统默认量化系数值为18， 18， 23。
		//释 义： 量化系数
		//	量化系数是强烈影响 MPEG 和 H.264 标准中编码图像质量和码率的参数，当量化系数越低，图像质量
		//	就会越高， 码率也就越高，反之，图形质量就会越低，码率也就越低
		//参 数： 
		//	HANDLE hChannelHandle；通道句柄
		//	int IQuantVal； I 帧量化系数，取值范围： 12-30
		//	int PQuantVal； P 帧量化系数。取值范围： 12-30(目前无效)
		//	int BQuantVal； B 帧量化系数。取值范围： 12-30(目前无效)
		SetDefaultQuant(struEncodeChan[ChannelNum].hChanHandle, 12, 12, 12);
		//设置编码帧结构和帧率。支持动态修改
		//HANDLE hChannelHandle；通道句柄
		//	int KeyFrameIntervals；关键帧间隔。取值范围１-400，系统默认为 100
		//	int BFrames； B 帧数量，取值为 0 或者 2，系统默认为 2
		//	int PFrames； P 帧数量。目前暂取值无效
		//	int FrameRate；帧率，帧率范围 1-25(PAL)、 1-30(NTSC)
		SetIBPMode(struEncodeChan[ChannelNum].hChanHandle, 250, 2, 1, 25);
		//说 明： 设置编码码流控制方式。配合 SetupBitrateControl 使用。当设置为变码率(brVBR)时，最大比特
		//	率将作为编码码流上限，由 DSP 在码流上限下自动控制码率，一般会自动回落到最低的状态(由设定的图
		//	像质量参数和关键帧间隔决定)，能最大程度地降低带宽和存储空间，但存储容量一般难以估算；当设置为
		//	定码率(brCBR)时，以最大比特率作为编码码率参数恒定输出码流，不会自动回落到低码流状态，存储容量
		//	可根据设定码率的大小进行估算。
		SetBitrateControlMode(struEncodeChan[ChannelNum].hChanHandle, brVBR);
		//说 明： 设置编码的最大比特率。设置为 0 时码流控制无效，设置为某一最大比特率时，当编码码流超
		//过该值时， DSP 会自动调整编码参数来保证不超过最大比特率，当编码码流低于最大比特率时， DSP 不进
		//	行干涉。调整误差<10%。
		SetupBitrateControl(struEncodeChan[ChannelNum].hChanHandle, 20000000);

		//设置视频参数
		//参 数： 
		//	HANDLE hChannelHandle；通道句柄
		//	int Brightness；亮度值(0-255)
		//	int Contrast；对比度(0-127)
		//	int Saturation；饱和度(0-127)
		//	int Hue；色调(0-255)
		SetVideoPara(struEncodeChan[ChannelNum].hChanHandle, 128, 64, 64, 0);
		//参 数：
		//	hChannelHandle ； 通道句柄
		//	sharpness； 锐化参数， 取值范围[0,3]，值越大锐度越大。
		SetEncoderVideoSharpness(struEncodeChan[ChannelNum].hChanHandle, 1);
		//说 明： 设置视频信号检测的灵敏度。如果视频信号的强度比较弱，或者信号通断的切换比较频繁，会
		//	出现“无视频信号”的提示字样，为了避免提示字样影响图像，可以更改视频信号检测的灵敏度。灵敏度
		//	取值越大，检测精度越低，出现“无视频信号”提示字样的频率越低。当将 value 值设置为 0xffffffff 时，
		//	将不会再出现“无视频信号”的提示字样。
		SetVideoDetectPrecision(struEncodeChan[ChannelNum].hChanHandle, 20);
		//说 明： 设置视频信号的输入位置。 (x， y)为系统处理图像的左上角在摄像机输入的原始图像中的坐标，
		//	某些摄像机输入的图像在预览时可能在左边会有黑边，可以通过此函数进行调节， x 必须设置为 2 的整数
		//	倍。 (x， y)的取值和摄像机的型号有关，如果指定的值和摄像机的输入参数不匹配，可能会导致图像静止、
		//	水平垂直方向滚动或者黑屏，请谨慎使用。
		SetInputVideoPosition(struEncodeChan[ChannelNum].hChanHandle, 8, 2);
		//设置是否采用反隔行算法以及采用反隔行时的强度
		//参 数： 
		//	HANDLE hChannelHandle；通道句柄
		//	UINT mode； 0 表示该通道不进行反隔行变换,此时 level 参数无效； 1 表示使用旧的算法； 2 表示使用
		//	默认算法(系统默认值)。 DS-52xx 系列设备、 DS-42xx 系列板卡不支持 mode=1
		//	UINT level；当 mode＝1 时有效，其它时无效。 0－10，反隔行变换的强度逐渐加强， 0 最弱，对图像
		//	的损失最小， 10 最强，对图像的损失最大。
		SetDeInterlace(struEncodeChan[ChannelNum].hChanHandle, 1, 1);
	}
	

	return 1;

}

int Class_HKcard_4008::startDecEncData(int ChannelNum, void (CALLBACK* DecCBFun)(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2))
{
	//for (int i=0;i<ChannelNum;i++)
	{
		StartVideoCapture(struEncodeChan[ChannelNum].hChanHandle);
	}
	
	Sleep(500);
	///////-----------------decode---------------------------------------------------
	DWORD nLength  =  PlayM4_GetFileHeadLength();
	PBYTE pFileHead  =  new BYTE[nLength];

	//VideoFile.setBkpFileSize(50*1024*1024);
	//AudioFile.setBkpFileSize(50*1024*1024);
	//m_pHFile = fopen(VFileName,"wb+");
	//A_pHFile = fopen(AFileName,"wb+");
	//for (int i=0;i<ChannelNum;i++)
	{
	
		if (m_lPort[ChannelNum] == -1)
		{
			PlayM4_GetPort(&m_lPort[ChannelNum]);
		}
		PlayM4_SetDecCBStream(m_lPort[ChannelNum],3);
		PlayM4_SetDecodeFrameType(m_lPort[ChannelNum],0);
		PlayM4_SyncToAudio(m_lPort[ChannelNum],TRUE);

		memcpy(pFileHead, struEncodeChan[ChannelNum].szFileHeader, nLength);

		PlayM4_SetStreamOpenMode(m_lPort[ChannelNum],STREAME_REALTIME);

		if (!PlayM4_OpenStream(m_lPort[ChannelNum],pFileHead, nLength,1024*1000))
		{
			//m_strPlayFileName="";
			//printf("文件打不开");
			return -1;
		}

		PlayM4_SetDecCallBack(m_lPort[ChannelNum],DecCBFun);
		//PlayM4_SetAudioCallBack(m_lPort,WaveCBFun, 0);
		if(!PlayM4_Play(m_lPort[ChannelNum],NULL))
		{
			//printf("start decode...\n");
			return -2;
		}
		Sleep(100);
		if(!PlayM4_PlaySound(m_lPort[ChannelNum]))
		{
			//MessageBoxW();
			return -3;
		}
	}

	return 1;
}

int Class_HKcard_4008::stop(int ChannelNum)
{
	//for( int i = 0; i < ChannelNum; i++)
	{
		// 停止解码
		PlayM4_Stop(m_lPort[ChannelNum]);
		//停止播放声音
		PlayM4_StopSound();

		//关闭流，回收源数据缓冲
		PlayM4_CloseStream(m_lPort[ChannelNum]);

		//释放播放库端口号
		PlayM4_FreePort(m_lPort[ChannelNum]);
		m_lPort[ChannelNum] = -1;
	
	
		StopVideoCapture(struEncodeChan[ChannelNum].hChanHandle); 
		struEncodeChan[ChannelNum].bGethead = FALSE; 
		struEncodeChan[ChannelNum].uChlength =0;
		StopSubVideoCapture(struEncodeChan[ChannelNum].hChanHandle); 
		ChannelClose(struEncodeChan[ChannelNum].hChanHandle);
		if (g_pSettingValue[ChannelNum]->bMotionDetect)
		{
			StopMotionDetection(struEncodeChan[ChannelNum].hChanHandle); 
		}
	}
	return 1;
}
