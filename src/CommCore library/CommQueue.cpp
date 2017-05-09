// Packet queueing workshop
//
#include "CommCore.h"

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::QueuePacketExists(u_long lStamp)
{
	//	_log_message("QueuePacketExists()");

	for (int i = 0; i < m_uFrameCount; i++)
		if (m_FrameList[i].m_lpFrame->m_lStamp == lStamp)
			return TRUE;
	return FALSE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::QueueAddPacket(PEER_ADDR			PeerAddr,
	PEER_PORT			PeerPort,
	LPCC_PK_RAW_FRAME	lpRawFrame,
	u_short				uSize)
{
	//	_log_message("QueueAddPacket()");

	if (m_uFrameCount >= MAX_QUEUE_LEN)
		return FALSE;

	m_FrameList[m_uFrameCount].m_lpFrame = lpRawFrame;
	m_FrameList[m_uFrameCount].m_uSize = uSize;
	m_FrameList[m_uFrameCount].m_PeerAddr.s_addr = PeerAddr.s_addr;
	m_FrameList[m_uFrameCount].m_PeerPort = PeerPort;
	m_FrameList[m_uFrameCount].m_dwSendTime = GetTickCount();
	m_FrameList[m_uFrameCount].m_uRetrCount = 0;

	m_uFrameCount++;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------
BOOL CCommCore::QueueDropPacket(int iFrameNum)
{
	//	_log_message("QueueDropPacket()");

	free(m_FrameList[iFrameNum].m_lpFrame);

	memmove(&m_FrameList[iFrameNum],
		&m_FrameList[iFrameNum + 1],
		(m_uFrameCount - iFrameNum - 1) * sizeof(FRAME_ENTRY));

	m_uFrameCount--;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::QueueDropConfirmedPacket(u_long lStamp)
{
	//	_log_message("QueueDropConfirmedPacket()");

	for (int i = 0; i < m_uFrameCount; i++)
		if (m_FrameList[i].m_lpFrame->m_lStamp == lStamp) {
			return QueueDropPacket(i);
			break;
		};
	return FALSE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::QueueProcess()
{
	DWORD dwTime;
	BOOL bDrops;

	int j = 0;
	do 
	{
		bDrops = FALSE;
		for (int i = j; i < m_uFrameCount; i++) 
		{
			if (m_FrameList[i].m_uRetrCount > RETRY_COUNT) 
			{
				QueueDropPacket(i);
				bDrops = TRUE;
				j = i;
				break;
			}
		}
	} while (bDrops);

	dwTime = GetTickCount();

	for (u_short i = 0; i < m_uFrameCount; i++)
	{
		if ((dwTime - m_FrameList[i].m_dwSendTime) > RETRY_TIME)
		{
			m_FrameList[i].m_dwSendTime = dwTime;
			m_FrameList[i].m_uRetrCount++;
			ReSendFrame(i);
		}
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------

BOOL CCommCore::QueueClearAll()
{
	//	_log_message("QueueClearAll()");

	for (int i = 0; i < m_uFrameCount; i++)
		free(m_FrameList[i].m_lpFrame);

	m_uFrameCount = 0;

	return TRUE;
}

// ---------------------------------------------------------------------------------------------
