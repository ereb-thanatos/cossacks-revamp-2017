#include "../Main executable/common.h"
#include "IntExplorer.h"
#include "ParseRQ.h"
#include <crtdbg.h>
#include <assert.h>
#include "GameOnMap.h"
#include <sys\utime.h>
#include <process.h>
#include <stdarg.h>
__declspec( dllimport ) void SetCurPtr( int v );
extern __declspec( dllimport ) bool GameInProgress;
bool LOGMODE = 0;
void ReportIt( char* s, ... )
{
	if (!LOGMODE)return;
	SYSTEMTIME ST;
	GetSystemTime( &ST );
	FILE* F = fopen( "s.log", "a" );
	if (F)
	{
		char ach[16384];
		sprintf( ach, "%d:%d %dms                               ", ST.wMinute, ST.wSecond, ST.wMilliseconds );
		va_list va;
		va_start( va, s );
		vsprintf( ach + 18, s, va );
		va_end( va );
		fprintf( F, "%s", ach );
		fclose( F );
	};
};
//data Xcange format:
//request to server:
//dev_name|command1&parm1&parm2&... |command2 parm1 ...
//respond from server:
//
struct OneRequest
{
	DWORD DevIndex;
	DWORD Handle;
	DWORD IHandle;
	DWORD Opt;
};
struct OneSXP_Device
{
	char Name[32];
	fnSendRequest* SendRequest;
	fnGetRequestResult* GetRequestResult;
	fnCloseRequest* CloseRequest;
	fnProcess* Process;
	fnCloseAll* CloseAll;
	HMODULE H;
};
class SXP_DevScope
{
public:
	OneRequest* REQ;
	int NReq;
	int MaxReq;
	DWORD CurReq;

	OneSXP_Device* DEVS;
	int NDEVS;
	SXP_DevScope();
	~SXP_DevScope();

	DWORD SendRequest( sicExplorer* SXP, char* request, bool AllowNewWindow, bool Auto );
	DWORD SendRequest( sicExplorer* SXP, ParsedRQ* P1, bool AllowNewWindow, bool Auto );

	DWORD GetRequestResult( DWORD Handle, ParsedRQ* Result );
	void  CloseRequest( DWORD Handle );
	void  Process();
	void  CloseAll();
	void  Shutdown();
	void  RegisterDevice( char* ID, char* DLL_Path );
};
SXP_DevScope::SXP_DevScope()
{
	memset( this, 0, sizeof *this );
	CurReq = 1;
};
SXP_DevScope::~SXP_DevScope()
{
	Shutdown();
};

void FilterRQ2Send( sicExplorer* SXP, ParsedRQ* RQ, bool AllowNew );

DWORD SXP_DevScope::SendRequest( sicExplorer* SXP, char* request, bool AllowNewWindow, bool Auto )
{
	ParsedRQ P1;
	P1.Parse( request );
	if (LOGMODE)
	{
		//---------DEBUG----------
		char CCC[16384];
		P1.UnParse( CCC, 16384 );
		ReportIt( "%s\n", CCC );
		//------------------------
	}
	return SendRequest( SXP, &P1, AllowNewWindow, Auto );
}

DWORD SXP_DevScope::SendRequest( sicExplorer* SXP, ParsedRQ* P1, bool AllowNewWindow, bool Auto )
{
	FilterRQ2Send( SXP, P1, AllowNewWindow );
	for (int i = 0; i < P1->NComm; i++)if (!P1->Comm[i].ComID[0])
	{
		P1->DelComm( i );
		i--;
	};
	for (int i = 0; i < NDEVS; i++)if (!strcmp( DEVS[i].Name, P1->DevName ))
	{
		int sz = P1->Compact( nullptr, 0 );
		char* data = (char*) malloc( sz );
		P1->Compact( data, sz );
		if (DEVS[i].SendRequest)
		{
			DWORD H = DEVS[i].SendRequest( data, sz );
			free( data );
			if (NReq >= MaxReq)
			{
				MaxReq += 64;
				REQ = (OneRequest*) realloc( REQ, MaxReq * sizeof OneRequest );
			};
			REQ[NReq].DevIndex = i;
			REQ[NReq].Handle = H;
			REQ[NReq].IHandle = CurReq;
			REQ[NReq].Opt = Auto;
			CurReq++;
			NReq++;
			return CurReq - 1;
		}
		else free( data );
	};
	return 0;
};
DWORD SXP_DevScope::GetRequestResult( DWORD Handle, ParsedRQ* Result )
{
	Result->Clear();
	for (int i = 0; i < NReq; i++)if (Handle == REQ[i].IHandle)
	{
		if (DEVS[REQ[i].DevIndex].GetRequestResult)
		{
			char* res;
			int sz;
			DWORD R = DEVS[REQ[i].DevIndex].GetRequestResult( REQ[i].Handle, &res, &sz );
			if (R == 128)
			{//success
				Result->Extract( res, sz );
				return 128;
			}
			else return R;
		};
	};
	return 1;
};
void  SXP_DevScope::CloseRequest( DWORD Handle )
{
	for (int i = 0; i < NReq; i++)if (Handle == REQ[i].IHandle)
	{
		if (DEVS[REQ[i].DevIndex].CloseRequest)
		{
			DEVS[REQ[i].DevIndex].CloseRequest( REQ[i].Handle );
		};
		if (i < NReq - 1)
		{
			memcpy( REQ + i, REQ + i + 1, ( NReq - i - 1 ) * sizeof OneRequest );
		};
		NReq--;
	};
};
void  SXP_DevScope::Process()
{
	for (int i = 0; i < NDEVS; i++)if (DEVS[i].Process)DEVS[i].Process();
};
void  SXP_DevScope::CloseAll()
{
	for (int i = 0; i < NDEVS; i++)if (DEVS[i].CloseAll)DEVS[i].CloseAll();
};
void  SXP_DevScope::Shutdown()
{
	for (int i = 0; i < NDEVS; i++)
	{
		FreeLibrary( DEVS[i].H );
	};
	if (DEVS)free( DEVS );
	int idx = CurReq;
	memset( this, 0, sizeof *this );
	CurReq = idx;
};
void SXP_DevScope::RegisterDevice( char* ID, char* DLL_Path )
{
	HMODULE H = LoadLibrary( DLL_Path );
	if (H)
	{
		DEVS = (OneSXP_Device*) realloc( DEVS, ( NDEVS + 1 ) * sizeof OneSXP_Device );
		char STR[64];
		sprintf( STR, "?%s_SendRequest@@YAKPADH@Z", ID );
		DEVS[NDEVS].SendRequest = (fnSendRequest*) GetProcAddress( H, STR );
		sprintf( STR, "?%s_GetRequestResult@@YAKKPAPADPAH@Z", ID );
		DEVS[NDEVS].GetRequestResult = (fnGetRequestResult*) GetProcAddress( H, STR );
		sprintf( STR, "?%s_Process@@YAXXZ", ID );
		DEVS[NDEVS].Process = (fnProcess*) GetProcAddress( H, STR );
		sprintf( STR, "?%s_CloseAll@@YAXXZ", ID );
		DEVS[NDEVS].CloseAll = (fnCloseAll*) GetProcAddress( H, STR );
		sprintf( STR, "?%s_CloseRequest@@YAXK@Z", ID );
		DEVS[NDEVS].CloseRequest = (fnCloseRequest*) GetProcAddress( H, STR );
		DEVS[NDEVS].H = H;
		strcpy( DEVS[NDEVS].Name, ID );
		NDEVS++;
	};
}

SXP_DevScope DEVSCOPE;

void InitDevs()
{
	DEVSCOPE.RegisterDevice( "LF", "LF_Server.DLL" );
	DEVSCOPE.RegisterDevice( "DS", "DipServer.DLL" );
	DEVSCOPE.RegisterDevice( "GW", "GW_Server.DLL" );
}

DWORD SendGlobalRequest( sicExplorer* SXP, char* data, bool allow )
{
	return DEVSCOPE.SendRequest( SXP, data, allow, 1 );
}


extern sicExplorer SXP[8];

int LoadDefaults( byte** Data )
{
	ResFile RF = RReset( "Internet\\Cash\\default.cml" );
	if (RF != INVALID_HANDLE_VALUE)
	{
		int sz = RFileSize( RF );
		if (sz)
		{
			*Data = (byte*) malloc( sz );
			RBlockRead( RF, *Data, sz );
		}
		else
		{
			*Data = nullptr;
		};
		RClose( RF );
		return sz;
	};
	return 0;
};
char* SaveCurrentResult( OneSicWindow* OSW, char* dest )
{
	if (OSW->Result)
	{
		sprintf( dest, "Internet/Cash/tempcml_%X.cml", GetTickCount() );
		ResFile F = RRewrite( dest );
		if (F != INVALID_HANDLE_VALUE)
		{
			RBlockWrite( F, OSW->Result, strlen( OSW->Result ) );
			RClose( F );
			return dest;
		};
	};
	return nullptr;
};
char* SaveCurrentResultTo( OneSicWindow* OSW, char* dest )
{
	if (OSW->Result)
	{
		ResFile F = RRewrite( dest );
		if (F != INVALID_HANDLE_VALUE)
		{
			RBlockWrite( F, OSW->Result, strlen( OSW->Result ) );
			RClose( F );
			return dest;
		};
	};
	return nullptr;
};
void EraseTempFiles()
{
	WIN32_FIND_DATA FD;
	HANDLE H = FindFirstFile( "Internet\\Cash\\tempcml_*.cml", &FD );
	if (H&&H != INVALID_HANDLE_VALUE)
	{
		do
		{
			char cc[64];
			sprintf( cc, "Internet\\Cash\\%s", FD.cFileName );
			DeleteFile( cc );
		} while (FindNextFile( H, &FD ));
		FindClose( H );
	};
};
void exec_LW_time( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				OSW->TimeLimit[0] = GetTickCount() + atoi( par[0] );
				strncpy( OSW->TimeLimitRequest[0], par[1], 159 );
				OSW->TimeLimitRequest[0][159] = 0;
			};
		};
	};
};
void exec_LW_move( int Np, char** par, int* size )
{
	if (Np < 3)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				int dx = atoi( par[0] );
				int dy = atoi( par[1] );
				int ds = atoi( par[2] );
				BIGMAP.RelativeMove( dx, dy );
				BIGMAP.ChangeScale( ds );
			};
		};
	};
};
void exec_LW_lockall( int Np, char** par, int* size )
{
	SetCurPtr( 14 );
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int j = 0; j < OSW->NBoxes; j++)
				{
					DialogsSystem* DSS = &OSW->Boxes[j].DSS;
					if (DSS)
					{
						for (int q = 0; q < MAXDLG; q++)
						{
							if (DSS->DSS[q])DSS->DSS[q]->OnUserClick = nullptr;
						};
					};
				};
			};
		};
	};
};
void exec_LW_lockbox( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;

		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int t = 0; t < Np - 1; t++)
				{
					char* box = par[t];
					for (int j = 0; j < OSW->NBoxes; j++)if (!strcmp( OSW->Boxes[j].Name, box ))
					{
						DialogsSystem* DSS = &OSW->Boxes[j].DSS;
						if (DSS)
						{
							for (int q = 0; q < MAXDLG; q++)
							{
								if (DSS->DSS[q])DSS->DSS[q]->OnUserClick = nullptr;
							};
						};
					};
				};
			};
		};
	};
};
void exec_LW_lock( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int t = 0; t < Np - 1; t++)
				{
					char* box = par[t];
					for (int j = 0; j < OSW->NBoxes; j++)
					{
						OneBox* OBX = OSW->Boxes + j;
						DialogsSystem* DSS = &OSW->Boxes[j].DSS;
						if (DSS)
						{
							for (int p = 0; p < OBX->NISPOS; p++)if (!strcmp( OBX->ISPOS[p].ID, box ))
							{
								for (int v = OBX->ISPOS[p].StartSD; v < OBX->ISPOS[p].FinSD; v++)
								{
									if (DSS->DSS[v])DSS->DSS[v]->OnUserClick = nullptr;
								};
							};
						};
					};
				};
			};
		};
	};
};
void exec_LW_visbox( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		bool vis = atoi( par[0] ) != 0;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int t = 1; t < Np - 1; t++)
				{
					char* box = par[t];
					for (int j = 0; j < OSW->NBoxes; j++)if (!strcmp( OSW->Boxes[j].Name, box ))
					{
						DialogsSystem* DSS = &OSW->Boxes[j].DSS;
						if (DSS)
						{
							for (int q = 0; q < MAXDLG; q++)
							{
								if (DSS->DSS[q])
								{
									DSS->DSS[q]->Visible = vis;
									DSS->DSS[q]->Enabled = vis;
								};
							};
						};
					};
				};
			};
		};
	};
};
void exec_LW_vis( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		bool vis = atoi( par[0] ) != 0;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int t = 1; t < Np - 1; t++)
				{
					char* box = par[t];
					for (int j = 0; j < OSW->NBoxes; j++)
					{
						OneBox* OBX = OSW->Boxes + j;
						DialogsSystem* DSS = &OSW->Boxes[j].DSS;
						if (DSS)
						{
							for (int p = 0; p < OBX->NISPOS; p++)if (!strcmp( OBX->ISPOS[p].ID, box ))
							{
								for (int v = OBX->ISPOS[p].StartSD; v < OBX->ISPOS[p].FinSD; v++)
								{
									if (DSS->DSS[v])
									{
										DSS->DSS[v]->Visible = vis;
										DSS->DSS[v]->Enabled = vis;
									};
								};
							};
						};
					};
				};
			};
		};
	};
};
void exec_LW_enbbox( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		bool vis = atoi( par[0] ) != 0;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int t = 1; t < Np - 1; t++)
				{
					char* box = par[t];
					for (int j = 0; j < OSW->NBoxes; j++)if (!strcmp( OSW->Boxes[j].Name, box ))
					{
						DialogsSystem* DSS = &OSW->Boxes[j].DSS;
						if (DSS)
						{
							for (int q = 0; q < MAXDLG; q++)
							{
								if (DSS->DSS[q])
								{
									DSS->DSS[q]->Enabled = vis;
								};
							};
						};
					};
				};
			};
		};
	};
};
void exec_LW_enb( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		bool vis = atoi( par[0] ) != 0;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				for (int t = 1; t < Np - 1; t++)
				{
					char* box = par[t];
					for (int j = 0; j < OSW->NBoxes; j++)
					{
						OneBox* OBX = OSW->Boxes + j;
						DialogsSystem* DSS = &OSW->Boxes[j].DSS;
						if (DSS)
						{
							for (int p = 0; p < OBX->NISPOS; p++)if (!strcmp( OBX->ISPOS[p].ID, box ))
							{
								for (int v = OBX->ISPOS[p].StartSD; v < OBX->ISPOS[p].FinSD; v++)
								{
									if (DSS->DSS[v])
									{
										DSS->DSS[v]->Enabled = vis;
									};
								};
							};
						};
					};
				};
			};
		};
	};
};
void exec_LW_mapdata( int Np, char** par, int* size )
{
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				BIGMAP.UpdateMapData( (byte*) par[0] );
			};
		};
	};
};
void SendSmartRequest( sicExplorer* SXP, char* Str );
void Replace( char** str, char* src, char* dst, int& MaxL );

void exec_LW_show( int Np, char** par )
{
	SetCurPtr( 0 );
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				sicExplorer* SXP_local = OSW->EXP;
				char* REF = OSW->REF;
				char* NEWST = par[0];
				if (NEWST[0] == ':')
				{
					SendSmartRequest( SXP_local, NEWST + 1 );
					return;
				};
				if (NEWST[0] == '<'&&OSW->Result)
				{
					//replacing block
					int L00, LNS;
					do
					{
						char BLOCK[32];
						int p = 0;
						while (NEWST[p] != '>'&&p < 31)p++;
						memcpy( BLOCK, NEWST, p + 1 );
						BLOCK[p + 1] = 0;
						char* nsend = strstr( NEWST + 1, BLOCK );
						LNS = strlen( NEWST );
						L00 = LNS;
						if (nsend)
						{
							LNS = nsend - NEWST + p + 1;
						};
						char* sstart = strstr( OSW->Result, BLOCK );
						if (sstart)
						{
							int pos = sstart - OSW->Result;
							char* send = strstr( sstart + 1, BLOCK );
							if (send)
							{
								int L = send - sstart + strlen( BLOCK );
								int L1 = LNS;
								int L0 = strlen( OSW->Result );
								int pos1 = send - OSW->Result + strlen( BLOCK );
								if (L1 > L)OSW->Result = (char*) realloc( OSW->Result, L0 + 1 - L + L1 );
								memmove( OSW->Result + pos1 + L1 - L, OSW->Result + pos1, L0 - pos1 + 1 );
								memcpy( OSW->Result + pos, NEWST, L1 );
								//if(OSW->CurrTempCml[0]&&OSW->Result)SaveCurrentResultTo(OSW,OSW->CurrTempCml);
							};
						};
						if (LNS != L00)
						{
							strcpy( NEWST, NEWST + LNS );
							char* ss = strchr( NEWST, '<' );
							if (ss&&ss != NEWST)
							{
								strcpy( NEWST, ss );
							};
						};
					} while (LNS != L00&&NEWST[0] == '<');
					OSW->ReParse();
					if (OSW->Result)SaveCurrentResultTo( OSW, "Internet/cash/tmpcml_0000.cml" );
				}
				else
				{
					char tmname[64];
					char* res = SaveCurrentResult( OSW, tmname );
					OSW->REF = nullptr;
					OSW->Erase();
					OSW->REF = REF;
					byte* Data = nullptr;
					int sz = LoadDefaults( &Data );
					OSW->Result = (char*) realloc( OSW->Result, sz + strlen( par[0] ) + 1 );
					if (sz)memcpy( OSW->Result, Data, sz );
					if (Data)free( Data );
					strcpy( OSW->Result + sz, par[0] );
					strcpy( OSW->WinID, win );
					OSW->EXP = SXP_local;
					OSW->Ready = 0;
					OSW->Parsed = 0;

					for (int V = 0; V < NTIME; V++)
					{
						OSW->TimeLimit[V] = 0;
						OSW->TimeLimitRequest[V][0] = 0;
					}

					if (res && OSW->Result)
					{
						char ccc[128];
						sprintf( ccc, "LW_pfile&%s", res );
						int L = strlen( OSW->Result );
						Replace( &OSW->Result, "<!goback!>", ccc, L );
						strcpy( OSW->CurrTempCml, res );

						SaveCurrentResultTo( OSW, "Internet/cash/tmpcml_0000.cml" );
					}
				}
			}
		}
	}
}

bool InReparse = 0;
void exec_LW_file( int Np, char** par )
{
	SetCurPtr( 0 );
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				sicExplorer* SXP_local = OSW->EXP;
				ResFile F = RReset( par[0] );
				if (F != INVALID_HANDLE_VALUE)
				{
					int sz = RFileSize( F );
					char* NEWST = (char*) malloc( sz + 1 );
					RBlockRead( F, NEWST, sz );
					RClose( F );
					NEWST[sz] = 0;
					if (NEWST[0] == ':')
					{
						SendSmartRequest( SXP_local, NEWST + 1 );
						return;
					};
					if (NEWST[0] == '<'&&OSW->Result)
					{
						//replacing block
						int L00, LNS;
						do
						{
							char BLOCK[32];
							int p = 0;
							while (NEWST[p] != '>'&&p < 31)p++;
							memcpy( BLOCK, NEWST, p + 1 );
							BLOCK[p + 1] = 0;
							char* nsend = strstr( NEWST + 1, BLOCK );
							LNS = strlen( NEWST );
							L00 = LNS;
							if (nsend)
							{
								LNS = nsend - NEWST + p + 1;
							};
							char* sstart = strstr( OSW->Result, BLOCK );
							if (sstart)
							{
								int pos = sstart - OSW->Result;
								char* send = strstr( sstart + 1, BLOCK );
								if (send)
								{
									int L = send - sstart + strlen( BLOCK );
									int L1 = LNS;
									int L0 = strlen( OSW->Result );
									int pos1 = send - OSW->Result + strlen( BLOCK );
									if (L1 > L)OSW->Result = (char*) realloc( OSW->Result, L0 + 1 - L + L1 );
									memmove( OSW->Result + pos1 + L1 - L, OSW->Result + pos1, L0 - pos1 + 1 );
									memcpy( OSW->Result + pos, NEWST, L1 );
									//if(OSW->CurrTempCml[0]&&OSW->Result)SaveCurrentResultTo(OSW,OSW->CurrTempCml);
								};
							};
							if (LNS != L00)
							{
								strcpy( NEWST, NEWST + LNS );
								char* ss = strchr( NEWST, '<' );
								if (ss&&ss != NEWST)
								{
									strcpy( NEWST, ss );
								};
							};
						} while (LNS != L00&&NEWST[0] == '<');
						InReparse = 1;
						OSW->ReParse();
						InReparse = 0;
						if (OSW->Result)SaveCurrentResultTo( OSW, "Internet/cash/tmpcml_0000.cml" );
					}
					else
					{
						char tmname[64];
						char* res = SaveCurrentResult( OSW, tmname );
						char* REF = OSW->REF;
						OSW->REF = nullptr;
						OSW->Erase();
						OSW->REF = REF;
						byte* Data = nullptr;
						int siz = LoadDefaults( &Data );
						OSW->Result = (char*) realloc( OSW->Result, siz + sz + 1 );
						if (siz)memcpy( OSW->Result, Data, siz );
						if (Data)free( Data );
						memcpy( OSW->Result + siz, NEWST, sz );
						OSW->Result[sz + siz] = 0;
						strcpy( OSW->WinID, win );
						OSW->EXP = SXP_local;
						OSW->Ready = 0;
						OSW->Parsed = 0;
						for (int V = 0; V < NTIME; V++)
						{
							OSW->TimeLimit[V] = 0;
							OSW->TimeLimitRequest[V][0] = 0;
						};
						if (res&&OSW->Result)
						{
							char ccc[128];
							sprintf( ccc, "LW_pfile&%s", res );
							int L = strlen( OSW->Result );
							Replace( &OSW->Result, "<!goback!>", ccc, L );
							strcpy( OSW->CurrTempCml, res );
						};
						if (OSW->Result)SaveCurrentResultTo( OSW, "Internet/cash/tmpcml_0000.cml" );
					};
					free( NEWST );
				};
			};
		};
	};
};
void exec_LW_pfile( int Np, char** par )
{
	SetCurPtr( 0 );
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int i = 0; i < NW; i++)
		{
			OneSicWindow* OSW = SX->Windows[i];
			if (!strcmp( OSW->WinID, win ))
			{
				sicExplorer* SXP_local = OSW->EXP;
				ResFile F = RReset( par[0] );
				if (F != INVALID_HANDLE_VALUE)
				{
					int sz = RFileSize( F );
					char* NEWST = (char*) malloc( sz + 1 );
					RBlockRead( F, NEWST, sz );
					RClose( F );
					NEWST[sz] = 0;
					char* REF = OSW->REF;
					OSW->REF = nullptr;
					OSW->Erase();
					OSW->REF = REF;
					OSW->Result = (char*) malloc( sz + 1 );
					memcpy( OSW->Result, NEWST, sz );
					OSW->Result[sz] = 0;
					strcpy( OSW->WinID, win );
					OSW->EXP = SXP_local;
					OSW->Ready = 0;
					OSW->Parsed = 0;
					for (int V = 0; V < NTIME; V++)
					{
						OSW->TimeLimit[V] = 0;
						OSW->TimeLimitRequest[V][0] = 0;
					};
					free( NEWST );
					if (OSW->Result)SaveCurrentResultTo( OSW, "Internet/cash/tmpcml_0000.cml" );
				};
			};
		};
	};
};
void exec_LW_cfile( int Np, char** par, int* size )
{
	if (Np < 1)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int q = 0; q < NW; q++)
		{
			OneSicWindow* OSW = SX->Windows[q];
			if (!strcmp( OSW->WinID, win ))
			{
				OSW->UPHANDLE = 0;
				for (int i = 0; i < Np - 1; i += 2)if (size[i + 1] < 64)
				{
					char cc[256];
					sprintf( cc, "Internet\\cash\\%s", par[i + 1] );
					ResFile F = RRewrite( cc );
					if (F != INVALID_HANDLE_VALUE)
					{
						RBlockWrite( F, par[i], size[i] );
						RClose( F );
					};
					//char* tc=par[i+1];
					//for(int i=0;i<SX->NDownl;i++)if(!strcmp(SX->DOWNL[i].Name,tc)){
					//	memset(SX->DOWNL[i].ReqMask,0,128);
					//};
				};
			};
		};
	};
};
void exec_LW_bonus( int Np, char** par, int* size )
{
	if (Np < 1)return;
	char MON[32];
	memcpy( MON, par[0], size[0] );
	MON[size[0]] = 0;
	int V = 0;
	V = atoi( MON );
	char VV[10];
	VV[0] = 1;
	*( (DWORD*) ( VV + 1 ) ) = V;
	SetExComm( VV, 5 );
};
void exec_LW_message( int Np, char** par, int* size )
{
	if (Np < 1)return;
	char VV[1024];
	VV[0] = 0;
	memcpy( VV + 1, par[0], size[0] );
	VV[1 + size[0]] = 0;
	SetExComm( VV, 2 + size[0] );
}

void exec_LW_abort( int Np, char** par, int* size )
{
	ExitProcess( 0 );
}

void SendSmartRequest( sicExplorer* SXP, char* Str );
void exec_LW_mupdate( int Np, char** par, int* size )
{
	if (Np < 1)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		char ccc[256];
		sprintf( ccc, "Internet\\Cash\\%s.date", BIGMAP.Title );
		FILE* F = fopen( ccc, "r" );
		if (F)
		{
			fscanf( F, "%u", &BIGMAP.UpdateData );
			fclose( F );
			sprintf( ccc, "GW|mupdate&%s\\00&%d\\00", BIGMAP.Title, BIGMAP.UpdateData );
			SendSmartRequest( SXP, ccc );
		};
	};
};
ResFile ForceOpenFile( char* Name )
{
	char tmp[512];
	char tmp1[512];
	int pos = 0;
	char c;
	do
	{
		c = Name[pos];
		while (0 != c && '\\' != c && '/' != c)
		{
			c = Name[++pos];
		}

		if (c)
		{
			memcpy( tmp, Name, pos );
			tmp[pos] = 0;
			sprintf( tmp1, "Internet/Cash/%s", tmp );
			CreateDirectory( tmp1, 0 );
			pos++;
		};
	} while (c);
	sprintf( tmp1, "Internet/Cash/%s", Name );
	return RRewrite( tmp1 );
};
void exec_LW_mpt( int Np, char** par, int* size )
{
	if (Np < 4)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int q = 0; q < NW; q++)
		{
			OneSicWindow* OSW = SX->Windows[q];
			if (!strcmp( OSW->WinID, win ))
			{
				char GP[64];
				memcpy( GP, par[0], size[0] );
				GP[size[0]] = 0;
				int dx = atoi( par[1] );
				int dy = atoi( par[2] );
				int N = ( Np - 4 ) / 3;
				for (int i = 0; i < N; i++)
				{
					int x = atoi( par[3 + i * 3] );
					int y = atoi( par[4 + i * 3] );
					int Sp = atoi( par[5 + i * 3] );
					BIGMAP.AddMapPicture( GP, x, y, dx, dy, Sp );
				};
			};
		};
	};
}

void exec_LW_mpclear( int Np, char** par, int* size )
{
	BIGMAP.ClearMapPictures();
}

bool NOXCFILE = 0;

void exec_LW_xcfile( int Np, char** par, int* size )
{
	if (Np < 3)
	{
		return;
	}
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int q = 0; q < NW; q++)
		{
			OneSicWindow* OSW = SX->Windows[q];
			if (!strcmp( OSW->WinID, win ))
			{
				OSW->UPHANDLE = 0;
				ResFile F = ForceOpenFile( par[0] );
				if (F != INVALID_HANDLE_VALUE)
				{
					RBlockWrite( F, par[2], size[2] );
					RClose( F );
					_utimbuf UTB;
					UTB.actime = 0;
					sscanf( par[1], "%lld", &UTB.actime );
					sscanf( par[1], "%lld", &UTB.modtime );
					char tmp[512];
					sprintf( tmp, "Internet/Cash/%s", par[0] );
					_utime( tmp, &UTB );
					NOXCFILE = 1;
					if (OSW->Result)
					{
						OSW->ReParse();
					}
					NOXCFILE = 0;
				}
			}
		}
	}
}

void exec_LW_key( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int NW = SX->NWindows;
		for (int q = 0; q < NW; q++)
		{
			OneSicWindow* OSW = SX->Windows[q];
			if (!strcmp( OSW->WinID, win ))
			{
				strncpy( SX->ACCESSKEY, par[0], 15 );
				SX->ACCESSKEY[15] = 0;
			};
		};
	};
};
void exec_LW_gvar( int Np, char** par, int* size )
{
	if (Np < 2)return;
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		int nn = Np >> 1;
		for (int i = 0; i < nn; i++)
		{
			char* name = par[i + i];
			char* val = par[i + i + 1];
			char* VAL = SX->GetVar( name );
			if (!VAL)
			{
				VAL = SX->CreateVar( name, 512 );
			};
			if (VAL)
			{
				strncpy( VAL, val, 511 );
				VAL[511] = 0;
			};
		};
	};
};
//LW_compose&dest_file&src1&src2&...
void exec_LW_compose( int Np, char** par, int* size )
{
	if (Np < 3)return;
	char fnm[256];
	sprintf( fnm, "Internet\\Cash\\%s", par[0] );
	ResFile F = RRewrite( fnm );
	if (F != INVALID_HANDLE_VALUE)
	{
		for (int i = 1; i < Np - 1; i++)
		{
			sprintf( fnm, "Internet\\Cash\\%s", par[i] );
			ResFile F1 = RReset( fnm );
			if (F1 != INVALID_HANDLE_VALUE)
			{
				int sz = RFileSize( F1 );
				void* Buf = malloc( sz );
				RBlockRead( F1, Buf, sz );
				RBlockWrite( F, Buf, sz );
				free( Buf );
				RClose( F1 );
				DeleteFile( fnm );
			};
		};
		RClose( F );
	};
};
typedef void LW_proc_type( char** strs, int nstrs );
void exec_LW_proc( int Np, char** par, int* size )
{
	LW_proc_type* pro = (LW_proc_type*) atoi( par[0] );
	pro( par + 1, Np - 1 );
};
DWORD GetTableHash( OneSXPTable* TB, int Line );
//deleting elements from the table
void exec_LW_dtbl( int Np, char** par, int* size )
{
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		OneSXPTable* SXT = SXP->GetTable( par[0] );
		if (SXT)
		{
			int nc = SXT->NCol;
			DWORD* T = (DWORD*) par[1];
			int N = size[1] >> 2;
			for (int i = 0; i < SXT->NLines; i++)
			{
				DWORD HV = GetTableHash( SXT, i );
				for (int j = 0; j < N; j++)if (HV == T[j])
				{
					int pos0 = i*nc;
					for (int q = 0; q < nc; q++)if (SXT->Lines[pos0 + q])
					{
						free( SXT->Lines[pos0 + q] );
						SXT->Lines[pos0 + q] = nullptr;
					};
					if (i < SXT->NLines - 1)
					{
						memmove( SXT->Lines + pos0, SXT->Lines + pos0 + nc, ( ( SXT->NLines - 1 - i )*nc ) << 2 );
					};
					i--;
					SXT->NLines--;
					j = N;
				};
			};
		};
	};
};
//adding new elements to table
void exec_LW_tbl( int Np, char** par, int* size )
{
	char* win = par[Np - 1];
	char widx = win[0];
	if (widx >= '0'&&widx <= '7')
	{
		sicExplorer* SX = SXP + widx - '0';
		OneSXPTable* SXT = SX->GetTable( par[0] );
		if (SXT)
		{
			int nc = SXT->NCol;
			int Nne = 0;
			Nne = atoi( par[1] );
			if (Np < 2 + nc*Nne)return;
			SXT->Lines = (char**) realloc( SXT->Lines, ( SXT->NLines + Nne ) * 4 * SXT->NCol );
			SXT->Refs = (int*) realloc( SXT->Refs, ( SXT->NLines + Nne ) << 2 );
			int p0 = SXT->NLines*nc;
			for (int i = 0; i < Nne; i++)
			{
				for (int j = 0; j < nc; j++)
				{
					SXT->Lines[p0 + i*nc + j] = (char*) malloc( strlen( par[2 + i*nc + j] ) + 1 );
					strcpy( SXT->Lines[p0 + i*nc + j], par[2 + i*nc + j] );
				};
				SXT->Refs[SXT->NLines + i] = SXT->NLines + i;
			};
			SXT->NLines += Nne;
		};
	};
};
void exec_LW_dfp( int Np, char** par, int* size );
void exec_LW_dff( int Np, char** par, int* size );
void exec_LW_uprec( int Np, char** par, int* size );
void exec_LW_showrec( int Np, char** par, int* size );
void ProcessDataXchange()
{
	int N = DEVSCOPE.NReq;
	for (int i = 0; i < N; i++)
	{
		if (DEVSCOPE.REQ[i].Opt & 1)
		{
			if (DEVSCOPE.DEVS[DEVSCOPE.REQ[i].DevIndex].GetRequestResult)
			{
				char* res;
				int sz;
				DWORD R = DEVSCOPE.DEVS[DEVSCOPE.REQ[i].DevIndex].GetRequestResult( 
					DEVSCOPE.REQ[i].Handle, &res, &sz );
				if (R == 128)
				{//success
					ParsedRQ P1;
					P1.Extract( res, sz );
					for (int j = 0; j < P1.NComm; j++)
					{
						char* com = P1.Comm[j].ComID;
						if (!strcmp( com, "LW_show" ))exec_LW_show( P1.Comm[j].NParams, P1.Comm[j].Params );
						else
							if (!strcmp( com, "LW_cfile" ))
							{
								exec_LW_cfile( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
							}
							else
								if (!strcmp( com, "LW_key" ))
								{
									exec_LW_key( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
								}
								else
									if (!strcmp( com, "LW_gvar" ))
									{
										exec_LW_gvar( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
									}
									else
										if (!strcmp( com, "LW_file" ))
										{
											exec_LW_file( P1.Comm[j].NParams, P1.Comm[j].Params );
										}
										else
											if (!strcmp( com, "LW_tbl" ))
											{
												exec_LW_tbl( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
											}
											else
												if (!strcmp( com, "LW_dtbl" ))
												{
													exec_LW_dtbl( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
												}
												else
													if (!strcmp( com, "LW_dfp" ))
													{
														exec_LW_dfp( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
													}
													else
														if (!strcmp( com, "LW_dff" ))
														{
															exec_LW_dff( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
														}
														else
															if (!strcmp( com, "LW_uprec" ))
															{
																exec_LW_uprec( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
															}
															else
																if (!strcmp( com, "LW_showrec" ))
																{
																	exec_LW_showrec( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																}
																else
																	if (!strcmp( com, "LW_time" ))
																	{
																		exec_LW_time( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																	}
																	else
																		if (!strcmp( com, "LW_compose" ))
																		{
																			exec_LW_compose( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																		}
																		else
																			if (!strcmp( com, "LW_move" ))
																			{
																				exec_LW_move( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																			}
																			else
																				if (!strcmp( com, "LW_mapdata" ))
																				{
																					exec_LW_mapdata( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																				}
																				else
																					if (!strcmp( com, "LW_lockall" ))
																					{
																						exec_LW_lockall( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																					}
																					else
																						if (!strcmp( com, "LW_lock" ))
																						{
																							exec_LW_lock( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																						}
																						else
																							if (!strcmp( com, "LW_lockbox" ))
																							{
																								exec_LW_lockbox( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																							}
																							else
																								if (!strcmp( com, "LW_visbox" ))
																								{
																									exec_LW_visbox( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																								}
																								else
																									if (!strcmp( com, "LW_vis" ))
																									{
																										exec_LW_vis( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																									}
																									else
																										if (!strcmp( com, "LW_enbbox" ))
																										{
																											exec_LW_lockbox( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																										}
																										else
																											if (!strcmp( com, "LW_enb" ))
																											{
																												exec_LW_lockbox( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																											}
																											else
																												if (!strcmp( com, "LW_pfile" ))
																												{
																													exec_LW_pfile( P1.Comm[j].NParams, P1.Comm[j].Params );
																												}
																												else
																													if (!strcmp( com, "LW_xcfile" ))
																													{
																														exec_LW_xcfile( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																													}
																													else
																														if (!strcmp( com, "LW_mpclear" ))
																														{
																															exec_LW_mpclear( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																														}
																														else
																															if (!strcmp( com, "LW_mpt" ))
																															{
																																exec_LW_mpt( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																															}
																															else
																																if (!strcmp( com, "LW_bonus" ))
																																{
																																	exec_LW_bonus( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																																}
																																else
																																	if (!strcmp( com, "LW_message" ))
																																	{
																																		exec_LW_message( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																																	}
																																	else
																																		if (!strcmp( com, "LW_mupdate" ))
																																		{
																																			exec_LW_mupdate( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																																		}
																																		else
																																			if (!strcmp( com, "LW_proc" ))
																																			{
																																				exec_LW_proc( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																																			}
																																			else
																																				if (!strcmp( com, "LW_abort" ))
																																				{
																																					exec_LW_abort( P1.Comm[j].NParams, P1.Comm[j].Params, P1.Comm[j].ParamSize );
																																				};
						//will grow there
					};
					DEVSCOPE.CloseRequest( DEVSCOPE.REQ[i].IHandle );
					i--;
					N--;
				};
			};
		};
	};
};

int LAST_GID = 1;

void FilterRQ2Send( sicExplorer* SXP, ParsedRQ* RQ, bool AllowNew )
{
	int sz = RQ->UnParse( nullptr, 0 );
	char* str = (char*) malloc( sz + 1 );
	RQ->UnParse( str, sz );
	char GID[16] = "";
	if (SXP->CurWPosition < SXP->NWindows)
	{
		strcpy( GID, SXP->Windows[SXP->CurWPosition]->WinID );
	};
	for (int i = 0; i < RQ->NComm; i++)
	{
		char* cid = RQ->Comm[i].ComID;

		if (cid[0] == 'L'&&cid[1] == 'W'&&cid[2] == '_')
		{
			bool del = 1;
			if (!strcmp( cid, "LW_new" ))
			{
				//open in new window
				del = 1;
				if (AllowNew)
				{
					char cc[16];
					sprintf( cc, "%d%d", SXP->ID, LAST_GID );
					strcpy( GID, cc );
					LAST_GID++;
					SXP->NewWindow( str, cc );
					SXP->CurWPosition = SXP->NWindows - 1;
				};
			}
			else if (!strcmp( cid, "LW_file" ))
			{
				if (SXP->CurWPosition < SXP->NWindows)
				{
					char* win = SXP->Windows[SXP->CurWPosition]->WinID;
					RQ->AddParamToCom( i, win, strlen( win ) );
					//RQ->AddParamToCom(i,"",1);
					exec_LW_file( RQ->Comm[i].NParams, RQ->Comm[i].Params );
				};
			}
			else if (!strcmp( cid, "LW_show" ))
			{
				if (SXP->CurWPosition < SXP->NWindows)
				{
					char* win = SXP->Windows[SXP->CurWPosition]->WinID;
					RQ->AddParamToCom( i, win, strlen( win ) );
					//RQ->AddParamToCom(i,"",1);
					exec_LW_show( RQ->Comm[i].NParams, RQ->Comm[i].Params );
				};
			}
			else if (!strcmp( cid, "LW_key" ))
			{
				char* win = SXP->Windows[SXP->CurWPosition]->WinID;
				RQ->AddParamToCom( i, win, strlen( win ) );
				exec_LW_key( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
			}
			else if (!strcmp( cid, "LW_gvar" ))
			{
				char* win = SXP->Windows[SXP->CurWPosition]->WinID;
				RQ->AddParamToCom( i, win, strlen( win ) );
				exec_LW_gvar( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
			}
			else if (!strcmp( cid, "LW_cfile" ))
			{
				char* win = SXP->Windows[SXP->CurWPosition]->WinID;
				RQ->AddParamToCom( i, win, strlen( win ) );
				exec_LW_cfile( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
			}
			else if (!strcmp( cid, "LW_tbl" ))
			{
				char* win = SXP->Windows[SXP->CurWPosition]->WinID;
				RQ->AddParamToCom( i, win, strlen( win ) );
				exec_LW_tbl( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
			}
			else if (!strcmp( cid, "LW_dtbl" ))
			{
				char* win = SXP->Windows[SXP->CurWPosition]->WinID;
				RQ->AddParamToCom( i, win, strlen( win ) );
				exec_LW_dtbl( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
			}
			else
				if (!strcmp( cid, "LW_showrec" ))
				{
					char* win = SXP->Windows[SXP->CurWPosition]->WinID;
					RQ->AddParamToCom( i, win, strlen( win ) );
					exec_LW_showrec( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
				}
				else
					if (!strcmp( cid, "LW_time" ))
					{
						char* win = SXP->Windows[SXP->CurWPosition]->WinID;
						RQ->AddParamToCom( i, win, strlen( win ) );
						exec_LW_time( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
					}
					else
						if (!strcmp( cid, "LW_compose" ))
						{
							char* win = SXP->Windows[SXP->CurWPosition]->WinID;
							RQ->AddParamToCom( i, win, strlen( win ) );
							exec_LW_compose( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
						}
						else
							if (!strcmp( cid, "LW_move" ))
							{
								char* win = SXP->Windows[SXP->CurWPosition]->WinID;
								RQ->AddParamToCom( i, win, strlen( win ) );
								exec_LW_move( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
							}
							else
								if (!strcmp( cid, "LW_mapdata" ))
								{
									char* win = SXP->Windows[SXP->CurWPosition]->WinID;
									RQ->AddParamToCom( i, win, strlen( win ) );
									exec_LW_mapdata( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
								}
								else
									if (!strcmp( cid, "LW_lockall" ))
									{
										char* win = SXP->Windows[SXP->CurWPosition]->WinID;
										RQ->AddParamToCom( i, win, strlen( win ) );
										exec_LW_lockall( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
									}
									else
										if (!strcmp( cid, "LW_lock" ))
										{
											char* win = SXP->Windows[SXP->CurWPosition]->WinID;
											RQ->AddParamToCom( i, win, strlen( win ) );
											exec_LW_lock( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
										}
										else
											if (!strcmp( cid, "LW_lockbox" ))
											{
												char* win = SXP->Windows[SXP->CurWPosition]->WinID;
												RQ->AddParamToCom( i, win, strlen( win ) );
												exec_LW_lockbox( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
											}
											else
												if (!strcmp( cid, "LW_visbox" ))
												{
													char* win = SXP->Windows[SXP->CurWPosition]->WinID;
													RQ->AddParamToCom( i, win, strlen( win ) );
													exec_LW_visbox( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
												}
												else
													if (!strcmp( cid, "LW_vis" ))
													{
														char* win = SXP->Windows[SXP->CurWPosition]->WinID;
														RQ->AddParamToCom( i, win, strlen( win ) );
														exec_LW_vis( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
													}
													else
														if (!strcmp( cid, "LW_enbbox" ))
														{
															char* win = SXP->Windows[SXP->CurWPosition]->WinID;
															RQ->AddParamToCom( i, win, strlen( win ) );
															exec_LW_enbbox( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
														}
														else
															if (!strcmp( cid, "LW_enb" ))
															{
																char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																RQ->AddParamToCom( i, win, strlen( win ) );
																exec_LW_enb( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
															}
															else
																if (!strcmp( cid, "LW_pfile" ))
																{
																	char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																	RQ->AddParamToCom( i, win, strlen( win ) );
																	exec_LW_pfile( RQ->Comm[i].NParams, RQ->Comm[i].Params );
																}
																else
																	if (!strcmp( cid, "LW_xcfile" ))
																	{
																		char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																		RQ->AddParamToCom( i, win, strlen( win ) );
																		exec_LW_xcfile( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																	}
																	else
																		if (!strcmp( cid, "LW_mpclear" ))
																		{
																			char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																			RQ->AddParamToCom( i, win, strlen( win ) );
																			exec_LW_mpclear( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																		}
																		else
																			if (!strcmp( cid, "LW_mpt" ))
																			{
																				char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																				RQ->AddParamToCom( i, win, strlen( win ) );
																				exec_LW_mpt( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																			}
																			else
																				if (!strcmp( cid, "LW_bonus" ))
																				{
																					char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																					RQ->AddParamToCom( i, win, strlen( win ) );
																					exec_LW_bonus( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																				}
																				else
																					if (!strcmp( cid, "LW_message" ))
																					{
																						char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																						RQ->AddParamToCom( i, win, strlen( win ) );
																						exec_LW_message( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																					}
																					else
																						if (!strcmp( cid, "LW_mupdate" ))
																						{
																							char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																							RQ->AddParamToCom( i, win, strlen( win ) );
																							exec_LW_mupdate( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																						}
																						else
																							if (!strcmp( cid, "LW_proc" ))
																							{
																								char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																								RQ->AddParamToCom( i, win, strlen( win ) );
																								exec_LW_proc( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																							}
																							else
																								if (!strcmp( cid, "LW_abort" ))
																								{
																									char* win = SXP->Windows[SXP->CurWPosition]->WinID;
																									RQ->AddParamToCom( i, win, strlen( win ) );
																									exec_LW_abort( RQ->Comm[i].NParams, RQ->Comm[i].Params, RQ->Comm[i].ParamSize );
																								};

			if (del)RQ->Comm[i].ComID[0] = 0;
		};
	};
	for (int i = 0; i < RQ->NComm; i++)if (RQ->Comm[i].ComID[0])
	{
		RQ->AddParamToCom( i, GID, strlen( GID ) + 1 );
		RQ->AddParamToCom( i, SXP->ACCESSKEY, strlen( SXP->ACCESSKEY ) + 1 );
	};
	if (str)free( str );
};
//----------------==DOWNLOAD ENGINE==----------------//
struct OneDnProcess
{
	char FileName[128];
	char Server[8];
	char HashName[128];
	int  LastReqTime;
	int  DownloadedSize;
	bool NotFinished : 1;
	bool NeedToSendRequest : 1;
};
OneDnProcess* DNPR = nullptr;
int N_DNPR = 0;
int Max_DNPR = 0;
__declspec( dllexport )
void StartDownloadInternetFile( char* Name, char* Server, char* DestName )
{
	//search if process exists
	for (int i = 0; i < N_DNPR; i++)
	{
		if (!strcmp( DestName, DNPR[i].HashName ))return;
	};
	if (N_DNPR >= Max_DNPR)
	{
		Max_DNPR += 8;
		DNPR = (OneDnProcess*) realloc( DNPR, Max_DNPR * sizeof OneDnProcess );
	};

	//check if already downloaded
	char CC[1024];
	sprintf( CC, "Internet\\dnl\\%s.ready", DestName );
	ResFile F = RReset( CC );
	if (F != INVALID_HANDLE_VALUE)
	{
		RClose( F );
		return;
	};
	memset( DNPR + N_DNPR, 0, sizeof OneDnProcess );
	sprintf( CC, "Internet\\dnl\\%s.open", DestName );
	F = RReset( CC );
	if (F != INVALID_HANDLE_VALUE)
	{
		DNPR[N_DNPR].DownloadedSize = RFileSize( F );
		RClose( F );
	};
	strcpy( DNPR[N_DNPR].FileName, Name );
	strcpy( DNPR[N_DNPR].Server, Server );
	strcpy( DNPR[N_DNPR].HashName, DestName );
	DNPR[N_DNPR].LastReqTime = 0;
	N_DNPR++;
	ParsedRQ P1;
	strcpy( P1.DevName, Server );
	P1.AddComm( "dnfile" );
	P1.AddParam( Name, strlen( Name ) + 1 );
	P1.AddParam( DestName, strlen( DestName ) + 1 );
	P1.AddIntParam( DNPR[N_DNPR - 1].DownloadedSize );
	P1.AddIntParam( 8192 );
	DNPR[N_DNPR].LastReqTime = GetTickCount();
	P1.UnParse( CC, 1024 );
	SendGlobalRequest( SXP + 7, CC, 0 );
};
//int LastDNSize=0;
__declspec( dllexport )
void ProcessDownloadInternetFiles()
{
	int TIME = GetTickCount();
	for (int i = 0; i < N_DNPR; i++)
	{
		if (DNPR[i].NeedToSendRequest&&TIME - DNPR[i].LastReqTime > 16000)
		{
			char CC[1024];
			ParsedRQ P1;
			strcpy( P1.DevName, DNPR[i].Server );
			P1.AddComm( "dnfile" );
			P1.AddParam( DNPR[i].FileName, strlen( DNPR[i].FileName ) + 1 );
			P1.AddParam( DNPR[i].HashName, strlen( DNPR[i].HashName ) + 1 );
			P1.AddIntParam( DNPR[i].DownloadedSize );
			P1.AddIntParam( 8192 + 8192 );
			DNPR[i].LastReqTime = TIME;
			P1.UnParse( CC, 1024 );
			SendGlobalRequest( SXP + 7, CC, 0 );

		}
	}
}
char CurrUplID[128] = "";
int CurrUplPos = 0;
__declspec( dllexport ) void SendRecBuffer( byte* Data, int size, bool Final )
{
	if (CurrUplID[0])
	{
		ReportIt( "upfile(%s,%d+%d->%d)\n", CurrUplID, CurrUplPos, size, CurrUplPos + size );
		ParsedRQ P1;
		strcpy( P1.DevName, "GW" );
		P1.AddComm( "upfile" );
		P1.AddParam( CurrUplID, strlen( CurrUplID ) + 1 );
		P1.AddIntParam( Final );
		DWORD* DATA = (DWORD*) malloc( size + 8 );
		DATA[0] = CurrUplPos;
		DATA[1] = size;
		memcpy( DATA + 2, Data, size );
		CurrUplPos += size;
		P1.AddParam( (char*) DATA, size + 8 );
		int szz = P1.UnParse( nullptr, 0 );
		free( DATA );
		char* DB = (char*) malloc( szz + 16 );
		P1.UnParse( DB, szz + 16 );
		SendGlobalRequest( SXP + 7, DB, 0 );
		if (Final)
		{
			CurrUplID[0] = 0;
			CurrUplPos = 0;
		}
		free( DB );
	}
}

void exec_LW_dfp( int Np, char** par, int* size )
{
	if (Np >= 2 && size[1] >= 8)
	{
		DWORD* DD = (DWORD*) par[1];
		int ofs = DD[0];
		int sz = DD[1];
		char cc[128];
		sprintf( cc, "Internet\\dnl\\%s.open", par[0] );
		ResFile F = RReset( cc );
		int CSize = 0;
		if (F != INVALID_HANDLE_VALUE)
		{
			CSize = RFileSize( F );
			RClose( F );
			if (CSize != ofs)return;
		}
		else if (ofs)return;
		FILE* f = fopen( cc, "ab" );
		if (f)
		{
			fwrite( DD + 2, 1, sz, f );
			for (int i = 0; i < N_DNPR; i++)if (!strcmp( DNPR[i].HashName, par[0] ))
			{
				DNPR[i].DownloadedSize = CSize + sz;
				DNPR[i].LastReqTime = GetTickCount();
				if (sz != 8192)DNPR[i].NotFinished = 1;
				DNPR[i].NeedToSendRequest = 1;
			};
			fclose( f );
		};
	};
};
void exec_LW_dff( int Np, char** par, int* size )
{
	if (Np >= 2 && size[1] >= 8)
	{
		DWORD* DD = (DWORD*) par[1];
		int ofs = DD[0];
		int sz = DD[1];
		char cc[128];
		sprintf( cc, "Internet\\dnl\\%s.open", par[0] );
		ResFile F = RReset( cc );
		int CSize = 0;
		if (F != INVALID_HANDLE_VALUE)
		{
			CSize = RFileSize( F );
			RClose( F );
			if (CSize != ofs)return;
		}
		else if (ofs)return;
		FILE* f = fopen( cc, "ab" );
		if (f)
		{
			fwrite( DD + 2, 1, sz, f );
			for (int i = 0; i < N_DNPR; i++)if (!strcmp( DNPR[i].HashName, par[0] ))
			{
				if (i < N_DNPR - 1)memcpy( DNPR + i, DNPR + i + 1, ( N_DNPR - 1 - i ) * sizeof OneDnProcess );
				N_DNPR--;
				i--;
			};
			fclose( f );
			char cc1[128];
			sprintf( cc1, "Internet\\dnl\\%s.ready", par[0] );
			remove( cc1 );
			rename( cc, cc1 );
		};
	};
};
__declspec( dllimport )
void SetGameID( char* ID );
void exec_LW_uprec( int Np, char** par, int* size )
{
	if (Np < 1)return;
	CurrUplPos = 0;
	strncpy( CurrUplID, par[0] + 1, 127 );
	CurrUplID[127] = 0;
	SetGameID( CurrUplID );
	char c = par[0][0];
	if (c == '0')
	{
		SetGameID( "" );
		CurrUplID[0] = 0;
	}
	else
		if (c != 'R')
		{
			CurrUplID[0] = 0;
		};
};
__declspec( dllexport )
void PlayRecfile( char* RecFile );
void exec_LW_showrec( int Np, char** par, int* size )
{
	SetCurPtr( 0 );
	if (Np < 1)return;
	char cc[128];
	sprintf( cc, "GW|%s", par[0] );
	PlayRecfile( cc );
	SetCurPtr( 0 );
};

