// HttpComm.cpp: implementation of the CHttpComm class.
//
//////////////////////////////////////////////////////////////////////
//	int i=0;
//	while( (*(LPDWORD)(lpbBuffer+(i++))!=0x0A0D0A0D) && (i<dwRecvSize) );

#include <windows.h>
#pragma pack(1)
#include "HttpComm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHttpComm::CHttpComm()
{
	m_bInitialized = FALSE;

	WSADATA WSAData;

	// -- WSAStartup --------------------------------------------------------
	if (WSAStartup( MAKEWORD( 2, 2 ), &WSAData ) != 0)
	{
		return;
	}
	// -- creating query list -----------------------------------------------
	m_dwHandleAuto = 1;
	m_dwRequestCount = 0;
	m_pRequestList = (PHttpQuery) malloc( m_dwRequestCount * sizeof( SHttpQuery ) );
	// -- checking for proxy settings ---------------------------------------
	m_bUseProxy = FALSE;
	m_szProxyAddr[0] = '\0';
	m_dwProxyPort = 0;

	DWORD	dwSiz;

	HKEY hGscKey;

	if (RegOpenKeyEx( HKEY_CURRENT_USER,
		"Software\\GSC Game World",
		0x00,
		KEY_READ,
		&hGscKey ) == ERROR_SUCCESS)
	{

		dwSiz = 4;
		RegQueryValueEx( hGscKey,
			"httpcUseProxy",
			NULL,
			NULL,
			(LPBYTE) &m_bUseProxy,
			(LPDWORD) &dwSiz );
		dwSiz = 4;
		RegQueryValueEx( hGscKey,
			"httpcProxyPort",
			NULL,
			NULL,
			(LPBYTE) &m_dwProxyPort,
			(LPDWORD) &dwSiz );
		dwSiz = 256;
		RegQueryValueEx( hGscKey,
			"httpcProxyAddr",
			NULL,
			NULL,
			(LPBYTE) m_szProxyAddr,
			(LPDWORD) &dwSiz );

		RegCloseKey( hGscKey );
	};
	// -- All Ok ------------------------------------------------------------
	m_bInitialized = TRUE;
}

CHttpComm::~CHttpComm()
{
	if (!m_bInitialized)
		return;
	// -- WSACleanup --------------------------------------------------------
	if (WSACleanup() == SOCKET_ERROR)
	{
		return;
	};
	// -- All Ok ------------------------------------------------------------
}

DWORD CHttpComm::AddRequest( LPCSTR lpcszURL )
{
	if (!m_bInitialized)
		return 0;

	CHAR		szHostName[255];
	LPCSTR		lpszTemp = nullptr;

	m_pRequestList = (PHttpQuery) realloc( m_pRequestList, ( ++m_dwRequestCount ) * sizeof( SHttpQuery ) );
	// -- completing structure -----------------------------------------------
	m_pRequestList[m_dwRequestCount - 1].m_Stage = csConnecting;
	m_pRequestList[m_dwRequestCount - 1].m_dwHandle = m_dwHandleAuto;
	m_pRequestList[m_dwRequestCount - 1].m_lpvBuffer = malloc( RECV_BUFFER_SIZE );
	m_pRequestList[m_dwRequestCount - 1].m_lpszQuery = NULL;
	// -- socket ------------------------------------------------------------
	m_pRequestList[m_dwRequestCount - 1].m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (m_pRequestList[m_dwRequestCount - 1].m_Socket == INVALID_SOCKET)
	{
		FreeData( m_dwHandleAuto );
		return 0;
	};
	// -- non-blocking calls ------------------------------------------------
	u_long	lArgP = 0x01;
	if (ioctlsocket( m_pRequestList[m_dwRequestCount - 1].m_Socket, FIONBIO, &lArgP ) == SOCKET_ERROR)
	{
		FreeData( m_dwHandleAuto );
		return 0;
	};
	// -- bind --------------------------------------------------------------
	sockaddr_in locaddr;

	memset( &locaddr, 0x00, sizeof( sockaddr_in ) );

	locaddr.sin_family = AF_INET;
	locaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	locaddr.sin_port = htons( 0x00 );

	if (bind( m_pRequestList[m_dwRequestCount - 1].m_Socket, (sockaddr *) &locaddr, sizeof( sockaddr_in ) ) == SOCKET_ERROR)
	{
		FreeData( m_dwHandleAuto );
		return 0;
	};
	// -- grep for host name ------------------------------------------------
	if (m_bUseProxy)
	{
		strcpy( szHostName, m_szProxyAddr );
	}
	else
	{
		lpszTemp = strchr( lpcszURL, '/' );
		if (lpszTemp)
		{
			strncpy( szHostName, lpcszURL, ( lpszTemp - lpcszURL ) );
			szHostName[lpszTemp - lpcszURL] = '\0';
		}
		else
			strcpy( szHostName, lpcszURL );
	};
	// -- resolving site/proxy name to IPv4 address
	HOSTENT * pHostEnt;

	pHostEnt = gethostbyname( szHostName );

	if (!pHostEnt)
	{
		FreeData( m_dwHandleAuto );
		return 0;
	};

	in_addr	iaFirstAddress;

	memcpy( &iaFirstAddress, pHostEnt->h_addr_list[0], pHostEnt->h_length );
	// -- connect -----------------------------------------------------------
	sockaddr_in destaddr;

	memset( &destaddr, 0x00, sizeof( sockaddr_in ) );

	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = iaFirstAddress.s_addr;
	destaddr.sin_port = m_bUseProxy ? htons( m_dwProxyPort ) : htons( 80 );

	if (connect( m_pRequestList[m_dwRequestCount - 1].m_Socket, (sockaddr *) &destaddr, sizeof( sockaddr_in ) ) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			FreeData( m_dwHandleAuto );
			return 0;
		};
	};
	// -- malloc ------------------------------------------------------------
	m_pRequestList[m_dwRequestCount - 1].m_lpszQuery = (LPSTR) malloc( strlen( lpcszURL ) + 255 );
	if (!m_pRequestList[m_dwRequestCount - 1].m_lpszQuery)
	{
		FreeData( m_dwHandleAuto );
		return 0;
	};
	// -- preparing string --------------------------------------------------
	strcpy( m_pRequestList[m_dwRequestCount - 1].m_lpszQuery, "GET " );
	if (m_bUseProxy)
	{
		strcat( m_pRequestList[m_dwRequestCount - 1].m_lpszQuery, "http://" );
		strcat( m_pRequestList[m_dwRequestCount - 1].m_lpszQuery, lpcszURL );
	}
	else
	{
		strcat( m_pRequestList[m_dwRequestCount - 1].m_lpszQuery, lpszTemp ? lpszTemp : "/" );
	};
	strcat( m_pRequestList[m_dwRequestCount - 1].m_lpszQuery, " HTTP/1.0\n\n" );
	// -- returning handle ---------------------------------------------------
	return m_dwHandleAuto++;
}

VOID CHttpComm::ProcessRequests()
{
	for (DWORD i = 0; i < m_dwRequestCount; i++)
		switch (m_pRequestList[i].m_Stage)
		{
		case csConnecting:
		{
			fd_set write_fds;
			fd_set except_fds;
			struct timeval TimeOut;
			FD_ZERO( &write_fds );
			FD_SET( m_pRequestList[i].m_Socket, &write_fds );
			FD_ZERO( &except_fds );
			FD_SET( m_pRequestList[i].m_Socket, &except_fds );
			TimeOut.tv_sec = 0;
			TimeOut.tv_usec = 500;
			if (select( 0, NULL, &write_fds, &except_fds, &TimeOut ) != SOCKET_ERROR)
			{
				if (FD_ISSET( m_pRequestList[i].m_Socket, &except_fds ))
				{
					FreeData( m_pRequestList[i].m_dwHandle );
					return;
				};
				if (FD_ISSET( m_pRequestList[i].m_Socket, &write_fds ))
				{
					if (send( m_pRequestList[i].m_Socket, m_pRequestList[i].m_lpszQuery, strlen( m_pRequestList[i].m_lpszQuery ), 0 ) != SOCKET_ERROR)
					{
						m_pRequestList[i].m_Stage = csSending;
					}
				};
			};
			break;
		};
		case csSending:
		{
			fd_set	read_fds;
			fd_set except_fds;
			int		iRecvSize;
			struct timeval TimeOut;
			FD_ZERO( &read_fds );
			FD_SET( m_pRequestList[i].m_Socket, &read_fds );
			FD_ZERO( &except_fds );
			FD_SET( m_pRequestList[i].m_Socket, &except_fds );
			TimeOut.tv_sec = 0;
			TimeOut.tv_usec = 500;
			if (select( 0, NULL, &read_fds, &except_fds, &TimeOut ) != SOCKET_ERROR)
			{
				if (FD_ISSET( m_pRequestList[i].m_Socket, &except_fds ))
				{
					FreeData( m_pRequestList[i].m_dwHandle );
					return;
				};
				if (FD_ISSET( m_pRequestList[i].m_Socket, &read_fds ))
				{
					if (( iRecvSize = recv( m_pRequestList[i].m_Socket, (LPSTR) m_pRequestList[i].m_lpvBuffer, RECV_BUFFER_SIZE, 0 ) ) != SOCKET_ERROR)
					{
						m_pRequestList[i].m_Stage = csWaiting;
						m_pRequestList[i].m_dwDataSize = iRecvSize;
						m_pRequestList[i].m_lpvBuffer = realloc( m_pRequestList[i].m_lpvBuffer, iRecvSize );
						closesocket( m_pRequestList[i].m_Socket );
					}
				};
			};
			break;
		};
		case csReceiving:	break;
		case csWaiting:	break;
		};

}


int CHttpComm::GetData( DWORD dwHandle, LPVOID lpvBuffer, DWORD dwBufferSize )
{
	for (DWORD i = 0; i < m_dwRequestCount; i++)
	{
		if (m_pRequestList[i].m_dwHandle == dwHandle && m_pRequestList[i].m_Stage == csWaiting)
		{
			if (dwBufferSize < m_pRequestList[i].m_dwDataSize)
			{
				return 0;
			}

			DWORD j = 0;
			while (
				( *(LPDWORD) ( (LPBYTE) m_pRequestList[i].m_lpvBuffer
					+ ( j++ ) ) != 0x0A0D0A0D )
				&& ( j < m_pRequestList[i].m_dwDataSize )
				);

			memcpy( lpvBuffer, (LPBYTE) m_pRequestList[i].m_lpvBuffer + j, m_pRequestList[i].m_dwDataSize - j );

			return m_pRequestList[i].m_dwDataSize - j;
		}
	}
	return 0;
}


VOID CHttpComm::FreeData( DWORD dwHandle )
{
	for (DWORD i = 0; i < m_dwRequestCount; i++)
		if (m_pRequestList[i].m_dwHandle == dwHandle)
		{
			if (m_pRequestList[i].m_lpszQuery)
				free( m_pRequestList[i].m_lpszQuery );
			if (m_pRequestList[i].m_lpvBuffer)
				free( m_pRequestList[i].m_lpvBuffer );
			memmove( &m_pRequestList[i], &m_pRequestList[i + 1], ( m_dwRequestCount - i ) * sizeof( SHttpQuery ) );
			m_pRequestList = (PHttpQuery) realloc( m_pRequestList, ( --m_dwRequestCount ) * sizeof( SHttpQuery ) );
			return;
		};
}

