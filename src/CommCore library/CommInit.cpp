// Client/Server initialization & finalization
//
#include "CommCore.h"

// ---------------------------------------------------------------------------------------------
#ifdef CC_DEBUG
VOID CCommCore::DebugMessage( LPCSTR lpcszMessage )
{
	SYSTEMTIME		SysTime;
	GetLocalTime( &SysTime );

	m_DebugStream = fopen( "C:\\CommCore.log", "a+" );

	fprintf( m_DebugStream, "%2.2u:%2.2u:%2.2u.%3.3u		%s\n",
		SysTime.wHour,
		SysTime.wMinute,
		SysTime.wSecond,
		SysTime.wMilliseconds,
		lpcszMessage );

	fclose( m_DebugStream );
}
#endif //CC_DEBUG


CCommCore::CCommCore()
{
	m_uFrameCount = 0;
	m_lStamp = 0;
	m_uPeerCount = 0;
	m_bServer = TRUE;
	m_piNumber = 0;
	m_ssState = ssNone;
	m_csState = csNone;
	m_bBlockingCall = FALSE;
	m_piAutoInc = 0;
	m_lpbRecvBuffer = (LPBYTE) malloc( RECV_BUFFER_LENGTH );
	assert( m_lpbRecvBuffer );
	m_uMaxMsgSize = 0;
	lpIdleProc = NULL;
	lpEnumProc = NULL;
	m_szUserName[0] = '\0';
	m_szSessionName[0] = '\0';
	m_paServAddr.s_addr = inet_addr( "0.0.0.0" );
	m_paServPort = 0;
	m_lpbUserData = NULL;
	m_uUserDataSize = 0;
	m_bBlockingCall = FALSE;
	m_uMaxPeers = MAX_PEERS;
	GetCommCoreUID( m_szCCUID );
	m_uAddrCount = 0;
	m_dwOptions = 0;
	m_dwRxBytes = 0;
	m_dwTxBytes = 0;
	m_dwNxBytes = 0;

#ifdef CC_DEBUG
	DebugMessage( "-- Log Started ---------------------------------" );
#endif //CC_DEBUG
}

// ---------------------------------------------------------------------------------------------

CCommCore::~CCommCore()
{
#ifdef CC_DEBUG
	DebugMessage( "-- Log Stopped ---------------------------------" );
#endif //CC_DEBUG
	if (m_lpbRecvBuffer)
		free( m_lpbRecvBuffer );
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::InitServer( LPCSTR lpcszSessionName, LPCSTR lpcszUserName )
{
	_log_message( "InitServer()" );

	m_dwRxBytes = 0;
	m_dwTxBytes = 0;
	m_dwNxBytes = 0;

	strcpy( m_szUserName, lpcszUserName );

	m_bServer = TRUE;

	m_piNumber = 1;
	m_piAutoInc = 1;
	m_uPeerCount = 1;

	m_PeerList[0].m_bAlive = TRUE;
	m_PeerList[0].m_Id = 1;
	m_PeerList[0].m_uLatency = 0;
	m_PeerList[0].m_bOverNAT = FALSE;
	m_PeerList[0].m_ex_Addr.s_addr = m_dwAddrList[0];//??
	m_PeerList[0].m_ex_Port = htons( DATA_PORT );
	m_PeerList[0].m_lpbUserData = m_lpbUserData;
	m_PeerList[0].m_uUserDataSize = m_uUserDataSize;
	strcpy( m_PeerList[0].m_szUserName, m_szUserName );
	strcpy( m_szSessionName, lpcszSessionName );
	memcpy( m_PeerList[0].m_szCCUID, m_szCCUID, 22 );

	m_ssState = ssOpen;

	m_uMaxPeers = MAX_PEERS;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::InitClient( LPCSTR lpcszServerIP, LPCSTR lpcszUserName, unsigned short port )
{
	_log_message( "InitClient()" );

	m_dwRxBytes = 0;
	m_dwTxBytes = 0;
	m_dwNxBytes = 0;

	m_piAutoInc = 1;
	m_uPeerCount = 0;

	strcpy( m_szUserName, lpcszUserName );

	LPCC_PK_TRY_CONNECT	lpConnectPacket;

	lpConnectPacket = (LPCC_PK_TRY_CONNECT) malloc( sizeof( CC_PK_TRY_CONNECT ) + ( m_uAddrCount * sizeof( DWORD ) ) );

	m_bServer = FALSE;

	m_piNumber = BAD_PEER_ID;

	lpConnectPacket->m_cProtoVersion = CC_PROTO_VERSION;
	lpConnectPacket->m_uAddrCount = m_uAddrCount;
	strcpy( lpConnectPacket->m_szUserName, m_szUserName );
	memcpy( lpConnectPacket->m_szCCUID, m_szCCUID, 22 );
	memcpy( lpConnectPacket->m_dwAddrList, m_dwAddrList, ( m_uAddrCount * sizeof( DWORD ) ) );

	m_paServAddr.s_addr = inet_addr( lpcszServerIP );
	if (0 != port)
	{
		m_paServPort = htons( port );
	}
	else
	{
		m_paServPort = htons( DATA_PORT );
	}

	m_csState = csWait;

	if (!SendRawPacket( m_paServAddr, m_paServPort,
		CC_PT_TRY_CONNECT,
		(LPBYTE) lpConnectPacket,
		sizeof( CC_PK_TRY_CONNECT ) + ( m_uAddrCount * sizeof( DWORD ) ),
		TRUE,
		FALSE ))
	{
		free( lpConnectPacket );
		return FALSE;
	};

	free( lpConnectPacket );

	DWORD dwTime = GetTickCount();

	while (( ( GetTickCount() - dwTime ) < ( RETRY_TIME*( RETRY_COUNT + 3 ) ) ) && m_csState == csWait)
	{

		ReceiveData( NULL );
		QueueProcess();
		if (lpIdleProc)
			lpIdleProc();
	};

	if (m_csState != csConnected)
		return FALSE;

	m_ssState = ssOpen;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::DoneClient()
{
	_log_message( "DoneClient()" );

	CC_PK_HOST_EXIT		HostExitPacket;

	HostExitPacket.m_dwReserved = 0xFF;

	SendRawPacket( m_paServAddr,
		m_paServPort,
		CC_PT_HOST_EXIT,
		(LPBYTE) &HostExitPacket,
		sizeof( CC_PK_HOST_EXIT ),
		TRUE,
		FALSE );

	QueueClearAll();

	Cleanup();

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::DoneServer()
{
	_log_message( "DoneServer()" );

	int i;

	for (i = 1; i < m_uPeerCount; i++)
		SendDropClient( m_PeerList[i].m_Id );
	/*
		while(m_uFrameCount>0){
			ReceiveData(NULL);
			QueueProcess();
			if(lpIdleProc)
				lpIdleProc();
		};
	*/

	QueueClearAll();

	Cleanup();

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

VOID CCommCore::Cleanup()
{
	_log_message( "Cleanup()" );

	for (int i = 0; i < m_uPeerCount; i++)
		if (m_PeerList[i].m_lpbUserData)
		{
			free( m_PeerList[i].m_lpbUserData );
			m_PeerList[i].m_lpbUserData = NULL;
			m_PeerList[i].m_uUserDataSize = 0;
		}

	m_uPeerCount = 0;

	if (m_lpbUserData)
		free( m_lpbUserData );

	m_lpbUserData = NULL;
	m_uUserDataSize = 0;

	m_dwTxBytes = 0;
	m_dwNxBytes = 0;
	m_dwRxBytes = 0;

	m_csState = csNone;
	m_ssState = ssNone;
};