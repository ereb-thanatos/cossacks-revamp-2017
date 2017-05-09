//
//	UDP based Communication Core
//	GSC Game World
//	© 2001 Serguei I. Ivantsov aka ManOwaR Linux Lover
//
//	Jun 22, 2001		V0.1
//	Jul 09, 2001		V0.2
//	Aug	06, 2001		V0.4
// 
//
// Для корректной работы за NAT-ом, последний должен удовлетворять рекомендациям
// фирмы Microsoft на трансляцию UDP-пакетов через шлюз

#define _COOL_
//#define CC_DEBUG

#ifdef CC_DEBUG
#define _log_message(message) DebugMessage(message)
#else
#define _log_message(message) 
#endif //CC_DEBUG

#ifndef _COMM_CORE_H_INCLUDED_
#define _COMM_CORE_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/*
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <assert.h>
*/
#ifdef CC_DEBUG
#include <stdio.h>
#endif //CC_DEBUG

#pragma warning (disable : 4200)

#pragma pack(1)

// ---------------------------------------------------------------------------------------------

typedef in_addr PEER_ADDR;
typedef u_short	PEER_PORT;
typedef u_short	PEER_ID, *LPPEER_ID;

// ---------------------------------------------------------------------------------------------
// Константы
#define PROTO_ID				0x47534370	// Идентификатор протокола ('GSCp')
#define DATA_PORT				34000		// Порт данных (34000)
#define RETRY_COUNT				8			// Сколько раз нужно пытаться отослать пакет
											// в случае неподтверждения
											// после чего пакет удаляется из очереди

//in main exe's commcore.h default RETRY_TIME was 700
//in commlib's commcore.h default RETRY_TIME was 2800
#define RETRY_TIME				700			// Интервал повтора отправки пакет в миллисекундах
#define MAX_QUEUE_LEN			4096		// Максимальная длина очереди пакетов
											// ожидающих подтверждения
#define MAX_PEERS				7			// Максимальное число хостов 
#define CC_PROTO_VERSION		0x03		// Версия коммуникационного протокола
#define MAX_PACKET_STAMP		0xEE6B2800	// Максимальный номер пакета (используется для идентификации)
#define RECV_BUFFER_LENGTH		8192		// Размер приемного буфера (8 килобайт)
#define MAX_HOST_NAME			32			// Максимальная длина имени хоста

// ---------------------------------------------------------------------------------------------
// Описание хоста
typedef struct PEER_ENTRY
{						// Хост
	PEER_ADDR	m_ex_Addr;						// Внешний адрес хоста (или NAT-а)
	PEER_PORT	m_ex_Port;						// Внешний порт хоста (или NAT-а)
	PEER_ID		m_Id;							// Идентификатор хоста в списке сервера
	BOOL		m_bAlive;						// Доступен ли хост на данный момент
	BOOL		m_bOverNAT;						// Находится ли хост за NAT-ом по отношению к серверу
	u_short		m_uLatency;						// Время ответа хоста
	CHAR		m_szUserName[MAX_HOST_NAME];	// Название хоста / пользователя
	u_short		m_uUserDataSize;				// Размер пользовательских данных
	LPBYTE		m_lpbUserData;					// Указатель на пользовательские данные
	CHAR		m_szCCUID[23];					// Глобальный идентификатор хоста
} *LPPEER_ENTRY;

// ---------------------------------------------------------------------------------------------
// Типы пакетов (указываются в заголовке)
#define CC_PT_FRAME_CONFIRM		0x01	// Хост подтверждает прием пакета данных
#define CC_PT_HOST_ALIVE		0x02	// Хост подтверждает, что он живой; измерение Latency
#define CC_PT_SEND_DATA			0x03	// Пакет пользовательских данными без подтверждения
#define CC_PT_SEND_DATA_SECURE	0x04	// Пакет пользовательских данными с подтверждением
#define CC_PT_CHCK_HOST			0x05	// Пакет запроса доступности хоста; измерение Latency
#define CC_PT_TRY_CONNECT		0x06	// Попытка подключения к серверу
#define CC_PT_CONNECT_OK		0x07	// Разрешение на подключение к серверу
#define CC_PT_CONNECT_REJECT	0x08	// Отказ в подключении к серверу
#define CC_PT_SERVER_LIST		0x09	// Рассылка списка хостов (от сервера)
#define CC_PT_HOST_EXIT			0x0A	// Хост выходит из игры
#define CC_PT_HOST_DROP			0x0B	// Хост выброшен из игры (от сервера)
#define CC_PT_CONNECT_DIFF		0x0C	// Пакет приходит к хосту от другого хоста, если он знает кто сервер
#define CC_PT_HOST_DROP_OK		0x0D	// Сервер подтверждает отключение хоста

#define CC_PT_SEND_USER_NAME	0x0E	// Пользователь посылает свое имя
#define CC_PT_SEND_USER_DATA	0x0F	// Пользователь посылает свои данные

#define CC_PT_SEND_NEW_NAME		0x10	// Сервер отсылает данные об изменившимся имени пользователя
#define CC_PT_SEND_NEW_DATA		0x11	// Сервер отсылает данные об изменившихся данных пользователя

//
// ---------------------------------------------------------------------------------------------
enum ConnectState
{
	csNone, csWait, csRejected, csConnected, csTimedOut, csBadProto, csSynch
};

enum SessionState
{
	ssNone, ssOpen, ssClosed
};

// ---------------------------------------------------------------------------------------------
// Ошибки
#define	BAD_PEER_ID				255		// Неверный идентификатор хоста
#define CE_NOT_SERVER			0x01	// Попытка подключения не к серверу
#define CE_NO_ROOM				0x02	// Нет свободых слотов подключения
#define CE_NO_ERROR				0x03	// Нет ошибок
#define CE_BAD_VERSION			0x04	// Неверная версия протокола
#define CE_BAD_MSG_SIZE			0x05	// Недопустимая длина сообщения
#define CE_SESSION_CLOSED		0x06	// Невозможно подключить хост, т.к. сессия уже закрыта

// ---------------------------------------------------------------------------------------------
// Заголовки пакетов

// Общий вид кадра для протокола
typedef struct CC_PK_RAW_FRAME
{
	u_long	m_lProto;				// Тип протокола
	u_short	m_uType;				// Тип пакета
	u_long	m_lStamp;				// Уникальный номер пакета; 0, если не требуется подтверждение
	PEER_ID	m_PeerId;				// Идентификатор отправителя
// ----------------------------------
	BYTE	m_bData[];				// Специфические для пакета данные переменной длины
} *LPCC_PK_RAW_FRAME;

// CC_PT_FRAME_CONFIRM
typedef struct CC_PK_FRAME_CONFIRM
{
	u_long	m_lConfirmStamp;		// Уникальный номер пакета, подтверждающий прием
} *LPCC_PK_FRAME_CONFIRM;

// CC_PT_SEND_DATA
typedef struct CC_PK_SEND_DATA
{
	u_short	m_uSize;				// Размер поля данных
	BYTE	m_bData[];				// Поле данных переменной длины
} *LPCC_PK_SEND_DATA;

// CC_PT_TRY_CONNECT
typedef struct CC_PK_TRY_CONNECT
{
	CHAR		m_cProtoVersion;	// Номер версии протокола / приложения
	CHAR		m_szUserName[MAX_HOST_NAME];
	CHAR		m_szCCUID[23];
	u_short		m_uAddrCount;
	DWORD		m_dwAddrList[];
} *LPCC_PK_TRY_CONNECT;

// CC_PT_CONNECT_OK
typedef struct CC_PK_CONNECT_OK
{
	PEER_ID		m_Id;							// Номер хоста в списке сервера
	CHAR		m_szSessionName[MAX_HOST_NAME];	// Название сессии
	DWORD		m_dwOptions;
} *LPCC_PK_CONNECT_OK;

// CC_PT_CONNECT_REJECT
typedef struct CC_PK_CONNECT_REJECT
{
	u_short		m_uReason;			// Причина отказа
} *LPCC_PK_CONNECT_REJECT;

// CC_PT_SERVER_LIST
typedef struct CC_PK_SERVER_LIST
{
	CHAR		m_szSessionName[MAX_HOST_NAME];	// Название сессии
	u_short		m_uCount;						// Количество хостов
	BYTE		m_PeerList[];					// Список хостов переменной + динамические данные :)
} *LPCC_PK_SERVER_LIST;

// CC_PT_CHCK_HOST
typedef struct CC_PK_CHCK_HOST
{
	DWORD		m_dwTickCount;		// Текущее число тиков
} *LPCC_PK_CHCK_HOST;

// CC_PT_HOST_ALIVE
typedef struct CC_PK_HOST_ALIVE
{
	DWORD		m_dwTickCount;		// Число тиков в момент отправки пакета (оригинального)
} *LPCC_PK_HOST_ALIVE;

// CC_PT_HOST_EXIT
typedef struct CC_PK_HOST_EXIT
{
	DWORD		m_dwReserved;		//
} *LPCC_PK_HOST_EXIT;

// CC_PT_HOST_DROP
typedef struct CC_PK_HOST_DROP
{
	DWORD		m_dwReserved;		//
} *LPCC_PK_HOST_DROP;

// CC_PT_HOST_DROP_OK
typedef struct CC_PK_HOST_DROP_OK
{
	DWORD		m_dwReserved;		//
} *LPCC_PK_HOST_DROP_OK;

// CC_PT_SEND_USER_NAME
typedef struct CC_PK_SEND_USER_NAME
{
	CHAR		m_szUserName[MAX_HOST_NAME];
} *LPCC_PK_SEND_USER_NAME;

// CC_PT_SEND_USER_DATA
typedef struct CC_PK_SEND_USER_DATA
{
	u_short		m_uUserDataSize;
	BYTE		m_UserData[];
} *LPCC_PK_SEND_USER_DATA;

// CC_PT_SEND_NEW_NAME
typedef struct CC_PK_SEND_NEW_NAME
{
	PEER_ID		m_PeerId;
	CHAR		m_szUserName[MAX_HOST_NAME];
} *LPCC_PK_SEND_NEW_NAME;

// CC_PT_SEND_NEW_DATA
typedef struct CC_PK_SEND_NEW_DATA
{
	PEER_ID		m_PeerId;
	u_short		m_uUserDataSize;
	BYTE		m_UserData[];
} *LPCC_PK_SEND_NEW_DATA;

// ---------------------------------------------------------------------------------------------
// Описание пакета в очереди сообщений, ожидающих подтверждение
typedef struct FRAME_ENTRY
{						// Пакет
	LPCC_PK_RAW_FRAME			m_lpFrame;		// Указатель на пакет
	u_short						m_uSize;		// Размер пакета
	PEER_ADDR					m_PeerAddr;		// Адрес хоста
	PEER_PORT					m_PeerPort;		// Порт хоста
	DWORD						m_dwSendTime;	// Время последней отправки пакета
												// в миллисекундах
	u_short						m_uRetrCount;	// Количество попыток отправки пакета
} *LPFRAME_ENTRY;

// Описание функций обратного вызова
typedef BOOL( CALLBACK* LP_CC_IDLE_PROC )( );
typedef BOOL( CALLBACK* LP_CC_ENUM_PROC )( const PEER_ID PeerID, LPCSTR lpcszPeerName );

// ---------------------------------------------------------------------------------------------
// Класс ядра связи
class CCommCore
{
	// ---------------------------------------------------------------------------------------------
public:
	// ---------------------------------------------------------------------------------------------
	LP_CC_IDLE_PROC	lpIdleProc;				// Функция обратного вызова, вызывается ядром во время
											// работы блокирующих функций
	LP_CC_ENUM_PROC	lpEnumProc;				// Функция обратного вызова для перечисления хостов
// ---------------------------------------------------------------------------------------------
											// Послать сообщение выбранному хосту
	BOOL SendToPeer( PEER_ID piNumber, LPBYTE lpbBuffer, u_short u_Size, BOOL bSecure = FALSE );

	// Послать сообщение всем хостам
	BOOL SendToAll( LPBYTE lpbBuffer, u_short u_Size, BOOL bSecure = FALSE );

	u_short ReceiveData( LPBYTE lpbBuffer, LPPEER_ID lpPeerId = NULL );	// Извлекает один пакет из очереди
											// должна часто вызываться, во избежание переполнения буфера
	BOOL SendDropClient( PEER_ID PeerID );	// Сервер просит клиента выйти

	// IP адрес в виде строки с точками aaa.bbb.ccc.ddd
	BOOL InitClient( LPCSTR lpcszServerIP, LPCSTR lpcszUserName, unsigned short port );

	BOOL DoneClient();						// Клиент отсылает запрос на удаление серверу 
	BOOL DoneServer();						// Завершает работу сервера удаляя все хосты
	BOOL DeletePeer( PEER_ID piNumber );		// Удаляет хост из списка хостов
	BOOL InitServer( LPCSTR lpcszSessionName, LPCSTR lpcszUserName );	// Инициализирует сервер
	BOOL QueueProcess();					// Обрабатывает очередь отправки пакетов
											// Должна часто вызываться

	BOOL SendServerList();					// Рассылает список хостов


	BOOL	IsOverNAT( PEER_ID PeerId );


	LPCSTR	GetUserName( PEER_ID PeerId );
	BOOL	SetUserName( LPCSTR lpcszUserName );
	BOOL	SendUserName();

	BOOL	SetSessionName( LPCSTR lpcszSessionName );

	BOOL	GetUserData( PEER_ID PeerId, LPBYTE lpbUserData, u_short * puUserDataSize );
	BOOL	SetUserData( const LPBYTE lpcbUserData, u_short uUserDataSize );
	BOOL	SendUserData();

	BOOL	SendUdpHolePunch( sockaddr *server, char *content, const int content_len );

	VOID	GetServerAddress( LPSTR lpszServerAddress );

	BOOL	EnumPeers();

	BOOL	InitNetwork();					// Инициализация сетевого интерфейса
	BOOL	CloseNetwork();					// Закрытие сетевого интерфейса

	CCommCore();
	virtual ~CCommCore();

	PEER_ID	GetPeerID()
	{
		return m_piNumber;
	}

	u_short GetPeersCount()
	{
		return m_uPeerCount;
	}
	LPCSTR	GetSessionName()
	{
		return m_szSessionName;
	}

	u_short	GetMaxPeers()
	{
		return m_uMaxPeers;
	}
	VOID	SetMaxPeers( u_short uMaxPeers )
	{
		m_uMaxPeers = uMaxPeers;
	}

	VOID	CloseSession()
	{
		m_ssState = ssClosed;
	}

	BOOL	IsClient()
	{
		return !m_bServer;
	}
	BOOL	IsServer()
	{
		return m_bServer;
	}

	VOID	SetOptions( DWORD dwOptions )
	{
		m_dwOptions = dwOptions;
	}
	DWORD	GetOptions()
	{
		return m_dwOptions;
	}

	// Сколько получено байт
	DWORD	GetRxBytes()
	{
		return m_dwRxBytes;
	}
	// Сколько послано байт
	DWORD	GetTxBytes()
	{
		return m_dwTxBytes;
	}
	// Сколько перепослано байт
	DWORD	GetNxBytes()
	{
		return m_dwNxBytes;
	}
	// Какое число миллисекунд назад был принят последний пакет
	DWORD	GetRecvTimeOut()
	{
		return ( GetTickCount() - m_dwLastPacketTime );
	}

	// ---------------------------------------------------------------------------------------------
protected:
	// ---------------------------------------------------------------------------------------------
#ifdef CC_DEBUG
	FILE	*	m_DebugStream;
	VOID		DebugMessage( LPCSTR lpcszMessage );
#endif //CC_DEBUG

	CHAR		m_szUserName[MAX_HOST_NAME];		//
//	PEER_ADDR	m_paHostAddr;						//
//	CHAR		m_szDotAddr[18];					//
	PEER_ADDR	m_paServAddr;						//
	PEER_PORT	m_paServPort;						// True port of game host (recieved from server)

	DWORD		m_dwAddrList[8];					// Список адресов локальной машины в network order формате
	u_short		m_uAddrCount;						// Количество адресов локальной машины

	DWORD		m_dwLastPacketTime;

	u_short		m_uMaxPeers;
	BOOL		m_bOverNAT;

	DWORD		m_dwRxBytes;
	DWORD		m_dwTxBytes;
	DWORD		m_dwNxBytes;

	DWORD		m_dwOptions;

	CHAR		m_szSessionName[MAX_HOST_NAME];		//

	CHAR		m_szCCUID[23];

	LPBYTE		m_lpbRecvBuffer;					//

	SessionState	m_ssState;						//
	ConnectState	m_csState;						//
	BOOL			m_bBlockingCall;				// В режиме блокировки обрабатываются только
													// пакеты-подтверждения

	u_short		m_uRejectReason;					//

	u_short		m_uMaxMsgSize;						//

	SOCKET		m_DataSocket;						// Сокет, обслуживающий пользовательские сообщения

	u_long		m_lStamp;							// Автоинкрементный счетчик пакетов

	BOOL		m_bServer;							// Является ли хост сервером
	PEER_ID		m_piNumber;							// Идентификатор хоста в списке сервера
													// для сервера всегда равен 0x01
	PEER_ID		m_piAutoInc;						// Автоинкрементный счетчик хостов

	LPBYTE		m_lpbUserData;						// Динамические данные о пользователе
	u_short		m_uUserDataSize;					// Размер динамических данных о пользователе

	PEER_ENTRY	m_PeerList[MAX_PEERS];				// Список доступных хостов
	u_short		m_uPeerCount;						// Количество доступных хостов

	FRAME_ENTRY m_FrameList[MAX_QUEUE_LEN];			// Очередь сообщений, ожидающих подтверждения
	u_short		m_uFrameCount;						// Количество сообщений в очереди

// ---------------------------------------------------------------------------------------------

//	PEER_ID		GetIdBySender();					// Возвращает логический номер хоста
													// По его адресу/порту
	u_short GetPeerById( PEER_ID PeerId );			//
	u_short GetPeerByCCUID( LPCSTR lpcszCCUID );
	BOOL	DropPeer( u_short uPeer );					//

	BOOL InitSocket();
	BOOL CloseSocket();
	BOOL InitHost();

	BOOL QueueAddPacket( PEER_ADDR			PeerAddr,
		PEER_PORT			PeerPort,
		LPCC_PK_RAW_FRAME	lpRawFrame,
		u_short				uSize );

	BOOL SendRawPacket( PEER_ADDR			PeerAddr,				// Адрес хоста
		PEER_PORT			PeerPort,				// Порт хоста
		u_short				uType,					// Тип пакета
		LPBYTE				lpbBuffer,				// Буфер с данными
		u_short				uSize,					// Размер данных в байтах
		BOOL				bSecureMessage,			// TRUE, если нужна псевдо-гарантированная доставка данных
		BOOL				bWaitForCompletion );	// TRUE, если блокирующий вызов,
													// т.е. ждать подтверждения или таймаута


	BOOL ProcessServerList( LPCC_PK_SERVER_LIST lpServerList );

	BOOL ReSendFrame( u_short uFrameNum );
	VOID Cleanup();

	BOOL QueueDropPacket( int iFrameNum );
	BOOL QueueDropConfirmedPacket( u_long lStamp );
	BOOL QueueClearAll();
	BOOL QueuePacketExists( u_long lStamp );

	BOOL SendConfirmDataPacket( sockaddr_in * lpSender, u_long lStamp );
	BOOL SendConnectReject( sockaddr_in *lpSender, u_short uReason );
	BOOL SendConnectOk( sockaddr_in *lpSender, PEER_ID PeerId );
	BOOL SendDropOk( sockaddr_in *lpSender );

	BOOL SendNewName( PEER_ID PeerId );		// Отсылает информацию о имени	(сервер)
	BOOL SendNewData( PEER_ID PeerId );		// Отсылает информацию о дате	(сервер)

	VOID SetCommCoreUID( LPCSTR lpcszCCUID );
	VOID NewCommCoreUID( LPSTR lpszCCUID );
	VOID GetCommCoreUID( LPSTR lpszCCUID );

	// ---------------------------------------------------------------------------------------------
};

#pragma warning (default : 4200)

#endif // _COMM_CORE_H_INCLUDED_
