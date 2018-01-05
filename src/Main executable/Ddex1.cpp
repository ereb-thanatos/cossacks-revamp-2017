/*==========================================================================
 *
 *  Copyright (C) 1997-1998 Andrew(GSC). All Rights Reserved.
 *
 *  Revamped in 2017 by Эреб
 *
 ***************************************************************************/

#define NAME "CEW_KERNEL"
#define TITLE "Cossacks"

#include "ddini.h"

bool window_mode;
int screen_width;
int screen_height;
double screen_ratio;
DWORD window_style = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

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
#include "dialogs.h"
#include <assert.h>

#include "3DSurf.h"
#include "CDirSnd.h"
#include "GSound.h"
#include "dplay.h"
#include "MapSprites.h"
#include "VirtScreen.h"
#include <crtdbg.h>
#include "RealWater.h"
#include "Transport.h"
#include "AntiBug.h"
#include "3DBars.h"
#include "TopoGraf.h"
#include "StrategyResearch.h"

#include "Safety.h"
#include "NewAI.h"
#include "Danger.h"
#include "GP_Draw.h"
#include "Sort.h"
#include "Recorder.h"
#include "MPlayer.h"
#include "EinfoClass.h"
#include "3DmapEd.h"
#include "ActiveScenary.h"
#include "fonts.h"
#include "Dialogs\InitFonts.h"

#include "PlayerInfo.h"
extern PlayerInfo PINFO[8];

#define TIMER_ID 1
#define maxTask 32

bool AttackMode;
bool ChoosePosition;
bool DeathMode;
bool EditMapMode;
bool EgoFlag;
bool FASTMODE;
bool FastMode = false;
bool FullMini = true;
bool HealthMode;
bool HelpMode;
bool InfoMode;
bool MEditMode;
bool MUSTDRAW;

//Unknown mode classification
bool MiniMode;

bool PeaceMode;
bool SHOWSLIDE = true;
bool TransMode;
bool VHMode = 0;
bool fixed;

//Timespan in ms after last LastCTRLPressTime which allows setting unit control groups
const int kCtrlStickyTime = 50;

//Minimal delay between two PostDrawGameProcess() returns, in ms
const unsigned int kPostDrawInterval = 16; //~60 Hz

//Time of the last PostDrawGameProcess() return
unsigned long prev_postdraw_time = 0;

//Game version. Must match with other clients
__declspec( dllexport ) word dwVersion = 100;
__declspec( dllexport ) char LobbyVersion[32] = "1.00";
__declspec( dllexport ) char BuildVersion[32] = "V 1.00";

int CostThickness;
int HISPEED = 0;
int ReliefBrush;
int TerrBrush;
int AutoTime;
int BlobMode;
int CoalID;
int CurGroundTexture = 0;
int DrawGroundMode = 0;
int DrawPixMode = 0;
int Flips;
int FoodID;
int FrmDec = 2;
int GoldID;
int HeightEditMode;
int HiStyle;
int IronID;
int LASTRAND, LASTIND;
int LastAttackDelay = 0;
int MaxSizeX;
int MaxSizeY;
int Multip;
int NMONS;
int REALTIME;

//Multiplier, depends on MiniMode
//MiniMode ? 4 : 5
int Shifter;

//Game speed mode dependent SpeedShift variable. Used for controlling animations etc
int SpeedSh = 1;

int StoneID;
int TreeID;
int WaterEditMode;

//Game speed mode
//0: Slow mode
//1: Fast mode
int exFMode = 1;

//Timer Callback
int cadr;
int tima;
int tmtim;

//Main internal counter for intervals
int tmtmt;

//Last used display resolutions for both modes
int exRealLx, exRealLy;
int ex_other_RealLx, ex_other_RealLy; //Necessary for saving settings

static int Light = 0;

char* FormationStr = nullptr;

byte EditMedia;
byte LockGrid;
byte LockMode;
byte PauseMode = 0;
byte PlayerMask;
byte Quality;
word Creator;
static word MsPerFrame = 40;
CDirSound CDIRSND;
City CITY[8];
HugeExplosion HE;
Menu About;
Menu LoadFile;
Menu MainMenu;
Menu Options;
Nation WEP;
WIN32_FIND_DATA findfi;
Weapon Arrow;
Weapon Fire1;
Weapon FlyFire1;
Weapon Flystar;
Weapon Lasso;
Weapon Magvib;
Weapon Molnia;
Weapon MolniaVibux;
Weapon Sphere;
Weapon Vibux1;

extern bool AttGrMode;
extern bool BuildMode;
extern bool CINFMOD;
extern bool ChangeNation;
extern bool CheapMode;
extern bool FullScreenMode;
extern bool GameInProgress;
extern bool GameNeedToDraw;
extern bool GetCoord;
extern bool GoAndAttackMode;
extern bool LockPause;
extern bool MakeMenu;
extern bool MiniActive;
extern bool MultiTvar;
extern bool Recreate;
extern bool SetDestMode;
extern bool realLpressed;
extern bool realRpressed;

extern int FogMode;
extern int Inform;
extern int MaxAllowedComputerAI;
extern int MenuType;
extern int MidiSound;
extern int NMyUnits;
extern int NThemUnits;
extern int OrderSound;
extern int RealLx;
extern int RealLy;
extern int RealPause;
extern int ShowGameScreen;
extern int WarSound;
extern int WorkSound;
extern int curdx;
extern int curdy;
extern int curptr;
extern int sfVersion;

extern char SaveFileName[128];

extern byte ScanPressed[256];
extern byte SpecCmd;
extern word PlayerMenuMode;
extern word rpos;
extern BlockBars LockBars;
extern BlockBars UnLockBars;
extern CDirSound* CDS;
extern DPID MyDPID;
extern DPID ServerDPID;

__declspec( dllexport ) bool KeyPressed;
__declspec( dllexport ) int LastKey;

void InitDialogs();
void SFLB_LoadGame(char* fnm, bool LoadNation);

void CheckGP();
void ClearMaps();
void CmdChangeSpeed();
void CreateRandomHMap();
void CreateTotalLocking();
void CreateUnitsLocking();
void EraseAreas();
void GSSetup800();
void GetForces();
void HandleMultiplayer();
void Init3DMapSystem();
void InitDestn();
void InitFishMap();
void LoadMessages();
void LoadNewAimations();
void Loadtextures();
void MFix();
void OnMouseMoveRedraw();
void ProcessFishing();
void ProcessSprites();
void ProcessUFO();
void RenderAllMap();
void Reset3D();
void SaveGame(char* fnm, char* gg, int ID);
void SelectAllBuildings(byte NI);

void SetLight(int Ldx, int Ldy, int Ldz);
void TestTriangle();
void WinnerControl(bool);
void makeFden();
int processMainMenu();

//For parallel processable tasks
typedef void EventHandPro(void*);

struct EventsTag
{
	EventHandPro* Pro;
	int Type;
	int Handle;
	bool Blocking;
	void* Param;
};

void PlayerMenuWork();
int GetResID(char*);

EventsTag Events[maxTask];

int RegisterEventHandler(EventHandPro* pro, int Type, void* param)
{
	int i;
	for (i = 0; int(Events[i].Pro) && i < maxTask; i++);
	if (i >= maxTask)
	{
		return -1;
	}

	Events[i].Pro = pro;
	Events[i].Type = Type;
	Events[i].Handle = i;
	Events[i].Blocking = false;
	Events[i].Param = param;
	return i;
}

void CloseEventHandler(int i)
{
	memset(&Events[i], 0, sizeof Events[i]);
}

HWND hwnd;

//fonts
RLCTable RCross;
RLCTable mRCross;

int xxx;
void ShowFon1();
void WaterCorrection();
extern bool TexMapMod;
extern bool RiverEditMode;
void ClearCurve();
extern bool TexPieceMode;
extern int DrawPixMode;

void ClearModes()
{
	DrawPixMode = 0;
	DrawGroundMode = 0;
	HeightEditMode = false;
	MEditMode = false;
	LockMode = 0;
	WaterEditMode = false;
	SetWallBuildMode(0xFF, 0);
	TexMapMod = false;
	RiverEditMode = 0;
	ClearCurve();
	TexPieceMode = 0;
}

void TimerProc(void)
{
	if (PlayerMenuMode == 1)
	{
		ShowFon1();
		for (int j = 0; j < maxTask; j++)
		{
			if (Events[j].Pro)
			{
				(*(Events[j].Pro))(Events[j].Param);
			}
		}
		SetRLCWindow(0, 1, MaxSizeX, RSCRSizeY - 1, ScrWidth);
	}
	else
	{
		SetRLCWindow(0, 1, MaxSizeY, RSCRSizeY - 1, ScrWidth);
	}
	HandleMouse(mouseX, mouseY);
	MFix();
	FlipPages();
}

//Loading...
void LoadEconomy();
void LoadNations();
void LoadWeapon();
void LoadNation(char* fn, byte msk, byte NIndex);
void LoadAllNewMonsters();
void InitNewMonstersSystem();
void LoadWaveAnimations();
extern NewAnimation* Shar;
void DoGen();
void InitDeathList();
char* GetTextByID(char* ID);
void LoadBorders();

void SetupArrays();
extern byte* RivDir;
void Init_GP_IMG();
void ReadClanData();

extern bool InGame;
extern bool InEditor;
extern bool RUNMAPEDITOR;
extern bool RUNUSERMISSION;
extern char USERMISSPATH[128];

//Calculates window coordinates and locks cursor inside client area
void ClipCursorToWindowArea()
{
	if (!window_mode)
	{
		//Just in case
		return;
	}

	if (!InGame && !InEditor)
	{
		//Reset mouse locking in menues
		ClipCursor(nullptr);
		return;
	}

	//Determine absolute coordinates of window client area
	RECT client_coords;
	GetClientRect(hwnd, &client_coords);
	MapWindowPoints(hwnd, nullptr, (LPPOINT) & client_coords, 2);

	//Necessary for correct cursor capture
	//Using exact ClientRect causes cursor to freeze short of
	//right or bottom border when moving fast
	client_coords.right--;
	client_coords.bottom--;

	ClipCursor(&client_coords);
}

void ResizeAndCenterWindow()
{
	if (!window_mode)
	{
		//Just in case
		return;
	}

	RECT window_size;
	window_size.top = 0;
	window_size.left = 0;
	window_size.right = RealLx;
	window_size.bottom = RealLy;
	AdjustWindowRect(&window_size, window_style, FALSE);

	int width = window_size.right - window_size.left;
	int height = window_size.bottom - window_size.top;

	int x = screen_width / 2 - width / 2;
	int y = screen_height / 2 - height / 2;

	if (x < 0)
	{
		x = 0;
	}
	if (y < 0)
	{
		y = 0;
	}

	MoveWindow(hwnd, x, y, width, height, TRUE);

	ClipCursorToWindowArea();

	SetCursorPos(screen_width / 2, screen_height / 2);
}

//Load ids, textures etc
bool Loading()
{
	ReadClanData();
	RivDir = NULL;

	Init_GP_IMG();
	InitDeathList();
	InitNewMonstersSystem();
	InitFonts();
	LoadBorders();
	LoadMessages();
	LoadNations();
	LoadFon();
	LoadRDS();

	GoldID = GetResID("GOLD");
	FoodID = GetResID("FOOD");
	StoneID = GetResID("STONE");
	TreeID = GetResID("WOOD");
	CoalID = GetResID("COAL");
	IronID = GetResID("IRON");
	LoadEconomy();

	Loadtextures();
	LoadFog(1);
	LoadTiles();
	LoadLock();
	LoadNewAimations();
	LoadWeapon();
	InitExplosions();
	InitSprites();
	LoadAllWalls();
	LoadAllNewMonsters();
	LoadWaveAnimations();

	LoadAllNations(0);
	LoadAllNations(1);
	LoadAllNations(2);
	LoadAllNations(3);
	LoadAllNations(4);
	LoadAllNations(5);
	LoadAllNations(6);
	LoadAllNations(7);

	CITY[0].CreateCity(0);
	CITY[1].CreateCity(1);
	CITY[2].CreateCity(2);
	CITY[3].CreateCity(3);
	CITY[4].CreateCity(4);
	CITY[5].CreateCity(5);
	CITY[6].CreateCity(6);
	CITY[7].CreateCity(7);

	InitTopChange();
	LoadPalettes();
	InitPrpBar();

	SetMyNation( 0 );

	FormationStr = GetTextByID("FORMATION");

	return 1;
}

extern int CurPalette;

void SaveScreenShot(char* Name)
{
	byte PAL[1024];
	memset(PAL, 0, 1024);
	char ccx[120];
	sprintf(ccx, "%d\\agew_1.pal", CurPalette);
	ResFile f = RReset(ccx);
	int i;
	for (i = 0; i < 256; i++)
	{
		int ofs = i << 2;
		RBlockRead(f, PAL + ofs + 2, 1);
		RBlockRead(f, PAL + ofs + 1, 1);
		RBlockRead(f, PAL + ofs, 1);
	};
	RClose(f);
	f = RRewrite(Name);
	i = 0x4D42;
	RBlockWrite(f, &i, 2);
	i = RealLx * RealLy + 1080;
	RBlockWrite(f, &i, 4);
	i = 0;
	int j = 0x436;
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &j, 4);
	j = 0x28;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &RealLx, 4);
	RBlockWrite(f, &RealLy, 4);
	j = 0x080001;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &i, 4);
	j = 0x0B12;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, PAL, 1024);
	for (int i = 0; i < RealLy; i++)
	{
		char* pos = (char*)(int(ScreenPtr) + (RealLy - i - 1) * SCRSizeX);
		RBlockWrite(f, pos, RealLx);
	};
	RClose(f);
};

void SaveBMP8(char* Name, int lx, int ly, byte* Data)
{
	byte PAL[1024];
	memset(PAL, 0, 1024);
	char ccc[128];
	sprintf(ccc, "%d\\agew_1.pal", CurPalette);
	ResFile f = RReset(ccc);
	int i;
	for (i = 0; i < 256; i++)
	{
		int ofs = i << 2;
		RBlockRead(f, PAL + ofs + 2, 1);
		RBlockRead(f, PAL + ofs + 1, 1);
		RBlockRead(f, PAL + ofs, 1);
	};
	RClose(f);
	f = RRewrite(Name);
	i = 0x4D42;
	RBlockWrite(f, &i, 2);
	i = lx * ly + 1080;
	RBlockWrite(f, &i, 4);
	i = 0;
	int j = 0x436;
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &j, 4);
	j = 0x28;
	RBlockWrite(f, &j, 4);
	int LX = lx;
	int LY = ly;
	RBlockWrite(f, &LX, 4);
	RBlockWrite(f, &LY, 4);
	j = 0x080001;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &i, 4);
	j = 0x0B12;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, PAL, 1024);
	for (int i = 0; i < LY; i++)
	{
		char* pos = (char*)(int(Data) + (ly - i - 1) * lx);
		RBlockWrite(f, pos, lx);
	};
	RClose(f);
};

void SaveMiniScreenShot(char* Name)
{
	byte PAL[1024];
	memset(PAL, 0, 1024);
	ResFile f = RReset("agew_1.pal");
	int i;
	for (int i = 0; i < 256; i++)
	{
		int ofs = i << 2;
		RBlockRead(f, PAL + ofs + 2, 1);
		RBlockRead(f, PAL + ofs + 1, 1);
		RBlockRead(f, PAL + ofs, 1);
	};
	RClose(f);
	f = RRewrite(Name);
	i = 0x4D42;
	RBlockWrite(f, &i, 2);
	i = RealLx * RealLy + 1080;
	RBlockWrite(f, &i, 4);
	i = 0;
	int j = 0x436;
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &j, 4);
	j = 0x28;
	RBlockWrite(f, &j, 4);
	int LX = RealLx >> 2;
	int LY = RealLy >> 2;
	RBlockWrite(f, &LX, 4);
	RBlockWrite(f, &LY, 4);
	j = 0x080001;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &i, 4);
	j = 0x0B12;
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &j, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, &i, 4);
	RBlockWrite(f, PAL, 1024);
	for (int i = 0; i < LY; i++)
	{
		char* pos = (char*)(int(ScreenPtr) + (RealLy - i * 4 - 1) * SCRSizeX);
		for (int j = 0; j < LX; j++)RBlockWrite(f, pos + j * 4, 1);
	};
	RClose(f);
};

void SaveScreen()
{
	char ccc[128];
	CreateDirectory("Screenshots", 0);
	int i;
	for (i = 0; i < 1000; i++)
	{
		sprintf(ccc, "Screenshots\\screen%d.bmp", i);
		ResFile f = RReset(ccc);
		if (f == INVALID_HANDLE_VALUE)
		{
			RClose(f);
			goto zzz;
		};
		RClose(f);
	};
	i = 99;
zzz:
	sprintf(ccc, "Screenshots\\screen%d.bmp", i);
	SaveScreenShot(ccc);
};
extern bool NoText;
extern bool SHOWSLIDE;
void GFieldShow();
void SaveMiniScreenShot(char* Name);

void MiniRenderAllMap()
{
	SHOWSLIDE = true;
	int nx = div( msx, smaplx ).quot;
	int ny = div( msy, smaply ).quot;
	//if(nx>3)nx=3;
	//if(ny>3)ny=3;
	char ccc[128];
	NoText = true;
	for (int y = 0; y < ny; y++)
	{
		for (int x = 0; x < nx; x++)
		{
			mapx = x * smaplx;
			mapy = y * smaply;
			GFieldShow();
			FlipPages();
			int p = x + y * nx;
			if (p < 10)sprintf(ccc, "scr00%d.bmp", p);
			else if (p < 100)sprintf(ccc, "scr0%d.bmp", p);
			else sprintf(ccc, "scr%d.bmp", p);
			SaveMiniScreenShot(ccc);
		};
	};
	NoText = false;
};

void RenderAllMap()
{
	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		MiniRenderAllMap();
		return;
	};
	SHOWSLIDE = true;
	int nx = div( msx, smaplx ).quot;
	int ny = div( msy, smaply ).quot;
	//if(nx>3)nx=3;
	//if(ny>3)ny=3;
	char ccc[128];
	NoText = true;
	for (int y = 0; y < ny; y++)
	{
		for (int x = 0; x < nx; x++)
		{
			mapx = x * smaplx;
			mapy = y * smaply;
			GFieldShow();
			FlipPages();
			int p = x + y * nx;
			if (p < 10)sprintf(ccc, "scr00%d.bmp", p);
			else if (p < 100)sprintf(ccc, "scr0%d.bmp", p);
			else sprintf(ccc, "scr%d.bmp", p);
			SaveScreenShot(ccc);
		}
	}
	NoText = false;
}

/*
 * finiObjects
 *
 * finished with all objects we use; release them
 */
static void finiObjects(void)
{
	FreeDDObjects();
} /* finiObjects */

#define MaxQu 32
MouseStack MSTC[MaxQu];
MouseStack CURMS;
int NInStack = 0;

void AddMouseEvent(int x, int y, bool L, bool R)
{
	if (NInStack < MaxQu)
	{
		MSTC[NInStack].x = x;
		MSTC[NInStack].y = y;
		MSTC[NInStack].Lpressed = L;
		MSTC[NInStack].Rpressed = R;
		MSTC[NInStack].rLpressed = L;
		MSTC[NInStack].rRpressed = R;
		MSTC[NInStack].Control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		MSTC[NInStack].Shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		NInStack++;
	}
}

int LastUMX = 0;
int LastUMY = 0;
int LastUTime = 0;

MouseStack* ReadMEvent()
{
	if (NInStack)
	{
		CURMS = MSTC[0];
		if (NInStack > 1)
		{
			memcpy(MSTC, MSTC + 1, (NInStack - 1) * sizeof MouseStack);
		}
		NInStack--;
		return &CURMS;
	}
	return nullptr;
}

void ClearMStack()
{
	NInStack = 0;
}

extern bool unpress;
extern byte ScanPressed[256];

void UnPress()
{
	for (int i = 0; i < NInStack; i++)
	{
		MSTC[i].Lpressed = 0;
		MSTC[i].Rpressed = 0;
	}
	unpress = 1;
	memset(ScanPressed, 0, 256);
}

extern int CurPalette;
LRESULT CD_MCINotify(WPARAM wFlags, LONG lDevId);
int SHIFT_VAL = 0;
void HandleMouse(int x, int y);
extern bool PalDone;
byte KeyStack[32];
byte AsciiStack[32];
int NKeys = 0;
byte LastAsciiKey = 0;

void AddKey(byte Key, byte Ascii)
{
	if (32 <= NKeys)
	{
		//Push the stack back by one element
		memcpy(KeyStack, KeyStack + 1, 31);
		memcpy(AsciiStack, AsciiStack + 1, 31);
		NKeys--;
	}
	KeyStack[NKeys] = Key;
	AsciiStack[NKeys] = Ascii;
	NKeys++;
}

byte LastAscii = 0;
wchar_t last_unicode = 0;

int ReadKey()
{
	//Called only for chat input and resource transfer
	if (NKeys)
	{
		byte c = KeyStack[0];
		LastAscii = AsciiStack[0];
		if (NKeys)
		{
			memcpy(KeyStack, KeyStack + 1, NKeys - 1);
			memcpy(AsciiStack, AsciiStack + 1, NKeys - 1);
		}
		NKeys--;
		return c;
	}
	else
	{
		return -1;
	}
}

void ClearKeyStack()
{
	NKeys = 0;
}

extern bool GUARDMODE;
extern bool PATROLMODE;
extern byte NeedToPopUp;
short WheelDelta = 0;
void IAmLeft();
void LOOSEANDEXITFAST();
extern bool DoNewInet;
bool ReadWinString(GFILE* F, char* STR, int Max);
void OnWTPacket(WPARAM wSerial, LPARAM hCtx);

void CmdEndGame(byte NI, byte state, byte cause);

long FAR PASCAL WindowProc(HWND hWnd, UINT message,
                           WPARAM wParam, LPARAM lParam)
{
	static BYTE phase = 0;

	switch (message)
	{
	case 0xABCD:
		{
			GFILE* F = Gopen("UserMissions\\start.dat", "r");
			if (F)
			{
				ReadWinString(F, USERMISSPATH, 120);
				Gclose(F);
				if (lParam == 1)
				{
					RUNMAPEDITOR = 1;
				}
				if (lParam == 0)
				{
					RUNUSERMISSION = 1;
				}
			}
		}
		break;

	case 0x020A: //WM_MOUSEWHEEL:
		WheelDelta = (short)HIWORD(wParam);
		break;

	case MM_MCINOTIFY:
		CD_MCINotify(wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		wParam = wParam | MK_LBUTTON;
		Lpressed = true;
		realLpressed = true;
		fixed = false;
		SetMPtr(LOWORD(lParam), HIWORD(lParam), wParam);
		AddMouseEvent(mouseX, mouseY, Lpressed, Rpressed);
		break;

	case WM_LBUTTONUP:
		wParam = wParam & !MK_LBUTTON;
		if (fixed)
		{
			Lpressed = false;
		}
		realLpressed = false;

		SetMPtr(LOWORD(lParam), HIWORD(lParam), wParam);

		AddMouseEvent(mouseX, mouseY, Lpressed, Rpressed);

		//Double click
		if (!BuildMode //BUGFIX: Prevent unit selection while placing buildings
			&& (abs(mouseX - LastUMX) + abs(mouseY - LastUMY)) < 16
			&& GetTickCount() - LastUTime < 600)
		{
			//Select all units of selected type on screen
			SpecCmd = 241;
		}

		LastUMX = mouseX;
		LastUMY = mouseY;
		LastUTime = GetTickCount();
		break;

	case WM_RBUTTONDOWN:
		wParam = wParam | MK_RBUTTON;
		Rpressed = true;
		realRpressed = true;
		fixed = false;
		if (ScreenPtr)
		{
			SetMPtr(LOWORD(lParam), HIWORD(lParam), wParam);
		}
		AddMouseEvent(mouseX, mouseY, Lpressed, Rpressed);
		break;

	case WM_RBUTTONUP:
		wParam = wParam & !MK_RBUTTON;
		Rpressed = false;
		if (fixed)
		{
			Rpressed = false;
		}
		realRpressed = false;
		if (ScreenPtr)
		{
			SetMPtr(LOWORD(lParam), HIWORD(lParam), wParam);
		}
		AddMouseEvent(mouseX, mouseY, Lpressed, Rpressed);
		break;

	case WM_MOUSEMOVE:
		if (ScreenPtr)
		{
			if (LOWORD(lParam) != mouseX || HIWORD(lParam) != mouseY)
			{
				SetMPtr(LOWORD(lParam), HIWORD(lParam), wParam);
				OnMouseMoveRedraw();
			}
		}
		break;

	case WM_EXITSIZEMOVE:
		//Adjust cursor zone after window was moved
		ClipCursorToWindowArea();
		break;

	case WM_SIZE:
		if (SIZE_RESTORED == wParam)
		{
			//Restore cursor zone after window was minimized
			ClipCursorToWindowArea();
		}
		break;

	case WM_SETFOCUS:
		//Restore cursor zone after alt-tab
		ClipCursorToWindowArea();
		break;

	case WM_ACTIVATEAPP:
		bActive = wParam;
		if (bActive)
		{
			if (lpDDSPrimary)
			{
				CreateDDObjects(hwnd);
				LockSurface();
				UnlockSurface();
				LoadFog(CurPalette);
				char cc[64];
				sprintf(cc, "%d\\agew_1.pal", CurPalette);
				PalDone = 0;
				LoadPalette(cc);
			}
		}
		break;

	case WM_SETCURSOR:
		SetCursor(NULL);
		return TRUE;

	case WM_KEYDOWN:
		if (wParam < 256)
		{
			ScanPressed[wParam] = 1;
		}

		LastKey = wParam;
		KeyPressed = true;

		if (LastKey == VK_F11)
		{
			SaveScreen();
		}

		/*
		//Can't see where it was supposed to work. Cut it out.
		if (( !GameInProgress ) && LastKey == 'R' &&
			GetKeyState( VK_CONTROL ) & 0x8000)
		{
			//RecordMode = !RecordMode;//BUGFIX: remove switching record mode in real time
		}
		*/

		{
			int nVirtKey = (int)wParam;
			int lKeyData = lParam;
			byte PST[256];
			GetKeyboardState(PST);

			word ascii_key;
			int result = ToAscii(nVirtKey, lKeyData, PST, &ascii_key, 0);

			WCHAR u_buf[5] = {};
			if (1 <= ToUnicode(nVirtKey, lKeyData, PST, u_buf, 4, 0))
			{
				//Valid UTF character
				wchar_t unicode_char = u_buf[0];
				if (1040 <= unicode_char && 1103 >= unicode_char)
				{
					//UTF code is in cyrillic range
					//Adjust ascii code to match sprite index in mainfont.gp file
					//Sprites 192 to 255 ('А' to 'я')
					//(taken from russian cossacks version ALL.GSC)
					ascii_key = unicode_char - 848;
				}
			}

			if (1 == result)
			{
				LastAsciiKey = ascii_key;
			}
			else
			{
				LastAsciiKey = 0;
			}

			AddKey(wParam, LastAsciiKey);
		}
		break;

	case WM_CLOSE:
		//Leave game and assign defeat
		IAmLeft();
		LOOSEANDEXITFAST();
		break;

	case WM_DESTROY:
		finiObjects();
		PostQuitMessage(0);
		exit(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
 * doInit - do work required for every instance of the application:
 *                create the window, initialize data
 */
void ProcessGSaveMap();
void EditorKeyCheck();
void ProcessSaveInSquares();
void TestGenMap();
bool ShowStatistics();
bool EnterChatMode = 0;

char ChatString[128];
wchar_t unicode_chat_string[128];

void ProcessChatKeys();
extern int WaitState;

bool RetryVideo = 0;

extern byte PlayGameMode;
extern bool GameExit;
extern int LastCTRLPressTime;
bool CheckFNSend(int idx);
void ProcessVotingKeys();
extern bool RESMODE;
extern bool OptHidden;
extern word NPlayers;
bool CheckFlagsNeed();
void SetGameDisplayModeAnyway(int SizeX, int SizeY);

//Many different key checks for various game modes
void GameKeyCheck()
{
	if (PlayGameMode == 1)
	{
		if (KeyPressed)
		{
			GameExit = true;
			RetryVideo = 0;
			KeyPressed = 0;
			return;
		}
	}

	ProcessVotingKeys();

	if (EnterChatMode)
	{
		ProcessChatKeys();
		return;
	}

	if (EditMapMode)
	{
		EditorKeyCheck();
		return;
	}

	if (KeyPressed)
	{
		KeyPressed = false;
		int wParam = LastKey;
		switch (wParam)
		{
		case VK_ESCAPE:
			ClearModes();
			BuildMode = false;
			GetCoord = false;
			curptr = 0;
			curdx = 0;
			curdy = 0;
			PauseMode = 0;
			SetDestMode = false;
			GoAndAttackMode = false;
			GUARDMODE = 0;
			PATROLMODE = 0;

			if (WaitState == 1)
				WaitState = 2;

			if (ShowGameScreen)
				ShowGameScreen = 2;

			AttGrMode = 0;
			break;
		case ' ':
			SpecCmd = 111;
			break;
		case 8:
			SpecCmd = 112;
			break;
		case 'U':
			if (Inform != 2)
			{
				Inform = 2;
			}
			else
			{
				Inform = 0;
			}
			MiniActive = 0;
			Recreate = 1;
			break;
		case 'M':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				SpecCmd = 114;
			}
			else
			{
				FullMini = !FullMini;
			}
			MiniActive = 0;
			Recreate = 1;
			break;
		case VK_F12:
			MenuType = 1;
			MakeMenu = true;
			break;
		case VK_F1:
			if (!CheckFNSend(0))
			{
				MenuType = 4;
				MakeMenu = true;
			}
			break;
		case VK_F2:
			CheckFNSend(1);
			break;
		case VK_F3:
			CheckFNSend(2);
			break;
		case VK_F4:
			CheckFNSend(3);
			break;
		case VK_F5:
			CheckFNSend(4);
			break;
		case VK_F6:
			CheckFNSend(5);
			break;
		case VK_F7:
			CheckFNSend(6);
			break;
		case VK_F8:
			CheckFNSend(7);
			break;
		case VK_F9:
			if (!CheckFNSend(8))
			{
				Creator = 4096 + 255;
			}
			break;
		case 192:
			HealthMode = !HealthMode;
			break;
		case 46:
			SpecCmd = 200;
			break;

			/*
			case 'D':
				if (!( GetKeyState( VK_CONTROL ) & 0x8000 ))
				{
					if (NPlayers < 2)
					{
						if (( GetKeyState( VK_SHIFT ) & 0x8000 ))
						{
							switch (HISPEED)
							{
							case 0:
								HISPEED = 1;
								break;
							case 1:
								HISPEED = 2;
								break;
							case 2:
								HISPEED = 3;
								break;
							default:
								HISPEED = 0;
								break;
							}
						}
					}
				}
				else
				{
					//CmdChangeSpeed();//BUGFIX: real time speed changing
				}
				break;
			*/

		case 'A':
			if (GetKeyState(VK_CONTROL) & 0x8000)
				SpecCmd = 1;
			else if (NSL[MyNation])
				GoAndAttackMode = 1;
			break;
		case 'S':
			if (GetKeyState(VK_CONTROL) & 0x8000)
				SpecCmd = 201;
			break;
		case 'W':
			break;
		case 'J':
			if (PlayGameMode == 2)
			{
				int ExRX = RealLx;
				int ExRY = RealLy;
				if (RealLx != 1024 || RealLy != 768)
				{
					SetGameDisplayModeAnyway(1024, 768);
				}
				ShowStatistics();
				if (RealLx != ExRX || RealLy != ExRY)
				{
					SetGameDisplayModeAnyway(ExRX, ExRY);
				}
			}
			break;
		case 'K':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				RealPause -= 2;
			}
			else
			{
				RealPause += 2;
			}
			break;
		case 'Q':
			LockGrid += 2;
			if (LockGrid > 3)
			{
				LockGrid = 0;
			}
			MiniActive = 0;
			Recreate = 1;
			break;
		case 'B':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				SpecCmd = 9;
			}
			else
			{
				SpecCmd = 10;
			}
			break;
		case 'Z':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				SpecCmd = 11;
			}
			else
			{
				//Select all units of the selected type on screen
				SpecCmd = 241;
			}
			break;
		case 'F':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				SpecCmd = 13;
			}
			else
			{
				SpecCmd = 14;
			}
			break;
		case VK_NUMPAD1:
			if (MEditMode)
			{
				EditMedia = 0;
			}
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 0 );
				}
				PlayerMask = 1;
			}
			break;
		case VK_NUMPAD2:
			if (MEditMode)EditMedia = 1;
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 1 );
				}
				PlayerMask = 2;
			}
			break;
		case VK_NUMPAD3:
			if (MEditMode)
			{
				EditMedia = 2;
			}
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 2 );
				}
				PlayerMask = 4;
			}
			break;
		case VK_NUMPAD4:
			if (MEditMode)
			{
				EditMedia = 3;
			}
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 3 );
				}
				PlayerMask = 8;
			}
			break;
		case VK_NUMPAD5:
			if (MEditMode)
			{
				EditMedia = 4;
			}
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 4 );
				}
				PlayerMask = 16;
			}
			break;
		case VK_NUMPAD6:
			if (MEditMode)
			{
				BlobMode = 1;
			}
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 5 );
				}
				PlayerMask = 32;
			}
			break;
		case VK_NUMPAD7:
			if (MEditMode)
			{
				BlobMode = -1;
			}
			else
			{
				if (NPlayers < 2 && ChangeNation)
				{
					SetMyNation( 6 );
				}
				PlayerMask = 64;
			}
			break;
		case VK_NUMPAD8:
			if (NPlayers < 2 && ChangeNation)
			{
				SetMyNation( 7 );
			}
			PlayerMask = 128;
			break;
		case 'I':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				InfoMode = !InfoMode;
			}
			else
			{
				if (Inform != 1)
				{
					Inform = 1;
				}
				else
				{
					Inform = 0;
				}
				MiniActive = 0;
				Recreate = 1;
			}
			break;

		case VK_CAPITAL:
			EgoFlag = !EgoFlag;
			break;

		case 'O':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				if (PlayGameMode == 2 || CheckFlagsNeed())
				{
					OptHidden = !OptHidden;
					if (!OptHidden)
					{
						Inform = 0;
					}
				};
			}
			else
			{
				TransMode = !TransMode;
				MiniActive = 0;
				Recreate = 1;
			}
			break;
		case 'P':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				SpecCmd = 113;
			}
			else
			{
				if (MultiTvar)
				{
					NeedToPopUp = 2;
					Inform = 0;
				}
			}
			break;
		case 19: //Pause
			if (tmtmt > 32 && !LockPause)
			{
				SpecCmd = 137;
			}
			break;
		case 13:
			if (!RESMODE)
			{
				EnterChatMode = 1;
				ClearKeyStack();
			}
			break;
		default:
			if (wParam >= '0' && wParam <= '9')
			{
				if (GetTickCount() - LastCTRLPressTime < kCtrlStickyTime)
				{
					CmdMemSelection(MyNation, wParam - '0');
				}
				else
				{
					CmdRememSelection(MyNation, wParam - '0');
				}
			}
		}
	}
}

void CreateFastLocking();
void AddHill();
bool DelCurrentAZone();
void SelectNextGridMode();
void ProcessMapOptions();
void ResearchIslands();
void EnterRandomParams();
void GenerateRandomRoad(int idx);
bool CheckCurve();
void ClearCurve();
extern bool ToolsHidden;
void SetFractalTexture();
void AutoSMSSet();
void UpdateAllPieces();

extern int PEN_RADIUS;
extern int PEN_BRIGHTNESS;
void LoadCurPixTexture(char* Name);

void EditorKeyCheck()
{
	if (KeyPressed)
	{
		KeyPressed = false;
		int wParam = LastKey;
		switch (wParam)
		{
		case VK_RIGHT:
			//if(DrawPixMode||DrawGroundMode)TexStDX=(TexStDX+1)&7;
			break;
		case VK_UP:
			//if(DrawPixMode||DrawGroundMode)TexStDY=(TexStDY-1)&7;
			break;
		case VK_LEFT:
			//if(DrawPixMode||DrawGroundMode)TexStDX=(TexStDX-1)&7;
			break;
		case VK_DOWN:
			//if(DrawPixMode||DrawGroundMode)TexStDY=(TexStDY+1)&7;
			break;
		case 'E':
			FastMode = !FastMode;
			break;
		case '0':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);
			break;
		case '1':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 1;
			else ReliefBrush = 1;
			break;
		case '2':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 2;
			else ReliefBrush = 2;
			break;
		case '3':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 3;
			else ReliefBrush = 3;
			break;
		case '4':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 4;
			else ReliefBrush = 4;
			break;
		case '5':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 5;
			else ReliefBrush = 5;
			break;
		case '6':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 6;
			else ReliefBrush = 9;
			break;
		case '7':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 7;
			else ReliefBrush = 20;
			break;
		case '8':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 8;
			else ReliefBrush = 50;
			break;
		case '9':
			//if(DrawPixMode||DrawGroundMode)STBRR(wParam);else
			if (GetKeyState(VK_CONTROL) & 0x8000)CostThickness = 9;
			else ReliefBrush = 100;
			break;
		case 'H':
			//FullScreenMode=!FullScreenMode;
			//GameNeedToDraw=true;
			//GSSetup800();
			RenderAllMap();
			break;
		case 'V':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				ClearModes();
				NeedToPopUp = 20;
			}
			else
			{
				ClearModes();
				NeedToPopUp = 22;
				//SetFractalTexture();
				/*
				switch(Light){
				case 0:SetLight(-5,10,20);
					break;
				case 1:SetLight(-5,12,17);
					break;
				case 2:SetLight(-5,15,15);
					break;
				case 3:SetLight(-5,17,12);
					break;
				case 4:SetLight(-5,20,10);
					break;
				case 5:SetLight(-5,17,12);
					break;
				case 6:SetLight(-5,15,15);
					break;
				case 7:SetLight(-5,12,17);
				};
				Light++;
				if(Light>7)Light=0;
				*/
			};
			break;
		case 13:
			if (!(GetKeyState(VK_CONTROL) & 0x8000))
			{
				//MakeMenu=true;
				//MenuType=3;
				EnterChatMode = 1;
				ClearKeyStack();
			}
			else KeyPressed = true;
			break;
		case 'J':
			//RSCRSizeX++;
			//ShowStatistics();
			break;
		case 19: //Pause
			if (!LockPause)SpecCmd = 137;
			//PauseMode=!PauseMode;
			break;
		case VK_ESCAPE:
			AttGrMode = 0;
			if (CheckCurve())
			{
				ClearCurve();
			}
			else
			{
				ClearModes();
				BuildMode = false;
				//BuildWall=false;
				GetCoord = false;
				curptr = 0;
				curdx = 0;
				curdy = 0;
				GUARDMODE = 0;
				PATROLMODE = 0;
				PauseMode = false;
				SetDestMode = false;
				GoAndAttackMode = false;
				NeedToPopUp = 1;
			};
			//if(PlayerMenuMode==2)
			//MainMenu.ShowModal();
			//Options.ShowModal();
			break;
		case VK_F12:
			//UnlockSurface();
			//CloseExplosions();
			//ShutdownMultiplayer();
			//PostMessage(hwnd, WM_CLOSE, 0, 0);
			MenuType = 1;
			MakeMenu = true;
			break;
		case 192:
			HealthMode = !HealthMode;
			break;
		case VK_F1:
			HelpMode = !HelpMode;
			break;
		case VK_F2:
			NeedToPopUp = 6;
			break;
		case VK_F3:
			NeedToPopUp = 15;
			break;
		case VK_F4:
			NeedToPopUp = 4;
			break;
		case VK_F5:
			if (WaterEditMode)
			{
				WaterEditMode = 1;
				NeedToPopUp = 8;
			}
			else
			{
				NeedToPopUp = 5;
			};
			break;
		case 'U':
			if (GetKeyState(VK_CONTROL) & 0x8000)CINFMOD = !CINFMOD;
			else if (Inform != 2)Inform = 2;
			else Inform = 0;
			MiniActive = 0;
			Recreate = 1;
			//CINFMOD=0;
			break;
		case 'F':
			//SVSC.Zero();
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				TestGenMap();
			}
			else FogMode = !FogMode;
			//HeightEditMode=false;
			//MEditMode=true;
			//EditMedia=5;
			break;
		case VK_F6:
			if (WaterEditMode)
			{
				WaterEditMode = 2;
				NeedToPopUp = 9;
			}
			else
			{
				ClearModes();
				MakeMenu = true;
				MenuType = 6;
			};
			break;
		case VK_F7:
			if (WaterEditMode)
			{
				WaterEditMode = 3;
				NeedToPopUp = 10;
			}
			else Reset3D();
			break;
		case VK_F8:
			if (WaterEditMode)
			{
				WaterEditMode = 4;
				NeedToPopUp = 11;
			}
			else
			{
				NeedToPopUp = 3;
			};
			break;
		case 'N':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				AutoSMSSet();
			}
			else
			{
				NeedToPopUp = 7;
				//HeightEditMode=3;
				//MEditMode=false;
				//EditMedia=5;
			};
			break;
		case 46:
			if (!DelCurrentAZone())SpecCmd = 200;
			break;

			/*
			case 'D':
				if (!( GetKeyState( VK_CONTROL ) & 0x8000 ))
				{
					if (( GetKeyState( VK_SHIFT ) & 0x8000 ))//&& PlayGameMode)
					{
						switch (HISPEED)
						{
						case 0:
							HISPEED = 1;
							break;
						case 1:
							HISPEED = 2;
							break;
						case 2:
							HISPEED = 3;
							break;
						default:
							HISPEED = 0;
							break;
						};
					};
	
				}
				else
				{
					//CmdChangeSpeed();//BUGFIX: real time speed changing
				}
				break;
			*/

		case 'A':
			if (GetKeyState(VK_CONTROL) & 0x8000)SpecCmd = 1;
			else if (NSL[MyNation])GoAndAttackMode = 1;
			break;
		case 'S':
			//ClearModes();
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				//EnterRandomParams();
			}
			else
			{
				switch (LockMode)
				{
				case 0:
					NeedToPopUp = 12;
					break;
				case 1:
					NeedToPopUp = 13;
					break;
				case 2:
					NeedToPopUp = 14;
					break;
				case 3:
					NeedToPopUp = 1;
					break;
				};
			};
			break;
		case 'W':
			if (GetKeyState(VK_CONTROL) & 0x8000)ProcessSaveInSquares();
			else PeaceMode = !PeaceMode;
			break;
			/*
		case 'N':
			switch(RSCRSizeX){
				case 800:RSCRSizeX=1024;
						break;
				case 1024:RSCRSizeX=800+32;
						break;
				case 1280:RSCRSizeX=1600;
					break;
				case 1600:RSCRSizeX=800;
					break;
				default:
					RSCRSizeX=800;
			};*/
			/*if(RSCRSizeX!=1024)RSCRSizeX=1024;
			else RSCRSizeX=800;*/
			//break;
		case 'C':
			//CINFMOD=!CINFMOD;
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				SpecCmd = 217;
			}
			else
			{
				WaterEditMode = 1;
				NeedToPopUp = 8;
			};
			break;
		case 'X':
			//if(GetKeyState(VK_CONTROL)&0x8000)SpecCmd=5;
			//else SpecCmd=6;
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				EraseAreas();
				rando(); //!!
				CreateTotalLocking();
				ResearchIslands();
			}
			else
			{
				ClearModes();
				DrawPixMode = 1;
				NeedToPopUp = 21;
			};
			break;
		case 'Q':
			LockGrid += 2; //++;
			if (LockGrid > 3)LockGrid = 0;
			MiniActive = 0;
			Recreate = 1;
			break;
		case 'B':
			if (GetKeyState(VK_CONTROL) & 0x8000)SpecCmd = 9;
			else SpecCmd = 10;
			//ClearMaps();
			//CreateUnitsLocking();
			break;
		case 'Z':
			//if(DrawPixMode){
			//	PerformPixUndo();
			//};
			//if(GetKeyState(VK_CONTROL)&0x8000)SpecCmd=11;
			//else SpecCmd=12;

			break;
			//case VK_F11:
			//	WaterCorrection();
			//	break;
		case VK_F9:
			MEditMode = false;
			HeightEditMode = false;
			LockMode = 0;
			Creator = 4096 + 255;
			NeedToPopUp = 1;
			break;
		case VK_NUMPAD1:
		SetMyNation( 0 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 1;
			break;
		case VK_NUMPAD2:
		SetMyNation( 1 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 2;
			break;
		case VK_NUMPAD3:
		SetMyNation( 2 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 4;
			break;
		case VK_NUMPAD4:
		SetMyNation( 3 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 8;
			break;
		case VK_NUMPAD5:
		SetMyNation( 4 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 16;
			break;
		case VK_NUMPAD6:
		SetMyNation( 5 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 32;
			break;
		case VK_NUMPAD7:
		SetMyNation( 6 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 64;
			break;
		case VK_NUMPAD8:
		SetMyNation( 7 );
			MEditMode = false;
			HeightEditMode = false;
			PlayerMask = 128;
			break;
		case 'I':
			if (GetKeyState(VK_CONTROL) & 0x8000)InfoMode = !InfoMode;
			else if (Inform != 1)Inform = 1;
			else Inform = 0;
			MiniActive = 0;
			Recreate = 1;
			//InfoMode=1;
			break;
		case 'O':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				OptHidden = !OptHidden;
			}
			else
			{
				TransMode = !TransMode;
				MiniActive = 0;
				Recreate = 1;
			};
			break;
			//WaterCorrection();
			//if(MsPerFrame)MsPerFrame--;
		case 'P':
			//MsPerFrame++;
			//if(GetKeyState(VK_CONTROL)&0x8000)RotatePhiI();
			//RotatePhi();
			NeedToPopUp = 2;
			break;
		case 'R':
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				//ProcessMapOptions();
			}
			else
			{
				MEditMode = false;
				HeightEditMode = false;
				MakeMenu = true;
				MenuType = 31;
			};
			break;
		case 'L':
			/*
			if(!MiniMode)SetMiniMode();
			else ClearMiniMode();
			MEditMode=false;
			*/
			//ReverseLMode();
			break;
		case 'T':
			//HeightEditMode=false;
			//ChoosePosition=true;
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				ToolsHidden = !ToolsHidden;
			};
			break;
		case 'G':
			//if(GetKeyState(VK_SHIFT)&0x8000)CreateMapShot();
			if (GetKeyState(VK_CONTROL) & 0x8000)SelectNextGridMode();
			else SaveScreen();
			//SVSC.Grids=!SVSC.Grids;
			//SVSC.RefreshScreen();
			break;
		case 107: //NUM +
			if (HeightEditMode)HiStyle = 1;
			break;
		case 109: //NUM -
			if (HeightEditMode)HiStyle = 2;
			break;
		case 106: //NUM *
			if (HeightEditMode)HiStyle = 3;
			break;
		case 111: //NUM /
			if (HeightEditMode)HiStyle = 4;
			break;
		case 'M': //NUM 0
			//if(HeightEditMode)HiStyle=5;
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				//AddHill();
				GenerateRandomRoad(5);
			}
			else FullMini = !FullMini;
			MiniActive = 0;
			Recreate = 1;
			break;
		case 33:
			if (HeightEditMode)HiStyle = 7;
			break;
		case 34:
			if (HeightEditMode)HiStyle = 8;
			break;
		case 36:
			if (HeightEditMode)HiStyle = 9;
			break;
		default:
			if (wParam >= '0' && wParam <= '9')
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					int v = wParam - '0';
					SHIFT_VAL = SHIFT_VAL * 10 + v;
				}
				else
				{
					if (GetTickCount() - LastCTRLPressTime < kCtrlStickyTime)
					{
						CmdMemSelection(MyNation, wParam - '0');
					}
					else CmdRememSelection(MyNation, wParam - '0');
					//if(GetKeyState(VK_CONTROL)&0x8000)
					//	CmdMemSelection(MyNation,wParam-'0');
					//else CmdRememSelection(MyNation,wParam-'0');
				};
			};
		};
	};
};
void SERROR();
void SERROR1();
void SERROR2();
bool PalDone;

bool InitScreen()
{
	PalDone = false;
	CreateDDObjects(hwnd);
	PalDone = false;
	LoadPalette("agew_1.pal");
	if (!DDError)
	{
		LockSurface();

		UnlockSurface();

		if (!RealScreenPtr)
		{
			MessageBox(hwnd, "Unable to initialise Direct Draw. It is possible that hardware acceleration is turned off.",
			           "Loading error[1]", MB_ICONSTOP);
			exit(0);
		}

		return true;
	}
	else
	{
		PlayEffect(0, 0, 0);
	}
	return false;
}

BOOL CreateRGBDDObjects(HWND hwnd);

bool InitRGBScreen()
{
	PalDone = false;
	CreateRGBDDObjects(hwnd);
	PalDone = false;
	if (!DDError)
	{
		LockSurface();

		UnlockSurface();

		return true;
	}
	return false;
}

BOOL CreateRGB640DDObjects(HWND hwnd);

bool InitRGB640Screen()
{
	CreateRGB640DDObjects(hwnd);
	if (!DDError)
	{
		LockSurface();

		UnlockSurface();

		return true;
	}
	return false;
}

bool ProcessMessages();
extern int PlayMode;
void StopPlayCD();
void PlayRandomTrack();
bool First = 1;
bool ProcessMessagesEx();
void ClearRGB();

extern bool Lpressed;

void FilesExit();

//Register winapi window class, init DirectDraw, sounds and cursor
static BOOL doInit(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASS wc;
	char buf[256];

	//set up and register window class
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = NAME;
	RegisterClass(&wc);

	if (window_mode)
	{
		hwnd = CreateWindow(
			NAME,
			TITLE,
			window_style,
			50, 50,
			RealLx,
			RealLy,
			NULL,
			NULL,
			hInstance,
			NULL
		);
		ResizeAndCenterWindow();
	}
	else
	{
		hwnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			NAME,
			TITLE,
			WS_POPUP,
			0, 0,
			screen_width,
			screen_height,
			NULL,
			NULL,
			hInstance,
			NULL
		);
	}
	if (!hwnd)
	{
		return FALSE;
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);

	UpdateWindow(hwnd);

	CDIRSND.CreateDirSound(hwnd);

	CDS = &CDIRSND;

	LoadSounds("SoundList.txt");

	ResFile F = RReset("version.dat");
	if (F != INVALID_HANDLE_VALUE)
	{
		word B = 0;
		RBlockRead(F, &B, 2);
		RClose(F);
		if (B > 102)
		{
			MessageBox(hwnd, "Unable to use this testing version.", "WARNING!", 0);
			FilesExit();
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		}
	}

	if (!Loading())
	{
		FilesExit();
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return 0;
	}

	CurrentSurface = FALSE;

	//create the main DirectDraw object
	PalDone = false;

	KeyPressed = false;

	//Fullscreen? Prepare for small not stretched menu
	if (!window_mode)
	{
		//Set initial window resolution to native screen resolution
		if (1920 < screen_width)
		{
			//Limit max resolution for menu screen to fullhd
			//Also necessary for correct offsets in stats screen
			screen_width = 1920;
			screen_height = 1080;
		}
		RealLx = screen_width;
		RealLy = screen_height;
	}

	//Create the screen object with RealLx x RealLy resolution
	CreateDDObjects(hwnd);

	CHKALL();

	if (!DDError)
	{
		LockSurface();
		UnlockSurface();

		LockSurface();
		UnlockSurface();

		if (!RealScreenPtr)
		{
			MessageBox(hwnd, "Unable to initialise Direct Draw. It is possible that hardware acceleration is turned off.",
			           "Loading error[2]", MB_ICONSTOP);
			exit(0);
		}

		if (SetTimer(hwnd, TIMER_ID, 20, nullptr))
		{
			return TRUE;
		}
	}

	wsprintf(buf, "Direct Draw Init Failed\n");
	MessageBox(hwnd, buf, "ERROR", MB_OK);
	finiObjects();
	DestroyWindow(hwnd);
	return FALSE;
}

void AddDestn(byte x, byte y);
void ProcessNewMonsters();
void InitXShift();
void HandleMines();
void ControlGates();
void HealWalls();
void ProcessDeathList();
void ProcessEconomy();
void HandleMission();

void CheckTops()
{
	int NT = NAreas * NAreas;
};

void ResearchCurrentIsland(byte Nat);
void ResearchBestPortToFish(byte Nat);
extern int NInGold[8];
extern int NInIron[8];
extern int NInCoal[8];
extern int WasInGold[8];
extern int WasInIron[8];
extern int WasInCoal[8];
extern bool Tutorial;
void ProcessCostPoints();
void CheckArmies(City*);
void CheckGP();

void CmdSetSpeed(byte);
bool NOPAUSE = 1;
void EnumPopulation();
extern bool TutOver;
void HandleShar(Nation* NT);
void AddRandomBlobs();
void ProcessMapAutosave();

void ProcessGuard();
void DecreaseVeruVPobedu();

void PreDrawGameProcess()
{
	//Autosave in map editor every 5 min
	ProcessMapAutosave();

	//Something about ship traces?
	AddRandomBlobs();

	//DirectX related sound procedures
	CDS->ProcessSoundSystem();

	if (NOPAUSE)
	{
		//Refresh "peasants in mines" amounts
		for (int w = 0; w < 8; w++)
		{
			WasInGold[w] = NInGold[w];
			WasInCoal[w] = NInCoal[w];
			WasInIron[w] = NInIron[w];
			NInGold[w] = 0;
			NInCoal[w] = 0;
			NInIron[w] = 0;
		}
	}

	for (int g = 0; g < 8; g++)
	{
		if (CITY[g].Account < 0)
		{
			CITY[g].Account = 0;
		}
	}

	//Check if fast/slow mode was changed
	if (exFMode != SpeedSh)
	{
		CmdSetSpeed(exFMode);
	}

	//Calculate population values for all players
	if ((tmtmt % 256) == 32)
	{
		EnumPopulation();
	}

	//???
	ProcessCostPoints();

	//Auto-attack or guard logic?
	ProcessGuard();

	if (NOPAUSE)
	{
		//Refresh market exchange rates
		ProcessEconomy();

		//Take a guess...
		ProcessDeathList();
	}

	int tt = tmtmt % 256;

	//Remove dead wall cells
	HealWalls();

	if (LastAttackDelay)
	{
		LastAttackDelay--;
	}

	for (int i = 0; i < 8; i++)
	{
		memset(NATIONS[i].SoundMask, 0, 2048);
	}

	//NDestn = 0
	InitDestn();

	//Many different key checks for various game modes
	GameKeyCheck();

	//Take a guess...
	ProcessMessages();

	if (2 == tmtmt % 41)
	{
		//Crawl the map and enumerate objects
		CreateStrategyInfo();
	}

	//Take a guess...
	ProcessFishing();

	//Open and close gates, check for squashed units
	ControlGates();

	//Order the acquisition of resources from mines
	HandleMines();

	//Calculate XShift for... mirroring and water and stuff?
	InitXShift();

	//Long live compiler optimization!
	int tmtmt_div_256 = tmtmt / 256;
	int tmtmt_mod_256 = tmtmt % 256;
	if (1 == tmtmt_mod_256)
	{
		//Each player is processed every 256 * 8 internal tick
		int nation_byte = tmtmt_div_256 % 8;
		//Some island AI logic
		ResearchCurrentIsland(nation_byte);
	}

	int tmtmt_div_128 = tmtmt / 128;
	int tmtmt_mod_128 = tmtmt % 128;
	if (7 == tmtmt_mod_128)
	{
		//Each player is processed every 256 * 8 internal tick
		int nation_byte = tmtmt_div_128 % 8;

		//Take a guess...
		ResearchBestPortToFish(nation_byte);
	}

	if (SHOWSLIDE)
	{
		//Count peasants and city centers?
		WinnerControl(false);
	}

	if (tima != time(nullptr))
	{
		Flips = tmtim;
		tmtim = 0;
		tima = time(nullptr);
	}

	if (0 == tmtmt % 64)
	{
		LASTRAND = rando();
		LASTIND = rpos;
	}

	if (NOPAUSE)
	{
		tmtim++;

		tmtmt++;

		REALTIME += FrmDec;

		for (int g = 0; g < 8; g++)
		{
			if (CITY[g].AutoEraseTime)
			{
				CITY[g].AutoEraseTime--;
				if (!CITY[g].AutoEraseTime)
				{
					int SCORES[8];
					for (int i = 0; i < 8; i++)
					{
						SCORES[i] = CITY[i].Account;
					}

					for (int i = 0; i < MAXOBJECT; i++)
					{
						OneObject* OB = Group[i];
						if (OB && (!OB->Sdoxlo) && (OB->NNUM == g))
						{
							//erasing
							OB->delay = 6000;

							if (OB->LockType)
							{
								OB->RealDir = 32;
							}

							OB->Die();
							OB = Group[i];

							if (OB)
							{
								OB->Sdoxlo = 2500;
							}
						}
					}

					for (int i = 0; i < 8; i++)
					{
						CITY[i].Account = SCORES[i];
					}
				}
			}
		}
	}

	if (NOPAUSE)
	{
		ProcessSprites();
	}

	NMONS = 0;

	//Transport ships logic
	HandleTransport();

	int tmtmt_mod_8 = tmtmt % 8;
	//Process production queues, upgrades, farm growing etc
	if (0 == tmtmt_mod_8)
	{
		//For yourself - always
		CITY[0].ProcessCreation();
	}
	if (NOPAUSE)
	{
		//Every other player is processed every 8th internal tick
		if (tmtmt_mod_8 == 1)
		{
			CITY[1].ProcessCreation();
		}
		if (tmtmt_mod_8 == 2)
		{
			CITY[2].ProcessCreation();
		}
		if (tmtmt_mod_8 == 3)
		{
			CITY[3].ProcessCreation();
		}
		if (tmtmt_mod_8 == 4)
		{
			CITY[4].ProcessCreation();
		}
		if (tmtmt_mod_8 == 5)
		{
			CITY[5].ProcessCreation();
		}
		if (tmtmt_mod_8 == 6)
		{
			CITY[6].ProcessCreation();
		}

		if (tmtmt_mod_8 == 7 || TutOver)
		{
			HandleMission();
			TutOver = 0;
		}

		int xt = (tmtmt % 256);
		GNFO.Process();

		for (int i = 0; i < 8; i++)
		{
			Nation* NT = NATIONS + i;
			NT->Harch += NT->NGidot * ResPerUnit;
			int mult = 2000 >> SpeedSh;
			int DHarch = NT->Harch / mult;
			if (DHarch)
			{
				if (XRESRC( i, EatenRes ) > DHarch)
				{
					AddXRESRC( i, EatenRes, -DHarch );
					NATIONS[i].ResOnLife[EatenRes] += DHarch;
					NT->AddResource(EatenRes, -DHarch);
					NT->Harch -= mult * DHarch;
					if (!NT->Harch)
					{
						NT->Harch = 1;
					}
				}
				else
				{
					SetXRESRC( i, EatenRes, 0 );
					NT->Harch = 0;
				}
			}
			if (!NT->Harch)
			{
				if (XRESRC( i, FoodID ))NT->Harch = 1;
				NATIONS[i].ResOnLife[FoodID]++;
			}
			mult = 2000000 >> SpeedSh;
			for (int j = 0; j < 8; j++)
			{
				int R = NT->ResRem[j];
				R += NT->ResSpeed[j] * 100;
				div_t dd = div( R, mult );
				R = dd.rem;
				AddXRESRC( i, j, -dd.quot );
				NATIONS[i].ResOnLife[j] += dd.quot;
				if (XRESRC( i, j ) < 0)
				{
					NATIONS[i].ResOnLife[j] += XRESRC( i, j );
					SetXRESRC( i, j, 0 );
				}
				NT->AddResource(j, -dd.quot);
				NT->ResRem[j] = R;
				if (j == GoldID)
				{
					if (XRESRC( i, j ) < 2)
					{
						NT->GoldBunt = true;
					}
					else
					{
						NT->GoldBunt = false;
					}
				}
			}
		}


		ProcessNewMonsters();

		ObjTimer.Handle();
	}

	//Process explosion animations
	ProcessExpl();

	for (int i = 0; i < 8; i++)
	{
		//Place observation balloon
		HandleShar(NATIONS + i);
	}

	//Something about area linking?
	ProcessDynamicalTopology();
}

bool ProcessMessages();
extern word NPlayers;
void CmdSaveNetworkGame(byte NI, int ID, char* Name);
int SaveTime;
extern char SaveFileName[128];
void ProcessNature();
bool NeedEBuf = 0;
int GLOBALTIME = 0;
int PGLOBALTIME = 0;
int PitchTicks = 0;
int MaxPingTime = 0;
int RealPause = 0;
int RealStTime = 0;
int RealGameLength = 0;
int CurrentStepTime = 80;
unsigned long GetRealTime();

int SUBTIME = 0;
void ProcessScreen();
void GSYSDRAW();
extern int StepX;
extern int StepY;
int TAverage = 50;

void WaitToTime(int Time)
{
	int dt0 = int(Time) - int(GetRealTime());
	bool DoDraw = dt0 > (TAverage >> 2);
	do
	{
		if (DoDraw)
		{
			int T0 = GetRealTime();
			if (T0 - Time < 0)
			{
				int tt = T0;
				ProcessScreen();
				GSYSDRAW();
				int dt = GetRealTime() - tt;
				TAverage = (TAverage + TAverage + TAverage + dt) >> 2;
				SUBTIME += GetRealTime() - T0;
			}
		}
		ProcessMessages();
	}
	while (int(Time) - int(GetRealTime()) > 0);
	int ttx = GetRealTime();
	//SUBTIME=0;
}

int NeedCurrentTime = 0;
extern bool PreNoPause;
extern EXBUFFER EBufs[MaxPL];
void StopPlayCD();
void ProcessUpdate();
extern byte CaptState;
extern byte SaveState;
void WritePitchTicks();
void ReadPichTicks();
void ShowCentralText0(char* sss);
void CmdChangePeaceTimeStage(int Stage);
int PrevCheckTime = 0;
extern int PeaceTimeStage;
extern int PeaceTimeLeft;

void PostDrawGameProcess()
{
	RGAME.TryToFlushNetworkStream(0);
	if (PlayGameMode == 0 && NPlayers < 2)
	{
		PitchTicks = 0;
	}

	if (PlayGameMode == 0)
	{
		if (MaxPingTime)
		{
			WaitToTime(NeedCurrentTime);
		}
		else
		{
			PitchTicks = 0;
		}
	}

	if (PlayGameMode)
	{
		ReadPichTicks();
		if (PitchTicks)
		{
			MaxPingTime = 1;
		}
		else
		{
			MaxPingTime = 0;
		}
	}
	else
	{
		WritePitchTicks();
	}

	ProcessNature();

	NeedEBuf = 0;

	GLOBALTIME++;

	if (RealStTime == 0)
	{
		RealGameLength = 0;
	}

	NeedCurrentTime += CurrentStepTime;

	if (GLOBALTIME - PGLOBALTIME > PitchTicks)
	{
		CurrentStepTime -= CurrentStepTime >> 5;
		RealGameLength = GetRealTime() - RealStTime;
		HandleMultiplayer();

		SYN.Copy(&SYN1);
		PreNoPause = 0;
		ExecuteBuffer();

		if (PreNoPause)
		{
			NOPAUSE = 0;
		}

		PGLOBALTIME = GLOBALTIME;
		RealStTime = GetRealTime();

		if (PlayGameMode)
		{
			ReadPichTicks();
			if (PitchTicks)
			{
				MaxPingTime = 1;
			}
		}
		else
		{
			if (NPlayers > 1 && MaxPingTime)
			{
				if (CurrentStepTime)
				{
					PitchTicks = 4 + ((MaxPingTime) / CurrentStepTime);
				}
				else
				{
					PitchTicks = 0;
				}
			}
			else
			{
				PitchTicks = 0;
			}
			WritePitchTicks();
		}
	}

	if (!HISPEED)
	{
		SHOWSLIDE = true;
	}
	else
	{
		SHOWSLIDE = !div( tmtmt, HISPEED + 1 ).rem;
	}

	int difTime = GetRealTime() - AutoTime;

	ProcessUpdate();

	int MaxDT = 60000;

	switch (SaveState)
	{
	case 1:
		MaxDT = 60000 * 2;
		break;
	case 2:
		MaxDT = 60000 * 4;
		break;
	case 3:
		MaxDT = 60000 * 6;
		break;
	case 4:
		MaxDT = 60000 * 8;
		break;
	case 5:
		MaxDT = 60000 * 10;
		break;
	case 6:
		MaxDT = 60000 * 2000;
		break;
	}

	if (difTime > MaxDT && !(PlayGameMode || SaveState == 6))
	{
		if (NPlayers > 1)
		{
			for (int i = 0; i < NPlayers; i++)
			{
				if (EBufs[i].Enabled)
				{
					if (PINFO[i].PlayerID == MyDPID)
					{
						int NP = 0;
						for (int j = 0; j < NPlayers; j++)
						{
							if (EBufs[j].Enabled)
							{
								NP++;
							}
						}
						char cc1[128];
						sprintf(cc1, "NetAutoSave %d players", NP);
						CmdSaveNetworkGame(MyNation, 0, cc1);
					}
					i = 100;
				}
			}
			//SaveGame("AUTO.sav",SaveFileName,0);
		}
		else
		{
			if (!EditMapMode)
			{
				if (NATIONS[MyNation].VictState != 1 && !SCENINF.LooseGame)
				{
					ShowCentralText0(GetTextByID("Autosaving"));
					FlipPages();
					SaveGame("AUTO.sav", "auto.sav", 0);
				}
			}
		}
		AutoTime = GetRealTime();
	}

	if (!PrevCheckTime)
	{
		PrevCheckTime = GetRealTime();
	}

	if (GetRealTime() - PrevCheckTime > 90000)
	{
		PrevCheckTime = GetRealTime();
		if (PeaceTimeLeft / 60 < PeaceTimeStage)
		{
			CmdChangePeaceTimeStage(PeaceTimeLeft / 60);
		}
	}

	/* Multiplayer save functions
	if(NPlayers > 1 && MyDPID == ServerDPID && SaveTime - GetRealTime() > 60000*5)
	{
		CmdSaveNetworkGame(MyNation, GetRealTime(), "NETWORK SAVE");
		SaveTime = GetRealTime();
	}
	*/

	if (0 == prev_postdraw_time)
	{
		prev_postdraw_time = GetRealTime();
	}


	unsigned long time_since_last_call = 0;
	do
	{
		ProcessMessages();
		if (PauseMode)
		{
			GameKeyCheck();
		}
		time_since_last_call = GetRealTime() - prev_postdraw_time;
	}
	while (PauseMode || time_since_last_call < kPostDrawInterval);

	prev_postdraw_time = GetRealTime();
}

void InitWaves();
void AllGame();

extern byte MI_Mode;
extern int RES[8][8];

void PrepareToEdit()
{
	ClearMStack();
	MI_Mode = 1;
	ReliefBrush = 3;
	TerrBrush = 2;
	EditMedia = 0;
	HeightEditMode = false;
	MEditMode = false;
	EditMapMode = true;
	FogMode = 0;
	HelpMode = true;
	ChoosePosition = false;
	CheapMode = false;
	NMyUnits = 1;
	NThemUnits = 1;
	AutoTime = GetRealTime() + 180000;
	ObjTimer.~TimeReq();
	InitWaves();
	PeaceMode = false;
	LockMode = 0;
	SaveFileName[0] = 0;
	LockBars.Clear();
	UnLockBars.Clear();
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			RES[i][j] = 50000;
		}
	}
}

byte PlayGameMode = 0;

extern char CurrentMap[64];
extern int TIMECHANGE[8];
extern int AddTime;
extern int NeedAddTime;
extern byte XVIIIState;
extern char RECFILE[128];

void PerformNewUpgrade(Nation* NT, int UIndex, OneObject* OB);

bool IsGameActive();

void PrepareToGame()
{
	if (!PlayGameMode)
	{
		if (NPlayers > 1 && (IsGameActive() || use_gsc_network_protocol) && !RecordMode)
		{
			RecordMode = true;
			sprintf(RECFILE, "Autorecord\\%s", CurrentMap);
		}
	}

	RecordMode = true; //BUGFIX: always turn on recording
	if (RecordMode && !PlayGameMode)
	{
		RGAME.StartRecord(CurrentMap);
	}

	MI_Mode = 1;
	memset(TIMECHANGE, 0, sizeof TIMECHANGE);
	AddTime = 0;
	NeedAddTime = 0;

	ClearMStack();

	ReliefBrush = 3;
	TerrBrush = 2;
	EditMedia = 0;
	HeightEditMode = false;
	MEditMode = false;
	EditMapMode = false;
	FogMode = 1;
	HelpMode = false;
	ChoosePosition = false;
	CheapMode = false;
	NMyUnits = 1;
	NThemUnits = 1;

	SaveTime = GetRealTime();
	AutoTime = GetRealTime();
	ObjTimer.~TimeReq();

	InitWaves();

	PeaceMode = false;
	LockMode = 0;
	SaveFileName[0] = 0;

	/*
	//BUGFIX: Do not default to fast mode when starting a game
	FrmDec = 2;
	SpeedSh = 1;
	exFMode = 1;
	*/

	CurrentStepTime = 80;
}

lpCHAR FLIST[4096];
int NFILES = 0;

void CreateRadio();
extern int ScrollSpeed;
void UnLoading();

//Delete random generated *.m3d map files
void EraseRND()
{
	char** RNDF = nullptr;
	int NRND = 0;
	int MaxRND = 0;
	DWORD* RndData = nullptr;
	word* Ridx = nullptr;

	WIN32_FIND_DATA FD;
	HANDLE HF = FindFirstFile("RN? *.m3d", &FD);
	if (HF != INVALID_HANDLE_VALUE)
	{
		bool r = true;
		do
		{
			if (NRND >= MaxRND)
			{
				MaxRND += 300;
				RNDF = (char**)realloc(RNDF, 4 * MaxRND);
				RndData = (DWORD*)realloc(RndData, 2 * MaxRND);
				Ridx = (word*)realloc(Ridx, 2 * MaxRND);
			}
			Ridx[NRND] = NRND;
			RNDF[NRND] = new char[strlen(FD.cFileName) + 1];
			strcpy(RNDF[NRND], FD.cFileName);
			RndData[NRND] = FD.ftCreationTime.dwHighDateTime;
			NRND++;
			r = FindNextFile(HF, &FD) != 0;
		}
		while (r);
		if (NRND > 3)
		{
			SortClass SORT;
			SORT.CheckSize(NRND);
			memcpy(SORT.Parms, RndData, 4 * NRND);
			memcpy(SORT.Uids, Ridx, 2 * NRND);
			SORT.NUids = NRND;
			SORT.Sort();
			memcpy(Ridx, SORT.Uids, 2 * NRND);
			SORT.Copy(Ridx);
			for (int i = 0; i < NRND - 3; i++)
			{
				DeleteFile(RNDF[Ridx[i]]);
			}
		}
		if (NRND)
		{
			for (int i = 0; i < NRND; i++)
			{
				free(RNDF[i]);
			}
			free(RNDF);
			free(Ridx);
			free(RndData);
		}
	}
}

bool FilesInit();
void FilesExit();
void PlayCDTrack(int Id);
void PlayRandomTrack();
extern int PlayMode;
unsigned long GetRealTime();

//Create "Cossacks.reg" with Microsoft DirectPlay key
void CreateReg()
{
	char path[300];
	char path1[350];
	GetCurrentDirectory(300, path);
	int ps1 = 0;
	int ps = 0;
	char c;
	do
	{
		c = path[ps];
		if (c == '\\')
		{
			path1[ps1] = '\\';
			path1[ps1 + 1] = '\\';
			ps1 += 2;
		}
		else
		{
			path1[ps1] = c;
			ps1++;
		};
		ps++;
	}
	while (c);
	GFILE* f = Gopen("Cossacks.reg", "w");
	Gprintf(
		f,
		"REGEDIT4\n[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\DirectPlay]\n[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\DirectPlay\\Applications]\n[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\DirectPlay\\Applications\\Cossacks]\n\"CommandLine\"=\"\"\n\"CurrentDirectory\"=\"%s\"\n\"File\"=\"dmcr.exe\"\n\"Guid\"=\"{5BFDB060-06A4-11d0-9C4F-00A0C705475f}\"\n\"Path\"=\"%s\"\n",
		path1, path1);
	Gclose(f);
}

typedef bool tpShowDialog(int NModes, int* Sizex, int* Sizey, int* Current);
tpShowDialog* lpShowDialog;
extern int ModeLX[32];
extern int ModeLY[32];
extern int NModes;
bool EnumModesOnly();

int ROLL = 1;

void NRFUNC()
{
	ROLL = 0;
}

void SFLB_InitDialogs();

extern bool RUNMAPEDITOR;
extern bool RUNUSERMISSION;
extern char USERMISSPATH[128];

void TestHash();
void CheckIntegrity();
extern bool TOTALEXIT;
int GetRankByScore(int Score);

void StartExplorer();
void FinExplorer();

void __declspec( dllexport ) SFINIT2_InitLAND();

int PASCAL WinMain(
		HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow
	)
{
	char* ss = strstr(lpCmdLine, "/MAPEDITOR");
	if (ss)
	{
		RUNMAPEDITOR = 1;
		strcpy(USERMISSPATH, ss + 11);
	}
	else
	{
		ss = strstr(lpCmdLine, "/MISSION");
		if (ss)
		{
			RUNUSERMISSION = 1;
			strcpy(USERMISSPATH, ss + 9);
		}
	}

	if (strstr(lpCmdLine, "/window"))
	{
		window_mode = true;
	}
	else
	{
		window_mode = false;
	}

	if (strstr(lpCmdLine, "/borderless"))
	{
		window_mode = true;
		window_style = WS_POPUP;
	}

	//Init DirectDraw and find possible resolutions
	EnumModesOnly();

	//Create "Cossacks.reg" with Microsoft DirectPlay key
	CreateReg();

	//Load unrar.dll, call CGSCset::gOpen() to load archives
	if (!FilesInit())
	{
		FilesExit();
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	}

	//Delete random generated *.m3d map files
	EraseRND();

	//Pointer to the DirectDraw screen buffer
	ScreenPtr = nullptr;

	ChangeNation = false;
	MultiTvar = false;
	MEditMode = false;
	WaterEditMode = false;

	Shifter = 5;
	Multip = 0;
	AutoTime = 0;
	BlobMode = 0;
	CostThickness = 4;
	EditMedia = 0;
	CreateRadio();
	SpecCmd = 0;
	sfVersion = 285;
	Quality = 2;

	RealLx = 1024;
	RealLy = 768;
	exRealLx = 1024;
	exRealLy = 768;

	WarSound = 0;
	WorkSound = 0;
	OrderSound = 0;
	MidiSound = 0;

	//Zero 3D Bars variables (?)
	InitObjs3();

	//Load settings
	GFILE* fff = Gopen("mode.dat", "rt");
	ScrollSpeed = 5;
	if (fff)
	{
		//Distinguish between last window adn fullscreen resolutions
		int ex_window_x, ex_window_y, ex_x, ex_y;
		int dummy;
		//7th value was FPSTime
		Gscanf(fff, "%d%d%d%d%d%d%d%d%d%d%d%d",
		       &ex_window_x, &ex_window_y, &ex_x, &ex_y,
		       &WarSound, &OrderSound, &OrderSound, &MidiSound,
		       &dummy, &ScrollSpeed, &exFMode, &PlayMode);
		Gclose(fff);

		//Set last 'global resolution' according to current mode
		if (window_mode)
		{
			exRealLx = ex_window_x;
			exRealLy = ex_window_y;
			ex_other_RealLx = ex_x;
			ex_other_RealLy = ex_y;
		}
		else
		{
			exRealLx = ex_x;
			exRealLy = ex_y;
			ex_other_RealLx = ex_window_x;
			ex_other_RealLy = ex_window_y;
		}
	}
	GFILE* rec_settings_file = Gopen("rec.dat", "rt");
	if (rec_settings_file)
	{
		Gscanf(rec_settings_file, "%d%s", &RecordMode, &RECFILE);
		Gclose(rec_settings_file);
	}

	//Look if loaded values match possible screen resolutions
	bool ExMode = 0;
	for (int i = 0; i < NModes; i++)
	{
		if (ModeLX[i] == exRealLx && ModeLY[i] == exRealLy)
		{
			ExMode = 1;
		}
	}

	if (!ExMode)
	{
		//Loaded resolution not possible, reset do default
		exRealLx = 1024;
		exRealLy = 768;
	}

	//Save native display resolution for future use
	screen_width = GetSystemMetrics(SM_CXSCREEN);
	screen_height = GetSystemMetrics(SM_CYSCREEN);

	//Calculate native resolution aspect ratio
	double scale = 0.01;
	screen_ratio = (double)screen_width / screen_height;
	screen_ratio = (int)(screen_ratio / scale) * scale;

	WindX = 0;
	WindY = 0;
	WindX1 = 1023;
	WindY1 = 767;
	WindLx = 1024;
	WindLy = 768;

	MSG msg;

	tima = 0;
	PlayerMask = 1;
	Flips = 0;
	tmtim = 0;

	HealthMode = false;
	InfoMode = true;
	DeathMode = false;
	AttackMode = false;

	//Zero FishMap pointer
	InitFishMap();

	//Init Gates[32] array
	SetupGates();

	LockGrid = false;

	FILE* Fx = fopen("cew.dll", "r");
	if (!Fx)
	{
		MessageBox(nullptr, "CEW.DLL not found. Unable to run Cossacks.", "Error...", MB_ICONERROR);
		return 0;
	}
	else
	{
		fclose(Fx);
	}

	//Init buffers for national units?
	SetupNatList();

	//Something about fog?
	makeFden();

	PlayerMenuMode = 1;

	Creator = 4096 + 255;
	xxx = 0;
	cadr = 0;

	//MouseZones?
	InitZones();

	//Order execution buffer position = 0?
	InitEBuf();

	TransMode = false;
	MUSTDRAW = false;

	//Calculate XShift for... mirroring and water and stuff?
	InitXShift();

	//Read players.txt, load dialog resources
	SFLB_InitDialogs();

	//Water colors and buffers
	InitWater();

	//Load fonts(?)
	LoadRLC("xrcross.rlc", &RCross);

	memset(Events, 0, sizeof Events);

	//Probably just to define PREVT
	GetRealTime();

	//Register winapi window class, init DirectDraw, sounds and cursor
	if (!doInit(hInstance, nCmdShow))
	{
		return FALSE;
	}

	//Load specific palette and fog resources (alphas etc)
	LoadFog(2);
	LoadPalette("2\\agew_1.pal");

	//Init DirectPlay and DPInfo structure
	SetupMultiplayer(hInstance);

	//Init variables
	InitMultiDialogs();

	//UI color masking?
	SetupHint();

	//Main internal counter for intervals
	tmtmt = 0;

	REALTIME = 0;
	KeyPressed = false;

	OnMouseMoveRedraw();

	if (PlayMode)
	{
		PlayRandomTrack();
	}

	//Program loop to handle WM_QUIT; everything else handles AllGame()
	while (true)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				PostQuitMessage(msg.wParam);
				return 1;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Check if window has focus
		if (bActive)
		{
			//Load IntExplorer.dll
			StartExplorer();

			//Main game loop (runs until Exit button is clicked)
			AllGame();

			ClearScreen();
			UnLoading();
			CloseExplosions();
			ShutdownMultiplayer(1);

			//Distinguish between last window adn fullscreen resolutions
			int ex_window_x, ex_window_y, ex_x, ex_y;

			//Set last 'global resolution' according to current mode
			if (window_mode)
			{
				ex_window_x = exRealLx;
				ex_window_y = exRealLy;
				ex_x = ex_other_RealLx;
				ex_y = ex_other_RealLy;
			}
			else
			{
				ex_x = exRealLx;
				ex_y = exRealLy;
				ex_window_x = ex_other_RealLx;
				ex_window_y = ex_other_RealLy;
			}

			//Save settings before closing
			GFILE* fff = Gopen("mode.dat", "wt");
			if (fff)
			{
				//7th value was FPSTime
				Gprintf(fff, "%d %d %d %d %d %d %d %d %d %d %d %d",
				        ex_window_x, ex_window_y, ex_x, ex_y,
				        WarSound, OrderSound, OrderSound,
				        MidiSound, 0, ScrollSpeed, exFMode, PlayMode);
				Gclose(fff);
			}
			GFILE* rec_settings_file = Gopen("rec.dat", "wt");
			if (rec_settings_file)
			{
				Gprintf(rec_settings_file, "%d %s", RecordMode, RECFILE);
				Gclose(rec_settings_file);
			}

			FilesExit();
			StopPlayCD();
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			FinExplorer();
		}
	}
	return msg.wParam;
}
