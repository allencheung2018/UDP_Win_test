#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32")	

using namespace std;
class MulticastUDP
{
public:
	MulticastUDP(void);
	~MulticastUDP(void);
public:
	int InitialSock(char * localIP, char * destIP, int Port);
	int sendData(unsigned char * data, int len);
	int closeSock();
private:
	SOCKET m_socket;
	SOCKADDR_IN dest;
};

