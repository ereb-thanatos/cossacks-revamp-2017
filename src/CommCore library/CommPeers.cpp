// Peers manipulation routines
//
#include "CommCore.h"

// ---------------------------------------------------------------------------------------------
u_short CCommCore::GetPeerByCCUID(LPCSTR lpcszCCUID)
{
	//	_log_message("GetPeerByCCUID()");

	for (u_short i = 0; i < m_uPeerCount; i++)
	{
		if (memcmp(m_PeerList[i].m_szCCUID, lpcszCCUID, 22) == 0)
		{
			return i;
		}
	}

	return BAD_PEER_ID;
}

// ---------------------------------------------------------------------------------------------

u_short CCommCore::GetPeerById(PEER_ID PeerId)
{
	//	_log_message("GetPeerById()");

	for (u_short i = 0; i < m_uPeerCount; i++)
	{
		if (m_PeerList[i].m_Id == PeerId)
		{
			return i;
		}
	}

	return BAD_PEER_ID;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::DropPeer(u_short uPeer)
{
	_log_message("DropPeer()");

	memmove(&m_PeerList[uPeer], &m_PeerList[uPeer + 1], (m_uPeerCount - uPeer - 1) * sizeof(PEER_ENTRY));

	m_uPeerCount--;

	if (m_bServer)
		SendServerList();

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::DeletePeer(PEER_ID piNumber)
{
	_log_message("DeletePeer()");

	u_short
		uPeer = GetPeerById(piNumber);

	if (uPeer == BAD_PEER_ID)
		return FALSE;

	return DropPeer(uPeer);
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::EnumPeers()
{
	//	_log_message("EnumPeers()");

	if (lpEnumProc)
	{
		for (int i = 0; i < m_uPeerCount; i++)
		{
			if (m_PeerList[i].m_Id == m_piNumber)
			{
				lpEnumProc( m_piNumber, m_szUserName );
			}
			else
			{
				lpEnumProc( m_PeerList[i].m_Id, m_PeerList[i].m_szUserName );
			}
		}
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------
