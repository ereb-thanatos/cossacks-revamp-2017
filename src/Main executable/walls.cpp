#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mouse.h"
#include "MapDiscr.h"
#include "3DSurf.h"
#include "MapSprites.h"
#include "Walls.h"
#include "NewMon.h"
#include "GP_Draw.h"
#include "ZBuffer.h"
#include "path.h"
#include "3DBars.h"
#include <assert.h>
#include "GSound.h"
#include "TopoGraf.h"

void NLine( GFILE* f );
int mul3( int );
void ErrM( char* s );
int GetWTP1( int x, int y );
extern word TexList[128];
extern word NTextures;
WallCharacter* GetWChar( OneObject* OB );
int GetWCharID( OneObject* OB );
void AddToVisual( short uy, short x, short y, OneObject* OB, word FileID, word Sprite );

WallCluster::WallCluster()
{
	NCornPt = 0;
	CornPt = NULL;
	NCells = 0;
	Cells = NULL;
	WSys = NULL;
	LastX = 0;
	LastY = 0;
	FinalX = 0;
	FinalY = 0;
}

WallCluster::~WallCluster()
{
	if ( NCornPt )free( CornPt );
	NCornPt = 0;
	CornPt = NULL;
	NCells = 0;
	Cells = NULL;
	WSys = NULL;
	LastX = 0;
	LastY = 0;
	FinalX = 0;
	FinalY = 0;
}

void WallCluster::SetSize( int N )
{
	Cells = (WallCell*) realloc( (void*) Cells, ( NCells + 1 ) * sizeof WallCell );
	NCells = N;
}

static byte GD8[9] = { 7,6,5,0,0,4,1,2,3 };
static char DX8[8] = { 0,1,1,1,0,-1,-1,-1 };
static char DY8[8] = { -1,-1,0,1,1,1,0,-1 };

byte GetDir8( int dx, int dy )
{
	return GD8[dx + dx + dx + dy + 4];
}

void WallCluster::ConnectToPoint( short x, short y )
{
	ConnectToPoint( x, y, true );
}

void WallCluster::ConnectToPoint( short x, short y, bool Vis )
{
	if ( !NCells )
	{
		//first point 
		LastX = x;
		LastY = y;
		FinalX = x;
		FinalY = y;
		AddPoint( x, y, Vis );
		NCornPt = 1;
		CornPt = new word;
		CornPt[0] = 0;
	}
	else
	{
		if ( x == LastX && y == LastY )
		{//Same spot, do not calculate anything
			return;
		}

		int np = 0;
		short xp = x;
		short yp = y;
		short dx, dy;

		short lsx = LastX;
		short lsy = LastY;

		do
		{
			if ( 500 <= NCells )
			{//BUGFIX: Too big placement orders freeze multiplayer games
				break;
			}

			if ( NCells < 2 )
			{
				if ( !NCells )
				{//First section
					AddPoint( LastX, LastY, Vis );
				}
				dx = x - LastX;
				dy = y - LastY;
				if ( dx > 1 ) dx = 1;
				if ( dy > 1 ) dy = 1;
				if ( dx < -1 ) dx = -1;
				if ( dy < -1 ) dy = -1;
				lsx = LastX + dx;
				lsy = LastY + dy;
				AddPoint( lsx, lsy, Vis );
				np++;
			}
			else
			{
				short dx0 = Cells[NCells - 1].x - Cells[NCells - 2].x;
				short dy0 = Cells[NCells - 1].y - Cells[NCells - 2].y;
				char dir1 = GetDir8( dx0, dy0 );
				dx = x - lsx;
				dy = y - lsy;
				if ( dx > 1 )dx = 1;
				if ( dy > 1 )dy = 1;
				if ( dx < -1 )dx = -1;
				if ( dy < -1 )dy = -1;
				char dir2 = GetDir8( dx, dy );
				char ddir = ( dir2 - dir1 ) & 7;
				if ( ddir > 3 )ddir -= 8;
				char addir = abs( ddir );
				short x0 = Cells[NCells - 1].x;
				short y0 = Cells[NCells - 1].y;
				if ( addir <= 1 )
				{
					lsx = x0 + dx;
					lsy = y0 + dy;
					AddPoint( lsx, lsy, Vis );
				}
				else
				{
					if ( addir == 2 && ( dir1 == 0 || dir1 == 2 || dir1 == 4 || dir1 == 6 ) )
					{
						lsx = x0 + dx;
						lsy = y0 + dy;
						AddPoint( lsx, lsy, Vis );
					}
					else
					{
						if ( dir1 == 0 || dir1 == 2 || dir1 == 4 || dir1 == 6 )
						{
							if ( ddir < 0 )
							{
								dx = DX8[( dir1 - 2 ) & 7];
								dy = DY8[( dir1 - 2 ) & 7];
							}
							else
							{
								dx = DX8[( dir1 + 2 ) & 7];
								dy = DY8[( dir1 + 2 ) & 7];
							};
						}
						else
						{
							if ( ddir > 0 )
							{
								dx = DX8[( dir1 + 1 ) & 7];
								dy = DY8[( dir1 + 1 ) & 7];
							}
							else
							{
								dx = DX8[( dir1 - 1 ) & 7];
								dy = DY8[( dir1 - 1 ) & 7];
							};
						};
						lsx = x0 + dx;
						lsy = y0 + dy;
						AddPoint( lsx, lsy, Vis );
					}
				}
			}
		} while ( lsx != x || lsy != y );
	}
}

void WallCluster::AddPoint( short x, short y, bool Vis )
{
	SetSize( NCells + 1 );
	WallCell* WC = Cells + NCells - 1;
	WC->x = x;
	WC->y = y;
	WC->Type = Type;
	WC->Visible = Vis;
}

void WallCluster::UndoSegment()
{
	if ( !NCornPt )
	{
		return;
	}
	SetSize( CornPt[NCornPt - 1] + 1 );
	LastX = Cells[NCells - 1].x;
	LastY = Cells[NCells - 1].y;
}

//Places wall segment on map while in placement mode
//Called once on placement start
//Called twice when placing an edge (one left click)
//Called four times when finishing wall (double click)
void WallCluster::KeepSegment()
{
	CornPt = (word*) realloc( (void*) CornPt, ( NCornPt + 1 ) * 2 );
	CornPt[NCornPt] = NCells - 1;
	LastX = Cells[NCells - 1].x;
	LastY = Cells[NCells - 1].y;
	NCornPt++;
}

void WallCluster::SetPreviousSegment()
{
	if ( !NCornPt )return;
	UndoSegment();
	if ( NCornPt > 1 )
	{
		CornPt = (word*) realloc( (void*) CornPt, ( NCornPt - 1 ) << 1 );
		NCornPt--;
		UndoSegment();
	}
	else
	{

	};
};
void WallCluster::CreateSprites()
{
	if ( !NCells )
	{
		return;
	}

	if ( NCells == 1 )
	{
		Cells[0].Sprite = 8;
	}
	else
	{
		char dir0 = GetDir8( Cells[1].x - Cells[0].x, Cells[1].y - Cells[0].y );
		char dir1;
		//Cells[0].Visible=true;
		Cells[0].Sprite = dir0 & 3;
		dir0 = GetDir8( Cells[NCells - 1].x - Cells[NCells - 2].x, Cells[NCells - 1].y - Cells[NCells - 2].y );
		Cells[NCells - 1].Sprite = dir0 & 3;
		//Cells[NCells-1].Visible=true;
		for ( int i = 1; i < NCells - 1; i++ )
		{
			//Cells[i].Visible=true;
			dir0 = GetDir8( Cells[i].x - Cells[i - 1].x, Cells[i].y - Cells[i - 1].y );
			dir1 = GetDir8( Cells[i + 1].x - Cells[i].x, Cells[i + 1].y - Cells[i].y );
			char ddir = ( dir1 - dir0 ) & 7;
			if ( ddir > 3 )ddir -= 8;
			char addir = abs( ddir );
			if ( addir > 1 )Cells[i].Sprite = 12;
			else
			{
				if ( addir )
				{
					if ( ddir < 0 )Cells[i].Sprite = 4 + ( ( dir0 + 4 ) & 7 );
					else  Cells[i].Sprite = 4 + ( ( dir0 + 9 ) & 7 );
				}
				else Cells[i].Sprite = dir0 & 3;
			};
		};
		//assert(Cells[i].Sprite<14);
	};
};
//word Size
//data format:
//byte NI;
//byte Type;
//word NCells;
//word NIndex;
//word Health;
//By 1 cell:
//short x;
//short y;

int WallCluster::CreateData( word* Data, word Health )
{
	int size = 10 + ( NCells * 4 );

	Data[0] = word( size );
	( (byte*) Data )[2] = NI;
	( (byte*) Data )[3] = Type;
	Data[2] = NCells;
	Data[3] = NIndex;
	Data[4] = Health;

	for ( int i = 0; i < NCells; i++ )
	{
		Data[5 + i + i] = Cells[i].x;
		Data[6 + i + i] = Cells[i].y;
	}

	//FUNNY: When there are more than 500 wall elements to build
	//and size > ~2050, the game will crash.
	//Must not fix the bug here, or it will unsync multiplayer!
	return size;
}

extern bool CmdDone[ULIMIT];

void WallCluster::SendSelectedToWork( byte pNI, byte OrdType )
{
	int NSel = NSL[pNI];
	word* SMon = Selm[pNI];
	word* ser = SerN[pNI];
	int CurCell = 0;
	bool Opdone = false;
	for ( int i = 0; i < NSel; i++ )
	{
		if ( CurCell < NCells )
		{
			word MID = SMon[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->NNUM == pNI )
				{
					if ( OB->BuildWall( Cells[CurCell].x, Cells[CurCell].y, 16, OrdType, false ) )
					{
						Opdone = true;
						CurCell++;
					};
				};
			};
		};
	};
	if ( Opdone )
	{
		for ( int i = 0; i < NSel; i++ )
		{
			if ( CurCell < NCells )
			{
				word MID = SMon[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && OB->NNUM == pNI )
					{
						CmdDone[MID] = 0 != 0xFFFF;
					}
				}
			}
		}
	}
}

void WallCluster::CreateByData( word* Data )
{
	NI = ( (byte*) Data )[2];
	Type = ( (byte*) Data )[3];
	NCells = Data[2];
	NIndex = Data[3];
	word Health = Data[4];
	Nation* NT = &NATIONS[NI];
	NM = NT->Mon[NIndex]->newMons;
	Cells = new WallCell[NCells];
	for ( int j = 0; j < NCells; j++ )
	{
		WallCell* WC = &Cells[j];
		WC->x = Data[5 + j + j];
		WC->y = Data[6 + j + j];
		WC->Type = Type;
		WC->NI = NI;
		WC->Visible = 1;
	};
};
//----------------<   CLASS:  WallSystem   >---------------

WallSystem::WallSystem()
{
	WCL = NULL;
	NClusters = 0;
	memset( WRefs, 0, MaxLI * 4 );
};
WallSystem::~WallSystem()
{
	if ( !NClusters )return;
	for ( int i = 0; i < NClusters; i++ )
	{
		if ( WCL[i] )
		{
			if ( WCL[i]->Cells )free( WCL[i]->Cells );
			free( WCL[i] );
		};
	};
	free( WCL );
	WCL = NULL;
	NClusters = 0;
	memset( WRefs, 0, MaxLI * 4 );
}

void WallSystem::AddCluster( WallCluster* WC )
{
	if ( !WC->NCells )
	{
		return;
	}
	WCL = (WallCluster**) realloc( (void*) WCL, ( NClusters + 1 ) << 2 );
	WCL[NClusters] = new WallCluster;
	WallCluster* WCLUS = WCL[NClusters];
	WCLUS->NCells = WC->NCells;
	WCLUS->Cells = new WallCell[WC->NCells];
	WCLUS->Type = WC->Type;
	WCLUS->NM = WC->NM;
	WCLUS->NIndex = WC->NIndex;
	WCLUS->NI = WC->NI;
	memcpy( WCLUS->Cells, WC->Cells, WC->NCells * sizeof WallCell );
	WallCell* W1 = WCL[NClusters]->Cells;
	NClusters++;
	for ( int i = 0; i < WC->NCells; i++ )
	{
		if ( W1[i].Visible )
		{
			if ( !W1[i].StandOnLand( WCLUS ) )
			{
				W1[i].Visible = 0;
			}
		}
	}
}

void WallSystem::Show()
{
	for ( int i = 0; i < NClusters; i++ )
	{
		if ( WCL[i] )WCL[i]->View();
	};
};
//###----------------<   CLASS: WallCell   >----------------###
static char* Clu1 = "   ** "
"  ****"
" *****"
"***** "
" ***  "
"  *   ";

static char* Clu3 = "  *   "
" ***  "
"***** "
" *****"
"  ****"
"   ** ";

static char* Clu4 = "      "
" **** "
" **** "
" *****"
"  ****"
"   ** ";

static char* Clu5 = "   ** "
"  ****"
" *****"
" **** "
" **** "
"      ";

static char* Clu6 = "      "
"  *** "
" **** "
"***** "
" ***  "
"  *   ";

static char* Clu7 = "      "
" ***  "
" **** "
" *****"
" *****"
"   ** ";

static char* Clu8 = "  *   "
" ***  "
"***** "
" **** "
" **** "
"      ";

static char* Clu9 = "      "
" **** "
" **** "
"***** "
" ***  "
"  *   ";

static char* Clu10 = "   ** "
"  ****"
" *****"
" **** "
" **** "
"      ";

static char* Clu11 = "  *   "
" **** "
"***** "
" **** "
"  *** "
"      ";

static char* CluF = "      "
" **** "
" **** "
" **** "
" **** "
"      ";
char* GetLockData( byte id )
{
	switch ( id )
	{
	case 0:return CluF;
	case 1:return Clu1;
	case 2:return CluF;
	case 3:return Clu3;
	case 4:return Clu4;
	case 5:return Clu5;
	case 6:return Clu6;
	case 7:return Clu7;
	case 8:return Clu8;
	case 9:return Clu9;
	case 10:return Clu10;
	case 11:return Clu11;
	case 12:return CluF;
	};
	return NULL;
};
bool CheckBuildingsForWalls( int x, int y );
bool WallCell::CheckPosition()
{
	if ( !CheckBuildingsForWalls( int( x ) << 10, int( y ) << 10 ) )return false;
	int LI = GetLI( x, y );
	WallCell* WR = WRefs[LI];
	if ( WR )return false;
	//Check locking
	if ( ( x << 1 ) >= msx || ( y << 1 ) >= msy )return false;
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	char* Linf = GetLockData( Sprite );
	int Npt = 0;
	int NLock = 0;
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )
		{
			if ( CheckPt( x0, y0 ) )NLock++;
			Npt++;
		};
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
	if ( NLock > Npt - 2 )return false;
	return true;
};
int WallCell::GetLockStatus()
{
	if ( ( x << 1 ) >= msx || ( y << 1 ) >= msy )return 0;
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	char* Linf = GetLockData( Sprite & 15 );
	int LData = 0;
	int LDMask = 1;
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )
		{
			if ( CheckPt( x0, y0 ) )LData |= LDMask;
			LDMask <<= 1;
		};
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
	return LData;
};
bool GetTCStatus( int x, int y );
void WallCell::SetLockStatus()
{
	if ( ( x << 1 ) >= msx || ( y << 1 ) >= msy )return;
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	char* Linf = GetLockData( Sprite & 15 );
	int LData = Locks;
	int LDMask = 1;
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )
		{
			if ( LData&LDMask )BSetPt( x0, y0 );
			else BClrPt( x0, y0 );
			LDMask <<= 1;
		};
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
};
void WallCell::ClearLocking()
{
	//Check locking
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	char* Linf = GetLockData( Sprite & 15 );
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )BClrPt( x0, y0 );
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
	SetLockStatus();
	if ( GetTCStatus( x, y ) )DynamicalUnLockTopCell( x, y );
};
int NChar;
WallCharacter WChar[32];
WallCell** WRefs;
void SetTexturedRound( int x, int y, int rx, byte Tex );
bool WallCell::StandOnLand( WallCluster* WC )
{
	NewMonster* NM = WC->NM;
	if ( !( CheckPosition() && ApplyCost( WC->NI, WC->NIndex ) ) )
	{
		Visible = false;
		return false;
	};
	Nation* NT = &NATIONS[WC->NI];
	int ID = NT->CreateNewMonsterAt( ( int( x ) << 10 ) + 512, ( int( y ) << 10 ) + 512, WC->NIndex, true );
	if ( ID < 0 )return false;
	OneObject* OB = Group[ID];
	if ( !OB )return false;
	OB->Ready = false;
	OB->WallX = x;
	OB->WallY = y;
	OIndex = ID;
	int LI = GetLI( x, y );
	if ( LI < 0 || LI >= MaxLI )
	{
		Visible = false;
		return false;
	};
	WRefs[LI] = this;
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	Stage = 0;
	MaxStage = NM->ProduceStages;
	Health = 2;
	MaxHealth = NM->Life;
	OB->Life = Health;
	Type = WC->Type;
	NI = 0;
	WallCharacter* WCHAR = &WChar[Type];
	if ( WCHAR->NTex )
	{
		//memcpy(TexList,WCHAR->Tex,WCHAR->NTex*2);
		//NTextures=WCHAR->NTex;
		//int xx=(x<<6)+32+((int(rand()-16384)*WCHAR->TexPrec)>>14);
		//int yy=(y<<6)+32+((int(rand()-16384)*WCHAR->TexPrec)>>14);
		//SetTexturedRound(xx,(yy>>1)-GetHeight(xx,yy),WCHAR->TexRadius,0);
	};
	Locks = GetLockStatus();
	SprBase = ( WCHAR->NBuild - 1 ) * 13;
	char* Linf = GetLockData( Sprite );
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )BSetPt( x0, y0 );
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
	DynamicalLockTopCell( x, y );
	NI = WC->NI;
	return true;
};

void WallCell::CreateLocking( WallCluster* WC )
{
	NewMonster* NM = WC->NM;
	int LI = GetLI( x, y );
	if ( LI < 0 || LI >= MaxLI )
	{
		Visible = false;
		return;
	};
	WRefs[LI] = this;
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	WallCharacter* WCHAR = &WChar[Type];
	char* Linf = GetLockData( Sprite & 15 );
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )BSetPt( x0, y0 );
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
	DynamicalLockTopCell( x, y );
};
void WallCell::Landing( WallCluster* WC )
{
	NewMonster* NM = WC->NM;
	Nation* NT = &NATIONS[WC->NI];
	int ID = NT->CreateNewMonsterAt( ( int( x ) << 10 ) + 512, ( int( y ) << 10 ) + 512, WC->NIndex, true );
	if ( ID < 0 )return;;
	OneObject* OB = Group[ID];
	if ( !OB )return;
	OB->Ready = false;
	OB->WallX = x;
	OB->WallY = y;
	OIndex = ID;
	int LI = GetLI( x, y );
	if ( LI < 0 || LI >= MaxLI )
	{
		Visible = false;
		return;
	};
	WRefs[LI] = this;
	int x0 = ( x << 2 ) - 1;
	int y0 = ( y << 2 ) - 1;
	int Nx = 0;
	Locks = GetLockStatus();
	WallCharacter* WCHAR = &WChar[Type];
	char* Linf = GetLockData( Sprite & 15 );
	for ( int i = 0; i < 36; i++ )
	{
		if ( Linf[i] != ' ' )BSetPt( x0, y0 );
		x0++;
		Nx++;
		if ( Nx == 6 )
		{
			Nx = 0;
			x0 -= 6;
			y0++;
		};
	};
	DynamicalLockTopCell( x, y );
	NI = WC->NI;
};
void WallBuildStage( WallCell* WC )
{
	if ( WC->Stage < WC->MaxStage )
	{
		WC->Stage++;
		WC->Health = div( WC->MaxHealth*WC->Stage, WC->MaxStage ).quot;
		if ( WC->Health > WC->MaxHealth )WC->Health = WC->MaxHealth;
		SetLife( WC, WC->Health );
		OneObject* OB = Group[WC->OIndex];
		if ( OB )
		{
			OB->Life = WC->Health;
		};
		if ( WC->Stage == WC->MaxStage )
		{
			OneObject* OB = Group[WC->OIndex];
			if ( OB )OB->Ready = true;
		};
		//WallCharacter* WCR=&WChar[WC->Type];
		//WC->SprBase=13*div((WCR->NBuild)*(WC->MaxHealth-WC->Health),WC->MaxHealth).quot;
	}
	else
	{
		OneObject* OB = Group[WC->OIndex];
		if ( OB )
		{
			OB->Ready = true;
			if ( OB->Life < OB->MaxLife )
			{
				OB->Life += 25;
				if ( OB->Life > WC->MaxHealth )OB->Life = WC->MaxHealth;
				WC->Health = OB->Life;
				SetLife( WC, WC->Health );
			};
		};
	};
};
void SetCellLife( int x, int y, int Life, bool Base )
{
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( WCL )
		{
			WCL->Health = Life;
			OneObject* OB = Group[WCL->OIndex];
			//assert(OB);
			if ( OB )OB->Life = Life;
			if ( Base )
			{
				WallCharacter* WCR = &WChar[WCL->Type];
				WCL->SprBase = 13 * div( ( WCR->NBuild )*( WCL->MaxHealth - WCL->Health ), WCL->MaxHealth ).quot;
				WCL->Sprite &= 15;
			};
			if ( Life == 0 )OB->Die();
		};
	};
};
const int  Wdx[4] = { 0,1,1,1 };
const int  Wdy[4] = { -1,-1,0,1 };
void SetLife( WallCell* WC, int Health )
{
	WallCharacter* WCR = &WChar[WC->Type];
	if ( WC->Sprite >= 32 )
	{
		int wcx = WC->x;
		int wcy = WC->y;
		int spr = WC->Sprite & 15;
		//assert(spr<4);
		if ( WC->Sprite >= 64 )
		{
			switch ( spr )
			{
			case 0:
				if ( GetWTP1( wcx, wcy - 1 ) == spr + 32 )wcy--;
				else wcy++;
				break;
			case 1:
				if ( GetWTP1( wcx + 1, wcy - 1 ) == spr + 32 )
				{
					wcx++;
					wcy--;
				}
				else
				{
					wcx--;
					wcy++;
				};
				break;
			case 2:
				if ( GetWTP1( wcx + 1, wcy ) == spr + 32 )wcx++;
				else wcx--;
				break;
			case 3:
				if ( GetWTP1( wcx + 1, wcy + 1 ) == spr + 32 )
				{
					wcx++;
					wcy++;
				}
				else
				{
					wcx--;
					wcy--;
				};
				break;
			};
		};
		if ( Health < div( WC->MaxHealth, 3 ).quot )
		{
			//transforming
			int LI = GetLI( wcx, wcy );
			if ( LI >= 0 && LI < MaxLI )
			{
				WC = WRefs[LI];
				if ( WC )DelGate( WC->GateIndex );
			};
			int Maxh = WC->MaxHealth;
			SetCellLife( wcx, wcy, 0, true );
			SetCellLife( wcx + Wdx[spr], wcy + Wdy[spr], Maxh / 2, true );
			SetCellLife( wcx - Wdx[spr], wcy - Wdy[spr], Maxh / 2, true );
		}
		else
		{
			SetCellLife( wcx, wcy, Health, false );
			SetCellLife( wcx + Wdx[spr], wcy + Wdy[spr], Health, false );
			SetCellLife( wcx - Wdx[spr], wcy - Wdy[spr], Health, false );
		};
	}
	else
	{
		if ( Health > WC->MaxHealth )Health = WC->MaxHealth;
		WC->Health = Health;
		WC->SprBase = 13 * div( ( WCR->NBuild )*( WC->MaxHealth - WC->Health ), WC->MaxHealth ).quot;
	};
};
//Usage of walls
extern int NRLFiles;
extern char* RLNames[512];
void LoadWallSprites( char* Name, char* Gate, word Type )
{
	WChar[Type].RIndex = GPS.PreLoadGPImage( Name );
	WChar[Type].GateFile = GPS.PreLoadGPImage( Gate );

};
int GetIconByName( char* Name );
int GetResID( char* );
extern char* SoundID[512];
extern word NSounds;
int SearchStr( char** Res, char* s, int count );
void LoadAllWalls()
{
	GFILE* f1 = Gopen( "walls.lst", "r" );

	NChar = 0;
	if ( f1 )
	{
		int z, zz, p1, p2, p3, p4, p5, p6;
		char gx[128];
		char gy[128];
		char gz[128];
		char ic1[128];
		char ic2[128];
		char ic3[128];
		char ic4[128];
		char ic5[128];
		do
		{
			z = Gscanf( f1, "%s", gx );
			if ( z == 1 && gx[0] != '/' )
			{
				zz = Gscanf( f1, "%s%d%d%d%d%s%d%d%s%s%s%s%s", gy, &p1, &p2, &p3, &p4, gz, &p5, &p6, ic1, ic2, ic3, ic4, ic5 );
				if ( zz != 13 )
				{
					sprintf( gy, "Walls.lst : incorrect parameters for %s", gx );
					ErrM( gy );
				}
				else
				{
					WChar[NChar].Name = new char[strlen( gx ) + 1];
					strcpy( WChar[NChar].Name, gx );
					WChar[NChar].NBuild = p1;
					WChar[NChar].Ndamage = p2;
					WChar[NChar].dx = p3;
					WChar[NChar].dy = p4;
					WChar[NChar].GateDx = p5;
					WChar[NChar].GateDy = p6;
					WChar[NChar].UpgradeGateIcon = GetIconByName( ic1 );
					WChar[NChar].OpenGateIcon = GetIconByName( ic2 );
					WChar[NChar].CloseGateIcon = GetIconByName( ic3 );
					WChar[NChar].UpgradeTower1 = GetIconByName( ic4 );
					WChar[NChar].UpgradeTower2 = GetIconByName( ic5 );
					LoadWallSprites( gy, gz, NChar );
					zz = Gscanf( f1, "%s", ic1 );
					if ( strcmp( ic1, "GATE_COST" ) )
					{
						sprintf( gy, "Walls.lst : GATE_COST expected for %s", gx );
						ErrM( gy );
					};
					zz = Gscanf( f1, "%d", &p1 );
					if ( zz != 1 )
					{
						sprintf( gy, "Walls.lst : incorrect parameters for %s (GATE_COST)", gx );
						ErrM( gy );
					};
					memset( WChar[NChar].GateCost, 0, sizeof WChar[NChar].GateCost );
					for ( int j = 0; j < p1; j++ )
					{
						zz = Gscanf( f1, "%s%d", ic1, &p2 );
						if ( zz != 2 )
						{
							sprintf( gy, "Walls.lst : incorrect parameters for %s (GATE_COST)", gx );
							ErrM( gy );
						};
						p3 = GetResID( ic1 );
						if ( p3 == -1 )
						{
							sprintf( gy, "Walls.lst : invalid resource ID:%s for %s", ic1, gx );
							ErrM( gy );
						};
						WChar[NChar].GateCost[p3] = p2;
					};
					zz = Gscanf( f1, "%s", ic1 );
					if ( strcmp( ic1, "TEXTURE" ) )
					{
						sprintf( gy, "Walls.lst : TEXTURE expected for %s", gx );
						ErrM( gy );
					};
					zz = Gscanf( f1, "%d%d%d", &p1, &p2, &p3 );
					if ( zz != 3 )
					{
						sprintf( gy, "Walls.lst : invalid TEXTURE description for %s", gx );
						ErrM( gy );
					};
					WChar[NChar].TexRadius = p1;
					WChar[NChar].TexPrec = p2;
					WChar[NChar].NTex = p3;
					WChar[NChar].Tex = new short[p3];
					for ( int j = 0; j < p3; j++ )
					{
						zz = Gscanf( f1, "%d", &p1 );
						if ( zz != 1 )
						{
							sprintf( gy, "Walls.lst : invalid TEXTURE description for %s", gx );
							ErrM( gy );
						};
						WChar[NChar].Tex[j] = p1;
					};
					zz = Gscanf( f1, "%s%d%s%d%s", gy, &p1, ic1, &p2, ic2 );
					if ( zz != 5 )
					{
						sprintf( gy, "Walls.lst : invalid sound data for %s", gx );
						ErrM( gy );
					};
					if ( strcmp( gy, "GATE_SOUND" ) )
					{
						sprintf( gy, "Walls.lst : GATE_SOUND expected for %s", gx );
						ErrM( gy );
					};
					WChar[NChar].OpenKeyFrame = p1;
					WChar[NChar].CloseKeyFrame = p2;
					if ( strcmp( ic1, "NONE" ) )
					{
						p1 = SearchStr( SoundID, ic1, NSounds );
						if ( p1 == -1 )
						{
							sprintf( gy, "Walls.lst : Invalod sound ID (%s) for %s", ic1, gx );
							ErrM( gy );
						};
					}
					else p1 = -1;
					WChar[NChar].CloseGateSoundID = p1;
					if ( strcmp( ic2, "NONE" ) )
					{
						p2 = SearchStr( SoundID, ic2, NSounds );
						if ( p2 == -1 )
						{
							sprintf( gy, "Walls.lst : Invalod sound ID (%s) for %s", ic1, gx );
							ErrM( gy );
						};
					}
					else p2 = -1;
					WChar[NChar].OpenGateSoundID = p2;
					//strcat(gy,".rlc");

					NChar++;
				};
			}
			else
			{
				NLine( f1 );
			};
		} while ( z == 1 );
	}
	else
	{
		ErrM( "Could not open walls.lst" );
	};
	//LoadWallSprites("wall0.rlc",0);
};
int GetWallType( char* Name )
{
	for ( int i = 0; i < NChar; i++ )
	{
		if ( !strcmp( WChar[i].Name, Name ) )return i;
	};
	return -1;
};
extern bool TransMode;
void RegisterVisibleGP( word Index, int FileIndex, int SprIndex, int x, int y );
void WallCluster::View()
{
	int x0 = mapx << 5;
	int y0 = mul3( mapy ) << 3;
	int Lx = smaplx << 5;
	int Ly = mul3( smaply ) << 3;
	OneObject* CUR = NULL;
	if ( TransMode )
	{
		for ( int i = 0; i < NCells; i++ )
		{
			WallCell* WCL = Cells + i;
			if ( WCL->Visible )
			{
				if ( WCL->OIndex < ULIMIT )
				{
					CUR = Group[WCL->OIndex];
					if ( !( CUR && ( !CUR->Sdoxlo ) && CUR->WallX == WCL->x&&CUR->WallY == WCL->y ) )
						CUR = NULL;
				};
				CurDrawNation = WCL->NI;
				int xx = ( WCL->x << 6 ) - x0;
				int yy = ( mul3( WCL->y ) << 4 ) - y0;
				int dz = GetHeight( ( WCL->x << 6 ) + 32, ( WCL->y << 6 ) + 32 );
				WallCharacter* WCR = &WChar[WCL->Type];
				if ( xx > -128 && xx<Lx + 128, yy - dz>-128 - dz&&yy - dz < Ly + 128 - dz )
				{
					//AddOptPoint(ZBF_LO,0,0,xx-WCR->dx,yy-WCR->dy-dz,NULL,WCR->GateFile,WCL->Sprite-32,AV_TRANSPARENT|AV_SHADOWONLY);
					if ( WCL->Sprite >= 32 )
					{
						if ( WCL->Sprite < 64 )
						{
							AddOptPoint( ZBF_LO, 0, 0, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, NULL, WCR->GateFile, WCL->Sprite - 32, AV_TRANSPARENT | AV_SHADOWONLY );
							switch ( WCL->Sprite )
							{
							case 32:
								AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, NULL, WCR->GateFile, WCL->Sprite - 32, AV_TRANSPARENT | AV_WITHOUTSHADOW );
								break;
							case 33:
								AddOptLine( xx + 96, yy - 48, xx - 32, yy + 48, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, NULL, WCR->GateFile, WCL->Sprite - 32, AV_TRANSPARENT | AV_WITHOUTSHADOW );
								break;
							case 34:
								AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, NULL, WCR->GateFile, WCL->Sprite - 32, AV_TRANSPARENT | AV_WITHOUTSHADOW );
								break;
							case 35:
								AddOptLine( xx - 32, yy - 16, xx + 96, yy + 48, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, NULL, WCR->GateFile, WCL->Sprite - 32, AV_TRANSPARENT | AV_WITHOUTSHADOW );
								break;
							};
						};
					}
					else
					{
						AddOptPoint( ZBF_LO, 0, 0, xx - WCR->dx, yy - WCR->dy - dz, NULL, WCR->RIndex, WCL->SprBase + WCL->Sprite, AV_SHADOWONLY );
						AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16, xx - WCR->dx, yy - WCR->dy - dz, NULL, WCR->RIndex, WCL->SprBase + WCL->Sprite, AV_TRANSPARENT | AV_WITHOUTSHADOW );
					};
				};
			};
		};
	}
	else
	{
		;
		for ( int i = 0; i < NCells; i++ )
		{
			WallCell* WCL = Cells + i;
			if ( Cells[i].Visible )
			{
				if ( WCL->OIndex < ULIMIT )
				{
					CUR = Group[WCL->OIndex];
					if ( !( CUR && ( !CUR->Sdoxlo ) && CUR->WallX == WCL->x&&CUR->WallY == WCL->y ) )
						CUR = NULL;
				};
				CurDrawNation = WCL->NI;
				int xx = ( WCL->x << 6 ) - x0;
				int yy = ( mul3( WCL->y ) << 4 ) - y0;
				int dz = GetHeight( ( WCL->x << 6 ) + 32, ( WCL->y << 6 ) + 32 );
				WallCharacter* WCR = &WChar[WCL->Type];
				//OneObject* OB=Group[WCL->OIndex];
				if ( xx > -128 && xx<Lx + 128 && yy - dz>-128 && yy - dz < Ly + 128 )
				{
					if ( CUR )
					{
						bool IsSel = false;
						if ( CUR->Selected&GM( MyNation ) ) IsSel = true;
						if ( WCL->Sprite >= 32 )
						{
							if ( WCL->Sprite < 64 )
							{
								AddOptPoint( ZBF_LO, 0, 0, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, NULL, WCR->GateFile, WCL->Sprite - 32, AV_SHADOWONLY );
								RegisterVisibleGP( CUR->Index, WCR->GateFile, WCL->Sprite - 32, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16 );
								int gs = Gates[WCL->GateIndex].State + ( WCL->Sprite & 15 )*NGOpen + 4;
								switch ( WCL->Sprite )
								{
								case 32:
									if ( IsSel )
									{
										AddOptPoint( ZBF_NORMAL, xx + 32, yy + 48, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_PULSING | AV_DARK | AV_WITHOUTSHADOW );
										//RegisterVisibleGP(CUR->Index,WCR->GateFile,WCL->Sprite-32,xx-WCR->GateDx+32,yy-WCR->GateDy-dz+16);
									}
									else
									{
										AddOptPoint( ZBF_NORMAL, xx + 32, yy + 48, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_WITHOUTSHADOW );
										//RegisterVisibleGP(CUR->Index,WCR->GateFile,WCL->Sprite-32,xx-WCR->GateDx+32,yy-WCR->GateDy-dz+16);
									};
									AddOptPoint( ZBF_NORMAL, xx + 32, yy + 48, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_WITHOUTSHADOW );
									//RegisterVisibleGP(CUR->Index,WCR->GateFile,gs,xx-WCR->GateDx+32,yy-WCR->GateDy-dz+16);
									AddOptPoint( ZBF_LO, 0, 0, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_SHADOWONLY );
									break;
								case 33:
									if ( IsSel )AddOptLine( xx + 95, yy - 15 + 14, xx - 31, yy + 47 + 14, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_PULSING | AV_DARK | AV_WITHOUTSHADOW );
									else AddOptLine( xx + 95, yy - 15 + 14, xx - 31, yy + 47 + 14, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_WITHOUTSHADOW );
									//RegisterVisibleGP(CUR->Index,WCR->GateFile,WCL->Sprite-32,xx-31,yy+47+14);
									AddOptLine( xx + 95, yy - 15 - 14, xx - 31, yy + 47 - 14, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_WITHOUTSHADOW );
									//RegisterVisibleGP(CUR->Index,WCR->GateFile,gs,xx-WCR->GateDx+32,yy-WCR->GateDy-dz+16);
									AddOptPoint( ZBF_LO, 0, 0, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_SHADOWONLY );
									break;
								case 34:
									if ( IsSel )AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16 + 12, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_PULSING | AV_DARK | AV_WITHOUTSHADOW );
									else AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16 + 12, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_WITHOUTSHADOW );
									AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16 - 12, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_WITHOUTSHADOW );
									AddOptPoint( ZBF_LO, 0, 0, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_SHADOWONLY );
									break;
								case 35:
									if ( IsSel )AddOptLine( xx - 31, yy - 15 + 12, xx + 95, yy + 47 + 12, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_PULSING | AV_DARK | AV_WITHOUTSHADOW );
									else AddOptLine( xx - 32, yy - 15 + 12, xx + 95, yy + 47 + 12, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, WCL->Sprite - 32, AV_WITHOUTSHADOW );
									AddOptLine( xx - 32, yy - 15 - 12, xx + 95, yy + 47 - 12, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_WITHOUTSHADOW );
									AddOptPoint( ZBF_LO, 0, 0, xx - WCR->GateDx + 32, yy - WCR->GateDy - dz + 16, CUR, WCR->GateFile, gs, AV_SHADOWONLY );
									break;
								};
							};
						}
						else
						{
							AddOptPoint( ZBF_LO, 0, 0, xx - WCR->dx, yy - WCR->dy - dz, CUR, WCR->RIndex, WCL->SprBase + WCL->Sprite, AV_SHADOWONLY );
							if ( IsSel )AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16, xx - WCR->dx, yy - WCR->dy - dz, CUR, WCR->RIndex, WCL->SprBase + WCL->Sprite, AV_PULSING | AV_DARK | AV_WITHOUTSHADOW );
							else AddOptPoint( ZBF_NORMAL, xx + 32, yy + 16, xx - WCR->dx, yy - WCR->dy - dz, CUR, WCR->RIndex, WCL->SprBase + WCL->Sprite, AV_WITHOUTSHADOW );
						};
					};
				};
			};
		};
	};
};
void WallCluster::Preview()
{
	int x0 = mapx << 5;
	int y0 = mul3( mapy ) << 3;
	int Lx = smaplx << 5;
	int Ly = mul3( smaply ) << 3;
	for ( int i = 0; i < NCells; i++ )
	{
		WallCell* WCL = Cells + i;
		if ( Cells[i].Visible )
		{
			CurDrawNation = MyNation;
			int xx = ( WCL->x << 6 ) - x0;
			int yy = ( mul3( WCL->y ) << 4 ) - y0;
			int dz = GetHeight( ( WCL->x << 6 ) + 32, ( WCL->y << 6 ) + 32 );

			WallCharacter* WCR = &WChar[WCL->Type];
			if ( xx > -128 && xx<Lx + 128, yy - dz>-128 && yy - dz < Ly + 128 )
			{
				AddOptPoint( ZBF_LO, 0, 0, xx - WCR->dx, yy - WCR->dy - dz, NULL, WCR->RIndex, WCL->Sprite, AV_SHADOWONLY );
				if ( Cells[i].CheckPosition() )
				{
					AddOptPoint( ZBF_NORMAL, xx, yy + 320, xx - WCR->dx, yy - WCR->dy - dz, NULL, WCR->RIndex, WCL->Sprite, AV_PULSING | AV_WHITE | AV_WITHOUTSHADOW );
				}
				else
				{
					AddOptPoint( ZBF_NORMAL, xx, yy + 320, xx - WCR->dx, yy - WCR->dy - dz, NULL, WCR->RIndex, WCL->Sprite, AV_PULSING | AV_RED | AV_WITHOUTSHADOW );

				}
			}
		}
	}
}

WallCluster TMPCluster;
WallSystem WSys;
byte WallType;

bool BuildWall;
bool FirstWall;
void SetWallBuildMode( byte NI, word NIndex )
{
	TMPCluster.~WallCluster();
	if ( NI != 0xFF )
	{
		NewMonster* NM = NATIONS[NI].Mon[NIndex]->newMons;
		TMPCluster.NM = NM;
		TMPCluster.NIndex = NIndex;
		TMPCluster.NI = NI;
		BuildWall = true;
		WallType = NM->Sprite;
		TMPCluster.Type = WallType;
		FirstWall = true;
	}
	else
	{
		BuildWall = false;
	};
};
void WallHandleDraw()
{
	WSys.Show();
	if ( BuildWall )TMPCluster.Preview();
};
static int xpre = 100000000;
static int ypre = 100000000;
int ConvScrY( int x, int y );
void CmdCreateWall( byte NI );
extern byte SpecCmd;

void WallHandleMouse()
{
	if ( BuildWall )
	{
		//TMPCluster.Type=WallType;
		if ( Lpressed )
		{//Start or end of placement process
			int xx = mouseX - smapx;
			int yy = mouseY - smapy;

			xpre = xx;
			ypre = yy;

			if ( xx > 0 && yy > 0 && xx < ( smaplx << 5 ) && yy < ( mul3( smaply ) << 3 ) )
			{
				if ( !FirstWall )
				{//Place segment (edge or final)
					TMPCluster.KeepSegment();
				}
				else
				{
					FirstWall = false;
				}

				if ( !TMPCluster.NCells )
				{//Place first segment
					xx += ( mapx << 5 );
					yy = ( mul3( mapy ) << 3 ) + yy;//mul3(Prop43(yy+(mul3(mapy)<<3))>>6;
					yy = ConvScrY( xx, yy );
					xx >>= 6;
					yy >>= 5;
					TMPCluster.UndoSegment();
					TMPCluster.ConnectToPoint( xx, yy );
					TMPCluster.CreateSprites();
				}
			}
			Lpressed = false;
		}
		else
		{//Placement process
			if ( !FirstWall )
			{
				if ( SpecCmd == 241 )
				{//Create wall on double left click
					CmdCreateWall( TMPCluster.NI );
					SetWallBuildMode( 0xFF, 0 );
					Lpressed = false;
					SpecCmd = 0;
					return;
				}

				int xx = mouseX - smapx;
				int yy = mouseY - smapy;

				if ( xx > 0 && yy > 0 && xx < ( smaplx << 5 ) && yy < ( mul3( smaply ) << 3 ) )
				{//Placement in progress
					xx += ( mapx << 5 );
					yy = ( mul3( mapy ) << 3 ) + yy;//mul3(Prop43(yy+(mul3(mapy)<<3))>>6;
					yy = ConvScrY( xx, yy );
					xx >>= 6;
					yy >>= 5;
					TMPCluster.UndoSegment();
					TMPCluster.ConnectToPoint( xx, yy );
					TMPCluster.CreateSprites();
				}
			}
		}
	}
}

int GetTopDistanceW( int xa, int ya, int xb, int yb )
{
	if ( xa < 0 || ya < 0 || xb < 0 || yb < 0 ||
		xa >= TopLx || xb >= TopLx || ya >= TopLx || yb >= TopLx )return 0xFFFF;
	int Top1 = TopRef[xa + ( ya << TopSH )];
	int Top2 = TopRef[xb + ( yb << TopSH )];
	if ( Top1 == 0xFFFE )return Norma( xa - xb, ya - yb );
	if ( Top1 >= 0xFFFE || Top2 >= 0xFFFE )return 0xFFFF;
	if ( Top1 == Top2 )return Norma( xa - xb, ya - yb );
	int Next1 = MotionLinks[Top1*NAreas + Top2];
	if ( Next1 == 0xFFFF )return 0xFFFF;
	if ( Next1 == Top2 )return Norma( xa - xb, ya - yb );
	int Next2 = MotionLinks[Top2*NAreas + Top1];
	if ( Next2 == 0xFFFF )return 0xFFFF;
	if ( Next2 == Top1 )return Norma( xa - xb, ya - yb );
	Area* AR1 = TopMap + Next1;
	Area* AR2 = TopMap + Next2;
	if ( Next1 == Next2 )return Norma( xa - AR1->x, ya - AR1->y ) + Norma( xb - AR2->x, yb - AR2->y );
	return Norma( xa - AR1->x, ya - AR1->y ) + Norma( xb - AR2->x, yb - AR2->y ) + LinksDist[Next1*NAreas + Next2];
};
int GetTopDistance( int xa, int ya, int xb, int yb );
//Building the walls
bool GetFreeCell( int* x, int* y, int bx, int by, int *bdist )
{
	int x1 = ( *x << 2 ) + 1;
	int y1 = ( *y << 2 ) + 1;
	int xx = ( *x ) + 1;
	int yy = *y;
	int tx = bx;
	int ty = by;
	int bdst = *bdist;
	bool Fnd = false;
	if ( !CheckBar( x1 + 4, y1, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	yy++;
	if ( !CheckBar( x1 + 4, y1 + 4, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	xx--;
	if ( !CheckBar( x1, y1 + 4, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	xx--;
	if ( !CheckBar( x1 - 4, y1 + 4, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	yy--;
	if ( !CheckBar( x1 - 4, y1, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	yy--;
	if ( !CheckBar( x1 - 4, y1 - 4, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	xx++;
	if ( !CheckBar( x1, y1 - 4, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	xx++;
	if ( !CheckBar( x1 + 4, y1 - 4, 2, 2 ) )
	{
		int dst = GetTopDistanceW( tx, ty, xx, yy );
		if ( dst <= 0xFFFE && dst < bdst )
		{
			bdst = dst;
			*x = xx;
			*y = yy;
			Fnd = true;
		};
	};
	*bdist = bdst;
	return Fnd;
}

bool FindPlaceForWallBuilder( byte NI, int *x, int *y, int bx, int by )
{
	//1. search for wall segment
	int xx, yy, xx1, yy1;
	int bxx = 0;
	int byy = 0;
	xx = *x;
	yy = *y;
	int rr = 0;
	int drr = 0;
	bool onefnd = false;
	int bdst = 1000000;
	do
	{
		if ( rr )
		{
			for ( int dx = 0; dx < rr; dx++ )
			{
				xx1 = xx + dx;
				yy1 = yy;
				if ( xx1 >= 0 && yy1 >= 0 && xx1&&xx1 < MaxLIX&&yy1 < MaxLIY )
				{
					int LI = GetLI( xx1, yy1 );
					WallCell* WCL = WRefs[LI];
					if ( WCL&&WCL->NI == NI&&WCL->Health < WCL->MaxHealth )
					{
						if ( GetFreeCell( &xx1, &yy1, bx, by, &bdst ) )
						{
							bxx = xx1;
							byy = yy1;
							onefnd = true;
						}
					}
				}
			}

			for ( int dx = 0; dx < rr; dx++ )
			{
				xx1 = xx + rr;
				yy1 = yy + dx;
				if ( xx1 >= 0 && yy1 >= 0 && xx1&&xx1 < MaxLIX&&yy1 < MaxLIY )
				{
					int LI = GetLI( xx1, yy1 );
					WallCell* WCL = WRefs[LI];
					if ( WCL&&WCL->NI == NI&&WCL->Health < WCL->MaxHealth )
					{
						if ( GetFreeCell( &xx1, &yy1, bx, by, &bdst ) )
						{
							bxx = xx1;
							byy = yy1;
							onefnd = true;
						}
					}
				}
			}

			for ( int dx = 0; dx < rr; dx++ )
			{
				xx1 = xx + rr - dx;
				yy1 = yy + rr;
				if ( xx1 >= 0 && yy1 >= 0 && xx1&&xx1 < MaxLIX&&yy1 < MaxLIY )
				{
					int LI = GetLI( xx1, yy1 );
					WallCell* WCL = WRefs[LI];
					if ( WCL && WCL->NI == NI && WCL->Health < WCL->MaxHealth )
					{
						if ( GetFreeCell( &xx1, &yy1, bx, by, &bdst ) )
						{
							bxx = xx1;
							byy = yy1;
							onefnd = true;
						}
					}
				}
			}
			for ( int dx = 0; dx < rr; dx++ )
			{
				xx1 = xx;
				yy1 = yy + dx + 1;
				if ( xx1 >= 0 && yy1 >= 0 && xx1&&xx1 < MaxLIX&&yy1 < MaxLIY )
				{
					int LI = GetLI( xx1, yy1 );
					WallCell* WCL = WRefs[LI];
					if ( WCL&&WCL->NI == NI&&WCL->Health < WCL->MaxHealth )
					{
						if ( GetFreeCell( &xx1, &yy1, bx, by, &bdst ) )
						{
							bxx = xx1;
							byy = yy1;
							onefnd = true;
						}
					}
				}
			}
		}
		else
		{
			xx1 = xx;
			yy1 = yy;
			if ( xx1 >= 0 && yy1 >= 0 && xx1 && xx1 < MaxLIX && yy1 < MaxLIY )
			{
				int LI = GetLI( xx1, yy1 );
				WallCell* WCL = WRefs[LI];
				if ( WCL && WCL->NI == NI && WCL->Health < WCL->MaxHealth )
				{
					if ( GetFreeCell( &xx1, &yy1, bx, by, &bdst ) )
					{
						bxx = xx1;
						byy = yy1;
						onefnd = true;
					}
				}
			}
		}

		if ( onefnd )
		{
			drr++;
		}
		rr += 2;
		xx--;
		yy--;
	} while ( rr < 48 && drr < 4 );

	if ( onefnd )
	{
		*x = bxx;
		*y = byy;
		return true;
	}
	else
	{
		return false;
	}
}

void BuildWallLink( OneObject* OB );

bool OneObject::BuildWall( int xx, int yy, byte Prio, byte OrdType, bool TempBlock )
{
	if ( CheckOrderAbility() || PrioryLevel > Prio )
	{
		return false;
	}
	NewMonster* NM = newMons;
	if ( !NM->Work.Enabled )
	{
		return false;
	}
	if ( FindPlaceForWallBuilder( NNUM, &xx, &yy, x >> 2, y >> 2 ) )
	{
		Order1* Or1 = CreateOrder( OrdType );
		if ( !Or1 )
		{
			return false;
		}
		Or1->PrioryLevel = Prio & 127;
		if ( TempBlock )
		{
			Or1->OrderType = 123;
		}
		else
		{
			Or1->OrderType = 122;
		}
		Or1->OrderTime = 0;
		Or1->DoLink = &BuildWallLink;
		Or1->info.MoveToXY.x = xx;
		Or1->info.MoveToXY.y = yy;
		if ( !OrdType && TempBlock )
		{
			BSetBar( ( xx << 2 ) + 1, ( yy << 2 ) + 1, 2 );
		}
		MakeOrderSound( this, 2 );
		return true;
	}
	return false;
}

void TempUnLock( OneObject* OBJ )
{
	if ( OBJ->LocalOrder&&OBJ->LocalOrder->DoLink == &BuildWallLink&&
		OBJ->LocalOrder->OrderType == 123 )
	{
		OBJ->LocalOrder->OrderType = 122;
		int xx = ( OBJ->LocalOrder->info.MoveToXY.x << 2 ) + 1;
		int yy = ( OBJ->LocalOrder->info.MoveToXY.y << 2 ) + 1;
		BClrBar( xx, yy, 2 );
	}
}

bool CheckSideForBuild( byte NI, int x, int y )
{
	if ( x >= 0 && y >= 0 && x < MaxLIX&&y < MaxLIY )
	{
		int LI = GetLI( x, y );
		WallCell* WC = WRefs[LI];
		if ( WC&&WC->NI == NI&&WC->Health < WC->MaxHealth )return true;
	};
	return false;
};
void BuildWallLink( OneObject* OBJ )
{
	//checking for near wall
	if ( OBJ->NewAnm->NFrames - FrmDec > OBJ->NewCurSprite )return;
	rando();
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	OBJ->UnBlockUnit();
	int xx = OBJ->x >> 2;
	int yy = OBJ->y >> 2;
	//char ddir=-128;
	char rd = OBJ->RealDir;
	bool found = false;
	int bdd = 127;
	int bdx = 0;
	int bdy = 0;
	int dirr = 0;
	if ( CheckSideForBuild( OBJ->NNUM, xx + 1, yy ) )
	{
		bdd = abs( rd );
		bdx = 1;
		bdy = 0;
		//ddir=0;
		found = true;
		dirr = 0;
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx + 1, yy + 1 ) )
	{
		int bdd1 = abs( rd - 32 );
		if ( bdd1 < bdd || !found )
		{
			bdx = 1;
			bdy = 1;
			found = true;
			bdd = bdd1;
			dirr = 32;
		};
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx, yy + 1 ) )
	{
		int bdd1 = abs( rd - 64 );
		if ( bdd1 < bdd || !found )
		{
			bdx = 0;
			bdy = 1;
			found = true;
			bdd = bdd1;
			dirr = 64;
		};
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx - 1, yy + 1 ) )
	{
		int bdd1 = abs( rd - 96 );
		if ( bdd1 < bdd || !found )
		{
			bdx = -1;
			bdy = 1;
			found = true;
			bdd = bdd1;
			dirr = 96;
		};
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx - 1, yy ) )
	{
		int bdd1 = abs( rd - 128 );
		if ( bdd1 < bdd || !found )
		{
			bdx = -1;
			bdy = 0;
			found = true;
			bdd = bdd1;
			dirr = 128;
		};
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx - 1, yy - 1 ) )
	{
		int bdd1 = abs( rd - 160 );
		if ( bdd1 < bdd || !found )
		{
			bdx = -1;
			bdy = -1;
			found = true;
			bdd = bdd1;
			dirr = 150;
		};
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx, yy - 1 ) )
	{
		int bdd1 = abs( rd - 192 );
		if ( bdd1 < bdd || !found )
		{
			bdx = 0;
			bdy = -1;
			found = true;
			bdd = bdd1;
			dirr = 192;
		};
	};
	if ( CheckSideForBuild( OBJ->NNUM, xx + 1, yy - 1 ) )
	{
		int bdd1 = abs( rd - 224 );
		if ( bdd1 < bdd || !found )
		{
			bdx = 1;
			bdy = -1;
			found = true;
			bdd = bdd1;
			dirr = 224;
		};
	};
	if ( found )
	{
		char ddr = rd - dirr;
		if ( abs( ddr < 32 ) )
		{
			OBJ->RealDir = dirr;
			OBJ->GraphDir = dirr;
			OBJ->NewCurSprite = 0;
			OBJ->NewAnm = &OBJ->newMons->Work;
			OBJ->BlockUnit();
			OBJ->InMotion = false;
			int LI = GetLI( xx + bdx, yy + bdy );
			WallCell* WCL = WRefs[LI];
			WallBuildStage( WCL );
			OBJ->NothingTime = 20;
		}
		else
		{
			if ( ddr > 0 )OBJ->RealDir -= 32;
			else OBJ->RealDir += 32;
			OBJ->GraphDir = OBJ->RealDir;
		};
	}
	else
	{
		int xx = OBJ->LocalOrder->info.MoveToXY.x;
		int yy = OBJ->LocalOrder->info.MoveToXY.y;
		if ( xx != ( OBJ->x >> 2 ) || yy != ( OBJ->y >> 2 ) )
		{
			if ( OBJ->NothingTime > 10 )
			{
				if ( !FindPlaceForWallBuilder( OBJ->NNUM, &xx, &yy, OBJ->x >> 2, OBJ->y >> 2 ) )
				{
					OBJ->DeleteLastOrder();
					return;
				};
				if ( CheckBar( ( xx << 2 ) + 1, ( xx << 2 ) + 1, 2, 2 ) )
				{
					if ( !FindPlaceForWallBuilder( OBJ->NNUM, &xx, &yy, OBJ->x >> 2, OBJ->y >> 2 ) )
					{
						OBJ->DeleteLastOrder();
						return;
					};
				};
				OBJ->LocalOrder->info.MoveToXY.x = xx;
				OBJ->LocalOrder->info.MoveToXY.y = yy;
			};
			int xx1 = ( xx << 2 ) + 1;
			int yy1 = ( yy << 2 ) + 1;
			if ( CheckBar( xx1 - 1, yy1 - 1, 3, 3 ) )
			{
				xx1++;
				if ( CheckBar( xx1 - 1, yy1 - 1, 3, 3 ) )
				{
					yy1++;
					if ( CheckBar( xx1 - 1, yy1 - 1, 3, 3 ) )
					{
						xx1--;
						if ( CheckBar( xx1 - 1, yy1 - 1, 3, 3 ) )
						{
							yy1--;
						};
					};
				};
			};
			OBJ->CreatePath( xx1, yy1 );
		}
		else
		{
			if ( !FindPlaceForWallBuilder( OBJ->NNUM, &xx, &yy, OBJ->x >> 2, OBJ->y >> 2 ) )
			{
				OBJ->DeleteLastOrder();
				return;
			};
			OBJ->LocalOrder->info.MoveToXY.x = xx;
			OBJ->LocalOrder->info.MoveToXY.y = yy;
			OBJ->CreatePath( ( xx << 2 ) + 1, ( yy << 2 ) + 1 );

		};
	};
};
bool CheckWallClick( int x, int y )
{
	int xx = x >> 6;
	int yy = y >> 6;
	if ( xx >= 0 && yy >= 0 && xx < MaxLIX&&yy < MaxLIY )
	{
		int LI = GetLI( xx, yy );
		WallCell* WCL = WRefs[LI];
		if ( WCL )
		{
			if ( WCL->NI == MyNation&&WCL->Health < WCL->MaxHealth )return true;
		};
		if ( yy > 1 )
		{
			yy++;
			LI = GetLI( xx, yy );
			WallCell* WCL = WRefs[LI];
			if ( WCL )
			{
				if ( WCL->NI == MyNation&&WCL->Health < WCL->MaxHealth )return true;
			};
		};
	};
	return false;
};
void DetermineWallClick( int x, int y )
{
	int xx = x >> 6;
	int yy = y >> 6;
	if ( xx >= 0 && yy >= 0 && xx < MaxLIX&&yy < MaxLIY )
	{
		int LI = GetLI( xx, yy );
		WallCell* WCL = WRefs[LI];
		if ( WCL )
		{
			if ( WCL->NI == MyNation&&WCL->Health < WCL->MaxHealth )
			{
				CmdRepairWall( MyNation, xx, yy );
			};
		};
		if ( yy > 1 )
		{
			yy++;
			LI = GetLI( xx, yy );
			WallCell* WCL = WRefs[LI];
			if ( WCL )
			{
				if ( WCL->NI == MyNation&&WCL->Health < WCL->MaxHealth )
				{
					CmdRepairWall( MyNation, xx, yy );
				};
			};
		};
	};
};
int GetWTP( int x, int y )
{
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )return -1;
		if ( WCL->SprBase )return -1;
		return( WCL->Sprite );
	}
	else return -1;
};
int GetWTP1( int x, int y )
{
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )return -1;
		return( WCL->Sprite );
	}
	else return -1;
};
void SetWTP( int x, int y, int Sprite, int Base )
{
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )return;
		WCL->Sprite = Sprite;
		WCL->SprBase = Base;
	}
	else return;
};
bool CheckGateUpgrade( OneObject* OB )
{
	if ( !OB->Wall )
	{
		return false;
	}
	int x = OB->WallX;
	int y = OB->WallY;
	if ( x < 1 || y < 1 )
	{
		return false;
	}
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )
		{
			return false;
		}
		if ( WCL->SprBase || WCL->Sprite > 3 )
		{
			return false;
		}
		OneObject* ob = Group[WCL->OIndex];
		if ( ob )
		{
			//checking
			switch ( WCL->Sprite )
			{
			case 0:
				if ( !( GetWTP( x, y - 1 ) == WCL->Sprite&&GetWTP( x, y + 1 ) == WCL->Sprite ) )return false;
				break;
			case 1:
				if ( !( GetWTP( x + 1, y - 1 ) == WCL->Sprite&&GetWTP( x - 1, y + 1 ) == WCL->Sprite ) )return false;
				break;
			case 2:
				if ( !( GetWTP( x + 1, y ) == WCL->Sprite&&GetWTP( x - 1, y ) == WCL->Sprite ) )return false;
				break;
			case 3:
				if ( !( GetWTP( x + 1, y + 1 ) == WCL->Sprite&&GetWTP( x - 1, y - 1 ) == WCL->Sprite ) )return false;
				break;
			}
		}
	}
	return true;
}

bool CheckOpenGate( OneObject* OB )
{
	if ( !OB->Wall )
	{
		return false;
	}
	int x = OB->WallX;
	int y = OB->WallY;
	if ( x < 1 || y < 1 )
	{
		return false;
	}
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )
		{
			return false;
		}
		if ( WCL->Sprite >= 32 && WCL->Sprite < 64 )
		{
			return 0 != Gates[WCL->GateIndex].Locked;
		}
	}
	return false;
}

bool CheckCloseGate( OneObject* OB )
{
	if ( !OB->Wall )return false;
	int x = OB->WallX;
	int y = OB->WallY;
	if ( x < 1 || y < 1 )return false;
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )return false;
		if ( WCL->Sprite >= 32 && WCL->Sprite < 64 )
		{
			return !Gates[WCL->GateIndex].Locked;
		};
	};
	return false;
};
void CreateGatesLink( OneObject* OBJ );
//extern int RESRC[8][8];
bool CreateGates( OneObject* OB )
{
	if ( !OB->Wall )return false;
	WallCharacter* WCR = GetWChar( OB );
	if ( !WCR )return false;
	int NI = OB->NNUM;
	for ( int i = 0; i < 8; i++ )
	{
		if ( XRESRC( NI, i ) < WCR->GateCost[i] )return false;
	};
	for ( int i = 0; i < 8; i++ )
	{
		AddXRESRC( NI, i, -WCR->GateCost[i] );
		OB->Nat->ControlProduce( 0xFF, i, -WCR->GateCost[i] );
		OB->Nat->ResOnUpgrade[i] += WCR->GateCost[i];
	};
	int x = OB->WallX;
	int y = OB->WallY;
	if ( x < 1 || y < 1 )return false;
	int LI = GetLI( x, y );
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( WCL->SprBase || WCL->Sprite > 3 )return false;
		OneObject* ob = Group[WCL->OIndex];
		if ( ob )
		{
			//checking
			switch ( WCL->Sprite )
			{
			case 0:
				if ( !( GetWTP( x, y - 1 ) == WCL->Sprite&&GetWTP( x, y + 1 ) == WCL->Sprite ) )return false;
				break;
			case 1:
				if ( !( GetWTP( x + 1, y - 1 ) == WCL->Sprite&&GetWTP( x - 1, y + 1 ) == WCL->Sprite ) )return false;
				break;
			case 2:
				if ( !( GetWTP( x + 1, y ) == WCL->Sprite&&GetWTP( x - 1, y ) == WCL->Sprite ) )return false;
				break;
			case 3:
				if ( !( GetWTP( x + 1, y + 1 ) == WCL->Sprite&&GetWTP( x - 1, y - 1 ) == WCL->Sprite ) )return false;
				break;
			};
			Order1* OR1 = ob->CreateOrder( 0 );
			if ( !OR1 )return  false;
			OR1->DoLink = &CreateGatesLink;
			OR1->OrderType = 189;
			OR1->PrioryLevel = 0;
			ob->delay = 20;
			ob->MaxDelay = 20;
			switch ( WCL->Sprite )
			{
			case 0:
				SetWTP( x, y - 1, WCL->Sprite, 13 );
				SetWTP( x, y, WCL->Sprite, 26 );
				SetWTP( x, y + 1, WCL->Sprite, 13 );
				break;
			case 1:
				SetWTP( x + 1, y - 1, WCL->Sprite, 13 );
				SetWTP( x, y, WCL->Sprite, 26 );
				SetWTP( x - 1, y + 1, WCL->Sprite, 13 );
				break;
			case 2:
				SetWTP( x - 1, y, WCL->Sprite, 13 );
				SetWTP( x, y, WCL->Sprite, 26 );
				SetWTP( x + 1, y, WCL->Sprite, 13 );
				break;
			case 3:
				SetWTP( x - 1, y - 1, WCL->Sprite, 13 );
				SetWTP( x, y, WCL->Sprite, 26 );
				SetWTP( x + 1, y + 1, WCL->Sprite, 13 );
				break;
			};
		};
	};
	return true;
};
void CreateGatesLink( OneObject* OBJ )
{
	if ( OBJ->delay )return;
	if ( OBJ->Sdoxlo )
	{
		OBJ->DeleteLastOrder();
		return;
	};
	int x = OBJ->WallX;
	int y = OBJ->WallY;
	int spr = GetWTP1( x, y );
	switch ( spr )
	{
	case 0:
		SetWTP( x, y - 1, spr + 64, 0 );
		SetWTP( x, y, spr + 32, 0 );
		SetWTP( x, y + 1, spr + 64, 0 );
		break;
	case 1:
		SetWTP( x + 1, y - 1, spr + 64, 0 );
		SetWTP( x, y, spr + 32, 0 );
		SetWTP( x - 1, y + 1, spr + 64, 0 );
		break;
	case 2:
		SetWTP( x - 1, y, spr + 64, 0 );
		SetWTP( x, y, spr + 32, 0 );
		SetWTP( x + 1, y, spr + 64, 0 );
		break;
	case 3:
		SetWTP( x - 1, y - 1, spr + 64, 0 );
		SetWTP( x, y, spr + 32, 0 );
		SetWTP( x + 1, y + 1, spr + 64, 0 );
		break;
	};
	int gi = AddGate( x, y, OBJ->NNUM );
	int LI = GetLI( x, y );
	OBJ->ClearOrders();
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WCL = WRefs[LI];
		if ( !WCL )return;
		WCL->GateIndex = gi;
	}
	else return;
};
//Gates handling
Gate* Gates;
int NGates;
int MaxGates;

//Init Gates[32] array
void SetupGates()
{
	NGates = 0;
	MaxGates = 32;
	Gates = new Gate[MaxGates];
}

void InitGates()
{
	NGates = 0;
}

int AddGate( short x, short y, byte NI )
{
	int curg = NGates;
	int j;
	for ( j = 0; j < NGates&&Gates[j].NI != 0xFF; j++ );
	if ( j < NGates )curg = j;
	else
	{
		if ( NGates == MaxGates )
		{
			MaxGates += 32;
			Gates = (Gate*) realloc( Gates, MaxGates * sizeof Gate );
		};
		curg = NGates;
		NGates++;
	};
	Gates[curg].NI = NI;
	Gates[curg].NMask = 1 << NI;
	Gates[curg].State = 0;
	Gates[curg].x = x;
	Gates[curg].y = y;
	Gates[curg].delay = 0;
	Gates[curg].Locked = 1;
	int LI = GetLI( x, y );
	WallCell* WC = WRefs[LI];
	//assert(WC);
	Gates[curg].CharID = WC->Type;

	return curg;
};
void DelGate( int ID )
{
	//assert(ID<NGates);
	Gates[ID].NI = 0xFF;
};
bool CheckUnitsInCell( int cell, byte NMask, int x, int y, int R )
{
	cell += VAL_MAXCX + 1;
	if ( cell < 0 || cell >= VAL_MAXCIOFS )return false;
	int NMon = MCount[cell];
	if ( !NMon )return NULL;
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && ( !OB->Sdoxlo ) && OB->NMask&NMask&&Norma( x - OB->RealX, y - OB->RealY ) < R )return true;
		};
	};
	return false;
};
bool CheckNTPresence( int x, int y, int R, byte NMask )
{
	int cell = ( ( y / 128 ) * 128 ) + ( x / 128 );

	x *= 16;
	y *= 16;
	R *= 16;

	int rx1 = 1;

	OneObject* DestObj = nullptr;

	int mindist = 10000000;

	int rx2 = 3;

	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );

	byte* bpt = NPresence + stcell;

	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCIOFS )
			{
				if ( bpt[0] & NMask )
				{
					if ( CheckUnitsInCell( stcell, NMask, x, y, R ) )return true;
				};
			};
			stcell++;
			bpt++;
		};
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	};
	return false;
}

void ControlSquish();

extern int HISPEED;

void PlayOpenGate( Gate* Gat, WallCharacter* WCR )
{
	if ( WCR->OpenGateSoundID == -1 )
	{
		return;
	}

	if ( div( WCR->OpenKeyFrame, HISPEED + 1 ).quot == div( Gat->State, HISPEED + 1 ).quot )
	{
		int xx = ( int( Gat->x ) * 64 ) + 32;
		int yy = ( int( Gat->y ) * 64 ) + 32;
		AddEffect( xx, ( yy / 2 ) - GetHeight( xx, yy ), WCR->OpenGateSoundID );
	}
}

void PlayCloseGate( Gate* Gat, WallCharacter* WCR )
{
	if ( WCR->CloseGateSoundID == -1 )
	{
		return;
	}

	if ( div( WCR->CloseKeyFrame, HISPEED + 1 ).quot == div( Gat->State, HISPEED + 1 ).quot )
	{
		int xx = ( int( Gat->x ) * 64 ) + 32;
		int yy = ( int( Gat->y ) * 64 ) + 32;
		AddEffect( xx, ( yy / 2 ) - GetHeight( xx, yy ), WCR->CloseGateSoundID );
	}
}

//Open and close gates, check for squashed units
void ControlGates()
{
	ControlSquish();
	for ( int i = 0; i < NGates; i++ )
	{
		Gate* Gat = Gates + i;
		WallCharacter* WCR = WChar + Gat->CharID;
		if ( Gat->NI != 0xFF )
		{
			if ( Gat->Locked )
			{
				Gat->delay = 0;
				if ( Gat->State )
				{
					PlayCloseGate( Gat, WCR );
					Gat->State--;
				}
			}
			else
			{
				PlayOpenGate( Gat, WCR );
				Gat->State++;
				if ( Gat->State >= NGOpen - 1 )
				{
					Gat->State = NGOpen - 1;
					Gat->delay = 100;
				}
			}
		}
	}
}

extern word OWNER;

bool CheckVisibility( int x1, int y1, int x2, int y2, word MyID )
{
	x1 <<= 2;
	y1 <<= 2;
	x2 <<= 2;
	y2 <<= 2;
	int Dx = ( x2 - x1 );
	int Dy = ( y2 - y1 );
	int D = Norma( Dx, Dy );
	int N = ( D >> 11 ) + 1;
	Dx = div( Dx, N ).quot;
	Dy = div( Dy, N ).quot;
	for ( int i = 1; i < N; i++ )
	{
		x1 += Dx;
		y1 += Dy;
		if ( GetBar3DHeight( x1 >> 6, y1 >> 6 ) > 18 )
		{
			if ( OWNER != MyID )return false;
		};
	};
	return true;
};

//Kill units between gate doors
void SquishUnitsInCell( int cell, int x, int y, int R )
{
	cell += VAL_MAXCX + 1;

	if ( cell < 0 || cell >= VAL_MAXCIOFS )
	{
		return;
	}

	int NMon = MCount[cell];
	if ( !NMon )
	{
		return;
	}

	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && ( !OB->Sdoxlo ) && Norma( x - OB->RealX, y - OB->RealY ) < R )
			{
				if ( MFIELDS->CheckBar( OB->x, OB->y, OB->Lx, OB->Lx ) )
				{
					if ( !( OB->Wall || OB->BlockInfo ) )
					{
						OB->Die();
					}
				}
			}
		}
	}
}

void SquishUnits( int x, int y, int R, byte NMask )
{
	int cell = ( ( y / 128 ) * 128 ) + ( x / 128 );
	x *= 16;
	y *= 16;
	R *= 16;

	int rx1 = 1;

	OneObject* DestObj = nullptr;

	int mindist = 10000000;

	int rx2 = 3;

	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );

	byte* bpt = NPresence + stcell;
	byte nms = ~NMask;

	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCIOFS )
			{
				if ( bpt[0] )
				{
					SquishUnitsInCell( stcell, x, y, R );
				}
			}
			stcell++;
			bpt++;
		}
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	}
	return;
}

extern int tmtmt;

void ControlSquish()
{
	if ( 0 == tmtmt % 27 )
	{
		for ( int i = 0; i < NGates; i++ )
		{
			Gate* Gat = Gates + i;
			if ( Gat->NI != 0xFF )
			{
				SquishUnits( ( int( Gat->x ) * 64 ) + 32, ( int( Gat->y ) * 64 ) + 32, 120, Gat->NMask );
			}
		}
	}
}

void OpenCell( int x, int y )
{
	int LI = GetLI( x, y );
	x *= 4;
	y *= 4;

	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WC = WRefs[LI];
		if ( WC )
		{
			switch ( WC->Sprite & 15 )
			{
			case 0:
				MFIELDS->BClrSQ( x, y, 4, 4 );

				MFIELDS->BSetSQ( x, y - 1, 4, 1 );
				MFIELDS->BSetSQ( x, y + 4, 4, 1 );
				break;
			case 1:
				MFIELDS->BClrSQ( x + 2, y - 1, 2, 4 );
				MFIELDS->BClrSQ( x, y + 1, 2, 4 );
				MFIELDS->BClrPt( x + 1, y );
				MFIELDS->BClrPt( x + 2, y + 3 );
				MFIELDS->BClrPt( x + 4, y );
				MFIELDS->BClrPt( x + 4, y + 1 );
				MFIELDS->BClrPt( x - 1, y + 2 );
				MFIELDS->BClrPt( x - 1, y + 3 );

				MFIELDS->BSetPt( x + 3, y - 2 );
				MFIELDS->BSetPt( x + 4, y - 1 );
				MFIELDS->BSetPt( x + 5, y );
				MFIELDS->BSetPt( x - 2, y + 3 );
				MFIELDS->BSetPt( x - 1, y + 4 );
				MFIELDS->BSetPt( x, y + 5 );
				break;
			case 2:
				MFIELDS->BClrSQ( x, y, 4, 4 );

				MFIELDS->BSetSQ( x - 1, y, 1, 4 );
				MFIELDS->BSetSQ( x - 1, y, 1, 4 );
				break;
			case 3:
				MFIELDS->BClrSQ( x, y - 1, 2, 4 );
				MFIELDS->BClrSQ( x + 2, y + 1, 2, 4 );
				MFIELDS->BClrPt( x - 1, y );
				MFIELDS->BClrPt( x - 1, y + 1 );
				MFIELDS->BClrPt( x + 2, y );
				MFIELDS->BClrPt( x + 1, y + 3 );
				MFIELDS->BClrPt( x + 4, y + 2 );
				MFIELDS->BClrPt( x + 4, y + 3 );

				MFIELDS->BSetPt( x - 1, y - 1 );
				MFIELDS->BSetPt( x, y - 2 );
				MFIELDS->BSetPt( x - 2, y );
				MFIELDS->BSetPt( x + 4, y + 4 );
				MFIELDS->BSetPt( x + 5, y + 3 );
				MFIELDS->BSetPt( x + 3, y + 5 );
				break;
			}
		}
	}
}

void CloseCell( int x, int y )
{
	int LI = GetLI( x, y );
	x *= 4;
	y *= 4;
	if ( LI >= 0 && LI < MaxLI )
	{
		WallCell* WC = WRefs[LI];
		if ( WC )
		{
			switch ( WC->Sprite & 15 )
			{
			case 0:
				MFIELDS->BSetSQ( x, y, 4, 4 );
				break;
			case 1:
				MFIELDS->BSetSQ( x + 2, y - 1, 2, 4 );
				MFIELDS->BSetSQ( x, y + 1, 2, 4 );
				MFIELDS->BSetPt( x + 1, y );
				MFIELDS->BSetPt( x + 2, y + 3 );
				MFIELDS->BSetPt( x + 4, y );
				MFIELDS->BSetPt( x + 4, y + 1 );
				MFIELDS->BSetPt( x - 1, y + 2 );
				MFIELDS->BSetPt( x - 1, y + 3 );
				break;
			case 2:
				MFIELDS->BSetSQ( x, y, 4, 4 );
				break;
			case 3:
				MFIELDS->BSetSQ( x, y - 1, 2, 4 );
				MFIELDS->BSetSQ( x + 2, y + 1, 2, 4 );
				MFIELDS->BSetPt( x - 1, y );
				MFIELDS->BSetPt( x - 1, y + 1 );
				MFIELDS->BSetPt( x + 2, y );
				MFIELDS->BSetPt( x + 1, y + 3 );
				MFIELDS->BSetPt( x + 4, y + 2 );
				MFIELDS->BSetPt( x + 4, y + 3 );
				break;
			}
		}
	}
}

void CloseGates( OneObject* OB )
{
	if ( OB->Wall )
	{
		int LI = GetLI( OB->WallX, OB->WallY );
		if ( LI >= 0 && LI < MaxLI )
		{
			WallCell* WC = WRefs[LI];
			if ( WC&&WC->Sprite >= 32 && WC->Sprite < 64 )
			{
				Gates[WC->GateIndex].Locked = 1;
				CloseCell( OB->WallX, OB->WallY );
				DynamicalLockTopCell( OB->WallX, OB->WallY );
			}
		}
	}
}

void OpenGates( OneObject* OB )
{
	if ( OB->Wall )
	{
		int LI = GetLI( OB->WallX, OB->WallY );
		if ( LI >= 0 && LI < MaxLI )
		{
			WallCell* WC = WRefs[LI];
			if ( WC&&WC->Sprite >= 32 && WC->Sprite < 64 )
			{
				Gates[WC->GateIndex].Locked = 0;
				OpenCell( OB->WallX, OB->WallY );
				DynamicalUnLockTopCell( OB->WallX, OB->WallY );
			}
		}
	}
}