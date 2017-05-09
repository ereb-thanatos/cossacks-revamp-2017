#include "ddini.h"
#include <stdlib.h>
#include "ResFile.h"
#include "Fastdraw.h"
#include "MapDiscr.h"
#include "mouse.h"
#include "mode.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <crtdbg.h>
#include <math.h>

#include "Megapolis.h"

#include "fonts.h"
#include "WeaponID.h"

#include "3DSurf.h"
#include "GSound.h"
#include <assert.h>
#include "3DMapEd.h"
#include "MapSprites.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "ZBuffer.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "StrategyResearch.h"
#include "Curve.h"
#include "NewMon.h"
#include "ActiveScenary.h"
#include "ConstStr.h"
#include "DrawForm.h"
#include "IconTool.h"
#include "Dialogs.h"
#include "Path.h"
#include "Sort.h"
#include "MPlayer.h"
#include "Graphs.h"

#include "PlayerInfo.h"
extern PlayerInfo PINFO[8];

extern const int kMinorMessageDisplayTime;

int LastActionX = 0;
int LastActionY = 0;

extern int UNI_LINEDLY1;
extern int UNI_LINEDY1;
extern int UNI_LINEDLY2;
extern int UNI_LINEDY2;
extern int UNI_HINTDY1;
extern int UNI_HINTDLY1;
extern int UNI_HINTDY2;
extern int UNI_HINTDLY2;

bool NoPFOG = 0;
int Inform = 0;
bool LockPause = 0;
bool MiniActive = 0;
void ShowShipsDest( Brigade* BR );

word GetDir( int, int );
void HandleGeology();
void HandleShar( Nation* NT );
word CurZone;
int MRealX, MRealY;
void TestBlob();
bool NoText = false;
void DrawFog();
void ShowAllBars();
extern bool MEditMode;
extern byte EditMedia;
extern bool BuildWall;
extern int LOADED;
extern bool SHOWSLIDE;
extern bool CINFMOD;
void ShowMines( NewMonster* NM );
void ShowRLCItemDarkN( int x, int y, lpRLCTable lprt, int n, int Ints );
void ShowRLCItemRedN( int x, int y, lpRLCTable lprt, int n, int Ints );
int CheckCreationAbility( byte NI, NewMonster* NM, int* x2i, int* y2i, word* BLD, int NBLD );
void CmdGoToTransport( byte NI, word ID );
bool GetRealCoords( int xs, int ys, int* xr, int* yr );
extern bool KeyPressed;
extern bool Mode3D;
extern bool FullMini;
void TestTriangle();
extern int time1, time2, time3, time4;
int time6, time7, time8;
extern int PortBuiX, PortBuiY;
extern int HintX;
extern int HintY;
extern bool CheapMode;
extern int NMyUnits;
extern int NThemUnits;
extern RLCFont FPassive;
extern RLCFont FActive;
extern RLCFont FDisable;
int GetRLen( char* s, RLCFont* font );
word GetEnemy( int x, int y, byte NI );
int COUNTER;
extern int BlobMode;
extern int LASTRAND, LASTIND;
void ShowRMap();

word GetFog( int x, int y );
extern int FogMode;

extern int RealLx;
extern int RealLy;
extern int Pitch;
extern Forces NForces[8];
extern bool GetCoord;
extern UniqMethood* UNIM;
void LShow();
//Карта неустранимых препятствий
extern int PathAsks;
extern int NMONS;
extern bool AttackMode;
extern int nEused;
extern bool EUsage[1024];
extern Nation* curn;
extern int counter;
extern bool InfoMode;
extern int WaitCycle;
extern int tmtmt;
extern bool EditMapMode;
extern int HeightEditMode;
extern bool HelpMode;
extern bool ChoosePosition;
extern int LastKey;
extern bool PeaceMode;
extern byte LockMode;

const byte CLRT[8] = //IMPORTANT: hardcoded color values
{
	0xD0,//red
	0xD4,//blue
	0xD8,//cyan
	0xDC,//purple
	0xE0,//orange
	0xE4,//black
	0xE8,//white
	0xAD //brown (rogue mercs)
};

void ShowDestn();
void ShowMiniDestn();
extern byte RNTB[64][256];
void CreateMiniMap();
word Nsel;
int TerrHi;

bool BuildMode;
byte SpecCmd;
OneSlide* OSB;
byte blx;
byte bly;
word BuildingID;
Nation* BNat;
extern int NAsk;
extern int curptr;
int	smapx;
int	smapy;

//Some additional offsets
//Proportional to display resolution
//Observed values: 25, 32, 40, 60
int	smaplx, smaply;

//Coordinates of the view window of the player
//Range 1 to 421 for normal maps
//Range 1 to 901 for large (2x) maps
//Range 1 to 1861 for huge (4x) maps
int	mapx, mapy;

int	msx;
int msy;
int minix;
int	miniy;

static int StartX = 0;
static int StartY = 0;

bool MiniMade;
char Prompt[80];
int PromptTime;

byte minimap[maxmap][maxmap];

//Инф. о блокировании поверхности
byte Locking[1024];

#define ScreenSizeX32 ScreenSizeX-32

//Массив все монстров на карте
OneObject* Group[ULIMIT];

//Mассив для локальных команд
//Индексы использования
bool	AsmUsage[MaxAsmCount];
char	AsmBuf[MaxAsmCount*OneAsmSize];
int		LastAsmRequest;
//Массив для команд первого уровня
bool	OrdUsage[MaxOrdCount];
Order1  OrdBuf[MaxOrdCount];
int		LastOrdRequest;
//network sequence errors enumeration
extern int SeqErrorsCount;
extern int LastRandDif;
extern int curdx;
extern int curdy;
//---------------------------------------------G R I D S------------------------------------//
int GridX = 4;
int GridY = 4;
int GridLx = 16;
int GridLy = 24;
int GridNx = 230 / 16;
int GridNy = 230 / 32;
int GridMode = 0;

void SelectNextGridMode()
{
	switch (GridMode)
	{
	case 0:
		GridMode = 1;
		GridLx = 8;
		GridLy = 12;
		break;

	case 1:
		GridMode = 2;
		GridLx = 16;
		GridLy = 24;
		break;

	case 2:
		GridMode = 3;
		GridLx = 24;
		GridLy = 36;
		break;

	case 3:
		GridMode = 4;
		GridLx = 32;
		GridLy = 48;
		break;

	case 4:
		GridMode = 5;
		GridLx = 40;
		GridLy = 60;
		break;

	case 5:
		GridMode = 0;
		break;
	}
}

void DeleteInArea( int x0, int y0, int x1, int y1 );

void DelInGrid( int x, int y )
{
	if (!GridMode)
	{
		return;
	}

	int Gx = ( ( x - GridX * 64 ) >> 6 ) / GridLx;
	int Gy = ( ( y - GridY * 32 ) >> 5 ) / GridLy;

	if (Gx >= 0 && Gx < GridNx&&Gy >= 0 && Gy < GridNy)
	{
		DeleteInArea( ( Gx*GridLx + GridX ) * 64 - 1, ( Gy*GridLy + GridY ) * 32 - 1, ( ( Gx + 1 )*GridLx + GridX ) * 64 + 1, ( ( Gy + 1 )*GridLy + GridY ) * 32 + 1 );
	}
}

void DrawGrids()
{
	int x0 = ( mapx - GridX * 2 ) / ( 2 * GridLx );
	int Nx = 2 + smaplx / ( 2 * GridLx );
	int yMin, yMax, xMin, xMax;
	GridNx = ( msx - GridX * 4 ) / ( GridLx * 2 );
	GridNy = ( msx - GridY * 4 ) / ( GridLy * 2 );

	yMin = ( GridY << 1 ) - mapy;
	if (yMin < 0)yMin = 0;
	yMax = ( ( GridY + GridLy*GridNy ) << 1 ) - mapy;
	if (yMax > smaply)yMax = smaply;
	yMin = smapy + ( yMin << 4 );
	yMax = smapy + ( yMax << 4 );

	xMin = ( GridX << 1 ) - mapx;
	if (xMin < 0)xMin = 0;
	xMax = ( ( GridX + GridLx*GridNx ) << 1 ) - mapx;
	if (xMax > smaplx)xMax = smaplx;
	xMin = smapx + ( xMin << 5 );
	xMax = smapx + ( xMax << 5 );


	for (int i = 0; i < Nx + 1; i++)
	{
		int x = ( GridX + ( i + x0 )*GridLx ) << 1;
		if (x >= mapx&&x <= mapx + smaplx)
		{
			x = smapx + ( ( x - mapx ) << 5 );
			DrawLine( x, yMin, x, yMax, 0x9A );
		}
	}

	int y0 = ( mapy - GridY * 2 ) / ( 2 * GridLy );
	int Ny = 2 + smaply / ( 2 * GridLy );
	for (int i = 0; i < Ny + 1; i++)
	{
		int y = ( GridY + ( i + y0 )*GridLy ) << 1;
		if (y >= mapy&&y <= mapy + smaply)
		{
			y = smapy + ( ( y - mapy ) << 4 );
			DrawLine( xMin, y, xMax, y, 0x9A );
		};
	};
	if (GetKeyState( VK_CONTROL ) & 0x8000)
	{
		int Dx = GridLx * 32;
		for (int i = 0; i < Nx + 2; i++)
		{
			int x1 = ( GridX + ( i + x0 )*GridLx ) << 1;
			int x = smapx + ( ( x1 - mapx ) << 5 );
			if (x1 >= mapx&&x1 <= mapx + smaplx)
			{
				DrawLine( x + 1, yMin, x + 1, yMax, 0x9A );
				DrawLine( x - 1, yMin, x - 1, yMax, 0x9A );
			};
			if (x0 + i)DrawLine( x - Dx, yMin, x - Dx, yMax, 0x9A );
			if (x0 + i)DrawLine( x + Dx, yMin, x + Dx, yMax, 0x9A );
		};
		int y0 = ( mapy - GridY * 2 ) / ( 2 * GridLy );
		int Ny = 2 + smaply / ( 2 * GridLy );
		int Dy = GridLy * 16;
		for (int i = 0; i < Ny + 2; i++)
		{
			int y1 = ( GridY + ( i + y0 )*GridLy ) << 1;
			int y = smapy + ( ( y1 - mapy ) << 4 );
			if (y1 >= mapy&&y1 <= mapy + smaply)
			{
				DrawLine( xMin, y - 1, xMax, y - 1, 0x9A );
				DrawLine( xMin, y + 1, xMax, y + 1, 0x9A );
			};
			if (y0 + i)DrawLine( xMin, y - Dy, xMax, y - Dy, 0x9A );
			if (y0 + i)DrawLine( xMin, y + Dy, xMax, y + Dy, 0x9A );
		};
		x0--;
		y0--;
		if (x0 < 0)x0 = 0;
		if (y0 < 0)y0 = 0;
		int NX = ( RealLx >> 8 ) + 2;
		int NY = ( RealLy >> 7 ) + 2;
		int X0 = ( -( mapx & 7 ) ) << 5;
		int Y0 = ( -( mapy & 7 ) ) << 4;
		for (int IX = 0; IX < NX; IX++)
			for (int IY = 0; IY < NY; IY++)
			{
				int xx = X0 + IX * 256;
				int yy = Y0 + IY * 128;
				DrawLine( xx - 4, yy - 4, xx + 4, yy + 4, 0x9A );
				DrawLine( xx + 4, yy - 4, xx - 4, yy + 4, 0x9A );
				xx += 128;
				yy += 64;
				DrawLine( xx - 4, yy - 4, xx + 4, yy + 4, 0x7A );
				DrawLine( xx + 4, yy - 4, xx - 4, yy + 4, 0x7A );
			};


		for (int ix = 0; ix < Nx + 2; ix++)
			for (int iy = 0; iy < Ny + 2; iy++)
			{
				int x = ( ix + ix + x0 + x0 + 1 )*GridLx + GridX + GridX;
				int y = ( iy + iy + y0 + y0 + 1 )*GridLy + GridY + GridY;
				if (x >= mapx&&x <= mapx + smaplx&&y >= mapy&&y <= mapy + smaply&&x0 + ix < GridNx&&y0 + iy < GridNy)
				{
					int N = ix + x0 + ( iy + y0 )*GridNx;
					char cc[5];
					if (N < 10)sprintf( cc, "00%d", N );
					else if (N < 100)sprintf( cc, "0%d", N );
					else sprintf( cc, "%d", N );
					ShowString( smapx + ( x - mapx ) * 32, smapy + ( y - mapy ) * 16, cc, &BigWhiteFont );
				};
			};
	};
}

//------------------------------------------------------------------------------------------//

void ShowNewMonsters();

//Фон
int ConvY( int y )
{
	return div24( y * 32 );
}

void RedSquare( int x, int y )
{
	if (x < mapx || x >= mapx + smaplx || y < mapy || y >= mapy + smaply)
	{
		return;
	}

	int sco = int( ScreenPtr ) + smapx + ( ( x - mapx ) << 5 ) +
		( ( ( y - mapy ) << 5 ) + smapy )*SCRSizeX;

	int ddx = SCRSizeX + SCRSizeX - 32;
	__asm
	{
		push	edi
		mov		edi, sco
		mov		eax, 16
		uyu:	mov		ecx, 16
				uuu : mov		byte ptr[edi], clrRed
					  add		edi, 2
					  loop	uuu
					  add		edi, ddx
					  dec		al
					  jnz		uyu

					  pop		edi
	}
}

void RedMiniSquare( int x, int y )
{
	if (x < mapx || x >= mapx + smaplx || y < mapy || y >= mapy + smaply)
	{
		return;
	}

	int sco = int( ScreenPtr ) + smapx + ( ( x - mapx ) << 4 ) +
		( ( ( y - mapy ) << 4 ) + smapy )*SCRSizeX;

	int ddx = SCRSizeX + SCRSizeX - 16;
	__asm
	{
		push	edi
		mov		edi, sco
		mov		eax, 8
		uyu:	mov		ecx, 8
				uuu : mov		byte ptr[edi], clrRed
					  add		edi, 2
					  loop	uuu
					  add		edi, ddx
					  dec		al
					  jnz		uyu

					  pop		edi
	}
}

void RedBar( int x, int y, int lx, int ly )
{
	for (int i = 0; i < lx; i++)
		for (int j = 0; j < ly; j++)
			RedSquare( x + i, y + j );
}

void RedMiniBar( int x, int y, int lx, int ly )
{
	for (int i = 0; i < lx; i++)
		for (int j = 0; j < ly; j++)
			RedMiniSquare( x + i, y + j );
}

void WhiteSquare( int x, int y )
{
	if (x < mapx || x >= mapx + smaplx || y < mapy || y >= mapy + smaply)return;
	int sco = int( ScreenPtr ) + smapx + ( ( x - mapx ) << 5 ) +
		( ( ( y - mapy ) << 5 ) + smapy )*SCRSizeX;
	int ddx = SCRSizeX + SCRSizeX - 32;
	__asm {
		push	edi
		mov		edi, sco
		mov		eax, 16
		uyu:	mov		ecx, 16
				uuu : mov		byte ptr[edi], 255
					  add		edi, 2
					  loop	uuu
					  add		edi, ddx
					  dec		al
					  jnz		uyu

					  pop		edi
	}
}

void WhiteMiniSquare( int x, int y )
{
	if (x < mapx || x >= mapx + smaplx || y < mapy || y >= mapy + smaply)return;
	int sco = int( ScreenPtr ) + smapx + ( ( x - mapx ) << 4 ) +
		( ( ( y - mapy ) << 4 ) + smapy )*SCRSizeX;
	int ddx = SCRSizeX + SCRSizeX - 16;
	__asm {
		push	edi
		mov		edi, sco
		mov		eax, 8
		uyu:	mov		ecx, 8
				uuu : mov		byte ptr[edi], 255
					  add		edi, 2
					  loop	uuu
					  add		edi, ddx
					  dec		al
					  jnz		uyu

					  pop		edi
	};
};
void WhiteBar( int x, int y, int lx, int ly )
{
	for (int i = 0; i < lx; i++)
		for (int j = 0; j < ly; j++)
			WhiteSquare( x + i, y + j );
};
void WhiteMiniBar( int x, int y, int lx, int ly )
{
	for (int i = 0; i < lx; i++)
		for (int j = 0; j < ly; j++)
			WhiteMiniSquare( x + i, y + j );
};
extern int CurPalette;
void OutErr( LPCSTR s )
{
	if (CurPalette == 2)LoadPalette( "2\\agew_1.pal" );
	else LoadPalette( "0\\agew_1.pal" );
	MessageBox( hwnd, s, "Loading failed...", MB_ICONWARNING | MB_OK );
};

void CreateTimedHint( char* s, int time );
//Получить блок для InLineCom
char* GetAsmBlock()
{
	if (LastAsmRequest >= MaxAsmCount)LastAsmRequest = 0;
	int i;
	for (i = LastAsmRequest; i < MaxAsmCount&&AsmUsage[i]; i++);
	if (i < MaxAsmCount)
	{
		LastAsmRequest = i + 1;
		AsmUsage[i] = true;
		return &AsmBuf[i*OneAsmSize];
	}
	else
	{
		for (int i = 0; i < LastAsmRequest&&AsmUsage[i]; i++);
		if (i < LastAsmRequest)
		{
			LastAsmRequest = i + 1;
			AsmUsage[i] = true;
			return &AsmBuf[i*OneAsmSize];
		}
		else return nullptr;
	};
};
void FreeAsmBlock( char* p )
{
	int i = ( int( p ) - int( AsmBuf ) ) >> OneAShift;
	AsmUsage[i] = false;
	LastAsmRequest = i;
};
void InitAsmBuf()
{
	memset( AsmUsage, 0, sizeof AsmUsage );
	memset( AsmBuf, 0, sizeof AsmBuf );
	LastAsmRequest = 0;
};
//Получить блок для LocalOrder
int CHSM1;

Order1* GetOrdBlock()
{
	Order1* OR1 = new Order1;
	memset( OR1, 0, sizeof Order1 );
	return OR1;
}

void OneObject::FreeOrdBlock( Order1* p )
{
	free( p );
	return;
}

void InitOrdBuf()
{
	memset( OrdUsage, 0, sizeof OrdUsage );
	memset( OrdBuf, 0, sizeof OrdBuf );
	//LastOrdRequest=0;
};

void LoadLock()
{
	ResFile f = RReset( "lock.dat" );
	if (f == INVALID_HANDLE_VALUE)OutErr( "Can't load locking." );
	RBlockRead( f, Locking, RFileSize( f ) );
	RClose( f );
};
void LoadFon()
{

};
void ShowFon()
{
	return;

};
void ShowFon1()
{

};
void ShowBorder()
{
	return;

};
int mul3( int x )
{
	return x + x;//x+x+x;
};
void LoadTiles()
{

};
void SaveWalls( ResFile ff1 );
//void LoadWalls(ResFile ff1);
char MapName[128];
extern short HiMap[256][256];//Height of surface
byte mapPos[16];
int RES[8][8];

void PostLoadExtendedMap( char* s )
{
	memset( fmap, 0, LX_fmap*LX_fmap * 2 );
	ResFile f1 = RReset( s );
	int pos = 12 + 16 + 128 + 8 * 8 * 4 + ( msx*msy ) + ( msx + 1 )*( msy + 1 ) * 2;
	RSeek( f1, pos );
	//Load units
	int NU;
	RBlockRead( f1, &NU, 4 );
	pos += ( NU + 1 ) << 2;
	RSeek( f1, pos );
	RBlockRead( f1, &NU, 4 );
	byte NNUM, xu, yu;
	word NIndex;

	for (int i = 0; i < NU; i++)
	{
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &NNUM, 1 );
		RBlockRead( f1, &xu, 1 );
		RBlockRead( f1, &yu, 1 );
		Nation* NT = &NATIONS[NNUM];
	}

	RClose( f1 );
	memset( fmap, 0, LX_fmap*LX_fmap * 2 );
}

void ClearMaps();

void ClearMaps();
extern int MAXSPR;
void InitRiv();
extern MotionField UnitsField;

void NewMap( int szX, int szY )//Standard map size is 480x480
{
	PromptTime = 0;
	MAXOBJECT = 0;
	MAXSPR = 0;

	memset( Group, 0, sizeof Group );
	memset( NLocks, 0, sizeof NLocks );

	ClearMaps();

	BuildMode = false;

	memset( &HiMap[0][0], 0, sizeof HiMap );

	MFIELDS[0].ClearMaps();
	MFIELDS[1].ClearMaps();
	UnitsField.ClearMaps();

	MiniMade = false;
	Nsel = 0;

	InitRenderMap();

	msx = szX;
	msy = szY;
	InitRiv();
}

void CreateMiniMap();

const int drx[8] = { 0,1,1,1,0,-1,-1,-1 };
const int dry[8] = { -1,-1,0,1,1,1,0,-1 };

extern bool HealthMode;

void GetRect( OneObject* ZZ, int* x, int* y, int* Lx, int* Ly )
{
	int scx, scy;
	if (ZZ->newMons)
	{
		NewMonster* NMN = ZZ->newMons;
		scx = ( ZZ->RealX >> 4 ) - ( NMN->RectLx >> 1 ) + NMN->RectDx;
		scy = ( mul3( ZZ->RealY ) >> 6 ) - ( NMN->RectLy ) + NMN->RectDy;
		if (Mode3D)scy -= ZZ->RZ;
		*x = scx;
		*y = scy;
		*Lx = NMN->RectLx;
		*Ly = NMN->RectLy;
		return;
	};
};
void DrawMarker( OneObject* OB )
{
	if (OB && !OB->Wall)
	{
		int x, y, Lx, Ly;
		GetRect( OB, &x, &y, &Lx, &Ly );
		x += smapx - ( int( mapx ) << 5 );
		y += smapy - ( mul3( int( mapy ) ) << 3 );
		byte cl = clrYello;
		if (OB->ForceX || OB->ForceY)cl = clrRed;
		byte c1 = 0;
		byte c2 = 2;
		byte dc = 4;
		int ddy = -12;
		Hline( x + c1, y, x + c2, cl );
		Hline( x + Lx - 1 - c2, y, x + Lx - 1 - c1, cl );
		Hline( x + c1, y + Ly - 1, x + c2, cl );
		Hline( x + Lx - 1 - c2, y + Ly - 1, x + Lx - 1 - c1, cl );
		Vline( x, y + c1, y + c2, cl );
		Vline( x + Lx - 1, y + c1, y + c2, cl );
		Vline( x, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
		Vline( x + Lx - 1, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
		/*
		if(HealthMode){
			int mhs=Lx-c2-c2-dc-dc;
			int hs=mhs;
			if(OB->MaxLife>0){
				hs=div(mhs*OB->Life,OB->MaxLife).quot;
			};
			Vline(-2+x+c2+dc,y+ddy,y+ddy+2,255);
			Vline(-2+x+Lx-1-c2,y+ddy,y+ddy+2,255);
			Vline(-2+x+c2+dc+1,y+ddy,y+ddy,255);
			Vline(-2+x+c2+dc+1,y+ddy+2,y+ddy+2,255);
			Vline(-2+x+Lx-1-c2-1,y+ddy,y+ddy,255);
			Vline(-2+x+Lx-1-c2-1,y+ddy+2,y+ddy+2,255);
			Hline(-2+x+c2+dc+1,y+ddy+1,x+c2+hs+dc+1,clrGreen);
		};
		*/
		/*
		Hline(x+1,y+1,x+1,cl);
		Hline(x+1,y+Lx-2,x+1,cl);
		Hline(x+Lx-2,y+1,x+Lx-2,cl);
		Hline(x+Lx-2,y+Lx-2,x+Lx-2,cl);*/
	};
};
void DrawImMarker( OneObject* OB )
{
	if (OB)
	{
		int x, y, Lx, Ly;
		GetRect( OB, &x, &y, &Lx, &Ly );
		x--;
		y--;
		Lx += 2;
		Ly += 2;
		x += smapx - ( int( mapx ) << 5 );
		y += smapy - ( mul3( int( mapy ) ) << 3 );
		byte cl = 0xCD;
		//if(OB->ForceX||OB->ForceY)cl=0xCD;
		byte c1 = 0;
		byte c2 = 2;
		byte dc = 4;
		int ddy = -12;
		Hline( x + c1, y, x + c2, cl );
		Hline( x + Lx - 1 - c2, y, x + Lx - 1 - c1, cl );
		Hline( x + c1, y + Ly - 1, x + c2, cl );
		Hline( x + Lx - 1 - c2, y + Ly - 1, x + Lx - 1 - c1, cl );
		Vline( x, y + c1, y + c2, cl );
		Vline( x + Lx - 1, y + c1, y + c2, cl );
		Vline( x, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
		Vline( x + Lx - 1, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
	};
};
void DrawHealth( OneObject* OB )
{
	if (OB->Sdoxlo)return;
	int x, y, Lx, Ly;
	GetRect( OB, &x, &y, &Lx, &Ly );
	x += smapx - ( int( mapx ) << 5 );
	y += smapy - ( mul3( int( mapy ) ) << 3 );
	byte c1 = 0;
	byte c2 = 2;
	byte dc = 4;
	int ddy = -12;
	int mhs = Lx - c2 - c2 - dc - dc;
	int hs = mhs;
	if (OB->MaxLife > 0)
	{
		hs = div( mhs*OB->Life, OB->MaxLife ).quot;
	};
	Hline( -2 + x + c2 + dc + 1, y + ddy + 1, x + c2 + hs + dc + 1, clrGreen );
	Hline( -2 + x + c2 + dc + 1, y + ddy, x + c2 + hs + dc + 1, clrGreen );
	if (OB->newMons->ShowDelay)
	{
		int mxd = 1;
		if (OB->MaxDelay)mxd = OB->MaxDelay;
		else mxd = 1;
		if (mxd < OB->delay)mxd = OB->delay;
		hs = ( mhs*( mxd - OB->delay ) ) / mxd;
		Hline( -2 + x + c2 + dc + 1, y + ddy + 2, x + c2 + hs + dc + 1, 0xBF );
		Hline( -2 + x + c2 + dc + 1, y + ddy + 3, x + c2 + hs + dc + 1, 0xBF );
		Vline( -2 + x + c2 + dc, y + ddy, y + ddy + 3, 255 );
		Vline( -2 + x + Lx - 1 - c2, y + ddy, y + ddy + 3, 255 );
	}
	else
	{
		Vline( -2 + x + c2 + dc, y + ddy, y + ddy + 1, 255 );
		Vline( -2 + x + Lx - 1 - c2, y + ddy, y + ddy + 1, 255 );
	};
};
void DrawColorMarker( OneObject* OB, byte cl )
{
	if (OB)
	{
		int x, y, Lx, Ly;
		GetRect( OB, &x, &y, &Lx, &Ly );
		x += smapx - ( int( mapx ) << 5 );
		y += smapy - ( mul3( int( mapy ) ) << 3 );
		if (OB->ForceX || OB->ForceY)cl = clrRed;
		byte c1 = 0;
		byte c2 = 5;
		byte dc = 4;
		int ddy = -12;
		Hline( x + c1, y, x + c2, cl );
		Hline( x + Lx - 1 - c2, y, x + Lx - 1 - c1, cl );
		Hline( x + c1, y + Ly - 1, x + c2, cl );
		Hline( x + Lx - 1 - c2, y + Ly - 1, x + Lx - 1 - c1, cl );
		Vline( x, y + c1, y + c2, cl );
		Vline( x + Lx - 1, y + c1, y + c2, cl );
		Vline( x, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
		Vline( x + Lx - 1, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
		/*
		Hline(x+1,y+1,x+1,cl);
		Hline(x+1,y+Lx-2,x+1,cl);
		Hline(x+Lx-2,y+1,x+Lx-2,cl);
		Hline(x+Lx-2,y+Lx-2,x+Lx-2,cl);*/
	};
};
void DrawMiniMarker( OneObject* OB )
{
	if (OB)
	{
		int x, y, Lx, Ly;
		GetRect( OB, &x, &y, &Lx, &Ly );
		x = x >> 1;
		y = y >> 1;
		Lx = Lx >> 1;
		Ly = Ly >> 1;
		x += smapx - ( int( mapx ) << 4 );
		y += smapy - ( mul3( int( mapy ) ) << 2 );
		byte cl = clrYello;
		byte c1 = 0;
		byte c2 = 2;
		byte dc = 4;
		int ddy = -12;
		Hline( x + c1, y, x + c2, cl );
		Hline( x + Lx - 1 - c2, y, x + Lx - 1 - c1, cl );
		Hline( x + c1, y + Ly - 1, x + c2, cl );
		Hline( x + Lx - 1 - c2, y + Ly - 1, x + Lx - 1 - c1, cl );
		Vline( x, y + c1, y + c2, cl );
		Vline( x + Lx - 1, y + c1, y + c2, cl );
		Vline( x, y + Lx - 1 - c2, y + Ly - 1 - c1, cl );
		Vline( x + Lx - 1, y + Ly - 1 - c2, y + Ly - 1 - c1, cl );
		if (HealthMode)
		{
			int mhs = Lx - c2 - c2 - dc - dc;
			int hs = mhs;
			if (OB->MaxLife > 0)
			{
				hs = div( mhs*OB->Life, OB->MaxLife ).quot;
			};
			Vline( -2 + x + c2 + dc, y + ddy, y + ddy + 2, 255 );
			Vline( -2 + x + Lx - 1 - c2, y + ddy, y + ddy + 2, 255 );
			Vline( -2 + x + c2 + dc + 1, y + ddy, y + ddy, 255 );
			Vline( -2 + x + c2 + dc + 1, y + ddy + 2, y + ddy + 2, 255 );
			Vline( -2 + x + Lx - 1 - c2 - 1, y + ddy, y + ddy, 255 );
			Vline( -2 + x + Lx - 1 - c2 - 1, y + ddy + 2, y + ddy + 2, 255 );
			Hline( -2 + x + c2 + dc + 1, y + ddy + 1, x + c2 + hs + dc + 1, clrGreen );
		};
		/*
		Hline(x+1,y+1,x+1,cl);
		Hline(x+1,y+Lx-2,x+1,cl);
		Hline(x+Lx-2,y+1,x+Lx-2,cl);
		Hline(x+Lx-2,y+Lx-2,x+Lx-2,cl);*/
	};
};
void ShowHealth( OneObject* ZZ )
{
	if (!( ZZ->Selected&GM( MyNation ) ))return;
	DrawMarker( ZZ );
	return;
};
void ShowMiniHealth( OneObject* ZZ )
{
	if (!( ZZ->Selected&GM( MyNation ) ))return;
	DrawMiniMarker( ZZ );
	return;
};

static int tmt = 0;
char* GetTextByID( char* ID );
void CBar( int x, int y, int Lx, int Ly, byte c );

void GetDiscreta( int* x, int* y, int val );
char* WINNER = nullptr;
char* LOOSER = nullptr;
int PrevMissTime = 0;
int FirstMissTime = 0;
extern bool GameExit;
void GetSquare();
char* GetPName( int i );
int NatSquare[8] = { 0,0,0,0,0,0,0,0 };
int SumSquare[8];
int SumAccount[8];
extern City CITY[8];
extern word NPlayers;
void CreateSumm()
{
	memset( SumSquare, 0, 7 * 4 );
	memset( SumAccount, 0, 7 * 4 );
	for (int i = 0; i < 7; i++)
	{
		byte ms0 = 1 << i;
		byte ms = NATIONS[i].NMask;
		if (ms == ms0)
		{
			SumSquare[i] = NatSquare[i];
			SumAccount[i] = CITY[i].Account;
		}
		else
		{
			int ss = 0;
			int sa = 0;
			int na = 0;
			for (int j = 0; j < 7; j++)
			{
				if (NATIONS[j].NMask&ms)
				{
					ss += NatSquare[j];
					sa += CITY[j].Account;
					na++;
				};
			};
			SumSquare[i] = ss / na;
			SumAccount[i] = sa / na;

		};
	};
};

int GetBestPlayer()
{
	CreateSumm();
	if (PINFO[0].VictCond == 1)
	{
		int maxn = -1;
		int maxs = 0;
		int maxidv = 0;
		for (int i = 0; i < 7; i++)
		{
			int v = SumSquare[i];
			if (v > maxs)
			{
				maxs = v;
				maxn = i;
				maxidv = NatSquare[i];
			};
			if (v == maxs&&NatSquare[i] > maxidv)
			{
				maxidv = NatSquare[i];
				maxn = i;
			};
		};
		return maxn;
	}
	else
		if (PINFO[0].VictCond == 2)
		{
			int maxn = -1;
			int maxs = 0;
			int maxidv = 0;

			for (int i = 0; i < 7; i++)
			{
				int v = SumAccount[i];//*10000+CITY[i].Account;
				if (v > maxs)
				{
					maxs = v;
					maxn = i;
					maxidv = CITY[i].Account;
				};
				if (v == maxs&&CITY[i].Account > maxidv)
				{
					maxidv = CITY[i].Account;
					maxn = i;
				};
			};
			return maxn;
		}
		else return -1;
};
int SortPlayers( byte* Res, int* par )
{
	CreateSumm();
	int con = PINFO[0].VictCond;
	SortClass SC;
	SC.CheckSize( 8 );
	int np = 0;
	byte usemask = 0;
	for (int q = 0; q < 7; q++)
	{
		int xx = NatRefTBL[q];
		if (NATIONS[xx].ThereWasUnit/*NtNUnits[xx]*/)
		{
			byte ms = NATIONS[xx].NMask;
			if (!( usemask&ms ))
			{
				usemask |= ms;
				if (con == 1)SC.Parms[np] = -SumSquare[xx];
				else SC.Parms[np] = -SumAccount[xx];
				SC.Uids[np] = ms;
				np++;
			};
		};
	};
	SC.NUids = np;
	SC.Sort();
	word msks[8];
	SC.Copy( msks );
	int ps = 0;
	for (int q = 0; q < np; q++)
	{
		byte msk = byte( msks[q] );
		int mp = 0;
		byte m0 = 1;
		for (int p = 0; p < 7; p++)
		{
			int xx = NatRefTBL[p];
			m0 = 1 << xx;
			if (msk&m0&&NATIONS[xx].ThereWasUnit/*NtNUnits[xx]*/)
			{
				if (con == 1)SC.Parms[mp] = -NatSquare[xx];
				else SC.Parms[mp] = -CITY[xx].Account;
				SC.Uids[mp] = p;
				mp++;
			};
			m0 <<= 1;
		};
		if (mp)
		{
			SC.NUids = mp;
			SC.Sort();
			for (int p = 0; p < mp; p++)
			{
				Res[ps] = SC.Uids[p];
				par[ps] = SC.Parms[p];
				ps++;
			};
		};
	};

	return ps;
}

void ShowVictInFormat( char* Res, int idx, byte Type )
{
	switch (Type)
	{
	case 1:
		sprintf( Res, "%d", NatSquare[idx] );
		break;
	default:
		sprintf( Res, "%.2f", float( CITY[idx].Account ) / 100 );
	}
}

bool NoWinner = 0;
bool ShowStat = 0;
extern bool NOPAUSE;
void CmdEndGame( byte NI, byte state, byte cause );
unsigned long GetRealTime();
extern int WaitState;
extern bool EnterChatMode;
extern int ShowGameScreen;
extern byte PlayGameMode;

void ShowCentralText0( char* sss )
{
	char ssp[400];
	int ny = 0;
	int L = strlen( sss );
	for (int i = 0; i < L; i++)
	{
		if (sss[i] == '\\')
		{
			ny++;
		}
	}
	int MaxLx = 0;
	byte c;
	int MaxCur = 0;
	int p = 0;
	do
	{
		c = sss[p];
		p++;
		if (c != '\\'&&c)
		{
			int L = 1;
			MaxCur += GetRLCWidthUNICODE( BigYellowFont.RLC, (byte*) ( sss + p ), &L );
			p += L - 1;
		}
		else
		{
			if (MaxCur > MaxLx)
			{
				MaxLx = MaxCur;
			}
			MaxCur = 0;
		}
	} while (c);

	int y0 = smapy + ( smaply << ( Shifter - 2 ) );
	int x0 = smapx + ( smaplx << ( Shifter - 1 ) );

	int Dy = GetRLCHeight( BigYellowFont.RLC, 'y' ) + 5;

	y0 -= ( ( Dy*ny ) >> 1 ) + 32;
	MaxLx >>= 1;
	DrawStdBar1( x0 - MaxLx - 64, y0 - 8, x0 + MaxLx + 64, y0 + 20 + Dy*ny + 7 );
	p = 0;
	int p0 = 0;
	do
	{
		c = sss[p];
		p++;
		if (c != '\\'&&c)
		{
			ssp[p0] = c;
			ssp[p0 + 1] = 0;
			p0++;
		}
		else
		{
			int L = GetRLCStrWidth( ssp, &BigYellowFont ) / 2;
			ShowString( x0 - L, y0 - 4, ssp, &BigWhiteFont );
			y0 += Dy;
			p0 = 0;
		}
	} while (c);
}

void ShowWinner()
{
	if (NoWinner || PlayGameMode == 1)
	{
		return;
	}

	GetSquare();

	if (!WINNER)
	{
		WINNER = GetTextByID( "WINNER" );
	}

	if (!LOOSER)
	{
		LOOSER = GetTextByID( "LOOSER" );
	}

	char sss[512] = "";
	int MyVirtNation = NatRefTBL[MyNation];
	if (( !PlayGameMode ) && PINFO[0].VictCond && !PINFO[0].GameTime)
	{
		if (PINFO[0].VictCond)
		{
			//Land scope
			int n = GetBestPlayer();
			if (( ( 1 << n )&NATIONS[MyVirtNation].NMask ) && NMyUnits)
			{
				strcpy( sss, WINNER );
				CmdEndGame( MyNation, 2, 111 );
				LockPause = 1;
				if (GetRealTime() - PrevMissTime > 7000)
				{
					PrevMissTime = GetRealTime();
					FirstMissTime = GetRealTime();
				}
				else
				{
					if (WaitState == 2)
					{
						GameExit = true;
						ShowStat = 1;
						PrevMissTime = 0;
						SCENINF.Victory = false;
					};
					PrevMissTime = GetRealTime();
				};
			}
			else
			{
				strcpy( sss, LOOSER );
				LockPause = 1;
				CmdEndGame( MyNation, 1, 112 );
				if (GetRealTime() - PrevMissTime > 7000)
				{
					PrevMissTime = GetRealTime();
					FirstMissTime = GetRealTime();
				}
				else
				{
					PrevMissTime = GetRealTime();
					if (WaitState == 2)
					{
						GameExit = true;
						ShowStat = 1;
						SCENINF.LooseGame = false;
						PrevMissTime = 0;
					}
				}
			}
		}
	}

	if (SCENINF.hLib && !SCENINF.StandartVictory)
	{
		if (SCENINF.LooseGame)
		{
			if (!WaitState)
			{
				WaitState = 1;
			}
			if (SCENINF.LooseText)
			{
				strcpy( sss, SCENINF.LooseText );
			}
			else
			{
				strcpy( sss, LOOSER );
			}
			if (GetRealTime() - PrevMissTime > 7000)
			{
				PrevMissTime = GetRealTime();
				FirstMissTime = GetRealTime();
			}
			else
			{
				PrevMissTime = GetRealTime();
				if (GetRealTime() - FirstMissTime > 7000 || WaitState == 2)
				{
					GameExit = true;
					ShowStat = 0;
					SCENINF.LooseGame = false;
					PrevMissTime = 0;
				}
			}
		}
		else
			if (SCENINF.Victory)
			{
				if (!WaitState)
				{
					WaitState = 1;
				}
				if (SCENINF.VictoryText)
				{
					strcpy( sss, SCENINF.VictoryText );
				}
				else
				{
					strcpy( sss, WINNER );
				}
				if (GetRealTime() - PrevMissTime > 7000)
				{
					PrevMissTime = GetRealTime();
					FirstMissTime = GetRealTime();
				}
				else
				{
					if (GetRealTime() - FirstMissTime > 7000 || WaitState == 2)
					{
						GameExit = true;
						ShowStat = 0;
						PrevMissTime = 0;
						SCENINF.Victory = false;
					}
					PrevMissTime = GetRealTime();
					if (CurrentCampagin != -1 && CurrentMission != -1)
					{
						int idx = CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[CurrentMission];
						int id = ( idx >> 8 ) & 255;
						if (id < CAMPAGINS.SCamp[CurrentCampagin].NMiss)CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[id] |= 1;
						id = ( idx >> 16 ) & 255;
						if (id < CAMPAGINS.SCamp[CurrentCampagin].NMiss)CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[id] |= 1;
						id = ( idx >> 24 ) & 255;
						if (id < CAMPAGINS.SCamp[CurrentCampagin].NMiss)CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[id] |= 1;
						SavePlayerData();
					};
				};
			}
			else
			{
				if (SCENINF.CTextTime > 0 && SCENINF.CentralText)
				{
					strcpy( sss, SCENINF.CentralText );
				}
			}
	}
	else
	{
		if (NATIONS[MyNation].VictState && !sss[0])
		{
			if (NATIONS[MyNation].VictState == 1)
			{
				strcpy( sss, LOOSER );
				LockPause = 1;
			}
			else if (NATIONS[MyNation].VictState == 3)
			{
				strcpy( sss, DISCONNECTED );
				LockPause = 1;
			}
			else
			{
				strcpy( sss, WINNER );
				if (SCENINF.hLib&&CurrentCampagin != -1 && CurrentMission != -1)
				{
					int idx = CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[CurrentMission];
					int id = ( idx >> 8 ) & 255;
					if (id < CAMPAGINS.SCamp[CurrentCampagin].NMiss)CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[id] |= 1;
					id = ( idx >> 16 ) & 255;
					if (id < CAMPAGINS.SCamp[CurrentCampagin].NMiss)CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[id] |= 1;
					id = ( idx >> 24 ) & 255;
					if (id < CAMPAGINS.SCamp[CurrentCampagin].NMiss)CAMPAGINS.SCamp[CurrentCampagin].OpenIndex[id] |= 1;
					SavePlayerData();
				};
			};
			if (WaitState == 2)
			{
				GameExit = true;
				ShowStat = SCENINF.hLib == nullptr;
				PrevMissTime = 0;
				SCENINF.Victory = false;
				SCENINF.LooseGame = false;
			};
		}
		else
		{
			if (!NMyUnits)
			{
				LockPause = 1;
				CmdEndGame( MyNation, 1, 113 );
				strcpy( sss, LOOSER );
				if (GetRealTime() - PrevMissTime > 7000)
				{
					PrevMissTime = GetRealTime();
					FirstMissTime = GetRealTime();
				}
				else
				{
					PrevMissTime = GetRealTime();
					if (WaitState == 2)
					{
						GameExit = true;
						ShowStat = 1;
						SCENINF.LooseGame = false;
						PrevMissTime = 0;
					};
				};
			}
			else
			{
				if (!NThemUnits)
				{
					LockPause = 1;
					CmdEndGame( MyNation, 2, 114 );
					strcpy( sss, WINNER );
					if (GetRealTime() - PrevMissTime > 7000)
					{
						PrevMissTime = GetRealTime();
						FirstMissTime = GetRealTime();
					}
					else
					{
						if (WaitState == 2)
						{
							GameExit = true;
							ShowStat = 1;
							PrevMissTime = 0;
							SCENINF.Victory = false;
						};
						PrevMissTime = GetRealTime();
					}
				}
			}
		}
	}

	if (SCENINF.CTextTime&&SCENINF.CentralText&&SCENINF.CentralText[0] == '#')
	{
		strcpy( sss, SCENINF.CentralText + 1 );
		if (SCENINF.CTextTime)
		{
			SCENINF.CTextTime--;
		}
	}

	if (( !sss[0] ) && SCENINF.CTextTime&&SCENINF.CentralText)
	{
		strcpy( sss, SCENINF.CentralText );
	}

	if (ShowGameScreen == 2)
	{
		strcpy( sss, SER_SAVE );
	}

	if (sss[0] && !EnterChatMode)
	{
		ShowCentralText0( sss );
	}
}

//final procedures for graphics output
void ProcessWaveFrames();
void ShowVisualLess( int yend );
extern word rpos;
int time0;
word PreRpos;
void ShowPulse();
void AddPulse( word ObjID, byte c );
void ProcessFog1();
void CreateFogImage();
extern int DoLink_Time, SearchVictim_Time, CheckCapture_Time;
void SyncroDoctor();
void PrepareSound();
void ProcessSelectedTower();
extern City CITY[8];
void DrawBorder( Brigade* BR );
int PreviewBrig = -1;;
void GenShow();
void DrawAZones();
void ScenaryLights();
extern int NInGold[8];
extern int NInIron[8];
extern int NInCoal[8];
void ShowCostPlaces();
int CheckSmartCreationAbility( byte NI, NewMonster* NM, int* x2i, int* y2i );
bool Tutorial = 0;
bool NoPress = 0;
bool TutOver = 0;
int SStartX = 0;
int SStartY = 0;
extern int LastTimeStage;
extern int RealPause;
extern int CurrentStartTime[8];
extern int StartTime[8];
extern int LastStartTime[8];
extern int LastGameLength[8];
extern int CurrentStepTime;
void ShowChat();
void xLine( int x, int y, int x1, int y1, byte c );
void ShowExpl();
extern int BrigPnX;
extern int BrigPnY;
void ShowAbout();
void DrawShar( Nation* NT );
extern PingSumm PSUMM;
int GetMaxRealPing();
int GetAveragePing();
void CreateDiffStr( char* str );
extern int AddTime;
extern int NeedAddTime;
extern int NSTT;
int PRVGT = 0;
int GetPing( DWORD pid );
extern DWORD RealTime;
extern bool LockFog;
extern byte SYNBAD[8];
void ProcessRivEditor();
extern int NPROCM;
extern int TPROCM;
void DrawSMS();
void ShowLockBars();
void ProcessVotingView();
void ProcessVotingKeys();
extern int PeaceTimeLeft;
void CmdEndPT();
int GameTime = 0;
char* TMMS_NOPT = nullptr;
char* TMMS_PT = nullptr;
extern int CURTMTMT;
extern byte BalloonState;
extern byte CannonState;
extern byte XVIIIState;
extern int COUNTER1;
extern int COUNTER2;
void DottedLine( int x1, int y1, int x2, int y2, byte c )
{
	if (y1 == y2)
	{
		int n = ( x2 - x1 ) >> 1;
		int xx = x1;
		for (int i = 0; xx < x2; i++)
		{
			//if(i&1);
			int xx2 = xx + 4;
			if (xx2 > x2)xx2 = x2;
			if (!( i & 1 ))xLine( xx, y1, xx2, y2, c );
			xx += 4;
		}
	}
	else
	{
		xLine( x1, y1, x2, y2, c );
	}
}
extern bool RecordMode;
bool IsGameActive();
bool CheckGameTime();

void AddRecordAndRatingSymbols( char* s )
{
	char ccc[256];
	ccc[0] = 0;

	if (IsGameActive())
	{
		if (CheckGameTime())
		{
			strcpy( ccc, "0019" );
		}
		else
		{
			strcpy( ccc, "[ 0019-?]" );
		}
	}

	if ( RecordMode )
	{
		strcat( ccc, "001A" );
	}

	strcat( ccc, s );
	strcpy( s, ccc );
}

extern bool CanProduce;

void ProcessHints();

//Draw a lot of stuff on screen
void GFieldShow()
{
	PrepareSound();

	int uuu = ( tmt & 31 );
	char ggg[600];

	tmt++;

	time1 = GetRealTime();
	time0 = GetRealTime();

	if (SHOWSLIDE)
	{
		SetRLCWindow( smapx, smapy, smaplx << 5, mul3( smaply ) << 3, SCRSizeX );
		TestTriangle();
		TestBlob();
		time6 = GetRealTime() - time0;
		time0 = GetRealTime();
		ShowNewMonsters();
		time0 = GetRealTime();
		WallHandleDraw();
	}

	ShowExpl();

	for (int v = 0; v < 8; v++)
		DrawShar( NATIONS + v );

	if (SHOWSLIDE)
	{
		for (int i = 0; i < MBR4; i++)
		{
			DWORD p = BMASK[i];
			if (p)
			{
				for (int j = 0; j < 32; j++)
				{
					if (p & 1)
					{
						Brigade* BR = CITY[NatRefTBL[MyNation]].Brigs + ( i << 5 ) + j;
						if (BR->Enabled)
							DrawBorder( BR );
					}
					p >>= 1;
				}
			}
		}
		ShowZBuffer();
		ShowPulse();

		if (CINFMOD)
			ShowAllBars();

		if (!NoText)
			LShow();
	}

	if (!SHOWSLIDE)
		return;

	if (EditMapMode)
	{
		DrawAZones();
		if (!NoText)
		{
			for (int i = 0; i < 8; i++)
			{
				int x = mapPos[i << 1];
				int y = mapPos[( i << 1 ) + 1];
				if (x >= mapx&&y >= mapy&&x < mapx + smaplx&&y < mapy + smaply)
				{
					sprintf( ggg, "%d", i + 1 );
					ShowString( smapx + ( ( x - mapx ) << 5 ) + 8, smapy + ( ( y - mapy ) << 5 ) + 2, ggg, &BigWhiteFont );
				}
			}
		}
	}

	int v1 = 3;
	int v2 = 158;
	if (EditMapMode && !NoText)
	{
		if (ChoosePosition)
		{
			sprintf( ggg, GETSPOS, MyNation + 1 );//"Игрок %d : Выбор стартовой позиции."
		}
		else
		{
			sprintf( ggg, PLAYMES, MyNation + 1 );//"Игрок %d"
		}
		ShowString( smapx + 256, smapy + 64, ggg, &WhiteFont );
	}

	if (BuildMode && !NoText)
	{
		GeneralObject* GO = BNat->Mon[BuildingID];
		NewMonster* NM = GO->newMons;
		int xxx, yyy;
		if (GetRealCoords( mouseX, mouseY, &xxx, &yyy ))
		{
			if (NM->SpriteObject && NM->SpriteVisual != 0xFFFF)
			{
				SprGroup* SG = &COMPLEX;
				int id = NM->SpriteVisual;
				ObjCharacter* OC = &SG->ObjChar[id];
				if (OC->Stand)
				{
					int tm1 = div( tmtmt, OC->Delay ).quot;
					int fr = div( tm1, OC->Frames ).rem;
					int spr = fr*OC->Parts;
					int XX;
					int YY;
					NewAnimation* NA = OC->Stand;
					GetDiscreta( &xxx, &yyy, OC->Discret );
					XX = xxx - ( mapx << 5 ) - SG->Dx[id];
					YY = ( yyy / 2 ) - ( mapy * 16 ) - GetHeight( xxx, yyy ) - SG->Dy[id];

					if (CheckDist( xxx, yyy, SG->Radius[id] ))
					{
						if (NA->LineInfo)
						{
							for (int p = 0; p < NA->NFrames; p++)
							{
								NewFrame* OF = &NA->Frames[p];
								GPS.ShowGP( XX, YY, OF->FileID, OF->SpriteID, 0 );
							}
						}
						else
						{
							for (int p = 0; p < OC->Parts; p++)
							{
								NewFrame* OF = &NA->Frames[spr + p];
								GPS.ShowGP( XX, YY, OF->FileID, OF->SpriteID, 0 );
							}
						}
					}
					else
					{
						if (NA->LineInfo)
						{
							for (int p = 0; p < NA->NFrames; p++)
							{
								NewFrame* OF = &NA->Frames[p];
								GPS.ShowGPRedN( XX, YY, OF->FileID, OF->SpriteID, 0, 16 );
							}
						}
						else
						{
							for (int p = 0; p < OC->Parts; p++)
							{
								NewFrame* OF = &NA->Frames[spr + p];
								GPS.ShowGPRedN( XX, YY, OF->FileID, OF->SpriteID, 0, 16 );
							}
						}
					}
				}
			}
			else
			{
				if (NM->Wall)
				{
					if (!BuildWall)
					{
						SetWallBuildMode( NatRefTBL[MyNation], BuildingID );
					}
					BuildMode = false;
				}
				else
				{
					BuildWall = 0;

					if (!GO->Enabled)
					{
						BuildMode = false;
					}

					int xxx, yyy;
					if (GetRealCoords( mouseX, mouseY, &xxx, &yyy ))
					{
						int x0 = mouseX + NM->PicDx;
						int y0 = mouseY + NM->PicDy;

						NewAnimation* UP = &NM->StandHi;
						NewAnimation* DN = &NM->StandLo;
						NewAnimation* WR = &NM->Work;

						int NNN = 5 + int( 8 * ( sin( double( GetRealTime() ) / 100 ) + 2 ) );
						int xx3 = xxx * 16;
						int yy3 = yyy * 16;
						int AddGP = -1;
						int sprs[MaxAStages];

						if (NM->CompxCraft)
						{
							AddGP = NM->CompxCraft->Stages[0].GPID;
							for (int i = 0; i < MaxAStages; i++)
							{
								if (NM->CompxCraft->Stages[i].GPID != 0)
								{
									sprs[i] = NM->CompxCraft->Stages[i].Empty.SpriteID;
								}
								else
								{
									sprs[i] = -1;
								}
							}
						}

						if (CheckSmartCreationAbility( NatRefTBL[MyNation], NM, &xx3, &yy3 ) != -1)
						{
							x0 = ( xx3 / 16 ) - ( mapx * 32 ) + NM->PicDx;
							y0 = ( yy3 / 32 ) - ( mapy * 16 ) + NM->PicDy - GetHeight( xx3 / 16, yy3 / 16 );

							if (DN->LineInfo)
							{
								for (int p = 0; p < DN->NFrames; p++)
								{
									//Placing building
									NewFrame* OF = &DN->Frames[p];
									GPS.ShowGPDarkN( x0, y0, OF->FileID, OF->SpriteID, NatRefTBL[MyNation], NNN );
								}
							}
							else
							{
								if (UP->Enabled)
								{
									for (int i = 0; i < UP->Parts; i++)
									{
										NewFrame* NF = &UP->Frames[i];
										GPS.ShowGPDarkN( x0 + i*UP->PartSize, y0, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
									}
								}
								if (DN->Enabled)
								{
									for (int i = 0; i < DN->Parts; i++)
									{
										NewFrame* NF = &DN->Frames[i];
										GPS.ShowGPDarkN( x0 + i*DN->PartSize, y0, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
									}
								}
							}

							if (WR->Enabled)
							{
								//Mill sails in placement mode
								NewFrame* NF = WR->Frames;
								GPS.ShowGPDarkN( x0 + NF->dx, y0 + NF->dy, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
							}

							if (NM->Port)
							{
								//Placing port
								int px = ( PortBuiX * 16 ) - ( mapx * 32 );
								int py = ( PortBuiY * 8 ) - ( mapy * 16 ) - GetHeight( PortBuiX * 16, PortBuiY * 16 );
								int LL = NM->BuiDist * 8;

								NewAnimation* BUANM = NM->BuiAnm;
								if (BUANM)
								{
									//Buoys in placement mode
									NewFrame* NF = &BUANM->Frames[div( tmtmt, BUANM->NFrames ).rem];
									px += NF->dx;
									py += NF->dy;
									GPS.ShowGPDarkN( px - LL - LL, py, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
									GPS.ShowGPDarkN( px + LL + LL, py, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
									GPS.ShowGPDarkN( px, py - LL, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
									GPS.ShowGPDarkN( px, py + LL, NF->FileID, NF->SpriteID, NatRefTBL[MyNation], NNN );
								}
							}

							if (AddGP != -1)
							{
								for (int i = 0; i < MaxAStages; i++)
								{
									if (sprs[i] != -1)
									{
										GPS.ShowGPDarkN( x0, y0, AddGP, sprs[i], NatRefTBL[MyNation], NNN );
									}
								}
							}
						}
						else
						{
							if (DN->LineInfo)
							{
								for (int p = 0; p < DN->NFrames; p++)
								{
									//Placing building, impossible terrain (red blinking)
									NewFrame* OF = &DN->Frames[p];
									GPS.ShowGPRedN( x0, y0, OF->FileID, OF->SpriteID, 0, NNN );
								}
							}
							else
							{
								if (UP->Enabled)
								{
									for (int i = 0; i < UP->Parts; i++)
									{
										NewFrame* NF = &UP->Frames[i];
										GPS.ShowGPRedN( x0 + i*UP->PartSize, y0, NF->FileID, NF->SpriteID, 0, NNN );
									}
								}
								if (DN->Enabled)
								{
									for (int i = 0; i < DN->Parts; i++)
									{
										NewFrame* NF = &DN->Frames[i];
										GPS.ShowGPRedN( x0 + i*DN->PartSize, y0, NF->FileID, NF->SpriteID, 0, NNN );
									}
								}
							}

							if (WR->Enabled)
							{
								//Mill sails in placement mode on impossible terrain
								NewFrame* NF = WR->Frames;
								GPS.ShowGPRedN( x0 + NF->dx, y0 + NF->dy, NF->FileID, NF->SpriteID, 0, NNN );
							}

							if (AddGP != -1)
							{
								for (int i = 0; i < MaxAStages; i++)
								{
									if (sprs[i] != -1)
									{
										GPS.ShowGPRedN( x0, y0, AddGP, sprs[i], 0, NNN );
									}
								}
							}
						}
					}
				}
			}
		}
	}

	SetRLCWindow( 0, 0, COPYSizeX, RSCRSizeY, SCRSizeX );

	ProcessSelectedTower();

	ScenaryLights();

	if (SHOWSLIDE && FogMode && BalloonState != 2 && ( !NATIONS[NatRefTBL[MyNation]].Vision ) && !NoText)
	{
		time0 = GetRealTime();

		if (NOPAUSE && !( LockFog || NoPFOG ))
		{
			ProcessFog1();
		}

		CreateFogImage();

		DrawFog();

		time4 = GetRealTime() - time0;
	}

	DrawCurves();

	ShowCostPlaces();

	GenShow();

	if (GridMode)
	{
		DrawGrids();
	}

	ProcessVotingView();

	ShowChat();

	if (NoText)
	{
		return;
	}

	//Adjust timestamps for all hints
	ProcessHints();

	ShowDestn();

	time7 = GetRealTime() - time0;

	if (SeqErrorsCount)//IMPORTANT: show syncro warning message
	{
		int TTT = GetTickCount() / 1500;
		if (TTT & 1)
		{
			ShowString( HintX, HintY - 100, DELSYNC, &BigWhiteFont );
		}
		else
		{
			ShowString( HintX, HintY - 100, DELSYNC, &BigRedFont );
		}
		SeqErrorsCount = 0;
		SyncroDoctor();
	}

	if (EditMapMode)
	{
		ProcessRivEditor();
		if (LockMode)
		{
			ShowLockBars();
		}
	}
	else
	{
		ShowWinner();
	}

	DrawSMS();

	ShowRMap();

	int xxx0 = RealLx - 400;
	int yyy0 = smapy + RealLy - 170;

	if (HeightEditMode == 1)
	{
		ShowString( xxx0, yyy0, HIEDMOD, &YellowFont );//"Редактирование высот."
		yyy0 -= 20;
	}

	if (HeightEditMode == 2)
	{
		ShowString( xxx0, yyy0, ADDSPOB, &YellowFont );//"Добавить спрайтовые объекты."
		yyy0 -= 20;
	}

	if (HeightEditMode == 3)
	{
		ShowString( xxx0, yyy0, DELSPOB, &YellowFont );//"Удалить объекты."
		yyy0 -= 20;
	}

	if (MEditMode)
	{
		ShowString( xxx0, yyy0, TEXMODE, &YellowFont );//"Режим текстурирования."
		yyy0 -= 20;
	}

	if (LockMode == 1)
	{
		ShowString( xxx0, yyy0, UNBLOCK, &YellowFont );//"Режим принудительной разблокировки."
		yyy0 -= 20;
	}

	if (LockMode == 2)
	{
		ShowString( xxx0, yyy0, DOBLOCK, &YellowFont );//"Режим принудительной блокировки."
		yyy0 -= 20;
	}

	if (LockMode == 3)
	{
		ShowString( xxx0, yyy0, DFBLOCK, &YellowFont );//"Режим блокировки по умолчанию."
		yyy0 -= 20;
	}

	if (PeaceMode)
	{
		ShowString( xxx0, yyy0, PEACMOD, &YellowFont );//"Режим мира."
		yyy0 -= 20;
	}

	if (WaterEditMode)
	{
		switch (WaterEditMode)
		{
		case 1:
			ShowString( xxx0, yyy0, SETWATT, &YellowFont );//"Режим редактирования воды: установка воды."
			break;
		case 2:
			ShowString( xxx0, yyy0, DELWATT, &YellowFont );//"Режим редактирования воды: стирание воды."
			break;
		case 3:
			ShowString( xxx0, yyy0, SETCOST, &YellowFont );//"Режим редактирования воды: установка отмели."
			break;
		case 4:
			ShowString( xxx0, yyy0, DELCOST, &YellowFont );//"Режим редактирования воды: стирание отмели."
			break;
		}
		yyy0 -= 20;
	}

	if (!( PlayGameMode == 1 || ( SCENINF.hLib && !CanProduce ) || EditMapMode ))
	{
		int NG = NATIONS[NatRefTBL[MyNation]].NGidot;
		int NF = NATIONS[NatRefTBL[MyNation]].NFarms;
		if (NF <= NG)
		{
			GPS.ShowGPRedN( BrigPnX + 42, BrigPnY, BordGP, 92, MyNation, 9 + int( 8 * sin( float( GetTickCount() ) / 100 ) ) );
		}
		else
		{
			if (NF - NG < 5)
			{
				GPS.ShowGP( BrigPnX + 42, BrigPnY, BordGP, 92, MyNation );
			}
		}

	}

	if (tmtmt > 20 && !Inform)
	{
		if (LastTimeStage == -1)
		{
			LastTimeStage = GetRealTime();
		}
		else
		{
			int dd = GetRealTime() - LastTimeStage;

			if (!NOPAUSE)
			{
				dd = 0;
				LastTimeStage = GetRealTime();
			}

			if (dd > 1000)
			{
				dd /= 1000;

				if (PINFO[0].GameTime > dd)
				{
					PINFO[0].GameTime -= dd;
				}
				else
				{
					PINFO[0].GameTime = 0;
				}

				GameTime += dd;

				if (PeaceTimeLeft > 1)
				{
					PeaceTimeLeft -= dd;
					if (PeaceTimeLeft < 1)
					{
						PeaceTimeLeft = 1;
					}
				}
				else
				{
					if (PeaceTimeLeft == 1)
					{
						CmdEndPT();
					}
				}
				LastTimeStage += dd * 1000;
			}
		}

		char cc1[100];
		char ccr[24];
		int DY = 0;

		if (PINFO[0].VictCond)
		{
			int t = PINFO[0].GameTime;
			int s = t % 60;
			int m = ( t / 60 ) % 60;
			int h = t / 3600;
			if (s < 10 && m < 10)sprintf( cc1, "%d:0%d:0%d", h, m, s );
			else if (s < 10 && m >= 10)sprintf( cc1, "%d:%d:0%d", h, m, s );
			else if (s > 10 && m < 10)sprintf( cc1, "%d:0%d:%d", h, m, s );
			else sprintf( cc1, "%d:%d:%d", h, m, s );
			ShowString( minix + 2, miniy - 22 + 2 - 4 + DY, cc1, &SmallBlackFont );
			ShowString( minix, miniy - 22 - 4 + DY, cc1, &SmallWhiteFont );
			DY = -14;
		}

		byte ord[8];
		int  par[8];
		int no = SortPlayers( ord, par );
		byte prevms = 0;

		if (!( SCENINF.hLib || PlayGameMode == 1 ))
		{
			if (PeaceTimeLeft)
			{
				if (!TMMS_PT)TMMS_PT = GetTextByID( "TMMS_PT" );
				int H1 = GameTime / 3600;
				int H2 = PeaceTimeLeft / 3600;
				int M1 = ( GameTime / 60 ) % 60;
				int M2 = ( PeaceTimeLeft / 60 ) % 60;
				int S1 = GameTime % 60;
				int S2 = PeaceTimeLeft % 60;
				sprintf( cc1, TMMS_PT, H1, M1 / 10, M1 % 10, S1 / 10, S1 % 10, ( tmtmt + CURTMTMT ) / 25, H2, M2 / 10, M2 % 10, S2 / 10, S2 % 10 );
				char cc4[100];
				cc4[0] = 0;
				AddRecordAndRatingSymbols( cc4 );
				ShowString( minix + 2 + GetRLCStrWidth( cc4, &SmallBlackFont ), miniy - 22 + 2 - 4 + DY, cc1, &SmallBlackFont );
				AddRecordAndRatingSymbols( cc1 );
				ShowString( minix, miniy - 22 - 4 + DY, cc1, &SmallWhiteFont );
			}
			else
			{
				if (!TMMS_NOPT)
				{
					TMMS_NOPT = GetTextByID( "TMMS_NOPT" );
				}

				int H1 = GameTime / 3600;
				int M1 = ( GameTime / 60 ) % 60;
				int S1 = GameTime % 60;
				char cc4[100];
				cc4[0] = 0;
				sprintf( cc1, TMMS_NOPT, H1, M1 / 10, M1 % 10, S1 / 10, S1 % 10, ( tmtmt + CURTMTMT ) / 25 );
				AddRecordAndRatingSymbols( cc4 );
				ShowString( minix + 2 + GetRLCStrWidth( cc4, &SmallBlackFont ), miniy - 22 + 2 - 4 + DY, cc1, &SmallBlackFont );
				AddRecordAndRatingSymbols( cc1 );
				ShowString( minix, miniy - 22 - 4 + DY, cc1, &SmallWhiteFont );
			}

			if (no)
			{
				int y = miniy - 22 - 4 - no * 14 + DY;
				for (int q = 0; q < no; q++)
				{
					byte ms = NATIONS[NatRefTBL[ord[q]]].NMask;
					byte c = 0xD0 + 4 * NatRefTBL[ord[q]];
					if (!( ms&prevms ))
					{
						int w;
						for (w = q; w < no && ( NATIONS[NatRefTBL[ord[w]]].NMask&ms ); w++);
						w -= q;
						if (w > 1)
						{
							int y0 = y + 1 + 2;
							int y1 = y + 10 + ( w - 1 ) * 14;
							int yc = ( y0 + y1 ) >> 1;
							int x = minix - 2;
							xLine( x, y0, x - 1, y0 + 1, c );
							xLine( x - 2, y0 + 2, x - 2, yc - 2, c );
							xLine( x - 3, yc - 1, x - 4, yc, c );
							xLine( x - 3, yc + 1, x - 2, yc + 2, c );
							xLine( x - 2, yc + 3, x - 2, y1 - 2, c );
							xLine( x - 1, y1 - 1, x, y1, c );
							char cc[32];
							cc[0] = 0;

							switch (PINFO[0].VictCond)
							{
							case 1:
								sprintf( cc, "%d", SumSquare[NatRefTBL[ord[q]]] );
								break;
							default:
								sprintf( cc, "%.2f", float( SumAccount[NatRefTBL[ord[q]]] ) / 100 );
							};
							int xt = x - 7 - GetRLCStrWidth( cc, &SmallWhiteFont );
							ShowString( xt, yc - 5, cc, &SmallBlackFont );
							ShowString( xt - 1, yc - 6, cc, &SmallWhiteFont );
						};
					};
					prevms = ms;
					if (( !SYNBAD[ord[q]] ) || ( GetTickCount() >> 9 ) & 1)
					{
						CBar( minix + 3, y + 5, 4, 4, c );
						Xbar( minix + 2, y + 4, 6, 6, c + 1 );
						Xbar( minix + 1, y + 3, 8, 8, c + 3 );
					};
					ShowVictInFormat( ccr, NatRefTBL[ord[q]], PINFO[0].VictCond );
					sprintf( cc1, "%s (%s)", GetPName( ord[q] ), ccr );
					ShowString( minix + 18 + 2, y + 2, cc1, &SmallBlackFont );
					ShowString( minix + 18, y, cc1, &SmallWhiteFont );
					int L = GetRLCStrWidth( cc1, &SmallWhiteFont );
					int vs = NATIONS[ord[q]].VictState;
					if (vs == 1)xLine( minix - 1, y + 8, minix + 20 + L, y + 8, c );
					if (vs == 3)DottedLine( minix - 1, y + 8, minix + 20 + L, y + 8, c );
					y += 14;
				};
			};
		};
	}

	if (PlayGameMode == 1)
	{
		ShowAbout();
	}

	if (!( NOPAUSE || LockPause || EnterChatMode ))
	{
		int L = GetRLCStrWidth( PAUSETEXT, &BigWhiteFont );
		int x = ( RealLx - L ) >> 1;
		int y = RealLy >> 1;
		DrawStdBar1( x - 64, y - 8, x + L + 64, y + 20 + 7 );
		ShowString( x - 2, y - 4, PAUSETEXT, &BigWhiteFont );
	}

	int t0 = GetRealTime();
	if (!PRVGT)PRVGT = t0;
	if (t0 - PRVGT > 2000)
	{
		ADDGR( 0, t0, 0, 0xFF );
		ADDGR( 0, t0, ( AddTime * 100 ) >> 7, 0xD0 );
		ADDGR( 0, t0, ( NeedAddTime * 100 ) >> 7, 0xD4 );
		ADDGR( 1, t0, CurrentStepTime, 0xD0 );
		if (PSUMM.NPL && PSUMM.PSET[0].NPings > 5)
		{
			for (int i = 0; i < NPlayers; i++)
			{
				int p = GetPing( PINFO[i].PlayerID );
				ADDGR( 2, t0, p, 0xD0 + PINFO[i].ColorID * 4 );
			}
		}
		PRVGT = t0;
	}

	DrawAllGrp();

	int uxxx = mapx + ( ( mouseX - smapx ) >> 5 );
	int uyyy = mapy + div24( mouseY - smapy );

	uxxx = ( mapx << 1 ) + ( ( mouseX - smapx ) >> 4 );
	uyyy = ( mapy << 1 ) + ( ( mouseY - smapy ) >> 3 );

	if (InfoMode)
	{
		return;
	}
	int ssy = smapy + 3;

	ssy += 100;
	sprintf( ggg, "Hi=%d", TerrHi );
	ShowString( smapx, ssy, ggg, &WhiteFont );
	ssy += 20;
	int cxx = MRealX >> 7;
	int cyy = MRealY >> 7;

	cxx = MRealX >> 6;
	cyy = MRealY >> 6;
	sprintf( ggg, "TopZone:%d", TopRef[cxx + cyy*TopLx] );
	CurZone = TopRef[cxx + cyy*TopLx];
	ShowString( smapx, ssy, ggg, &WhiteFont );
	ssy += 20;

	sprintf( ggg, "LastKey=%d", LastKey );
	ShowString( smapx, ssy, ggg, &WhiteFont );
	ssy += 16;


	sprintf( ggg, "FPS:%d", Flips );
	counter = 0;
	ShowString( smapx, ssy, ggg, &WhiteFont );
	ssy += 20;

	if (NSL[MyNation])
	{
		word MID = Selm[MyNation][0];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB)
			{
				sprintf( ggg, "%s : ID=%d  x=%d  y=%d", OB->Ref.General->Message, OB->Index, OB->RealX >> 4, OB->RealY >> 4 );
				ShowString( smapx, ssy, ggg, &WhiteFont );
				ssy += 20;
				char ordmes[250];
				ordmes[0] = 0;

				sprintf( ggg, "Brigade: %d  BrIndex: %d", OB->BrigadeID, OB->BrIndex );
				ShowString( smapx, ssy, ggg, &WhiteFont );
				ssy += 16;
				if (OB->BrigadeID != 0xFFFF)
				{
					Brigade* BR = &OB->Nat->CITY->Brigs[OB->BrigadeID];
					if (BR->ArmyID != 0xFFFF)
					{
						sprintf( ggg, "Army: %d ", BR->ArmyID );
						ShowString( smapx, ssy, ggg, &WhiteFont );
						ssy += 16;
					}
				}

				if (OB->LocalOrder)
				{
					Order1* OR1 = OB->LocalOrder;
					char OSTR[400] = "ORDER:";
					char cc[16];
					int nn = 0;
					while (OR1&&nn < 20)
					{
						sprintf( cc, " %d", OR1->OrderType );
						strcat( OSTR, cc );
						OR1 = OR1->NextOrder;
						nn++;
					};
					if (OR1)strcat( OSTR, "....." );
					ShowString( smapx, ssy, OSTR, &WhiteFont );
					ssy += 16;
				}
			}
		}
	}
}

int MiniLx;
int MiniLy;

//Additional offsets for jump-scrolling on small minimap
int MiniX, MiniY;

DWORD BMASK[MBR4] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
void DrawMiniFog();
void ViewRoads( int x, int y );
extern int NROADS;
void ShowSafetyInfo( int x, int y );
void DrawMinAZones( int x, int y );
void DrawMinAZonesVis( int x, int y );
extern int LastAttackX;
extern int LastAttackY;
extern int AlarmDelay;
extern IconSet UpgPanel;
extern IconSet AblPanel;
void DrawMinAZonesVis( int x, int y );

//Copy minimap into screen buffer
void GMiniShow()
{
	//Inform=GetKeyState(VK_MENU);
	if (NoText || PlayGameMode == 1)
		return;

	int MAXMP = 120;

	if (FullMini)
	{
		if (RealLx == 800)
		{
			if (UpgPanel.GetMaxX( 12 ) < 10 && AblPanel.GetMaxX( 12 ) < 10 && !Inform)
			{
				MAXMP = 480;
			}
		}
		else
		{
			MAXMP = 480;
		}
	}

	if (!MiniMade)
		CreateMiniMap();

	int MaxMLX = msx >> ADDSH;
	int MaxMLY = msy >> ADDSH;
	MiniLx = MaxMLX;
	MiniLy = MaxMLY;

	if (MiniLx > MAXMP)
		MiniLx = MAXMP;

	if (MiniLy > MAXMP)
		MiniLy = MAXMP;

	MiniX = ( mapx + ( smaplx >> 1 ) - MiniLx ) >> ADDSH;
	MiniY = ( mapy + ( smaply >> 1 ) - MiniLy ) >> ADDSH;

	if (MiniX < 0)
		MiniX = 0;

	if (MiniY < 0)
		MiniY = 0;

	if (MiniX + MiniLx >= MaxMLX)
		MiniX = MaxMLX - MiniLx - 1;

	if (MiniY + MiniLy >= MaxMLY)
		MiniY = MaxMLY - MiniLy - 1;

	minix = RealLx - MiniLx - 12;
	miniy = RealLy - MiniLy - 16;

	//TESTING: display color palette instead of minimap
	//Dimensions of minimap : MiniLx * MiniLy
	//? ? ? : MiniX = MiniY = -1
	//Position ? : minix = 772; miniy = 512
	/*
	char colorbyte = 0x00;
	for (int row = 0; row < 16; row++)
	{
		for (int col = 0; col < 16; col++)
		{
			for (int cell_x = 0; cell_x < 15; cell_x++)
			{
				for (int cell_y = 0; cell_y < 15; cell_y++)
				{
					minimap[15 * row + cell_x][15 * col + cell_y] = colorbyte;
				}
			}
			colorbyte++;
		}
	}
	*/

	//Copy data from minimap buffer into the screen buffer
	int scr = (int) ScreenPtr + minix + ( miniy * SCRSizeX );
	int til = (int) &minimap + MiniX + ( MiniY * maxmap );
	for (int i = 0; i < MiniLy; i++)
	{
		memcpy( (void*) scr, (void*) til, MiniLx );//REFACTORED: __asm
		scr += SCRSizeX;
		til += maxmap;
	}

	OneObject* OO;
	int mxx, myy;

	byte val;
	byte mycl = CLRT[MyNation];

	memset( BMASK, 0, sizeof BMASK );

	byte mmsk = GM( MyNation );

	for (int g = 0; g < MAXOBJECT; g++)
	{
		OO = Group[g];
		if (OO && !OO->Sdoxlo)
		{
			mxx = ( OO->RealX >> ( 9 + ADDSH ) ) - MiniX;
			myy = ( OO->RealY >> ( 9 + ADDSH ) ) - MiniY;
			if (mxx >= 0 && myy >= 0 && mxx < MiniLx && myy < MiniLy)
			{
				val = CLRT[OO->NNUM];
				int scr = (int) ScreenPtr + minix + mxx + ( ( myy + miniy ) * SCRSizeX );
				if (OO->ImSelected & mmsk)
				{
					val = 0xFF;
					if (OO->BrigadeID != 0xFFFF)
					{
						Brigade* BR = OO->Nat->CITY->Brigs + OO->BrigadeID;
						if (BR->WarType)
						{
							int pp = BR->ID;
							int idx = pp >> 5;
							BMASK[idx] |= ( 1 << ( pp & 31 ) );
						}
					}
				}

				//IMPORTANT: tag units on minimap
				//Substitute colors that habe poor visibility on minimap
				switch (val)
				{
				case 0xE4: //Black player
					val = 0xFB; //Tag as yellow
					break;
				case 0xE8: //White player
					val = 0x0A; //Tag as pink
					break;
				case 0xDC://Purple
					val = 0xFD;//Tag as magenta
					break;
				}

				//Place a colored 2x2 square tag
				memset( (void*) scr, val, 2 );//REFACTORED: __asm
				memset( (void*) ( scr + SCRSizeX ), val, 2 );
			}
		}
	}


	if (EditMapMode)
	{
		int x = mapPos[( MyNation << 1 )] >> 1;
		int y = mapPos[( MyNation << 1 ) + 1] >> 1;

		SetRLCWindow( minix, miniy, MiniLx, MiniLy, SCRSizeX );
		Hline( minix + x - 2 - MiniX, miniy + y - MiniY, minix + x - 2 + 4 - MiniX, 255 );
		Vline( minix + x - MiniX, miniy + y - 2 - MiniY, miniy + y - 2 + 4 - MiniX, 255 );
		ShowMiniDestn();

		if (GetKeyState( VK_CONTROL ) & 0x8000)
			DrawMinAZones( minix - MiniX, miniy - MiniY );

		SetRLCWindow( 0, 0, COPYSizeX, RSCRSizeY, SCRSizeX );
	}
	else
	{
		SetRLCWindow( minix, miniy, MiniLx, MiniLy, SCRSizeX );
		ShowMiniDestn();
		SetRLCWindow( 0, 0, COPYSizeX, RSCRSizeY, SCRSizeX );

		if (GetKeyState( VK_CONTROL ) & 0x8000)
			DrawMinAZonesVis( minix - MiniX, miniy - MiniY );
	}

	if (PreviewBrig >= 0 && PreviewBrig < 100)
	{
		SetRLCWindow( minix, miniy, MiniLx, MiniLy, SCRSizeX );
		Brigade* BR = CITY[NatRefTBL[MyNation]].Brigs + PreviewBrig;
		int x = 0;
		int y = 0;
		int N = 0;

		for (int p = 0; p < BR->NMemb; p++)
		{
			word MID = BR->Memb[p];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB&&OB->Serial == BR->MembSN[p])
				{
					x += OB->RealX >> 10;
					y += OB->RealY >> 10;
					N++;
				}
			}
		}

		if (N)
		{
			x /= N;
			y /= N;
			x += minix + 1;
			y += miniy + 1;
			x -= MiniX;
			y -= MiniY;
			DrawLine( x - 5, y - 2, x - 5, y + 2, clrYello );
			DrawLine( x + 5, y - 2, x + 5, y + 2, clrYello );
			DrawLine( x - 2, y - 5, x + 1, y - 5, clrYello );
			DrawLine( x - 2, y + 5, x + 2, y + 5, clrYello );

			DrawLine( x - 5, y - 2, x - 2, y - 5, clrYello );
			DrawLine( x + 5, y - 2, x + 2, y - 5, clrYello );
			DrawLine( x - 2, y + 5, x - 5, y + 2, clrYello );
			DrawLine( x + 2, y + 5, x + 5, y + 2, clrYello );
		}
		SetRLCWindow( 0, 0, COPYSizeX, RSCRSizeY, SCRSizeX );
	}

	PreviewBrig = -1;

	if ( FogMode && ( !NATIONS[NatRefTBL[MyNation]].Vision ) && BalloonState != 2 )
	{
		DrawMiniFog();
	}

	HandleGeology();

	if (AlarmDelay && LastAttackX != -1)
	{
		SetRLCWindow( minix, miniy, MiniLx, MiniLy, SCRSizeX );
		int x0 = minix - MiniX + ( LastAttackX >> ( 9 + ADDSH ) );
		int y0 = miniy - MiniY + ( LastAttackY >> ( 9 + ADDSH ) );
		int R = ( ( 60 - AlarmDelay ) % 10 ) * 2;
		Xbar( x0 - R, y0 - R, R + R, R + R, 0xFF );
		AlarmDelay--;
		SetRLCWindow( 0, 0, COPYSizeX, RSCRSizeY, SCRSizeX );
	}

	Xbar( minix + ( mapx >> ADDSH ) - MiniX, miniy + ( mapy >> ADDSH ) - MiniY, smaplx >> ADDSH, smaply >> ADDSH, 0xFF );//14
	int mx2 = MiniLx;
	int my2 = MiniLy;
	byte ccc = 0xD0 + ( NatRefTBL[MyNation] << 2 );
	Hline( minix - 1, miniy - 1, minix + mx2, ccc );
	Hline( minix - 1, miniy + my2, minix + mx2, ccc );
	Vline( minix - 1, miniy - 1, miniy + my2, ccc );
	Vline( minix + mx2, miniy, miniy + my2, ccc );
	Hline( minix - 2, miniy - 2, minix + mx2 + 1, ccc + 1 );
	Hline( minix - 2, miniy + my2 + 1, minix + mx2 + 2, ccc + 1 );
	Vline( minix - 2, miniy - 2, miniy + my2 + 1, ccc + 1 );
	Vline( minix + mx2 + 1, miniy - 1, miniy + my2 + 1, ccc + 1 );
	DrawStdRect( minix - 2, miniy - 2, minix + mx2 + 2, miniy + my2 + 2 );
}

#define step 1

int StepX = 0;
int StepY = 0;

extern int  ReliefBrush;
extern int  TerrBrush;
bool SelSoundDone;
extern int SelSoundType;
extern bool SelSoundReady;
void DoSelSound( int type )
{
	if (!SelSoundDone)
	{
		SelSoundDone = true;
		SelSoundReady = true;
		SelSoundType = type;
	};
};
extern bool fixed;
extern bool realLpressed;
extern bool realRpressed;

//Fixes some mouse related bug?
void MFix()
{
	fixed = true;

	if (Lpressed)
	{
		Lpressed = realLpressed;
	}

	if (Rpressed)
	{
		Rpressed = realRpressed;
	}
}

void CmdCreateNewTerr( byte NI, int x, int y, word ID );
word GetNewEnemy( int xr, int yr, byte NI );
void ShowPen( int x, int y, int r, byte c );
void AddHi( int x, int y1, int r, int h );
void AverageHi( int x, int y1, int r );
void PlanarHi( int x, int y1, int r );
void delTrees( int x, int y, int r );
void addTrees( int x, int y, int r );
extern int HiStyle;
int Prop43( int y );
bool GetRealCoords( int xs, int ys, int* xr, int* yr )
{
	int xmx, ymy;
	int smlx = smaplx << Shifter;
	int smly = mul3( smaply ) << ( Shifter - 2 );
	if (xs > smapx&&ys > smapy&&xs < smapx + smlx&&ys < smly)
	{
		xmx = ( mapx << 5 ) + ( ( xs - smapx ) << ( 5 - Shifter ) );
		ymy = ( mul3( mapy ) << 3 ) + ( ( ys - smapy ) << ( 5 - Shifter ) );
		if (Mode3D)ymy = ConvScrY( xmx, ymy );
		*yr = Prop43( ymy );
		*xr = xmx;
		return true;
	}
	else return false;
};
void RandomHi( int x, int y1, int r, int ms, int fnd );
void AddHiPlanar( int x, int y1, int r, int h );
void CreatePlane( int x, int y1, int r );
void SetSpot( int x, int y );
word GetNewFriend( int xr, int yr, byte NI );
bool LockMouse = false;
bool WasSelectionBar = false;
void CmdSelectBuildings( byte NI );
int LastMx;
int LastMy;
void CHKS()
{
	//assert(StartX<100000);
};
bool StrelMode = false;

void ProcessRM_Load();

void ProcessRM_Save( int x0, int y0, int x1, int y1 );
void ProcessTexPiece( int x, int y, bool press );

//Configured speed of mouse scrolling = { 2, 3, ..., 10 }
int ScrollSpeed = 5;

//Hardcoded speed of arrow key scrolling = 4
const int kKeyboardScrollSpeed = 4;

int LastClickTime = 0;
bool LastRealLP = false;
int LastClcX, LastClcY;
extern int SHIFT_VAL;
extern bool SetDestMode;
extern char* FormationStr;
void ControlAZones( int x, int y );
void GoToCurSelPosition();
void CmdSelectIdlePeasants( byte );
void CmdSelectIdleMines( byte );

void ProcessInformation();

int InfAddY = 16;
int InfDY = 32 + InfAddY;
int InfDX = 10 + 243;
int InfY1 = 450 + InfAddY;
int InfY2 = 450 - 22 + InfAddY;
void CmdPause( byte NI );
word AdvancedGetNewEnemy( int rex, int rey, int xr, int yr, byte NI );
word AdvancedGetNewFriend( int rex, int rey, int xr, int yr, byte NI );
extern bool MouseOverZone;
bool MouseOverMiniMenu = 0;
int TestCapture( OneObject* OBJ );
bool CheckWallClick( int x, int y );
void CmdSelAllUnits( byte NI );
void CmdSelAllShips( byte NI );
void HandleMouse( int x, int y );
void EDGETEST();
void ControlZones();
void UnPress();
int OldLpressed = -1;
bool OverInform = 0;
bool unpress = 0;
void GlobalHandleMouse(bool process_scrolling);
void DrawZones();
void GameKeyCheck();

void ProcessScreen()
{
	GameKeyCheck();
	NoPFOG = 1;
	GFieldShow();
	NoPFOG = 0;
	GMiniShow();
	ShowProp();
	ShowAbility();
	DrawZones();
	GlobalHandleMouse(false);//BUGFIX: call rate was way to high
	MFix();
}

void HandleSMSChat( char* Mess );
void HandlwSMSMouse();
void ProcessRESSEND();
void ProcessMapInterface();
extern bool DRAWLOCK;
void ClearModes();
extern bool MultiTvar;
extern bool OptHidden;
bool CheckFlagsNeed();
void CmdVote( byte result );

extern int ExitNI;

//Check SpecCmd, read from MouseStack
void GlobalHandleMouse(bool process_scrolling)
{
	if (ExitNI != -1)
	{
		CmdEndGame( ExitNI, 1, 0 );
		ExitNI = -1;
	}

	if (SpecCmd == 199)
	{
		CmdEndGame( MyNation, 1, 115 );
		SpecCmd = 0;
	}

	if (SpecCmd == 171)
	{
		CmdVote( 1 );
		SpecCmd = 0;
	}

	if (SpecCmd == 172)
	{
		CmdVote( 2 );
		SpecCmd = 0;
	}

	DRAWLOCK = 0;

	ProcessInformation();

	//Handle resource transfer dialog if necessary
	ProcessRESSEND();

	if (NPlayers > 1)
	{
		if (CheckFlagsNeed())
		{
			if (Inform)
			{
				OptHidden = 1;
			}
			ProcessMapInterface();
		}
	}
	else
	{
		if (EditMapMode || MultiTvar || PlayGameMode == 2)
		{
			if (Inform)
			{
				OptHidden = 1;
			}
			ProcessMapInterface();
		}
		else
		{
			if (!BuildWall)
			{
				ClearModes();
			}
		}
	}

	if (DRAWLOCK)
	{
		ProcessTexPiece( ( mapx * 32 ) + ( RealLx / 2 ), ( mapy * 32 ) + RealLy, 0 );
		return;
	}

	OldLpressed = -1;
	unpress = 0;
	OverInform = 0;

	if (Inform && !NoPress)
	{
		if (mouseX > InfDY - 3 && mouseY < InfY1 + 8 && mouseX>RealLx - InfDX - 3)
		{
			OldLpressed = Lpressed;
			Lpressed = false;
			OverInform = 1;
			unpress = 1;
			UnPress();
		}
	}

	MouseStack* MEV = ReadMEvent();

	if (MEV)
	{
		int x0 = mouseX;
		int y0 = mouseY;
		bool lp = Lpressed;
		bool rp = Rpressed;
		bool rlp = realLpressed;
		bool rrp = realRpressed;
		bool Ctrl = ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0;
		bool Shift = ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
		int x, y;
		do
		{
			x = MEV->x;
			y = MEV->y;
			Lpressed = MEV->Lpressed;
			Rpressed = MEV->Rpressed;
			realLpressed = MEV->rLpressed;
			realRpressed = MEV->rRpressed;
			if (unpress)
			{
				UnPress();
				Lpressed = 0;
				Rpressed = 0;
			}
			bool ll = Lpressed;
			bool rr = Rpressed;

			ControlZones();

			if ( ll && !Lpressed )
			{
				unpress = 1;
			}

			if ( rr && !Rpressed )
			{
				unpress = 1;
			}

			if ( unpress && SpecCmd == 241 )
			{
				SpecCmd = 0;
			}

			HandleMouse( x, y );
			MEV = ReadMEvent();
		} while (MEV);

		Lpressed = lp;
		Rpressed = rp;

		if (unpress)
		{
			Lpressed = 0;
			Rpressed = 0;

			if (SpecCmd == 241)
			{
				SpecCmd = 0;
			}
		}
		realLpressed = rlp;
		realRpressed = rrp;
	}
	else
	{
		bool ll = Lpressed;
		bool rr = Rpressed;

		ControlZones();

		if (ll && !Lpressed)
		{
			unpress = 1;
		}
		if (rr && !Rpressed)
		{
			unpress = 1;
		}
		if (unpress)
		{
			Lpressed = 0;
			Rpressed = 0;
			if (SpecCmd == 241)
			{
				SpecCmd = 0;
			}
		}
		HandleMouse( mouseX, mouseY );
	}

	//Scrolling
	if (process_scrolling)
	{
		EDGETEST();
	}

	if (OldLpressed != -1)
	{
		Lpressed = OldLpressed != 0;
	}
}

extern char** NatNames;
extern bool RESMODE;
bool GUARDMODE = 0;
bool PATROLMODE = 0;
extern bool AttGrMode;
int GuardPtr1 = 10;
int GuardPtr2 = 12;
int PatrolPtr = 13;
int AttGrPtr = 9;
int EnterTrPtr = 11;
bool CheckTransportEntering( OneObject* OB );
void CmdSetGuardState( byte, word );
extern char HEIGHTSTR[12];
bool CheckCurveLinked( int x, int y );
void CmdAttackGround( byte NI, int x, int y );
char EXCOMM[1024];
bool HaveExComm = 0;
void CmdAddMoney( byte, DWORD );

extern const int kChatMessageDisplayTime;

__declspec( dllexport ) void SetExComm( char* Data, int size )
{
	if (size < 1024)
	{
		memcpy( EXCOMM, Data, size );
		HaveExComm = 1;
	}
}

//Assign orders based on mouse actions
void HandleMouse( int x, int y )
{
	if (HaveExComm)
	{
		switch (EXCOMM[0])
		{
		case 0://chat message
			CreateTimedHintEx( EXCOMM + 1, kChatMessageDisplayTime, 32 );//Chat message
			break;

		case 1://money bonus
			CmdAddMoney( MyNation, *( (DWORD*) ( EXCOMM + 1 ) ) );
			break;
		}

		HaveExComm = 0;
		memset( EXCOMM, 0, sizeof EXCOMM );
	}

	HandlwSMSMouse();

	if (GoAndAttackMode && !NSL[MyNation])
	{
		GoAndAttackMode = 0;
	}

	curptr = 0;

	//Check if a wall is beeing build
	if (Rpressed && BuildMode)
	{
		Rpressed = 0;
		BuildMode = false;

		SetWallBuildMode( 0xFF, 0 );

		UnPress();
	}
	if (Rpressed && BuildWall)
	{
		Rpressed = 0;
		SetWallBuildMode( 0xFF, 0 );

		UnPress();
	}

	byte MyMask = NATIONS[MyNation].NMask;
	byte LastSCMD = SpecCmd;

	NoPress = false;

	//Unit selection
	if (!( GetKeyState( VK_SHIFT ) & 0x8000 ))
	{
		if (SHIFT_VAL)
		{
			if (SHIFT_VAL > 0 && SHIFT_VAL < 99)
			{
				//Place unit selection order in ExBuf
				CmdSelBrig( MyNation, 0, SHIFT_VAL - 1 );
			}
		}
		SHIFT_VAL = 0;
	}

	bool CanCapt = 0;
	bool CanWork = 0;
	bool CanAttBLD = 0;
	bool CanEnter = 0;
	bool CanShoot = 0;
	byte MTMASK = 0;

	//Check for special actions on pointed spot
	//e.g. gather resource, build / repair, capture, enter transport
	if (!( MouseOverZone || OverInform || MouseOverMiniMenu ))
	{
		int N = ImNSL[MyNation];
		word* uni = ImSelm[MyNation];
		word* sns = ImSerN[MyNation];

		for (int i = 0; i < N; i++)
		{
			word MID = uni[i];
			if (MID != 0xFFFF)
			{
				OneObject* ME = Group[MID];
				if (ME && ME->Serial == sns[i] && !ME->Sdoxlo)
				{
					NewMonster* NM = ME->newMons;
					if (NM->AttBuild)
					{
						CanAttBLD = 1;
					}
					if (!NM->Capture)
					{
						CanCapt = 1;
					}
					if (NM->Peasant)
					{
						CanWork = 1;
					}
					if (!NM->Priest)
					{
						MTMASK |= NM->KillMask;
					}
					if (!( ME->NewBuilding || ME->LockType ))
					{
						CanEnter = 1;
					}
					if (NM->CanShoot)
					{
						CanShoot = 1;
					}
				}
			}
		}

		if (GoAndAttackMode)
		{
			curptr = 1;
		}

		if (SetDestMode)
		{
			curptr = 8;
		}

		if (AttGrMode)
		{
			curptr = AttGrPtr;
		}
	}

	if (( !Lpressed ) && WasSelectionBar)
	{
		int N = ImNSL[MyNation];
		if (N)
		{
			word* seli = ImSelm[MyNation];
			for (int k = 0; k < N; k++)
			{
				word MID = seli[k];
				if (MID != 0xFFFF)
				{
					OneObject* OB = Group[MID];
					if (OB && OB->Ready)
					{
						MakeOrderSound( OB, 4 );
					}
				}
			}
		}
		WasSelectionBar = false;
	}
	
	bool Ipanel = true;
	bool cooren = false;

	//Is the mouse pointer inside the minimap?
	bool mini = false;

	//Special calculated mouse coordinates
	int xmx = 0;
	int ymy = 0;

	//Check if mouse is above the minimap
	if (!LockMouse && x >= minix && y >= miniy && x < minix + ( msx / 2 ) && y < miniy + ( msy / 2 ))
	{
		SetRLCWindow( minix, miniy, msx >> ADDSH, msy >> ADDSH, SCRSizeX );
		Hline( x - 4, y, x - 4 + 8, 255 );
		Vline( x, y - 4, y - 4 + 8, 255 );

		SetRLCWindow( 0, 0, COPYSizeX, RSCRSizeY, SCRSizeX );

		cooren = true;

		xmx = ( x - minix + MiniX ) << ADDSH;
		ymy = ( y - miniy + MiniY ) << ADDSH;

		if (xmx < 1)
		{
			xmx = 1;
		}
		if (ymy < 1)
		{
			ymy = 1;
		}
		if (xmx >= msx)
		{
			xmx = msx - 1;
		}
		if (ymy >= msy)
		{
			ymy = msy - 1;
		}

		xmx = ( xmx * 32 ) + 16;
		ymy = ( mul3( ymy ) * 8 ) + 12;

		mini = true;
	}
	else
	{
		//Mouse is not over the minimap

		int smlx = smaplx << Shifter;
		int smly = mul3( smaply ) << ( Shifter - 2 );

		if (x > smapx && y > smapy && x < smapx + smlx && y < smly)
		{
			xmx = ( mapx * 32 ) + ( ( x - smapx ) << ( 5 - Shifter ) );
			ymy = ( mul3( mapy ) * 8 ) + ( ( y - smapy ) << ( 5 - Shifter ) );
			cooren = true;
		}
	}

	int ysy = ymy;

	if (Mode3D && cooren)
	{
		ysy = ConvScrY( xmx, ymy );
	}

	int yreal = Prop43( ysy );

	LastMx = xmx;
	LastMy = yreal;

	sprintf( HEIGHTSTR, "%d", GetHeight( LastMx, LastMy ) );

	if (cooren && EditMapMode)
	{
		ControlAZones( xmx, yreal );
	}

	bool SETDEST = false;
	bool DOGUARD = 0;
	bool DOPATROL = 0;
	bool DOATTGR = 0;

	if (PATROLMODE && Rpressed)
	{
		DOPATROL = 1;
		PATROLMODE = 0;
		Lpressed = false;
		Rpressed = false;

		UnPress();
	}

	if (( SetDestMode || GUARDMODE || PATROLMODE || AttGrMode ) && ( !mini ) && ( Lpressed || Rpressed ))
	{
		if (SetDestMode)
		{
			SETDEST = true;
		}

		if (GUARDMODE)
		{
			DOGUARD = 1;
		}

		if (PATROLMODE)
		{
			DOPATROL = 1;
		}
		PATROLMODE = 0;

		if (AttGrMode)
		{
			DOATTGR = 1;
		}
		AttGrMode = 0;

		SetDestMode = false;
		Lpressed = false;
		Rpressed = false;

		UnPress();
	}

	if (GetCoord)
	{
		if (Lpressed)
		{
			if (cooren)
			{
				if (UNIM)
				{
					UNIM( xmx / 32, ymy / 32 );
				}
			}

			Lpressed = false;
			curptr = 0;
			curdx = 0;
			curdy = 0;
			GetCoord = false;

			DoSelSound( 1 );

			MFix();

			goto PrInfo;
		}

		if (Rpressed)
		{
			curptr = 0;
			curdx = 0;
			curdy = 0;
			GetCoord = false;

			MFix();

			goto PrInfo;
		}
	}

	if (SpecCmd == 137)
	{
		CmdPause( MyNation );
	}

	if (SpecCmd == 1)
	{
		CmdSelAllUnits( MyNation );
	}

	if (SpecCmd == 2)
	{
		CmdCreateGoodSelection( MyNation, mapx * 32, mapy * 32, ( mapx + smaplx - 1 ) * 32, ( mapy + smaply - 1 ) * 32 );
	}

	//Select all peasants
	if (SpecCmd == 3)
	{
		CmdCreateGoodKindSelection( MyNation, 0, 0, msx << 5, mul3( msy ) << 3, 0 );
	}

	//On screen
	if (SpecCmd == 4)
	{
		CmdCreateGoodKindSelection( MyNation, mapx << 5, mapy << 5, ( mapx + smaplx - 1 ) << 5, mul3( mapy + smaply - 1 ) << 3, 0 );
	}

	//Select all warriors
	if (SpecCmd == 5)
	{
		CmdCreateGoodKindSelection( MyNation, 0, 0, msx << 5, mul3( msy ) << 3, 2 );
	}

	//On screen
	if (SpecCmd == 6)
	{
		CmdCreateGoodKindSelection( MyNation, mapx << 5, mapy << 5, mul3( mapx + smaplx - 1 ) << 3, ( mapy + smaply - 1 ) << 5, 2 );
	}

	if (SpecCmd == 9)
	{
		CmdSelectBuildings( MyNation );
	}

	if (SpecCmd == 200)
	{//DEL key pressed
		if (!RESMODE)
		{//BUGFIX: Don't kill if player is in resource transfer dialog
			CmdDie( MyNation );
		}
	}

	if (SpecCmd == 201)
	{
		CmdSelAllShips( MyNation );
	}

	if (SpecCmd == 111)
	{
		GoToCurSelPosition();
	}

	if (SpecCmd == 112)
	{
		if (LastActionX != 0)
		{
			mapx = ( LastActionX / 32 ) - ( smaplx / 2 );
			mapy = ( LastActionY / 32 ) - ( smaply / 2 ) - ( GetHeight( LastActionX, LastActionY ) / 16 );

			if (mapx <= 0)
			{
				mapx = 1;
			}

			if (mapy <= 0)
			{
				mapy = 1;
			}

			if (mapx + smaplx > msx + 1)
			{
				mapx = msx - smaplx + 1;
			}

			if (mapy + smaply > msy + 1)
			{
				mapy = msy - smaply + 1;
			}
		}
	}

	if (SpecCmd == 113)
	{
		CmdSelectIdlePeasants( MyNation );
	}

	if (SpecCmd == 114)
	{
		CmdSelectIdleMines( MyNation );
	}

	if (SpecCmd == 11)
	{
		word NSel = ImNSL[MyNation];
		if (NSel)
		{
			word MID = ImSelm[MyNation][0];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB)
				{
					CmdCreateGoodTypeSelection( MyNation, 0, 0, msx << 5, mul3( msy ) << 3, OB->NIndex );
				}
			}
		}
	}

	WallHandleMouse();

	if (SpecCmd == 241)
	{
		//Select all units of selected type on screen
		word NSel = ImNSL[MyNation];
		if (NSel)
		{
			word MID = ImSelm[MyNation][0];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB)
				{
					CmdCreateGoodTypeSelection( MyNation, mapx << 5, mapy << 4, ( mapx + smaplx - 1 ) << 5, ( mapy + smaply - 1 ) << 4, OB->NIndex );
				}
			}
		}
	}

	SpecCmd = 0;

	LastRealLP = Lpressed;

	int xxx;
	int yyy;

	if (cooren)
	{
		xxx = xmx / 32;
		yyy = div24( ymy );
		MRealX = xmx;
		MRealY = yreal;

		if (LastSCMD == 219)
		{
			DelInGrid( xmx, ymy );
		}

		if (!mini)
		{
			ProcessTexPiece( xmx, ( mapy * 32 ) + mouseY * 2, Lpressed );
		}

		if (LastSCMD == 218)
		{
			ProcessRM_Load();
		}

		TerrHi = GetHeight( xmx, yreal );

		if (MEditMode)
		{
			ShowPen( xmx, ysy, ReliefBrush * 16, 255 );
		}

		if (HeightEditMode == 1 || HeightEditMode == 10 || HeightEditMode == 11 || HeightEditMode == 12 || HeightEditMode == 13)
		{//edit height mode
			bool LockKey = 0;
			if (Lpressed)
			{
				switch (HeightEditMode)
				{
				case 1:
					HiStyle = 1;
					LockKey = 1;
					break;
				case 10:
					HiStyle = 7;
					LockKey = 1;
					break;
				case 11:
					HiStyle = 9;
					LockKey = 1;
					break;
				case 12:
					HiStyle = 3;
					LockKey = 1;
					break;
				};
			};
			if (Rpressed)
			{
				switch (HeightEditMode)
				{
				case 1:
					HiStyle = 2;
					LockKey = 1;
					break;
				case 10:
					HiStyle = 8;
					LockKey = 1;
					break;
				case 11:
					HiStyle = 5;
					LockKey = 1;
					break;
				case 12:
					HiStyle = 4;
					LockKey = 1;
					break;
				};
			};
			bool Shpress = ( GetKeyState( VK_SHIFT ) & 0x8000 ) || HeightEditMode == 13;
			if (CheckCurve() || Shpress)
			{
				bool LINK = CheckCurveLinked( xmx, yreal );
				if (( Lpressed&&LINK ) || Rpressed)Shpress = 0;
				if (LINK)ShowPen( xmx, ysy, 8, 0xFB );
				if (Lpressed)
				{
					AddPointToCurve( xmx, yreal, !Shpress, 2 );
					if (!Shpress)Lpressed = 0;
				}
				else if (!Shpress)AddPointToCurve( xmx, yreal, true, 2 );
			}
			else ShowPen( xmx, ysy, ReliefBrush << 4, 0xFB );
			switch (HiStyle)
			{
			case 1://+
				AddHi( xmx, ysy, ReliefBrush << 4, 10 );
				break;
			case 2://+
				AddHi( xmx, ysy, ReliefBrush << 4, -10 );
				break;
			case 3:// *
				AverageHi( xmx, ysy, ReliefBrush << 4 );
				break;
			case 4:// /
				PlanarHi( xmx, ysy, ReliefBrush << 4 );
				break;
			case 5:
				RandomHi( xmx, ysy, ReliefBrush << 4, 1, 1 );
				break;
			case 6:
				RandomHi( xmx, ysy, ReliefBrush << 4, 3, 3 );
				break;
			case 7://PgUP
				AddHiPlanar( xmx, ysy, ReliefBrush << 4, 3 );
				break;
			case 8://PgDN
				AddHiPlanar( xmx, ysy, ReliefBrush << 4, -3 );
				break;
			case 9://Home
				CreatePlane( xmx, ysy, ReliefBrush << 4 );
				break;
			};
			HiStyle = 0;
			if (LockKey)return;
		};
		if (HeightEditMode == 2)
		{//Add tree mode
			ShowPen( xmx, ysy, ReliefBrush << 4, 0xFA );
		};
		if (HeightEditMode == 3)
		{//Delete tree mode
			ShowPen( xmx, ysy, ReliefBrush << 4, 0xF9 );
		};
		if (LockMode == 1)
		{
			//Lock mode
			ShowPen( xmx, ysy, 32, 0xDC );//ReliefBrush<<4,0xDC);
		};
		if (LockMode == 2)
		{
			//Lock mode
			ShowPen( xmx, ysy, 32, 0xBB );//ReliefBrush<<4,0xBB);
		};
		if (LockMode == 3)
		{
			//Lock mode
			ShowPen( xmx, ysy, 32, 0xD8 );//ReliefBrush<<4,0xD8);
		};
		if (WaterEditMode&&WaterEditMode < 3)
		{
			byte cc1 = 0xB4;
			byte cc2 = 0xBA;
			if (WaterEditMode == 2)
			{
				cc1 = 0xA8;
				cc2 = 0xC8;
			};
			bool M3D = Mode3D;
			Mode3D = false;
			int rr = ReliefBrush << 5;
			ShowPen( xmx, ymy, rr, cc1 );
			int dr = CostThickness << 4;
			if (dr < rr)ShowPen( xmx, ymy, rr - dr, cc2 );
			Mode3D = M3D;
		};
		if (Lpressed && !( mini || BuildWall ))
		{
			if (ChoosePosition)
			{
				mapPos[( MyNation << 1 )] = xxx;
				mapPos[( MyNation << 1 ) + 1] = yyy;
				ChoosePosition = false;
			};
			if (HeightEditMode == 2)
			{
				//CreateBlob(xxx-1,yyy-1,4*BlobMode,ReliefBrush);
				addTrees( xmx, ysy, ReliefBrush << 4 );
				goto Edgetest;
			};
			if (HeightEditMode == 3)
			{
				//CreateBlob(xxx-1,yyy-1,4*BlobMode,ReliefBrush);
				delTrees( xmx, ysy, ReliefBrush << 4 );
				goto Edgetest;
			};
			if (WaterEditMode == 1)
			{
				SetWaterSpot( xmx, ysy, ReliefBrush );
				goto Edgetest;
			};
			if (WaterEditMode == 2)
			{
				EraseWaterSpot( xmx, ysy, ReliefBrush );
				goto Edgetest;
			};
			if (WaterEditMode == 3)
			{
				SetBrightSpot( xmx, ysy, ReliefBrush, true );
				goto Edgetest;
			};
			if (WaterEditMode == 4)
			{
				SetBrightSpot( xmx, ysy, ReliefBrush, false );
				goto Edgetest;
			};
			if (MEditMode)
			{
				bool Shpress = ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0;
				if (CheckCurve() || Shpress)
				{
					AddPointToCurve( xmx, yreal, !Shpress, 1 );
					if (!Shpress)Lpressed = 0;
				}
				else SetTexturedRound( xmx, ymy, ReliefBrush * 16, EditMedia );
				goto Edgetest;
			};
			if (LockMode == 1)
			{
				AddUnLockbar( xmx >> 6, yreal >> 6 );
				goto Edgetest;
			};
			if (LockMode == 2)
			{
				AddLockBar( xmx >> 6, yreal >> 6 );
				goto Edgetest;
			};
			if (LockMode == 3)
			{
				AddDefaultBar( xmx >> 6, yreal >> 6 );
				goto Edgetest;
			};
			if (Creator < 4096 + 200)
			{
				//for(int i=0;i<2;i++)for(int j=0;j<2;j++)
				CmdCreateNewTerr( MyNation, xmx, ConvY( ysy ), Creator | 32768 );
				//CmdCreateTerrain(MyNation,xxx,yyy,Creator);
				MFix();
				goto PrInfo;;
			};

			if (BuildMode)
			{
				//if(CheckGold(xxx,yyy,BNat->Mon[BuildingID])&&ManualFogCheck(xxx,yyy,NATIONS[MyNation].Mon[BuildingID]->SizeX))
				//	CmdCreateBuilding(MyNation,xxx,yyy,BuildingID);
				int x0 = xmx << 4;
				int y0 = ConvY( ysy ) << 4;
				if (CheckSmartCreationAbility( NatRefTBL[MyNation], NATIONS[MyNation].Mon[BuildingID]->newMons, &x0, &y0 ) != -1)
				{
					;
					CmdCreateBuilding( MyNation, x0, y0, BuildingID );
					if (!( GetKeyState( VK_SHIFT ) & 0x8000 ))BuildMode = false;
					Lpressed = false;
					UnPress();
				};
				MFix();
				goto PrInfo;
			};
		};
	}

	if (cooren)
	{
		int gx = xmx;
		int gy = ymy;
		int gx1 = StartX;
		int gy1 = StartY;
		int LStartY = ConvScrY( StartX, StartY );
		if (gx > gx1)
		{
			int z = gx; gx = gx1; gx1 = z;
		};
		if (gy > gy1)
		{
			int z = gy; gy = gy1; gy1 = z;
		};
		int xxx = xmx >> 5;
		int yyy = div24( ysy );
		int xxx1 = StartX >> 5;
		int yyy1 = div24( LStartY );
		if (xxx1 < xxx)
		{
			int z = xxx; xxx = xxx1; xxx1 = z;
		};
		if (yyy1 < yyy)
		{
			int z = yyy; yyy = yyy1; yyy1 = z;
		};

		if (Lpressed && !mini)
		{
			if (FogMode && GetFog( gx, gy * 2 ) < 900)
			{
				//Clicked area is hidden behind fog of war
				//Do NOT reveal enemy location through hints
			}
			else
			{
				//Show hint when clicking on enemy unit, building or formation
				word ENMI = GetNewEnemy( gx, gy, MyNation );
				if (ENMI != 0xFFFF)
				{
					OneObject* OBBE = Group[ENMI];
					if (OBBE)
					{
						char hint_string[128];

						//Check if unit is part of formation
						if (OBBE->InArmy && OBBE->BrigadeID != 0xFFFF)
						{
							Brigade* BR = OBBE->Nat->CITY->Brigs + OBBE->BrigadeID;

							int NU = 0;
							char* UName = nullptr;

							//Get unit name and count formation strength
							for (int i = 2; i < BR->NMemb; i++)
							{
								word MID = BR->Memb[i];
								if (MID != 0xFFFF)
								{
									OneObject* BOB = Group[MID];
									if (BOB && BOB->Serial == BR->MembSN[i])
									{
										NU++;
										UName = BOB->Ref.General->Message;
									}
								}
							}

							if (UName)
							{
								//Show hint with unit type and formation strength
								sprintf( hint_string, FormationStr, UName, NU );

								CreateTimedHint( hint_string, kMinorMessageDisplayTime );//Formation hint
							}
						}
						else
						{
							//No formation, just simple unit or building
							//Show hint with name and remaining hp
							sprintf( hint_string, "%s (%s) (%d/%d)",
								OBBE->newMons->Message,
								NatNames[OBBE->Ref.General->NatID],
								OBBE->Life,
								OBBE->MaxLife );

							CreateTimedHint( hint_string, kMinorMessageDisplayTime );//Unit hint
						}

						AddPulse( ENMI, 0xC9 );
					}
				}
			}

			if (!BuildWall)
			{
				if (!SelSoundDone)
				{
					SelSoundDone = true;
					SelSoundReady = true;
					SelSoundType = 0;
				}

				SetRLCWindow( smapx, smapy, smaplx << Shifter, mul3( smaply ) << ( Shifter - 2 ), SCRSizeX );

				int xz1 = smapx + ( ( StartX - ( mapx << 5 ) ) >> ( 5 - Shifter ) );
				int yz1 = smapy + ( ( StartY - ( mul3( mapy ) << 3 ) ) >> ( 5 - Shifter ) );
				int vdx = smapx - ( mapx << 5 );
				int vdy = smapy - ( mapy << 4 );

				Xbar( xz1, yz1, x - xz1, y - yz1, 254 );

				if (Norma( x - xz1, y - yz1 ) > 12)
				{
					NoPress = true;
				}

				if (x != xz1 || y != yz1)
				{
					Ipanel = false;
				}

				if (LastSCMD == 217)
				{
					ProcessRM_Save( xz1 - vdx, yz1 - vdy, x - vdx, y - vdy );
				}

				LockMouse = true;
				int nn = 0;
				CmdCreateGoodSelection( MyNation, gx, gy, gx1, gy1 );
				WasSelectionBar = true;
			}
		}
		else
		{
			if (!mini)
			{
				if (!BuildWall)
				{
					StartX = xmx;
					StartY = ymy;
					SelSoundReady = false;
					SelSoundDone = false;
				}
			}
			LockMouse = false;
		}

		if (ImNSL[MyNation] > 0 && !mini)
		{
			word Att = 0xFFFF;
			if (MTMASK || CanAttBLD || CanCapt)
			{
				Att = AdvancedGetNewEnemy( xmx, yreal, xmx, ( yreal >> 1 ) - GetHeight( xmx, yreal ), NatRefTBL[MyNation] );
				if (Att != 0xFFFF && !GoAndAttackMode)
				{
					OneObject* AOB = Group[Att];
					if (AOB)
					{
						if (AOB->newMons->CanBeKilledInside && AOB->NInside&&CanShoot)
						{
							curptr = 1;
						}
						if (AOB->newMons->MathMask&MTMASK)
						{
							curptr = 1;
						}
						else
						{
							if (AOB->NewBuilding || AOB->Wall || AOB->LockType)
							{
								if (CanAttBLD)
								{
									curptr = 1;
								}
							}
						}
						if (AOB->newMons->Capture && CanCapt&&TestCapture( AOB ) != -1)
						{
							curptr = 2;
						}
						if (GetKeyState( VK_CONTROL ) & 0x8000)
						{
							if (AOB->newMons->CanBeCapturedWhenFree)
							{
								curptr = 8;
							}
							else
							{
								curptr = 8;
							}
						}
					}
				}
			}

			word Friend = AdvancedGetNewFriend( xmx, yreal, xmx, ( yreal >> 1 ) - GetHeight( xmx, yreal ), MyNation );
			if (SETDEST)
			{
				CmdSetDst( MyNation, xmx << 4, yreal << 4 );
			}
			if (DOATTGR)
			{
				CmdAttackGround( MyNation, xmx << 4, yreal << 4 );
			}
			if (Friend != 0xFFFF)
			{
				OneObject* FRI = Group[Friend];
				if (FRI&&FRI->newMons->Transport&&CheckTransportEntering( FRI ) && CanEnter)
				{
					curptr = EnterTrPtr;
				}
			}
			byte rk = 0xFF;
			if (CanWork)
			{
				if (Friend != 0xFFFF)
				{
					OneObject* FR = Group[Friend];
					if (FR && ( FR->NewBuilding ) && !FR->Sdoxlo)
					{
						if (FR->Ready&&FR->Ref.General->MoreCharacter->MaxInside + FR->AddInside > FR->NInside)
						{
							curptr = 4;
						}
						if (FR->Life < FR->Ref.General->MoreCharacter->Life)
						{
							curptr = 3;
						}
					}
				}
				if (CheckWallClick( xmx, yreal ))
				{
					curptr = 3;
				}
				rk = DetermineResource( xmx, yreal );
				if (!curptr)
				{
					if (rk == TreeID)curptr = 6;
					if (rk == StoneID)curptr = 5;
					if (rk == FoodID)curptr = 7;
				}
			}
			if (Rpressed)
			{
				DoSelSound( 1 );
				Rpressed = false;
				UnPress();
				if (GetKeyState( VK_CONTROL ) & 0x8000)
				{
					CmdSetDst( MyNation, xmx << 4, yreal << 4 );
					SetDestMode = false;
				}
				if (!StrelMode)
				{
					StrelMode = true;
					SStartX = xmx;
					SStartY = yreal;
				}
			}
			else
			{
				if (StrelMode && !realRpressed)
				{
					//perform action
					bool AnimCurs = 1;
					StrelMode = false;
					int rx = xmx - SStartX;
					int ry = yreal - SStartY;
					int Nr = int( sqrt( rx*rx + ry*ry ) );
					short DIRECT = 512;
					if (Nr > 30)
					{
						DIRECT = GetDir( rx, ry );
						xmx = SStartX;
						yreal = SStartY;
					}
					xmx = SStartX;
					yreal = SStartY;
					if (rk < 0xFE && ( curptr == 5 || curptr == 6 || curptr == 7 ) && Nr <= 30)
					{
						CmdTakeRes( MyNation, xmx, yreal, rk );
					}
					if (Friend != 0xFFFF && curptr != 5 && curptr != 6 && curptr != 7 && Nr <= 30)
					{
						OneObject* OB = Group[Friend];
						if (OB&&OB->NewBuilding && !OB->Sdoxlo)
						{
							CmdBuildObj( MyNation, Friend );
							CmdGoToMine( MyNation, Friend );
							AddPulse( Friend, 0x9A );
						};
					};
					DetermineWallClick( xmx, yreal );
					if (Friend != 0xFFFF && Nr <= 30)
					{
						OneObject* FOBJ = Group[Friend];
						if (FOBJ)
						{
							if (FOBJ->newMons->Transport)
							{
								CmdGoToTransport( MyNation, Friend );
								AddPulse( Friend, 0x9A );
								goto PrInfo;;
							};
						};
					};

					if (Att != 0xFFFF && Nr <= 30)
					{
						OneObject* AOBJ = Group[Att];
						if (int( AOBJ ))
						{
							if (!( AOBJ->NMask&MyMask ))
							{
								if (GetKeyState( VK_CONTROL ) & 0x8000)
								{
									if (AOBJ->newMons->CanBeCapturedWhenFree)
									{
										CmdGoToMine( MyNation, Att );
									};
								}
								else
								{
									CmdAttackObj( MyNation, Att, DIRECT );
									CmdSendToXY( MyNation, xmx << 4, yreal << 4, DIRECT );
								};
								AddPulse( Att, 0xC9 );
							}
							else CmdSendToXY( MyNation, xmx << 4, yreal << 4, DIRECT );
						}
						else CmdSendToXY( MyNation, xmx << 4, yreal << 4, DIRECT );
					}
					else CmdSendToXY( MyNation, xmx << 4, yreal << 4, DIRECT );
				};
			};
			if (GUARDMODE)
			{
				if (DOGUARD)
				{
					CmdSetGuardState( MyNation, Friend );
					GUARDMODE = 0;
				};
				if (Friend != 0xFFFF)
				{
					curptr = GuardPtr2;
				}
				else
				{
					curptr = GuardPtr1;
				};
			};
			if (PATROLMODE)
			{
				curptr = PatrolPtr;
			};
			if (DOPATROL)
			{
				CmdPatrol( MyNation, xmx << 4, yreal << 4 );
			};
		}
		else
		{
			if (DOPATROL)
			{
				CmdPatrol( MyNation, xmx << 4, yreal << 4 );
			}
			else
				if (Rpressed&&ImNSL[MyNation] > 0)
				{
					CmdSendToXY( MyNation, xmx << 4, yreal << 4, 512 );
					Rpressed = false;
					UnPress();
				};
			if (!realRpressed)StrelMode = false;
		};
	};
	if (StrelMode)
	{
		int rx = xmx - SStartX;
		int ry = yreal - SStartY;
		int Nr = int( sqrt( rx*rx + ry*ry ) );
		if (Nr > 30)
		{
			int ux = rx * 20 / Nr;
			int uy = ry * 20 / Nr;
			int uxt = uy;
			int uyt = -ux;
			int dx = mapx << 5;
			int dy = mapy << 4;
			int H = GetHeight( SStartX, SStartY );
			int x0 = SStartX - dx;
			int y0 = ( SStartY >> 1 ) - dy - H;
			DrawLine( x0 - uxt, y0 - ( uyt >> 1 ), x0 + uxt, y0 + ( uyt >> 1 ), clrYello );
			DrawLine( x0 + uxt, y0 + ( uyt >> 1 ), x0 + rx - 2 * ux + uxt, y0 + ( ( ry - 2 * uy + uyt ) >> 1 ), clrYello );
			DrawLine( x0 + rx - 2 * ux + uxt, y0 + ( ( ry - 2 * uy + uyt ) >> 1 ), x0 + rx - 2 * ux + 2 * uxt, y0 + ( ( ry - 2 * uy + 2 * uyt ) >> 1 ), clrYello );
			DrawLine( x0 + rx - 2 * ux + 2 * uxt, y0 + ( ( ry - 2 * uy + 2 * uyt ) >> 1 ), x0 + rx, y0 + ( ry >> 1 ), clrYello );
			DrawLine( x0 + rx, y0 + ( ry >> 1 ), x0 + rx - 2 * ux - 2 * uxt, y0 + ( ( ry - 2 * uy - 2 * uyt ) >> 1 ), clrYello );
			DrawLine( x0 + rx - 2 * ux - 2 * uxt, y0 + ( ( ry - 2 * uy - 2 * uyt ) >> 1 ), x0 + rx - 2 * ux - uxt, y0 + ( ( ry - 2 * uy - uyt ) >> 1 ), clrYello );
			DrawLine( x0 + rx - 2 * ux - uxt, y0 + ( ( ry - 2 * uy - uyt ) >> 1 ), x0 - uxt, y0 - ( uyt >> 1 ), clrYello );
		};
	};
Edgetest:;
PrInfo:;
}

extern int LastScrollTime;

//Handle scrolling via mouse and keys and jumping via minimap
//Call rate: ~40 Hz
//FUNNY: Mostly called ingame from FastScreenProcess()
//Sometimes ProcessScreen() cuts in and raises call rate up to ~190 Hz
//This results in absurd scroll rates in multiplayer games
//Bug fixed by removing function call from ProcessScreen()
void EDGETEST()
{
	int x = mouseX;
	int y = mouseY;

	if (SHOWSLIDE)
	{
		bool MoveX = false;
		bool MoveY = false;
		int OLDMX = mapx;
		int OLDMY = mapy;

		//Check arrow keys
		if (!( EnterChatMode || RESMODE ))
		{
			if (GetKeyState( VK_RIGHT ) & 0x8000)
			{
				StepX = kKeyboardScrollSpeed;
				MoveX = true;
			}

			if (GetKeyState( VK_LEFT ) & 0x8000)
			{
				StepX = -kKeyboardScrollSpeed;
				MoveX = true;
			}

			if (GetKeyState( VK_UP ) & 0x8000)
			{
				StepY = -kKeyboardScrollSpeed;
				MoveY = true;
			}

			if (GetKeyState( VK_DOWN ) & 0x8000)
			{
				StepY = kKeyboardScrollSpeed;
				MoveY = true;
			}
		}

		//Check mouse scrolling (it has priority over arrow keys)
		//Left
		if (x < 6)
		{
			if (StepX > 0)
			{
				StepX = 0;
			}
			StepX--;
			if (StepX < -ScrollSpeed)
			{
				StepX = -ScrollSpeed;
			}
			MoveX = true;
		}
		//Up
		if (y < 6)
		{
			if (StepY > 0)
			{
				StepY = 0;
			}
			StepY--;
			if (StepY < -ScrollSpeed)
			{
				StepY = -ScrollSpeed;
			}
			MoveY = true;
		}
		//Right
		if (x > RealLx - 6)
		{
			if (StepX < 0)
			{
				StepX = 0;
			}
			StepX++;
			if (StepX > ScrollSpeed)
			{
				StepX = ScrollSpeed;
			}
			MoveX = true;
		}
		//Down
		if (y > RealLy - 6)
		{
			if (StepY < 0)
			{
				StepY = 0;
			}
			StepY++;
			if (StepY > ScrollSpeed)
			{
				StepY = ScrollSpeed;
			}
			MoveY = true;
		}

		//Sanity checks?
		if (StepX && !MoveX)
		{
			if (StepX > 0)
			{
				StepX--;
			}
			if (StepX < 0)
			{
				StepX++;
			}
		}
		if (StepY && !MoveY)
		{
			if (StepY > 0)
			{
				StepY--;
			}
			if (StepY < 0)
			{
				StepY++;
			}
		}

		//Adjust view window position
		if (StepX < 0)
		{
			mapx -= -StepX / 2;
		}
		else
		{
			mapx += StepX / 2;
		}
		if (StepY < 0)
		{
			mapy -= -StepY / 2;
		}
		else
		{
			mapy += StepY / 2;
		}

		//Stop at map borders
		if (mapx <= 0)
		{
			mapx = 1;
		}
		if (mapy <= 0)
		{
			mapy = 1;
		}
		if (mapx + smaplx > msx + 1)
		{
			mapx = msx - smaplx + 1;
		}
		if (mapy + smaply > msy + 1)
		{
			mapy = msy - smaply + 1;
		}

		//Jump-scroll via minimap clicking
		if (( !LockMouse ) && ( Lpressed ) && ( mouseX > minix ) && ( mouseY > miniy ) &&
			( mouseX < minix + ( msx / 2 ) ) && ( mouseY < miniy + ( msy / 2 ) ))
		{
			mapx = ( x - minix + MiniX - ( smaplx >> ( ADDSH + 1 ) ) ) << ADDSH;
			mapy = ( y - miniy + MiniY - ( smaply >> ( ADDSH + 1 ) ) ) << ADDSH;

			if (mapx <= 0)
			{
				mapx = 1;
			}
			if (mapy <= 0)
			{
				mapy = 1;
			}

			if (mapx + smaplx > msx + 1)
			{
				mapx = msx - smaplx + 1;
			}
			if (mapy + smaply > msy + 1)
			{
				mapy = msy - smaply + 1;
			}

			if (!FullMini)
			{
				Lpressed = false;
			}
		}

		if (mapx != OLDMX || mapy != OLDMY || Lpressed || Rpressed)
		{
			LastScrollTime = GetTickCount();
		}
	}

	if (GetKeyState( VK_SHIFT ) & 0x8000)
	{
		curptr = 0;
	}

	//Adjust Lpressed and Rpressed
	MFix();
}

void doooo()
{
	Group[0]->Selected = true;
	Nsel = 1;
	mapx = 20;
	mapy = 20;
}

void ReRenderMap();

void SetMiniMode()
{
	if (MiniMode)
	{
		return;
	}

	ReRenderMap();

	mapx = mapx - ( smaplx / 2 );
	mapy = mapy - ( smaply / 2 );

	if (mapx < 1)
	{
		mapx = 1;
	}

	if (mapy < 1)
	{
		mapy = 1;
	}

	smaplx = smaplx * 2;
	smaply = smaply * 2;

	MiniMode = true;
	Shifter = 4;
	Multip = 1;

	if (mapx + smaplx > msx)
	{
		mapx = msx - smaplx;
	}

	if (mapy + smaply > msy)
	{
		mapy = msy - smaply;
	}
}

void ClearMiniMode()
{
	MiniMode = false;

	ReRenderMap();

	Shifter = 5;
	Multip = 0;

	mapx += ( smaplx / 4 );
	mapy += ( smaply / 4 );
	smaplx = ( RSCRSizeX - 160 ) / 32;
	smaply = div24( RSCRSizeY );
}

int CurSelPos = 0;
void GoToCurSelPosition()
{
	if (ImNSL[MyNation])
	{
		if (CurSelPos >= ImNSL[MyNation])CurSelPos = 0;
		int MaxSteps = ImNSL[MyNation];
		OneObject* OBJ = nullptr;
		do
		{
			word MID = ImSelm[MyNation][CurSelPos];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB&&OB->Serial == ImSerN[MyNation][CurSelPos])OBJ = OB;
			};
			if (!OBJ)
			{
				CurSelPos++;
				if (CurSelPos >= ImNSL[MyNation])CurSelPos = 0;
			};
			MaxSteps--;
		} while (MaxSteps&&OBJ == nullptr);
		if (OBJ)
		{
			mapx = ( OBJ->RealX >> 9 ) - ( smaplx >> 1 );
			mapy = ( OBJ->RealY >> 9 ) - ( smaply >> 1 ) - ( OBJ->RZ >> 4 );
			if (mapx <= 0)mapx = 1;
			if (mapy <= 0)mapy = 1;
			if (mapx + smaplx > msx + 1)mapx = msx - smaplx + 1;
			if (mapy + smaply > msy + 1)mapy = msy - smaply + 1;
			int x0 = OBJ->RealX;
			int y0 = OBJ->RealY;
			OBJ = nullptr;
			MaxSteps = ImNSL[MyNation];
			do
			{
				word MID = ImSelm[MyNation][CurSelPos];
				if (MID != 0xFFFF)
				{
					OneObject* OB = Group[MID];
					if (OB&&OB->Serial == ImSerN[MyNation][CurSelPos] && Norma( OB->RealX - x0, OB->RealY - y0 ) > 512 * 16)
					{
						OBJ = OB;
					};
				};
				if (!OBJ)
				{
					CurSelPos++;
					if (CurSelPos >= ImNSL[MyNation])CurSelPos = 0;
				};
				MaxSteps--;
			} while (MaxSteps && OBJ == nullptr);
		}
	}
}

DialogsSystem INFORM( 0, 0 );
DialogsSystem UNITINF( 0, 0 );
DialogsSystem MINIMENU( 0, 0 );

extern bool ToolsHidden;
extern bool OptHidden;

void ClearMINIMENU()
{
	MINIMENU.CloseDialogs();
}

int InformMode = 0;
bool Recreate = 0;
int GlobalInfDY = 0;
int MaxGlobalInfDY = 0;
bool InfClick( SimpleDialog* SD )
{
	InformMode = SD->UserParam;
	Lpressed = false;
	VScrollBar* VS = (VScrollBar*) INFORM.DSS[0];
	VS->SPos = 0;
	GlobalInfDY = 0;
	return true;
};
char* MMITM[5] = { "UM_MENU","UM_INF1","UM_INF2","UM_MINI","UM_HIST" };
void ShowHistory();
extern int MenuType;
extern bool MakeMenu;
extern byte LockGrid;
byte ShowHSTR = 0;
extern bool TransMode;
void CreateNEWMAP();
void PrepareGameMedia( byte myid, bool );
void ShowLoading();
void RetryCQuest();
void CmdOfferVoting();
extern bool RESMODE;
extern int HISPEED;

bool IngameYesNoDialog( char* dialog_text );

//Callback function for ingame menu on top of the screen
bool MiniChoose( SimpleDialog* SD )
{
	switch (SD->UserParam)
	{
	case 0://Menu
		MenuType = 1;
		MakeMenu = 1;
		break;

	case 1://Unit info
		if (Inform != 2)
		{
			Inform = 2;
		}
		else
		{
			Inform = 0;
		}
		break;

	case 2://game info
		if (Inform != 1)
		{
			Inform = 1;
		}
		else
		{
			Inform = 0;
		}
		break;

	case 3://Mini map
		FullMini = !FullMini;
		break;

	case 4:
		LockGrid = 2 - LockGrid;
		break;

	case 5:
		TransMode = !TransMode;
		break;

	case 6://Surrender
		if (IngameYesNoDialog( GetTextByID( "CONFIRM" ) ))
		{
			LockPause = 1;
			WaitState = 1;
			ShowStat = SCENINF.StandartVictory || !SCENINF.hLib;
			if (SCENINF.hLib && !SCENINF.StandartVictory)
			{
				SCENINF.LooseGame = 1;
			}
			CmdEndGame( MyNation, 1, 116 );
		}
		break;

	case 37://Replay
		if (IngameYesNoDialog( GetTextByID( "CONFIRM" ) ))
		{
			ShowHSTR = 2;
		}
		break;

	case 38://History
		if (NATIONS[MyNation].NHistory)
		{
			ShowHSTR = 1;
		}
		break;

	case 39://show toolbox
		ToolsHidden = 0;
		break;

	case 40://show toolbox options
		OptHidden = 0;
		break;

	case 41:
		ShowHSTR = 23;
		break;

	case 32://tutorial
		TutOver = 1;
		break;

	case 33://current issue
		ShowHSTR = 3;
		break;

	case 72://resource transfer
		RESMODE = 1;
		UnPress();
		KeyPressed = 0;
		break;

	case 42:
		HISPEED = 0;
		break;

	case 43:
		HISPEED = 2;
		break;

	case 44:
		HISPEED = 50;
		break;

	case 45:
		FogMode = 0;
		break;

	case 46:
		FogMode = 1;
		break;

	case 73:
		CmdOfferVoting();
		break;
	}

	Recreate = 1;
	Lpressed = false;
	MiniActive = 0;

	return false;
}

bool IsGameActive();
extern bool VotingMode;

//Shows ingame menu on top of the screen
void CreateMiniMenu()
{
	MINIMENU.CloseDialogs();
	Recreate = 0;
	char* MMITM[32];
	int MMID[32];
	int NInf = 0;

	if (Tutorial)
	{
		MMITM[NInf] = GetTextByID( "TUTORIAL" );
		MMID[NInf] = 32;
		NInf++;
		MMITM[NInf] = GetTextByID( "UM_CURR" );
		MMID[NInf] = 33;
		NInf++;
	}

	MMITM[NInf] = GetTextByID( "UM_MENU" );
	MMID[NInf] = 0;
	NInf++;

	if (Inform == 2)
	{
		MMITM[NInf] = GetTextByID( "UM_INF2" );
	}
	else
	{
		MMITM[NInf] = GetTextByID( "UM_INF1" );
	}

	MMID[NInf] = 1;
	NInf++;

	if (Inform == 1)
	{
		MMITM[NInf] = GetTextByID( "UM_INF4" );
	}
	else
	{
		MMITM[NInf] = GetTextByID( "UM_INF3" );
	}

	MMID[NInf] = 2;
	NInf++;
	MMITM[NInf] = GetTextByID( "UM_MINI" );
	MMID[NInf] = 3;
	NInf++;

	if (LockGrid)
	{
		MMITM[NInf] = GetTextByID( "UI_LOCK2" );
	}
	else
	{
		MMITM[NInf] = GetTextByID( "UI_LOCK1" );
	}

	MMID[NInf] = 4;
	NInf++;

	if (TransMode)
	{
		MMITM[NInf] = GetTextByID( "UM_TRANS1" );
	}
	else
	{
		MMITM[NInf] = GetTextByID( "UM_TRANS2" );
	}

	MMID[NInf] = 5;
	NInf++;

	if (!( PlayGameMode || EditMapMode ))
	{
		MMITM[NInf] = GetTextByID( "UM_SURRENDER" );
		MMID[NInf] = 6;
		NInf++;
	}

	if (NPlayers > 1 && ( IsGameActive() ) && !( VotingMode || PlayGameMode ))
	{
		MMITM[NInf] = GetTextByID( "UM_NORT" );
		MMID[NInf] = 73;
		NInf++;
	}

	if (NPlayers == 1 && !( PlayGameMode || EditMapMode ))
	{
		MMITM[NInf] = GetTextByID( "UM_REPLAY" );
		MMID[NInf] = 37;
		NInf++;
	}

	if (NPlayers > 1 && !PlayGameMode)
	{
		MMITM[NInf] = GetTextByID( "SENDR_L1" );
		MMID[NInf] = 72;
		NInf++;
	}

	if (NATIONS[MyNation].NHistory)
	{
		MMITM[NInf] = GetTextByID( "UM_HIST" );
		MMID[NInf] = 38;
		NInf++;
	}

	if (ToolsHidden&&EditMapMode)
	{
		MMITM[NInf] = GetTextByID( "ED_TLSHOW" );
		MMID[NInf] = 39;
		NInf++;
	}

	if (OptHidden && ( EditMapMode || PlayGameMode == 2 || CheckFlagsNeed() ))
	{
		MMITM[NInf] = GetTextByID( "ED_TOPSHOW" );
		MMID[NInf] = 40;
		NInf++;
	}

	if (EditMapMode)
	{
		MMITM[NInf] = GetTextByID( "NEWMPHDR" );
		MMID[NInf] = 41;
		NInf++;
	}

	if (PlayGameMode)
	{//Replay options

		/*
		 //Remove speed changing from repalys
		if (HISPEED != 1)
		{
			MMITM[NInf] = GetTextByID( "REP_NORM" );
			MMID[NInf] = 42;
			NInf++;
		}

		if (HISPEED != 2)
		{
			MMITM[NInf] = GetTextByID( "REP_FAST" );
			MMID[NInf] = 43;
			NInf++;
		}

		if (HISPEED != 50)
		{
			MMITM[NInf] = GetTextByID( "REP_VERY" );
			MMID[NInf] = 44;
			NInf++;
		}
		*/

		if (FogMode)
		{
			MMITM[NInf] = GetTextByID( "REP_NOFOG" );
			MMID[NInf] = 45;
			NInf++;
		}
		else
		{
			MMITM[NInf] = GetTextByID( "REP_FOG" );
			MMID[NInf] = 46;
			NInf++;
		}
	}

	int MaxL = 0;
	for (int i = 0; i < NInf; i++)
	{
		int L = GetRLCStrWidth( MMITM[i], &WhiteFont );
		if (L > MaxL)
		{
			MaxL = L;
		}
	}
	MaxL += 24;
	MINIMENU.BaseX = ( RealLx - MaxL ) / 2;
	MINIMENU.BaseY = 38;

	int NMinLines = 6 + ( NATIONS[MyNation].NHistory != 0 );
	if (!MiniActive)
	{
		NMinLines = 0;
		if (Tutorial)
		{
			NInf = 2;
		}
	}

	SimpleDialog* VP = MINIMENU.addViewPort( 0, 0, MaxL, NInf * 24 );
	VP->Enabled = 1;

	int y0 = 0;
	for (int i = 0; i < NInf; i++)
	{
		GP_TextButton* TB = MINIMENU.addGP_TextButtonLimited( nullptr,
			0, y0, MMITM[i], BordGP, 74 + ( i % 3 ), 77 + ( i % 3 ), MaxL,
			&WhiteFont, &YellowFont );
		TB->OnUserClick = &MiniChoose;
		TB->UserParam = MMID[i];
		y0 += 24;
	}
}

int DetermineNationAI( byte Nat );
void LoadAIFromDLL( byte Nat, char* Name );
void REPLAY()
{
	bool lib = SCENINF.hLib != nullptr;
	int n = MISSLIST.CurrentMission;
	int LAND[8];
	int DIFF[8];
	int RONM[8];
	int STRS[8];
	for (int j = 0; j < 8; j++)
	{
		LAND[j] = CITY[j].LandType;
		DIFF[j] = CITY[j].Difficulty;
		RONM[j] = CITY[j].ResOnMap;
		STRS[j] = CITY[j].StartRes;
	};
	ShowLoading();
	PrepareGameMedia( 0, 0 );
	for (int j = 0; j < 7; j++)
	{
		CITY[j].LandType = LAND[j];
		CITY[j].Difficulty = DIFF[j];
		CITY[j].ResOnMap = RONM[j];
		CITY[j].StartRes = STRS[j];
	};


	PINFO[0].GameTime = 0;
	PINFO[0].VictCond = 0;

	if (lib && !( SCENINF.hLib || n == -1 ))
	{
		MISSLIST.CurrentMission = n;
		for (int i = 0; i < 7; i++)CITY[i].Difficulty = DIFF[i];
		SCENINF.Load( MISSLIST.MISS[n].DLLPath, MISSLIST.MISS[n].Description );
	}
}

void ProcessMiniMenu()
{
	if (PlayGameMode == 1)
	{
		return;
	}
	if (Recreate)
	{
		CreateMiniMenu();
	}
	bool MInZone = mouseX > ( RealLx / 2 ) - 14 && mouseX < ( RealLx / 2 ) + 14 && mouseY < 23;
	if (MInZone)
	{
		GPS.ShowGP( ( RealLx - 28 ) / 2, 1, BordGP, 82, 0 );
	}

	if (Lpressed && MInZone && !NoPress)
	{
		MiniActive = !MiniActive;
		CreateMiniMenu();
		Lpressed = 0;
		UnPress();
	}

	if (!MINIMENU.DSS[0])
	{
		CreateMiniMenu();
	}

	MouseOverMiniMenu = MINIMENU.DSS[0]->MouseOver;
	if (Lpressed && !( MInZone || MouseOverMiniMenu ))
	{
		MiniActive = 0;
	}

	if (MiniActive)
	{
		if (NoPress)
		{
			bool Lpress = Lpressed;
			Lpressed = 0;
			ShowHSTR = 0;
			MINIMENU.ProcessDialogs();
			Lpressed = Lpress;
		}
		else
		{
			ShowHSTR = 0;
			MINIMENU.ProcessDialogs();
		}
		DrawStdRect( MINIMENU.BaseX, MINIMENU.BaseY, MINIMENU.DSS[0]->x1, MINIMENU.DSS[0]->y1 );
		if (ShowHSTR == 1)
			ShowHistory();
		if (ShowHSTR == 2)
			REPLAY();
		if (ShowHSTR == 3)
			RetryCQuest();
		if (ShowHSTR == 23)
			CreateNEWMAP();
	}
	else
	{
		if (Tutorial)
		{
			if (!MINIMENU.DSS[1])
				CreateMiniMenu();
			if (NoPress)
			{
				bool Lpress = Lpressed;
				Lpressed = 0;
				ShowHSTR = 0;
				MINIMENU.ProcessDialogs();
				Lpressed = Lpress;
			}
			else
			{
				ShowHSTR = 0;
				MINIMENU.ProcessDialogs();
			}
			DrawStdRect( MINIMENU.BaseX, MINIMENU.BaseY, MINIMENU.DSS[0]->x1, MINIMENU.DSS[0]->y1 );
			if (ShowHSTR == 1)
				ShowHistory();
			if (ShowHSTR == 2)
				REPLAY();
			if (ShowHSTR == 3)
				RetryCQuest();
		}
	}
}

struct TrainInfo
{
	word Cost[6];
	int CostTo[6];
	char* Name;
	short AddValue[8];
	short AttValue[8];
};
struct SD_Strip
{
	SimpleDialog* SD;
	int Page;
};
class UnitInfo
{
public:
	int UnitType;//0-unit,1-artillery,2-ship,3-building
	char* Name;
	short CurATT[8];
	short CurSHL[8];
	TrainInfo ATTACK[6];
	int NATT;
	TrainInfo SHIELD[6];
	int NSHL;
	byte ATTMask;
	byte SHLMask;
	int NMoreUpg;
	word UPID[15];
	int Cost[6];
	short Shield;
	short GoldUpkeep;
	short IronUpkeep;
	short CoalUpkeep;
	short StoneUpkeep;
	short WoodUpkeep;
	//short FoodUpkeep;
	short BuildStages;
	byte* Picture;
	short Lx, Ly;
	word  UnitID;
	SD_Strip* SDS;
	int N_SD;
	int MaxSD;
	int Y1Line;
	char** Hints;
	int NHints;
	int MaxHints;
	char* AddHint( char* Hint );
	UnitInfo();
	~UnitInfo();
	void Close();
	void Create( int ID );
	void AddSD( SimpleDialog* SD, int Page );
};
UnitInfo::UnitInfo()
{
	memset( this, 0, sizeof UnitInfo );
};
void UnitInfo::Close()
{
	if (Picture)free( Picture );
	if (NHints)
	{
		for (int i = 0; i < NHints; i++)free( Hints[i] );
		free( Hints );
	};
	if (SDS)free( SDS );
	memset( this, 0, sizeof UnitInfo );
};
char* UnitInfo::AddHint( char* Hint )
{
	if (NHints >= MaxHints)
	{
		MaxHints += 64;
		Hints = (char**) realloc( Hints, MaxHints * 4 );
	};
	Hints[NHints] = new char[strlen( Hint ) + 1];
	strcpy( Hints[NHints], Hint );
	NHints++;
	return Hints[NHints - 1];
};
UnitInfo::~UnitInfo()
{
	Close();
};
void UnitInfo::AddSD( SimpleDialog* SD, int Page )
{
	if (N_SD >= MaxSD)
	{
		MaxSD += 32;
		SDS = (SD_Strip*) realloc( SDS, MaxSD * sizeof SD_Strip );
	};
	SDS[N_SD].SD = SD;
	SDS[N_SD].Page = Page;
	N_SD++;
};
extern char** NatsIDS;
int UI_Current = 0;
void UnitInfo::Create( int ID )
{
	if (ID >= NATIONS->NMon)return;
	UI_Current = ID;
	Close();
	//1.Picture
	char* File = nullptr;
	char ccc[80];
	//int Lx,Ly;
	GeneralObject* GO = NATIONS[0].Mon[ID];
	NewMonster* NM = GO->newMons;
	int pid = NM->PictureID;
	UnitType = NM->InfType;
	Name = NM->Message;
	switch (NM->InfType)
	{
	case 0://unit
		File = "Units.bmp";
		Lx = 104;
		Ly = 132;
		break;
	case 1://artillery
		File = "Artillery.bmp";
		Lx = 132;
		Ly = 114;
		break;
	case 2://Ship
		File = "Ships.bmp";
		Lx = 132;
		Ly = 151;
		break;
	case 3://Buildings
		sprintf( ccc, "%s_BLD.bmp", NatsIDS[GO->NatID] );
		File = ccc;
		Lx = 240;
		Ly = 153;
		break;
	};
	if (!File)return;
	ResFile F = RReset( File );
	if (F != INVALID_HANDLE_VALUE)
	{
		int Max = ( RFileSize( F ) - 0x436 ) / Lx / Ly;
		if (pid < Max)
		{
			Picture = new byte[Lx*Ly];
			RSeek( F, ( Max - pid - 1 )*Lx*Ly + 0x436 );
			for (int i = 0; i < Ly; i++)RBlockRead( F, Picture + Lx*( Ly - i - 1 ), Lx );
		};
		RClose( F );
	};
	//CurATT
	for (int i = 0; i < 4; i++)if (NM->MaxDamage[i])CurATT[NM->WeaponKind[i]] = NM->MaxDamage[i];
	//CurSHL
	for (int i = 0; i < 9; i++)if (NM->Protection[i])CurSHL[i + 1] = NM->Protection[i];
	Shield = NM->Shield;
	//ATTACK - search for upgrades
	int NUP = 0;
	Nation* NT = NATIONS;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 12 && NU->UnitGroup == nullptr&&NU->UnitType == 0 && NU->UnitValue == ID&&NU->CtgGroup == nullptr&&NU->CtgType == 1)
		{//Upgrade on attack
			if (NU->CtgGroup)for (int j = 0; j < NU->NCtg; j++)ATTACK[NUP].AddValue[NU->CtgGroup[j]] = NU->Value;
			else ATTACK[NUP].AddValue[NU->CtgValue] = NU->Value;
			memcpy( ATTACK[NUP].Cost, NU->Cost, 2 * 6 );
			if (NUP)
			{
				for (int j = 0; j < 8; j++)ATTACK[NUP].AttValue[j] = ATTACK[NUP - 1].AttValue[j] + ATTACK[NUP].AddValue[j];
				for (int j = 0; j < 6; j++)ATTACK[NUP].CostTo[j] = ATTACK[NUP - 1].CostTo[j] + NU->Cost[j];
			}
			else
			{
				for (int j = 0; j < 8; j++)ATTACK[NUP].AttValue[j] = CurATT[j] + ATTACK[NUP].AddValue[j];
				for (int j = 0; j < 6; j++)ATTACK[NUP].CostTo[j] = NU->Cost[j];
			};
			NU->NatID += 32;
			ATTACK[NUP].Name = NU->Message;
			NUP++;
		};
	};
	NATT = NUP;
	for (int j = 0; j < NATT; j++)
		for (int k = 0; k < 8; k++)if (ATTACK[j].AddValue[k])ATTMask |= 1 << k;
	//for(j=0;j<8;j++)if(CurATT[j])ATTMask|=1<<j;
	//SHIELD - search for upgrades
	NUP = 0;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 2 && NU->UnitGroup == nullptr&&NU->UnitType == 0 && NU->UnitValue == ID)
		{//Upgrade on protection
			if (NU->CtgGroup)for (int j = 0; j < NU->NCtg; j++)SHIELD[NUP].AddValue[NU->CtgGroup[j]] = NU->Value;
			else SHIELD[NUP].AddValue[NU->CtgValue] = NU->Value;
			memcpy( SHIELD[NUP].Cost, NU->Cost, 2 * 6 );
			if (NUP)
			{
				for (int j = 0; j < 8; j++)SHIELD[NUP].AttValue[j] = SHIELD[NUP - 1].AttValue[j] + SHIELD[NUP].AddValue[j];
				for (int j = 0; j < 6; j++)SHIELD[NUP].CostTo[j] = SHIELD[NUP - 1].CostTo[j] + NU->Cost[j];
			}
			else
			{
				for (int j = 0; j < 8; j++)SHIELD[NUP].AttValue[j] = CurSHL[j + 1] + SHIELD[NUP].AddValue[j];
				for (int j = 0; j < 6; j++)SHIELD[NUP].CostTo[j] = NU->Cost[j];
			};
			NU->NatID += 32;
			SHIELD[NUP].Name = NU->Message;
			NUP++;
		};
	};
	NSHL = NUP;
	for (int j = 0; j < NSHL; j++)
		for (int k = 0; k < 8; k++)if (SHIELD[j].AddValue[k])SHLMask |= 1 << k;
	//for(j=0;j<8;j++)if(CurSHL[j])SHLMask|=1<<j;
	//Other upgrades
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		bool add = false;
		if (NU->NatID < 32)
		{
			if (GO->NatID == NU->NatID)
			{
				int UCTG = NU->CtgUpgrade;
				if (( UCTG < 18 ) && UCTG != 11 && UCTG != 14 && UCTG != 15)
				{
					if (NU->UnitGroup)
					{
						for (int p = 0; p < NU->NUnits && !add; p++)if (NU->UnitGroup[p] == ID)add = true;
					}
					else if (NU->UnitValue == ID)add = true;
				};
				if (NM->Artilery && ( UCTG == 5 || UCTG == 10 ))add = false;
				if (NM->Peasant&&UCTG >= 19 && UCTG <= 21)add = true;
			};
		}
		else
		{
			NU->NatID -= 32;
		};
		if (add&&NMoreUpg < 15)
		{
			UPID[NMoreUpg] = i;
			NMoreUpg++;
		};
	};
	//cost
	memcpy( Cost, NM->NeedRes, 6 * 4 );
	//Upkeep
	if (NM->ResConsID == GoldID)GoldUpkeep = ( int( NM->ResConsumer ) * 100 ) / 80;
	if (NM->ResConsID == StoneID)StoneUpkeep = ( int( NM->ResConsumer ) * 100 ) / 800;
	if (NM->ResConsID == TreeID)WoodUpkeep = ( int( NM->ResConsumer ) * 100 ) / 800;
	if (NM->NShotRes)
	{
		for (int i = 0; i < NM->NShotRes; i++)
		{
			int r = NM->ShotRes[i + i];
			if (r == IronID)IronUpkeep = NM->ShotRes[i + i + 1];
			if (r == CoalID)CoalUpkeep = NM->ShotRes[i + i + 1];
		};
	};
	BuildStages = NM->ProduceStages;
	UnitID = ID;
};
int UI_CurPage = 0;
int UI_PrmPage[4] = { 0,0,0,3 };
int UI_Style = 0;
UnitInfo UINF;
bool UI_PageSelect( SimpleDialog* SD )
{
	UI_PrmPage[UINF.UnitType] = SD->UserParam;
	Lpressed = false;
	return true;
};
int CreateUNITINF_UNIT( int ID );
extern int NNations;

bool UI_NextBTN( SimpleDialog* SD )
{
	for (int i = 0; i < NNations; i++)
	{
		int N = NATIONS->NUnits[i];
		for (int j = 0; j < N; j++)
		{
			if (NATIONS->UnitsIDS[i][j] == UI_Current)
			{
				if (j < N - 1)
				{
					UI_Current = NATIONS->UnitsIDS[i][j + 1];
					if (UI_Current == 0xFFFF)
					{
						UI_Current = NATIONS->UnitsIDS[i][j + 2];
					}

					return true;
				}
				else
				{
					for (int k = 0; k < NNations; k++)
					{
						if (NATIONS->NUnits[( i + k + 1 ) % NNations])
						{
							UI_Current = NATIONS->UnitsIDS[( i + k + 1 ) % NNations][0];
							if (UI_Current == 0xFFFF)
							{
								UI_Current = NATIONS->UnitsIDS[( i + k + 1 ) % NNations][1];
							}

							return true;
						}
					}
				}
			}
		}
	}

	Lpressed = false;
	return true;
}

bool UI_PrevBTN( SimpleDialog* SD )
{
	for (int i = 0; i < NNations; i++)
	{
		int N = NATIONS->NUnits[i];

		for (int j = 0; j < N; j++)
		{
			if (NATIONS->UnitsIDS[i][j] == UI_Current)
			{

				if (j > 1)
				{
					UI_Current = NATIONS->UnitsIDS[i][j - 1];

					if (UI_Current == 0xFFFF)
					{
						UI_Current = NATIONS->UnitsIDS[i][j - 2];
					}

					return true;
				}
				else
				{
					for (int k = 0; k < NNations; k++)
					{
						int id = ( i - k - 1 + NNations + NNations ) % NNations;
						if (NATIONS->NUnits[id])
						{
							UI_Current = NATIONS->UnitsIDS[id][NATIONS->NUnits[id] - 1];

							return true;
						}
					}
				}
			}
		}
	}

	Lpressed = false;

	return true;
}

void ClearUINF()
{
	UINF.Close();
};
extern short WeaponIcn[32];
void GetCostString( word* Cost, char* str )
{
	int zp = 0;
	for (int i = 0; i < 6; i++)
	{
		if (Cost[i])
		{
			if (zp)sprintf( str, ", %s: %d", RDS[i].Name, Cost[i] );
			else sprintf( str, "%s: %d", RDS[i].Name, Cost[i] );
			str += strlen( str );
			zp++;
		};
	};
};
void GetCostString( int* Cost, char* str )
{
	int zp = 0;
	for (int i = 0; i < 6; i++)
	{
		if (Cost[i])
		{
			if (zp)sprintf( str, ", %s: %d", RDS[i].Name, Cost[i] );
			else sprintf( str, "%s: %d", RDS[i].Name, Cost[i] );
			str += strlen( str );
			zp++;
		};
	};
};
int CreateUNITINF_UNIT( int ID )
{
	if (!NATIONS->NMon)return 0;
	int DY = 16 + UNI_LINEDLY1;
	int DDY = UNI_LINEDY1;

	UNITINF.CloseDialogs();
	int DD1 = 75;
	int D = 18;
	int x0 = RealLx - InfDX;
	int y0 = InfDY;
	UINF.Create( ID );
	UNITINF.BaseX = x0 - 1;
	UNITINF.BaseY = y0 + 31;
	int dx = 0;
	int dy = 0;
	if (UINF.UnitType == 3)dx = 3;
	//if(UINF.UnitType==1)dy=-5;
	if (UINF.Picture)UNITINF.addBPXView( nullptr, dx, dy, UINF.Lx, UINF.Ly, 1, 1, 1, UINF.Picture, nullptr );
	GPPicture* Panel = nullptr;
	GP_TextButton* General = nullptr;
	GP_TextButton* Description = nullptr;
	GP_TextButton* UnitUpgrades = nullptr;
	GP_TextButton* Next = nullptr;
	GP_TextButton* Previous = nullptr;
	GP_TextButton* PriceUpg = nullptr;
	RLCFont* FN1 = &SmallWhiteFont;
	RLCFont* FN2 = &SmallYellowFont;
	RLCFont* FN3 = &SmallWhiteFont;
	RLCFont* FN4 = &SmallYellowFont;
	RLCFont* FN5 = &fn10;
	UNITINF.addTextButton( nullptr, 4, -24, UINF.Name, &WhiteFont, &WhiteFont, &WhiteFont, 0 );
	UNITINF.HintX = -1;
	UNITINF.HintY = 280;
	UNITINF.HintFont = &SmallBlackFont1;
	int dxx = 0;
	int dyy = 0;
	switch (UINF.UnitType)
	{
	case 0://Unit
		dxx = 2;
		dyy = 1 + UNI_LINEDY2;
		Panel = UNITINF.addGPPicture( nullptr, 0, 0, BordGP, 19 );

		UNITINF.addGPPicture( nullptr, 115 + dxx, 11 + dyy, BordGP, 54 );
		General = UNITINF.addGP_TextButton( nullptr, 115 + dxx, 11 + dyy, BTN_General, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 115 + dxx, 33 + dyy, BordGP, 54 );
		Description = UNITINF.addGP_TextButton( nullptr, 115 + dxx, 33 + dyy, BTN_Descr, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 115 + dxx, 55 + dyy, BordGP, 54 );
		UnitUpgrades = UNITINF.addGP_TextButton( nullptr, 115 + dxx, 55 + dyy, BTN_Upgr, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 115 + dxx, 77 + dyy, BordGP, 54 );
		Next = UNITINF.addGP_TextButton( nullptr, 115 + dxx, 77 + dyy, BTN_Next, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 115 + dxx, 99 + dyy, BordGP, 54 );
		Previous = UNITINF.addGP_TextButton( nullptr, 115 + dxx, 99 + dyy, BTN_Prev, BordGP, 55, FN1, FN2 );
		break;
	case 1://Artillery
		dxx = 3;
		dyy = -2 + UNI_LINEDY2;
		Panel = UNITINF.addGPPicture( nullptr, 0, 0, BordGP, 21 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 5 + dyy, BordGP, 56 );
		General = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 5 + dyy, BTN_General, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 27 + dyy, BordGP, 56 );
		Description = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 27 + dyy, BTN_Descr, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 49 + dyy, BordGP, 56 );
		UnitUpgrades = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 49 + dyy, BTN_Upgr, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 71 + dyy, BordGP, 56 );
		Next = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 71 + dyy, BTN_Next, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 93 + dyy, BordGP, 56 );
		Previous = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 93 + dyy, BTN_Prev, BordGP, 57, FN1, FN2 );
		break;
	case 2://Ship
		dxx = 3;
		dyy = 16 + UNI_LINEDY2;
		Panel = UNITINF.addGPPicture( nullptr, 0, 0, BordGP, 20 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 4 + dyy, BordGP, 56 );
		General = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 4 + dyy, BTN_General, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 26 + dyy, BordGP, 56 );
		Description = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 26 + dyy, BTN_Descr, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 48 + dyy, BordGP, 56 );
		UnitUpgrades = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 48 + dyy, BTN_Upgr, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 70 + dyy, BordGP, 56 );
		Next = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 70 + dyy, BTN_Next, BordGP, 57, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 137 + dxx, 93 + dyy, BordGP, 56 );
		Previous = UNITINF.addGP_TextButton( nullptr, 137 + dxx, 93 + dyy, BTN_Prev, BordGP, 57, FN1, FN2 );
		break;
	case 3://building
		Panel = UNITINF.addGPPicture( nullptr, 0, 0, BordGP, 4 );

		UNITINF.addGPPicture( nullptr, 5, 159, BordGP, 54 );
		Description = UNITINF.addGP_TextButton( nullptr, 5, 159 + UNI_LINEDY2, BTN_Descr, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 124, 159, BordGP, 54 );
		PriceUpg = UNITINF.addGP_TextButton( nullptr, 124, 159 + UNI_LINEDY2, BTN_PriceUpg, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 124, 182, BordGP, 54 );
		Next = UNITINF.addGP_TextButton( nullptr, 124, 182 + UNI_LINEDY2, BTN_Next, BordGP, 55, FN1, FN2 );

		UNITINF.addGPPicture( nullptr, 5, 182, BordGP, 54 );
		Previous = UNITINF.addGP_TextButton( nullptr, 5, 182 + UNI_LINEDY2, BTN_Prev, BordGP, 55, FN1, FN2 );
		break;
	};
	if (General)
	{
		General->OnClick = &UI_PageSelect;
		General->UserParam = 0;
	};
	if (Description)
	{
		Description->OnClick = &UI_PageSelect;
		Description->UserParam = 1;
	};
	if (UnitUpgrades)
	{
		UnitUpgrades->OnClick = &UI_PageSelect;
		UnitUpgrades->UserParam = 2;
	};
	if (PriceUpg)
	{
		PriceUpg->OnClick = &UI_PageSelect;
		PriceUpg->UserParam = 3;
	};
	if (Next)
	{
		Next->OnClick = &UI_NextBTN;
	};
	if (Previous)
	{
		Previous->OnClick = &UI_PrevBTN;
	};
	UINF.Y1Line = y0 + GPS.GetGPHeight( BordGP, Panel->SpriteID ) + 35;
	int yy = GPS.GetGPHeight( BordGP, Panel->SpriteID ) + 16;
	int xx = 13;
	SimpleDialog* SD;
	char cc[1000];
	int yy0 = yy;
	UNITINF.addClipper( 0, UINF.Y1Line + 2, RealLx - 1, InfY1 );
	int YMAX = InfY1 - UNITINF.BaseY;
	if (General)
	{
		//creating "General" page, price and upkeep information 
		SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_Price" ), FN3, FN3, FN3, 0 );
		UINF.AddSD( SD, 0 );
		yy += DY;
		for (int i = 0; i < 6; i++)
		{
			SD = UNITINF.addGPPicture( nullptr, xx + 3 + 35 * i, yy, BordGP, 37 + i );
			UINF.AddSD( SD, 0 );
		};
		yy += DY + 10;
		SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 35 );
		UINF.AddSD( SD, 0 );
		for (int i = 0; i < 6; i++)if (UINF.Cost[i])
		{
			sprintf( cc, "%d", UINF.Cost[i] );
			SD = UNITINF.addTextButton( nullptr, xx + 17 + 34 * i, yy + 3, cc, FN5, FN5, FN5, 1 );
			UINF.AddSD( SD, 0 );
		};
		yy += 22;
		if (UINF.GoldUpkeep || UINF.IronUpkeep || UINF.CoalUpkeep || UINF.StoneUpkeep || UINF.WoodUpkeep)
		{
			SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_Upkeep" ), FN3, FN3, FN3, 0 );
			UINF.AddSD( SD, 0 );
			yy += DY;
			if (UINF.GoldUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UGold" ), UINF.GoldUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 0 );
				yy += DY;
			};
			if (UINF.WoodUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UWood" ), UINF.WoodUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 0 );
				yy += DY;
			};
			if (UINF.StoneUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UStone" ), UINF.StoneUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 0 );
				yy += DY;
			};
			if (UINF.IronUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UIron" ), UINF.IronUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 0 );
				yy += DY;
			};
			if (UINF.CoalUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UCoal" ), UINF.CoalUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 0 );
				yy += DY;
			};
		};
		sprintf( cc, GetTextByID( "UI_STAGE" ), UINF.BuildStages );
		SD = UNITINF.addTextButton( nullptr, xx, yy, cc, FN3, FN3, FN3, 0 );
		UINF.AddSD( SD, 0 );
		yy += DY;
		NewMonster* NM = NATIONS->Mon[UINF.UnitID]->newMons;
		if (NM->Capture)
		{
			if (NM->Building)
			{
				SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_BCAP" ), FN3, FN3, FN3, 0 );
			}
			else
			{
				SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_CAPT" ), FN3, FN3, FN3, 0 );
			};
			yy += DY;
			UINF.AddSD( SD, 0 );
		};
		sprintf( cc, GetTextByID( "UI_ACCNT" ), float( NM->Ves ) / 100 );
		SD = UNITINF.addTextButton( nullptr, xx, yy, cc, FN3, FN3, FN3, 0 );
		UINF.AddSD( SD, 0 );
		yy += DY;
		if (NM->Capture)
		{
			sprintf( cc, "%s %s", GetTextByID( "UI_ACHINT" ), GetTextByID( "UI_ACHCAP" ) );
			SD->Hint = UINF.AddHint( cc );
		}
		else SD->Hint = GetTextByID( "UI_ACHINT" );
	};
	if (PriceUpg)
	{
		SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_Price" ), FN3, FN3, FN3, 0 );
		UINF.AddSD( SD, 3 );
		yy += DY;
		for (int i = 0; i < 6; i++)
		{
			SD = UNITINF.addGPPicture( nullptr, xx + 3 + 35 * i, yy, BordGP, 37 + i );
			UINF.AddSD( SD, 3 );
		};
		yy += 24;
		SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 35 );
		UINF.AddSD( SD, 3 );
		for (int i = 0; i < 6; i++)if (UINF.Cost[i])
		{
			sprintf( cc, "%d", UINF.Cost[i] );
			SD = UNITINF.addTextButton( nullptr, xx + 17 + 34 * i, yy + 3, cc, FN5, FN5, FN5, 1 );
			UINF.AddSD( SD, 3 );
		};
		yy += 22;
		if (UINF.GoldUpkeep || UINF.IronUpkeep || UINF.CoalUpkeep || UINF.StoneUpkeep || UINF.WoodUpkeep)
		{
			SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_Upkeep" ), FN3, FN3, FN3, 0 );
			UINF.AddSD( SD, 3 );
			yy += DY;
			if (UINF.GoldUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UGold" ), UINF.GoldUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 3 );
				yy += DY;
			};
			if (UINF.WoodUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UWood" ), UINF.WoodUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 3 );
				yy += DY;
			};
			if (UINF.StoneUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UStone" ), UINF.StoneUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 3 );
				yy += DY;
			};
			if (UINF.IronUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UIron" ), UINF.IronUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 3 );
				yy += DY;
			};
			if (UINF.CoalUpkeep)
			{
				sprintf( cc, GetTextByID( "UI_UCoal" ), UINF.CoalUpkeep );
				SD = UNITINF.addTextButton( nullptr, xx + 3, yy, cc, FN4, FN4, FN4, 0 );
				UINF.AddSD( SD, 3 );
				yy += DY;
			};
		};
		sprintf( cc, GetTextByID( "UI_STAGE" ), UINF.BuildStages );
		SD = UNITINF.addTextButton( nullptr, xx, yy, cc, FN3, FN3, FN3, 0 );
		UINF.AddSD( SD, 3 );
		yy += DY;
		NewMonster* NM = NATIONS->Mon[UINF.UnitID]->newMons;
		if (NM->Capture)
		{
			if (NM->Building)
			{
				SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_BCAP" ), FN3, FN3, FN3, 0 );
			}
			else
			{
				SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_CAPT" ), FN3, FN3, FN3, 0 );
			};
			yy += DY;
			UINF.AddSD( SD, 3 );
		};
		sprintf( cc, GetTextByID( "UI_ACCNT" ), float( NM->Ves ) / 100 );
		SD = UNITINF.addTextButton( nullptr, xx, yy, cc, FN3, FN3, FN3, 0 );
		UINF.AddSD( SD, 3 );
		yy += DY;
		if (NM->Capture)
		{
			sprintf( cc, "%s %s", GetTextByID( "UI_ACHINT" ), GetTextByID( "UI_ACHCAP" ) );
			SD->Hint = UINF.AddHint( cc );
		}
		else SD->Hint = GetTextByID( "UI_ACHINT" );
		if (UINF.NMoreUpg)
		{
			for (int i = 0; i < UINF.NMoreUpg; i++)
			{
				int xs = xx + ( i % 5 ) * 44 - 5;
				int ys = yy + ( i / 5 ) * 44;
				NewUpgrade* NU = NATIONS->UPGRADE[UINF.UPID[i]];
				if (NU->IconFileID != 0xFFFF)
				{
					SD = UNITINF.addGPPicture( nullptr, xs - 1, ys - 1, 0, NU->IconSpriteID );
					UINF.AddSD( SD, 3 );
				};
				SD = UNITINF.addGPPicture( nullptr, xs, ys, BordGP, 34 );
				char* hnt = cc;
				sprintf( hnt, "%s/", NU->Message );
				hnt += strlen( hnt );
				strcpy( hnt, GetTextByID( "UI_Cost" ) );
				hnt += strlen( hnt );
				GetCostString( NU->Cost, hnt );
				SD->Hint = UINF.AddHint( cc );
				//SD->Hint=NU->Message;
				UINF.AddSD( SD, 3 );
			};
			yy += ( UINF.NMoreUpg / 5 ) * 44;
			if (UINF.NMoreUpg % 5)yy += 44;
		};
		yy += 4;
		int dd = yy - YMAX;
		if (dd > 0)
		{
			UNITINF.addClipper( 0, 0, RealLx - 1, RealLy - 1 );
			VScrollBar* GLSB = UNITINF.addNewGP_VScrollBar( nullptr, xx + 220, yy0 - 11, InfY1 - UINF.Y1Line, dd, 0, 3, 0 );
			GLSB->OnesDy = 10;
			UNITINF.addClipper( 0, UINF.Y1Line + 2, RealLx - 1, InfY1 );
			GLSB->ScrDy = 100;
			for (int i = 0; i < UINF.N_SD; i++)if (UINF.SDS[i].Page == 3)UINF.SDS[i].SD->ParentSB = GLSB;
			UINF.AddSD( GLSB, 3 );
		};
	};
	yy = yy0;
	if (UnitUpgrades)
	{
		if (UINF.NMoreUpg)
		{
			for (int i = 0; i < UINF.NMoreUpg; i++)
			{
				int xs = xx + ( i % 5 ) * 44 - 5;
				int ys = yy + ( i / 5 ) * 44;
				NewUpgrade* NU = NATIONS->UPGRADE[UINF.UPID[i]];
				if (NU->IconFileID != 0xFFFF)
				{
					SD = UNITINF.addGPPicture( nullptr, xs - 1, ys - 1, 0, NU->IconSpriteID );
					UINF.AddSD( SD, 2 );
				};
				SD = UNITINF.addGPPicture( nullptr, xs, ys, BordGP, 34 );
				char* hnt = cc;
				sprintf( hnt, "%s/", NU->Message );
				hnt += strlen( hnt );
				strcpy( hnt, GetTextByID( "UI_Cost" ) );
				hnt += strlen( hnt );
				GetCostString( NU->Cost, hnt );
				SD->Hint = UINF.AddHint( cc );
				UINF.AddSD( SD, 2 );
			};
			yy += ( UINF.NMoreUpg / 5 ) * 44;
			if (UINF.NMoreUpg % 5)yy += 44;
		};
		if (UINF.NATT)
		{
			int NAL = 0;
			for (int j = 0; j < 8; j++)if (UINF.ATTMask&( 1 << j ))NAL++;
			if (NAL)
			{
				//yy+=3;
				SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_AttUpg" ), FN3, FN3, FN3, 0 );
				UINF.AddSD( SD, 2 );
				yy += DY;
				SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 33 );
				UINF.AddSD( SD, 2 );
				yy += DY;
				int CAL = 0;
				for (int j = 0; j < 8; j++)if (UINF.ATTMask&( 1 << j ))
				{
					if (CAL < NAL - 1)SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 32 );
					else SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 31 );
					UINF.AddSD( SD, 2 );
					SD = UNITINF.addGPPicture( nullptr, xx + 1, yy, 4, WeaponIcn[j] );
					UINF.AddSD( SD, 2 );
					sprintf( cc, "UI_SATT%d", j );
					SD = UNITINF.addViewPort( xx + 14, yy, 35, 14 );
					UINF.AddSD( SD, 2 );
					SD->Hint = GetTextByID( cc );
					sprintf( cc, "UI_ATT%d", j );
					SD = UNITINF.addViewPort( xx, yy, 14, 14 );
					UINF.AddSD( SD, 2 );
					SD->Hint = GetTextByID( cc );
					for (int k = 0; k < UINF.NATT; k++)
					{
						if (UINF.ATTACK[k].AddValue[j])
						{
							sprintf( cc, "+%d", UINF.ATTACK[k].AddValue[j] );
							SD = UNITINF.addTextButton( nullptr, xx + 62 + 26 * k, yy, cc, FN5, FN5, FN5, 1 );
							UINF.AddSD( SD, 2 );
							SD = UNITINF.addViewPort( xx + 49 + 26 * k, yy, 26, 14 );
							UINF.AddSD( SD, 2 );
							char* hnt = cc;
							char cc1[32];
							sprintf( cc1, "UI_ATT%d", j );
							sprintf( cc, "%s/%s%d/%s", GetTextByID( cc1 ), GetTextByID( "UI_AttTo" ), UINF.ATTACK[k].AttValue[j], GetTextByID( "UI_Cost" ) );
							hnt += strlen( hnt );
							GetCostString( UINF.ATTACK[k].Cost, hnt );
							hnt += strlen( hnt );
							sprintf( hnt, "/%s", GetTextByID( "UI_CostTo" ) );
							hnt += strlen( hnt );
							GetCostString( UINF.ATTACK[k].CostTo, hnt );
							SD->Hint = UINF.AddHint( cc );
						};
					};
					sprintf( cc, "%d", UINF.CurATT[j] );
					SD = UNITINF.addTextButton( nullptr, xx + 31, yy, cc, FN5, FN5, FN5, 1 );
					UINF.AddSD( SD, 2 );
					yy += 14;
					CAL++;
				};
				yy += 3;
			};
		};
		if (UINF.NSHL)
		{
			int NAL = 0;
			for (int j = 0; j < 8; j++)if (UINF.SHLMask&( 1 << j ))NAL++;
			if (NAL)
			{
				SD = UNITINF.addTextButton( nullptr, xx, yy, GetTextByID( "UI_ShlUpg" ), FN3, FN3, FN3, 0 );
				UINF.AddSD( SD, 2 );
				yy += DY;
				SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 33 );
				UINF.AddSD( SD, 2 );
				yy += DY - 1;
				int CAL = 0;
				for (int j = 0; j < 8; j++)if (UINF.SHLMask&( 1 << j ))
				{
					if (CAL < NAL - 1)SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 32 );
					else SD = UNITINF.addGPPicture( nullptr, xx, yy, BordGP, 31 );
					UINF.AddSD( SD, 2 );
					SD = UNITINF.addGPPicture( nullptr, xx + 1, yy, 4, WeaponIcn[j] + 7 );
					UINF.AddSD( SD, 2 );
					sprintf( cc, "UI_SSHL%d", j );
					SD = UNITINF.addViewPort( xx + 14, yy, 35, 14 );
					UINF.AddSD( SD, 2 );
					SD->Hint = GetTextByID( cc );
					sprintf( cc, "UI_SHL%d", j );
					SD = UNITINF.addViewPort( xx, yy, 14, 14 );
					UINF.AddSD( SD, 2 );
					SD->Hint = GetTextByID( cc );
					for (int k = 0; k < UINF.NSHL; k++)
					{
						if (UINF.SHIELD[k].AddValue[j])
						{
							sprintf( cc, "+%d", UINF.SHIELD[k].AddValue[j] );
							SD = UNITINF.addTextButton( nullptr, xx + 62 + 26 * k, yy, cc, FN5, FN5, FN5, 1 );
							UINF.AddSD( SD, 2 );
							SD = UNITINF.addViewPort( xx + 49 + 26 * k, yy, 26, 14 );
							UINF.AddSD( SD, 2 );
							char* hnt = cc;
							char cc1[32];
							sprintf( cc1, "UI_SHL%d", j );
							sprintf( cc, "%s/%s%d/%s", GetTextByID( cc1 ), GetTextByID( "UI_ShlTo" ), UINF.SHIELD[k].AttValue[j], GetTextByID( "UI_Cost" ) );
							hnt += strlen( hnt );
							GetCostString( UINF.SHIELD[k].Cost, hnt );
							hnt += strlen( hnt );
							sprintf( hnt, "/%s", GetTextByID( "UI_CostTo" ) );
							hnt += strlen( hnt );
							GetCostString( UINF.SHIELD[k].CostTo, hnt );
							SD->Hint = UINF.AddHint( cc );
						};
					};
					sprintf( cc, "%d", UINF.CurSHL[j + 1] );
					SD = UNITINF.addTextButton( nullptr, xx + 31, yy, cc, FN5, FN5, FN5, 1 );
					UINF.AddSD( SD, 2 );
					yy += 14;
					CAL++;
				};
				yy += 3;
			};
		};
		yy += 4;
		int dd = yy - YMAX;
		if (dd > 0)
		{
			UNITINF.addClipper( 0, 0, RealLx - 1, RealLy - 1 );
			VScrollBar* GLSB = UNITINF.addNewGP_VScrollBar( nullptr, xx + 220, yy0 - 11, InfY1 - UINF.Y1Line, dd, 0, 3, 0 );
			GLSB->OnesDy = 10;
			UNITINF.addClipper( 0, UINF.Y1Line + 2, RealLx - 1, InfY1 );
			GLSB->ScrDy = 100;
			for (int i = 0; i < UINF.N_SD; i++)if (UINF.SDS[i].Page == 2)UINF.SDS[i].SD->ParentSB = GLSB;
			UINF.AddSD( GLSB, 2 );
		};
	};
	yy = yy0;
	if (Description)
	{
		SD = UNITINF.addTextButton( nullptr, xx - 4, yy - 3, GetTextByID( "BTN_Descr" ), &SmallWhiteFont, &SmallWhiteFont, &SmallWhiteFont, 0 );
		UINF.AddSD( SD, 1 );
		sprintf( cc, "Description\\%s.md", NATIONS->Mon[UINF.UnitID]->newMons->MD_File );
		TextViewer* TV = UNITINF.addTextViewer( nullptr, xx - 4, yy - 3 + 16, 215, InfY1 - UINF.Y1Line - 16 - 16, cc, &SmallYellowFont );
		TV->SymSize += 2;
		TV->PageSize = ( TV->y1 - TV->y ) / TV->SymSize;
		UINF.AddSD( TV, 1 );
		VScrollBar* SB = UNITINF.addNewGP_VScrollBar( nullptr, xx + 220, yy - 11, InfY1 - UINF.Y1Line, 1, 0, 3, 0 );
		UINF.AddSD( SB, 1 );
		TV->AssignScroll( SB );

	};
	//UNITINF.addClipper(0,0,RealLx-1,RealLy-1);
	int cpg = UI_PrmPage[UINF.UnitType];
	for (int i = 0; i < UINF.N_SD; i++)UINF.SDS[i].SD->Visible = UINF.SDS[i].Page == cpg;
	return true;
};
void ShowUnitInfo()
{
	if (UINF.N_SD)
	{
		int cpg = UI_PrmPage[UINF.UnitType];
		for (int i = 0; i < UINF.N_SD; i++)UINF.SDS[i].SD->Visible = UINF.SDS[i].Page == cpg;
		if (UI_Current != UINF.UnitID)CreateUNITINF_UNIT( UI_Current );
		DrawScrollDoubleTable( RealLx - InfDX, InfDY, RealLx - 10, InfY1, UINF.Y1Line );
		//bool L=Lpressed;
		//if(!Allow)Lpressed=false;
		TempWindow TW;
		PushWindow( &TW );
		UNITINF.ProcessDialogs();
		PopWindow( &TW );
		//Lpressed=L;
	}
	else
	{
		CreateUNITINF_UNIT( 0 );
	};
};
int Renew1Time = 0;
int NatForces[8] = { 0,0,0,0,0,0,0,0 };
int NatFin[8] = { 0,0,0,0,0,0,0,0 };
int NatFlot[8] = { 0,0,0,0,0,0,0,0 };
int GetFinPower( int* Fin, int Nation );
char* GetPName( int i );
void GetSquare()
{
	memset( NatSquare, 0, sizeof NatSquare );
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB&&OB->Ready && ( !OB->Sdoxlo ) && OB->NInside >= 5 && OB->newMons->Usage == MineID)
		{
			NatSquare[OB->NNUM]++;
			if (OB->NInside >= 10)NatSquare[OB->NNUM]++;
		};
	};
	/*
	memset(NatSquare,0,4*8);
	byte nms[256];
	memset(nms,0,256);
	for(int i=0;i<MAXOBJECT;i++){
		OneObject* OB=Group[i];
		if(OB&&!OB->Sdoxlo){
			if(!OB->NewBuilding){
				byte Usage=OB->newMons->Usage;
				if(Usage==GaleraID||Usage==FregatID||Usage==LinkorID||Usage==ShebekaID||Usage==IaxtaID){
					NatFlot[OB->NNUM]++;
				}else{
					if(!OB->LockType){
						if(Usage!=PeasantID)NatForces[OB->NNUM]++;
					};
				};
			};
			int xx=OB->RealX>>14;
			int yy=OB->RealY>>14;
			if(xx>=0&&xx<16&&yy>=0&&yy<16)nms[xx+(yy<<4)]|=1<<OB->NNUM;
		};
	};
	//square
	for(i=0;i<255;i++){
		switch(nms[i]){
		case 1:
			NatSquare[0]++;
			break;
		case 2:
			NatSquare[1]++;
			break;
		case 4:
			NatSquare[2]++;
			break;
		case 8:
			NatSquare[3]++;
			break;
		case 16:
			NatSquare[4]++;
			break;
		case 32:
			NatSquare[5]++;
			break;
		case 64:
			NatSquare[6]++;
			break;
		case 128:
			NatSquare[7]++;
			break;
		};
	};
	*/
};

int ShowNationalForces( int x, int y, bool Header )
{
	int xx = x + 5;
	if (Header)
	{
		ShowString( x - 7, y - 18, NationalForces, &WhiteFont );
		return 0;
	}

	int tt = GetRealTime();

	if (tt - Renew1Time > 10000)
	{
		memset( NatForces, 0, 4 * 8 );
		memset( NatFin, 0, 4 * 8 );
		memset( NatFlot, 0, 4 * 8 );
		/*
		memset(NatSquare,0,4*8);
		byte nms[256];
		memset(nms,0,256);
		for(int i=0;i<MAXOBJECT;i++){
			OneObject* OB=Group[i];
			if(OB&&!OB->Sdoxlo){
				if(!OB->NewBuilding){
					byte Usage=OB->newMons->Usage;
					if(Usage==GaleraID||Usage==FregatID||Usage==LinkorID||Usage==ShebekaID||Usage==IaxtaID){
						NatFlot[OB->NNUM]++;
					}else{
						if(!OB->LockType){
							if(Usage!=PeasantID)NatForces[OB->NNUM]++;
						};
					};
				};
				int xx=OB->RealX>>14;
				int yy=OB->RealY>>14;
				if(xx>=0&&xx<16&&yy>=0&&yy<16)nms[xx+(yy<<4)]|=1<<OB->NNUM;
			};
		};
		//square
		for(i=0;i<255;i++){
			switch(nms[i]){
			case 1:
				NatSquare[0]++;
				break;
			case 2:
				NatSquare[1]++;
				break;
			case 4:
				NatSquare[2]++;
				break;
			case 8:
				NatSquare[3]++;
				break;
			case 16:
				NatSquare[4]++;
				break;
			case 32:
				NatSquare[5]++;
				break;
			case 64:
				NatSquare[6]++;
				break;
			case 128:
				NatSquare[7]++;
				break;
			};
		};
		*/
		//finances
		for (int i = 0; i < 8; i++)
		{
			int RRR[8];
			for (int p = 0; p > 8; p++)RRR[p] = XRESRC( i, p );
			NatFin[i] = GetFinPower( RRR, i );
		};
	};
	int y0 = y + 14;
	//int xx=x+(210-GetRLCStrWidth(NationalForces,&BigYellowFont))/2;
	//ShowString(xx,y,NationalForces,&BigYellowFont);
	if (tmtmt > 20 && PINFO[0].VictCond)
	{
		if (LastTimeStage == -1)
		{
			LastTimeStage = GetRealTime();
		}
		else
		{
			int dd = GetRealTime() - LastTimeStage;
			if (dd > 1000)
			{
				dd /= 1000;

				if (PINFO[0].GameTime > dd)
					PINFO[0].GameTime -= dd;
				else
					PINFO[0].GameTime = 0;

				GameTime += dd;

				if (PeaceTimeLeft > 1)
				{
					PeaceTimeLeft -= dd;

					if (PeaceTimeLeft < 1)
						PeaceTimeLeft = 1;
				}
				else
				{
					if (PeaceTimeLeft == 1)
						CmdEndPT();
				}
				LastTimeStage += dd * 1000;
			}
		}

		char cc1[100];
		int t = PINFO[0].GameTime;
		int s = t % 60;
		int m = ( t / 60 ) % 60;
		int h = t / 3600;

		if (s < 10 && m < 10)
		{
			sprintf( cc1, "%d:0%d:0%d", h, m, s );
		}
		else
		{
			if (s < 10 && m >= 10)
			{
				sprintf( cc1, "%d:%d:0%d", h, m, s );
			}
			else
			{
				if (s > 10 && m < 10)
				{
					sprintf( cc1, "%d:0%d:%d", h, m, s );
				}
				else
				{
					sprintf( cc1, "%d:%d:%d", h, m, s );
				}
			}
		}

		ShowString( x + 2, y0 + 2, cc1, &BigBlackFont );
		ShowString( x, y0, cc1, &BigWhiteFont );
		y0 += 30;
		int v = GetBestPlayer();
		if (v != -1)
		{
			sprintf( cc1, BESTPL, GetPName( v ) );
			ShowString( x + 2, y0 + 2, cc1, &BlackFont );
			ShowString( x, y0, cc1, &WhiteFont );
		}
		y0 += 18;
	}

	byte res[8];
	int par[8];
	int np = SortPlayers( res, par );
	for (int q = 0; q < np; q++)
	{
		int i = res[q];
		//CBar(x+2,y0,16,16,0xD0+i*4);
		char* GPN = GetPName( i );
		if (strcmp( GPN, "???" ) && GPN[0])
		{
			GPS.ShowGP( x, y0, BordGP, 90, NatRefTBL[i] );
			ShowString( x + 40, y0 + 6, GPN, &WhiteFont );
			y0 += 28;
			GPS.ShowGP( x - 11 + 8, y0, BordGP, 91, 0 );
			//if(NatSquare[i]<3)NatSquare[i]=3;
			char ccr[24];
			ShowVictInFormat( ccr, NatRefTBL[i], 2 );
			ShowString( x + 4, y0 + 3, INFO_G2, &SmallYellowFont );
			ShowString( x + 190 - 5 - GetRLCStrWidth( ccr, &SmallYellowFont ) / 2, y0 + 4, ccr, &SmallYellowFont );
			y0 += 24;
			ShowVictInFormat( ccr, NatRefTBL[i], 1 );
			ShowString( x + 4, y0 + 2, INFO_G1, &SmallYellowFont );
			ShowString( x + 190 - 5 - GetRLCStrWidth( ccr, &SmallYellowFont ) / 2, y0 + 3, ccr, &SmallYellowFont );
			y0 += 24;
		};
	};
	return y0 - y;
};
struct HashItem
{
	char* Message;
	DWORD HashKey;
	int Param1;
	int Param2;
};
class NamesHash
{
public:
	int NHash;
	int MaxHash;
	HashItem* HASH;
	void AddString( char* Str, int Parm1, int Parm2 );
	NamesHash();
	~NamesHash();
};
NamesHash::NamesHash()
{
	NHash = 0;
	MaxHash = 0;
	HASH = nullptr;
};
NamesHash::~NamesHash()
{
	if (HASH)
	{
		free( HASH );
		NHash = 0;
		MaxHash = 0;
	};
};
void NamesHash::AddString( char* Str, int Parm1, int Parm2 )
{
	for (int i = 0; i < NHash; i++)
	{
		if (!strcmp( Str, HASH[i].Message ))
		{
			HASH[i].Param1 += Parm1;
			HASH[i].Param2 += Parm2;
			return;
		};
	};
	if (NHash >= MaxHash)
	{
		MaxHash += 32;
		HASH = (HashItem*) realloc( HASH, MaxHash * sizeof HashItem );
	};
	HASH[NHash].Param1 = Parm1;
	HASH[NHash].Param2 = Parm2;
	HASH[NHash].Message = Str;
	NHash++;
};
int ShowUnitsList( int x, int y, int Lx, word* Value )
{
	Nation* NTM = NATIONS + MyNation;
	//word* KILL=NTM->NKilled;
	int DY = 15 + UNI_LINEDLY1;
	int DDY = UNI_LINEDY1;

	int N2 = NTM->NMon;
	int y0 = y + 14;
	char cc[100];
	int xx = x + 5;
	NamesHash UNITS;
	NamesHash BUILDINGS;
	GeneralObject** GOS = NTM->Mon;
	for (int i = 0; i < N2; i++)
	{
		if (Value[i])
		{
			if (GOS[i]->newMons->Building)
				BUILDINGS.AddString( GOS[i]->Message, Value[i], 0 );
			else UNITS.AddString( GOS[i]->Message, Value[i], 0 );
		};
	};

	int x0 = x + Lx;//Lx=205
	if (UNITS.NHash)
	{
		ShowString( x + 5, y0, GI_UNITS, &SmallWhiteFont );
		y0 += 17;
		Hline( x - 2, y0, x0 + 5, 70 );
		int N = UNITS.NHash;
		for (int i = 0; i < N; i++)
		{
			sprintf( cc, "%d", UNITS.HASH[i].Param1 );
			ShowString( x + 5, y0 + 1 + DDY, UNITS.HASH[i].Message, &SmallYellowFont );
			ShowString( x0 - GetRLCStrWidth( cc, &SmallYellowFont ), y0 + 1 + DDY, cc, &SmallYellowFont );
			Hline( x - 2, y0 + DY, x0 + 5, 70 );
			Vline( x0 - 45, y0, y0 + DY, 70 );
			Vline( x - 2, y0, y0 + DY, 70 );
			Vline( x0 + 5, y0, y0 + DY, 70 );
			y0 += DY - 1;
		};
		y0 += 4;
	};
	if (BUILDINGS.NHash)
	{
		ShowString( x + 5, y0, GI_BLD, &SmallWhiteFont );
		y0 += 17;
		Hline( x - 2, y0, x0 + 5, 70 );
		int N = BUILDINGS.NHash;
		for (int i = 0; i < N; i++)
		{
			sprintf( cc, "%d", BUILDINGS.HASH[i].Param1 );
			ShowString( x + 5, y0 + 1 + DDY, BUILDINGS.HASH[i].Message, &SmallYellowFont );
			ShowString( x0 - GetRLCStrWidth( cc, &SmallYellowFont ), y0 + 1 + DDY, cc, &SmallYellowFont );
			Hline( x - 2, y0 + DY, x0 + 5, 70 );
			Vline( x0 - 45, y0, y0 + DY, 70 );
			Vline( x - 2, y0, y0 + DY, 70 );
			Vline( x0 + 5, y0, y0 + DY, 70 );
			y0 += DY - 1;
		};
		y0 += 4;
	};
	return y0 - y;
};
int ShowDeathList( int x, int y, bool Header )
{
	//Nation* NTM=NATIONS+MyNation;
	//word* KILL=NTM->NKilled;
	//int N2=NTM->NMon;
	//int y0=y+14;
	//char cc[100];
	//int xx=x+5;
	if (Header)
	{
		ShowString( x - 7, y - 18, INFO_G3, &WhiteFont );
		return y;
	};
	return ShowUnitsList( x, y, 205, NATIONS[NatRefTBL[MyNation]].NKilled );
	/*
	NamesHash UNITS;
	NamesHash BUILDINGS;
	GeneralObject** GOS=NTM->Mon;
	for(int i=0;i<N2;i++){
		if(KILL[i]){
			if(GOS[i]->newMons->Building)
				BUILDINGS.AddString(GOS[i]->Message,KILL[i],0);
			else UNITS.AddString(GOS[i]->Message,KILL[i],0);
		};
	};

	int x0=x+205;
	if(UNITS.NHash){
		ShowString(x+5,y0,GI_UNITS,&SmallWhiteFont);
		y0+=17;
		Hline(x-2,y0,x0+5,70);
		int N=UNITS.NHash;
		for(i=0;i<N;i++){
			sprintf(cc,"%d",UNITS.HASH[i].Param1);
			ShowString(x+5,y0+1,UNITS.HASH[i].Message,&SmallYellowFont);
			ShowString(x0-GetRLCStrWidth(cc,&SmallYellowFont),y0+1,cc,&SmallYellowFont);
			Hline(x-2,y0+15,x0+5,70);
			Vline(x0-45,y0,y0+15,70);
			Vline(x-2,y0,y0+15,70);
			Vline(x0+5,y0,y0+15,70);
			y0+=15;
		};
		y0+=4;
	};
	if(BUILDINGS.NHash){
		ShowString(x+5,y0,GI_BLD,&SmallWhiteFont);
		y0+=17;
		Hline(x-2,y0,x0+5,70);
		int N=BUILDINGS.NHash;
		for(i=0;i<N;i++){
			sprintf(cc,"%d",BUILDINGS.HASH[i].Param1);
			ShowString(x+5,y0+1,BUILDINGS.HASH[i].Message,&SmallYellowFont);
			ShowString(x0-GetRLCStrWidth(cc,&SmallYellowFont),y0+1,cc,&SmallYellowFont);
			Hline(x-2,y0+15,x0+5,70);
			Vline(x0-45,y0,y0+15,70);
			Vline(x-2,y0,y0+15,70);
			Vline(x0+5,y0,y0+15,70);
			y0+=15;
		};
		y0+=4;
	};
	return y0-y;
	*/
};

int GetCurrentCost( byte Buy, byte Sell, int Amount );
int GetNominalCost( byte Buy, byte Sell, int Amount );
byte DECO[6] = { 0,3,2,1,4,5 };
byte EECO[6] = { 2,0,4,5,1,3 };

//Displays market information and exchange rates
int ShowEconomy( int x, int y, bool Header )
{
	int xx = x + 5;
	if (Header)
	{
		ShowString( x - 7, y - 18, Economica, &WhiteFont );
		return 0;
	};
	char cc[128];
	int y0 = y + 33 - 3;
	x -= 3;
	for (int ix = 0; ix < 6; ix++)
	{
		for (int iy = 0; iy < 6; iy++)
		{
			int xx = x + 44 + ix * 32;
			int yy = y0 + 20 + iy * 27;
			if (ix != iy)
			{
				int CCost = GetCurrentCost( EECO[ix], EECO[iy], 1000 );
				sprintf( cc, "%d", CCost );
				ShowString( xx - ( GetRLCStrWidth( cc, &SmallWhiteFont ) ) / 2, yy - 5, cc, &SmallWhiteFont );
				int NCost = GetNominalCost( EECO[ix], EECO[iy], 1000 );
				sprintf( cc, "%d", NCost );
				ShowString( xx - ( GetRLCStrWidth( cc, &SmallYellowFont ) ) / 2, yy + 5, cc, &SmallYellowFont );
				if (NCost < CCost)
				{
					Xbar( xx - 15, yy - 8, 31, 26, 0x96 );
				};
			};
			if (ix == 0)
			{
				GPS.ShowGP( xx - 35 - 13, yy - 13 + 5, BordGP, 37 + DECO[iy], 0 );
			};
			if (iy == 0)
			{
				GPS.ShowGP( xx - 13, yy - 25 - 13, BordGP, 37 + DECO[ix], 0 );
			};
		};
	};
	y0 += 175 + 5;
	char* ccx = E_Comment;
	do
	{
		int pos = 0;
		while (ccx[0] != '\\'&&ccx[0] != 0)
		{
			cc[pos] = ccx[0];
			pos++;
			ccx++;
		};
		cc[pos] = 0;
		ShowString( x + 10, y0, cc, &SmallYellowFont );
		y0 += 15;
		if (ccx[0] == '\\')ccx++;
	} while (ccx[0]);
	return 0;
}

int ShowDoxod( int x, int y, bool Header1 )
{
	int DY = 15 + UNI_LINEDLY1;
	int DDY = UNI_LINEDY1;

	int xx;
	if (Header1)
	{
		ShowString( x - 7, y - 18, INFO_G4, &WhiteFont );
		return 0;
	}

	int y0 = y + 14;
	char cc[128];
	ShowString( x, y0, INFO_R1, &SmallWhiteFont );
	y0 += DY + 1;
	int x0 = x + 208;
	Hline( x - 5, y0, x0 + 5, 70 );
	ShowString( x, y0 + 1 + DDY, RDS[CoalID].Name, &SmallYellowFont );
	sprintf( cc, "%d", NInCoal[NatRefTBL[MyNation]] );
	ShowString( x + 196 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
	y0 += DY;
	Vline( x - 5, y0, y0 - DY, 70 );
	Vline( x0 + 5, y0, y0 - DY, 70 );
	Vline( x0 - 30, y0, y0 - DY, 70 );
	Hline( x - 5, y0, x0 + 5, 70 );

	ShowString( x, y0 + 1 + DDY, RDS[IronID].Name, &SmallYellowFont );
	sprintf( cc, "%d", NInIron[NatRefTBL[MyNation]] );
	ShowString( x + 196 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
	y0 += DY;
	Vline( x - 5, y0, y0 - DY, 70 );
	Vline( x0 + 5, y0, y0 - DY, 70 );
	Vline( x0 - 30, y0, y0 - DY, 70 );
	Hline( x - 5, y0, x0 + 5, 70 );

	ShowString( x, y0 + 1 + DDY, RDS[GoldID].Name, &SmallYellowFont );
	sprintf( cc, "%d", NInGold[NatRefTBL[MyNation]] );
	ShowString( x + 196 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
	y0 += DY;
	Vline( x - 5, y0, y0 - DY, 70 );
	Vline( x0 + 5, y0, y0 - DY, 70 );
	Vline( x0 - 30, y0, y0 - DY, 70 );
	Hline( x - 5, y0, x0 + 5, 70 );
	//----------------------------------------------//
	y0 += 2;
	ShowString( x, y0, INFO_R2, &SmallWhiteFont );
	y0 += DY + 1;
	Hline( x - 5, y0, x0 + 5, 70 );
	ShowString( x, y0 + 1 + DDY, RDS[TreeID].Name, &SmallYellowFont );
	sprintf( cc, "%d", CITY[NatRefTBL[MyNation]].WoodSpeed );
	ShowString( x + 196 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
	y0 += DY;
	Vline( x - 5, y0, y0 - DY, 70 );
	Vline( x0 + 5, y0, y0 - DY, 70 );
	Vline( x0 - 30, y0, y0 - DY, 70 );
	Hline( x - 5, y0, x0 + 5, 70 );

	ShowString( x, y0 + 1 + DDY, RDS[StoneID].Name, &SmallYellowFont );
	sprintf( cc, "%d", CITY[NatRefTBL[MyNation]].StoneSpeed );
	ShowString( x + 196 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
	y0 += DY;
	Vline( x - 5, y0, y0 - DY, 70 );
	Vline( x0 + 5, y0, y0 - DY, 70 );
	Vline( x0 - 30, y0, y0 - DY, 70 );
	Hline( x - 5, y0, x0 + 5, 70 );

	ShowString( x, y0 + 1 + DDY, RDS[FoodID].Name, &SmallYellowFont );
	sprintf( cc, "%d", CITY[NatRefTBL[MyNation]].FoodSpeed );
	ShowString( x + 196 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
	y0 += DY;
	Vline( x - 5, y0, y0 - DY, 70 );
	Vline( x0 + 5, y0, y0 - DY, 70 );
	Vline( x0 - 30, y0, y0 - DY, 70 );
	Hline( x - 5, y0, x0 + 5, 70 );
	DY = 14;

	/*
		sprintf(cc,INFO_R1,NInCoal[MyNation]);
		ShowString(x+5,y0,cc,&SmallYellowFont);
		y0+=DY;
		sprintf(cc,INFO_R2,NInIron[MyNation]);
		ShowString(x+5,y0,cc,&SmallYellowFont);
		y0+=DY;
		sprintf(cc,INFO_R3,NInGold[MyNation]);
		ShowString(x+5,y0,cc,&SmallYellowFont);
		y0+=DY;
		sprintf(cc,INFO_W,CITY[MyNation].WoodSpeed);
		ShowString(x+5,y0,cc,&SmallYellowFont);
		y0+=DY;
		sprintf(cc,INFO_S,CITY[MyNation].StoneSpeed);
		ShowString(x+5,y0,cc,&SmallYellowFont);
		y0+=DY;
		sprintf(cc,INFO_F,CITY[MyNation].FoodSpeed);
		ShowString(x+5,y0,cc,&SmallYellowFont);
		y0+=DY;
		*/
	y0 += 10;
	bool Header = false;
	int N = NATIONS[NatRefTBL[MyNation]].NMon;
	word* RAM = CITY[NatRefTBL[MyNation]].ReadyAmount;
	GeneralObject** GO = NATIONS[NatRefTBL[MyNation]].Mon;
	int Itog = 0;
	NamesHash RASXODB;
	NamesHash RASXODU;
	for (int i = 0; i < N; i++)
	{
		NewMonster* NM = GO[i]->newMons;
		if (NM->ResConsumer&&NM->ResConsID == GoldID)
		{
			int Np = ( int( NM->ResConsumer )*int( RAM[i] ) ) / 80;
			if (Np)
			{
				if (NM->Building)RASXODB.AddString( NM->Message, Np, RAM[i] );
				else RASXODU.AddString( NM->Message, Np, RAM[i] );
			};
		};
	};
	if (RASXODB.NHash + RASXODU.NHash)
	{

		xx = x + ( 210 - GetRLCStrWidth( INFO_R4, &YellowFont ) ) / 2;
		ShowString( xx, y0, INFO_R4, &WhiteFont );
		y0 += 20;
		char* ccx = INFO_R7;
		do
		{
			int pos = 0;
			while (ccx[0] != '\\'&&ccx[0] != 0)
			{
				cc[pos] = ccx[0];
				pos++;
				ccx++;
			};
			cc[pos] = 0;
			ShowString( x, y0, cc, &SmallYellowFont );
			y0 += DY;
			if (ccx[0] == '\\')ccx++;
		} while (ccx[0]);
		y0 += 5;
		ShowString( x + 160 - GetRLCStrWidth( INFO_R5, &SmallYellowFont ) / 2, y0, INFO_R5, &SmallWhiteFont );
		ShowString( x + 200 - 4 - GetRLCStrWidth( INFO_R6, &SmallYellowFont ) / 2, y0, INFO_R6, &SmallWhiteFont );
		//y0+=20;

		N = RASXODU.NHash;
		if (N)
		{
			ShowString( x, y0, GI_UNITS, &SmallWhiteFont );
			y0 += DY + 1;
			Hline( x - 5, y0, x0 + 5, 70 );
			for (int i = 0; i < N; i++)
			{
				ShowString( x, y0 + 1 + DDY, RASXODU.HASH[i].Message, &SmallYellowFont );
				sprintf( cc, "%d", RASXODU.HASH[i].Param2 );
				ShowString( x + 160 + 1 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
				int Np = RASXODU.HASH[i].Param1;
				if (Np % 10)sprintf( cc, "%d.%d", Np / 10, Np % 10 );
				else sprintf( cc, "%d", Np / 10 );
				ShowString( x + 200 - 4 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
				Hline( x - 5, y0 + 15, x0 + 5, 70 );
				Vline( x0 - 45 + 15, y0, y0 + DY, 70 );
				Vline( x0 - 85 + 20, y0, y0 + DY, 70 );
				Vline( x - 5, y0, y0 + DY, 70 );
				Vline( x0 + 5, y0, y0 + DY, 70 );
				y0 += DY;
				Itog += Np;
			};
		};
		N = RASXODB.NHash;
		if (N)
		{
			if (RASXODU.NHash)
			{
				y0 += 3;
			};
			ShowString( x, y0, GI_BLD, &SmallWhiteFont );
			y0 += 16;
			Hline( x - 5, y0, x0 + 5, 70 );
			for (int i = 0; i < N; i++)
			{
				ShowString( x, y0 + 1 + DDY, RASXODB.HASH[i].Message, &SmallYellowFont );
				sprintf( cc, "%d", RASXODB.HASH[i].Param2 );
				ShowString( x + 160 + 1 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
				int Np = RASXODB.HASH[i].Param1;
				if (Np % 10)sprintf( cc, "%d.%d", Np / 10, Np % 10 );
				else sprintf( cc, "%d", Np / 10 );
				ShowString( x + 200 - 4 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 1 + DDY, cc, &SmallYellowFont );
				Hline( x - 5, y0 + 15, x0 + 5, 70 );
				Vline( x0 - 45 + 15, y0, y0 + DY, 70 );
				Vline( x0 - 85 + 20, y0, y0 + DY, 70 );
				Vline( x - 5, y0, y0 + DY, 70 );
				Vline( x0 + 5, y0, y0 + DY, 70 );
				y0 += DY;
				Itog += Np;
			};
		};
	};
	if (Itog)
	{
		y0 += 5;
		char ccx[16];
		if (Itog % 10)sprintf( ccx, "%d.%d", Itog / 10, Itog % 10 );
		else sprintf( ccx, "%d", Itog / 10 );
		//sprintf(cc,INFO_R8,ccx);
		ShowString( x + 200 - 4 - GetRLCStrWidth( ccx, &SmallYellowFont ) / 2, y0, ccx, &SmallWhiteFont );
		ShowString( x, y0, INFO_R8, &SmallWhiteFont );
		y0 += 16;
	};
	return y0 - y;
};
word LastSelID = 0xFFFF;

void ProcessInformation()
{
	ProcessMiniMenu();

	if (NoPress&&mouseX > RealLx - InfDX - 32)
	{
		return;
	}

	if (Inform == 2)
	{
		ShowUnitInfo();

		if (ImNSL[MyNation])
		{
			word MID = ImSelm[MyNation][0];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == ImSerN[MyNation][0] && LastSelID != OB->Index && !OB->Sdoxlo)
				{
					if (OB->NIndex != UINF.UnitID)
					{
						CreateUNITINF_UNIT( OB->NIndex );
						LastSelID = OB->Index;
					}
				}
			}
		}
		else
		{
			LastSelID = 0xFFFF;
		}

		return;
	}

	if (Inform == 1)
	{
		DrawScrollDoubleTable1( RealLx - 10 - 243, 32 + InfAddY, RealLx - 10, InfY1, InfY1 - 25 );
		TempWindow TMW;
		PushWindow( &TMW );
		int x0 = RealLx - 243;
		int y0 = InfDY + 25;
		int x1 = RealLx - 24;
		int y1 = InfY2 - 15;
		int ys = y0;
		int yfin = 0;

		switch (InformMode)
		{
		case 0:
			ShowNationalForces( x0, y0, 1 );
			IntersectWindows( x0 - 10, y0 + 7, x1 + 20, y1 + 6 );
			yfin = ShowNationalForces( x0, y0 - GlobalInfDY, 0 );
			break;

		case 1:
			ShowDeathList( x0, y0, 1 );
			IntersectWindows( x0 - 10, y0 + 7, x1 + 20, y1 + 6 );
			yfin = ShowDeathList( x0, y0 - GlobalInfDY, 0 );
			break;

		case 2:
			ShowEconomy( x0, y0, 1 );
			IntersectWindows( x0 - 10, y0 + 7, x1 + 20, y1 + 6 );
			yfin = ShowEconomy( x0, y0 - GlobalInfDY, 0 );
			break;

		case 3:
			ShowDoxod( x0, y0, 1 );
			IntersectWindows( x0 - 10, y0 + 7, x1 + 20, y1 + 6 );
			yfin = ShowDoxod( x0, y0 - GlobalInfDY, 0 );
			break;
		}

		MaxGlobalInfDY = yfin - y1 + y0;
		if (MaxGlobalInfDY > 0)
		{
			VScrollBar* VS = (VScrollBar*) INFORM.DSS[0];
			VS->SMaxPos = MaxGlobalInfDY;
			GlobalInfDY = VS->SPos;
			VS->Enabled = 1;
			VS->Visible = 1;
		}
		else
		{
			VScrollBar* VS = (VScrollBar*) INFORM.DSS[0];
			VS->Enabled = 0;
			VS->Visible = 0;
		}

		PopWindow( &TMW );

		bool L = Lpressed;
		INFORM.ProcessDialogs();
		Lpressed = L;
	}
}

void CreateINFORM()
{
	CreateMiniMenu();
	CreateUNITINF_UNIT( UINF.UnitID );
	INFORM.CloseDialogs();
	INFORM.addNewGP_VScrollBar( nullptr, RealLx - 22, 60 + InfAddY, 364 + InfY1 - InfAddY - 450, 1, 0, 3, 0 );
	int DD1 = 75;
	int D = 18;
	INFORM.HintFont = &SmallBlackFont1;
	INFORM.HintX = -1;
	INFORM.HintY = 350;
	INFORM.addGPPicture( nullptr, RealLx - InfDX - 1, InfY2, BordGP, 85 );
	SimpleDialog* TB = INFORM.addViewPort( RealLx - InfDX - 1 + 71, InfY2 + 2, 25, 19 );
	TB->OnClick = &InfClick;
	TB->UserParam = 0;
	TB->Hint = GetTextByID( "NationalForces" );
	TB = INFORM.addViewPort( RealLx - InfDX - 1 + 96, InfY2 + 2, 26, 19 );
	TB->OnClick = &InfClick;
	TB->UserParam = 1;
	TB->Hint = GetTextByID( "DeathList" );
	TB = INFORM.addViewPort( RealLx - InfDX - 1 + 122, InfY2 + 2, 26, 19 );
	TB->OnClick = &InfClick;
	TB->UserParam = 2;
	TB->Hint = GetTextByID( "Economica" );
	TB = INFORM.addViewPort( RealLx - InfDX - 1 + 148, InfY2 + 2, 26, 19 );
	TB->OnClick = &InfClick;
	TB->UserParam = 3;
	TB->Hint = GetTextByID( "Doxod" );
}

//--------------Resource sending--------------//
DialogsSystem RESSEND( 0, 0 );
bool RESMODE = 0;
bool PrevRESMODE = 0;
int RSN_CurPlayerTo = 0;
int RSN_CurResType = 0;
int RSN_CurResAmount = 1000;
ComboBox* RSN_USER = nullptr;
ComboBox* RSN_RESID = nullptr;
InputBox* RSN_IB = nullptr;
char RSN_RESAM[32] = "";
char* STR1 = nullptr;
char* STR2 = nullptr;
char* STR3 = nullptr;
char* STR4 = nullptr;
char* STR5 = nullptr;
char* STR6 = nullptr;
int CBB_GPFILE = 0;
#define WinLX 350
#define WinLY 120
extern char* ResNames[8];
int GAME_CLICK_RESULT = -1;
bool ON_GAME_MCLICK( SimpleDialog* SD )
{
	GAME_CLICK_RESULT = SD->UserParam;
	return 0;
}

void ClearKeyStack();


void CreateRESSEND()
{
	RESSEND.CloseDialogs();
	ClearKeyStack();
	KeyPressed = 0;
	LastKey = 0;
	int X0 = ( RealLx - WinLX ) / 2;
	int Y0 = ( RealLy - WinLY ) / 2;
	if (!STR1)
	{
		STR1 = GetTextByID( "SENDR_L1" );
		STR2 = GetTextByID( "SENDR_L2" );
		STR3 = GetTextByID( "SENDR_L3" );
		STR4 = GetTextByID( "SENDR_L4" );
		STR5 = GetTextByID( "SENDR_L5" );
		STR6 = GetTextByID( "SENDR_L6" );
	}

	int FDY = 40;
	RESSEND.addTextButton( nullptr, X0 + 16, Y0 + 5, STR1, &YellowFont, &YellowFont, &YellowFont, 0 );
	RESSEND.addTextButton( nullptr, X0 + 16, Y0 + FDY, STR2, &YellowFont, &YellowFont, &YellowFont, 0 );
	RESSEND.addTextButton( nullptr, X0 + 16, Y0 + FDY + 26, STR3, &YellowFont, &YellowFont, &YellowFont, 0 );
	RESSEND.addTextButton( nullptr, X0 + 16, Y0 + FDY + 52, STR4, &YellowFont, &YellowFont, &YellowFont, 0 );
	RSN_USER = RESSEND.addGP_ComboBoxDLX( nullptr, X0 + 150, Y0 + FDY, 180, CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );

	for (int i = 0; i < NPlayers; i++)
	{
		RSN_USER->AddLine( PINFO[i].name );
	}

	RSN_USER->CurLine = RSN_CurPlayerTo;
	if (RSN_USER->CurLine >= RSN_USER->NLines)
	{
		RSN_USER->CurLine = 0;
	}

	RSN_RESID = RESSEND.addGP_ComboBoxDLX( nullptr,
		X0 + 150, Y0 + FDY + 26, 180,
		CBB_GPFILE, 0, 9, 0, &WhiteFont, &YellowFont, nullptr );

	for (int i = 0; i < 6; i++)
	{
		RSN_RESID->AddLine( RDS[i].Name );
	}

	RSN_RESID->CurLine = RSN_CurResType;
	if (RSN_RESID->CurLine >= RSN_RESID->NLines)
	{
		RSN_RESID->CurLine = 0;
	}

	ColoredBar* CB = RESSEND.addColoredBar( X0 + 150, Y0 + FDY + 26 * 2, 180, 21, 0x4D );
	CB->Style = 1;

	RSN_IB = RESSEND.addInputBox( nullptr,
		X0 + 150, Y0 + FDY + 26 * 2,
		RSN_RESAM, 10, 120, 26, &YellowFont, &WhiteFont );

	int XB = X0 + 10;
	int YB = Y0 + WinLY + 7;

	RESSEND.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 8 );
	RESSEND.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 11 );

	GP_TextButton* OKBTN = RESSEND.addGP_TextButton( nullptr,
		XB + 9, YB + 20, STR5, CBB_GPFILE, 9, &WhiteFont, &YellowFont );
	OKBTN->OnUserClick = &ON_GAME_MCLICK;
	OKBTN->UserParam = 1;

	XB = X0 + 180;
	YB = Y0 + WinLY + 7;

	RESSEND.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 8 );
	RESSEND.addGPPicture( nullptr, XB, YB, CBB_GPFILE, 11 );

	GP_TextButton* CANCELBTN = RESSEND.addGP_TextButton( nullptr,
		XB + 9, YB + 20, STR6, CBB_GPFILE, 9, &WhiteFont, &YellowFont );
	CANCELBTN->OnUserClick = &ON_GAME_MCLICK;
	CANCELBTN->UserParam = 0;

	sprintf( RSN_RESAM, "%d", RSN_CurResAmount );
}

void DrawHdrTable( int x0, int y0, int x1, int y1 );
void CmdGiveMoney( byte SrcNI, byte DstNI, byte Res, int Amount );
int ReadKey();

//Process resource transfer dialog
void ProcessRESSEND()
{
	if (RESMODE && !PrevRESMODE)
	{
		CreateRESSEND();
	}

	PrevRESMODE = RESMODE;

	if (RESMODE)
	{
		int x0 = ( RealLx - WinLX ) / 2;
		int y0 = ( RealLy - WinLY ) / 2;
		DrawHdrTable( x0, y0, x0 + WinLX, y0 + WinLY );
		GAME_CLICK_RESULT = -1;
		int K;
		do
		{
			KeyPressed = 0;
			K = ReadKey();
			if (13 == K)
			{//Enter
				GAME_CLICK_RESULT = 1;
				K = -1;
			}
			if (21 == K)
			{//Escape
				GAME_CLICK_RESULT = 0;
				K = -1;
			}
			if (-1 != K)
			{
				KeyPressed = 1;
				LastKey = K;
				RSN_IB->OnKeyDown( RSN_IB );
			}
		} while (K != -1);

		KeyPressed = 0;
		LastKey = 0;

		RESSEND.ProcessDialogs();

		if (GAME_CLICK_RESULT == 0)
		{
			PrevRESMODE = 0;
			RESMODE = 0;
		}

		if (GAME_CLICK_RESULT == 1)
		{
			PrevRESMODE = 0;
			RESMODE = 0;
			int z = sscanf( RSN_RESAM, "%d", &RSN_CurResAmount );
			if (z == 1)
			{
				CmdGiveMoney( NatRefTBL[MyNation],
					NatRefTBL[PINFO[RSN_USER->CurLine].ColorID],
					RSN_RESID->CurLine,
					RSN_CurResAmount );
			}
		}

		RSN_CurResType = RSN_RESID->CurLine;
		RSN_CurPlayerTo = RSN_USER->CurLine;
		Lpressed = 0;
		UnPress();
	}
}
