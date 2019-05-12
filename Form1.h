extern "C"
{
#include "sdl/SDL.h"
#include "libavutil/time.h"
};

#include "UDPWin.h"
#include "PlayVideoDataProtocol.h"
#include "YUVEncoder.h"
#include "ACCEncoder.h"
#include "MulticastUDP.h"
#include "Class_HKcard_4008.h"
#include "H264DecodeYUV.h"
#include "Mp3Decoder.h"
#include "Osdchar.h"

#include "NoticePanelform.h"

#include <cstdlib>
#include <string>
#include <fstream>
#include <string.h>
#include <msclr/marshal.h> 
#include <windows.h>
#include <winuser.h>

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define BREAK_EVENT  (SDL_USEREVENT + 2)

#define QUEUE_SIZE_AUDIO 10
#define QUEUE_SIZE_VIDEO 15

#pragma once

typedef struct RawData {
	uint8_t data[5120];
	Uint32 len;
}RawData;

typedef struct RawDataVideo {
	uint8_t data[608256];
	Uint32 len;
}RawDataVideo;

typedef struct MediaQueue{
	RawData dataq[QUEUE_SIZE_AUDIO];
	int pictq_size;
	int pictq_rindex;
	int pictq_windex;
	SDL_mutex *pictq_mutex;
	SDL_cond *pictq_cond;
}MediaQueue;

typedef struct MediaQueueVideo{
	RawDataVideo dataq[QUEUE_SIZE_VIDEO];
	int pictq_size;
	int pictq_rindex;
	int pictq_windex;
	SDL_mutex *pictq_mutex;
	SDL_cond *pictq_cond;
}MediaQueueVideo;

struct  AudioDataStru ADataEnc[MAX_PORTS];
struct VideoDataStru VDataEnc[MAX_PORTS];
unsigned char ucATemp[MAX_AUDIOLENGTH] = {0};
unsigned char ucVTemp[MAX_VEDIOLENGTH] = {0};
char VFileName[] = {"HK_cn0.h264"};
char VFileName_1[] = {"HK_cn1.h264"};
char VFileName_org[] = {"HK_cn1_org.h264"};
char AFileName[] = {"HK_cn0.aac"};
char AFileName_1[] = {"HK_cn1.pcm"};
FILE *V_pHFile, *V_pHFile_1, *V_pHFile_org;
FILE *A_pHFile, *A_pHFile_1;
bool bFlagAVData[MAX_PORTS] = {false};
int Vframecnt = 0;
int Aframecnt = 0;	
char cLocalIP[20] = {"192.168.1.89"};
char videofile[200] = {0};
char cScrlText[] = "比使用“易木雨软件工作室”ts3，生成点阵字库快5-6倍的点阵字库生成器；ts3还要收费，本软件是免费的。主要功能有：创建全汉字库、创建短汉字库、显示汉字库、显示汉字库和ASCII字库；可以生成12x12、16x16、24x24、32x32、40x40、60x60的点阵字库，同时可以结合各种字体，生成各字库。本人找了很多款点阵生成的工具，也花了些时间，感觉这款用得还不错，所以分享给有需要的网友，希望对您有所帮助。";
bool isShowNotice = false;
uint8_t  outRGB[VIDEO_WIDTH*VIDEO_HEIGHT*3] = {0};
bool bPrewAudio = false;
static  Uint32  audio_len_ch0, audio_len_ch1; 
static  Uint8  *audio_pos_ch0, *audio_pos_ch1;
MediaQueue *isAudio;
MediaQueueVideo *isVideo;

DWORD WINAPI ThreadFunc(LPVOID);
DWORD WINAPI ThreadFunc2(LPVOID);
DWORD WINAPI ThreadReadVideoFile(LPVOID);
DWORD WINAPI ThreadPrewVideo1(LPVOID );
DWORD WINAPI ThreadPrewAudio1(LPVOID );
int __cdecl StreamDirectReadCallback(ULONG channelNum, void *DataBuf, DWORD Length, int frameType, void *context);
void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2);
void CALLBACK DecCBFun_1(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2);
void  fill_audio_ch1(void *udata,Uint8 *stream,int len);
char initScrollText(char *cstxt, int x, int y);

int sendAudioData(MulticastUDP *AudioMUDP, uint8_t *data, int datalen)
{
	unsigned char ucSendData[PVP_LENGTH]={0};
	int PCMlen = datalen;
	char	AVData[SEND_LENGTH] ={0};
	int alen=0;
	int a_pnum = PCMlen/SEND_LENGTH;
	int a_rmd = PCMlen%SEND_LENGTH;
	memcpy(ucSendData,PVP_HEADER,4);
	ucSendData[4] = PCMlen&0xff;
	ucSendData[5] = (PCMlen>>8)&0xff;
	ucSendData[6] = (PCMlen>>16)&0xff;
	ucSendData[7] = (PCMlen>>24)&0xff;
	ucSendData[8] = (a_pnum+1)&0xff;
	ucSendData[12] = 0x03;		//Command 
	ucSendData[13] = 0x33;		//Command ID
	for (int i=0;i<(a_pnum+1);i++)
	{
		if (i < a_pnum)
		{
			memcpy(AVData, data +i*SEND_LENGTH, SEND_LENGTH);
			alen = SEND_LENGTH;
		}
		if (i == a_pnum && a_rmd != 0)
		{
			memcpy(AVData, data +i*SEND_LENGTH, a_rmd);
			alen = a_rmd;
		}
		ucSendData[10] = (i+1)&0xff;
		ucSendData[22] = (alen)&0xff;
		ucSendData[23] = (alen>>8)&0xff;
		ucSendData[24] = (alen>>16)&0xff;
		ucSendData[25] = (alen>>24)&0xff;
		memcpy(ucSendData+30, AVData, alen);
		int pktLen = alen + 6 +16 ;
		memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
		AudioMUDP->sendData(ucSendData,pktLen+12);
	}

	return 1;
}

int sendVideoData(MulticastUDP *VideoMUDP, uint8_t *data, int datalen)
{
	unsigned char ucSendData[PVP_LENGTH]={0};
	char	AVData[SEND_LENGTH] ={0};
	int actDataLen = 0, vlen=0;
	int v_pnum = datalen/SEND_LENGTH;
	int v_rmd = datalen%SEND_LENGTH;
	memcpy(ucSendData,PVP_HEADER,4);
	ucSendData[4] = datalen&0xff;
	ucSendData[5] = (datalen>>8)&0xff;
	ucSendData[6] = (datalen>>16)&0xff;
	ucSendData[7] = (datalen>>24)&0xff;
	ucSendData[8] = (v_pnum+1)&0xff;
	ucSendData[12] = 0x03;		//Command 
	ucSendData[13] = 0x33;		//Command ID
	for (int i=0;i<(v_pnum+1);i++)
	{
		if (i < v_pnum)
		{
			memcpy(AVData,data +i*SEND_LENGTH,SEND_LENGTH);
			vlen = SEND_LENGTH;
		}
		if (i == v_pnum && v_rmd != 0)
		{
			memcpy(AVData,data +i*SEND_LENGTH,v_rmd);
			vlen = v_rmd;
		}
		ucSendData[10] = (i+1)&0xff;
		ucSendData[14] = vlen&0xff;
		ucSendData[15] = (vlen>>8)&0xff;
		memcpy(ucSendData+30, AVData, vlen);
		int pktLen = vlen + 6 +16 ;
		memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
		VideoMUDP->sendData(ucSendData,pktLen+12);
	}
	return 1;
}

DWORD WINAPI ThreadFunc(LPVOID p)
{
	bool *bflag = (bool *)p;
	int iChNum = 0;		
	int in_w=VIDEO_WIDTH,in_h=VIDEO_HEIGHT;
	int y_size,h264_size, ACC_size,sendnum=0, actDataLen, pktLen, ret;
	int v_pnum, v_rmd, a_pnum, a_rmd, max_pnum;
	unsigned char data_aout[10000]={0}, data_vout[100000]={0};
	unsigned char ucSendData[PVP_LENGTH]={0};
	YUVEncoder CVideo(in_w,in_h);
	ACCEncoder CAudio;
	MulticastUDP MUtest;
	Class_HKcard_4008 test1;
	H264DecodeYUV testH264Dec(VIDEO_WIDTH, VIDEO_HEIGHT);
	
	CAudio.initializeCoder();
	y_size = in_w * in_h;
	CVideo.initializeCoder();
	V_pHFile = fopen(VFileName,"wb");
	V_pHFile_org = fopen(VFileName_org,"wb");
	A_pHFile = fopen(AFileName,"wb");
	MUtest.InitialSock(cLocalIP,"224.0.1.2",7838);		//CH0
	testH264Dec.initialDecoder();
	if(test1.initial() > 0)
		printf("Initialize Card successful!\n");
	else
	{
		MessageBoxW(NULL, L"请检查视频卡！", L"提示",MB_ICONERROR);
		//MessageBox(NULL, "请检查视频卡！", "提示",MB_ICONERROR);
		return -1;
	}
	test1.openChannel(iChNum,StreamDirectReadCallback, ENC_4CIF_FORMAT);
	ret = test1.startDecEncData(iChNum,DecCBFun);
	if (ret < 0)
	{
		wchar_t s[100];
		swprintf_s(s, 25,L"错误代码：%d\n", ret);
		//System::String ^ss = gcnew System::String(s);
		MessageBoxW(NULL, (LPCWSTR)( s), (LPCWSTR)L"编解码PlayM4错误！ ",MB_ICONERROR);
		return -2;
	}
	
	*bflag = true;
	while(*bflag)
	{
		if (ADataEnc[iChNum].bReady)
		{
			for (int i=0;i<MAX_AUDIOFRAME_ENC;i++)
			{
				ACC_size = CAudio.encodeFrame(data_aout, ADataEnc[iChNum].indata + i*CAudio.buffer_size);
				if (ACC_size > 0)
				{
					//fwrite(data_aout,ACC_size,1,A_pHFile);
					memcpy(ADataEnc[iChNum].outdata + ADataEnc[iChNum].outlen, data_aout, ACC_size);
					ADataEnc[iChNum].outlen += ACC_size;
					if (ADataEnc[iChNum].outlen > 0)
					{
						a_pnum = ADataEnc[iChNum].outlen/SEND_LENGTH;
						a_rmd = ADataEnc[iChNum].outlen%SEND_LENGTH;
						for (int i=0;i<(a_pnum+1);i++)
						{
							int actDataLen = 0, alen=0;
							char	AVData[SEND_LENGTH] ={0};
							if (i < a_pnum)
							{
								memcpy(AVData,ADataEnc[iChNum].outdata +i*SEND_LENGTH,SEND_LENGTH);
								actDataLen += SEND_LENGTH;
								alen = SEND_LENGTH;
							}
							if (i == a_pnum && a_rmd != 0)
							{
								memcpy(AVData,ADataEnc[iChNum].outdata +i*SEND_LENGTH,a_rmd);
								actDataLen += a_rmd;
								alen = a_rmd;
							}
							memcpy(ucSendData,PVP_HEADER,4);
							pktLen = actDataLen + 6 +16 ;
							ucSendData[4] = pktLen&0xff;
							ucSendData[5] = (pktLen>>8)&0xff;
							ucSendData[6] = (pktLen>>16)&0xff;
							ucSendData[7] = (pktLen>>24)&0xff;
							ucSendData[8] = (a_pnum+1)&0xff;
							ucSendData[10] = (i+1)&0xff;
							ucSendData[12] = 0x03;		//Command 
							ucSendData[13] = 0x33;		//Command ID
							//ucSendData[14] = vlen&0xff;
							//ucSendData[15] = (vlen>>8)&0xff;
							//ucSendData[16] = (vlen>>16)&0xff;
							//ucSendData[17] = (vlen>>24)&0xff;
							ucSendData[22] = (alen)&0xff;
							ucSendData[23] = (alen>>8)&0xff;
							ucSendData[24] = (alen>>16)&0xff;
							ucSendData[25] = (alen>>24)&0xff;
							memcpy(ucSendData+30, AVData, actDataLen);
							memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
							MUtest.sendData(ucSendData,pktLen+12);
							memset(ucSendData,0,PVP_LENGTH);
						}
						fwrite(ADataEnc[iChNum].outdata,ADataEnc[iChNum].outlen,1,A_pHFile);
						ADataEnc[iChNum].outlen = 0;
						Aframecnt += MAX_AUDIOFRAME_ENC;
					}
				}
				ACC_size = 0;
			}
			ADataEnc[iChNum].bReady = false;
		}

		if (VDataEnc[iChNum].bReady)
		{
			VDataEnc[iChNum].outlen = 0;
			for (int i=0;i<MAX_VEDIOFRAME_ENC;i++)
			{
				int temp = 1.5*i*y_size;
				h264_size = CVideo.encodeFrameYV12(data_vout, VDataEnc[iChNum].indata + temp);
				if (h264_size >0)
				{
					//fwrite(data_vout,h264_size,1,V_pHFile);
					//fwrite(VDataEnc.indata, y_size*1.5, 1, V_pHFile);
					memcpy(VDataEnc[iChNum].outdata + VDataEnc[iChNum].outlen, data_vout, h264_size);
					VDataEnc[iChNum].outlen += h264_size;
				}
				h264_size = 0;
			}
			VDataEnc[iChNum].bReady = false;
		}
		if (VDataEnc[iChNum].outlen > 0)
		{
			v_pnum = VDataEnc[iChNum].outlen/SEND_LENGTH;
			v_rmd = VDataEnc[iChNum].outlen%SEND_LENGTH;
			for (int i=0;i<(v_pnum+1);i++)
			{
				int actDataLen = 0, vlen=0;
				char	AVData[SEND_LENGTH] ={0};
				if (i < v_pnum)
				{
					memcpy(AVData,VDataEnc[iChNum].outdata +i*SEND_LENGTH,SEND_LENGTH);
					actDataLen += SEND_LENGTH;
					vlen = SEND_LENGTH;
				}
				if (i == v_pnum && v_rmd != 0)
				{
					memcpy(AVData,VDataEnc[iChNum].outdata +i*SEND_LENGTH,v_rmd);
					actDataLen += v_rmd;
					vlen = v_rmd;
				}
				memcpy(ucSendData,PVP_HEADER,4);
				pktLen = actDataLen + 6 +16 ;
				ucSendData[4] = pktLen&0xff;
				ucSendData[5] = (pktLen>>8)&0xff;
				ucSendData[6] = (pktLen>>16)&0xff;
				ucSendData[7] = (pktLen>>24)&0xff;
				ucSendData[8] = (v_pnum+1)&0xff;
				ucSendData[10] = (i+1)&0xff;
				ucSendData[12] = 0x03;		//Command 
				ucSendData[13] = 0x33;		//Command ID
				ucSendData[14] = vlen&0xff;
				ucSendData[15] = (vlen>>8)&0xff;
				ucSendData[16] = (vlen>>16)&0xff;
				ucSendData[17] = (vlen>>24)&0xff;
				//ucSendData[22] = (alen)&0xff;
				//ucSendData[23] = (alen>>8)&0xff;
				//ucSendData[24] = (alen>>16)&0xff;
				//ucSendData[25] = (alen>>24)&0xff;
				memcpy(ucSendData+30, AVData, actDataLen);
				memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
				MUtest.sendData(ucSendData,pktLen+12);
				memset(ucSendData,0,PVP_LENGTH);
			}
			fwrite(VDataEnc[iChNum].outdata,VDataEnc[iChNum].outlen,1,V_pHFile);
			VDataEnc[iChNum].outlen =0;
			Vframecnt +=MAX_VEDIOFRAME_ENC;
		}
		//Sleep(100);
	}
/*	Vframecnt = 0;
	Aframecnt = 0;*/	
	test1.stop(iChNum);
	CAudio.closeCoder();
	CVideo.closeCoder();
	MUtest.closeSock();
	testH264Dec.closeDecoder();
	fclose(V_pHFile);
	fclose(V_pHFile_org);
	fclose(A_pHFile);
	//MessageBoxW(NULL, L"Thread Start!", L"提示",MB_OK);
	return 1;
}

int64_t videoclk, lastvideoclk, showclock = 0, sumvideoclk;
float avgvideoclk;
Uint64 videocnt;
//test
FILE *faudiolog, *fvideolog;

DWORD WINAPI ThreadFunc2(LPVOID p)
{
	bool *bflag = (bool *)p;
	int iChNum = 1;	
	int in_w=VIDEO_WIDTH,in_h=VIDEO_HEIGHT;
	int y_size,h264_size, ACC_size,sendnum=0, actDataLen, pktLen, ret;
	int v_pnum, v_rmd, a_pnum, a_rmd, max_pnum;
	unsigned char data_aout[10000]={0}, data_vout[100000]={0};
	unsigned char ucSendData[PVP_LENGTH]={0};
	YUVEncoder CVideo(in_w,in_h);
	ACCEncoder CAudio;
	MulticastUDP MUtest;
	Class_HKcard_4008 CHKtest;
	H264DecodeYUV testH264Dec(VIDEO_WIDTH, VIDEO_HEIGHT);
	uint8_t *datatemp = new uint8_t[5120];
	//test
	fvideolog = fopen("videolog.txt","w");
	faudiolog = fopen("audiolog.txt", "w");

	//字幕公告
	int noticeHeight = 535;
	ifstream outClientFile("Notice.txt", ios::in);
	outClientFile.seekg(0,std::ios::end);
	unsigned char *cNoteContent;
	int fsize = outClientFile.tellg();
	cNoteContent = new unsigned char[fsize+1];		//增加一个字节结尾，否则会出现乱码
	memset(cNoteContent,0,fsize+1);
	outClientFile.seekg(0, std::ios::beg);    // go back to the beginning 
	outClientFile.read((char *)cNoteContent, fsize);
	int temp = outClientFile.gcount();
	if(fsize > 0)
	{
		initScrollText((char *)cNoteContent, 704, noticeHeight);
	}
	else{
		MessageBoxW(NULL, L"公告内容为空！", L"提示",MB_ICONERROR);
		return -2;
	}

	CAudio.initializeCoder();
	y_size = in_w * in_h;
	CVideo.initializeCoder();
	V_pHFile_1 = fopen(VFileName_1,"wb+");
	V_pHFile_org = fopen(VFileName_org,"wb+");
	A_pHFile_1 = fopen(AFileName_1,"wb");
	MUtest.InitialSock(cLocalIP,"224.0.1.2",7838);		//CH1
	testH264Dec.initialDecoder();
	if(CHKtest.initial() > 0)
		printf("Initialize Card successful!\n");
	else
	{
		MessageBoxW(NULL, L"请检查视频卡！", L"提示",MB_ICONERROR);
		return -1;
	}
	CHKtest.openChannel(iChNum,StreamDirectReadCallback, ENC_4CIF_FORMAT);
	ret = CHKtest.startDecEncData(iChNum,DecCBFun_1);
	if (ret < 0)
	{
		wchar_t s[100];
		swprintf_s(s, 25,L"错误代码：%d\n", ret);
		//System::String ^ss = gcnew System::String(s);
		MessageBoxW(NULL, (LPCWSTR)( s), (LPCWSTR)L"编解码PlayM4错误！ ",MB_ICONERROR);
		return -2;
	}
	Aframecnt = 0;
	Vframecnt = 0;
	avgvideoclk = 0;
	videocnt = 0;
	sumvideoclk = 0;
	

		*bflag = true;
	while(*bflag)
	{
		if (ADataEnc[iChNum].bReady)
		{
			//for (int i=0;i<MAX_AUDIOFRAME_ENC;i++)
			{
				//ACC_size = CAudio.encodeFrame(data_aout, ADataEnc[iChNum].indata + i*CAudio.buffer_size);
				ACC_size = ADataEnc[iChNum].inlen;			//不解码直接送PCM数据
				//audio_len_ch1 = ACC_size;
				//audio_pos_ch1 = ADataEnc[iChNum].indata;
				ADataEnc[iChNum].inlen = 0;
				if (ACC_size > 0)
				{
					//memcpy(ADataEnc[iChNum].outdata + ADataEnc[iChNum].outlen, data_aout, ACC_size);
					memcpy(ADataEnc[iChNum].outdata, ADataEnc[iChNum].indata, ACC_size);
					memcpy(isAudio->dataq[isAudio->pictq_windex].data, ADataEnc[iChNum].outdata, ACC_size);
					//isAudio->dataq[isAudio->pictq_windex].data = datatemp;			// 预览数据
					isAudio->dataq[isAudio->pictq_windex].len = ACC_size;
					if (++isAudio->pictq_windex == QUEUE_SIZE_AUDIO)
					{
						isAudio->pictq_windex = 0;
					}
					SDL_LockMutex(isAudio->pictq_mutex);
					isAudio->pictq_size++;
					SDL_UnlockMutex(isAudio->pictq_mutex);
					
					ADataEnc[iChNum].outlen += ACC_size;
					//fwrite(ADataEnc[iChNum].outdata, ADataEnc[iChNum].outlen, 1, A_pHFile_1);
					if (ADataEnc[iChNum].outlen > 0)
					{
						//a_pnum = ADataEnc[iChNum].outlen/SEND_LENGTH;
						//a_rmd = ADataEnc[iChNum].outlen%SEND_LENGTH;
						//for (int i=0;i<(a_pnum+1);i++)
						//{
						//	int actDataLen = 0, alen=0;
						//	char	AVData[SEND_LENGTH] ={0};
						//	if (i < a_pnum)
						//	{
						//		memcpy(AVData,ADataEnc[iChNum].outdata +i*SEND_LENGTH,SEND_LENGTH);
						//		actDataLen += SEND_LENGTH;
						//		alen = SEND_LENGTH;
						//	}
						//	if (i == a_pnum && a_rmd != 0)
						//	{
						//		memcpy(AVData,ADataEnc[iChNum].outdata +i*SEND_LENGTH,a_rmd);
						//		actDataLen += a_rmd;
						//		alen = a_rmd;
						//	}
						//	memcpy(ucSendData,PVP_HEADER,4);
						//	pktLen = actDataLen + 6 +16 ;
						//	ucSendData[4] = pktLen&0xff;
						//	ucSendData[5] = (pktLen>>8)&0xff;
						//	ucSendData[6] = (pktLen>>16)&0xff;
						//	ucSendData[7] = (pktLen>>24)&0xff;
						//	ucSendData[8] = (a_pnum+1)&0xff;
						//	ucSendData[10] = (i+1)&0xff;
						//	ucSendData[12] = 0x03;		//Command 
						//	ucSendData[13] = 0x33;		//Command ID
						//	//ucSendData[14] = vlen&0xff;
						//	//ucSendData[15] = (vlen>>8)&0xff;
						//	//ucSendData[16] = (vlen>>16)&0xff;
						//	//ucSendData[17] = (vlen>>24)&0xff;
						//	ucSendData[22] = (alen)&0xff;
						//	ucSendData[23] = (alen>>8)&0xff;
						//	ucSendData[24] = (alen>>16)&0xff;
						//	ucSendData[25] = (alen>>24)&0xff;
						//	memcpy(ucSendData+30, AVData, actDataLen);
						//	memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
						//	MUtest.sendData(ucSendData,pktLen+12);
						//	memset(ucSendData,0,PVP_LENGTH);
						//}
						//if (ADataEnc[iChNum].outlen > 0)
						//{
						//	fwrite(ADataEnc[iChNum].outdata,ADataEnc[iChNum].outlen,1,A_pHFile_1);
						//}
						//ADataEnc[iChNum].outlen = 0;
						sendAudioData(&MUtest, ADataEnc[iChNum].outdata, ADataEnc[iChNum].outlen);
						Aframecnt += MAX_AUDIOFRAME_ENC;
						ADataEnc[iChNum].outlen = 0;
					}
				}
				ACC_size = 0;
			}
			ADataEnc[iChNum].bReady = false;
		}

		if (VDataEnc[iChNum].bReady)
		{
			VDataEnc[iChNum].outlen = 0;
			for (int i=0;i<MAX_VEDIOFRAME_ENC;i++)
			{
				//叠加字幕
				if (isShowNotice)
				{
					if(0 == OSD_FeedFrameYUV420(VDataEnc[iChNum].indata, IMAGEWIDTH, IMAGEHEIGHT))
					{
						OSD_SetPositionCharObj(0,704,noticeHeight);//设置叠加的位置
					}
				}

				isVideo->dataq[isVideo->pictq_windex].len = VDataEnc[iChNum].inlen;
				memcpy(isVideo->dataq[isVideo->pictq_windex].data, VDataEnc[iChNum].indata, 
					isVideo->dataq[isVideo->pictq_windex].len);
				if (++isVideo->pictq_windex == QUEUE_SIZE_VIDEO)
				{ 
					isVideo->pictq_windex = 0;
				}
				SDL_LockMutex(isVideo->pictq_mutex);
				isVideo->pictq_size++;
				SDL_UnlockMutex(isVideo->pictq_mutex);

				videoclk = av_gettime()/1000;
				showclock = videoclk - lastvideoclk;
				lastvideoclk = videoclk;
				if (videocnt > 0)
				{
					sumvideoclk += showclock;
					avgvideoclk = sumvideoclk*1.0 / videocnt; 
				}
				videocnt += 1;
				fprintf(fvideolog, "Receiving --- showclock = %d\t", showclock);
				fprintf(fvideolog, "avgvideoclk = %3.2f\n", avgvideoclk);

				int temp = 1.5*i*y_size;
				h264_size = CVideo.encodeFrameYV12(data_vout, VDataEnc[iChNum].indata + temp);
				if (h264_size >0)
				{
					//int lenRGB = testH264Dec.decodeFrametoRGB2((char *)outRGB,data_vout,h264_size);
					//fwrite(data_vout,h264_size,1,V_pHFile_1);
					//fwrite(outRGB, lenRGB, 1, V_pHFile);
					memcpy(VDataEnc[iChNum].outdata + VDataEnc[iChNum].outlen, data_vout, h264_size);
					VDataEnc[iChNum].outlen += h264_size;
					if (VDataEnc[iChNum].outlen > 0)
					{
						//v_pnum = VDataEnc[iChNum].outlen/SEND_LENGTH;
						//v_rmd = VDataEnc[iChNum].outlen%SEND_LENGTH;
						//for (int i=0;i<(v_pnum+1);i++)
						//{
						//	int actDataLen = 0, vlen=0;
						//	char	AVData[SEND_LENGTH] ={0};
						//	if (i < v_pnum)
						//	{
						//		memcpy(AVData,VDataEnc[iChNum].outdata +i*SEND_LENGTH,SEND_LENGTH);
						//		actDataLen += SEND_LENGTH;
						//		vlen = SEND_LENGTH;
						//	}
						//	if (i == v_pnum && v_rmd != 0)
						//	{
						//		memcpy(AVData,VDataEnc[iChNum].outdata +i*SEND_LENGTH,v_rmd);
						//		actDataLen += v_rmd;
						//		vlen = v_rmd;
						//	}
						//	memcpy(ucSendData,PVP_HEADER,4);
						//	pktLen = actDataLen + 6 +16 ;
						//	ucSendData[4] = pktLen&0xff;
						//	ucSendData[5] = (pktLen>>8)&0xff;
						//	ucSendData[6] = (pktLen>>16)&0xff;
						//	ucSendData[7] = (pktLen>>24)&0xff;
						//	ucSendData[8] = (v_pnum+1)&0xff;
						//	ucSendData[10] = (i+1)&0xff;
						//	ucSendData[12] = 0x03;		//Command 
						//	ucSendData[13] = 0x33;		//Command ID
						//	ucSendData[14] = vlen&0xff;
						//	ucSendData[15] = (vlen>>8)&0xff;
						//	ucSendData[16] = (vlen>>16)&0xff;
						//	ucSendData[17] = (vlen>>24)&0xff;
						//	//ucSendData[22] = (alen)&0xff;
						//	//ucSendData[23] = (alen>>8)&0xff;
						//	//ucSendData[24] = (alen>>16)&0xff;
						//	//ucSendData[25] = (alen>>24)&0xff;
						//	memcpy(ucSendData+30, AVData, actDataLen);
						//	memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
						//	MUtest.sendData(ucSendData,pktLen+12);
						//	memset(ucSendData,0,PVP_LENGTH);
						//}
						//int lenRGB = testH264Dec.decodeFrametoRGB16((char *)outRGB,VDataEnc[iChNum].outdata,VDataEnc[iChNum].outlen);
						//if (VDataEnc[iChNum].outlen >0)//lenRGB > 0)
						//{
						//	//fwrite(outRGB,lenRGB,1,V_pHFile_1);
						//	fwrite(VDataEnc[iChNum].outdata,VDataEnc[iChNum].outlen,1,V_pHFile_1);
						//}
						sendVideoData(&MUtest, VDataEnc[iChNum].outdata, VDataEnc[iChNum].outlen);
						VDataEnc[iChNum].outlen =0;
						Vframecnt +=MAX_VEDIOFRAME_ENC;
					}
				}
				h264_size = 0;
			}
			VDataEnc[iChNum].bReady = false;
		}
		//if (isVideo->pictq_size > 0)
		//{
		//	SDL_Event event;
		//	event.type = REFRESH_EVENT;
		//	SDL_PushEvent(&event);
		//}else{
		//	SDL_Delay(5);
		//}
	}
	delete[] datatemp;
	delete[] cNoteContent;
	CHKtest.stop(iChNum);
	CAudio.closeCoder();
	CVideo.closeCoder();
	MUtest.closeSock();
	testH264Dec.closeDecoder();
	fclose(V_pHFile_1);
	fclose(V_pHFile_org);
	fclose(A_pHFile_1);

	return 1;
}

int __cdecl StreamDirectReadCallback(ULONG channelNum, void *DataBuf, DWORD Length, int frameType, void *context)
{
	BOOL breakable = FALSE; 
	BOOL subbreakable = FALSE; 
	//CString	csOutputStr; 
	//CHTDemoDlg *hkdlg = (CHTDemoDlg *)context; 
	UINT i = 0;

	DWORD dwRet = 0;
	DWORD dwReadedLen = 0;
	PBYTE pBufferTemp = NULL;
	DWORD dwBufSizeTemp = 0;
	int   times=0;

	//头文件
	if (frameType == PktSysHeader && Length == 40)
	{
		struEncodeChan[channelNum].bGethead = TRUE; 
		memcpy(struEncodeChan[channelNum].szFileHeader, (unsigned char *)DataBuf, Length); 
		//memcpy(struEncodeChan[0].szFileHeader, (unsigned char *)DataBuf, Length); 
		m_iFileHeaderLen = Length; 
	}
	if (frameType == PktSubSysHeader && Length == 40)
	{
		struEncodeChan[channelNum].bGetSubhead = TRUE; 
		memcpy(struEncodeChan[channelNum].szSubFileHeader, (unsigned char *)DataBuf, Length); 
		m_iFileHeaderLen = Length; 
	}

	if(frameType == PktIFrames)
	{
		breakable = TRUE; 
	}
	if(frameType == PktSubIFrames)
	{
		subbreakable = TRUE; 
	}
	//写入主通道文件数据
	if(frameType ==PktIFrames || frameType ==PktPFrames
		|| frameType == PktBBPFrames || frameType == PktBPFrames 
		|| frameType == PktSysHeader
		|| frameType ==PktAudioFrames)	
	{
		if (struEncodeChan[channelNum].bGethead )//&& struEncodeChan[channelNum].hFileHandle)
		{
			//fwritefileCS(struEncodeChan[channelNum].hFileHandle,g_csRecordFile[channelNum],DataBuf,Length);
			struEncodeChan[channelNum].uChlength += Length; 
		}

		BOOL bFlag    = FALSE;
		int  nError   = 0;
		while (1)
		{
			//if (frameType ==PktAudioFrames)
			//{
			//	bFlag = PlayM4_InputAudioData(m_lPort,(PBYTE)DataBuf, Length);
			//	if (bFlag == FALSE)
			//	{
			//		nError = PlayM4_GetLastError(m_lPort);
			//		//若缓冲区满，则重复送入数据
			//		if (nError == PLAYM4_BUF_OVER)
			//		{
			//			Sleep(2);
			//			continue;
			//		}
			//	}
			//	break;
			//}
			bFlag = PlayM4_InputData(m_lPort[channelNum],(PBYTE)DataBuf,Length);
			//bFlag = PlayM4_InputVideoData(m_lPort,(PBYTE)DataBuf,Length);
			if (bFlag == FALSE)
			{
				nError = PlayM4_GetLastError(m_lPort[channelNum]);
				//若缓冲区满，则重复送入数据
				if (nError == PLAYM4_BUF_OVER)
				{
					Sleep(2);
					continue;
				}
			}
			//若送入成功，则继续读取数据送入到播放库缓冲
			break; 
		}

		if (
			frameType ==PktIFrames 
			//|| frameType ==PktPFrames 
			|| frameType == PktBBPFrames 
			//|| frameType == PktBPFrames
			|| frameType == PktSysHeader
			)
		{
			//int size_RGB = testH264Dec.decodeFrame((char *)outRGB, DataBuf, Length);
			//fwrite(outRGB, size_RGB, 1, V_pHFile_org);
			//fwrite(DataBuf, Length, 1, V_pHFile_org);
		}
	}
	//
	//if (frameType ==PktAudioFrames )
	//{
	//	if (struEncodeChan[0].hFileHandle)
	//	{
	//		fwritefileCS(struEncodeChan[0].hFileHandle,g_csRecordFile[0],DataBuf,Length);
	//		struEncodeChan[0].uChlength += Length;
	//	}	
	//}
	//写入子通道文件数据
	if (frameType ==PktAudioFrames
		|| frameType ==PktSubIFrames
		|| frameType ==PktSubPFrames || frameType == PktSubBBPFrames 
		|| frameType == PktSubSysHeader)		
	{
		if (struEncodeChan[channelNum].hSubFileHandle 
			&& struEncodeChan[channelNum].bGetSubhead)
		{
			//fwritefileCS(Class_HKcard_4008::struEncodeChan[channelNum].hSubFileHandle,g_csSubRecordFile[channelNum],DataBuf,Length);
			struEncodeChan[channelNum].uSubChlength += Length; 
		}	
	}

	//解码通道解码编码通道实时流
	//for (i = 0; i < g_nTotalDecChannels; i++)
	//{
	//	if(struDecodeChan[i].bOpenRealStream[channelNum])
	//	{
	//		if(struDecodeChan[i].bOpenStreamSuccessed == FALSE)
	//		{
	//			if(HW_OpenStream(struDecodeChan[i].hChannelHandle, struEncodeChan[channelNum].szFileHeader, Length) != HWERR_SUCCESS)
	//			{
	//				return 0; 
	//			}
	//			else
	//			{
	//				hkdlg->myoutput[i].StartDecodeVideoPreview(FALSE); 
	//				dwRet = HW_Play(struDecodeChan[i].hChannelHandle);
	//				if (dwRet != 0)
	//				{
	//					csOutputStr.Format("StreamDirectReadCallback->HW_Play failed, ret=%x", dwRet);
	//					g_pMainDlg->DebugOutput(i,csOutputStr,FAIL_LOG);	
	//				}
	//				else
	//				{
	//					struDecodeChan[i].bDecPlay = TRUE; 
	//				}
	//				struDecodeChan[i].bOpenStreamSuccessed = TRUE; 
	//			}

	//		}
	//		if(frameType ==PktAudioFrames 
	//			|| frameType == PktIFrames || frameType == PktPFrames 
	//			|| frameType == PktBBPFrames || frameType == PktBPFrames)
	//		{
	//			if (struDecodeChan[i].bOpenStreamSuccessed == TRUE)
	//			{
	//				dwReadedLen = 0;
	//				times = 0;

	//				while((dwReadedLen<Length)&&(times++<10))
	//				{
	//					//Modified at 2012-05-29
	//					pBufferTemp = (unsigned char *)DataBuf+dwReadedLen;
	//					dwBufSizeTemp = Length - dwReadedLen;
	//					dwRet=HW_InputData(struDecodeChan[i].hChannelHandle, pBufferTemp, dwBufSizeTemp);
	//					dwReadedLen += dwRet;
	//					if(dwReadedLen<Length)
	//					{
	//						Sleep(15);
	//					}
	//				}
	//				//HW_InputData(g_hDecChannelHandle[i],(unsigned char *)DataBuf,Length);
	//			}
	//		}
	//	}
	//}

	if (NULL == g_pSettingValue[channelNum] || NULL == g_pSubSettingValue[channelNum])
	{
		return 0; 
	}
	struEncodeChan[channelNum].iFileSize = g_pSettingValue[channelNum]->iFileSize; 
	struEncodeChan[channelNum].iSubFileSize = g_pSubSettingValue[channelNum]->iFileSize; 
	//if (!g_pMainDlg->m_bSwitchFileByTime)
	//{
	//	if ((struEncodeChan[channelNum].uChlength > (struEncodeChan[channelNum].iFileSize * 1024 * 1024)) && breakable)
	//	{
	//		struEncodeChan[channelNum].uChlength = 0; 
	//		hkdlg->StopCaptureChannel(channelNum); 
	//		struEncodeChan[channelNum].bGethead = TRUE; 
	//		hkdlg->StartCaptureChannel(g_szCurrentDriver, channelNum); 
	//		//增加一句，将当前I帧写入刚切换的文件头数据之后
	//		fwritefileCS(struEncodeChan[channelNum].hFileHandle,g_csRecordFile[channelNum],DataBuf,Length);
	//	}
	//	if ((struEncodeChan[channelNum].uSubChlength > (struEncodeChan[channelNum].iSubFileSize * 1024 * 1024)) && subbreakable)
	//	{
	//		struEncodeChan[channelNum].uSubChlength = 0; 
	//		hkdlg->StopSubCaptureChannel(channelNum); 
	//		struEncodeChan[channelNum].bGetSubhead = TRUE; 
	//		hkdlg->StartSubCaptureChannel(g_szCurrentDriver, channelNum); 
	//		//增加一句，将当前I帧写入刚切换的文件头数据之后
	//		fwritefileCS(struEncodeChan[channelNum].hSubFileHandle,g_csSubRecordFile[channelNum],DataBuf,Length);
	//	}
	//}
	//else
	//{
	//if (struEncodeChan[channelNum].bSwitchFile && breakable)
	//{
	//	struEncodeChan[channelNum].bSwitchFile = FALSE; 
	//	struEncodeChan[channelNum].uChlength = 0; 
	//	StopCaptureChannel(channelNum); 
	//	struEncodeChan[channelNum].bGethead = TRUE; 
	//	StartCaptureChannel(g_szCurrentDriver, channelNum); 
	//	//增加一句，将当前I帧写入刚切换的文件头数据之后
	//	fwritefileCS(struEncodeChan[channelNum].hFileHandle,g_csRecordFile[channelNum],DataBuf,Length);
	//}
	//if (struEncodeChan[channelNum].bSwitchFile && subbreakable)
	//{
	//	struEncodeChan[channelNum].bSubSwitchFile = FALSE; 
	//	struEncodeChan[channelNum].uSubChlength = 0; 
	//	StopSubCaptureChannel(channelNum); 
	//	struEncodeChan[channelNum].bGetSubhead = TRUE; 
	//	StartSubCaptureChannel(g_szCurrentDriver, channelNum); 
	//	//增加一句，将当前I帧写入刚切换的文件头数据之后
	//	fwritefileCS(struEncodeChan[channelNum].hSubFileHandle,g_csSubRecordFile[channelNum],DataBuf,Length);
	//}
	//}

	return 0; 
}

////----------------------decode------------------------------
void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
	static int cntARead[MAX_PORTS] ={0};
	static int cntVRead[MAX_PORTS] ={0};
	int		numPort = 0;

	//if (nPort = m_lPort[0])
	//{
	//	numPort = 0;
	//}
	//else if (nPort = m_lPort[1])
	//{
	//	numPort = 1;
	//}
	//else if (nPort = m_lPort[2])
	//{
	//	numPort = 2;
	//}
	//else
	//{
	//	numPort = 3;
	//}
	//---------------------------------------
	// 获取解码后音频数据
	if (pFrameInfo->nType == T_AUDIO16)
	{
		FILE *Atemp;
		//AudioFile.writeAppend(pBuf,nSize);
		//fwrite(pBuf,nSize,1,A_pHFile);
		Atemp = fopen("atemp.txt","a");
		fprintf(Atemp,"%d  ", nSize);
		memcpy(ucATemp +cntARead[numPort]*nSize, pBuf, nSize);
		ADataEnc[numPort].inlen += nSize;
		cntARead[numPort] += 1;
		if (cntARead[numPort]==MAX_AUDIOFRAME_ENC)		//注释后每帧发送一次
		{
			memcpy(ADataEnc[numPort].indata, ucATemp, ADataEnc[numPort].inlen);
			ADataEnc[numPort].bReady = true;
			//ADataEnc[numPort].inlen = 0;
			cntARead[numPort] = 0;
		}
		fclose(Atemp);
		//printf("test:: get audio data !\n");
	}

	////---------------------------------------
	//// 获取解码后视频数据
	else if ( pFrameInfo->nType == T_YV12 ) 
	{   
		//VideoFile.writeAppend(pBuf, nSize);
		memcpy(ucVTemp +cntVRead[numPort]*nSize, pBuf, nSize);
		VDataEnc[numPort].inlen += nSize;
		cntVRead[numPort] +=1;
		if (cntVRead[numPort] == MAX_VEDIOFRAME_ENC)
		{
			memcpy(VDataEnc[numPort].indata, ucVTemp, VDataEnc[numPort].inlen);
			VDataEnc[numPort].bReady = true;
			VDataEnc[numPort].inlen = 0;
			cntVRead[numPort] = 0;
		}
		
		//fwrite(pBuf,nSize,1,m_pHFile);
		//printf("test:: get video data !\n");
	}
	//printf("nType = %d\n", pFrameInfo->nType);
}

void CALLBACK DecCBFun_1(long nPort,char * pBuf,long nSize,FRAME_INFO * pFrameInfo, long nReserved1,long nReserved2)
{
	static int cntARead[MAX_PORTS] ={0};
	static int cntVRead[MAX_PORTS] ={0};
	static int adatalen = 0, vdatalen = 0;
	int		numPort = 1;

	//---------------------------------------
	// 获取解码后音频数据
	if (pFrameInfo->nType == T_AUDIO16)
	{
		//FILE *Atemp;
		//Atemp = fopen("atemp.txt","a");
		//fprintf(Atemp,"%d  ", nSize);
		//memcpy(ucATemp +ADataEnc[numPort].inlen, pBuf, nSize);
		memcpy(ucATemp +adatalen, pBuf, nSize);
		adatalen += nSize;
		cntARead[numPort] += 1;
		if (cntARead[numPort]==MAX_AUDIOFRAME_ENC)		//这里还是要缓冲几帧音频数据再发送，否则视频数据可能影响到音频数据
		{
			ADataEnc[numPort].inlen = adatalen;
			memcpy(ADataEnc[numPort].indata, ucATemp, ADataEnc[numPort].inlen);
			//fwrite(ADataEnc[numPort].indata, ADataEnc[numPort].inlen, 1, A_pHFile_1);
			ADataEnc[numPort].bReady = true;
			bPrewAudio = true;
			adatalen = 0;
			cntARead[numPort] = 0;
		}
		//fclose(Atemp);
		//printf("test:: get audio data !\n");
	}

	////---------------------------------------
	//// 获取解码后视频数据
	else if ( pFrameInfo->nType == T_YV12 ) 
	{   
		//VideoFile.writeAppend(pBuf, nSize);
		memcpy(ucVTemp +cntVRead[numPort]*nSize, pBuf, nSize);
		vdatalen += nSize;
		
		cntVRead[numPort] +=1;
		if (cntVRead[numPort] == MAX_VEDIOFRAME_ENC)
		{
			VDataEnc[numPort].inlen = vdatalen;
			memcpy(VDataEnc[numPort].indata, ucVTemp, VDataEnc[numPort].inlen);
			VDataEnc[numPort].bReady = true;
			//VDataEnc[numPort].inlen = 0;
			vdatalen  = 0;
			cntVRead[numPort] = 0;
		}
	}
}

namespace UDP_Win_test {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	using std::ofstream;
	using std::ifstream;
	using std::ios;
	using std::endl;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  playToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  listToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;

	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::DataGridView^  dataGridView1;




	private: System::Windows::Forms::BindingSource^  bindingSource1;
	private: System::Windows::Forms::Button^  button5;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column1;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column2;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column3;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column4;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column5;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column6;
	private: System::Windows::Forms::Button^  button6;
	private: System::Windows::Forms::RadioButton^  radioButton1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel1;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Button^  button7;
	private: System::Windows::Forms::Button^  button8;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;



	private: System::ComponentModel::IContainer^  components;
	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		HANDLE hThread_fileplay;
		DWORD threadId_fileplay;
		HANDLE hThread_VIDPrew0, hThread_AUDPrew0, hThread_VIDPrew1, hThread_AUDPrew1;
		DWORD threadId_VIDPrew0, threadId_AUDPrew0, threadId_VIDPrew1, threadId_AUDPrew1;
	private: System::Windows::Forms::Button^  button9;
	private: System::Windows::Forms::Timer^  timer2;
	private: System::Windows::Forms::Button^  button11;


	private: System::Windows::Forms::Button^  button10;
			 
	public:
		


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->playToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->listToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->helpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->dataGridView1 = (gcnew System::Windows::Forms::DataGridView());
			this->Column1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column4 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column5 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column6 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->bindingSource1 = (gcnew System::Windows::Forms::BindingSource(this->components));
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->radioButton1 = (gcnew System::Windows::Forms::RadioButton());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button8 = (gcnew System::Windows::Forms::Button());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->button10 = (gcnew System::Windows::Forms::Button());
			this->timer2 = (gcnew System::Windows::Forms::Timer(this->components));
			this->button11 = (gcnew System::Windows::Forms::Button());
			this->menuStrip1->SuspendLayout();
			this->statusStrip1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bindingSource1))->BeginInit();
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->button1->Location = System::Drawing::Point(325, 412);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(108, 46);
			this->button1->TabIndex = 0;
			this->button1->Text = L"探测设备";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->playToolStripMenuItem, 
				this->listToolStripMenuItem, this->helpToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(1216, 25);
			this->menuStrip1->TabIndex = 1;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// playToolStripMenuItem
			// 
			this->playToolStripMenuItem->Name = L"playToolStripMenuItem";
			this->playToolStripMenuItem->Size = System::Drawing::Size(43, 21);
			this->playToolStripMenuItem->Text = L"&Play";
			// 
			// listToolStripMenuItem
			// 
			this->listToolStripMenuItem->Name = L"listToolStripMenuItem";
			this->listToolStripMenuItem->Size = System::Drawing::Size(39, 21);
			this->listToolStripMenuItem->Text = L"&List";
			// 
			// helpToolStripMenuItem
			// 
			this->helpToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->aboutToolStripMenuItem});
			this->helpToolStripMenuItem->Name = L"helpToolStripMenuItem";
			this->helpToolStripMenuItem->Size = System::Drawing::Size(47, 21);
			this->helpToolStripMenuItem->Text = L"&Help";
			// 
			// aboutToolStripMenuItem
			// 
			this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
			this->aboutToolStripMenuItem->Size = System::Drawing::Size(111, 22);
			this->aboutToolStripMenuItem->Text = L"&About";
			this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::aboutToolStripMenuItem_Click);
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripStatusLabel1});
			this->statusStrip1->Location = System::Drawing::Point(0, 461);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(1216, 22);
			this->statusStrip1->TabIndex = 2;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this->toolStripStatusLabel1->Name = L"toolStripStatusLabel1";
			this->toolStripStatusLabel1->Size = System::Drawing::Size(131, 17);
			this->toolStripStatusLabel1->Text = L"toolStripStatusLabel1";
			// 
			// timer1
			// 
			this->timer1->Interval = 500;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// button2
			// 
			this->button2->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->button2->Location = System::Drawing::Point(464, 412);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(108, 46);
			this->button2->TabIndex = 4;
			this->button2->Text = L"切换频道";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button3
			// 
			this->button3->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->button3->Location = System::Drawing::Point(601, 412);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(108, 46);
			this->button3->TabIndex = 5;
			this->button3->Text = L"开机";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// button4
			// 
			this->button4->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->button4->Location = System::Drawing::Point(738, 412);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(108, 46);
			this->button4->TabIndex = 6;
			this->button4->Text = L"休眠";
			this->button4->UseVisualStyleBackColor = true;
			// 
			// dataGridView1
			// 
			this->dataGridView1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->dataGridView1->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView1->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(6) {this->Column1, 
				this->Column2, this->Column3, this->Column4, this->Column5, this->Column6});
			this->dataGridView1->Location = System::Drawing::Point(25, 28);
			this->dataGridView1->Name = L"dataGridView1";
			this->dataGridView1->RowTemplate->Height = 23;
			this->dataGridView1->Size = System::Drawing::Size(998, 371);
			this->dataGridView1->TabIndex = 7;
			this->dataGridView1->Enter += gcnew System::EventHandler(this, &Form1::dataGridView1_Enter);
			// 
			// Column1
			// 
			this->Column1->HeaderText = L"地址";
			this->Column1->Name = L"Column1";
			// 
			// Column2
			// 
			this->Column2->HeaderText = L"状态";
			this->Column2->Name = L"Column2";
			this->Column2->Width = 150;
			// 
			// Column3
			// 
			this->Column3->HeaderText = L"IP地址";
			this->Column3->Name = L"Column3";
			// 
			// Column4
			// 
			this->Column4->HeaderText = L"命令端口";
			this->Column4->Name = L"Column4";
			// 
			// Column5
			// 
			this->Column5->HeaderText = L"数据端口";
			this->Column5->Name = L"Column5";
			// 
			// Column6
			// 
			this->Column6->HeaderText = L"频道号";
			this->Column6->Name = L"Column6";
			// 
			// button5
			// 
			this->button5->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button5->Location = System::Drawing::Point(1035, 38);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(108, 46);
			this->button5->TabIndex = 8;
			this->button5->Text = L"保存地址";
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &Form1::button5_Click);
			// 
			// button6
			// 
			this->button6->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button6->Location = System::Drawing::Point(1035, 111);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(108, 46);
			this->button6->TabIndex = 9;
			this->button6->Text = L"频道1播放";
			this->button6->UseVisualStyleBackColor = true;
			this->button6->Click += gcnew System::EventHandler(this, &Form1::button6_Click);
			// 
			// radioButton1
			// 
			this->radioButton1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->radioButton1->AutoCheck = false;
			this->radioButton1->AutoSize = true;
			this->radioButton1->Location = System::Drawing::Point(1044, 228);
			this->radioButton1->Name = L"radioButton1";
			this->radioButton1->Size = System::Drawing::Size(71, 16);
			this->radioButton1->TabIndex = 10;
			this->radioButton1->TabStop = true;
			this->radioButton1->Text = L"字幕公告";
			this->radioButton1->UseVisualStyleBackColor = true;
			this->radioButton1->Click += gcnew System::EventHandler(this, &Form1::radioButton1_Click);
			// 
			// comboBox1
			// 
			this->comboBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(1065, 427);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(150, 20);
			this->comboBox1->TabIndex = 11;
			this->comboBox1->Text = L"IPAddress";
			this->comboBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::comboBox1_SelectedIndexChanged);
			// 
			// label1
			// 
			this->label1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(1013, 431);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(53, 12);
			this->label1->TabIndex = 12;
			this->label1->Text = L"本机IP：";
			// 
			// button7
			// 
			this->button7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button7->Location = System::Drawing::Point(1035, 182);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(108, 46);
			this->button7->TabIndex = 13;
			this->button7->Text = L"频道2播放";
			this->button7->UseVisualStyleBackColor = true;
			this->button7->Click += gcnew System::EventHandler(this, &Form1::button7_Click);
			// 
			// button8
			// 
			this->button8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button8->Location = System::Drawing::Point(1035, 250);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(108, 46);
			this->button8->TabIndex = 14;
			this->button8->Text = L"文件播放";
			this->button8->UseVisualStyleBackColor = true;
			this->button8->Click += gcnew System::EventHandler(this, &Form1::button8_Click);
			// 
			// button9
			// 
			this->button9->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button9->Location = System::Drawing::Point(1149, 111);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(60, 46);
			this->button9->TabIndex = 15;
			this->button9->Text = L"预览1";
			this->button9->UseVisualStyleBackColor = true;
			this->button9->Click += gcnew System::EventHandler(this, &Form1::button9_Click);
			// 
			// button10
			// 
			this->button10->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button10->Enabled = false;
			this->button10->Location = System::Drawing::Point(1149, 182);
			this->button10->Name = L"button10";
			this->button10->Size = System::Drawing::Size(60, 46);
			this->button10->TabIndex = 16;
			this->button10->Text = L"预览2";
			this->button10->UseVisualStyleBackColor = true;
			this->button10->Click += gcnew System::EventHandler(this, &Form1::button10_Click);
			// 
			// timer2
			// 
			this->timer2->Interval = 35;
			this->timer2->Tick += gcnew System::EventHandler(this, &Form1::timer2_Tick);
			// 
			// button11
			// 
			this->button11->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->button11->Location = System::Drawing::Point(876, 412);
			this->button11->Name = L"button11";
			this->button11->Size = System::Drawing::Size(108, 46);
			this->button11->TabIndex = 17;
			this->button11->Text = L"添加字幕公告";
			this->button11->UseVisualStyleBackColor = true;
			this->button11->Click += gcnew System::EventHandler(this, &Form1::button11_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1216, 483);
			this->Controls->Add(this->button11);
			this->Controls->Add(this->button10);
			this->Controls->Add(this->button9);
			this->Controls->Add(this->button8);
			this->Controls->Add(this->button7);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->comboBox1);
			this->Controls->Add(this->radioButton1);
			this->Controls->Add(this->button6);
			this->Controls->Add(this->button5);
			this->Controls->Add(this->dataGridView1);
			this->Controls->Add(this->button4);
			this->Controls->Add(this->button3);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->menuStrip1);
			this->MainMenuStrip = this->menuStrip1;
			this->Name = L"Form1";
			this->Text = L"Form1";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->Shown += gcnew System::EventHandler(this, &Form1::Form1_Shown);
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->bindingSource1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void aboutToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
				 MessageBox::Show(L"Copyright Skyeyes",L"About ",MessageBoxButtons::OK,MessageBoxIcon::Exclamation);
			 }
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			UDPWin udpTest;
			unsigned char ucSendData[PVP_LENGTH]={0};
			unsigned char ucrevcData[PVP_LENGTH] = {0};
			int	iRevcNum = 0,dstPort = 1,iTemp;
			String^ SdstIP; String^ SdstPort;
			char cDstIP[20],cTemp2[20];

			//udpTest.GetLocalIP(cLocalIP);
			//strcpy(cLocalIP, cxt.marshal_as<char const*>(comboBox1->SelectedItem->ToString()));
			//cLocalIP = cxt.marshal_as<char const*>(comboBox1->SelectedItem->ToString());
			for (int j=0; j<dataGridView1->RowCount-1;j++)
			{
				SdstIP = dataGridView1->Rows[j]->Cells[2]->Value->ToString();
				SdstPort = dataGridView1->Rows[j]->Cells[3]->Value->ToString();
				iTemp = SdstIP->Length;
				for (int i =0;i<iTemp;i++)
				{
					cDstIP[i] = SdstIP[i];
				}
				cDstIP[iTemp] = '\0';
				iTemp = SdstPort->Length;
				for (int i =0;i<iTemp;i++)
				{
					cTemp2[i] = SdstPort[i];
				}
				cTemp2[iTemp] = '\0';
				dstPort = atoi(cTemp2);

				memcpy(ucSendData,PVP_HEADER,4);
				ucSendData[4] = 36&0xff;
				ucSendData[8] = 1;
				ucSendData[10] = 1;
				ucSendData[12] = 0x01;		//Command 
				ucSendData[13] = 0x31;		//Command ID
				memcpy(ucSendData+ ucSendData[4] +8,PVP_TAILER,4);

				udpTest.sendudp(cLocalIP, cDstIP, dstPort, dstPort, 
					(char*)&ucSendData[0], ucSendData[4] +12);
				iRevcNum = udpTest.receiveudp(cLocalIP, cDstIP, dstPort, dstPort, 
					(char*)&ucrevcData[0], PVP_LENGTH);
				if (iRevcNum == 74)
				{
					if (ucrevcData[13] == 0)
					{
						dataGridView1->Rows[j]->Cells[1]->Value = L"正常执行探测设备";
						dataGridView1->Rows[j]->Cells[1]->Style->ForeColor = Color::Black;
					}
					else
					{
						dataGridView1->Rows[j]->Cells[1]->Value = L"非正常执行";
					}
				}
				else
				{
					dataGridView1->Rows[j]->Cells[1]->Value = L"无法连接";
					dataGridView1->Rows[j]->Cells[1]->Style->ForeColor = Color::Red;
				}
				dataGridView1->Refresh();
			}
		 }
	private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) 
		 {			 
			 ofstream outClientFile("client.txt", ios::out);

			 setlocale(LC_ALL,"chs");
			 String^ t1,^t2,^t3,^t4,^t5,^t6;
			 wchar_t d1[20]={0},d3[20]={0},d4[20]={0},d5[20]={0},d6[20]={0};
			 wchar_t d2[50]={0};
			 int temp;

			 temp = dataGridView1->RowCount - 1;
			for (int i=0;i<temp;i++)
			{
				t1 = dataGridView1->Rows[i]->Cells[0]->Value->ToString();
				
				if (dataGridView1->Rows[i]->Cells[1]->Value)
				{
					t2 = dataGridView1->Rows[i]->Cells[1]->Value->ToString();
				}
				else
				{
					t2 = L"unconnected";
				}
				if (dataGridView1->Rows[i]->Cells[2]->Value)
				{
					t3 = dataGridView1->Rows[i]->Cells[2]->Value->ToString();
				}
				else{
					MessageBoxW(NULL, L"请输入正确IP地址！", L"提示",MB_ICONERROR);
					return;
				}
				t4 = dataGridView1->Rows[i]->Cells[3]->Value->ToString();
				t5 = dataGridView1->Rows[i]->Cells[4]->Value->ToString();
				t6 = dataGridView1->Rows[i]->Cells[5]->Value->ToString();
				memset(d1,0,20);
				memset(d2,0,20);
				memset(d3,0,20);
				memset(d4,0,20);
				memset(d5,0,20);
				memset(d6,0,20);
				for (int i=0;i<t1->Length;i++)
				{	d1[i] = t1[i];	}
				for (int i=0;i<t2->Length;i++)
				{	d2[i] = t2[i];	}
				for (int i=0;i<t3->Length;i++)
				{	d3[i] = t3[i];	}
				for (int i=0;i<t4->Length;i++)
				{	d4[i] = t4[i];	}
				for (int i=0;i<t5->Length;i++)
				{	d5[i] = t5[i];	}
				for (int i=0;i<t6->Length;i++)
				{	d6[i] = t6[i];	}
				//outClientFile<<(char*)d1<<" " <<(char*)d2<<" "<<(char*)d3<<" "<<(char*)d4<<" "<<(char*)d5<<" "<<(char*)d6<<endl;
				outClientFile<<(char*)d1<<endl;
			 }
			 //dataGridView1->Rows->Add(0,"office","192.168","5555");
		}
private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e)
		 {
			 UDPWin udpTest;
			 unsigned char ucSendData[PVP_LENGTH]={0};
			 unsigned char ucrevcData[PVP_LENGTH] = {0};
			 int	iRevcNum = 0,dstPort = 1,iTemp;
			 String^ SdstIP; String^ SdstPort;
			 char cDstIP[20],cTemp2[20];

			 SdstIP = dataGridView1->Rows[0]->Cells[2]->Value->ToString();
			 SdstPort = dataGridView1->Rows[0]->Cells[3]->Value->ToString();
			 iTemp = SdstIP->Length;
			 for (int i =0;i<iTemp;i++)
			 {
				 cDstIP[i] = SdstIP[i];
			 }
			 cDstIP[iTemp] = '\0';
			 iTemp = SdstPort->Length;
			 for (int i =0;i<iTemp;i++)
			 {
				 cTemp2[i] = SdstPort[i];
			 }
			 cTemp2[iTemp] = '\0';
			 dstPort = atoi(cTemp2);

			 memcpy(ucSendData,PVP_HEADER,4);
			 ucSendData[4] = 8&0xff;
			 ucSendData[8] = 1;
			 ucSendData[10] = 1;
			 ucSendData[12] = 0x02;		//Command 
			 ucSendData[13] = 0x32;		//Command ID
			 memcpy(ucSendData+ ucSendData[4] +8,PVP_TAILER,4);

			 udpTest.sendudp(cLocalIP, cDstIP, dstPort, dstPort, 
				 (char*)&ucSendData[0], ucSendData[4] +12);
			 iRevcNum = udpTest.receiveudp(cLocalIP, cDstIP, dstPort, dstPort, 
				 (char*)&ucrevcData[0], PVP_LENGTH);
			 if (iRevcNum == 22)
			 {
				 if (ucrevcData[13] == 0)
				 {
					 dataGridView1->Rows[0]->Cells[1]->Value = L"正常执行频道切换";
				 }
				 else
				 {
					 dataGridView1->Rows[0]->Cells[1]->Value = L"非正常执行";
				 }
			 }
		 }
private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 UDPWin udpTest;
			 unsigned char ucSendData[PVP_LENGTH]={0};
			 unsigned char ucrevcData[PVP_LENGTH] = {0};
			 int	iRevcNum = 0,dstPort = 1,iTemp;
			 String^ SdstIP; String^ SdstPort;
			 char cDstIP[20],cTemp2[20];

			 SdstIP = dataGridView1->Rows[0]->Cells[2]->Value->ToString();
			 SdstPort = dataGridView1->Rows[0]->Cells[3]->Value->ToString();
			 iTemp = SdstIP->Length;
			 for (int i =0;i<iTemp;i++)
			 {
				 cDstIP[i] = SdstIP[i];
			 }
			 cDstIP[iTemp] = '\0';
			 iTemp = SdstPort->Length;
			 for (int i =0;i<iTemp;i++)
			 {
				 cTemp2[i] = SdstPort[i];
			 }
			 cTemp2[iTemp] = '\0';
			 dstPort = atoi(cTemp2);

			 memcpy(ucSendData,PVP_HEADER,4);
			 ucSendData[4] = 6&0xff;
			 ucSendData[8] = 1;
			 ucSendData[10] = 1;
			 ucSendData[12] = 0x04;		//Command 
			 ucSendData[13] = 0x34;		//Command ID
			 memcpy(ucSendData+ ucSendData[4] +8,PVP_TAILER,4);

			 udpTest.sendudp(cLocalIP, cDstIP, dstPort, dstPort, 
				 (char*)&ucSendData[0], ucSendData[4] +12);
			 iRevcNum = udpTest.receiveudp(cLocalIP, cDstIP, dstPort, dstPort, 
				 (char*)&ucrevcData[0], PVP_LENGTH);
			 if (iRevcNum == 18)
			 {
				 if (ucrevcData[13] == 0)
				 {
					 dataGridView1->Rows[0]->Cells[1]->Value = L"正常执行开机";
				 }
				 else
				 {
					 dataGridView1->Rows[0]->Cells[1]->Value = L"非正常执行";
				 }
			 }
		 }
private: System::Void Form1_Shown(System::Object^  sender, System::EventArgs^  e) 
		 {
			 ifstream outClientFile("client.txt", ios::in);

			 wchar_t d1[20]={0},d3[20]={0},d4[20]={0},d5[20]={0},d6[20]={0};
			 wchar_t d2[50];
			 String ^s1;
			 int i=0,temp;
			 UDPWin udp1;
			 IPInfo localIPList[10];
			 int iIPcnt=0;

			 outClientFile>>(char*)d1;
			 dataGridView1->Rows[i]->Cells[0]->Value = gcnew String(d1);
			 while (outClientFile>>(char*)d1>>(char*)d2>>(char*)d3>>(char*)d4>>(char*)d5>>(char*)d6)
			 {
				dataGridView1->Rows->Add();
				int j=0;
				while (d1[j] != '\0')
				{	s1 += (wchar_t)d1[j];	j++;	}
				 dataGridView1->Rows[i]->Cells[0]->Value =s1;
				 s1 = ""; j = 0;
				 while (d2[j] != '\0')
				 {	s1 += (wchar_t)d2[j];	j++;	}
				 dataGridView1->Rows[i]->Cells[1]->Value =s1;
				 s1 = ""; j = 0;
				 while (d3[j] != '\0')
				 {	s1 += (wchar_t)d3[j];	j++;	}
				 dataGridView1->Rows[i]->Cells[2]->Value =s1;
				 s1 = "";  j = 0;
				 while (d4[j] != '\0')
				 {	s1 += (wchar_t)d4[j];	j++;	}
				 dataGridView1->Rows[i]->Cells[3]->Value =s1;
				 s1 = ""; j = 0;
				 while (d5[j] != '\0')
				 {	s1 += (wchar_t)d5[j];	j++;	}
				 dataGridView1->Rows[i]->Cells[4]->Value =s1;
				 s1 = ""; j = 0;
				 while (d6[j] != '\0')
				 {	s1 += (wchar_t)d6[j];	j++;	}
				 dataGridView1->Rows[i]->Cells[5]->Value =s1;
				 s1 = ""; j = 0;
				 i++;
				memset(d1,0,20);
				memset(d2,0,20);
				memset(d3,0,20);
				memset(d4,0,20);
				memset(d5,0,20);
				memset(d6,0,20);
			 }
			 udp1.GetLocalIPs(localIPList, 10,&iIPcnt);
			 for (int i=0;i<iIPcnt;i++)
			 {
				 String ^strString = gcnew String(localIPList[i].ip);
				 comboBox1->Items->Add(strString);
			 }
			 String ^strString = gcnew String(localIPList[iIPcnt-1].ip);
			 comboBox1->Text = strString;
			 //dataGridView1->Rows->Add(L"Meeting",L"unconnected",L"192.168.1.188",L"9999",L"5552",L"00");
		 }
private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 /*UDPWin udpTest;
			 MulticastUDP MUtest;
			 unsigned char ucSendData[PVP_LENGTH]={0};
			 unsigned char ucrevcData[PVP_LENGTH] = {0};
			 int	iRevcNum = 0,dstPort = 1,iTemp;
			 String^ SdstIP; String^ SdstPort;
			 char cDstIP[20],cTemp2[20], localIP[20]={0};

			 SdstIP = dataGridView1->Rows[0]->Cells[2]->Value->ToString();
			 SdstPort = dataGridView1->Rows[0]->Cells[4]->Value->ToString();
			 iTemp = SdstIP->Length;
			 for (int i =0;i<iTemp;i++)
			 {
				 cDstIP[i] = SdstIP[i];
			 }
			 cDstIP[iTemp] = '\0';
			 iTemp = SdstPort->Length;
			 for (int i =0;i<iTemp;i++)
			 {
				 cTemp2[i] = SdstPort[i];
			 }
			 cTemp2[iTemp] = '\0';
			 dstPort = atoi(cTemp2);*/
			 /*
			 memcpy(ucSendData,PVP_HEADER,4);
			 ucSendData[4] = 6&0xff;
			 ucSendData[8] = 1;
			 ucSendData[10] = 1;
			 ucSendData[12] = 0x03;		//Command 
			 ucSendData[13] = 0x33;		//Command ID
			 memcpy(ucSendData+ ucSendData[4] +8,PVP_TAILER,4);

			 udpTest.GetLocalIP(localIP);
			 udpTest.sendudp("192.168.1.189", cDstIP, dstPort, dstPort, 
				 (char*)&ucSendData[0], ucSendData[4] +12);
			 iRevcNum = udpTest.receiveudp("192.168.1.189", cDstIP, dstPort, dstPort, 
				 (char*)&ucrevcData[0], PVP_LENGTH);
			 if (iRevcNum == 18)
			 {
				 if (ucrevcData[13] == 0)
				 {
					 dataGridView1->Rows[0]->Cells[1]->Value = L"正常执行接收数据包";
				 }
				 else
				 {
					 dataGridView1->Rows[0]->Cells[1]->Value = L"非正常执行";
				 }
			 }*/
			 HANDLE hThread;
			 DWORD threadId;
			 int iChNum = 0;

			 if (button6->Text == L"停止播放")
			 {
				 bFlagAVData[iChNum] = false;
				 timer1->Enabled = false;
				 button6->Text = L"频道1播放";
				 CloseHandle(hThread);
				 return;
			 }

			 //bFlagAVData = true;
			 
			 hThread = CreateThread(NULL,	 0, ThreadFunc, &bFlagAVData[iChNum], 0, &threadId );
			 if (hThread == false)
			 {
				 MessageBoxW(NULL, L"启动线程失败！", L"提示",MB_ICONERROR);
				 //MessageBox(NULL, "启动线程失败！", "提示",MB_ICONERROR);
				 return;
			 }
			 bFlagAVData[iChNum] = true;
			 Sleep(1000);
			 if (bFlagAVData[iChNum])
			 {
				 timer1->Enabled = true;
				 button6->Text = L"停止播放";
			 }
			 //----------------------------------------------
			 //FILE *fp_vin, *fp_ain;
			 //int y_size,h264_size, ACC_size,sendnum=0, actDataLen, pktLen, ret;
			 //int v_pnum, v_rmd, a_pnum, a_rmd, max_pnum;
			 //int in_w=704,in_h=576;	
			 //YUVEncoder CVideo(in_w,in_h);
			 //ACCEncoder CAudio;
			 //int framenum=300, Vframecnt = 0,Aframecnt = 0;	
			 //unsigned char *data_ain, *data_vin;
			 //unsigned char data_aout[10000]={0}, data_vout[100000]={0};
			 //char c;
			 //char filename_vin[]= "D://YNZZ_Info//PlayVideo//Code//HKCard_test//HKCard_test//HK_cn1.yuv";
			 //char filename_ain[]= "D://YNZZ_Info//PlayVideo//Code//HKCard_test//HKCard_test//HK_cn1.pcm";
			 //Class_HKcard_4008 test1;

			 //Input raw data
			 //fp_ain = fopen(filename_ain, "rb");
			 //if (!fp_ain) {
				// printf("Could not open %s\n", filename_ain);
				// //return -1;
			 //}
			 //fp_vin = fopen(filename_vin, "rb");
			 //if (!fp_vin) {
				// printf("Could not open %s\n", filename_vin);
				// //return -1;
			 //}

			 //CAudio.initializeCoder();
			 //data_ain = new unsigned char[CAudio.buffer_size];
			 //y_size = in_w * in_h;
			 //data_vin = new unsigned char[y_size*1.5];
			 //CVideo.initializeCoder();
			 //V_pHFile = fopen(VFileName,"wb+");
			 //A_pHFile = fopen(AFileName,"wb+");
			 //MUtest.InitialSock("192.168.1.189","224.0.1.2",7838);
			 //if(test1.initial() > 0)
				// printf("Initialize Card successful!\n");
			 //else
			 //{
				// MessageBoxW(NULL, L"请检查视频卡！", L"提示",MB_ICONERROR);
				// return;
			 //}
			 //test1.openChannel(1,StreamDirectReadCallback, ENC_4CIF_FORMAT);
			 //ret = test1.startDecEncData(1,DecCBFun);
			 //if (ret < 0)
			 //{
				// MessageBoxW(NULL,(LPCWSTR) L"编解码PlayM4错误！"+ret, L"提示",MB_ICONERROR);
			 //}
			 //Read raw YUV data
			 //while (radioButton1->Checked)
			 {
				 ////Read PCM
				 //if (fread(data_ain, 1, CAudio.buffer_size, fp_ain) <= 0){
					// break;	 }
				 //else if(feof(fp_ain)){
					// break;	 }
				 ////Read YUV
				 //if(fread(data_vin,1,int(y_size*1.5),fp_vin)<= 0) {
					// break; }
				 //else if(feof(fp_vin)){
					// break;	 }
				 //ACC_size = CAudio.encodeFrame(data_aout,data_ain);
				 //h264_size = CVideo.encodeFrameYV12(data_vout,data_vin);
				 
				 //if (ADataEnc.bReady)
				 //{
					// for (int i=0;i<5;i++)
					// {
					//	 ACC_size = CAudio.encodeFrame(data_aout, ADataEnc.indata + i*CAudio.buffer_size);
					//	 if (ACC_size > 0)
					//	 {
					//		 //fwrite(data_aout,ACC_size,1,A_pHFile);
					//		 memcpy(ADataEnc.outdata + ADataEnc.outlen, data_aout, ACC_size);
					//		 ADataEnc.outlen += ACC_size;
					//	 }
					//	 ACC_size = 0;
					// }
					// ADataEnc.bReady = false;
				 //}
				
				 //if (VDataEnc.bReady)
				 //{
					//VDataEnc.outlen = 0;
					// for (int i=0;i<5;i++)
					//{
					//	int temp = 1.5*i*y_size;
					//	h264_size = CVideo.encodeFrameYV12(data_vout, VDataEnc.indata + temp);
					//	if (h264_size >0)
					//	{
					//		//fwrite(data_vout,h264_size,1,V_pHFile);
					//		memcpy(VDataEnc.outdata + VDataEnc.outlen, data_vout, h264_size);
					//		VDataEnc.outlen += h264_size;
					//	}
					//	h264_size = 0;
					//}
					// VDataEnc.bReady = false;
				 //}
				 //if (VDataEnc.outlen > 0)
				 //{
					// v_pnum = VDataEnc.outlen/SEND_LENGTH;
					// v_rmd = VDataEnc.outlen%SEND_LENGTH;
					// for (int i=0;i<(v_pnum+1);i++)
					// {
					//	 int actDataLen = 0, vlen=0;
					//	 char	AVData[SEND_LENGTH] ={0};
					//	 if (i < v_pnum)
					//	 {
					//		 memcpy(AVData,VDataEnc.outdata +i*SEND_LENGTH,SEND_LENGTH);
					//		 actDataLen += SEND_LENGTH;
					//		 vlen = SEND_LENGTH;
					//	 }
					//	 if (i == v_pnum && v_rmd != 0)
					//	 {
					//		 memcpy(AVData,VDataEnc.outdata +i*SEND_LENGTH,v_rmd);
					//		 actDataLen += v_rmd;
					//		 vlen = v_rmd;
					//	 }
					//	 memcpy(ucSendData,PVP_HEADER,4);
					//	 pktLen = actDataLen + 6 +16 ;
					//	 ucSendData[4] = pktLen&0xff;
					//	 ucSendData[5] = (pktLen>>8)&0xff;
					//	 ucSendData[6] = (pktLen>>16)&0xff;
					//	 ucSendData[7] = (pktLen>>24)&0xff;
					//	 ucSendData[8] = (v_pnum+1)&0xff;
					//	 ucSendData[10] = (i+1)&0xff;
					//	 ucSendData[12] = 0x03;		//Command 
					//	 ucSendData[13] = 0x33;		//Command ID
					//	 ucSendData[14] = vlen&0xff;
					//	 ucSendData[15] = (vlen>>8)&0xff;
					//	 ucSendData[16] = (vlen>>16)&0xff;
					//	 ucSendData[17] = (vlen>>24)&0xff;
					//	 //ucSendData[22] = (alen)&0xff;
					//	 //ucSendData[23] = (alen>>8)&0xff;
					//	 //ucSendData[24] = (alen>>16)&0xff;
					//	 //ucSendData[25] = (alen>>24)&0xff;
					//	 memcpy(ucSendData+30, AVData, actDataLen);
					//	 memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
					//	 MUtest.sendData(ucSendData,pktLen+12);
					//	 memset(ucSendData,0,PVP_LENGTH);
					// }
					// VDataEnc.outlen =0;
					// Vframecnt +=5;
				 //}
				 //if (ADataEnc.outlen > 0)
				 //{
					//  a_pnum = ADataEnc.outlen/SEND_LENGTH;
					//  a_rmd = ADataEnc.outlen%SEND_LENGTH;
					//  for (int i=0;i<(a_pnum+1);i++)
					//  {
					//	  int actDataLen = 0, alen=0;
					//	  char	AVData[SEND_LENGTH] ={0};
					//	  if (i < a_pnum)
					//	  {
					//		  memcpy(AVData,ADataEnc.outdata +i*SEND_LENGTH,SEND_LENGTH);
					//		  actDataLen += SEND_LENGTH;
					//		  alen = SEND_LENGTH;
					//	  }
					//	  if (i == a_pnum && a_rmd != 0)
					//	  {
					//		  memcpy(AVData,ADataEnc.outdata +i*SEND_LENGTH,a_rmd);
					//		  actDataLen += a_rmd;
					//		  alen = a_rmd;
					//	  }
					//	  memcpy(ucSendData,PVP_HEADER,4);
					//	  pktLen = actDataLen + 6 +16 ;
					//	  ucSendData[4] = pktLen&0xff;
					//	  ucSendData[5] = (pktLen>>8)&0xff;
					//	  ucSendData[6] = (pktLen>>16)&0xff;
					//	  ucSendData[7] = (pktLen>>24)&0xff;
					//	  ucSendData[8] = (a_pnum+1)&0xff;
					//	  ucSendData[10] = (i+1)&0xff;
					//	  ucSendData[12] = 0x03;		//Command 
					//	  ucSendData[13] = 0x33;		//Command ID
					//	  //ucSendData[14] = vlen&0xff;
					//	  //ucSendData[15] = (vlen>>8)&0xff;
					//	  //ucSendData[16] = (vlen>>16)&0xff;
					//	  //ucSendData[17] = (vlen>>24)&0xff;
					//	  ucSendData[22] = (alen)&0xff;
					//	  ucSendData[23] = (alen>>8)&0xff;
					//	  ucSendData[24] = (alen>>16)&0xff;
					//	  ucSendData[25] = (alen>>24)&0xff;
					//	  memcpy(ucSendData+30, AVData, actDataLen);
					//	  memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
					//	  MUtest.sendData(ucSendData,pktLen+12);
					//	  memset(ucSendData,0,PVP_LENGTH);
					//  }
					//  ADataEnc.outlen = 0;
					//  Aframecnt += 5;
				 //}
				 //if (h264_size > 0 || ACC_size >0)
				 //{
					// 
					// v_pnum = h264_size/VIDEO_LENGTH;
					// v_rmd = h264_size%VIDEO_LENGTH;
					// //if(v_rmd != 0) v_pnum +=1;
					// a_pnum = ACC_size/AUDIO_LENGTH;
					// a_rmd = ACC_size%AUDIO_LENGTH;
					// //if(a_rmd != 0) a_pnum +=1;
					// max_pnum = a_pnum > v_pnum ? a_pnum : v_pnum;
					// for (int i=0;i<(max_pnum+1);i++)
					// {
					//	 int actDataLen = 0, vlen=0, alen=0;
					//	 char	AVData[PVP_LENGTH] ={0};
					//	 if (i < v_pnum)
					//	 {
					//		 memcpy(AVData,data_vout +i*VIDEO_LENGTH,VIDEO_LENGTH);
					//		 actDataLen += VIDEO_LENGTH;
					//		 vlen = VIDEO_LENGTH;
					//	 }
					//	 if (i == v_pnum && v_rmd != 0)
					//	 {
					//		 memcpy(AVData,data_vout +i*VIDEO_LENGTH,v_rmd);
					//		 actDataLen += v_rmd;
					//		 vlen = v_rmd;
					//	 }
					//	 if (i < a_pnum)
					//	 {
					//		 memcpy(AVData+vlen,data_aout +i*AUDIO_LENGTH,AUDIO_LENGTH);
					//		 actDataLen += AUDIO_LENGTH;
					//		 alen = AUDIO_LENGTH;
					//	 }
					//	 if (i == a_pnum && a_rmd != 0)
					//	 {
					//		 memcpy(AVData+vlen,data_aout +i*AUDIO_LENGTH,a_rmd);
					//		 actDataLen += a_rmd;
					//		 alen = a_rmd;
					//	 }
					//	memcpy(ucSendData,PVP_HEADER,4);
					//	pktLen = actDataLen + 6 +16 ;
					//	ucSendData[4] = pktLen&0xff;
					//	ucSendData[5] = (pktLen>>8)&0xff;
					//	ucSendData[6] = (pktLen>>16)&0xff;
					//	ucSendData[7] = (pktLen>>24)&0xff;
					//	ucSendData[8] = (max_pnum+1)&0xff;
					//	ucSendData[10] = (i+1)&0xff;
					//	ucSendData[12] = 0x03;		//Command 
					//	ucSendData[13] = 0x33;		//Command ID
					//	ucSendData[14] = vlen&0xff;
					//	ucSendData[15] = (vlen>>8)&0xff;
					//	ucSendData[16] = (vlen>>16)&0xff;
					//	ucSendData[17] = (vlen>>24)&0xff;
					//	ucSendData[22] = (alen)&0xff;
					//	ucSendData[23] = (alen>>8)&0xff;
					//	ucSendData[24] = (alen>>16)&0xff;
					//	ucSendData[25] = (alen>>24)&0xff;
					//	memcpy(ucSendData+30, AVData, actDataLen);
					//	memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
					//	//udpTest.sendudp("192.168.1.189", cDstIP, dstPort, dstPort, (char*)&ucSendData[0], pktLen +12);
					//	MUtest.sendData(ucSendData,pktLen+12);
					//	memset(ucSendData,0,PVP_LENGTH);
					// }
				 //}
				 //memset(data_ain, 0, CAudio.buffer_size);
				 //memset(data_vin, 0 , y_size*1.5);
				 //framecnt++;
				 //toolStripStatusLabel1->Text = L"已发送：" + Convert::ToString(Vframecnt) + L"帧/" + Aframecnt + L"帧";
				 //statusStrip1->Refresh();
			 }
			 //CAudio.closeCoder();
			 //CVideo.closeCoder();
			 //MUtest.closeSock();
			 ////scanf("%c\n",&c);
			 //delete data_ain, data_vin;
			 //fclose(fp_vin);
			 //fclose(fp_ain);
			 //test1.stop(1);

			 //fclose(V_pHFile);
			 //fclose(A_pHFile);
		 }
	private: System::Void dataGridView1_Enter(System::Object^  sender, System::EventArgs^  e) {
				 dataGridView1->Refresh();
			 }
private: System::Void radioButton1_Click(System::Object^  sender, System::EventArgs^  e) {

			 if (radioButton1->Checked)
			 {
				 radioButton1->Checked = false;
				 isShowNotice = false;
			 }
			 else
			 {
				 radioButton1->Checked = true;
				 isShowNotice = true;
			 }
		 }
private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) 
		 {
			 GetFramesStatistics(struEncodeChan[1].hChanHandle, &frame_statistics); 
			 toolStripStatusLabel1->Text = L"已发送：" + Convert::ToString(Vframecnt) + L"帧/" + Aframecnt + L"帧" + frame_statistics.VideoFrames + "/" + frame_statistics.AudioFrames + "/" + frame_statistics.FramesLost + "/" + frame_statistics.QueueOverflow + "/" + frame_statistics.CurBps;
			 statusStrip1->Refresh();

			 if(!button10->Enabled && Vframecnt > 0)
				 button10->Enabled = true;
		 }
private: System::Void comboBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
		 {
			 msclr::interop::marshal_context cxt;

			 strcpy(cLocalIP, cxt.marshal_as<char const*>(comboBox1->SelectedItem->ToString()));
		 }
private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 HANDLE hThread;
			 DWORD threadId;
			 int iChNum = 1;

			 if (button7->Text == L"停止播放")
			 {
				 bFlagAVData[iChNum] = false;
				 timer1->Enabled = false;
				 timer2->Enabled = false;
				 button7->Text = L"频道2播放";
				 button10->Enabled = false;
				 CloseHandle(hThread);
				 av_freep(isAudio);
				 av_freep(isVideo);
				 return;
			 }

			 //bFlagAVData = true;

			 hThread = CreateThread(NULL,	 0, ThreadFunc2, &bFlagAVData[iChNum],	0, &threadId );
			 if (hThread == false)
			 {
				 MessageBoxW(NULL, L"启动线程失败！", L"提示",MB_ICONERROR);
				 //MessageBox(NULL, "启动线程失败！", "提示",MB_ICONERROR);
				 return;
			 }
			 bFlagAVData[iChNum] = true;
			 //Sleep(1000);
			 if (bFlagAVData[iChNum])
			 {
				 timer1->Enabled = true;
				 button7->Text = L"停止播放";
				 isAudio = (MediaQueue *)av_mallocz(sizeof(MediaQueue));		//为预览准备
				 isAudio->pictq_mutex = SDL_CreateMutex();
				 isAudio->pictq_cond  = SDL_CreateCond();
				 isVideo = (MediaQueueVideo *)av_mallocz(sizeof(MediaQueueVideo));
				 isVideo->pictq_mutex = SDL_CreateMutex();
				 isVideo->pictq_cond  = SDL_CreateCond();
				 /*while(isVideo->pictq_size <= QUEUE_SIZE) Sleep(400);
				 button10->Enabled = true;*/
			 }
			
		 }
private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
			 static bool bplayFlag = false;
			 DWORD exitCode1 = 0;

			 if (button8->Text == L"停止播放")
			 {
				 bplayFlag = false;
				 button8->Text = L"文件播放";
				 GetExitCodeThread(hThread_fileplay, &exitCode1);
				 if ( exitCode1 == STILL_ACTIVE )
					CloseHandle(hThread_fileplay);
				 return;
			 }	

			 openFileDialog1->Filter = "video files (*.avi)|*.avi|All files (*.*)|*.*";
			 if ( openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
			 {
				 if ( openFileDialog1->FileName == nullptr )
				 {
					 return;
				 }
			 }
			 else
			 {
				 return;
			 }
			 msclr::interop::marshal_context cxt;
			 strcpy(videofile, cxt.marshal_as<char const*>(openFileDialog1->FileName ));

			 hThread_fileplay = CreateThread(NULL,	 0, ThreadReadVideoFile, &bplayFlag,	0, (LPDWORD)threadId_fileplay);
			 if (hThread_fileplay == false)
			 {
				 MessageBoxW(NULL, L"启动线程失败！", L"提示",MB_ICONERROR);
				 return;
			 }
			 bplayFlag = true;
			 Sleep(10);
			 GetExitCodeThread(hThread_fileplay, &exitCode1);
			 if ( exitCode1 == STILL_ACTIVE )
			 {
				 button8->Text = L"停止播放";
			 }
			 else
			 {
				 CloseHandle(hThread_fileplay);
			 }
			 

		 }
private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e) {

		 }
private: System::Void button10_Click(System::Object^  sender, System::EventArgs^  e) {

			 hThread_VIDPrew1 = CreateThread(NULL,	 0, ThreadPrewVideo1, NULL,	0, (LPDWORD)threadId_VIDPrew1);
			 if (hThread_VIDPrew1 == false)
			 {
				 MessageBoxW(NULL, L"启动线程失败！", L"提示",MB_ICONERROR);
				 return;
			 }
			 //timer2->Enabled = true;
			 //hThread_AUDPrew1 = CreateThread(NULL,	 0, ThreadPrewAudio1, NULL,	0, (LPDWORD)threadId_AUDPrew1);
			 //if (hThread_AUDPrew1 == false)
			 //{
				// MessageBoxW(NULL, L"启动线程失败！", L"提示",MB_ICONERROR);
				// return;
			 //}
		 }
private: System::Void timer2_Tick(System::Object^  sender, System::EventArgs^  e) {
			 SDL_Event event;
			 event.type = REFRESH_EVENT;
			 SDL_PushEvent(&event);
		 }
private: System::Void button11_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			NoticePanelform ^form2 = gcnew NoticePanelform();
			 form2->Show();
		 }
};
}


int thread_exit=0;
int thread_pause=0;
int sfp_refresh_thread(void *opaque){
	thread_exit=0;
	thread_pause=0;

	while (!thread_exit) {
		if(!thread_pause){
			SDL_Event event;
			event.type = REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(1);
	}
	thread_exit=0;
	thread_pause=0;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = REFRESH_EVENT;
	SDL_PushEvent(&event);
	return 0;
}

DWORD WINAPI ThreadPrewVideo1(LPVOID p)
{
	int screen_w=500,screen_h=500;
	const int pixel_w=704,pixel_h=576;
	int iFramelenYUV = pixel_w*pixel_h*1.5;
	uint8_t *buffer = new uint8_t[iFramelenYUV];

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	SDL_Window *screen; 
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0); 
	Uint32 pixformat=0;

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat= SDL_PIXELFORMAT_YV12;  

	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);
	SDL_Rect sdlRect;  
	//FIX: If window is resize
	sdlRect.x = 0;  
	sdlRect.y = 0;  
	sdlRect.w = screen_w;  
	sdlRect.h = screen_h;  

	//SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
	SDL_Event event;

	//SDL_AudioSpec
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = 16000;//44100; 
	wanted_spec.format = AUDIO_S16SYS; 
	wanted_spec.channels = 1; 
	wanted_spec.silence = 0; 
	wanted_spec.samples = 2560;//1024; 
	wanted_spec.callback = fill_audio_ch1; 

	if (SDL_OpenAudio(&wanted_spec, NULL)<0){ 
		printf("can't open audio.\n"); 
		return -1; 
	}
	SDL_LockMutex(isAudio->pictq_mutex);
	isAudio->pictq_size = QUEUE_SIZE_AUDIO;
	SDL_UnlockMutex(isAudio->pictq_mutex);
	//if ((isAudio->pictq_rindex = isAudio->pictq_windex - 5) < 0)		//延时X批音频数据播放
	//{
	//	isAudio->pictq_rindex += QUEUE_SIZE;
	//}
	isAudio->pictq_windex = 0;
	isAudio->pictq_rindex = 0;
	SDL_PauseAudio(0);	//play audio
	
	//isVideo->pictq_windex = 0;
	//isVideo->pictq_rindex = 0;
	if((isVideo->pictq_rindex = (isVideo->pictq_windex - 6)) < 0)		//延时X帧数据播放:实测经验值
		isVideo->pictq_rindex += QUEUE_SIZE_VIDEO;
	SDL_LockMutex(isVideo->pictq_mutex);
	isVideo->pictq_size = QUEUE_SIZE_VIDEO;							//初始化保存15帧
	SDL_UnlockMutex(isVideo->pictq_mutex);
	SDL_AddTimer(10, sdl_refresh_timer_cb, NULL);	//play video
	//int iChNum = 1;	
	while(1){
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==REFRESH_EVENT){
			int64_t delay,stp =  av_gettime();
			RawDataVideo* vtemp = &isVideo->dataq[isVideo->pictq_rindex];
			if (isVideo->pictq_size > QUEUE_SIZE_VIDEO)	//视频延时
			{
				fprintf(fvideolog, "playing ---- windex = %d\t rindex = %d\t showclock = %d\n", 
					isVideo->pictq_windex, isVideo->pictq_rindex, showclock);
				memcpy(buffer,vtemp->data, vtemp->len);
				if (++isVideo->pictq_rindex == QUEUE_SIZE_VIDEO)
				{
					isVideo->pictq_rindex = 0;
				}
				SDL_LockMutex(isVideo->pictq_mutex);
				isVideo->pictq_size--;
				SDL_UnlockMutex(isVideo->pictq_mutex);
			}
			else{
				SDL_AddTimer(1, sdl_refresh_timer_cb, NULL);
				continue;
			}
			SDL_UpdateTexture( sdlTexture, NULL, buffer, pixel_w);  

			SDL_RenderClear( sdlRenderer );   
			SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);  
			SDL_RenderPresent( sdlRenderer );
			//SDL_Delay(30);
			
			delay = (av_gettime() - stp)/1000;
			fprintf(fvideolog, "playing ---- size = %d\t usedtime = %d\n", isVideo->pictq_size, delay);
			if (delay >= 40)		//立即刷新
			{
				SDL_AddTimer(1, sdl_refresh_timer_cb, NULL);
			}
			else
			{
				if (isVideo->pictq_size >= QUEUE_SIZE_AUDIO + 4)			//防止视频重叠
				{
					SDL_AddTimer(1, sdl_refresh_timer_cb, NULL);
				}else if (isVideo->pictq_size >= QUEUE_SIZE_AUDIO + 3)
				{
					SDL_AddTimer(5, sdl_refresh_timer_cb, NULL);
				}
				else if (isVideo->pictq_size >= QUEUE_SIZE_AUDIO + 2)
				{
					SDL_AddTimer(10, sdl_refresh_timer_cb, NULL);
				}
				else if (isVideo->pictq_size >= QUEUE_SIZE_AUDIO + 1)
				{
					SDL_AddTimer(15, sdl_refresh_timer_cb, NULL);
				}
				else{
					SDL_AddTimer((40-delay), sdl_refresh_timer_cb, NULL);
				}
			}		

		}else if(event.type==SDL_KEYDOWN){
			//Pause
			if(event.key.keysym.sym==SDLK_SPACE)
				thread_pause=!thread_pause;
		}else if(event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				int w, h, x, y;
				SDL_GetWindowSize(screen, &w, &h);
				sdlRect.w = w;
				sdlRect.h = h;
				SDL_GetWindowPosition(screen, &x, &y);
				sdlRect.x = 0;
				sdlRect.y = 0;
			}
			
			//SDL_SetWindowSize(screen, event.window., event.resize.h);
		}
		else if(event.type==SDL_QUIT){
			thread_exit=1;
			fprintf(fvideolog, "exiting..\n");
			break;
		}else if(event.type==BREAK_EVENT){
			break;
		}
		//if (bPrewAudio)
		//{
		//	bPrewAudio = false;
		//	audio_len_ch1 = ADataEnc[iChNum].inlen;
		//	audio_pos_ch1 = ADataEnc[iChNum].indata;
		//	//Play
		//	SDL_PauseAudio(0);
		//}
	}
	SDL_Quit();
	fclose(fvideolog);
	fclose(faudiolog);
	delete[] buffer;
}
 
/* Audio Callback
 * The audio function callback takes the following parameters: 
 * stream: A pointer to the audio buffer to be filled 
 * len: The length (in bytes) of the audio buffer 
 * 
*/ 

void  fill_audio_ch1(void *udata,Uint8 *stream,int len){ 
	//SDL 2.0
	RawData *ap;
	SDL_memset(stream, 0, len);
	//if(audio_len_ch1==0)		/*  Only  play  if  we  have  data  left  */ 
	//		return; 
	//len=(len>audio_len_ch1?audio_len_ch1:len);	/*  Mix  as  much  data  as  possible  */ 
	if (isAudio->pictq_size > QUEUE_SIZE_AUDIO)		//音频延迟
	{
		ap = &isAudio->dataq[isAudio->pictq_rindex];
		SDL_MixAudio(stream,ap->data,ap->len,SDL_MIX_MAXVOLUME);
		if (++isAudio->pictq_rindex == QUEUE_SIZE_AUDIO)
			isAudio->pictq_rindex = 0;
		SDL_LockMutex(isAudio->pictq_mutex);
		isAudio->pictq_size--;
		SDL_UnlockMutex(isAudio->pictq_mutex);
		fprintf(faudiolog, "size = %d\t windex = %d\t rindex = %d\n", isAudio->pictq_size, isAudio->pictq_windex, isAudio->pictq_rindex);
	}
	
	//audio_pos_ch1 += len; 
	//audio_len_ch1 -= len; 
} 
DWORD WINAPI ThreadPrewAudio1(LPVOID p)
{

	//SDL_AudioSpec
	//SDL_AudioSpec wanted_spec;
	//wanted_spec.freq = 16000;//44100; 
	//wanted_spec.format = AUDIO_S16SYS; 
	//wanted_spec.channels = 1; 
	//wanted_spec.silence = 0; 
	//wanted_spec.samples = 2560;//1024; 
	//wanted_spec.callback = fill_audio_ch1; 

	//if (SDL_OpenAudio(&wanted_spec, NULL)<0){ 
	//	printf("can't open audio.\n"); 
	//	return -1; 
	//}
	int iChNum = 1;
	while(!thread_exit)
	{
		if (bPrewAudio)
		{
			bPrewAudio = false;
			audio_len_ch1 = ADataEnc[iChNum].inlen;
			audio_pos_ch1 = ADataEnc[iChNum].indata;
			//Play
			SDL_PauseAudio(0);
		}
		
		//while(audio_len_ch1>0)//Wait until finish
		//	SDL_Delay(1); 
	}

	//SDL_Quit();
	return 0;
}

DWORD WINAPI ThreadReadVideoFile(LPVOID p)
{
	bool *bflag = (bool *)p;
	uint8_t ucSendData[PVP_LENGTH] = {0};
	MulticastUDP MUtest;

	AVFormatContext *ifmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex=-1,audioindex=-1;
	char *out_filename_v = "cuc_ieschool.h264";//输出文件名（Output file URL）
	char *out_filename_a = "cuc_ieschool.mp3";
  
	ret = MUtest.InitialSock(cLocalIP,"224.0.1.2",7838);		//CH0
	if (ret < 0)
	{
		MessageBoxW(NULL, L"setsockopt failed!请设置本机IP", L"提示",MB_ICONERROR);
		ExitThread(1);
	}
	const int DSTWIDTH = 704, DSTHEIGHT = 576;
	H264DecodeYUV h264dectest(DSTWIDTH,DSTHEIGHT);
	uint8_t *dataYUV = new uint8_t[DSTWIDTH*DSTHEIGHT*2];
	uint8_t *datah264 = new uint8_t[100000];
	//YUVEncoder CVideo(DSTWIDTH,DSTHEIGHT);
	int h264_size=0, YUVlen;
	Mp3Decoder audiotest;
	uint8_t *PCMdata = new uint8_t[10240];
	int PCMlen =0;
	double audiousedtime = 0;


	FILE *fp_audio=fopen(out_filename_a,"wb+");  
	FILE *fp_video=fopen(out_filename_v,"wb+");
	av_register_all();
	//输入（Input）
	if ((ret = avformat_open_input(&ifmt_ctx, videofile, 0, 0)) < 0) {
		printf( "Could not open input file.");
		return -1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf( "Failed to retrieve input stream information");
		return -2;
	}

	videoindex=-1;
	for(i=0; i<ifmt_ctx->nb_streams; i++) {
		if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
		}else if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			audioindex=i;
		}
	}
	//Dump Format------------------
	printf("\nInput Video===========================\n");
	av_dump_format(ifmt_ctx, 0, videofile, 0);
	printf("\n======================================\n");
	int fps = av_q2d(ifmt_ctx->streams[0]->avg_frame_rate);	//帧率
	//实际解码得到的帧数与264数据的帧数----我猜的
	double cof = 1.0*ifmt_ctx->streams[0]->nb_frames/ifmt_ctx->streams[0]->nb_index_entries;		//
	double VideoGap = cof*1000.0/fps;

	YUVEncoder CVideo(DSTWIDTH, DSTHEIGHT, fps);
	/*
	FIX: H.264 in some container format (FLV, MP4, MKV etc.) need 
	"h264_mp4toannexb" bitstream filter (BSF)
		*Add SPS,PPS in front of IDR frame
		*Add start code ("0,0,0,1") in front of NALU
	H.264 in some container (MPEG2TS) don't need this BSF.
	*/
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif

start:
	while( *bflag)
	{
		clock_t start_clock = clock();
		if(av_read_frame(ifmt_ctx, &pkt)<0) 
		{
			av_seek_frame(ifmt_ctx, 0, 0, 0);
			continue;
		}
		memset(ucSendData,0,PVP_LENGTH);	//清空发送buffer
		if(pkt.stream_index==videoindex){
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, ifmt_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
			//printf("Write Video Packet. size:%d\tpts:%d\n",pkt.size,pkt.pts);
			YUVlen = h264dectest.decodeFrametoYUV420_scale((char *)dataYUV,pkt.data,pkt.size);
			if (YUVlen > 0)
			{
				h264_size = CVideo.encodeFrame(datah264, dataYUV);
				if (h264_size > 0)
				{
					fwrite(datah264,1,h264_size,fp_video);
				}
			}		
			//int h264len = pkt.size;
			if (h264_size > 0)
			{
				char	AVData[SEND_LENGTH] ={0};
				int actDataLen = 0, vlen=0;
				int v_pnum = h264_size/SEND_LENGTH;
				int v_rmd = h264_size%SEND_LENGTH;
				memcpy(ucSendData,PVP_HEADER,4);
				ucSendData[4] = h264_size&0xff;
				ucSendData[5] = (h264_size>>8)&0xff;
				ucSendData[6] = (h264_size>>16)&0xff;
				ucSendData[7] = (h264_size>>24)&0xff;
				ucSendData[8] = (v_pnum+1)&0xff;
				ucSendData[12] = 0x03;		//Command 
				ucSendData[13] = 0x33;		//Command ID
				for (int i=0;i<(v_pnum+1);i++)
				{
					if (i < v_pnum)
					{
						memcpy(AVData,datah264 +i*SEND_LENGTH,SEND_LENGTH);
						//actDataLen += SEND_LENGTH;
						vlen = SEND_LENGTH;
					}
					if (i == v_pnum && v_rmd != 0)
					{
						memcpy(AVData,datah264 +i*SEND_LENGTH,v_rmd);
						//actDataLen += v_rmd;
						vlen = v_rmd;
					}
					ucSendData[10] = (i+1)&0xff;
					
					ucSendData[14] = vlen&0xff;
					ucSendData[15] = (vlen>>8)&0xff;
					//ucSendData[16] = (vlen>>16)&0xff;
					//ucSendData[17] = (vlen>>24)&0xff;
					//ucSendData[22] = (alen)&0xff;
					//ucSendData[23] = (alen>>8)&0xff;
					//ucSendData[24] = (alen>>16)&0xff;
					//ucSendData[25] = (alen>>24)&0xff;
					memcpy(ucSendData+30, AVData, vlen);
					int pktLen = vlen + 6 +16 ;
					memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
					MUtest.sendData(ucSendData,pktLen+12);
				}
			}
			double ttmp = (clock() - start_clock)*1000/(double)CLOCKS_PER_SEC;
			while( ttmp < (VideoGap - audiousedtime))
			{
				ttmp = (clock() - start_clock)*1000/(double)CLOCKS_PER_SEC;
				Sleep(1);
			}
			audiousedtime = 0.0;
		}else if(pkt.stream_index==audioindex){
			/*
			AAC in some container format (FLV, MP4, MKV etc.) need to add 7 Bytes
			ADTS Header in front of AVPacket data manually.
			Other Audio Codec (MP3...) works well.
			*/
			//printf("Write Audio Packet. size:%d\tpts:%d\n",pkt.size,pkt.pts);
			PCMlen = audiotest.decodeFrame(PCMdata,pkt.data,pkt.size);
			//if (PCMlen > 0)
			//{
			//	fwrite(PCMdata,1,PCMlen,fp_audio);
			//}
			if (PCMlen > 0)
			{
				char	AVData[SEND_LENGTH] ={0};
				int alen=0;
				int a_pnum = PCMlen/SEND_LENGTH;
				int a_rmd = PCMlen%SEND_LENGTH;
				memcpy(ucSendData,PVP_HEADER,4);
				ucSendData[4] = PCMlen&0xff;
				ucSendData[5] = (PCMlen>>8)&0xff;
				ucSendData[6] = (PCMlen>>16)&0xff;
				ucSendData[7] = (PCMlen>>24)&0xff;
				ucSendData[8] = (a_pnum+1)&0xff;
				ucSendData[12] = 0x03;		//Command 
				ucSendData[13] = 0x33;		//Command ID
				for (int i=0;i<(a_pnum+1);i++)
				{
					if (i < a_pnum)
					{
						memcpy(AVData, PCMdata +i*SEND_LENGTH, SEND_LENGTH);
						alen = SEND_LENGTH;
					}
					if (i == a_pnum && a_rmd != 0)
					{
						memcpy(AVData, PCMdata +i*SEND_LENGTH, a_rmd);
						alen = a_rmd;
					}
					ucSendData[10] = (i+1)&0xff;
					ucSendData[22] = (alen)&0xff;
					ucSendData[23] = (alen>>8)&0xff;
					ucSendData[24] = (alen>>16)&0xff;
					ucSendData[25] = (alen>>24)&0xff;
					memcpy(ucSendData+30, AVData, alen);
					int pktLen = alen + 6 +16 ;
					memcpy(ucSendData+ pktLen +8,PVP_TAILER,4);
					MUtest.sendData(ucSendData,pktLen+12);
				}
			}
			clock_t end_clock = clock();
			audiousedtime += (end_clock- start_clock)*1000/(double)CLOCKS_PER_SEC;
		}
		av_free_packet(&pkt);
	}
	
 //	if (*bflag == true)
	//{
	//	goto start;
	//}


#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);  
#endif

	avformat_close_input(&ifmt_ctx);

	fclose(fp_video);
	fclose(fp_audio);

	MUtest.closeSock();
	h264dectest.closeDecoder();
	audiotest.close();
	delete[] dataYUV;
	delete[] datah264;
	delete[] PCMdata;


	if (ret < 0 && ret != AVERROR_EOF) {
		printf( "Error occurred.\n");
		return -3;
	}
	return 1;
}

char initScrollText(char *cstxt, int x, int y)
{
	O_STRINGATTR g_oAttrTop;

	OSD_Init("HZK16.dat","ASCII.dat");
	//初始化属性
	g_oAttrTop.sizeW=32;
	g_oAttrTop.sizeH=32;
	g_oAttrTop.eActionType=e_SCROLL_LEFT;//e_STATIC;
	g_oAttrTop.actionValue1=1;//移动速度
	g_oAttrTop.actionValue2=2;//移动方向及间隔
	g_oAttrTop.osdR=224;
	g_oAttrTop.osdG=224;
	g_oAttrTop.osdB=224;
	//g_oAttrTop.font = "宋体";

	OSD_CreateObjCharObj(0,cstxt,g_oAttrTop);//创建顶端字符对象
	OSD_SetPositionCharObj(0,x,y);//设置叠加的位置

	return ERR_NONE;
}

