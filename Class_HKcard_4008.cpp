#include "StdAfx.h"
#include "Class_HKcard_4008.h"

STRU_ENCODE_CHAN struEncodeChan[MAX_CHANNELS];				//����ͨ����ز���
STRU_DECODE_CHAN struDecodeChan[MAX_CHANNELS];				//����ͨ����ز���
PSETTING_VALUE g_pSettingValue[MAX_CHANNELS];						//ͨ�����ò���
PSUBSETTING_VALUE g_pSubSettingValue[MAX_CHANNELS]; 
FRAMES_STATISTICS   frame_statistics; 

int			m_iFileHeaderLen;
LONG	m_lPort[MAX_PORTS] = {-1,-1,-1,-1,-1,-1,-1,-1};

Class_HKcard_4008::Class_HKcard_4008(void)
{
	g_nTotalDspCnt = 0;
	g_nTotalEncChannels = 0;			 //����ͨ����
	g_nTotalDecChannels = 0;			 //����ͨ����
	g_nTotalDispChannels = 0;			 //��ʾͨ����
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
		g_pSettingValue[i]->iIQuant = 15;			//I ֡����ϵ����ȡֵ��Χ�� 12-30
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
		g_pSettingValue[i]->csOsdFormat2 = "¼��ͨ��"; 

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
	
		//������ͨ������ֱ��ʡ�֧�ֶ�̬�޸ġ�
		SetEncoderPictureFormat(struEncodeChan[ChannelNum].hChanHandle, encodeType); 
	
		//������ͨ�����������͡��˺���������������ǰ�������á�
		SetStreamType(struEncodeChan[ChannelNum].hChanHandle, streamType); 
		//����ͼ������ϵ�������ڵ���ͼ������������ϵ��ԽСͼ������Խ�ߡ�ϵͳĬ������ϵ��ֵΪ18�� 18�� 23��
		//�� �壺 ����ϵ��
		//	����ϵ����ǿ��Ӱ�� MPEG �� H.264 ��׼�б���ͼ�����������ʵĲ�����������ϵ��Խ�ͣ�ͼ������
		//	�ͻ�Խ�ߣ� ����Ҳ��Խ�ߣ���֮��ͼ�������ͻ�Խ�ͣ�����Ҳ��Խ��
		//�� ���� 
		//	HANDLE hChannelHandle��ͨ�����
		//	int IQuantVal�� I ֡����ϵ����ȡֵ��Χ�� 12-30
		//	int PQuantVal�� P ֡����ϵ����ȡֵ��Χ�� 12-30(Ŀǰ��Ч)
		//	int BQuantVal�� B ֡����ϵ����ȡֵ��Χ�� 12-30(Ŀǰ��Ч)
		SetDefaultQuant(struEncodeChan[ChannelNum].hChanHandle, 12, 12, 12);
		//���ñ���֡�ṹ��֡�ʡ�֧�ֶ�̬�޸�
		//HANDLE hChannelHandle��ͨ�����
		//	int KeyFrameIntervals���ؼ�֡�����ȡֵ��Χ��-400��ϵͳĬ��Ϊ 100
		//	int BFrames�� B ֡������ȡֵΪ 0 ���� 2��ϵͳĬ��Ϊ 2
		//	int PFrames�� P ֡������Ŀǰ��ȡֵ��Ч
		//	int FrameRate��֡�ʣ�֡�ʷ�Χ 1-25(PAL)�� 1-30(NTSC)
		SetIBPMode(struEncodeChan[ChannelNum].hChanHandle, 250, 2, 1, 25);
		//˵ ���� ���ñ����������Ʒ�ʽ����� SetupBitrateControl ʹ�á�������Ϊ������(brVBR)ʱ��������
		//	�ʽ���Ϊ�����������ޣ��� DSP �������������Զ��������ʣ�һ����Զ����䵽��͵�״̬(���趨��ͼ
		//	�����������͹ؼ�֡�������)�������̶ȵؽ��ʹ���ʹ洢�ռ䣬���洢����һ�����Թ��㣻������Ϊ
		//	������(brCBR)ʱ��������������Ϊ�������ʲ����㶨��������������Զ����䵽������״̬���洢����
		//	�ɸ����趨���ʵĴ�С���й��㡣
		SetBitrateControlMode(struEncodeChan[ChannelNum].hChanHandle, brVBR);
		//˵ ���� ���ñ�����������ʡ�����Ϊ 0 ʱ����������Ч������Ϊĳһ��������ʱ��������������
		//����ֵʱ�� DSP ���Զ����������������֤�������������ʣ�����������������������ʱ�� DSP ����
		//	�и��档�������<10%��
		SetupBitrateControl(struEncodeChan[ChannelNum].hChanHandle, 20000000);

		//������Ƶ����
		//�� ���� 
		//	HANDLE hChannelHandle��ͨ�����
		//	int Brightness������ֵ(0-255)
		//	int Contrast���Աȶ�(0-127)
		//	int Saturation�����Ͷ�(0-127)
		//	int Hue��ɫ��(0-255)
		SetVideoPara(struEncodeChan[ChannelNum].hChanHandle, 128, 64, 64, 0);
		//�� ����
		//	hChannelHandle �� ͨ�����
		//	sharpness�� �񻯲����� ȡֵ��Χ[0,3]��ֵԽ�����Խ��
		SetEncoderVideoSharpness(struEncodeChan[ChannelNum].hChanHandle, 1);
		//˵ ���� ������Ƶ�źż��������ȡ������Ƶ�źŵ�ǿ�ȱȽ����������ź�ͨ�ϵ��л��Ƚ�Ƶ������
		//	���֡�����Ƶ�źš�����ʾ������Ϊ�˱�����ʾ����Ӱ��ͼ�񣬿��Ը�����Ƶ�źż��������ȡ�������
		//	ȡֵԽ�󣬼�⾫��Խ�ͣ����֡�����Ƶ�źš���ʾ������Ƶ��Խ�͡����� value ֵ����Ϊ 0xffffffff ʱ��
		//	�������ٳ��֡�����Ƶ�źš�����ʾ������
		SetVideoDetectPrecision(struEncodeChan[ChannelNum].hChanHandle, 20);
		//˵ ���� ������Ƶ�źŵ�����λ�á� (x�� y)Ϊϵͳ����ͼ������Ͻ�������������ԭʼͼ���е����꣬
		//	ĳЩ����������ͼ����Ԥ��ʱ��������߻��кڱߣ�����ͨ���˺������е��ڣ� x ��������Ϊ 2 ������
		//	���� (x�� y)��ȡֵ����������ͺ��йأ����ָ����ֵ������������������ƥ�䣬���ܻᵼ��ͼ��ֹ��
		//	ˮƽ��ֱ����������ߺ����������ʹ�á�
		SetInputVideoPosition(struEncodeChan[ChannelNum].hChanHandle, 8, 2);
		//�����Ƿ���÷������㷨�Լ����÷�����ʱ��ǿ��
		//�� ���� 
		//	HANDLE hChannelHandle��ͨ�����
		//	UINT mode�� 0 ��ʾ��ͨ�������з����б任,��ʱ level ������Ч�� 1 ��ʾʹ�þɵ��㷨�� 2 ��ʾʹ��
		//	Ĭ���㷨(ϵͳĬ��ֵ)�� DS-52xx ϵ���豸�� DS-42xx ϵ�а忨��֧�� mode=1
		//	UINT level���� mode��1 ʱ��Ч������ʱ��Ч�� 0��10�������б任��ǿ���𽥼�ǿ�� 0 ��������ͼ��
		//	����ʧ��С�� 10 ��ǿ����ͼ�����ʧ���
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
			//printf("�ļ��򲻿�");
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
		// ֹͣ����
		PlayM4_Stop(m_lPort[ChannelNum]);
		//ֹͣ��������
		PlayM4_StopSound();

		//�ر���������Դ���ݻ���
		PlayM4_CloseStream(m_lPort[ChannelNum]);

		//�ͷŲ��ſ�˿ں�
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
