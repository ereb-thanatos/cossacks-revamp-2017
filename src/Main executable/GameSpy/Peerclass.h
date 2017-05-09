#include "Peer.h";
struct TPE_Player{
	char Name[64];
	int Ping;
	int Status;
	bool ready;
};
class TPE_Room{
public:
	char Name[64];
	char version[12];
	char HostAddress[32];
	int Ping;
	int Type;
	int Nplayers;
	int MaxPlayer;
	GServer G;
};
class TotalPeerEngine;
class TPE_MyRoom:public TPE_Room{
public:
	PEER Peer;
	TotalPeerEngine* TPE;
	TPE_Player* Players;
	bool Host;
	int NPlayers;
	int MaxPlayers;
	bool Change;
	bool ChangeChat;
	bool RoomConnected;
	void AddPlayer(char* nick);
	void DelPlayer(char* nick);
	void SetPing(char* nick,int Ping);
	void SetReady(char* nick,bool State);
	void ChangeMyReady();
	void ResetMyReady();
	void SetMyReady(bool);
	void Refresh();
	void RunGame();
};
struct ChatString{
	char* PlName;
	char* Message;
};
class ChatQueue{
public:
	int NCStr;
	int MaxCStr;
	ChatString* CStr;
	void Clear();
	void Add(char* nick,char* Message);
};
class TotalPeerEngine{
public:
	int PTIME;
	int ConnStartTime;
	bool Change;
	bool ChangeChat;
	bool ChangeLocalChat;
	bool ChangeServer;
	//-----------------//
	bool Runned;
	DWORD HOSTIP;
	char HostMessage[128];
	//-----------------//
	TotalPeerEngine();
	~TotalPeerEngine();
	char MyNick[64];
	int State;
	int ErrorType;
	PEER Peer;
	int NPlayers;
	int MaxPlayers;
	TPE_Player* Players;
	int NRooms;
	int MaxRooms;
	TPE_Room* Room;
	TPE_MyRoom* MyRoom;
	bool Connected;
	ChatQueue GlobalChat;
	ChatQueue LocalChat;
	void AddPlayer(char* nick);
	void DelPlayer(char* nick);
	void SetPing(char* nick,int Ping);
	void Connect(char* Nick);
	bool Disconnect();
	void Process();
	void SendGlobalChat(char* Message);
	void SendLocalChat(char* Message);
	void RefreshPlayers();
	//--------------//
	void CreateStagingRoom(char* Name,char* Password,int MaxPlayers);
	void AddServer(char* Name,GServer server);
	void DelServer(char* Name,GServer server);
	void UpdateServer(char* Name,GServer server);
	void ClearServers();
	void RefreshSessions();
	void JoinRoom(TPE_Room* Room);
	void LeaveMyRoom();
	//--------VISUAL FUNCTIONALITY-------//
	bool NeedReconnect();
	bool CanJoin();
	bool CanCreate();
	bool IsInStagingRoom();
	bool StagingStateChanged();

};
extern TotalPeerEngine TPEN;
