#pragma once

#include <windows.h>
#include <stdio.h>
#include <string>

using namespace std;
#include "HikVisionSdk.h"
#include "WindowsPlayM4.h"

#define MAX_CHANNELS 100
#define MAX_DISP_CHAN_CNT	128
#define VIDEO_WIDTH	704
#define VIDEO_HEIGHT	576
#define MAX_VEDIOLENGTH		int(VIDEO_WIDTH*VIDEO_HEIGHT*1.5*5)
#define MAX_AUDIOLENGTH		10*1024
#define MAX_VEDIOFRAME_ENC	1
#define MAX_AUDIOFRAME_ENC	4
#define MAX_PORTS	8


typedef struct STRU_ENCODE_CHAN
{
	VideoStandard_t chstandard;     //ͨ����ʽ
	BOOL	 bVideoCap; 			//������ͨ��¼��״̬
	BOOL	 bVideoSubCap; 			//������ͨ��¼��״̬
	int	     iPreFramerate; 		//����Ԥ����ʾ֡��
	BOOL	 bChanPreset;					 //����ͨ��Ԥ��״̬��treeview��ѡ����Ϊtrue��
	HANDLE	 hChanHandle;			//����ͨ�����
	UINT	 iFileSize; 			//��ͨ��¼���ļ���С
	UINT	 iSubFileSize; 			//��ͨ��¼���ļ���С
	BOOL	 bVideoPre; 			//����/����ͨ��Ԥ��״̬
	FILE *	 hFileHandle; 			//��ͨ��¼���ļ����
	FILE *	 hSubFileHandle; 		//��ͨ��¼���ļ����
	BOOL     bSwitchFile; 			//��ͨ�����ļ���־
	BOOL	 bSubSwitchFile;		//��ͨ���л��ļ���־
	UINT	 uChlength; 					 //��ͨ����ǰ¼���ļ�����
	UINT	 uSubChlength;                   //��ͨ����ǰ¼���ļ�����
	BOOL	 bGethead;						 //�Ƿ��ȡ¼���ļ�ͷ
	BOOL	 bGetSubhead;					 //�Ƿ��ȡ��ͨ��¼���ļ�ͷ
	unsigned char szFileHeader[100]; 	 //��ͨ���ļ�ͷ
	unsigned char szSubFileHeader[100];  //��ͨ���ļ�ͷ
	STRU_ENCODE_CHAN()
	{
		iPreFramerate = 25;
		iFileSize = 50;
		iSubFileSize = 10;
		bVideoPre = TRUE;
		hFileHandle = NULL;
		hSubFileHandle = NULL;
		bSwitchFile = FALSE;
		bSubSwitchFile = FALSE;
		uChlength = 0;
		uSubChlength = 0;
		bGethead = FALSE;
		bGetSubhead = FALSE;
	}
}ENCODE_CHAN;

//����ͨ����ز����ṹ��
typedef struct STRU_DECODE_CHAN 
{
	char		 *csFilePath; 						     //�����ļ���·����
	DISPLAY_PARA dsPara;							 //��Ƶ��ʾ����
	BOOL		 bDecPlay; 							 //�Ƿ��ڽ��벥��
	FILE*		 decFile; 							 //�����ļ�
	BOOL		 bOpenRealStream[MAX_CHANNELS];		//����ͨ���Ƿ����ڽ������ͨ����ʵʱ����,�±��ʾ����ͨ����
	BOOL		 bOpenStreamSuccessed; 
	BOOL		 bYUVCapSel; 					 //��¼ԭʼͼ��¼��
	ULONGLONG	 lMaxCapFileSize; 				 //�����ļ��������
	BOOL		 bChannelPreset;					 //����ͨ��Ԥ��״̬��treeview��ѡ����Ϊtrue��
	HANDLE		 hChannelHandle;					 //����ͨ�����
	FILE		 *pYUVFile; 				 //ÿ��ͨ����ǰ�����ļ�
	ULONGLONG	 lYUVSize; 						 //����¼���ļ���С
	UINT		 uDecTimerId;			//����ʱ��ID
	BOOL		 bFirstCapFile;				//ץȡ����SDK���н�����������ݲ�������ļ�ʱ����ʾ�Ƿ��ǵ�һ���ļ�
	STRU_DECODE_CHAN()
	{
		bYUVCapSel = 0; 
		bDecPlay = FALSE;
		bFirstCapFile = FALSE;
	}
}DECODE_CHAN;

typedef struct{
	int		iEncodeType; 
	int		iStreamType; 
	BOOL	bSubSetting; 
	int		iBFrames; 
	int		iFileSize; 
	int		iFramerate; 
	int		iIFramesInterval; 
	int		iIQuant; 
	BOOL	bCBR; 
	int		iPQuant; 
	int		iQlimitMax; 
	int		iIQlimitMin; 
	int		iMaxBps; 
	BOOL	bIQLimit; 
	int		iPFrames; 
	int		iBQuant; 
	int     iEFrames; 

	BOOL	bLogoEnable; 
	BOOL	bLogoTranslucent; 
	BOOL	bLogoTwink; 
	int		iLogoB; 
	int		iLogoG; 
	int		iLogoHide; 
	int		iLogoR; 
	int		iLogoShow; 
	int		iLogoX; 
	int		iLogoY; 
	BOOL	bOsdTranslucent; 
	BOOL	bOsdEnable; 
	int		iOsdBrightness; 
	int		iOsdRollback; 
	int		iOsdX1; 
	int		iOsdX2; 
	int		iOsdY1; 
	int		iOsdY2; 
	string	csOsdFormat2; 

	BOOL	bMask; 
	BOOL    bMaskColor; 
	int		iMaskAreaNum; 
	int		iMaskAreaNumColor; 
	int		iMaskX; 
	int		iMaskY; 
	int		iMaskSizeX; 
	int		iMaskSizeY; 
	int		iMaskXColor; 
	int		iMaskYColor; 
	int		iMaskSizeXColor; 
	int		iMaskSizeYColor; 
	int     iMaskR; 
	int		iMaskG; 
	int		iMaskB; 

	BOOL	bDeInterLace; 
	int		iDeInterlaceLevel; 
	int		iPreDetectPrecision; 
	int		iPreX; 
	int		iPreY; 

	BOOL	bMotionDetect; 
	BOOL	bPrivateDate; 
	BOOL	bSelfAdapt; 
	BOOL	bMDCap; 
	int		iFastMD; 
	int		iMDGrade; 
	int		iSlowMD; 
	int		iThreshold; 
	int		iDelayMD; 
#ifdef Ver_6_1
	BOOL	bDeNoise;
	UINT	iDeNoiseLevel;

	BOOL    bSetColorMode;
	UINT	iViColorMode;

	int     nStreamPackType;
	int     nAudioEncodeType;
#endif
}SETTING_VALUE, *PSETTING_VALUE; 

typedef struct{
	int		iEncodeType; 
	int		iStreamType; 
	BOOL	bSubSetting; 
	int		iBFrames; 
	int		iFileSize; 
	int		iFramerate; 
	int		iIFramesInterval; 
	int		iIQuant; 
	BOOL	bCBR; 
	int		iPQuant; 
	int		iQlimitMax; 
	int		iIQlimitMin; 
	int		iMaxBps; 
	BOOL	bIQLimit; 
	int		iPFrames; 
	int		iBQuant; 
	int     iEFrames; 
#ifdef Ver_6_1
	int     nStreamPackType;   //���������ʽ
	int     nAudioEncodeType;  //��Ƶ���뷽ʽ
#endif

}SUBSETTING_VALUE, *PSUBSETTING_VALUE; 

struct VideoDataStru
{
	bool bReady;
	unsigned char indata[MAX_VEDIOLENGTH];
	unsigned char outdata[100000];
	int inlen;
	int outlen;
};

struct AudioDataStru
{
	bool bReady;
	unsigned char indata[MAX_AUDIOLENGTH];
	unsigned char outdata[10000];
	int inlen;
	int outlen;
};

extern STRU_ENCODE_CHAN struEncodeChan[MAX_CHANNELS];				//����ͨ����ز���
extern STRU_DECODE_CHAN struDecodeChan[MAX_CHANNELS];				//����ͨ����ز���
extern PSETTING_VALUE g_pSettingValue[MAX_CHANNELS];						//ͨ�����ò���
extern PSUBSETTING_VALUE g_pSubSettingValue[MAX_CHANNELS]; 
extern FRAMES_STATISTICS   frame_statistics; 

extern int			m_iFileHeaderLen;
extern LONG	m_lPort[MAX_PORTS] ;

class Class_HKcard_4008
{
public:
	Class_HKcard_4008(void);
	~Class_HKcard_4008(void);

	int initial();
	void InitParameter();
	int openChannel(int ChannelNum, STREAM_DIRECT_READ_CALLBACK StreamDirectReadCallback, PictureFormat_t encodeType);
	int startDecEncData(int ChannelNum, void (CALLBACK* DecCBFun)(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2));
	int stop(int ChannelNum);

private:
	void *context;
	
	VideoStandard_t SDvs;
	UINT		g_nTotalDspCnt;
	UINT		g_nTotalEncChannels;			 //����ͨ����
	UINT		g_nTotalDecChannels;			 //����ͨ����
	UINT		g_nTotalDispChannels;			 //��ʾͨ����
	
	UINT		g_nAudioOutPort[MAX_CHANNELS*2];			 //��Ƶ���
	int			g_nEncUseTimes[MAX_CHANNELS] ;				 //�������ĳ����ͨ���������
	int			g_nDecUseTimes[MAX_CHANNELS] ;				 //�������ĳ����ͨ���������
	int			g_iDispArea[MAX_DISP_CHAN_CNT] ;			 //��ǰͨ������Ƶ����Ǿ����������
	DWORD		g_DisChoice[MAX_DISP_CHAN_CNT][16] ;		 //���ͨ�����������Ӧ�Ľ�������ͨ��
	DWORD		g_LastDisChoice[MAX_DISP_CHAN_CNT][16] ;	 //���ͨ�����������ϴζ�Ӧ�Ľ�������ͨ��
	int			g_ZeroOrOne[MAX_DISP_CHAN_CNT][16] ;		 //��Ƶ�����������ʾ�Ƿ�һ��ͨ�����������0��ʾһ�Σ�1��ʾ�ڶ������
};

