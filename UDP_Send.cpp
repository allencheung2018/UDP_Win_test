//UDPԭʼ��������

/*++
  ��������д�˸�UDP��ԭʼ��������,�󲿷ִ����Ǵ�Windows��������COPY������
  ����һ���.ֱ�ӵ���sendudp�Ϳ�����������ԭʼUDP����. 


--*/


#include "UDPWin.h"


//

// 



//

//


// 

int main(int argc, char **argv)
{
    char cTemp;
	char cpShow[1000] = {0};
	UDPWin udpTest;
	while(1)
	 {
		//udpTest.sendudp("192.168.1.189", "192.168.1.188", 5551, 5552, "Send test !", 50);
		//for(int i=0;i<100000;i++);
		memset(cpShow,0,1000);
		udpTest.receiveudp("192.168.1.5", "192.168.1.10",5551, 5552, cpShow, 512);
		printf("cpShow = %s\n",cpShow);
		//cTemp = getchar();
		//udpTest.receiveudp("192.168.1.189", "192.168.1.188",5551, 5552, cpShow, 50);
		//printf("cpShow = %s\n",cpShow);
		cTemp = 'a';
		if (cTemp == 'q')
		{
			break;
		}
	 }
}/**/

