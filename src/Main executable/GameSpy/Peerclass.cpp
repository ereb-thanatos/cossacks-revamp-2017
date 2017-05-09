#include "stdafx.h";
#include "PeerClass.h";
TotalPeerEngine TPEN;
void TotalPeerEngine::Process(){
	if(GetTickCount()-PTIME>300&&Connected){
		RefreshPlayers();
		if(MyRoom&&MyRoom->RoomConnected){
			MyRoom->Refresh();
		};
		PTIME=GetTickCount();
	};
	peerThink(Peer);
};
void TotalPeerEngine::SendGlobalChat(char* Message){
	if(!Connected)return;
	if(!strcmp(Message,"@@@GETID")){
		char ccc[128];
		sprintf(ccc,"PROFILE# %d",LOGIN.ProfileID);
		GlobalChat.Add(MyNick,ccc);
	}else peerMessageRoom(Peer,TitleRoom,Message);
};
void TotalPeerEngine::AddPlayer(char* nick){
	if(!Connected)return;
	for(int i=0;i<NPlayers;i++)if(!strcmp(nick,Players[i].Name)){
		Players[i].Status&=1023;
		return;
	};
	if(NPlayers>=MaxPlayers){
		MaxPlayers+=32;
		Players=(TPE_Player*)realloc(Players,MaxPlayers*sizeof TPE_Player);
	};
	strcpy(Players[NPlayers].Name,nick);
	Players[NPlayers].Ping=-1;
	Players[NPlayers].Status=0;
	NPlayers++;
	Change=1;
};
void TotalPeerEngine::DelPlayer(char* nick){
	if(!Connected)return;
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,nick)){
		if(i<NPlayers-1)
			memcpy(Players+i,Players+i+1,(NPlayers-i-i)*sizeof TPE_Player);
		NPlayers--;
		i--;
	};
	Change=1;
};
void TotalPeerEngine::SetPing(char* nick,int Ping){
	if(!Connected)return;
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,nick)){
		Players[i].Ping=Ping;
	};
	Change=1;
};
TotalPeerEngine::TotalPeerEngine(){
	memset(this,0,sizeof TotalPeerEngine);
	PTIME=GetTickCount();
};
TotalPeerEngine::~TotalPeerEngine(){
	if(Connected){
		peerClearTitle(Peer);
		peerDisconnect(Peer);
		peerShutdown(Peer);
		peerLeaveRoom(Peer,TitleRoom);
		Peer=NULL;
	};
	if(MaxPlayers)free(Players);
	memset(this,0,sizeof TotalPeerEngine);
};
// Called when the connection to the server gets disconnected.
//////////////////////////////////////////////////////////////
void PEER_DisconnectCallback(
		PEER peer,  // The peer object.
		const char * reason,  // The reason for the disconnection.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	TPE->Connected=0;
	TPE->State=2;
	if(TPE->MyRoom){
		if(TPE->MyRoom->RoomConnected){
			if(TPE->Players)free(TPE->MyRoom->Players);
		};
		free(TPE->MyRoom);
		TPE->MyRoom=NULL;

	};
	//TPE->Connect(TPE->MyNick);
};

// Called when a message is sent to a room the local player is in.
//////////////////////////////////////////////////////////////////
void PEER_RoomMessageCallback(
		PEER peer,  // The peer object.
		RoomType roomType,  // The type of room that the message was in.
		const char * nick,  // The nick of the player who sent the message.
		const char * message,  // The text of the message.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(roomType==TitleRoom){
		TPE->GlobalChat.Add((char*)nick,(char*)message);
		TPE->ChangeChat=1;
	}else{
		if(TPE->MyRoom&&TPE->MyRoom->RoomConnected)
			TPE->LocalChat.Add((char*)nick,(char*)message);
		TPE->ChangeLocalChat=1;
	};
};

// Called when the name of a room the player is in changes.
// The new name can be checked with peerGetRoomName.
// PANTS|09.11.00
///////////////////////////////////////////////////////////
void PEER_RoomNameChangedCallback(
		PEER peer,  // The peer object.
		RoomType roomType,  // The type of room that the name changed in.
		void * param  // User-data
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;

};

// Called when a private message is received from another player.
/////////////////////////////////////////////////////////////////
void PEER_PlayerMessageCallback(
		PEER peer,  // The peer object.
		const char * nick,  // The nick of the player who sent the message.
		const char * message,  // The text of the message.
		void * param  // User-data
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
};

// Called when a player's ready state changes,
// from a call to peerSetReady().
//////////////////////////////////////////////
void PEER_ReadyChangedCallback(
		PEER peer,  // The peer object.
		const char * nick,  // The nick of the player who's ready state changed.
		PEERBool ready,  // The player's new ready state.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(TPE->MyRoom&&TPE->MyRoom->RoomConnected){
		TPE->MyRoom->SetReady((char*)nick,ready);
	};
};

// Called when the host of a staging room launches the game,
// with a call to peerGameStart().
////////////////////////////////////////////////////////////
void PEER_GameStartedCallback(
		PEER peer,  // The peer object.
		unsigned int IP,  // The IP of the host, in network-byte order. PANTS|09.11.00 - was unsigned long
		const char * message,  // A message that was passed into peerGameStart().
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	TPE->Runned=1;
	TPE->HOSTIP=IP;
	//strcpy(TPE->HostMessage,message);
};

// A player joined a room.
//////////////////////////
void PEER_PlayerJoinedCallback(
		PEER peer,  // The peer object.
		RoomType roomType,  // The type of room that the player joined.
		const char * nick,  // The nick of the player that joined.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(roomType==StagingRoom){
		if(TPE->MyRoom&&TPE->MyRoom->RoomConnected){
			//TPE->MyRoom->ChangeMyReady();
			//TPE->MyRoom->ChangeMyReady();
		};
	};
	TPE->AddPlayer((char*)nick);
};

// A player left a room.
////////////////////////
void PEER_PlayerLeftCallback(
		PEER peer,  // The peer object.
		RoomType roomType,  // The type of room that the player left.
		const char * nick,  // The nick of the player that left.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	TPE->DelPlayer((char*)nick);
};

// A player in one of the rooms changed his nick.
/////////////////////////////////////////////////
void PEER_PlayerChangedNickCallback(
		PEER peer,  // The peer object.
		RoomType roomType,  // The type of the room the nick changed was in.
		const char * oldNick,  // The player's old nick.
		const char * newNick,  // The player's new nick.
		void * param  // User-data.
	){

};

// An updated ping for a player, who may be in any room(s).
///////////////////////////////////////////////////////////
void PEER_PingCallback(
		PEER peer,  // The peer object.
		const char * nick,  // The other player's nick.
		int ping,  // The ping.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	TPE->SetPing((char*)nick,ping);
	if(TPE->MyRoom&&TPE->MyRoom->RoomConnected)TPE->MyRoom->SetPing((char*)nick,ping);
};

// An updated cross-ping between two players in the staging room.
/////////////////////////////////////////////////////////////////
void PEER_CrossPingCallback(
		PEER peer,  // The peer object.
		const char * nick1,  // The first player's nick.
		const char * nick2,  // The second player's nick.
		int crossPing,  // The cross-ping.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
};

// Type used for the GOA callbacks (basic, info, rules, players).
/////////////////////////////////////////////////////////////////
void PEER_GOABasic(
		PEER peer,  // The peer object.
		PEERBool playing,  // PEERTrue if the game is being played.
		char * outbuf,  // Output buffer.
		int maxlen,  // Max data that can be written to outbuf
		void * param // User-data
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;

};
void PEER_GOAInfo(
		PEER peer,  // The peer object.
		PEERBool playing,  // PEERTrue if the game is being played.
		char * outbuf,  // Output buffer.
		int maxlen,  // Max data that can be written to outbuf
		void * param // User-data
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;

};
void PEER_GOAPlayers(
		PEER peer,  // The peer object.
		PEERBool playing,  // PEERTrue if the game is being played.
		char * outbuf,  // Output buffer.
		int maxlen,  // Max data that can be written to outbuf
		void * param // User-data
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;

};
void PEER_GOARules(
		PEER peer,  // The peer object.
		PEERBool playing,  // PEERTrue if the game is being played.
		char * outbuf,  // Output buffer.
		int maxlen,  // Max data that can be written to outbuf
		void * param // User-data
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;

};
bool NormName(char* cc){
	int L=strlen(cc);
	bool ch=0;
	for(int i=0;i<L;i++){
		unsigned char c=cc[i];
		if(!(c=='_'||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||
			c=='{'||c=='}'||c=='['||c==']'||c=='('||c==')')){
			c='_';
			cc[i]=c;
			ch=1;
		}
	};
	return ch;
};
void PEER_NickErrorCallback(
		PEER peer,  // The peer object.
		int type,  // The type of nick error
			// PEER_IN_USE:  The nick is already being used by someone else.
			// PEER_INVALID: The nick contains invalid characters.
		const char * nick,  // The bad nick.
		void * param  // User-data.
	){
	char ccx[128];
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(type==PEER_IN_USE){
		TPE->State=1;
		strcpy(ccx,nick);
		char* cc0=strstr(ccx,"{1}");
		if(cc0)strcpy(cc0,"{2}");
		else{
			cc0=strstr(ccx,"{2}");
			if(cc0)strcpy(cc0,"{3}");
			else{
				cc0=strstr(ccx,"{3}");
				if(cc0)strcpy(cc0,"{4}");
				else{
					cc0=strstr(ccx,"{4}");
					if(cc0)strcpy(cc0,"{5}");
					else{
						cc0=strstr(ccx,"{5}");
						if(cc0){
							TPE->ErrorType=2;
							TPE->State=2;
						}else{
							strcat(ccx,"{1}");
						};
					};
				};
			};
		};
		if(TPE->State==1){
			strcpy(TPE->MyNick,ccx);
			peerRetryWithNick(TPE->Peer,ccx);
		};
	}else{
		strcpy(ccx,nick);
		if(NormName(ccx)){
		}else{
			TPE->State=2;
			TPE->ErrorType=2;
		};
	};
};
void PEER_EnumPlayersCallback(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		RoomType roomType,  // The room whose players are being enumerated.
		int index,  // The index of the player (0 to (N - 1)).  -1 when finished.
		const char * nick,  // The nick of the player.
		PEERBool host,  // PEERTrue if this player is a host in this room (has ops).
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(success){
		if(roomType==TitleRoom){
			TPE->AddPlayer((char*)nick);
		};
	};
};
void PEER_ListingGamesCallback(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		const char * name,  // The name of the game being listed.
		GServer server,  // The server object for this game.
		PEERBool staging,  // If PEERTrue, this is a staging room and not a running game.
		int msg,  // The type of message this is.
			// PEER_CLEAR:
			//   Clear the list.  This has the same effect as if this was called
			//   with PEER_REMOVE for every server listed.
			// PEER_ADD:
			//   This is a new server. Add it to the list.
			// PEER_UPDATE:
			//   This server is already on the list, and its been updated.
			// PEER_REMOVE:
			//   Remove this server from the list.  The server object for this server
			//   should not be used again after this callback returns.
		int progress,  // The percent of servers that have been added.
		void * param  // User-data.
	){
	if(!success)return;
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	switch(msg){
	case PEER_CLEAR:
		TPE->ClearServers();
		break;
	case PEER_ADD:
		TPE->AddServer((char*)name,server);
		break;
	case PEER_UPDATE:
		TPE->UpdateServer((char*)name,server);
		break;
	case PEER_REMOVE:
		TPE->DelServer((char*)name,server);
		break;
	};
};
// This gets called when the connection attempt finishes.
/////////////////////////////////////////////////////////
void PEER_JoinRoomCallback(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		RoomType roomType,  // The type of room joined/created.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(success&&roomType==TitleRoom){
		TPE->Connected=1;
		TPE->State=5;
		peerEnumPlayers(TPE->Peer,TitleRoom,&PEER_EnumPlayersCallback,param);
		peerStartListingGames(TPE->Peer,&PEER_ListingGamesCallback,param);
	};

};
void PEER_ConnectCallback(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	TPE->State=1;
	if(success){
		TPE->Connected=0;
		TPE->State=3;
		PEERBool PINGS[NumRooms];
		PINGS[TitleRoom]=PEERTrue;
		PINGS[GroupRoom]=PEERTrue;
		PINGS[StagingRoom]=PEERTrue;
		PEERBool PINGS_CROSS[NumRooms];
		PINGS_CROSS[TitleRoom]=PEERFalse;
		PINGS_CROSS[GroupRoom]=PEERFalse;
		PINGS_CROSS[StagingRoom]=PEERFalse;
		peerSetTitle(TPE->Peer,"gmtest","HA6zkS", "gmtest", "HA6zkS", 20, PINGS, PINGS_CROSS);
		peerJoinTitleRoom(TPE->Peer,&PEER_JoinRoomCallback,param,PEERFalse);
	}else{
		if(GetTickCount()-TPE->ConnStartTime<60000){
			peerConnect(TPE->Peer,TPE->MyNick,0,&PEER_NickErrorCallback,&PEER_ConnectCallback,param,PEERFalse);
		}else 
			TPE->ErrorType=1;
	};
};
//ERRORS:
//0-No error
//1-Unable to connect to chat server
//2-Invalid user name
void TotalPeerEngine::Connect(char* Nick){
	PEERCallbacks PCB;
	ConnStartTime=GetTickCount();
	strcpy(MyNick,Nick);
	NormName(MyNick);
	PCB.crossPing=&PEER_CrossPingCallback;
	PCB.disconnected=&PEER_DisconnectCallback;
	PCB.gameStarted=&PEER_GameStartedCallback;
	PCB.ping=&PEER_PingCallback;
	PCB.playerChangedNick=&PEER_PlayerChangedNickCallback;
	PCB.playerJoined=&PEER_PlayerJoinedCallback;
	PCB.playerLeft=&PEER_PlayerJoinedCallback;
	PCB.playerMessage=&PEER_PlayerMessageCallback;
	PCB.readyChanged=&PEER_ReadyChangedCallback;
	PCB.roomMessage=&PEER_RoomMessageCallback;
	PCB.roomNameChanged=&PEER_RoomNameChangedCallback;
	PCB.GOABasic=&PEER_GOABasic;
	PCB.GOAInfo=&PEER_GOAInfo;
	PCB.GOAPlayers=&PEER_GOAPlayers;;
	PCB.GOARules=&PEER_GOARules;
	PCB.param=(void*)this;
	Runned=0;
	Peer=peerInitialize(&PCB);
	State=1;
	ErrorType=0;
	Connected=0;
	peerConnect(Peer,MyNick,0,&PEER_NickErrorCallback,&PEER_ConnectCallback,this,PEERFalse);
};
void TotalPeerEngine::RefreshPlayers(){
	if(!Connected)return;
	//NPlayers=0;
	int N0=NPlayers;
	int S=0;
	for(int i=0;i<NPlayers;i++){
		int L=strlen(Players[i].Name);
		char* c=Players[i].Name;
		for(int j=0;j<L;j++)S+=c[j]+(int(c[j+1])<<8);
		Players[i].Status|=1024;
	};
	peerEnumPlayers(Peer,TitleRoom,&PEER_EnumPlayersCallback,this);
	for(i=0;i<NPlayers;i++){
		if(Players[i].Status&1024){
			DelPlayer(Players[i].Name);
		};
	};
	int S1=0;
	for(i=0;i<NPlayers;i++){
		int L=strlen(Players[i].Name);
		char* c=Players[i].Name;
		for(int j=0;j<L;j++)S1+=c[j]+(int(c[j+1])<<8);
	};
	if(N0==NPlayers&&S1==S)Change=false;
};
void ChatQueue::Clear(){
	for(int i=0;i<NCStr;i++){
		free(CStr[i].Message);
		free(CStr[i].PlName);
	};
	if(CStr){
		free(CStr);
	};
	memset(this,0,sizeof ChatQueue);
};
void ChatQueue::Add(char* nick,char* Message){
	if(NCStr>=MaxCStr){
		MaxCStr+=16;
		CStr=(ChatString*)realloc(CStr,MaxCStr*sizeof ChatString);
	};
	CStr[NCStr].Message=new char[strlen(Message)+1];
	strcpy(CStr[NCStr].Message,Message);
	CStr[NCStr].PlName=new char[strlen(nick)+1];
	strcpy(CStr[NCStr].PlName,nick);
	NCStr++;
};
void PEER_EnumPlayersCallback2(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		RoomType roomType,  // The room whose players are being enumerated.
		int index,  // The index of the player (0 to (N - 1)).  -1 when finished.
		const char * nick,  // The nick of the player.
		PEERBool host,  // PEERTrue if this player is a host in this room (has ops).
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(success&&roomType==StagingRoom&&TPE->MyRoom){
		TPE->MyRoom->AddPlayer((char*)nick);
	};
};
void PEER_EnumPlayersCallback3(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		RoomType roomType,  // The room whose players are being enumerated.
		int index,  // The index of the player (0 to (N - 1)).  -1 when finished.
		const char * nick,  // The nick of the player.
		PEERBool host,  // PEERTrue if this player is a host in this room (has ops).
		void * param  // User-data.
	){
	TPE_MyRoom* TPR=(TPE_MyRoom*)param;
	if(success&&roomType==StagingRoom&&TPR){
		TPR->AddPlayer((char*)nick);
		PEERBool PB;
		peerGetReady(TPR->Peer,nick,&PB);
		TPR->SetReady((char*)nick,bool(PB));
	};
};
void PEER_JoinRoomCallback2(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		RoomType roomType,  // The type of room joined/created.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(success&&roomType==StagingRoom){
		TPE->MyRoom->RoomConnected=1;
		//peerEnumPlayers(TPE->Peer,StagingRoom,&PEER_EnumPlayersCallback2,param);
		TPE->MyRoom->Host=1;
	}else{
		free(TPE->MyRoom);
		TPE->MyRoom=NULL;
	};

};
void PEER_JoinRoomCallback3(
		PEER peer,  // The peer object.
		PEERBool success,  // PEERTrue if success, PEERFalse if failure.
		RoomType roomType,  // The type of room joined/created.
		void * param  // User-data.
	){
	TotalPeerEngine* TPE=(TotalPeerEngine*)param;
	if(success&&roomType==StagingRoom){
		TPE->MyRoom->RoomConnected=1;
		//peerEnumPlayers(TPE->Peer,StagingRoom,&PEER_EnumPlayersCallback2,param);
		TPE->MyRoom->Host=0;
	}else{
		free(TPE->MyRoom);
		TPE->MyRoom=NULL;
	};
};
void TotalPeerEngine::CreateStagingRoom(char* Name,char* Password,int MaxPlayers){
	if(!Connected)return;
	MyRoom=new TPE_MyRoom;
	memset(MyRoom,0,sizeof TPE_MyRoom);
	MyRoom->TPE=this;
	strcpy(MyRoom->Name,Name);
	MyRoom->MaxPlayer=MaxPlayers;
	MyRoom->Peer=Peer;
	peerCreateStagingRoom(Peer,Name,MaxPlayers,Password,&PEER_JoinRoomCallback2,this,PEERFalse);
};
void TotalPeerEngine::AddServer(char* Name,GServer server){
	if(!Connected)return;
	if(NRooms>=MaxRooms){
		MaxRooms+=32;
		Room=(TPE_Room*)realloc(Room,MaxRooms*sizeof(TPE_Room));
	};
	Room[NRooms].G=server;
	strcpy(Room[NRooms].Name,Name);
	NRooms++;
	ChangeServer=1;
};
void TotalPeerEngine::DelServer(char* Name,GServer server){
	if(!Connected)return;
	for(int i=0;i<NRooms;i++)if(Room[i].G==server&&!strcmp(Room[i].Name,Name)){
		if(i<NRooms-1)memcpy(Room+i,Room+i+1,(NRooms-i-1)*sizeof TPE_Room);
		NRooms--;
		ChangeServer=1;
		return;
	};
};
void TotalPeerEngine::UpdateServer(char* Name,GServer server){
};
void TotalPeerEngine::ClearServers(){
	NRooms=0;
	ChangeServer=1;
};
void TPE_MyRoom::AddPlayer(char* nick){
	for(int i=0;i<NPlayers;i++)if(!strcmp(nick,Players[i].Name)){
		Players[i].Status&=1023;
		return;
	};
	if(NPlayers>=MaxPlayers){
		MaxPlayers+=32;
		Players=(TPE_Player*)realloc(Players,MaxPlayers*sizeof TPE_Player);
	};
	strcpy(Players[NPlayers].Name,nick);
	Players[NPlayers].Ping=-1;
	Players[NPlayers].Status=0;
	Players[NPlayers].ready=0;
	NPlayers++;
	Change=1;
};
void TPE_MyRoom::DelPlayer(char* nick){
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,nick)){
		if(i<NPlayers-1)
			memcpy(Players+i,Players+i+1,(NPlayers-i-i)*sizeof TPE_Player);
		NPlayers--;
		i--;
	};
	Change=1;
};
void TPE_MyRoom::SetPing(char* nick,int Ping){
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,nick)){
		Players[i].Ping=Ping;
	};
	Change=1;
};
void TPE_MyRoom::SetReady(char* nick,bool State){
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,nick)){
		Players[i].ready=State;
	};
	Change=1;
};
void TPE_MyRoom::Refresh(){
	if(!(this&&RoomConnected))return;
	int N0=NPlayers;
	int S=0;
	for(int i=0;i<NPlayers;i++){
		int L=strlen(Players[i].Name);
		char* c=Players[i].Name;
		for(int j=0;j<L;j++)S+=c[j]+(int(c[j+1])<<8);
		S+=Players[i].ready;
		Players[i].Status|=1024;
	};
	peerEnumPlayers(Peer,StagingRoom,&PEER_EnumPlayersCallback3,this);
	for(i=0;i<NPlayers;i++){
		if(Players[i].Status&1024){
			DelPlayer(Players[i].Name);
		};
	};
	bool PRHOST=0;
	int S1=0;
	for(i=0;i<NPlayers;i++){
		int L=strlen(Players[i].Name);
		char* c=Players[i].Name;
		for(int j=0;j<L;j++)S1+=c[j]+(int(c[j+1])<<8);
		S1+=Players[i].ready;
		if(peerIsPlayerHost(Peer,Players[i].Name,StagingRoom))PRHOST=1;
	};
	if(N0==NPlayers&&S1==S)Change=false;
	if(!PRHOST)TPE->LeaveMyRoom();
};
void TotalPeerEngine::RefreshSessions(){
	peerStartListingGames(Peer,&PEER_ListingGamesCallback,this);
};
void TotalPeerEngine::SendLocalChat(char* Message){
	if(!Connected)return;
	if(MyRoom&&MyRoom->RoomConnected){
		peerMessageRoom(Peer,StagingRoom,Message);
	};
};
void TotalPeerEngine::JoinRoom(TPE_Room* Room){
	if(!Connected)return;
	if(MyRoom)return;
	MyRoom=new TPE_MyRoom;
	memset(MyRoom,0,sizeof TPE_MyRoom);
	MyRoom->TPE=this;
	strcpy(MyRoom->Name,Room->Name);
	MyRoom->MaxPlayer=Room->MaxPlayer;
	MyRoom->Peer=Peer;
	peerJoinStagingRoom(Peer,Room->G,"",PEER_JoinRoomCallback2,this,PEERFalse);
};
void TotalPeerEngine::LeaveMyRoom(){
	if(!Connected)return;
	if(MyRoom&&MyRoom->RoomConnected){
		peerLeaveRoom(Peer,StagingRoom);
		if(MyRoom->Players){
			free(MyRoom->Players);
		};
		free(MyRoom);
		MyRoom=NULL;
	}else{
		if(MyRoom){
			free(MyRoom);
			MyRoom=NULL;
		};
	};
};
void TPE_MyRoom::ChangeMyReady(){
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,TPE->MyNick)){
		peerSetReady(Peer,PEERBool(!Players[i].ready));
	};
};
void TPE_MyRoom::ResetMyReady(){
	for(int i=0;i<NPlayers;i++)if(!strcmp(Players[i].Name,TPE->MyNick)){
		peerSetReady(Peer,PEERBool(Players[i].ready));
	};
};
void TPE_MyRoom::SetMyReady(bool State){
	peerSetReady(Peer,PEERBool(PEERBool(State)));
};
void TPE_MyRoom::RunGame(){
	if(Host){
		peerStartGame(Peer,"GO GO GO",PEERFalse);
		TPE->Runned=1;
		TPE->HOSTIP=0;
	};
};