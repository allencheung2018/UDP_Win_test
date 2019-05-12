#ifndef _OSDCHAR_H_
#define _OSDCHAR_H_

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

//������
#define ERR_NONE 0x00
#define ERR_OBJEXIST 0x01
#define ERR_OBJNOTEXIST 0x02
#define ERR_FILEOPENFAIL 0x03
#define ERR_PARA 0x04

//�ַ�����Ϊ����
typedef enum {
	e_SCROLL_LEFT,e_SCROLL_RIGHT,e_SCROLL_UP,e_SCROLL_DOWN,e_MOVE_RAND,e_STATIC,e_SLOWHIDE,e_SPARK	
}E_ACTIONTYPE;
//��Ϊ���
/*
e_ROLL:
   -val����������������ٶ�
   +val�����ҹ����������ٶ�
e_STATIC:
   ����Ϊ��
e_SLOWHIDE:
   ͣ��ʱ��
e_SPARK:
   ��˸���
*/
//�����ַ�����������
typedef struct _O_STRINGATTR{
	char osdR,osdG,osdB;//�ַ���ɫ
	char font;//��������
	char sizeW,sizeH;//�ַ������С��ֻ��Ϊ16*16����32*32
	E_ACTIONTYPE eActionType;//��Ϊ����
	int actionValue1;//��Ϊ��
	int actionValue2;//��Ϊ��
	
	/*
	_O_STRINGATTR()
	{
		osdR=255;
		osdG=255;
		osdB=255;
	}
	*/
	
}O_STRINGATTR,*PO_STRINGATTR,*LPO_STRINGATTR; 

//���ӵ��ַ�������
typedef struct _O_OBJCHAR{
	int x,y,w,h;//���ӵ�λ��
	char szStr[1024];//Ҫ���ӵ��ַ�
	O_STRINGATTR oAttrChar;//����

}O_OBJCHAR,*PO_OBJCHAR,*LPO_OBJCHAR;

//Ҫ���ӵ�ͼ��Դ�ķֱ���
#define  IMAGEWIDTH     704
#define  IMAGEHEIGHT    576
//�ӿں���
extern char OSD_CreateObjCharObj(int strID,char *szStr,O_STRINGATTR OAttrCharObj);
extern char OSD_DeleteObjCharObj(int strID);
extern char OSD_SetContentCharObj(int strID,char *szStr);
extern char OSD_SetPositionCharObj(int strID,int x,int y);
extern char OSD_SetAttrCharObj(int strID,O_STRINGATTR OAttrCharObj);
extern int OSD_FeedFrameYUV420(unsigned char * pYUV420Frame,int iSrcWidth, int iSrcHeight);
extern char OSD_Init(char *szPathHZK,char *szPathASCII);
extern void OSD_Release();
#endif