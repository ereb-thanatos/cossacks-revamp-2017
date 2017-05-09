struct OneChatPlayer{
	char Nick[32];
	char Info[64];
	char mail[64];
	DWORD IP;
	bool InfoReqSent:1;
	bool ValidInfo:1;
	bool ValidMail:1;
	bool Muted:1;
};
class ChatMsg{
public:
	char** Nick;
	char** Message;
	int NMsg;
	int MaxMsg;
	ChatMsg();
	~ChatMsg();
	void Add(char* nick,char* msg);
	bool RemoveOne(char* nick,char* buf,int Len);
};
class OneChannel{
public:
	int NPlayers;
	int MaxPlayers;
	OneChatPlayer* Players;
};
#define NCHNL 2
class ChatSystem{
public:
	CHAT chat;
	chatGlobalCallbacks globalCallbacks;
	chatChannelCallbacks channelCallbacks;
	bool Connected;
	bool Error;
	CHATBool enterChannelSuccess[NCHNL];
	//CHATBool enterChannelSuccess;
	char serverAddress[128];
	int  port;
	char chatNick[128];
	char chatUser[128];
	char chatName[128];
	char chatChannel[NCHNL][128];
	//char chatChannel[128];
	CHATBool quit;
	int LastAccessTime;
	int CurChannel;
	OneChannel CCH[NCHNL];

	int NPlayers();
	OneChatPlayer* Players();
	
	int NAbsPlayers;
	int MaxAbsPlayers;
	OneChatPlayer* AbsPlayers;

	ChatMsg Private;
	ChatMsg Common[NCHNL];
	//------------------//
	ChatSystem();
	~ChatSystem();
	void AddPlayer(char* Nick,int c);
	void DelPlayer(char* Nick,int c);
	void AddAbsentPlayer(char* Nick);
	void DelAbsentPlayer(char* Nick);
	void CheckMessage(char* message);
	void Setup();
	bool ConnectToChat(char* Nick,char* Info,char* mail,char* Serv);
	void Disconnect();
	void Process();
	void SortPlayers(OneChatPlayer* PL,int N);
};
extern ChatSystem CSYS;