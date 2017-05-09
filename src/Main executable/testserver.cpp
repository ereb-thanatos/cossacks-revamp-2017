#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "multipl.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <time.h>
#include "Nucl.h"

#include "Megapolis.h"
#include "Dialogs.h"
#include "fonts.h"
#include "dpchat.h"
#include "dplobby.h"
#include "GSound.h"
#include "3DGraph.h"
#include "3DMapEd.h"
#include "MapSprites.h"
#include <assert.h>
#include <math.h>
#include "NewMon.h"
#include "IconTool.h"
#include "GP_Draw.h"
#include "3DRandMap.h"
#include <crtdbg.h>
#include "ActiveScenary.h"
#include "DrawForm.h"
#include "Conststr.h"
#include <Process.h>
#include "MPlayer.h"
#include "Recorder.h"
#include "GSINC.H"
#include "TopoGraf.h"

#include "CEngine\goaceng.h"
#include "StrategyResearch.h"
#include "Safety.h"
#include "EinfoClass.h"
#include "mode.h"

#pragma pack(1)

#include "IR.H"
#include "bmptool.h"

typedef void fnInitSXP();
typedef void fnRunSXP( int, char*, int x, int y, int x1, int y1 );
typedef void fnProcessSXP( int, DialogsSystem* );
typedef void fnSXP_Operation( int );
typedef char* fnGetAccessKey( int );
typedef void fnSetAccessKey( int, char* );
typedef void tpSXP_SetVar( int Index, char* Name, char* value );
typedef char* tpSXP_GetVar( int Index, char* Name );
typedef void tpOpenRef( int Index, char* home );
typedef void tpResizeSXP( int Index, int x, int y, int x1, int y1 );
typedef void tpStartDownloadInternetFile( char* Name, char* Server, char* DestName );
typedef void tpProcessDownloadInternetFiles();
typedef void tpSendRecBuffer( byte* Data, int size, bool Final );
extern bool KeyPressed;
extern int LastKey;

HMODULE H_Exp = nullptr;
HMODULE H_ExpOld = nullptr;
fnInitSXP* InitSXP = nullptr;
fnRunSXP* RunSXP = nullptr;
fnProcessSXP* ProcessSXP = nullptr;
fnGetAccessKey* GetAccessKey = nullptr;
fnSetAccessKey* SetAccessKey = nullptr;
fnSXP_Operation* SXP_StepBack = nullptr;
fnSXP_Operation* SXP_StepForw = nullptr;
fnSXP_Operation* SXP_Refresh = nullptr;
tpSXP_SetVar* SXP_SetVar = nullptr;
tpSXP_GetVar* SXP_GetVar = nullptr;
tpOpenRef* OpenRef = nullptr;
tpResizeSXP* ResizeSXP = nullptr;
tpStartDownloadInternetFile* StartDownloadInternetFile = nullptr;
tpProcessDownloadInternetFiles* ProcessDownloadInternetFiles = nullptr;
tpSendRecBuffer* SendRecBuffer = nullptr;

void InitDipFunctions();

__declspec( dllexport ) void StartExplorer()
{
	//attempt to use advanced version
	ResFile F = RReset( "Internet\\Cash\\IntExplorerNew.dll" );
	if (F != INVALID_HANDLE_VALUE)
	{
		int sz = RFileSize( F );
		void* buf = malloc( sz );
		RBlockRead( F, buf, sz );
		RClose( F );
		F = RRewrite( "IntExplorer.dll" );
		if (F != INVALID_HANDLE_VALUE)
		{
			RBlockWrite( F, buf, sz );
			RClose( F );
		}
		free( buf );
		DeleteFile( "Internet\\Cash\\IntExplorerNew.dll" );
	}

	H_Exp = LoadLibrary( "IntExplorer.dll" );

	if (H_Exp)
	{
		InitSXP = (fnInitSXP*) GetProcAddress( H_Exp, "InitSXP" );

		if (InitSXP)
		{
			InitSXP();
		}

		RunSXP = (fnRunSXP*) GetProcAddress( H_Exp, "RunSXP" );
		ProcessSXP = (fnProcessSXP*) GetProcAddress( H_Exp, "ProcessSXP" );
		GetAccessKey = (fnGetAccessKey*) GetProcAddress( H_Exp, "?GetAccessKey@@YAPADH@Z" );
		SetAccessKey = (fnSetAccessKey*) GetProcAddress( H_Exp, "?SetAccessKey@@YAXHPAD@Z" );
		SXP_StepBack = (fnSXP_Operation*) GetProcAddress( H_Exp, "SXP_StepBack" );
		SXP_StepForw = (fnSXP_Operation*) GetProcAddress( H_Exp, "SXP_StepForw" );
		SXP_Refresh = (fnSXP_Operation*) GetProcAddress( H_Exp, "SXP_Refresh" );
		SXP_SetVar = (tpSXP_SetVar*) GetProcAddress( H_Exp, "SXP_SetVar" );
		SXP_GetVar = (tpSXP_GetVar*) GetProcAddress( H_Exp, "SXP_GetVar" );
		OpenRef = (tpOpenRef*) GetProcAddress( H_Exp, "OpenRef" );
		ResizeSXP = (tpResizeSXP*) GetProcAddress( H_Exp, "ResizeSXP" );
		StartDownloadInternetFile = (tpStartDownloadInternetFile*) GetProcAddress( H_Exp, "?StartDownloadInternetFile@@YAXPAD00@Z" );
		ProcessDownloadInternetFiles = (tpProcessDownloadInternetFiles*) GetProcAddress( H_Exp, "?ProcessDownloadInternetFiles@@YAXXZ" );
		SendRecBuffer = (tpSendRecBuffer*) GetProcAddress( H_Exp, "?SendRecBuffer@@YAXPAEH_N@Z" );
	}
	InitDipFunctions();
}

__declspec( dllexport ) void FinExplorer()
{
	if (H_Exp)
	{
		FreeLibrary( H_Exp );

		H_Exp = nullptr;
		InitSXP = nullptr;
		RunSXP = nullptr;
		ProcessSXP = nullptr;

		if (H_ExpOld)
		{
			FreeLibrary( H_ExpOld );
		}
	}
}

__declspec( dllexport ) void ExplorerBack( int Index )
{
	if (SXP_StepBack)
	{
		SXP_StepBack( Index );
	}
}

__declspec( dllexport )void ExplorerForw( int Index )
{
	if (SXP_StepBack)
	{
		SXP_StepForw( Index );
	}
}

__declspec( dllexport )void ExplorerRefresh( int Index )
{
	if (SXP_Refresh)
	{
		SXP_Refresh( Index );
	}
}

__declspec( dllexport ) void RunExplorer( int Index, char* ref, int x, int y, int x1, int y1 )
{
	if (RunSXP)
	{
		RunSXP( Index, ref, x, y, x1, y1 );
	}
}

__declspec( dllexport )void ProcessExplorer( int Index )
{
	if (ProcessSXP)
	{
		ProcessSXP( Index, nullptr );
	}
}

__declspec( dllexport ) void ProcessExplorerDSS( int Index, DialogsSystem* DSS )
{
	if (ProcessSXP)
	{
		ProcessSXP( Index, DSS );
	}
}

__declspec( dllexport ) void ExplorerSetVar( int Index, char* Name, char* value )
{
	if (SXP_SetVar)
	{
		SXP_SetVar( Index, Name, value );
	}
}

__declspec( dllexport ) char* ExplorerGetVar( int Index, char* Name )
{
	if (SXP_SetVar)
	{
		return SXP_GetVar( Index, Name );
	}
	else
	{
		return nullptr;
	}
}

__declspec( dllexport ) void ExplorerOpenRef( int Index, char* ref )
{
	if (OpenRef)
	{
		OpenRef( Index, ref );
	}
}

__declspec( dllexport ) void ExplorerResize( int Index, int x, int y, int x1, int y1 )
{
	if (ResizeSXP)
	{
		ResizeSXP( Index, x, y, x1, y1 );
	}
}

bool ProcessNewInternetLogin();

typedef void fnVoid();
typedef void fnVoidLPB( byte* );
typedef void fnVoidLPC( char* );
typedef void tpSaveAllDipData( byte** ptr, int* size );
typedef void tpPerformDipCommand( char* Data, int size );
typedef void tpLoadAllDipData( byte* ptr, int size );
typedef void tpStartDownloadInternetFile( char* Name, char* Server, char* DestName );
typedef void tpProcessDownloadInternetFiles();
typedef void tpSendRecBuffer( byte* Data, int size, bool Final );

fnVoid* ProcessDipRelations = nullptr;
fnVoid* InitDipForThisMap = nullptr;
fnVoid* ClearAllDipData = nullptr;
fnVoidLPB* FreeSaveDipData = nullptr;
tpSaveAllDipData* SaveAllDipData = nullptr;
tpLoadAllDipData* LoadAllDipData = nullptr;
tpPerformDipCommand* PerformDipCommand = nullptr;

void InitDipFunctions()
{
	HMODULE HDIP = GetModuleHandle( "DipServer.dll" );
	ProcessDipRelations = (fnVoid*) GetProcAddress( HDIP, "?ProcessDipRelations@@YAXXZ" );
	InitDipForThisMap = (fnVoid*) GetProcAddress( HDIP, "?InitDipForThisMap@@YAXXZ" );
	LoadAllDipData = (tpLoadAllDipData*) GetProcAddress( HDIP, "?LoadAllDipData@@YAXPAEH@Z" );
	ClearAllDipData = (fnVoid*) GetProcAddress( HDIP, "?ClearAllDipData@@YAXXZ" );
	FreeSaveDipData = (fnVoidLPB*) GetProcAddress( HDIP, "?FreeSaveDipData@@YAXPAE@Z" );
	SaveAllDipData = (tpSaveAllDipData*) GetProcAddress( HDIP, "?SaveAllDipData@@YAXPAPAEPAH@Z" );
	PerformDipCommand = (tpPerformDipCommand*) GetProcAddress( HDIP, "?PerformDipCommand@@YAXPADH@Z" );
}
