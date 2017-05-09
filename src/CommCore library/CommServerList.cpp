// Server list processing, sending and receiving
//
#include "CommCore.h"

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::SendServerList()
{
	_log_message("SendServerList()");

	LPCC_PK_SERVER_LIST		lpServerList;
	LPBYTE					lpbOffset;

	int						iUserDataSize = 0;
	int						iServerListSize = 0;
	int						i;


	for (i = 0; i < m_uPeerCount; i++)
		iUserDataSize += m_PeerList[i].m_uUserDataSize;

	iServerListSize = sizeof(CC_PK_SERVER_LIST) + m_uPeerCount * sizeof(PEER_ENTRY) + iUserDataSize;

	lpServerList = (LPCC_PK_SERVER_LIST)malloc(iServerListSize);
	assert(lpServerList);

	lpServerList->m_uCount = m_uPeerCount;
	strcpy(lpServerList->m_szSessionName, m_szSessionName);

	lpbOffset = lpServerList->m_PeerList;

	for (i = 0; i < m_uPeerCount; i++) {
		memcpy(lpbOffset, &m_PeerList[i], sizeof(PEER_ENTRY));
		if (m_PeerList[i].m_lpbUserData)
			memcpy(lpbOffset + sizeof(PEER_ENTRY), m_PeerList[i].m_lpbUserData, m_PeerList[i].m_uUserDataSize);
		lpbOffset += sizeof(PEER_ENTRY) + m_PeerList[i].m_uUserDataSize;
	};

	for (i = 1; i < m_uPeerCount; i++)
		if (!SendRawPacket(m_PeerList[i].m_ex_Addr,
			m_PeerList[i].m_ex_Port,
			CC_PT_SERVER_LIST,
			(LPBYTE)lpServerList,
			(u_short)iServerListSize,
			TRUE,
			FALSE))
		{
			free(lpServerList);
			return FALSE;
		};

	free(lpServerList);

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::ProcessServerList(LPCC_PK_SERVER_LIST lpServerList)
{
	_log_message("ProcessServerList()");

	int			i;
	LPBYTE		lpbOffset;

	PEER_PORT	ppaExPort[MAX_PEERS];
	PEER_ADDR	paaExAddr[MAX_PEERS];
	PEER_ID		piaId[MAX_PEERS];
	u_short		OldPeerCount = m_uPeerCount;
	// ----------------------------------------------------------------------
	for (i = 0; i < OldPeerCount; i++) {
		ppaExPort[i] = m_PeerList[i].m_ex_Port;
		paaExAddr[i].s_addr = m_PeerList[i].m_ex_Addr.s_addr;
		piaId[i] = m_PeerList[i].m_Id;
	};
	// ----------------------------------------------------------------------
	for (i = 0; i < m_uPeerCount; i++)
		if (m_PeerList[i].m_lpbUserData)
			free(m_PeerList[i].m_lpbUserData);

	m_uPeerCount = lpServerList->m_uCount;
	strcpy(m_szSessionName, lpServerList->m_szSessionName);

	lpbOffset = lpServerList->m_PeerList;

	for (i = 0; i < m_uPeerCount; i++) {
		memcpy(&m_PeerList[i], lpbOffset, sizeof(PEER_ENTRY));
		if (m_PeerList[i].m_lpbUserData) {
			m_PeerList[i].m_lpbUserData = (LPBYTE)malloc(m_PeerList[i].m_uUserDataSize);
			assert(m_PeerList[i].m_lpbUserData);
			memcpy(m_PeerList[i].m_lpbUserData, lpbOffset + sizeof(PEER_ENTRY), m_PeerList[i].m_uUserDataSize);
		};

		lpbOffset += sizeof(PEER_ENTRY) + m_PeerList[i].m_uUserDataSize;
	};

	// ----------------------------------------------------------------------
	for (i = 0; i < m_uPeerCount; i++) {
		for (int j = 0; j < OldPeerCount; j++) {
			if (m_PeerList[i].m_Id == piaId[j]) {
				if ((m_PeerList[i].m_ex_Port != ppaExPort[j]) ||
					(m_PeerList[i].m_ex_Addr.s_addr != paaExAddr[j].s_addr)) {

					m_PeerList[i].m_ex_Port = ppaExPort[j];
					m_PeerList[i].m_ex_Addr.s_addr = paaExAddr[j].s_addr;
				};
				break;
			};
		};
	};
	// ----------------------------------------------------------------------

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

