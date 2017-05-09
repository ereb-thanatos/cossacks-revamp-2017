

#include "CommCore.h"
#include <stdlib.h>
#include <stdio.h>

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SetSessionName( LPCSTR lpcszSessionName )
{
	_log_message( "SetSessionName()" );

	strcpy( m_szSessionName, lpcszSessionName );
	return TRUE;
}

// ---------------------------------------------------------------------------------------------

LPCSTR CCommCore::GetUserName( PEER_ID PeerId )
{
	_log_message( "GetUserName()" );

	if (PeerId == m_piNumber)
		return m_szUserName;

	u_short uPeerNum = GetPeerById( PeerId );

	if (uPeerNum == BAD_PEER_ID)
		return NULL;

	return m_PeerList[uPeerNum].m_szUserName;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SetUserName( LPCSTR lpcszUserName )
{
	_log_message( "SetUserName()" );

	strcpy( m_szUserName, lpcszUserName );

	return TRUE;//SendUserName();
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendUserName()
{
	_log_message( "SendUserName()" );

	if (m_bServer)
	{

		strcpy( m_PeerList[0].m_szUserName, m_szUserName );

		//		return SendServerList();
		return SendNewName( m_piNumber );

	}
	else
	{

		CC_PK_SEND_USER_NAME	SendUserNamePacket;

		strcpy( SendUserNamePacket.m_szUserName, m_szUserName );

		return SendRawPacket( m_paServAddr,
			m_paServPort,
			CC_PT_SEND_USER_NAME,
			(LPBYTE) &SendUserNamePacket,
			sizeof( CC_PK_SEND_USER_NAME ),
			TRUE,
			FALSE );
	};
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::GetUserData( PEER_ID PeerId, LPBYTE lpbUserData, u_short * puUserDataSize )
{
	_log_message( "GetUserData()" );

	if (PeerId == m_piNumber)
	{
		memcpy( lpbUserData, m_lpbUserData, m_uUserDataSize );
		if (puUserDataSize)
		{
			*puUserDataSize = m_uUserDataSize;
		}
		return TRUE;
	}

	u_short uPeerNum = GetPeerById( PeerId );

	if (uPeerNum == BAD_PEER_ID)
	{
		if (puUserDataSize)
		{
			*puUserDataSize = 0;
		}
		return FALSE;
	}

	memcpy( lpbUserData, m_PeerList[uPeerNum].m_lpbUserData, m_PeerList[uPeerNum].m_uUserDataSize );

	if (puUserDataSize)
	{
		*puUserDataSize = m_PeerList[uPeerNum].m_uUserDataSize;
	}

	return TRUE;
}


// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SetUserData( const LPBYTE lpcbUserData, u_short uUserDataSize )
{
	_log_message( "SetUserData()" );

	if (m_lpbUserData)
	{
		free( m_lpbUserData );
	}

	m_lpbUserData = (LPBYTE) malloc( uUserDataSize );
	assert( m_lpbUserData );

	m_uUserDataSize = uUserDataSize;

	memcpy( m_lpbUserData, lpcbUserData, uUserDataSize );

	return TRUE;//SendUserData();
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendUserData()
{
	_log_message( "SendUserData()" );

	if (!m_lpbUserData)
	{
		return FALSE;
	}

	if (m_bServer)
	{

		free( m_PeerList[0].m_lpbUserData );

		m_PeerList[0].m_uUserDataSize = m_uUserDataSize;
		m_PeerList[0].m_lpbUserData = (LPBYTE) malloc( m_uUserDataSize );
		assert( m_PeerList[0].m_lpbUserData );

		memcpy( m_PeerList[0].m_lpbUserData, m_lpbUserData, m_uUserDataSize );
		m_PeerList[0].m_uUserDataSize = m_uUserDataSize;

		//		return SendServerList();
		return SendNewData( m_piNumber );
	}
	else
	{

		LPCC_PK_SEND_USER_DATA	lpSendUserDataPacket;

		lpSendUserDataPacket = (LPCC_PK_SEND_USER_DATA) malloc( sizeof( CC_PK_SEND_USER_DATA ) + m_uUserDataSize );
		assert( lpSendUserDataPacket );

		lpSendUserDataPacket->m_uUserDataSize = m_uUserDataSize;

		memcpy( lpSendUserDataPacket->m_UserData, m_lpbUserData, m_uUserDataSize );

		BOOL bRes = SendRawPacket( m_paServAddr,
			m_paServPort,
			CC_PT_SEND_USER_DATA,
			(LPBYTE) lpSendUserDataPacket,
			sizeof( CC_PK_SEND_USER_DATA ) + m_uUserDataSize,
			TRUE,
			FALSE );
		free( lpSendUserDataPacket );

		return bRes;
	};
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendDropClient( PEER_ID PeerID )
{
	_log_message( "SendDropClient()" );

	CC_PK_HOST_DROP		HostDropPacket;

	u_short	uPeerNum;

	uPeerNum = GetPeerById( PeerID );

	if (BAD_PEER_ID == uPeerNum)
	{
		return FALSE;
	}

	HostDropPacket.m_dwReserved = 0xFF;

	return SendRawPacket( m_PeerList[uPeerNum].m_ex_Addr,
		m_PeerList[uPeerNum].m_ex_Port,
		CC_PT_HOST_DROP,
		(LPBYTE) &HostDropPacket,
		sizeof( CC_PK_HOST_DROP ),
		TRUE,
		FALSE );
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendToAll( LPBYTE lpbBuffer, u_short u_Size, BOOL bSecure )
{
	_log_message( "SendToAll()" );

	for (int i = 0; i < m_uPeerCount; i++)
		if (!SendToPeer( m_PeerList[i].m_Id, lpbBuffer, u_Size, bSecure ))
			return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendToPeer( PEER_ID piNumber, LPBYTE lpbBuffer, u_short uSize, BOOL bSecure )
{
	_log_message( "SendToPeer()" );

	if (piNumber == m_piNumber)
		return TRUE;

	LPCC_PK_SEND_DATA	lpFrame;
	u_short				uFrameSize;

	uFrameSize = sizeof( CC_PK_SEND_DATA ) + uSize;

	lpFrame = (LPCC_PK_SEND_DATA) malloc( uFrameSize );

	lpFrame->m_uSize = uSize;
	memcpy( lpFrame->m_bData, lpbBuffer, uSize );

	u_short	uPeerNum = GetPeerById( piNumber );
	if (uPeerNum == BAD_PEER_ID)
		return FALSE;

	BOOL bRes = SendRawPacket( ( uPeerNum == 0 ) ? m_paServAddr : m_PeerList[uPeerNum].m_ex_Addr,
		m_PeerList[uPeerNum].m_ex_Port,
		CC_PT_SEND_DATA,
		(LPBYTE) lpFrame,
		uFrameSize,
		bSecure,
		FALSE );

	free( lpFrame );

	return bRes;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendConfirmDataPacket( sockaddr_in *lpSender, u_long lStamp )
{
	_log_message( "SendConfirmDataPacket()" );

	CC_PK_FRAME_CONFIRM		ConfPacket;

	ConfPacket.m_lConfirmStamp = lStamp;

	if (!SendRawPacket( lpSender->sin_addr,
		lpSender->sin_port,
		CC_PT_FRAME_CONFIRM,
		(LPBYTE) &ConfPacket,
		sizeof( CC_PK_FRAME_CONFIRM ),
		FALSE,
		FALSE ))
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendConnectReject( sockaddr_in *lpSender, u_short uReason )
{
	_log_message( "SendConnectReject()" );

	CC_PK_CONNECT_REJECT	RejectPacket;

	RejectPacket.m_uReason = uReason;

	if (!SendRawPacket( lpSender->sin_addr,
		lpSender->sin_port,
		CC_PT_CONNECT_REJECT,
		(LPBYTE) &RejectPacket,
		sizeof( CC_PK_CONNECT_REJECT ),
		TRUE,
		FALSE ))
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendConnectOk( sockaddr_in *lpSender, PEER_ID PeerId )
{
	_log_message( "SendConnectOk()" );

	CC_PK_CONNECT_OK	ConnectOkPacket;

	ConnectOkPacket.m_Id = PeerId;
	ConnectOkPacket.m_dwOptions = m_dwOptions;

	strcpy( ConnectOkPacket.m_szSessionName, m_szSessionName );

	if (!SendRawPacket( lpSender->sin_addr,
		lpSender->sin_port,
		CC_PT_CONNECT_OK,
		(LPBYTE) &ConnectOkPacket,
		sizeof( CC_PK_CONNECT_OK ),
		TRUE,
		FALSE ))
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendDropOk( sockaddr_in *lpSender )
{
	_log_message( "SendDropOk()" );

	CC_PK_HOST_DROP_OK	HostDropOkPacket;

	HostDropOkPacket.m_dwReserved = 0x00;

	if (!SendRawPacket( lpSender->sin_addr,
		lpSender->sin_port,
		CC_PT_HOST_DROP_OK,
		(LPBYTE) &HostDropOkPacket,
		sizeof( CC_PK_HOST_DROP_OK ),
		TRUE,
		FALSE ))
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

VOID CCommCore::SetCommCoreUID( LPCSTR lpcszCCUID )
{
	_log_message( "SetCommCoreUID()" );

	HKEY	hKey;

	RegCreateKeyEx( HKEY_CURRENT_USER,
		"Software\\GSC Game World",
		0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL );

	RegSetValueEx( hKey, "CCUID", 0, REG_SZ, (unsigned char*) lpcszCCUID, strlen( lpcszCCUID ) + 1 );

	RegCloseKey( hKey );
}

// ---------------------------------------------------------------------------------------------

VOID CCommCore::NewCommCoreUID( LPSTR lpszCCUID )
{
	_log_message( "NewCommCoreUID()" );

	CHAR	szComputerName[255];
	CHAR	szCCUID[255];
	DWORD	dwSize = 64;
	DWORD	dwTicks;
	int		iRand;

	GetComputerName( szComputerName, &dwSize );

	for (iRand = dwSize; iRand < 8; iRand++)
		szComputerName[iRand] = '0';

	szComputerName[iRand] = '\0';

	dwTicks = GetTickCount();

	srand( dwTicks );

	iRand = rand();

	sprintf( szCCUID, "%-8.8s-%8.8X-%4.4X", szComputerName, dwTicks, iRand );

	SetCommCoreUID( szCCUID );

	strcpy( lpszCCUID, szCCUID );
}

// ---------------------------------------------------------------------------------------------

VOID CCommCore::GetCommCoreUID( LPSTR lpszCCUID )
{
	_log_message( "GetCommCoreUID()" );

	HKEY	hKey;
	CHAR	szCCUID[64];
	DWORD	dwSize = 64;

	if (RegOpenKeyEx( HKEY_CURRENT_USER,
		"Software\\GSC Game World",
		0,
		KEY_ALL_ACCESS,
		&hKey ) != ERROR_SUCCESS)
	{
		NewCommCoreUID( lpszCCUID );
		return;
	};

	if (RegQueryValueEx( hKey,
		"CCUID",
		0,
		NULL,
		(unsigned char*) szCCUID,
		&dwSize ) != ERROR_SUCCESS)
	{
		RegCloseKey( hKey );
		NewCommCoreUID( lpszCCUID );
		return;
	};

	RegCloseKey( hKey );

	strcpy( lpszCCUID, szCCUID );
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendNewName( PEER_ID PeerId )			// Отсылает информацию о имени	(сервер)
{
	_log_message( "SendNewName()" );

	CC_PK_SEND_NEW_NAME			SendNewNamePacket;
	u_short						uPeerNum;

	uPeerNum = GetPeerById( PeerId );

	SendNewNamePacket.m_PeerId = PeerId;
	strcpy( SendNewNamePacket.m_szUserName, m_PeerList[uPeerNum].m_szUserName );

	for (int i = 1; i < m_uPeerCount; i++)
		if (m_PeerList[i].m_Id != PeerId)
			SendRawPacket( m_PeerList[i].m_ex_Addr,
				m_PeerList[i].m_ex_Port,
				CC_PT_SEND_NEW_NAME,
				(LPBYTE) &SendNewNamePacket,
				sizeof( CC_PK_SEND_NEW_NAME ),
				TRUE,
				FALSE );

	return TRUE;
}
// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendNewData( PEER_ID PeerId )			// Отсылает информацию о дате	(сервер)
{
	_log_message( "SendNewData()" );

	LPCC_PK_SEND_NEW_DATA		pSendNewDataPacket = NULL;
	u_short						uPacketSize = 0;
	u_short						uPeerNum;

	uPeerNum = GetPeerById( PeerId );

	if (uPeerNum == BAD_PEER_ID)
		return FALSE;

	uPacketSize = sizeof( CC_PK_SEND_NEW_DATA ) + m_PeerList[uPeerNum].m_uUserDataSize;

	pSendNewDataPacket = (LPCC_PK_SEND_NEW_DATA) malloc( uPacketSize );

	pSendNewDataPacket->m_PeerId = PeerId;
	pSendNewDataPacket->m_uUserDataSize = m_PeerList[uPeerNum].m_uUserDataSize;
	memcpy( pSendNewDataPacket->m_UserData,
		m_PeerList[uPeerNum].m_lpbUserData,
		m_PeerList[uPeerNum].m_uUserDataSize );

	for (int i = 1; i < m_uPeerCount; i++)
		if (m_PeerList[i].m_Id != PeerId)
		{
			SendRawPacket( m_PeerList[i].m_ex_Addr,
				m_PeerList[i].m_ex_Port,
				CC_PT_SEND_NEW_DATA,
				(LPBYTE) pSendNewDataPacket,
				uPacketSize,
				TRUE,
				FALSE );

		};

	free( pSendNewDataPacket );

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::IsOverNAT( PEER_ID PeerId )
{
	_log_message( "IsOverNAT()" );

	if (PeerId == 1)
		return FALSE;

	u_short uPeerNum;

	uPeerNum = GetPeerById( PeerId );

	if (uPeerNum == BAD_PEER_ID)
		return FALSE;

	return m_PeerList[uPeerNum].m_bOverNAT;
}

// ---------------------------------------------------------------------------------------------
VOID CCommCore::GetServerAddress( LPSTR lpszServerAddress )
{
	if (lpszServerAddress)
	{
		strcpy( lpszServerAddress, inet_ntoa( m_paServAddr ) );
	}
}
// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendUdpHolePunch( sockaddr *server, char *content, const int content_len )
{
	const int res = sendto( m_DataSocket, content, content_len, 0, server, sizeof( sockaddr_in ) );

	if (SOCKET_ERROR == res)
	{
		return FALSE;
	}

	return TRUE;
}
