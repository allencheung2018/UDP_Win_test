/*******************************************************************************
* File  name	 : Osdchar.c
* Description    : ʵ��yuv�ϵ���Ļ���ӣ�ͬʱ֧�ֹ�������ɫ�任������������С�ɵ��ȹ���.
					���ļ�Ϊ��C�ļ����������������⼰����ϵͳ���á���ʵ�ֿ�ƽ̨���ܡ�
* Wrote by/Date  : gaoc@devison.com/2010.02.27
* Modify/Date    :
* Project		 : V30E
*******************************************************************************/
#include "StdAfx.h"
#include "Osdchar.h"

//�������
unsigned char srcRGBBuf[IMAGEWIDTH*IMAGEHEIGHT*3];
unsigned char dstRGBBuf[IMAGEWIDTH*IMAGEHEIGHT*3];
unsigned char srcYUVBuf[IMAGEWIDTH*IMAGEHEIGHT*3/2];
unsigned char dstYUVBuf[IMAGEWIDTH*IMAGEHEIGHT*3/2];
unsigned char subYUVBuf[IMAGEWIDTH*IMAGEHEIGHT*3/2];

//ת����ռ�
long int crv_tab[256];
long int cbu_tab[256];
long int cgu_tab[256];
long int cgv_tab[256];
long int tab_76309[256];
unsigned char clp[1024];

//ȫ����
char		g_szDefault[128]="���ڴ˴�����ȱʡ��������";
O_OBJCHAR	*g_pAllObjCharObj[64];
char		*g_szAllCharObj[64];
FILE		*g_fpHZKLIB = NULL;
FILE		*g_fpASCII = NULL;
unsigned int g_frameCounter=0;

//ת������
#define MY(a,b,c) (( a*  0.2989  + b*  0.5866  + c*  0.1145))
#define MU(a,b,c) (( a*(-0.1688) + b*(-0.3312) + c*  0.5000 + 128))
#define MV(a,b,c) (( a*  0.5000  + b*(-0.4184) + c*(-0.0816) + 128))
//��С�ж�
#define DY(a,b,c) (MY(a,b,c) > 255 ? 255 : (MY(a,b,c) < 0 ? 0 : MY(a,b,c)))
#define DU(a,b,c) (MU(a,b,c) > 255 ? 255 : (MU(a,b,c) < 0 ? 0 : MU(a,b,c)))
#define DV(a,b,c) (MV(a,b,c) > 255 ? 255 : (MV(a,b,c) < 0 ? 0 : MV(a,b,c)))
//ת������
double YuvToRgb[3][3]= {1, 0, 1.4022,
                        1, -0.3456,-0.7145,
                        1, 1.771, 0};
/*******************************************************************************
˵��:
	1.���к���ǰ������
	2.��OSD_XXX(XX,XX,...)��ʽ���ֵĺ�����Ϊ�ӿں���
	3.��_XXX(XX,XX,...)��ʽ���ֵĺ�����Ϊ˽�к���
	4.��p_XXX��ʽ�ľ�Ϊָ��������g_XXX��ʽ���ֵľ�Ϊȫ����

WroteBy/Date:
	gaoc@Dvision.com/2010.03.05

Modify:
*******************************************************************************/
//�ӿں���
char OSD_CreateObjCharObj(int strID,char *szStr,O_STRINGATTR OAttrCharObj);
char OSD_DeleteObjCharObj(int strID);
char OSD_SetContentCharObj(int strID,char *szStr);
char OSD_SetPositionCharObj(int strID,int x,int y);
char OSD_SetAttrCharObj(int strID,O_STRINGATTR OAttrCharObj);
int OSD_FeedFrameYUV420(unsigned char* pYUV420Frame,int iSrcWidth, int iSrcHeight);
char OSD_Init(char *szPathHZK,char *szPathASCII);
void OSD_Release();
//�ڲ�����
void _InitDitherTab();
void _RGB24ToYUV420(unsigned char *RGB, int nWidth,int nHeight, //Դ
					unsigned char *YUV, unsigned long nLen);//Ŀ��
void _YUV420ToRGB24(unsigned char *src_yuv, //Դ
					unsigned char *dst_rgb, int width, int height);//Ŀ��
void _YUV420ToRGB24_reDraw(unsigned char *src_yuv, //Դ
	unsigned char *dst_rgb, int width, int height, bool isRedraw);//Ŀ��
void _YUV420ToYUV422(char* pYUV420Buf, int iSrcWidth, int iSrcHeight, //Դ
					 char* pYUV422Buf);//Ŀ��
void _YUV422ToYUV420(char* pYUV422Buf, int iSrcWidth, int iSrcHeight, //Դ
					 char* pYUV420Buf);//Ŀ��
void _GetSubReginFromYUV420(unsigned char *src_yuv, int srcW, int srcH, //yuvԴͼ
					   unsigned char *sub_yuv, int x, int y, int subW, int subH); //yuv������
void _SetSubReginToYUV420(unsigned char *src_yuv, int srcW, int srcH, //yuvԴͼ
						  unsigned char *sub_yuv, int x, int y, int subW, int subH); //yuv������
void _OverlapCaptionOnRGB(unsigned char* srcRgbBuf, int nWidth,int nHeight,//rgbԴͼ
						  char* pCaption, O_STRINGATTR *pOAttrCharObj);//Ҫ���ӵ����֣��������Լ����
void _OverlapCaptionOnRGB_16to32(unsigned char* srcRgbBuf, int nWidth,int nHeight,//rgbԴͼ
	char* pCaption, O_STRINGATTR *pOAttrCharObj);//Ҫ���ӵ����֣��������Լ����
void _OverlapCaptionOnRGB_32(unsigned char* srcRgbBuf, int nWidth,int nHeight,//rgbԴͼ
	char* pCaption, O_STRINGATTR *pOAttrCharObj);//Ҫ���ӵ����֣��������Լ����
char _OverLapCaptionOnYUV420(unsigned char *src_yuv, int srcW, int srcH,//Դͼ�����
							 int xStart, int yStart, int dstW, int dstH, //Ҫ���ӵ����ּ�����
							 char* pCaption,O_STRINGATTR *pOAttrCharObj);//Ҫ���ӵ����ּ���������
void _OverLapCaptionOnYUV422Raw(char* pCharcode,int column, int row, int imageWidth, int imageHeight, char *pYUVbuffer,
								char OsdY,char OsdU,char OsdV);


/*******************************************************************************
˵��:
	������OSD_XXX(XX,XX,...)��ʽ���ֵĺ�����Ϊ�ӿں���

WroteBy/Date:
	gaoc@Dvision.com/2010.03.05

Modify:
*******************************************************************************/
//�������ӵ��ַ������������ַ�������
char OSD_CreateObjCharObj(int strID,char *szStr,O_STRINGATTR OAttrCharObj)
{
	int temp = strlen(szStr);
	szStr=strlen(szStr)==0?g_szDefault:szStr;
		
	if(g_pAllObjCharObj[strID]==NULL){
		g_pAllObjCharObj[strID]=(O_OBJCHAR *)malloc(sizeof(O_OBJCHAR));
		assert(g_pAllObjCharObj[strID]);
		strcpy(g_pAllObjCharObj[strID]->szStr,szStr);
		g_pAllObjCharObj[strID]->oAttrChar=OAttrCharObj;
		g_pAllObjCharObj[strID]->w=g_pAllObjCharObj[strID]->oAttrChar.sizeW;
		g_pAllObjCharObj[strID]->h=g_pAllObjCharObj[strID]->oAttrChar.sizeH;
	}else{
		return ERR_OBJEXIST;//���ش����룬�����Ѿ�����
	}
		
	return ERR_NONE;
}
//ɾ�����ӵ��ַ�������
char OSD_DeleteObjCharObj(int strID)
{
	if(g_pAllObjCharObj[strID]==NULL)
		return ERR_OBJNOTEXIST;//���ش����룬���󲻴���
	else{
		free(g_pAllObjCharObj[strID]);
		g_pAllObjCharObj[strID]=NULL;
	}
	
	return ERR_NONE;
}
//�ı���ַ���������
char OSD_SetAttrCharObj(int strID,O_STRINGATTR OAttrCharObj)
{
	if(g_pAllObjCharObj[strID]==NULL)
		return ERR_OBJNOTEXIST;//���ش����룬���󲻴���
	else{
		g_pAllObjCharObj[strID]->oAttrChar=OAttrCharObj;
	}
	
	return ERR_NONE;	
}
//�ı��ַ���������
char OSD_SetContentCharObj(int strID,char *szStr)
{
	if(g_pAllObjCharObj[strID]==NULL)
		return ERR_OBJNOTEXIST;//���ش����룬���󲻴���
	else{
		strcpy(g_pAllObjCharObj[strID]->szStr,szStr);
	}
	
	return ERR_NONE;	
}
//�ı��ַ�������λ��
char OSD_SetPositionCharObj(int strID,int x,int y)
{
	if(g_pAllObjCharObj[strID]==NULL)
		return ERR_OBJNOTEXIST;//���ش����룬���󲻴���
	else{
		g_pAllObjCharObj[strID]->x=x;
		g_pAllObjCharObj[strID]->y=y;
		//�趨���
		g_pAllObjCharObj[strID]->w = 32;
		g_pAllObjCharObj[strID]->h = 32;
	}
	
	return ERR_NONE;
}
//��������Ҫ���ӵ��ַ���ͨ���ú���װ��ͼ�������֡
int OSD_FeedFrameYUV420(unsigned char* pYUV420Frame,int iSrcWidth, int iSrcHeight)
{
	int i=0,temp;
	g_frameCounter++;

	//�����б���������ַ�������
	for(i=0;i<1;i++){
		//������������һ��
		if(g_pAllObjCharObj[i]==NULL)
			continue;
		
		switch(g_pAllObjCharObj[i]->oAttrChar.eActionType){
			case e_SCROLL_LEFT://����ǹ�����Ļ,����Ҫ�޸������е���λ�ò���
				temp = strlen(g_pAllObjCharObj[i]->szStr);
				g_pAllObjCharObj[i]->x=g_frameCounter%
					(g_pAllObjCharObj[i]->oAttrChar.actionValue1)==0?
					g_pAllObjCharObj[i]->x-g_pAllObjCharObj[i]->oAttrChar.actionValue2:
				g_pAllObjCharObj[i]->x;
				//�˴�Ϊ����߶ȵ�һ��
				if (abs(g_pAllObjCharObj[i]->x) >= temp*g_pAllObjCharObj[i]->h/2
					&& g_pAllObjCharObj[i]->x < 0)
				{
					temp = 0;
				}
				//if (g_pAllObjCharObj[i]->x <= 0)
				//{
				//	OSD_SetContentCharObj(0,g_pAllObjCharObj[i]->szStr+2);
				//	g_pAllObjCharObj[i]->x = 16;
				//}
				break;
			case e_SCROLL_RIGHT://����ǹ�����Ļ,����Ҫ�޸������е���λ�ò���
				g_pAllObjCharObj[i]->x=g_frameCounter%
					(g_pAllObjCharObj[i]->oAttrChar.actionValue1)==0?
					g_pAllObjCharObj[i]->x+g_pAllObjCharObj[i]->oAttrChar.actionValue2:
				g_pAllObjCharObj[i]->x;
				break;
			case e_SCROLL_UP://����ǹ�����Ļ,����Ҫ�޸������е���λ�ò���
				g_pAllObjCharObj[i]->y=g_frameCounter%
					(g_pAllObjCharObj[i]->oAttrChar.actionValue1)==0?
					g_pAllObjCharObj[i]->y-g_pAllObjCharObj[i]->oAttrChar.actionValue2:
				g_pAllObjCharObj[i]->y;
				break;
			case e_SCROLL_DOWN://����ǹ�����Ļ,����Ҫ�޸������е���λ�ò���
				g_pAllObjCharObj[i]->y=g_frameCounter%
					(g_pAllObjCharObj[i]->oAttrChar.actionValue1)==0?
					g_pAllObjCharObj[i]->y+g_pAllObjCharObj[i]->oAttrChar.actionValue2:
				g_pAllObjCharObj[i]->y;
				break;												
			case e_STATIC://����Ǿ�̬��Ļ
				break;
			case e_SLOWHIDE://���������������Ļ
				break;
			case e_SPARK://�������˸��Ļ
				break;
			default:
				break;
			}
		_OverLapCaptionOnYUV420(pYUV420Frame, iSrcWidth, iSrcHeight,//Դͼ�����
							 g_pAllObjCharObj[i]->x, g_pAllObjCharObj[i]->y,g_pAllObjCharObj[i]->w, g_pAllObjCharObj[i]->h, //Ҫ���ӵ�λ��
							 g_pAllObjCharObj[i]->szStr,&(g_pAllObjCharObj[i]->oAttrChar));//Ҫ���ӵ����ּ���������
	}	
	return temp;
}
//��ʼ��
char OSD_Init(char *szPathHZK,char *szPathASCII)
{
	//���غ��ֵ����ֿ�
	if((g_fpHZKLIB=fopen(szPathHZK,"rb"))==NULL){
		return ERR_FILEOPENFAIL;
	}

	//����ascii�����ֿ�
	if((g_fpASCII=fopen(szPathASCII,"rb"))==NULL){
		return ERR_FILEOPENFAIL;
	}	
	
	//��ʼ��ת����
	_InitDitherTab();

	return ERR_NONE;	
}
//����
void OSD_Release()
{
	//�ر��ֿ��ļ�
	if(g_fpHZKLIB)
		fclose(g_fpHZKLIB);
	if(g_fpASCII)
		fclose(g_fpASCII);
}


/*******************************************************************************
˵��:
	������_XXX(XX,XX,...)��ʽ���ֵĺ�����Ϊ˽�к���

WroteBy/Date:
	gaoc@Dvision.com/2010.03.05

Modify:
*******************************************************************************/
//��ʼ��ת����
void _InitDitherTab()
{	
    long int crv,cbu,cgu,cgv;
    int i,ind;

    crv = 104597; cbu = 132201; 
    cgu = 25675; cgv = 53279;

    for (i = 0; i < 256; i++){
        crv_tab[i] = (i-128) * crv;
        cbu_tab[i] = (i-128) * cbu;
        cgu_tab[i] = (i-128) * cgu;
        cgv_tab[i] = (i-128) * cgv;
        tab_76309[i] = 76309*(i-16);
    }

    for (i=0; i<384; i++){
        clp[i] =0;
	}

    ind=384;
    for (i=0;i<256; i++){
        clp[ind++]=i;
	}

    ind=640;
    for (i=0;i<384;i++){
        clp[ind++]=255;
	}
}
//���ߺ���:RGB24תYUV420
void _RGB24ToYUV420(unsigned char *RGB, int nWidth,int nHeight, unsigned char *YUV, unsigned long nLen)
{
    //��������
    int i,x,y,j;
    unsigned char *Y = NULL;
    unsigned char *U = NULL;
    unsigned char *V = NULL;
    
    Y = YUV;
    U = YUV + nWidth*nHeight;
    V = U + ((nWidth*nHeight)>>2);

    for(y=0; y < nHeight; y++)
	{
        for(x=0; x < nWidth; x++)
        {
            j = y*nWidth + x;
            i = j*3;

			Y[j] = (unsigned char)(DY(RGB[i+2], RGB[i+1], RGB[i]));

            if(x%2 == 1 && y%2 == 1)
            {
                j = (nWidth>>1) * (y>>1) + (x>>1);
                //����i����Ч
                U[j] = (unsigned char)
                       ((DU(RGB[i +2 ], RGB[i+1], RGB[i]) + 
                         DU(RGB[i-1], RGB[i-2], RGB[i-3]) +
                         DU(RGB[i+2  -nWidth*3], RGB[i+1-nWidth*3], RGB[i-nWidth*3]) +
                         DU(RGB[i-1-nWidth*3], RGB[i-2-nWidth*3], RGB[i-3-nWidth*3]))/4);

                V[j] = (unsigned char)
                       ((DV(RGB[i+2  ], RGB[i+1], RGB[i]) + 
                         DV(RGB[i-1], RGB[i-2], RGB[i-3]) +
                         DV(RGB[i+2  -nWidth*3], RGB[i+1-nWidth*3], RGB[i-nWidth*3]) +
                         DV(RGB[i-1-nWidth*3], RGB[i-2-nWidth*3], RGB[i-3-nWidth*3]))/4);
            }
        }
	}

	nLen = nWidth * nHeight+(nWidth * nHeight)/2;
}
//���ߺ���:YUV420תRGB
void _YUV420ToRGB24(unsigned char *src_yuv, unsigned char *dst_rgb, int width, int height)
{
	int y1,y2,u,v; 
    unsigned char *py1,*py2;
    int i,j, c1, c2, c3, c4;
    unsigned char *d1, *d2;

	unsigned char *srcY = src_yuv;
	unsigned char *srcU = src_yuv + width*height;
	unsigned char *srcV = src_yuv + width*height + (width/2)*(height/2);

    py1=srcY;
    py2=py1+width;
    d1=dst_rgb;
    d2=d1+3*width;
    for (j = 0; j < height; j += 2) 
	{ 
        for (i = 0; i < width; i += 2)
		{
            u = *srcU++;
            v = *srcV++;

            c1 = crv_tab[v];
            c2 = cgu_tab[u];
            c3 = cgv_tab[v];
            c4 = cbu_tab[u];

			//up-left
            y1 = tab_76309[*py1++];    
            *d1++ = clp[384+((y1 + c4)>>16)];
            *d1++ = clp[384+((y1 - c2 - c3)>>16)];
            *d1++ = clp[384+((y1 + c1)>>16)]; 

            //down-left
            y2 = tab_76309[*py2++];
            *d2++ = clp[384+((y2 + c4)>>16)];
            *d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c1)>>16)];

            //up-right
            y1 = tab_76309[*py1++];
            *d1++ = clp[384+((y1 + c4)>>16)];
            *d1++ = clp[384+((y1 - c2 - c3)>>16)];
            *d1++ = clp[384+((y1 + c1)>>16)]; 

            //down-right
            y2 = tab_76309[*py2++];
            *d2++ = clp[384+((y2 + c4)>>16)];
            *d2++ = clp[384+((y2 - c2 - c3)>>16)];
            *d2++ = clp[384+((y2 + c1)>>16)]; 
        }
        d1 += 3*width;
        d2 += 3*width;
        py1+=   width;
        py2+=   width;
    } 
}
void _YUV420ToRGB24_reDraw(unsigned char *src_yuv, /*Դ */ unsigned char *dst_rgb, int width, int height, bool isRedraw)
{
	int y1,y2,u,v; 
	unsigned char *py1,*py2;
	int i,j, c1, c2, c3, c4;
	unsigned char *d1, *d2;

	unsigned char *srcY = src_yuv;
	unsigned char *srcU = src_yuv + width*height;
	unsigned char *srcV = src_yuv + width*height + (width/2)*(height/2);
	unsigned char colorR = 0, colorG = 0, colorB = 72;

	py1=srcY;
	py2=py1+width;
	d1=dst_rgb;
	d2=d1+3*width;
	for (j = 0; j < height; j += 2) 
	{ 
		for (i = 0; i < width; i += 2)
		{
			u = *srcU++;
			v = *srcV++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
			if (isRedraw)
			{
				*d1++ = colorR;
				*d1++ = colorG;
				*d1++ = colorB;
			}
			else
			{
				y1 = tab_76309[*py1++];    
				*d1++ = clp[384+((y1 + c4)>>16)];
				*d1++ = clp[384+((y1 - c2 - c3)>>16)];
				*d1++ = clp[384+((y1 + c1)>>16)]; 
			}

			//down-left
			if (isRedraw)
			{
				*d2++ = colorR;
				*d2++ = colorG;
				*d2++ = colorB;
			}
			else{
				y2 = tab_76309[*py2++];
				*d2++ = clp[384+((y2 + c4)>>16)];
				*d2++ = clp[384+((y2 - c2 - c3)>>16)];
				*d2++ = clp[384+((y2 + c1)>>16)];
			}
			

			//up-right
			if (isRedraw)
			{
				*d1++ = colorR;
				*d1++ = colorG;
				*d1++ = colorB;
			}
			else
			{
				y1 = tab_76309[*py1++];
				*d1++ = clp[384+((y1 + c4)>>16)];
				*d1++ = clp[384+((y1 - c2 - c3)>>16)];
				*d1++ = clp[384+((y1 + c1)>>16)]; 
			}
		
			//down-right
			if (isRedraw)
			{
				*d2++ = colorR;
				*d2++ = colorG;
				*d2++ = colorB;
			}
			else{
				y2 = tab_76309[*py2++];
				*d2++ = clp[384+((y2 + c4)>>16)];
				*d2++ = clp[384+((y2 - c2 - c3)>>16)];
				*d2++ = clp[384+((y2 + c1)>>16)]; 
			}
		}
		d1 += 3*width;
		d2 += 3*width;
		py1+=   width;
		py2+=   width;
	} 
}
//���ߺ���:YUV420תYUV422�ĺ���
void _YUV420ToYUV422(char* pYUV420Buf, int iSrcWidth, int iSrcHeight, char* pYUV422Buf)
{
	unsigned int nIamgeSize = iSrcWidth*iSrcHeight;
	int i;
	if((pYUV420Buf == NULL) || (pYUV422Buf == NULL))
	{
		return;
	}
	//Copy Y
	for(i = 0;i < iSrcHeight; i ++)
	{
		memcpy(pYUV422Buf + i*iSrcWidth,pYUV420Buf + i*iSrcWidth,iSrcWidth);
	}
	//Copy U
	for (i = 0; i < iSrcHeight/2; i ++) 
	{
		memcpy(pYUV422Buf + nIamgeSize + (2*i)*iSrcWidth/2,pYUV420Buf + nIamgeSize + i*iSrcWidth/2,iSrcWidth/2);
		memcpy(pYUV422Buf + nIamgeSize + (2*i+1)*iSrcWidth/2,pYUV420Buf + nIamgeSize + i*iSrcWidth/2,iSrcWidth/2);
	}
	//Copy V
	for (i = 0; i < iSrcHeight/2; i ++) 
	{
		memcpy(pYUV422Buf + nIamgeSize*3/2 + (2*i)*iSrcWidth/2,pYUV420Buf + nIamgeSize*5/4 + i*iSrcWidth/2,iSrcWidth/2);
		memcpy(pYUV422Buf + nIamgeSize*3/2 + (2*i+1)*iSrcWidth/2,pYUV420Buf + nIamgeSize*5/4 + i*iSrcWidth/2,iSrcWidth/2);
	}
}
//���ߺ���:YUV422תYUV420�ĺ���
void _YUV422ToYUV420(char* pYUV422Buf, int iSrcWidth, int iSrcHeight, char* pYUV420Buf)
{
	unsigned int nIamgeSize = iSrcWidth*iSrcHeight;
	int i;

	if((pYUV422Buf == NULL) || (pYUV420Buf == NULL))
	{
		return;
	}
	//Copy Y
	for(i = 0;i < iSrcHeight; i ++)
	{
		memcpy(pYUV420Buf + i*iSrcWidth,pYUV422Buf + i*iSrcWidth,iSrcWidth);
	}
	//Copy U
	for (i = 0; i < iSrcHeight/2; i ++) 
	{
		memcpy(pYUV420Buf + nIamgeSize + i*iSrcWidth/2,pYUV422Buf + nIamgeSize + 2*i*iSrcWidth/2,iSrcWidth/2);
	}
	//Copy V
	for (i = 0; i < iSrcHeight/2; i ++) 
	{
		memcpy(pYUV420Buf + nIamgeSize*5/4 + i*iSrcWidth/2,pYUV422Buf + nIamgeSize*3/2 + 2*i*iSrcWidth/2,iSrcWidth/2);
	}
}
//��YUVԴͼ�ϴ���ʼλ��Ϊ(x,y)�ĵط���ȡָ����С��������
void _GetSubReginFromYUV420(unsigned char *src_yuv, int srcW, int srcH, //yuvԴͼ
					   unsigned char *sub_yuv, int x, int y, int subW, int subH)//yuv�������xy���꼰���
{
	unsigned char *pSrcY, *pSrcU, *pSrcV;
	unsigned char *pSubY, *pSubU, *pSubV;
	int i;
	int srcTmp = srcW*srcH;
	int subTmp = subW*subH;

	if(src_yuv == NULL || sub_yuv == NULL)
		return;

	if(subW>srcW || (x+subW)>srcW || subH>srcH || (y+subH)>srcH)
	{
		return;
	}

	if (x < 0)	//��xΪ����ʱ������ͼ���ֻ���趨Ϊԭͼ��
	{
		x = 0;
	}

	//����Y����
	for(i=0; i<subH; i++)
	{
		pSrcY = src_yuv + srcW*(y+i) + x;
		pSubY = sub_yuv + subW*i;
		memcpy(pSubY, pSrcY, subW);
	}

	//����U����
	for(i=0; i<(subH/2); i++)
	{
		pSrcU = (src_yuv + srcTmp) + (srcW/2)*(y/2+i) + x/2;
		pSubU = (sub_yuv + subTmp) + (subW/2)*i;
		memcpy(pSubU, pSrcU, subW/2);
	}

	//����V����
	for(i=0; i<(subH/2); i++)
	{
		pSrcV = (src_yuv + srcTmp + srcTmp/4)+ (srcW/2)*(y/2+i) + x/2;
		pSubV = (sub_yuv + subTmp + subTmp/4) + (subW/2)*i;
		memcpy(pSubV, pSrcV, subW/2);
	}
}
//�޸�yuv
void _SetSubReginToYUV420(unsigned char *src_yuv, int srcW, int srcH, //yuvԴͼ
					   unsigned char *sub_yuv, int x, int y, int subW, int subH)//yuv������	
{
	unsigned char *pSrcY, *pSrcU, *pSrcV;
	unsigned char *pSubY, *pSubU, *pSubV;
	int i;
	int srcTmp = srcW*srcH;
	int subTmp = subW*subH;

	if(src_yuv == NULL || sub_yuv == NULL)
		return;

	if(subW>srcW || (x+subW)>srcW || subH>srcH || (y+subH)>srcH)
	{
		return;
	}

	if (x < 0)	//��xΪ����ʱ������ͼ���ֻ���趨Ϊԭͼ��
	{
		x = 0;
	}

	//����Y����
	for(i=0; i<subH; i++)
	{
		pSrcY = src_yuv + srcW*(y+i) + x;
		pSubY = sub_yuv + subW*i;
		memcpy(pSrcY, pSubY, subW);
	}

	//����U����
	for(i=0; i<(subH/2); i++)
	{
		pSrcU = (src_yuv + srcTmp) + (srcW/2)*(y/2+i) + x/2;
		pSubU = (sub_yuv + subTmp) + (subW/2)*i;
		memcpy(pSrcU, pSubU, subW/2);
	}

	//����V����
	for(i=0; i<(subH/2); i++)
	{
		pSrcV = (src_yuv + srcTmp + srcTmp/4)+ (srcW/2)*(y/2+i) + x/2;
		pSubV = (sub_yuv + subTmp + subTmp/4) + (subW/2)*i;
		memcpy(pSrcV, pSubV, subW/2);
	}
}
//��RGB�ϵ�������
void _OverlapCaptionOnRGB(unsigned char* srcRgbBuf, int nWidth,int nHeight, 
						  char* pCharcode, O_STRINGATTR *pOAttrCharObj)
{
	int i,j,k,m;
	unsigned char qh,wh;
	unsigned long offset;
	unsigned int pixelCount;
	char frontBuffer[32]={0x00,0x00,
						0x00,0x00,
						0x00,0x00,
						0x7F,0xF0,
						0x18,0x3C,
						0x18,0x0E,
						0x18,0x0E,
						0x18,0x0E,
						0x18,0x0F,
						0x18,0x0E,
						0x18,0x0E,
						0x18,0x0C,
						0x18,0x38,
						0x7F,0xE0,
						0x00,0x00,
						0x00,0x00};
	int iStartXpos=0;
	int iStartYpos=0;
	unsigned int nStrlen = strlen(pCharcode);
	unsigned char bIsChar = 0;
	unsigned char *totalfont;
	int lenFont, xPosition;
	//test
	FILE *fRGBdata = fopen("RGBOut.rgb","w+");

	//���Ա���
	if ((srcRgbBuf == NULL) || (pCharcode == NULL)){
		return;
	}

	nHeight = 32;

	xPosition = g_pAllObjCharObj[0]->x;

	if (xPosition < 0)
	{
		lenFont = nWidth - xPosition + nHeight;
	}
	else{
		lenFont = nWidth + nHeight;//nWidth%nHeight ? (nWidth/nHeight+1)*nHeight : nWidth;
	}
	
	totalfont = new unsigned char[lenFont*nHeight*3];
	if (xPosition < 0)
	{
		for (int i = 0;i<nHeight;i++)
		{
			memcpy(totalfont+ (i*lenFont-xPosition)*3, srcRgbBuf + i*nWidth*3, nWidth*3);
		}
	}
	else{
		for (int i = 0;i<nHeight;i++)
		{
			memcpy(totalfont+ i*lenFont*3, srcRgbBuf + i*nWidth*3, nWidth*3);
		}
	}
	

	//�����ȡ�ַ����������
	for(m = 0; m < nStrlen;){
		
		memset(frontBuffer,0,sizeof(frontBuffer));
		
		
		//ȡ�õ�����Ϣ
		if (pCharcode[m]&0x80){//����
			qh=pCharcode[m] - 0xa0; 
			wh=pCharcode[m+1] - 0xa0;
			offset = (94*(qh-1)+(wh-1))*32;	
			fseek(g_fpHZKLIB,offset,SEEK_SET);
			fread(frontBuffer,32,1,g_fpHZKLIB);
			m += 2;
			bIsChar = 0;
		}else{//�ַ�
			offset = pCharcode[m]*32;
			fseek(g_fpASCII,offset,SEEK_SET);
			fread(frontBuffer,32,1,g_fpASCII);	
			m ++;
			bIsChar = 1;
		}

		//����
		for(j=0;j<16;j++){	//����Ϊʲ��16��
			pixelCount = 0;
			for(i=0;i<2;i++){
				for(k=0;k<8;k++){
					if(((frontBuffer[j*2+i]>>(7-k)) & 0x1) != 0){	
						int rectindex = (lenFont*j*2 + i*8*2 + k*2 + iStartXpos)*3;
						totalfont[rectindex] = pOAttrCharObj->osdB;
						totalfont[rectindex+1] = pOAttrCharObj->osdG;
						totalfont[rectindex+2] = pOAttrCharObj->osdR;
						totalfont[rectindex+3] = pOAttrCharObj->osdB;
						totalfont[rectindex+4] = pOAttrCharObj->osdG;
						totalfont[rectindex+5] = pOAttrCharObj->osdR;
						rectindex =(lenFont*(2*j+1) + i*8*2 + k*2 + iStartXpos)*3;
						totalfont[rectindex] = pOAttrCharObj->osdB;
						totalfont[rectindex+1] = pOAttrCharObj->osdG;
						totalfont[rectindex+2] = pOAttrCharObj->osdR;
						totalfont[rectindex+3] = pOAttrCharObj->osdB;
						totalfont[rectindex+4] = pOAttrCharObj->osdG;
						totalfont[rectindex+5] = pOAttrCharObj->osdR;
					}
					//if (k%2==0){
						pixelCount++; 
					//}
				 }
			}	
		}

		//���ֺ��ּ��ַ���ռ���
		if (bIsChar == 0){
			iStartXpos += nHeight;
		}else{
			iStartXpos += nHeight/2;
		}

		if(iStartXpos>=(lenFont-nHeight) || m > nStrlen-1){
			if (xPosition < 0)
			{
				for (int i = 0;i<nHeight;i++)
				{
					//����2�����ص㣬�������嶥������
					memcpy(srcRgbBuf + i*nWidth*3 + 2*nWidth*3, totalfont + (i*lenFont-xPosition)*3, nWidth*3);
				}
			}
			else
			{
				for (int i = 0;i<nHeight;i++)
				{
					memcpy(srcRgbBuf + i*nWidth*3 + 2*nWidth*3, totalfont + i*lenFont*3, nWidth*3);
				}
			}
			
			//if (iStartXpos >= 16*3)
			{
				fwrite(srcRgbBuf, 1, nWidth*nHeight*3, fRGBdata);
				fflush(fRGBdata);
				fclose(fRGBdata);
			}
			delete[] totalfont;
			return ;
		}
		
	}
}
//��RGB�ϵ�������32���ֵ���
void _OverlapCaptionOnRGB_16to32(unsigned char* srcRgbBuf, int nWidth,int nHeight, 
	char* pCharcode, O_STRINGATTR *pOAttrCharObj)
{
	int i,j,k,m;
	unsigned char qh,wh;
	unsigned long offset;
	unsigned int pixelCount;
	const int size_32hz = nHeight*nHeight/8;
	char *frontBuffer = new char[size_32hz];
	int iStartXpos=0;
	int iStartYpos=0;
	unsigned int nStrlen = strlen(pCharcode);
	unsigned char bIsChar = 0;
	//test
	FILE *fRGBdata = fopen("RGBOut.rgb","w+");
	FILE *fHZdata = fopen("HZOut.data","w+");

	//���Ա���
	if ((srcRgbBuf == NULL) || (pCharcode == NULL)){
		return;
	}


	//�����ȡ�ַ����������
	for(m = 0; m < nStrlen;){

		memset(frontBuffer,0,size_32hz);


		//ȡ�õ�����Ϣ
		if (pCharcode[m]&0x80){//����
			qh=pCharcode[m] - 0xa0; 
			wh=pCharcode[m+1] - 0xa0;
			offset = (94*(qh-1)+(wh-1))*nHeight;					
			fseek(g_fpHZKLIB,offset,SEEK_SET);
			fread(frontBuffer,1,32,g_fpHZKLIB);
			m += 2;
			bIsChar = 0;
		}else{//�ַ�
			offset = pCharcode[m]*nHeight;							
			fseek(g_fpASCII,offset,SEEK_SET);
			fread(frontBuffer,1,32,g_fpASCII);	
			m ++;
			bIsChar = 1;
		}
		fwrite(frontBuffer, 1 ,32 , fHZdata);
		fclose(fHZdata);

		//����
		for(j=0;j<16;j++){	
			pixelCount = 0;
			for(i=0;i<2;i++){		
				for(k=0;k<8;k++){
					int tindex = nWidth*3*j+(i*8+k+iStartXpos)*3;
					//srcRgbBuf[tindex] = 18;
					//srcRgbBuf[tindex+1] = 17;
					//srcRgbBuf[tindex+2] = 55;
					if(((frontBuffer[j*2+i]>>(7-k)) & 0x1) != 0)
					{		
						tindex = (j*nWidth+i*8+k+iStartXpos)*3;	//����j*2������������;i��k������2������������32
						//tindex = nWidth*3*j+(i*8+k+iStartXpos)*3;
						srcRgbBuf[tindex]=pOAttrCharObj->osdB;
						srcRgbBuf[tindex+1]=pOAttrCharObj->osdG;
						srcRgbBuf[tindex+2]=pOAttrCharObj->osdR;

						//srcRgbBuf[tindex+3]=pOAttrCharObj->osdB;
						//srcRgbBuf[tindex+4]=pOAttrCharObj->osdG;
						//srcRgbBuf[tindex+5]=pOAttrCharObj->osdR;

						//tindex = nWidth*3*(j+1)*2+((i+1)*8*2+(k+1)*2+iStartXpos)*3;
						//srcRgbBuf[tindex]=pOAttrCharObj->osdB;
						//srcRgbBuf[tindex+1]=pOAttrCharObj->osdG;
						//srcRgbBuf[tindex+2]=pOAttrCharObj->osdR;
						//srcRgbBuf[tindex+3]=pOAttrCharObj->osdB;
						//srcRgbBuf[tindex+4]=pOAttrCharObj->osdG;
						//srcRgbBuf[tindex+5]=pOAttrCharObj->osdR;
					}
					//if (k%2==0){
					pixelCount++; 
					//}
				}
			}	
		}

		//���ֺ��ּ��ַ���ռ���
		if (bIsChar == 0){
			iStartXpos += 32;		//��Ϊ32
		}else{
			iStartXpos += 16;
		}

		if(iStartXpos>nWidth){
			if (iStartXpos >= 32*20)
			{
				fwrite(srcRgbBuf, 1, iStartXpos*nHeight*3, fRGBdata);
				fflush(fRGBdata);
				fclose(fRGBdata);
			}
			delete[] frontBuffer;
			return ;
		}

	}
}
//��RGB�ϵ�������32���ֵ���
void _OverlapCaptionOnRGB_32(unsigned char* srcRgbBuf, int nWidth,int nHeight, 
	char* pCharcode, O_STRINGATTR *pOAttrCharObj)
{
	int i,j,k,m;
	unsigned char qh,wh;
	unsigned long offset;
	unsigned int pixelCount;
	const int size_32hz = nHeight*nHeight/8;
	char *frontBuffer = new char[size_32hz];
	int iStartXpos=0;
	int iStartYpos=0;
	unsigned int nStrlen = strlen(pCharcode);
	unsigned char bIsChar = 0;
	//test
	FILE *fRGBdata = fopen("RGBOut.rgb","w+");
	FILE *fHZdata = fopen("HZOut.data","w+");

	//���Ա���
	if ((srcRgbBuf == NULL) || (pCharcode == NULL)){
		return;
	}


	//�����ȡ�ַ����������
	for(m = 0; m < nStrlen;){

		memset(frontBuffer,0,size_32hz);


		//ȡ�õ�����Ϣ
		if (pCharcode[m]&0x80){//����
			qh=pCharcode[m] - 0xa0; 
			wh=pCharcode[m+1] - 0xa0;
			offset = (94*(qh-1)+(wh-1))*size_32hz;					
			fseek(g_fpHZKLIB,offset,SEEK_SET);
			fread(frontBuffer,size_32hz,1,g_fpHZKLIB);
			m += 2;
			bIsChar = 0;
		}else{//�ַ�
			offset = pCharcode[m]*size_32hz;							
			fseek(g_fpASCII,offset,SEEK_SET);
			fread(frontBuffer,1,size_32hz,g_fpASCII);	
			m ++;
			bIsChar = 1;
		}
		fwrite(frontBuffer, 1 ,size_32hz , fHZdata);
		fclose(fHZdata);

		//����
		for(j=0;j<32;j++){	
			pixelCount = 0;
			for(i=0;i<4;i++){		
				for(k=0;k<8;k++){
					if(((frontBuffer[j*4+i]>>(7-k)) & 0x1) != 0){		
						int tindex = nWidth*3*j+(i*8+k+iStartXpos)*3;	//����j*2������������;i��k������2������������32
						srcRgbBuf[tindex]=pOAttrCharObj->osdB;
						srcRgbBuf[tindex+1]=pOAttrCharObj->osdG;
						srcRgbBuf[tindex+2]=pOAttrCharObj->osdR;

						//srcRgbBuf[tindex+3]=pOAttrCharObj->osdB;
						//srcRgbBuf[tindex+4]=pOAttrCharObj->osdG;
						//srcRgbBuf[tindex+5]=pOAttrCharObj->osdR;

						//tindex = nWidth*3*(j+1)*2+((i+1)*8*2+(k+1)*2+iStartXpos)*3;
						//srcRgbBuf[tindex]=pOAttrCharObj->osdB;
						//srcRgbBuf[tindex+1]=pOAttrCharObj->osdG;
						//srcRgbBuf[tindex+2]=pOAttrCharObj->osdR;
						//srcRgbBuf[tindex+3]=pOAttrCharObj->osdB;
						//srcRgbBuf[tindex+4]=pOAttrCharObj->osdG;
						//srcRgbBuf[tindex+5]=pOAttrCharObj->osdR;
					}
					//if (k%2==0){
					pixelCount++; 
					//}
				}
			}	
		}

		//���ֺ��ּ��ַ���ռ���
		if (bIsChar == 0){
			iStartXpos += 32;		//��Ϊ32
		}else{
			iStartXpos += 16;
		}

		if(iStartXpos>nWidth){
			fwrite(srcRgbBuf, 1, nWidth*nHeight*3, fRGBdata);
			fclose(fRGBdata);
			delete[] frontBuffer;
			return ;
		}

	}
}
//�����YUV420�ϵ�����Ļ
char _OverLapCaptionOnYUV420(unsigned char *src_yuv, int srcW, int srcH,//Դͼ�����
							 int xStart, int yStart, int dstW, int dstH,//Ҫ���ӵ��ַ�����λ�ü����
							 char* pCaption,O_STRINGATTR *pOAttrCharObj)//Ҫ���ӵ��ַ�������������
{
	/*
	FILE *pSubYuv = fopen("AsubYUV.yuv","wb");
	FILE *pSubRGB = fopen("AsubRGB.rgb","wb");
	FILE *pDstYuv = fopen("AdstYUV.yuv","wb");
	FILE *pDstRGB = fopen("AdstRGB.rgb","wb");
	*/
	
	int sub_W=0;
	int sub_H=pOAttrCharObj->sizeH;
	int m=0;
	int nStrlen=strlen(pCaption);
	//����Ҫ��ȡ��������
	for(m = 0; m < nStrlen;){	
		if (pCaption[m]&0x80)
			m += 2;
		else//�ַ�
			m += 1;
		
		//sub_W+=pOAttrCharObj->sizeW;
	}
	sub_W = m * dstH/2;

	
	sub_W=(sub_W+xStart)>srcW?(srcW-xStart):sub_W;
	sub_H=(sub_H+yStart)>srcH?(srcH-yStart):sub_H;

	if(sub_W > srcW)		//��xΪ����ʱ������ͼ���ֻ���趨Ϊԭͼ��
	{
		sub_W = srcW;
	}
	
	//�����������¸�2�����ص�
	yStart = yStart -2;
	sub_H = sub_H + 4;
	
	assert(src_yuv&&pCaption&&pOAttrCharObj);

	
	//����
	_GetSubReginFromYUV420(src_yuv, srcW, srcH, 
		subYUVBuf, xStart, yStart, sub_W, sub_H);
	//fwrite(subYUVBuf,1,sub_W*sub_H*3/2,pSubYuv);
	//fflush(pSubYuv);
	//fclose(pSubYuv);

	//yuvת����RGB
	//_YUV420ToRGB24(subYUVBuf, srcRGBBuf, sub_W, sub_H);
	_YUV420ToRGB24_reDraw(subYUVBuf, srcRGBBuf, sub_W, sub_H, true);
	//fwrite(srcRGBBuf,1,sub_W*sub_H*3,pSubRGB);
	//fflush(pSubRGB);
	//fclose(pSubRGB);
	
	//��RGB����ɵ���
	_OverlapCaptionOnRGB(srcRGBBuf, sub_W, sub_H, 	pCaption, pOAttrCharObj);
	//_OverlapCaptionOnRGB_16to32(srcRGBBuf, sub_W, sub_H, 	pCaption, pOAttrCharObj);
	//fwrite(dstRGBBuf,1,sub_W*sub_H*3,pDstRGB);
	//fflush(pDstRGB);
	//fclose(pDstRGB);
	
	//rgbת����YUV
	_RGB24ToYUV420(srcRGBBuf, sub_W, sub_H, 
				dstYUVBuf, sub_W * sub_H *3/2);
	//fwrite(dstYUVBuf,1,sub_W*sub_H*3/2,pDstYuv);
	//fflush(pDstYuv);
	//fclose(pDstYuv);
	
	//�������
	_SetSubReginToYUV420(src_yuv, srcW, srcH, 
					dstYUVBuf, xStart, yStart ,sub_W, sub_H);

	return ERR_NONE;
}
//ֱ����YUV422�ϵ����ַ�
void _OverLapCaptionOnYUV422Raw(char* pCharcode,int column, int row, int imageWidth, int imageHeight, char *pYUVbuffer,char OsdY,char OsdU,char OsdV)
{
	int i,j,k,m;
	unsigned char qh,wh;
	unsigned long offset;
	unsigned int pixelCount;
	char frontBuffer[32]={0x00,0x00,
						0x00,0x00,
						0x00,0x00,
						0x7F,0xF0,
						0x18,0x3C,
						0x18,0x0E,
						0x18,0x0E,
						0x18,0x0E,
						0x18,0x0F,
						0x18,0x0E,
						0x18,0x0E,
						0x18,0x0C,
						0x18,0x38,
						0x7F,0xE0,
						0x00,0x00,
						0x00,0x00};
	int iStartXpos=100;
	int iStartYpos=100;
	unsigned int nStrlen = strlen(pCharcode);
	unsigned char bIsChar = 0;
	int temp1=0;
	int temp2=0;

	//���Ա���
	if ((pCharcode == NULL) || (pYUVbuffer == NULL)){
		return;
	}


	//�����ȡ�ַ����������
	for(m = 0; m < nStrlen;){
		
		memset(frontBuffer,0,sizeof(frontBuffer));
		
		
		//ȡ�õ�����Ϣ
		if (pCharcode[m]&0x80){//����
			qh=pCharcode[m] - 0xa0; 
			wh=pCharcode[m+1] - 0xa0;
			offset = (94*(qh-1)+(wh-1))*32;	
			fseek(g_fpHZKLIB,offset,SEEK_SET);
			fread(frontBuffer,32,1,g_fpHZKLIB);
			m += 2;
			bIsChar = 0;
		}else{//�ַ�
			offset = pCharcode[m]*32;
			fseek(g_fpASCII,offset,SEEK_SET);
			fread(frontBuffer,32,1,g_fpASCII);	
			m ++;
			bIsChar = 1;
		}
		
		
		//����
		for(j=0;j<16;j++){
			pixelCount = 0;
			for(i=0;i<2;i++){
				for(k=0;k<8;k++){
					if(((frontBuffer[j*2+i]>>(7-k)) & 0x1) != 0){		
						pYUVbuffer[((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)] = OsdU;
						pYUVbuffer[((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)+1] = OsdY;
						//pYUVbuffer[((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)+2] = OsdY;
						//pYUVbuffer[((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)+3] = OsdU;
						
						//temp1=imageWidth*imageHeight*5/4+((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)/2;//ɫ���޸�
						//temp2=((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)*2;
						//temp1=imageWidth*imageHeight*5/4+(j+iStartYpos)/2*352+(iStartXpos+pixelCount)/2;
						//pYUVbuffer[(int)(((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)/4)+imageWidth*imageHeight] = 100;
						//pYUVbuffer[temp1] = 250;
						//pYUVbuffer[temp2] = 200;
						//pYUVbuffer[((j+iStartYpos)*imageWidth+iStartXpos+pixelCount)+2] = OsdV;
						//buffer[((index+row)*imageWidth+column+pixelCount)*2] = OsdU;
			 			//buffer[((index+row)*imageWidth+column+pixelCount)*2+1] = OsdY;
					}
					//if (k%2==0){
						pixelCount++; 
					//}
				 }
			}	
		}

		//���ֺ��ּ��ַ���ռ���
		if (bIsChar == 0){
			iStartXpos += 32;
		}else{
			iStartXpos += 16;
		}
		
	}
}

