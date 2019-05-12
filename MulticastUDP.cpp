#include "stdafx.h"
#include "MulticastUDP.h"


MulticastUDP::MulticastUDP(void)
{
	m_socket = INVALID_SOCKET;
}


MulticastUDP::~MulticastUDP(void)
{
	WSACleanup();
}

int MulticastUDP::InitialSock(char * localIP, char * destIP, int Port)
{
	WSADATA			wsd;
	ULONG				localif;
	int							ret;

	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{
		//printf("WSAStartup() failed: %d\n", GetLastError());
		cout<<"WSAStartup() failed: %d\n"<<GetLastError();
		return -1;
	}

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_socket)
	{
		cout<<"failed to create socket"<<endl;
		return -2;
	}

	localif = inet_addr(localIP);
	ret = setsockopt(m_socket,IPPROTO_IP, IP_MULTICAST_IF, (char*)&localif, sizeof(localif));
	if(ret == SOCKET_ERROR)
	{
		cout<<"setsockopt failed! Error:"<<WSAGetLastError()<<endl;
		closesocket (m_socket);
		return-3;
	}

	dest.sin_family = AF_INET;
	dest.sin_port = htons(Port);
	dest.sin_addr.s_addr = inet_addr(destIP);

	return 1;
}

int MulticastUDP::sendData(unsigned char * data, int len)
{
	int			ret;
	ret = sendto(m_socket,(char *)data,len,0,(SOCKADDR *)&dest,sizeof(dest));
	if(ret == SOCKET_ERROR)
	{
		cout<<"sendto failed! Error: %d"<<WSAGetLastError ()<<endl;
		closesocket (m_socket);
		return -1;
	}
	else
	{
		//cout<<"sent %d bytes"<<len<<endl;
		return ret;
	}
}

int MulticastUDP::closeSock()
{
	shutdown(m_socket,0x01);

	closesocket(m_socket);

	return 1;
}
