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

const int							PVP_LENGTH = 1024;								//�û����ݰ��ĳ��ȡ����ϰ�ͷʵ��Ϊ1066�ֽ�
const int							VIDEO_LENGTH = 800;
const int							AUDIO_LENGTH = 190;
const int							SEND_LENGTH = 990;
const unsigned char		PVP_HEADER[4] = {0x48,0x72,0x49,0x3c};
const unsigned char		PVP_TAILER[4] = {0x3e,0x68,0x52,0x69};

/*********************************************************
*
* @brief							�����в�����Э�����
* @param inbuf				Ҫд�������
* @param outbuf			���ݿ�Ĵ�С
* @param inlen				���������ֽ���
* @return							����������ݰ����ֽ���
* @remark						�������ݳ���20~1024
*
**********************************************************/
int parseMessage(char* inbuf, char* outbuf, int inlen);

/*********************************************************
*
* @brief							�����в�������ý�����ݷְ�����
* @param indata				Ҫ���͵�����
* @param inlen				�����ֽ���
* @return							���ط��͵����ݰ���
* @remark						�������ݳ��ȴ���1000�ͷְ�
*
**********************************************************/
int sendMediaData(char* inVideoData, int inVideoLen, char* inAudioData, int inAudioLen);

#endif	//PLAYVIDEODATAPROTOCOL