// HttpComm.h: interface for the CHttpComm class.
//
//////////////////////////////////////////////////////////////////////

enum ConnectionStage{csConnecting,csSending,csReceiving,csWaiting,csError};

#define RECV_BUFFER_SIZE	16*1024

typedef struct SHttpQuery
{
	DWORD				m_dwHandle;			// handle
	SOCKET				m_Socket;			// socket
	LPSTR				m_lpszQuery;		// query
	LPVOID				m_lpvBuffer;		// receive buffer
	DWORD				m_dwDataSize;		// received data size
	ConnectionStage		m_Stage;			// connection stage
}*PHttpQuery;

class CHttpComm  
{
public:
	VOID		ProcessRequests();
	DWORD		AddRequest(LPCSTR lpcszURL);
	int			GetData(DWORD dwHandle, LPVOID lpvBuffer, DWORD dwBufferSize);
	VOID		FreeData(DWORD dwHandle);
	CHttpComm();
	virtual ~CHttpComm();
protected:
	BOOL		m_bInitialized;
	CHAR		m_szProxyAddr[255];
	DWORD		m_bUseProxy;
	DWORD		m_dwProxyPort;
	DWORD		m_dwRequestCount;
	DWORD		m_dwHandleAuto;
	PHttpQuery	m_pRequestList;
};


