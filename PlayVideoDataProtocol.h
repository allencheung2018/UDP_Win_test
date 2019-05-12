/*********************************************************
//
//@FileName          PLAYVIDEODATAPROTOCOL.h
//@Creator              Allen
//@version              v1.0
//@Comment          Define PlayVideo Protocol.
//
**********************************************************/
#ifndef PLAYVIDEODATAPROTOCOL
#define PLAYVIDEODATAPROTOCOL

#include <string.h>

const int							PVP_LENGTH = 1024;								//用户数据包的长度、加上包头实际为1066字节
const int							VIDEO_LENGTH = 800;
const int							AUDIO_LENGTH = 190;
const int							SEND_LENGTH = 990;
const unsigned char		PVP_HEADER[4] = {0x48,0x72,0x49,0x3c};
const unsigned char		PVP_TAILER[4] = {0x3e,0x68,0x52,0x69};

/*********************************************************
*
* @brief							机顶盒播放器协议解析
* @param inbuf				要写入的数据
* @param outbuf			数据块的大小
* @param inlen				输入数据字节数
* @return							返回输出数据包的字节数
* @remark						输入数据长度20~1024
*
**********************************************************/
int parseMessage(char* inbuf, char* outbuf, int inlen);

/*********************************************************
*
* @brief							机顶盒播放器多媒体数据分包发送
* @param indata				要发送的数据
* @param inlen				数据字节数
* @return							返回发送的数据包数
* @remark						输入数据长度大于1000就分包
*
**********************************************************/
int sendMediaData(char* inVideoData, int inVideoLen, char* inAudioData, int inAudioLen);

#endif	//PLAYVIDEODATAPROTOCOL