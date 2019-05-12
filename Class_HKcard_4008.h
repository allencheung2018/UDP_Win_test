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
	VideoStandard_t chstandard;     //通道制式
	BOOL	 bVideoCap; 			//编码主通道录像状态
	BOOL	 bVideoSubCap; 			//编码子通道录像状态
	int	     iPreFramerate; 		//编码预览显示帧率
	BOOL	 bChanPreset;					 //解码通道预览状态（treeview中选中则为true）
	HANDLE	 hChanHandle;			//编码通道句柄
	UINT	 iFileSize; 			//主通道录像文件大小
	UINT	 iSubFileSize; 			//子通道录像文件大小
	BOOL	 bVideoPre; 			//编码/解码通道预览状态
	FILE *	 hFileHandle; 			//主通道录像文件句柄
	FILE *	 hSubFileHandle; 		//子通道录像文件句柄
	BOOL     bSwitchFile; 			//主通道换文件标志
	BOOL	 bSubSwitchFile;		//子通道切换文件标志
	UINT	 uChlength; 					 //主通道当前录像文件长度
	UINT	 uSubChlength;                   //子通道当前录像文件长度
	BOOL	 bGethead;						 //是否获取录像文件头
	BOOL	 bGetSubhead;					 //是否获取子通道录像文件头
	unsigned char szFileHeader[100]; 	 //主通道文件头
	unsigned char szSubFileHeader[100];  //子通道文件头
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

//解码通道相关参数结构体
typedef struct STRU_DECODE_CHAN 
{
	char		 *csFilePath; 						     //解码文件（路径）
	DISPLAY_PARA dsPara;							 //视频显示参数
	BOOL		 bDecPlay; 							 //是否在解码播放
	FILE*		 decFile; 							 //解码文件
	BOOL		 bOpenRealStream[MAX_CHANNELS];		//解码通道是否正在解码编码通道的实时码流,下标表示编码通道号
	BOOL		 bOpenStreamSuccessed; 
	BOOL		 bYUVCapSel; 					 //记录原始图像录像
	ULONGLONG	 lMaxCapFileSize; 				 //单个文件最大容量
	BOOL		 bChannelPreset;					 //解码通道预览状态（treeview中选中则为true）
	HANDLE		 hChannelHandle;					 //解码通道句柄
	FILE		 *pYUVFile; 				 //每个通道当前解码文件
	ULONGLONG	 lYUVSize; 						 //解码录像文件大小
	UINT		 uDecTimerId;			//解码时钟ID
	BOOL		 bFirstCapFile;				//抓取送入SDK进行解码的码流数据并保存成文件时，标示是否是第一个文件
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
	int     nStreamPackType;   //码流打包方式
	int     nAudioEncodeType;  //音频编码方式
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

extern STRU_ENCODE_CHAN struEncodeChan[MAX_CHANNELS];				//编码通道相关参数
extern STRU_DECODE_CHAN struDecodeChan[MAX_CHANNELS];				//解码通道相关参数
extern PSETTING_VALUE g_pSettingValue[MAX_CHANNELS];						//通道设置参数
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
	UINT		g_nTotalEncChannels;			 //编码通道数
	UINT		g_nTotalDecChannels;			 //解码通道数
	UINT		g_nTotalDispChannels;			 //显示通道数
	
	UINT		g_nAudioOutPort[MAX_CHANNELS*2];			 //音频输出
	int			g_nEncUseTimes[MAX_CHANNELS] ;				 //矩阵输出某编码通道输出次数
	int			g_nDecUseTimes[MAX_CHANNELS] ;				 //矩阵输出某解码通道输出次数
	int			g_iDispArea[MAX_DISP_CHAN_CNT] ;			 //当前通道的视频矩阵非矩阵输出区域
	DWORD		g_DisChoice[MAX_DISP_CHAN_CNT][16] ;		 //输出通道各个区域对应的解码或编码通道
	DWORD		g_LastDisChoice[MAX_DISP_CHAN_CNT][16] ;	 //输出通道各个区域上次对应的解码或编码通道
	int			g_ZeroOrOne[MAX_DISP_CHAN_CNT][16] ;		 //视频输出参数，表示是否一个通道二次输出，0表示一次，1表示第二次输出
};

