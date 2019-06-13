#include "../Main executable/common.h"
#include "Chat\chat.h"
#include "cs_chat.h"
#include "../IntExplorer library/ParseRQ.h"
#include <stdarg.h>

#include <cstring>
#include <cstdlib>

__declspec( dllimport ) int GetSound( char* Name );
__declspec( dllimport ) void PlayEffect( int n, int pan, int vol );

void PlayStart()
{
	PlayEffect( GetSound( "START" ), 0, 0 );
}

bool ReadWinString( GFILE* F, char* STR, int Max )
{
	STR[0] = 0;
	int cc = 0;
	int nn = 0;
	while (!( cc == 0x0A || cc == EOF ))
	{
		cc = Ggetch( F );
		if (Max > 2 && !( cc == 0x0A || cc == EOF ))
		{
			STR[nn] = (char) cc;
			nn++;
			Max--;
		};
	};
	STR[nn] = 0;
	return ( cc != EOF ) || STR[0];
}

class IniFile
{
	char** Vars;
	char** Values;
	int NVal;
	int MaxVal;
public:
	void Clear()
	{
		for (int i = 0; i < NVal; i++)
		{
			free( Vars[i] );
			free( Values[i] );
		}

		if (Vars)
		{
			free( Vars );
		}

		if (Values)
		{
			free( Values );
		}

		memset( this, 0, sizeof *this );
	}

	bool Load( char* name )
	{
		GFILE* F = Gopen( name, "r" );
		if (F)
		{
			char ccc[512];
			char ccc1[512];
			bool done = 0;
			do
			{
				done = ReadWinString( F, ccc, 511 );
				if (done)
				{
					if (NVal >= MaxVal)
					{
						MaxVal += 32;
						Vars = (char**) realloc( Vars, MaxVal << 2 );
						Values = (char**) realloc( Values, MaxVal << 2 );
					}
					sscanf( ccc, "%s", ccc1 );
					char* ssv = ccc + strlen( ccc1 ) + 1;
					Vars[NVal] = (char*) malloc( strlen( ccc1 ) + 1 );
					strcpy( Vars[NVal], ccc1 );
					Values[NVal] = (char*) malloc( strlen( ssv ) + 1 );
					strcpy( Values[NVal], ssv );
					NVal++;
				}
			} while (done);
			Gclose( F );
			return true;
		}
		return false;
	}

	char* GetStr( char* ID )
	{
		for (int i = 0; i < NVal; i++)if (!strcmp( ID, Vars[i] ))return Values[i];
		return ID;
	}

	int GetInt( char* ID )
	{
		char* v = GetStr( ID );
		return atoi( v );
	}

	IniFile()
	{
		memset( this, 0, sizeof *this );
	}

	~IniFile()
	{
		Clear();
	}
};

__declspec( dllimport ) void ExplorerBack( int Index );
__declspec( dllimport ) void ExplorerForw( int Index );
__declspec( dllimport ) void ExplorerRefresh( int Index );
__declspec( dllimport ) void ExplorerSetVar( int Index, char* Name, char* value );
__declspec( dllimport ) char* ExplorerGetVar( int Index, char* Name );
__declspec( dllimport ) char LobbyVersion[32];
__declspec( dllimport ) word dwVersion;
__declspec( dllimport ) void RunExplorer( int Index, char* ref, int x, int y, int x1, int y1 );
__declspec( dllimport ) void ProcessExplorer( int Index );
__declspec( dllimport ) void ProcessExplorerDSS( int Index, DialogsSystem* DSS );
__declspec( dllimport ) void ExplorerOpenRef( int Index, char* ref );
__declspec( dllimport ) void ExplorerResize( int Index, int x, int y, int x1, int y1 );

ChatSystem CSYS;

__declspec( dllimport ) byte GetPaletteColor( int r, int g, int b );

__declspec( dllimport ) void xLine( int x, int y, int x1, int y1, byte c );

int NCHATS[3] = { 0,0,0 };
int MAXCHATS[3] = { 0,0,0 };

char** ChatMess[3] = { nullptr,nullptr,nullptr };
char** ChatSender[3] = { nullptr,nullptr,nullptr };

void AddPrimitiveChat( char* Nick, char* str,
	char** &ChatMess, char** &ChatSender,
	int &NCHATS, int &MAXCHATS )
{
	if (NCHATS >= MAXCHATS)
	{
		MAXCHATS += 32;
		ChatMess = (char**) realloc( ChatMess, 4 * MAXCHATS );
		ChatSender = (char**) realloc( ChatSender, 4 * MAXCHATS );
	}
	if (NCHATS > 512)
	{
		free( ChatMess[0] );
		free( ChatSender[0] );
		memcpy( ChatMess, ChatMess + 1, NCHATS * 4 );
		memcpy( ChatSender, ChatSender + 1, NCHATS * 4 );
		NCHATS--;
	}
	ChatMess[NCHATS] = new char[strlen( str ) + 1];
	strcpy( ChatMess[NCHATS], str );
	ChatSender[NCHATS] = new char[strlen( Nick ) + 1];
	strcpy( ChatSender[NCHATS], Nick );
	NCHATS++;
}

void AddChatString( char* Nick, char* str, int MaxLx, lpRLCFont FONT,
	char** &ChatMess, char** &ChatSender,
	int &NCHATS, int &MAXCHATS )
{
	char ACCAMULATOR[256];
	int sp = 0;
	int LL = strlen( str );
	sprintf( ACCAMULATOR, "%s: ", Nick );
	int LName = GetRLCStrWidth( ACCAMULATOR, FONT );
	if (LName == 32)LName++;
	ACCAMULATOR[0] = 0;
	do
	{
		int Lx = GetRLCStrWidth( ACCAMULATOR, FONT ) + LName;
		int Lx1 = strlen( ACCAMULATOR );
		ACCAMULATOR[Lx1] = str[sp];
		ACCAMULATOR[Lx1 + 1] = 0;
		sp++;
		if (Lx > MaxLx)
		{
			if (LName != 32)
			{
				AddPrimitiveChat( Nick, ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
				LName = 32;
				ACCAMULATOR[0] = 0;
			}
			else
			{
				AddPrimitiveChat( "", ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
				ACCAMULATOR[0] = 0;
			}
		}
	} while (sp < LL);
	if (ACCAMULATOR[0])
	{
		if (LName != 32)
		{
			AddPrimitiveChat( Nick, ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
			LName = 32;
			ACCAMULATOR[0] = 0;
		}
		else
		{
			AddPrimitiveChat( "", ACCAMULATOR, ChatMess, ChatSender, NCHATS, MAXCHATS );
			ACCAMULATOR[0] = 0;
		}
	}
}

struct TempWindow
{
	int WindX;
	int WindY;
	int WindLx;
	int WindLy;
	int WindX1;
	int WindY1;
	int ScrWidth;
};

void PushWindow( TempWindow* W )
{
	W->ScrWidth = ScrWidth;
	W->WindLx = WindLx;
	W->WindLy = WindLy;
	W->WindX = WindX;
	W->WindX1 = WindX1;
	W->WindY = WindY;
	W->WindY1 = WindY1;
}

void PopWindow( TempWindow* W )
{
	ScrWidth = W->ScrWidth;
	WindLx = W->WindLx;
	WindLy = W->WindLy;
	WindX = W->WindX;
	WindX1 = W->WindX1;
	WindY = W->WindY;
	WindY1 = W->WindY1;
}

void IntersectWindows( int x0, int y0, int x1, int y1 )
{
	if (x0 > WindX)
	{
		WindX = x0;
	}
	if (y0 > WindY)
	{
		WindY = y0;
	}
	if (x1 < WindX1)
	{
		WindX1 = x1;
	}
	if (y1 < WindY1)
	{
		WindY1 = y1;
	}
	if (WindX1 < WindX)
	{
		WindX1 = 0;
		WindX = 0;
	}
	if (WindY1 < WindY)
	{
		WindY1 = 0;
		WindY = 0;
	}
	WindLx = WindX1 - WindX + 1;
	WindLy = WindY1 - WindY + 1;
}

__declspec( dllimport ) void ShowClanString( int x, int y, char* s, byte State, RLCFont* Fn, RLCFont* Fn1, int DY );

CustomBorder* CUBM;
CustomBorder* CUBC;
bool CM_Vis = 0;
bool CC_Vis = 0;
byte RCOLOR = 0;

__declspec( dllimport ) int menu_x_off;
__declspec( dllimport ) int menu_y_off;

void Draw_PLIST( int x, int y, int Lx, int Ly, int Index, byte Active, int param )
{
	if (!RCOLOR)
	{
		RCOLOR = GetPaletteColor( 255, 0, 0 );
	}

	TempWindow TW;
	PushWindow( &TW );

	IntersectWindows( menu_x_off, menu_y_off, 976 + menu_x_off, 768 + menu_y_off );

	if (Index < CSYS.CCH[CSYS.CurChannel].NPlayers)
	{
		if (Active == 1)
		{
			CUBC->Visible = 1;
			CUBC->y = y;
			CUBC->y1 = y + 25;
			CC_Vis = 1;
		}

		if (Active == 2)
		{
			CUBM->Visible = 1;
			CUBM->y = y;
			CUBM->y1 = y + 25;
			CM_Vis = 1;
		}

		char nickname[256];
		strcpy( nickname, CSYS.CCH[CSYS.CurChannel].Players[Index].Nick );

		RLCFont* FONT = Active == 2 ? &WhiteFont : &YellowFont;
		ShowClanString( x + 6, y + 5, nickname, Active, FONT, nullptr, 0 );

		if (CSYS.CCH[CSYS.CurChannel].Players[Index].Muted)
		{
			xLine( x + 3, y + 12, x + 8 + GetRLCStrWidth( nickname, &YellowFont ), y + 12, RCOLOR );
			xLine( x + 3, y + 13, x + 8 + GetRLCStrWidth( nickname, &YellowFont ), y + 13, RCOLOR );
		}
	}
	else
	{
		Index -= CSYS.CCH[CSYS.CurChannel].NPlayers;
		if (Index < CSYS.NAbsPlayers)
		{
			RLCFont* FONT = &RedFont;
			ShowString( x + 6, y + 3, CSYS.AbsPlayers[Index].Nick, FONT );
			if (CSYS.AbsPlayers[Index].Muted)
			{
				int L = GetRLCStrWidth( CSYS.AbsPlayers[Index].Nick, &YellowFont );
				xLine( x + 3, y + 12, x + 8 + L, y + 12, RCOLOR );
				xLine( x + 3, y + 13, x + 8 + L, y + 13, RCOLOR );
			}
		}
	}
	PopWindow( &TW );
}

__declspec( dllimport ) void DrawStdBar2( int x0, int y0, int x1, int y1, int GP );

void DRAWBOX( int x, int y, int Lx, int Ly, int Idx, byte Active, int param )
{
	DrawStdBar2( x, y, x + Lx - 1, y + Ly - 1, param );
}
__declspec( dllimport ) char LASTCLICKCHATNAME[128];

bool CheckForPersonalChat( char* STR )
{
	if (STR[0] == '-'&&STR[1] == '-'&&STR[2] == '>')
	{
		char* SS = strchr( STR, ':' );
		if (SS)
		{
			int L = SS - STR - 3;
			if (L > 0)
			{
				char SS3[256];
				memcpy( SS3, STR + 3, L );
				SS3[L] = 0;
				if (CSYS.Connected)
				{
					chatSendUserMessage( CSYS.chat, SS3, SS + 1, 0 );
					char ccc[128];
					sprintf( ccc, "%s-->%s", CSYS.chatNick, SS3 );
					CSYS.Common[CSYS.CurChannel].Add( ccc, SS + 1 );
					return true;
				}
			}
		}
	}
	return false;
}

bool CheckPersonality( char* MESSAGE )
{
	if (LASTCLICKCHATNAME[0])
	{
		char CC4[512];
		CC4[0] = 0;
		if (MESSAGE[0] == '-'&&MESSAGE[1] == '-'&&MESSAGE[2] == '>')
		{
			char* SS = strchr( MESSAGE, ':' );
			if (SS)
			{
				strcpy( CC4, SS + 1 );
			}
			else
			{
				strcpy( CC4, MESSAGE );
			}
		}
		else
		{
			strcpy( CC4, MESSAGE );
		}
		sprintf( MESSAGE, "-->%s:%s", LASTCLICKCHATNAME, CC4 );
		LASTCLICKCHATNAME[0] = 0;
		return true;
	}
	return false;
}

extern __declspec( dllimport ) bool GameInProgress;

__declspec( dllimport ) void DarkScreen();
__declspec( dllimport ) void SendPings();
__declspec( dllimport ) bool EnumPlr();

void EnterPersonalMessage( char* Nick, bool Active )
{
	char nick[128];

	strcpy( nick, Nick );

	LocalGP BARS( "Interface\\bor2" );

	DarkScreen();

	//No need for offset, dialog box is centered relative to screen
	DialogsSystem DSS( 0, 0 );

	int DL = 60;

	int Y0 = RealLy / 2 - 44 + 10 + 70 + 3;
	int X0 = RealLx / 2 - 70 - DL - 80;

	DSS.addGPPicture( nullptr, X0, Y0, BARS.GPID, 14 );
	GPS.ShowGP( X0, Y0, BARS.GPID, 17, 0 );
	GP_TextButton* OK = DSS.addGP_TextButton( nullptr, X0 + 9, Y0 + 20, GetTextByID( "@OK" ), BARS.GPID, 15, &WhiteFont, &YellowFont );
	OK->OnUserClick = &MMItemChoose;
	OK->UserParam = mcmOk;

	X0 = RealLx / 2 + 70 + DL - 80;
	DSS.addGPPicture( nullptr, X0, Y0, BARS.GPID, 14 );
	GPS.ShowGP( X0, Y0, BARS.GPID, 17, 0 );
	GP_TextButton* CANCEL = DSS.addGP_TextButton( nullptr, X0 + 9, Y0 + 20, GetTextByID( "@CANCEL" ), BARS.GPID, 15, &WhiteFont, &YellowFont );
	CANCEL->OnUserClick = &MMItemChoose;
	CANCEL->UserParam = mcmCancel;

	char MESSAGE[256] = "";

	CustomBox* CBOX = DSS.addCustomBox(
		RealLx / 2 - 170 - DL,
		RealLy / 2 - 44 + 10,
		340 + DL * 2,
		80 - 10,
		&DRAWBOX );
	CBOX->param = BARS.GPID;

	InputBox* IB = DSS.addInputBox( nullptr,
		RealLx / 2 - 143 - 10 - DL,
		RealLy / 2 - 2, MESSAGE, 120,
		290 + DL * 2,
		20,
		&WhiteFont, &YellowFont );
	IB->Active = 1;

	char cc2[128];
	sprintf( cc2, GetTextByID( "ENTERMESS" ), nick );
	DSS.addTextButton( nullptr, RealLx / 2, RealLy / 2 - 40 + 10, cc2, &YellowFont, &YellowFont, &YellowFont, 1 );
	ColoredBar* CB = DSS.addColoredBar( RealLx / 2 - 150 - DL, RealLy / 2 - 4 + 1, 300 + DL * 2, 24 - 1, 135 );//107
	CB->Style = 1;

	ItemChoose = -1;
	LastKey = 0;
	KeyPressed = 0;
	do
	{
		ProcessMessages();
		StdKeys();
		DSS.MarkToDraw();
		DSS.ProcessDialogs();
		DSS.RefreshView();
		if (!Active)
		{
			EnumPlr();
			SendPings();
		};
	} while (ItemChoose == -1 && !GameInProgress);
	if (ItemChoose == mcmOk)
	{
		chatSendUserMessage( CSYS.chat, Nick, MESSAGE, 0 );
		char ccc[512];
		sprintf( ccc, "%s-->%s", CSYS.chatNick, nick );
		CSYS.Common[CSYS.CurChannel].Add( ccc, MESSAGE );
		CSYS.Private.Add( ccc, MESSAGE );
	};
	ItemChoose = -1;
}

__declspec( dllexport ) void SendPrivateMessage( char* Nick, char* MESSAGE )
{
	if (CSYS.Connected)
	{
		chatSendUserMessage( CSYS.chat, Nick, MESSAGE, 0 );
		char ccc[512];
		sprintf( ccc, "%s-->%s", CSYS.chatNick, Nick );
		CSYS.Common[CSYS.CurChannel].Add( ccc, MESSAGE );
		CSYS.Private.Add( ccc, MESSAGE );
	};
}

void ShowCentralMessage2C( char* Message, int GPIDX )
{
	int L = GetRLCStrWidth( Message, &WhiteFont );
	int DX = 0;
	DrawStdBar2(
		( RealLx - L - 64 ) / 2 - DX,
		( RealLy - 70 ) / 2 + 18,
		( RealLx + L + 64 ) / 2 - DX,
		( RealLy - 70 ) / 2 + 55,
		GPIDX );
	ShowString(
		( RealLx - L ) / 2 + 1 - DX,
		( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2 + 1,
		Message, &BlackFont );
	ShowString(
		( RealLx - L ) / 2 - DX,
		( RealLy - GetRLCHeight( YellowFont.RLC, 'W' ) ) / 2,
		Message, &YellowFont );
}

__declspec( dllimport ) void FlipPages( void );

struct RoomInfo
{
	char Name[128];
	char Nick[64];
	char RoomIP[32];
	DWORD Profile;
	char GameID[64];
	int MaxPlayers;

	//Additional members to pass data from server to main exe / CommCore
	long player_id; //Necessary for host to send udp hole punching packets
	unsigned short port; //Udp hole punching port or real port of game host
	unsigned udp_interval; //Udp hole punching packet interval
	char udp_server[16]; //IP of udp hole punching server
};

RoomInfo* GRIF = nullptr;

__declspec( dllexport ) void SetGameID( char* ID )
{
	if (GRIF)
	{
		strncpy( GRIF->GameID, ID, 63 );
		GRIF->GameID[63] = 0;
	}
}

__declspec( dllexport ) void GetGameID( char* s )
{
	s[0] = 0;
	if (GRIF)
	{
		strncpy( s, GRIF->GameID, 63 );
		s[63] = 0;
	}
}

bool NeedSmallSize = 0;

bool NeedBigSize = 0;

bool LockResize = 0;

__declspec( dllexport ) void SetChatWState( int ID, int State )
{
	switch (ID)
	{
	case 0:
		NeedSmallSize = State != 0;
		break;
	case 1:
		NeedBigSize = State != 0;
		break;
	case 2:
		LockResize = State != 0;
		break;
	}
}

void RunHTTPC()
{
	PROCESS_INFORMATION ProcessInformation;
	STARTUPINFO StartUpInfo = {};
	StartUpInfo.cb = sizeof( STARTUPINFO );
	StartUpInfo.wShowWindow = SW_SHOWMINNOACTIVE;
	StartUpInfo.dwFlags = STARTF_USESHOWWINDOW;

	CreateProcess( "httpc.exe",
		nullptr, nullptr, nullptr, false,
		0x00, nullptr, nullptr,
		&StartUpInfo,
		&ProcessInformation );
}

__declspec( dllimport ) void ClearGChat();

__declspec( dllimport ) void SetCurPtr( int v );

bool RunHomePageAnyway = 0;

__declspec( dllexport ) void GoHomeAnyway()
{
	RunHomePageAnyway = 1;
}

bool WasHere = 0;

__declspec( dllexport ) int Process_GSC_ChatWindow( bool Active, RoomInfo* RIF )
{
	GRIF = RIF;

	//Zero out additional variables in advance
	RIF->player_id = 0;
	RIF->port = 0;
	RIF->udp_interval = 0;
	memset( RIF->udp_server, 0, 16 );


	ClearGChat();

	IniFile INI;
	INI.Load( "Internet\\gscchat.ini" );

	LoadFog( INI.GetInt( "PALETTE_INDEX" ) );

	DarkScreen();

	SlowLoadPalette( INI.GetStr( "PALETTE_FILE" ) );

	SQPicture Back( "Interface\\Back_shell.bmp" );
	LocalGP SCROL( "Internet\\pix\\_Slider" );
	LocalGP BTNS( "Interface\\igame" );
	LocalGP IB2( "Internet\\pix\\i_bor2" );
	LocalGP ISB( "Internet\\pix\\is_btns" );
	LocalGP BOR2( "Interface\\bor2" );

	char CHATSERV[128] = "";
	char* Chat = ExplorerGetVar( 2, "%CHAT" );

	if (Chat)
	{
		strcpy( CHATSERV, Chat );
	}
	if (Active)
	{
		GameInProgress = 0;
	}

	if (Active && !CSYS.Connected)
	{
		RunHTTPC();

		Back.Draw( menu_x_off, menu_y_off );

		DarkScreen();

		ShowCentralMessage2C( GetTextByID( "ICCONN" ), BOR2.GPID );

		FlipPages();

		CSYS.Setup();

		char* Nick = ExplorerGetVar( 2, "%NICK" );
		char* Prof = ExplorerGetVar( 2, "%PROF" );
		char* Mail = ExplorerGetVar( 2, "%MAIL" );
		if (!Nick)
			Nick = "UNKNOWN";
		if (!Prof)
			Prof = "";
		if (!Mail)
			Mail = "";
		char PRF[64];
		sprintf( PRF, "GSCPROF_%s", Prof );
		CSYS.Disconnect();
		CSYS.ConnectToChat( Nick, PRF, Mail, CHATSERV );
		if (!CSYS.Connected)
		{
			Back.Draw( menu_x_off, menu_y_off );
			DarkScreen();
			ShowCentralMessage2C( GetTextByID( "ICUNCON" ), BOR2.GPID );
			FlipPages();
			KeyPressed = 0;
			do
			{
				ProcessMessages();
			} while (!KeyPressed);
			return false;
		}
	}
	//-------------
	int PXA[8];//={37,38,39,40,41,42,44,43};
	int PXP[8];//={37+11,38+11,39+11,40+11,41+11,42+11,44+11,43+11};
	sscanf( INI.GetStr( "PLLIST_ACTIVE_CORNERS" ), "%d%d%d%d%d%d%d%d", PXA, PXA + 1, PXA + 2, PXA + 3, PXA + 4, PXA + 5, PXA + 6, PXA + 7 );
	sscanf( INI.GetStr( "PLLIST_PASSIVE_CORNERS" ), "%d%d%d%d%d%d%d%d", PXP, PXP + 1, PXP + 2, PXP + 3, PXP + 4, PXP + 5, PXP + 6, PXP + 7 );

	DialogsSystem DSS( menu_x_off, menu_y_off );
	DSS.addPicture( nullptr, 0, 0, &Back, &Back, &Back );
	DSS.HintX = 16 + menu_x_off;
	DSS.HintY = 768 - 60 + menu_y_off;
	DSS.HintFont = &SmallWhiteFont;

	CUBM = DSS.addCustomBorder(
		INI.GetInt( "PLLIST_SELECTION_X" ),
		0,
		INI.GetInt( "PLLIST_SELECTION_X" ) + INI.GetInt( "PLLIST_SELECTION_LX" ),
		0,
		IB2.GPID, PXA, PXA, INI.GetInt( "PLLIST_ACTIVE_BGROUND" ), INI.GetInt( "PLLIST_ACTIVE_BGROUND" ) );
	CUBM->Visible = 0;
	CUBC = DSS.addCustomBorder(
		INI.GetInt( "PLLIST_SELECTION_X" ),
		0,
		INI.GetInt( "PLLIST_SELECTION_X" ) + INI.GetInt( "PLLIST_SELECTION_LX" ),
		0,
		IB2.GPID, PXP, PXP, INI.GetInt( "PLLIST_PASSIVE_BGROUND" ), INI.GetInt( "PLLIST_PASSIVE_BGROUND" ) );
	CUBC->Visible = 0;

	ChatViewer* CHVIEW[3];
	VScrollBar* CHSCR[3];

	CHVIEW[0] = DSS.addChatViewer( nullptr,
		INI.GetInt( "CHAT_WINDOW_X" ),
		INI.GetInt( "CHAT_WINDOW_Y" ),
		INI.GetInt( "CHAT_WINDOW_LINES" ),
		INI.GetInt( "CHAT_WINDOW_LINES_DIST" ),
		INI.GetInt( "CHAT_WINDOW_WIDTH" ), &ChatMess[0], &ChatSender[0], &NCHATS[0] );
	char CHATMESSAGE[256] = "";
	InputBox* CHATBOX = DSS.addInputBox( nullptr,
		INI.GetInt( "ENTER_CHAT_STRING_X" ),
		INI.GetInt( "ENTER_CHAT_STRING_Y" ),
		CHATMESSAGE, 200, INI.GetInt( "ENTER_CHAT_STRING_LX" ), INI.GetInt( "ENTER_CHAT_STRING_LY" ), &YellowFont, &WhiteFont );
	CHATBOX->Active = 1;
	CHSCR[0] = DSS.addNewGP_VScrollBar( nullptr,
		INI.GetInt( "CHAT_SCROLLER_X" ),
		INI.GetInt( "CHAT_SCROLLER_Y" ),
		INI.GetInt( "CHAT_SCROLLER_LY" ), 1, 0, SCROL.GPID, 0 );
	CHSCR[0]->Visible = 0;
	int MAXCHLX = INI.GetInt( "CHAT_WINDOW_WIDTH" );

	CHVIEW[1] = DSS.addChatViewer( nullptr,
		INI.GetInt( "CHAT_WINDOW_X" ),
		INI.GetInt( "CHAT_WINDOW_Y" ),
		INI.GetInt( "CHAT_WINDOW_LINES" ), INI.GetInt( "CHAT_WINDOW_LINES_DIST" ),
		INI.GetInt( "CHAT_WINDOW_WIDTH" ), &ChatMess[1], &ChatSender[1], &NCHATS[1] );
	CHSCR[1] = DSS.addNewGP_VScrollBar( nullptr, INI.GetInt( "CHAT_SCROLLER_X" ), INI.GetInt( "CHAT_SCROLLER_Y" ), INI.GetInt( "CHAT_SCROLLER_LY" ), 1, 0, SCROL.GPID, 0 );
	CHSCR[1]->Visible = 0;

	CHVIEW[2] = DSS.addChatViewer( nullptr,
		INI.GetInt( "CHAT_WINDOW_X" ),
		INI.GetInt( "CHAT_WINDOW_Y" ),
		INI.GetInt( "CHAT_WINDOW_LINES" ), INI.GetInt( "CHAT_WINDOW_LINES_DIST" ),
		INI.GetInt( "CHAT_WINDOW_WIDTH" ), &ChatMess[2], &ChatSender[2], &NCHATS[2] );
	CHSCR[2] = DSS.addNewGP_VScrollBar( nullptr, INI.GetInt( "CHAT_SCROLLER_X" ), INI.GetInt( "CHAT_SCROLLER_Y" ), INI.GetInt( "CHAT_SCROLLER_LY" ), 1, 0, SCROL.GPID, 0 );
	CHSCR[2]->Visible = 0;

	VScrollBar* PLVS = DSS.addNewGP_VScrollBar( nullptr,
		INI.GetInt( "PLAYERS_SCROLLER_X" ),
		INI.GetInt( "PLAYERS_SCROLLER_Y" ),
		INI.GetInt( "PLAYERS_SCROLLER_LY" ), 1, 0, SCROL.GPID, 0 );
	PLVS->ScrDy = 6 * 10;
	PLVS->OnesDy = 10;

	ComplexBox* PLLIST = DSS.addComplexBox(
		INI.GetInt( "PLAYERS_LIST_X" ),
		INI.GetInt( "PLAYERS_LIST_Y" ),
		INI.GetInt( "PLAYERS_LIST_NY" ),
		INI.GetInt( "PLAYERS_LIST_LINE_LY" ), &Draw_PLIST, -1, 0 );
	PLLIST->param = BTNS.GPID;
	PLLIST->VS = PLVS;
	PLLIST->OneLy = INI.GetInt( "PLAYERS_LIST_LINE_LY" );
	PLLIST->x1 = PLLIST->x + 150;

	GP_Button* IE_BACK = DSS.addGP_Button( nullptr,
		INI.GetInt( "EXPLORER_BACK_BUTTON_X" ),
		INI.GetInt( "EXPLORER_BACK_BUTTON_Y" ), ISB.GPID, 0, 1 );
	IE_BACK->OnUserClick = &MMItemChoose;
	IE_BACK->UserParam = 107;
	IE_BACK->Visible = 0 != INI.GetInt( "EXPLORER_BACK_BUTTON_VISIBLE" );

	GP_Button* IE_REFR = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_REFRESH_BUTTON_X" ), INI.GetInt( "EXPLORER_REFRESH_BUTTON_Y" ), ISB.GPID, 2, 3 );
	IE_REFR->OnUserClick = &MMItemChoose;
	IE_REFR->UserParam = 108;
	IE_REFR->Visible = 0 != INI.GetInt( "EXPLORER_REFRESH_BUTTON_VISIBLE" );

	GP_Button* IE_FORW = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_FORWARD_BUTTON_X" ), INI.GetInt( "EXPLORER_FORWARD_BUTTON_Y" ), ISB.GPID, 4, 5 );
	IE_FORW->OnUserClick = &MMItemChoose;
	IE_FORW->UserParam = 109;
	IE_FORW->Visible = 0 != INI.GetInt( "EXPLORER_FORWARD_BUTTON_VISIBLE" );

	GP_Button* IE_HOME = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_HOME_BUTTON_X" ), INI.GetInt( "EXPLORER_HOME_BUTTON_Y" ), ISB.GPID, 6, 7 );
	IE_HOME->OnUserClick = &MMItemChoose;
	IE_HOME->UserParam = 110;
	IE_HOME->Visible = 0 != INI.GetInt( "EXPLORER_HOME_BUTTON_VISIBLE" );
	IE_HOME->Hint = GetTextByID( "VIEW_HOMEPAGE" );

	GP_Button* IE_BTLS = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_BATTLES_BUTTON_X" ), INI.GetInt( "EXPLORER_BATTLES_BUTTON_Y" ), ISB.GPID, 8, 9 );
	IE_BTLS->OnUserClick = &MMItemChoose;
	IE_BTLS->UserParam = 111;
	IE_BTLS->Hint = GetTextByID( "INHIN3" );
	IE_BTLS->Visible = 0 != INI.GetInt( "EXPLORER_BATTLES_BUTTON_VISIBLE" );

	GP_Button* IE_T100 = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_TOP100_BUTTON_X" ), INI.GetInt( "EXPLORER_TOP100_BUTTON_Y" ), ISB.GPID, 26, 27 );
	IE_T100->OnUserClick = &MMItemChoose;
	IE_T100->UserParam = 112;
	IE_T100->Visible = 0 != INI.GetInt( "EXPLORER_TOP100_BUTTON_VISIBLE" );
	IE_T100->Hint = GetTextByID( "INHIN1" );

	GP_Button* IE_SZUP = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_RESIZE_X" ), INI.GetInt( "EXPLORER_RESIZE_Y" ), ISB.GPID, 16, 17 );
	IE_SZUP->OnUserClick = &MMItemChoose;
	IE_SZUP->UserParam = 113;
	IE_SZUP->Hint = GetTextByID( "VIEW_RESIZE" );

	GP_Button* IE_SZDN = DSS.addGP_Button( nullptr, INI.GetInt( "EXPLORER_RESIZE_X" ), INI.GetInt( "EXPLORER_RESIZE_Y" ), ISB.GPID, 18, 19 );
	IE_SZDN->OnUserClick = &MMItemChoose;
	IE_SZDN->UserParam = 114;
	IE_SZDN->Hint = GetTextByID( "VIEW_RESIZE" );

	GP_Button* CH_COMN = DSS.addGP_Button( nullptr, INI.GetInt( "CHAT_SWITCH_BUTTON_X" ), INI.GetInt( "CHAT_SWITCH_BUTTON_Y" ), ISB.GPID, 10, 11 );
	CH_COMN->OnUserClick = &MMItemChoose;
	CH_COMN->UserParam = 105;
	CH_COMN->Hint = GetTextByID( "CHAT_ALL" );
	GP_Button* CH_PSNL = DSS.addGP_Button( nullptr, INI.GetInt( "CHAT_SWITCH_BUTTON_X" ), INI.GetInt( "CHAT_SWITCH_BUTTON_Y" ), ISB.GPID, 12, 13 );
	CH_PSNL->OnUserClick = &MMItemChoose;
	CH_PSNL->UserParam = 106;
	CH_PSNL->Hint = GetTextByID( "CHAT_PERSONAL" );

	GP_Button* IE_AVER = DSS.addGP_Button( nullptr, INI.GetInt( "CHAT_VERSION_SWITCH_X" ), INI.GetInt( "CHAT_VERSION_SWITCH_Y" ), ISB.GPID, 30, 31 );
	IE_AVER->OnUserClick = &MMItemChoose;
	IE_AVER->UserParam = 103;
	IE_AVER->Hint = GetTextByID( "CHAT_COMMON" );

	GP_Button* IE_CVER = DSS.addGP_Button( nullptr, INI.GetInt( "CHAT_VERSION_SWITCH_X" ), INI.GetInt( "CHAT_VERSION_SWITCH_Y" ), ISB.GPID, 32, 33 );
	IE_CVER->OnUserClick = &MMItemChoose;
	IE_CVER->UserParam = 104;
	IE_CVER->Hint = GetTextByID( "CHAT_VERSION" );

	GP_Button* PL_INFO = DSS.addGP_Button( nullptr, INI.GetInt( "PLAYER_INFO_BUTTON_X" ), INI.GetInt( "PLAYER_INFO_BUTTON_Y" ), ISB.GPID, 22, 23 );
	PL_INFO->OnUserClick = &MMItemChoose;
	PL_INFO->UserParam = 117;
	PL_INFO->Hint = GetTextByID( "INHIN2" );

	GP_Button* PL_CHAT = DSS.addGP_Button( nullptr, INI.GetInt( "PERSONAL_CHAT_BUTTON_X" ), INI.GetInt( "PERSONAL_CHAT_BUTTON_Y" ), ISB.GPID, 24, 25 );
	PL_CHAT->OnUserClick = &MMItemChoose;
	PL_CHAT->UserParam = 100;
	PL_CHAT->Hint = GetTextByID( "INHIN4" );

	GP_Button* PL_HEAR = DSS.addGP_Button( nullptr, INI.GetInt( "PLAYER_MUTE_X" ), INI.GetInt( "PLAYER_MUTE_Y" ), ISB.GPID, 28, 29 );
	PL_HEAR->OnUserClick = &MMItemChoose;
	PL_HEAR->UserParam = 101;
	PL_HEAR->Hint = GetTextByID( "INHIN6" );

	GP_Button* PL_MUTE = DSS.addGP_Button( nullptr, INI.GetInt( "PLAYER_MUTE_X" ), INI.GetInt( "PLAYER_MUTE_Y" ), ISB.GPID, 20, 21 );
	PL_MUTE->OnUserClick = &MMItemChoose;
	PL_MUTE->UserParam = 102;
	PL_MUTE->Hint = GetTextByID( "INHIN5" );

	GP_Button* PL_JOIN = DSS.addGP_Button( nullptr, INI.GetInt( "JOIN_TO_PLAYER_X" ), INI.GetInt( "JOIN_TO_PLAYER_Y" ), ISB.GPID, 14, 15 );
	PL_JOIN->OnUserClick = &MMItemChoose;
	PL_JOIN->UserParam = 115;
	PL_JOIN->Hint = GetTextByID( "JOINPLGM" );
	PL_JOIN->Visible = 0 != INI.GetInt( "JOIN_TO_PLAYER_VISIBLE" );

	if (Active)
	{
		GP_Button* CANCEL = DSS.addGP_Button( nullptr, INI.GetInt( "CHAT_CANCEL_BUTTON_X" ), INI.GetInt( "CHAT_CANCEL_BUTTON_Y" ), ISB.GPID, INI.GetInt( "CHAT_CANCEL_BUTTON_STATE1" ), INI.GetInt( "CHAT_CANCEL_BUTTON_STATE2" ) );
		CANCEL->OnUserClick = &MMItemChoose;
		CANCEL->UserParam = mcmCancel;
		CANCEL->Hint = GetTextByID( "IG_CANCEL" );
	}
	else
	{
		GP_Button* CANCEL = DSS.addGP_Button( nullptr, INI.GetInt( "CHAT_CANCEL_BUTTON_X" ), INI.GetInt( "CHAT_CANCEL_BUTTON_Y" ), ISB.GPID, INI.GetInt( "CHAT_CANCEL_BUTTON_STATE1" ) + 2, INI.GetInt( "CHAT_CANCEL_BUTTON_STATE2" ) + 2 );
		CANCEL->OnUserClick = &MMItemChoose;
		CANCEL->UserParam = mcmCancel;
		CANCEL->Hint = GetTextByID( "IG_BACK" );
	}

	if (Active)
	{
		ResFile FH = RReset( "Internet\\Cash\\home.cmd" );
		if (FH != INVALID_HANDLE_VALUE)
		{
			int sz = RFileSize( FH );
			char* buf = (char*) malloc( sz + 1 );
			RBlockRead( FH, buf, sz );
			buf[sz] = 0;
			RunExplorer( 0, buf,
				INI.GetInt( "EXPLORER_X" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y" ) + menu_y_off,
				INI.GetInt( "EXPLORER_X1" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y1" ) + menu_y_off );

			if (RunHomePageAnyway&&WasHere)
			{
				RunHomePageAnyway = 0;
				ExplorerOpenRef( 0, buf );
			}
			free( buf );
			RClose( FH );
		}
		else
		{
			RunExplorer( 0, "LF|LW_new|open&req.txt",
				INI.GetInt( "EXPLORER_X" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y" ) + menu_y_off,
				INI.GetInt( "EXPLORER_X1" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y1" ) + menu_y_off );
		}

		if (RunHomePageAnyway && WasHere)
		{
			FH = RReset( "Internet\\Cash\\home1.cmd" );
			if (FH == INVALID_HANDLE_VALUE)
				FH = RReset( "Internet\\Cash\\home.cmd" );
			if (FH != INVALID_HANDLE_VALUE)
			{
				int sz = RFileSize( FH );
				char* buf = (char*) malloc( sz + 1 );
				RBlockRead( FH, buf, sz );
				buf[sz] = 0;
				RunHomePageAnyway = 0;
				ExplorerOpenRef( 0, buf );
				free( buf );
				RClose( FH );
			}
		}
	}

	WasHere = 1;

	bool PERSONAL = 0;
	bool newpersonal = 0;
	bool BIGSIZE = 0;
	bool FENTER = 1;
	int T0 = GetTickCount() - 10000;
	SetCurPtr( 0 );
	do
	{
		if (GetTickCount() - T0 > 2000)
		{
			char CC[32];
			sprintf( CC, "LW_gvar&%%ASTATE&%d", Active );
			ExplorerOpenRef( 0, CC );
			T0 = GetTickCount();
		}

		if (!CSYS.Connected)
		{
			Back.Draw( menu_x_off, menu_y_off );

			DarkScreen();

			ShowCentralMessage2C( GetTextByID( "ICCONN" ), BOR2.GPID );

			FlipPages();

			CSYS.Disconnect();

			CSYS.Setup();

			char* Nick = ExplorerGetVar( 2, "%NICK" );
			char* Prof = ExplorerGetVar( 2, "%PROF" );
			char* Mail = ExplorerGetVar( 2, "%MAIL" );
			if (!Nick)Nick = "UNKNOWN";
			if (!Prof)Prof = "";
			if (!Mail)Mail = "";
			char PRF[64];
			sprintf( PRF, "GSCPROF_%s", Prof );
			CSYS.ConnectToChat( Nick, PRF, Mail, CHATSERV );
			if (!CSYS.Connected)
			{
				Back.Draw( menu_x_off, menu_y_off );
				DarkScreen();
				ShowCentralMessage2C( GetTextByID( "ICUNCON" ), BOR2.GPID );
				FlipPages();
				KeyPressed = 0;
				do
				{
					ProcessMessages();
				} while (!KeyPressed);
				return false;
			}
		}

		int NCL = CHVIEW[0]->ScrNy;
		if (BIGSIZE)
		{
			IE_SZUP->Visible = 0;
			IE_SZUP->Enabled = 0;
			IE_SZDN->Visible = 1;
			IE_SZDN->Enabled = 1;
		}
		else
		{
			IE_SZUP->Visible = 1;
			IE_SZUP->Enabled = 1;
			IE_SZDN->Visible = 0;
			IE_SZDN->Enabled = 0;
		}

		if (LASTCLICKCHATNAME[0])
		{
			int N = CSYS.CCH[CSYS.CurChannel].NPlayers;
			for (int i = 0; i < N; i++)
			{
				if (!strcmp( CSYS.CCH[CSYS.CurChannel].Players[i].Nick, LASTCLICKCHATNAME ))
				{
					PLLIST->CurrentItem = i;
					PLLIST->FirstVisItem = i;
					PLVS->SPos = i * 10;
					if (PLVS->SPos > PLVS->SMaxPos)
					{
						PLVS->SPos = PLVS->SMaxPos;
					}
					PLLIST->CurrentItem = PLVS->SPos / 10;
					PLLIST->FirstVisItem = PLVS->SPos / 10;
				}
			}
		}

		if (CheckPersonality( CHATMESSAGE ))
		{
			CHATBOX->CursPos = strlen( CHATMESSAGE );
		}

		if (CSYS.CurChannel == 0)
		{
			IE_CVER->Visible = 1;
			IE_CVER->Enabled = 1;
			IE_AVER->Visible = 0;
			IE_AVER->Enabled = 0;
		}
		else
		{
			IE_CVER->Visible = 0;
			IE_CVER->Enabled = 0;
			IE_AVER->Visible = 1;
			IE_AVER->Enabled = 1;
		}

		if (PERSONAL)
		{
			CH_PSNL->Visible = 0;
			CH_PSNL->Enabled = 0;
			CH_COMN->Visible = 1;
			CH_COMN->Enabled = 1;
			CHVIEW[0]->Visible = 1;
			CHVIEW[0]->Enabled = 1;
			CHVIEW[1]->Visible = 0;
			CHVIEW[1]->Enabled = 0;
			CHVIEW[2]->Visible = 0;
			CHVIEW[2]->Enabled = 0;
			newpersonal = 0;
		}
		else
		{
			CH_PSNL->Visible = 1;
			CH_PSNL->Enabled = 1;
			CH_COMN->Visible = 0;
			CH_COMN->Enabled = 0;
			CHVIEW[0]->Visible = 0;
			CHVIEW[0]->Enabled = 0;
			CHVIEW[1]->Visible = CSYS.CurChannel == 0;
			CHVIEW[1]->Enabled = CSYS.CurChannel == 0;
			CHVIEW[2]->Visible = CSYS.CurChannel == 1;
			CHVIEW[2]->Enabled = CSYS.CurChannel == 1;
			if (newpersonal)
			{
				CH_PSNL->PassiveFrame = 12 + ( ( GetTickCount() / 600 ) & 1 );
			}
			else CH_PSNL->PassiveFrame = 13;
		}

		for (int i = 0; i < 3; i++)
		{
			CHSCR[i]->Visible = 0;
			CHSCR[i]->Enabled = 0;
		}

		CC_Vis = 0;
		CM_Vis = 0;

		char NICK[64];
		char MESS[512];

		bool adc = 0;

		while (CSYS.Common[CSYS.CurChannel].RemoveOne( NICK, MESS, 512 ))
		{
			AddChatString( NICK, MESS, MAXCHLX, &YellowFont, ChatMess[CSYS.CurChannel + 1], ChatSender[CSYS.CurChannel + 1], NCHATS[CSYS.CurChannel + 1], MAXCHATS[CSYS.CurChannel + 1] );
			adc = 1;
		}

		if (NCHATS[CSYS.CurChannel + 1] > NCL)
		{
			CHSCR[CSYS.CurChannel + 1]->SMaxPos = NCHATS[CSYS.CurChannel + 1] - NCL;
			if (adc)
			{
				CHSCR[CSYS.CurChannel + 1]->SPos = CHSCR[CSYS.CurChannel + 1]->SMaxPos;
			}
			CHSCR[CSYS.CurChannel + 1]->Visible = CHVIEW[CSYS.CurChannel + 1]->Visible;
			CHSCR[CSYS.CurChannel + 1]->Enabled = CHVIEW[CSYS.CurChannel + 1]->Enabled;
			CHVIEW[CSYS.CurChannel + 1]->ChatDY = CHSCR[CSYS.CurChannel + 1]->SPos;
		}
		else
		{
			CHSCR[CSYS.CurChannel + 1]->Visible = 0;
			CHSCR[CSYS.CurChannel + 1]->SPos = 0;
			CHVIEW[CSYS.CurChannel + 1]->ChatDY = 0;
		}

		if (FENTER)
		{
			for (int i = 0; i < 3; i++)
			{
				CHSCR[i]->SPos = CHSCR[i]->SMaxPos;
			}
			FENTER = 0;
		}

		adc = 0;
		if (CSYS.Private.NMsg)
		{
			newpersonal = 1;
			while (CSYS.Private.RemoveOne( NICK, MESS, 512 ))
			{
				AddChatString( NICK, MESS, MAXCHLX, &YellowFont, ChatMess[0], ChatSender[0], NCHATS[0], MAXCHATS[0] );
				adc = 1;
			}
		}

		if (NCHATS[0] > NCL)
		{
			CHSCR[0]->SMaxPos = NCHATS[0] - NCL;
			if (adc)CHSCR[0]->SPos = CHSCR[0]->SMaxPos;
			CHSCR[0]->Visible = CHVIEW[0]->Visible;
			CHSCR[0]->Enabled = CHVIEW[0]->Enabled;
			CHVIEW[0]->ChatDY = CHSCR[0]->SPos;
		}
		else
		{
			CHSCR[0]->Visible = 0;
			CHSCR[0]->SPos = 0;
			CHVIEW[0]->ChatDY = 0;
		}

		if (LastKey == 13)
		{
			if (CHATMESSAGE[0])
			{
				if (!CheckForPersonalChat( CHATMESSAGE ))
				{
					chatSendChannelMessage( CSYS.chat, CSYS.chatChannel[CSYS.CurChannel], CHATMESSAGE, 0 );
				}
				CHATMESSAGE[0] = 0;
			}
			LastKey = 0;
		}

		PLLIST->N = CSYS.NAbsPlayers + CSYS.CCH[CSYS.CurChannel].NPlayers;
		DSS.ProcessDialogs();
		ProcessExplorerDSS( 0, &DSS );
		GRIF = RIF;
		DSS.RefreshView();
		CUBC->Visible = CC_Vis;
		CUBM->Visible = CM_Vis;
		char CURRPL[128];
		char CURRPR[128];
		char CURRML[128];
		int p = PLLIST->CurrentItem;
		CURRPL[0] = 0;
		CURRPR[0] = 0;
		CURRML[0] = 0;
		if (p >= 0 && p < CSYS.CCH[CSYS.CurChannel].NPlayers)
		{
			strcpy( CURRPL, CSYS.CCH[CSYS.CurChannel].Players[p].Nick );
			strcpy( CURRPR, CSYS.CCH[CSYS.CurChannel].Players[p].Info );
			strcpy( CURRML, CSYS.CCH[CSYS.CurChannel].Players[p].mail );

			if (CSYS.CCH[CSYS.CurChannel].Players[p].Muted)
			{
				PL_MUTE->Visible = 0;
				PL_MUTE->Enabled = 0;
				PL_HEAR->Visible = 1;
				PL_HEAR->Enabled = 1;
			}
			else
			{
				PL_MUTE->Visible = 1;
				PL_MUTE->Enabled = 1;
				PL_HEAR->Visible = 0;
				PL_HEAR->Enabled = 0;
			}
		}
		else
		{
			p -= CSYS.CCH[CSYS.CurChannel].NPlayers;
			if (p >= 0 && p < CSYS.NAbsPlayers)
			{
				strcpy( CURRPL, CSYS.AbsPlayers[p].Nick );
				strcpy( CURRPR, CSYS.AbsPlayers[p].Info );
				strcpy( CURRML, CSYS.AbsPlayers[p].mail );
				if (CSYS.AbsPlayers[p].Muted)
				{
					PL_MUTE->Visible = 0;
					PL_MUTE->Enabled = 0;
					PL_HEAR->Visible = 1;
					PL_HEAR->Enabled = 1;
				}
				else
				{
					PL_MUTE->Visible = 1;
					PL_MUTE->Enabled = 1;
					PL_HEAR->Visible = 0;
					PL_HEAR->Enabled = 0;
				}
			}
		}

		CSYS.Process();

		ExplorerSetVar( 0, "%CP_NICK", CURRPL );
		ExplorerSetVar( 0, "%CP_PROF", CURRPL );
		ExplorerSetVar( 0, "%CP_MAIL", CURRPL );
		ExplorerSetVar( 0, "%CV_LOBBY", LobbyVersion );

		char vv[32];

		sprintf( vv, "%d", dwVersion );

		ExplorerSetVar( 0, "%CV_ENGINE", vv );

		if (CURRPL[0])
		{
			for (int i = 0; i < CSYS.CCH[CSYS.CurChannel].NPlayers; i++)
			{
				if (!strcmp( CSYS.CCH[CSYS.CurChannel].Players[i].Nick, CURRPL ))
				{
					PLLIST->CurrentItem = i;
				}
			}
			for (int i = 0; i < CSYS.NAbsPlayers; i++)
			{
				if (!strcmp( CSYS.AbsPlayers[i].Nick, CURRPL ))
				{
					PLLIST->CurrentItem = i + CSYS.CCH[CSYS.CurChannel].NPlayers;
				}
			}
		}

		if (ItemChoose == 100)
		{//Personal message
			ItemChoose = -1;
			char NICK_local[64];
			NICK_local[0] = 0;
			int p_local = PLLIST->CurrentItem;
			if (p_local < CSYS.CCH[CSYS.CurChannel].NPlayers)
			{
				strcpy( NICK_local, CSYS.CCH[CSYS.CurChannel].Players[p_local].Nick );
			}
			else
			{
				if (p_local < CSYS.CCH[CSYS.CurChannel].NPlayers + CSYS.NAbsPlayers)
				{
					p_local -= CSYS.CCH[CSYS.CurChannel].NPlayers;
					strcpy( NICK_local, CSYS.AbsPlayers[p_local].Nick );
				}
			}
			if (NICK_local[0])
			{
				EnterPersonalMessage( NICK_local, Active );
			}
		}

		if (ItemChoose == 101 || ItemChoose == 102)
		{
			p = PLLIST->CurrentItem;
			if (p >= 0 && p < CSYS.CCH[CSYS.CurChannel].NPlayers)
			{
				CSYS.CCH[CSYS.CurChannel].Players[p].Muted = ItemChoose == 102;
			}
			else
			{
				p -= CSYS.CCH[CSYS.CurChannel].NPlayers;
				if (p >= 0 && p < CSYS.NAbsPlayers)
				{
					CSYS.AbsPlayers[p].Muted = ItemChoose == 102;
				};
			};
			ItemChoose = -1;
		};
		if (ItemChoose == 103 || ItemChoose == 104)
		{
			CSYS.CurChannel = ItemChoose == 104;
			ItemChoose = -1;
		};
		if (ItemChoose == 105 || ItemChoose == 106)
		{
			PERSONAL = ItemChoose == 106;
			ItemChoose = -1;
		};
		if (ItemChoose == 107)
		{
			ExplorerBack( 0 );
			ItemChoose = -1;
		};
		if (ItemChoose == 108)
		{
			ExplorerRefresh( 0 );
			ItemChoose = -1;
		};
		if (ItemChoose == 110)
		{
			//home
			ResFile F = RReset( "Internet\\Cash\\home.cmd" );
			if (F != INVALID_HANDLE_VALUE)
			{
				int sz = RFileSize( F );
				char* buf = (char*) malloc( sz + 1 );
				RBlockRead( F, buf, sz );
				buf[sz] = 0;
				ExplorerOpenRef( 0, buf );
				free( buf );
				RClose( F );
			};
			ItemChoose = -1;
		};
		if (ItemChoose == 111)
		{
			//battles
			ResFile F = RReset( "Internet\\Cash\\rooms.cmd" );
			if (F != INVALID_HANDLE_VALUE)
			{
				int sz = RFileSize( F );
				char* buf = (char*) malloc( sz + 1 );
				RBlockRead( F, buf, sz );
				buf[sz] = 0;
				ExplorerOpenRef( 0, buf );
				free( buf );
				RClose( F );
			};
			ItemChoose = -1;
		};
		if (ItemChoose == 112)
		{
			//top 100
			ResFile F = RReset( "Internet\\Cash\\t100.cmd" );
			if (F != INVALID_HANDLE_VALUE)
			{
				int sz = RFileSize( F );
				char* buf = (char*) malloc( sz + 1 );
				RBlockRead( F, buf, sz );
				buf[sz] = 0;
				ExplorerOpenRef( 0, buf );
				free( buf );
				RClose( F );
			};
			ItemChoose = -1;
		};
		if (ItemChoose == 115)
		{
			//join to player

			if (CURRPR[0] && strstr( CURRPR, "~GSCPROF_" ))
			{
				ResFile F = RReset( "Internet\\Cash\\join_pl.cmd" );
				if (F != INVALID_HANDLE_VALUE)
				{
					int sz = RFileSize( F );
					char* buf = (char*) malloc( sz + 1 );
					RBlockRead( F, buf, sz );
					buf[sz] = 0;
					char ccc[512];
					sprintf( ccc, buf, CURRPR + 9 );
					ExplorerOpenRef( 0, ccc );
					free( buf );
					RClose( F );
				};
			};
			ItemChoose = -1;
		};
		if (ItemChoose == 117)
		{
			//player info 
			if (CURRPR[0] && strstr( CURRPR, "~GSCPROF_" ))
			{
				ResFile F = RReset( "Internet\\Cash\\pl_info.cmd" );
				if (F != INVALID_HANDLE_VALUE)
				{
					int sz = RFileSize( F );
					char* buf = (char*) malloc( sz + 1 );
					RBlockRead( F, buf, sz );
					buf[sz] = 0;
					char ccc[512];
					sprintf( ccc, buf, CURRPR + 9 );
					ExplorerOpenRef( 0, ccc );
					free( buf );
					RClose( F );
				};
			};
			ItemChoose = -1;
		}

		//Resize game list
		if (( ItemChoose == 113 || ItemChoose == 114 ) && LockResize)
			ItemChoose = -1;
		if (BIGSIZE && NeedSmallSize)
			ItemChoose = 114;
		if (( !BIGSIZE ) && NeedBigSize)
			ItemChoose = 113;
		if (ItemChoose == 113)
		{
			//enlarge
			int DLY = INI.GetInt( "EXPLORER_DLY" );
			Back.LoadPicture( "Interface\\Back_Shell_02.bmp" );
			ExplorerResize( 0,
				INI.GetInt( "EXPLORER_X" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y" ) + menu_y_off,
				INI.GetInt( "EXPLORER_X1" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y1" ) + menu_y_off + DLY );
			BIGSIZE = 1;
			ItemChoose = -1;
			for (int i = 0; i < 3; i++)
			{
				CHVIEW[i]->y = CHVIEW[i]->y + DLY;
				CHSCR[i]->y = CHSCR[i]->y + DLY;
				CHVIEW[i]->ScrNy = 1;
				CHSCR[i]->LineLy -= DLY;

				NCL = 1;
				CHSCR[i]->SMaxPos = NCHATS[i] - NCL;
				CHSCR[i]->SPos = CHSCR[i]->SMaxPos;
				CHSCR[i]->Visible = CHVIEW[i]->Visible;
				CHSCR[i]->Enabled = CHVIEW[i]->Enabled;
				CHVIEW[i]->ChatDY = CHSCR[i]->SPos;
			};
		};
		if (ItemChoose == 114)
		{
			//minimize
			int DLY = INI.GetInt( "EXPLORER_DLY" );
			Back.LoadPicture( "Interface\\Back_Shell.bmp" );
			ExplorerResize( 0,
				INI.GetInt( "EXPLORER_X" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y" ) + menu_y_off,
				INI.GetInt( "EXPLORER_X1" ) + menu_x_off,
				INI.GetInt( "EXPLORER_Y1" ) + menu_y_off );
			BIGSIZE = 0;
			for (int i = 0; i < 3; i++)
			{
				CHVIEW[i]->y = CHVIEW[i]->y - DLY;
				CHSCR[i]->y = CHSCR[i]->y - DLY;
				CHVIEW[i]->ScrNy = INI.GetInt( "CHAT_WINDOW_LINES" );
				CHSCR[i]->LineLy += DLY;

				NCL = INI.GetInt( "CHAT_WINDOW_LINES" );
				CHSCR[i]->SMaxPos = NCHATS[i] - NCL;
				CHSCR[i]->SPos = CHSCR[i]->SMaxPos;
				CHSCR[i]->Visible = CHVIEW[i]->Visible;
				CHSCR[i]->Enabled = CHVIEW[i]->Enabled;
				CHVIEW[i]->ChatDY = CHSCR[i]->SPos;
			};
			ItemChoose = -1;
		};

		char* VAR = ExplorerGetVar( 0, "%COMMAND" );
		if (VAR)
		{
			if (!strcmp( VAR, "CGAME" ))
			{
				char* GID = ExplorerGetVar( 0, "%CG_GAMEID" );
				if (GID)
				{
					char* GAMENAME = ExplorerGetVar( 0, "%CG_GAMENAME" );
					if (GAMENAME)
					{
						char* MaxPL = ExplorerGetVar( 0, "%CG_MAXPL" );
						if (MaxPL)
						{
							int mp = atoi( MaxPL );
							if (mp < 2)mp = 2;
							if (mp > 7)mp = 7;
							RIF->MaxPlayers = mp;
							strncpy( RIF->GameID, GID, 63 );
							RIF->GameID[63] = 0;
							strncpy( RIF->Name, GAMENAME, 127 );
							RIF->Name[127] = 0;
							strcpy( RIF->Nick, CSYS.chatNick );
							RIF->Profile = atoi( CSYS.chatUser + 8 );
							VAR[0] = 0;
							char* PlName_local = ExplorerGetVar( 0, "%CG_PLNAME" );
							if (PlName_local)
							{
								strcpy( RIF->Nick, PlName_local );
							}

							//Variables for udp hole punching
							//Store them inside RoomInfo so main exe can pick them up

							//Player's ID
							const char *player_id_str = ExplorerGetVar( 0, "%PROF" );
							if (player_id_str)
							{
								char *end;
								const long player_id = std::strtol( player_id_str, &end, 10 );
								if (player_id && '\0' == end[0])
								{//Conversion successful, string countained one number only
									RIF->player_id = player_id;
								}
							}

							//Server port
							const char *hole_port_str = ExplorerGetVar( 0, "%CG_HOLEPORT" );
							if (hole_port_str)
							{
								char *end;
								const unsigned long port = std::strtoul( hole_port_str, &end, 10 );
								if (port && '\0' == end[0])
								{//Conversion successful, string countained one number only
									RIF->port = (unsigned short) port;
								}
							}
							//Server ip
							const char *hole_host_str = ExplorerGetVar( 0, "%CG_HOLEHOST" );
							if (hole_host_str)
							{
								std::strcpy( RIF->udp_server, hole_host_str );
								RIF->udp_server[15] = 0;
							}
							//Interval for packets
							const char *hole_interval_str = ExplorerGetVar( 0, "%CG_HOLEINT" );
							if (hole_interval_str)
							{
								char *end;
								const long interval = std::strtol( hole_interval_str, &end, 10 );
								if (interval && '\0' == end[0])
								{//Conversion successful, string countained one number only
									RIF->udp_interval = interval;
								}
							}

							return 1;
						}
					}
				}
			}//CGAME end
			else
			{
				if (!strcmp( VAR, "JGAME" ))
				{
					char* GID = ExplorerGetVar( 0, "%CG_GAMEID" );
					if (GID)
					{
						char* GAMENAME = ExplorerGetVar( 0, "%CG_GAMENAME" );
						if (GAMENAME)
						{
							char* MaxPL = ExplorerGetVar( 0, "%CG_MAXPL" );
							if (MaxPL)
							{
								char* GAMEIP = ExplorerGetVar( 0, "%CG_IP" );
								if (GAMEIP)
								{
									strcpy( RIF->RoomIP, GAMEIP );
									int mp = atoi( MaxPL );
									if (mp < 2)mp = 2;
									if (mp > 7)mp = 7;
									RIF->MaxPlayers = mp;
									strncpy( RIF->GameID, GID, 63 );
									RIF->GameID[63] = 0;
									strncpy( RIF->Name, GAMENAME, 127 );
									RIF->Name[127] = 0;
									strcpy( RIF->Nick, CSYS.chatNick );
									RIF->Profile = atoi( CSYS.chatUser + 8 );
									VAR[0] = 0;
									char* PlName_local = ExplorerGetVar( 0, "%CG_PLNAME" );
									if (PlName_local)
									{
										strcpy( RIF->Nick, PlName_local );
									}

									const char *port_str = ExplorerGetVar( 0, "%CG_PORT" );
									if (port_str)
									{
										char *end;
										const unsigned long port = std::strtoul( port_str, &end, 10 );
										if (port && '\0' == end[0])
										{//Conversion successful, string countained one number only
											RIF->port = (unsigned short) port;
										}
									}

									return 2;
								}
							}
						}
					}
				}//JGAME end
			}//VAR end
			VAR[0] = 0;
		}

		//if(GameInProgress&&!Active)ItemChoose=mcmCancel;
		if (!Active)
		{
			EnumPlr();
			SendPings();
		}
	} while (ItemChoose != mcmCancel && !GameInProgress);

	if (Active)
	{
		CSYS.Disconnect();
	}
	return false;
}

bool InRoom = 0;

__declspec( dllexport ) void LeaveGSCRoom()
{
	if (GRIF)
	{
		InRoom = 0;
		ExplorerOpenRef( 0, "GW|leave" );
		//GRIF=nullptr;
	}
}

__declspec( dllimport ) bool CheckUsingAI();

__declspec( dllexport ) void StartGSCGame( char* Options, char* Map,
	int NPlayers, int* Profiles, char** Nations, int* Teams, int* Colors )
{
	if (GRIF)
	{
		char PREID[128] = "";
		if (strstr( Map, ".sav" ) || strstr( Map, ".SAV" ))
		{
			ResFile F = RReset( Map );
			if (F != INVALID_HANDLE_VALUE)
			{
				RSeek( F, RFileSize( F ) - 64 );
				RBlockRead( F, PREID, 64 );
				RClose( F );
			};
		};
		char cc[2048];
		ParsedRQ P1;
		strcpy( P1.DevName, "GW" );
		P1.AddComm( "start" );
		//P1.AddParam(Options,strlen(Options)+1);
		cc[0] = 0;
		if (PREID[0])
		{
			sprintf( cc, "sav:[%s]", PREID );
		};
		if (CheckUsingAI())
		{
			strcat( cc, "<AI>" );
		};
		P1.AddParam( cc, strlen( cc ) + 1 );
		P1.AddParam( Map, strlen( Map ) + 1 );
		P1.AddIntParam( NPlayers );
		for (int i = 0; i < NPlayers; i++)
		{
			P1.AddIntParam( Profiles[i] );
			P1.AddParam( Nations[i], strlen( Nations[i] ) + 1 );
			P1.AddIntParam( Teams[i] );
			P1.AddIntParam( Colors[i] );
		};

		memset( cc, 0, sizeof cc );
		P1.UnParse( cc, 2048 );
		ExplorerOpenRef( 0, cc );
		CSYS.Disconnect();
	}
}

struct OnePlayerReport
{
	DWORD Profile;
	byte State;
	word Score;
	word Population;
	DWORD ReachRes[6];
	word NBornP;
	word NBornUnits;
};

__declspec( dllexport ) void ReportGSCGame( int time, int NPlayers, OnePlayerReport* OPR )
{

	if (!GRIF)
	{
		return;
	}

	if (!GRIF->GameID[0])
	{
		return;
	}

	ParsedRQ P1;
	byte DATA[4096];
	char REQ[4096];
	strcpy( P1.DevName, "GW" );
	P1.AddComm( "stats" );
	memcpy( DATA, &time, 4 );
	DATA[4] = (byte) NPlayers;
	memcpy( DATA + 5, OPR, sizeof OnePlayerReport );
	P1.AddParam( (char*) DATA, 5 + sizeof OnePlayerReport );
	P1.AddParam( GRIF->GameID, strlen( GRIF->GameID ) + 1 );
	P1.UnParse( REQ, 4096 );
	ExplorerOpenRef( 0, REQ );
};
__declspec( dllexport )
void ReportAliveState( int NPlayers, int* Profiles )
{
	ParsedRQ P1;
	byte DATA[4096];
	char REQ[4096];
	strcpy( P1.DevName, "GW" );
	P1.AddComm( "alive" );
	memcpy( DATA, &NPlayers, 4 );
	memcpy( DATA + 4, Profiles, NPlayers << 2 );
	P1.AddParam( (char*) DATA, 4 + ( NPlayers << 2 ) );
	P1.UnParse( REQ, 4096 );
	ExplorerOpenRef( 0, REQ );
};
int LastSVS_Time = 0;
int LastPID;
byte LastState;
__declspec( dllexport )

void SendVictoryState( int PlayerID, byte State )
{
	int T = GetTickCount();
	if (LastSVS_Time && T - LastSVS_Time < 3000 && LastPID == PlayerID && State == LastState)
	{
		return;
	}

	LastSVS_Time = T;
	LastPID = PlayerID;
	LastState = State;


	if (!GRIF)
	{
		return;
	}

	ParsedRQ P1;
	char REQ[4096];
	strcpy( P1.DevName, "GW" );
	P1.AddComm( "endgame" );
	P1.AddParam( GRIF->GameID, strlen( GRIF->GameID ) + 1 );
	P1.AddIntParam( PlayerID );
	P1.AddIntParam( State );
	P1.UnParse( REQ, 4096 );
	ExplorerOpenRef( 0, REQ );
}