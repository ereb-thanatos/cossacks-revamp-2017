
#include <winsock2.h> 
#include <stdio.h> 
#include <stdlib.h> 

struct OneIPAddress{
	DWORD IP;
	int LastPingTime;
	int LastRequestTime;
	int Ping;
};
class PingEngine{
public:
	SOCKET sockRaw; 
	bool IsInit;
	bool InitFailed;
	OneIPAddress* Requests;
	int NRequests;
	int GetPing(DWORD IP);
	void Process();
	int LastPingTime;
	void Setup();
	bool SendPingToIP(DWORD IP);
	PingEngine();
	~PingEngine();
};
 
#define ICMP_ECHO 8 
#define ICMP_ECHOREPLY 0 
 
#define ICMP_MIN 8 // minimum 8 byte icmp packet (just header) 
 
typedef struct iphdr { 
unsigned int h_len:4;          // length of the header 
unsigned int version:4;        // Version of IP 
unsigned char tos;             // Type of service 
unsigned short total_len;      // total length of the packet 
unsigned short ident;          // unique identifier 
unsigned short frag_and_flags; // flags 
unsigned char  ttl;  
unsigned char proto;           // protocol (TCP, UDP etc) 
unsigned short checksum;       // IP checksum 
 
unsigned int sourceIP; 
unsigned int destIP; 
 
}IpHeader; 
 
// 
// ICMP header 
// 
typedef struct _ihdr { 
  BYTE i_type; 
  BYTE i_code; 
  USHORT i_cksum; 
  USHORT i_id; 
  USHORT i_seq; 
  ULONG timestamp; 
}IcmpHeader; 
 
#define STATUS_FAILED 0xFFFF 
#define DEF_PACKET_SIZE 4 
#define MAX_PACKET 1024 
 
#define xmalloc(s) malloc(s)
#define xfree(p)   free(p)
 
void fill_icmp_data(char *, int); 
USHORT checksum(USHORT *, int); 
void decode_resp(char *,int ,struct sockaddr_in *); 
PingEngine::PingEngine(){
	memset(this,0,sizeof *this);
};
PingEngine::~PingEngine(){
	if(IsInit){

	};
};
void PingEngine::Setup(){
	IsInit=0;
	if(InitFailed||IsInit)return;
	InitFailed=1;
	WSADATA wsaData; 
	int bread; 
	int timeout = 1; 
 	if (WSAStartup(MAKEWORD(2,1),&wsaData) != 0)return;
 	sockRaw = WSASocket (AF_INET,SOCK_RAW,IPPROTO_ICMP,NULL,0,0); 
 	if (sockRaw == INVALID_SOCKET)return;
	bread = setsockopt(sockRaw,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)); 
	if(bread == SOCKET_ERROR)return;
	timeout = 1000; 
	bread = setsockopt(sockRaw,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout)); 
	if(bread == SOCKET_ERROR)return;
	u_long	lArgP=0x01;
	ioctlsocket(sockRaw,FIONBIO,&lArgP);
	IsInit=1;
};
USHORT checksum(USHORT *buffer, int size);
int seq_no=0;
bool PingEngine::SendPingToIP(DWORD IP){
	sockaddr_in dest;
	memset(&dest,0,sizeof(dest)); 
 	dest.sin_addr.s_addr = IP; 
	dest.sin_family = AF_INET; 
	int datasize = DEF_PACKET_SIZE; 
 	datasize += sizeof(IcmpHeader);   
 	char icmp_data[MAX_PACKET];
	memset(icmp_data,0,MAX_PACKET); 
	fill_icmp_data(icmp_data,datasize); 
	((IcmpHeader*)icmp_data)->i_cksum = 0; 
	((IcmpHeader*)icmp_data)->timestamp = GetTickCount(); 
	((IcmpHeader*)icmp_data)->i_seq = (USHORT)seq_no++; 
	((IcmpHeader*)icmp_data)->i_cksum = checksum((USHORT*)icmp_data,datasize); 
	DWORD bwrote = sendto(sockRaw,icmp_data,datasize,0,(struct sockaddr*)&dest,sizeof(dest));
	if (bwrote == SOCKET_ERROR)return false;
	return true;
};
int pcallT=0;
bool decode_resp(char *buf, int bytes,struct sockaddr_in *from,int* time);
void PingEngine::Process(){
	if(!IsInit)return;
	int T=GetTickCount();
	if(!pcallT)pcallT=T;
	if(T-pcallT<10)return;
	pcallT=T;
	char recvbuf[MAX_PACKET];
	sockaddr_in from;
	int L=sizeof from;
	DWORD bread = recvfrom(sockRaw,recvbuf,MAX_PACKET,0,(sockaddr*)&from,&L); 
	if (bread != SOCKET_ERROR){ 
		int dt;
		if(decode_resp(recvbuf,bread,&from,&dt)&&dt<4000){
			DWORD IP=from.sin_addr.s_addr;
			for(int i=0;i<NRequests;i++){
				if(Requests[i].IP==IP){
					if(!Requests[i].Ping)Requests[i].Ping=dt;
					else Requests[i].Ping=(Requests[i].Ping*3+dt)>>2;
					if(Requests[i].Ping<1)Requests[i].Ping=1;
				};
			};
		}; 
	};
	if(T-LastPingTime>4000){
		int BestID=-1;
		int LastT=T;
		for(int i=0;i<NRequests;i++){
			if(T-Requests[i].LastRequestTime<1000){
				int rt=Requests[i].LastPingTime;
				if(rt==-1&&LastT!=-1){
					LastT=-1;
					BestID=i;
					break;
				}else
				if(LastT!=-1&&rt<LastT){
					LastT=rt;
					BestID=i;
				};
			};
		};
		if(BestID!=-1){
			//pinging
			if(SendPingToIP(Requests[BestID].IP)){
				LastPingTime=T;
				Requests[BestID].LastPingTime=T;
			}else LastPingTime=T-900;
		};
	};
	
};
int PingEngine::GetPing(DWORD IP){
	if(!IsInit)return false;
	for(int i=0;i<NRequests;i++)if(Requests[i].IP==IP){
		Requests[i].LastRequestTime=GetTickCount();
		return Requests[i].Ping;
	};
	Requests=(OneIPAddress*)realloc(Requests,(NRequests+1)*sizeof OneIPAddress);
	memset(Requests+NRequests,0,sizeof OneIPAddress);
	Requests[NRequests].LastPingTime=-1;
	Requests[NRequests].IP=IP;
	Requests[NRequests].LastRequestTime=GetTickCount();
	NRequests++;
	return 0;
};
USHORT checksum(USHORT *buffer, int size){ 
 
  unsigned long cksum=0; 
 
  while(size >1) { 
cksum+=*buffer++; 
size -=sizeof(USHORT); 
  } 
   
  if(size ) { 
cksum += *(UCHAR*)buffer; 
  } 
 
  cksum = (cksum >> 16) + (cksum & 0xffff); 
  cksum += (cksum >>16); 
  return (USHORT)(~cksum); 
} 
void fill_icmp_data(char * icmp_data, int datasize){ 
 
  IcmpHeader *icmp_hdr; 
  char *datapart; 
 
  icmp_hdr = (IcmpHeader*)icmp_data; 
 
  icmp_hdr->i_type = ICMP_ECHO; 
  icmp_hdr->i_code = 0; 
  icmp_hdr->i_id = (USHORT)GetCurrentProcessId(); 
  icmp_hdr->i_cksum = 0; 
  icmp_hdr->i_seq = 0; 
   
  datapart = icmp_data + sizeof(IcmpHeader); 
  // 
  // Place some junk in the buffer. 
  // 
  memset(datapart,'E', datasize - sizeof(IcmpHeader)); 
 
} 
bool decode_resp(char *buf, int bytes,struct sockaddr_in *from,int* time) { 
 	IpHeader *iphdr; 
	IcmpHeader *icmphdr; 
	unsigned int iphdrlen; 
	iphdr = (IpHeader *)buf; 
 	iphdrlen = iphdr->h_len * 4 ; // number of 32-bit words *4 = bytes 
 	icmphdr = (IcmpHeader*)(buf + iphdrlen); 
 	if (icmphdr->i_type != ICMP_ECHOREPLY) { 
		return false;
	} 
	if (icmphdr->i_id != (USHORT)GetCurrentProcessId()) { 
		return false;
	};
	*time=GetTickCount()-icmphdr->timestamp;
	return true;
}
PingEngine PE;
int GETPING(DWORD IP){
	if(!PE.IsInit)PE.Setup();
	PE.Process();
	return PE.GetPing(IP);
};