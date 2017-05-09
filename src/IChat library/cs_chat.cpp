#include "../Main executable/common.h"
#pragma pack(4)
#include "Chat\chat.h"
#pragma pack(1)
#include "cs_chat.h"

#define current_time GetTickCount
void ChatSystem::AddPlayer( char* Nick, int c )
{
	int i;
	for (i = 0; i < CCH[c].NPlayers; i++)if (!strcmp( Nick, CCH[c].Players[i].Nick ))return;
	if (CCH[c].NPlayers >= CCH[c].MaxPlayers)
	{
		CCH[c].MaxPlayers += 64;
		CCH[c].Players = (OneChatPlayer*) realloc( CCH[c].Players, CCH[c].MaxPlayers * sizeof OneChatPlayer );
	}
	memset( CCH[c].Players + CCH[c].NPlayers, 0, sizeof OneChatPlayer );
	strcpy( CCH[c].Players[CCH[c].NPlayers].Nick, Nick );
	CCH[c].NPlayers++;
}

void ChatSystem::DelPlayer( char* Nick, int c )
{
	for (int i = 0; i < CCH[c].NPlayers; i++)if (!strcmp( CCH[c].Players[i].Nick, Nick ))
	{
		if (i < CCH[c].NPlayers - 1)
		{
			memcpy( CCH[c].Players + i, CCH[c].Players + i + 1, ( CCH[c].NPlayers - i - 1 ) * sizeof OneChatPlayer );
		};
		CCH[c].NPlayers--;
		return;
	};
}

void ChatSystem::AddAbsentPlayer( char* Nick )
{
	int i;
	for (i = 0; i < NAbsPlayers; i++)if (!strcmp( Nick, AbsPlayers[i].Nick ))return;
	if (NAbsPlayers >= MaxAbsPlayers)
	{
		MaxAbsPlayers += 64;
		AbsPlayers = (OneChatPlayer*) realloc( AbsPlayers, MaxAbsPlayers * sizeof OneChatPlayer );
	};
	memset( AbsPlayers + NAbsPlayers, 0, sizeof OneChatPlayer );
	strcpy( AbsPlayers[NAbsPlayers].Nick, Nick );
	NAbsPlayers++;
};
void ChatSystem::DelAbsentPlayer( char* Nick )
{
	for (int i = 0; i < NAbsPlayers; i++)if (!strcmp( AbsPlayers[i].Nick, Nick ))
	{
		if (i < NAbsPlayers - 1)
		{
			memcpy( AbsPlayers + i, AbsPlayers + i + 1, ( NAbsPlayers - i - 1 ) * sizeof OneChatPlayer );
		};
		NAbsPlayers--;
		return;
	};
};
void ChatSystem::Disconnect()
{
	if (Connected&&chat)
	{
		for (int i = 0; i < NCHNL; i++)if (enterChannelSuccess[i])
		{
			chatLeaveChannel( chat, chatChannel[i], "" );
			enterChannelSuccess[i] = CHATFalse;
			CCH[i].NPlayers = 0;
		};
		chatDisconnect( chat );
	};
};
void Raw( CHAT chat, const char * raw, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}
void Disconnected( CHAT chat, const char * reason, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->Connected = 0;
}

void ChangedNickCallback( CHAT chat, CHATBool success, const char * oldNick, const char * newNick, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}
__declspec( dllimport ) void AddToGChat( char* Nick, char* Message );
__declspec( dllimport ) void ClearGChat();
void PlayStart();
void PrivateMessage( CHAT chat, const char * user, const char * message, int type, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		CC->CheckMessage( (char*) message );
		if (!strcmp( message, "@@@NFO \\$flags$\\g\\" ))
		{
			CC->AddAbsentPlayer( (char*) user );
		}
		else if (!strstr( message, "@@@" ))
		{
			bool mute = 0;
			int c = CSYS.CurChannel;
			for (int i = 0; i < CSYS.CCH[c].NPlayers; i++)
				if (( !strcmp( CSYS.CCH[c].Players[i].Nick, user ) ) && CSYS.CCH[c].Players[i].Muted)mute = 1;
			for (int i = 0; i < CSYS.NAbsPlayers; i++)
				if (( !strcmp( CSYS.AbsPlayers[i].Nick, user ) ) && CSYS.AbsPlayers[i].Muted)mute = 1;
			if (!mute)
			{
				CC->Private.Add( (char*) user, (char*) message );
				char cc[128];
				sprintf( cc, "%s%s:", user, GetTextByID( "ONLYOU" ) );
				CC->Common[CC->CurChannel].Add( cc, (char*) message );
				AddToGChat( cc, (char*) message );
			};
			PlayStart();
		};
		CC->LastAccessTime = current_time();
	};
}

void Invited( CHAT chat, const char * channel, const char * user, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void ChannelMessage( CHAT chat, const char * channel, const char * user, const char * message, int type, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		for (int q = 0; q < NCHNL; q++)if (!strcmp( CC->chatChannel[q], channel ))
		{
			CC->CheckMessage( (char*) message );
			if (!strcmp( message, "@@@NFO \\$flags$\\g\\" ))
			{
				CC->AddAbsentPlayer( (char*) user );
			}
			else if (!strstr( message, "@@@" ))
			{
				bool mute = 0;
				for (int i = 0; i < CSYS.CCH[q].NPlayers; i++)
					if (( !strcmp( CSYS.CCH[q].Players[i].Nick, user ) ) && CSYS.CCH[q].Players[i].Muted)mute = 1;
				for (int i = 0; i < CSYS.NAbsPlayers; i++)
					if (( !strcmp( CSYS.AbsPlayers[i].Nick, user ) ) && CSYS.AbsPlayers[i].Muted)mute = 1;
				if (!mute)CC->Common[q].Add( (char*) user, (char*) message );
			};
		};
		CC->LastAccessTime = current_time();
	};
}

void Kicked( CHAT chat, const char * channel, const char * user, const char * reason, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void UserJoined( CHAT chat, const char * channel, const char * user, int mode, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		CC->DelAbsentPlayer( (char*) user );
		for (int i = 0; i < NCHNL; i++)if (!strcmp( CC->chatChannel[i], channel ))CC->AddPlayer( (char*) user, i );
		CC->LastAccessTime = current_time();
	};
}
void DelPlayer( char* Nick );
void UserParted( CHAT chat, const char * channel, const char * user, int why, const char * reason, const char * kicker, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		for (int i = 0; i < NCHNL; i++)if (!strcmp( CC->chatChannel[i], channel ))CC->DelPlayer( (char*) user, i );
		CC->LastAccessTime = current_time();
	};

}

void UserChangedNick( CHAT chat, const char * channel, const char * oldNick, const char * newNick, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void UserModeChanged( CHAT chat, const char * channel, const char * user, int mode, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void TopicChanged( CHAT chat, const char * channel, const char * topic, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void ChannelModeChanged( CHAT chat, const char * channel, CHATChannelMode * mode, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void UserListUpdated( CHAT chat, const char * channel, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
}

void ConnectCallback( CHAT chat, CHATBool success, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)CC->LastAccessTime = current_time();
	if (success)CC->Connected = 1;
	else CC->Error = 1;
}

void NickErrorCallback( CHAT chat, int type, const char * nick, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		CC->LastAccessTime = current_time();
		if (type == CHAT_IN_USE)
		{
			char NICK[64];
			strcpy( NICK, nick );
			char* ss;
			if (!strchr( NICK, '{' ))
			{
				strcat( NICK, "{1}" );
			}
			else
			{
				ss = strstr( NICK, "{1}" );
				if (ss)
				{
					strcpy( ss, "{2}" );
				}
				else
				{
					ss = strstr( NICK, "{2)" );
					if (ss)
					{
						strcpy( ss, "{3}" );
					}
					else
					{
						ss = strstr( NICK, "{3)" );
						if (ss)
						{
							strcpy( ss, "{4}" );
						}
						else
						{
							ss = strstr( NICK, "{5)" );
							if (ss)
							{
								strcpy( ss, "{6}" );
							};
						};
					};
				};
			};
			if (strcmp( NICK, nick ))chatRetryWithNick( chat, NICK );
			else
			{
				if (chat)chatDisconnect( chat );
				CC->Connected = 0;
				chat = 0;
			};
		}
		else if (type == CHAT_INVALID)
		{
			if (chat)chatDisconnect( chat );
			CC->Connected = 0;
		}
	};
};

void EnumUsersCallback( CHAT chat, CHATBool success, const char * channel, int numUsers, const char ** users, int * modes, void * param );
void EnterChannelCallback( CHAT chat, CHATBool success, CHATEnterResult result, const char * channel, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		if (!success)CC->Error = 1;
		CC->LastAccessTime = current_time();
		for (int i = 0; i < NCHNL; i++)if (!strcmp( CC->chatChannel[i], channel ))
		{
			CC->enterChannelSuccess[i] = success;
			chatEnumUsers( CC->chat, channel, &EnumUsersCallback, CC, CHATFalse );
		};
	};
}

void GetUserInfoCallback( CHAT chat, CHATBool success, const char * nick, const char * user, const char * name, const char * address, int numChannels, const char ** channels, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		if (!success)
		{
			CC->DelAbsentPlayer( (char*) nick );
			//CC->DelPlayer((char*)nick);
		}
		else
		{
			for (int q = 0; q < NCHNL; q++)
			{
				for (int i = 0; i < CC->CCH[q].NPlayers; i++)if (!strcmp( CC->CCH[q].Players[i].Nick, nick ))
				{
					strcpy( CC->CCH[q].Players[i].Info, user );
					strcpy( CC->CCH[q].Players[i].mail, name );
				};
			};
			for (int i = 0; i < CC->NAbsPlayers; i++)if (!strcmp( CC->AbsPlayers[i].Nick, nick ))
			{
				strcpy( CC->AbsPlayers[i].Info, user );
				strcpy( CC->AbsPlayers[i].mail, name );
			};
		};
	};
}

void EnumUsersCallback( CHAT chat, CHATBool success, const char * channel, int numUsers, const char ** users, int * modes, void * param )
{
	ChatSystem* CC = (ChatSystem*) param;
	if (CC)
	{
		if (success)
		{
			for (int q = 0; q < NCHNL; q++)if (!strcmp( CC->chatChannel[q], channel ))
			{
				for (int i = 0; i < numUsers; i++)CC->AddPlayer( (char*) users[i], q );
			};
		};
	};
};
void ChatSystem::CheckMessage( char* message )
{
	char *cc;
	char cc3[128];
	BOOL PRESENT;
	int i;
	cc = strstr( message, "@@@ADDABPL " );
	if (cc)
	{
		cc += 11;
		do
		{
			char* cc1 = strstr( cc, " " );
			if (cc1)
			{
				int L = cc1 - cc;
				memcpy( cc3, cc, L );
				cc3[L] = 0;
				cc = cc1 + 1;
			}
			else
			{
				strcpy( cc3, cc );
				cc = NULL;
			};
			PRESENT = 0;
			for (int q = 0; q < NCHNL; q++)for (i = 0; i < CCH[q].NPlayers; i++)
				if (!strcmp( CCH[q].Players[i].Nick, cc3 ))PRESENT = 1;
			if (!PRESENT)AddAbsentPlayer( cc3 );
		} while (cc);
	};
};
void ChatBroadcastKeyChanged( CHAT chat,
	const char * channel,
	const char * user,
	const char * key,
	const char * value,
	void * param )
{};

void ChatSystem::Setup()
{
	strcpy( serverAddress, "peerchat.gamespy.com" );
	port = 6667;
	strcpy( chatChannel[0], "#GSP!cossacks" );
	strcpy( chatChannel[1], "#GSP!cossacks!A2V001" );
	//strcpy(chatChannel, "#GSP!cossacks");
	memset( &globalCallbacks, 0, sizeof( chatGlobalCallbacks ) );

	globalCallbacks.raw = Raw;
	globalCallbacks.disconnected = Disconnected;
	globalCallbacks.privateMessage = PrivateMessage;
	globalCallbacks.invited = Invited;
	globalCallbacks.param = this;

	memset( &channelCallbacks, 0, sizeof( chatChannelCallbacks ) );
	channelCallbacks.channelMessage = ChannelMessage;
	channelCallbacks.channelModeChanged = ChannelModeChanged;
	channelCallbacks.kicked = Kicked;
	channelCallbacks.topicChanged = TopicChanged;
	channelCallbacks.userParted = UserParted;
	channelCallbacks.userJoined = UserJoined;
	channelCallbacks.userListUpdated = UserListUpdated;
	channelCallbacks.userModeChanged = UserModeChanged;
	channelCallbacks.userChangedNick = UserChangedNick;
	channelCallbacks.param = this;
	channelCallbacks.broadcastKeyChanged = &ChatBroadcastKeyChanged;
};
void ChatSystem::Process()
{
	if (chat)chatThink( chat );
	for (int q = 0; q < NCHNL; q++)SortPlayers( CCH[q].Players, CCH[q].NPlayers );
	SortPlayers( AbsPlayers, NAbsPlayers );
	int NPL = CCH[NCHNL - 1].NPlayers;
	int NS = 0;
	for (int i = 0; i < NPL&&NS < 4; i++)
	{
		if (rand() < 2000 && !CCH[NCHNL - 1].Players[i].InfoReqSent)
		{
			chatGetUserInfo( chat, CCH[NCHNL - 1].Players[i].Nick, &GetUserInfoCallback, this, CHATFalse );
			CCH[NCHNL - 1].Players[i].InfoReqSent = 1;
			NS++;
		};
	};
}

__declspec( dllexport ) void ChatProcess()
{
	if (CSYS.chat)chatThink( CSYS.chat );
}

bool ChatSystem::ConnectToChat( char* Nick, char* Info, char* Mail, char* Chat )
{
	strcpy( chatNick, Nick );
	strcpy( chatUser, Info );
	strcpy( chatName, Mail );
	Error = 0;
	Disconnect();
	Connected = 0;
	Setup();
	strcpy( serverAddress, Chat );
	chat = chatConnect( serverAddress, port, chatNick, chatUser, chatName, &globalCallbacks, NickErrorCallback, ConnectCallback, this, CHATFalse );
	int t0 = current_time();
	do
	{
		ProcessMessages();
		Process();
	} while (!( Connected || Error || current_time() - t0 > 40000 ));
	if (Error || current_time() - t0 > 40000)
	{
		Connected = 0;
		Error = 1;
		if (chat)chatDisconnect( chat );
	}
	else
	{
		for (int q = 0; q < NCHNL; q++)
			chatEnterChannel( chat, chatChannel[q], NULL, &channelCallbacks, EnterChannelCallback, this, CHATFalse );
		t0 = current_time();
		do
		{
			ProcessMessages();
			Process();
		} while (!( ( enterChannelSuccess[0] && enterChannelSuccess[1] ) || Error || current_time() - t0 > 20000 ));
		if (Error || current_time() - t0 > 20000)
		{
			Connected = 0;
			Error = 1;
			if (chat)chatDisconnect( chat );
		};
	};
	return Connected;
};
ChatSystem::ChatSystem()
{
	memset( this, 0, sizeof* this );
};
ChatSystem::~ChatSystem()
{
	for (int q = 0; q < NCHNL; q++)
	{
		if (CCH[q].Players)free( CCH[q].Players );
	};
	if (AbsPlayers)free( AbsPlayers );
	memset( this, 0, sizeof* this );
};
ChatMsg::ChatMsg()
{
	memset( this, 0, sizeof *this );
};
ChatMsg::~ChatMsg()
{
	for (int i = 0; i < NMsg; i++)
	{
		free( Message[i] );
		free( Nick[i] );
	};
	if (Message)free( Message );
	if (Nick)free( Nick );
	memset( this, 0, sizeof *this );
};
void ChatMsg::Add( char* nick, char* msg )
{
	if (NMsg >= MaxMsg)
	{
		MaxMsg += 16;
		Message = (char**) realloc( Message, MaxMsg << 2 );
		Nick = (char**) realloc( Nick, MaxMsg << 2 );
	};
	Message[NMsg] = (char*) malloc( strlen( msg ) + 1 );
	strcpy( Message[NMsg], msg );
	Nick[NMsg] = (char*) malloc( strlen( nick ) + 1 );
	strcpy( Nick[NMsg], nick );
	NMsg++;
};
bool ChatMsg::RemoveOne( char* nick, char* buf, int Len )
{
	if (NMsg)
	{
		strcpy( nick, Nick[0] );
		strncpy( buf, Message[0], Len - 1 );
		buf[Len - 1] = 0;
		free( Nick[0] );
		free( Message[0] );
		if (NMsg > 1)
		{
			memmove( Nick, Nick + 1, ( NMsg - 1 ) << 2 );
			memmove( Message, Message + 1, ( NMsg - 1 ) << 2 );
		};
		NMsg--;
		return true;
	}
	else return false;
};
void ChatSystem::SortPlayers( OneChatPlayer* PL, int N )
{
	bool change = 0;
	do
	{
		change = 0;
		for (int i = 1; i < N; i++)
		{
			int r = stricmp( PL[i].Nick, PL[i - 1].Nick );
			if (r < 0)
			{
				byte PLX[sizeof OneChatPlayer];
				memcpy( &PLX, PL + i, sizeof OneChatPlayer );
				memcpy( PL + i, PL + i - 1, sizeof OneChatPlayer );
				memcpy( PL + i - 1, &PLX, sizeof OneChatPlayer );
				change = 1;
			};
		};
	} while (change);
};
int ChatSystem::NPlayers()
{
	return CCH[CurChannel].NPlayers;
};
OneChatPlayer* ChatSystem::Players()
{
	return CCH[CurChannel].Players;
};