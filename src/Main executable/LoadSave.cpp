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
#include <assert.h>

#include "3DSurf.h"
#include "crtdbg.h"
#include "3DBars.h"
#include "Path.h"
#include "MapSprites.h"
#include "StrategyResearch.h"
#include "TopoGraf.h"

#include "Safety.h"
#include "NewAI.h"
#include "Danger.h"
#include "ActiveZone.h"
#include "ActiveScenary.h"
#include "Graphs.h"
#include "EinfoClass.h"
#include "VirtScreen.h"

#include "PlayerInfo.h"
extern PlayerInfo PINFO[8];

extern const int kMinorMessageDisplayTime;

extern word SelCenter[8];
void CreateFishMap();
extern int LightDX;
extern int LightDY;
extern int LightDZ;
extern word NNewMon;
extern NewMonster NewMon[512];
extern word NucList[128];
extern word NucSN[128];
extern bool NDone[128];
extern word NNuc;
char GameName[128];
extern City CITY[8];
extern bool EUsage[8192];
extern AnmObject* GAnm[8192];

void LoadNation( char* fn, byte NIndex, byte );

extern word FlyMops[256][256];
extern byte Locking[1024];//Инф. о блокировании поверхности

extern int RealLx;
extern int RealLy;
extern int Pitch;
extern bool MiniMade;
void SetSumms();
extern bool SetDestMode;
void ClearUniCash();

class SaveBuf
{
public:
	byte* Buf;
	int Pos;
	int Size;
	int RealSize;
	SaveBuf();
	void Init();
	~SaveBuf();
	void Clear();
	void SaveToFile( ResFile f1 );
	void LoadFromFile( ResFile f1 );
};

SaveBuf::SaveBuf()
{
	Buf = NULL;
	Size = 0;
	RealSize = 0;
	Pos = 0;
};
void SaveBuf::Init()
{
	Buf = NULL;
	Size = 0;
	RealSize = 0;
	Pos = 0;
};
void SaveBuf::Clear()
{
	if (Buf)free( Buf );
	Init();
};
SaveBuf::~SaveBuf()
{
	Clear();
}

void SaveBuf::SaveToFile( ResFile f1 )
{
	if (Buf)
		RBlockWrite( f1, Buf, Size );
	Clear();
}

void SaveBuf::LoadFromFile( ResFile f1 )
{
	Clear();
	Size = RFileSize( f1 );
	RealSize = Size;
	Buf = new byte[Size];
	RBlockRead( f1, Buf, Size );
};
void xBlockWrite( SaveBuf* SB, void* Data, int Size )
{
	while (SB->Size + Size > SB->RealSize)
	{
		SB->RealSize += 65536;
		SB->Buf = (byte*) realloc( SB->Buf, SB->RealSize );
	};
	memcpy( SB->Buf + SB->Size, Data, Size );
	SB->Size += Size;
};
void xBlockRead( SaveBuf* SB, void* Data, int Size )
{
	//assert(SB->Buf&&SB->Pos+Size<=SB->Size);
	if (SB->Buf)memcpy( Data, SB->Buf + SB->Pos, Size );
	SB->Pos += Size;
};
void SAVMES( SaveBuf* ff1, char* mes )
{
	char mss[16];
	memset( mss, 0, 16 );
	int len = strlen( mes );
	if (len > 10)len = 16;
	memcpy( mss, mes, len );
	xBlockWrite( ff1, mss, 16 );
};
void SAVMES1( SaveBuf* ff1, char* mes, int parm )
{
	char mss[16];
	char xxx[28];
	sprintf( xxx, "%s %d", mes, parm );
	memset( mss, 0, 16 );
	int len = strlen( xxx );
	if (len > 16)len = 16;
	memcpy( mss, xxx, len );
	xBlockWrite( ff1, mss, 16 );
};
void LOADMES( SaveBuf* ff1 )
{
	char zzz[16];
	xBlockRead( ff1, zzz, 16 );
};
extern int CurPalette;
void LOutErr( LPCSTR s )
{
	if (CurPalette == 2)LoadPalette( "2\\agew_1.pal" );
	else LoadPalette( "0\\agew_1.pal" );
	MessageBox( hwnd, s, "Loading failed...", MB_ICONWARNING | MB_OK );
};
void SOutErr( LPCSTR s )
{
	MessageBox( hwnd, s, "Saving failed...", MB_ICONWARNING | MB_OK );
};
void SavePackArray( SaveBuf* ff1, word defval, word* dest, int size )
{
	int szs = 0;
	for (int i = 0; i < size; i++)if (dest[i] != defval)szs++;
	xBlockWrite( ff1, &szs, 4 );
	xBlockWrite( ff1, &defval, 2 );
	for (int i = 0; i < size; i++)if (dest[i] != defval)
	{
		xBlockWrite( ff1, &i, 3 );
		xBlockWrite( ff1, &dest[i], 2 );
	};
};
void LoadPackArray( SaveBuf* ff1, word* dest, int size )
{
	int szs;
	word defv;
	xBlockRead( ff1, &szs, 2 );
	xBlockRead( ff1, &defv, 2 );
	for (int i = 0; i < size; i++)dest[i] = defv;
	int ofst = 0;
	word vall;
	for (int i = 0; i < szs; i++)
	{
		ofst = 0;
		xBlockRead( ff1, &ofst, 3 );
		xBlockRead( ff1, &vall, 2 );
		dest[ofst] = vall;
	};
};
extern int NNations;
void Nation::CloseNation()
{
	if (hLibAI)
	{
		FreeLibrary( hLibAI );
		DLLName = NULL;
		ProcessAIinDLL = NULL;
	};
	for (int i = 0; i < NMon; i++)
	{
		if (Mon[i])
		{
			Mon[i]->CloseGO();
			free( Mon[i] );
		};
		if (PACount[i] && PAble[i])
		{
			free( PAble[i] );
			free( AIndex[i] );
			free( AHotKey[i] );
		};
		PACount[i] = 0;
	};
	for (int i = 0; i < NHistory; i++)
	{
		free( History[i] );
	};
	VictState = 0;
	NHistory = 0;
	History = NULL;
	NMon = 0;
	for (int i = 0; i < NCOND; i++)
	{
		if (CLSize[i])free( CLRef[i] );
	};
	NGidot = 0;
	NFarms = 0;
	NArtdep = 0;
	for (int i = 0; i < 4; i++)NArtUnits[i] = 0;
	NCOND = 0;
	Harch = 0;
	if (NLmenus)free( Lmenus );
	if (NWmenus)free( Wmenus );
	if (NAmenus)free( Amenus );
	if (NCmenus)free( Cmenus );
	for (int i = 0; i < NIcons; i++)
	{
		WIcon* icc = wIcons[i];
		if (icc->Message)free( icc->Message );
		if (icc->SubList)free( icc->SubList );
		free( wIcons[i] );
	};
	if (NUnits)
	{
		for (int i = 0; i < NNations; i++)
		{
			if (NUnits[i])
			{
				for (int j = 0; j < NUnits[i]; j++)
				{
					if (UnitNames[i][j])free( UnitNames[i][j] );
				};
				free( UnitsIDS[i] );
				free( UnitNames[i] );
			};
		};
		free( UnitsIDS );
		free( UnitNames );
		free( NUnits );
		NUnits = 0;
	};
	for (int i = 0; i < NUpgrades; i++)
	{
		NewUpgrade* NU = UPGRADE[i];
		if (NU->AutoEnable)free( NU->AutoEnable );
		if (NU->AutoPerform)free( NU->AutoPerform );
		if (NNUM == 0)
		{
			if (NU->UnitGroup)free( NU->UnitGroup );
			if (NU->ValGroup)free( NU->ValGroup );
			if (NU->CtgGroup)free( NU->CtgGroup );
		};
		free( NU );
	};
	NUpgrades = 0;
	//erasing walls
	for (int i = 0; i < WSys.NClusters; i++)
	{
		WallCluster* WCL = WSys.WCL[i];
		if (WCL->Cells)free( WCL->Cells );
		if (WCL->CornPt)free( WCL->CornPt );
		free( WCL );
	};
	if (WSys.WCL)free( WSys.WCL );
	WSys.WCL = NULL;
	WSys.NClusters = NULL;
	ThereWasUnit = 0;
	if (Popul)
	{
		free( Popul );
		NPopul = 0;
		Popul = NULL;
		MaxPopul = 0;
	};
	if (UpgIDS)
	{
		free( UpgIDS );
		free( UpgTime );
		NUpgMade = 0;
		MaxUpgMade = 0;
		UpgIDS = NULL;
		UpgTime = NULL;
	};
	VictState = 0;
};
void GeneralObject::CloseGO()
{
	//if(Message)free(Message);
	if (NUpgrades)free( Upg );
	if (NIcons)free( IRefs );
	free( MoreCharacter );
	free( MonsterID );
};
void OneObject::CloseObject()
{
	if (NInside)free( Inside );
	if (PathX)free( PathX );
	if (PathY)free( PathY );
	PathX = NULL;
	PathY = NULL;
	CPdestX = 0;
	CPdestY = 0;
	NIPoints = 0;
	CurIPoint = 0;
	ClearOrders();
	//if(InLineCom)FreeAsmLink();
	if (IFire)
	{
		IFire->Erase();
		free( IFire );
	};
};
void InitPulse();
void DeleteAllSprites();
void InitBlobs();
void StopDynamicalTopology();
void InitDeathList();
extern MotionField UnitsField;

extern int NInGold[8];
extern int NInIron[8];
extern int NInCoal[8];
extern int WasInGold[8];
extern int WasInIron[8];
extern int WasInCoal[8];
extern int LastAttackX;
extern int LastAttackY;
extern int AlarmDelay;
extern int tmtmt;
extern char MapScenaryDLL[200];
void ReloadECO();
int GetEconomyData( byte** EC );
int LastTimeStage = -1;
extern bool NoWinner;
extern bool NOPAUSE;
extern DWORD RealTime;
void ClearUINF();
extern int FirstMissTime;
extern bool Tutorial;
extern word FIELDID;
void ClearFog();
extern int PrevMissTime;
extern int GLOBALTIME;
extern int PGLOBALTIME;
extern int PREVGLOBALTIME;
bool AddUnitsToCash( byte NI, word ID );
void ClearUniCash();
void GetCorrectMoney( byte NI, int* MONEY );
extern int RealPause;
extern int RealStTime;
extern int RealGameLength;
extern int CurrentStepTime;
extern bool NOPAUSE;
extern bool LockPause;
void ClearHints();
extern int WaitState;
int StartTmtmt = 0;
extern bool ShowStat;
extern int _pr_Nx;
void CloseExplosions();
extern int AI_Registers[8][32];
extern int NThemUnits;
extern int NMyUnits;
extern char LASTSAVEFILE[64];
extern int LastAddSpr;
extern byte* RivDir;
void ClearSMS();
extern int NeedITR;
void DoNormalTBL();
extern int GameTime;
extern int PeaceTimeLeft;
int CURTMTMT = 0;

extern byte BalloonState;
extern byte CannonState;
extern byte NoArtilleryState;
extern byte XVIIIState;
extern byte CaptState;
extern byte SaveState;
extern byte DipCentreState;
extern byte ShipyardState;
extern byte MarketState;

extern bool BreefUInfo;
extern bool RESMODE;
extern word COMPSTART[8];
extern char DEFPLNAMES[8][64];
extern bool AttGrMode;
extern word* TopIslands;
extern int HISPEED;
extern int HISPEED;
extern bool ChangeNation;
extern bool MultiTvar;
extern int FogMode;
void UnPress();
extern byte MI_Mode;
extern int DefaultResTBL[12];
extern int  NInResTBL;
extern int* ResTBL;
extern int CUR_TOOL_MODE;
extern bool TP_Made;
void ClearModes();
void EndGSC_Reporting();
extern bool HaveExComm;
void EraseRND();
extern int ExitNI;

//Zero a LOT of variables and pointers
void UnLoading()
{
	ExitNI = -1;

	if (!RivDir)
		return;

	EraseRND();
	HaveExComm = 0;
	ClearModes();
	ResTBL = DefaultResTBL;
	NInResTBL = 6;
	MI_Mode = 1;
	CUR_TOOL_MODE = 0;
	TP_Made = 0;
	HISPEED = 0;

	for (int i = 0; i < 8; i++)
	{
		DEFPLNAMES[8][0] = 0;
	}

	if (TopIslands)
	{
		free( TopIslands );
	}

	UnPress();
	ChangeNation = 0;
	MultiTvar = 0;
	FogMode = 1;
	TopIslands = nullptr;
	RESMODE = 0;
	AttGrMode = 0;
	BreefUInfo = 0;
	BalloonState = 0;
	CannonState = 0;
	XVIIIState = 0;
	CaptState = 0;
	SaveState = 2;
	GameTime = 0;
	CURTMTMT = 0;
	PeaceTimeLeft = 0;
	DoNormalTBL();
	NeedITR = 100;
	ClearSMS();
	memset( NPresence, 0, VAL_MAXCIOFS );

	LastAddSpr = 0;
	CLRGR();
	LASTSAVEFILE[0] = 0;
	memset( &AI_Registers, 0, sizeof AI_Registers );
	CloseExplosions();
	NThemUnits = 1;
	NMyUnits = 1;
	_pr_Nx = -1;
	ShowStat = 0;
	StartTmtmt = 0;
	WaitState = 0;
	ClearHints();
	NOPAUSE = 1;
	LockPause = 0;
	CurrentStepTime = 80;
	RealPause = 0;
	RealStTime = 0;
	RealGameLength = 0;
	ClearUniCash();
	ClearFog();
	FIELDID = 0xFFFF;
	Tutorial = 0;
	ClearUINF();
	NOPAUSE = 1;
	NoWinner = 0;
	LastTimeStage = -1;
	Creator = 4096 + 255;
	MapScenaryDLL[0] = 0;
	ReloadECO();
	GoAndAttackMode = 0;
	memset( BLDList, 0xFF, VAL_MAXCIOFS * 2 );
	tmtmt = 0;
	REALTIME = 0;
	RealTime = 0;
	GLOBALTIME = 0;
	PGLOBALTIME = 0;
	PREVGLOBALTIME = 0;
	LastAttackX = -1;
	LastAttackY = -1;
	AlarmDelay = 0;
	FirstMissTime = -10000;
	PrevMissTime = -10000;

	for (int w = 0; w < 8; w++)
	{
		NInGold[w] = 0;
		NInIron[w] = 0;
		NInCoal[w] = 0;
		WasInGold[w] = 0;
		WasInIron[w] = 0;
		WasInCoal[w] = 0;
	}

	MISSLIST.CurrentMission = -1;
	SCENINF.UnLoading();
	SCENINF.LooseGame = 0;
	SCENINF.Victory = 0;
	InitDANGER();
	SetDestMode = false;
	InitDeathList();
	InitPulse();
	InitBlobs();
	InitGates();
	InitNatList();
	ClearAllZones();
	DeleteAllSprites();
	ClearObjs3();

	for (int i = 0; i < 8; i++)
	{
		NATIONS[i].CloseNation();
		CITY[i].CloseCity();
		CITY[i].MyIsland = 0xFF;
	}

	for (int i = 0; i < MaxObject; i++)
	{
		if (Group[i])
		{
			Group[i]->CloseObject();
			Group[i] = NULL;
		}
	}

	memset( &OBJECTS, 0, sizeof OBJECTS );

	for (int i = 0; i < 80; i++)
	{
		if (int( SelSet[i].Member ))
			free( SelSet[i].Member );

		if (int( SelSet[i].SerialN ))
			free( SelSet[i].SerialN );

		memset( SelSet + i, 0, sizeof SelSet[i] );
	}

	for (int i = 0; i < 8; i++)
	{
		if (int( Selm[i] ))
			free( Selm[i] );

		if (int( SerN[i] ))
			free( SerN[i] );

		Selm[i] = nullptr;
		SerN[i] = nullptr;

		if (int( ImSelm[i] ))
			free( ImSelm[i] );

		if (int( ImSerN[i] ))
			free( ImSerN[i] );

		ImSelm[i] = nullptr;
		ImSerN[i] = nullptr;
		SelCenter[i] = 0;
		NSL[i] = 0;
		ImNSL[i] = 0;

		Nation* NT = NATIONS + i;
		memset( NT->NKilled, 0, sizeof NT->NKilled );
		memset( NT->NProduced, 0, sizeof NT->NProduced );
		memset( NT->ResTotal, 0, 32 );
		memset( NT->ResOnMines, 0, 32 );
		memset( NT->ResOnBuildings, 0, 32 );
		memset( NT->ResOnUpgrade, 0, 32 );
		memset( NT->ResOnUnits, 0, 32 );
		memset( NT->ResOnLife, 0, 32 );
	}

	UnitsField.ClearMaps();
	memset( EUsage, 0, sizeof EUsage );
	NNuc = 0;
	memset( NucList, 0, sizeof NucList );
	memset( NucSN, 0, sizeof NucSN );
	memset( NDone, 0, sizeof NDone );
	memset( WRefs, 0, VAL_MAXCIOFS * 4 * 4 );
	ClearTopChange();
	StopDynamicalTopology();
	if (MineList)
	{
		free( MineList );
		MineList = nullptr;
		NMines = 0;
		MaxMine = 0;
	}

	if (Sprites)
		free( Sprites );

	Sprites = nullptr;
	MaxSprt = 0;
}

//SAVING...
void SaveRLE1( SaveBuf* SB, void* data, int Size )
{
	byte* Data = (byte*) data;
	byte bff[256];
	int dpos = 0;
	int bfpos = 0;
	bool ctype = false;
	byte pre = 0;
	xBlockWrite( SB, &Size, 4 );
	while (dpos < Size)
	{
		byte cur = Data[dpos];
		dpos++;
		if (bfpos > 125)
		{
			if (ctype)
			{
				bfpos |= 128;
				xBlockWrite( SB, &bfpos, 1 );
				xBlockWrite( SB, bff, 1 );
				bfpos = 0;
			}
			else
			{
				xBlockWrite( SB, &bfpos, 1 );
				xBlockWrite( SB, bff, bfpos );
				bfpos = 0;
			};
		};
		if (ctype&&pre == cur)
		{
			bfpos++;
		}
		else
		{
			if (ctype)
			{
				bfpos |= 128;
				xBlockWrite( SB, &bfpos, 1 );
				xBlockWrite( SB, bff, 1 );
				bfpos = 0;
				ctype = 0;
			};
			if (( !ctype ) && dpos < Size - 3)
			{
				if (cur == Data[dpos] && cur == Data[dpos + 1])
				{
					if (bfpos)
					{
						xBlockWrite( SB, &bfpos, 1 );
						xBlockWrite( SB, bff, bfpos );
						bfpos = 0;
					};
					ctype = 1;
					bff[0] = cur;
					bfpos = 3;
					dpos += 2;
				};
			};
			if (!ctype)
			{
				bff[bfpos] = cur;
				bfpos++;
			};
		};
		pre = cur;
	};
	if (bfpos)
	{
		if (ctype)
		{
			bfpos |= 128;
			xBlockWrite( SB, &bfpos, 1 );
			xBlockWrite( SB, bff, 1 );
			bfpos = 0;
		}
		else
		{
			xBlockWrite( SB, &bfpos, 1 );
			xBlockWrite( SB, bff, bfpos );
			bfpos = 0;
		};
	};
};
void LoadRLE1( SaveBuf* SB, void* data )
{
	byte* Data = (byte*) data;
	int Size;
	xBlockRead( SB, &Size, 4 );
	int dpos = 0;
	while (dpos < Size)
	{
		byte x;
		xBlockRead( SB, &x, 1 );
		if (x & 128)
		{
			byte y;
			xBlockRead( SB, &y, 1 );
			memset( Data + dpos, y, x - 128 );
			dpos += x - 128;
		}
		else
		{
			xBlockRead( SB, Data + dpos, x );
			dpos += x;
		};
	};
};
static int sfHeader = 'FSAK';
int sfVersion;
extern char CurrentMap[64];
extern MotionField UnitsField;
extern byte* FishMap;
extern int FishLx;
void SaveMap( SaveBuf* SB )
{
	xBlockWrite( SB, &msx, 4 );
	xBlockWrite( SB, &msy, 4 );
	xBlockWrite( SB, CurrentMap, 64 );
	xBlockWrite( SB, &LightDX, 4 );
	xBlockWrite( SB, &LightDY, 4 );
	xBlockWrite( SB, &LightDZ, 4 );
	SaveRLE1( SB, UnitsField.MapV, MAPSY*BMSX );
	SaveRLE1( SB, MFIELDS[0].MapV, MAPSY*BMSX );
	SaveRLE1( SB, MFIELDS[1].MapV, MAPSY*BMSX );
	if (FishMap)
	{
		xBlockWrite( SB, &FishLx, 4 );
		SaveRLE1( SB, FishMap, FishLx*FishLx );
	}
	else
	{
		int i = 0;
		xBlockWrite( SB, &i, 4 );
	};

};
extern bool BuildMode;
void Load3DMapLandOnly( char* Map );
void LoadMap( SaveBuf* SB )
{
	xBlockRead( SB, &msx, 4 );
	xBlockRead( SB, &msy, 4 );
	xBlockRead( SB, CurrentMap, 64 );
	xBlockRead( SB, &LightDX, 4 );
	xBlockRead( SB, &LightDY, 4 );
	xBlockRead( SB, &LightDZ, 4 );
	Load3DMapLandOnly( CurrentMap );
	LoadRLE1( SB, UnitsField.MapV );
	LoadRLE1( SB, MFIELDS[0].MapV );
	LoadRLE1( SB, MFIELDS[1].MapV );
	//UnitsField.ClearMaps();
	SetLight( 0, 20, 30 );
	if (FishMap)
	{
		free( FishMap );
		FishMap = NULL;
		FishLx = 0;
	};
	xBlockRead( SB, &FishLx, 4 );
	if (FishLx)
	{
		FishMap = new byte[FishLx*FishLx];
		LoadRLE1( SB, FishMap );
	};
};
void SaveNations( SaveBuf* SB )
{

	int tt = 0x4954414E;//'NATI'
	xBlockWrite( SB, &tt, 4 );
	SAVMES( SB, " NATSAVE" );
	byte nn = 8;
	xBlockWrite( SB, &nn, 1 );
	for (tt = 0; tt < 8; tt++)
	{
		SAVMES1( SB, " NATION: ", tt );
		Nation* NT = &NATIONS[tt];
		//RBlockWrite(ff1,NT->SCRIPT,16);
		byte mm = NT->NNUM | ( NT->VictState << 4 );
		xBlockWrite( SB, &NT->NMask, 1 );
		xBlockWrite( SB, &mm, 1 );
		SAVMES1( SB, " MONSAMOUNT :", NT->NMon );
		xBlockWrite( SB, &NT->NMon, 4 );
		xBlockWrite( SB, &NT->Harch, 2 );
		xBlockWrite( SB, &NT->SharStage, int( &NT->UID_PEASANT ) - int( &NT->SharStage ) );
		//Informtion about monsters(GeneralObject)
		word NMON = 0;
		for (int i = 0; i < NT->NMon; i++)
		{
			GeneralObject* GO = NT->Mon[i];
			if (GO->MoreCharacter->Changed)NMON++;
		};
		xBlockWrite( SB, &NMON, 2 );
		for (int i = 0; i < NT->NMon; i++)
		{
			GeneralObject* GO = NT->Mon[i];
			if (GO->MoreCharacter->Changed)
			{
				SAVMES1( SB, " NMONSTER:", i );
				xBlockWrite( SB, &i, 2 );
				byte nn = 0;
				if (GO->Enabled)nn = nn | 1;
				if (GO->CondEnabled)nn = nn | 2;
				if (GO->ManualDisable)nn |= 4;
				xBlockWrite( SB, &nn, 1 );
				xBlockWrite( SB, GO->MoreCharacter, sizeof AdvCharacter );
			};
		};

		//Information about city
		SAVMES( SB, "CITYINFO" );
		City* CT = &CITY[tt];
		xBlockWrite( SB, CT->UnitAmount, NT->NMon * 2 );
		xBlockWrite( SB, CT->ReadyAmount, NT->NMon * 2 );
		xBlockWrite( SB, &CT->NGroups, 4 );
		if (CT->NGroups)
		{
			xBlockWrite( SB, CT->NGroupsInSet, CT->NGroups * 2 );
			for (int i = 0; i < CT->NGroups; i++)
			{
				xBlockWrite( SB, CT->GroupsSet[i], CT->NGroupsInSet[i] * 2 );
			};
		};

		//RBlockWrite(SB,&NT->NGidot,4);
		//RBlockWrite(SB,&NT->NFarms,4);

		//Upgrades information
		SAVMES( SB, "UPGRADEINFO" );
		int NUPG = -NT->NUpgrades;
		xBlockWrite( SB, &NUPG, 4 );
		for (int i = 0; i < NT->NUpgrades; i++)
		{
			NewUpgrade* NU = NT->UPGRADE[i];
			byte x = 0;
			if (NU->Done)x |= 1;
			if (NU->Enabled)x |= 2;
			if (NU->PermanentEnabled)x |= 4;
			if (NU->IsDoing)x |= 8;
			if (NU->ManualDisable)x |= 16;
			xBlockWrite( SB, &x, 1 );
			xBlockWrite( SB, &NU->CurStage, 2 );
		};
		//end of upgrade information
		xBlockWrite( SB, &NtNUnits[tt], 4 );
		xBlockWrite( SB, &NtMaxUnits[tt], 4 );
		xBlockWrite( SB, NatList[tt], 2 * NtNUnits[tt] );
		xBlockWrite( SB, NT->NKilled, NT->NMon * 2 );
		xBlockWrite( SB, NT->ResTotal, 32 );
		xBlockWrite( SB, NT->ResOnMines, 32 );
		xBlockWrite( SB, NT->ResOnBuildings, 32 );
		xBlockWrite( SB, NT->ResOnUpgrade, 32 );
		xBlockWrite( SB, NT->ResOnUnits, 32 );
		xBlockWrite( SB, NT->ResOnLife, 32 );
		xBlockWrite( SB, &NT->NHistory, 4 );
		for (int i = 0; i < NT->NHistory; i++)
		{
			int L = strlen( NT->History[i] ) + 1;
			xBlockWrite( SB, &L, 4 );
			xBlockWrite( SB, NT->History[i], L );
		};
		xBlockWrite( SB, &NT->NPopul, 4 );
		if (NT->NPopul)
		{
			xBlockWrite( SB, NT->Popul, 2 * NT->NPopul );
		};
		xBlockWrite( SB, &NT->NUpgMade, 4 );
		if (NT->NUpgMade)
		{
			xBlockWrite( SB, NT->UpgIDS, 2 * NT->NUpgMade );
			xBlockWrite( SB, NT->UpgTime, 4 * NT->NUpgMade );
		};
		//New information
		xBlockWrite( SB, NT->ResBuy, 32 );
		xBlockWrite( SB, NT->ResSell, 32 );
		xBlockWrite( SB, NT->NProduced, NT->NMon * 2 );
		xBlockWrite( SB, &NT->NAccount, 4 );
		if (NT->NAccount)
		{
			xBlockWrite( SB, NT->Account, 2 * NT->NAccount );
		};
		xBlockWrite( SB, &NT->ThereWasUnit, 1 );
	};
}

bool LoadNations( SaveBuf* SB )
{
	LoadAllNations( 0 );
	LoadAllNations( 1 );
	LoadAllNations( 2 );
	LoadAllNations( 3 );
	LoadAllNations( 4 );
	LoadAllNations( 5 );
	LoadAllNations( 6 );
	LoadAllNations( 7 );

	int z;
	xBlockRead( SB, &z, 4 );
	if (z != 0x4954414E)
	{
		LOutErr( "Save file is corrupted." );
	}

	LOADMES( SB );
	byte nn;
	xBlockRead( SB, &nn, 1 );
	for (int tt = 0; tt < 8; tt++)
	{
		LOADMES( SB );
		Nation* NT = &NATIONS[tt];
		xBlockRead( SB, &NT->NMask, 1 );
		byte mm;
		xBlockRead( SB, &mm, 1 );
		NT->NNUM = mm & 7;
		NT->VictState = 0;//mm>>4;
		LOADMES( SB );
		xBlockRead( SB, &NT->NMon, 4 );
		xBlockRead( SB, &NT->Harch, 2 );
		xBlockRead( SB, &NT->SharStage, int( &NT->UID_PEASANT ) - int( &NT->SharStage ) );
		//Informtion about monsters(GeneralObject)
		int NMON = 0;
		xBlockRead( SB, &NMON, 2 );
		for (int i = 0; i < NMON; i++)
		{
			LOADMES( SB );
			word ID;
			xBlockRead( SB, &ID, 2 );
			GeneralObject* GO = NT->Mon[ID];
			//LOADMES(SB);
			byte nn;
			xBlockRead( SB, &nn, 1 );
			GO->Enabled = ( nn & 1 ) != 0;
			GO->CondEnabled = ( nn & 2 ) != 0;
			GO->ManualDisable = ( nn & 4 ) != 0;
			xBlockRead( SB, GO->MoreCharacter, sizeof AdvCharacter );
		};

		//Information about city
		LOADMES( SB );
		City* CT = &CITY[tt];
		xBlockRead( SB, CT->UnitAmount, NT->NMon * 2 );
		xBlockRead( SB, CT->ReadyAmount, NT->NMon * 2 );
		xBlockRead( SB, &CT->NGroups, 4 );
		if (CT->NGroups)
		{
			CT->NGroupsInSet = new word[CT->NGroups];
			CT->GroupsSet = (word**) malloc( CT->NGroups * 4 );
			xBlockRead( SB, CT->NGroupsInSet, CT->NGroups * 2 );
			for (int i = 0; i < CT->NGroups; i++)
			{
				CT->GroupsSet[i] = new word[CT->NGroupsInSet[i] * 2];
				xBlockRead( SB, CT->GroupsSet[i], CT->NGroupsInSet[i] * 2 );
			};
		};

		//RBlockRead(SB,&NT->NGidot,4);
		//RBlockRead(SB,&NT->NFarms,4);

		//Upgrades information
		LOADMES( SB );
		int NUPG;

		xBlockRead( SB, &NUPG, 4 );
		NT->NUpgrades = abs( NUPG );
		for (int i = 0; i < NT->NUpgrades; i++)
		{
			NewUpgrade* NU = NT->UPGRADE[i];
			byte x;
			xBlockRead( SB, &x, 1 );
			NU->Done = ( x & 1 ) != 0;
			NU->Enabled = ( x & 2 ) != 0;
			NU->PermanentEnabled = ( x & 4 ) != 0;
			NU->IsDoing = ( x & 8 ) != 0;
			NU->ManualDisable = ( x & 16 ) != 0;
			xBlockRead( SB, &NU->CurStage, 2 );
		};
		//end of upgrade information
		free( NatList[tt] );
		xBlockRead( SB, &NtNUnits[tt], 4 );
		xBlockRead( SB, &NtMaxUnits[tt], 4 );
		NatList[tt] = new word[NtMaxUnits[tt]];
		xBlockRead( SB, NatList[tt], 2 * NtNUnits[tt] );
		xBlockRead( SB, NT->NKilled, NT->NMon * 2 );
		xBlockRead( SB, NT->ResTotal, 32 );
		xBlockRead( SB, NT->ResOnMines, 32 );
		xBlockRead( SB, NT->ResOnBuildings, 32 );
		xBlockRead( SB, NT->ResOnUpgrade, 32 );
		xBlockRead( SB, NT->ResOnUnits, 32 );
		xBlockRead( SB, NT->ResOnLife, 32 );
		xBlockRead( SB, &NT->NHistory, 4 );
		NT->History = (char**) malloc( NT->NHistory * 4 );
		for (int i = 0; i < NT->NHistory; i++)
		{
			int L;
			xBlockRead( SB, &L, 4 );
			NT->History[i] = new char[L];
			xBlockRead( SB, NT->History[i], L );
		};
		xBlockRead( SB, &NT->NPopul, 4 );
		if (NT->NPopul)
		{
			NT->Popul = new word[NT->NPopul];
			xBlockRead( SB, NT->Popul, 2 * NT->NPopul );
			NT->MaxPopul = NT->NPopul;
		};
		xBlockRead( SB, &NT->NUpgMade, 4 );
		if (NT->NUpgMade)
		{
			NT->UpgIDS = new word[NT->NUpgMade];
			NT->UpgTime = new int[NT->NUpgMade];
			xBlockRead( SB, NT->UpgIDS, 2 * NT->NUpgMade );
			xBlockRead( SB, NT->UpgTime, 4 * NT->NUpgMade );
			NT->MaxUpgMade = NT->NUpgMade;
		};
		if (NUPG <= 0)
		{
			xBlockRead( SB, NT->ResBuy, 32 );
			xBlockRead( SB, NT->ResSell, 32 );
			xBlockRead( SB, NT->NProduced, NT->NMon * 2 );
			xBlockRead( SB, &NT->NAccount, 4 );
			if (NT->NAccount)
			{
				NT->Account = new word[NT->NAccount];
				xBlockRead( SB, NT->Account, 2 * NT->NAccount );
			};
			NT->MaxAccount = NT->NAccount;
		};
		xBlockRead( SB, &NT->ThereWasUnit, 1 );
	};
	return true;
};
void BuildObjLink( OneObject* OB );
void ProduceObjLink( OneObject* OB );
void AttackObjLink( OneObject* OB );
void TakeResLink( OneObject* OB );
void PerformUpgradeLink( OneObject* OB );
void GoToMineLink( OneObject* OB );
void LeaveMineLink( OneObject* OB );
void TakeResourceFromSpriteLink( OneObject* OBJ );
void NewMonsterSendToLink( OneObject* OB );
void DeleteBlockLink( OneObject* OBJ );
void SetUnlimitedLink( OneObject* OB );
void ClearUnlimitedLink( OneObject* OB );
void WaterNewMonsterSendToLink( OneObject* OB );
void AbsorbObjectLink( OneObject* OB );
void LeaveShipLink( OneObject* OB );
void BuildWallLink( OneObject* OB );
void WaterAttackLink( OneObject* OBJ );
void LeaveTransportLink( OneObject* OB );
void GoToTransportLink( OneObject* OBJ );
void FishingLink( OneObject* OBJ );
void CreateGatesLink( OneObject* OBJ );
void NewMonsterSmartSendToLink( OneObject* OBJ );
void AI_AttackPointLink( OneObject* OBJ );
void GrenaderSupermanLink( OneObject* OBJ );
void GotoFinePositionLink( OneObject* OB );
void NewMonsterPreciseSendToLink( OneObject* OB );
void RotUnitLink( OneObject* OB );
void PatrolLink( OneObject* OBJ );
void RotateShipAndDieLink( OneObject* OBJ );
void SearchArmyLink( OneObject* OBJ );
void NewAttackPointLink( OneObject* OBJ );
void MakeOneShotLink( OneObject* OBJ );
#define OrdCount 32
ReportFn* ORDLIST[OrdCount] = {
	&BuildObjLink,
	&ProduceObjLink,
	&AttackObjLink,
	&TakeResLink,
	&PerformUpgradeLink,
	&GoToMineLink,
	&LeaveMineLink,
	&TakeResourceFromSpriteLink,
	&NewMonsterSendToLink,
	&DeleteBlockLink,
	&SetUnlimitedLink,
	&ClearUnlimitedLink,
	&WaterNewMonsterSendToLink,
	&AbsorbObjectLink,
	&LeaveShipLink,
	&BuildWallLink,
	&WaterAttackLink,
	&LeaveTransportLink,
	&GoToTransportLink,
	&FishingLink,
	&CreateGatesLink,
	&NewMonsterSmartSendToLink,
	&AI_AttackPointLink,
	&GrenaderSupermanLink,
	&GotoFinePositionLink,
	&NewMonsterPreciseSendToLink,
	&RotUnitLink,
	&PatrolLink,
	&RotateShipAndDieLink,
	&SearchArmyLink,
	&NewAttackPointLink,
	&MakeOneShotLink
};
#ifdef STARFORCE
ReportFn** PTR_LAND = NULL;
void __declspec( dllexport ) SFINIT2_InitLAND()
{
	PTR_LAND = (ReportFn**) GlobalAlloc( GMEM_FIXED, 4 * OrdCount );
	memcpy( PTR_LAND, ORDLIST, 4 * OrdCount );
};
void FREELAND()
{
	if (PTR_LAND)GlobalFree( PTR_LAND );
};
#endif
int GetOrderKind( ReportFn* RF )
{
#ifdef STARFORCE
	if (PTR_LAND)
	{
		for (int i = 0; i < OrdCount; i++)if (RF == PTR_LAND[i])return i;
	};
	return -1;
#endif
	for (int i = 0; i < OrdCount; i++)if (RF == ORDLIST[i])return i;
	return -1;
};
ReportFn* GetOrderRef( int i )
{
	if (i == -1 || i >= OrdCount)return NULL;
	return ORDLIST[i];
};
void SaveObjects( SaveBuf* SB )
{
	int tt = 'SJBO';
	xBlockWrite( SB, &tt, 4 );
	int NObjects = 0;
	SAVMES( SB, " OBJECTSINFO" );
	for (int i = 0; i < MAXOBJECT; i++)if (Group[i])NObjects++;
	xBlockWrite( SB, &NObjects, 4 );
	xBlockWrite( SB, &MAXOBJECT, 4 );
	//Information for every object
	for (tt = 0; tt < MAXOBJECT; tt++)
	{
		OneObject* OB = Group[tt];
		if (OB)
		{
			SAVMES1( SB, " OBJECT:", OB->Index );
			xBlockWrite( SB, &OB->Index, 2 );
			xBlockWrite( SB, &OB->NeedPath, int( &OB->Inside ) - int( &OB->NeedPath ) );
			if (OB->NInside)
			{
				xBlockWrite( SB, OB->Inside, OB->NInside << 1 );
			};
			//saving index of selection group
			int pp;
			if (OB->GroupIndex)pp = div( int( OB->GroupIndex ) - int( SelSet ), sizeof SelGroup ).quot;
			else pp = 255;
			xBlockWrite( SB, &pp, 1 );
			SAVMES( SB, " ORDERINFO" );
			//Order information saving
			//1. calculating orders
			pp = 0;
			Order1* ORDR = OB->LocalOrder;
			while (ORDR)
			{
				pp++;
				ORDR = ORDR->NextOrder;
			};
			xBlockWrite( SB, &pp, 2 );
			if (pp)
			{
				//2.saving order information
				Order1 ORD1;
				ORDR = OB->LocalOrder;
				ORD1 = *OB->LocalOrder;
				while (ORDR)
				{
					ORD1.NextOrder = NULL;
					ORD1.DoLink = (ReportFn*) ( GetOrderKind( ORDR->DoLink ) );
					//assert(int(ORD1.DoLink)!=-1);
					xBlockWrite( SB, &ORD1, sizeof ORD1 );
					ORDR = ORDR->NextOrder;
					if (ORDR)ORD1 = *ORDR;
				};
			};
			//NewAnimations
			if (OB->HiLayer)pp = int( OB->HiLayer ) - int( OB->newMons );
			else pp = -1;
			xBlockWrite( SB, &pp, 4 );
			if (OB->NewAnm)pp = int( OB->NewAnm ) - int( OB->newMons );
			else pp = -1;
			xBlockWrite( SB, &pp, 4 );
			if (OB->LoLayer)pp = int( OB->LoLayer ) - int( OB->newMons );
			else pp = -1;
			xBlockWrite( SB, &pp, 4 );
		};
	};
};
void CreateGround( OneObject* G );
void LoadObjects( SaveBuf* SB )
{
	int tt;
	xBlockRead( SB, &tt, 4 );
	if (tt != 'SJBO')LOutErr( "Save file is corrupted." );;
	int NObjects = 0;
	LOADMES( SB );
	xBlockRead( SB, &NObjects, 4 );
	xBlockRead( SB, &MAXOBJECT, 4 );
	//Information for every object
	for (tt = 0; tt < NObjects; tt++)
	{
		LOADMES( SB );
		word IND;
		xBlockRead( SB, &IND, 2 );
		OneObject* OB = OBJECTS + IND;
		Group[IND] = OB;
		memset( OB, 0, sizeof OneObject );
		OB->Index = IND;
		xBlockRead( SB, &OB->NeedPath, int( &OB->Inside ) - int( &OB->NeedPath ) );
		Nation* NT = &NATIONS[OB->NNUM];
		OB->Nat = NT;
		OB->Ref.General = NT->Mon[OB->NIndex];
		OB->newMons = OB->Ref.General->newMons;

		if (OB->NInside)
		{
			OB->Inside = new word[OB->NInside];
			xBlockRead( SB, OB->Inside, OB->NInside << 1 );
		};
		//index of selection group
		int pp = 0;
		xBlockRead( SB, &pp, 1 );
		if (pp != 255)OB->GroupIndex = SelSet + pp;
		else OB->GroupIndex = NULL;
		LOADMES( SB );
		//Order information
		pp = 0;
		xBlockRead( SB, &pp, 2 );
		Order1* PRE = NULL;
		Order1* OR1;
		for (int j = 0; j < pp; j++)
		{
			OR1 = GetOrdBlock();
			if (!j)OB->LocalOrder = OR1;
			xBlockRead( SB, OR1, sizeof Order1 );
			OR1->DoLink = GetOrderRef( int( OR1->DoLink ) );
			if (PRE)PRE->NextOrder = OR1;
			PRE = OR1;
		};
		//NewAnimations
		xBlockRead( SB, &pp, 4 );
		if (pp != -1)OB->HiLayer = (NewAnimation*) ( int( OB->newMons ) + pp );
		xBlockRead( SB, &pp, 4 );
		if (pp != -1)OB->NewAnm = (NewAnimation*) ( int( OB->newMons ) + pp );
		xBlockRead( SB, &pp, 4 );
		if (pp != -1)OB->LoLayer = (NewAnimation*) ( int( OB->newMons ) + pp );
		if (OB->InFire)CreateGround( OB );
		OB->ImSelected = OB->Selected;
	};

};
void SaveSelection( SaveBuf* SB )
{
	int i = 'SELI';
	xBlockWrite( SB, &i, 4 );
	SAVMES( SB, " SELECTINFO" );
	for (int i = 0; i < 80; i++)
	{
		SelGroup* SG = &SelSet[i];
		xBlockWrite( SB, &SG->NMemb, ( sizeof SelGroup ) - 8 );
		if (SG->NMemb)
		{
			xBlockWrite( SB, SG->Member, SG->NMemb << 1 );
			xBlockWrite( SB, SG->SerialN, SG->NMemb << 1 );
		};
	};
	/*
	for(i=0;i<8;i++){
		xBlockWrite(SB,&NSL[i],2);
		if(NSL[i]){
			xBlockWrite(SB,Selm[i],NSL[i]<<1);
			xBlockWrite(SB,SerN[i],NSL[i]<<1);
		};
	};
	*/
};
void LoadSelection( SaveBuf* SB )
{
	int i;
	xBlockRead( SB, &i, 4 );
	LOADMES( SB );
	if (i != 'SELI')LOutErr( "Save file is corrupted." );
	for (int i = 0; i < 80; i++)
	{
		SelGroup* SG = &SelSet[i];
		xBlockRead( SB, &SG->NMemb, ( sizeof SelGroup ) - 8 );
		if (SG->NMemb)
		{
			SG->Member = new word[SG->NMemb];
			SG->SerialN = new word[SG->NMemb];
			xBlockRead( SB, SG->Member, SG->NMemb << 1 );
			xBlockRead( SB, SG->SerialN, SG->NMemb << 1 );
		}
		else
		{
			SG->Member = NULL;
			SG->SerialN = NULL;
		};
	};
	/*
	for(i=0;i<8;i++){
		xBlockRead(SB,&NSL[i],2);
		SelCenter[i]=0;
		if(NSL[i]){
			Selm[i]=new word[NSL[i]];
			SerN[i]=new word[NSL[i]];
			xBlockRead(SB,Selm[i],NSL[i]<<1);
			xBlockRead(SB,SerN[i],NSL[i]<<1);
			ImSelm[i]=new word[NSL[i]];
			ImSerN[i]=new word[NSL[i]];
			memcpy(ImSelm[i],Selm[i],NSL[i]*2);
			memcpy(ImSerN[i],SerN[i],NSL[i]*2);
		}else{
			Selm[i]=NULL;
			SerN[i]=NULL;
		};
		ImNSL[i]=NSL[i];
	};
	*/
	//setting the selection
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB)
		{
			OB->Selected = 0;
			OB->ImSelected = 0;
		};
	};
	for (int NI = 0; NI < 8; NI++)
	{

	};
};
void SaveWalls( SaveBuf* SB )
{
	int i = 'LLAW';
	xBlockWrite( SB, &i, 4 );
	SAVMES( SB, " WALLSINFO" );
	xBlockWrite( SB, &WSys.NClusters, 4 );
	for (int i = 0; i < WSys.NClusters; i++)
	{
		WallCluster* WCL = WSys.WCL[i];
		xBlockWrite( SB, &WCL->Type, 1 );
		xBlockWrite( SB, &WCL->NIndex, 2 );
		xBlockWrite( SB, &WCL->NI, 1 );
		xBlockWrite( SB, &WCL->NM->MyIndex, 2 );
		xBlockWrite( SB, &WCL->NCells, 4 );
		xBlockWrite( SB, WCL->Cells, int( WCL->NCells ) * sizeof( WallCell ) );
	};
	xBlockWrite( SB, &NGates, 4 );
	xBlockWrite( SB, &MaxGates, 4 );
	xBlockWrite( SB, Gates, NGates * sizeof Gate );
	SAVMES( SB, " RESOURCES" );
	int RRR[8][8];
	for (int p = 0; p < 8; p++)
	{
		for (int q = 0; q < 8; q++)
		{
			RRR[p][q] = XRESRC( p, q );
		};
	};
	xBlockWrite( SB, &RRR[0][0], sizeof RRR );
	xBlockWrite( SB, &RESADD[0][0], sizeof RESADD );
};
void SetTexturedRound( int x, int y, int rx, byte Tex );
extern word TexList[128];
extern word NTextures;
void LoadWalls( SaveBuf* SB )
{
	int i;
	xBlockRead( SB, &i, 4 );
	if (i != 'LLAW')LOutErr( "Save file is corrupted." );
	LOADMES( SB );
	xBlockRead( SB, &WSys.NClusters, 4 );
	WSys.WCL = (WallCluster**) malloc( 4 * WSys.NClusters );
	for (int i = 0; i < WSys.NClusters; i++)
	{
		WallCluster* WCL = new WallCluster;
		WSys.WCL[i] = WCL;
		xBlockRead( SB, &WCL->Type, 1 );
		xBlockRead( SB, &WCL->NIndex, 2 );
		xBlockRead( SB, &WCL->NI, 1 );
		word MIND;
		xBlockRead( SB, &MIND, 2 );
		WCL->NM = &NewMon[MIND];
		xBlockRead( SB, &WCL->NCells, 4 );
		WCL->Cells = new WallCell[WCL->NCells];
		xBlockRead( SB, WCL->Cells, int( WCL->NCells ) * sizeof( WallCell ) );
		for (int j = 0; j < WCL->NCells; j++)
		{
			WallCell* WCX = &WCL->Cells[j];
			int LI = GetLI( WCX->x, WCX->y );
			if (WCX->Visible&&LI < MaxLI)
			{
				WRefs[LI] = WCX;
				WallCharacter* WCHAR = &WChar[WCX->Type];
				if (WCHAR->NTex)
				{
					//memcpy(TexList,WCHAR->Tex,WCHAR->NTex*2);
					//NTextures=WCHAR->NTex;
					//int xx=(int(WCX->x)<<6)+32+((int(rand()-16384)*WCHAR->TexPrec)>>14);
					//int yy=(int(WCX->y)<<6)+32+((int(rand()-16384)*WCHAR->TexPrec)>>14);
					//SetTexturedRound(xx,(yy>>1)-GetHeight(xx,yy),WCHAR->TexRadius,0);
				};
			};
			//assert(LI<MaxLI);
		};
	};
	free( Gates );
	xBlockRead( SB, &NGates, 4 );
	xBlockRead( SB, &MaxGates, 4 );
	Gates = new Gate[MaxGates];
	xBlockRead( SB, Gates, NGates * sizeof Gate );
	LOADMES( SB );
	int RRR[8][8];
	xBlockRead( SB, &RRR[0][0], sizeof RRR );
	for (int p = 0; p < 8; p++)
	{
		for (int q = 0; q < 8; q++)
		{
			SetXRESRC( p, q, RRR[p][q] );
		};
	};

	for (int i = 0; i < 8; i++)
	{
		Nation* NT = NATIONS + i;
		NT->SELO.Init();
		NT->ARMY.Init();
		NT->SCIENCE.Init();
		for (int j = 0; j < 8; j++)
		{
			NT->GENERAL.RESAM[j] = XRESRC( i, j );
			NT->GENERAL.RESRM[j] = 0;
		};
	};
	xBlockRead( SB, &RESADD[0][0], sizeof RESADD );
};
//extern Nation WEP;
#define MaxExpl 8192
void SaveAnmObj( SaveBuf* SB )
{
	int i = 'OMNA';
	xBlockWrite( SB, &i, 4 );
	int NExplosions = 0;
	for (int i = 0; i < MaxExpl; i++)
	{
		if (EUsage[i])NExplosions++;
	};
	xBlockWrite( SB, &NExplosions, 4 );
	for (int i = 0; i < MaxExpl; i++)
	{
		if (EUsage[i])
		{
			xBlockWrite( SB, &i, 2 );
			AnmObject NAN = *( GAnm[i] );
			if (NAN.Sender)NAN.Sender = (OneObject*) ( NAN.Sender->Index );
			else NAN.Sender = (OneObject*) 0xFFFFFFFF;
			NAN.Weap = (Weapon*) NAN.Weap->MyIndex;
			xBlockWrite( SB, &NAN.x, sizeof( AnmObject ) - 4 );
		};
	};
};
void LoadAnmObj( SaveBuf* SB )
{
	CloseExplosions();
	InitExplosions();
	int i;
	xBlockRead( SB, &i, 4 );
	if (i != 'OMNA')LOutErr( "Save file is corrupted." );
	int NExplosions;
	xBlockRead( SB, &NExplosions, 4 );
	memset( EUsage, 0, sizeof EUsage );
	for (int i = 0; i < NExplosions; i++)
	{
		word ai;
		xBlockRead( SB, &ai, 2 );
		EUsage[ai] = 1;
		AnmObject* NAN = GAnm[ai];
		xBlockRead( SB, &NAN->x, sizeof( AnmObject ) - 4 );
		if (int( NAN->Sender ) != -1)NAN->Sender = Group[int( NAN->Sender )];
		else NAN->Sender = NULL;
		NAN->Weap = WPLIST[int( NAN->Weap )];
		NAN->NewAnm = NAN->Weap->NewAnm;
	};
};
extern int MAXSPR;
void SaveSprites( SaveBuf* SB )
{
	int i = 'IRPS';
	xBlockWrite( SB, &i, 4 );
	int NSprt = 0;
	xBlockWrite( SB, &MAXSPR, 4 );
	for (int i = 0; i < MaxSprt; i++)if (Sprites[i].Enabled)NSprt++;
	xBlockWrite( SB, &NSprt, 4 );
	for (int i = 0; i < MaxSprt; i++)
	{
		if (Sprites[i].Enabled)
		{
			xBlockWrite( SB, &i, 4 );
			OneSprite OS = Sprites[i];
			xBlockWrite( SB, &OS, int( &OS.SG ) - int( &OS ) );
			byte typ = 0;
			if (OS.SG == &STONES)typ = 1;
			if (OS.SG == &HOLES)typ = 2;
			if (OS.SG == &COMPLEX)typ = 3;
			xBlockWrite( SB, &typ, 1 );
			xBlockWrite( SB, &OS.Index, int( &OS.Damage ) - int( &OS.Index ) + 1 );
		};
	};
	//timer
	xBlockWrite( SB, &ObjTimer.NMembers, 8 );
	if (ObjTimer.NMembers)
	{
		xBlockWrite( SB, ObjTimer.IDS, 4 * ObjTimer.NMembers );
		xBlockWrite( SB, ObjTimer.Kinds, ObjTimer.NMembers );
	};
	//sprite references
	NSprt = 0;
	for (int i = 0; i < VAL_SPRSIZE; i++)if (NSpri[i])NSprt++;
	xBlockWrite( SB, &NSprt, 4 );
	for (int i = 0; i < VAL_SPRSIZE; i++)if (NSpri[i])
	{
		xBlockWrite( SB, &i, 4 );
		xBlockWrite( SB, &NSpri[i], 1 );
		xBlockWrite( SB, SpRefs[i], int( NSpri[i] ) << 2 );
	};
};
void LoadSprites( SaveBuf* SB )
{
	int i;
	xBlockRead( SB, &i, 4 );
	if (i != 'IRPS')LOutErr( "Save file is corrupted." );
	xBlockRead( SB, &MAXSPR, 4 );
	MaxSprt = ( ( MAXSPR >> 15 ) + 1 ) << 15;
	Sprites = new OneSprite[MaxSprt];
	memset( Sprites, 0, MaxSprt * sizeof OneSprite );
	int NSprt;
	xBlockRead( SB, &NSprt, 4 );
	for (int i = 0; i < NSprt; i++)
	{
		int spid;
		xBlockRead( SB, &spid, 4 );
		OneSprite* OS = &Sprites[spid];
		xBlockRead( SB, OS, int( &OS->SG ) - int( OS ) );
		byte typ;
		xBlockRead( SB, &typ, 1 );
		switch (typ)
		{
		case 0:
			OS->SG = &TREES;
			break;
		case 1:
			OS->SG = &STONES;
			break;
		case 2:
			OS->SG = &HOLES;
			break;
		case 3:
			OS->SG = &COMPLEX;
			break;
		};
		xBlockRead( SB, &OS->Index, int( &OS->Damage ) - int( &OS->Index ) + 1 );
		OS->OC = &OS->SG->ObjChar[OS->SGIndex];
	};
	//timer
	xBlockRead( SB, &ObjTimer.NMembers, 8 );
	if (ObjTimer.MaxMembers)
	{
		ObjTimer.IDS = new int[ObjTimer.MaxMembers << 1];
		ObjTimer.Kinds = new byte[ObjTimer.MaxMembers];
	};
	if (ObjTimer.NMembers)
	{
		xBlockRead( SB, ObjTimer.IDS, 4 * ObjTimer.NMembers );
		xBlockRead( SB, ObjTimer.Kinds, ObjTimer.NMembers );
	};
	//sprite references
	xBlockRead( SB, &NSprt, 4 );
	for (int i = 0; i < NSprt; i++)
	{
		int p;
		xBlockRead( SB, &p, 4 );
		xBlockRead( SB, &NSpri[p], 1 );
		int  N1 = NSpri[p];
		if (N1)
		{
			N1 = ( N1 | 15 ) + 1;
			SpRefs[p] = new int[N1];
			xBlockRead( SB, SpRefs[p], int( NSpri[p] ) << 2 );
		}
		else SpRefs[p] = NULL;
	};
};
void Save3DBars( SaveBuf* SB )
{
	int i = 'RAB3';
	xBlockWrite( SB, &i, 4 );
	xBlockWrite( SB, &NBars, 4 );
	int NB = 0;
	for (int i = 0; i < NBars; i++)if (OBARS[i])NB++;
	xBlockWrite( SB, &NB, 2 );
	for (int i = 0; i < NBars; i++)
	{
		OneBar* ONB = OBARS[i];
		if (ONB)
		{
			xBlockWrite( SB, &i, 2 );
			xBlockWrite( SB, ONB, sizeof( OneBar ) );
		};
	};
	NB = 0;
	for (int i = 0; i < B3SZ; i++)if (NObj3[i])NB++;
	xBlockWrite( SB, &NB, 4 );
	for (int i = 0; i < B3SZ; i++)if (NObj3[i])
	{
		xBlockWrite( SB, &i, 4 );
		xBlockWrite( SB, &NObj3[i], 2 );
		xBlockWrite( SB, Obj3Map[i], NObj3[i] << 1 );
	};
};

void Load3DBars( SaveBuf* SB )
{
	int i;
	xBlockRead( SB, &i, 4 );
	if (i != 'RAB3')LOutErr( "Save file is corrupted." );
	xBlockRead( SB, &NBars, 4 );
	int NB = 0;
	xBlockRead( SB, &NB, 2 );
	for (int i = 0; i < NB; i++)
	{
		word id;
		xBlockRead( SB, &id, 2 );
		OneBar* ONB = new OneBar;
		OBARS[id] = ONB;
		xBlockRead( SB, ONB, sizeof( OneBar ) );
	};
	xBlockRead( SB, &NB, 4 );
	for (int i = 0; i < NB; i++)
	{
		int zz;
		xBlockRead( SB, &zz, 4 );
		xBlockRead( SB, &NObj3[zz], 2 );
		Obj3Map[zz] = new word[NObj3[zz]];
		xBlockRead( SB, Obj3Map[zz], NObj3[zz] << 1 );
	};
};
extern bool NeedProcessTop;
extern bool WasOnlyOpen;
extern word NChAreas;
extern word MaxChAreas;
extern word* ChAreas;
//procesing variables
extern int CurIStart;
extern int TmpChanges;
extern word* TmpMLinks;
extern word* TmpMDist;
extern int MinChX;
extern int MaxChX;
extern int MinChY;
extern int MaxChY;
void LS_SaveTopology( SaveBuf* SB )
{
	xBlockWrite( SB, &NAreas, 4 );
	for (int j = 0; j < NAreas; j++)
	{
		Area Ar1 = TopMap[j];
		xBlockWrite( SB, &Ar1, sizeof Area );
		Ar1.MaxLink = Ar1.NLinks;
		if (Ar1.NMines)xBlockWrite( SB, Ar1.MinesIdx, Ar1.NMines << 1 );
		if (Ar1.NLinks)xBlockWrite( SB, Ar1.Link, Ar1.NLinks << 2 );
	};
	xBlockWrite( SB, MotionLinks, NAreas*NAreas * 2 );
	xBlockWrite( SB, LinksDist, NAreas*NAreas * 2 );
	xBlockWrite( SB, TopRef, 2 * TopLx*TopLy );
	//saving dynamical topology
	xBlockWrite( SB, &NeedProcessTop, 1 );
	xBlockWrite( SB, &WasOnlyOpen, 1 );
	xBlockWrite( SB, &NChAreas, 2 );
	xBlockWrite( SB, &MaxChAreas, 2 );
	if (NChAreas)xBlockWrite( SB, ChAreas, NChAreas << 1 );
	xBlockWrite( SB, &CurIStart, 4 );
	xBlockWrite( SB, &TmpChanges, 4 );
	xBlockWrite( SB, &MinChX, 4 );
	xBlockWrite( SB, &MaxChX, 4 );
	xBlockWrite( SB, &MinChY, 4 );
	xBlockWrite( SB, &MaxChY, 4 );
	byte t = 0;
	if (TmpMLinks)
	{
		t = 1;
		xBlockWrite( SB, &t, 1 );
		xBlockWrite( SB, TmpMLinks, NAreas*NAreas * 2 );
		xBlockWrite( SB, TmpMDist, NAreas*NAreas * 2 );
	}
	else xBlockWrite( SB, &t, 1 );

};
void CreateRoadsNet();
void ClearLinkInfo();
void LS_LoadTopology( SaveBuf* SB )
{
	ClearLinkInfo();
	xBlockRead( SB, &NAreas, 4 );
	MaxArea = NAreas;
	TopMap = new Area[NAreas];
	MotionLinks = new word[NAreas*NAreas];
	LinksDist = new word[NAreas*NAreas];
	for (int j = 0; j < NAreas; j++)
	{
		Area* Ar1 = TopMap + j;
		xBlockRead( SB, Ar1, sizeof Area );
		if (Ar1->NMines)Ar1->MinesIdx = new word[Ar1->NMines];
		else Ar1->MinesIdx = NULL;
		if (Ar1->MaxLink)Ar1->Link = new word[Ar1->MaxLink << 1];
		else Ar1->Link = NULL;
		if (Ar1->NMines)xBlockRead( SB, Ar1->MinesIdx, Ar1->NMines << 1 );
		if (Ar1->NLinks)xBlockRead( SB, Ar1->Link, Ar1->NLinks << 2 );
	};
	xBlockRead( SB, MotionLinks, 2 * NAreas*NAreas );
	xBlockRead( SB, LinksDist, 2 * NAreas*NAreas );
	xBlockRead( SB, TopRef, 2 * TopLx*TopLy );
	//loading dynamical topology
	xBlockRead( SB, &NeedProcessTop, 1 );
	xBlockRead( SB, &WasOnlyOpen, 1 );
	xBlockRead( SB, &NChAreas, 2 );
	xBlockRead( SB, &MaxChAreas, 2 );
	if (MaxChAreas)ChAreas = new word[MaxChAreas];
	if (NChAreas)xBlockRead( SB, ChAreas, NChAreas << 1 );
	xBlockRead( SB, &CurIStart, 4 );
	xBlockRead( SB, &TmpChanges, 4 );
	xBlockRead( SB, &MinChX, 4 );
	xBlockRead( SB, &MaxChX, 4 );
	xBlockRead( SB, &MinChY, 4 );
	xBlockRead( SB, &MaxChY, 4 );
	byte t;
	xBlockRead( SB, &t, 1 );
	if (t)
	{
		TmpMLinks = new word[NAreas* NAreas];
		TmpMDist = new word[NAreas* NAreas];
		xBlockRead( SB, TmpMLinks, NAreas*NAreas * 2 );
		xBlockRead( SB, TmpMDist, NAreas*NAreas * 2 );
	};
	//CreateRoadsNet();
};
//--------------------------Saving AI-------------------------//
#define NLS_CFN 6
int CheckBuilders( OneObject* OB, City* CT );
int CheckFree( OneObject* OB, City* CT );
int CheckFieldWorker( OneObject* OB, City* CT );
int CheckFreeArmy( OneObject* OB, City* CT );
int CheckStoneWorker( OneObject* OB, City* CT );
int CheckWoodWorker( OneObject* OB, City* CT );
CheckWorkFn* LS_CFN[NLS_CFN] = {
	&CheckBuilders,
	&CheckFree,
	&CheckFieldWorker,
	&CheckFreeArmy,
	&CheckStoneWorker,
	&CheckWoodWorker
};
#define NBR_ORD 14
void B_LocalSendToLink( Brigade* BR );
void B_WideLocalSendToLink( Brigade* BR );
void B_GlobalSendToLink( Brigade* BR );
void B_CaptureMineLink( Brigade* BR );
void B_MakeBattleLink( Brigade* BR );
void B_KeepPositionsLink( Brigade* BR );
void HumanLocalSendToLink( Brigade* BR );
void B_HumanGlobalSendToLink( Brigade* BR );
void B_BitvaLink( Brigade* BR );
void B_LeaveAttackLink( Brigade* BR );
void HumanEscapeLink( Brigade* BR );
void LocalSendShipsLink( Brigade* BR );
void MakeShipBattleLink( Brigade* BR );
void MakeDiversionLink( Brigade* BR );
BrigadeLink* BR_ORD[NBR_ORD] = {
	&B_LocalSendToLink,
	&B_WideLocalSendToLink,
	&B_GlobalSendToLink,
	&B_CaptureMineLink,
	&B_MakeBattleLink,
	&B_KeepPositionsLink,
	&HumanLocalSendToLink,
	&B_HumanGlobalSendToLink,
	&B_BitvaLink,
	&B_LeaveAttackLink,
	&HumanEscapeLink,
	&LocalSendShipsLink,
	&MakeShipBattleLink,
	&MakeDiversionLink
};
#define NAR_ORD 6
void ArmyLocalSendToLink( AI_Army* ARM );
void ArmyConnectToArmyLink( AI_Army* ARM );
void ArmyMakeBattleLink( AI_Army* ARM );
void A_BitvaLink( AI_Army* ARM );
void A_DiversiaLink( AI_Army* ARM );
void ArmyMakeDiversiaLink( AI_Army* ARM );
ArmyLink* AR_ORD[NAR_ORD] = {
	&ArmyLocalSendToLink,
	&ArmyConnectToArmyLink,
	&ArmyMakeBattleLink,
	&A_BitvaLink,
	&A_DiversiaLink,
	&ArmyMakeDiversiaLink
};
#define N_IBR 3
void TakeNewMineBrain( Idea* ID );
void UpgradeMineBrain( Idea* ID );
void BuildWallBrain( Idea* ID );
IdeaBrain* IBR[N_IBR] = {
	&TakeNewMineBrain,
	&UpgradeMineBrain,
	&BuildWallBrain
};

char* GetPName( int i );
void SaveAI( SaveBuf* SB )
{
	//assert(_CrtCheckMemory());
	int N = 0;
	int chk = 654321;
	//for(int i=0;i<8;i++)if(NATIONS[i].AI_Enabled)N++;
	xBlockWrite( SB, &chk, 4 );
	xBlockWrite( SB, &AI_Registers, sizeof AI_Registers );
	for (int i = 1; i < 8; i++)
	{
		PINFO[i].GameTime = PINFO[0].GameTime;
		PINFO[i].VictCond = PINFO[0].VictCond;
	};
	for (int ni = 0; ni < 8; ni++)
	{
		byte pr = NATIONS[ni].AI_Enabled;
		xBlockWrite( SB, &pr, 1 );
		xBlockWrite( SB, &ni, 2 );
		//char* PN=GetPName(ni);
		xBlockWrite( SB, &PINFO[ni].ColorID, 1 );
		xBlockWrite( SB, &PINFO[ni].GameTime, 2 );
		xBlockWrite( SB, &PINFO[ni].VictCond, 1 );
		xBlockWrite( SB, &PINFO[ni].NationID, 1 );
		xBlockWrite( SB, &PINFO[ni].name, 32 );
		City* CT = CITY + ni;
		Nation* NT = NATIONS + ni;
		int CurAI = -1;
		xBlockWrite( SB, &CT->Account, 4 );
		/*
		if(NT->DLLName){
			for(int j=0;j<GlobalAI.NAi;j++){
				if(!strcmp(GlobalAI.Ai[j].LandAI[0],NT->DLLName))CurAI=j;
			};
		};
		*/
		xBlockWrite( SB, &CurAI, 4 );
		if (pr)xBlockWrite( SB, CT, sizeof City );
		//details:
		//1.Brigades
		word N = 0;
		for (int k = 0; k < MaxBrig; k++)
		{
			if (CT->Brigs[k].Enabled)N++;
		};
		xBlockWrite( SB, &N, 2 );
		for (int k = 0; k < MaxBrig; k++)
		{
			if (CT->Brigs[k].Enabled)
			{
				Brigade* BRR = CT->Brigs + k;
				xBlockWrite( SB, &k, 2 );
				xBlockWrite( SB, &BRR->NMemb, int( &BRR->BM ) - int( &BRR->NMemb ) + sizeof( BrigMemb ) );
				xBlockWrite( SB, BRR->Memb, BRR->NMemb * 2 );
				xBlockWrite( SB, BRR->MembSN, BRR->NMemb * 2 );
				xBlockWrite( SB, &BRR->SN, 2 );
				xBlockWrite( SB, &BRR->ErasureTime, 2 );
				if (BRR->posX)
				{
					xBlockWrite( SB, &BRR->MaxMemb, 2 );
					xBlockWrite( SB, BRR->posX, BRR->NMemb * 4 );
					xBlockWrite( SB, BRR->posY, BRR->NMemb * 4 );
				}
				else
				{
					word r = 0;
					xBlockWrite( SB, &r, 2 );
				};
				chk = 1234500;
				xBlockWrite( SB, &chk, 4 );
				if (BRR->CFN)
				{
					int j;
					for (j = 0; j < NLS_CFN&&BRR->CFN != LS_CFN[j]; j++);
					//assert(j<NLS_CFN);
					xBlockWrite( SB, &j, 1 );
				}
				else
				{
					byte c = 0xFF;
					xBlockWrite( SB, &c, 1 );
				};
				chk = 1234501;
				xBlockWrite( SB, &chk, 4 );
				if (BRR->BOrder)
				{
					//enumerating orders
					BrigadeOrder* BOR = BRR->BOrder;
					N = 0;
					while (BOR)
					{
						N++;
						BOR = BOR->Next;
					};
					xBlockWrite( SB, &N, 2 );
					BOR = BRR->BOrder;
					while (BOR)
					{
						xBlockWrite( SB, &BOR->Size, 2 );
						BrigadeOrder BO1 = *BOR;
						BO1.Message = NULL;
						BO1.Next = NULL;
						int j;
						for (j = 0; j < NBR_ORD&&BO1.BLink != BR_ORD[j]; j++);
						//assert(j<NBR_ORD);
						BO1.BLink = (BrigadeLink*) j;
						xBlockWrite( SB, &BO1, sizeof BO1 );
						if (BOR->Size > sizeof BO1)xBlockWrite( SB, BOR + 1, BOR->Size - sizeof BO1 );
						BOR = BOR->Next;
					};
				}
				else
				{
					N = 0;
					xBlockWrite( SB, &N, 2 );
				};
				chk = 1234502;
				xBlockWrite( SB, &chk, 4 );
			};
		};
		if (pr)
		{
			//assert(_CrtCheckMemory());
			chk = 123450;
			xBlockWrite( SB, &chk, 4 );
			//2.Armies
			N = 0;
			for (int i = 0; i < MaxArm; i++)
			{
				if (CT->ARMS[i].Enabled)N++;
			};
			xBlockWrite( SB, &N, 2 );
			for (int i = 0; i < MaxArm; i++)
			{
				if (CT->ARMS[i].Enabled)
				{
					xBlockWrite( SB, &i, 2 );
					AI_Army* AR = CT->ARMS + i;
					xBlockWrite( SB, &AR->NExBrigs, int( &AR->ArmyID ) - int( &AR->NExBrigs ) + 2 );
					for (int j = 0; j < AR->NExBrigs; j++)
					{
						ExtendedBrigade EBR = AR->ExBrigs[j];
						EBR.Brig = (Brigade*) ( EBR.Brig->ID );
						xBlockWrite( SB, &EBR, sizeof( EBR ) );
					};
					xBlockWrite( SB, &AR->NI, 1 );
					//order
					if (AR->AOrder)
					{
						N = 0;
						ArmyOrder* AOR = AR->AOrder;
						while (AOR)
						{
							N++;
							AOR = AOR->Next;
						};
						xBlockWrite( SB, &N, 2 );
						AOR = AR->AOrder;
						while (AOR)
						{
							xBlockWrite( SB, &AOR->Size, 2 );
							ArmyOrder AO1 = *AOR;
							AO1.Message = NULL;
							AO1.Next = NULL;
							int j;
							for (j = 0; j < NAR_ORD&&AOR->ALink != AR_ORD[j]; j++);
							//assert(j<NAR_ORD);
							AO1.ALink = (ArmyLink*) j;
							xBlockWrite( SB, &AO1, sizeof AO1 );
							if (AOR->Size > sizeof AO1)xBlockWrite( SB, &AOR[1], AOR->Size - sizeof AO1 );
							AOR = AOR->Next;
						};
					}
					else
					{
						N = 0;
						xBlockWrite( SB, &N, 2 );
					};
				};
			};
			//assert(_CrtCheckMemory());
			chk = 123451;
			xBlockWrite( SB, &chk, 4 );
			if (CT->NDefArms)
			{
				xBlockWrite( SB, CT->DefArms, 2 * CT->NDefArms );
			};
			chk = 123452;
			xBlockWrite( SB, &chk, 4 );
			if (CT->NDefn)
			{
				for (int j = 0; j < CT->NDefn; j++)
				{
					DefendInfo* DIN = CT->DefInf + j;
					xBlockWrite( SB, DIN, int( &DIN->Def ) - int( DIN ) );
					if (DIN->NDefenders)
					{
						xBlockWrite( SB, DIN->Def, 2 * DIN->NDefenders );
						xBlockWrite( SB, DIN->DefSN, 2 * DIN->NDefenders );
					};
				};
			};
			chk = 123453;
			//assert(_CrtCheckMemory());
			xBlockWrite( SB, &chk, 4 );
			//Information storing
			if (CT->INFORM)
			{
				Inform* INF = CT->INFORM;
				N = 0;
				while (INF)
				{
					N++;
					INF = INF->Next;
				};
				xBlockWrite( SB, &N, 2 );
				INF = CT->INFORM;
				while (INF)
				{
					xBlockWrite( SB, &INF->Size, 2 );
					xBlockWrite( SB, INF, INF->Size );
					INF = INF->Next;
				};
			};
			//assert(_CrtCheckMemory());
			chk = 123454;
			xBlockWrite( SB, &chk, 4 );
			//ideas
			if (CT->IDEA)
			{
				Idea* IDE = CT->IDEA;
				N = 0;
				while (IDE)
				{
					N++;
					IDE = IDE->NextIdea;
				};
				xBlockWrite( SB, &N, 2 );
				IDE = CT->IDEA;
				while (IDE)
				{
					xBlockWrite( SB, &IDE->DataSize, 4 );
					int j;
					for (j = 0; j < N_IBR&&IDE->Brain != IBR[j]; j++);
					//assert(j<N_IBR);
					xBlockWrite( SB, &j, 4 );
					if (IDE->DataSize)xBlockWrite( SB, IDE->IdeaData, IDE->DataSize );
					IDE = IDE->NextIdea;
				};
			};
			chk = 123455;
			xBlockWrite( SB, &chk, 4 );
		};
	};
	//Saving Enemy Info
	int NSE = 0;
	for (int i = 0; i < 8; i++)
	{
		if (GNFO.EINF[i])
		{
			int j;
			for (j = 0; j < i; j++)if (GNFO.EINF[i] == GNFO.EINF[j])j = i + 1;
			if (j == i)NSE++;
		};
	};
	xBlockWrite( SB, &NSE, 1 );
	for (int i = 0; i < 8; i++)
	{
		if (GNFO.EINF[i])
		{
			int j;
			for (j = 0; j < i; j++)if (GNFO.EINF[i] == GNFO.EINF[j])j = i + 1;
			if (j == i)
			{
				int k;
				for (k = 0; k < 8; k++)if (GNFO.EINF[i] == GNFO.EINF[k])xBlockWrite( SB, &k, 1 );
				k = 255;
				xBlockWrite( SB, &k, 1 );
				//essential part
				EnemyInfo* EINF = GNFO.EINF[i];
				xBlockWrite( SB, EINF->TowsID, sizeof EINF->TowsID );
				xBlockWrite( SB, EINF->TowsSN, sizeof EINF->TowsSN );
				xBlockWrite( SB, EINF->TowsMaxR, sizeof EINF->TowsMaxR );
				xBlockWrite( SB, EINF->TowDanger, sizeof EINF->TowDanger );
				xBlockWrite( SB, &EINF->CurTow, sizeof EINF->CurTow );
				xBlockWrite( SB, &EINF->CurR, sizeof EINF->CurR );
				xBlockWrite( SB, &EINF->CurXip, sizeof EINF->CurXip );
				xBlockWrite( SB, &EINF->MaxTowR, sizeof EINF->MaxTowR );
				SaveRLE1( SB, EINF->InflMap, TopLx*TopLx * 4 );

				//enemy walls
				xBlockWrite( SB, &EINF->NEnWalls, 4 );

				if (EINF->NEnWalls)
				{
					xBlockWrite( SB, EINF->WallsX, EINF->NEnWalls << 1 );
					xBlockWrite( SB, EINF->WallsY, EINF->NEnWalls << 1 );
				}
				//danger objects
				xBlockWrite( SB, &EINF->NDINF, 4 );

				if (EINF->NDINF)
				{
					xBlockWrite( SB, EINF->DINF, EINF->NDINF * sizeof DangerInfo );
				}

				//safe places for grenaders
				N = 0;
				for (int ii = 0; ii < SafeLX*SafeLX; ii++)
				{
					if (EINF->SCINF[ii])
					{
						N++;
					}
				}

				xBlockWrite( SB, &N, 2 );

				for (int ii = 0; ii < SafeLX*SafeLX; ii++)
				{
					if (EINF->SCINF[ii])
					{
						xBlockWrite( SB, &ii, 4 );
						xBlockWrite( SB, EINF->SCINF[ii], 64 * sizeof SafeCellInfo );
					}
				}

				chk = 132457;
				xBlockWrite( SB, &chk, 4 );
			}
		}
	}

	xBlockWrite( SB, InfoMap, VAL_SPRNX*VAL_SPRNX );
	int NN = NMines + 1024;
	xBlockWrite( SB, &NN, 2 );
	xBlockWrite( SB, &MaxMine, 2 );
	xBlockWrite( SB, MineList, 4 * NMines );

	chk = 123457;
	xBlockWrite( SB, &chk, 4 );
}

char* std_mess = "[?]";
void LoadAIFromDLL( byte Nat, char* Name );
char* GetTextByID( char* ID );
extern word NPlayers;

void LoadAI( SaveBuf* SB )
{
	int chk;
	xBlockRead( SB, &chk, 4 );

	if (chk == 654321)
	{
		xBlockRead( SB, &AI_Registers, sizeof AI_Registers );
	}

	if (NPlayers == 1)
	{
		for (int i = 0; i < 8; i++)
		{
			PINFO[i].ColorID = i;
		}
	}

	bool used[8] = { 0,0,0,0,0,0,0,0 };

	for (int ni = 0; ni < 8; ni++)
	{
		word cnt;
		byte pr;
		xBlockRead( SB, &pr, 1 );
		xBlockRead( SB, &cnt, 2 );
		byte CID, VictCond, NatID;
		word GT;
		char PName[32];
		xBlockRead( SB, &CID, 1 );
		xBlockRead( SB, &GT, 2 );
		xBlockRead( SB, &VictCond, 1 );
		xBlockRead( SB, &NatID, 1 );
		xBlockRead( SB, &PName, 32 );

		for (int i = 0; i < 8; i++)
		{
			if (PINFO[i].ColorID == CID && !used[i])
			{
				PINFO[i].GameTime = GT;
				PINFO[i].VictCond = VictCond;
				PINFO[i].NationID = NatID;
				strcpy( PINFO[i].name, PName );
				used[i] = 1;
				i = 8;
			}
		}

		Nation* NT = NATIONS + cnt;
		City* CT = CITY + cnt;
		NT->AI_Enabled = pr != 0;
		xBlockRead( SB, &CT->Account, 4 );
		int CurAI = -1;
		xBlockRead( SB, &CurAI, 4 );

		if (pr)
		{
			xBlockRead( SB, CT, sizeof City );
		}

		if (CT->AIID[0])
		{
			LoadAIFromDLL( ni, GetTextByID( CT->AIID ) );
		}

		NT->CITY->MAX_WORKERS = NT->DEFAULT_MAX_WORKERS;
		CT->Nat = NT;

		//details:
		//1.Brigades
		word N;
		word NBRIGA;
		for (int g = 0; g < MaxBrig; g++)
		{
			CT->Brigs[g].NMemb = 0;
			CT->Brigs[g].MaxMemb = 0;
			CT->Brigs[g].Memb = nullptr;
			CT->Brigs[g].MembSN = nullptr;
			CT->Brigs[g].posX = nullptr;
			CT->Brigs[g].posY = nullptr;
			CT->Brigs[g].PosCreated = 0;
			CT->Brigs[g].BOrder = nullptr;
			CT->Brigs[g].CT = CT;
			CT->Brigs[g].ID = g;
		};

		xBlockRead( SB, &NBRIGA, 2 );

		for (int k = 0; k < NBRIGA; k++)
		{
			word brg;
			xBlockRead( SB, &brg, 2 );
			Brigade* BRR = CT->Brigs + brg;
			BRR->Enabled = true;
			xBlockRead( SB, &BRR->NMemb, int( &BRR->BM ) - int( &BRR->NMemb ) + sizeof( BrigMemb ) );

			if (BRR->MaxMemb)
			{
				BRR->Memb = new word[BRR->MaxMemb];
				BRR->MembSN = new word[BRR->MaxMemb];
			}

			xBlockRead( SB, BRR->Memb, BRR->NMemb * 2 );
			xBlockRead( SB, BRR->MembSN, BRR->NMemb * 2 );
			xBlockRead( SB, &BRR->SN, 2 );
			xBlockRead( SB, &BRR->ErasureTime, 2 );
			word r;
			xBlockRead( SB, &r, 2 );

			if (r)
			{
				BRR->posX = new int[BRR->MaxMemb];
				BRR->posY = new int[BRR->MaxMemb];
				xBlockRead( SB, BRR->posX, BRR->NMemb * 4 );
				xBlockRead( SB, BRR->posY, BRR->NMemb * 4 );
			}
			else
			{
				BRR->posX = nullptr;
				BRR->posY = nullptr;
			}

			BRR->CT = CT;

			xBlockRead( SB, &chk, 4 );

			byte c;
			xBlockRead( SB, &c, 1 );

			if (c == 0xFF)
			{
				BRR->CFN = nullptr;
			}
			else
			{
				BRR->CFN = LS_CFN[c];
			}

			xBlockRead( SB, &chk, 4 );

			xBlockRead( SB, &N, 2 );
			if (N)
			{
				BRR->BOrder = nullptr;
				word szz;
				for (int p = 0; p < N; p++)
				{
					xBlockRead( SB, &szz, 2 );
					BrigadeOrder* BOR = BRR->CreateOrder( 2, szz );
					xBlockRead( SB, BOR, szz );
					BOR->BLink = BR_ORD[int( BOR->BLink )];
					BOR->Message = std_mess;
				}
			}
			else
			{
				BRR->BOrder = nullptr;
			}

			xBlockRead( SB, &chk, 4 );
		}

		if (pr)
		{
			xBlockRead( SB, &chk, 4 );

			//2.Armies
			word NARM;
			xBlockRead( SB, &NARM, 2 );
			for (int i = 0; i < NARM; i++)
			{
				word aid;
				xBlockRead( SB, &aid, 2 );
				AI_Army* AR = CT->ARMS + aid;
				AR->Enabled = true;
				xBlockRead( SB, &AR->NExBrigs, int( &AR->ArmyID ) - int( &AR->NExBrigs ) + 2 );
				AR->CT = CT;
				AR->NT = NT;

				if (AR->MaxExBrigs)
				{
					AR->ExBrigs = new ExtendedBrigade[AR->MaxExBrigs];
				}
				else
				{
					AR->ExBrigs = nullptr;
				}

				for (int j = 0; j < AR->NExBrigs; j++)
				{
					ExtendedBrigade* EBR = AR->ExBrigs + j;
					xBlockRead( SB, EBR, sizeof( ExtendedBrigade ) );
					EBR->Brig = CT->Brigs + int( EBR->Brig );
				}

				xBlockRead( SB, &AR->NI, 1 );
				word naor;
				xBlockRead( SB, &naor, 2 );
				AR->AOrder = nullptr;
				for (int k = 0; k < naor; k++)
				{
					word aos;
					xBlockRead( SB, &aos, 2 );
					ArmyOrder* AOR = AR->CreateOrder( 2, aos );
					xBlockRead( SB, AOR, aos );
					AOR->ALink = AR_ORD[int( AOR->ALink )];
					AOR->Message = std_mess;
				}
			}
			xBlockRead( SB, &chk, 4 );

			if (CT->NDefArms)
			{
				CT->DefArms = new word[CT->MaxDefArms];
				xBlockRead( SB, CT->DefArms, 2 * CT->NDefArms );
			}
			xBlockRead( SB, &chk, 4 );

			if (CT->NDefn)
			{
				CT->DefInf = new DefendInfo[CT->MaxDefn];
				for (int j = 0; j < CT->NDefn; j++)
				{
					DefendInfo* DIN = CT->DefInf + j;
					xBlockRead( SB, DIN, int( &DIN->Def ) - int( DIN ) );
					if (DIN->MaxDefs)
					{
						DIN->Def = new word[DIN->MaxDefs];
						DIN->DefSN = new word[DIN->MaxDefs];
					}
					else
					{
						DIN->Def = nullptr;
						DIN->DefSN = nullptr;
					}

					if (DIN->NDefenders)
					{
						xBlockRead( SB, DIN->Def, 2 * DIN->NDefenders );
						xBlockRead( SB, DIN->DefSN, 2 * DIN->NDefenders );
					}
				}
			}
			xBlockRead( SB, &chk, 4 );

			//Information storing
			if (CT->INFORM)
			{
				CT->INFORM = nullptr;
				word ninf;
				xBlockRead( SB, &ninf, 2 );
				for (int p = 0; p < ninf; p++)
				{
					word infs;
					xBlockRead( SB, &infs, 2 );
					Inform* dat = (Inform*) malloc( infs );
					CT->AddInform( dat, nullptr );
					Inform* NEXT = dat->Next;
					Inform* PRE = dat->Previous;
					dat->CT = CT;
					xBlockRead( SB, dat, infs );
					dat->Next = NEXT;
					dat->Previous = PRE;
				}
			}
			xBlockRead( SB, &chk, 4 );

			//ideas
			if (CT->IDEA)
			{
				CT->IDEA = nullptr;
				word nide;
				xBlockRead( SB, &nide, 2 );

				for (int p = 0; p < nide; p++)
				{
					int dsize;
					xBlockRead( SB, &dsize, 4 );
					Idea* IDE = CT->AddIdea( nullptr, true );
					IDE->IdeaData = malloc( dsize );
					IDE->DataSize = dsize;
					int j;
					xBlockRead( SB, &j, 4 );
					IDE->Brain = IBR[j];

					if (IDE->DataSize)
					{
						xBlockRead( SB, IDE->IdeaData, IDE->DataSize );
					}

					IDE = IDE->NextIdea;
				}
			}

			xBlockRead( SB, &chk, 4 );
		}
	}

	int NSE = 0;
	xBlockRead( SB, &NSE, 1 );

	for (int i = 0; i < NSE; i++)
	{
		int CIID = 0;
		xBlockRead( SB, &CIID, 1 );
		int FirstID = -1;

		while (CIID != 0xFF)
		{
			if (FirstID == -1)
			{
				GNFO.EINF[CIID] = new EnemyInfo;
				GNFO.EINF[CIID]->ALLOCATE();
				FirstID = CIID;
			}
			else
			{
				GNFO.EINF[CIID] = GNFO.EINF[FirstID];
			}

			xBlockRead( SB, &CIID, 1 );
		}

		EnemyInfo* EINF = GNFO.EINF[FirstID];
		xBlockRead( SB, EINF->TowsID, sizeof EINF->TowsID );
		xBlockRead( SB, EINF->TowsSN, sizeof EINF->TowsSN );
		xBlockRead( SB, EINF->TowsMaxR, sizeof EINF->TowsMaxR );
		xBlockRead( SB, EINF->TowDanger, sizeof EINF->TowDanger );
		xBlockRead( SB, &EINF->CurTow, sizeof EINF->CurTow );
		xBlockRead( SB, &EINF->CurR, sizeof EINF->CurR );
		xBlockRead( SB, &EINF->CurXip, sizeof EINF->CurXip );
		xBlockRead( SB, &EINF->MaxTowR, sizeof EINF->MaxTowR );

		//xBlockRead(SB,EINF->InflMap,TopLx*TopLx*4);
		LoadRLE1( SB, EINF->InflMap );

		//enemy walls
		xBlockRead( SB, &EINF->NEnWalls, 4 );

		if (EINF->NEnWalls)
		{
			xBlockRead( SB, EINF->WallsX, EINF->NEnWalls << 1 );
			xBlockRead( SB, EINF->WallsY, EINF->NEnWalls << 1 );
		}

		//danger objects
		xBlockRead( SB, &EINF->NDINF, 4 );

		if (EINF->NDINF)
		{
			xBlockRead( SB, EINF->DINF, EINF->NDINF * sizeof DangerInfo );
		}

		//safe places for grenaders
		word N = 0;
		xBlockRead( SB, &N, 2 );
		memset( EINF->SCINF, 0, sizeof EINF->SCINF );

		for (int ii = 0; ii < N; ii++)
		{
			int j;
			xBlockRead( SB, &j, 4 );
			EINF->SCINF[j] = new SafeCellInfo[64];
			xBlockRead( SB, EINF->SCINF[j], 64 * sizeof SafeCellInfo );
		}
		chk = 132457;
		xBlockRead( SB, &chk, 4 );
	}

	xBlockRead( SB, InfoMap, VAL_SPRNX*VAL_SPRNX );
	xBlockRead( SB, &NMines, 2 );

	if (NMines >= 1024)
	{
		NMines -= 1024;
		xBlockRead( SB, &MaxMine, 2 );
		if (MaxMine)
		{
			MineList = new int[MaxMine];
			xBlockRead( SB, MineList, 4 * NMines );
		}
	}
	else
	{
		xBlockRead( SB, &MaxMine, 2 );
		if (MaxMine)
		{
			MineList = new int[MaxMine];
			xBlockRead( SB, MineList, 2 * NMines );
		}

		NMines = 0;
	}

	xBlockRead( SB, &chk, 4 );
}

void SaveActiveObjects( SaveBuf* f1 )
{
	int sz;
	xBlockWrite( f1, &NAZones, 4 );
	xBlockWrite( f1, &NAGroups, 4 );

	for (int i = 0; i < NAZones; i++)
	{
		ActiveZone* AZ = AZones + i;
		xBlockWrite( f1, AZ, sizeof ActiveZone );
		sz = strlen( AZ->Name ) + 1;
		xBlockWrite( f1, &sz, 1 );
		xBlockWrite( f1, AZ->Name, sz );
	}

	for (int i = 0; i < NAGroups; i++)
	{
		ActiveGroup* AG = AGroups + i;
		xBlockWrite( f1, AG, sizeof ActiveGroup );
		sz = strlen( AG->Name ) + 1;
		xBlockWrite( f1, &sz, 1 );
		xBlockWrite( f1, AG->Name, sz );
		xBlockWrite( f1, AG->Units, AG->N * 2 );
		xBlockWrite( f1, AG->Serials, AG->N * 2 );
	}
}

void LoadActiveObjects( SaveBuf* f1 )
{
	xBlockRead( f1, &NAZones, 4 );
	xBlockRead( f1, &NAGroups, 4 );
	MaxAZones = NAZones;
	MaxAGroups = NAGroups;
	AZones = new ActiveZone[MaxAZones];
	AGroups = new ActiveGroup[MaxAGroups];

	for (int i = 0; i < NAZones; i++)
	{
		int sz = 0;
		ActiveZone* AZ = AZones + i;
		xBlockRead( f1, AZ, sizeof ActiveZone );
		xBlockRead( f1, &sz, 1 );
		AZ->Name = new char[sz];
		xBlockRead( f1, AZ->Name, sz );
	}

	for (int i = 0; i < NAGroups; i++)
	{
		int sz = 0;
		ActiveGroup* AG = AGroups + i;
		xBlockRead( f1, AG, sizeof ActiveGroup );
		xBlockRead( f1, &sz, 1 );
		AG->Name = new char[sz];
		xBlockRead( f1, AG->Name, sz );
		AG->Units = new word[AG->N];
		AG->Serials = new word[AG->N];
		xBlockRead( f1, AG->Units, AG->N * 2 );
		xBlockRead( f1, AG->Serials, AG->N * 2 );
	}
}

void CreateMissText();

extern char PlName[64];

void SaveMission( SaveBuf* SB )
{
	xBlockWrite( SB, &MISSLIST.CurrentMission, 4 );

	xBlockWrite( SB, &SCENINF.hLib, 4 );

	if (!SCENINF.hLib)
	{
		return;
	}

	xBlockWrite( SB, PlName, 64 );
	SavePlayerData();
	xBlockWrite( SB, &CurrentCampagin, 4 );
	xBlockWrite( SB, &CurrentMission, 4 );
	xBlockWrite( SB, SCENINF.TIME, sizeof SCENINF.TIME );
	xBlockWrite( SB, SCENINF.TRIGGER, sizeof SCENINF.TRIGGER );
	xBlockWrite( SB, SCENINF.LSpot, sizeof SCENINF.LSpot );
	xBlockWrite( SB, SCENINF.TextDisable, 26 );
	xBlockWrite( SB, &SCENINF.NSaves, 4 );

	for (int i = 0; i < SCENINF.NSaves; i++)
	{
		xBlockWrite( SB, &SCENINF.SaveSize[i], 4 );
		xBlockWrite( SB, SCENINF.SaveZone[i], SCENINF.SaveSize[i] );
	}

	xBlockWrite( SB, &SCENINF.NUGRP, 4 );
	xBlockWrite( SB, &SCENINF.MaxUGRP, 4 );

	for (int i = 0; i < SCENINF.NUGRP; i++)
	{
		UnitsGroup* UG = SCENINF.UGRP + i;
		xBlockWrite( SB, &UG->N, 4 );
		if (UG->N)
		{
			xBlockWrite( SB, UG->IDS, UG->N * 2 );
			xBlockWrite( SB, UG->SNS, UG->N * 2 );
		}
	}

	xBlockWrite( SB, &SCENINF.Victory, 1 );
	xBlockWrite( SB, &SCENINF.LooseGame, 1 );
}

void LoadMission( SaveBuf* SB )
{
	int cms = -1;

	xBlockRead( SB, &cms, 4 );

	int L = 0;

	xBlockRead( SB, &L, 4 );

	if (!L)
	{
		return;
	}

	xBlockRead( SB, PlName, 64 );

	SFLB_LoadPlayerData();

	xBlockRead( SB, &CurrentCampagin, 4 );
	xBlockRead( SB, &CurrentMission, 4 );
	MISSLIST.CurrentMission = cms;

	if (cms != -1)
	{
		SCENINF.Load( MISSLIST.MISS[cms].DLLPath, MISSLIST.MISS[cms].Description );
	}

	xBlockRead( SB, SCENINF.TIME, sizeof SCENINF.TIME );
	xBlockRead( SB, SCENINF.TRIGGER, sizeof SCENINF.TRIGGER );
	xBlockRead( SB, SCENINF.LSpot, sizeof SCENINF.LSpot );
	xBlockRead( SB, SCENINF.TextDisable, 26 );
	xBlockRead( SB, &cms, 4 );

	for (int i = 0; i < cms; i++)
	{
		int sz;
		xBlockRead( SB, &sz, 4 );
		xBlockRead( SB, SCENINF.SaveZone[i], sz );
	}

	for (int i = 0; i < SCENINF.NUGRP; i++)
	{
		UnitsGroup* UG = SCENINF.UGRP + i;
		if (UG->N)
		{
			free( UG->IDS );
			free( UG->SNS );
		}
	}

	xBlockRead( SB, &SCENINF.NUGRP, 4 );
	xBlockRead( SB, &SCENINF.MaxUGRP, 4 );
	SCENINF.UGRP = (UnitsGroup*) realloc( SCENINF.UGRP, SCENINF.MaxUGRP * sizeof UnitsGroup );

	for (int i = 0; i < SCENINF.NUGRP; i++)
	{
		UnitsGroup* UG = SCENINF.UGRP + i;
		xBlockRead( SB, &UG->N, 4 );
		if (UG->N)
		{
			UG->IDS = new word[UG->N];
			UG->SNS = new word[UG->N];
			xBlockRead( SB, UG->IDS, UG->N * 2 );
			xBlockRead( SB, UG->SNS, UG->N * 2 );
		}
		else
		{
			UG->IDS = nullptr;
			UG->SNS = nullptr;
		}
	}

	xBlockRead( SB, &SCENINF.Victory, 1 );
	xBlockRead( SB, &SCENINF.LooseGame, 1 );

	CreateMissText();

	if (SCENINF.LooseGame || SCENINF.Victory)
	{
		NOPAUSE = 0;
		LockPause = 1;
	}
}

void SaveCost( SaveBuf* SB )
{
	xBlockWrite( SB, &NCost, 4 );
	if (NCost)
	{
		xBlockWrite( SB, COSTPL, NCost * sizeof CostPlace );
	}
}

void LoadCost( SaveBuf* SB )
{
	int NC = 0;
	xBlockRead( SB, &NC, 4 );
	NCost = NC;

	if (!NC)
	{
		return;
	}

	if (NC > MaxCost)
	{
		MaxCost = NC;
		COSTPL = (CostPlace*) realloc( COSTPL, MaxCost * sizeof CostPlace );
	}
	xBlockRead( SB, COSTPL, NC * sizeof CostPlace );
}

extern word rpos;
extern word NPlayers;
extern int tmtmt;

void CreateMaskForSaveFile( char* Name )
{
	ResFile F = RReset( Name );
	if (F != INVALID_HANDLE_VALUE)
	{
		RSeek( F, 14 );
		RBlockRead( F, &LOADNATMASK, 4 );
		RClose( F );
	}
}

void ResearchIslands();

int GetMapSUMM( char* Name )
{
	ResFile SB = RReset( Name );

	if (SB == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	int s = RFileSize( SB );
	RClose( SB );
	return s;
}

int GetCurGamePtr( byte** Ptr );

extern int MaxPeaceTime;

extern int PeaceTimeStage;

__declspec( dllimport )
void GetGameID( char* s );

void PreSaveGame( SaveBuf* SB, char* Messtr, int ID )
{
	xBlockWrite( SB, &sfHeader, 4 );
	xBlockWrite( SB, &sfVersion, 4 );
	xBlockWrite( SB, &ID, 4 );

	int NNN = 0;
	for (int i = 0; i < 7; i++)
	{
		if (PINFO[i].name[0])
		{
			NNN++;
		}
	}

	xBlockWrite( SB, &NNN, 2 );
	xBlockWrite( SB, &LOADNATMASK, 4 );
	int sl = strlen( Messtr ) + 1;
	xBlockWrite( SB, &sl, 1 );
	xBlockWrite( SB, Messtr, sl );
	xBlockWrite( SB, &mapx, 4 );
	xBlockWrite( SB, &mapy, 4 );

	for (int i = 0; i < 7; i++)
	{
		if (PINFO[i].name[0])
		{
			xBlockWrite( SB, PINFO[i].name, 32 );
			xBlockWrite( SB, &PINFO[i].ColorID, 1 );
		}
	}
	xBlockWrite( SB, NatRefTBL, 8 );

	SaveMap( SB );
	SaveNations( SB );
	SaveObjects( SB );
	SaveSelection( SB );
	SaveWalls( SB );
	SaveSprites( SB );
	SaveAnmObj( SB );
	Save3DBars( SB );
	SaveCost( SB );
	SaveAI( SB );
	LS_SaveTopology( SB );
	SaveActiveObjects( SB );
	SaveMission( SB );
	byte* bfr;
	int sz = GetEconomyData( &bfr );
	xBlockWrite( SB, bfr, sz );
	word ddd = 0xFFFF;
	xBlockWrite( SB, &ddd, 2 );
	xBlockWrite( SB, &rpos, 2 );
	xBlockWrite( SB, &REALTIME, 4 );
	int ttt = tmtmt + StartTmtmt;
	xBlockWrite( SB, &ttt, 4 );
	ttt = 1;
	xBlockWrite( SB, &ttt, 1 );

	for (int i = 0; i < 8; i++)
	{
		xBlockWrite( SB, &CITY[i].Difficulty, 1 );
	}

	xBlockWrite( SB, &PeaceTimeLeft, 4 );
	xBlockWrite( SB, &MaxPeaceTime, 4 );
	xBlockWrite( SB, &PeaceTimeStage, 4 );
	int v = tmtmt + CURTMTMT;
	xBlockWrite( SB, &v, 4 );
	xBlockWrite( SB, &GameTime, 4 );
	byte* Ptr;
	sz = GetCurGamePtr( &Ptr );

	xBlockWrite( SB, &sz, 4 );
	xBlockWrite( SB, Ptr, sz );
	xBlockWrite( SB, &BalloonState, 4 );
	xBlockWrite( SB, &CannonState, 4 );
	xBlockWrite( SB, &NoArtilleryState, 4 );
	xBlockWrite( SB, &XVIIIState, 4 );
	xBlockWrite( SB, &DipCentreState, 4 );
	xBlockWrite( SB, &ShipyardState, 4 );
	xBlockWrite( SB, &MarketState, 4 );

	byte NN = MyNation;
	xBlockWrite( SB, &NN, 8 );
	xBlockWrite( SB, NatRefTBL, 8 );
	int tt = 2;
	xBlockWrite( SB, &tt, 1 );
	xBlockWrite( SB, COMPSTART, 16 );
	xBlockWrite( SB, &SaveState, 1 );

	char GMID[64] = "";
	GetGameID( GMID );
	xBlockWrite( SB, GMID, 64 );
}

void SaveGame( char* Name, char* Messtr, int ID )
{
	SaveBuf SB;
	PreSaveGame( &SB, Messtr, ID );

	char ttt[128];
	strcpy( ttt, Name );

	if (!strstr( ttt, ".sav" ))
	{
		strcat( ttt, ".sav" );
	}

	ResFile f1 = RRewrite( ttt );
	if (f1 != INVALID_HANDLE_VALUE)
	{
		SB.SaveToFile( f1 );
		RClose( f1 );
	}
	else
	{
		char cc[100];
		sprintf( cc, "Failed to create %s", Name );
		CreateTimedHint( cc, kMinorMessageDisplayTime );//Failed to create %s
	}
}

void ResearchCurrentIsland( byte Nat );
void SetMonstersInCells();
void CreateMiniMap();
void UpdateCurGame();

char PL_Names[8][32];
byte PL_Colors[8];
int PL_NPlayers = 0;
byte PL_NatRefTBL[8];

void SFLB_PreLoadGame( SaveBuf* SB, bool LoadNation )
{
	int i;
	UnLoading();
	InitExplosions();
	xBlockRead( SB, &i, 4 );

	if (i != sfHeader)
	{
		return;
	}

	xBlockRead( SB, &i, 4 );

	if (i != sfVersion)
	{
		return;
	}

	xBlockRead( SB, &i, 4 );

	PL_NPlayers = 0;
	xBlockRead( SB, &PL_NPlayers, 2 );
	xBlockRead( SB, &LOADNATMASK, 4 );

	int sl = 0;
	xBlockRead( SB, &sl, 1 );
	xBlockRead( SB, GameName, sl );

	xBlockRead( SB, &mapx, 4 );
	xBlockRead( SB, &mapy, 4 );

	for (int i = 0; i < PL_NPlayers; i++)
	{
		xBlockRead( SB, PL_Names[i], 32 );
		xBlockRead( SB, PL_Colors + i, 1 );
	}
	xBlockRead( SB, PL_NatRefTBL, 8 );

	LoadMap( SB );
	LoadNations( SB );
	LoadObjects( SB );
	LoadSelection( SB );
	LoadWalls( SB );
	LoadSprites( SB );
	LoadAnmObj( SB );
	Load3DBars( SB );
	LoadCost( SB );
	LoadAI( SB );
	LS_LoadTopology( SB );
	LoadActiveObjects( SB );

	if (MapScenaryDLL[0])
	{
		char ccc[128];
		strcpy( ccc, MapScenaryDLL );
		char* cc1 = strstr( ccc, "." );

		if (cc1)
		{
			strcpy( cc1, ".txt" );
		}

		SCENINF.Load( MapScenaryDLL, ccc );
	}

	LoadMission( SB );

	byte* bfr;

	int sz = GetEconomyData( &bfr );

	xBlockRead( SB, bfr, sz );

	word ddd;

	xBlockRead( SB, &ddd, 2 );

	if (ddd == 0xFFFF)
	{
		xBlockRead( SB, &rpos, 2 );
		xBlockRead( SB, &REALTIME, 4 );
	}

	xBlockRead( SB, &tmtmt, 4 );

	StartTmtmt = 0;

	int NZ = 0;

	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB)
		{
			if (OB->Index == 0)
			{
				NZ++;
			}
		}
	}

	GNFO.Clear();
	GNFO.Setup();

	GTOP[0].LinksDist = LinksDist;
	GTOP[0].MotionLinks = MotionLinks;
	GTOP[0].NAreas = NAreas;
	GTOP[0].TopMap = TopMap;
	GTOP[0].TopRef = TopRef;

	ResearchIslands();

	for (int i = 0; i < 8; i++)
	{
		ResearchCurrentIsland( i );
	}

	SetMonstersInCells();

	int ttt = 0;

	xBlockRead( SB, &ttt, 1 );

	if (ttt == 1)
	{
		for (int i = 0; i < 8; i++)
		{
			xBlockRead( SB, &CITY[i].Difficulty, 1 );
		}
	}

	xBlockRead( SB, &PeaceTimeLeft, 4 );
	xBlockRead( SB, &MaxPeaceTime, 4 );
	xBlockRead( SB, &PeaceTimeStage, 4 );

	xBlockRead( SB, &CURTMTMT, 4 );
	xBlockRead( SB, &GameTime, 4 );

	byte* Ptr;
	sz = GetCurGamePtr( &Ptr );

	xBlockRead( SB, &sz, 4 );
	xBlockRead( SB, Ptr, sz );
	xBlockRead( SB, &BalloonState, 4 );
	xBlockRead( SB, &CannonState, 4 );
	xBlockRead( SB, &NoArtilleryState, 4 );
	xBlockRead( SB, &XVIIIState, 4 );
	xBlockRead( SB, &DipCentreState, 4 );
	xBlockRead( SB, &ShipyardState, 4 );
	xBlockRead( SB, &MarketState, 4 );

	if (LoadNation)
	{
		SetMyNation( 0 );
		byte NN;
		xBlockRead( SB, &NN, 1 );
		SetMyNation( NN );
		byte BUF[8];
		xBlockRead( SB, BUF, 7 );
		xBlockRead( SB, NatRefTBL, 8 );
	}
	else
	{
		byte BUF[8];
		xBlockRead( SB, BUF, 8 );
		xBlockRead( SB, BUF, 8 );
	}

	int tt = 0;

	xBlockRead( SB, &tt, 1 );

	if (tt)
	{
		xBlockRead( SB, COMPSTART, 16 );
	}

	if (tt == 2)
	{
		xBlockRead( SB, &SaveState, 1 );
	}

	UpdateCurGame();
	CreateMiniMap();
}

void SFLB_LoadGame( char* fnm, bool LoadNation )
{
	SaveBuf SB;
	ResFile f1 = RReset( fnm );
	SB.LoadFromFile( f1 );
	SFLB_PreLoadGame( &SB, LoadNation );
	RClose( f1 );
}


//-----------------------------------
#define ARR_RETVAL(x,y,ARR,sz) if(x<0)x=0;if(y<0)y=0;if(x>=sz)x=sz-1;if(y>=sz)y=sz-1;return ARR[x+y*sz];
//1)fmap
int LX_fmap;

word GetV_fmap( int x, int y )
{
	ARR_RETVAL( x, y, fmap, LX_fmap );
}

extern int FMSX;
extern int FMSX2;
extern word* fmap1;

extern byte* MCount;
extern word* BLDList;

extern byte* NPresence;
extern byte* TmpMC;
extern WallCell** WRefs;
extern byte* NSpri;
extern int** SpRefs;
extern byte* WaterDeep;
extern byte* WaterBright;
extern word** Obj3Map;
extern word*  NObj3;
extern byte* InfoMap;
extern byte* CantBuild;
extern word* TopRef;
extern word* WTopRef;

extern short* THMap;// Map of heights in vertices
extern byte* TexMap;//Map of textures in vertices
extern byte* SectMap;//Map of sections on lines

//Bitshift for mapsize dependent calculations
//1 = normal maps
//2 = large (2x) maps
//3 = huge (4x) maps
int ADDSH = 1;

int VAL_SHFCX;
int VAL_MAXCX;
int VAL_MAXCIOFS;
int VAL_SPRNX;
int VAL_SPRSIZE;
int VAL_MAPSX;

int MapShift;
int WLX;
int WMPSIZE;
int MaxWX;
int MaxWY;
int MAPSX;
int MAPSY;
int MAPSHF;
int BMSX;
int B3SX;
int B3SY;
int B3SZ;
int TopLx;
int TopLy;
int MaxTop;
int TopSH;
int MaxSector;
int MaxTH;
int MTHShift;
int VertInLine;
int SectInLine;
int MaxPointIndex;
int MaxLineIndex;
int StratLx;
int StratLy;
int StratSH;
int MAXCIOFS;
int TSX;
int SafeMLx;
int SafeMSH;
int TSH;
int VertLx;
int MaxLI;
int MaxLIX;
int MaxLIY;
int MAXCX;
int DangLx;
int DangSH;
extern int SqMaxX;
extern int SqMaxY;
extern int SqDX;
extern int SqDY;

extern byte* DANGMAP;
extern word* DCHTIME;
int RivNX;
int RivSH;
extern byte* RivDir;
extern byte* RivVol;
void InitRiv();

struct VertOver
{
	short xs;
	short ys;
	short xz;
	int   v;
	bool Visible;
	byte* Data;
};

class OverTriangle
{
public:
	VertOver** TRIANG;
	word*      NTRIANG;
	byte** Buffer;
	int MaxElm;
	int CurElm;
	OverTriangle();
	~OverTriangle();
	void Clear();
	void AddTriangle( int i );
	void CreateFullMap();
	void Show();
	void ShowElement( int Sq, int idx );
};

extern OverTriangle OTRI;
void Init3DMapSystem();
extern VirtualScreen SVSC;
void FreeArrays();

int ARRSZ = 0;
word* MRef = nullptr;//32768 bytes memset
word* MCash = nullptr;
int MaxMCash = 0;
int CurMCash = 0;
int ACTUAL_ADDSH;
void CreateFractal();
int TEXARR[8] = { 0,0,6,6,5,5,4,4 };
int GetFractalVal( int x, int y );

void ClearArrays()
{
	memset( fmap, 0, LX_fmap*LX_fmap * 2 );
	memset( fmap1, 0, LX_fmap*LX_fmap * 2 );
	memset( RivDir, 0, RivNX*RivNX );
	memset( RivVol, 0, RivNX*RivNX );
	memset( MCount, 0, VAL_MAXCIOFS );
	memset( MRef, 0xFF, VAL_MAXCIOFS * 2 );//32768
	memset( BLDList, 0, VAL_MAXCIOFS * 2 );
	memset( NPresence, 0, VAL_MAXCIOFS );
	memset( NSpri, 0, VAL_SPRSIZE );
	memset( SpRefs, 0, VAL_SPRSIZE * 4 );
	memset( WaterDeep, 0, ( VAL_MAPSX*VAL_MAPSX ) >> 2 );
	memset( WaterBright, 0, ( VAL_MAPSX*VAL_MAPSX ) >> 2 );

	for (int i = 0; i < 2; i++)
	{
		memset( MFIELDS[i].MapV, 0, MAPSY*BMSX );
	}

	memset( UnitsField.MapV, 0, MAPSY*BMSX );
	memset( NObj3, 0, B3SZ * 2 );
	memset( InfoMap, 0, VAL_SPRNX*VAL_SPRNX );
	memset( CantBuild, 0, VAL_SPRNX*VAL_SPRNX );
	memset( TopRef, 0, TopLx*TopLx * 2 );
	memset( WTopRef, 0, WTopLx * WTopLx * 2 );
	memset( THMap, 0, ( MaxTH + 1 )*MaxTH * 2 );
	memset( TexMap, 0, ( MaxTH + 1 )*MaxTH );

	if (ADDSH == 1)
	{
		memset( SectMap, 0, MaxSector*MaxTH * 6 );
	}

	for (int i = 0; i < VertLx*VertLx; i++)
	{
		OTRI.TRIANG[i] = nullptr;
		OTRI.NTRIANG[i] = 0;
	}

	memset( DANGMAP, 0, DangLx*DangLx );
	memset( DCHTIME, 0, DangLx*DangLx * 2 );
}

void SetFractalTexture()
{
	int v = 0;

	CreateFractal();

	for (int iy = 0; iy < MaxTH; iy++)
	{
		for (int ix = 0; ix < MaxTH + 1; ix++)
		{
			int vx = GetFractalVal( ix << 8, iy << 8 );

			vx = vx / 64;

			if (vx < 0)
			{
				vx = 0;
			}

			if (vx > 7)
			{
				vx = 7;
			}

			int t0 = TexMap[v];

			if (!t0)
			{
				TexMap[v] = TEXARR[vx];
			}

			if (t0 == 255)
			{
				TexMap[v] = TEXARR[vx] + 128;
			}

			v++;
		}
	}
}

void TEST_GM();

void SetupFog();

void SetupArrays()
{
	if (ADDSH == ACTUAL_ADDSH)
	{
		ClearArrays();
		return;
	}

	ACTUAL_ADDSH = ADDSH;
	ARRSZ = 0;
	if (RivDir)
	{
		FreeArrays();
	}
	int TOTAL = 0;
	LX_fmap = 134 << ( ADDSH - 1 );
	fmap = new word[LX_fmap*LX_fmap];
	int szz = LX_fmap*LX_fmap * 2;
	ARRSZ += szz;
	memset( fmap, 0, LX_fmap*LX_fmap * 2 );
	fmap1 = new word[LX_fmap*LX_fmap];
	ARRSZ += szz;
	memset( fmap1, 0, LX_fmap*LX_fmap * 2 );
	MAXCX = ( 64 << ADDSH );
	TOTAL += LX_fmap*LX_fmap;
	FMSX = ( 134 << ( ADDSH - 1 ) );
	FMSX2 = ( 2 * FMSX );
	VAL_SHFCX = 6 + ADDSH;
	VAL_MAXCX = ( 64 << ADDSH );
	VAL_MAXCIOFS = ( VAL_MAXCX*VAL_MAXCX );
	VAL_SPRNX = 64 << ADDSH;
	VAL_SPRSIZE = VAL_SPRNX*VAL_SPRNX;
	VAL_MAPSX = 512 << ADDSH;
	MAPSX = VAL_MAPSX;
	MAPSY = MAPSX;
	MapShift = ( 9 + ADDSH );
	WLX = ( 128 << ( 1 + ADDSH ) );
	MaxWX = ( MAPSX >> 1 );
	MaxWY = ( MAPSY >> 1 );
	MAPSHF = ( 6 + ADDSH );
	BMSX = ( 64 << ADDSH );
	B3SX = ( MAPSX >> ( B3SHIFT - 4 ) );
	B3SY = ( MAPSY >> ( B3SHIFT - 4 ) );
	B3SZ = ( B3SX*B3SY );
	TopLx = ( MAPSX >> 2 );
	TopLy = ( MAPSY >> 2 );
	MaxTop = ( TopLx*TopLy );
	TopSH = ( 7 + ADDSH );
	MaxSector = 128 << ADDSH;
	MaxTH = MaxSector * 2;
	MTHShift = 8 + ADDSH;
	VertInLine = MaxSector + MaxSector + 1;
	SectInLine = MaxSector * 6;
	MaxPointIndex = ( MaxTH + 1 )*MaxTH;
	MaxLineIndex = MaxSector*MaxTH * 6;
	StratLx = ( MAXCX >> 1 );
	StratLy = ( MAXCX >> 1 );
	MAXCIOFS = ( MAXCX*MAXCX );
	TSX = ( TopLx >> 2 );
	SafeMLx = ( TopLx >> 4 );
	SafeMSH = ( TopSH - 4 );
	TSH = ( TopSH - 2 );
	VertLx = TopLx >> 3;
	MaxLI = ( MAXCX*MAXCX * 4 );
	MaxLIX = ( MAXCX + MAXCX );
	MaxLIY = ( MAXCX + MAXCX );
	StratSH = ( 5 + ADDSH );
	DangLx = ( TopLx >> 1 );
	DangSH = ( TopSH - 1 );
	RivNX = ( 64 << ADDSH );
	RivSH = ( 6 + ADDSH );
	SqMaxX = ( 480 << ( ADDSH - 1 ) ) * 32;
	SqMaxY = ( 480 << ( ADDSH - 1 ) ) * 32;
	SqDX = ( 480 << ( ADDSH - 1 ) ) * 32 / 256;
	SqDY = ( 480 << ( ADDSH - 1 ) ) * 32 / 256;
	SVSC.SetSize( RealLx, RealLy );

	RivDir = new byte[RivNX*RivNX];
	szz = RivNX*RivNX;
	ARRSZ += szz;
	memset( RivDir, 0, RivNX*RivNX );
	RivVol = new byte[RivNX*RivNX];
	ARRSZ += szz;
	memset( RivVol, 0, RivNX*RivNX );

	VAL_MAXCX = ( 64 << ADDSH );

	MCount = new byte[VAL_MAXCIOFS];

	szz = VAL_MAXCIOFS;
	ARRSZ += szz;
	memset( MCount, 0, VAL_MAXCIOFS );
	MRef = new word[VAL_MAXCIOFS];//unsigned short MRef[16384]
	memset( MRef, 0xFF, VAL_MAXCIOFS * 2 );
	szz = VAL_MAXCIOFS * 2;
	ARRSZ += szz;
	BLDList = new word[VAL_MAXCIOFS];
	szz = VAL_MAXCIOFS * 2;
	ARRSZ += szz;
	memset( BLDList, 0, VAL_MAXCIOFS * 2 );
	NPresence = new byte[VAL_MAXCIOFS];
	szz = VAL_MAXCIOFS;
	ARRSZ += szz;
	memset( NPresence, 0, VAL_MAXCIOFS );
	TmpMC = new byte[VAL_MAXCIOFS];
	szz = VAL_MAXCIOFS;
	ARRSZ += szz;

	WRefs = (WallCell**) calloc( VAL_MAXCIOFS * 4 * 4, 1 );
	szz = VAL_MAXCIOFS * 4 * 4;
	ARRSZ += szz;
	NSpri = new byte[VAL_SPRSIZE];
	szz = VAL_SPRSIZE;
	ARRSZ += szz;
	memset( NSpri, 0, VAL_SPRSIZE );
	SpRefs = (int**) calloc( VAL_SPRSIZE * 4, 1 );
	szz = VAL_SPRSIZE * 4;
	ARRSZ += szz;
	memset( SpRefs, 0, VAL_SPRSIZE * 4 );
	WaterDeep = new byte[( VAL_MAPSX*VAL_MAPSX ) >> 2];
	szz = ( VAL_MAPSX*VAL_MAPSX ) >> 2;
	ARRSZ += szz;
	memset( WaterDeep, 0, ( VAL_MAPSX*VAL_MAPSX ) >> 2 );
	WaterBright = new byte[( VAL_MAPSX*VAL_MAPSX ) >> 2];
	ARRSZ += szz;
	memset( WaterBright, 0, ( VAL_MAPSX*VAL_MAPSX ) >> 2 );
	WMPSIZE = ( ( MAPSX*MAPSY ) >> 2 );

	for (int i = 0; i < 2; i++)
	{
		MFIELDS[i].Allocate();
	}

	UnitsField.Allocate();
	Obj3Map = (word**) calloc( B3SZ * 4, 1 );
	szz = B3SZ * 4;
	ARRSZ += szz;
	NObj3 = new word[B3SZ];
	szz = B3SZ * 2;
	ARRSZ += szz;
	memset( NObj3, 0, B3SZ * 2 );
	InfoMap = new byte[VAL_SPRNX*VAL_SPRNX];
	szz = VAL_SPRNX*VAL_SPRNX;
	ARRSZ += szz;
	memset( InfoMap, 0, VAL_SPRNX*VAL_SPRNX );
	CantBuild = new byte[VAL_SPRNX*VAL_SPRNX];
	ARRSZ += szz;
	memset( CantBuild, 0, VAL_SPRNX*VAL_SPRNX );
	TopRef = new word[TopLx*TopLx];
	szz = TopLx*TopLx * 2;
	ARRSZ += szz;
	memset( TopRef, 0, TopLx*TopLx * 2 );
	WTopRef = new word[WTopLx*WTopLx];
	szz = WTopLx*WTopLx * 2;
	ARRSZ += szz;
	memset( WTopRef, 0, WTopLx*WTopLx * 2 );
	THMap = new short[( MaxTH + 1 )*MaxTH];
	szz = ( MaxTH + 1 )*MaxTH * 2;
	ARRSZ += szz;
	memset( THMap, 0, ( MaxTH + 1 )*MaxTH * 2 );
	//AddTHMap=new byte [(MaxTH+1)*MaxTH];
	//memset(AddTHMap,0,(MaxTH+1)*MaxTH);
	TexMap = new byte[( MaxTH + 1 )*MaxTH];
	szz = ( MaxTH + 1 )*MaxTH;
	ARRSZ += szz;
	memset( TexMap, 0, ( MaxTH + 1 )*MaxTH );

	if (ADDSH == 1)
	{
		SectMap = new byte[MaxSector*MaxTH * 6];
		szz = MaxSector*MaxTH * 6;
		ARRSZ += szz;
		memset( SectMap, 0, MaxSector*MaxTH * 6 );
	}
	else
	{
		SectMap = NULL;
	}

	OTRI.TRIANG = (VertOver**) calloc( VertLx*VertLx * 4, 1 );
	szz = VertLx*VertLx * 4;
	ARRSZ += szz;
	OTRI.NTRIANG = new word[VertLx*VertLx];
	szz = VertLx*VertLx * 2;
	ARRSZ += szz;

	for (int i = 0; i < VertLx*VertLx; i++)
	{
		OTRI.TRIANG[i] = NULL;
		OTRI.NTRIANG[i] = 0;
	}

	DANGMAP = new byte[DangLx*DangLx];
	szz = DangLx*DangLx;
	ARRSZ += szz;
	memset( DANGMAP, 0, DangLx*DangLx );
	DCHTIME = new word[DangLx*DangLx];
	szz = DangLx*DangLx * 2;
	ARRSZ += szz;
	memset( DCHTIME, 0, DangLx*DangLx * 2 );
	Init3DMapSystem();
	//SetFractalTexture();
	TEST_GM();
	SetupFog();
}

extern byte* NatDeals;

void FreeArrays()
{
	if (!RivDir)return;
	ACTUAL_ADDSH = 0;
	free( RivDir );
	RivDir = NULL;
	free( RivVol );
	RivVol = NULL;
	free( MCount );
	MCount = NULL;
	//free(NMsList);
	//NMsList=NULL;
	free( MRef );
	MRef = NULL;
	free( MCash );
	MCash = NULL;
	MaxMCash = 0;
	CurMCash = 0;

	free( BLDList );
	BLDList = NULL;
	free( NPresence );
	NPresence = NULL;
	free( TmpMC );
	TmpMC = NULL;
	free( WRefs );
	WRefs = NULL;
	free( NSpri );
	NSpri = NULL;
	free( SpRefs );
	SpRefs = NULL;
	free( WaterDeep );
	WaterDeep = NULL;
	free( WaterBright );
	WaterBright = NULL;

	for (int i = 0; i < 2; i++)
	{
		MFIELDS[i].FreeAlloc();
	}

	UnitsField.FreeAlloc();
	free( Obj3Map );
	Obj3Map = NULL;
	free( NObj3 );
	NObj3 = NULL;
	free( InfoMap );
	InfoMap = NULL;
	free( CantBuild );
	CantBuild = NULL;
	free( TopRef );
	TopRef = NULL;
	free( WTopRef );
	WTopRef = NULL;
	free( THMap );
	THMap = NULL;
	//free(AddTHMap);
	//AddTHMap=NULL;
	free( TexMap );
	TexMap = NULL;

	if (SectMap)
	{
		free( SectMap );
	}

	SectMap = NULL;
	free( OTRI.TRIANG );
	OTRI.TRIANG = NULL;
	free( OTRI.NTRIANG );
	OTRI.NTRIANG = NULL;
	free( DANGMAP );
	DANGMAP = NULL;
	free( DCHTIME );
	DCHTIME = NULL;
	GNFO.Clear();
	MaxSprt = 0;
	free( NatDeals );
	NatDeals = 0;
}

void MotionField::Allocate()
{
	//MapH=new byte[MAPSY*BMSX];
	//memset(MapH,0,MAPSY*BMSX);
	MapV = new byte[MAPSY*BMSX];
	memset( MapV, 0, MAPSY*BMSX );
}

void MotionField::FreeAlloc()
{
	//free(MapH);
	free( MapV );
}

void EnemyInfo::ALLOCATE()
{
	ProtectionMap = new byte[MAXCIOFS];
	SupMortBestID = new word[MAXCIOFS];
	SupMortLastTime = new word[MAXCIOFS];
	InflMap = new DWORD[TopLx*TopLx];
	SafeMAP = new byte[SafeMLx*SafeMLx];
	SCINF = (SafeCellInfo**) calloc( SafeLX*SafeLX * 4, 1 );
	NUN = new word[TSX*TSX];
	TMAP = new byte[TSX*TSX];
	GAINF.ArmDistr = new word[StratLx*StratLy];
	memset( GAINF.ArmDistr, 0xFF, StratLx*StratLy * 2 );
}

void EnemyInfo::FREE()
{
	free( ProtectionMap );
	free( SupMortBestID );
	free( SupMortLastTime );
	free( GAINF.ArmDistr );
}

word GetNMSL( int i )
{
	int v = i >> 6;// : 64

	if (v < VAL_MAXCIOFS)//BUGFIX: prevent access violation in MRef[v] when v > 128^2
	{
		int idx = MRef[v];

		if (idx != 0xFFFF)
		{
			return MCash[( idx << 6 ) + ( i & 63 )];
		}
	}

	return 0;
}

void SetNMSL( int i, word W )
{
	int v = i >> 6;
	int idx = MRef[v];
	if (idx == 0xFFFF)
	{
		if (CurMCash >= MaxMCash)
		{
			MaxMCash += 512;
			MCash = (word*) realloc( MCash, MaxMCash << 7 );
		}
		idx = CurMCash;
		CurMCash++;
		MRef[v] = idx;
	}
	MCash[( idx << 6 ) + ( i & 63 )] = W;
}

void CleanNMSL()
{
	CurMCash = 0;
	memset( MRef, 0xFF, VAL_MAXCIOFS << 1 );
}
