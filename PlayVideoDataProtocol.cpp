/*********************************************************
//
//@FileName       	PlayVideoDataProtocol.cpp
//@Creator           	Allen
//@version            	v0.1
//@Comment			电视盒播放器通信协议
									
//
**********************************************************/
#include "stdafx.h"
#include "PlayVideoDataProtocol.h"
#include "UDPWin.h"


/*********************************************************
*
* @func parseMessage
*
**********************************************************/
int parseMessage(char* inbuf, char* outbuf, int inlen)
{
	unsigned char		ucCommand = 0x0;
	int		len =0,outlen =0;

	ucCommand = inbuf[12];
	switch (ucCommand)
	{
	case 0x01 :							// 设备探测
		len = 62;
		outbuf[4] = len&0xff;
		outbuf[8] = 1;
		outbuf[10] = 1;
		outbuf[12] = inbuf[13];
		outbuf[13] = 0;
		outbuf[36] = 0;
		break;
	case 0x02 :							//切换频道
		len = 10;
		outbuf[4] = len&0xff;
		outbuf[8] = 1;
		outbuf[10] = 1;
		outbuf[12] = inbuf[13];
		outbuf[13] = 0;
		outbuf[14] = 5;
		outbuf[16] = 6;
		break;
	case 0x03:							//数据包
		len = 6;
		outbuf[4] = len&0xff;
		outbuf[8] = 1;
		outbuf[10] = 1;
		outbuf[12] = inbuf[13];
		outbuf[13] = 0;
		break;
	case 0x04:							//开机/休眠
		len = 6;
		outbuf[4] = len&0xff;
		outbuf[8] = 1;
		outbuf[10] = 1;
		outbuf[12] = inbuf[13];
		outbuf[13] = 0;
		break;
	default:

		break;
	}
	outlen = len + 12;
	memcpy(outbuf,inbuf,4);
	memcpy(outbuf+outlen-4,inbuf+inlen-4,4);
	
	return outlen;
}
/*********************************************************
*
* @func sendMediaData
*
**********************************************************/
int sendMediaData(char* inVideoData, int inVideoLen, char* inAudioData, int inAudioLen)
{
	int VPknum=0, VPksize=0,APknum=0,APksize=0, Sendnum;
	char sendData[PVP_LENGTH];
	UDPWin udpTest;

	if ((inVideoLen + inAudioLen) > PVP_LENGTH)
	{
		VPknum  = inVideoLen/VIDEO_LENGTH;
		VPksize = inVideoLen%VIDEO_LENGTH;
		Sendnum = max(VPknum, APknum);
	}
	else
	{
		Sendnum = 1;
	}
	for (int i=0;i<Sendnum;i++)
	{
		memcpy(sendData, inVideoData+i*VIDEO_LENGTH,VIDEO_LENGTH);

	}

	return 1;
}
