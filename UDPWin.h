#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")
//#pragma comment(lib,"ws2_32.lib")

// Set the packing to a 1 byte boundary
#include <pshpack1.h>

// Define the IPv4 header. Make the version and length field one
// character since we can't declare two 4 bit fields without
// the compiler aligning them on at least a 1 byte boundary.
//
typedef struct ip_hdr
{
	unsigned char  ip_verlen;        // 4-bit IPv4 version
	// 4-bit header length (in 32-bit words)
	unsigned char  ip_tos;           // IP type of service
	unsigned short ip_totallength;   // Total length
	unsigned short ip_id;            // Unique identifier 
	unsigned short ip_offset;        // Fragment offset field
	unsigned char  ip_ttl;           // Time to live
	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
	unsigned short ip_checksum;      // IP checksum
	unsigned int   ip_srcaddr;       // Source address
	unsigned int   ip_destaddr;      // Source address
} IPV4_HDR, *PIPV4_HDR, FAR * LPIPV4_HDR;

//
// Define the UDP header 
//
typedef struct udp_hdr
{
	unsigned short src_portno;       // Source port no.
	unsigned short dst_portno;       // Dest. port no.
	unsigned short udp_length;       // Udp packet length
	unsigned short udp_checksum;     // Udp checksum (optional)
} UDP_HDR, *PUDP_HDR;

typedef struct tagIPInfo  
{  
	char ip[30];  
}IPInfo;

#define UDPSize	1052
// Restore the byte boundary back to the previous value
#include <poppack.h>

#pragma once
class UDPWin
{
public:
	UDPWin(void);
	~UDPWin(void);
private:
	// Function: checksum
	//
	// Description:
	//    This function calculates the 16-bit one's complement sum
	//    for the supplied buffer.
	//
	USHORT checksum(USHORT *buffer, int size);
	// Function: InitIpv4Header
	//
	// Description:
	//    Initialize the IPv4 header with the version, header length,
	//    total length, ttl, protocol value, and source and destination
	//    addresses.
	//
	int InitIpv4Header(char *buf, 	char *src, char *dest, 	int payloadlen);
	// Function: ComputeUdpPseudoHeaderChecksumV4
	//
	// Description:
	//    Compute the UDP pseudo header checksum. The UDP checksum is based
	//    on the following fields:
	//       o source IP address
	//       o destination IP address
	//       o 8-bit zero field
	//       o 8-bit protocol field
	//       o 16-bit UDP length
	//       o 16-bit source port
	//       o 16-bit destination port
	//       o 16-bit UDP packet length
	//       o 16-bit UDP checksum (zero)
	//       o UDP payload (padded to the next 16-bit boundary)
	//    This routine copies these fields to a temporary buffer and computes
	//    the checksum from that.
	//
	void ComputeUdpPseudoHeaderChecksumV4(void *iphdr,	UDP_HDR *udphdr, char *payload, int payloadlen);
	// Function: InitUdpHeader
	//
	// Description:
	//    Setup the UDP header which is fairly simple. Grab the ports and
	//    stick in the total payload length.
	//
	int InitUdpHeader(char *buf, int srcprt, int dstprt, int payloadlen);
public:
	// Function: sendudp
	//
	// Description:
	//    Send the udp packets with RAW SOCKET
	//
	int sendudp(char *srcip, char *dstip, int srcprt, int dstprt, char *buf, int bufsize);
	// Function: receiveudp
	//
	// Description:
	//    Receive the udp packets with RAW SOCKET
	//
	int receiveudp(char *srcip, char *dstip, int srcprt, int dstprt, char *buf, int bufsize);
	// Function: GetLocalIP
	//
	// Description:
	//    Get local IP address with string
	//
	bool GetLocalIP(char* ip);
	bool GetLocalIPs(IPInfo* ips,int maxCnt,int* cnt);
};

