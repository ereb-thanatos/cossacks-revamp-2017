// Network initialization part
//
#include "CommCore.h"

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::InitNetwork()
{
	_log_message("InitNetwork()");

	WSADATA WSAData;

	if(WSAStartup(MAKEWORD(2,2),&WSAData)!=0)
		return FALSE;

	if(!InitSocket())
		return FALSE;

	if(!InitHost())
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::InitHost()
{
	_log_message("InitHost()");

	if(gethostname(m_szUserName,255)==SOCKET_ERROR)
		return FALSE;

	HOSTENT * pHostEnt;

	pHostEnt=gethostbyname(m_szUserName);

	if(!pHostEnt)
		return FALSE;

	m_uAddrCount=0;

	while(pHostEnt->h_addr_list[m_uAddrCount]){
		memcpy(&m_dwAddrList[m_uAddrCount],pHostEnt->h_addr_list[m_uAddrCount],pHostEnt->h_length);
		m_uAddrCount++;
	};

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::CloseNetwork()
{
	_log_message("CloseNetwork()");

	if(!CloseSocket())
		return FALSE;

	if(WSACleanup()==SOCKET_ERROR)
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::InitSocket()
{
	_log_message("InitSocket()");

	m_DataSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(m_DataSocket==INVALID_SOCKET)
		return FALSE;

	u_long	lArgP=0x01;
	if(ioctlsocket(m_DataSocket,FIONBIO,&lArgP)==SOCKET_ERROR)
		return FALSE;

	u_long	lMaxSize;
	int		iSizeOfMaxSize=sizeof(u_long);
	if(getsockopt(m_DataSocket,SOL_SOCKET,SO_MAX_MSG_SIZE,(char *)&lMaxSize,&iSizeOfMaxSize)==SOCKET_ERROR)
		return FALSE;

	m_uMaxMsgSize=(u_short)lMaxSize;

	sockaddr_in locaddr;

	locaddr.sin_family=AF_INET;
	locaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	locaddr.sin_port=htons(DATA_PORT);

	if(bind(m_DataSocket,(sockaddr *)&locaddr,sizeof(sockaddr_in))==SOCKET_ERROR)
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::CloseSocket()
{
	_log_message("CloseSocket()");

	if(closesocket(m_DataSocket)==SOCKET_ERROR)
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------
