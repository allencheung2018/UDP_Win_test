#include "stdafx.h"
#include "UDPWin.h"

#include <stdlib.h>

UDPWin::UDPWin(void)
{
}


UDPWin::~UDPWin(void)
{
}

USHORT UDPWin::checksum(USHORT *buffer, int size)
{
	unsigned long cksum=0;

	while (size > 1)
	{
		cksum += *buffer++;
		size  -= sizeof(USHORT);   
	}
	if (size)
	{
		cksum += *(UCHAR*)buffer;   
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16); 

	return (USHORT)(~cksum); 
}

int UDPWin::InitIpv4Header(char *buf, char *src, char *dest, int payloadlen)
{
	IPV4_HDR    *v4hdr=NULL;

	v4hdr = (IPV4_HDR *)buf;

	v4hdr->ip_verlen      = (4 << 4) | (sizeof(IPV4_HDR) / sizeof(unsigned long));
	v4hdr->ip_tos         = 0;
	v4hdr->ip_totallength = htons(sizeof(IPV4_HDR) + payloadlen);
	v4hdr->ip_id          = 0;
	v4hdr->ip_offset      = 0;
	v4hdr->ip_ttl         = 128;
	v4hdr->ip_protocol    = 0x11;
	v4hdr->ip_checksum    = 0;
	v4hdr->ip_srcaddr     = inet_addr(src);
	v4hdr->ip_destaddr    = inet_addr(dest);

	v4hdr->ip_checksum    = checksum((unsigned short *)v4hdr, sizeof(IPV4_HDR));

	return sizeof(IPV4_HDR);
}

void UDPWin::ComputeUdpPseudoHeaderChecksumV4(void *iphdr, UDP_HDR *udphdr, char *payload, int payloadlen)
{
	IPV4_HDR     *v4hdr=NULL;
	unsigned long zero=0;
	char          buf[UDPSize],
		*ptr=NULL;
	int           chksumlen=0,
		i;

	ptr = buf;

	v4hdr = (IPV4_HDR *)iphdr;

	// Include the source and destination IP addresses
	memcpy(ptr, &v4hdr->ip_srcaddr,  sizeof(v4hdr->ip_srcaddr));  
	ptr += sizeof(v4hdr->ip_srcaddr);
	chksumlen += sizeof(v4hdr->ip_srcaddr);

	memcpy(ptr, &v4hdr->ip_destaddr, sizeof(v4hdr->ip_destaddr)); 
	ptr += sizeof(v4hdr->ip_destaddr);
	chksumlen += sizeof(v4hdr->ip_destaddr);

	// Include the 8 bit zero field
	memcpy(ptr, &zero, 1);
	ptr++;
	chksumlen += 1;

	// Protocol
	memcpy(ptr, &v4hdr->ip_protocol, sizeof(v4hdr->ip_protocol)); 
	ptr += sizeof(v4hdr->ip_protocol);
	chksumlen += sizeof(v4hdr->ip_protocol);

	// UDP length
	memcpy(ptr, &udphdr->udp_length, sizeof(udphdr->udp_length)); 
	ptr += sizeof(udphdr->udp_length);
	chksumlen += sizeof(udphdr->udp_length);

	// UDP source port
	memcpy(ptr, &udphdr->src_portno, sizeof(udphdr->src_portno)); 
	ptr += sizeof(udphdr->src_portno);
	chksumlen += sizeof(udphdr->src_portno);

	// UDP destination port
	memcpy(ptr, &udphdr->dst_portno, sizeof(udphdr->dst_portno)); 
	ptr += sizeof(udphdr->dst_portno);
	chksumlen += sizeof(udphdr->dst_portno);

	// UDP length again
	memcpy(ptr, &udphdr->udp_length, sizeof(udphdr->udp_length)); 
	ptr += sizeof(udphdr->udp_length);
	chksumlen += sizeof(udphdr->udp_length);

	// 16-bit UDP checksum, zero 
	memcpy(ptr, &zero, sizeof(unsigned short));
	ptr += sizeof(unsigned short);
	chksumlen += sizeof(unsigned short);

	// payload
	memcpy(ptr, payload, payloadlen);
	ptr += payloadlen;
	chksumlen += payloadlen;

	// pad to next 16-bit boundary
	for(i=0 ; i < payloadlen%2 ; i++, ptr++)
	{
		printf("pad one byte\n");
		*ptr = 0;
		ptr++;
		chksumlen++;
	}

	// Compute the checksum and put it in the UDP header
	udphdr->udp_checksum = checksum((USHORT *)buf, chksumlen);

	return;
}

int UDPWin::InitUdpHeader(char *buf, int srcprt, int dstprt, int payloadlen)
{
	UDP_HDR *udphdr=NULL;

	udphdr = (UDP_HDR *)buf;
	udphdr->src_portno = htons(srcprt);
	udphdr->dst_portno = htons(dstprt);
	udphdr->udp_length = htons(sizeof(UDP_HDR) + payloadlen);

	return sizeof(UDP_HDR);
}

int UDPWin::sendudp(char *srcip, char *dstip, int srcprt, int dstprt, char *buf, int bufsize)
{
	WSADATA            wsd;
	SOCKET             s;
	char sendbuf[UDPSize]={0};
	int           iphdrlen,		allsize,		udphdrlen;
	int     optlevel,		option,		optval,		rc;  
	SOCKADDR_IN    ReceiverAddr;

	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(dstprt);    
	ReceiverAddr.sin_addr.s_addr = inet_addr(dstip);

	allsize = sizeof(IPV4_HDR) + sizeof(UDP_HDR) + bufsize;
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{
		printf("WSAStartup() failed: %d\n", GetLastError());
		return -1;
	}

	s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
		return -1;
	}

	// Enable the IP header include option 
	optval = 1;
	optlevel = IPPROTO_IP;
	option   = IP_HDRINCL;
	rc = setsockopt(s, optlevel, option, (char *)&optval, sizeof(optval));
	if (rc == SOCKET_ERROR)
	{
		fprintf(stderr, "setsockopt: IP_HDRINCL failed: %d\n", WSAGetLastError());
		return -1;
	}


	// Initialize the v4 header
	iphdrlen = InitIpv4Header(
		sendbuf, 
		srcip, 
		dstip, 
		bufsize
		);

	// Initialize the UDP header
	udphdrlen = InitUdpHeader(
		&sendbuf[iphdrlen], 
		srcprt, 
		dstprt, 
		bufsize
		);

	// Compute the UDP checksum
	ComputeUdpPseudoHeaderChecksumV4(
		sendbuf, 
		(UDP_HDR *)&sendbuf[iphdrlen], 
		buf, 
		bufsize
		);

	// Copy the payload to the end of the header
	memcpy(&sendbuf[iphdrlen + udphdrlen], buf, bufsize);

	rc = sendto(
		s,
		sendbuf,
		allsize,
		0,
		(const struct sockaddr*)&ReceiverAddr,
		sizeof(ReceiverAddr)
		);
	if (rc == SOCKET_ERROR)
	{
		printf("sendto() failed: %d\n", WSAGetLastError());
	}
	else
	{
		printf("sent %d bytes\n", rc);
	}
	closesocket(s) ;
	WSACleanup() ;

	return 0;
}

int UDPWin::receiveudp(char *srcip, char *dstip, int srcprt, int dstprt, char *buf, int bufsize)
{
	WSADATA				wsd;
	SOCKET				receivingSocket;
	char sendbuf[UDPSize]={0};
	int           iphdrlen,		allsize,		udphdrlen;
	int     optlevel,		option,		optval,		rc;  
	SOCKADDR_IN    ReceiverAddr;
	SOCKADDR_IN	SenderAddr;
	int			recvAddrSize	=	sizeof(SenderAddr);
	int			num=0;

	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{
		printf("WSAStartup() failed: %d\n", GetLastError());
		return -1;
	}

	//receivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	receivingSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (receivingSocket == INVALID_SOCKET)
	{
		fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
		return -1;
	}
	int nNetTimeout = 1000;				//设置接收超时
	if(setsockopt(receivingSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int))!=0){
		return FALSE;
	}

	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(srcprt);//(dstprt);    
	ReceiverAddr.sin_addr.s_addr = inet_addr(srcip);//(dstip);

	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = htons(dstprt);
	SenderAddr.sin_addr.s_addr = inet_addr(dstip);


	if(bind(receivingSocket,(SOCKADDR *)&ReceiverAddr,sizeof(ReceiverAddr)) == -1)
	{ 
		printf ("moon,you are wrong!: Failed to bind Port %d.\n",srcprt);
		return (0);
	}
	else
	{
		printf("OK，moon!: Bind the Port %d sucessfully.\n",srcprt);
	}
	printf("num = %d\n",num);
	num = recvfrom(receivingSocket,buf,bufsize,0,//|MSG_OOB,
		(SOCKADDR *)&SenderAddr,&recvAddrSize);
	if (num > 0)
	{
		printf("num = %d\n",num);
	}
	

	closesocket(receivingSocket);
	WSACleanup();

	return num;
}

bool UDPWin::GetLocalIP(char* ip)
{
	//1.初始化wsa  
	WSADATA wsaData;  
	int ret=WSAStartup(MAKEWORD(2,2),&wsaData);  
	if (ret!=0)  
	{  
		return false;  
	}  
	//2.获取主机名  
	char hostname[256];  
	ret=gethostname(hostname,sizeof(hostname));  
	if (ret==SOCKET_ERROR)  
	{  
		return false;  
	}  
	//3.获取主机ip  
	HOSTENT* host=gethostbyname(hostname);  
	if (host==NULL)  
	{  
		return false;  
	}  
	//4.转化为char*并拷贝返回  
	strcpy(ip,inet_ntoa(*(in_addr*)*host->h_addr_list));  
	return true;
}

bool UDPWin::GetLocalIPs(IPInfo* ips,int maxCnt,int* cnt)
{
	//1.初始化wsa  
	WSADATA wsaData;  
	int ret=WSAStartup(MAKEWORD(2,2),&wsaData);  
	if (ret!=0)  
	{  
		return false;  
	}  
	//2.获取主机名  
	char hostname[256];  
	ret=gethostname(hostname,sizeof(hostname));  
	if (ret==SOCKET_ERROR)  
	{  
		return false;  
	}  
	//3.获取主机ip  
	HOSTENT* host=gethostbyname(hostname);  
	if (host==NULL)  
	{  
		return false;  
	}  
	//4.逐个转化为char*并拷贝返回  
	*cnt=host->h_length<maxCnt?host->h_length:maxCnt;  
	for (int i=0;i<*cnt;i++)  
	{  
		in_addr* addr=(in_addr*)*host->h_addr_list;  
		strcpy(ips[i].ip,inet_ntoa(addr[i]));  
	}  
	return true; 
}

