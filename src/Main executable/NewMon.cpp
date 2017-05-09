//New monsters format
//*******************************************************//
//         K   K  EEEEE  RRRR   N    N  EEEEE  L         //
//         K  K   E      R   R  NN   N  E      L         //
//         K K    E      R   R  N N  N  E      L         //
//         KK     EEEE   RRRR   N  N N  EEEE   L         //
//         K K    E      R   R  N   NN  E      L         //
//         K  K   E      R    R N    N  E      L         //
//         K   K  EEEEE  R    R N    N  EEEEE  LLLLL     //
//*******************************************************//
#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "ActiveScenary.h"
#include "fog.h"
#include "Megapolis.h"

#include <assert.h>
#include "walls.h"
#include "mode.h"
#include "GSound.h"
#include "MapSprites.h"
#include "NewMon.h"
#include "Math.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "NewUpgrade.h"
#include "ZBuffer.h"
#include "Path.h"
#include "Transport.h"
#include "3DBars.h"
#include "CDirSnd.h"
#include "NewAI.h"
#include "3DMapEd.h"
#include "TopoGraf.h"

#include "Fonts.h"
#include "Safety.h"
#include "3DGraph.h"
#include "Nature.h"
#include "ConstStr.h"
#include "StrategyResearch.h"
#include "EinfoClass.h"
#include "Sort.h"

extern const int kImportantMessageDisplayTime;
extern const int kSystemMessageDisplayTime;
extern const int kMinorMessageDisplayTime;

extern byte CaptState;
extern bool NOPAUSE;
extern ReportFn* AttackLink;
void CheckArmies( City* );
void DestructBuilding( OneObject* OB );
int GetTopology( int x, int y );
int GetTopology( int* x, int* y );
bool CheckTopDirectWay( int x0, int y0, int x1, int y1, byte Type );
word CheckMotionThroughEnemyAbility( OneObject* OB, int px, int py );
bool CheckBuildPossibility( byte NI, int x, int y );
extern bool FastMode;
void GotoFinePosition( OneObject* OB );
extern bool AllowPathDelay;
extern byte   WeaponFlags[32];
NewAnimation* Shar;
NewAnimation* SharBuild;

int SearchStr( char** Res, char* s, int count );
NewAnimation* GetNewAnimationByName( char* Name );
int GetResID( char* gy );
int GetResByName( char* gy );
void PerformMotion2( OneObject* OB );
int GetWeaponType( char* Name );
extern bool PeaceMode;
extern byte LockMode;
extern bool TransMode;
extern char* SoundID[MaxSnd];
extern word NSounds;
int CheckPt( int x, int y );
void LoadIcons();
int Prop43( int y );
extern byte* NPresence;
extern bool Mode3D;
extern short TSin[257];
extern short TCos[257];
extern short TAtg[257];
int PortBuiX, PortBuiY;
void NLine( GFILE* f );
int GetUnitKind( char* Name );
void ShowRLCItemPal( int x, int y, lpRLCTable lprt, int n, byte* Pal );
void ShowRLCItemGrad( int x, int y, lpRLCTable lprt, int n, byte* Pal );
void normstr( char* str );
void InitFlags( GeneralObject* GO );
int mul3( int );
void DrawMarker( OneObject* OB );
void DrawMiniMarker( OneObject* OB );
void ClearMaps();
int GetWallType( char* Name );
short LastDirection = 512;
void CheckCapture( OneObject* OBJ );

//01-Order
//02-Attack
//04-select
void AddUnlimitedEffect( int x, int y, int id );

void MakeOrderSound( OneObject* OB, byte SMask )
{
	if ( SMask == 4 && OB->NNUM != MyNation )
	{
		return;
	}

	Nation* NT = OB->Nat;
	if ( NT->SoundMask[OB->NIndex] & SMask )
	{
		return;
	}

	NT->SoundMask[OB->NIndex] |= SMask;
	NewMonster* NM = OB->newMons;
	int SID = -1;

	switch ( SMask )
	{
	case 1:
		SID = NM->AttackSoundID;
		break;
	case 2:
		SID = NM->OrderSoundID;
		break;
	case 4:
		SID = NM->SelectSoundID;
	}

	if ( SID == -1 )
	{
		return;
	}

	AddSingleEffect( OB->RealX >> 4, ( OB->RealY >> 5 ) - OB->RZ, SID );
}

void ConvertToUp( char* str )
{
	while ( str[0] )
	{
		if ( str[0] >= 'a'&&str[0] <= 'z' )
		{
			str[0] -= 'a' - 'A';
		}
		str++;
	}
}

int GetRAngle( int dx, int dy, int Angle )
{
	int ang;
	int sx = abs( dx );
	int sy = abs( dy );
	if ( !( dx || dy ) )
	{
		return 0;
	}

	if ( sx > sy )
	{
		ang = div( dy << 8, sx ).quot;
		if ( dx < 0 )
		{
			ang = 1024 - ang;
		}
	}
	else
	{
		ang = 512 - div( dx << 8, sy ).quot;
		if ( dy < 0 )
		{
			ang = 2048 - ang;
		}
	}

	ang = Angle - ang;

	while ( ang < 0 )
	{
		ang += 2048;
	}

	if ( ang > 1792 )
	{
		ang -= 2048;
	}

	return ang;
}

int GetLAngle( int dx, int dy, int Angle )
{
	int ang;
	int sx = abs( dx );
	int sy = abs( dy );

	if ( !( dx || dy ) )
	{
		return 0;
	}

	if ( sx > sy )
	{
		ang = div( dy << 8, sx ).quot;
		if ( dx < 0 )
		{
			ang = 1024 - ang;
		}
	}
	else
	{
		ang = 512 - div( dx << 8, sy ).quot;
		if ( dy < 0 )
		{
			ang = 2048 - ang;
		}
	}

	ang -= Angle;
	while ( ang < 0 )
	{
		ang += 2048;
	}

	if ( ang > 1792 )
	{
		ang -= 2048;
	}

	return ang;
}

bool PInside( int x, int y, int x1, int y1, int xp, int yp )
{
	if ( xp >= x&&xp <= x1&&yp >= y&&yp <= y1 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void OneObject::ClearOrders()
{
	while ( LocalOrder )
	{
		DeleteLastOrder();
	}
	Attack = 0;
	PrioryLevel = 0;
	EnemyID = 0xFFFF;
}

void InitAllGame()
{
	ClearMaps();

	memset( Group, 0, sizeof Group );
	memset( Sprites, 0, sizeof Sprites );
	memset( NSL, 0, sizeof NSL );
}

void DosToWin( char* Str )
{
	int len = strlen( Str );
	for ( int i = 0; i < len; i++ )
	{
		byte c = Str[i];
		if ( c >= 0x80 && c < 0xB0 )
		{
			c += 0x40;
		}
		else
		{
			if ( c >= 0xE0 && c < 0xF0 )
			{
				c += 0x10;
			}
		}
		Str[i] = c;
	}
}

extern int CurPalette;
extern bool PalDone;

void ErrM( char* s )
{
	if ( PalDone )
	{
		if ( CurPalette == 2 )
		{
			LoadPalette( "2\\agew_1.pal" );
		}
		else
		{
			LoadPalette( "0\\agew_1.pal" );
		}
	}
	MessageBox( hwnd, s, "LOADING FAILED...", MB_ICONWARNING | MB_OK );
}

void NEPar( char* name, int line, char* Sect, int Need )
{
	char gx[128];
	sprintf( gx, "%s,%d : %s : Not enough parameters. Need: %d", name, line, Sect, Need );
	ErrM( gx );
}

void InvPar( char* name, int line, char* Sect, char* parm )
{
	char gx[128];
	sprintf( gx, "%s,%d : %s : Invalid parameter: %s", name, line, Sect, parm );
	ErrM( gx );
}

void IncPar( char* name, int line, char* Sect )
{
	char gx[128];
	sprintf( gx, "%s,%d : %s : Incorrect parameters", name, line, Sect );
	ErrM( gx );
}

void UpConv( char* str )
{
	int i = 0;
	char c;
	do
	{
		c = str[i];

		if ( c >= 'a'&&c <= 'z' )
		{
			c -= 0x20;
		}

		str[i] = c;
		i++;
	} while ( c != 0 );
}

//------------------------------------------New RLC information
word RLCNSpr[1024 + 512];
word NNewMon;
NewMonster NewMon[512];
char* MonNames[512];
//-----------------New weapon animation discription------------
NewAnimation WNewAnm[256];
char* WAnmNames[256];
word NNewAnm;
//-------------------------------------------------------------
int CurrentCursorGP = 0;
int AnmCursorGP = 0;
extern int CBB_GPFILE;

void LoadMapInterface();

void InitNewMonstersSystem()
{
	char* Name = "icons";
	GPS.PreLoadGPImage( Name, false );
	GPS.PreLoadGPImage( "smicons", false );
	GPS.PreLoadGPImage( "flag", false );
	GPS.PreLoadGPImage( "Interface\\GScroll", false );
	GPS.PreLoadGPImage( "smicons1", false );
	GPS.PreLoadGPImage( "Cursor_00" );
	GPS.PreLoadGPImage( "Cursor_01" );
	GPS.PreLoadGPImage( "Cursor_02" );
	CBB_GPFILE = GPS.PreLoadGPImage( "Interface\\LongBox0" );
	LoadMapInterface();
	AnmCursorGP = GPS.PreLoadGPImage( "Interface\\CursGo" );
	NNewMon = 0;
	NNewAnm = 0;
	LoadIcons();
	UFONTS.LoadFonts();
}

NewAnimation::NewAnimation()
{
	Enabled = false;
	ActiveFrame = 0xFF;
	LineInfo = nullptr;
}

NewMonster::NewMonster()
{
	memset( &MotionL, 0, sizeof NewMonster );
}

//--------------------Icons------------------
void ReadKeyCodes();
char* IconNames[512];
byte KeyCodes[512][2];
word NIcons;
void LoadMapInterface();
void LoadIcons()
{
	NIcons = 0;
	GFILE* f = Gopen( "IconList.txt", "r" );

	char gx[128];
	int z;
	if ( f )
	{
		do
		{
			z = Gscanf( f, "%s", gx );
			if ( !strcmp( gx, "[END]" ) )return;
			IconNames[NIcons] = new char[strlen( gx ) + 1];
			ConvertToUp( gx );
			strcpy( IconNames[NIcons], gx );
			NIcons++;
		} while ( z == 1 );
	}
	else
	{
		ErrM( "Unable to open IconList.txt" );
	};
	ReadKeyCodes();
};
int GetIconByName( char* Name )
{
	ConvertToUp( Name );
	for ( int i = 0; i < NIcons; i++ )
	{
		if ( !strcmp( IconNames[i], Name ) )return i;
	};
	return -1;
};
char* ReadStatement( char* Src, char* Dst )
{
	Dst[0] = 0;
	if ( Src )
	{
		while ( Src[0] == ' ' || Src[0] == 9 )Src++;
		int z = sscanf( Src, "%s", Dst );
		if ( z == 1 )
		{
			Src += strlen( Dst );
			return Src;
		}
		else return 0;
	}
	else return nullptr;
};
#define NKEYS 68
char* KeyNames[NKEYS] = { "0","1","2","3","4","5","6","7","8","9",
"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
"NUM0","NUM1","NUM2","NUM3","NUM4","NUM5","NUM6","NUM7","NUM8","NUM9",
"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","PGUP","PGDN","HOME","END","INS","NUM+","NUM-","NUM*","NUM/","<",">","?" };

byte ScanKeys[NKEYS] = { '0','1','2','3','4','5','6','7','8','9',
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
VK_NUMPAD0,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4,VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9,
VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8,VK_F9,VK_F10,VK_NEXT,VK_PRIOR,VK_HOME,VK_END,VK_INSERT,107,109,106,111,188,190,191 };

bool ReadWinString( GFILE* F, char* STR, int Max );
void ReadKeyCodes()
{
	memset( &KeyCodes[0][0], 0, sizeof KeyCodes );
	GFILE* F = Gopen( "Data\\Keyboard.dat", "r" );
	if ( F )
	{
		char ccc[257];
		bool Done;
		do
		{
			Done = ReadWinString( F, ccc, 256 );
			if ( Done&&ccc[0] != '/' )
			{
				_strupr( ccc );
				char DST[256];
				char* Next;
				byte ScanCode = 0;
				byte KeyState = 0;
				int NCODES = 0;
				bool IconMode = 0;
				Next = ccc;
				do
				{
					Next = ReadStatement( Next, DST );
					if ( Next )
					{
						if ( IconMode )
						{
							int id = GetIconByName( DST );
							if ( id == -1 )
							{
								char err[128];
								sprintf( err, "Unknown icon ID: %s", DST );
								ErrM( err );
							}
							else
							{
								KeyCodes[id][0] = ScanCode;
								KeyCodes[id][1] = KeyState;
							};
						}
						else
						{
							if ( DST[0] == ':' )
							{
								IconMode = 1;
							}
							else
								if ( !strcmp( DST, "CTRL" ) )KeyState |= 1;
								else if ( !strcmp( DST, "ALT" ) )KeyState |= 2;
								else if ( !strcmp( DST, "SHIFT" ) )KeyState |= 4;
								else
								{
									ScanCode = 255;
									for ( int j = 0; j < NKEYS; j++ )if ( !strcmp( DST, KeyNames[j] ) )ScanCode = j;
									if ( ScanCode == 255 )
									{
										char err[128];
										sprintf( err, "Unknown key code: '%s'", DST );
										ErrM( err );
										ScanCode = 0;
									};
								};
						};
					};
				} while ( Next );
			};
		} while ( Done );
	}
	else
	{
		ErrM( "Unable to open Data\\Keyboard.dat" );
	};
};
//-----------Loading additional New animations----------
extern NewAnimation* Shar;
void LoadNewAimations()
{
	GFILE* f1 = Gopen( "weapon.ads", "r" );

	char Fn[128];
	char gx[128];
	char gy[128];
	char gz[128];
	int MaxRLC = -1;
	word RLCRef[128];
	short RLCdx[128];
	short RLCdy[128];
	int z1, z2, z3, p1, p2, p3;
	bool finish = false;
	int Line = 1;
	int z = 0;
	if ( f1 )
	{
		do
		{
			z = Gscanf( f1, "%s", gx );
			if ( z != 1 )IncPar( "weapon.ads", Line, "?" );
			if ( gx[0] != '/'&&gx[0] != '[' )
			{
				if ( gx[0] == '@' )
				{
					//Animation;
					gx[0] = '#';
					WAnmNames[NNewAnm] = new char[strlen( gx ) + 1];
					strcpy( WAnmNames[NNewAnm], gx );
					NewAnimation* NANM = &WNewAnm[NNewAnm];
					int z4;
					if ( NANM )
					{
						z = Gscanf( f1, "%d%d%d%d", &z1, &z2, &z3, &z4 );
						if ( z != 4 )NEPar( "weapon.ads", Line, gx, 4 );
						//NANM->StartDx=NAStartDx;
						//NANM->StartDy=NAStartDy;
						//NANM->Parts=NAParts;
						//NANM->PartSize=NAPartSize;
						NANM->Enabled = true;
						NANM->Rotations = z1;
						NANM->SoundID = -1;
						NANM->ActiveFrame = 0;
						NANM->ActivePtX = new short[z1];
						NANM->ActivePtY = new short[z1];
						NANM->LineInfo = nullptr;
						for ( int i = 0; i < z1; i++ )
						{
							NANM->ActivePtX[i] = 0;
							NANM->ActivePtY[i] = 0;
						};
						NANM->TicksPerFrame = 1;
						int dz, nz;
						p1 = RLCRef[z2];
						if ( z3 >= z4 )
						{
							if ( z3*z1 >= RLCNSpr[p1] )
							{
								sprintf( gy, "Weapon.ads,line %d : Reference to RLC sprite is beyond the end of RLC file.", Line );
								ErrM( gy );
							};
							dz = -1;
							nz = z3 - z4 + 1;
						}
						else
						{
							if ( z4*z1 >= RLCNSpr[p1] )
							{
								sprintf( gy, "Weapon.ads,line %d : Reference to RLC sprite is beyond the end of RLC file.", Line );
								ErrM( gy );
							};
							dz = 1;
							nz = z4 - z3 + 1;


						};
						NANM->NFrames = nz;
						NANM->Frames = new NewFrame[nz];
						for ( int i = 0; i < nz; i++ )
						{
							NewFrame* NF = &NANM->Frames[i];
							NF->FileID = p1;
							NF->SpriteID = z3;
							z3 += dz;
							NF->dx = RLCdx[z2];
							NF->dy = RLCdy[z2];
						};
						NNewAnm++;
						NLine( f1 );
						Line++;
					}
					else
					{
						gx[0] = '@';
						sprintf( gy, "Weapon.ads,line %d : Unknown animation type: %s", Line, gx );
						ErrM( gy );
					};
				}
				else
					if ( gx[0] == '$' )
					{
						//Animation;
						gx[0] = '#';
						WAnmNames[NNewAnm] = new char[strlen( gx ) + 1];
						strcpy( WAnmNames[NNewAnm], gx );
						NewAnimation* NANM = &WNewAnm[NNewAnm];
						int z4, dx, dy;
						if ( NANM )
						{
							z = Gscanf( f1, "%d%d%d%d%d%d", &dx, &dy, &z1, &z2, &z3, &z4 );
							if ( z != 6 )NEPar( "weapon.ads", Line, gx, 6 );
							//NANM->StartDx=NAStartDx;
							//NANM->StartDy=NAStartDy;
							//NANM->Parts=NAParts;
							//NANM->PartSize=NAPartSize;
							NANM->Enabled = true;
							NANM->Rotations = z1;
							NANM->ActiveFrame = 0;
							NANM->SoundID = -1;
							NANM->ActivePtX = new short[z1];
							NANM->ActivePtY = new short[z1];
							NANM->LineInfo = nullptr;
							for ( int i = 0; i < z1; i++ )
							{
								NANM->ActivePtX[i] = 0;
								NANM->ActivePtY[i] = 0;
							};
							NANM->TicksPerFrame = 1;
							int dz, nz;
							p1 = RLCRef[z2];
							if ( z3 >= z4 )
							{
								if ( z3*z1 >= RLCNSpr[p1] )
								{
									sprintf( gy, "Weapon.ads,line %d : Reference to RLC sprite is beyond the end of RLC file.", Line );
									ErrM( gy );
								};
								dz = -1;
								nz = z3 - z4 + 1;
							}
							else
							{
								if ( z4*z1 >= RLCNSpr[p1] )
								{
									sprintf( gy, "Weapon.ads,line %d : Reference to RLC sprite is beyond the end of RLC file.", Line );
									ErrM( gy );
								};
								dz = 1;
								nz = z4 - z3 + 1;


							};
							NANM->NFrames = nz;
							NANM->Frames = new NewFrame[nz];
							for ( int i = 0; i < nz; i++ )
							{
								NewFrame* NF = &NANM->Frames[i];
								NF->FileID = p1;
								NF->SpriteID = z3;
								z3 += dz;
								NF->dx = dx;
								NF->dy = dy;
							};
							NNewAnm++;
							NLine( f1 );
							Line++;
						}
						else
						{
							gx[0] = '@';
							sprintf( gy, "Weapon.ads,line %d : Unknown animation type: %s", Line, gx );
							ErrM( gy );
						};
					}
					else
						if ( gx[0] == '#' )
						{
							//Animation;
							WAnmNames[NNewAnm] = new char[strlen( gx ) + 1];
							strcpy( WAnmNames[NNewAnm], gx );
							NewAnimation* NANM = &WNewAnm[NNewAnm];
							z = Gscanf( f1, "%d%d", &z1, &z2 );
							if ( z != 2 )IncPar( "weapon.ads", Line, gx );
							NANM->Enabled = true;
							NANM->NFrames = z2;
							NANM->Rotations = z1;
							NANM->ActiveFrame = 0xFF;
							NANM->ActivePtX = new short[z1];
							NANM->ActivePtY = new short[z1];
							NANM->SoundID = -1;
							NANM->LineInfo = nullptr;
							for ( int i = 0; i < z1; i++ )
							{
								NANM->ActivePtX[i] = 0;
								NANM->ActivePtY[i] = 0;
							};
							NANM->TicksPerFrame = 1;
							NANM->Frames = new NewFrame[z2];
							for ( int i = 0; i < z2; i++ )
							{
								z = Gscanf( f1, "%d%d", &z1, &z3 );
								if ( z != 2 )IncPar( "weapon.ads", Line, gx );
								if ( z1 > MaxRLC )
								{
									sprintf( gy, "Weapon.ads,line %d : Incorrect RLC reference: %d", Line, z1 );
									ErrM( gy );
								};
								NewFrame* NF = &NANM->Frames[i];
								NF->FileID = RLCRef[z1];
								if ( z3 >= RLCNSpr[RLCRef[z1]] )
								{
									sprintf( gy, "Weapon.ads,line %d : Reference to RLC sprite is beyond the end of file: %d", Line, z3 );
									ErrM( gy );
								};
								NF->SpriteID = z3;
								NF->dx = RLCdx[z1];
								NF->dy = RLCdy[z1];
							};
							NNewAnm++;
							NLine( f1 );
							Line++;
						}
						else
							if ( gx[0] == '%' )
							{
								//Animation;
								int ddx, ddy;
								WAnmNames[NNewAnm] = new char[strlen( gx ) + 1];
								strcpy( WAnmNames[NNewAnm], gx );
								NewAnimation* NANM = &WNewAnm[NNewAnm];
								z = Gscanf( f1, "%d%d%d%d", &ddx, &ddy, &z1, &z2 );
								if ( z != 4 )IncPar( "weapon.ads", Line, gx );
								NANM->Enabled = true;
								NANM->NFrames = z2;
								NANM->Rotations = z1;
								NANM->ActiveFrame = 0xFF;
								NANM->HotFrame = 0;
								NANM->SoundID = -1;
								NANM->ActivePtX = new short[z1];
								NANM->ActivePtY = new short[z1];
								NANM->LineInfo = nullptr;
								for ( int i = 0; i < z1; i++ )
								{
									NANM->ActivePtX[i] = 0;
									NANM->ActivePtY[i] = 0;
								};
								NANM->TicksPerFrame = 1;
								NANM->Frames = new NewFrame[z2];
								for ( int i = 0; i < z2; i++ )
								{
									z = Gscanf( f1, "%d%d", &z1, &z3 );
									if ( z != 2 )IncPar( "weapon.ads", Line, gx );
									if ( z1 > MaxRLC )
									{
										sprintf( gy, "Weapon.ads,line %d : Incorrect RLC reference: %d", Line, z1 );
										ErrM( gy );
									};
									NewFrame* NF = &NANM->Frames[i];
									NF->FileID = RLCRef[z1];
									if ( z3 >= RLCNSpr[RLCRef[z1]] )
									{
										sprintf( gy, "Weapon.ads,line %d : Reference to RLC sprite is beyond the end of file: %d", Line, z3 );
										ErrM( gy );
									};
									NF->SpriteID = z3;
									NF->dx = ddx;
									NF->dy = ddy;
								};
								NNewAnm++;
								NLine( f1 );
								Line++;
							}
							else
							{
								//Simple command
								if ( !strcmp( gx, "USERLC" ) )
								{
									z = Gscanf( f1, "%d%s%s%d%d", &p1, gy, gz, &p2, &p3 );
									if ( z != 5 )IncPar( "weapon.ads", Line, "USERLC" );
									if ( p1 > MaxRLC )MaxRLC = p1;
									UpConv( gy );
									int nr = GPS.PreLoadGPImage( gy, !strcmp( gz, "SHADOW" ) );
									RLCRef[p1] = nr;
									RLCdx[p1] = p2;
									RLCdy[p1] = p3;
									NLine( f1 );
									Line++;
								}
								else
									if ( !strcmp( gx, "SOUND" ) )
									{
										z = Gscanf( f1, "%d%s%s", &p1, gy, Fn );
										if ( z != 3 )IncPar( "Weapon.ads", Line, gx );
										NewAnimation* nan = GetNewAnimationByName( gy );
										if ( nan )
										{
											nan->SoundID = SearchStr( SoundID, Fn, NSounds );
											if ( nan->SoundID == -1 )
											{
												sprintf( gz, "%s,line %d :%s: Unknown sound:%s", "Weapon.ads", Line, gx, Fn );
												ErrM( gz );
											};
											nan->HotFrame = p1;
										}
										else
										{
											sprintf( gz, "%s,line %d :%s: Unknown animation:%s", "Weapon.ads", Line, gx, gy );
											ErrM( gz );
										};
									}
									else
									{
										sprintf( gy, "Weapon.ads,line %d : Unknown command : %s", Line, gx );
										ErrM( gy );
									};
							};
			}
			else
			{
				if ( !strcmp( gx, "[END]" ) )finish = true;
				NLine( f1 );
				Line++;
			};
		} while ( ( !finish ) && z != -1 );
		Gclose( f1 );
		Shar = GetNewAnimationByName( "#SHAR" );
		if ( !Shar )
		{
			//ErrM("Could not find special animation:#SHAR");
		};
		SharBuild = GetNewAnimationByName( "#SHARBUILD" );
		if ( !SharBuild )
		{
			//ErrM("Could not find special animation:#SHARBUILD");
		};
		return;
	}
	else
	{
		if ( !f1 )ErrM( "Could not open Weapon.ads" );
		return;
	};

};
NewAnimation* GetNewAnimationByName( char* Name )
{
	for ( int i = 0; i < NNewAnm; i++ )
		if ( !strcmp( WAnmNames[i], Name ) )return &WNewAnm[i];
	return nullptr;
};
//-----------Loading new monsters discriptions----------
NewAnimation* NewMonster::LoadNewAnimationByName( char* gx )
{
	NewAnimation* NANM = nullptr;
	if ( !strcmp( gx, "#MOTION_L" ) )NANM = &MotionL; else
		if ( !strcmp( gx, "#MOTION_R" ) )NANM = &MotionR; else
			if ( !strcmp( gx, "#MOTION_LB" ) )NANM = &MotionLB; else
				if ( !strcmp( gx, "#MOTION_RB" ) )NANM = &MotionRB; else
					if ( !strcmp( gx, "#MINISTEP_L" ) )NANM = &MiniStepL; else
						if ( !strcmp( gx, "#MINISTEP_R" ) )NANM = &MiniStepL; else
							if ( !strcmp( gx, "#MINISTEP_LB" ) )NANM = &MiniStepL; else
								if ( !strcmp( gx, "#MINISTEP_RB" ) )NANM = &MiniStepL; else
									if ( !strcmp( gx, "#ATTACK" ) )NANM = &Attack[0]; else
										if ( !strcmp( gx, "#PATTACK" ) )NANM = &PAttack[0]; else
											if ( !strcmp( gx, "#UATTACK" ) )NANM = &UAttack[0]; else
												if ( !strcmp( gx, "#DEATH" ) )NANM = &Death; else
													if ( !strcmp( gx, "#DEATHLIE1" ) )NANM = &DeathLie1; else
														if ( !strcmp( gx, "#DEATHLIE2" ) )NANM = &DeathLie2; else
															if ( !strcmp( gx, "#DEATHLIE3" ) )NANM = &DeathLie3; else
																if ( !strcmp( gx, "#STAND" ) )NANM = &Stand; else
																	if ( !strcmp( gx, "#PSTAND" ) )NANM = &PStand[0]; else
																		if ( !strcmp( gx, "#WORK" ) )NANM = &Work; else
																			if ( !strcmp( gx, "#FIST" ) )NANM = &Fist; else
																				if ( !strcmp( gx, "#STANDHI" ) )NANM = &StandHi; else
																					if ( !strcmp( gx, "#STANDLO" ) )NANM = &StandLo; else
																						if ( !strcmp( gx, "#BUILDHI" ) )NANM = &BuildHi; else
																							if ( !strcmp( gx, "#BUILD" ) )NANM = &Build; else
																								if ( !strcmp( gx, "#DAMAGE" ) )NANM = &Damage; else
																									if ( !strcmp( gx, "#TRANS01" ) )NANM = &Trans01; else
																										if ( !strcmp( gx, "#TRANS10" ) )NANM = &Trans10; else
																											if ( !strcmp( gx, "#ATTACK1" ) )NANM = &Attack[1]; else
																												if ( !strcmp( gx, "#PSTAND1" ) )NANM = &PStand[1]; else
																													if ( !strcmp( gx, "#PATTACK0" ) )NANM = &PAttack[0]; else
																														if ( !strcmp( gx, "#UATTACK0" ) )NANM = &UAttack[0]; else
																															if ( !strcmp( gx, "#ATTACK0" ) )NANM = &Attack[0]; else
																																if ( !strcmp( gx, "#PSTAND0" ) )NANM = &PStand[0]; else
																																	if ( !strcmp( gx, "#MOTION_L0" ) )NANM = &PMotionL[0]; else
																																		if ( !strcmp( gx, "#MOTION_R0" ) )NANM = &PMotionR[0]; else
																																			if ( !strcmp( gx, "#MOTION_LB0" ) )NANM = &PMotionLB[0]; else
																																				if ( !strcmp( gx, "#MOTION_RB0" ) )NANM = &PMotionRB[0]; else
																																					if ( !strcmp( gx, "#MINISTEP_L0" ) )NANM = &PMiniStepL[0]; else
																																						if ( !strcmp( gx, "#MINISTEP_R0" ) )NANM = &PMiniStepL[0]; else
																																							if ( !strcmp( gx, "#MINISTEP_LB0" ) )NANM = &PMiniStepL[0]; else
																																								if ( !strcmp( gx, "#MINISTEP_RB0" ) )NANM = &PMiniStepL[0]; else
																																									if ( !strcmp( gx, "#PATTACK1" ) )NANM = &PAttack[1]; else
																																										if ( !strcmp( gx, "#UATTACK1" ) )NANM = &UAttack[1]; else
																																											if ( !strcmp( gx, "#ATTACK1" ) )NANM = &Attack[1]; else
																																												if ( !strcmp( gx, "#PSTAND1" ) )NANM = &PStand[1]; else
																																													if ( !strcmp( gx, "#MOTION_L1" ) )NANM = &PMotionL[1]; else
																																														if ( !strcmp( gx, "#MOTION_R1" ) )NANM = &PMotionR[1]; else
																																															if ( !strcmp( gx, "#MOTION_LB1" ) )NANM = &PMotionLB[1]; else
																																																if ( !strcmp( gx, "#MOTION_RB1" ) )NANM = &PMotionRB[1]; else
																																																	if ( !strcmp( gx, "#MINISTEP_L1" ) )NANM = &PMiniStepL[1]; else
																																																		if ( !strcmp( gx, "#MINISTEP_R1" ) )NANM = &PMiniStepL[1]; else
																																																			if ( !strcmp( gx, "#MINISTEP_LB1" ) )NANM = &PMiniStepL[1]; else
																																																				if ( !strcmp( gx, "#MINISTEP_RB1" ) )NANM = &PMiniStepL[1]; else
																																																					if ( !strcmp( gx, "#PATTACK2" ) )NANM = &PAttack[2]; else
																																																						if ( !strcmp( gx, "#UATTACK2" ) )NANM = &UAttack[2]; else
																																																							if ( !strcmp( gx, "#ATTACK2" ) )NANM = &Attack[2]; else
																																																								if ( !strcmp( gx, "#PSTAND2" ) )NANM = &PStand[2]; else
																																																									if ( !strcmp( gx, "#MOTION_L2" ) )NANM = &PMotionL[2]; else
																																																										if ( !strcmp( gx, "#MOTION_R2" ) )NANM = &PMotionR[2]; else
																																																											if ( !strcmp( gx, "#MOTION_LB2" ) )NANM = &PMotionLB[2]; else
																																																												if ( !strcmp( gx, "#MOTION_RB2" ) )NANM = &PMotionRB[2]; else
																																																													if ( !strcmp( gx, "#MINISTEP_L2" ) )NANM = &PMiniStepL[2]; else
																																																														if ( !strcmp( gx, "#MINISTEP_R2" ) )NANM = &PMiniStepL[2]; else
																																																															if ( !strcmp( gx, "#MINISTEP_LB2" ) )NANM = &PMiniStepL[2]; else
																																																																if ( !strcmp( gx, "#MINISTEP_RB2" ) )NANM = &PMiniStepL[2]; else
																																																																	if ( !strcmp( gx, "#PATTACK3" ) )NANM = &PAttack[3]; else
																																																																		if ( !strcmp( gx, "#UATTACK3" ) )NANM = &UAttack[3]; else
																																																																			if ( !strcmp( gx, "#ATTACK3" ) )NANM = &Attack[3]; else
																																																																				if ( !strcmp( gx, "#PSTAND3" ) )NANM = &PStand[3]; else
																																																																					if ( !strcmp( gx, "#MOTION_L3" ) )NANM = &PMotionL[3]; else
																																																																						if ( !strcmp( gx, "#MOTION_R3" ) )NANM = &PMotionR[3]; else
																																																																							if ( !strcmp( gx, "#MOTION_LB3" ) )NANM = &PMotionLB[3]; else
																																																																								if ( !strcmp( gx, "#MOTION_RB3" ) )NANM = &PMotionRB[3]; else
																																																																									if ( !strcmp( gx, "#MINISTEP_L3" ) )NANM = &PMiniStepL[1]; else
																																																																										if ( !strcmp( gx, "#MINISTEP_R3" ) )NANM = &PMiniStepL[1]; else
																																																																											if ( !strcmp( gx, "#MINISTEP_LB3" ) )NANM = &PMiniStepL[1]; else
																																																																												if ( !strcmp( gx, "#MINISTEP_RB3" ) )NANM = &PMiniStepL[1]; else
																																																																													if ( !strcmp( gx, "#REST" ) )NANM = &Rest; else
																																																																														if ( !strcmp( gx, "#REST1" ) )NANM = &Rest1; else
																																																																															if ( !strcmp( gx, "#ROTATE_R" ) )NANM = &RotateR; else
																																																																																if ( !strcmp( gx, "#ROTATE_L" ) )NANM = &RotateL; else
																																																																																	if ( !strcmp( gx, "#WORKTREE" ) )NANM = &WorkTree; else
																																																																																		if ( !strcmp( gx, "#WORKSTONE" ) )NANM = &WorkStone; else
																																																																																			if ( !strcmp( gx, "#WORKFIELD" ) )NANM = &WorkField; else
																																																																																				if ( !strcmp( gx, "#BUILDLO_0" ) )NANM = &PMotionL[0]; else
																																																																																					if ( !strcmp( gx, "#BUILDLO_1" ) )NANM = &PMotionL[1]; else
																																																																																						if ( !strcmp( gx, "#BUILDLO_2" ) )NANM = &PMotionL[2]; else
																																																																																							if ( !strcmp( gx, "#BUILDLO_3" ) )NANM = &PMotionL[3]; else
																																																																																								if ( !strcmp( gx, "#BUILDHI_0" ) )NANM = &PMotionR[0]; else
																																																																																									if ( !strcmp( gx, "#BUILDHI_1" ) )NANM = &PMotionR[1]; else
																																																																																										if ( !strcmp( gx, "#BUILDHI_2" ) )NANM = &PMotionR[2]; else
																																																																																											if ( !strcmp( gx, "#BUILDHI_3" ) )NANM = &PMotionR[3]; else
																																																																																												if ( !strcmp( gx, "#HIANIMATION" ) )NANM = &Work; else
																																																																																													if ( !strcmp( gx, "#TRANSX3" ) )NANM = &TransX3; else
																																																																																														if ( !strcmp( gx, "#TRANS3X" ) )NANM = &Trans3X;
	return NANM;
};
int GetMatherialType( char* str )
{
	if ( !strcmp( str, "BODY" ) )return 1; else
		if ( !strcmp( str, "STONE" ) )return 2; else
			if ( !strcmp( str, "WOOD" ) )return 4; else
				if ( !strcmp( str, "IRON" ) )return 8; else
					if ( !strcmp( str, "FLY" ) )return 16; else
						if ( !strcmp( str, "BUILDING" ) )return 32; else
							if ( !strcmp( str, "WOOD_BUILDING" ) )return 64; else
								if ( !strcmp( str, "STENA" ) )return 128;
	return -1;
};
int GetWeaponIndex( char* str );
char* GetTextByID( char* ID );
extern bool ProtectionMode;
bool NewMonster::CreateFromFile( char* name )
{
	if ( !name )
	{
		return false;
	}
	NewMonster();
	char Fn[128];
	char gx[128];
	char gy[128];
	char gz[128];
	int MaxRLC = -1;
	word RLCRef[128];
	short RLCdx[128];
	short RLCdy[128];
	byte  RLCParts[128];
	byte  RLCPartSize[128];
	int z1, z2, z3, z4, p1, p2, p3, p4, p5, p6, xx, yy;
	sprintf( Fn, "%s.md", name );
	ProtectionMode = 1;
	GFILE* f1 = Gopen( Fn, "r" );
	ProtectionMode = 0;

	bool finish = false;
	short NAStartDx = 0;
	short NAStartDy = 0;
	byte NAParts = 1;
	byte NAPartSize = 96;
	RectDx = 0;
	RectDy = 3;
	RectLx = 40;
	RectLy = 64;
	ResEff = 100;
	IconFileID = 0xFFFF;
	CostPercent = 100;
	ProduceStages = 64;
	BattleForce = 32;
	AnmUpShift = 16;
	Archer = 0;

	AttackSoundID = -1;
	OrderSoundID = -1;
	SelectSoundID = -1;
	BornSoundID = -1;
	DeathSoundID = -1;
	StrikeSoundID = -1;
	MinRotator = 16;
	UnitRadius = 16;
	Ves = 10;
	for ( int i = 0; i < NAttTypes; i++ )
	{
		Rate[i] = 16;
		WeaponKind[i] = 0;
		DamageDecr[i] = 65535;
	}
	int Line = 1;
	int z = 0;
	Sprite = 0xFFFF;
	SpriteVisual = 0xFFFF;
	MD_File = new char[strlen( name ) + 1];
	strcpy( MD_File, name );
	ConvertToUp( name );

	sprintf( gx, "%s.MD", name );
	Message = GetTextByID( gx );
	if ( Message == gx )
	{
		sprintf( gy, "Could not find name for monster %s", gx );
		ErrM( gy );
	}

	if ( f1 )
	{
		do
		{
			z = Gscanf( f1, "%s", gx );
			if ( z != 1 )
			{
				IncPar( name, Line, "COMMAND" );
			}
			if ( gx[0] != '/'&&gx[0] != '[' )
			{
				if ( gx[0] == '#' )
				{
					//Animation;
					NewAnimation* NANM = LoadNewAnimationByName( gx );
					if ( NANM )
					{
						z = Gscanf( f1, "%d%d", &z1, &z2 );
						if ( z != 2 )
						{
							IncPar( name, Line, gx );
						}
						NANM->StartDx = NAStartDx;
						NANM->StartDy = NAStartDy;
						NANM->Parts = NAParts;
						NANM->PartSize = NAPartSize;
						NANM->Enabled = true;
						NANM->NFrames = z2;
						NANM->Rotations = z1;
						int nrot = z1;
						NANM->ActiveFrame = 0;
						NANM->ActivePtX = new short[z1];
						NANM->ActivePtY = new short[z1];
						NANM->HotFrame = 0;
						NANM->SoundID = -1;
						NANM->LineInfo = nullptr;
						for ( int i = 0; i < z1; i++ )
						{
							NANM->ActivePtX[i] = 0;
							NANM->ActivePtY[i] = 0;
						}
						NANM->TicksPerFrame = 1;
						NANM->Frames = new NewFrame[z2];
						for ( int i = 0; i < z2; i++ )
						{
							z = Gscanf( f1, "%d%d", &z1, &z3 );
							if ( z != 2 )
							{
								IncPar( name, Line, gx );
							}
							if ( z1 > MaxRLC )
							{
								sprintf( gy, "%s,line %d : Incorrect RLC reference: %d", name, Line, z1 );
								ErrM( gy );
							}
							NewFrame* NF = &NANM->Frames[i];
							NF->FileID = RLCRef[z1];
							if ( ( z3 + 1 )*nrot > RLCNSpr[RLCRef[z1]] )
							{
								sprintf( gy, "%s,line %d : Reference to RLC sprite is beyond the end of file: %d", name, Line, z3 );
								ErrM( gy );
							}
							NF->SpriteID = z3;
							NF->dx = RLCdx[z1];
							NF->dy = RLCdy[z1];
						}
						NLine( f1 );
						Line++;
					}
					else
					{
						sprintf( gy, "%s,line %d : Unknown animation type: %s", name, Line, gx );
						ErrM( gy );
					}
				}
				else
				{
					if ( gx[0] == '@' )
					{
						//Animation;
						gx[0] = '#';
						NewAnimation* NANM = LoadNewAnimationByName( gx );
						if ( NANM )
						{
							z = Gscanf( f1, "%d%d%d%d", &z1, &z2, &z3, &z4 );
							if ( z != 4 )
							{
								NEPar( name, Line, gx, 4 );
							}
							NANM->StartDx = NAStartDx;
							NANM->StartDy = NAStartDy;
							NANM->Parts = NAParts;
							NANM->PartSize = NAPartSize;
							NANM->Enabled = true;
							NANM->Rotations = z1;
							NANM->ActiveFrame = 0;
							NANM->HotFrame = 0;
							NANM->SoundID = -1;
							NANM->ActivePtX = new short[z1];
							NANM->ActivePtY = new short[z1];
							NANM->LineInfo = nullptr;
							for ( int i = 0; i < z1; i++ )
							{
								NANM->ActivePtX[i] = 0;
								NANM->ActivePtY[i] = 0;
							}
							NANM->TicksPerFrame = 1;
							int dz, nz;
							p1 = RLCRef[z2];
							if ( z3 >= z4 )
							{
								if ( ( z3 + 1 )*z1 > RLCNSpr[p1] )
								{
									sprintf( gy, "%s,line %d : Reference to RLC sprite is beyond the end of RLC file.", name, Line );
									ErrM( gy );
								}
								dz = -1;
								nz = z3 - z4 + 1;
							}
							else
							{
								if ( ( z4 + 1 )*z1 > RLCNSpr[p1] )
								{
									sprintf( gy, "%s,line %d : Reference to RLC sprite is beyond the end of RLC file.", name, Line );
									ErrM( gy );
								}
								dz = 1;
								nz = z4 - z3 + 1;


							}
							NANM->NFrames = nz;
							NANM->Frames = new NewFrame[nz];
							for ( int i = 0; i < nz; i++ )
							{
								NewFrame* NF = &NANM->Frames[i];
								NF->FileID = p1;
								NF->SpriteID = z3;
								z3 += dz;
								NF->dx = RLCdx[z2];
								NF->dy = RLCdy[z2];
							}
							NLine( f1 );
							Line++;
						}
						else
						{
							gx[0] = '@';
							sprintf( gy, "%s,line %d : Unknown animation type: %s", name, Line, gx );
							ErrM( gy );
						}
					}
					else
					{
						int Line0 = Line;
						//Simple command
						if ( !strcmp( gx, "FARM" ) )
						{
							NInFarm = UnitsPerFarm;
							Farm = true;
							NLine( f1 );
							Ves = 50;
							Line++;
						}
						else
							if ( !strcmp( gx, "ARCHER" ) )
							{
								Archer = 1;
								Line++;
								NLine( f1 );
							}
							else
								if ( !strcmp( gx, "FORCE" ) )
								{
									z = Gscanf( f1, "%d", &p1 );
									if ( z != 1 )IncPar( name, Line, gx );
									Force = p1;
									Line++;
									NLine( f1 );
								}
								else
									if ( !strcmp( gx, "UNITRADIUS" ) )
									{
										z = Gscanf( f1, "%d", &p1 );
										if ( z != 1 )IncPar( name, Line, gx );
										UnitRadius = p1;
										Line++;
										NLine( f1 );
									}
									else
										if ( !strcmp( gx, "ANMSH" ) )
										{
											z = Gscanf( f1, "%d", &p1 );
											if ( z != 1 )IncPar( name, Line, gx );
											AnmUpShift = p1;
											Line++;
											NLine( f1 );
										}
										else
											if ( !strcmp( gx, "FORWARD" ) )
											{
												ShotForward = true;
												NLine( f1 );
												Line++;
											}
											else
												if ( !strcmp( gx, "OFFICER" ) )
												{
													Officer = true;
													NLine( f1 );
													Line++;
												}
												else
													if ( !strcmp( gx, "BARABAN" ) )
													{
														Baraban = true;
														NLine( f1 );
														Line++;
													}
													else
														if ( !strcmp( gx, "NO_HUNGRY" ) )
														{
															NotHungry = true;
															NLine( f1 );
															Line++;
														}
														else
															if ( !strcmp( gx, "MFARM" ) )
															{
																z = Gscanf( f1, "%d", &p1 );
																if ( z != 1 )IncPar( name, Line, gx );
																NInFarm = p1;
																Farm = true;
																NLine( f1 );
																Line++;
															}
															else
															{
																if ( !strcmp( gx, "USAGE" ) )
																{
																	z = Gscanf( f1, "%s", &gy );
																	if ( z != 1 )
																	{
																		IncPar( name, Line, gx );
																	}
																	if ( !strcmp( gy, FarmIDS ) )
																	{
																		Usage = FarmID;
																	}
																	else
																	{
																		if ( !strcmp( gy, MelnicaIDS ) )
																		{
																			Usage = MelnicaID;
																			Ves = 100;
																		}
																		else
																		{
																			if ( !strcmp( gy, SkladIDS ) )
																			{
																				Usage = SkladID;
																				Ves = 30;
																			}
																			else
																			{
																				if ( !strcmp( gy, TowerIDS ) )Usage = TowerID; else
																					if ( !strcmp( gy, CenterIDS ) )Usage = CenterID; else
																						if ( !strcmp( gy, MineIDS ) )Usage = MineID; else
																							if ( !strcmp( gy, FieldIDS ) )Usage = FieldID; else
																								if ( !strcmp( gy, PeasantIDS ) )Usage = PeasantID; else
																									if ( !strcmp( gy, FastHorseIDS ) )Usage = FastHorseID;
																									else
																									{
																										if ( !strcmp( gy, MortiraIDS ) )
																										{
																											Usage = MortiraID;
																											Artilery = true;
																											//AttBuild=true;
																										}
																										else
																											if ( !strcmp( gy, PushkaIDS ) )
																											{
																												Usage = PushkaID;
																												Artilery = true;
																												//AttBuild=true;
																											}
																											else
																												if ( !strcmp( gy, MultiCannonIDS ) )
																												{
																													Usage = MultiCannonID;
																													Artilery = true;
																													//AttBuild=true;
																												}
																												else
																												{
																													if ( !strcmp( gy, GrenaderIDS ) )Usage = GrenaderID; else
																														if ( !strcmp( gy, HardWallIDS ) )Usage = HardWallID; else
																															if ( !strcmp( gy, WeakWallIDS ) )Usage = WeakWallID; else
																																if ( !strcmp( gy, LinkorIDS ) )Usage = LinkorID; else
																																	if ( !strcmp( gy, WeakIDS ) )Usage = WeakID; else
																																		if ( !strcmp( gy, FisherIDS ) )Usage = FisherID; else
																																			if ( !strcmp( gy, ArtDepoIDS ) )Usage = ArtDepoID; else
																																				if ( !strcmp( gy, SupMortIDS ) )
																																				{
																																					Usage = SupMortID;
																																					Artilery = true;
																																					//AttBuild=true;
																																				}
																																				else
																																				{
																																					if ( !strcmp( gy, PortIDS ) )Usage = PortID; else
																																						if ( !strcmp( gy, LightInfIDS ) )Usage = LightInfID; else
																																							if ( !strcmp( gy, StrelokIDS ) )Usage = StrelokID; else
																																								if ( !strcmp( gy, HardHorceIDS ) )Usage = HardHorceID; else
																																									if ( !strcmp( gy, HorseStrelokIDS ) )Usage = HorseStrelokID; else
																																										if ( !strcmp( gy, FregatIDS ) )Usage = FregatID; else
																																											if ( !strcmp( gy, GaleraIDS ) )Usage = GaleraID; else
																																												if ( !strcmp( gy, IaxtaIDS ) )Usage = IaxtaID; else
																																													if ( !strcmp( gy, ShebekaIDS ) )Usage = ShebekaID; else
																																														if ( !strcmp( gy, ParomIDS ) )Usage = ParomID; else
																																															if ( !strcmp( gy, ArcherIDS ) )Usage = ArcherID;
																																															else IncPar( name, Line, gx );
																																				}
																												}
																									}
																			}
																		}
																	}
																	NLine( f1 );
																	Line++;
																}
																else
																{
																	if ( !strcmp( gx, "RESSUBST" ) )
																	{
																		ResSubst = 1;
																		NLine( f1 );
																		Line++;
																	}
																	else
																	{
																		if ( !strcmp( gx, "PRIEST" ) )
																		{
																			Priest = 1;
																			NLine( f1 );
																			Line++;
																		}
																		else
																		{
																			if ( !strcmp( gx, "FISHING" ) )
																			{
																				z = Gscanf( f1, "%d%d", &p1, &p2 );
																				if ( z != 2 )IncPar( name, Line, gx );
																				FishSpeed = p1;
																				FishAmount = p2;
																				NLine( f1 );
																				Line++;
																			}
																			else
																			{
																				if ( !strcmp( gx, "VESACCEL" ) )
																				{
																					z = Gscanf( f1, "%d%d", &p1, &p2 );
																					if ( z != 2 )IncPar( name, Line, gx );
																					VesStart = p1;
																					VesFin = p2;
																					NLine( f1 );
																					Line++;
																				}
																				else
																				{
																					if ( !strcmp( gx, "ORDER_SOUND" ) )
																					{
																						z = Gscanf( f1, "%s", gy );
																						OrderSoundID = SearchStr( SoundID, gy, NSounds );
																						if ( OrderSoundID == -1 )
																						{
																							sprintf( gz, "%s,line %d:%s: Unknown sound:%s", name, Line, gx, gy );
																						};
																						Line++;
																						NLine( f1 );
																					}
																					else
																					{
																						if ( !strcmp( gx, "SELECT_SOUND" ) )
																						{
																							z = Gscanf( f1, "%s", gy );
																							SelectSoundID = SearchStr( SoundID, gy, NSounds );
																							if ( SelectSoundID == -1 )
																							{
																								sprintf( gz, "%s,line %d:%s: Unknown sound:%s", name, Line, gx, gy );
																							};
																							Line++;
																							NLine( f1 );
																						}
																						else
																						{
																							if ( !strcmp( gx, "ATTACK_SOUND" ) )
																							{
																								z = Gscanf( f1, "%s", gy );
																								AttackSoundID = SearchStr( SoundID, gy, NSounds );
																								if ( AttackSoundID == -1 )
																								{
																									sprintf( gz, "%s,line %d:%s: Unknown sound:%s", name, Line, gx, gy );
																								};
																								Line++;
																								NLine( f1 );
																							}
																							else
																							{
																								if ( !strcmp( gx, "BORN_SOUND" ) )
																								{
																									z = Gscanf( f1, "%s", gy );
																									BornSoundID = SearchStr( SoundID, gy, NSounds );
																									if ( BornSoundID == -1 )
																									{
																										sprintf( gz, "%s,line %d:%s: Unknown sound:%s", name, Line, gx, gy );
																									};
																									Line++;
																									NLine( f1 );
																								}
																								else
																								{
																									if ( !strcmp( gx, "AUTO_NO_ATTACK" ) )
																									{
																										AutoNoAttack = 1;
																										Line++;
																										NLine( f1 );
																									}
																									else
																									{
																										if ( !strcmp( gx, "STANDGROUND" ) )
																										{
																											CanStandGr = 1;
																											Line++;
																											NLine( f1 );
																										}
																										else
																										{
																											if ( !strcmp( gx, "AUTO_STAND_GROUND" ) )
																											{
																												AutoStandGround = 1;
																												Line++;
																												NLine( f1 );
																											}
																											else
																											{
																												if ( !strcmp( gx, "FASTUNIT" ) )
																												{
																													z = Gscanf( f1, "%d", &p1 );
																													MaxAIndex = p1;
																													Line++;
																													NLine( f1 );
																												}
																												else
																												{
																													if ( !strcmp( gx, "DEATH_SOUND" ) )
																													{
																														z = Gscanf( f1, "%s", gy );
																														DeathSoundID = SearchStr( SoundID, gy, NSounds );
																														if ( DeathSoundID == -1 )
																														{
																															sprintf( gz, "%s,line %d:%s: Unknown sound:%s", name, Line, gx, gy );
																														}
																														Line++;
																														NLine( f1 );
																													}
																													else
																													{
																														if ( !strcmp( gx, "STRIKE_SOUND" ) )
																														{
																															z = Gscanf( f1, "%s", gy );
																															StrikeSoundID = SearchStr( SoundID, gy, NSounds );
																															if ( StrikeSoundID == -1 )
																															{
																																sprintf( gz, "%s,line %d:%s: Unknown sound:%s", name, Line, gx, gy );
																															}
																															Line++;
																															NLine( f1 );
																														}
																														else
																														{
																															if ( !strcmp( gx, "WAVES" ) )
																															{
																																z = Gscanf( f1, "%d%d%d%d", &p5, &p6, &p1, &p2 );
																																if ( z != 4 )
																																{
																																	IncPar( name, Line, gx );
																																}
																																WaveDZ = p1;
																																NWaves = p2;
																																WavePoints = new short[p2 * 2];
																																for ( int i = 0; i < p2; i++ )
																																{
																																	z = Gscanf( f1, "%d%d", &p3, &p4 );
																																	if ( z != 2 )
																																	{
																																		IncPar( name, Line, gx );
																																	}
																																	WavePoints[i + i] = p3 - p5;
																																	WavePoints[i + i + 1] = ( p4 - p6 - p1 ) << 1;
																																}
																																NLine( f1 );
																																Line++;
																															}
																															else
																															{
																																if ( !strcmp( gx, "COSTPERCENT" ) )
																																{
																																	z = Gscanf( f1, "%d", &p1 );
																																	if ( z != 1 )
																																	{
																																		IncPar( name, Line, gx );
																																	}
																																	CostPercent = p1;
																																	NLine( f1 );
																																	Line++;
																																}
																																else
																																{
																																	if ( !strcmp( gx, "TORG" ) )
																																	{
																																		Rinok = true;
																																		NLine( f1 );
																																		Line++;
																																	}
																																	else
																																	{
																																		if ( !strcmp( gx, "BATTLEFORCE" ) )
																																		{
																																			z = Gscanf( f1, "%d", &p1 );
																																			if ( z != 1 )
																																			{
																																				IncPar( name, Line, gx );
																																			}
																																			BattleForce = p1;
																																			NLine( f1 );
																																			Line++;
																																		}
																																		else
																																		{
																																			if ( !strcmp( gx, "SLOWDEATH" ) )
																																			{
																																				SlowDeath = true;
																																				NLine( f1 );
																																				Line++;
																																			}
																																		}
																																	}
																																}
																															}
																														}
																													}
																												}
																											}
																										}
																									}
																								}
																							}
																						}
																					}
																				}
																			}
																		}
																	}
																}
															}
						if ( Line0 == Line )
							if ( !strcmp( gx, "VISION" ) )
							{
								z = Gscanf( f1, "%d", &p1 );
								if ( p1 < 0 || p1>8 )
								{
									sprintf( gy, "%s, Line %d :%s: Argument must be 0..8", name, Line, gx );
									ErrM( gy );
								};
								if ( z != 1 )IncPar( name, Line, gx );
								VisionType = p1;
								NLine( f1 );
								Line++;
							}
							else
								if ( !strcmp( gx, "ROTATE" ) )
								{
									z = Gscanf( f1, "%d", &p1 );
									if ( z != 1 )
									{
										sprintf( gy, "%s, Line %d : Invalid %s value", name, Line, gx );
										ErrM( gy );
									};
									MinRotator = p1;
									NLine( f1 );
									Line++;
								}
								else
									if ( !strcmp( gx, "RAZBROS" ) )
									{
										z = Gscanf( f1, "%d", &p1 );
										if ( z != 1 )IncPar( name, Line, gx );
										Razbros = p1;
										NLine( f1 );
										Line++;
									}
									else
										if ( !strcmp( gx, "VES" ) )
										{
											z = Gscanf( f1, "%d", &p1 );
											if ( z != 1 )IncPar( name, Line, gx );
											Ves = p1;
											NLine( f1 );
											Line++;
										}
										else
											if ( !strcmp( gx, "EXPLRADIUS" ) )
											{
												z = Gscanf( f1, "%d", &p1 );
												if ( z != 1 )IncPar( name, Line, gx );
												ExplRadius = p1;
												NLine( f1 );
												Line++;
											}
											else
												if ( !strcmp( gx, "RESCONSUMER" ) )
												{
													z = Gscanf( f1, "%s%d", gy, &p2 );
													if ( z != 2 )IncPar( name, Line, gx );
													p1 = GetResID( gy );
													if ( p1 == -1 )
													{
														sprintf( gz, "%s , Line %d :%s: Unknown resource ID:%s", name, Line, gx, gy );
														ErrM( gz );
													};
													ResConsID = p1;
													ResConsumer = p2;
													NLine( f1 );
													Line++;
												}
												else
													if ( !strcmp( gx, "NAEMNIK" ) )
													{
														Behavior = 2;
														NLine( f1 );
														Line++;
													}
													else
														if ( !strcmp( gx, "TURNOFF" ) )
														{
															Behavior = 1;
															NLine( f1 );
															Line++;
														}
														else
															if ( !strcmp( gx, "SOUND" ) )
															{
																z = Gscanf( f1, "%d%s%s", &p1, gy, Fn );
																if ( z != 3 )IncPar( name, Line, gx );
																NewAnimation* nan = LoadNewAnimationByName( gy );
																if ( nan )
																{
																	nan->SoundID = SearchStr( SoundID, Fn, NSounds );
																	if ( nan->SoundID == -1 )
																	{
																		sprintf( gz, "%s,line %d :%s: Unknown sound:%s", name, Line, gx, Fn );
																		ErrM( gz );
																	};
																	nan->HotFrame = p1;
																}
																else
																{
																	sprintf( gz, "%s,line %d :%s: Unknown animation:%s", name, Line, gx, gy );
																	ErrM( gz );
																};
															}
															else
																if ( !strcmp( gx, "SHOWDELAY" ) )
																{
																	ShowDelay = true;
																	NLine( f1 );
																	Line++;
																}
																else
																	if ( !strcmp( gx, "ARTPODGOTOVKA" ) )
																	{
																		Artpodgotovka = 1;
																		NLine( f1 );
																		Line++;
																	}
																	else
																		if ( !strcmp( gx, "NOFULLDESTRUCT" ) )
																		{
																			NoDestruct = 1;
																			NLine( f1 );
																			Line++;
																		}
																		else
																			if ( !strcmp( gx, "CAPTURE" ) )
																			{
																				Capture = true;
																				NLine( f1 );
																				Line++;
																			}
																			else
																				if ( !strcmp( gx, "RASTRATA_NA_VISTREL" ) )
																				{
																					z = Gscanf( f1, "%d%d", &p4, &p1 );
																					if ( z != 2 )IncPar( name, Line, gx );
																					ShotRes = new word[p1 * 2];
																					NShotRes = p1;
																					for ( int i = 0; i < p1; i++ )
																					{
																						z = Gscanf( f1, "%s%d", gy, &p2 );
																						if ( z != 2 )IncPar( name, Line, gx );
																						p3 = GetResByName( gy );
																						if ( p3 >= 0 && p3 < 100 )
																						{
																							ShotRes[i * 2] = p3;
																							ShotRes[i * 2 + 1] = p2;
																						}
																						else
																						{
																							sprintf( gz, "%s,line %d :%s: Unknown resource:%s", name, Line, gx, gy );
																							ErrM( gz );
																						};
																					};
																					ResAttType = p4;
																					ResAttType1 = p4;
																					NLine( f1 );
																					Line++;
																				}
																				else
																					if ( !strcmp( gx, "RASTRATA_NA_VISTREL2" ) )
																					{
																						z = Gscanf( f1, "%d%d%d", &p4, &p5, &p1 );
																						if ( z != 3 )IncPar( name, Line, gx );
																						ShotRes = new word[p1 * 2];
																						NShotRes = p1;
																						for ( int i = 0; i < p1; i++ )
																						{
																							z = Gscanf( f1, "%s%d", gy, &p2 );
																							if ( z != 2 )IncPar( name, Line, gx );
																							p3 = GetResByName( gy );
																							if ( p3 >= 0 && p3 < 100 )
																							{
																								ShotRes[i * 2] = p3;
																								ShotRes[i * 2 + 1] = p2;
																							}
																							else
																							{
																								sprintf( gz, "%s,line %d :%s: Unknown resource:%s", name, Line, gx, gy );
																								ErrM( gz );
																							};
																						};
																						ResAttType = p4;
																						ResAttType1 = p5;
																						NLine( f1 );
																						Line++;
																					}
																					else
																						if ( !strcmp( gx, "PORT" ) )
																						{
																							z = Gscanf( f1, "%s%d%d", gy, &p1, &p2 );
																							if ( z != 3 )IncPar( name, Line, gx );
																							BuiAnm = GetNewAnimationByName( gy );
																							if ( !BuiAnm )
																							{
																								sprintf( gz, "%s,line %d : Unknown animation: %s", name, Line, gy );
																								ErrM( gz );
																							};
																							BuiDist = p1;
																							MaxPortDist = p2;
																							Port = true;
																							NLine( f1 );
																							Line++;
																						}
																						else
																							if ( !strcmp( gx, "MULTIWEAPON" ) )
																							{
																								int rx, ry;
																								z = Gscanf( f1, "%d%d%d%d", &rx, &ry, &p1, &p2 );
																								if ( z != 4 )IncPar( name, Line, gx );
																								VesloRotX = rx;
																								VesloRotY = ry;
																								MultiWpZ = p1;
																								NMultiWp = p2;
																								MultiWpX = new short[p2];
																								MultiWpY = new short[p2];
																								for ( int i = 0; i < p2; i++ )
																								{
																									z = Gscanf( f1, "%d%d", &p3, &p4 );
																									MultiWpX[i] = ( p3 - rx );
																									MultiWpY[i] = ( p4 - ry + p1 ) << 1;
																								};
																								NLine( f1 );
																								Line++;
																							}
																							else
																								if ( !strcmp( gx, "TWOPARTS" ) )
																								{
																									z = Gscanf( f1, "%s%d", gy, &p1 );
																									if ( z != 2 )IncPar( name, Line, gx );
																									UpperPart = LoadNewAnimationByName( gy );
																									if ( !UpperPart )
																									{
																										sprintf( gz, "%s,Line %d :%s: Unknown animation:%s", name, Line, gx, gy );
																										ErrM( gz );
																									};
																									TwoParts = true;
																									UpperPartShift = p1;
																									NLine( f1 );
																									Line++;
																								}
																								else
																									if ( !strcmp( gx, "BUILDBAR" ) )
																									{
																										z = Gscanf( f1, "%d%d%d%d", &p1, &p2, &p3, &p4 );
																										if ( z != 4 )IncPar( name, Line, gx );
																										BuildX0 = PicDx + ( p1 << 4 );
																										BuildY0 = ( PicDy + ( p2 << 3 ) ) << 1;
																										BuildX1 = PicDx + ( p3 << 4 );
																										BuildY1 = ( PicDy + ( p4 << 3 ) ) << 1;
																										NLine( f1 );
																										Line++;
																									}
																									else
																										if ( !strcmp( gx, "REFLECT" ) )
																										{
																											z = Gscanf( f1, "%s", gy );
																											if ( z != 1 )IncPar( name, Line, gx );
																											Reflection = LoadNewAnimationByName( gy );
																											if ( !Reflection )
																											{
																												sprintf( gz, "%s,Line %d :%s: Unknown animation:%s", name, Line, gx, gy );
																												ErrM( gz );
																											};
																										}
																										else
																											if ( !strcmp( gx, "VESLA" ) )
																											{
																												z = Gscanf( f1, "%d%d%s%d", &p1, &p2, gy, &p3 );
																												if ( z != 4 )IncPar( name, Line, gx );
																												Veslo = LoadNewAnimationByName( gy );
																												if ( !Veslo )
																												{
																													sprintf( gz, "%s,Line %d :%s: Unknown animation:%s", name, Line, gx, gy );
																													ErrM( gz );
																												};
																												VesloRotX = p1;
																												VesloRotY = p2;
																												VesloX = new short[p3];
																												VesloY = new short[p3];
																												NVesel = p3;
																												int xx, yy;
																												for ( int i = 0; i < p3; i++ )
																												{
																													z = Gscanf( f1, "%d%d", &xx, &yy );
																													if ( z != 2 )IncPar( name, Line, gx );
																													VesloX[i] = xx - p1;
																													VesloY[i] = ( yy - p2 ) << 1;
																												};
																												NLine( f1 );
																												Line++;
																											}
																											else
																												if ( !strcmp( gx, "LINEARSORT" ) )
																												{
																													z = Gscanf( f1, "%d", &p1 );
																													if ( z != 1 )IncPar( name, Line, gx );
																													LinearLength = p1;
																													NLine( f1 );
																													Line++;
																												}
																												else
																													if ( !strcmp( gx, "GEOMETRY" ) )
																													{
																														z = Gscanf( f1, "%d%d%d", &p1, &p2, &p3 );
																														if ( z != 3 )IncPar( name, Line, "GEOMETRY" );
																														Radius1 = p1 << 4;
																														Radius2 = p2 << 4;
																														MotionDist = p3;
																														for ( int i = 0; i < 256; i++ )
																														{
																															POneStepDX[i] = ( TCos[i] * MotionDist ) >> 4;
																															POneStepDY[i] = ( TSin[i] * MotionDist ) >> 4;
																															OneStepDX[i] = ( TCos[i] * MotionDist ) >> 8;
																															OneStepDY[i] = ( TSin[i] * MotionDist ) >> 8;
																														};
																														NLine( f1 );
																														Line++;
																													}
																													else
																														if ( !strcmp( gx, "LINESORT" ) )
																														{
																															z = Gscanf( f1, "%s", gz );
																															NewAnimation* NANM = LoadNewAnimationByName( gz );
																															if ( NANM )
																															{
																																short* LINF = new short[NANM->NFrames << 2];
																																NANM->LineInfo = LINF;
																																int LoIdx[20];
																																int NLo = 0;
																																int MinY = 10000;
																																int MinX = 10000;
																																int MaxX = -10000;
																																for ( int i = 0; i < NANM->NFrames; i++ )
																																{
																																	int x1, y1, x2, y2;
																																	z = Gscanf( f1, "%s", gy );
																																	int p = i << 2;
																																	if ( !strcmp( gy, "LINE" ) )
																																	{
																																		z = Gscanf( f1, "%d%d%d%d", &x1, &y1, &x2, &y2 );
																																		if ( z != 4 )
																																		{
																																			IncPar( name, Line, gx );
																																		};
																																		LINF[p] = x1;
																																		LINF[p + 1] = y1;
																																		LINF[p + 2] = x2;
																																		LINF[p + 3] = y2;
																																		if ( y1 < MinY )MinY = y1;
																																		if ( x1 < MinX )MinX = x1;
																																		if ( x1 > MaxX )MaxX = x1;
																																		if ( y2 < MinY )MinY = y2;
																																		if ( x2 < MinX )MinX = x2;
																																		if ( x2 > MaxX )MaxX = x2;
																																	}
																																	else
																																		if ( !strcmp( gy, "POINT" ) )
																																		{
																																			z = Gscanf( f1, "%d%d", &x1, &y1 );
																																			if ( z != 2 )
																																			{
																																				IncPar( name, Line, gx );
																																			};
																																			int p = i << 2;
																																			LINF[p] = x1;
																																			LINF[p + 1] = y1;
																																			LINF[p + 2] = x1;
																																			LINF[p + 3] = y1;
																																			if ( y1 < MinY )MinY = y1;
																																			if ( x1 < MinX )MinX = x1;
																																			if ( x1 > MaxX )MaxX = x1;
																																		}
																																		else
																																			if ( !strcmp( gy, "GROUND" ) )
																																			{
																																				LINF[p] = -10000;
																																				LINF[p + 1] = -10000;
																																				LINF[p + 2] = -10000;
																																				LINF[p + 3] = -10000;
																																				if ( NLo < 20 )
																																				{
																																					LoIdx[NLo] = i;
																																					NLo++;
																																				};
																																			}
																																			else
																																				if ( !strcmp( gy, "TOP" ) )
																																				{
																																					LINF[p] = 10000;
																																					LINF[p + 1] = 10000;
																																					LINF[p + 2] = 10000;
																																					LINF[p + 3] = 10000;
																																				}
																																				else
																																				{
																																					sprintf( gy, "%s,line %d : Unknown sort type(LINE,POINT,GROUND or TOP expected): %s", name, Line, gy );
																																					ErrM( gy );
																																				};
																																};
																																if ( NLo )
																																{
																																	MinY = -10;
																																	int avx = ( MinX + MaxX ) >> 1;
																																	for ( int i = 0; i < NLo; i++ )
																																	{
																																		int idx = LoIdx[i] << 2;
																																		LINF[idx] = avx;
																																		LINF[idx + 1] = MinY;
																																		LINF[idx + 2] = avx;
																																		LINF[idx + 3] = MinY;
																																	};
																																};
																															}
																															else
																															{
																																sprintf( gy, "%s,line %d : Unknown animation type: %s", name, Line, gz );
																																ErrM( gy );
																															};
																															NLine( f1 );
																															Line++;
																														}
																														else
																															if ( !strcmp( gx, "INFO" ) )
																															{
																																z = Gscanf( f1, "%d%d", &p1, &p2 );
																																if ( z != 2 )IncPar( name, Line, gx );
																																InfType = p1;
																																PictureID = p2;
																																NLine( f1 );
																																Line++;
																															}
																															else
																																if ( !strcmp( gx, "3DBARS" ) )
																																{
																																	z = Gscanf( f1, "%d", &p1 );
																																	if ( z != 1 )IncPar( name, Line, gx );
																																	NBars = p1;
																																	Bars3D = new short[p1 * 5];
																																	int q = 0;
																																	for ( int i = 0; i < p1; i++ )
																																	{
																																		int XB, YB, L1, L2, Hi;
																																		z = Gscanf( f1, "%d %d %d %d %d", &XB, &YB, &L1, &L2, &Hi );
																																		if ( z != 5 )IncPar( name, Line, gx );
																																		Bars3D[q] = XB;
																																		Bars3D[q + 1] = YB;
																																		Bars3D[q + 2] = L1;
																																		Bars3D[q + 3] = L2;
																																		Bars3D[q + 4] = Hi;
																																		q += 5;
																																	};
																																	Line++;
																																	NLine( f1 );
																																}
																																else
																																	if ( !strcmp( gx, "LOCATION" ) )
																																	{
																																		z = Gscanf( f1, "%d%d%d%d", &p1, &p2, &p3, &p4 );
																																		if ( z != 4 )IncPar( name, Line, "LOCATION" );
																																		PicDx = p1;
																																		PicDy = p2;
																																		PicLx = p3;
																																		PicLy = p4;
																																		NLine( f1 );
																																		Line++;
																																	}
																																	else
																																		if ( !strcmp( gx, "RESOURCEBASE" ) )
																																		{
																																			z = Gscanf( f1, "%d", &p1 );
																																			if ( z != 1 )IncPar( name, Line, "RESOURCEBASE" );
																																			byte ms = 0;
																																			for ( int i = 0; i < p1; i++ )
																																			{
																																				z = Gscanf( f1, "%s", gy );
																																				if ( z != 1 )NEPar( name, Line, "RESOURCEBASE", p1 );
																																				if ( !strcmp( gy, "WOOD" ) )
																																				{
																																					ms |= 1;
																																				}
																																				else
																																					if ( !strcmp( gy, "GOLD" ) )
																																					{
																																						ms |= 2;
																																					}
																																					else
																																						if ( !strcmp( gy, "STONE" ) )
																																						{
																																							ms |= 4;
																																						}
																																						else
																																							if ( !strcmp( gy, "FOOD" ) )
																																							{
																																								ms |= 8;
																																							}
																																							else
																																							{
																																								InvPar( name, Line, "RESOURCEBASE", gy );
																																							};
																																				ResConcentrator = ms;
																																			};
																																			NLine( f1 );
																																			Line++;
																																		}
																																		else
																																			if ( !strcmp( gx, "PORTION" ) )
																																			{
																																				z = Gscanf( f1, "%d", &p1 );
																																				if ( z != 1 )IncPar( name, Line, "PORTION" );
																																				for ( int i = 0; i < p1; i++ )
																																				{
																																					z = Gscanf( f1, "%s%d", gy, &p2 );
																																					if ( z != 2 )NEPar( name, Line, "PORTION", p1 );
																																					byte ms = 0;
																																					if ( !strcmp( gy, "WOOD" ) )
																																					{
																																						ms = 0;
																																					}
																																					else
																																						if ( !strcmp( gy, "GOLD" ) )
																																						{
																																							ms = 1;
																																						}
																																						else
																																							if ( !strcmp( gy, "STONE" ) )
																																							{
																																								ms = 2;
																																							}
																																							else
																																								if ( !strcmp( gy, "FOOD" ) )
																																								{
																																									ms = 3;
																																								}
																																								else
																																								{
																																									InvPar( name, Line, "PORTION", gy );
																																								};
																																					MaxResPortion[ms] = p2;
																																				};
																																				NLine( f1 );
																																				Line++;
																																			}
																																			else
																																				if ( !strcmp( gx, "MEDIA" ) )
																																				{
																																					z = Gscanf( f1, "%s", gy );
																																					if ( !strcmp( gy, "LAND" ) )LockType = 0;
																																					else if ( !strcmp( gy, "WATER" ) )LockType = 1;
																																					else
																																					{
																																						sprintf( gz, "%s, Line %d :%s: Unknown media: %s", name, Line, gx, gy );
																																						ErrM( gz );
																																					};
																																					NLine( f1 );
																																					Line++;
																																				}
																																				else
																																					if ( !strcmp( gx, "MOTIONSTYLE" ) )
																																					{
																																						z = Gscanf( f1, "%s", gy );
																																						if ( !strcmp( gy, "FASTROTATE&MOVE" ) )MotionStyle = 0;
																																						else if ( !strcmp( gy, "SLOWROTATE" ) )MotionStyle = 1;
																																						else if ( !strcmp( gy, "SHEEPS" ) )MotionStyle = 2;
																																						else
																																						{
																																							sprintf( gz, "%s, Line %d :%s: Unknown media: %s", name, Line, gx, gy );
																																							ErrM( gz );
																																						};
																																						NLine( f1 );
																																						Line++;
																																					}
																																					else
																																						if ( !strcmp( gx, "WATERROUND" ) )
																																						{
																																							WaterActive = true;
																																							NLine( f1 );
																																							Line++;
																																						}
																																						else
																																							if ( !strcmp( gx, "SPRITES" ) )
																																							{
																																								z = Gscanf( f1, "%s%s", gy, gz );
																																								p1 = COMPLEX.GetIndexByName( gy );
																																								p2 = COMPLEX.GetIndexByName( gz );
																																								if ( p1 == -1 )
																																								{
																																									sprintf( Fn, "%s, line %d : %s : Unknown Sprite Object : %s", name, Line, gx, gy );
																																									ErrM( Fn );
																																								};
																																								if ( p2 == -1 )
																																								{
																																									sprintf( Fn, "%s, line %d : %s : Unknown Sprite Object : %s", name, Line, gx, gz );
																																									ErrM( Fn );
																																								};
																																								Sprite = p1;
																																								SpriteVisual = p2;
																																								NLine( f1 );
																																								Line++;
																																							}
																																							else
																																								if ( !strcmp( gx, "EXPLMEDIA" ) )
																																								{
																																									z = Gscanf( f1, "%s%d", gy, &p1 );
																																									if ( z != 2 )NEPar( name, Line, gx, 2 );
																																									p2 = GetExMedia( gy );
																																									if ( p2 == -1 )
																																									{
																																										sprintf( gz, "%s , line %d : %s :Unknown media", name, Line, gy );
																																										ErrM( gz );
																																									};
																																									ExplosionMedia = p2;
																																									EMediaRadius = p1;
																																									NLine( f1 );
																																									Line++;
																																								}
																																								else
																																									if ( !strcmp( gx, "SHOTS" ) )
																																									{
																																										z = Gscanf( f1, "%d", &p1 );
																																										if ( z != 1 )IncPar( name, Line, gx );
																																										NShotPt = p1;
																																										ShotPtX = new short[p1];
																																										ShotPtY = new short[p1];
																																										ShotDir = 0;
																																										ShotDir = 0;
																																										for ( int i = 0; i < p1; i++ )
																																										{
																																											z = Gscanf( f1, "%d%d", &p2, &p3 );
																																											if ( z != 2 )IncPar( name, Line, gx );
																																											ShotPtX[i] = p2;
																																											ShotPtY[i] = p3;
																																										};
																																										NLine( f1 );
																																										Line++;
																																									}
																																									else
																																										if ( !strcmp( gx, "EX_SHOTS" ) )
																																										{
																																											z = Gscanf( f1, "%d", &p1 );
																																											if ( z != 1 )IncPar( name, Line, gx );
																																											NShotPt = p1;
																																											ShotPtX = new short[p1];
																																											ShotPtY = new short[p1];
																																											ShotPtZ = new short[p1];
																																											ShotDir = new byte[p1];
																																											ShotDiff = new byte[p1];
																																											ShotMinR = new word[p1];
																																											ShotMaxR = new word[p1];

																																											for ( int i = 0; i < p1; i++ )
																																											{
																																												//x y z minr maxr dir diff
																																												int x, y, z, minr, maxr, dir, diff;
																																												int zz = Gscanf( f1, "%d%d%d%d%d%d%d", &x, &y, &z, &minr, &maxr, &dir, &diff );
																																												if ( zz != 7 )IncPar( name, Line, gx );
																																												ShotPtX[i] = x;
																																												ShotPtY[i] = y;
																																												ShotPtZ[i] = z;
																																												ShotMinR[i] = minr;
																																												ShotMaxR[i] = maxr;
																																												ShotDir[i] = dir;
																																												ShotDiff[i] = diff;
																																												NLine( f1 );
																																												Line++;
																																											};
																																										}
																																										else
																																											if ( !strcmp( gx, "FOGGING" ) )
																																											{
																																												z = Gscanf( f1, "%d%d", &p1, &p2 );
																																												if ( z != 2 )NEPar( name, Line, "FOGGING", 2 );
																																												Fogging.WProb = div( p1 << 15, 101 ).quot;
																																												Fogging.NWeap = p2;
																																												Fogging.Weap = new word[p2];
																																												for ( int i = 0; i < p2; i++ )
																																												{
																																													z = Gscanf( f1, "%s", gz );
																																													if ( z != 1 )NEPar( name, Line, "FOGGING", 2 + p2 );
																																													p3 = GetWeaponIndex( gz );
																																													if ( p3 == -1 )
																																													{
																																														sprintf( gy, "%s,line %d :FOGGING: Unknown weapon ID: %s", name, Line, gz );
																																														ErrM( gy );
																																													};
																																													Fogging.Weap[i] = p3;
																																												};
																																												NLine( f1 );
																																												Line++;
																																											}
																																											else
																																												if ( !strcmp( gx, "DESTRUCT" ) )
																																												{
																																													z = Gscanf( f1, "%d%d", &p1, &p2 );
																																													if ( z != 2 )NEPar( name, Line, "DESTRUCT", 2 );
																																													Destruct.WProb = p1;
																																													Destruct.NWeap = p2;
																																													Destruct.Weap = new word[p2];
																																													for ( int i = 0; i < p2; i++ )
																																													{
																																														z = Gscanf( f1, "%s", gz );
																																														if ( z != 1 )NEPar( name, Line, "DESTRUCT", 2 + p2 );
																																														p3 = GetWeaponIndex( gz );
																																														if ( p3 == -1 )
																																														{
																																															sprintf( gy, "%s,line %d :DESTRUCT: Unknown weapon ID: %s", name, Line, gz );
																																															ErrM( gy );
																																														};
																																														Destruct.Weap[i] = p3;
																																													};
																																													NLine( f1 );
																																													Line++;
																																												}
																																												else
																																													if ( !strcmp( gx, "FIRE" ) )
																																													{
																																														z = Gscanf( f1, "%d%d", &p1, &p2 );
																																														if ( z != 2 )NEPar( name, Line, "FIRE", 2 );
																																														Fire.WProb = div( p1 << 15, 101 ).quot;
																																														Fire.NWeap = p2;
																																														Fire.Weap = new word[p2];
																																														for ( int i = 0; i < p2; i++ )
																																														{
																																															z = Gscanf( f1, "%s", gz );
																																															if ( z != 1 )NEPar( name, Line, "FOGGING", 2 + p2 );
																																															p3 = GetWeaponIndex( gz );
																																															if ( p3 == -1 )
																																															{
																																																sprintf( gy, "%s,line %d :FOGGING: Unknown weapon ID: %s", name, Line, gz );
																																																ErrM( gy );
																																															};
																																															Fire.Weap[i] = p3;
																																														};
																																														NLine( f1 );
																																														Line++;
																																													}
																																													else
																																														if ( !strcmp( gx, "LOCKPOINTS" ) )
																																														{
																																															z = Gscanf( f1, "%d", &p1 );
																																															if ( z != 1 )IncPar( name, Line, "LOCKPOINTS" );
																																															NLockPt = p1;
																																															LockX = new byte[p1];
																																															LockY = new byte[p1];
																																															for ( int i = 0; i < p1; i++ )
																																															{
																																																z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																if ( z != 2 )IncPar( name, Line, "LOCKPOINTS" );
																																																LockX[i] = p2;
																																																LockY[i] = p3;
																																															};
																																															NLine( f1 );
																																															Line++;
																																														}
																																														else
																																															if ( !strcmp( gx, "STAGELOCKPOINTS" ) )
																																															{
																																																z = Gscanf( f1, "%d%d", &p2, &p1 );
																																																if ( z != 2 )IncPar( name, Line, gx );

																																																NSLockPt[p2] = p1;
																																																SLockX[p2] = new byte[p1];
																																																SLockY[p2] = new byte[p1];
																																																for ( int i = 0; i < p1; i++ )
																																																{
																																																	z = Gscanf( f1, "%d%d", &p4, &p3 );
																																																	if ( z != 2 )IncPar( name, Line, "STAGELOCKPOINTS" );
																																																	SLockX[p2][i] = p4;
																																																	SLockY[p2][i] = p3;
																																																};
																																																NLine( f1 );
																																																Line++;
																																															}
																																															else
																																																if ( !strcmp( gx, "BUILDLOCKPOINTS" ) )
																																																{
																																																	z = Gscanf( f1, "%d", &p1 );
																																																	if ( z != 1 )IncPar( name, Line, gx );
																																																	NBLockPt = p1;
																																																	BLockX = new byte[p1];
																																																	BLockY = new byte[p1];
																																																	for ( int i = 0; i < p1; i++ )
																																																	{
																																																		z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																		if ( z != 2 )IncPar( name, Line, gx );
																																																		BLockX[i] = p2;
																																																		BLockY[i] = p3;
																																																	};
																																																	NLine( f1 );
																																																	Line++;
																																																}
																																																else
																																																	if ( !strcmp( gx, "EXPLPOINTS" ) )
																																																	{
																																																		int st;
																																																		z = Gscanf( f1, "%d%d", &st, &p1 );
																																																		if ( z != 2 )IncPar( name, Line, gx );
																																																		if ( st < MaxAStages&&CompxCraft )
																																																		{
																																																			CompxCraft->Stages[st].NExplPoints = p1;
																																																			CompxCraft->Stages[st].ExplCoor = new short[p1 * 2];
																																																			for ( int i = 0; i < p1; i++ )
																																																			{
																																																				z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																				if ( z != 2 )IncPar( name, Line, gx );
																																																				CompxCraft->Stages[st].ExplCoor[i + i] = p2;
																																																				CompxCraft->Stages[st].ExplCoor[i + i + 1] = p3;
																																																			};
																																																		};
																																																		NLine( f1 );
																																																		Line++;
																																																	}
																																																	else
																																																		if ( !strcmp( gx, "CHECKPOINTS" ) )
																																																		{
																																																			z = Gscanf( f1, "%d", &p1 );
																																																			if ( z != 1 )IncPar( name, Line, "CHECKPOINTS" );
																																																			NCheckPt = p1;
																																																			CheckX = new byte[p1];
																																																			CheckY = new byte[p1];
																																																			for ( int i = 0; i < p1; i++ )
																																																			{
																																																				z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																				if ( z != 2 )IncPar( name, Line, "CHECKPOINTS" );
																																																				CheckX[i] = p2;
																																																				CheckY[i] = p3;
																																																			};
																																																			NLine( f1 );
																																																			Line++;
																																																			//creating checking round
																																																			//1. center mass
																																																			int xc = 0;
																																																			int yc = 0;
																																																			for ( int i = 0; i < p1; i++ )
																																																			{
																																																				xc += CheckX[i];
																																																				yc += CheckY[i];
																																																			};
																																																			xc = div( xc, p1 ).quot;
																																																			yc = div( yc, p1 ).quot;
																																																			int maxd = 0;
																																																			int r;
																																																			for ( int i = 0; i < p1; i++ )
																																																			{
																																																				r = Norma( int( CheckX[i] ) - xc, int( CheckY[i] ) - yc );
																																																				if ( r > maxd )maxd = r;
																																																			};
																																																			CenterMX = xc;
																																																			CenterMY = yc;
																																																			BRadius = maxd;
																																																		}
																																																		else
																																																			if ( !strcmp( gx, "BORNPOINTS" ) )
																																																			{
																																																				z = Gscanf( f1, "%d", &p1 );
																																																				if ( z != 1 )IncPar( name, Line, "BORNPOINTS" );
																																																				NBornPt = p1;
																																																				BornPtX = new char[p1];
																																																				BornPtY = new char[p1];
																																																				for ( int i = 0; i < p1; i++ )
																																																				{
																																																					z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																					if ( z != 2 )IncPar( name, Line, "BORNPOINTS" );
																																																					BornPtX[i] = p2;
																																																					BornPtY[i] = p3;
																																																				};
																																																				NLine( f1 );
																																																				Line++;
																																																			}
																																																			else
																																																				if ( !strcmp( gx, "BUILDPOINTS" ) )
																																																				{
																																																					z = Gscanf( f1, "%d", &p1 );
																																																					if ( z != 1 )IncPar( name, Line, "BUILDPOINTS" );
																																																					NBuildPt = p1;
																																																					BuildPtX = new char[p1];
																																																					BuildPtY = new char[p1];
																																																					for ( int i = 0; i < p1; i++ )
																																																					{
																																																						z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																						if ( z != 2 )IncPar( name, Line, "BUILDPOINTS" );
																																																						BuildPtX[i] = p2;
																																																						BuildPtY[i] = p3;
																																																					};
																																																					NLine( f1 );
																																																					Line++;
																																																				}
																																																				else
																																																					if ( !strcmp( gx, "CONCENTRATOR" ) )
																																																					{
																																																						z = Gscanf( f1, "%d", &p1 );
																																																						if ( z != 1 )IncPar( name, Line, "CONCENTRATOR" );
																																																						NConcPt = p1;
																																																						ConcPtX = new char[p1];
																																																						ConcPtY = new char[p1];
																																																						for ( int i = 0; i < p1; i++ )
																																																						{
																																																							z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																							if ( z != 2 )IncPar( name, Line, "CONCENTRATOR" );
																																																							ConcPtX[i] = p2;
																																																							ConcPtY[i] = p3;
																																																						};
																																																						NLine( f1 );
																																																						Line++;
																																																					}
																																																					else
																																																						if ( !strcmp( gx, "DAMPOINTS" ) )
																																																						{
																																																							z = Gscanf( f1, "%d", &p1 );
																																																							if ( z != 1 )IncPar( name, Line, "DAMPOINTS" );
																																																							NDamPt = p1;
																																																							DamPtX = new char[p1];
																																																							DamPtY = new char[p1];
																																																							for ( int i = 0; i < p1; i++ )
																																																							{
																																																								z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																								if ( z != 2 )IncPar( name, Line, "DAMPOINTS" );
																																																								DamPtX[i] = p2;
																																																								DamPtY[i] = p3;
																																																							};
																																																							NLine( f1 );
																																																							Line++;
																																																						}
																																																						else
																																																							if ( !strcmp( gx, "FIRES" ) )
																																																							{
																																																								z = Gscanf( f1, "%d", &p1 );
																																																								if ( z != 1 )IncPar( name, Line, gx );
																																																								FireX[0] = new short[p1];
																																																								FireY[0] = new short[p1];
																																																								NFires[0] = p1;
																																																								for ( int i = 0; i < p1; i++ )
																																																								{
																																																									z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																									if ( z != 2 )IncPar( name, Line, gx );
																																																									FireX[0][i] = p2;
																																																									FireY[0][i] = p3;
																																																								};
																																																								NLine( f1 );
																																																								Line++;
																																																							}
																																																							else
																																																								if ( !strcmp( gx, "STAGE_FIRES" ) )
																																																								{
																																																									int st = 0;
																																																									z = Gscanf( f1, "%d%d", &st, &p1 );
																																																									if ( z != 2 )IncPar( name, Line, gx );
																																																									if ( CompxCraft&&st < MaxAStages )
																																																									{
																																																										CompxCraft->Stages[st].FireX[0] = new short[p1];
																																																										CompxCraft->Stages[st].FireY[0] = new short[p1];
																																																										CompxCraft->Stages[st].NFires[0] = p1;
																																																										for ( int i = 0; i < p1; i++ )
																																																										{
																																																											z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																											if ( z != 2 )IncPar( name, Line, gx );
																																																											CompxCraft->Stages[st].FireX[0][i] = p2;
																																																											CompxCraft->Stages[st].FireY[0][i] = p3;
																																																										};
																																																									};
																																																									NLine( f1 );
																																																									Line++;
																																																								}
																																																								else
																																																									if ( !strcmp( gx, "SMOKE" ) )
																																																									{
																																																										z = Gscanf( f1, "%d", &p1 );
																																																										if ( z != 1 )IncPar( name, Line, gx );
																																																										FireX[1] = new short[p1];
																																																										FireY[1] = new short[p1];
																																																										NFires[1] = p1;
																																																										for ( int i = 0; i < p1; i++ )
																																																										{
																																																											z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																											if ( z != 2 )IncPar( name, Line, gx );
																																																											FireX[1][i] = p2;
																																																											FireY[1][i] = p3;
																																																										};
																																																										NLine( f1 );
																																																										Line++;
																																																									}
																																																									else
																																																										if ( !strcmp( gx, "STAGE_SMOKE" ) )
																																																										{
																																																											int st = 0;
																																																											z = Gscanf( f1, "%d%d", &st, &p1 );
																																																											if ( z != 2 )IncPar( name, Line, gx );
																																																											if ( CompxCraft&&st < MaxAStages )
																																																											{
																																																												CompxCraft->Stages[st].FireX[1] = new short[p1];
																																																												CompxCraft->Stages[st].FireY[1] = new short[p1];
																																																												CompxCraft->Stages[st].NFires[1] = p1;
																																																												for ( int i = 0; i < p1; i++ )
																																																												{
																																																													z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																													if ( z != 2 )IncPar( name, Line, gx );
																																																													CompxCraft->Stages[st].FireX[1][i] = p2;
																																																													CompxCraft->Stages[st].FireY[1][i] = p3;
																																																												};
																																																											};
																																																											NLine( f1 );
																																																											Line++;
																																																										}
																																																										else
																																																											if ( !strcmp( gx, "BUILDSTAGES" ) )
																																																											{
																																																												z = Gscanf( f1, "%d", &p1 );
																																																												if ( z != 1 )IncPar( name, Line, "BUILDSTAGES" );
																																																												ProduceStages = p1;
																																																												NLine( f1 );
																																																												Line++;
																																																											}
																																																											else
																																																												if ( !strcmp( gx, "FOUNDATION" ) )
																																																												{
																																																													z = Gscanf( f1, "%d", &p1 );
																																																													if ( z != 1 )IncPar( name, Line, gx );
																																																													NHideTri = p1;
																																																													HideTriX = new short[p1];
																																																													HideTriY = new short[p1];
																																																													for ( int i = 0; i < p1; i++ )
																																																													{
																																																														z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																														if ( z != 2 )IncPar( name, Line, gx );
																																																														HideTriX[i] = p2 + PicDx;
																																																														HideTriY[i] = p3 + PicDy;
																																																													};
																																																													NLine( f1 );
																																																													Line++;
																																																												}
																																																												else
																																																													if ( !strcmp( gx, "SETANMPARAM" ) )
																																																													{
																																																														z = Gscanf( f1, "%d%d%d%d", &p1, &p2, &p3, &p4 );
																																																														if ( z != 4 )IncPar( name, Line, "SETANMPARAM" );
																																																														NAStartDx = p1;
																																																														NAStartDy = p2;
																																																														NAParts = p3;
																																																														NAPartSize = p4;
																																																														NLine( f1 );
																																																														Line++;
																																																													}
																																																													else
																																																														if ( !strcmp( gx, "RECTANGLE" ) )
																																																														{
																																																															z = Gscanf( f1, "%d%d%d%d", &p1, &p2, &p3, &p4 );
																																																															if ( z != 4 )IncPar( name, Line, "RECTANGLE" );
																																																															RectDx = p1;
																																																															RectDy = p2;
																																																															RectLx = p3;
																																																															RectLy = p4;
																																																															NLine( f1 );
																																																															Line++;
																																																														}
																																																														else
																																																															if ( !strcmp( gx, "DAMAGE" ) )
																																																															{
																																																																z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																																if ( z != 2 )IncPar( name, Line, "DAMAGE" );
																																																																MinDamage[p2] = p3;
																																																																MaxDamage[p2] = p3;
																																																																NLine( f1 );
																																																																Line++;
																																																															}
																																																															else
																																																																if ( !strcmp( gx, "DAMAGEDEC" ) )
																																																																{
																																																																	z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																																	if ( z != 2 )IncPar( name, Line, "DAMAGE" );
																																																																	DamageDecr[p2] = p3;
																																																																	NLine( f1 );
																																																																	Line++;
																																																																}
																																																																else
																																																																	if ( !strcmp( gx, "DIRECTTRANS" ) )
																																																																	{
																																																																		z = Gscanf( f1, "%d%d", &p2, &p3 );
																																																																		if ( z != 2 )IncPar( name, Line, gx );
																																																																		TransMask[p2] |= 1 << p3;
																																																																		TransMask[p3] |= 1 << p2;
																																																																		NLine( f1 );
																																																																		Line++;
																																																																	}
																																																																	else
																																																																		if ( !strcmp( gx, "USETRANSX" ) )
																																																																		{
																																																																			z = Gscanf( f1, "%d", &p2 );
																																																																			if ( z != 1 )IncPar( name, Line, gx );
																																																																			TransXMask |= 1 << p2;
																																																																			NLine( f1 );
																																																																			Line++;
																																																																		}
																																																																		else
																																																																			if ( !strcmp( gx, "SLOWRECHARGE" ) )
																																																																			{
																																																																				SlowRecharge = 1;
																																																																				NLine( f1 );
																																																																				Line++;
																																																																			}
																																																																			else
																																																																				if ( !strcmp( gx, "NOPAUSEDATTACK" ) )
																																																																				{
																																																																					z = Gscanf( f1, "%d", &p2 );
																																																																					if ( z != 1 )IncPar( name, Line, gx );
																																																																					NoWaitMask |= 1 << p2;
																																																																					NLine( f1 );
																																																																					Line++;
																																																																				}
																																																																				else
																																																																					if ( !strcmp( gx, "EXDAMAGE" ) )
																																																																					{
																																																																						z = Gscanf( f1, "%d%d%d", &p1, &p2, &p3 );
																																																																						if ( z != 3 )IncPar( name, Line, "EXDAMAGE" );
																																																																						MinDamage[p1] = p2;
																																																																						MaxDamage[p1] = p3;
																																																																						NLine( f1 );
																																																																						Line++;
																																																																					}
																																																																					else
																																																																						if ( !strcmp( gx, "LIFE" ) )
																																																																						{
																																																																							z = Gscanf( f1, "%d", &p1 );
																																																																							if ( z != 1 )IncPar( name, Line, "LIFE" );
																																																																							Life = p1;
																																																																							NLine( f1 );
																																																																							Line++;
																																																																						}
																																																																						else
																																																																							if ( !strcmp( gx, "STORMFORCE" ) )
																																																																							{
																																																																								z = Gscanf( f1, "%d", &p1 );
																																																																								if ( z != 1 )IncPar( name, Line, gx );
																																																																								StormForce = p1;
																																																																								NLine( f1 );
																																																																								Line++;
																																																																							}
																																																																							else
																																																																								if ( !strcmp( gx, "OPPOSIT" ) )
																																																																								{
																																																																									z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																									if ( z != 2 )IncPar( name, Line, gx );
																																																																									MinOposit = p1;
																																																																									MaxOposit = p2;
																																																																									NLine( f1 );
																																																																									Line++;
																																																																								}
																																																																								else
																																																																									if ( !strcmp( gx, "CANBEKILLEDINSIDE" ) )
																																																																									{
																																																																										CanBeKilledInside = 1;
																																																																										NLine( f1 );
																																																																										Line++;
																																																																									}
																																																																									else
																																																																										if ( !strcmp( gx, "CANBECAPTUREDWHENFREE" ) )
																																																																										{
																																																																											CanBeCapturedWhenFree = 1;
																																																																											NLine( f1 );
																																																																											Line++;
																																																																										}
																																																																										else
																																																																											if ( !strcmp( gx, "CANSHOOT" ) )
																																																																											{
																																																																												CanShoot = 1;
																																																																												NLine( f1 );
																																																																												Line++;
																																																																											}
																																																																											else
																																																																												if ( !strcmp( gx, "CANSTORM" ) )
																																																																												{
																																																																													CanStorm = 1;
																																																																													NLine( f1 );
																																																																													Line++;
																																																																												}
																																																																												else
																																																																													if ( !strcmp( gx, "PROMAX" ) )
																																																																													{
																																																																														z = Gscanf( f1, "%d", &p1 );
																																																																														if ( z != 1 )IncPar( name, Line, gx );
																																																																														PromaxPercent = p1;
																																																																														NLine( f1 );
																																																																														Line++;
																																																																													}
																																																																													else
																																																																														if ( !strcmp( gx, "ADDSHOTRADIUS" ) )
																																																																														{
																																																																															z = Gscanf( f1, "%d", &p1 );
																																																																															if ( z != 1 )IncPar( name, Line, gx );
																																																																															AddShotRadius = p1;
																																																																															NLine( f1 );
																																																																															Line++;
																																																																														}
																																																																														else
																																																																															if ( !strcmp( gx, "TIME" ) )
																																																																															{
																																																																																z = Gscanf( f1, "%d", &p1 );
																																																																																if ( z != 1 )IncPar( name, Line, gx );
																																																																																ProduceStages = p1;
																																																																																NLine( f1 );
																																																																																Line++;
																																																																															}
																																																																															else
																																																																																if ( !strcmp( gx, "KIND" ) )
																																																																																{
																																																																																	z = Gscanf( f1, "%d", &p1 );
																																																																																	if ( z != 1 )IncPar( name, Line, "KINO" );
																																																																																	Kind = p1;
																																																																																	NLine( f1 );
																																																																																	Line++;
																																																																																}
																																																																																else
																																																																																	if ( !strcmp( gx, "UNITKIND" ) )
																																																																																	{
																																																																																		z = Gscanf( f1, "%s", gz );
																																																																																		if ( z != 1 )IncPar( name, Line, gx );
																																																																																		p1 = GetUnitKind( gz );
																																																																																		if ( p1 == -1 )
																																																																																		{
																																																																																			sprintf( "%s, Line %d :%s: Unknown kind of unit(see [UNITKIND],NRES.DAT):%s", name, Line, gx, gz );
																																																																																			ErrM( gy );
																																																																																		};
																																																																																		Kind = p1;
																																																																																	}
																																																																																	else
																																																																																		if ( !strcmp( gx, "MESSAGE" ) )
																																																																																		{
																																																																																			z = Gscanf( f1, "%s", gz );
																																																																																			//if(z!=1)IncPar(name,Line,"MESSAGE");
																																																																																			//normstr(gz);
																																																																																			//DosToWin(gz);
																																																																																			//Message=new char[strlen(gz)+1];
																																																																																			//strcpy(Message,gz);
																																																																																			NLine( f1 );
																																																																																			Line++;
																																																																																		}
																																																																																		else
																																																																																			if ( !strcmp( gx, "COST" ) )
																																																																																			{
																																																																																				NLine( f1 );
																																																																																				Line++;
																																																																																			}
																																																																																			else
																																																																																				if ( !strcmp( gx, "PRICE" ) )
																																																																																				{
																																																																																					z = Gscanf( f1, "%d", &p1 );
																																																																																					if ( z != 1 )IncPar( name, Line, "COST" );
																																																																																					for ( int i = 0; i < p1; i++ )
																																																																																					{
																																																																																						z = Gscanf( f1, "%s%d", gy, &p2 );
																																																																																						int r = GetResByName( gy );
																																																																																						if ( r >= 0 && r < 100 )
																																																																																						{
																																																																																							NeedRes[r] = p2;
																																																																																						}
																																																																																						else
																																																																																						{
																																																																																							sprintf( gz, "%s,line %d:%s: Unknown resource:%s", name, Line, gx, gy );
																																																																																							ErrM( gz );
																																																																																						};
																																																																																					};
																																																																																					NLine( f1 );
																																																																																					Line++;
																																																																																				}
																																																																																				else
																																																																																					if ( !strcmp( gx, "SHIELD" ) )
																																																																																					{
																																																																																						z = Gscanf( f1, "%d", &p1 );
																																																																																						if ( z != 1 )IncPar( name, Line, "SHIELD" );
																																																																																						Shield = p1;
																																																																																						NLine( f1 );
																																																																																						Line++;
																																																																																					}
																																																																																					else
																																																																																						if ( !strcmp( gx, "RANGE" ) )
																																																																																						{
																																																																																							z = Gscanf( f1, "%d%d%d%d%d%d", &p1, &p2, &p3, &p4, &p5, &p6 );
																																																																																							if ( z != 6 )IncPar( name, Line, gx );
																																																																																							AttRange = p1 << 8;
																																																																																							MinAttRange = p2 << 8;
																																																																																							MaxAttRange = p3 << 8;
																																																																																							MaxNearRange = p4 << 8;
																																																																																							AttType = p5;
																																																																																							VisRange = p6;
																																																																																							NLine( f1 );
																																																																																							Line++;
																																																																																						}
																																																																																						else
																																																																																							if ( !strcmp( gx, "SEARCH_ENEMY_RADIUS" ) )
																																																																																							{
																																																																																								z = Gscanf( f1, "%d", &p1 );
																																																																																								if ( z != 1 )IncPar( name, Line, gx );
																																																																																								VisRange = p1 << 4;
																																																																																								NLine( f1 );
																																																																																								Line++;
																																																																																							}
																																																																																							else
																																																																																								if ( !strcmp( gx, "MATHERIAL" ) )
																																																																																								{
																																																																																									z = Gscanf( f1, "%d", &p1 );
																																																																																									if ( z != 1 )IncPar( name, Line, gx );
																																																																																									for ( int i = 0; i < p1; i++ )
																																																																																									{
																																																																																										z = Gscanf( f1, "%s", gx );
																																																																																										if ( z != 1 )IncPar( name, Line, gx );
																																																																																										p2 = GetMatherialType( gx );
																																																																																										if ( p2 != -1 )
																																																																																										{
																																																																																											MathMask |= p2;
																																																																																											if ( !strcmp( gx, "BUILDING" ) )
																																																																																											{
																																																																																												AttBuild = true;
																																																																																											};
																																																																																										}
																																																																																										else
																																																																																										{
																																																																																											sprintf( gz, "%s, Line: %d :MATHERIAL: Unknown matherial:%s", name, Line, gy );
																																																																																											ErrM( gz );
																																																																																										};
																																																																																									};
																																																																																								}
																																																																																								else
																																																																																									if ( !strcmp( gx, "CANKILL" ) )
																																																																																									{
																																																																																										z = Gscanf( f1, "%d", &p1 );
																																																																																										if ( z != 1 )IncPar( name, Line, gx );
																																																																																										for ( int i = 0; i < p1; i++ )
																																																																																										{
																																																																																											z = Gscanf( f1, "%s", gx );
																																																																																											if ( z != 1 )IncPar( name, Line, gx );
																																																																																											p2 = GetMatherialType( gx );
																																																																																											if ( p2 != -1 )
																																																																																											{
																																																																																												KillMask |= p2;
																																																																																											}
																																																																																											else
																																																																																											{
																																																																																												sprintf( gz, "%s, Line: %d :CANKILL: Unknown matherial:%s", name, Line, gx );
																																																																																												ErrM( gz );
																																																																																											};
																																																																																										};
																																																																																										for ( int i = 0; i < NAttTypes; i++ )
																																																																																										{
																																																																																											AttackMask[i] = KillMask;
																																																																																										};
																																																																																										NLine( f1 );
																																																																																										Line++;
																																																																																									}
																																																																																									else
																																																																																										if ( !strcmp( gx, "ATTMASK" ) )
																																																																																										{
																																																																																											z = Gscanf( f1, "%d%d", &p1, &p4 );
																																																																																											if ( z != 2 )IncPar( name, Line, gx );
																																																																																											AttackMask[p1] = 0;
																																																																																											for ( int k = 0; k < p4; k++ )
																																																																																											{
																																																																																												z = Gscanf( f1, "%s", gy );
																																																																																												p2 = GetMatherialType( gy );
																																																																																												if ( p2 != -1 )
																																																																																												{
																																																																																													AttackMask[p1] |= p2;
																																																																																													if ( !strcmp( gy, "BUILDING" ) )
																																																																																													{
																																																																																														AttBuild = true;
																																																																																													};
																																																																																												}
																																																																																												else
																																																																																												{
																																																																																													sprintf( gz, "%s, Line: %d :ATTMASK: Unknown matherial:%s", name, Line, gy );
																																																																																													ErrM( gz );
																																																																																												};
																																																																																											};
																																																																																											NLine( f1 );
																																																																																											Line++;
																																																																																										}
																																																																																										else
																																																																																											if ( !strcmp( gx, "RATE" ) )
																																																																																											{
																																																																																												z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																																												if ( z != 2 )IncPar( name, Line, "RATE" );
																																																																																												if ( p1 > NAttTypes )
																																																																																												{
																																																																																													sprintf( gy, "%s,line %d : Attack type index in RATE must be less 4", name, Line );
																																																																																													ErrM( gy );
																																																																																												};
																																																																																												Rate[p1] = p2;
																																																																																												NLine( f1 );
																																																																																												Line++;
																																																																																											}
																																																																																											else
																																																																																												if ( !strcmp( gx, "ZALP" ) )
																																																																																												{
																																																																																													z = Gscanf( f1, "%d", &p1 );
																																																																																													if ( z != 1 )IncPar( name, Line, gx );
																																																																																													if ( p1 > 255 )p1 = 255;
																																																																																													MaxZalp = p1;
																																																																																													NLine( f1 );
																																																																																													Line++;
																																																																																												}
																																																																																												else
																																																																																													if ( !strcmp( gx, "WEAPONKIND" ) )
																																																																																													{
																																																																																														z = Gscanf( f1, "%d%s", &p1, gy );
																																																																																														if ( z != 2 )IncPar( name, Line, gx );
																																																																																														if ( p1 >= NAttTypes )
																																																																																														{
																																																																																															sprintf( gz, "%s,Line %d :%s: Too big attack index", name, Line, gx );
																																																																																															ErrM( gz );
																																																																																														};
																																																																																														int zz2 = GetWeaponType( gy );
																																																																																														if ( zz2 == -1 )
																																																																																														{
																																																																																															sprintf( gz, "%s,Line %d :%s: Unknown weapon kind(see NRES.DAT):%s", name, Line, gx, gy );
																																																																																															ErrM( gz );
																																																																																														};
																																																																																														WeaponKind[p1] = zz2;
																																																																																														NLine( f1 );
																																																																																														Line++;
																																																																																													}
																																																																																													else
																																																																																														if ( !strcmp( gx, "PROTECTION" ) )
																																																																																														{
																																																																																															z = Gscanf( f1, "%d", &p1 );
																																																																																															if ( z != 1 )IncPar( name, Line, gx );
																																																																																															for ( int i = 0; i < p1; i++ )
																																																																																															{
																																																																																																z = Gscanf( f1, "%s%d", gy, &p2 );
																																																																																																if ( z != 2 )IncPar( name, Line, gx );
																																																																																																int zz2 = GetWeaponType( gy );
																																																																																																if ( zz2 == -1 )
																																																																																																{
																																																																																																	sprintf( gz, "%s,Line %d :%s: Unknown weapon kind(see NRES.DAT):%s", name, Line, gx, gy );
																																																																																																	ErrM( gz );
																																																																																																};
																																																																																																Protection[zz2] = p2;//div(p2*255,100).quot;
																																																																																															};
																																																																																															NLine( f1 );
																																																																																															Line++;
																																																																																														}
																																																																																														else
																																																																																															if ( !strcmp( gx, "SETACTIVEPOINT" ) )
																																																																																															{
																																																																																																z = Gscanf( f1, "%s%d", gx, &p1 );
																																																																																																if ( z != 2 )IncPar( name, Line, "SETACTIVEPOINT" );
																																																																																																NewAnimation* NANM = LoadNewAnimationByName( gx );
																																																																																																if ( NANM )
																																																																																																{
																																																																																																	for ( int j = 0; j < NANM->Rotations; j++ )
																																																																																																	{
																																																																																																		z = Gscanf( f1, "%d%d", &xx, &yy );
																																																																																																		if ( z != 2 )
																																																																																																		{
																																																																																																			sprintf( gy, "%s,line %d : Not enough active points. Must be : %d", name, Line, NANM->Rotations );
																																																																																																			ErrM( gy );
																																																																																																		};
																																																																																																		NANM->ActivePtX[j] = xx;
																																																																																																		NANM->ActivePtY[j] = yy;
																																																																																																		NANM->ActiveFrame = p1;
																																																																																																	};
																																																																																																}
																																																																																																else
																																																																																																{
																																																																																																	sprintf( gy, "%s,line %d : Unknown animation type: %s", name, Line, gx );
																																																																																																	ErrM( gy );
																																																																																																};
																																																																																																NLine( f1 );
																																																																																																Line++;
																																																																																															}
																																																																																															else
																																																																																																if ( !strcmp( gx, "DET_RADIUS" ) )
																																																																																																{
																																																																																																	z = Gscanf( f1, "%d%d%d", &p1, &p2, &p3 );
																																																																																																	if ( z != 3 )IncPar( name, Line, gx );
																																																																																																	if ( p1 >= NAttTypes )
																																																																																																	{
																																																																																																		sprintf( gy, "%s,line %d : Attack type index in ATTACK_RADIUS is too big.", name, Line );
																																																																																																		ErrM( gy );
																																																																																																	};
																																																																																																	DetRadius1[p1] = p2;
																																																																																																	DetRadius2[p1] = p3;
																																																																																																	NLine( f1 );
																																																																																																	Line++;
																																																																																																}
																																																																																																else
																																																																																																	if ( !strcmp( gx, "ATTACK_RADIUS" ) )
																																																																																																	{
																																																																																																		z = Gscanf( f1, "%d%d%d", &p1, &p2, &p3 );
																																																																																																		if ( z != 3 )IncPar( name, Line, gx );
																																																																																																		if ( p1 >= NAttTypes )
																																																																																																		{
																																																																																																			sprintf( gy, "%s,line %d : Attack type index in ATTACK_RADIUS is too big.", name, Line );
																																																																																																			ErrM( gy );
																																																																																																		};
																																																																																																		AttackRadius1[p1] = p2;
																																																																																																		AttackRadius2[p1] = p3;
																																																																																																		DetRadius1[p1] = p2;
																																																																																																		DetRadius2[p1] = p3;
																																																																																																		NLine( f1 );
																																																																																																		Line++;
																																																																																																	}
																																																																																																	else
																																																																																																		if ( !strcmp( gx, "ADD_ATTACK_RADIUS" ) )
																																																																																																		{
																																																																																																			z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																																																			if ( z != 2 )IncPar( name, Line, "ADD_ATTACK_RADIUS" );
																																																																																																			if ( p1 >= NAttTypes )
																																																																																																			{
																																																																																																				sprintf( gy, "%s,line %d : Attack type index in ADD_ATTACK_RADIUS is too big.", name, Line );
																																																																																																				ErrM( gy );
																																																																																																			};
																																																																																																			AttackRadiusAdd[p1] = p2;
																																																																																																			NLine( f1 );
																																																																																																			Line++;
																																																																																																		}
																																																																																																		else
																																																																																																			if ( !strcmp( gx, "ATTACK_PAUSE" ) )
																																																																																																			{
																																																																																																				z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																																																				if ( z != 2 )IncPar( name, Line, "ATTACK_PAUSE" );
																																																																																																				if ( p1 >= NAttTypes )
																																																																																																				{
																																																																																																					sprintf( gy, "%s,line %d : Attack type index in ATTACK_PAUSE is too big.", name, Line );
																																																																																																					ErrM( gy );
																																																																																																				};
																																																																																																				AttackPause[p1] = p2;
																																																																																																				NLine( f1 );
																																																																																																				Line++;
																																																																																																			}
																																																																																																			else
																																																																																																				if ( !strcmp( gx, "ATTACK_ANGLES" ) )
																																																																																																				{
																																																																																																					z = Gscanf( f1, "%d%d%d", &p1, &p2, &p3 );
																																																																																																					if ( z != 3 )IncPar( name, Line, "ATTACK_ANGLES" );
																																																																																																					if ( p1 >= NAttTypes )
																																																																																																					{
																																																																																																						sprintf( gy, "%s,line %d : Attack type index in SHOT_ANGLE is too big.", name, Line );
																																																																																																						ErrM( gy );
																																																																																																					};
																																																																																																					AngleUp[p1] = p3;
																																																																																																					AngleDn[p1] = p2;
																																																																																																					NLine( f1 );
																																																																																																					Line++;
																																																																																																				}
																																																																																																				else
																																																																																																					if ( !strcmp( gx, "ZPOINTS" ) )
																																																																																																					{
																																																																																																						z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																																																						if ( z != 2 )IncPar( name, Line, "ZPOINTS" );
																																																																																																						SrcZPoint = p1;
																																																																																																						DstZPoint = p2;
																																																																																																						NLine( f1 );
																																																																																																						Line++;
																																																																																																					}
																																																																																																					else
																																																																																																						if ( !strcmp( gx, "ICON" ) )
																																																																																																						{
																																																																																																							z = Gscanf( f1, "%s", &gy );
																																																																																																							if ( z != 1 )IncPar( name, Line, "ICON" );
																																																																																																							p2 = GetIconByName( gy );
																																																																																																							if ( p2 == -1 )
																																																																																																							{
																																																																																																								sprintf( gz, "%s,%d : Unknown Icon ID: %s", name, Line, gy );
																																																																																																								ErrM( gz );
																																																																																																							};
																																																																																																							IconFileID = 0;
																																																																																																							IconID = p2;
																																																																																																							NLine( f1 );
																																																																																																							Line++;
																																																																																																						}
																																																																																																						else
																																																																																																							if ( !strcmp( gx, "ADDSECTION" ) )
																																																																																																							{
																																																																																																								//ADDSECTION Index Gp AddPts Empty x y  Stage1 x y Ready x y Dead x y
																																																																																																								int Idx, gp, apt, st1, x1, y1, st2, x2, y2, st3, x3, y3, st4, x4, y4;
																																																																																																								int z = Gscanf( f1, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", &Idx, &gp, &apt,
																																																																																																									&st1, &x1, &y1,
																																																																																																									&st2, &x2, &y2,
																																																																																																									&st3, &x3, &y3,
																																																																																																									&st4, &x4, &y4 );
																																																																																																								if ( z == 15 )
																																																																																																								{
																																																																																																									if ( !CompxCraft )
																																																																																																									{
																																																																																																										CompxCraft = new ComplexBuilding;
																																																																																																										memset( CompxCraft, 0, sizeof ComplexBuilding );
																																																																																																									};
																																																																																																									if ( Idx >= MaxAStages )
																																																																																																									{
																																																																																																										sprintf( gy, "%s,line %d : ADDSECTION : Too large stage index", name, Line );
																																																																																																										ErrM( gy );
																																																																																																									}
																																																																																																									else
																																																																																																									{
																																																																																																										CompxCraft->Mask |= 1 << Idx;
																																																																																																										CompxCraft->Stages[Idx].GPID = RLCRef[gp];
																																																																																																										CompxCraft->Stages[Idx].AddPoints = apt;

																																																																																																										CompxCraft->Stages[Idx].Empty.SpriteID = st1;
																																																																																																										CompxCraft->Stages[Idx].Empty.SortX = x1;
																																																																																																										CompxCraft->Stages[Idx].Empty.SortY = y1;

																																																																																																										CompxCraft->Stages[Idx].Stage1.SpriteID = st2;
																																																																																																										CompxCraft->Stages[Idx].Stage1.SortX = x2;
																																																																																																										CompxCraft->Stages[Idx].Stage1.SortY = y2;

																																																																																																										CompxCraft->Stages[Idx].Ready.SpriteID = st3;
																																																																																																										CompxCraft->Stages[Idx].Ready.SortX = x3;
																																																																																																										CompxCraft->Stages[Idx].Ready.SortY = y3;

																																																																																																										CompxCraft->Stages[Idx].Dead.SpriteID = st4;
																																																																																																										CompxCraft->Stages[Idx].Dead.SortX = x4;
																																																																																																										CompxCraft->Stages[Idx].Dead.SortY = y4;

																																																																																																									};
																																																																																																								}
																																																																																																								else
																																																																																																								{
																																																																																																									sprintf( gy, "%s,line %d : ADDSECTION : Not enough or wrong parameters", name, Line );
																																																																																																									ErrM( gy );
																																																																																																								};
																																																																																																							}
																																																																																																							else
																																																																																																								if ( !strcmp( gx, "WEAPON" ) )
																																																																																																								{
																																																																																																									z = Gscanf( f1, "%d%s", &p1, gz );
																																																																																																									if ( z != 2 )IncPar( name, Line, "WEAPON" );
																																																																																																									if ( p1 >= NAttTypes )
																																																																																																									{
																																																																																																										sprintf( gy, "%s,line %d :WEAPON: Index too big : %d", name, Line, p1 );
																																																																																																										ErrM( gy );
																																																																																																									};
																																																																																																									p2 = GetWeaponIndex( gz );
																																																																																																									if ( p2 == -1 )
																																																																																																									{
																																																																																																										sprintf( gy, "%s,line %d :WEAPON: Unknown weapon ID: %s", name, Line, gz );
																																																																																																										ErrM( gy );
																																																																																																									};
																																																																																																									DamWeap[p1] = WPLIST[p2];
																																																																																																									NLine( f1 );
																																																																																																									Line++;
																																																																																																								}
																																																																																																								else
																																																																																																									if ( !strcmp( gx, "USERLC" ) )
																																																																																																									{
																																																																																																										z = Gscanf( f1, "%d%s%s%d%d", &p1, gy, gz, &p2, &p3 );
																																																																																																										if ( z != 5 )IncPar( name, Line, "USERLC" );
																																																																																																										if ( p1 > MaxRLC )MaxRLC = p1;
																																																																																																										UpConv( gy );
																																																																																																										//search for RLC file in global table
																																																																																																										int nr = GPS.PreLoadGPImage( gy, !strcmp( gz, "SHADOW" ) );
																																																																																																										RLCRef[p1] = nr;
																																																																																																										RLCdx[p1] = p2;
																																																																																																										RLCdy[p1] = p3;
																																																																																																										RLCParts[p1] = 1;
																																																																																																										RLCPartSize[p1] = 0;
																																																																																																										NLine( f1 );
																																																																																																										Line++;
																																																																																																									}
																																																																																																									else
																																																																																																										if ( !strcmp( gx, "USERLCEXT" ) )
																																																																																																										{
																																																																																																											z = Gscanf( f1, "%d%d%d%s%s%d%d", &p1, &p4, &p5, gy, gz, &p2, &p3 );
																																																																																																											if ( z != 7 )IncPar( name, Line, "USERLCEXT" );
																																																																																																											if ( p1 > MaxRLC )MaxRLC = p1;
																																																																																																											UpConv( gy );
																																																																																																											//search for RLC file in global table
																																																																																																											int nr = GPS.PreLoadGPImage( gy, !strcmp( gz, "SHADOW" ) );
																																																																																																											RLCRef[p1] = nr;
																																																																																																											RLCdx[p1] = p2;
																																																																																																											RLCdy[p1] = p3;
																																																																																																											RLCParts[p1] = p4;
																																																																																																											RLCPartSize[p2] = p5;
																																																																																																											NLine( f1 );
																																																																																																											Line++;
																																																																																																										}
																																																																																																										else
																																																																																																											if ( !strcmp( gx, "NAME" ) )
																																																																																																											{
																																																																																																												z = Gscanf( f1, "%s", gy );
																																																																																																												if ( z != 1 )IncPar( name, Line, "NAME" );
																																																																																																												Name = new char[strlen( gy ) + 1];
																																																																																																												strcpy( Name, gy );
																																																																																																												NLine( f1 );
																																																																																																												Line++;
																																																																																																											}
																																																																																																											else
																																																																																																												if ( !strcmp( gx, "COST_FOOD" ) )
																																																																																																												{
																																																																																																													z = Gscanf( f1, "%d", &p1 );
																																																																																																													if ( z != 1 )NEPar( name, Line, "COST_FOOD", 1 );
																																																																																																													NeedRes[3] = p1;
																																																																																																													NLine( f1 );
																																																																																																													Line++;
																																																																																																												}
																																																																																																												else
																																																																																																													if ( !strcmp( gx, "BUILDING" ) )
																																																																																																													{
																																																																																																														Building = true;
																																																																																																														Ves = 500;
																																																																																																														NLine( f1 );
																																																																																																														Line++;
																																																																																																													}
																																																																																																													else
																																																																																																														if ( !strcmp( gx, "SPRITEOBJECT" ) )
																																																																																																														{
																																																																																																															SpriteObject = true;
																																																																																																															Building = true;
																																																																																																															NLine( f1 );
																																																																																																															Line++;
																																																																																																														}
																																																																																																														else
																																																																																																															if ( !strcmp( gx, "FLAGS" ) )
																																																																																																															{
																																																																																																																int xc, N, x1, x2, y2, dy;
																																																																																																																z = Gscanf( f1, "%d%d", &xc, &N );
																																																																																																																if ( z == 2 )
																																																																																																																{
																																																																																																																	FLAGS = (Flags3D*) malloc( sizeof( Flags3D ) - 48 * 2 + N * 8 );
																																																																																																																	FLAGS->N = N;
																																																																																																																	FLAGS->Xr = xc;
																																																																																																																	for ( int i = 0; i < N; i++ )
																																																																																																																	{
																																																																																																																		z = Gscanf( f1, "%d%d%d%d", &x1, &x2, &y2, &dy );
																																																																																																																		int i3 = i << 2;
																																																																																																																		FLAGS->Points[i3] = x1;
																																																																																																																		FLAGS->Points[i3 + 1] = x2;
																																																																																																																		FLAGS->Points[i3 + 2] = y2;
																																																																																																																		FLAGS->Points[i3 + 3] = dy;
																																																																																																																	};
																																																																																																																}
																																																																																																																else
																																																																																																																{
																																																																																																																	sprintf( gz, "%s: Invalid FLAGS parameters", name );
																																																																																																																	ErrM( gz );
																																																																																																																};
																																																																																																																NLine( f1 );
																																																																																																																Line++;
																																																																																																															}
																																																																																																															else
																																																																																																																if ( !strcmp( gx, "WALLOBJECT" ) )
																																																																																																																{
																																																																																																																	Wall = true;
																																																																																																																	SpriteObject = true;
																																																																																																																	Building = true;
																																																																																																																	z = Gscanf( f1, "%s", gy );
																																																																																																																	if ( z != 1 )IncPar( name, Line, gx );
																																																																																																																	p1 = GetWallType( gy );
																																																																																																																	if ( p1 == -1 )
																																																																																																																	{
																																																																																																																		sprintf( gz, "%s, %d :%s: Unknown wall type: %s", name, Line, gx, gy );
																																																																																																																		ErrM( gz );
																																																																																																																	}
																																																																																																																	else
																																																																																																																	{
																																																																																																																		Sprite = p1;
																																																																																																																	};
																																																																																																																	NLine( f1 );
																																																																																																																	Line++;
																																																																																																																}
																																																																																																																else
																																																																																																																	if ( !strcmp( gx, "PRODUCER" ) )
																																																																																																																	{
																																																																																																																		Producer = true;
																																																																																																																		z = Gscanf( f1, "%d", &p1 );
																																																																																																																		if ( z != 1 )IncPar( name, Line, gx );
																																																																																																																		ProdType = 0;
																																																																																																																		for ( int i = 0; i < p1; i++ )
																																																																																																																		{
																																																																																																																			z = Gscanf( f1, "%s", gy );
																																																																																																																			if ( z != 1 )IncPar( name, Line, gx );
																																																																																																																			if ( !strcmp( gy, "WOOD" ) )ProdType |= 1; else
																																																																																																																				if ( !strcmp( gy, "GOLD" ) )ProdType |= 2; else
																																																																																																																					if ( !strcmp( gy, "STONE" ) )ProdType |= 4; else
																																																																																																																						if ( !strcmp( gy, "FOOD" ) )ProdType |= 8; else
																																																																																																																							if ( !strcmp( gy, "IRON" ) )ProdType |= 16; else
																																																																																																																								if ( !strcmp( gy, "COAL" ) )ProdType |= 32; else
																																																																																																																								{
																																																																																																																									sprintf( gz, "%s,Line %d : %s : Unknown resource type : %s.\n Must be one of WOOD,STONE,GOLD,FOOD,IRON,COAL.", name, Line, gx, gy );
																																																																																																																									ErrM( gz );
																																																																																																																								};
																																																																																																																		};
																																																																																																																		z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																																																																		if ( z != 2 )IncPar( name, Line, gx );
																																																																																																																		FreeAdd = p1;
																																																																																																																		PeasantAdd = p2;
																																																																																																																		NLine( f1 );
																																																																																																																		Line++;
																																																																																																																	}
																																																																																																																	else
																																																																																																																		if ( !strcmp( gx, "PEASANT" ) )
																																																																																																																		{
																																																																																																																			Peasant = true;
																																																																																																																			NLine( f1 );
																																																																																																																			Line++;
																																																																																																																		}
																																																																																																																		else
																																																																																																																			if ( !strcmp( gx, "UNITABSORBER" ) )
																																																																																																																			{
																																																																																																																				z = Gscanf( f1, "%d", &p1 );
																																																																																																																				if ( z != 1 )NEPar( name, Line, "UNITABSORBER", 1 );
																																																																																																																				UnitAbsorber = true;
																																																																																																																				MaxInside = p1;
																																																																																																																				NLine( f1 );
																																																																																																																				Line++;
																																																																																																																			}
																																																																																																																			else
																																																																																																																				if ( !strcmp( gx, "TRANSPORT" ) )
																																																																																																																				{
																																																																																																																					z = Gscanf( f1, "%d", &p1 );
																																																																																																																					if ( z != 1 )NEPar( name, Line, gx, 1 );
																																																																																																																					Transport = true;
																																																																																																																					MaxInside = p1;
																																																																																																																					NLine( f1 );
																																																																																																																					Line++;
																																																																																																																				}
																																																																																																																				else
																																																																																																																					if ( !strcmp( gx, "PEASANTABSORBER" ) )
																																																																																																																					{
																																																																																																																						z = Gscanf( f1, "%d", &p1 );
																																																																																																																						if ( z != 1 )NEPar( name, Line, "PEASANTABSORBER", 1 );
																																																																																																																						PeasantAbsorber = true;
																																																																																																																						MaxInside = p1;
																																																																																																																						NLine( f1 );
																																																																																																																						Line++;
																																																																																																																					}
																																																																																																																					else
																																																																																																																						if ( !strcmp( gx, "ARTDEPO" ) )
																																																																																																																						{
																																																																																																																							p1 = 5; p2 = 5; p3 = 5; p4 = 5;
																																																																																																																							z = Gscanf( f1, "%d%d%d%d", &p1, &p2, &p3, &p4 );
																																																																																																																							ArtDepo = true;
																																																																																																																							ArtCap[0] = p1;
																																																																																																																							ArtCap[1] = p2;
																																																																																																																							ArtCap[2] = p3;
																																																																																																																							ArtCap[3] = p4;
																																																																																																																							NLine( f1 );
																																																																																																																							Line++;
																																																																																																																						}
																																																																																																																						else
																																																																																																																							if ( !strcmp( gx, "MAXINDEPO" ) )
																																																																																																																							{
																																																																																																																								z = Gscanf( f1, "%d%d", &p1, &p2 );
																																																																																																																								if ( z != 2 )NEPar( name, Line, gx, 1 );
																																																																																																																								if ( p1 > 3 )InvPar( name, Line, gx, "first parameter must be<4" );
																																																																																																																								NInArtDepot = p2;
																																																																																																																								ArtSet = p1 + 1;
																																																																																																																								NLine( f1 );
																																																																																																																								Line++;
																																																																																																																							}
																																																																																																																							else
																																																																																																																							{
																																																																																																																								sprintf( gy, "%s,line %d : Unknown command : %s", name, Line, gx );
																																																																																																																								ErrM( gy );
																																																																																																																							};
					};
				}
			}
			else
			{
				if ( !strcmp( gx, "[END]" ) )
				{
					finish = true;
				}
				NLine( f1 );
				Line++;
			}
		}while ( ( !finish ) && z != -1 );
		Gclose( f1 );
		return true;
	}
	else
	{
		sprintf( gx, "Could not open new monster discription file %s.nm", name );
		ErrM( gx );
		return false;
	};
};
extern bool ProtectionMode;
void LoadAllNewMonsters()
{
	ProtectionMode = 1;
	GFILE* f1 = Gopen( "nmlist.txt", "r" );
	ProtectionMode = 0;

	if ( f1 )
	{
		char mname[128];
		int z;
		do
		{
			mname[0] = 0;
			z = Gscanf( f1, "%s", mname );
			if ( mname[0] )
			{
				if ( NewMon[NNewMon].CreateFromFile( mname ) )
				{
					NewMon[NNewMon].MyIndex = NNewMon;
					MonNames[NNewMon] = new char[strlen( NewMon[NNewMon].Name ) + 1];
					strcpy( MonNames[NNewMon], NewMon[NNewMon].Name );
					NNewMon++;
				};
			};
		} while ( z != -1 && mname[0] );
	}
	else
	{
		ErrM( "Could not open NMLIST.TXT" );
	};
}

bool CreateGOByName( GeneralObject* GO, char* name, char* newName )
{
	Visuals* VS = (Visuals*) GO;

	memset( GO, 0, sizeof Visuals );

	for ( int i = 0; i < NNewMon; i++ )
	{
		if ( !strcmp( MonNames[i], name ) )
		{
			NewMonster* NM = &NewMon[i];
			InitFlags( GO );
			//VS->info.Basic.MinDamage=NM->MinDamage;
			//VS->info.Basic.MaxDamage=NM->MaxDamage;
			VS->newMons = NM;
			//VS->capMagic=NM->Magic;
			//VS->info.Basic.MaxLife=NM->Life;
			//VS->info.Basic.MaxShield=NM->Shield;
			//VS->info.Basic.AttackRange=NM->AttRange;
			//VS->VisRadius=NM->VisRange;
			VS->newMons = NM;
			for ( int j = 1; j < 4; j++ )
			{
				GO->ResourceID[j] = 255;
				GO->ResAmount[j] = 0;
			};
			GO->Message = NM->Message;
			GO->MonsterID = new char[strlen( newName ) + 1];
			strcpy( GO->MonsterID, newName );
			GO->NStages = byte( NM->Time );
			GO->cost = byte( NM->Res1cost );
			GO->Kind = byte( NM->Kind );
			GO->ResourceID[0] = 1;
			GO->ResAmount[0] = NM->Res1cost;;
			GO->ManualDisable = 0;
			if ( NM->Res2cost )
			{
				GO->ResourceID[1] = 2;
				GO->ResAmount[1] = NM->Res2cost;
			};
			if ( NM->Res3cost )
			{
				GO->ResourceID[2] = 3;
				GO->ResAmount[2] = NM->Res3cost;
			};
			//GO->SpotSize=NM->VisRange;
			//GO->SpotType=2;
			GO->NStages = byte( NM->ProduceStages );
			GO->MoreCharacter = new AdvCharacter;
			GO->CondEnabled = false;
			GO->Enabled = false;
			GO->Branch = 0xFF;
			CreateAdvCharacter( GO->MoreCharacter, GO->newMons );
			return true;
		};
	};
	char ddd[128];
	sprintf( ddd, "Could not create GeneralObject by name: %s", name );
	ErrM( ddd );
	return false;
};
//----------------Elements of the terrain: trees,stones,...-----------
int  SprGroup::GetIndexByName( char* Name )
{
	if ( !strcmp( Name, "NONE" ) )return 0xFFFF;
	for ( int i = 0; i < NSpr; i++ )
	{
		if ( ObjChar[i].Name && !strcmp( ObjChar[i].Name, Name ) )return i;
	};
	return -1;
};
int GetResByName( char* gy )
{
	if ( !strcmp( gy, "WOOD" ) )return 0;
	else if ( !strcmp( gy, "GOLD" ) )return 1;
	else if ( !strcmp( gy, "STONE" ) )return 2;
	else if ( !strcmp( gy, "FOOD" ) )return 3;
	else if ( !strcmp( gy, "IRON" ) )return 4;
	else if ( !strcmp( gy, "COAL" ) )return 5;
	else if ( !strcmp( gy, "NONE" ) )return 0xFE;
	else if ( !strcmp( gy, "REMOVE" ) )return 0xFF;
	else return -1;
};
void SprGroup::LoadSprites( char* fname )
{
	char fn[128];
	sprintf( fn, "%s.lst", fname );
	GFILE* f1 = Gopen( fn, "r" );

	char str[128];
	char gy[128];
	char gx[128];
	int z, nl, dx, dy, rr, z1, z2, parm1;
	int Line = 1;
	int p4, p5;
	if ( f1 )
	{
		z = Gscanf( f1, "%s%d", gy, &nl );
		if ( z != 2 )IncPar( fname, 0, "?" );
		NSpr = nl;
		Items = new word[nl];
		for ( int i = 0; i < nl; i++ )Items[i] = i;
		Dx = new short[nl];
		Dy = new short[nl];
		Radius = new short[nl];
		ObjChar = new ObjCharacter[nl];
		memset( ObjChar, 0, nl * sizeof ObjCharacter );
		NLine( f1 );
		Line++;
		UpConv( gy );
		if ( !strcmp( gy, "COMPLEX" ) )
		{
			//Loading complex
			FileID = 0xFFFF;
			for ( int i = 0; i < nl; i++ )
			{
				z = Gscanf( f1, "%s", gx );
				if ( gx[0] != 0 && gx[0] != '/' )
				{
					z = Gscanf( f1, "%d%d%d%s%d%d%d%d", &dx, &dy, &rr, str, &p4, &p5, &z1, &z2 );
					if ( z != 8 )IncPar( fname, Line, gy );
					Dx[i] = dx;
					Dy[i] = dy;
					Radius[i] = rr;
					ObjCharacter* OBC = &ObjChar[i];
					OBC->Name = new char[strlen( gx ) + 1];
					strcpy( OBC->Name, gx );
					OBC->ResType = 0xFF;
					OBC->WorkRadius = 32;
					OBC->WorkAmount = 0;
					OBC->ResPerWork = 0;
					OBC->WNextObj = i;
					OBC->DNextObj = i;
					OBC->DamageAmount = 0;
					OBC->IntResType = 0xFF;
					OBC->IntResPerWork = 0;
					OBC->IntWorkRadius = 32;
					OBC->TimeAmount = 0;
					OBC->TNextObj = i;
					OBC->Parts = p4;
					if ( !p5 )p5 = 1;
					OBC->Delay = p5;
					OBC->Z0 = z1;
					OBC->DZ = z2;
					OBC->Stand = GetNewAnimationByName( str );
					char ggg[128];
					sprintf( ggg, "%sS", str );
					OBC->Shadow = GetNewAnimationByName( ggg );
					if ( OBC->Stand )
					{
						OBC->Frames = div( OBC->Stand->NFrames, p4 ).quot;
						if ( !OBC->Frames )
						{
							sprintf( gx, "%s, Line %d : %s : Too many parts (more then sprites in animation) : %s", fname, Line, gy, str );
							ErrM( gx );
						};
					}
					else
					{
						sprintf( gx, "%s, Line %d : %s : Unknown Animation : %s", fname, Line, gy, str );
						ErrM( gx );
					};
					NLine( f1 );
					Line++;
				}
				else
				{
					NLine( f1 );
					Line++;
					i--;
				};
			};
		}
		else
		{
			//search for RLC file in global table
			int nr = GPS.PreLoadGPImage( gy, false );
			FileID = nr;
			for ( int i = 0; i < nl; i++ )
			{
				z = Gscanf( f1, "%s", gx );
				if ( gx[0] != 0 && gx[0] != '/' )
				{
					z = Gscanf( f1, "%d%d%d", &dx, &dy, &rr );
					if ( z != 3 )IncPar( fname, Line, "" );
					Dx[i] = dx;
					Dy[i] = dy;
					Radius[i] = rr;
					ObjCharacter* OBC = &ObjChar[i];
					OBC->Name = new char[strlen( gx ) + 1];
					strcpy( OBC->Name, gx );
					OBC->ResType = 0xFF;
					OBC->WorkAmount = 0;
					OBC->WorkRadius = 32;
					OBC->ResPerWork = 0;
					OBC->WNextObj = i;
					OBC->DNextObj = i;
					OBC->DamageAmount = 0;
					OBC->IntResType = 0xFF;
					OBC->IntResPerWork = 0;
					OBC->IntWorkRadius = 32;
					OBC->TimeAmount = 0;
					OBC->TNextObj = i;
					OBC->Parts = 1;
					OBC->Z0 = 0;
					OBC->DZ = 0;
					OBC->Frames = 1;
					OBC->Stand = nullptr;
					OBC->Discret = 1;
					NLine( f1 );
					Line++;
				}
				else
				{
					NLine( f1 );
					Line++;
				};
			};
		};
	}
	else
	{
		sprintf( str, "Could not open sprites discription : %s", fn );
		ErrM( str );
	};
	Gclose( f1 );
	//loading RSR file
	sprintf( fn, "%s.rsr", fname );
	f1 = Gopen( fn, "r" );

	if ( f1 )
	{
		int mode = 0;
		do
		{
			z = Gscanf( f1, "%s", gx );
			if ( z != 1 )IncPar( fn, 0, "?" );
			if ( gx[0] == '[' )
			{
				if ( !strcmp( gx, "[WORKTRANSFORM]" ) )mode = 1;
				else
					if ( !strcmp( gx, "[FIRETRANSFORM]" ) )mode = 2;
					else
						if ( !strcmp( gx, "[SOURCES]" ) )mode = 3;
						else
							if ( !strcmp( gx, "[TIMETRANSFORM]" ) )mode = 4;
							else
								if ( !strcmp( gx, "[INTERNAL_SOURCE]" ) )mode = 5;
								else
									if ( !strcmp( gx, "[DISCRET]" ) )mode = 6;
									else
										if ( !strcmp( gx, "[LOCK]" ) )mode = 7;
										else
											if ( !strcmp( gx, "[END]" ) )mode = 255;
											else
											{
												sprintf( gy, "Unknown section in <%s.rsr> : %s", fname, gx );
												ErrM( gy );
											};
			}
			else
			{
				if ( gx[0] == '/' )NLine( f1 );
				else
				{
					switch ( mode )
					{
					case 1://[WORKTRANSFORM]
					case 2://[FIRETRANSFORM]
					case 4://[TIMETRANSFORM]
						z = Gscanf( f1, "%s%d", gy, &parm1 );
						if ( z != 2 )IncPar( fn, 0, "[WORK/FIRE/TIMETRANSFORM]" );
						z1 = GetIndexByName( gx );
						if ( z1 == -1 )
						{
							sprintf( str, "%s : Unknown sprite ID :%s", fn, gx );
							ErrM( str );
						};
						z2 = GetIndexByName( gy );
						if ( z2 == -1 )
						{
							sprintf( str, "%s : Unknown sprite ID :%s", fn, gy );
							ErrM( str );
						};
						if ( mode == 1 )
						{
							ObjChar[z1].WNextObj = z2;
							ObjChar[z1].WorkAmount = parm1;
						};
						if ( mode == 2 )
						{
							ObjChar[z1].DNextObj = z2;
							ObjChar[z1].DamageAmount = parm1;
						};
						if ( mode == 4 )
						{
							ObjChar[z1].TNextObj = z2;
							ObjChar[z1].TimeAmount = parm1;
						};
						break;
					case 3://[SOURCES]
						z = Gscanf( f1, "%s%d%d", gy, &parm1, &rr );
						if ( z != 3 )IncPar( fn, 0, "[SOURCES]" );
						z1 = GetIndexByName( gx );
						if ( z1 == -1 )
						{
							sprintf( str, "%s : Unknown sprite ID :%s", fn, gx );
							ErrM( str );
						};
						z2 = GetResByName( gy );
						if ( z2 == -1 )
						{
							sprintf( str, "%s : Unknown resource type :%s", fn, gy );
							ErrM( str );
						};
						ObjChar[z1].ResType = z2;
						ObjChar[z1].ResPerWork = parm1;
						ObjChar[z1].WorkRadius = rr;
						break;
					case 5://[INTERNAL_SOURCE]
						z = Gscanf( f1, "%s%d%d", gy, &parm1, &rr );
						if ( z != 3 )IncPar( fn, 0, "[INTERNAL_SOURCE]" );
						z1 = GetIndexByName( gx );
						if ( z1 == -1 )
						{
							sprintf( str, "%s : Unknown sprite ID :%s", fn, gx );
							ErrM( str );
						};
						z2 = GetResByName( gy );
						if ( z2 == -1 )
						{
							sprintf( str, "%s : Unknown resource type :%s", fn, gy );
							ErrM( str );
						};
						ObjChar[z1].IntResType = z2;
						ObjChar[z1].IntResPerWork = parm1;
						ObjChar[z1].IntWorkRadius = rr;
						break;
					case 6:
						z = Gscanf( f1, "%d", &parm1 );
						if ( z != 1 )IncPar( fn, 0, "[DISCRET]" );
						z1 = GetIndexByName( gx );
						if ( z1 == -1 )
						{
							sprintf( str, "%s : Unknown sprite ID :%s", fn, gx );
							ErrM( str );
						};
						ObjChar[z1].Discret = parm1;
						break;
					case 7://[LOCK]
						z = Gscanf( f1, "%d", &parm1 );
						if ( z != 1 )IncPar( fn, 0, "[LOCK]" );
						z1 = GetIndexByName( gx );
						if ( z1 == -1 )
						{
							sprintf( str, "%s : Unknown sprite ID :%s", fn, gx );
							ErrM( str );
						}
						else
						{
							ObjChar[z1].NLockPt = parm1;
							ObjChar[z1].LockX = new short[parm1];
							ObjChar[z1].LockY = new short[parm1];
							for ( int i = 0; i < parm1; i++ )
							{
								int x, y;
								z = Gscanf( f1, "%d%d", &x, &y );
								if ( z == 2 )
								{
									ObjChar[z1].LockX[i] = x;
									ObjChar[z1].LockY[i] = y;
								}
								else
								{
									IncPar( fn, 0, "[LOCK]" );
								};
							};
						};
						break;
					};
				};
			};
		} while ( mode != 255 );
		Gclose( f1 );
	};

};
//-----------------------------------------------------------------------//
//                                                                       //
//                       PHYSICS OF THE NEW MONSTER                      //
//           New monsters choose freedom from cells through cells!       //
//-----------------------------------------------------------------------//
byte* MCount;//amount of monsters in 4x4 cell; 16384 elements
byte* TmpMC; //amount of monsters in 4x4 cell,
						  //this is used only for force calculating
//word* NMsList;//array of ID of new monsters
word* BLDList;
void ZMem( byte* pntr, int siz )
{
	__asm {
		push	edi
		cld
		xor		eax, eax
		mov		edi, pntr
		mov		ecx, siz
		shr		ecx, 2
		or ecx, ecx
		jz		www
		//		cli
		rep		stosd
		//sti
		www : pop		edi
	};
};



//Creating list of monsters according to cells

word GetDir( int dx, int dy )
{
	int PhDir;
	if ( dx != 0 || dy != 0 )
	{
		int adx = abs( dx );
		int ady = abs( dy );
		if ( adx > ady )PhDir = byte( TAtg[div( ady << 8, adx ).quot] ); else
			PhDir = 64 - byte( TAtg[div( adx << 8, ady ).quot] );
		if ( dx < 0 )PhDir = 128 - PhDir;
		if ( dy < 0 )PhDir = 256 - PhDir;
	}
	else PhDir = 64 + 128;
	return ( PhDir + 1024 ) & 255;
};
//calculating forces, that acts to every monster
void CalculateForces()
{
	int nmon, FX, FY, FX1, FY1, ofs0, ofs1, ofs2, R1, R2, RE1, RE2;
	int ox, oy, nmon1, dr;
	OneObject* OB;
	OneObject* EN;
	int NNN = VAL_MAXCX*( VAL_MAXCX - 1 ) - 1;
	int NNN2 = ( 1 + VAL_MAXCX )*MAXINCELL;
	int NNN3 = ( VAL_MAXCX - 3 )*MAXINCELL;
	for ( int i = VAL_MAXCX + 1; i < NNN; i++ )
	{
		nmon = TmpMC[i];
		ofs2 = i << SHFCELL;
		while ( nmon )
		{
			nmon--;
			TmpMC[i]--;
			OB = Group[GetNMSL( ofs2 + nmon )];
			if ( OB )
			{
				ox = OB->RealX;
				oy = OB->RealY;
				R1 = OB->Radius1;
				R2 = OB->Radius2;
				ofs0 = i - 1 - VAL_MAXCX;
				ofs1 = ofs2 - NNN2;
				FX = 0; FY = 0;
				for ( int ddy = 0; ddy < 3; ddy++ )
				{
					for ( int ddx = 0; ddx < 3; ddx++ )
					{
						nmon1 = TmpMC[ofs0];
						if ( nmon1 )
						{
							for ( int p = 0; p < nmon1; p++ )
							{
								EN = Group[GetNMSL( ofs1 + p )];
								if ( EN )
								{
									//CENRAL LOOP !!!
									dr = Norma( ox - EN->RealX, oy - EN->RealY );
									RE1 = EN->Radius1;
									RE2 = EN->Radius2;
									FX1 = 0;
									FY1 = 0;
									if ( RE1 + R1 >= dr )
									{
										//COLLISION!!!!!!!
										EN->Collision = true;
										OB->Collision = true;
										if ( dr )
										{
											//Force, that acts on OB
											FX1 = div( ( ox - EN->RealX ) << 8, dr ).quot;
											FY1 = div( ( oy - EN->RealY ) << 8, dr ).quot;
										}
										else
										{
											FX = 256;
											FY = 256;
										};
									}
									else
									{
										if ( RE2 + R2 >= dr )
										{
											//Inside interaction zone
											//Force, that acts on OB
											FX1 = div( ( ox - EN->RealX ) << 5, dr ).quot;
											FY1 = div( ( oy - EN->RealY ) << 5, dr ).quot;
										};
									};
									if ( FX1 || FY1 )
									{
										EN->ForceX -= FX1;
										EN->ForceY -= FY1;
										OB->ForceX += FX1;
										OB->ForceY += FY1;
									};
								};
							};
						};
						ofs0++;
						ofs1 += MAXINCELL;
					};
					ofs0 += VAL_MAXCX - 3;
					ofs1 += NNN3;
				};
			};

		};
	};
};
//-----------------------VISUALISATION------------------------
//----VISUALISATION DATA----
/*
class ZBuffer{
	short UnitLogY[512];
	short UnitX[512];
	short UnitY[512];
	word  UFileID[512];
	word  USpriteID[512];
	word  NURef[512];
	OneObject* OBJS[512];
	word NVUnits;
	word FirstUN;
	void AddToZBuffer(short uy,short x,short y,OneObject* OB,word FileID,word Sprite);
	void ShowZBuffer();
	void InitZBuffer();
};
void ZBuffer::InitZBuffer(){
	NVUnits=0;
	FirstUN=0;
};
void ZBuffer::AddToZBuffer(short uy,short x,short y,OneObject* OB,word FileID,word Sprite){
	if(NVUnits>=512)return;
	if(!NVUnits){
		NURef[0]=0xFFFF;
		UnitLogY[0]=uy;
		UnitX[0]=x;
		UnitY[0]=y;
		UFileID[0]=FileID;
		USpriteID[0]=Sprite;
		OBJS[0]=OB;
		NVUnits=1;
	}else{
		word un=FirstUN;
		word un0=0xFFFF;
		word un1;
		do{
			un1=NURef[un];
			if(uy<UnitLogY[un]){
				if(un0!=0xFFFF)
					NURef[un0]=NVUnits;
				else FirstUN=NVUnits;
				NURef[NVUnits]=un;
				UnitLogY[NVUnits]=uy;
				UnitX[NVUnits]=x;
				UnitY[NVUnits]=y;
				OBJS[NVUnits]=OB;
				UFileID[NVUnits]=FileID;
				USpriteID[NVUnits]=Sprite;
				NVUnits++;
				return;
			};
			if(un1==0xFFFF){
				NURef[un]=NVUnits;
				NURef[NVUnits]=0xFFFF;
				UnitLogY[NVUnits]=uy;
				UnitX[NVUnits]=x;
				UnitY[NVUnits]=y;
				UFileID[NVUnits]=FileID;
				USpriteID[NVUnits]=Sprite;
				OBJS[NVUnits]=OB;
				NVUnits++;
				return;
			};
			un0=un;
			un=un1;
		}while(true);
	};
};
void ZBuffer::ShowZBuffer(){
	if(!NVUnits)return;
	int Lx1=smaplx<<Shifter;
	int Ly1=mul3(smaply)<<(Shifter-2);
	SetRLCWindow(smapx,smapy,Lx1,Ly1,SCRSizeX);
	word fu=FirstUN;
	word spr,FID;
	do{
		spr=USpriteID[fu];
		FID=UFileID[fu];
		//Shadows
		OneObject* OB=OBJS[fu];
		if(OB&&OB->Selected&&OB->NNUM==MyNation){
			if(MiniMode)DrawMiniMarker(OB);
			else DrawMarker(OB);
		};
		GPS.ShowGP(smapx+UnitX[fu],smapy+UnitY[fu],FID,spr,0);
		fu=NURef[fu];
	}while(fu!=0xFFFF);
};
ZBuffer UpperZBF;
ZBuffer MediumZ[32];
ZBuffer LowerZBF;
*/
short UnitLogY[4096];
short UnitLogX[4096];
short UnitX[4096];
short UnitY[4096];
word  UFileID[4096];
word  USpriteID[4096];
word  NURef[4096];
int   UParam1[4096];
int   UParam2[4096];
OneObject* OBJS[4096];
word NVUnits;
word FirstUN;
// - - - - -//-------\\- - - - - 
//----------\\Hashing//----------
word Hash16[4096];
word Hash64[1024];
word Hash256[256];
void InitHash()
{
	memset( Hash16, 0, sizeof Hash16 );
	memset( Hash64, 0, sizeof Hash64 );
	memset( Hash256, 0, sizeof Hash256 );
};

//----------//-------\\----------
void InitVisual()
{
	NVUnits = 0;
	FirstUN = 0;
	InitHash();
};
/*
void AddToVisual(short uy,short x,short y,OneObject* OB,word FileID,word Sprite,int Param1,int Param2){
	int ux=x;
	if(!NVUnits){
		NURef[0]=0xFFFF;
		UnitLogY[0]=uy;
		UnitX[0]=x;
		UnitY[0]=y;
		UParam1[0]=Param1;
		UParam2[0]=Param2;
		UFileID[0]=FileID;
		USpriteID[0]=Sprite;
		OBJS[0]=OB;
		NVUnits=1;
		int uuy=(int(uy)+32768)>>4;
		Hash16[uuy]=0;
		Hash64[uuy>>2]=0;
		Hash256[uuy]>>4=0;
	}else{
		word un;
		int uuy=(int(uy)+32768)>>4;
		un=Hash16[uuy];
		if(un==0xFFFF){
			un=Hash64[uuy>>2];
			if(un==0xFFFF){
				un=Hash256[uuy>>4];
			};
		};
		if(un==0xFFFF)un=FirstUN;
		word un0=0xFFFF;
		word un1;
		do{
			un1=NURef[un];
			if(uy<UnitLogY[un]){
				if(un0!=0xFFFF)
					NURef[un0]=NVUnits;
				else FirstUN=NVUnits;
				NURef[NVUnits]=un;
				UnitLogY[NVUnits]=uy;
				UnitLogX[NVUnits]=ux;
				UnitX[NVUnits]=x;
				UnitY[NVUnits]=y;
				OBJS[NVUnits]=OB;
				UFileID[NVUnits]=FileID;
				USpriteID[NVUnits]=Sprite;
				UParam1[NVUnits]=Param1;
				UParam2[NVUnits]=Param2;
				if(un==Hash16[uuy]){
					Hash16[uuy]=NVUnits;
					uuy>>=2;
					if(Hash64[uuy]==un){
						Hash64[uuy]=NVUnits;
						uuy>>=2;
						if(Hash256[uuy]==un){
							hash256[uuy]=NVUnits;
						};
					};
				};
				NVUnits++;
				return;
			};
			if(un1==0xFFFF){
				NURef[un]=NVUnits;
				NURef[NVUnits]=0xFFFF;
				UnitLogY[NVUnits]=uy;
				UnitX[NVUnits]=x;
				UnitY[NVUnits]=y;
				UFileID[NVUnits]=FileID;
				USpriteID[NVUnits]=Sprite;
				UParam1[NVUnits]=Param1;
				UParam2[NVUnits]=Param2;
				OBJS[NVUnits]=OB;
				NVUnits++;
				return;
			};
			un0=un;
			un=un1;
		}while(true);
	};
};
*/
void AddToVisual( short uy, short x, short y, OneObject* OB, word FileID, word Sprite )
{
	//assert(RLCNSpr[FileID]>(Sprite&4095));
	if ( RLCNSpr[FileID] <= ( Sprite & 4095 ) )return;
	if ( !NVUnits )
	{
		NURef[0] = 0xFFFF;
		UnitLogY[0] = uy;
		UnitX[0] = x;
		UnitY[0] = y;
		UParam1[0] = 0;
		UParam2[0] = 0;
		UFileID[0] = FileID;
		USpriteID[0] = Sprite;
		OBJS[0] = OB;
		NVUnits = 1;
	}
	else
	{
		word un = FirstUN;
		word un0 = 0xFFFF;
		word un1;
		do
		{
			un1 = NURef[un];
			if ( uy < UnitLogY[un] )
			{
				if ( un0 != 0xFFFF )
					NURef[un0] = NVUnits;
				else FirstUN = NVUnits;
				NURef[NVUnits] = un;
				UnitLogY[NVUnits] = uy;
				UnitX[NVUnits] = x;
				UnitY[NVUnits] = y;
				OBJS[NVUnits] = OB;
				UFileID[NVUnits] = FileID;
				USpriteID[NVUnits] = Sprite;
				UParam1[NVUnits] = 0;
				UParam2[NVUnits] = 0;
				NVUnits++;
				return;
			};
			if ( un1 == 0xFFFF )
			{
				NURef[un] = NVUnits;
				NURef[NVUnits] = 0xFFFF;
				UnitLogY[NVUnits] = uy;
				UnitX[NVUnits] = x;
				UnitY[NVUnits] = y;
				UFileID[NVUnits] = FileID;
				USpriteID[NVUnits] = Sprite;
				UParam1[NVUnits] = 0;
				UParam2[NVUnits] = 0;
				OBJS[NVUnits] = OB;
				NVUnits++;
				return;
			};
			un0 = un;
			un = un1;
		} while ( true );
	};
};
void AddToVisual( short uy, short x, short y, OneObject* OB, word FileID, word Sprite, int Param1, int Param2 )
{
	if ( !NVUnits )
	{
		NURef[0] = 0xFFFF;
		UnitLogY[0] = uy;
		UnitX[0] = x;
		UnitY[0] = y;
		UParam1[0] = Param1;
		UParam2[0] = Param2;
		UFileID[0] = FileID;
		USpriteID[0] = Sprite;
		OBJS[0] = OB;
		NVUnits = 1;
	}
	else
	{
		word un = FirstUN;
		word un0 = 0xFFFF;
		word un1;
		do
		{
			un1 = NURef[un];
			if ( uy < UnitLogY[un] )
			{
				if ( un0 != 0xFFFF )
					NURef[un0] = NVUnits;
				else FirstUN = NVUnits;
				NURef[NVUnits] = un;
				UnitLogY[NVUnits] = uy;
				UnitX[NVUnits] = x;
				UnitY[NVUnits] = y;
				OBJS[NVUnits] = OB;
				UFileID[NVUnits] = FileID;
				USpriteID[NVUnits] = Sprite;
				UParam1[NVUnits] = Param1;
				UParam2[NVUnits] = Param2;
				NVUnits++;
				return;
			};
			if ( un1 == 0xFFFF )
			{
				NURef[un] = NVUnits;
				NURef[NVUnits] = 0xFFFF;
				UnitLogY[NVUnits] = uy;
				UnitX[NVUnits] = x;
				UnitY[NVUnits] = y;
				UFileID[NVUnits] = FileID;
				USpriteID[NVUnits] = Sprite;
				UParam1[NVUnits] = Param1;
				UParam2[NVUnits] = Param2;
				OBJS[NVUnits] = OB;
				NVUnits++;
				return;
			};
			un0 = un;
			un = un1;
		} while ( true );
	};
};
void ShowRLCItemTrans8( int x, int y, lpRLCTable lprt, int n );
void ShowVisual()
{
	if ( !NVUnits )return;
	int Lx1 = smaplx << Shifter;
	int Ly1 = mul3( smaply ) << ( Shifter - 2 );
	SetRLCWindow( smapx, smapy, Lx1, Ly1, SCRSizeX );
	word fu = FirstUN;
	word spr, FID;
	do
	{
		spr = USpriteID[fu];
		FID = UFileID[fu];
		//Shadows
		OneObject* OB = OBJS[fu];
		if ( OB&&OB->Selected&GM( MyNation ) )
		{
			if ( MiniMode )DrawMiniMarker( OB );
			else DrawMarker( OB );
		};
		GPS.ShowGP( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, 0 );
		fu = NURef[fu];
	} while ( fu != 0xFFFF );
};
//Param1 values:
//bits 0..2-visibility options
//0-simple
//1-pulsing(Param2-pointer to pulsing data)
//2-transparent(Param2-pointer to transparency data)
//3-encoded with palette(Param2-pointer to palette)
//4-shading with mask(Param2-pointer to gradient data)
extern byte fog[8192 + 1024];
extern byte wfog[8192];
extern byte rfog[8192];
extern byte darkfog[40960];
extern byte yfog[8192];
extern byte trans8[65536];
void AddToVisual( short uy, short x, short y, OneObject* OB, word FileID, word Sprite, int Options )
{
	int CType = Options & 0x0F;
	int CProp = Options & 0xF0;
	int Param1 = CType;
	int Param2 = 0;
	switch ( CType )
	{
	case AV_PULSING:
		switch ( CProp )
		{
		case AV_RED:
			Param2 = int( yfog );
			break;
		case AV_WHITE:
			Param2 = int( wfog );
			break;
		case AV_DARK:
			Param2 = int( fog + 1024 );
			break;
		default:
			Param1 = 0;
		};
		break;
	case AV_TRANSPARENT:
		break;
	case AV_PALETTE:
		switch ( CProp )
		{
		case AV_RED:
			Param2 = int( yfog + 2048 );
			break;
		case AV_WHITE:
			Param2 = int( wfog + 2048 );
			break;
		case AV_DARK:
			Param2 = int( fog + 1024 + 2048 );
			break;
		default:
			Param1 = 0;
		};
		break;
	case AV_GRADIENT:
		switch ( CProp )
		{
		case AV_RED:
			Param2 = int( yfog );
			break;
		case AV_WHITE:
			Param2 = int( wfog );
			break;
		case AV_DARK:
			Param2 = int( fog + 1024 );
			break;
		default:
			Param1 = 0;
		};
		break;
	};
	if ( !NVUnits )
	{
		NURef[0] = 0xFFFF;
		UnitLogY[0] = uy;
		UnitX[0] = x;
		UnitY[0] = y;
		UParam1[0] = Param1;
		UParam2[0] = Param2;
		UFileID[0] = FileID;
		USpriteID[0] = Sprite;
		OBJS[0] = OB;
		NVUnits = 1;
	}
	else
	{
		word un = FirstUN;
		word un0 = 0xFFFF;
		word un1;
		do
		{
			un1 = NURef[un];
			if ( uy < UnitLogY[un] )
			{
				if ( un0 != 0xFFFF )
					NURef[un0] = NVUnits;
				else FirstUN = NVUnits;
				NURef[NVUnits] = un;
				UnitLogY[NVUnits] = uy;
				UnitX[NVUnits] = x;
				UnitY[NVUnits] = y;
				OBJS[NVUnits] = OB;
				UFileID[NVUnits] = FileID;
				USpriteID[NVUnits] = Sprite;
				UParam1[NVUnits] = Param1;
				UParam2[NVUnits] = Param2;
				NVUnits++;
				return;
			};
			if ( un1 == 0xFFFF )
			{
				NURef[un] = NVUnits;
				NURef[NVUnits] = 0xFFFF;
				UnitLogY[NVUnits] = uy;
				UnitX[NVUnits] = x;
				UnitY[NVUnits] = y;
				UFileID[NVUnits] = FileID;
				USpriteID[NVUnits] = Sprite;
				UParam1[NVUnits] = Param1;
				UParam2[NVUnits] = Param2;
				OBJS[NVUnits] = OB;
				NVUnits++;
				return;
			};
			un0 = un;
			un = un1;
		} while ( true );
	};
};
void ShowVisualLess( int yend )
{
	if ( !NVUnits )return;
	int Lx1 = smaplx << Shifter;
	int Ly1 = mul3( smaply ) << ( Shifter - 2 );
	SetRLCWindow( smapx, smapy, Lx1, Ly1, SCRSizeX );
	word fu = FirstUN;
	word spr, FID;
	bool LocTrans;
	int ylog;
	do
	{
		ylog = UnitLogY[fu];
		if ( ylog < yend )
		{
			NVUnits--;
			spr = USpriteID[fu];
			FID = UFileID[fu];
			//if(spr>=RLCNSpr[FID])spr=RLCNSpr[FID]-1;
			int par1 = UParam1[fu];
			int par2 = UParam2[fu];
			byte nat = 0;
			//Shadows
			LocTrans = false;
			OneObject* OB = OBJS[fu];
			if ( OB )
			{
				nat = OB->NNUM;
				if ( OB->Selected&GM( MyNation ) )
				{
					if ( MiniMode )DrawMiniMarker( OB );
					else DrawMarker( OB );
				};
			};
			if ( TransMode&&OB&&OB->NewBuilding )LocTrans = true;
			switch ( par1 )
			{
			case AV_NORMAL:
				if ( LocTrans )GPS.ShowGPTransparent( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, nat );
				else GPS.ShowGP( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, nat );
				break;
			case AV_PULSING:
			{
				int NNN = 5 + int( 8 * ( sin( double( GetTickCount() ) / 100 ) + 2 ) );
				GPS.ShowGPPal( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, nat, (byte*) par2 + ( NNN << 8 ) );
			};
			break;
			case AV_TRANSPARENT:
				GPS.ShowGPTransparent( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, nat );
				break;
			case AV_PALETTE:
				GPS.ShowGPPal( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, nat, (byte*) par2 );
				break;
			case AV_GRADIENT:
				GPS.ShowGPGrad( smapx + UnitX[fu], smapy + UnitY[fu], FID, spr, nat, (byte*) par2 );
				break;
			};
			fu = NURef[fu];
			FirstUN = fu;
		};
	} while ( fu != 0xFFFF && ylog < yend );
};
bool PInside( int x, int y, int x1, int y1, int xp, int yp );
extern int time1;
extern int time2;
extern int tmtmt;
#ifdef CONQUEST
void QShowFiresEx( OneObject* OB, int x0, int y0, int FirIDX, NewAnimation** PreFires, NewAnimation** FiresAnm, int NFiresAnm, int FIndex )
{
	if ( !NFiresAnm )return;
	//determining the current stage
	word ms = OB->StageState;
	int curst = 0;
	NewMonster* NM = OB->newMons;
	int LF0 = 0;
	int LF1 = OB->Ref.General->MoreCharacter->Life;
	int LF00 = LF0;
	int LF10 = LF1;
	for ( int i = 0; i < MaxAStages; i++ )
	{
		LF0 = LF1;
		word ms1 = ( ms >> ( i * 3 ) ) & 7;
		if ( ms1 == 2 )
		{
			LF1 += NM->CompxCraft->Stages[i].AddPoints;
			curst = i + 1;
			LF00 = LF0;
			LF10 = LF1;
		};
	};
	int NFir;
	if ( curst )NFir = NM->CompxCraft->Stages[curst - 1].NFires[FirIDX];
	else NFir = NM->NFires[FirIDX];

	int scdy = mapy << 4;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	if ( NFir )
	{
		int L2 = ( LF10 - LF00 ) >> 1;
		int LM = ( LF10 + LF00 ) >> 1;

		//L2+=L2>>1;
		if ( OB->Life >= LM )return;

		int NOG = NFir*( LM - OB->Life ) / L2;
		if ( NOG > NFir )NOG = NFir;
		FireInfo* GFIN = OB->IFire;
		if ( GFIN&&GFIN->BStage != curst )
		{
			GFIN->Erase();
			free( GFIN );
			OB->IFire = nullptr;
			GFIN = nullptr;
		};
		if ( !GFIN )
		{
			GFIN = new FireInfo;
			for ( int k = 0; k < 2; k++ )GFIN->Objs[k] = 0;
			OB->IFire = GFIN;
			GFIN->BStage = curst;
		};
		FireObjectInfo* FIN = GFIN->Objs[FIndex];
		if ( !FIN )
		{
			FIN = new FireObjectInfo;
			GFIN->Objs[FIndex] = FIN;
			FIN->FireSprite = new byte[NFir];
			FIN->FireStage = new byte[NFir];
			FIN->NeedFires = NOG;
			FIN->RealFires = 0;
			FIN->delay = 8 + ( rand() & 7 );
			/*
			if(NOG>4){
				FIN->RealFires=NOG;
				for(int i=0;i<NOG;i++){
					NewAnimation* NANM=FiresAnm[div(i,NFiresAnm).rem];
					if(NANM->NFrames){
						FIN->FireStage[i]=2;
						FIN->FireSprite[i]=div(i*133+tmtmt,NANM->NFrames).rem;
						FIN->delay=8+(rand()&15);
					};
				};
			};
			*/
		}
		else
		{
			FIN->NeedFires = NOG;
		};
		if ( FIN->NeedFires > FIN->RealFires )
		{
			int J = FIN->RealFires;
			if ( FIN->delay )FIN->delay--;
			else
			{
				FIN->RealFires++;
				NewAnimation* NANM = PreFires[J%NFiresAnm];
				if ( NANM )
				{
					FIN->FireStage[J] = 1;
					FIN->FireSprite[J] = 0;
				}
				else
				{
					FIN->FireStage[J] = 2;
					FIN->FireSprite[J] = 0;
				};
				FIN->delay = 8 + ( rand() & 15 );
			};
			for ( int p = 0; p < J; p++ )if ( FIN->FireStage[p] == 3 )
			{
				FIN->FireStage[p] = 2;
				FIN->FireSprite[p] = 0;
			};
		};
		if ( FIN->NeedFires < FIN->RealFires )
		{
			FIN->RealFires = FIN->NeedFires;
		};
		for ( int i = 0; i < FIN->RealFires; i++ )
		{
			int ANI = div( i, NFiresAnm ).rem;
			byte sta = FIN->FireStage[i];
			NewAnimation* NANM = nullptr;
			FIN->FireSprite[i]++;
			switch ( sta )
			{
			case 1:
				NANM = PreFires[ANI];
				if ( FIN->FireSprite[i] == NANM->NFrames )
				{
					FIN->FireSprite[i] = 0;
					FIN->FireStage[i] = 2;
					sta = 2;
				};
				break;
			case 2:
				NANM = FiresAnm[ANI];
				if ( FIN->FireSprite[i] == NANM->NFrames )
				{
					FIN->FireSprite[i] = 0;
				};
				break;
			};
			switch ( sta )
			{
			case 1:
				NANM = PreFires[ANI];
				break;
			case 2:
				NANM = FiresAnm[ANI];
				break;
			case 3:
				NANM = PostFires[0][ANI];
			};
			if ( NANM&&NANM->NFrames )
			{
				int csp = FIN->FireSprite[i];
				NewFrame* NF = &NANM->Frames[csp];
				int x = x0; +NM->FireX[FIndex][i];
				int y = y0; +NM->FireY[FIndex][i];
				if ( curst == 0 )
				{
					x += NM->FireX[FIndex][i];
					y += NM->FireY[FIndex][i];
				}
				else
				{
					x += NM->CompxCraft->Stages[curst - 1].FireX[FIndex][i];
					y += NM->CompxCraft->Stages[curst - 1].FireY[FIndex][i];
				};
				PlayAnimation( NANM, csp, x + ( mapx << 5 ), y + ( mapy << 4 ) );
				AddPoint( x, ( OB->RealY >> 5 ) - scdy + 200 + i, x + NF->dx, y + NF->dy, nullptr, NF->FileID, NF->SpriteID, 0, 0 );
			};
		};
	};
};
#endif
void QShowFires( OneObject* OB, int x0, int y0, int FirIDX, NewAnimation** PreFires, NewAnimation** FiresAnm, int NFiresAnm, int FIndex )
{
	if ( !NFiresAnm )return;
#ifdef CONQUEST
	if ( OB->StageState )
	{
		QShowFiresEx( OB, x0, y0, FirIDX, PreFires, FiresAnm, NFiresAnm, FIndex );
		return;
	};
#endif
	int NFir = OB->newMons->NFires[FirIDX];
	NewMonster* NM = OB->newMons;
	int scdy = mapy << 4;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	if ( NFir )
	{
		int L2 = ( ADC->Life >> 1 );
		L2 += L2 >> 1;
		if ( OB->Life >= L2 || !L2 )return;
		int NOG = div( NFir*( L2 - OB->Life ), L2 ).quot;
		FireInfo* GFIN = OB->IFire;
		if ( !GFIN )
		{
			GFIN = new FireInfo;
			for ( int k = 0; k < 2; k++ )GFIN->Objs[k] = 0;
			OB->IFire = GFIN;
		};
		FireObjectInfo* FIN = GFIN->Objs[FIndex];
		if ( !FIN )
		{
			FIN = new FireObjectInfo;
			GFIN->Objs[FIndex] = FIN;
			FIN->FireSprite = new byte[NFir];
			FIN->FireStage = new byte[NFir];
			FIN->NeedFires = NOG;
			FIN->RealFires = 0;
			FIN->delay = 8 + ( rand() & 7 );
			/*
			if(NOG>4){
				FIN->RealFires=NOG;
				for(int i=0;i<NOG;i++){
					NewAnimation* NANM=FiresAnm[div(i,NFiresAnm).rem];
					if(NANM->NFrames){
						FIN->FireStage[i]=2;
						FIN->FireSprite[i]=div(i*133+tmtmt,NANM->NFrames).rem;
						FIN->delay=8+(rand()&15);
					};
				};
			};
			*/
		}
		else
		{
			FIN->NeedFires = NOG;
		};
		if ( FIN->NeedFires > FIN->RealFires )
		{
			int J = FIN->RealFires;
			if ( FIN->delay )FIN->delay--;
			else
			{
				FIN->RealFires++;
				NewAnimation* NANM = PreFires[div( J, NFiresAnm ).rem];
				if ( NANM )
				{
					FIN->FireStage[J] = 1;
					FIN->FireSprite[J] = 0;
				}
				else
				{
					FIN->FireStage[J] = 2;
					FIN->FireSprite[J] = 0;
				};
				FIN->delay = 8 + ( rand() & 15 );
			};
			for ( int p = 0; p < J; p++ )if ( FIN->FireStage[p] == 3 )
			{
				FIN->FireStage[p] = 2;
				FIN->FireSprite[p] = 0;
			};
		};
		if ( FIN->NeedFires < FIN->RealFires )
		{
			FIN->RealFires = FIN->NeedFires;
		};
		for ( int i = 0; i < FIN->RealFires; i++ )
		{
			int ANI = div( i, NFiresAnm ).rem;
			byte sta = FIN->FireStage[i];
			NewAnimation* NANM = nullptr;
			FIN->FireSprite[i]++;
			switch ( sta )
			{
			case 1:
				NANM = PreFires[ANI];
				if ( FIN->FireSprite[i] == NANM->NFrames )
				{
					FIN->FireSprite[i] = 0;
					FIN->FireStage[i] = 2;
					sta = 2;
				};
				break;
			case 2:
				NANM = FiresAnm[ANI];
				if ( FIN->FireSprite[i] == NANM->NFrames )
				{
					FIN->FireSprite[i] = 0;
				};
				break;
			};
			switch ( sta )
			{
			case 1:
				NANM = PreFires[ANI];
				break;
			case 2:
				NANM = FiresAnm[ANI];
				break;
			case 3:
				NANM = PostFires[0][ANI];
			};
			if ( NANM&&NANM->NFrames )
			{
				int csp = FIN->FireSprite[i];
				NewFrame* NF = &NANM->Frames[csp];
				int x = x0 + NM->FireX[FIndex][i];
				int y = y0 + NM->FireY[FIndex][i];
				PlayAnimation( NANM, csp, x + ( mapx << 5 ), y + ( mapy << 4 ) );
				AddPoint( x, ( OB->RealY >> 5 ) - scdy + 100 + i, x + NF->dx, y + NF->dy, nullptr, NF->FileID, NF->SpriteID, 0, 0 );
			};
		};
	};
}

void ShowFires( OneObject* OB, int x0, int y0 )
{
	if ( !OB )return;
	AdvCharacter* ACR = OB->Ref.General->MoreCharacter;
	if ( OB->Stage == ACR->ProduceStages )
		for ( int k = 0; k < 2; k++ )QShowFires( OB, x0, y0, k, PreFires[k], FiresAnm[k], NFiresAnm[k], k );
}

void FireObjectInfo::Erase()
{
	free( FireSprite );
	free( FireStage );
}

void FireInfo::Erase()
{
	for ( int k = 0; k < 2; k++ )
		if ( Objs[k] )
		{
			Objs[k]->Erase();
			free( Objs[k] );
		};
}

bool ShowProducedShip( OneObject* Port, int CX, int CY );

void ShowNewMonsters()
{
	time1 = GetTickCount();

	ClearZBuffer();

	int x0 = mapx * 32;
	int y0 = mul3( mapy ) * 8;
	int Lx1 = smaplx * 32;
	int Ly1 = mul3( smaply ) * 8;
	int mpdy = mapy * 16;
	int xx, yy;

	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			if ( OB->NewMonst && !OB->Hidden )
			{
				xx = ( OB->RealX / 16 ) - x0;
				yy = ( mul3( OB->RealY ) / 64 ) - y0;
				int zz = yy;
				if ( OB->LockType == 1 )
				{
					int tmm = tmtmt % 64;

					if ( tmm > 31 )
						zz++;
				}

				if ( Mode3D )
				{
					zz -= OB->RZ + int( OB->OverEarth );
				}

				if ( xx > -128 && zz > -128 && xx < Lx1 + 128 && zz < Ly1 + 128 )
				{
					NewAnimation* NAM = OB->NewAnm;
					int csp = OB->NewCurSprite;
					if ( NAM && NAM->Enabled && csp < NAM->NFrames )
					{
						PlayAnimation( NAM, csp, xx + x0, yy + y0 );
						int octs = ( NAM->Rotations - 1 ) * 2;
						if ( !octs )
						{
							NewFrame* NF = &NAM->Frames[csp];
							AddPoint( xx, yy, ( xx + NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, NF->SpriteID, 0, 0 );
						}
						else
						{
							int octs, oc2, sesize, oc1, ocM;
							if ( NAM->Rotations & 1 )
							{
								octs = ( NAM->Rotations - 1 ) * 2;
								oc2 = NAM->Rotations - 1;
								if ( !octs )
								{
									octs = 1;
								}
								sesize = div( 255, octs << 1 ).quot;
								oc1 = octs;
								ocM = oc2;
							}
							else
							{
								octs = NAM->Rotations * 2;
								oc2 = NAM->Rotations - 1;
								ocM = oc2 + 1;
								if ( !octs )
								{
									octs = 1;
								}
								sesize = 0;
								oc1 = octs - 1;
							}

							byte dir = ( ( ( OB->RealDir + 64 + sesize ) & 255 )*octs ) >> 8;
							byte dir2 = dir;

							NewFrame* NF = &NAM->Frames[csp];
							NewMonster* NM = OB->newMons;
							bool Downpart = false;
							NewAnimation* UpAnm = nullptr;
							NewFrame* UNF = nullptr;

							if ( NM->TwoParts && ( NAM == &NM->Stand || NAM == &NM->MotionL || NAM == &NM->MotionR ) )
							{
								Downpart = true;
								UpAnm = NM->UpperPart;
								if ( UpAnm )
								{
									UNF = &UpAnm->Frames[csp];
								}
							}

							if ( OB->Sdoxlo && !NAM->NFrames )
							{
								zz -= 256;
							}

							if ( OB->Sdoxlo > 100 )
							{
								if ( dir < ocM )
								{
									AddOptPoint( ZBF_LO, xx, yy - 256,
										( xx - NF->dx ) >> ( 5 - Shifter ),
										( ( zz + NF->dy ) >> ( 5 - Shifter ) ),
										OB, NF->FileID,
										oc2 - dir + NAM->Rotations*NF->SpriteID + 4096,
										AV_TRANSPARENT );
								}
								else
								{
									dir = oc1 - dir;
									AddOptPoint( ZBF_LO, xx, yy - 256,
										( xx + NF->dx ) >> ( 5 - Shifter ),
										( ( zz + NF->dy ) >> ( 5 - Shifter ) ),
										OB, NF->FileID,
										oc2 - dir + NAM->Rotations*NF->SpriteID,
										AV_TRANSPARENT );
								}
							}
							else
							{
								if ( dir < ocM )
								{
									if ( Downpart )
									{
										AddLoPoint( ( xx - NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, oc2 - dir + NAM->Rotations*NF->SpriteID + 4096, 0, 0 );
										if ( UpAnm )
										{
											AddPoint( xx, yy + NM->UpperPartShift, ( xx - UNF->dx ) >> ( 5 - Shifter ), ( ( zz + UNF->dy ) >> ( 5 - Shifter ) ), OB, UNF->FileID, oc2 - dir + NAM->Rotations*UNF->SpriteID + 4096, 0, 0 );
										}
									}
									else
									{
										if ( NM->LinearLength )
										{
											int LL = NM->LinearLength >> 1;
											int dx = ( LL*TCos[OB->RealDir] ) >> 8;
											int dy = ( LL*TSin[OB->RealDir] ) >> 9;
											AddLine( xx - dx, yy - dy, xx + dx, yy + dy, ( xx - NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, oc2 - dir + NAM->Rotations*NF->SpriteID + 4096, 0, 0 );
										}
										else
										{
											AddPoint( xx, yy, ( xx - NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, oc2 - dir + NAM->Rotations*NF->SpriteID + 4096, 0, 0 );
										}
									}
								}
								else
								{
									dir = oc1 - dir;
									if ( Downpart )
									{
										AddLoPoint( ( xx + NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, oc2 - dir + NAM->Rotations*NF->SpriteID, 0, 0 );
										if ( UpAnm )
										{
											AddPoint( xx, yy + NM->UpperPartShift, ( xx + UNF->dx ) >> ( 5 - Shifter ), ( ( zz + UNF->dy ) >> ( 5 - Shifter ) ), OB, UNF->FileID, oc2 - dir + NAM->Rotations*UNF->SpriteID, 0, 0 );
										}
									}
									else
									{
										if ( NM->LinearLength )
										{
											int LL = NM->LinearLength >> 1;
											int dx = ( LL*TCos[OB->RealDir] ) >> 8;
											int dy = ( LL*TSin[OB->RealDir] ) >> 9;
											AddLine( xx - dx, yy - dy, xx + dx, yy + dy, ( xx + NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, oc2 - dir + NAM->Rotations*NF->SpriteID, 0, 0 );
										}
										else
										{
											AddPoint( xx, yy, ( xx + NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, oc2 - dir + NAM->Rotations*NF->SpriteID, 0, 0 );
										}
									}
								}

								if ( NM->FLAGS && !OB->Sdoxlo )
								{
									int N = NM->FLAGS->N;
									int xc = NM->FLAGS->Xr;
									int yc = -OB->NewAnm->Frames->dy;
									short* pt = NM->FLAGS->Points;
									int SIN = TSin[OB->RealDir];
									int COS = TCos[OB->RealDir];

									for ( int i = 0; i < N; i++ )
									{
										int i3 = i << 2;
										int dx = xc - pt[i3 + 1];
										int dy = xc - pt[i3];
										int dz = -pt[i3 + 2] + yc + ( dy >> 1 );
										int dx1 = ( ( dx*COS - dy*SIN ) >> 8 );
										int dy1 = ( ( dx*SIN + dy*COS ) >> 9 );
										AddPoint( xx + dx1, yy + dy1 + pt[i3 + 3], xx + dx1 - 48, yy + dy1 - dz - 28, OB, 2, ( tmtmt + 19 * i ) % 52, 0, 0 );
									}
								}

								if ( NM->Reflection )
								{
									csp = 0;
									NewAnimation* NAM = NM->Reflection;
									if ( OB->Sdoxlo )
									{
										if ( OB->NewAnm == &NM->Death )
										{
											NAM++;
										}
										else
										{
											if ( OB->NewAnm == &NM->DeathLie1 )
											{
												NAM += 2;
											}
											else
											{
												NAM = nullptr;
											}
										}
									}

									if ( NAM && NAM->Rotations != 1 )
									{
										int Rocts, Roc2, Rsesize, Roc1, RocM;
										if ( NAM->Rotations & 1 )
										{
											Rocts = ( NAM->Rotations - 1 ) * 2;
											Roc2 = NAM->Rotations - 1;

											if ( !Rocts )
											{
												Rocts = 1;
											}

											Rsesize = div( 255, Rocts << 1 ).quot;
											Roc1 = Rocts;
											RocM = Roc2;
										}
										else
										{
											Rocts = NAM->Rotations * 2;
											Roc2 = NAM->Rotations - 1;
											RocM = Roc2 + 1;

											if ( !Rocts )
											{
												Rocts = 1;
											}

											Rsesize = 0;
											Roc1 = Rocts - 1;
										}

										byte Rdir = ( ( ( OB->RealDir + 64 + Rsesize ) & 255 )*Rocts ) >> 8;
										NewFrame* NF = &NAM->Frames[csp];

										if ( Rdir < RocM )
										{
											AddOptPoint( ZBF_LO, 0, 0, ( xx - NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, Roc2 - Rdir + NAM->Rotations*NF->SpriteID + 4096, 256 + 512 );
										}
										else
										{
											Rdir = Roc1 - Rdir;
											AddOptPoint( ZBF_LO, 0, 0, ( xx + NF->dx ) >> ( 5 - Shifter ), ( ( zz + NF->dy ) >> ( 5 - Shifter ) ), OB, NF->FileID, Roc2 - Rdir + NAM->Rotations*NF->SpriteID, 256 + 512 );
										}
									}
								}
							}

							if ( NM->NVesel && !OB->Sdoxlo )
							{
								int Dang = ( OB->RealDir + 128 ) & 255;
								int SIN = TSin[Dang];
								int COS = TCos[Dang];
								NewAnimation* VES = NM->Veslo;
								int cspL = OB->LeftVeslo & 127;
								int cspR = OB->RightVeslo & 127;
								int LDir = ( int( OB->RealDir ) + 128 ) & 255;
								int RDir = ( int( -OB->RealDir ) + 256 ) & 255;

								NewFrame* VNFL = &VES->Frames[cspL];
								NewFrame* VNFR = &VES->Frames[cspR];
								int ROTS = VES->Rotations;

								if ( VES )
								{
									for ( int i = 0; i < NM->NVesel; i++ )
									{
										int vx = NM->VesloX[i];
										int vy = NM->VesloY[i];
										int dx = ( ( vx*COS - vy*SIN ) >> 8 );
										int dy = ( ( vx*SIN + vy*COS ) >> 9 );
										int dx1 = ( ( vx*COS + vy*SIN ) >> 8 );
										int dy1 = ( ( vx*SIN - vy*COS ) >> 9 );
										if ( LDir >= 0 )
										{
											AddPoint( xx + dx, yy + dy,
												( xx + dx + VNFL->dx ), ( zz + dy + VNFL->dy ),
												nullptr, VNFL->FileID,
												( ( LDir*ROTS ) >> 8 ) + VNFL->SpriteID*ROTS, 0, 0 );
										}

										if ( RDir >= 0 )
										{
											AddPoint( xx + dx1, yy + dy1,
												( xx + dx1 - VNFR->dx ), ( zz + dy1 + VNFL->dy ),
												nullptr, VNFL->FileID,
												( ( RDir*ROTS ) >> 8 ) + VNFR->SpriteID*ROTS + 4096, 0, 0 );
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if ( OB->NewBuilding && !OB->Hidden )
				{
					xx = ( OB->RealX >> 4 ) - x0;
					yy = ( mul3( OB->RealY ) >> 6 ) - y0;
					int zz = yy;

					if ( Mode3D )
					{
						zz -= OB->RZ;
					}

					NewMonster* NM = OB->newMons;
					if ( NM )
					{
						int xx0 = xx + NM->PicDx;
						int yy0 = zz + NM->PicDy;
						int xx1 = xx0 + NM->PicLx - 1;
						int yy1 = yy0 + NM->PicLy - 1;
						int CSP;
						if ( NM->Reflection )
						{
							NewFrame* NF = NM->Reflection->Frames;
							AddOptPoint( ZBF_LO, 0, 0, xx0, yy0, nullptr, NF->FileID, NF->SpriteID, 256 + 512 );
						}

						if ( NM->FLAGS && OB->LoLayer == &NM->StandLo )
						{
							int xre = ( OB->RealX >> 4 ) - x0;
							int yre = ( OB->RealY >> 5 ) - y0;
							int Yre = yre - OB->RZ;
							int NF = NM->FLAGS->N;
							short* PT = NM->FLAGS->Points;
							for ( int i = 0; i < NF; i++ )
							{
								int i3 = i << 2;
								AddPoint( xre, yre, xx0 + PT[i3 + 1] - 48, yy0 + PT[i3 + 2] - 28, OB, 2, ( tmtmt + i * 19 ) % 52, 0, 0 );
							}
						}

						if ( NM->Port )
						{
							int px = ( OB->WallX * 16 ) - ( mapx * 32 );
							int py = ( OB->WallY * 8 ) - ( mapy * 16 ) - GetHeight( PortBuiX * 16, PortBuiY * 16 );
							int LL = NM->BuiDist * 8;

							//Check if a ship is in production and show it
							if ( !ShowProducedShip( OB, px, py ) )
							{
								//No ship, show port buoy
								NewAnimation* BUANM = NM->BuiAnm;
								if ( BUANM )
								{
									NewFrame* NF1 = &BUANM->Frames[div( tmtmt, BUANM->NFrames ).rem];

									//Fix buoy color before drawing
									CurDrawNation = OB->Nat->NNUM;

									AddOptPoint( ZBF_LO, px, py, px + NF1->dx, py + NF1->dy,
										nullptr, NF1->FileID, NF1->SpriteID, AV_TRANSPARENT );
								}
							}
						}

						if ( xx1 > 0 && xx0 <= Lx1 && yy1 > 0 && yy0 < Ly1 )
						{
							NewAnimation* ANM = OB->HiLayer;
							int ANM_DX = 0;
							int ANM_DY = 0;
							if ( ANM && ANM->Enabled )
							{
								if ( !OB->NewState )
								{
									CSP = 0;
								}
								else
								{
									CSP = OB->NewCurSprite;
								}

								int xs = ( OB->RealX >> 4 ) + ANM->StartDx - x0;
								int yy = ( mul3( OB->RealY ) >> 6 ) - y0;
								int ys = yy + ANM->StartDy;
								ANM_DX = ANM->StartDx;
								ANM_DY = ANM->StartDy;

								if ( Mode3D )
								{
									ys -= OB->RZ;
								}

								for ( int i = 0; i < ANM->Parts; i++ )
								{
									NewFrame* NF = &ANM->Frames[i + CSP];
									AddHiPoint( xs + NF->dx, ys + NF->dy, OB, NF->FileID, NF->SpriteID, 0, 0 );
									xs += ANM->PartSize;
								}
							}

							ANM = OB->NewAnm;
							CSP = OB->NewCurSprite;

							if ( ANM && ANM->Enabled )
							{
								if ( CSP >= ANM->NFrames )
								{
									OB->NewCurSprite = 0;
									CSP = 0;
								}

								int xre = ( OB->RealX >> 4 ) - x0;
								int yre = ( mul3( OB->RealY ) >> 6 ) - y0;
								int xs = xre + ANM->StartDx;
								ANM_DX = ANM->StartDx;
								ANM_DY = ANM->StartDy;
								int ys = yre + ANM->StartDy;

								if ( Mode3D )
								{
									ys -= OB->RZ;
								}

								for ( int i = 0; i < ANM->Parts; i++ )
								{
									NewFrame* NF = &ANM->Frames[i + CSP];
									AddPoint( xre + NF->dx, yre + NM->AnmUpShift + NF->dy, xs + NF->dx, ys + NF->dy, OB, NF->FileID, NF->SpriteID, 0, 0 );
									xs += ANM->PartSize;
								}

								if ( NOPAUSE )
								{
									OB->NewCurSprite += ANM->Parts;;
								}
							}

							ANM = OB->LoLayer;
							if ( ANM && ANM->Enabled )
							{
								ANM_DX = ANM->StartDx;
								ANM_DY = ANM->StartDy;
								if ( ANM->LineInfo )
								{
									short* LINF = ANM->LineInfo;
									int nf = ANM->NFrames;
									int pos = 0;
									for ( int i = 0; i < ANM->NFrames; i++ )
									{
										int xs = ( OB->RealX >> 4 ) + ANM->StartDx - x0;
										int yy = ( mul3( OB->RealY ) >> 6 ) - y0;
										int ys = yy + ANM->StartDy;
										int ysz = ys;
										if ( Mode3D )
										{
											ys -= OB->RZ;
										}
										int x1 = LINF[pos];
										int y1 = LINF[pos + 1];
										int x2 = LINF[pos + 2];
										int y2 = LINF[pos + 3];
										NewFrame* NF = &ANM->Frames[i];

										if ( x1 == -10000 )
										{
											AddLoPoint( xs + NF->dx, ys + NF->dy, OB, NF->FileID, NF->SpriteID, 0, 0 );
										}
										else
										{
											if ( x1 == 10000 )
											{
												AddHiPoint( xs + NF->dx, ys + NF->dy, OB, NF->FileID, NF->SpriteID, 0, 0 );
											}
											else
											{
												AddLine( xs + x1, ysz + y1, xs + x2, ysz + y2, xs + NF->dx, ys + NF->dy, OB, NF->FileID, NF->SpriteID, 0, 0 );
											}
										}
										pos += 4;
									}
								}
								else
								{
									if ( !OB->NewState )
									{
										CSP = 0;
									}
									else
									{
										CSP = OB->NewCurSprite;
									}

									int xs = ( OB->RealX >> 4 ) + ANM->StartDx - x0;
									int yy = ( mul3( OB->RealY ) >> 6 ) - y0;
									int ys = yy + ANM->StartDy;
									int ysz = ys;
									if ( Mode3D )
									{
										ys -= OB->RZ;
									}
									int np = ANM->Parts - 1;
									for ( int i = 0; i < ANM->Parts; i++ )
									{
										NewFrame* NF = &ANM->Frames[i + CSP];
										AddLoPoint( xs + NF->dx, ys + NF->dy, OB, NF->FileID, NF->SpriteID, 0, 0 );
										xs += ANM->PartSize;
									}
								}
							}

							if ( !OB->Sdoxlo )
							{
								ShowFires( OB, xx0, yy0 );
							}
						}
					}
				}
			}
		}
	}

	ShowTrianglesOwerUnits();

	PreShowSprites();

	time2 = GetTickCount() - time1;
}

void RunLeftVeslo( OneObject* OB, bool State )
{
	if ( State&&OB->LeftVeslo >= 128 )OB->LeftVeslo -= 128;
	if ( ( !State ) && OB->LeftVeslo < 128 )OB->LeftVeslo += 128;
};
void RunRightVeslo( OneObject* OB, bool State )
{
	if ( State&&OB->RightVeslo >= 128 )OB->RightVeslo -= 128;
	if ( ( !State ) && OB->RightVeslo < 128 )OB->RightVeslo += 128;
};
#define MinRot 16
void OneObject::ProcessNewMotion()
{
	if ( NewMonst )
	{
		NewMonster* NWM = newMons;
		int dst;
		int FdestX = 0;
		int FdestY = 0;
		bool Fdest = false;
		byte StepDir = 1;//0-back,1-stay,2-forward
		if ( DestX > 0 )
		{
			dst = Norma( RealX - DestX, RealY - DestY );
			if ( dst <= NWM->MotionDist )DestX = -100;
			else
			{
				FdestX = div( ( DestX - RealX ) << 6, dst ).quot;
				FdestY = div( ( DestY - RealY ) << 6, dst ).quot;
				Fdest = true;
			};
		};
		if ( Collision )
		{
			int Ux = TSin[RealDir];
			int Uy = TCos[RealDir];
			int Csign = ForceX*Ux + ForceY*Uy;
			if ( Csign > 0 )//Collision pushes forward
				StepDir = 2;
			else StepDir = 0;
		}
		else
		{
			FdestX += ForceX;
			FdestY += ForceY;
			if ( FdestX || FdestY )
			{
				char ndrr = char( GetDir( FdestX, FdestY ) );
				char dir1 = char( ndrr - RealDir );
				if ( abs( dir1 ) > MinRot )
				{
					if ( dir1 > 0 )RealDir += MinRot;
					else RealDir -= MinRot;
				}
				else RealDir = ndrr;
				StepDir = 2;
			};
		};
		//has previous animation finished?
		if ( NewAnm )
		{
			if ( NewAnm->NFrames - FrmDec > NewCurSprite )
			{
				//still not finished
				if ( ( MotionDir == 2 && !StepDir ) ||
					( StepDir == 2 && !MotionDir ) )MotionDir = 1;
				if ( MotionDir == 0 )
				{
					RealX -= NWM->OneStepDX[RealDir];
					RealY -= NWM->OneStepDY[RealDir];
				}
				else
				{
					if ( MotionDir == 2 )
					{
						RealX += NWM->OneStepDX[RealDir];
						RealY += NWM->OneStepDY[RealDir];
					};
				};
				NewCurSprite += FrmDec;
			}
			else
			{
				//animation finished
				MotionDir = StepDir;
				if ( MotionDir != 1 )
				{
					//in motion
					LeftLeg = !LeftLeg;
					if ( LeftLeg )NewAnm = &NWM->MotionL;
					else NewAnm = &NWM->MotionR;
					if ( MotionDir == 0 )
					{
						RealX -= NWM->OneStepDX[RealDir];
						RealY -= NWM->OneStepDY[RealDir];
					}
					else
					{
						RealX += NWM->OneStepDX[RealDir];
						RealY += NWM->OneStepDY[RealDir];
					};
					NewCurSprite = 0;
				}
				else
				{
					NewAnm = &NWM->Stand;
					NewCurSprite = 0;
				};
			};
		};
		x = RealX >> 9;
		y = RealY >> 9;
		if ( StandTime < 8 )RZ = GetUnitHeight( RealX >> 4, RealY >> 4 );
	};
};
//---------------------NEW MOTION ALGORITM----------------------//
// Flags & Variables in OneObject
//  
//   bool CanMove:1
//   bool ImpMotion:1;
//   bool NeedForceCalc:1;
//   byte RotCntr;
//   int BestNX;
//	 int BestNY;
//   int ForceX;
//   int ForceY;
void RotateMon( OneObject* OB, char angle )
{
	//OB->NothingTime=0;
	OB->RealDir = OB->GraphDir;
	OB->RealDir += angle;
	OB->GraphDir += angle;
	OB->UnBlockUnit();
	if ( OB->AlwaysLock )OB->WeakBlockUnit();
};
void SetMonstersInCells()
{
	ZMem( MCount, VAL_MAXCX*VAL_MAXCX );
	ZMem( TmpMC, VAL_MAXCX*VAL_MAXCX );
	memset( BLDList, 0xFF, VAL_MAXCX*VAL_MAXCX * 2 );
	CleanNMSL();
	OneObject* OB;
	int ofst, ofst1, k;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OB = Group[i];
		if ( OB && !OB->Sdoxlo )
		{
			if ( OB->NewMonst )
			{
				ofst = ( OB->RealX >> 11 ) + ( ( OB->RealY >> 11 ) << VAL_SHFCX ) + VAL_MAXCX + 1;
				ofst1 = ofst << SHFCELL;
				if ( ofst < VAL_MAXCX*VAL_MAXCX )
				{
					k = MCount[ofst];
					if ( k < MAXINCELL )
					{
						MCount[ofst]++;
						TmpMC[ofst]++;
						SetNMSL( ofst1 + k, OB->Index );
					};
				};
			}
			else
			{
				if ( OB->NewBuilding )
				{
					ofst = ( OB->RealX >> 11 ) + ( ( OB->RealY >> 11 ) << VAL_SHFCX ) + VAL_MAXCX + 1;
					BLDList[ofst] = OB->Index;
				};
			};
		};
	};
};
//checking position for monster MID in  (x,y)
bool CheckPosition( int x, int y, int R, word MID )
{
	int cx = ( x >> 11 );
	int cy = ( y >> 11 );
	int ofs0 = (cx) +( ( cy ) << VAL_SHFCX );
	int ofs1 = ofs0 << SHFCELL;
	for ( int dy = 0; dy < 3; dy++ )
	{
		for ( int dx = 0; dx < 3; dx++ )
		{
			int NMon = MCount[ofs0];
			if ( ofs0 < MAXCIOFS )
			{
				for ( int z = 0; z < NMon; z++ )
				{
					word MD = GetNMSL( z + ofs1 );
					if ( MD != MID )
					{
						OneObject* OB = Group[MD];
						//if(OB&&Norma(OB->BestNX-x,OB->BestNY-y)<R+OB->Radius2)return false;
						if ( OB && ( !OB->Sdoxlo ) && Norma( OB->RealX - x, OB->RealY - y ) < R + OB->Radius2 )return false;
					};
				};
			};
			ofs0++;
			ofs1 += MAXINCELL;
		};
		ofs0 += VAL_MAXCX - 3;
		ofs1 += ( VAL_MAXCX - 3 ) << SHFCELL;
	};
	return true;
};
bool CheckBar( int x, int y, int Lx, int Ly );
bool CheckTerra( int x, int y, int Lx, int Media )
{
	int xs = ( x - ( ( Lx ) << 7 ) ) >> 8;
	int ys = ( y - ( ( Lx ) << 7 ) ) >> 8;
	if ( MFIELDS[Media].CheckBar( xs, ys, Lx, Lx ) )return false;
	else return true;
};
int FORCEX;
int FORCEY;

void PushMonsters()
{
	FORCEX = 0;
	FORCEY = 0;
	return;
}

void TryToStand( OneObject* OB, bool rest )
{
	rando();
	OB->StandTime++;
	OB->InMotion = false;
	OB->NothingTime++;
	OB->RealDir = OB->GraphDir;
	OB->CheckState();
	NewMonster* NMN = OB->newMons;
	if ( OB->NewState > 4 )OB->NewState = 0;
	if ( OB->LocalNewState > 4 )OB->LocalNewState = 0;
	OB->UnBlockUnit();
	if ( OB->AlwaysLock )OB->WeakBlockUnit();
	if ( OB->NewState != OB->LocalNewState )
	{
		if ( OB->LocalNewState != 0 )
		{
			int n1 = OB->LocalNewState;
			int n2 = OB->NewState;
			if ( n2&&NMN->TransMask[n1 - 1] & ( 1 << ( n2 - 1 ) ) )
			{
				OB->LocalNewState = n2;
				return;
			}
			else
			{
				if ( n1 == 1 && n2 == 2 )
				{
					if ( NMN->Trans01.Enabled )
					{
						OB->NewAnm = &NMN->Trans01;
						OB->NewCurSprite = 0;
						OB->LocalNewState = 2;
						OB->InMotion = false;
						OB->BackMotion = false;
						return;
					};
				}
				else
					if ( n1 == 2 && n2 == 1 )
					{
						if ( NMN->Trans10.Enabled )
						{
							OB->NewAnm = &NMN->Trans10;
							OB->NewCurSprite = 0;
							OB->LocalNewState = 1;
							OB->InMotion = false;
							OB->BackMotion = false;
							return;
						};
					}
					else
						if ( n2 == 4 )
						{
							if ( n1&&NMN->TransXMask&( 1 << ( n1 - 1 ) ) )
							{
								OB->NewAnm = &NMN->TransX3;
								OB->NewCurSprite = 0;
								OB->LocalNewState = n2;
								OB->InMotion = false;
								OB->BackMotion = false;
								return;
							};
						}
						else
							if ( n1 == 4 )
							{
								if ( n2&&NMN->TransXMask&( 1 << ( n2 - 1 ) ) )
								{
									OB->NewAnm = &NMN->Trans3X;
									OB->NewCurSprite = 0;
									OB->LocalNewState = n2;
									OB->InMotion = false;
									OB->BackMotion = false;
									return;
								};
							};
			};
			//try to transform in state 0
			NewAnimation* UA = &NMN->UAttack[OB->LocalNewState - 1];
			if ( UA->Enabled )
			{
				OB->NewAnm = UA;
				OB->NewCurSprite = 0;
				OB->LocalNewState = 0;
				OB->InMotion = false;
				OB->BackMotion = false;
				return;
			}
			else
			{
				OB->NewAnm = &NMN->Stand;
				OB->NewCurSprite = 0;
				OB->LocalNewState = 0;
				OB->InMotion = false;
				OB->BackMotion = false;
				return;
			};
		}
		else
		{
			//now we can transform unit to state NewState
			NewAnimation* PA = &NMN->PAttack[OB->NewState - 1];
			if ( PA->Enabled )
			{
				OB->NewAnm = PA;
				OB->NewCurSprite = 0;
				OB->LocalNewState = OB->NewState;
				OB->InMotion = false;
				OB->BackMotion = false;
				return;
			}
			else
			{
				NewAnimation* AT = &NMN->PStand[OB->NewState - 1];
				if ( AT->Enabled )
				{
					OB->NewAnm = AT;
					OB->NewCurSprite = 0;
					OB->LocalNewState = OB->NewState;
					OB->InMotion = false;
					OB->BackMotion = false;
					return;
				}
				else
				{
					OB->NewAnm = &NMN->Stand;
					OB->NewCurSprite = 0;
					OB->LocalNewState = OB->NewState;
					OB->InMotion = false;
					OB->BackMotion = false;
					return;
				};
			};
		};
	};
	if ( OB->NewState )
	{
		NewAnimation* AT = &NMN->PStand[OB->NewState - 1];
		if ( AT->Enabled )
		{
			OB->NewAnm = AT;
			OB->NewCurSprite = 0;
			OB->InMotion = false;
			OB->BackMotion = false;
			return;
		}
		else
		{
			OB->NewAnm = &NMN->Stand;
			OB->NewCurSprite = 0;
			OB->InMotion = false;
			OB->BackMotion = false;
			return;
		};
	}
	else
	{
		if ( rest && rando() < 512 )
		{
			if ( rando() < 16384 && NMN->Rest.Enabled )
			{
				OB->NewAnm = &NMN->Rest;
			}
			else
			{
				if ( NMN->Rest1.Enabled )
					OB->NewAnm = &NMN->Rest1;
				else
					OB->NewAnm = &NMN->Stand;
			}
		}
		else
		{
			OB->NewAnm = &NMN->Stand;
		}

		OB->NewCurSprite = 0;
		OB->InMotion = false;
		OB->BackMotion = false;
		return;
	};
}

void OneObject::CheckState()
{
	if ( RType != 0xFF )
	{
		switch ( RType )
		{
		case 0://wood
			NewState = 2;
			GroundState = 2;
			break;
		case 1:
		case 2:
			NewState = 3;
			GroundState = 3;
			break;
		case 3:
			NewState = 4;
			GroundState = 4;
		};
	};
}

int GetPrecHeight( int x, int y );

//attempt to move back or forward NewDir
//if(Dirc)attempt to move back
//returns:
//-1 : land is locked there
//1  : motion succesfull
//0  : imposible to move because of other monsters

int TryToMove( OneObject* OB, byte NewDir, bool Dirc )
{
	rando();
	if ( OB->LocalNewState != OB->NewState )
	{
		TryToStand( OB, 0 );
		return 0;
	}

	if ( Dirc && OB->BackDelay )
	{
		TryToStand( OB, false );
		return 1;
	}

	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	byte RDIR = NewDir;
	int retval = 0;
	if ( Dirc )
	{
		RDIR += 128;
		OB->BackDelay = 20;
	}

	byte Media = OB->LockType;

	OB->CheckState();

	int nf = NMN->MotionL.NFrames >> SpeedSh;
	int odx = NMN->OneStepDX[RDIR] << SpeedSh;
	int ody = NMN->OneStepDY[RDIR] << SpeedSh;
	int bdx = nf*odx;
	int bdy = nf*ody;

	if ( OB->NewState )
	{
		bdx = ( bdx*ADC->Rate[OB->NewState - 1] ) >> 4;
		bdy = ( bdy*ADC->Rate[OB->NewState - 1] ) >> 4;
		OB->Speed = ADC->Rate[OB->NewState - 1];
	}
	else
	{
		bdx = ( bdx*ADC->Rate[0] ) >> 4;
		bdy = ( bdy*ADC->Rate[0] ) >> 4;
		OB->Speed = ADC->Rate[0];
	}

	bool Unlim = OB->UnlimitedMotion;

	if ( !( Unlim || CheckTerra( OB->RealX + bdx, OB->RealY + bdy, OB->Lx, Media ) ) )
	{
		return -1;
	}

	if ( Unlim || CheckPosition( OB->RealX + bdx, OB->RealY + bdy, OB->Radius2, OB->Index ) )
	{
		retval = 1;
	}
	else
	{
		retval = 0;
	}

	NewAnimation* GO = nullptr;

	if ( OB->MaxAIndex )
	{
		if ( Dirc )
		{
			if ( OB->LeftLeg )
			{
				GO = &NMN->PMotionLB[OB->CurAIndex];
			}
			else
			{
				GO = &NMN->PMotionRB[OB->CurAIndex];
			};
			if ( OB->CurAIndex )OB->CurAIndex--;
			else
			{
				OB->CurAIndex = OB->MaxAIndex;
				OB->LeftLeg = !OB->LeftLeg;
			};
		}
		else
		{
			if ( OB->LeftLeg )
			{
				GO = &NMN->PMotionL[OB->CurAIndex];
			}
			else
			{
				GO = &NMN->PMotionR[OB->CurAIndex];
			};
			if ( OB->CurAIndex < OB->MaxAIndex )OB->CurAIndex++;
			else
			{
				OB->CurAIndex = 0;
				OB->LeftLeg = !OB->LeftLeg;
			};
		};
	}

	if ( OB->NewState && !GO )
	{
		if ( Dirc )
		{
			//Back motion
			GO = &NMN->PMotionLB[OB->NewState - 1];
			if ( GO->Enabled )
			{
				if ( !OB->LeftLeg )GO = &NMN->PMotionRB[OB->NewState - 1];
			}
			else GO = nullptr;
		}
		else
		{
			GO = &NMN->PMotionL[OB->NewState - 1];
			if ( GO->Enabled )
			{
				if ( !OB->LeftLeg )GO = &NMN->PMotionR[OB->NewState - 1];
			}
			else
			{
				GO = nullptr;
			}
		}
	}

	if ( OB->LocalNewState && !GO )
	{
		NewAnimation* NANM = &NMN->UAttack[OB->LocalNewState - 1];
		if ( NANM->Enabled )
		{
			OB->NewAnm = NANM;
			OB->NewCurSprite = 0;
			OB->InMotion = false;
			OB->LocalNewState = 0;
			return false;
		}
	}

	if ( !GO )
	{
		if ( Dirc )
		{
			if ( OB->LeftLeg )
				GO = &NMN->MotionLB;
			else
				GO = &NMN->MotionRB;
		}
		else
		{
			if ( OB->LeftLeg )
				GO = &NMN->MotionL;
			else
				GO = &NMN->MotionR;
		}

		if ( !GO->Enabled )
		{
			GO = &NMN->Stand;
		}
	}

	OB->Speed = 16;
	OB->UnBlockUnit();
	OB->BestNX = OB->RealX + bdx;
	OB->BestNY = OB->RealY + bdy;
	int dx1 = odx;
	int dy1 = ody;

	OB->RealDir = NewDir;
	OB->GraphDir = NewDir;
	OB->NewAnm = GO;
	if ( !OB->MaxAIndex )OB->LeftLeg = !OB->LeftLeg;
	OB->RealVx = dx1 >> SpeedSh;
	OB->RealVy = dy1 >> SpeedSh;
	//assert(abs(dx1)<16*256);
	OB->RealX += dx1;//((NMN->OneStepDX[RDIR]*OB->Speed)>>4)<<SpeedSh;
	OB->RealY += dy1;//((NMN->OneStepDY[RDIR]*OB->Speed)>>4)<<SpeedSh;
	OB->InMotion = true;
	OB->NewCurSprite = FrmDec - 1;;
	OB->BackMotion = Dirc;
	OB->NothingTime = 0;
	int N = OB->NewAnm->NFrames;
	OB->StandTime = 0;
	return retval;
	//}else return 0;
};
int NewTryToMove( OneObject* OB, byte NewDir, bool Dirc, int dx, int dy )
{
	rando();
	if ( Dirc&&OB->BackDelay )
	{
		TryToStand( OB, false );
		return 1;
	};
	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	byte RDIR = NewDir;
	int retval = 0;
	if ( Dirc )
	{
		RDIR += 128;
		OB->BackDelay = 20;
	};
	byte Media = OB->LockType;
	OB->CheckState();
	int nf = NMN->MotionL.NFrames >> SpeedSh;
	int bdx = nf*dx;
	int bdy = nf*dy;
	NewAnimation* GO = nullptr;
	if ( OB->MaxAIndex )
	{
		if ( Dirc )
		{
			if ( OB->LeftLeg )
			{
				GO = &NMN->PMotionLB[OB->CurAIndex];
			}
			else
			{
				GO = &NMN->PMotionRB[OB->CurAIndex];
			};
			if ( OB->CurAIndex )OB->CurAIndex--;
			else
			{
				OB->CurAIndex = OB->MaxAIndex;
				OB->LeftLeg = !OB->LeftLeg;
			};
		}
		else
		{
			if ( OB->LeftLeg )
			{
				GO = &NMN->PMotionL[OB->CurAIndex];
			}
			else
			{
				GO = &NMN->PMotionR[OB->CurAIndex];
			};
			if ( OB->CurAIndex < OB->MaxAIndex )OB->CurAIndex++;
			else
			{
				OB->CurAIndex = 0;
				OB->LeftLeg = !OB->LeftLeg;
			};
		};
	};
	if ( OB->NewState && !GO )
	{
		if ( Dirc )
		{
			//Back motion
			GO = &NMN->PMotionLB[OB->NewState - 1];
			if ( GO->Enabled )
			{
				if ( !OB->LeftLeg )GO = &NMN->PMotionRB[OB->NewState - 1];
			}
			else GO = nullptr;
		}
		else
		{
			GO = &NMN->PMotionL[OB->NewState - 1];
			if ( GO->Enabled )
			{
				if ( !OB->LeftLeg )GO = &NMN->PMotionR[OB->NewState - 1];
			}
			else GO = nullptr;
		};
	};
	if ( OB->LocalNewState && !GO )
	{
		NewAnimation* NANM = &NMN->UAttack[OB->LocalNewState - 1];
		if ( NANM->Enabled )
		{
			OB->NewAnm = NANM;
			OB->NewCurSprite = 0;
			OB->InMotion = false;
			OB->LocalNewState = 0;
			return false;
		};
	};
	if ( !GO )
	{
		if ( Dirc )
		{
			if ( OB->LeftLeg )GO = &NMN->MotionLB;
			else GO = &NMN->MotionRB;
		}
		else
		{
			if ( OB->LeftLeg )GO = &NMN->MotionL;
			else GO = &NMN->MotionR;
		};
		if ( !GO->Enabled )GO = &NMN->Stand;
	};
	OB->Speed = 16;
	OB->UnBlockUnit();
	OB->RealVx = dx;
	OB->RealVy = dy;
	//assert(abs(dx)<16*256);
	OB->NewAnm = GO;
	if ( !OB->MaxAIndex )OB->LeftLeg = !OB->LeftLeg;
	OB->RealX += dx << SpeedSh;
	OB->RealY += dy << SpeedSh;
	OB->InMotion = true;
	OB->NewCurSprite = FrmDec - 1;
	OB->BackMotion = Dirc;
	OB->NothingTime = 0;
	//int N=OB->NewAnm->NFrames;
	OB->StandTime = 0;
	return retval;
	//}else return 0;
};
bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs,
	int xd, int yd, int zd,
	OneObject* OB );
void CreateFullPath( int x1, int y1, OneObject* OB );
void AddDestn( int x, int y );
extern int tmtmt;
//extern int RESRC[8][8];
void CalculateMotionX0()
{
	if ( !( tmtmt & 31 ) )
	{
		for ( int i = 0; i < MAXOBJECT; i++ )
		{
			OneObject* OB = Group[i];
			if ( OB&&OB->NZalp < OB->newMons->MaxZalp )OB->NZalp++;
		};
	};
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			NewMonster* NM = OB->newMons;
			if ( OB->delay )
			{
				if ( NM->SlowRecharge )
				{
					if ( !OB->LocalOrder )
					{
						int cf = OB->NewCurSprite >> SpeedSh;
						int nf = ( OB->NewAnm->NFrames - 1 ) >> SpeedSh;
						if ( nf == cf )
						{
							if ( OB->NewState != 4 )
							{
								OB->NewState = 4;
								TryToStand( OB, 0 );
							}
							else
							{
								NewAnimation* ANM = &NM->Attack[3];
								if ( OB->NewAnm != ANM )
								{
									OB->NewAnm = ANM;
									OB->NewCurSprite = 0;
									OB->InMotion = 0;
									OB->BackMotion = 0;
								};
							};
						};
					};
				}
				else
				{
					OB->delay--;
				};
			};
			if ( OB->NewBuilding && !OB->Sdoxlo )
			{
				//if(OB->Weap)OB->SearchVictim();
				if ( OB->DstX&&OB->ImSelected&&OB->NNUM == MyNation )AddDestn( OB->DstX >> 4, OB->DstY >> 4 );
				if ( OB->LocalOrder )OB->LocalOrder->DoLink( OB );
				int HLife = OB->MaxLife >> 1;
				if ( OB->Life < HLife )
				{

					bool DoFog = false;
					HLife >>= 1;
					int Prob = NM->Fogging.WProb;
					if ( OB->Life < HLife )
					{
						if ( rando() < Prob )DoFog = true;
					}
					else
					{
						Prob >>= 1;
						if ( rando() < Prob )DoFog = true;
					};
					if ( DoFog&&NM->Fogging.NWeap&&OB->Stage == OB->NStages )
					{
						Prob = ( rando()*NM->Fogging.NWeap ) >> 15;
						Weapon* WP = WPLIST[NM->Fogging.Weap[Prob]];
						Prob = ( rando()*NM->NDamPt ) >> 15;
						int xx, yy;
						OB->GetCornerXY( &xx, &yy );
						if ( NM->NDamPt )
						{
							xx += NM->DamPtX[Prob];
							yy += NM->DamPtY[Prob];
						};
						xx <<= 4;
						yy <<= 4;
						Create3DAnmObject( WP, xx, yy, GetHeight( xx, yy ) + 4, xx, yy, -1000, nullptr );
					};
				};
			}
			else
				if ( OB->NewMonst/*&&OB->NewCurSprite==OB->NewAnm->NFrames-1*/ )
				{
					OB->ForceX = OB->NextForceX;
					OB->ForceY = OB->NextForceY;
					OB->NextForceX = 0;
					OB->NextForceY = 0;

				};
			if ( OB->Sdoxlo && !OB->Hidden )
			{
				//death processing
				NewMonster* NMN = OB->newMons;
				if ( NMN->Death.Enabled )
				{
					if ( OB->NewAnm == &NMN->Death )
					{
						OB->NewCurSprite += FrmDec;
						if ( OB->NewCurSprite >= NMN->Death.NFrames - FrmDec )
						{
							if ( NMN->DeathLie1.Enabled )
							{
								OB->NewAnm = &NMN->DeathLie1;
								OB->NewCurSprite = 0;
							}
							else
							{
								OB->DeletePath();
								OB->ClearOrders();
								DelObject( OB );
								Group[OB->Index] = nullptr;
								OB = nullptr;//died.	
							};
						};
					}
					else
					{
						OB->Sdoxlo++;
						if ( OB->Sdoxlo > 200 )
						{
							if ( NMN->DeathLie2.Enabled )
							{
								OB->NewAnm = &NMN->DeathLie2;
								OB->NewCurSprite = 0;
							}
							else
							{
								OB->DeletePath();
								OB->ClearOrders();
								DelObject( OB );
								Group[OB->Index] = nullptr;
								OB = nullptr;//died.	
							};
						};
						if ( OB&&OB->Sdoxlo > 400 )
						{
							if ( NMN->DeathLie2.Enabled )
							{
								OB->NewAnm = &NMN->DeathLie3;
								OB->NewCurSprite = 0;
							}
							else
							{
								OB->DeletePath();
								OB->ClearOrders();
								DelObject( OB );
								Group[OB->Index] = nullptr;
								OB = nullptr;//died.	
							};
						};
						if ( OB&&OB->Sdoxlo > 600 )
						{
							OB->DeletePath();
							OB->ClearOrders();
							DelObject( OB );
							Group[OB->Index] = nullptr;
							OB = nullptr;//died.	
						};
					};
					if ( OB )OB->Sdoxlo++;
				};
			};
		};
	};
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NewMonst && !OB->Sdoxlo )
		{
			OB->SearchVictim();
			NewMonster* NMN = OB->newMons;
			if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
			{
				int FX = 0;
				int FY = 0;
				OB->BestNX = OB->RealX;
				OB->BestNY = OB->RealY;
				if ( OB->DestX > 0 )
				{
					//int uux=(OB->DestX-(OB->Lx<<7))>>8;
					//int uuy=(OB->DestY-(OB->Ly<<7))>>8;
					int dst = Norma( OB->RealX - OB->DestX, OB->RealY - OB->DestY );
					if ( dst > 256 )
					{
						FX = div( ( OB->DestX - OB->RealX ) << 7, dst ).quot;
						FY = div( ( OB->DestY - OB->RealY ) << 7, dst ).quot;
					}
					else OB->DestX = -100;
				};
				bool ExtForce = OB->ForceX || OB->ForceY;
				FX += OB->ForceX;
				FY += OB->ForceY;
				bool Try1 = false;
			uuuu:
				if ( FX || FY )
				{
					char BestDir = char( GetDir( FX, FY ) );
					int bdx = NMN->MotionL.NFrames*NMN->OneStepDX[byte( BestDir )];
					int bdy = NMN->MotionL.NFrames*NMN->OneStepDY[byte( BestDir )];
					if ( !CheckTerra( OB->RealX + bdx, OB->RealY + bdy, OB->Lx, 0 ) )
					{
						if ( ( !Try1 ) && ( OB->PathX || OB->DestX < 0 ) )
						{
							int OFX = FX;
							int OFY = FY;
							int xs0 = ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 8;
							int ys0 = ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 8;
							int xs1 = ( OB->RealX + bdx - ( ( OB->Lx ) << 7 ) ) >> 8;
							int ys1 = ( OB->RealY + bdy - ( ( OB->Lx ) << 7 ) ) >> 8;
							if ( CheckBar( xs0, ys1, OB->Lx, OB->Lx ) )FY = 0;
							if ( CheckBar( xs1, ys0, OB->Lx, OB->Lx ) )FX = 0;
							if ( FX&&FY&&OB->DestX > 0 )
							{
								if ( abs( FX ) > abs( FY ) )FY = 0;
								else FX = 0;
							};
							if ( !( FX || FY ) )
							{
								OB->DeletePath();
								//if(OB->DestX>0){
								//	if(OB->PathX&&OB->NIPoints){
								//		OB->CreatePrePath(OB->PathX[OB->NIPoints-1],OB->PathY[OB->NIPoints-1]);
								//	}else{
								//		CreateFullPath((OB->DestX-(OB->Lx<<7))>>8,(OB->DestY-(OB->Lx<<7))>>8,OB);
								//	};
								//};
							}
							else
							{
								;
								Try1 = true;
								goto uuuu;
							};
						};
					};
					char ddir = OB->RealDir - BestDir;
					Try1 = false;
					if ( ExtForce )
					{
						if ( abs( ddir ) > 64 )
						{
							BestDir = BestDir + 128;
							ddir = OB->RealDir - BestDir;
							if ( ddir < 8 )
							{
								OB->RealDir = BestDir;
							}
							else
							{
								if ( ddir > 0 )OB->RealDir -= 8;
								else OB->RealDir += 8;
							};
							OB->GraphDir = OB->RealDir;
							int trmv = TryToMove( OB, OB->RealDir, true );
							if ( trmv != -1 )Try1 = true;
							if ( trmv == 0 )
							{
								PushMonsters();
								OB->NextForceX += FORCEX;
								OB->NextForceY += FORCEY;
							};
						}
						else
						{
							if ( abs( ddir ) < 8 )OB->RealDir = BestDir;
							else
							{
								if ( ddir > 0 )OB->RealDir -= 8;
								else OB->RealDir += 8;
							};
							OB->GraphDir += 8;
							int trmv = TryToMove( OB, OB->RealDir, false );
							if ( trmv == 0 )
							{
								PushMonsters();
								OB->NextForceX += FORCEX;
								OB->NextForceY += FORCEY;
							};
							if ( trmv != -1 )Try1 = true;
						};
					};
					if ( !Try1 )
					{
						if ( abs( ddir ) < MinRot )
						{
							OB->RealDir = BestDir;
							OB->GraphDir = OB->RealDir;
							int trmv = TryToMove( OB, OB->RealDir, false );
							if ( trmv == 0 )
							{
								PushMonsters();
								OB->NextForceX += FORCEX;
								OB->NextForceY += FORCEY;
							};
							if ( trmv == -1 )
							{
								if ( OB->DestX > 0 )
								{
									if ( OB->PathX&&OB->NIPoints )
									{
										OB->CreatePrePath( OB->PathX[OB->NIPoints - 1], OB->PathY[OB->NIPoints - 1] );
										TryToStand( OB, false );
									}
									else
									{
										CreateFullPath( ( OB->DestX - ( OB->Lx << 7 ) ) >> 8, ( OB->DestY - ( OB->Lx << 7 ) ) >> 8, OB );
										TryToStand( OB, false );
									};
								}
								else
								{
									TryToStand( OB, false );
								};
							};
							OB->RotCntr = 0;
						}
						else
						{
							if ( ddir > 0 )
							{
								RotateMon( OB, -MinRot );
								TryToMove( OB, OB->RealDir, false );
							}
							else
							{
								RotateMon( OB, MinRot );
								TryToMove( OB, OB->RealDir, false );
							};
						};
					};
				}
				else
				{
					TryToStand( OB, true );
				};
			}
			else
			{
				if ( OB->InMotion )
				{
					int FX = 0;
					int FY = 0;
					if ( OB->DestX > 0 )
					{
						int dst = Norma( OB->RealX - OB->DestX, OB->RealY - OB->DestY );
						if ( dst > 256 )
						{
							FX = div( ( OB->DestX - OB->RealX ) << 7, dst ).quot;
							FY = div( ( OB->DestY - OB->RealY ) << 7, dst ).quot;
						}
						else OB->DestX = -100;
					};
					FX += OB->ForceX;
					FY += OB->ForceY;
					char Bdir = char( GetDir( FX, FY ) );
					if ( OB->BackMotion )Bdir += (byte) 128;
					char ddir = OB->RealDir - Bdir;
					if ( abs( ddir ) < 8 )OB->RealDir = Bdir;
					else
					{
						if ( ddir > 0 )OB->RealDir -= 8;
						else OB->RealDir += 8;
					};
					int xx1 = OB->RealX;
					int yy1 = OB->RealY;
					OB->GraphDir = OB->RealDir;
					if ( OB->BackMotion )
					{
						xx1 -= ( NMN->OneStepDX[OB->RealDir] * OB->Speed ) >> 4;
						yy1 -= ( NMN->OneStepDY[OB->RealDir] * OB->Speed ) >> 4;
					}
					else
					{
						xx1 += ( NMN->OneStepDX[OB->RealDir] * OB->Speed ) >> 4;
						yy1 += ( NMN->OneStepDY[OB->RealDir] * OB->Speed ) >> 4;
					};
					if ( CheckTerra( xx1, yy1, OB->Lx, 0 ) )
					{
						OB->RealX = xx1;
						OB->RealY = yy1;
					};
					if ( CheckPosition( OB->RealX, OB->RealY, OB->Radius2, OB->Index ) )
						PushMonsters();
				};
				OB->NewCurSprite += FrmDec;
			};
			OB->x = ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 8;
			OB->y = ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 8;
			OB->RZ = GetHeight( ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 4, ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 4 );
			if ( OB->LocalOrder )
			{
				OB->LocalOrder->DoLink( OB );
			}
			else OB->PrioryLevel = 0;;
		};
	};
};
void UnitLight( OneObject* OB );
void CorrectLockPosition( OneObject* OB );
void MotionHandler0( OneObject* OB )
{

	if ( OB->PathDelay >= FrmDec )OB->PathDelay -= FrmDec;
	else OB->PathDelay = 0;
	if ( OB->BackDelay >= FrmDec )OB->BackDelay -= FrmDec;
	else OB->BackDelay = 0;
	NewMonster* NMN = OB->newMons;
	int MRot = NMN->MinRotator;
	int MRot1 = MRot << 1;
	if ( MRot1 > MinRot )MRot1 = MinRot;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	NMN->AdvChar = ADC;
	if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
	{
		if ( ( !OB->UnlimitedMotion ) && MFIELDS[OB->LockType].CheckBar( OB->x, OB->y, OB->Lx, OB->Lx ) )
		{
			CorrectLockPosition( OB );
		};
		int FX = 0;
		int FY = 0;
		OB->BestNX = OB->RealX;
		OB->BestNY = OB->RealY;
		if ( OB->DestX > 0 )
		{
			int dst = Norma( OB->RealX - OB->DestX, OB->RealY - OB->DestY );
			if ( dst > 256 )
			{
				FX = div( ( OB->DestX - OB->RealX ) << 7, dst ).quot;
				FY = div( ( OB->DestY - OB->RealY ) << 7, dst ).quot;
			}
			else OB->DestX = -100;
		};
		if ( OB->UnlimitedMotion )
		{
			OB->ForceX = 0;
			OB->ForceY = 0;
		};
		bool ExtForce = OB->ForceX || OB->ForceY;
		FX += OB->ForceX;
		FY += OB->ForceY;
		bool Try1 = false;
	uuuu:
		if ( FX || FY )
		{
			char BestDir = char( GetDir( FX, FY ) );
			int bdx = NMN->MotionL.NFrames*NMN->OneStepDX[byte( BestDir )];
			int bdy = NMN->MotionL.NFrames*NMN->OneStepDY[byte( BestDir )];
			if ( !OB->UnlimitedMotion )
			{
				if ( !CheckTerra( OB->RealX + bdx, OB->RealY + bdy, OB->Lx, OB->LockType ) )
				{
					if ( ( !Try1 ) && ( OB->PathX || OB->DestX < 0 ) )
					{
						int OFX = FX;
						int OFY = FY;
						int xs0 = ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 8;
						int ys0 = ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 8;
						int xs1 = ( OB->RealX + bdx - ( ( OB->Lx ) << 7 ) ) >> 8;
						int ys1 = ( OB->RealY + bdy - ( ( OB->Lx ) << 7 ) ) >> 8;
						if ( CheckBar( xs0, ys1, OB->Lx, OB->Lx ) )FY = 0;
						if ( CheckBar( xs1, ys0, OB->Lx, OB->Lx ) )FX = 0;
						if ( FX&&FY&&OB->DestX > 0 )
						{
							if ( abs( FX ) > abs( FY ) )FY = 0;
							else FX = 0;
						};
						if ( !( FX || FY ) )
						{
							OB->DeletePath();
						}
						else
						{
							;
							Try1 = true;
							goto uuuu;
						};
					};
				};
			};
			char ddir = OB->RealDir - BestDir;
			Try1 = false;
			if ( ExtForce )
			{
				if ( abs( ddir ) > 64 )
				{
					BestDir = BestDir + 128;
					ddir = OB->RealDir - BestDir;
					if ( ddir < 8 )
					{
						OB->RealDir = BestDir;
					}
					else
					{
						if ( ddir > 0 )OB->RealDir -= 8;
						else OB->RealDir += 8;
					};
					OB->GraphDir = OB->RealDir;
					int trmv = TryToMove( OB, OB->RealDir, true );
					if ( trmv != -1 )Try1 = true;
					if ( trmv == 0 )
					{
						PushMonsters();
						OB->NextForceX += FORCEX;
						OB->NextForceY += FORCEY;
					};
				}
				else
				{
					if ( abs( ddir ) < 8 )OB->RealDir = BestDir;
					else
					{
						if ( ddir > 0 )OB->RealDir -= 8;
						else OB->RealDir += 8;
					};
					OB->GraphDir += 8;
					int trmv = TryToMove( OB, OB->RealDir, false );
					if ( trmv == 0 )
					{
						PushMonsters();
						OB->NextForceX += FORCEX;
						OB->NextForceY += FORCEY;
					};
					if ( trmv != -1 )Try1 = true;
				};
			};
			if ( !Try1 )
			{
				if ( abs( ddir ) < MRot1 )
				{
					OB->RealDir = BestDir;
					OB->GraphDir = OB->RealDir;
					int trmv = TryToMove( OB, OB->RealDir, false );
					if ( trmv == 0 )
					{
						PushMonsters();
						OB->NextForceX += FORCEX;
						OB->NextForceY += FORCEY;
					};
					if ( trmv == -1 )
					{
						if ( OB->DestX > 0 )
						{
							if ( OB->PathDelay )
							{
								byte CDir = OB->RealDir;
								byte ddr = 16;
								int NN = NMN->MotionL.NFrames;
								byte olddir = OB->RealDir;
								for ( int i = 0; i < 8; i++ )
								{
									byte CDR1 = CDir - ddr;
									OB->RealDir = CDR1;
									OB->GraphDir = CDR1;
									int tmtrv = TryToMove( OB, OB->RealDir, false );
									if ( tmtrv != -1 )
									{
										goto hhh1;
									};
									CDR1 = CDir + ddr;
									OB->RealDir = CDR1;
									OB->GraphDir = CDR1;
									tmtrv = TryToMove( OB, OB->RealDir, false );
									if ( tmtrv != -1 )
									{
										goto hhh1;
									};
									ddr += 16;
								};
								OB->RealDir = olddir;
								OB->GraphDir = olddir;
							hhh1:;
							}
							else
							{
								if ( OB->PathX&&OB->NIPoints )
								{

									OB->CreatePrePath( OB->PathX[OB->NIPoints - 1], OB->PathY[OB->NIPoints - 1] );
									TryToStand( OB, false );
									if ( AllowPathDelay )OB->PathDelay = 32 + ( rando() & 7 );
								}
								else
								{

									CreateFullPath( ( OB->DestX - ( OB->Lx << 7 ) ) >> 8, ( OB->DestY - ( OB->Lx << 7 ) ) >> 8, OB );
									TryToStand( OB, false );
								};
							};
						}
						else
						{
							TryToStand( OB, false );
						};
					};
					OB->RotCntr = 0;
				}
				else
				{
					if ( ddir > 0 )
					{
						RotateMon( OB, -MRot );//OB->RealDir-=MinRot;
						//	if(OB->RotCntr>1){
				//		if(TryToMove(OB,OB->RealDir,false)!=1){
						TryToStand( OB, false );
						//		}else{
						//			OB->InMotion=true;
						//			OB->NewCurSprite=0;
						//			if(OB->LeftLeg)OB->NewAnm=&NMN->MotionL;
						//			else OB->NewAnm=&NMN->MotionR;
						//			OB->LeftLeg=!OB->LeftLeg;
						//			//OB->NewAnm=&NMN->MotionL;
						//			OB->RotCntr=0;
						//		};
					}
					else
					{
						RotateMon( OB, MRot );//OB->RealDir+=MinRot;
				//		if(TryToMove(OB,OB->RealDir,false)!=1){
						TryToStand( OB, false );
						//		}else{
						//			OB->InMotion=true;
						//			OB->NewCurSprite=0;
						//			if(OB->LeftLeg)OB->NewAnm=&NMN->MotionL;
						//			else OB->NewAnm=&NMN->MotionR;
						//			OB->LeftLeg=!OB->LeftLeg;
						//			//OB->NewAnm=&NMN->MotionR;
						//		OB->RotCntr=0;
						//		};
					};
				};
			};
		}
		else
		{
			TryToStand( OB, true );
		};
	}
	else
	{
		if ( OB->InMotion )
		{
			OB->UnBlockUnit();
			if ( OB->BackMotion )
			{
				OB->RealX -= ( NMN->OneStepDX[OB->RealDir] * OB->Speed ) >> 4;
				OB->RealY -= ( NMN->OneStepDY[OB->RealDir] * OB->Speed ) >> 4;
			}
			else
			{
				//OB->RealX+=(NMN->OneStepDX[OB->RealDir]*OB->Speed)>>4;
				//OB->RealY+=(NMN->OneStepDY[OB->RealDir]*OB->Speed)>>4;
				OB->RealX += ( NMN->OneStepDX[OB->RealDir] );
				OB->RealY += ( NMN->OneStepDY[OB->RealDir] );
			};
		};
		OB->NewCurSprite += FrmDec;
	};
	OB->x = ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 8;
	OB->y = ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 8;
	if ( OB->AlwaysLock )OB->WeakBlockUnit();
	if ( OB->StandTime < 8 )OB->RZ = GetUnitHeight( ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 4, ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 4 );
	//if(OB->LocalOrder){
	//	OB->LocalOrder->DoLink(OB);
	//}else OB->PrioryLevel=0;
	//OB->SearchVictim();
};
void SWAP( short* x, short* y )
{
	short t = *x;
	( *x ) = ( *y );
	*y = t;
};
void SWAP( word* x, word* y )
{
	word t = *x;
	( *x ) = ( *y );
	*y = t;
};
void SWAP( int* x, int* y )
{
	int t = *x;
	( *x ) = ( *y );
	*y = t;
};
void SWAP( byte*x, byte* y )
{
	byte t = *x;
	( *x ) = ( *y );
	*y = t;
};
void SWAP( char* x, char* y )
{
	char t = *x;
	( *x ) = ( *y );
	*y = t;
};
void SWAP( NewAnimation** x, NewAnimation** y )
{
	NewAnimation* t = *x;
	( *x ) = ( *y );
	*y = t;
};
extern MotionField UnitsField;
void AttackObjLink( OneObject* OBJ );
bool RemoveFoolsInCell( int cell, int x, int y, int Lx, OneObject* MyObj )
{
	cell += VAL_MAXCX + 1;
	word MyMid = MyObj->Index;
	int mx1 = x + Lx - 1;
	int my1 = y + Lx - 1;
	int NMon = MCount[cell];
	if ( !NMon )return false;
	int ofs1 = cell << SHFCELL;
	word MID;
	int cx = ( x << 1 ) + Lx;
	int cy = ( y << 1 ) + Lx;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Index != MyMid && !( OB->InMotion || OB->BlockInfo ) )
			{
				int olx = OB->Lx;
				int ox = ( ( OB->x ) << 1 ) + olx;
				int oy = ( ( OB->y ) << 1 ) + olx;
				int dx = abs( ox - cx );
				int dy = abs( oy - cy );
				if ( !( dx >= ( olx + Lx ) || dy >= ( olx + Lx ) ) )
				{
					if ( OB->NMask&MyObj->NMask )
					{
						//intersection
						if ( OB->StandTime > 20 && OB->Lx == MyObj->Lx )
						{
							if ( OB->NewAnm->NFrames - FrmDec <= OB->NewCurSprite )
							{
								SWAP( &OB->x, &MyObj->x );
								SWAP( &OB->y, &MyObj->y );
								SWAP( &OB->RealX, &MyObj->RealX );
								SWAP( &OB->RealY, &MyObj->RealY );
								SWAP( &OB->RealDir, &MyObj->RealDir );
								SWAP( &OB->GraphDir, &MyObj->GraphDir );
								//SWAP(&OB->NewAnm,&MyObj->NewAnm);
								//SWAP(&OB->NewCurSprite,&MyObj->NewCurSprite);
								//SWAP(&OB->LocalNewState,&MyObj->LocalNewState);
								//SWAP(&OB->NewState,&MyObj->NewState);
								/*
								Order1* OR1=OB->LocalOrder;
								Order1* OR2=MyObj->LocalOrder;
								Group[OB->Index]=MyObj;
								Group[MyObj->Index]=OB;
								word id=OB->Index;
								OB->Index=MyObj->Index;
								MyObj->Index=id;
								OB->LocalOrder=OR2;
								MyObj->LocalOrder=OR1;
								id=OB->Serial;
								OB->Serial=MyObj->Serial;
								MyObj->Serial=id;
								id=
								*/
								OB->MoveStage = 0;
								MyObj->MoveStage = 0;
							};
							return true;
						};
						int rx = OB->RealX >> 5;
						int ry = OB->RealY >> 5;
						UnitsField.BClrBar( OB->x, OB->y, OB->Lx );
						bool NeedRemove = true;
						int lx2 = olx << 2;
						for ( int r = 0; r < 8 && NeedRemove; r++ )
						{
							char* xi = Rarr[r].xi;
							char* yi = Rarr[r].yi;
							int N = Rarr[r].N;
							for ( int j = 0; j < N&&NeedRemove; j++ )
							{
								int rx1 = rx + xi[j];
								int ry1 = ry + yi[j];
								int sx = ( rx1 + lx2 ) >> 3;
								int sy = ( ry1 + lx2 ) >> 3;
								if ( !( UnitsField.CheckBar( sx, sy, olx, olx ) || CheckBar( sx, sy, olx, olx ) ) )
								{
									ox = sx + sx + olx;
									oy = sy + sy + olx;
									dx = abs( ox - cx );
									dy = abs( oy - cy );
									if ( dx >= ( olx + Lx ) || dy >= ( olx + Lx ) )
									{
										OB->x = sx;
										OB->y = sy;
										OB->RealX = rx1 << 5;
										OB->RealY = ry1 << 5;
										UnitsField.BSetBar( OB->x, OB->y, OB->Lx );
										NeedRemove = false;
									};
								};
							};
						};
						if ( NeedRemove )
						{
							UnitsField.BSetBar( OB->x, OB->y, OB->Lx );
						};
					}
					else
					{
						if ( MyObj->LocalOrder&&MyObj->LocalOrder->DoLink == &AttackObjLink&&
							MyObj->LocalOrder->info.MoveToObj.ObjIndex == OB->Index )return true;
						else if ( MyObj->EnemyID == 0xFFFF )MyObj->AttackObj( OB->Index, 128 + 4, 1 );
					};
				};
			};
		};
	};
	return false;
};
void RemoveFools( int x, int y, int Lx, OneObject* MyMid )
{
	int cell = ( x >> 3 ) + ( ( y >> 3 ) << VAL_SHFCX );

	if ( RemoveFoolsInCell( cell, x, y, Lx, MyMid ) )return;

	if ( RemoveFoolsInCell( cell - 1, x, y, Lx, MyMid ) )return;
	if ( RemoveFoolsInCell( cell + 1, x, y, Lx, MyMid ) )return;
	if ( RemoveFoolsInCell( cell - VAL_MAXCX, x, y, Lx, MyMid ) )return;
	if ( RemoveFoolsInCell( cell + VAL_MAXCX, x, y, Lx, MyMid ) )return;

	if ( RemoveFoolsInCell( cell - VAL_MAXCX + 1, x, y, Lx, MyMid ) )return;
	if ( RemoveFoolsInCell( cell + VAL_MAXCX + 1, x, y, Lx, MyMid ) )return;;
	if ( RemoveFoolsInCell( cell - VAL_MAXCX - 1, x, y, Lx, MyMid ) )return;
	if ( RemoveFoolsInCell( cell + VAL_MAXCX - 1, x, y, Lx, MyMid ) )return;
};
void SmartTryToMove( OneObject* OB, byte NewDir, bool Dirc )
{
	if ( !OB->LockType )
	{
		char BestDir = NewDir;
		if ( Dirc )BestDir += (byte) 128;
		NewMonster* NMN = OB->newMons;
		int mrot = NMN->MinRotator;
		bool NeedToPush = false;
		char dirr = 0;
		switch ( OB->MoveStage )
		{
		case 0:
			dirr = BestDir;
			OB->MoveStage = 1;
			break;
		case 1:
			dirr = BestDir + mrot;
			OB->MoveStage = 2;
			break;
		case 2:
			dirr = BestDir - mrot;
			OB->MoveStage = 3;
			break;
		case 3:
			dirr = BestDir + mrot + mrot;
			OB->MoveStage = 4;
			break;
		case 4:
			dirr = BestDir - mrot - mrot;
			OB->MoveStage = 5;
			break;
		case 5:
			dirr = BestDir + mrot + mrot + mrot;
			OB->MoveStage = 6;
			break;
		case 6:
			dirr = BestDir - mrot - mrot - mrot;
			OB->MoveStage = 255;
			break;
		case 255:
			NeedToPush = true;
			dirr = BestDir;
			break;
		};
		int nst = NMN->MotionL.NFrames >> SpeedSh;
		int dx0 = ( NMN->OneStepDX[byte( dirr )] * nst ) << SpeedSh;
		int dy0 = ( NMN->OneStepDY[byte( dirr )] * nst ) << SpeedSh;
		int LX = OB->Lx;
		UnitsField.BClrBar( OB->x, OB->y, LX );
		int xfin = ( OB->RealX + dx0 - ( LX << 7 ) ) >> 8;
		int yfin = ( OB->RealY + dy0 - ( LX << 7 ) ) >> 8;
		if ( UnitsField.CheckBar( xfin, yfin, LX, LX ) || CheckBar( xfin, yfin, LX, LX ) )
		{
			TryToStand( OB, false );
			UnitsField.BSetBar( OB->x, OB->y, LX );
			if ( NeedToPush )
			{
				RemoveFools( xfin, yfin, LX, OB );
			};
		}
		else
		{
			if ( Dirc )dirr += (byte) 128;
			OB->RealDir = dirr;
			OB->GraphDir = dirr;
			NewMonster* NM = OB->newMons;
			int dx = NM->OneStepDX[byte( dirr )];
			int dy = NM->OneStepDY[byte( dirr )];
			NewTryToMove( OB, dirr, Dirc, dx, dy );
			UnitsField.BSetBar( xfin, yfin, LX );
			OB->x = xfin;
			OB->y = yfin;
			OB->MoveStage = 0;
		};
	};
};
void MotionHandler1( OneObject* OB )
{

	//if(OB->delay)OB->delay--;
	if ( OB->PathDelay >= FrmDec )OB->PathDelay -= FrmDec;
	else OB->PathDelay = 0;
	if ( OB->BackDelay >= FrmDec )OB->BackDelay -= FrmDec;
	else OB->BackDelay = 0;
	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	NMN->AdvChar = ADC;
	if ( NMN->WaterActive )SpotByUnit( OB->RealX, OB->RealY, 35, OB->RealDir );
	if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
	{
		if ( OB->DestX > 0 )
		{
			int dx = OB->DestX - OB->RealX;
			int dy = OB->DestY - OB->RealY;
			int Nrm = Norma( dx, dy );
			char BestDir = char( GetDir( dx, dy ) );
			char ddir = BestDir - OB->RealDir;
			int mrot = NMN->MinRotator;
			bool NeedToPush = false;
			if ( abs( ddir ) <= mrot )
			{
				char dirr = 0;
				switch ( OB->MoveStage )
				{
				case 0:
					dirr = BestDir;
					OB->MoveStage = 1;
					break;
				case 1:
					dirr = BestDir + mrot;
					OB->MoveStage = 2;
					break;
				case 2:
					dirr = BestDir - mrot;
					OB->MoveStage = 3;
					break;
				case 3:
					dirr = BestDir + mrot + mrot;
					OB->MoveStage = 4;
					break;
				case 4:
					dirr = BestDir - mrot - mrot;
					OB->MoveStage = 5;
					break;
				case 5:
					dirr = BestDir + mrot + mrot + mrot;
					OB->MoveStage = 6;
					break;
				case 6:
					dirr = BestDir - mrot - mrot - mrot;
					OB->MoveStage = 255;
					break;
				case 255:
					NeedToPush = true;
					dirr = BestDir;
					break;
				};
				int nst = NMN->MotionL.NFrames;
				int dxx = NMN->OneStepDX[byte( dirr )];
				int dyy = NMN->OneStepDY[byte( dirr )];
				int dx0 = dxx*nst;
				int dy0 = dyy*nst;
				int LX = OB->Lx;
				UnitsField.BClrBar( OB->x, OB->y, LX );
				int xrfin = OB->RealX + dx0;
				int yrfin = OB->RealY + dy0;
				int xfin = ( xrfin - ( LX << 7 ) ) >> 8;
				int yfin = ( yrfin - ( LX << 7 ) ) >> 8;
				bool OnTheWay = CheckBar( xfin, yfin, LX, LX );
				bool unlim = !OB->UnlimitedMotion;
				if ( unlim && ( OnTheWay || UnitsField.CheckBar( xfin, yfin, LX, LX ) ) )
				{
					if ( OnTheWay&&NeedToPush )
					{
						if ( OB->PathX&&OB->NIPoints )
						{

							OB->CreatePrePath( OB->PathX[OB->NIPoints - 1], OB->PathY[OB->NIPoints - 1] );
							if ( AllowPathDelay )OB->PathDelay = 32 + ( rando() & 7 );
						}
						else
						{

							CreateFullPath( ( OB->DestX - ( OB->Lx << 7 ) ) >> 8, ( OB->DestY - ( OB->Lx << 7 ) ) >> 8, OB );
						};
						OB->MoveStage = 0;
					};

					TryToStand( OB, false );
					UnitsField.BSetBar( OB->x, OB->y, LX );
					if ( NeedToPush )
					{
						RemoveFools( xfin, yfin, LX, OB );
					};
				}
				else
				{
					if ( unlim )
					{
						word MID = CheckMotionThroughEnemyAbility( OB, xrfin, yrfin );
						if ( MID != 0xFFFF )
						{

							TryToStand( OB, false );
							goto JJJ;
						};
					};
					TryToMove( OB, dirr, false );
					OB->RealDir = dirr;
					OB->GraphDir = dirr;
					UnitsField.BSetBar( xfin, yfin, LX );
					OB->x = xfin;
					OB->y = yfin;
					OB->MoveStage = 0;
				JJJ:;
				};
			}
			else
			{
				if ( ddir > 0 )OB->RealDir += mrot;
				else OB->RealDir -= mrot;
				OB->GraphDir = OB->RealDir;
			};
		}
		else
		{

			TryToStand( OB, true );
			OB->MoveStage = 0;
		};
	}
	else
	{
		if ( OB->InMotion )
		{
			OB->UnBlockUnit();
			//if(OB->BackMotion){
			//	OB->RealX-=(NMN->OneStepDX[OB->RealDir]*OB->Speed)>>4;
			//	OB->RealY-=(NMN->OneStepDY[OB->RealDir]*OB->Speed)>>4;
			//}else{
			//	OB->RealX+=(NMN->OneStepDX[OB->RealDir]*OB->Speed)>>4;
			//	OB->RealY+=(NMN->OneStepDY[OB->RealDir]*OB->Speed)>>4;
			//};
			OB->RealX += OB->RealVx << SpeedSh;
			OB->RealY += OB->RealVy << SpeedSh;
			if ( OB->AlwaysLock )OB->WeakBlockUnit();
		};
		OB->NewCurSprite += FrmDec;
	};
	if ( OB->StandTime < 2 )OB->RZ = GetUnitHeight( ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 4, ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 4 );
};
//for slow rotated objects

int CheckMotionAbility( OneObject* OB, char dirr, bool Remove )
{
	if ( OB->UnlimitedMotion )
	{
		return 0;
	}
	NewMonster* NMN = OB->newMons;
	int nst = NMN->MotionL.NFrames;
	int dxx = NMN->OneStepDX[byte( dirr )];
	int dyy = NMN->OneStepDY[byte( dirr )];
	int dx0 = dxx*nst;
	int dy0 = dyy*nst;
	int LX = OB->Lx;
	UnitsField.BClrBar( OB->x, OB->y, LX );
	int xfin = ( OB->RealX + dx0 - ( LX << 7 ) ) >> 8;
	int yfin = ( OB->RealY + dy0 - ( LX << 7 ) ) >> 8;
	if ( Remove )
	{
		UnitsField.BSetBar( OB->x, OB->y, LX );
		RemoveFools( xfin, yfin, LX, OB );
	}
	else
	{
		bool NewPath = CheckBar( xfin, yfin, LX, LX );
		if ( NewPath || UnitsField.CheckBar( xfin, yfin, LX, LX ) )
		{
			if ( NewPath )
			{
				return -1;
			}
			else
			{
				return -2;
			}
		}
	}
	return 0;
}

void MotionHandler3( OneObject* OB )
{
	//if(OB->delay)OB->delay--;
	if ( OB->PathDelay >= FrmDec )OB->PathDelay -= FrmDec;
	else OB->PathDelay = 0;
	if ( OB->BackDelay >= FrmDec )OB->BackDelay -= FrmDec;
	else OB->BackDelay = 0;
	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	NMN->AdvChar = ADC;
	if ( NMN->WaterActive )SpotByUnit( OB->RealX, OB->RealY, 35, OB->RealDir );
	if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
	{
		if ( OB->DestX > 0 )
		{
			int dx = OB->DestX - OB->RealX;
			int dy = OB->DestY - OB->RealY;
			int Nrm = Norma( dx, dy );
			char BestDir = char( GetDir( dx, dy ) );
			char ddir = BestDir - OB->RealDir;
			int mrot = NMN->MinRotator;
			int ChRes, ChRes0;
			char NewBestDir = 0;
			int LX = OB->Lx;
			UnitsField.BClrBar( OB->x, OB->y, LX );
			bool CanMove = true;
			bool NeedPush = false;
			bool NeedPath = false;
			ChRes0 = CheckMotionAbility( OB, BestDir, false );
			if ( ChRes0 )
			{
				ChRes = CheckMotionAbility( OB, BestDir + 16, false );
				if ( ChRes )
				{
					ChRes = CheckMotionAbility( OB, BestDir - 16, false );
					if ( ChRes )
					{
						ChRes = CheckMotionAbility( OB, BestDir + 32, false );
						if ( ChRes )
						{
							ChRes = CheckMotionAbility( OB, BestDir - 32, false );
							if ( ChRes )
							{
								ChRes = CheckMotionAbility( OB, BestDir + 48, false );
								if ( ChRes )
								{
									ChRes = CheckMotionAbility( OB, BestDir - 48, false );
									if ( ChRes )
									{
										ChRes = CheckMotionAbility( OB, BestDir - 54, false );
										if ( ChRes )
										{
											ChRes = CheckMotionAbility( OB, BestDir + 54, false );
											if ( ChRes )
											{
												if ( ChRes0 == -1 )
												{
													NeedPath = true;
												}
												else
												{
													NeedPush = true;
												}
												CanMove = false;
											}
											else NewBestDir = BestDir + 54;
										}
										else NewBestDir = BestDir - 54;
									}
									else NewBestDir = BestDir - 48;
								}
								else NewBestDir = BestDir + 48;
							}
							else NewBestDir = BestDir - 32;
						}
						else NewBestDir = BestDir + 32;
					}
					else NewBestDir = BestDir - 16;
				}
				else NewBestDir = BestDir + 16;
			}
			else NewBestDir = BestDir;
			if ( CanMove )
			{
				OB->MoveStage = 0;
				char ddir = NewBestDir - OB->RealDir;
				if ( abs( ddir ) <= mrot )
				{
					if ( NeedPath )
					{
						if ( OB->PathX&&OB->NIPoints )
						{
							OB->CreatePrePath( OB->PathX[OB->NIPoints - 1], OB->PathY[OB->NIPoints - 1] );
							if ( AllowPathDelay )OB->PathDelay = 32 + ( rando() & 7 );
						}
						else
						{
							CreateFullPath( ( OB->DestX - ( OB->Lx << 7 ) ) >> 8, ( OB->DestY - ( OB->Lx << 7 ) ) >> 8, OB );
						};
						OB->MoveStage = 0;
						UnitsField.BSetBar( OB->x, OB->y, LX );
						TryToStand( OB, false );
					}
					else if ( NeedPush )
					{
						if ( OB->MoveStage > 4 )CheckMotionAbility( OB, NewBestDir, true );
						else OB->MoveStage++;
						UnitsField.BSetBar( OB->x, OB->y, LX );
					};
					if ( CanMove )
					{
						int nst = NMN->MotionL.NFrames >> SpeedSh;
						int dxx = NMN->OneStepDX[byte( NewBestDir )] << SpeedSh;
						int dyy = NMN->OneStepDY[byte( NewBestDir )] << SpeedSh;
						int dx0 = dxx*nst;
						int dy0 = dyy*nst;
						int xfin = ( OB->RealX + dx0 - ( LX << 7 ) ) >> 8;
						int yfin = ( OB->RealY + dy0 - ( LX << 7 ) ) >> 8;
						NewTryToMove( OB, NewBestDir, false, dxx >> SpeedSh, dyy >> SpeedSh );
						OB->RealDir = NewBestDir;
						OB->GraphDir = NewBestDir;
						UnitsField.BSetBar( xfin, yfin, LX );
						OB->x = xfin;
						OB->y = yfin;
						OB->MoveStage = 0;
					}
					else
					{
						UnitsField.BSetBar( OB->x, OB->y, LX );
						TryToStand( OB, false );
					};
				}
				else
				{
					if ( ddir > 0 )OB->RealDir += mrot;
					else OB->RealDir -= mrot;
					OB->GraphDir = OB->RealDir;
					UnitsField.BSetBar( OB->x, OB->y, LX );
					TryToStand( OB, false );
				};
			};
		}
		else
		{
			TryToStand( OB, true );
			OB->MoveStage = 0;
		};
	}
	else
	{
		if ( OB->InMotion )
		{
			OB->UnBlockUnit();
			//if(OB->BackMotion){
			//	OB->RealX-=(NMN->OneStepDX[OB->RealDir]*OB->Speed)>>4;
			//	OB->RealY-=(NMN->OneStepDY[OB->RealDir]*OB->Speed)>>4;
			//}else{
			//	OB->RealX+=(NMN->OneStepDX[OB->RealDir]*OB->Speed)>>4;
			//	OB->RealY+=(NMN->OneStepDY[OB->RealDir]*OB->Speed)>>4;
			//};
			OB->RealX += OB->RealVx << SpeedSh;
			OB->RealY += OB->RealVy << SpeedSh;
			if ( OB->AlwaysLock )OB->WeakBlockUnit();
		};
		OB->NewCurSprite += FrmDec;
	};
	OB->RZ = GetUnitHeight( ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 4, ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 4 );
};
void MotionHandler2( OneObject* OB )
{
	//if(OB->delay)OB->delay--;
	if ( OB->PathDelay >= FrmDec )OB->PathDelay -= FrmDec;
	else OB->PathDelay = 0;
	if ( OB->BackDelay >= FrmDec )OB->BackDelay -= FrmDec;
	else OB->BackDelay = 0;
	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	NMN->AdvChar = ADC;
	if ( NMN->WaterActive )SpotByUnit( OB->RealX, OB->RealY, 35, OB->RealDir );
	if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
	{
		int FX = 0;
		int FY = 0;
		OB->BestNX = OB->RealX;
		OB->BestNY = OB->RealY;
		if ( OB->DestX > 0 )
		{
			//int uux=(OB->DestX-(OB->Lx<<7))>>8;
			//int uuy=(OB->DestY-(OB->Ly<<7))>>8;
			int dst = Norma( OB->RealX - OB->DestX, OB->RealY - OB->DestY );
			if ( dst > 256 )
			{
				FX = div( ( OB->DestX - OB->RealX ) << 7, dst ).quot;
				FY = div( ( OB->DestY - OB->RealY ) << 7, dst ).quot;
			}
			else OB->DestX = -100;
		};
		if ( OB->UnlimitedMotion )
		{
			OB->ForceX = 0;
			OB->ForceY = 0;
		};
		bool ExtForce = OB->ForceX || OB->ForceY;
		FX += OB->ForceX;
		FY += OB->ForceY;
		bool Try1 = false;
	uuuu:
		if ( FX || FY )
		{
			char BestDir = char( GetDir( FX, FY ) );
			int bdx = NMN->MotionL.NFrames*NMN->OneStepDX[byte( BestDir )];
			int bdy = NMN->MotionL.NFrames*NMN->OneStepDY[byte( BestDir )];
			if ( !OB->UnlimitedMotion )
			{
				if ( !CheckTerra( OB->RealX + bdx, OB->RealY + bdy, OB->Lx, OB->LockType ) )
				{
					if ( ( !Try1 ) && ( OB->PathX || OB->DestX < 0 ) )
					{
						int OFX = FX;
						int OFY = FY;
						int xs0 = ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 8;
						int ys0 = ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 8;
						int xs1 = ( OB->RealX + bdx - ( ( OB->Lx ) << 7 ) ) >> 8;
						int ys1 = ( OB->RealY + bdy - ( ( OB->Lx ) << 7 ) ) >> 8;
						if ( CheckBar( xs0, ys1, OB->Lx, OB->Lx ) )FY = 0;
						if ( CheckBar( xs1, ys0, OB->Lx, OB->Lx ) )FX = 0;
						if ( FX&&FY&&OB->DestX > 0 )
						{
							if ( abs( FX ) > abs( FY ) )FY = 0;
							else FX = 0;
						};
						if ( !( FX || FY ) )
						{
							OB->DeletePath();
						}
						else
						{
							;
							Try1 = true;
							goto uuuu;
						};
					};
				};
			};
			char ddir = OB->RealDir - BestDir;
			Try1 = false;
			if ( ExtForce )
			{
				if ( abs( ddir ) > 64 )
				{
					BestDir = BestDir + 128;
					ddir = OB->RealDir - BestDir;
					if ( ddir < 8 )
					{
						OB->RealDir = BestDir;
					}
					else
					{
						if ( ddir > 0 )OB->RealDir -= 8;
						else OB->RealDir += 8;
					};
					OB->GraphDir = OB->RealDir;
					int trmv = TryToMove( OB, OB->RealDir, true );
					if ( trmv != -1 )Try1 = true;
					if ( trmv == 0 )
					{
						PushMonsters();
						OB->NextForceX += FORCEX;
						OB->NextForceY += FORCEY;
					};
				}
				else
				{
					if ( abs( ddir ) < 8 )OB->RealDir = BestDir;
					else
					{
						if ( ddir > 0 )OB->RealDir -= 8;
						else OB->RealDir += 8;
					};
					OB->GraphDir += 8;
					int trmv = TryToMove( OB, OB->RealDir, false );
					if ( trmv == 0 )
					{
						PushMonsters();
						OB->NextForceX += FORCEX;
						OB->NextForceY += FORCEY;
					};
					if ( trmv != -1 )Try1 = true;
				};
			};
			if ( !Try1 )
			{
				if ( abs( ddir ) < MinRot )
				{
					OB->RealDir = BestDir;
					OB->GraphDir = OB->RealDir;
					int trmv = TryToMove( OB, OB->RealDir, false );
					if ( trmv == 0 )
					{
						PushMonsters();
						OB->NextForceX += FORCEX;
						OB->NextForceY += FORCEY;
					};
					if ( trmv == -1 )
					{
						if ( OB->DestX > 0 )
						{
							if ( OB->PathX&&OB->NIPoints )
							{
								OB->CreatePrePath( OB->PathX[OB->NIPoints - 1], OB->PathY[OB->NIPoints - 1] );
								TryToStand( OB, false );
							}
							else
							{
								CreateFullPath( ( OB->DestX - ( OB->Lx << 7 ) ) >> 8, ( OB->DestY - ( OB->Lx << 7 ) ) >> 8, OB );
								TryToStand( OB, false );
							};
						}
						else
						{
							TryToStand( OB, false );
						};
					};
					OB->RotCntr = 0;
				}
				else
				{
					if ( ddir > 0 )
					{
						RotateMon( OB, -MinRot );//OB->RealDir-=MinRot;
						//	if(OB->RotCntr>1){
						if ( TryToMove( OB, OB->RealDir, false ) != 1 )
						{
							TryToStand( OB, false );
						}
						else
						{
							OB->InMotion = true;
							OB->NewCurSprite = 0;
							if ( OB->LeftLeg )OB->NewAnm = &NMN->MotionL;
							else OB->NewAnm = &NMN->MotionR;
							OB->LeftLeg = !OB->LeftLeg;
							//OB->NewAnm=&NMN->MotionL;
							OB->RotCntr = 0;
						};
					}
					else
					{
						RotateMon( OB, MinRot );//OB->RealDir+=MinRot;
						if ( TryToMove( OB, OB->RealDir, false ) != 1 )
						{
							TryToStand( OB, false );
						}
						else
						{
							OB->InMotion = true;
							OB->NewCurSprite = 0;
							if ( OB->LeftLeg )OB->NewAnm = &NMN->MotionL;
							else OB->NewAnm = &NMN->MotionR;
							OB->LeftLeg = !OB->LeftLeg;
							//OB->NewAnm=&NMN->MotionR;
							OB->RotCntr = 0;
						};
					};
				};
			};
		}
		else
		{
			TryToStand( OB, true );
		};
	}
	else
	{
		if ( OB->InMotion )
		{
			OB->UnBlockUnit();
			if ( OB->BackMotion )
			{
				OB->RealX -= ( NMN->OneStepDX[OB->RealDir] * OB->Speed ) >> 4;
				OB->RealY -= ( NMN->OneStepDY[OB->RealDir] * OB->Speed ) >> 4;
			}
			else
			{
				OB->RealX += ( NMN->OneStepDX[OB->RealDir] * OB->Speed ) >> 4;
				OB->RealY += ( NMN->OneStepDY[OB->RealDir] * OB->Speed ) >> 4;
			};
		};
		OB->NewCurSprite += FrmDec;
	};
	OB->x = ( OB->RealX - ( ( OB->Lx ) << 7 ) ) >> 8;
	OB->y = ( OB->RealY - ( ( OB->Lx ) << 7 ) ) >> 8;
	OB->RZ = 0;//GetHeight((OB->RealX-((OB->Lx)<<7))>>4,(OB->RealY-((OB->Lx)<<7))>>4);
	//if(OB->LocalOrder){
	//	OB->LocalOrder->DoLink(OB);
	//}else{
	//	OB->PrioryLevel=0;
	//	//OB->AlwaysLock=false;
	//};
	//OB->SearchVictim();
};
void CreateOrdersList( OneObject* OB, char* Str )
{
	char ccc[32];
	Str[0] = 0;
	if ( OB->LocalOrder )
	{
		Order1* OR1 = OB->LocalOrder;
		while ( OR1 )
		{
			sprintf( ccc, " %d", OR1->OrderType );
			strcat( Str, ccc );
			rando();
			OR1 = OR1->NextOrder;
		};
	}
	else
	{
		strcpy( Str, "No orders." );
	};

};
char scs[16];
char* its1( int i )
{
	sprintf( scs, " %d", i );
	return scs;
};
void CreateTimedHint( char* s, int time );
int DoLink_Time, SearchVictim_Time, CheckCapture_Time;

extern HGLOBAL PTR_MISS;
int rppx = 0;

void LongProcesses()
{

	int T0 = GetTickCount();

	int d = tmtmt & 7;
	int d1 = tmtmt & 15;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			if ( OB->Nat->AI_Enabled )
			{
				int use = OB->newMons->Usage;
				if ( use != TowerID )
				{
					if ( ( i & 7 ) == d )OB->SearchVictim();
				}
				else
				{
					OB->SearchVictim();
				};
			}
			else
			{
				int use = OB->newMons->Usage;
				if ( use == PeasantID )
				{
					if ( ( i & 15 ) == d1 )OB->SearchVictim();
				}
				else
				{
					OB->SearchVictim();
				};
			};

		};
	};

	SearchVictim_Time = GetTickCount() - T0;
	T0 = GetTickCount();

	for ( int i = 0; i < 8; i++ )
	{
		int N = NtNUnits[i];
		if ( N )
		{
			for ( int j = 0; j < NtNUnits[i]; j++ )
			{
				OneObject* OB = Group[NatList[i][j]];
				if ( OB && !OB->Sdoxlo )
				{


					Order1* OR1 = OB->LocalOrder;
					//int orr=-1;
					if ( OR1 )
					{
						//Order1 ORR=*OR1;

						OR1->DoLink( OB );
					}
					else
					{

						OB->PrioryLevel = 0;
						if ( OB->StandTime == 48 )
						{

							GotoFinePosition( OB );
						};
					};
					if ( OB->NewBuilding )
					{
						if ( OB->delay >= FrmDec )OB->delay -= FrmDec;
						else OB->delay = 0;
					}
					else
					{
						switch ( OB->newMons->MotionStyle )
						{
						case 0:
							MotionHandler1( OB );


							break;
						case 1:
							MotionHandler3( OB );

							break;
						case 2:
							//MotionHandler2(OB);
							PerformMotion2( OB );

							break;
						};
					};
				};
			};
		};
	};

	DoLink_Time = GetTickCount() - T0;
	T0 = GetTickCount();
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		int mm = i & 31;
		OneObject* OB = Group[i];
		if ( OB && !OB->Sdoxlo )
		{
			if ( ( tmtmt & 31 ) == mm && ( OB->newMons->Capture || !OB->Ready ) )
			{
				CheckCapture( OB );
			}
		}
	}

	CheckCapture_Time = GetTickCount() - T0;
}

void CheckArmies( City* );

void EliminateBuilding( OneObject* OB );

void CalculateMotion()
{
	byte MyNT = NatRefTBL[MyNation];

	LongProcesses();

	byte Mask = NATIONS[NatRefTBL[MyNation]].NMask;
	bool sce = !( ( SCENINF.hLib == nullptr )/*||SCENINF.StandartVictory*/ );
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			if ( !OB->Sdoxlo )
			{
				NewMonster* NM = OB->newMons;
				if ( !NM->SlowRecharge )
				{
					if ( OB->delay >= FrmDec )OB->delay -= FrmDec;
					else OB->delay = 0;
				}
				else
				{
					if ( OB->delay )
					{
						if ( OB->StandTime > 20 && !OB->LocalOrder )
						{
							int cf = OB->NewCurSprite >> SpeedSh;
							int nf = ( OB->NewAnm->NFrames - 1 ) >> SpeedSh;
							if ( nf == cf )
							{
								if ( OB->NewState != 4 )
								{
									OB->NewState = 4;
									TryToStand( OB, 0 );
								}
								else
								{
									NewAnimation* ANM = &NM->Attack[3];
									if ( OB->NewAnm != ANM )
									{
										if ( OB->delay > ANM->NFrames )
											OB->delay -= ANM->NFrames;
										else
											OB->delay = 0;
										OB->NewAnm = ANM;
										OB->NewCurSprite = 0;
										OB->InMotion = 0;
										OB->BackMotion = 0;
									}
								}
							}
						}
					}
					else
					{
						if ( OB->NewState == 4 )
						{
							OB->NewState = OB->GroundState;
						}
					}
				}

				word HARCH = OB->Nat->Harch;
				if ( !( HARCH || OB->NewBuilding || rando() > 50 ) )
				{
					if ( !OB->newMons->NotHungry )
					{
						if ( OB->NNUM == MyNation )
						{
							CreateTimedHintEx( VERYHUN, kImportantMessageDisplayTime, 32 );//WARNING! A famine has befallen your state!
						}
						OB->Die();
					}
				}

				if ( sce )
				{
					if ( OB->NNUM == MyNT )UnitLight( OB );
				}
				else if ( OB->NMask&Mask )UnitLight( OB );
				if ( OB->NewBuilding )
				{
					int HLife = OB->MaxLife >> 1;
					if ( OB->Life < HLife )
					{

						bool DoFog = false;
						HLife >>= 1;
						int Prob = NM->Fogging.WProb;
						if ( OB->Life < HLife )
						{
							if ( rando() < Prob )DoFog = true;
						}
						else
						{
							Prob >>= 1;
							if ( rando() < Prob )DoFog = true;
						};
						if ( DoFog&&NM->Fogging.NWeap&&OB->Stage == OB->NStages )
						{
							Prob = ( rando()*NM->Fogging.NWeap ) >> 15;
							Weapon* WP = WPLIST[NM->Fogging.Weap[Prob]];
							Prob = ( rando()*NM->NDamPt ) >> 15;
							int xx, yy;
							OB->GetCornerXY( &xx, &yy );
							if ( NM->NDamPt )
							{
								xx += NM->DamPtX[Prob];
								yy += NM->DamPtY[Prob];
							};
							xx <<= 4;
							yy <<= 4;
							Create3DAnmObject( WP, xx, yy, GetHeight( xx, yy ) + 4, xx, yy, -1000, nullptr );
						};
					};
				};
				if ( OB->DstX&&OB->ImSelected&GM( MyNation ) )AddDestn( OB->DstX >> 4, OB->DstY >> 4 );
			}
			else if ( !OB->Hidden )
			{
				//death processing
				NewMonster* NMN = OB->newMons;
				if ( OB->NewBuilding )
				{
					OB->Sdoxlo++;
					if ( OB->Sdoxlo > 1000 )
					{
						OB->LoLayer = &NMN->DeathLie2;
					};
					if ( OB->Sdoxlo > 2000 )
					{
						EliminateBuilding( OB );
						DelObject( OB );
						Group[OB->Index] = nullptr;
					};
				}
				else
					if ( NMN->Death.Enabled )
					{
						if ( OB->Sdoxlo == 1 )
						{
							OB->NewAnm = &NMN->Death;
							OB->NewCurSprite = 0;
						};
						if ( OB->NewAnm == &NMN->Death )
						{
							OB->NewCurSprite += FrmDec;
							if ( OB->NewCurSprite >= NMN->Death.NFrames - FrmDec )
							{
								if ( NMN->DeathLie1.Enabled )
								{
									OB->NewAnm = &NMN->DeathLie1;
									OB->NewCurSprite = 0;
								}
								else
								{
									OB->DeletePath();
									OB->ClearOrders();
									DelObject( OB );
									Group[OB->Index] = nullptr;
									OB = nullptr;//died.	
								};
							};
						}
						else
						{
							OB->Sdoxlo++;
							if ( OB->Sdoxlo > 1200 )
							{
								if ( NMN->DeathLie2.Enabled )
								{
									OB->NewAnm = &NMN->DeathLie2;
									OB->NewCurSprite = 0;
								}
								else
								{
									//OneObject OBBX=*OB;
									OB->DeletePath();
									OB->ClearOrders();
									DelObject( OB );
									Group[OB->Index] = nullptr;
									OB = nullptr;//died.	
								};
							};
							if ( OB&&OB->Sdoxlo > 1600 )
							{
								if ( NMN->DeathLie2.Enabled )
								{
									OB->NewAnm = &NMN->DeathLie3;
									OB->NewCurSprite = 0;
								}
								else
								{
									OB->DeletePath();
									OB->ClearOrders();
									DelObject( OB );
									Group[OB->Index] = nullptr;
									OB = nullptr;//died.	
								};
							};
							if ( OB&&OB->Sdoxlo > 2000 )
							{
								OB->DeletePath();
								OB->ClearOrders();
								DelObject( OB );
								Group[OB->Index] = nullptr;
								OB = nullptr;//died.	
							};
						};
						if ( OB )OB->Sdoxlo++;
					};
			};
		};
	};

};
void CalculateMotionV2()
{
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NewMonst )
		{
			OB->ForceX = OB->NextForceX;
			OB->ForceY = OB->NextForceY;
			OB->NextForceX = 0;
			OB->NextForceY = 0;
			if ( OB->Sdoxlo )
			{
				//death processing
				NewMonster* NMN = OB->newMons;
				if ( NMN->Death.Enabled )
				{
					if ( OB->NewAnm == &NMN->Death )
					{
						OB->NewCurSprite += FrmDec;
					};
					if ( OB->Sdoxlo < 3 && OB->NewAnm != &NMN->Death )
					{
						OB->NewAnm = &NMN->Death;
						OB->NewCurSprite = 0;
						OB->InMotion = false;
						OB->BackMotion = false;
					}
					else
					{
						if ( OB->Sdoxlo > 3 && OB->NewCurSprite >= NMN->Death.NFrames - FrmDec )
						{
							//finally dead
							if ( OB->Sdoxlo < 100 )OB->NewAnm = &NMN->DeathLie1;
							else if ( OB->Sdoxlo < 300 )OB->NewAnm = &NMN->DeathLie2;
							else if ( OB->Sdoxlo < 600 )OB->NewAnm = &NMN->DeathLie3;
							else
							{
								//erasing of the monster
								if ( OB->PathX )free( OB->PathX );
								if ( OB->PathY )free( OB->PathY );
								Group[OB->Index] = nullptr;
								free( OB );
								OB = nullptr;
							};
							if ( OB )OB->NewCurSprite = 0;
						};
					};
					OB->Sdoxlo++;
				};
			};
		}
		else
		{
			//erasing of the monster
			//if(OB->PathX)free(OB->PathX);
			//if(OB->PathY)free(OB->PathY);
			//Group[OB->Index]=nullptr;
			//free(OB);
			//OB=nullptr;
		};
	};
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NewMonst && !OB->Sdoxlo )
		{
			NewMonster* NMN = OB->newMons;
			if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
			{
				int FX = 0;
				int FY = 0;
				OB->BestNX = OB->RealX;
				OB->BestNY = OB->RealY;
				if ( OB->DestX > 0 )
				{
					//int uux=(OB->DestX-(OB->Lx<<7))>>8;
					//int uuy=(OB->DestY-(OB->Ly<<7))>>8;
					int dst = Norma( OB->RealX - OB->DestX, OB->RealY - OB->DestY );
					if ( dst > 256 )
					{
						FX = div( ( OB->DestX - OB->RealX ) << 8, dst ).quot;
						FY = div( ( OB->DestY - OB->RealY ) << 8, dst ).quot;
					}
					else OB->DestX = -100;
				};
				FX += OB->ForceX;
				FY += OB->ForceY;
				bool Try1 = false;
			uuuu:
				if ( FX || FY )
				{
					char BestDir = char( GetDir( FX, FY ) );
					//need optimisation for* !:
					int bdx = NMN->MotionL.NFrames*NMN->OneStepDX[byte( BestDir )];
					int bdy = NMN->MotionL.NFrames*NMN->OneStepDY[byte( BestDir )];
					if ( !CheckTerra( OB->RealX + bdx, OB->RealY + bdy, OB->Lx, OB->LockType ) )
					{
						if ( ( !Try1 ) && ( OB->PathX || OB->DestX < 0 ) )
						{
							int OFX = FX;
							int OFY = FY;
							int xs0 = ( OB->RealX - ( OB->Lx << 7 ) ) >> 8;
							int ys0 = ( OB->RealY - ( OB->Ly << 7 ) ) >> 8;
							int xs1 = ( OB->RealX + bdx - ( OB->Lx << 7 ) ) >> 8;
							int ys1 = ( OB->RealY + bdy - ( OB->Lx << 7 ) ) >> 8;
							if ( CheckBar( xs0, ys1, OB->Lx, OB->Lx ) )FY = 0;
							if ( CheckBar( xs1, ys0, OB->Lx, OB->Lx ) )FX = 0;
							if ( FX&&FY&&OB->DestX > 0 )
							{
								if ( abs( FX ) > abs( FY ) )FY = 0;
								else FX = 0;
							};
							if ( !( FX || FY ) )
							{
								//	FX=-OFX;
								//	FY=-OFY;
							};
							Try1 = true;
							goto uuuu;
						};
					};
					char ddir = OB->RealDir - BestDir;
					if ( abs( ddir ) < MinRot )
					{
						OB->RealDir = BestDir;
						OB->GraphDir = OB->RealDir;
						int trmv = TryToMove( OB, OB->RealDir, false );
						if ( !trmv )
						{
							int bdx = NMN->MotionL.NFrames*NMN->OneStepDX[OB->RealDir];
							int bdy = NMN->MotionL.NFrames*NMN->OneStepDY[OB->RealDir];
							PushMonsters();
							OB->NextForceX += FORCEX;
							OB->NextForceY += FORCEY;
							TryToStand( OB, false );
						}
						else
						{
							if ( trmv == -1 )
							{
								if ( OB->DestX > 0 )
								{
									if ( OB->PathX&&OB->NIPoints )
									{
										OB->CreatePrePath( OB->PathX[OB->NIPoints - 1], OB->PathY[OB->NIPoints - 1] );
										TryToStand( OB, false );
										OB->PathDelay = 32 + ( rando() & 7 );
									}
									else
									{

										CreateFullPath( ( OB->DestX - ( OB->Lx << 7 ) ) >> 8, ( OB->DestY - ( OB->Lx << 7 ) ) >> 8, OB );
										TryToStand( OB, false );
									};
								}
								else
								{
									TryToStand( OB, false );
								};
							};
						};
						OB->RotCntr = 0;
					}
					else
					{
						if ( ddir > 0 )
						{
							RotateMon( OB, -MinRot );//OB->RealDir-=MinRot;
							if ( OB->RotCntr > 1 )
							{
								if ( TryToMove( OB, OB->RealDir, false ) != 1 )
								{
									TryToStand( OB, false );
								}
								else
								{
									OB->InMotion = true;
									OB->NewCurSprite = 0;
									if ( OB->LeftLeg )OB->NewAnm = &NMN->MotionL;
									else OB->NewAnm = &NMN->MotionR;
									OB->LeftLeg = !OB->LeftLeg;
									//OB->NewAnm=&NMN->MotionL;
									OB->RotCntr = 0;
								};
							}
							else
							{
								TryToStand( OB, false );
							};
						}
						else
						{
							RotateMon( OB, MinRot );//OB->RealDir+=MinRot;
							if ( OB->RotCntr > 1 )
							{
								if ( TryToMove( OB, OB->RealDir, false ) != 1 )
								{
									TryToStand( OB, false );
								}
								else
								{
									OB->InMotion = true;
									OB->NewCurSprite = 0;
									if ( OB->LeftLeg )OB->NewAnm = &NMN->MotionL;
									else OB->NewAnm = &NMN->MotionR;
									OB->LeftLeg = !OB->LeftLeg;
									//OB->NewAnm=&NMN->MotionR;
									OB->RotCntr = 0;
								};
							}
							else
							{
								TryToStand( OB, false );
							};
						};
					};
				}
				else
				{
					TryToStand( OB, true );
				};
			}
			else
			{
				if ( OB->InMotion )
				{
					OB->UnBlockUnit();
					OB->RealX += NMN->OneStepDX[OB->RealDir];
					OB->RealY += NMN->OneStepDY[OB->RealDir];
				};
				OB->NewCurSprite += FrmDec;
			};
			OB->x = ( OB->RealX - ( OB->Lx << 7 ) ) >> 8;
			OB->y = ( OB->RealY - ( OB->Lx << 7 ) ) >> 8;
			OB->RZ = GetHeight( ( OB->RealX - ( OB->Lx << 7 ) ) >> 4, ( OB->RealY - ( OB->Lx << 7 ) ) >> 4 );
			if ( OB->LocalOrder )
			{
				OB->LocalOrder->DoLink( OB );
			};
		};
	};
};
//--------------------------------------------------------------//
int RoundX( int x )
{
	return ( x >> 8 ) << 8;
};
int RoundY( int y )
{
	return ( div( y, 8 * 16 ).quot )*( 8 * 16 );
};
void BSetPt( int x, int y );
void CreateAveragePlane( int x, int y, int r );
extern word LastObject;
bool CheckSpritesInArea( int x, int y, int r );
bool CheckBuildingsInArea( int x0, int y0, int x1, int y1, word* BLD, int Nb )
{
	if ( x0 == x1&&y0 == y1 )return true;
	int nx0 = x0 + y0;
	int ny0 = y0 - x0;
	int nx1 = x1 + y1;
	int ny1 = y1 - x1;
	int nxc = ( nx0 + nx1 ) >> 1;
	int nyc = ( ny0 + ny1 ) >> 1;
	int Rx = abs( nx1 - nx0 ) >> 1;
	int Ry = abs( ny1 - ny0 ) >> 1;
	for ( int i = 0; i < Nb; i++ )
	{
		OneObject* OB = Group[BLD[i]];
		if ( OB )
		{
			NewMonster* NM = OB->newMons;
			if ( NM->Building )
			{
				int bx0 = OB->RealX + ( NM->BuildX0 << 4 );
				int by0 = OB->RealY + ( NM->BuildY0 << 4 );
				int bx1 = OB->RealX + ( NM->BuildX1 << 4 );
				int by1 = OB->RealY + ( NM->BuildY1 << 4 );
				if ( bx0 != bx1 || by0 != by1 )
				{
					int bnx0 = bx0 + by0;
					int bny0 = by0 - bx0;
					int bnx1 = bx1 + by1;
					int bny1 = by1 - bx1;
					int bnxc = ( bnx0 + bnx1 ) >> 1;
					int bnyc = ( bny0 + bny1 ) >> 1;
					int bRx = abs( bnx1 - bnx0 ) >> 1;
					int bRy = abs( bny1 - bny0 ) >> 1;
					if ( abs( bnxc - nxc ) < bRx + Rx&&abs( bnyc - nyc ) < bRy + Ry )return false;
				};
			};
		};
	};
	return true;
};
bool CheckBuildingsForWalls( int x, int y )
{
	int nr = 4;
	int nr1 = nr + nr + 1;
	int cx = ( x >> 11 );
	int cy = ( y >> 11 );

	int nxc = x + y;
	int nyc = y - x;

	int ofs0 = ( cx - nr ) + ( ( cy - nr ) << VAL_SHFCX ) + VAL_MAXCX + 1;
	int ofs1 = ofs0 << SHFCELL;
	for ( int dy = 0; dy < nr1; dy++ )
	{
		for ( int dx = 0; dx < nr1; dx++ )
		{
			if ( ofs0 >= 0 && ofs0 < MAXCIOFS )
			{
				word Mon1 = BLDList[ofs0];
				if ( Mon1 != 0xFFFF )
				{
					OneObject* OB = Group[Mon1];
					if ( OB )
					{
						NewMonster* NM = OB->newMons;
						if ( NM->Building )
						{
							int bx0 = OB->RealX + ( NM->BuildX0 << 4 );
							int by0 = OB->RealY + ( NM->BuildY0 << 4 );
							int bx1 = OB->RealX + ( NM->BuildX1 << 4 );
							int by1 = OB->RealY + ( NM->BuildY1 << 4 );
							if ( bx0 != bx1 || by0 != by1 )
							{
								int bnx0 = bx0 + by0;
								int bny0 = by0 - bx0;
								int bnx1 = bx1 + by1;
								int bny1 = by1 - bx1;
								int bnxc = ( bnx0 + bnx1 ) >> 1;
								int bnyc = ( bny0 + bny1 ) >> 1;
								int bRx = abs( bnx1 - bnx0 ) >> 1;
								int bRy = abs( bny1 - bny0 ) >> 1;
								if ( abs( bnxc - nxc ) < bRx + 2048 && abs( bnyc - nyc ) < bRy + 2048 )return false;
							};
						};
					};

				};
			};
			ofs0++;
			ofs1 += MAXINCELL;
		};
		ofs0 += VAL_MAXCX - nr1;
		ofs1 += ( VAL_MAXCX - nr1 ) << SHFCELL;
	};
	return true;
};
bool CheckMonstersInArea( int x, int y, int r )
{
	int nr = ( r >> 11 ) + 1;
	int nr1 = nr + nr + 1;
	int cx = ( x >> 11 );
	int cy = ( y >> 11 );
	int ofs0 = ( cx - nr ) + ( ( cy - nr ) << VAL_SHFCX );
	int ofs1 = ofs0 << SHFCELL;
	for ( int dy = 0; dy < nr1; dy++ )
	{
		for ( int dx = 0; dx < nr1; dx++ )
		{
			if ( ofs0 >= 0 && ofs0 < MAXCIOFS )
			{
				int NMon = MCount[ofs0];
				for ( int z = 0; z < NMon; z++ )
				{
					word MD = GetNMSL( z + ofs1 );
					OneObject* OB = Group[MD];
					if ( OB&&OB->NewMonst&&Norma( OB->RealX - x, OB->RealY - y ) < r )return false;
				};
			};
			ofs0++;
			ofs1 += MAXINCELL;
		};
		ofs0 += VAL_MAXCX - nr1;
		ofs1 += ( VAL_MAXCX - nr1 ) << SHFCELL;
	};
	return true;
};
int CheckMinePosition( NewMonster* NM, int* xi, int* yi, int r );
bool FindPortPlace( NewMonster* NM, int x, int y, int* BuiX, int* BuiY );
int CheckVLine( int x, int y, int Lx );
int CheckHLine( int x, int y, int Lx );
bool CheckSpritesInAreaNew( int x, int y, int r, bool Erase );
void EraseTreesInPoint( int x, int y );
int CreateBLDList( byte NI, word* BLD, int MaxBLD, int x0, int y0 )
{
	int N = 0;
	word* Units = NatList[NI];
	int Nu = NtNUnits[NI];
	for ( int i = 0; i < Nu; i++ )
	{
		OneObject* OB = Group[Units[i]];
		if ( OB&&OB->NewBuilding&&Norma( OB->RealX - x0, OB->RealY - y0 ) < 1000 * 16 && !OB->Sdoxlo )
		{
			if ( N < MaxBLD )
			{
				BLD[N] = Units[i];
				N++;
			};
		};
	};
	return N;
};
int CheckCreationAbility( byte NI, NewMonster* NM, int* x2i, int* y2i, word* BLD, int NBLD )
{
	word TotBLD[256];
	if ( NM->Building&&NI != 0xFF && !BLD )
	{
		NBLD = CreateBLDList( NI, TotBLD, 256, *x2i, *y2i );
		BLD = TotBLD;
	};
	int NBLD1 = NBLD;
	byte Use = NM->Usage;
	if ( NI != 0xFF && NATIONS[NI].AI_Enabled&&Use != SkladID )NBLD1 = 0;
	int x2 = *x2i;
	int y2 = *y2i;
	int xs = x2 >> 9;
	int ys = y2 >> 9;
	if ( xs <= 0 || ys <= 0 || xs >= msx || ys >= msy )return -1;
	if ( NI != 0xFF && !CheckBuildPossibility( NI, x2 >> 4, y2 >> 4 ) )return -1;
	if ( NM->Building )
	{
		int x, y, xm, ym;
		int ID = 0;
		int PicSX = NM->PicDx << 4;
		int PicSY = Prop43( NM->PicDy ) << 4;
		xm = ( x2 + PicSX ) >> 8;
		ym = ( y2 + PicSY ) >> 8;
		x = ( ( xm + NM->CenterMX ) << 8 ) + 128;
		y = ( ( ym + NM->CenterMY ) << 8 ) + 128;
		int r = ( NM->BRadius + 2 ) << 8;
		if ( NM->ProdType )
		{
			int xx = x2;
			int yy = y2;
			ID = CheckMinePosition( NM, &xx, &yy, r );
			*x2i = xx;
			*y2i = yy;
			x2 = xx;
			y2 = yy;
			xm = ( x2 + PicSX ) >> 8;
			ym = ( y2 + PicSY ) >> 8;
			x = ( ( xm + NM->CenterMX ) << 8 ) + 128;
			y = ( ( ym + NM->CenterMY ) << 8 ) + 128;
			if ( ID == -1 )return -1;
		}
		else
		{
			bool ETree = false;
			for ( int p = 0; p < NBLD1; p++ )
			{
				OneObject* OB = Group[BLD[p]];
				if ( OB->Ready&&Norma( OB->RealX - x, OB->RealY - y ) < 600 * 16 )ETree = true;
			};
			if ( !CheckSpritesInAreaNew( x, y, r, ETree ) )return -1;
		};
		if ( !CheckMonstersInArea( x, y, r ) )return -1;
		if ( !CheckBuildingsInArea( x + ( NM->BuildX0 << 4 ), y + ( NM->BuildY0 << 4 ), x + ( NM->BuildX1 << 4 ), y + ( NM->BuildY1 << 4 ), BLD, NBLD ) )return -1;

		//checking locking information
		int np = NM->NCheckPt;
		byte* Cpx = NM->CheckX;
		byte* Cpy = NM->CheckY;
		int maxZ = -100000;
		int minZ = 100000;
		if ( NM->ProdType )
		{
			int nn = 0;
			for ( int i = 0; i < np; i++ )
			{
				int xx = xm + Cpx[i];
				int yy = ym + Cpy[i];
				int z = GetHeight( xx << 4, yy << 4 );
				if ( z > maxZ )maxZ = z;
				if ( z < minZ )minZ = z;
				if ( CheckPt( xx, yy ) )nn++;
				if ( nn > 2 )return -1;
			};
		}
		else
		{
			for ( int i = 0; i < np; i++ )
			{
				int xx = xm + Cpx[i];
				int yy = ym + Cpy[i];
				int z = GetHeight( xx << 4, yy << 4 );
				if ( z > maxZ )maxZ = z;
				if ( z < minZ )minZ = z;
				if ( CheckVLine( xx, yy - 4, 8 ) )return -1;
				if ( CheckHLine( xx - 3, yy, 6 ) )return -1;
			};
		};
		if ( NM->ProdType )maxZ = minZ;
		if ( abs( maxZ - minZ ) > 50 )return -1;
		if ( NM->Port )
		{
			if ( !FindPortPlace( NM, x, y, &PortBuiX, &PortBuiY ) )return -1;
		};
		return ID;
	}
	else
	{
		int mLx = ( NM->Radius2 + 3 ) >> 7;
		if ( !mLx )mLx = 1;
		if ( !CheckTerra( x2, y2, mLx, NM->LockType ) )return -1;
		if ( CheckPosition( x2, y2, NM->Radius2 + 8, 0xFFFF ) )return 0;
		else return -1;
	};
};
int CheckSmartCreationAbility( byte NI, NewMonster* NM, int* x2i, int* y2i )
{
	word TotBLD[256];
	int Nb = 0;
	if ( NM->Building )Nb = CreateBLDList( NI, TotBLD, 256, *x2i, *y2i );
	int v = CheckCreationAbility( NI, NM, x2i, y2i, TotBLD, Nb );
	if ( NM->Port )return v;
	if ( v == -1 )
	{
		int x0 = ( ( *x2i ) >> 8 ) << 8;
		int y0 = ( ( *y2i ) >> 8 ) << 8;
		for ( int r = 1; r < 10; r++ )
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int N = Rarr[r].N;
			for ( int j = 0; j < N; j++ )
			{
				int xc = x0 + ( int( xi[j] ) << 8 );
				int yc = y0 + ( int( yi[j] ) << 8 );
				int xc0 = xc;
				int yc0 = yc;
				v = CheckCreationAbility( NI, NM, &xc, &yc, TotBLD, Nb );
				if ( v != -1 )
				{
					*x2i = xc;
					*y2i = yc;
					return v;
				};
			};
		};
		return -1;
	}
	else return v;
};
int CheckAISmartCreationAbility( byte NI, NewMonster* NM, int* x2i, int* y2i )
{
	word TotBLD[256];
	int Nb = 0;
	if ( NM->Building )Nb = CreateBLDList( NI, TotBLD, 256, *x2i, *y2i );
	int v = CheckCreationAbility( NI, NM, x2i, y2i, TotBLD, Nb );
	if ( NM->Port )return v;
	if ( v == -1 )
	{
		int x0 = ( ( *x2i ) >> 8 ) << 8;
		int y0 = ( ( *y2i ) >> 8 ) << 8;
		for ( int r = 1; r < 10; r++ )
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int N = Rarr[r].N;
			for ( int j = 0; j < N; j++ )
			{
				int xc = x0 + ( int( xi[j] ) << 9 );
				int yc = y0 + ( int( yi[j] ) << 9 );
				v = CheckCreationAbility( NI, NM, &xc, &yc, TotBLD, Nb );
				if ( v != -1 )
				{
					*x2i = xc;
					*y2i = yc;
					return v;
				};
			};
		};
		return -1;
	}
	else return v;
};
int SmartCreationUnit( byte NI, int NIndex, int x, int y )
{
	int v = CheckAISmartCreationAbility( NI, NATIONS[NI].Mon[NIndex]->newMons, &x, &y );
	if ( v != -1 )
	{
		return NATIONS[NI].CreateNewMonsterAt( x, y, NIndex, false );
	}
	else return -1;
};

extern City CITY[8];
extern byte BalloonState;
extern byte CannonState;
extern byte XVIIIState;
void GetUnitCost( byte NI, word NIndex, int* Cost )
{
	Nation* NT = &NATIONS[NI];
	GeneralObject* GO = NT->Mon[NIndex];
	AdvCharacter* ADC = GO->MoreCharacter;
	NewMonster* NM = GO->newMons;
	for ( int i = 0; i < 8; i++ )Cost[i] = ADC->NeedRes[i];
	if ( NM->CostPercent == 100 )return;
	int NUNITS = CITY[NI].UnitAmount[NIndex];
	int Percent = NM->CostPercent;
	bool EXPENS = 0;
	if ( NM->Usage == PushkaID&&CannonState == 2 )EXPENS = 1;
	int TIMES = 1000;
	for ( int j = 0; j < NUNITS; j++ )
	{
		if ( TIMES < 1000000 )
		{
			TIMES = ( TIMES*Percent ) / 100;
			for ( int i = 0; i < 8; i++ )
			{
				Cost[i] = div( Cost[i] * Percent, 100 ).quot;
				if ( EXPENS )Cost[i] *= 10;
				if ( Cost[i] < 0 )Cost[i] = -Cost[i];
			};
		};
	};
};
void GetUnitCost( byte NI, word NIndex, int* Cost, word Power )
{
	Nation* NT = &NATIONS[NI];
	GeneralObject* GO = NT->Mon[NIndex];
	AdvCharacter* ADC = GO->MoreCharacter;
	NewMonster* NM = GO->newMons;
	for ( int i = 0; i < 8; i++ )Cost[i] = ADC->NeedRes[i];
	if ( NM->CostPercent == 100 )return;
	int NUNITS = CITY[NI].UnitAmount[NIndex];
	int Percent = NM->CostPercent;
	bool EXPENS = 0;
	if ( NM->Usage == PushkaID&&CannonState == 2 )EXPENS = 1;
	int TIMES = 1000;
	for ( int j = 0; j < Power; j++ )
	{
		if ( TIMES < 1000000 )
		{
			TIMES = ( TIMES*Percent ) / 100;
			for ( int i = 0; i < 8; i++ )
			{
				Cost[i] = ( Cost[i] * Percent ) / 100;
				if ( EXPENS )Cost[i] *= 10;
				if ( Cost[i] < 0 )Cost[i] = -Cost[i];
			};
		};
	};
};
bool CheckCostHint( byte NI, word NIndex )
{
	Nation* NT = &NATIONS[NI];
	GeneralObject* GO = NT->Mon[NIndex];
	AdvCharacter* ADC = GO->MoreCharacter;
	int Cost[8];
	GetUnitCost( NI, NIndex, Cost );
	char NRES = 0;
	char CCC[128];
	CCC[0] = 0;
	for ( int i = 0; i < 8; i++ )
	{
		if ( XRESRC( NI, i ) < Cost[i] )
		{
			if ( CCC[0] )strcat( CCC, "," );
			strcat( CCC, RDS[i].Name );
			NRES++;
		};
	};
	if ( CCC[0] )
	{
		char CC1[200];
		if ( NRES > 1 )
		{//"Not enough resources: %s"
			sprintf( CC1, NETRESS, CCC );
		}
		else
		{//"Not enough resource: %s"
			sprintf( CC1, NETRES1, CCC );
		}
		CreateTimedHint( CC1, kMinorMessageDisplayTime );//Not enough resources: %s
		return false;
	};
	return true;
};
bool CheckCost( byte NI, word NIndex )
{
	Nation* NT = &NATIONS[NI];
	GeneralObject* GO = NT->Mon[NIndex];
	AdvCharacter* ADC = GO->MoreCharacter;
	int Cost[8];
	GetUnitCost( NI, NIndex, Cost );
	for ( int i = 0; i < 8; i++ )
		if ( XRESRC( NI, i ) < Cost[i] )return false;
	return true;
};
bool ApplyCost( byte NI, word NIndex )
{
	Nation* NT = &NATIONS[NI];
	GeneralObject* GO = NT->Mon[NIndex];
	AdvCharacter* ADC = GO->MoreCharacter;
	int Cost[8];
	GetUnitCost( NI, NIndex, Cost );
	if ( !CheckCost( NI, NIndex ) )return false;
	for ( int i = 0; i < 8; i++ )
	{
		AddXRESRC( NI, i, -Cost[i] );
		NT->ControlProduce( GO->Branch, i, -Cost[i] );
		if ( GO->newMons->Building )NT->ResOnBuildings[i] += Cost[i];
		else NT->ResOnUnits[i] += Cost[i];
	};
	return true;
};
bool ApplyCostUpgrade( byte NI, word NIndex )
{
	Nation* NT = &NATIONS[NI];
	GeneralObject* GO = NT->Mon[NIndex];
	AdvCharacter* ADC = GO->MoreCharacter;
	int Cost[8];
	GetUnitCost( NI, NIndex, Cost );
	if ( !CheckCost( NI, NIndex ) )return false;
	for ( int i = 0; i < 8; i++ )
	{
		AddXRESRC( NI, i, -Cost[i] );
		NT->ControlProduce( GO->Branch, i, -Cost[i] );
		NT->ResOnUpgrade[i] += Cost[i];
	};
	return true;
};
void GetDiscreta( int* x, int* y, int val )
{
	int xx1 = ( *x ) - val;
	int yy1 = ( *y );
	int p = ( xx1 + yy1 ) >> 1;
	int q = ( xx1 - yy1 ) >> 1;
	p = div( p + ( val >> 1 ), val ).quot*val;
	q = div( q + ( val >> 1 ), val ).quot*val;
	*x = p + q;
	*y = p - q;
};
extern MotionField UnitsField;
extern bool SubCost;
extern bool SpriteSuccess;
void FindUnitPosition( int* x, int *y, NewMonster* NM )
{
	int x0 = *x;
	int y0 = *y;
	int SH = 9;
	if ( NM->Usage == FastHorseID )SH++;
	for ( int r = 0; r < 50; r++ )
	{
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		if ( rando() < 16384 )
		{
			for ( int j = 0; j < N; j++ )
			{
				int x1 = x0 + ( int( xi[j] ) << SH );
				int y1 = y0 + ( int( yi[j] ) << SH );
				if ( !CheckCreationAbility( 0xFF, NM, &x1, &y1, nullptr, 0 ) )
				{
					*x = x1;
					*y = y1;
					return;
				};
			};
		}
		else
		{
			for ( int j = N - 1; j >= 0; j-- )
			{
				int x1 = x0 + ( int( xi[j] ) << SH );
				int y1 = y0 + ( int( yi[j] ) << SH );
				if ( !CheckCreationAbility( 0xFF, NM, &x1, &y1, nullptr, 0 ) )
				{
					*x = x1;
					*y = y1;
					return;
				};
			};
		};
	};
};
void CHKS();
void CreateFields( byte NI, int x, int y, int n )
{
	Nation* NT = NATIONS + NI;
	for ( int ix = -3; ix <= 3; ix++ )
		for ( int iy = -3; iy <= 3; iy++ )
		{
			int xx = x + ( ( ix + iy ) * 64 * 16 );
			int yy = y + ( ( ix - iy ) * 64 * 16 );
			NT->CreateNewMonsterAt( xx, yy, n, false );
		};
};
bool GroundBox = 1;
extern byte NTex1[32];
extern byte NTex2[32];
void CreateGround( OneObject* G );
int GetTotalUnits()
{
	int N = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		if ( Group[i] )
		{
			N++;
		}
	}
	return N;
}

extern bool Loadingmap;
void CreateGrassGround( int x, int y );
int Nation::CreateNewMonsterAt( int rx, int ry, int n, bool Anyway )
{
	int maxx = ( msx + 2 ) << ( 5 + 4 );
	if ( rx<1024 || ry<1024 || rx>maxx || ry>maxx )
	{
		return -1;
	}
	rando();
	int x = rx;
	int y = ry;
	GeneralObject* GO = Mon[n];
	NewMonster* NM = GO->newMons;
	//if(!FindPortPlace(NM,x,y,&PortBuiX,&PortBuiY))return -1;
	AdvCharacter* ADC = GO->MoreCharacter;
	int ID = -1;
	if ( GO->LockID != 0xFFFF )
	{
		if ( CITY->UnitAmount[GO->LockID] >= GO->NLockUnits )return-1;
	};
	if ( ( !Anyway ) )
	{
		if ( ( !CheckBuildPossibility( NNUM, rx >> 4, ry >> 4 ) ) ||
			( !CheckCost( NNUM, n ) ) )return -1;
		rando();
	};
	rando();
	if ( NM->SpriteObject && !NM->Wall )
	{
		rando();
		ObjCharacter* OC = &COMPLEX.ObjChar[NM->Sprite];
		int sx = rx >> 4;
		int sy = ry >> 4;
		GetDiscreta( &sx, &sy, OC->Discret );
		//accurate check
		int cxx = sx >> 4;
		int cyy = sy >> 4;
		if ( !CheckBar( cxx - 4, cyy - 3, 8, 15 ) )
		{
			int H1 = GetHeight( sx - 64, sy );
			int H2 = GetHeight( sx + 64, sy );
			int H3 = GetHeight( sx, sy - 64 );
			int H4 = GetHeight( sx, sy + 64 );
			int Ha = ( H1 + H2 + H3 + H4 ) >> 2;
			if ( abs( H1 - Ha ) < 40 && abs( H2 - Ha ) < 32 && abs( H3 - Ha ) < 32 && abs( H4 - Ha ) < 32 )
			{
				addSprite( sx, sy, &COMPLEX, NM->Sprite );
				if ( SpriteSuccess )
				{
					RenewInfoMap( sx, sy );
					if ( !Anyway )ApplyCost( NNUM, n );
					CreateGrassGround( sx, sy );
				};
			};
		};
		return -1;
	};
	if ( !( Anyway || NM->Wall ) )
	{
		ID = CheckCreationAbility( NNUM, GO->newMons, &x, &y, nullptr, 0 );
		if ( ID == -1 )return -1;
		rando();
	};
	if ( NM->Building && !Anyway )
	{
		ApplyCost( NNUM, n );
	};
	if ( NM->ResSubst )
	{
		int xxx = x >> 4;
		int yyy = y >> 4;
		for ( int i = 0; i < MaxSprt; i++ )if ( Sprites[i].Enabled )
		{
			OneSprite* OS = Sprites + i;
			if ( OS->OC->IntResType < 8 && Norma( OS->x - xxx, OS->y - yyy ) < 64 )
			{
				i = MaxSprt;
				byte rt = OS->OC->IntResType;
				if ( rt == IronID )
				{
					n++;
				}
				else if ( rt == CoalID )
				{
					n += 2;
				};
				GO = Mon[n];
				NM = GO->newMons;
				ADC = GO->MoreCharacter;
			};
		};
	}

	int PicSX = NM->PicDx << 4;
	int PicSY = Prop43( NM->PicDy ) << 4;

	if ( NM->Building && !NM->Wall )
	{
		x = RoundX( rx + PicSX ) - PicSX;
		y = RoundX( ry + PicSY ) - PicSY;
	}

	int mLx = ( GO->newMons->Radius2 + 3 ) >> 7;
	if ( !mLx )
	{
		mLx = 1;
	}


	int i;
	for ( i = 0; i < MaxObj && int( Group[i] ); i++ );

	if ( i >= MaxObj )
	{
		return -1;
	}

	rando();

	if ( n >= NMon )
	{
		return -1;
	}

	rando();

	if ( i >= MAXOBJECT )
	{
		MAXOBJECT = i + 1;
	}

	Group[i] = OBJECTS + i;
	memset( Group[i], 0, sizeof OneObject );
	LastObject = i;
	//Cell8x8* CELL=&TCInf[NNUM][y>>11][x>>11];
	OneObject* G = Group[i];
	//AddOrderEffect(x>>9,y>>9,GO->BornSound);
	//CELL->UnitsAmount[GO->Kind]++;
	if ( NM->Port )
	{
		G->WallX = PortBuiX;
		G->WallY = PortBuiY;
		int LL = NM->BuiDist;
		//		MFIELDS[1].BSetBar(PortBuiX-LL-1,PortBuiY-1,3);
		//		MFIELDS[1].BSetBar(PortBuiX+LL-1,PortBuiY-1,3);
		//		MFIELDS[1].BSetBar(PortBuiX-1,PortBuiY-LL-1,3);
		//		MFIELDS[1].BSetBar(PortBuiX-1,PortBuiY+LL-1,3);
	};
	G->Usage = NM->Usage;
	if ( G->Usage == ArcherID&&rando() < 2600 )G->Usage = GrenaderID;
	G->Guard = 0xFFFF;
	G->GroundState = 0;
	G->DoWalls = false;
	G->InPatrol = false;
	G->AddDamage = 0;
	G->AddShield = 0;
	G->StandGround = false;
	G->InArmy = false;
	G->Zombi = 0;
	G->CurAIndex = 0;
	G->MaxAIndex = NM->MaxAIndex;
	G->Ustage = 0;
	G->NUstages = 0;
	G->SingleUpgLevel = 2;
	G->InternalLock = true;
	G->IFire = nullptr;
	G->DefaultSettings( GO );
	G->AlwaysLock = false;
	G->LockType = NM->LockType;
	G->OverEarth = 0;
	G->NewMonst = false;
	G->NewBuilding = false;
	G->Wall = false;
	G->NothingTime = 0;
	G->Hidden = false;
	G->UnlimitedMotion = false;
	G->NoBuilder = false;
	G->DoNotCall = false;
	G->BackSteps = 11;
	G->BackReserv = 0;
	G->AddInside = 0;
	G->PersonalDelay = 100;
	G->BrigadeID = 0xFFFF;
	G->BrIndex = 0;
	G->AutoKill = false;
	G->StandGround = NM->AutoStandGround;
	G->NoSearchVictim = NM->AutoNoAttack;
#ifdef CONQUEST
	if ( NM->CompxCraft )
	{
		G->StageState = 32768;
	};
#endif
	if ( GO->newMons->Building && !NM->Wall )G->NewBuilding = true;
	else
	{
		if ( NM->Wall )G->Wall = true;
		else G->NewMonst = true;
	};



	//G->MadeSel=0;
	//G->MadeUnSel=0;
	G->ImSelected = 0;
	G->NewState = 0;
	G->LocalNewState = 0;
	G->newMons = GO->newMons;
	G->RealX = x;
	G->RealY = y;
	G->DestX = -100;
	G->DestY = -100;
	G->RealVx = 0;
	G->RealVy = 0;
	G->RealDir = rando() & 255;
	G->GraphDir = G->RealDir;
	G->Media = 0;
	G->Nat = this;
	G->Ready = true;
	G->delay = 0;
	G->MaxDelay = 0;
	G->NearBase = 0xFFFF;
	G->RStage = 0;
	G->RType = 0xFF;
	G->RAmount = 0;
	G->NNUM = NNUM;
	G->NIndex = n;
	G->Index = i;
	G->BackMotion = 0;
	Visuals* m;
	m = (Visuals*) Mon[n];
	G->Selected = false;
	G->Borg = false;
	G->Life = ADC->Life;
	G->MaxLife = ADC->Life;
	G->Ref.Visual = m;
	G->LocalOrder = nullptr;
	G->PrioryLevel = 0;
	G->Sdoxlo = false;
	G->NMask = NMask;
	G->Attack = false;
	G->EnemyID = 0xFFFF;
	G->Height = 0;
	G->SafeWall = false;
	G->NewAnm = &G->newMons->Stand;
	G->NewCurSprite = 0;
	G->InMotion = false;
	G->RotCntr = 0;
	G->BestNX = G->RealX;
	G->BestNY = G->RealY;
	G->NextForceX = 0;
	G->NextForceY = 0;
	G->LeftLeg = true;
	G->MotionDir = 1;
	G->Radius1 = GO->newMons->Radius1;
	G->Radius2 = GO->newMons->Radius2;
	G->Lx = mLx;
	G->Ly = mLx;
	G->x = ( G->RealX - ( G->Lx << 7 ) ) >> 8;
	G->y = ( G->RealY - ( G->Lx << 7 ) ) >> 8;
	G->BlockInfo = 0;
	G->AbRes = NM->ResConcentrator;
	G->Kills = 0;
	G->NZalp = NM->MaxZalp;
	AddObject( G );
	if ( !G->NewBuilding )
	{
		CITY->Account += NM->Ves;
	}

	if ( G->LockType == 1 && !G->NewBuilding )
	{
		int NewX = G->RealX;
		int NewY = G->RealY;
		int LX = G->Lx;
		int ULX = ( LX ) << 7;
		int NewLockX = ( NewX - ULX ) >> 8;
		int NewLockY = ( NewY - ULX ) >> 8;
		UnitsField.BSetBar( NewLockX, NewLockY, LX );
		G->LLock = true;
	}
	else
	{
		if ( !( G->NewBuilding || G->Wall ) )
		{
			UnitsField.BSetBar( G->x, G->y, G->Lx );
		}
	}

	NProduced[G->NIndex]++;
	G->RZ = GetHeight( x >> 4, y >> 4 );
	if ( G->NewBuilding && !NM->Wall )
	{
		G->LoLayer = &GO->newMons->PMotionL[0];
		G->HiLayer = &GO->newMons->PMotionR[0];
		G->Stage = 0;
		if ( NM->NBLockPt )
		{
			int x0 = ( x + PicSX ) >> 8;
			int y0 = ( y + PicSY ) >> 8;
			for ( int i = 0; i < NM->NBLockPt; i++ )
			{
				int xx = x0 + NM->BLockX[i];
				int yy = y0 + NM->BLockY[i];
				BSetPt( xx, yy );
				EraseTreesInPoint( xx, yy );
			}
		}
		else
		{
			if ( NM->NLockPt )
			{
				int x0 = ( x + PicSX ) >> 8;
				int y0 = ( y + PicSY ) >> 8;
				for ( int i = 0; i < NM->NLockPt; i++ )
				{
					int xx = x0 + NM->LockX[i];
					int yy = y0 + NM->LockY[i];
					BSetPt( xx, yy );
					EraseTreesInPoint( xx, yy );
				}
			}
		}

		G->Life = 2;
		G->Ready = false;
		if ( NM->ProdType )
		{
			G->TakeResourceFromSprite( ID );
		}
	}

	if ( NM->NBars )
	{
		int bx0 = ( G->RealX >> 4 ) + NM->PicDx;
		int by0 = ( G->RealY >> 4 ) + ( NM->PicDy << 1 );
		int N = NM->NBars;
		for ( int i = 0; i < N; i++ )
		{
			int pp = i * 5;
			int XB0 = NM->Bars3D[pp] + bx0;
			int YB0 = ( NM->Bars3D[pp + 1] << 1 ) + by0;
			int L1 = NM->Bars3D[pp + 2];
			int L2 = NM->Bars3D[pp + 3];
			Add3DBar( XB0, YB0, XB0 + L1 + L2, YB0 + L1 - L2, NM->Bars3D[pp + 4], G->Index, G->Index );
		}
	}

	NGidot++;
	if ( G->NewBuilding )
	{
		if ( NM->NHideTri )
		{
			SetTrianglesState( G->RealX >> 4, ( ( G->RealY - 4 ) >> 5 ) - G->RZ, NM->HideTriX, NM->HideTriY, NM->NHideTri, false );
		}
		RenewInfoMap( G->RealX >> 4, G->RealY >> 4 );
	}
	else
	{
		CITY->RegisterNewUnit( G );
	}

	if ( GroundBox )
	{
		CreateGround( G );
	}

	if ( G->NewBuilding && !Loadingmap )
	{
		CITY->EnumUnits();
		CITY->RefreshAbility();
	}

	return G->Index;
}

void CreateGround( OneObject* G )
{
	NewMonster* NM = G->newMons;
	if ( NM->Building )
	{
		byte Use = NM->Usage;
		if ( Use == MelnicaID || Use == SkladID || Use == MineID )
		{
			return;
		}

		G->InFire = 1;
		GeneralObject* GO = G->Ref.General;
		byte tex1 = NTex1[GO->NatID];
		byte tex2 = NTex2[GO->NatID];
		int CX = G->RealX >> 4;
		int CY = G->RealY >> 4;
		int X0 = CX + NM->BuildX0;
		int Y0 = CY + NM->BuildY0;
		int X1 = CX + NM->BuildX1;
		int Y1 = CY + NM->BuildY1;
		int D = ( Y1 - Y0 + X1 - X0 ) >> 1;
		int D2 = D;
		int DD1 = 40;
		int DD2 = 0;
		if ( !D )
		{
			DD1 = 80;
			DD2 = 40;
		}

		if ( tex2 != 0xFF )
		{
			NCurves = 0;
			NTextures = 1;
			TexList[0] = tex2;
			AddPointToCurve( X0 - 40, Y0, 0, 1 );
			AddPointToCurve( X0 + D, Y0 + D2 + 40, 0, 1 );
			AddPointToCurve( X1 + 40, Y1, 0, 1 );
			AddPointToCurve( X1 - D, Y1 - D2 - 40, 1, 1 );
		}

		if ( tex1 != 0xFF )
		{
			NCurves = 0;
			NTextures = 1;
			TexList[0] = tex1;
			AddPointToCurve( X0, Y0, 0, 1 );
			AddPointToCurve( X0 + D, Y0 + D2, 0, 1 );
			AddPointToCurve( X1, Y1, 0, 1 );
			AddPointToCurve( X1 - D, Y1 - D2, 1, 1 );
		}
	}
}

void CreateGrassGround( int x, int y )
{
	y += 32;
	NCurves = 0;
	NTextures = 2;
	TexList[0] = 7;
	TexList[1] = 11;
	AddPointToCurve( x - 100, y, 0, 1 );
	AddPointToCurve( x, y - 100, 0, 1 );
	AddPointToCurve( x + 100, y, 0, 1 );
	AddPointToCurve( x, y + 100, 1, 1 );
}

void OneObject::GetCornerXY( int* px, int* py )
{
	NewMonster* NM = newMons;
	int PicSX = NM->PicDx << 4;
	int PicSY = Prop43( NM->PicDy ) << 4;
	*px = ( RealX + PicSX ) >> 8;
	*py = ( RealY + PicSY ) >> 8;
}

void NewMonsterSendToLink( OneObject* OB );

bool ParkWaterNewMonster( OneObject* OB, int x, int y, byte Prio, byte OrdType );

#undef NewMonsterSendTo

void OneObject::NewMonsterSendTo( int px, int py, byte Prio, byte OrdType )
{
	NewMonster* NM = newMons;
	if ( Ref.General->OFCR && InArmy )
	{
		return;
	}

	if ( NM->Transport )
	{
		bool CTR = CheckTransportOnParking( this, px >> 8, py >> 8 );
		if ( ParkWaterNewMonster( this, px, py, Prio, OrdType ) )
		{
			return;
		}
	}

	if ( !NM->Transport )
	{
		if ( CheckOrderAbility() )
		{
			return;
		}
	}

	if ( NM->Building )
	{
		return;
	}

	if ( PrioryLevel > Prio )
	{
		return;
	}

	Order1* Or1 = CreateOrder( OrdType );
	if ( !int( Or1 ) )
	{
		return;
	}

	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 12;
	Or1->OrderTime = 0;
	Or1->DoLink = &NewMonsterSendToLink;
	Or1->info.MoveToXY.x = ( px - ( ( Lx ) << 7 ) ) >> 8;
	Or1->info.MoveToXY.y = ( py - ( ( Lx ) << 7 ) ) >> 8;
	Order1* LOR = LocalOrder;
	DestX = px;
	DestY = py;
	UnBlockUnit();
	PrioryLevel = Or1->PrioryLevel;
	MakeOrderSound( this, 2 );
	StandTime = 0;
	PathDelay = 0;
	if ( Or1->PrioryLevel == 0 )
	{
		InPatrol = true;
	}
}

#define NewMonsterSendTo(x,y,Prio,OrdType) NewMonsterSendTo(x,y,Prio,OrdType)

bool FindBestPosition( OneObject* OB, int* xd, int* yd, int R0 );

void NewMonsterSendToLink( OneObject* OB )
{
	if ( OB->PathDelay&&OB->StandTime > 64 )
	{
		if ( int( OB->LocalOrder ) )
		{
			OB->DeleteLastOrder();
			return;
		}
	}

	OB->PrioryLevel = OB->LocalOrder->PrioryLevel;
	OB->NewState = OB->GroundState;
	int xx1 = OB->LocalOrder->info.MoveToXY.x;
	int yy1 = OB->LocalOrder->info.MoveToXY.y;
	int Lx = OB->Lx;

	if ( ( !OB->UnlimitedMotion ) && CheckBar( xx1 - 1, yy1 - 1, Lx + 2, Lx + 2 ) )
	{
		if ( FindBestPosition( OB, &xx1, &yy1, 80 ) )
		{
			OB->LocalOrder->info.MoveToXY.x = xx1;
			OB->LocalOrder->info.MoveToXY.y = yy1;
		}
	}

	if ( OB->DistTo( xx1, yy1 ) <= 1 )
	{
		if ( int( OB->LocalOrder ) )
		{
			OB->DeleteLastOrder();
		}
	}
	else
	{
		OB->CreatePath( xx1, yy1 );
	}
}

void NewMonsterPreciseSendToLink( OneObject* OB );

#undef NewMonsterPreciseSendTo

void OneObject::NewMonsterPreciseSendTo( int px, int py, byte Prio, byte OrdType )
{
	NewMonster* NM = newMons;
	if ( NM->Transport )
	{
		bool CTR = CheckTransportOnParking( this, px >> 8, py >> 8 );
		if ( ParkWaterNewMonster( this, px, y, Prio, OrdType ) )
		{
			return;
		}
	}
	if ( !NM->Transport )
	{
		if ( CheckOrderAbility() )return;
	}
	if ( NM->Building )
	{
		return;
	}
	if ( PrioryLevel > Prio )
	{
		return;
	}
	Order1* Or1 = CreateOrder( OrdType );
	if ( !int( Or1 ) )
	{
		return;
	}
	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 12;
	Or1->OrderTime = 0;
	Or1->DoLink = &NewMonsterPreciseSendToLink;
	Or1->info.MoveToXY.x = px;
	Or1->info.MoveToXY.y = py;

	Order1* LOR = LocalOrder;
	DestX = px;
	DestY = py;
	UnBlockUnit();
	MakeOrderSound( this, 2 );
	StandTime = 0;
	PathDelay = 0;
}

#define NewMonsterPreciseSendTo(x,y,Prio,OrdType) NewMonsterPreciseSendTo(x,y,Prio,OrdType)

void NewMonsterPreciseSendToLink( OneObject* OB )
{
	if ( OB->PathDelay&&OB->StandTime > 64 )
	{
		if ( int( OB->LocalOrder ) )
		{
			OB->DeleteLastOrder();
			return;
		}
	}
	OB->PrioryLevel = OB->LocalOrder->PrioryLevel;
	OB->NewState = OB->GroundState;
	int xx2 = OB->LocalOrder->info.MoveToXY.x;
	int yy2 = OB->LocalOrder->info.MoveToXY.y;
	int Lx = OB->Lx;
	int xx1 = ( xx2 - ( Lx << 7 ) ) >> 8;
	int yy1 = ( yy2 - ( Lx << 7 ) ) >> 8;

	int dr = Norma( xx2 - OB->RealX, yy2 - OB->RealY );
	if ( dr < 1024 )
	{
		OB->DestX = xx2;
		OB->DestY = yy2;
	};
	if ( dr < 96 )
	{
		if ( xx1 != OB->x || yy1 != OB->y )
		{
			UnitsField.BClrBar( OB->x, OB->y, Lx );
			if ( !UnitsField.CheckBar( xx1, yy1, Lx, Lx ) )
			{
				OB->x = xx1;
				OB->y = yy1;
				UnitsField.BSetBar( xx1, yy1, Lx );
				TryToStand( OB, false );
				OB->DeleteLastOrder();
				OB->DestX = -1;
				return;
			};
		}
		else
			if ( int( OB->LocalOrder ) )
			{
				OB->DestX = -1;
				OB->DeleteLastOrder();
			};
	}
	else
	{
		if ( dr >= 1024 )OB->CreatePath( xx1, yy1 );
		if ( dr < 768 && OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec&&UnitsField.CheckBar( xx1, yy1, Lx, Lx ) )
		{
			TryToStand( OB, false );
			OB->DestX = -1;
			OB->DeleteLastOrder();
		};
	};
}

void ProcessNewMonsters()
{
	SetupSpot();
	SetMonstersInCells();
	CalculateMotion();
	return;
}

typedef  bool CHOBJ( OneObject* OB, int N );
void GetRect( OneObject* ZZ, int* x, int* y, int* Lx, int* Ly );
bool PInside( int x, int y, int x1, int y1, int xp, int yp );
word CheckCoorInGP( int x, int y );
word GoodSelectNewMonsters( byte NI, int xr, int yr, int xr1, int yr1, word *Collect, word* Ser, bool WRITE, CHOBJ* FN, int NN, int MAX )
{
	byte NIX = NatRefTBL[NI];
	int NSLC = 0;
	if ( abs( xr - xr1 ) < 5 && abs( yr - yr1 ) < 5 )
	{
		int xx = xr - ( mapx << 5 );
		int yy = yr - ( mapy << 4 );
		word IDX = CheckCoorInGP( xx, yy );
		if ( IDX != 0xFFFF )
		{
			OneObject* OB = Group[IDX];
			if ( OB&&OB->newMons && ( !( ( OB->ImSelected&GM( NI ) ) || OB->Hidden || OB->Sdoxlo || OB->UnlimitedMotion ) ) && OB->NNUM == NIX && ( ( !FN ) || FN( OB, NN ) ) )
			{
				if ( WRITE )
				{
					Collect[NSLC] = OB->Index;
					Ser[NSLC] = OB->Serial;
					OB->ImSelected |= GM( NI );
				};
				NSLC++;
			};
		};
		return NSLC;
	};

	int x, y, x1p, y1p, x1, y1;
	int xp = xr >> 5;
	int yp = div24( yr );
	int xp1 = xr1 >> 5;
	int yp1 = div24( yr1 );
	if ( xp > xp1 )
	{
		x = xp1;
		x1 = xp;
	}
	else
	{
		x1 = xp1;
		x = xp;
	};
	if ( yp > yp1 )
	{
		y = yp1;
		y1 = yp;
	}
	else
	{
		y1 = yp1;
		y = yp;
	};
	x1p = x1;
	y1p = y1;
	int mx = ( msx >> 2 ) - 1;
	int my = ( msy >> 2 ) - 1;
	int x0 = ( x >> 2 ) - 1;
	if ( x0 < 0 )x0 = 0;
	int y0 = ( y >> 2 ) - 1;
	if ( y0 < 0 )y0 = 0;
	if ( x1 & 3 )x1 = ( x1 >> 2 ) + 2;
	else x1 = ( x1 >> 2 ) + 1;
	if ( y1 & 3 )y1 = ( y1 >> 2 ) + 2;
	else y1 = ( y1 >> 2 ) + 1;
	if ( x1 > mx )x1 = mx;
	if ( y1 > my )y1 = my;
	x = xr;
	y = yr;
	x1p = xr1;
	y1p = yr1;
	bool UPresent = false;
	for ( int ii = 0; ii < MAXOBJECT; ii++ )
	{
		OneObject* OB = Group[ii];
		if ( OB&&OB->newMons && ( !( ( OB->ImSelected&GM( NI ) ) || OB->Hidden || OB->Sdoxlo || OB->UnlimitedMotion ) ) && OB->NNUM == NIX && ( ( !FN ) || FN( OB, NN ) ) )
		{
			int xs, ys, xs1, ys1, LY;
			GetRect( OB, &xs, &ys, &xs1, &LY );
			ys1 = ys + LY - 1;
			xs1 += xs - 1;
			if ( PInside( x, y, x1p, y1p, xs, ys ) ||
				PInside( x, y, x1p, y1p, xs1, ys ) ||
				PInside( x, y, x1p, y1p, xs, ys1 ) ||
				PInside( x, y, x1p, y1p, xs1, ys1 ) ||
				PInside( xs, ys, xs1, ys1, x, y ) ||
				PInside( xs, ys, xs1, ys1, x1p, y ) ||
				PInside( xs, ys, xs1, ys1, x, y1p ) ||
				PInside( xs, ys, xs1, ys1, x1p, y1p ) )
			{
				if ( NSLC < MAX )
				{
					if ( WRITE )
					{
						Collect[NSLC] = ii;
						Ser[NSLC] = OB->Serial;
						OB->ImSelected |= GM( NI );
					};
					NSLC++;
					if ( !( OB->NewBuilding || OB->Wall ) )UPresent = true;
				}
				else assert( 0 );
			};
		};
	};
	if ( UPresent&&WRITE )
	{
		for ( int i = 0; i < NSLC; i++ )
		{
			OneObject* OB = Group[Collect[i]];
			if ( OB && ( OB->NewBuilding || OB->Wall ) )
			{
				Collect[i] = 0xFFFF;
				OB->ImSelected &= ~GM( NI );
			};
		};
	};
	return NSLC;
};
void DrawMarker( OneObject* OB );
void DrawMiniMarker( OneObject* OB );
void ShowNMHealth( OneObject* OB )
{
	if ( MiniMode )DrawMiniMarker( OB );
	else DrawMarker( OB );
};

const int Power2[24] =
{
		0x00000001,0x00000003,0x00000007,0x0000000F,
		0x0000001F,0x0000003F,0x0000007F,0x000000FF,
		0x000001FF,0x000003FF,0x000007FF,0x00000FFF,
		0x00001FFF,0x00003FFF,0x00007FFF,0x0000FFFF,
		0x0001FFFF,0x0003FFFF,0x0007FFFF,0x000FFFFF,
		0x001FFFFF,0x003FFFFF,0x007FFFFF,0x00FFFFFF
};

void ClearMaps()
{
	MFIELDS->ClearMaps();
	UnitsField.ClearMaps();
}

void BSetPt( int x, int y )
{
	MFIELDS->BSetPt( x, y );
};
void BClrPt( int x, int y )
{
	MFIELDS->BClrPt( x, y );
};

void BSetBar( int x, int y, int Lx )
{
	MFIELDS->BSetBar( x, y, Lx );
};
void BClrBar( int x, int y, int Lx )
{
	MFIELDS->BClrBar( x, y, Lx );
};
void BSetSQ( int x, int y, int Lx, int Ly )
{
	MFIELDS->BSetSQ( x, y, Lx, Ly );
};
void BClrSQ( int x, int y, int Lx, int Ly )
{
	MFIELDS->BClrSQ( x, y, Lx, Ly );
};
int CheckPt( int x, int y )
{
	return MFIELDS->CheckPt( x, y );
};
int CheckHLine( int x, int y, int Lx )
{
	return MFIELDS->CheckHLine( x, y, Lx );
};
int CheckVLine( int x, int y, int Lx )
{
	return MFIELDS->CheckVLine( x, y, Lx );
}

bool CheckBar( int x, int y, int Lx, int Ly )
{
	return MFIELDS->CheckBar( x, y, Lx, Ly );
}

byte NewCirc[16];

void SetLock( int x, int y, char val )
{
	if ( val )
	{
		MFIELDS->BSetBar( x << 1, y << 1, 2 );
	}
	else
	{
		MFIELDS->BClrBar( x << 1, y << 1, 2 );
	}
}

//------------------------------------------------//
//          KERNEL OF THE MOTION ENGINE           //
//            Search for the best way             //
//------------------------------------------------//
#define MaxP (4096)

word pxx[MaxP + 160];
word pyy[MaxP + 160];
word RVIS[MaxP + 160];
word LVIS[MaxP + 160];

int GetLAngle( int dx, int dy, int Angle );
int GetRAngle( int dx, int dy, int Angle );

//divide by 16
int div24( int y )
{
	return y / 16;
}

//multiply by 2
int Prop43( int y )
{
	return y * 2;
}

void CBar( int x, int y, int Lx, int Ly, byte c );
extern byte LockGrid;
extern bool VHMode;
void TopShow();

void LShow()
{
	if ( !LockGrid )
	{
		return;
	}

	int ox = mapx << 5;
	int oy = mapy << 5;
	MotionField* MFI;

	if ( LockGrid == 1 )
		MFI = &UnitsField;
	else
		MFI = &MFIELDS[LockGrid - 2];

	for ( int ix = 0; ix < smaplx << 1; ix++ )
	{
		int iy = 0;
		bool nydone = true;
		do
		{
			int xx0 = mapx + mapx + ix;
			int yy0 = mapy + mapy + iy;
			bool res;
			if ( VHMode )res = MFI->CheckHLine( xx0, yy0, 1 ) != 0;
			else res = MFI->CheckVLine( xx0, yy0, 1 ) != 0;
			int yy = smapy + ( mul3( iy ) << 2 );
			int yy1 = yy;

			if ( Mode3D )
			{
				yy1 -= GetUnitHeight( ox + ( ix << 4 ), oy + ( iy << 4 ) );
			}

			if ( res )
			{
				int dz1 = 0;
				int dz2 = 0;
				int dz3 = 0;
				int dz4 = 0;

				if ( Mode3D )
				{
					dz1 = GetUnitHeight( ox + ( ix << 4 ), oy + ( iy << 4 ) );
					dz2 = GetUnitHeight( ox + ( ix << 4 ) + 16, oy + ( iy << 4 ) );
					dz3 = GetUnitHeight( ox + ( ix << 4 ), oy + ( iy << 4 ) + 16 );
					dz4 = GetUnitHeight( ox + ( ix << 4 ) + 16, oy + ( iy << 4 ) + 16 );
				}

				int xx = smapx + ( ix << 4 );
				DrawLine( xx, yy - dz1, xx + 16, yy + 8 - dz4, 254 );
				DrawLine( xx, yy + 8 - dz3, xx + 16, yy - dz2, 254 );

				if ( !MFI->CheckPt( xx0 + 1, yy0 ) )
				{
					DrawLine( xx + 16, yy - dz2, xx + 16, yy + 8 - dz4, 255 );
				}

				if ( !MFI->CheckPt( xx0 - 1, yy0 ) )
				{
					DrawLine( xx, yy - dz1, xx, yy + 8 - dz3, 255 );
				}

				if ( !MFI->CheckPt( xx0, yy0 - 1 ) )
				{
					DrawLine( xx, yy - dz1, xx + 16, yy - dz2, 255 );
				}

				if ( !MFI->CheckPt( xx0, yy0 + 1 ) )
				{
					DrawLine( xx, yy + 8 - dz3, xx + 16, yy + 8 - dz4, 255 );
				}
			}

			if ( yy1 > WindY1 )
			{
				nydone = false;
			}

			iy++;
		} while ( nydone );
	}
	TopShow();
}

void TopShow()
{
	if ( LockGrid != 1 )
	{
		return;
	}

	int ox = mapx << 5;
	int oy = mapy << 5;
	int nx = ( smaplx >> 1 ) + 1;
	int stx = mapx >> 1;
	int sty = mapy >> 1;
	for ( int ix = 0; ix < nx; ix++ )
	{
		int iy = 0;
		bool nydone = true;
		do
		{
			int xx0 = stx + ix;
			int yy0 = sty + iy;
			int yy = smapy + ( mul3( iy ) << 4 );
			int yy1 = yy;
			if ( Mode3D )
			{
				yy1 -= GetHeight( ox + ( ix << 6 ), oy + ( iy << 6 ) );
			}

			int top = TopRef[xx0 + yy0*TopLx];
			if ( top < 0xFFFE )
			{
				int dz1 = 0;
				int dz2 = 0;
				int dz3 = 0;
				int dz4 = 0;
				int tc = 0xD0 + ( top & 31 );

				if ( Mode3D )
				{
					dz1 = GetHeight( ox + ( ix << 6 ), oy + ( iy << 6 ) );
					dz2 = GetHeight( ox + ( ix << 6 ) + 64, oy + ( iy << 6 ) );
					dz3 = GetHeight( ox + ( ix << 6 ), oy + ( iy << 6 ) + 64 );
					dz4 = GetHeight( ox + ( ix << 6 ) + 64, oy + ( iy << 6 ) + 64 );
				}

				int xx = smapx + ( ix << 6 );
				DrawLine( xx, yy - dz1, xx + 64, yy + 32 - dz4, tc );
				DrawLine( xx, yy + 32 - dz3, xx + 64, yy - dz2, tc );
				if ( xx0 % 3 == 1 && yy0 % 3 == 1 )
				{
					bool TCL = true;
					for ( int dx = -1; dx < 2; dx++ )
						for ( int dy = -1; dy < 2; dy++ )
						{
							if ( SafeTopRef( xx0 + dx, yy0 + dy ) >= 0xFFFE )TCL = false;
						}
					if ( TCL )
					{
						DrawLine( xx + 1, yy - dz1, xx + 64, yy + 32 - dz4 - 1, tc );
						DrawLine( xx + 1, yy + 32 - dz3, xx + 64, yy - dz2 + 1, tc );
						DrawLine( xx, yy - dz1 + 1, xx + 64 - 1, yy + 32 - dz4, tc );
						DrawLine( xx, yy + 32 - dz3 - 1, xx + 64 - 1, yy - dz2, tc );
					}
				}
				if ( TopRef[xx0 + 1 + yy0*TopLx] != top )DrawLine( xx + 63, yy - dz2, xx + 63, yy + 32 - dz4, 255 );
				if ( xx0 > 0 && TopRef[xx0 - 1 + yy0*TopLx] != top )DrawLine( xx, yy - dz1, xx, yy + 32 - dz3, 255 );
				if ( TopRef[xx0 + yy0*TopLx + TopLx] != top )DrawLine( xx, yy + 31 - dz3, xx + 63, yy + 31 - dz4, 255 );
				if ( yy0 > 0 && TopRef[xx0 + yy0*TopLx - TopLx] != top )DrawLine( xx, yy - dz1, xx + 63, yy - dz2, 255 );
			}

			if ( yy1 > WindY1 )
			{
				nydone = false;
			}

			iy++;
		} while ( nydone );
	}
}

//------------------ATTACK POINT---------------------//
//it is used for the mortira,pushka(AI)
#define ATTP_ALLOWMOTION	1
#define ATTP_FINDPLACE		2
#define ATTP_IMMEDIATECHECK 4
#define ATTP_SINGLESHOT     8
#define ATTP_IMMEDIATEFINDPLACE 16
//return values:
//>0-ready
//-1-Too far or too near
//-2-prepiatsvia
int PredictShot( Weapon* Weap, int xs, int ys, int zs, int xd, int yd, int zd, word Index );
int CheckDamageAbility( OneObject* OB, int x, int y, int z, byte Nation, int Soft )
{
	//1.Let us determine weapon
	int ox = OB->RealX >> 4;
	int oy = OB->RealY >> 4;
	int oz = OB->RZ + OB->newMons->SrcZPoint;
	int dst = Norma( x - ox, y - oy );
	if ( Soft )dst -= 48;
	int dstx = dst + ( ( z - oz ) << 1 );
	if ( dstx < 0 )dstx = 0;
	int NeedState = -1;
	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	bool NWATER = !OB->LockType;
	int DRMAX = 0;
	if ( Soft == 3 )DRMAX = 280;
	for ( int i = 0; i < NAttTypes; i++ )
	{
		int wpk = NMN->WeaponKind[i];
		if ( NWATER&&WeaponFlags[wpk] & 1 )
		{
			if ( dst > ADC->AttackRadius1[i] && dstx < ADC->AttackRadius2[i] )
			{
				NeedState = i;
			};
		}
		else
		{
			int R2 = ADC->AttackRadius2[i];
			if ( R2 )R2 += DRMAX;
			if ( dst >= ADC->AttackRadius1[i] && dst <= R2 )
			{
				NeedState = i;
			};
		};
	};
	if ( NeedState == -1 )return -1;
	Weapon* WP = NMN->DamWeap[NeedState];
	if ( !WP )return -1;
	int res = PredictShot( WP, ox, oy, oz, x, y, z, OB->Index );
	if ( res == -1 )return NeedState;
	if ( res >= 8192 )return -2;
	OneObject* OBJ = Group[res];
	if ( OBJ&&OBJ->NNUM != Nation )return NeedState;
	return -2;

};
int FastCheckDamageAbility( OneObject* OB, int x, int y, int z, byte Nation, int Soft )
{
	//1.Let us determine weapon
	int ox = OB->RealX >> 4;
	int oy = OB->RealY >> 4;
	int oz = OB->RZ + OB->newMons->SrcZPoint;
	int dst = Norma( x - ox, y - oy );
	if ( Soft )dst -= 48;
	int dstx = dst + ( ( z - oz ) << 1 );
	if ( dstx < 0 )dstx = 0;
	int NeedState = -1;
	NewMonster* NMN = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	bool NWATER = !OB->LockType;
	int DRMAX = 0;
	if ( Soft == 3 )DRMAX = 280;
	for ( int i = 0; i < NAttTypes; i++ )
	{
		int wpk = NMN->WeaponKind[i];
		if ( NWATER&&WeaponFlags[wpk] & 1 )
		{
			if ( dst > ADC->AttackRadius1[i] && dstx < ADC->AttackRadius2[i] )
			{
				NeedState = i;
			};
		}
		else
		{
			int R2 = ADC->AttackRadius2[i];
			if ( R2 )R2 += DRMAX;
			if ( dst >= ADC->AttackRadius1[i] && dst <= R2 )
			{
				NeedState = i;
			};
		};
	};
	if ( NeedState == -1 )return -1;
	Weapon* WP = NMN->DamWeap[NeedState];
	if ( !WP )return -1;
	int res = PredictShot( WP, ox, oy, oz, x, y, z, OB->Index );
	if ( res == -1 )return NeedState;
	if ( res >= 8192 )return -2;
	OneObject* OBJ = Group[res];
	if ( OBJ&&OBJ->NNUM != Nation )return NeedState;
	return -2;

};
int FindPlaceForAttack( OneObject* OB, int x, int y, int z, byte Nation,
	int MaxSteps, int SearchMethod, int* Newx, int* Newy )
{
	int OldRX = OB->RealX;
	int OldRY = OB->RealY;
	int myx = OldRX >> 4;
	int myy = OldRY >> 4;
	int DX = x - myx;
	int DY = y - myy;
	int N = Norma( DX, DY );
	if ( !( DX || DY ) )return 1;
	DX = ( DX << 5 ) / N;
	DY = ( DY << 5 ) / N;
	DX += ( rando() & 15 ) - 8;
	DY += ( rando() & 15 ) - 8;
	if ( SearchMethod == -1 )
	{
		//move forward
		myx += DX;
		myy += DY;
		MaxSteps <<= 1;
		for ( int i = 0; i < MaxSteps; i++ )
		{
			if ( CheckPt( myx >> 4, myy >> 4 ) )
			{
				OB->RealX = OldRX;
				OB->RealY = OldRY;
				return SearchMethod;
			}
			else
			{
				OB->RealX = ( myx << 4 );
				OB->RealY = ( myy << 4 );
				int r = CheckDamageAbility( OB, x, y, z, Nation, false );
				if ( !r )
				{
					OB->RealX = OldRX;
					OB->RealY = OldRY;
					*Newx = myx;
					*Newy = myy;
					return 0;
				};
			};
			myx += DX;
			myy += DY;
		};
	}
	else
	{
		//strafe
		int lfx = myx + DY;
		int lfy = myy - DX;
		int rfx = myx - DY;
		int rfy = myy + DX;
		bool LMove = true;
		bool RMove = true;
		for ( int i = 0; i < MaxSteps && ( LMove || RMove ); i++ )
		{
			if ( LMove )
			{
				if ( !CheckPt( lfx >> 4, lfy >> 4 ) )
				{
					OB->RealX = lfx << 4;
					OB->RealY = lfy << 4;
					int r = CheckDamageAbility( OB, x, y, z, Nation, false );
					if ( !r )
					{
						OB->RealX = OldRX;
						OB->RealY = OldRY;
						*Newx = lfx;
						*Newy = lfy;
						return 0;
					};
				}
				else LMove = false;
				lfx += DY;
				lfy -= DX;
			};
			if ( RMove )
			{
				if ( !CheckPt( rfx >> 4, rfy >> 4 ) )
				{
					OB->RealX = rfx << 4;
					OB->RealY = rfy << 4;
					int r = CheckDamageAbility( OB, x, y, z, Nation, false );
					if ( !r )
					{
						OB->RealX = OldRX;
						OB->RealY = OldRY;
						*Newx = rfx;
						*Newy = rfy;
						return 0;
					};
				}
				else RMove = false;
				rfx += DY;
				rfy -= DX;
			};
		};
	};
	OB->RealX = OldRX;
	OB->RealY = OldRY;
	return SearchMethod;
};
//only for pushka and mortira
void CreateRazbros( OneObject* OBJ, Coor3D* C3D );
bool MoveAwayInSector( int x, int y, int r0, int r1, char Dir, int ddir );

void AI_AttackPointLink( OneObject* OBJ )
{
	if ( OBJ->delay )
	{
		return;
	}
	int dax = OBJ->LocalOrder->info.AttackXY.ox;
	int day = OBJ->LocalOrder->info.AttackXY.oy;
	int x = OBJ->LocalOrder->info.AttackXY.x;
	int y = OBJ->LocalOrder->info.AttackXY.y;
	int z = OBJ->LocalOrder->info.AttackXY.z;
	int myx = OBJ->RealX >> 4;
	int myy = OBJ->RealY >> 4;
	int RRot = OBJ->newMons->MinRotator;
	int rot2 = RRot << 1;
	if ( rot2 > 16 )
	{
		rot2 = 16;
	}
	if ( Norma( dax - myx, day - myy ) < 32 )
	{
		//rotation
		char Ndir = char( GetDir( x - myx, y - myy ) );
		char ddir = Ndir - OBJ->RealDir;
		if ( abs( ddir ) < rot2 )
		{
			//can make shot now
			//1.let us determine method of attack
			int r = CheckDamageAbility( OBJ, x, y, z, OBJ->NNUM, true );
			if ( r < 0 )
			{
				OBJ->DeleteLastOrder();
				return;
			}
			NewMonster* NMN = OBJ->newMons;
			if ( NMN->Usage != SupMortID&&NMN->Usage != MortiraID )
			{
				if ( MoveAwayInSector( OBJ->RealX, OBJ->RealY, 16 * 16, 200 * 16, OBJ->RealDir, 33 ) )return;
			}
			//2.checking of resources
			bool AllowShot = true;
			AdvCharacter* ADC = OBJ->Ref.General->MoreCharacter;
			if ( NMN->NShotRes )
			{
				for ( int k = 0; k < NMN->NShotRes; k++ )
				{
					if ( XRESRC( OBJ->NNUM, NMN->ShotRes[k + k] ) < NMN->ShotRes[k + k + 1] )
					{
						AllowShot = false;
					}
				}
				if ( AllowShot )
				{
					for ( int k = 0; k < NMN->NShotRes; k++ )
					{
						AddXRESRC( OBJ->NNUM, NMN->ShotRes[k + k], -NMN->ShotRes[k + k + 1] );
						OBJ->Nat->AddResource( byte( NMN->ShotRes[k + k] ), -NMN->ShotRes[k + k + 1] );
						OBJ->Nat->ResOnLife[NMN->ShotRes[k + k]] += NMN->ShotRes[k + k + 1];
					}
				}
			}
			//3.source point
			OBJ->delay = ADC->AttackPause[r];
			OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
			OBJ->delay = OBJ->MaxDelay;

			Weapon* WP = NMN->DamWeap[r];
			//pubuxxxxxxxxx...
			if ( WP )
			{
				//----------
				NewAnimation* UNAM = NMN->Attack + r;
				int uocts = ( UNAM->Rotations - 1 ) * 2;
				int uoc2 = UNAM->Rotations - 1;
				if ( !uocts )uocts = 1;
				int usesize = div( 255, uocts << 1 ).quot;
				byte udir = ( ( ( OBJ->RealDir + 64 + usesize ) & 255 )*uocts ) >> 8;
				NewFrame* NF = &UNAM->Frames[0];
				//----------
				NewAnimation* NAM = WP->NewAnm;
				int octs = ( NAM->Rotations - 1 ) * 2;
				int oc2 = NAM->Rotations - 1;
				if ( !octs )octs = 1;
				int sesize = div( 255, octs << 1 ).quot;
				byte dir = ( ( ( OBJ->RealDir + 64 + sesize ) & 255 )*octs ) >> 8;
				int x0, y0, z0, x1, y1, z1;
				if ( udir < uoc2 )
				{
					int udir1 = uoc2 - udir;
					x0 = ( OBJ->RealX >> 4 ) - ( UNAM->ActivePtX[udir1] + NF->dx );
					y0 = ( OBJ->RealY >> 4 ) + Prop43( UNAM->ActivePtY[udir1] + NF->dy + NMN->SrcZPoint );
				}
				else
				{
					int udir1 = uoc2 - uocts + udir;
					x0 = ( OBJ->RealX >> 4 ) + ( UNAM->ActivePtX[udir1] + NF->dx );
					y0 = ( OBJ->RealY >> 4 ) + Prop43( UNAM->ActivePtY[udir1] + NF->dy + NMN->SrcZPoint );
				}
				Coor3D C3D;
				C3D.x = x;
				C3D.y = y;
				C3D.z = z;
				CreateRazbros( OBJ, &C3D );
				z0 = OBJ->RZ + NMN->SrcZPoint;
				if ( udir < uoc2 )
				{
					udir = uoc2 - udir;
					x1 = C3D.x;
					y1 = C3D.y;
					z1 = C3D.z + NMN->DstZPoint;
					Create3DAnmObject( WP, x0, y0, z0, x1, y1, z1, OBJ, OBJ->NewState - 1, 0xFFFF );
				}
				else
				{
					udir = uoc2 - uocts + udir;
					x1 = C3D.x;
					y1 = C3D.y;
					z1 = C3D.z + NMN->DstZPoint;
					Create3DAnmObject( WP, x0, y0, z0, x1, y1, z1, OBJ, OBJ->NewState - 1, 0xFFFF );
				}
			}
			if ( OBJ->LocalOrder->info.AttackXY.wep != 255 )
			{
				OBJ->DeleteLastOrder();
			}
			return;
		}
		else
		{
			//Need to rotate
			if ( ddir > 0 )OBJ->RealDir += RRot;
			else OBJ->RealDir -= RRot;
			OBJ->GraphDir = OBJ->RealDir;
			OBJ->DeletePath();
			OBJ->DestX = -1;
			OBJ->DestY = -1;
		}
	}
	else
	{
		OBJ->CreatePath( dax >> 4, day >> 4 );
	}
}

bool OneObject::AttackPoint( int px, int py, int z, byte Times, byte Flags, byte OrdType )
{
	int ImmCanDam = CheckDamageAbility( this, px, py, z, NNUM, false );
	if ( ( delay&&Times != 255 ) || LocalOrder&&LocalOrder->DoLink == &AI_AttackPointLink )
	{
		return false;
	}
	if ( ( Flags&ATTP_IMMEDIATECHECK ) && ImmCanDam < 0 )
	{
		return false;
	}
	int r = 0;
	int Newx = -1;
	int Newy = -1;
	if ( ImmCanDam < 0 )
	{
		r = FindPlaceForAttack( this, px, py, z, NNUM, 30, ImmCanDam, &Newx, &Newy );
	}
	else
	{
		Newx = RealX >> 4;
		Newy = RealY >> 4;
	}
	if ( ( ImmCanDam < 0 ) && ( Flags&ATTP_IMMEDIATEFINDPLACE ) && r )
	{
		return false;
	}
	Order1* OR1 = CreateOrder( OrdType );
	if ( !OR1 )
	{
		return false;
	}
	OR1->info.AttackXY.ox = Newx;
	OR1->info.AttackXY.oy = Newy;
	OR1->DoLink = &AI_AttackPointLink;
	OR1->OrderType = 177;
	OR1->OrderTime = 0;
	OR1->info.AttackXY.x = px;
	OR1->info.AttackXY.y = py;
	OR1->info.AttackXY.z = z;
	OR1->info.AttackXY.wep = Times;
	return true;
}

//Special for pushek
int GetEnemyDifference( int cell, byte Mask )
{
	int NHim = 0;
	int NMy = 0;
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )
	{
		return 0;
	}
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				if ( OB->NMask&Mask )NMy++;
				else NHim++;
			}
		}
	}
	if ( NMy > 5 || NHim < 3 || NHim <= NMy )
	{
		return -1;
	}
	return NHim - NMy;
}

void SearchEnemyForAIArtillery( OneObject* OB )
{
	if ( OB->delay )
	{
		return;
	}
	int cell = ( ( OB->RealY >> 11 ) << VAL_SHFCX ) + ( OB->RealX >> 11 );
	NewMonster* NM = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	int rr = ADC->MaxR_Attack;
	int rx0 = ( OB->RealX >> 4 ) + 64;
	int ry0 = ( OB->RealY >> 4 ) + 64;
	int x0 = OB->RealX >> 11;
	int y0 = OB->RealY >> 11;
	EnemyInfo* EIN = GNFO.EINF[OB->NNUM];
	//search for towers
	DangerInfo* DF = EIN->DINF;
	int xxx = OB->RealX >> 4;
	int yyy = OB->RealY >> 4;
	for ( int p = 0; p < EIN->NDINF; p++ )
	{
		if ( Norma( DF->x - xxx, DF->y - yyy ) < rr )
		{
			OB->AttackPoint( DF->x, DF->y, DF->z + 32, 1, ATTP_IMMEDIATEFINDPLACE, 1 );
			if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &AI_AttackPointLink )
			{
				return;
			}
		}
		DF++;
	}
	//-----------------
	int rx1 = ( rr >> 7 ) + 1;
	int EnX = -1;
	int EnY = -1;
	int mindist = 10000000;
	int rx2 = rx1 + rx1 + 1;
	int stcell;
	int MaxDif = -1;
	for ( int i = 0; i < 64; i++ )
	{
		int dx = ( ( rando()*rx2 ) >> 15 ) - rx1;
		int dy = ( ( rando()*rx2 ) >> 15 ) - rx1;
		int x1 = rx0 + ( dx << 7 );
		int y1 = ry0 + ( dy << 7 );
		if ( Norma( x1 - rx0, y1 - ry0 ) < rr )
		{
			stcell = cell + dx + ( dy << VAL_SHFCX );
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( NPresence[stcell] )
				{
					int N = GetEnemyDifference( stcell, OB->NMask );
					if ( N > MaxDif )
					{
						MaxDif = N;
						EnX = x1;
						EnY = y1;
					}
				}
			}
		}
	}
	if ( MaxDif > 5 )
	{
		OB->AttackPoint( EnX, EnY, GetHeight( EnX, EnY ) + 32, 1, ATTP_IMMEDIATEFINDPLACE, 1 );
	}
	else
	{
		//search of wall to erase
		if ( rando() < 4096 )
		{
			rr -= 510;
			for ( int p = 0; p < EIN->NEnWalls; p++ )
			{
				int xx1 = ( int( EIN->WallsX[p] ) << 6 ) + 32;
				int yy1 = ( int( EIN->WallsY[p] ) << 6 ) + 32;
				if ( Norma( xx1 - xxx, yy1 - yyy ) < rr )
				{
					OB->AttackPoint( xx1, yy1, GetHeight( xx1, yy1 ) + 5, 1, ATTP_IMMEDIATEFINDPLACE, 1 );
					if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &AI_AttackPointLink )
					{
						return;
					}
				}
				DF++;
			}
		}
	}
}

void SearchEnemyForMortira( OneObject* OB )
{
	if ( OB->delay )
	{
		return;
	}
	int cell = ( ( OB->RealY >> 11 ) << VAL_SHFCX ) + ( OB->RealX >> 11 );
	NewMonster* NM = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	int rr = ADC->MaxR_Attack + 400;
	int rx0 = ( OB->RealX >> 4 ) + 64;
	int ry0 = ( OB->RealY >> 4 ) + 64;
	int x0 = OB->RealX >> 11;
	int y0 = OB->RealY >> 11;
	int xxx = OB->RealX >> 4;
	int yyy = OB->RealY >> 4;
	EnemyInfo* EIN = GNFO.EINF[OB->NNUM];
	if ( rando() < 8192 )
	{
		rr -= 510;
		for ( int p = 0; p < EIN->NEnWalls; p++ )
		{
			int xx1 = ( int( EIN->WallsX[p] ) << 6 ) + 32;
			int yy1 = ( int( EIN->WallsY[p] ) << 6 ) + 32;
			if ( Norma( xx1 - xxx, yy1 - yyy ) < rr )
			{
				OB->AttackPoint( xx1, yy1, GetHeight( xx1, yy1 ) + 5, 1, ATTP_IMMEDIATEFINDPLACE, 1 );
				if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &AI_AttackPointLink )return;
			};
		};
	};
	rr -= 390;
	//search for towers
	DangerInfo* DF = EIN->DINF;
	for ( int p = 0; p < EIN->NDINF; p++ )
	{
		if ( Norma( DF->x - xxx, DF->y - yyy ) < rr )
		{
			OB->AttackPoint( DF->x, DF->y, DF->z + 32, 1, ATTP_IMMEDIATECHECK, 1 );
			if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &AI_AttackPointLink )return;
		};
		DF++;
	};
	//-----------------
	int rx1 = ( rr >> 7 ) + 1;
	int EnX = -1;
	int EnY = -1;
	int mindist = 10000000;
	int rx2 = rx1 + rx1 + 1;
	int stcell;
	int MaxDif = -1;
	for ( int i = 0; i < 64; i++ )
	{
		int dx = ( ( rando()*rx2 ) >> 15 ) - rx1;
		int dy = ( ( rando()*rx2 ) >> 15 ) - rx1;
		int x1 = rx0 + ( dx << 7 );
		int y1 = ry0 + ( dy << 7 );
		if ( Norma( x1 - rx0, y1 - ry0 ) < rr )
		{
			stcell = cell + dx + ( dy << VAL_SHFCX );
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( NPresence[stcell] )
				{
					int N = GetEnemyDifference( stcell, OB->NMask );
					if ( N > MaxDif )
					{
						MaxDif = N;
						EnX = x1;
						EnY = y1;
					};
				};
			};
		};
	};
	if ( MaxDif > 5 )
	{
		OB->AttackPoint( EnX, EnY, GetHeight( EnX, EnY ) + 32, 1, ATTP_IMMEDIATECHECK, 1 );
	};
};
void AIArtilleryAgainstTowers( OneObject* OB )
{
	if ( OB->delay )
	{
		return;
	}
	int cell = ( ( OB->RealY >> 11 ) << VAL_SHFCX ) + ( OB->RealX >> 11 );
	NewMonster* NM = OB->newMons;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	int rr = ADC->MaxR_Attack + 900;
	int rx0 = ( OB->RealX >> 4 ) + 64;
	int ry0 = ( OB->RealY >> 4 ) + 64;
	int x0 = OB->RealX >> 11;
	int y0 = OB->RealY >> 11;
	//search for towers
	EnemyInfo* EIN = GNFO.EINF[OB->NNUM];
	DangerInfo* DF = EIN->DINF;
	int xxx = OB->RealX >> 4;
	int yyy = OB->RealY >> 4;
	for ( int p = 0; p < EIN->NDINF; p++ )
	{
		if ( Norma( DF->x - xxx, DF->y - yyy ) < rr )
		{
			OB->AttackPoint( DF->x, DF->y, DF->z + 32, 1, ATTP_IMMEDIATEFINDPLACE, 1 );
			if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &AI_AttackPointLink )return;
		};
		DF++;
	};
};
//---------------------ATTACK------------------------//
void AttackObjLink( OneObject* OBJ );
void BuildObjLink( OneObject* OBJ );
void ProduceObjLink( OneObject* OBJ );
void WaterAttackLink( OneObject* OBJ );
bool OneObject::AttackObj( word OID, int Prio )
{
	return AttackObj( OID, Prio, 0 );
}
bool OneObject::AttackObj( word OID, int Prio, byte OrdType )
{
	return AttackObj( OID, Prio, OrdType, 0 );
}
void B_BitvaLink( Brigade* BR );
void GrenaderSupermanLink( OneObject* OBJ );
void EraseBrigade( Brigade* BR );
void SetAttState( Brigade* BR, bool Val );
bool OneObject::AttackObj( word OID, int Prio1, byte OrdType, word NTimes )
{
	if ( Sdoxlo || Hidden || UnlimitedMotion || !Ready )
	{
		return false;
	}
	if ( CheckOrderAbility() )
	{
		return false;
	}
	if ( LocalOrder&&LocalOrder->DoLink == &ProduceObjLink )
	{
		return false;
	}
	//assert(OID!=0xFFFF);
	byte Prio;
	if ( Prio1 == 254 )
	{
		Prio = 16 + 128;
		StandGround = 0;
	}
	else
	{
		Prio = Prio1;
	}
	//if((Prio&127)>=16&&!InArmy)StandGround=0;
	if ( newMons->MaxInside && !newMons->DamWeap[0] )
	{
		return false;
	}
	OneObject* EOB = Group[OID];
	if ( !EOB )
	{
		//DeleteLastOrder();
		return 0;
	}
	if ( EOB->Sdoxlo )
	{
		//DeleteLastOrder();
		return 0;
	}
	if ( !CheckAttAbility( this, OID ) )
	{
		if ( OID && ( !( LockType || NewBuilding || newMons->Capture ) ) && EOB->newMons->Capture )
		{
			NewMonsterSmartSendTo( EOB->RealX >> 4, EOB->RealY >> 4, 0, 0, 128, 0 );
		}
		return false;
	}
	bool STRELOK = false;
	Brigade* BR = nullptr;
	byte Use = newMons->Usage;// == GrenaderID;//BUGFIX: Typo / error. See if statement below
	if ( InArmy && ( Use == GrenaderID || Use == ArcherID ) && ( EOB->NewBuilding || EOB->Wall ) )
	{
		BR = CITY[NNUM].Brigs + BrigadeID;
		SetAttState( BR, 0 );
		EraseBrigade( BR );
	}

	if ( ( !Zombi ) && InArmy && Prio1 != 15 + 128 && !( newMons->Artilery || EOB->NewBuilding ) )
	{
		//Formation
		BR = CITY[NNUM].Brigs + BrigadeID;
		if ( !BR->Strelki )
		{
			//Formation of non-shooting units
			if ( Prio1 == 254 )
			{
				return true;
			}
			else
			{
				if ( BR->BOrder )
				{
					if ( BR->BOrder->BLink != &B_BitvaLink )
					{
						if ( BR->AttEnm )
						{
							if ( Norma( RealX - EOB->RealX, RealY - EOB->RealY ) < 100 * 16 )
							{
								BR->Bitva();
							}
							return true;
						}
						else
						{
							return true;
						}
					}
				}
				else
				{
					BR->Bitva();
					return true;
				}
			}
		}
		else
		{
			//Formation of shooting units
			if ( BR->BOrder && ( BR->BOrder->Prio & 127 ) )
			{
				return true;
			}
			STRELOK = true;
		}
	}

	if ( NoSearchVictim && Prio < 128 )
	{
		return true;
	}
	NoSearchVictim = false;
	//if(CheckOrderAbility())return false;
	//if(LocalOrder&&LocalOrder->DoLink==&ProduceObjLink)return false;
	//assert(OID!=0xFFFF);
	//if(newMons->MaxInside)return false;
	//if(!CheckAttAbility(this,OID))return false;
	//if(Prio>=16)StandGround=false;
	//	WAttackObj(OID,Prio);
	//	return;
	//};
	//if(Media==2){
	//	FlyAttack(OID,Prio);
	//	return;
	//};
	OneObject* OB = Group[OID];
	if ( ( !OB ) || OB->UnlimitedMotion )
	{
		return false;
	}
	if ( Prio < PrioryLevel/*&&!Attack*/ )
	{
		return false;
	}
	if ( Prio < 5 && EnemyID != 0xFFFF )
	{
		OneObject* EOB = Group[EnemyID];
		if ( EOB )
		{
			int dist1 = Norma( OB->RealX - RealX, OB->RealY - RealY );
			int dist2 = Norma( EOB->RealX - RealX, EOB->RealY - RealY );
			if ( dist1 <= Ref.General->MoreCharacter->MinR_Attack || dist2 <= dist1 )
			{
				return false;
			}
		}
	}
	//if(Prio<16&&Prio==PrioryLevel)return false;
	if ( !Ready )
	{
		return false;
	}
	//if(Weap)return;
	//ClearOrders();
	if ( RAmount )
	{
		RAmount = 0;
		RType = 0xFF;
		GroundState = 0;
	}
	if ( !LocalOrder )
	{
		Attack = false;
	}
	if ( !int( OB ) || OB->Sdoxlo )
	{
		return false;
	}
	//Important=true;
	StandTime = 0;
	if ( Attack )
	{
		if ( !( LocalOrder&&LocalOrder->DoLink == &GrenaderSupermanLink ) )
		{
			EnemyID = OID;
			EnemySN = OB->Serial;
			if ( LocalOrder && ( LocalOrder->DoLink == &AttackObjLink || LocalOrder->DoLink == &WaterAttackLink ) )
			{
				PrioryLevel = Prio & 127;
				LocalOrder->PrioryLevel = PrioryLevel;
			}
			return true;
		}
	}
	if ( newMons->Priest )
	{
		if ( OB->Life >= OB->Ref.General->MoreCharacter->Life )
		{
			return false;
		}
		if ( !( NMask&OB->NMask ) )
		{
			return false;
		}
	}
	else
	{
		if ( NMask&OB->NMask )
		{
			return false;
		}
	}
	if ( InPatrol && OrdType == 0 )
	{
		OrdType = 1;
	}
	Order1* Or1 = CreateOrder( OrdType );
	if ( !int( Or1 ) )
	{
		return false;
	}
	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 3;
	Or1->OrderTime = 0;
	Or1->DoLink = AttackLink;
	if ( newMons->NMultiWp )
	{
		Or1->DoLink = &WaterAttackLink;
	}
	Or1->info.BuildObj.ObjX = OB->x;
	Or1->info.BuildObj.ObjY = OB->y;
	Or1->info.BuildObj.SN = NTimes;
	Or1->info.BuildObj.AttMethod = 0xFF;
	EnemyID = OID;
	EnemySN = OB->Serial;
	Attack = true;
	//if(int(InLineCom))FreeAsmLink();
	if ( Prio >= 16 )
	{
		MakeOrderSound( this, 1 );
	}
	RType = 0xFF;
	//PrioryLevel=Prio&127;
	//if(CrowdRef)CrowdRef->AddToAttackQueue(OID);
	//AlwaysLock=true;
	PathDelay = 0;
	StandTime = 0;
	if ( STRELOK )
	{
		if ( PrioryLevel == 0 )
		{
			if ( BR->BOrder )
			{
				BR->ClearBOrders();
			}
			int N = BR->NMemb;
			word* mem = BR->Memb;
			word* sns = BR->MembSN;
			for ( int i = 2; i < N; i++ )
			{
				if ( mem[i] != 0xFFFF )
				{
					OneObject* OB = Group[mem[i]];
					if ( OB != this && OB->EnemyID == 0xFFFF )
					{
						OB->AttackObj( OID, Prio, OrdType, NTimes );
					}
				}
			}
		}
	}
	return true;
}

void NewAttackPointLink( OneObject* OBJ );

bool OneObject::NewAttackPoint( int px, int py, int Prio1, byte OrdType, word NTimes )
{
	if ( Sdoxlo || Hidden || UnlimitedMotion )
	{
		return false;
	}
	if ( CheckOrderAbility() )
	{
		return false;
	}
	if ( LocalOrder&&LocalOrder->DoLink == &ProduceObjLink )
	{
		return false;
	}
	//assert(OID!=0xFFFF);
	byte Prio;
	if ( Prio1 == 254 )
	{
		Prio = 16 + 128;
		StandGround = 0;
	}
	else
	{
		Prio = Prio1;
	}
	//if((Prio&127)>=16&&!InArmy)StandGround=0;
	if ( newMons->MaxInside )
	{
		return false;
	}
	if ( Prio < PrioryLevel/*&&!Attack*/ )
	{
		return false;
	}
	if ( !Ready )
	{
		return false;
	}
	//if(Weap)return;
	//ClearOrders();
	if ( RAmount )
	{
		RAmount = 0;
		RType = 0xFF;
		GroundState = 0;
	}
	Order1* Or1 = CreateOrder( OrdType );
	if ( !int( Or1 ) )
	{
		return false;
	}
	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 76;//ГЂГІГ ГЄГ 
	Or1->OrderTime = 0;
	Or1->DoLink = &NewAttackPointLink;

	Or1->info.BuildObj.ObjX = px >> 4;
	Or1->info.BuildObj.ObjY = py >> 4;
	Or1->info.BuildObj.SN = NTimes;
	Or1->info.BuildObj.AttMethod = 0xFF;
	DstX = px << 4;
	DstY = py << 4;
	EnemyID = 0xFFFF;
	EnemySN = 0xFFFF;
	Attack = 0;
	//if(int(InLineCom))FreeAsmLink();
	if ( Prio >= 16 )
	{
		MakeOrderSound( this, 1 );
	}
	RType = 0xFF;
	//PrioryLevel=Prio&127;
	//if(CrowdRef)CrowdRef->AddToAttackQueue(OID);
	//AlwaysLock=true;
	PathDelay = 0;
	StandTime = 0;

	return true;
}

void SetDestUnit( OneObject* OBJ, OneObject* OB, short OLDX, short OLDY, short NEWX, short NEWY )
{
	if ( !OB->newMons->Artilery )
	{
		int dr = Norma( NEWX - OBJ->x, NEWY - OBJ->y );
		int dr1 = Norma( NEWX - OBJ->CPdestX, NEWY - OBJ->CPdestY );
		bool renew = 0;
		if ( dr > 500 )
		{
			if ( dr1 > 30 )renew = 1;
		}
		else if ( dr > 100 )
		{
			if ( dr1 > 30 )renew = 1;
		}
		else if ( dr > 50 )
		{
			if ( dr1 > 20 )renew = 1;
		}
		else if ( dr > 30 && dr1 > 10 )renew = 1;
		if ( dr > 30 )
		{
			if ( renew )
			{
				OBJ->PathDelay = 0;
				OBJ->CreatePath( NEWX, NEWY );
			}
			else OBJ->CreatePath( OBJ->CPdestX, OBJ->CPdestY );
			return;
		};
	};
	/*
	if(OBJ->newMons->MaxAttRange<150){
		OBJ->CreatePath(OB->x,OB->y);
		return;
	};
	*/
	if ( !OBJ->PathX )
	{
		OBJ->DestX = OB->RealX;
		OBJ->DestY = OB->RealY;
		OBJ->LocalOrder->info.BuildObj.ObjX = NEWX;
		OBJ->LocalOrder->info.BuildObj.ObjY = NEWY;
	}
	else
	{
		int dis = Norma( OBJ->x - NEWX, OBJ->y - NEWY );
		int dis1 = Norma( NEWX - OLDX, NEWY - OLDY );
		int dds = 0;
		if ( dis > 100 )dds = 20;
		else if ( dis > 50 )dds = 10;
		else if ( dis > 30 )dds = 5;
		else if ( dis > 10 )dds = 3;
		else dds = 2;
		if ( dis1 >= dds )
		{
			OBJ->DeletePath();
			//OBJ->DestX=OB->RealX;
			//OBJ->DestY=OB->RealY;
			OBJ->CreatePath( NEWX, NEWY );
			OBJ->LocalOrder->info.BuildObj.ObjX = NEWX;
			OBJ->LocalOrder->info.BuildObj.ObjY = NEWY;
		}
		else OBJ->CreatePath( OLDX, OLDY );
	};
};

bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs,
	int xd, int yd, int zd,
	OneObject* OB );
extern bool ShotRecommended;
void CreateRazbros( OneObject* OBJ, Coor3D* C3D )
{
	AdvCharacter* ADC = OBJ->Ref.General->MoreCharacter;
	if ( !ADC->Razbros )return;
	int R = Norma( ( ( OBJ->RealX ) >> 4 ) - C3D->x, ( OBJ->RealY >> 4 ) - C3D->y ) >> 5;
	C3D->x += div( ( int( rando() >> 5 ) - 512 )*R*ADC->Razbros, 32000 ).quot;
	C3D->y += div( ( int( rando() >> 5 ) - 512 )*R*ADC->Razbros, 32000 ).quot;
};
int CheckPointForDamageAbility( OneObject* OBJ, int x, int y, int z )
{
	int dst = Norma( ( OBJ->RealX >> 4 ) - x, ( OBJ->RealY >> 4 ) - y );
	int dst1 = dst - ( int( OBJ->RZ + OBJ->newMons->SrcZPoint - z ) << 1 );
	NewMonster* NMN = OBJ->newMons;
	AdvCharacter* ADC = OBJ->Ref.General->MoreCharacter;
	if ( dst<ADC->AttackRadius1[0] || dst1>ADC->AttackRadius2[0] )return -1;
	int xx, yy, x0, y0;
	OBJ->GetCornerXY( &xx, &yy );
	xx <<= 8;
	yy <<= 8;
	int mindis = 1000000;
	int bsp = 0;
	int np = NMN->NShotPt;
	int xx1 = x << 4;
	int yy1 = y << 4;
	int zzz = NMN->SrcZPoint;
	int DZZ = OBJ->RZ;
	for ( int i = 0; i < np; i++ )
	{
		x0 = xx + ( NMN->ShotPtX[i] << 4 );
		y0 = yy + ( Prop43( NMN->ShotPtY[i] ) << 4 );
		dst = Norma( x0 - xx1, y0 - yy1 );
		if ( dst < mindis )
		{
			mindis = dst;
			bsp = i;
		};
	};
	x0 = xx + ( NMN->ShotPtX[bsp] << 4 );
	y0 = yy + ( Prop43( NMN->ShotPtY[bsp] ) << 4 ) + ( zzz << 5 );
	Weapon* Weap = NMN->DamWeap[0];
	if ( Weap )
	{
		int wx0 = x0 >> 4;
		int wy0 = ( y0 >> 4 ) + 6;
		int wz0 = 3 + DZZ + zzz;
		int WRes = PredictShot( Weap, wx0, wy0, wz0, x, y, z, OBJ->Index );
		if ( WRes != -1 ) return -1;
		return 0;
	};
	return -1;
};
bool TestUnitsInSectorForGrenade( OneObject* GRE );
bool AttGroundMod = 0;
void NewAttackPointLink( OneObject* OBJ )
{
	AttGroundMod = 1;
	AttackObjLink( OBJ );
	AttGroundMod = 0;
};
extern int ADDRANGE;
extern int ADDPAUSE;
void AttackObjLink( OneObject* OBJ )
{
	//if(OBJ->StandTime>70/*&&OBJ->PathDelay*/){
	//	OBJ->DeleteLastOrder();
	//	return;
	//};
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	byte prl = OBJ->PrioryLevel;
	NewMonster* NMN = OBJ->newMons;
	AdvCharacter* ADC = OBJ->Ref.General->MoreCharacter;
	OBJ->DestX = -1;
	//OBJ->UnBlockUnit();
	word OID = OBJ->EnemyID;
	word OSN = OBJ->EnemySN;
	if ( OID >= ULIMIT && !AttGroundMod )
	{
		OBJ->DeleteLastOrder();
		return;
	};
	OneObject TMPOBJ;
	OneObject* OB;
	if ( AttGroundMod )
	{
		memset( &TMPOBJ, 0, sizeof OneObject );
		OB = &TMPOBJ;
		TMPOBJ.RealX = OBJ->DstX;
		TMPOBJ.RealY = OBJ->DstY;
		TMPOBJ.x = OBJ->DstX >> 8;
		TMPOBJ.y = OBJ->DstY >> 8;
		TMPOBJ.RZ = GetHeight( OBJ->RealX >> 4, OBJ->RealY >> 4 );
		TMPOBJ.Index = 0xFFFF;
		TMPOBJ.Serial = 0xFFFF;
		TMPOBJ.Ref.General = NATIONS[7].Mon[0];
		TMPOBJ.newMons = NATIONS[7].Mon[0]->newMons;
		TMPOBJ.NIndex = 7;
		OID = 0xFFFF;
		OSN = 0xFFFF;
	}
	else OB = Group[OID];
	if ( ( ( !OB ) || ( OB->Sdoxlo > 8 ) || OSN != OB->Serial ) || OBJ->TurnOff )
	{
		if ( OBJ->PrioryLevel >= 16 && OBJ->NNUM == MyNation&&OBJ->TurnOff )
		{
			char ccc[128];
			sprintf( ccc, GetTextByID( "CSNOGL" ), OBJ->Ref.General->Message );
			CreateTimedHint( ccc, kMinorMessageDisplayTime );//You have no gold. %s cannot shoot.
		};
		OBJ->DeleteLastOrder();
		return;
	};
	if ( NMN->Priest )
	{
		if ( !OB->NMask&OBJ->NMask )
		{
			OBJ->DeleteLastOrder();
			return;
		};
	}
	else
	{
		if ( OB->NMask&OBJ->NMask )
		{
			OBJ->DeleteLastOrder();
			return;
		};
	};


	byte maska = OB->newMons->MathMask;
	if ( OB->newMons->CanBeKilledInside )
	{
		if ( OB->NInside )maska = 0xFF;
	};
	short OLDX = OBJ->LocalOrder->info.BuildObj.ObjX;
	short OLDY = OBJ->LocalOrder->info.BuildObj.ObjY;
	word NTIMES = OBJ->LocalOrder->info.BuildObj.SN;
	short NEWX = OB->RealX >> 8;
	short NEWY = OB->RealY >> 8;
	if ( OBJ->NewBuilding )
	{
		if ( OBJ->delay )
		{
			//OBJ->delay--;
			return;
		};
		if ( NMN->ShotDir )
		{
			if ( !OBJ->NInside )
			{
				OBJ->DeleteLastOrder();
				return;
			};
			int xx, yy;
			OBJ->GetCornerXY( &xx, &yy );
			xx *= 16;
			yy *= 16;
			int N = NMN->NShotPt;
			for ( int i = 0; i < N; i++ )
			{
				int x = xx + NMN->ShotPtX[i];
				int y = NMN->ShotPtY[i];
				int z = NMN->ShotPtZ[i];
				y = yy + ( ( y + z ) << 1 );
				z += OBJ->RZ;
				char dir = NMN->ShotDir[i];
				byte diff = NMN->ShotDiff[i];
				int minr = NMN->ShotMinR[i];
				int maxr = NMN->ShotMaxR[i];

				int rx = OB->RealX >> 4;
				int ry = OB->RealY >> 4;

				int dst = Norma( x - rx, y - ry );
				char rdir = GetDir( rx - x, ry - y );
				char ddir = abs( rdir - dir );
				if ( dst >= minr&&dst <= maxr&&ddir < diff )
				{
					//can shot
					Weapon* Weap = NMN->DamWeap[0];
					if ( Weap )
					{
						Coor3D C3D;
						word DestObj = 0xFFFF;
						if ( OB->GetDamagePoint( &C3D, 0 ) )DestObj = OID;
						int wx0 = x;
						int wy0 = y;
						int wz0 = z;
						int wx1 = C3D.x;
						int wy1 = C3D.y;
						int wz1 = C3D.z;
						int WRes = -1;

						if ( !NMN->SlowRecharge )
						{
							WRes = PredictShot( Weap, wx0, wy0, wz0, wx1, wy1, wz1, OBJ->Index );
						}

						if ( WRes != -1 )
						{//something is in the way, cannot shoot

							if ( WRes > 8192 )
							{
								//ground
								if ( OBJ->NNUM == MyNation )
								{
									//char atts[128];
									//sprintf( atts, MESHPOV, OBJ->newMons->Message );
									//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: The terrain profile prevents you from shooting.
								}
							}
							else
							{
								OneObject* OBX = Group[WRes];
								if ( OBX )
								{
									if ( ShotRecommended && !( OBX->NMask & OBJ->NMask ) )
									{
										goto DoShotA;
									}
									if ( OBJ->NNUM == MyNation )
									{
										//char atts[128];
										//sprintf( atts, MESHOBJ, OBJ->newMons->Message, OBX->newMons->Message );
										//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: %s prevents you from shooting.
									}
								}
							}

							OBJ->delay = 100;

							if ( OBJ->delay > OBJ->MaxDelay )
							{
								OBJ->delay = OBJ->MaxDelay;
							}

							OBJ->DeleteLastOrder();
						}
					DoShotA:
						bool AllowShot = true;
						if ( NMN->NShotRes )
						{
							for ( int k = 0; k < NMN->NShotRes; k++ )
							{
								if ( XRESRC( OBJ->NNUM, NMN->ShotRes[k + k] ) < NMN->ShotRes[k + k + 1] )
								{
									AllowShot = false;
								}
							}

							if ( AllowShot )
							{
								for ( int k = 0; k < NMN->NShotRes; k++ )
								{
									AddXRESRC( OBJ->NNUM, NMN->ShotRes[k + k], -NMN->ShotRes[k + k + 1] );
									OBJ->Nat->AddResource( byte( NMN->ShotRes[k + k] ), -NMN->ShotRes[k + k + 1] );
									OBJ->Nat->ResOnLife[NMN->ShotRes[k + k]] += NMN->ShotRes[k + k + 1];
								}
							}
							else
							{
								if ( OBJ->NNUM == MyNation&&OBJ->PrioryLevel >= 16 )
								{
									char ccc[150];
									sprintf( ccc, GetTextByID( "CSNORS" ), OBJ->Ref.General->Message );
									CreateTimedHint( ccc, 64 );//You have neither coal nor iron. %s cannot shoot.
								}
								OBJ->DeleteLastOrder();
								return;
							}
						};

						if ( AllowShot )
						{
							CreateRazbros( OBJ, &C3D );
							Create3DAnmObject( Weap, wx0, wy0, wz0, wx1, wy1, wz1, OBJ, 0, DestObj );
							//determining the pause
							if ( OBJ->NInside )
							{
								OBJ->delay = ADC->AttackPause[0];
								OBJ->delay = ( int( OBJ->delay )*OBJ->PersonalDelay ) / ( 100 * OBJ->NInside );
								OBJ->MaxDelay = OBJ->delay;
							}
							else
							{
								OBJ->delay = ADC->AttackPause[0];
								OBJ->delay = ( int( OBJ->delay )*OBJ->PersonalDelay ) / 100;
								OBJ->MaxDelay = OBJ->delay;
								OBJ->DeleteLastOrder();
								return;
							};
							if ( NTIMES )
							{
								OBJ->LocalOrder->info.BuildObj.SN--;
								if ( NTIMES == 1 )
								{
									OBJ->DeleteLastOrder();
									return;
								};
							};
							return;
						};
					};
					OBJ->DeleteLastOrder();
					return;
				};
			};
			OBJ->DeleteLastOrder();
			return;
		};

		if ( !NMN->NShotPt )
		{
			OBJ->DeleteLastOrder();
			return;
		};
		int dst = ( Norma( OBJ->RealX - OB->RealX, OBJ->RealY - OB->RealY ) >> 4 );
		int dst1 = dst - ( int( OBJ->RZ + OBJ->newMons->SrcZPoint - ( OB->RZ + OB->newMons->SrcZPoint ) ) << 1 );
		if ( dst<ADC->AttackRadius1[0] || dst1>ADC->AttackRadius2[0] )
		{
			OBJ->DeleteLastOrder();
			return;
		};
		int xx, yy, x0, y0;
		OBJ->GetCornerXY( &xx, &yy );
		xx <<= 8;
		yy <<= 8;
		int mindis = 1000000;
		int bsp = 0;
		int np = NMN->NShotPt;
		int xx1 = OB->RealX;
		int yy1 = OB->RealY;
		int zzz = NMN->SrcZPoint;
		int DZZ = OBJ->RZ;
		for ( int i = 0; i < np; i++ )
		{
			x0 = xx + ( NMN->ShotPtX[i] << 4 );
			y0 = yy + ( Prop43( NMN->ShotPtY[i] ) << 4 );
			dst = Norma( x0 - xx1, y0 - yy1 );
			if ( dst < mindis )
			{
				mindis = dst;
				bsp = i;
			};
		};
		x0 = xx + ( NMN->ShotPtX[bsp] << 4 );
		y0 = yy + ( Prop43( NMN->ShotPtY[bsp] ) << 4 ) + ( zzz << 5 );
		Weapon* Weap = NMN->DamWeap[0];
		if ( Weap )
		{
			Coor3D C3D;
			word DestObj = 0xFFFF;
			if ( OB->GetDamagePoint( &C3D, 0 ) )DestObj = OID;
			int wx0 = x0 >> 4;
			int wy0 = ( y0 >> 4 ) + 6;
			int wz0 = 3 + DZZ + zzz;
			int wx1 = C3D.x;
			int wy1 = C3D.y;
			int wz1 = C3D.z;
			int WRes = PredictShot( Weap, wx0, wy0, wz0, wx1, wy1, wz1, OBJ->Index );
			if ( WRes != -1 )
			{
				//something on the way...
				if ( WRes > 8192 )
				{
					//ground
					if ( OBJ->NNUM == MyNation )
					{
						//char atts[128];
						//sprintf( atts, MESHPOV, OBJ->newMons->Message );
						//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: The terrain profile prevents you from shooting.
					}
				}
				else
				{
					OneObject* OBX = Group[WRes];
					if ( OBX )
					{
						if ( ShotRecommended && !( OBX->NMask&OBJ->NMask ) )goto DoShot0;
						if ( OBJ->NNUM == MyNation )
						{
							//char atts[128];
							//sprintf( atts, MESHOBJ, OBJ->newMons->Message, OBX->newMons->Message );
							//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: %s prevents you from shooting.
						}
					}
				}
				OBJ->delay = 100;

				if ( OBJ->delay > OBJ->MaxDelay )OBJ->delay = OBJ->MaxDelay;

				OBJ->DeleteLastOrder();
				return;
			};
		DoShot0:;
			bool AllowShot = true;
			if ( NMN->NShotRes )
			{
				for ( int k = 0; k < NMN->NShotRes; k++ )
				{
					if ( XRESRC( OBJ->NNUM, NMN->ShotRes[k + k] ) < NMN->ShotRes[k + k + 1] )AllowShot = false;
				};
				if ( AllowShot )
				{
					for ( int k = 0; k < NMN->NShotRes; k++ )
					{
						AddXRESRC( OBJ->NNUM, NMN->ShotRes[k + k], -NMN->ShotRes[k + k + 1] );
						OBJ->Nat->AddResource( byte( NMN->ShotRes[k + k] ), -NMN->ShotRes[k + k + 1] );
						OBJ->Nat->ResOnLife[NMN->ShotRes[k + k]] += NMN->ShotRes[k + k + 1];
					};
				}
				else
				{
					if ( OBJ->NNUM == MyNation&&OBJ->PrioryLevel >= 16 )
					{
						char ccc[150];
						sprintf( ccc, GetTextByID( "CSNORS" ), OBJ->Ref.General->Message );
						CreateTimedHint( ccc, 64 );//You have neither coal nor iron. %s cannot shoot.
					};
					OBJ->DeleteLastOrder();
					return;
				};
			};

			if ( AllowShot )
			{
				CreateRazbros( OBJ, &C3D );
				Create3DAnmObject( Weap, wx0, wy0, wz0, wx1, wy1, wz1, OBJ, 0, DestObj );
				OBJ->delay = ADC->AttackPause[0];

				OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
				OBJ->delay = OBJ->MaxDelay;

				if ( NTIMES )
				{
					OBJ->LocalOrder->info.BuildObj.SN--;
					if ( NTIMES == 1 )
					{
						OBJ->DeleteLastOrder();
						return;
					};
				};
			};
		};
		return;
	};





	int nst = OBJ->NewState;
	bool DELAY;
	bool CANSETDELAY = 1;
	bool NEEDSLOWRECHARGE = 0;
#ifdef CONQUEST
	DELAY = OBJ->delay;
	if ( DELAY )
	{
		if ( NMN->NoWaitMask&( 1 << ( nst - 1 ) ) )DELAY = 0;
		CANSETDELAY = 0;
	};
#else
	DELAY = 0 != OBJ->delay;
#endif
	if ( DELAY&&NMN->SlowRecharge )
	{
		NEEDSLOWRECHARGE = 1;
	};
	if ( nst && !DELAY )
	{
		NewAnimation* Anm = &NMN->Attack[OBJ->NewState - 1];

		if ( Anm->Enabled )
		{

			int af = Anm->ActiveFrame;
			if ( !af )af = 1;
			if ( FrmDec == 2 && ( af & 1 ) )af++;
			if ( OBJ->NewAnm == Anm&&OBJ->NewCurSprite == af )
			{
				if ( NMN->Usage == PushkaID&&OBJ->NewState == 2 && prl < 10 )
				{
					if ( !TestUnitsInSectorForGrenade( OBJ ) )
					{
						OBJ->DeleteLastOrder();
						return;
					};
				};
				//checking the distance
				int x0 = OB->RealX;
				int y0 = OB->RealY;
				char EnDir = char( GetDir( x0 - OBJ->RealX, y0 - OBJ->RealY ) );
				char ddir = EnDir - OBJ->RealDir;
				OBJ->RealDir = EnDir;
				OBJ->GraphDir = OBJ->RealDir;
				int dr1 = ADC->AttackRadius1[OBJ->NewState - 1];
				int dr2 = ADC->AttackRadius2[OBJ->NewState - 1] +
					NMN->AttackRadiusAdd[OBJ->NewState - 1];
				int DPAUS = 100;
				if ( OBJ->NewState == 1 && AttGroundMod )
				{
					dr2 = ( ADDRANGE*dr2 ) / 100;
					DPAUS = ADDPAUSE;
				};
				int MyZ = OBJ->RZ + NMN->SrcZPoint;
				int EnZ = OB->RZ + OB->newMons->DstZPoint;
				int dst = ( Norma( x0 - OBJ->RealX, y0 - OBJ->RealY ) >> 4 );
				int dst1 = dst + OB->newMons->AddShotRadius;
				byte Wpk = NMN->WeaponKind[OBJ->NewState - 1];
				if ( WeaponFlags[Wpk] & 1 )dst1 -= ( ( MyZ - EnZ ) << 1 );
				//int angl=0;
				//if(dst>0)angl=div((EnZ-MyZ)<<6,dst).quot;
				//dst-=(MyZ-EnZ)>>1;
				if ( dst >= dr1&&dst1 <= dr2/*&&angl>=NMN->AngleDn[OBJ->NewState-1]&&

					angl<=NMN->AngleUp[OBJ->NewState-1]*/ )
				{
					Weapon* WP = NMN->DamWeap[OBJ->NewState - 1];
					if ( CANSETDELAY )
					{
						OBJ->delay = ADC->AttackPause[OBJ->NewState - 1];


						OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
						OBJ->MaxDelay = ( int( OBJ->MaxDelay )*DPAUS ) / 100;
						OBJ->delay = OBJ->MaxDelay;

					};

					//pubuxxxxxxxxx...
					if ( WP )
					{
						//----------
						NewAnimation* UNAM = OBJ->NewAnm;
						int csp = OBJ->NewCurSprite;
						int uocts = ( UNAM->Rotations - 1 ) * 2;
						int uoc2 = UNAM->Rotations - 1;
						if ( !uocts )uocts = 1;
						int usesize = div( 255, uocts << 1 ).quot;
						byte udir = ( ( ( OBJ->RealDir + 64 + usesize ) & 255 )*uocts ) >> 8;
						NewFrame* NF = &UNAM->Frames[csp];
						//----------
						NewAnimation* NAM = WP->NewAnm;
						int octs = ( NAM->Rotations - 1 ) * 2;
						int oc2 = NAM->Rotations - 1;
						if ( !octs )octs = 1;
						int sesize = div( 255, octs << 1 ).quot;
						byte dir = ( ( ( OBJ->RealDir + 64 + sesize ) & 255 )*octs ) >> 8;
						Coor3D C3D;
						word DestObj = 0xFFFF;
						if ( OB->GetDamagePoint( &C3D, 0 ) )DestObj = OID;
						if ( OB->Wall )DestObj = OID;
						//OBJ->BlockUnit();
						int x0, y0, z0, x1, y1, z1;
						if ( udir < uoc2 )
						{
							int udir1 = uoc2 - udir;
							x0 = ( OBJ->RealX >> 4 ) - ( UNAM->ActivePtX[udir1] + NF->dx );
							y0 = ( OBJ->RealY >> 4 ) + Prop43( UNAM->ActivePtY[udir1] + NF->dy + NMN->SrcZPoint );
						}
						else
						{
							int udir1 = uoc2 - uocts + udir;
							x0 = ( OBJ->RealX >> 4 ) + ( UNAM->ActivePtX[udir1] + NF->dx );
							y0 = ( OBJ->RealY >> 4 ) + Prop43( UNAM->ActivePtY[udir1] + NF->dy + NMN->SrcZPoint );
						};

						z0 = OBJ->RZ + NMN->SrcZPoint;
						x1 = C3D.x;
						y1 = C3D.y;
						z1 = C3D.z;
						int WRes = PredictShot( WP, x0, y0, z0, x1, y1, z1, OBJ->Index );
						if ( WRes != -1 )
						{
							//something on the way...
							if ( WRes >= ULIMIT )
							{
								//ground
								if ( OBJ->NNUM == MyNation )
								{
									//char atts[128];
									//sprintf( atts, MESHPOV, OBJ->newMons->Message );
									//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: The terrain profile prevents you from shooting.
								}
							}
							else
							{
								if ( WRes == OB->Index )goto DoShot;
								OneObject* OBX = Group[WRes];
								if ( OBX )
								{
									if ( ShotRecommended && !( OBX->NMask&OBJ->NMask ) )goto DoShot;
									if ( OBJ->NNUM == MyNation )
									{
										//char atts[128];
										//sprintf( atts, MESHOBJ, OBJ->newMons->Message, OBX->newMons->Message );
										//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: %s prevents you from shooting.
									}
								}
							}
							//attempt to move
							if ( !OBJ->StandGround )
							{
								int dxx = y1 - y0;
								int dyy = x0 - x1;
								int nn = Norma( dxx, dyy );
								dxx = ( dxx * 20 ) / nn;
								dyy = ( dyy * 20 ) / nn;
								bool LAllow = true;
								bool RAllow = true;
								int XL = x0;
								int YL = y0;
								int XR = x0;
								int YR = y0;
								for ( int j = 0; j < 10 && ( LAllow || RAllow ); j++ )
								{
									XL += dxx;
									YL += dyy;
									XR -= dxx;
									YR -= dyy;
									if ( CheckPt( XL >> 4, YL >> 4 ) )LAllow = false;
									if ( CheckPt( XR >> 4, YR >> 4 ) )RAllow = false;
									if ( LAllow&&PredictShot( WP, XL, YL, z0, x1, y1, z1, OBJ->Index ) == -1 )
									{
										OBJ->NewMonsterSendTo( ( XL + dxx ) << 4, ( YL + dyy ) << 4, 128 + 16, 1 );
										return;
									};
									if ( RAllow&&PredictShot( WP, XR, YR, z0, x1, y1, z1, OBJ->Index ) == -1 )
									{
										OBJ->NewMonsterSendTo( ( XR - dxx ) << 4, ( YR - dyy ) << 4, 128 + 16, 1 );
										return;
									};
									if ( CheckPt( XL >> 4, YL >> 4 ) )LAllow = false;
									if ( CheckPt( XR >> 4, YR >> 4 ) )RAllow = false;
								};
								if ( CANSETDELAY )
								{
									OBJ->delay = 16;

									if ( OBJ->delay > OBJ->MaxDelay )OBJ->delay = OBJ->MaxDelay;

								}
								OBJ->DeleteLastOrder();
								return;
							}
							else
							{
								OBJ->DeleteLastOrder();
								return;
							}
						}
					DoShot:;
						bool AllowShot = true;
						if ( !AllowShot )return;
						if ( udir < uoc2 )
						{
							udir = uoc2 - udir;
							CreateRazbros( OBJ, &C3D );
							x1 = C3D.x;
							y1 = C3D.y;
							z1 = C3D.z + NMN->DstZPoint;
							Create3DAnmObject( WP, x0, y0, z0, x1, y1, z1, OBJ, OBJ->NewState - 1, DestObj );
						}
						else
						{
							udir = uoc2 - uocts + udir;
							CreateRazbros( OBJ, &C3D );
							x1 = C3D.x;
							y1 = C3D.y;
							z1 = C3D.z + NMN->DstZPoint;
							Create3DAnmObject( WP, x0, y0, z0, x1, y1, z1, OBJ, OBJ->NewState - 1, DestObj );
						}

						if ( NTIMES )
						{
							OBJ->LocalOrder->info.BuildObj.SN--;
							if ( NTIMES == 1 )
							{
								OBJ->DeleteLastOrder();
								return;
							};
						};
					}
					else
					{
						if ( NMN->Priest )
						{
							if ( OB->Life < OB->Ref.General->MoreCharacter->Life )
							{
								OB->Life += OBJ->Ref.General->MoreCharacter->MaxDamage[0];
								if ( OB->Life > OB->Ref.General->MoreCharacter->Life )
								{
									OB->Life = OB->Ref.General->MoreCharacter->Life;
								}
							}
							else
							{
								OBJ->DeleteLastOrder();
								return;
							}
						}
						else
						{
							OB->MakeDamage( 0, ADC->MaxDamage[OBJ->NewState - 1], OBJ, OBJ->NewState - 1 );
							AddEffect( OBJ->RealX >> 4, ( OBJ->RealY >> 5 ) - OBJ->RZ, NMN->StrikeSoundID );
							OB->AttackObj( OBJ->Index, 2 );
						}

						if ( NTIMES )
						{
							OBJ->LocalOrder->info.BuildObj.SN--;
							if ( NTIMES == 1 )
							{
								OBJ->DeleteLastOrder();
								return;
							}
						}
					}
					return;
				}
			}
		}
	}

	if ( OBJ->NewCurSprite >= OBJ->NewAnm->NFrames - FrmDec )
	{
		if ( DELAY )
		{
			NewAnimation* Anm = &NMN->Attack[OBJ->NewState - 1];
			if ( Anm->Enabled )
			{
				if ( OBJ->NewAnm == Anm )
				{
					if ( NMN->SlowRecharge )
					{
						int ns = OBJ->NewState;
						if ( ns )
						{
							if ( ns == 4 || WeaponFlags[NMN->WeaponKind[ns - 1]] & 1 )
							{
								OBJ->NewState = 4;
								OBJ->LocalNewState = 4;
							}
							else
							{
								TryToStand( OBJ, false );
							}
						}
						else
						{
							TryToStand( OBJ, false );
						}
					}
					else
					{
						TryToStand( OBJ, false );
					}
				}
			}
		}
		else
		{
			if ( NMN->SlowRecharge )
			{
				NewAnimation* Anm = &NMN->Attack[OBJ->NewState - 1];
				if ( Anm->Enabled&&OBJ->NewAnm == Anm )
				{
					int ns = OBJ->NewState;
					if ( ns )
					{
						if ( ns == 4 || WeaponFlags[NMN->WeaponKind[ns - 1]] & 1 )
						{
							OBJ->NewState = 4;
							OBJ->LocalNewState = 4;
						}
						else
						{
							TryToStand( OBJ, false );
						}
					}
					else
					{
						TryToStand( OBJ, false );
					}
				}
				else
				{
					TryToStand( OBJ, false );
				}
			}
		}

		//animation has finished and now we can analyse all
		//determining the need state
		int x0 = OB->RealX;
		int y0 = OB->RealY;
		int dst = Norma( x0 - OBJ->RealX, y0 - OBJ->RealY ) >> 4;

		int MyZ = OBJ->RZ + NMN->SrcZPoint;
		int EnZ = OB->RZ + OB->newMons->DstZPoint;
		int dst1 = dst;
		int dstx = dst - ( ( MyZ - EnZ ) << 1 );

		int NeedState = -1;

		for ( int i = 0; i < NAttTypes; i++ )
		{
			int wpk = NMN->WeaponKind[i];
			if ( WeaponFlags[wpk] & 1 )
			{
				if ( dst1 >= ADC->DetRadius1[i] && dstx <= ADC->DetRadius2[i] && ( NMN->AttackMask[i] & maska ) )
				{
					NeedState = i;
				};
			}
			else
			{
				if ( dst >= ADC->DetRadius1[i] && dst <= ADC->DetRadius2[i] && ( NMN->AttackMask[i] & maska ) )
				{
					NeedState = i;
				};
			};
		}

		if ( NeedState == -1 )
		{
			OBJ->DeleteLastOrder();
			if ( !OBJ->StandGround )OBJ->NewMonsterSmartSendTo( OB->RealX >> 4, OB->RealY >> 4, 0, 0, OBJ->PrioryLevel, 1 );
			return;
		}

		int wpk = NMN->WeaponKind[NeedState];
		if ( WeaponFlags[wpk] & 1 )
		{
			dst = dstx;
			if ( NEEDSLOWRECHARGE )
			{
				if ( OBJ->LocalNewState != 4 )
				{
					OBJ->NewState = 4;
					return;
				}
				else
				{
					NewAnimation* NRE = &NMN->Attack[3];
					if ( OBJ->NewAnm != NRE )
					{
						if ( OBJ->delay > NRE->NFrames )OBJ->delay -= NRE->NFrames;
						else OBJ->delay = 0;
						OBJ->NewAnm = &NMN->Attack[3];
						OBJ->NewCurSprite = 0;
						OBJ->InMotion = 0;
						OBJ->BackMotion = 0;
						return;
					}
					else
					{
						if ( OBJ->delay > NRE->NFrames )
						{
							OBJ->delay -= NRE->NFrames;
							OBJ->NewAnm = &NMN->Attack[3];
							OBJ->NewCurSprite = 0;
							OBJ->InMotion = 0;
							OBJ->BackMotion = 0;
							//OB->delay-=NRE->NFrames;
							return;
						};
						OBJ->delay = 0;
					};
				};
			};
		};
		//checking distance
		NewAnimation* NAMO = &NMN->PMotionL[NeedState];
		bool AMove = NAMO->Enabled;//&&!NMN->MaxAIndex;
		int MRot = NMN->MinRotator;
		int MRot1 = MRot + ( MRot >> 1 );
		if ( MRot1 > 16 )MRot1 = 16;
		int DR2 = 0;
		if ( NeedState == 0 && AttGroundMod )
		{
			DR2 = ( ( ADDRANGE - 100 )*ADC->AttackRadius2[NeedState] ) / 100;
		};
		if ( dst1 >= ADC->AttackRadius1[NeedState] && dst < ADC->AttackRadius2[NeedState] + DR2 )
		{

			//can attack at once
			//checking angle of rotation
			char EnDir = char( GetDir( x0 - OBJ->RealX, y0 - OBJ->RealY ) );
			char ddir = EnDir - OBJ->RealDir;
			if ( abs( ddir ) < MRot1 )
			{

				OBJ->RealDir = EnDir;
				OBJ->GraphDir = EnDir;
				if ( OBJ->NewState == NeedState + 1 )
				{

					if ( !DELAY )
					{

						//checking attack ability
						Weapon* WP = NMN->DamWeap[OBJ->NewState - 1];
						if ( WP )
						{
							Coor3D C3D;
							word DestObj = 0xFFFF;
							if ( OB->GetDamagePoint( &C3D, 0 ) )DestObj = OID;
							if ( OB->Wall )DestObj = OID;
							//OBJ->BlockUnit();
							int x0, y0, z0, x1, y1, z1;
							x0 = ( OBJ->RealX >> 4 );
							y0 = ( OBJ->RealY >> 4 );
							z0 = OBJ->RZ + NMN->SrcZPoint;
							x1 = C3D.x;
							y1 = C3D.y;
							z1 = C3D.z;
							int WRes = PredictShot( WP, x0, y0, z0, x1, y1, z1, OBJ->Index );
							if ( WRes != -1 )
							{
								//something on the way...
								if ( WRes >= ULIMIT )
								{
									//ground
									if ( OBJ->NNUM == MyNation )
									{
										//char atts[128];
										//sprintf( atts, MESHPOV, OBJ->newMons->Message );
										//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: The terrain profile prevents you from shooting.
									}
								}
								else
								{
									if ( WRes == OB->Index )
										goto DoShot1;

									OneObject* OBX = Group[WRes];
									if ( OBX )
									{
										if ( ShotRecommended && !( OBX->NMask&OBJ->NMask ) )
											goto DoShot1;

										if ( OBJ->NNUM == MyNation )
										{
											//char atts[128];
											//sprintf( atts, MESHOBJ, OBJ->newMons->Message, OBX->newMons->Message );
											//CreateTimedHint( atts, kMinorMessageDisplayTime );//%s: %s prevents you from shooting.
										}
									}
								}
								//attempt to move

								if ( !OBJ->StandGround )
								{
									int dxx = y1 - y0;
									int dyy = x0 - x1;
									int nn = Norma( dxx, dyy );
									dxx = ( dxx * 20 ) / nn;
									dyy = ( dyy * 20 ) / nn;
									bool LAllow = true;
									bool RAllow = true;
									int XL = x0;
									int YL = y0;
									int XR = x0;
									int YR = y0;
									for ( int j = 0; j < 10 && ( LAllow || RAllow ); j++ )
									{
										XL += dxx;
										YL += dyy;
										XR -= dxx;
										YR -= dyy;
										if ( CheckPt( XL >> 4, YL >> 4 ) )LAllow = false;
										if ( CheckPt( XR >> 4, YR >> 4 ) )RAllow = false;
										if ( LAllow&&PredictShot( WP, XL, YL, z0, x1, y1, z1, OBJ->Index ) == -1 )
										{
											OBJ->NewMonsterSendTo( ( XL + dxx ) << 4, ( YL + dyy ) << 4, 128 + 16, 1 );
											return;
										};
										if ( RAllow&&PredictShot( WP, XR, YR, z0, x1, y1, z1, OBJ->Index ) == -1 )
										{
											OBJ->NewMonsterSendTo( ( XR - dxx ) << 4, ( YR - dyy ) << 4, 128 + 16, 1 );
											return;
										};
										if ( CheckPt( XL >> 4, YL >> 4 ) )LAllow = false;
										if ( CheckPt( XR >> 4, YR >> 4 ) )RAllow = false;
									}
									if ( CANSETDELAY )
									{
										OBJ->delay = 20;
										OBJ->MaxDelay = 20;
										if ( OBJ->delay > OBJ->MaxDelay )
										{
											OBJ->delay = OBJ->MaxDelay;
										}

									}
									OBJ->DeleteLastOrder();
									return;
								}
								else
								{
									OBJ->DeleteLastOrder();
									return;
								}
							}
						DoShot1:;
							if ( ( NMN->ResAttType == OBJ->NewState - 1 || NMN->ResAttType1 == OBJ->NewState - 1 ) && NMN->NShotRes )
							{
								bool AllowShot = true;
								for ( int k = 0; k < NMN->NShotRes; k++ )
								{
									if ( XRESRC( OBJ->NNUM, NMN->ShotRes[k + k] ) < NMN->ShotRes[k + k + 1] )AllowShot = false;
								};
								if ( AllowShot )
								{
									for ( int k = 0; k < NMN->NShotRes; k++ )
									{
										AddXRESRC( OBJ->NNUM, NMN->ShotRes[k + k], -NMN->ShotRes[k + k + 1] );
										OBJ->Nat->AddResource( byte( NMN->ShotRes[k + k] ), -NMN->ShotRes[k + k + 1] );
										OBJ->Nat->ResOnLife[NMN->ShotRes[k + k]] += NMN->ShotRes[k + k + 1];
									};
								}
								else
								{
									if ( OBJ->NNUM == MyNation&&OBJ->PrioryLevel >= 16 )
									{
										char buf[150];
										sprintf( buf, GetTextByID( "CSNORS" ), OBJ->Ref.General->Message );
										CreateTimedHintEx( buf, kImportantMessageDisplayTime, 32 );//You have neither coal nor iron. %s cannot shoot.
									};
									OBJ->DeleteLastOrder();
									return;
								};
							};
						};
						if ( !DELAY )
						{
							OBJ->NewAnm = &NMN->Attack[OBJ->NewState - 1];
							OBJ->InMotion = false;
							OBJ->BackMotion = false;
							OBJ->NewCurSprite = 0;
							return;
						}
						else
						{

							TryToStand( OBJ, 0 );
						};
					};
				}
				else
				{
					OBJ->NewState = NeedState + 1;
					return;
				};
			}
			else
			{

				if ( !AMove )
				{

					if ( abs( ddir ) > MRot1 )
					{
						OBJ->NewState = OBJ->GroundState;

						TryToStand( OBJ, false );
					};
					if ( ddir > 0 )RotateMon( OBJ, MRot );//OBJ->RealDir+=16;
					else RotateMon( OBJ, -MRot );//OBJ->RealDir-=16;
				}
				else
				{

					if ( ddir > 0 )RotateMon( OBJ, MRot );//OBJ->RealDir+=16;
					else RotateMon( OBJ, -MRot );//OBJ->RealDir-=16;
				};
			};
		}
		else
		{

			if ( OBJ->StandGround )
			{

				OBJ->DeleteLastOrder();
				return;
			};
			if ( dst > ADC->AttackRadius2[NeedState] + DR2 )
			{

				Order1* OR1 = OB->LocalOrder;
				if ( OBJ->Zombi&&OB->InMotion && ( OB->EnemyID == 0xFFFF ) )
				{

					int dde = GetDir( OB->RealX - OBJ->RealX, OB->RealY - OBJ->RealY );
					char ddr = OB->RealDir - dde;
					if ( abs( ddr ) < 64 )
					{

						OBJ->DeleteLastOrder();
						return;
					};
				};
				if ( AMove )
				{

					if ( OBJ->NewState != NeedState + 1 )
					{

						OBJ->NewState = NeedState + 1;
					}
					else
					{

						SetDestUnit( OBJ, OB, OLDX, OLDY, NEWX, NEWY );
					};
				}
				else
				{

					if ( OBJ->NewState != OBJ->GroundState )
					{

						OBJ->NewState = OBJ->GroundState;

						TryToStand( OBJ, false );
					}
					else
					{

						SetDestUnit( OBJ, OB, OLDX, OLDY, NEWX, NEWY );
					};
				};
				return;
			}
			else
			{

				if ( !OBJ->BackSteps )
				{
					if ( OBJ->BackReserv < 32 )
					{

						OBJ->BackReserv++;
						OBJ->PrioryLevel = 0;

						TryToStand( OBJ, false );
						OBJ->DestX = -1;
						OBJ->DestY = -1;
						//OBJ->SearchVictim();
						return;
					}
					else
					{
						OBJ->BackSteps = 11;
						OBJ->BackReserv = 0;
					};
				};
				char EnDir = char( GetDir( x0 - OBJ->RealX, y0 - OBJ->RealY ) );
				int dds = ADC->AttackRadius1[NeedState] - dst1;
				if ( dds > 16 )
				{
					char Endr = EnDir + 128;
					char ddir = Endr - OBJ->RealDir;
					if ( abs( ddir ) < MRot1 )
					{
						OBJ->RealDir = Endr;
						OBJ->GraphDir = Endr;
						SmartTryToMove( OBJ, OBJ->RealDir, false );
						OBJ->BackSteps--;
						return;
					}
					else
					{
						if ( ddir > 0 )RotateMon( OBJ, MRot );
						else RotateMon( OBJ, -MRot );
						return;
					};
				}
				else
				{
					char Endr = EnDir + 128;
					char ddir1 = Endr - OBJ->RealDir;
					char ddir = EnDir - OBJ->RealDir;
					if ( abs( ddir1 ) < abs( ddir ) )
					{
						//forward
						if ( abs( ddir1 ) < MRot1 )
						{
							OBJ->RealDir = Endr;
							OBJ->GraphDir = Endr;
							SmartTryToMove( OBJ, OBJ->RealDir, false );
							OBJ->BackSteps--;
							return;
						}
						else
						{
							if ( ddir1 > 0 )RotateMon( OBJ, MRot );
							else RotateMon( OBJ, -MRot );
							return;
						}
					}
					else
					{
						//back
						if ( abs( ddir ) < MRot1 )
						{
							OBJ->RealDir = EnDir;
							OBJ->GraphDir = EnDir;
							SmartTryToMove( OBJ, OBJ->RealDir, true );
							OBJ->BackSteps--;
							return;
						}
						else
						{
							if ( ddir > 0 )RotateMon( OBJ, MRot );
							else RotateMon( OBJ, -MRot );
							return;
						}
					}
				}
			}
		}
	}
}

void MakeOneShotLink( OneObject* OBJ )
{
	int cf = OBJ->NewCurSprite >> SpeedSh;
	int ns = OBJ->NewState;
	if ( !ns )
	{
		OBJ->DeleteLastOrder();
		OBJ->NewState = 4;
		OBJ->LocalNewState = 4;
		OBJ->StandTime = 20;
		return;
	};
	NewMonster* NM = OBJ->newMons;
	NewAnimation* NMA = &NM->Attack[ns - 1];
	if ( OBJ->NewAnm != NMA )
	{
		OBJ->DeleteLastOrder();
		OBJ->NewState = 4;
		OBJ->LocalNewState = 4;
		OBJ->StandTime = 20;
		return;
	};
	int hf = NMA->ActiveFrame >> SpeedSh;;

	if ( cf > hf + 1 )
	{
		OBJ->DeleteLastOrder();
		OBJ->NewState = 4;
		OBJ->LocalNewState = 4;
		OBJ->StandTime = 20;
		return;
	};

	if ( cf >= hf )
	{
		Weapon* WP = NM->DamWeap[OBJ->NewState - 1];
		if ( WP )
		{
			NewAnimation* UNAM = OBJ->NewAnm;
			int csp = OBJ->NewCurSprite;
			int uocts = ( UNAM->Rotations - 1 ) * 2;
			int uoc2 = UNAM->Rotations - 1;
			if ( !uocts )uocts = 1;
			int usesize = div( 255, uocts << 1 ).quot;
			byte udir = ( ( ( OBJ->RealDir + 64 + usesize ) & 255 )*uocts ) >> 8;
			NewFrame* NF = &UNAM->Frames[csp];
			//----------
			NewAnimation* NAM = WP->NewAnm;
			int octs = ( NAM->Rotations - 1 ) * 2;
			int oc2 = NAM->Rotations - 1;
			if ( !octs )octs = 1;
			int sesize = div( 255, octs << 1 ).quot;
			byte dir = ( ( ( OBJ->RealDir + 64 + sesize ) & 255 )*octs ) >> 8;
			int x0, y0, z0, x1, y1, z1;
			if ( udir < uoc2 )
			{
				int udir1 = uoc2 - udir;
				x0 = ( OBJ->RealX >> 4 ) - ( UNAM->ActivePtX[udir1] + NF->dx );
				y0 = ( OBJ->RealY >> 4 ) + Prop43( UNAM->ActivePtY[udir1] + NF->dy + NM->SrcZPoint );
			}
			else
			{
				int udir1 = uoc2 - uocts + udir;
				x0 = ( OBJ->RealX >> 4 ) + ( UNAM->ActivePtX[udir1] + NF->dx );
				y0 = ( OBJ->RealY >> 4 ) + Prop43( UNAM->ActivePtY[udir1] + NF->dy + NM->SrcZPoint );
			};
			z0 = OBJ->RZ + NM->SrcZPoint;
			x1 = int( OBJ->LocalOrder->info.AttackXY.x ) << 2;
			y1 = int( OBJ->LocalOrder->info.AttackXY.y ) << 2;
			z1 = int( OBJ->LocalOrder->info.AttackXY.z );
			if ( udir < uoc2 )
			{
				udir = uoc2 - udir;
				Create3DAnmObject( WP, x0, y0, z0, x1, y1, z1, OBJ, OBJ->NewState - 1, 0xFFFF );
			}
			else
			{
				udir = uoc2 - uocts + udir;
				Create3DAnmObject( WP, x0, y0, z0, x1, y1, z1, OBJ, OBJ->NewState - 1, 0xFFFF );
			};
			OBJ->delay = OBJ->Ref.General->MoreCharacter->AttackPause[OBJ->NewState - 1];
			OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
			OBJ->delay = OBJ->MaxDelay;

			OBJ->DeleteLastOrder();
			OBJ->NewState = 4;
			OBJ->LocalNewState = 4;
			OBJ->StandTime = 20;
		};
	};
};
void MakeOneShot( OneObject* OB, int xd, int yd, int zd )
{
	Order1* OR1 = OB->CreateOrder( 1 );
	OR1->info.AttackXY.ox = OB->Index;
	OR1->info.AttackXY.x = xd >> 2;
	OR1->info.AttackXY.y = yd >> 2;
	OR1->info.AttackXY.z = zd;
	OR1->DoLink = &MakeOneShotLink;
	OR1->PrioryLevel = 255;
	OB->PrioryLevel = 255;
	MakeOneShotLink( OB );
};
void NewMonsterSmartSendToLink( OneObject* OBJ );
int SmartLink = 0x153829;
void __stdcall CDGINIT_INIT1()
{
	SmartLink = int( &NewMonsterSmartSendToLink ) + 0x7654391;
};
void WaterAttackLink( OneObject* OBJ )
{
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	int NTimes = OBJ->LocalOrder->info.BuildObj.SN;
	bool ai = OBJ->Nat->AI_Enabled;
	NewMonster* NMN = OBJ->newMons;
	if ( !NMN->NMultiWp )
	{
		OBJ->DeleteLastOrder();
		return;
	};
	AdvCharacter* ADC = OBJ->Ref.General->MoreCharacter;
	OBJ->DestX = -1;
	//OBJ->UnBlockUnit();
	word OID = OBJ->EnemyID;
	word OSN = OBJ->EnemySN;
	OneObject* OB = Group[OID];
	if ( !OB || OB->Sdoxlo || OBJ->TurnOff || OSN != OB->Serial )
	{
		if ( OBJ->PrioryLevel >= 16 && OBJ->NNUM == MyNation&&OBJ->TurnOff )
		{
			char buf[128];
			sprintf( buf, GetTextByID( "CSNOGL" ), OBJ->Ref.General->Message );
			CreateTimedHintEx( buf, kImportantMessageDisplayTime, 32 );//You have no gold. %s cannot shoot.
		};
		OBJ->DeleteLastOrder();
		return;
	};
	int dst = Norma( OBJ->RealX - OB->RealX, OBJ->RealY - OB->RealY ) >> 4;
	//detecting attack method
	int AttMethod = -1;
	for ( int i = 0; i < NAttTypes; i++ )
	{
		if ( ADC->DetRadius1[i] <= dst&&ADC->DetRadius2[i] >= dst )AttMethod = i;
	};
	if ( AttMethod != -1 )
	{
		byte ATM = OBJ->LocalOrder->info.BuildObj.AttMethod;
		if ( ATM != 0xFF )
		{
			if ( !OB->NewBuilding )AttMethod = ATM;
		};
		//if(OB->StandTime>5)dst+=90;
		//checking actual distance
		if ( dst >= ADC->AttackRadius1[AttMethod] && dst <= ADC->AttackRadius2[AttMethod] )
		{
			/*if(ai&&OBJ->delay>30&&(((tmtmt/70>>7)%6)==3)&&dst>ADC->AttackRadius2[AttMethod]-400){
				int ux=OBJ->x;
				int uy=OBJ->y;
				int DX=OB->x>ux?1:-1;
				int DY=OB->x>uy?1:-1;
				int LX=OBJ->Lx;
				UnitsField.BClrBar(ux,uy,LX);
				if((UnitsField.CheckBar(ux+DX,uy,LX,LX)||MFIELDS[1].CheckBar(ux+DX,uy,LX,LX))&&
					(UnitsField.CheckBar(ux,uy+DY,LX,LX)||MFIELDS[1].CheckBar(ux,uy+DY,LX,LX))){
					UnitsField.BSetBar(ux,uy,LX);
				}else{
					UnitsField.BSetBar(ux,uy,LX);
					OBJ->CreatePath(OB->x,OB->y);
				};
			}else*/
			{
				OBJ->DeletePath();
				//checking rotation
				char endir = char( GetDir( OB->RealX - OBJ->RealX, OB->RealY - OBJ->RealY ) );
				char RBort = OBJ->RealDir - 64;
				char LBort = OBJ->RealDir + 64;
				if ( NMN->ShotForward )
				{
					RBort = OBJ->RealDir;
					LBort = RBort;
				};
				char dL = LBort - endir;
				char dR = RBort - endir;
				bool MakeShot = false;
				int ShotX = 0;
				int ShotY = 0;
				if ( abs( dL ) <= abs( dR ) )
				{
					//shot by the left side
					if ( abs( endir - LBort ) < 8 )
					{
						//pubuhhhhhh!!!!!!!!!!
						if ( !OBJ->delay )
						{
							bool AllowShot = true;
							if ( NMN->NShotRes )
							{
								for ( int k = 0; k < NMN->NShotRes; k++ )
								{
									if ( XRESRC( OBJ->NNUM, NMN->ShotRes[k + k] ) < NMN->ShotRes[k + k + 1] )AllowShot = false;
								};
								if ( AllowShot )
								{
									for ( int k = 0; k < NMN->NShotRes; k++ )
									{
										AddXRESRC( OBJ->NNUM, NMN->ShotRes[k + k], -NMN->ShotRes[k + k + 1] );
										OBJ->Nat->AddResource( byte( NMN->ShotRes[k + k] ), -NMN->ShotRes[k + k + 1] );
										OBJ->Nat->ResOnLife[NMN->ShotRes[k + k]] += NMN->ShotRes[k + k + 1];
									};
								}
								else
								{
									if ( OBJ->NNUM == MyNation&&OBJ->PrioryLevel >= 16 )
									{
										char buf[150];
										sprintf( buf, GetTextByID( "CSNORS" ), OBJ->Ref.General->Message );
										CreateTimedHintEx( buf, kImportantMessageDisplayTime, 32 );//You have neither coal nor iron. %s cannot shoot.
									};
									OBJ->DeleteLastOrder();
									return;
								};
							};
							if ( AllowShot )
							{
								if ( NMN->ShotForward )
								{
									int pus = ( int( NMN->NMultiWp )*int( rando() ) ) >> 15;
									int vx = NMN->MultiWpX[pus];
									int vy = NMN->MultiWpY[pus];
									int Dang = ( OBJ->RealDir + 192 ) & 255;
									int SIN = TSin[Dang];
									int COS = TCos[Dang];
									ShotX = ( ( vx*COS - vy*SIN ) >> 8 );
									ShotY = ( ( vx*SIN + vy*COS ) >> 8 );
									if ( NMN->MaxZalp&&OBJ->NZalp )
									{
										OBJ->NZalp--;
									}
									else
									{
										OBJ->NZalp = NMN->MaxZalp;
										OBJ->delay = NMN->AttackPause[AttMethod];
										OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
										OBJ->delay = OBJ->MaxDelay;
									};
									MakeShot = true;
								}
								else
								{
									int pus = ( int( NMN->NMultiWp )*int( rando() ) ) >> 15;
									int vx = NMN->MultiWpX[pus];
									int vy = NMN->MultiWpY[pus];
									int Dang = ( OBJ->RealDir + 128 ) & 255;
									int SIN = TSin[Dang];
									int COS = TCos[Dang];
									ShotX = ( ( vx*COS + vy*SIN ) >> 8 );
									ShotY = ( ( vx*SIN - vy*COS ) >> 8 );
									if ( NMN->MaxZalp&&OBJ->NZalp )
									{
										OBJ->NZalp--;
									}
									else
									{
										OBJ->NZalp = NMN->MaxZalp;
										OBJ->delay = NMN->AttackPause[AttMethod];
										OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
										OBJ->delay = OBJ->MaxDelay;
									};
									MakeShot = true;
								};
							};
						};
					}
					else
					{
						if ( dL > 0 )OBJ->RealDir -= 2;
						else OBJ->RealDir += 2;
						OBJ->GraphDir = OBJ->RealDir;
					};
				}
				else
				{
					//shot by the right side
					if ( abs( dR ) < 8 )
					{
						//pubuhhhhhh!!!!!!!!!!
						if ( !OBJ->delay )
						{
							bool AllowShot = true;
							if ( NMN->NShotRes )
							{
								for ( int k = 0; k < NMN->NShotRes; k++ )
								{
									if ( XRESRC( OBJ->NNUM, NMN->ShotRes[k + k] ) < NMN->ShotRes[k + k + 1] )AllowShot = false;
								};
							};
							if ( AllowShot )
							{
								for ( int k = 0; k < NMN->NShotRes; k++ )
								{
									AddXRESRC( OBJ->NNUM, NMN->ShotRes[k + k], -NMN->ShotRes[k + k + 1] );
									OBJ->Nat->AddResource( byte( NMN->ShotRes[k + k] ), -NMN->ShotRes[k + k + 1] );
									OBJ->Nat->ResOnLife[NMN->ShotRes[k + k]] += NMN->ShotRes[k + k + 1];
								};
								int pus = ( int( NMN->NMultiWp )*int( rando() ) ) >> 15;
								int vx = NMN->MultiWpX[pus];
								int vy = NMN->MultiWpY[pus];
								int Dang = ( OBJ->RealDir + 128 ) & 255;
								int SIN = TSin[Dang];
								int COS = TCos[Dang];
								ShotX = ( ( vx*COS - vy*SIN ) >> 8 );
								ShotY = ( ( vx*SIN + vy*COS ) >> 8 );
								if ( NMN->MaxZalp&&OBJ->NZalp )
								{
									OBJ->NZalp--;
								}
								else
								{
									OBJ->NZalp = NMN->MaxZalp;
									OBJ->delay = NMN->AttackPause[AttMethod];
									OBJ->MaxDelay = div( int( OBJ->delay )*OBJ->PersonalDelay, 100 ).quot;
									OBJ->delay = OBJ->MaxDelay;
								};
								MakeShot = true;
							}
							else
							{
								//OBJ->delay=20;
								if ( OBJ->NNUM == MyNation&&OBJ->PrioryLevel >= 16 )
								{
									char buf[150];
									sprintf( buf, GetTextByID( "CSNORS" ), OBJ->Ref.General->Message );
									CreateTimedHintEx( buf, kImportantMessageDisplayTime, 32 );//You have neither coal nor iron. %s cannot shoot.
								};
								OBJ->DeleteLastOrder();
								return;
							};
						};
					}
					else
					{
						if ( dR > 0 )OBJ->RealDir -= 2;
						else OBJ->RealDir += 2;
						OBJ->GraphDir = OBJ->RealDir;
					};
				};
				if ( MakeShot )
				{
					Weapon* WP = NMN->DamWeap[AttMethod];
					if ( WP )
					{
						Coor3D C3D;
						word DestObj = 0xFFFF;
						if ( OB->GetDamagePoint( &C3D, 0 ) )DestObj = OID;
						if ( OB->Wall )DestObj = OID;
						CreateRazbros( OBJ, &C3D );

						Create3DAnmObject( WP, ( OBJ->RealX >> 4 ) + ShotX, ( OBJ->RealY >> 4 ) + ShotY,/*OBJ->RZ*/+NMN->MultiWpZ,
							C3D.x, C3D.y, C3D.z, OBJ, AttMethod, 0xFFFF );
						if ( NTimes == 1 )
						{
							OBJ->DeleteLastOrder();
						}
						else
						{
							if ( NTimes > 1 )OBJ->LocalOrder->info.BuildObj.SN--;
						};
					};
				};
			};
		}
		else
		{
			if ( dst > NMN->AttackRadius2[AttMethod] )
			{
				if ( NTimes > 0 )
				{
					OBJ->DeleteLastOrder();
					return;
				};
				if ( !OBJ->StandGround )
				{
					OBJ->CreatePath( OB->x, OB->y );
				}
				else OBJ->DeleteLastOrder();
				//OBJ->DeleteLastOrder();
			}
			else
			{
				OBJ->DeleteLastOrder();
			};
		};
	}
	else OBJ->DeleteLastOrder();
};
//-----------------------Search for the enemy------------------------
word GetNewEnemy( int xr, int yr, byte NI )
{
	NI = NatRefTBL[NI];
	byte nms = 1 << NI;
	OneObject* OBX = nullptr;
	for ( int ii = 0; ii < MAXOBJECT; ii++ )
	{
		OneObject* OB = Group[ii];
		if ( OB && ( !( ( nms&OB->NMask ) || OB->Sdoxlo ) ) )
		{
			int xs, ys, xs1, ys1, LY;
			GetRect( OB, &xs, &ys, &xs1, &LY );
			ys1 = ys + LY - 1;
			xs1 += xs - 1;
			if ( PInside( xs, ys, xs1, ys1, xr, yr ) )
			{
				if ( ( !OBX ) || OBX->NewBuilding )
				{
					OBX = Group[ii];
				};
			};
		};
	};
	if ( OBX )return OBX->Index;
	else return 0xFFFF;
};
word AdvancedGetNewEnemyInCell( int cx, int cy, int xr, int yr, byte EMask )
{
	if ( cx < 0 || cy < 0 || cx >= VAL_MAXCX || cy >= VAL_MAXCX )return 0xfFFF;
	int cell = VAL_MAXCX + 1 + cx + ( cy << VAL_SHFCX );
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && ( !( OB->NMask&EMask ) ) && !OB->Sdoxlo )
			{
				int xs, ys, xs1, ys1, LY;
				GetRect( OB, &xs, &ys, &xs1, &LY );
				ys1 = ys + LY - 1;
				xs1 += xs - 1;
				if ( OB->InMotion && ( !OB->LockType ) )
				{
					int dx = OB->newMons->OneStepDX[byte( OB->RealDir )] >> 2;
					int dy = OB->newMons->OneStepDY[byte( OB->RealDir )] >> 2;
					for ( int j = 0; j < 3; j++ )
					{
						if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
						xs -= dx;
						ys -= dy;
						xs1 -= dx;
						ys1 -= dy;
					};
				};
				if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
			};
		};
	};
	MID = BLDList[cell];
	if ( MID != 0xFFFF )
	{
		OneObject* OB = Group[MID];
		if ( OB && ( !( OB->NMask&EMask ) ) && !OB->Sdoxlo )
		{
			int xs, ys, xs1, ys1, LY;
			GetRect( OB, &xs, &ys, &xs1, &LY );
			ys1 = ys + LY - 1;
			xs1 += xs - 1;
			if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
		};
	};
	//search for wall
	int xx1 = cx << 1;
	int yy1 = cy << 1;
	for ( int dx = 0; dx < 2; dx++ )for ( int dy = 0; dy < 2; dy++ )
	{
		int LI = GetLI( xx1 + dx, yy1 + dy );
		WallCell* WCL = WRefs[LI];
		if ( WCL )
		{
			word MID = WCL->OIndex;
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && ( !( OB->NMask&EMask ) ) && !OB->Sdoxlo )
				{
					int xs, ys, xs1, ys1, LY;
					GetRect( OB, &xs, &ys, &xs1, &LY );
					ys1 = ys + LY - 1;
					xs1 += xs - 1;
					if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
				};
			};
		};
	};
	return 0xFFFF;
};
word AdvancedGetNewEnemy( int rex, int rey, int xr, int yr, byte NI )
{
	byte nms = NATIONS[NI].NMask;
	int x0 = rex >> 7;
	int y0 = rey >> 7;
	for ( int ix = -2; ix < 3; ix++ )
	{
		for ( int iy = -2; iy < 3; iy++ )
		{
			word MID = AdvancedGetNewEnemyInCell( x0 + ix, y0 + iy, xr, yr, nms );
			if ( MID != 0xFFFF )return MID;
		};
	};
	return 0xFFFF;
};
word GetNewFriend( int xr, int yr, byte NI )
{
	byte nms = 1 << NI;
	for ( int ii = 0; ii < MAXOBJECT; ii++ )
	{
		OneObject* OB = Group[ii];
		if ( OB&&OB->NNUM == NI && !OB->Sdoxlo )
		{
			int xs, ys, xs1, ys1, LY;
			GetRect( OB, &xs, &ys, &xs1, &LY );
			ys1 = ys + LY - 1;
			xs1 += xs - 1;
			if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return ii;
		};
	};
	return 0xFFFF;
};
word AdvancedGetNewFriendInCell( int cx, int cy, int xr, int yr, byte NI )
{
	if ( cx < 0 || cy < 0 || cx >= VAL_MAXCX || cy >= VAL_MAXCX )return 0xfFFF;
	int cell = VAL_MAXCX + 1 + cx + ( cy << VAL_SHFCX );
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->NNUM == NI && !OB->Sdoxlo )
			{
				int xs, ys, xs1, ys1, LY;
				GetRect( OB, &xs, &ys, &xs1, &LY );
				ys1 = ys + LY - 1;
				xs1 += xs - 1;
				if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
			};
		};
	};
	MID = BLDList[cell];
	if ( MID != 0xFFFF )
	{
		OneObject* OB = Group[MID];
		if ( OB&&OB->NNUM == NI && !OB->Sdoxlo )
		{
			int xs, ys, xs1, ys1, LY;
			GetRect( OB, &xs, &ys, &xs1, &LY );
			ys1 = ys + LY - 1;
			xs1 += xs - 1;
			if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
		};
	};
	int xx1 = cx << 1;
	int yy1 = cy << 1;
	for ( int dx = 0; dx < 2; dx++ )for ( int dy = 0; dy < 2; dy++ )
	{
		int LI = GetLI( xx1 + dx, yy1 + dy );
		WallCell* WCL = WRefs[LI];
		if ( WCL )
		{
			word MID = WCL->OIndex;
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->NNUM == NI && !OB->Sdoxlo )
				{
					int xs, ys, xs1, ys1, LY;
					GetRect( OB, &xs, &ys, &xs1, &LY );
					ys1 = ys + LY - 1;
					xs1 += xs - 1;
					if ( PInside( xs, ys, xs1, ys1, xr, yr ) )return MID;
				};
			};
		};
	};
	return 0xFFFF;
};
word AdvancedGetNewFriend( int rex, int rey, int xr, int yr, byte NI )
{
	NI = NatRefTBL[NI];
	int x0 = rex >> 7;
	int y0 = rey >> 7;
	for ( int ix = -2; ix < 3; ix++ )
	{
		for ( int iy = -2; iy < 3; iy++ )
		{
			word MID = AdvancedGetNewFriendInCell( x0 + ix, y0 + iy, xr, yr, NI );
			if ( MID != 0xFFFF )return MID;
		};
	};
	return 0xFFFF;
};
//------------------Block&UnBlock---------------------//
void OneObject::WeakBlockUnit()
{
	if ( BlockInfo )return;
	/*
	if(!CheckPt(x,y)){
		BlockInfo|=1;
		BSetPt(x,y);
	};
	if(!CheckPt(x,y-1)){
		BlockInfo|=2;
		BSetPt(x-1,y-1);
	};
	if(!CheckPt(x,y+1)){
		BlockInfo|=4;
		BSetPt(x,y+1);
	};
	if(!CheckPt(x-1,y)){
		BlockInfo|=8;
		BSetPt(x+1,y);
	};
	if(!CheckPt(x-1,y)){
		BlockInfo|=16;
		BSetPt(x-1,y);
	};
	return;
	//--------------------
	*/
	/*if(!CheckPt(x-1,y-1)){
		BlockInfo|=1;
		BSetPt(x-1,y-1);
	};
	if(!CheckPt(x,y-1)){
		BlockInfo|=2;
		BSetPt(x,y-1);
	};
	if(!CheckPt(x+1,y-1)){
		BlockInfo|=4;
		BSetPt(x+1,y-1);
	};
	if(!CheckPt(x-1,y)){
		BlockInfo|=8;
		BSetPt(x-1,y);
	};*/
	if ( !CheckPt( x, y ) )
	{
		BlockInfo |= 16;
		BSetPt( x, y );
	};
	/*
	if(!CheckPt(x+1,y)){
		BlockInfo|=32;
		BSetPt(x+1,y);
	};
	if(!CheckPt(x-1,y+1)){
		BlockInfo|=64;
		BSetPt(x-1,y+1);
	};
	if(!CheckPt(x,y+1)){
		BlockInfo|=128;
		BSetPt(x,y+1);
	};
	if(!CheckPt(x+1,y+1)){
		BlockInfo|=256;
		BSetPt(x+1,y+1);
	};*/
};
void OneObject::BlockUnit()
{
	if ( BlockInfo )return;
	/*
	if(!CheckPt(x,y)){
		BlockInfo|=1;
		BSetPt(x,y);
	};
	if(!CheckPt(x,y-1)){
		BlockInfo|=2;
		BSetPt(x-1,y-1);
	};
	if(!CheckPt(x,y+1)){
		BlockInfo|=4;
		BSetPt(x,y+1);
	};
	if(!CheckPt(x-1,y)){
		BlockInfo|=8;
		BSetPt(x+1,y);
	};
	if(!CheckPt(x-1,y)){
		BlockInfo|=16;
		BSetPt(x-1,y);
	};
	return;
	//--------------------
	*/
	if ( !CheckPt( x - 1, y - 1 ) )
	{
		BlockInfo |= 1;
		BSetPt( x - 1, y - 1 );
	};
	if ( !CheckPt( x, y - 1 ) )
	{
		BlockInfo |= 2;
		BSetPt( x, y - 1 );
	};
	if ( !CheckPt( x + 1, y - 1 ) )
	{
		BlockInfo |= 4;
		BSetPt( x + 1, y - 1 );
	};
	if ( !CheckPt( x - 1, y ) )
	{
		BlockInfo |= 8;
		BSetPt( x - 1, y );
	};
	if ( !CheckPt( x, y ) )
	{
		BlockInfo |= 16;
		BSetPt( x, y );
	};
	if ( !CheckPt( x + 1, y ) )
	{
		BlockInfo |= 32;
		BSetPt( x + 1, y );
	};
	if ( !CheckPt( x - 1, y + 1 ) )
	{
		BlockInfo |= 64;
		BSetPt( x - 1, y + 1 );
	};
	if ( !CheckPt( x, y + 1 ) )
	{
		BlockInfo |= 128;
		BSetPt( x, y + 1 );
	};
	if ( !CheckPt( x + 1, y + 1 ) )
	{
		BlockInfo |= 256;
		BSetPt( x + 1, y + 1 );
	};
};
void OneObject::UnBlockUnit()
{
	if ( !BlockInfo )return;
	/*if(BlockInfo&1)BClrPt(x,y);
	if(BlockInfo&2)BClrPt(x,y-1);
	if(BlockInfo&4)BClrPt(x,y+1);
	if(BlockInfo&8)BClrPt(x+1,y);
	if(BlockInfo&16)BClrPt(x-1,y);
	return;*/
	if ( BlockInfo & 1 )BClrPt( x - 1, y - 1 );
	if ( BlockInfo & 2 )BClrPt( x, y - 1 );
	if ( BlockInfo & 4 )BClrPt( x + 1, y - 1 );
	if ( BlockInfo & 8 )BClrPt( x - 1, y );
	if ( BlockInfo & 16 )BClrPt( x, y );
	if ( BlockInfo & 32 )BClrPt( x + 1, y );
	if ( BlockInfo & 64 )BClrPt( x - 1, y + 1 );
	if ( BlockInfo & 128 )BClrPt( x, y + 1 );
	if ( BlockInfo & 256 )BClrPt( x + 1, y + 1 );
	BlockInfo = 0;
};
void OneObject::DeletePath()
{
	if ( PathX )
	{
		free( PathX );
		free( PathY );
		PathX = nullptr;
		PathY = nullptr;
		CPdestX = (byte) -1;
		NIPoints = 0;
		CurIPoint = 0;

	};
};
//Creation the building
bool OneObject::FindPoint( int* x1, int* y1, byte Flags )
{
	NewMonster* NM = newMons;
	char* px = nullptr;
	char* py = nullptr;
	int Npt = 0;
	if ( Flags&FP_FIND_WORKPOINT )
	{
		px = NM->BuildPtX;
		py = NM->BuildPtY;
		Npt = NM->NBuildPt;
	};
	if ( Flags&FP_FIND_DAMPOINT )
	{
		px = NM->DamPtX;
		py = NM->DamPtY;
		Npt = NM->NDamPt;
	};
	if ( Flags&FP_CONCENTRATION )
	{
		px = NM->ConcPtX;
		py = NM->ConcPtY;
		Npt = NM->NConcPt;
	};
	if ( !Npt )return false;
	if ( Flags&FP_NEAREST_POINT )
	{
		int xx = *x1;
		int yy = *y1;
		int dist = 10000;
		int dis;
		int PicSX = NM->PicDx << 4;
		int PicSY = Prop43( NM->PicDy ) << 4;
		int x0 = ( RealX + PicSX ) >> 8;
		int y0 = ( RealY + PicSY ) >> 8;
		int x2, y2;
		int x3 = 0;
		int y3 = 0;
		if ( Flags&FP_UNLOCKED_POINT )
		{
			for ( int i = 0; i < Npt; i++ )
			{
				x2 = x0 + px[i];
				y2 = y0 + py[i];
				dis = Norma( x2 - xx, y2 - yy );
				if ( dis < dist && !CheckBar( x2 - 1, y2 - 1, 3, 3 ) )
				{
					dist = dis;
					x3 = x2;
					y3 = y2;
				};
			};
		}
		else
		{
			for ( int i = 0; i < Npt; i++ )
			{
				x2 = x0 + px[i];
				y2 = y0 + py[i];
				dis = Norma( x2 - xx, y2 - yy );
				if ( dis < dist )
				{
					dist = dis;
					x3 = x2;
					y3 = y2;
				};
			};
		};
		if ( dist < 10000 )
		{
			*x1 = x3;
			*y1 = y3;
			return true;
		};
	};
	return false;
};
void CmdFieldBar( byte NI, word n );
void CreateFields( byte NI, int x, int y, int n );
void InvitePeasant( OneObject* Mine );
extern bool EditMapMode;
void OneObject::NextStage()
{
	if ( !NewBuilding || Life == MaxLife )return;
	NewMonster* NM = newMons;
	AdvCharacter* ACR = Ref.General->MoreCharacter;
	if ( Stage < ACR->ProduceStages )
	{
		Stage++;
		Life = div( Stage*MaxLife, ACR->ProduceStages ).quot;
		int nn = div( Stage << 2, ACR->ProduceStages ).quot;
		LoLayer = &NM->PMotionL[nn];
		HiLayer = &NM->PMotionR[nn];
		NewCurSprite = 0;
		if ( Stage == ACR->ProduceStages )
		{
			if ( NM->NBLockPt )
			{
				int xx, yy;
				this->GetCornerXY( &xx, &yy );
				for ( int i = 0; i < NM->NBLockPt; i++ )
				{
					BClrPt( xx + NM->BLockX[i], yy + NM->BLockY[i] );
				};
				for ( int i = 0; i < NM->NLockPt; i++ )
				{
					BSetPt( xx + NM->LockX[i], yy + NM->LockY[i] );
				};
			};
			Ready = true;
			LoLayer = &NM->StandLo;
			HiLayer = &NM->StandHi;
			NewAnm = &NM->Work;
			NewCurSprite = 0;
			byte lUsage = newMons->Usage;
			if ( lUsage == TowerID )GNFO.AddTow( this );
			if ( lUsage == MelnicaID && ( !Nat->AI_Enabled ) && Nat->CITY->ReadyAmount[NIndex] == 0 )
			{
				CreateFields( NNUM, RealX, RealY, Nat->PAble[NIndex][0] );
			};
			if ( lUsage == MineID && !EditMapMode )
			{

				InvitePeasant( this );
			};
			if ( NNUM == MyNation )
			{
				char ccc[200];
				sprintf( ccc, BLDBLD, Ref.General->Message );
				CreateTimedHint( ccc, kMinorMessageDisplayTime );//%s has been successfully built.
				LastActionX = RealX >> 4;
				LastActionY = RealY >> 4;
			};
			Nat->CITY->Account += newMons->Ves;
		};
	}
	else
	{
		Life += 20;
		LoLayer = &NM->StandLo;
		HiLayer = &NM->StandHi;
		NewAnm = &NM->Work;
		//NewCurSprite=0;
		if ( Life > MaxLife )Life = MaxLife;
		//Ready=true;
		if ( newMons->Usage == TowerID&&NNUM == 0 )GNFO.AddTow( this );
	};
};
void OneObject::ClearBuildPt()
{
	NewMonster* NM = newMons;
	char* px = NM->BuildPtX;
	char* py = NM->BuildPtY;
	int Npt = NM->NBuildPt;
	if ( !Npt )return;
	int PicSX = NM->PicDx << 4;
	int PicSY = Prop43( NM->PicDy ) << 4;
	int x0 = ( RealX + PicSX ) >> 8;
	int y0 = ( RealY + PicSY ) >> 8;
	for ( int i = 0; i < Npt; i++ )BClrPt( x0 + px[i], y0 + py[i] );
};
void BuildObjLink( OneObject* OBJ );
void TakeResLink( OneObject* OB );
bool OneObject::BuildObj( word OID, int Prio, bool LockPoint, byte OrdType )
{

	if ( CheckOrderAbility() )return false;
	if ( Prio < PrioryLevel&&Prio < 100 )return false;
	if ( LocalOrder&&LocalOrder->DoLink == &BuildObjLink&&LocalOrder->info.BuildObj.ObjIndex == OID )return true;
	NewMonster* NM = newMons;
	if ( !NM->Work.Enabled )return false;
	OneObject* OB = Group[OID];
	if ( !int( OB ) || OB->Sdoxlo )return false;
	if ( !OB->NewBuilding )return false;
	if ( !( OB->NMask&NMask ) )return false;
	if ( OB->Life == OB->MaxLife )return false;
	int xx = x;
	int yy = y;
	if ( OB->FindPoint( &xx, &yy, FP_NEAREST_POINT | FP_FIND_WORKPOINT | FP_UNLOCKED_POINT ) )
	{
		if ( OrdType == 0 || OrdType == 2 )NewMonsterSmartSendTo( xx << 4, yy << 4, 0, 0, Prio, OrdType );
		if ( OrdType == 2 && LocalOrder&&LocalOrder->DoLink == &TakeResLink )
		{
			DeleteLastOrder();
			OrdType = 1;
		};
		//Important=true;
		Order1* Or1;
		if ( !OrdType )Or1 = CreateOrder( 2 );
		else Or1 = CreateOrder( OrdType );
		if ( !int( Or1 ) )return false;
		Or1->OrderType = 89;//Build
		Or1->OrderTime = 0;
		Or1->DoLink = &BuildObjLink;
		Or1->info.BuildObj.ObjIndex = OID;
		Or1->info.BuildObj.ObjX = xx;
		Or1->info.BuildObj.ObjY = yy;
		Or1->info.BuildObj.SN = OB->Serial;
		Or1->PrioryLevel = Prio & 127;
		if ( LockPoint )BSetPt( xx, yy );
		//PrioryLevel=Prio&127;	
		//PrioryLevel=LocalOrder->PrioryLevel;
		if ( OrdType == 1 )NewMonsterSmartSendTo( xx << 4, yy << 4, 0, 0, Prio, 1 );

		return true;
	};
	return false;
};
void InvitePeasant( OneObject* Mine )
{
	if ( !SCENINF.hLib )
	{
		if ( Mine->Nat->AI_Enabled )return;
	};
	word PLIST[64];
	int Ri[64];
	int NP = Mine->Ref.General->newMons->MaxInside;
	if ( NP > 64 )NP = 64;
	byte NI = Mine->NNUM;
	int NN = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && ( !OB->Sdoxlo ) && OB->NNUM == NI&&OB->Ref.General->newMons->Peasant &&
			( ( OB->LocalOrder&&OB->LocalOrder->DoLink == &BuildObjLink&&OB->LocalOrder->info.BuildObj.ObjIndex == Mine->Index ) ||
			( !OB->LocalOrder ) ) )
		{
			int R = Norma( OB->RealX - Mine->RealX, OB->RealY - Mine->RealY ) >> 4;


			if ( R < 512 )
			{

				if ( NN < 64 )
				{
					PLIST[NN] = i;
					Ri[NN] = R;
					if ( !OB->LocalOrder )R += 500;
					NN++;
				};
			};
		};
	};

	UNISORT.CheckSize( NN );
	for ( int i = 0; i < NN; i++ )
	{
		UNISORT.Parms[i] = Ri[i];
		UNISORT.Uids[i] = PLIST[i];


	};
	UNISORT.NUids = NN;
	UNISORT.Sort();
	for ( int i = 0; i < NN&&i < NP; i++ )Group[UNISORT.Uids[i]]->GoToMine( Mine->Index, 128 + 16 );
};
void TakeResLink( OneObject* OBJ );
void InviteAI_Peasants( OneObject* Mine )
{
	if ( Mine->Nat->CITY->FreePS > Mine->Nat->CITY->Nat->MIN_PBRIG )return;
	word PLIST[512];
	int Ri[512];
	int NP = Mine->Ref.General->newMons->MaxInside + Mine->AddInside - Mine->NInside;
	if ( NP < 0 )NP = 0;
	if ( NP > 64 )NP = 64;
	byte NI = Mine->NNUM;
	int NN = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && ( !OB->Sdoxlo ) && OB->NNUM == NI&&OB->Ref.General->newMons->Peasant &&
			( ( OB->LocalOrder&&OB->LocalOrder->DoLink == &TakeResLink ) || ( !OB->LocalOrder ) ) )
		{
			int R = Norma( OB->RealX - Mine->RealX, OB->RealY - Mine->RealY ) >> 4;


			if ( R < 2048 )
			{

				if ( NN < 512 )
				{
					PLIST[NN] = i;
					Ri[NN] = R;
					NN++;
				};
			};
		};
	};

	UNISORT.CheckSize( NN );
	for ( int i = 0; i < NN; i++ )
	{
		UNISORT.Parms[i] = Ri[i];
		UNISORT.Uids[i] = PLIST[i];


	};
	UNISORT.NUids = NN;
	UNISORT.Sort();
	for ( int i = 0; i < NN&&i < NP; i++ )
	{
		//Group[UNISORT.Uids[i]]->Serial+=35;
		OneObject* PS = Group[UNISORT.Uids[i]];
		if ( PS )
		{
			if ( PS->BrigadeID != 0xFFFF )
			{
				Brigade* BR = PS->Nat->CITY->Brigs + PS->BrigadeID;
				Brigade* BRD = &PS->Nat->CITY->InMines;
				if ( BR != BRD )
				{
					BR->RemoveOne( PS->BrIndex, BRD );
				};
			};
			PS->DoNotCall = 1;
			PS->GoToMine( Mine->Index, 128 + 127 );
		};
	};
};
void BuildObjLink( OneObject* OBJ )
{

	if ( OBJ->NewBuilding )
	{
		OBJ->DeleteLastOrder();
		return;
	};
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	if ( OBJ->NewCurSprite < OBJ->NewAnm->NFrames - FrmDec )return;
	NewMonster* NM = OBJ->newMons;
	word OID = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	int ObjX = OBJ->LocalOrder->info.BuildObj.ObjX;
	int ObjY = OBJ->LocalOrder->info.BuildObj.ObjY;
	word OSN = OBJ->LocalOrder->info.BuildObj.SN;
	OBJ->UnBlockUnit();
	OneObject* OB = Group[OID];
	//OBJ->Important=true;
	Order1* Or1;
	if ( ( !OB ) || OB->Sdoxlo || OB->Life == OB->MaxLife || OSN != OB->Serial )
	{
		if ( OB&&OB->NNUM == MyNation&&OB->Life == OB->MaxLife )AddUnlimitedEffect( OB->RealX >> 4, ( OB->RealY >> 5 ) - OB->RZ, OB->newMons->BornSoundID );
		if ( OBJ->LocalOrder )
		{
			OBJ->DeleteLastOrder();
			OBJ->DeletePath();
			return;
		};
	};

	int dst = Norma( OBJ->x - ObjX, OBJ->y - ObjY );
	if ( dst < 7 )
	{
		if ( CheckPt( ObjX, ObjY ) )
		{
			if ( OB->FindPoint( &ObjX, &ObjY, FP_NEAREST_POINT | FP_FIND_WORKPOINT | FP_UNLOCKED_POINT ) )
			{
				OBJ->LocalOrder->info.BuildObj.ObjX = ObjX;
				OBJ->LocalOrder->info.BuildObj.ObjY = ObjY;
				return;
			}
			else
			{
				OBJ->DeleteLastOrder();
				OBJ->DeletePath();
				return;
			};

		};
	};
	if ( dst <= 1 )
	{
		OBJ->DestX = -1;
		OBJ->DeletePath();
		/*if(dst==1&&OBJ->NothingTime<5){
			OBJ->DestX=(ObjX<<8);
			OBJ->DestY=(ObjY<<8);
		}else{;*/
		//we can build now
		char dir = char( GetDir( OB->RealX - OBJ->RealX, OB->RealY - OBJ->RealY ) );
		char ddir = dir - OBJ->RealDir;
		if ( abs( ddir ) < 16 )
		{
			OBJ->RealDir = dir;
			OBJ->GraphDir = OBJ->RealDir;
			OBJ->NewAnm = &NM->Work;
			OBJ->NewCurSprite = 0;
			OBJ->InMotion = false;
			OBJ->BackMotion = false;
			OBJ->WeakBlockUnit();
			OB->NextStage();
		}
		else
		{
			if ( ddir > 0 )RotateMon( OBJ, 16 );
			else RotateMon( OBJ, -16 );
			TryToStand( OBJ, false );
		};
		//};
	}
	else
	{
		if ( CheckPt( ObjX, ObjY ) )
		{
			ObjX = OBJ->x;
			ObjY = OBJ->y;
			if ( OB->FindPoint( &ObjX, &ObjY, FP_NEAREST_POINT | FP_FIND_WORKPOINT | FP_UNLOCKED_POINT ) )
			{
				OBJ->LocalOrder->info.BuildObj.ObjX = ObjX;
				OBJ->LocalOrder->info.BuildObj.ObjY = ObjY;
			}
			else
			{
				Or1 = OBJ->LocalOrder->NextOrder;
				OBJ->FreeOrdBlock( OBJ->LocalOrder );
				OBJ->LocalOrder = Or1;
				//OBJ->Important=false;
				TryToStand( OBJ, true );
				return;
			};
		};
		if ( OBJ->NothingTime > 8 )
		{
			if ( OBJ->CheckBlocking() )
			{
				OBJ->DeleteBlocking();
			}
			else OBJ->CreatePath( ObjX, ObjY );
		}
		else OBJ->CreatePath( ObjX, ObjY );
	};
};
//x,y-coordinates of point on the 2D plane (unit:pix)
//returnfs index of building,otherwise 0xFFFF
word DetermineBuilding( int x, int y, byte NMask )
{
	int xx = x >> 4;
	int yy = y >> 4;
	int xr = x << 4;
	int yr = y << 4;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->NewBuilding && ( OB->NMask&NMask ) )
		{
			NewMonster* NM = OB->newMons;
			int rr = NM->PicLx << 4;
			if ( Norma( xr - OB->RealX, yr - OB->RealY ) < rr )
			{
				//precise checking
				int PicSX = NM->PicDx << 4;
				int PicSY = Prop43( NM->PicDy ) << 4;
				int x0 = ( OB->RealX + PicSX ) >> 8;
				int y0 = ( OB->RealY + PicSY ) >> 8;
				byte* px = NM->CheckX;
				byte* py = NM->CheckY;
				int Npt = NM->NCheckPt;
				for ( int j = 0; j < Npt; j++ )
					if ( x0 + px[j] == xx&&y0 + py[j] == yy ) return i;
			};
		};
	};
	return 0xFFFF;
};
bool OneObject::CheckBlocking()
{
	return ( CheckHLine( x - 1, y - 1, 3 ) || CheckHLine( x - 1, y + 1, 3 ) ||
		CheckPt( x - 1, y ) || CheckPt( x + 1, y ) );
};
void DeleteBlockLink( OneObject* OBJ );
void OneObject::DeleteBlocking()
{
	Order1* OR1 = GetOrdBlock();
	if ( !OR1 )return;//torba
	//PrioryLevel=0;
	DeletePath();
	OR1->OrderType = 100;
	OR1->DoLink = &DeleteBlockLink;
	if ( !CheckPt( x, y + 2 ) )
	{
		OR1->info.DelBlock.EndX = x;
		OR1->info.DelBlock.EndY = y + 2;
		if ( CheckPt( x, y + 1 ) )
		{
			OR1->info.DelBlock.LockX = x;
			OR1->info.DelBlock.LockY = y + 1;
		}
		else
		{
			OR1->info.DelBlock.LockX = -1;
			OR1->info.DelBlock.LockY = -1;
		};
	}
	else
		if ( !CheckPt( x, y - 2 ) )
		{
			OR1->info.DelBlock.EndX = x;
			OR1->info.DelBlock.EndY = y - 2;
			if ( CheckPt( x, y - 1 ) )
			{
				OR1->info.DelBlock.LockX = x;
				OR1->info.DelBlock.LockY = y - 1;
			}
			else
			{
				OR1->info.DelBlock.LockX = -1;
				OR1->info.DelBlock.LockY = -1;
			};
		}
		else
			if ( !CheckPt( x - 2, y ) )
			{
				OR1->info.DelBlock.EndX = x - 2;
				OR1->info.DelBlock.EndY = y;
				if ( CheckPt( x - 1, y ) )
				{
					OR1->info.DelBlock.LockX = x - 1;
					OR1->info.DelBlock.LockY = y;
				}
				else
				{
					OR1->info.DelBlock.LockX = -1;
					OR1->info.DelBlock.LockY = -1;
				};
			}
			else
				if ( !CheckPt( x + 2, y ) )
				{
					OR1->info.DelBlock.EndX = x + 2;
					OR1->info.DelBlock.EndY = y;
					if ( CheckPt( x + 1, y ) )
					{
						OR1->info.DelBlock.LockX = x + 1;
						OR1->info.DelBlock.LockY = y;
					}
					else
					{
						OR1->info.DelBlock.LockX = -1;
						OR1->info.DelBlock.LockY = -1;
					};
				};
	NothingTime = 0;
	OR1->NextOrder = LocalOrder;
	LocalOrder = OR1;
};
void DeleteBlockLink( OneObject* OBJ )
{
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	if ( OBJ->NewCurSprite < OBJ->NewAnm->NFrames - FrmDec )return;
	int lox = OBJ->LocalOrder->info.DelBlock.LockX;
	int loy = OBJ->LocalOrder->info.DelBlock.LockY;
	int enx = OBJ->LocalOrder->info.DelBlock.EndX;
	int eny = OBJ->LocalOrder->info.DelBlock.EndY;
	if ( lox > 0 )BClrPt( lox, loy );
	BClrPt( enx, eny );
	OBJ->DestX = enx << 8;
	OBJ->DestY = eny << 8;
	if ( OBJ->NothingTime > 9 || Norma( OBJ->RealX - OBJ->DestX, OBJ->RealY - OBJ->DestY ) < 128 )
	{
		Order1* OR1 = OBJ->LocalOrder;
		OBJ->LocalOrder = OR1->NextOrder;
		OBJ->FreeOrdBlock( OR1 );
		OBJ->NothingTime = 0;
		OBJ->DeletePath();
	};
};
//-----------Inverse references from (x,y)->Index----------//
/*word MonInd[MAPSX*MAPSX];//512k
#define MAXMI (MAPSX*MAPSY)
inline word GetMI(int x,int y){
	__asm{
		mov		ebx,y
		shl		ebx,9
		add		ebx,x
		cmp		ebx,0
		jl		nona
		cmp		ebx,MAXMI
		jge		nona
		shl		ebx,1
		mov		ax,[eax]
		jmp		done
nona:	mov		ax,0xFFFF
done:
	};
	return;
};
inline void SetMI(int x,int y,word MI){
	__asm{
		mov		eax,y
		shl		eax,9
		add		eax,x
		cmp		eax,0
		jl		nona
		cmp		eax,MAXMI
		jge		nona
		shl		eax,1
		mov		bx,MI
		mov		[eax],bx
nona:
	};
	return;
};*/
int ExplMedia;
void DamageInCell( int cell, int x, int y, int r, word Damage, OneObject* Sender, word Attr )
{
	if ( cell < 0 || cell >= VAL_MAXCX*VAL_MAXCX )return;
	int NMon = MCount[cell];
	if ( !NMon )return;
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				NewMonster* NM = OB->newMons;
				int rr = int( NM->EMediaRadius + NM->ExplRadius ) << 4;
				int dr = Norma( OB->RealX - x, OB->RealY - y );
				if ( OB&&dr < r + rr )
				{
					//Unit is in danger !
					if ( dr < rr )ExplMedia = NM->ExplosionMedia;
					OB->MakeDamage( 0, Damage, Sender, byte( Attr ) );
				};
			};
		};
	};
};
void MakeRoundDamage( int x, int y, int r, word Damage, OneObject* Sender, word Attr )
{
	int cx = ( x >> 11 );
	int cy = ( y >> 11 );
	int cell = cx + ( cy << VAL_SHFCX ) + VAL_MAXCX + 1;
	ExplMedia = -1;
	DamageInCell( cell - VAL_MAXCX - 1, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell - VAL_MAXCX, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell - VAL_MAXCX + 1, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell - 1, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell + 1, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell + VAL_MAXCX - 1, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell + VAL_MAXCX, x, y, r, Damage, Sender, Attr );
	DamageInCell( cell + VAL_MAXCX + 1, x, y, r, Damage, Sender, Attr );
};
bool OneObject::GetDamagePoint( Coor3D* dp, int Precise )
{
	NewMonster* NM = newMons;
	if ( NM->NBars )
	{
		int bx0 = ( RealX >> 4 ) + NM->PicDx;
		int by0 = ( RealY >> 4 ) + ( NM->PicDy << 1 );
		int N = ( int( rando() )*int( NM->NBars ) ) >> 15;
		int pp = N * 5;
		int XB0 = NM->Bars3D[pp] + bx0;
		int YB0 = ( NM->Bars3D[pp + 1] << 1 ) + by0;
		int L1 = NM->Bars3D[pp + 2];
		int L2 = NM->Bars3D[pp + 3];
		dp->x = XB0 + ( ( L1 + L2 ) >> 1 );
		dp->y = YB0 + ( ( L1 - L2 ) >> 1 );
		dp->z = GetHeight( dp->x, dp->y ) + ( NM->Bars3D[pp + 4] >> 1 );
	}
	else
	{
		dp->x = RealX >> 4;
		dp->y = RealY >> 4;
		dp->z = RZ + NM->DstZPoint;
	};
	x += ( rando()*Precise ) >> 15;
	y += ( rando()*Precise ) >> 15;
	return NewBuilding;
};

//Type:
//0 - single order (previous orders will be erased)
//1 - add order to the head of link
//2 - add order to the tile of link
Order1* OneObject::CreateOrder( byte Type )
{
	if ( UnlimitedMotion )return nullptr;
	Order1* OR1 = GetOrdBlock();
	Order1* OR2;
	if ( !OR1 )return nullptr;
	switch ( Type )
	{
	case 1:
		OR1->NextOrder = LocalOrder;
		LocalOrder = OR1;
		break;
	case 2:
		OR1->NextOrder = nullptr;
		if ( LocalOrder )
		{
			OR2 = LocalOrder;
			while ( OR2->NextOrder )OR2 = OR2->NextOrder;
			OR2->NextOrder = OR1;
		}
		else LocalOrder = OR1;
		break;
	default:
		ClearOrders();
		LocalOrder = OR1;
		OR1->NextOrder = nullptr;
	};
	return OR1;
};
void NewMonsterSmartSendToLink( OneObject* OBJ );
int GetOrderKind( ReportFn* RF );
#undef DeleteLastOrder
void NewAttackPointLink( OneObject* OBJ );
void MakeOneShotLink( OneObject* OBJ );

void OneObject::DeleteLastOrder()
{
	DeletePath();
	DestX = -1;
	Order1* OR1 = LocalOrder;
	if ( !OR1 )
	{
		return;
	}

	if ( OR1->DoLink == &AttackObjLink || OR1->DoLink == &WaterAttackLink )
	{
		Attack = false;
		if ( newMons->SlowRecharge )
		{
			int ns = NewState;
			if ( ns && WeaponFlags[newMons->WeaponKind[ns - 1]] & 1 && NewAnm == &newMons->Attack[ns - 1] )
			{
				if ( EnemyID != 0xFFFF )
				{
					OneObject* EOB = Group[EnemyID];
					if ( EOB && EOB->Sdoxlo > 8 )
					{
						MakeOneShot( this, EOB->RealX >> 4, EOB->RealY >> 4, EOB->RZ + 40 );
					};
				};
				if ( LocalOrder->DoLink != &MakeOneShotLink )
				{
					NewState = 4;
					LocalNewState = 4;
					StandTime = 20;
				};

			};
		};
		EnemyID = 0xFFFF;
		EnemySN = 0xFFFF;
	};
	if ( OR1->DoLink == NewAttackPointLink )
	{
		DstX = -1;
		DstY = -1;
	};
	if ( LocalOrder )
	{

	};
	LocalOrder = OR1->NextOrder;
	FreeOrdBlock( OR1 );
	//search for patrol commands
	bool Patr = false;
	bool Atta = false;
	OR1 = LocalOrder;
	while ( OR1 )
	{
		if ( OR1->DoLink == &AttackObjLink )
		{
			Atta = true;
		}
		else
		{
			if ( OR1->DoLink == &NewMonsterSmartSendToLink
				|| OR1->DoLink == NewMonsterSendToLink
				&& OR1->PrioryLevel < 5 )
			{
				Patr = true;
			}
		}
		OR1 = OR1->NextOrder;
	}
	Attack = Atta;
	InPatrol = Patr;
	UnBlockUnit();
}

#define DeleteLastOrder() DeleteLastOrder()

int TestCapture( OneObject* OBJ );

OneObject* SearchEnemyInCell( int cell, byte nmask, byte mmask, byte Priest )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];

	if ( !NMon )
	{
		return nullptr;
	}

	int ofs1 = cell << SHFCELL;
	word MID;

	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				if ( Priest == 3 )
				{
					if ( OB->newMons->Capture && TestCapture( OB ) == 0 )
					{
						return OB;
					}
				}
				else
				{
					if ( Priest == 2 && ( !( OB->NMask&nmask ) ) && OB->newMons->Capture && TestCapture( OB ) == 0 )
					{
						return OB;
					}
					else
					{
						if ( ( !( OB->NMask & nmask ) ) && OB->newMons->MathMask & mmask )
						{
							if ( Priest == 1 )
							{
								if ( OB->Life < OB->Ref.General->MoreCharacter->Life )
								{
									return OB;
								}
							}
							else
							{
								return OB;
							}
						}
					}
				}
			}
		}
	}
	return nullptr;
}

OneObject* SearchEnemyGroupInCell( int cell, byte nmask, byte mmask )
{
	cell += VAL_MAXCX + 1;// += 129

	int NMon = MCount[cell];

	if ( NMon < 3 )
	{
		return nullptr;
	}
	int ofs1 = cell << SHFCELL;//* 64
	word MID;

	for ( int i = 0; i < NMon; i++ )//NMon can be 0 to 255
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && OB->NMask & nmask && OB->newMons->MathMask & mmask && !OB->LockType )
			{
				return OB;
			}
		}
	}
	return nullptr;
}

bool CheckVisibility( int x1, int y1, int x2, int y2, word MyID );
void SearchEnemyForAIArtillery( OneObject* OB );
void AIArtilleryAgainstTowers( OneObject* OB );
void SearchEnemyForMortira( OneObject* OB );
void EraseBrigade( Brigade* BR );
int TestCapture( OneObject* OBJ );
void SearchVictimForAI_Archer( OneObject* OB )
{
	int x0 = OB->RealX >> 11;
	int y0 = OB->RealY >> 11;
	NewMonster* NM = OB->newMons;
	int rr = NM->VisRange;
	int rx1 = ( rr >> 11 ) + 1;
	int rx2 = rx1 + rx1 + 1;
	byte nmask = ~OB->NMask;
	OneObject* DestObj = nullptr;
	int stcell;
	int NSR = 6;
	int RMIN = rr + 100;
	for ( int i = 0; i < 6; i++ )
	{
		int dx = x0 + ( ( rando()*rx2 ) >> 15 ) - rx1;
		int dy = y0 + ( ( rando()*rx2 ) >> 15 ) - rx1;
		if ( dx >= 0 && dy > 0 && dx < VAL_MAXCX - 2 && dy < VAL_MAXCX - 2 )
		{
			stcell = dx + ( dy << VAL_SHFCX );
			word BMID = BLDList[stcell + VAL_MAXCX + 1];
			if ( BMID != 0xFFFF )
			{
				OneObject* BOB = Group[BMID];
				if ( BOB && ( !BOB->Sdoxlo ) && BOB->NMask&nmask )
				{
					int r = Norma( BOB->RealX - OB->RealX, BOB->RealY - OB->RealY ) >> 4;
					if ( r < RMIN )
					{
						RMIN = r;
						DestObj = BOB;
					};
				};
			}
			else
			{
				int wcell = GetLI( dx + dx, dy + dy );
				for ( int vx = 0; vx < 2; vx++ )
					for ( int vy = 0; vy < 2; vy++ )
					{
						int wcl = wcell + vx + ( vy << TopSH );
						WallCell* WCL = WRefs[wcl];
						if ( WCL )
						{
							if ( WCL->NI == 0 && WCL->OIndex != 0xFFFF )
							{
								OneObject* BOB = Group[WCL->OIndex];
								if ( BOB && !BOB->Sdoxlo )
								{
									int r = Norma( BOB->RealX - OB->RealX, BOB->RealY - OB->RealY ) >> 4;
									if ( r < RMIN )
									{
										RMIN = r;
										DestObj = BOB;
									};
								};
							};
						};
					};
			};
		};
	};
	if ( DestObj )OB->AttackObj( DestObj->Index, 16 + 128, 0, 0 );
}

int GetUnitActivity( OneObject* OB );

void OneObject::SearchVictim()
{
	if ( NoSearchVictim )
	{
		return;
	}

	int ac1 = GetUnitActivity( this );
	if ( ac1 )
	{
		if ( newMons->Peasant || newMons->Usage == TowerID || newMons->Artilery || Media )return;
	}

	NewMonster* NM = newMons;

	if ( NM->Archer && Nat->AI_Enabled )
	{
		return;
		//if(delay)return;
		//SearchVictimForAI_Archer(this);
		//return;
	}

	int minR = int( Ref.General->MoreCharacter->MinR_Attack ) << 4;

	if ( LockType && Nat->AI_Enabled && EnemyID == 0xFFFF )
	{
		if ( delay )
		{
			return;
		}

		bool Kill = false;

		//search victim for AI Battleship
		int x0 = RealX >> 11;
		int y0 = RealY >> 11;
		int maxx = msx >> 2;
		int DY = TopLx >> 1;
		//int cell=((y0)<<VAL_SHFCX)+(x0>11);

		int rr = NM->VisRange;

		if ( NM->Usage == GaleraID )
		{
			rr = NM->AttackRadius2[1];
		}

		int rx1 = ( rr >> 11 ) + 1;

		byte MMASK = NMask;
		byte nmask = ~NMask;
		byte mmask = NM->KillMask;
		OneObject* DestObj = nullptr;

		int mindist = 10000000;
		int dist;
		int rx2 = rx1 + rx1 + 1;
		int stcell;

		for ( int i = 0; i < 6; i++ )
		{
			int dx = x0 + ( ( rando()*rx2 ) >> 15 ) - rx1;
			int dy = y0 + ( ( rando()*rx2 ) >> 15 ) - rx1;

			if ( x0 > 0 && x0 < maxx && y0 >= 0 && y0 < maxx )
			{
				stcell = dx + ( dy << VAL_SHFCX );//dx + (dy * 128)
				byte BPT = NPresence[stcell];
				if ( ( BPT & nmask ) && !( ( BPT & MMASK ) ||
					( NPresence[stcell - 1] & MMASK ) || ( NPresence[stcell + 1] & MMASK ) ||
					( NPresence[stcell - DY] & MMASK ) || ( NPresence[stcell + DY] & MMASK ) ) )
				{
					OneObject *OB = SearchEnemyGroupInCell( stcell, nmask, mmask );
					if ( OB )
					{
						dist = Norma( OB->RealX - RealX, OB->RealY - RealY );
						if ( dist > minR && dist < mindist )
						{
							mindist = dist;
							DestObj = OB;
						}
					}
				}
			}
		}
		if ( DestObj )
		{
			if ( LocalOrder )
			{
				AttackObj( DestObj->Index, 128 + 16, 1, 1 );
			}
			else
			{
				AttackObj( DestObj->Index, 128 + 16, 1, 4 );
			}
		}
		return;
	}

	if ( Zombi )
	{
		if ( newMons->Usage == PushkaID )
		{
			SearchEnemyForAIArtillery( this );
			return;
		}
		if ( newMons->Usage == MortiraID )
		{
			SearchEnemyForMortira( this );
			return;
		}
		if ( newMons->Usage == SupMortID && GNFO.EINF[NNUM] && !delay )
		{
			int BX, BY;
			word MID = GNFO.EINF[NNUM]->SearchBestEnemyAndPlaceForSupermortira( this, &BX, &BY );
			if ( MID != 0xFFFF )
			{
				if ( BrigadeID != 0xFFFF )
				{
					Nat->CITY->UnRegisterNewUnit( this );
					Zombi = true;
				}
				if ( BX != RealX )
				{
					NewMonsterSmartSendTo( BX >> 4, BY >> 4, 0, 0, 128 + 64, 0 );
					delay = 16;
					MaxDelay = 16;
				}
				else
				{
					OneObject* OB = Group[MID];
					if ( OB )
					{
						AttackPoint( OB->RealX >> 4, OB->RealY >> 4, OB->RZ + 32, 1, ATTP_IMMEDIATECHECK, 0 );
					}
				}
			}
			else
			{
				//need to connect to nearest brigade
				if ( BrigadeID == 0xFFFF )
				{
					//search for the nearest army
					AI_Army* BestArmy = nullptr;
					int MinTopDist = 1000000;
					int BestFinalTop = 0xFFFF;
					int MyTop = GetTopology( RealX >> 4, RealY >> 4 );
					if ( MyTop < 0xFFFE )
					{
						int TOPSTART = MyTop*NAreas;
						AI_Army* ARM = Nat->CITY->ARMS;
						for ( int i = 0; i < MaxArm; i++ )
						{
							if ( ARM->Enabled && ( ARM->Spec == 0 || ARM->Spec == 1 ) )
							{
								int top = ARM->TopPos;
								if ( top < 0xFFFE )
								{
									int topdst = LinksDist[top + TOPSTART];
									if ( top == MyTop )
									{
										topdst = 0;
									}
									if ( topdst < MinTopDist )
									{
										MinTopDist = topdst;
										BestFinalTop = top;
										BestArmy = ARM;
									}
								}
							}
							ARM++;
						}
						if ( BestFinalTop != 0xFFFF )
						{
							int NextTop = MotionLinks[TOPSTART + BestFinalTop];
							if ( NextTop == BestFinalTop || MinTopDist == 0 )
							{
								//we can connect now!
								City* CT = Nat->CITY;
								int BID = CT->GetFreeBrigade();
								if ( BID != -1 )
								{
									Brigade* BR = CT->Brigs + BID;
									BR->Enabled = true;
									BR->AddObject( this );
									BestArmy->AddBrigade( BR );
									BR->DeleteAll();
									EraseBrigade( BR );
									BestArmy->Parad();
								}
							}
							else
							{
								//move to next topology zone
								if ( NextTop < 0xFFFE )
								{
									Area* AR = TopMap + NextTop;
									NewMonsterSendTo( int( AR->x ) << 10, int( AR->y ) << 10, 128 + 16, 0 );
								}
							}
						}
					}
					delay = 50;
					MaxDelay = 50;
				}
			}
			//AIArtilleryAgainstTowers(this);
			return;
		}

		int cell = ( ( RealY >> 11 ) << VAL_SHFCX ) + ( RealX >> 11 );
		NewMonster* NM = newMons;
		int rr = NM->VisRange;
		int rx1 = 6;
		byte nmask = ~NMask;
		byte mmask = NM->KillMask;
		OneObject* DestObj = nullptr;
		int mindist = 10000000;
		int dist;
		int rx2 = rx1 + rx1 + 1;
		int stcell;

		for ( int i = 0; i < 6; i++ )
		{
			int dx = ( ( rando()*rx2 ) >> 15 ) - rx1;
			int dy = ( ( rando()*rx2 ) >> 15 ) - rx1;
			stcell = cell + dx + ( dy << VAL_SHFCX );
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( NPresence[stcell] & nmask )
				{
					OneObject*OB = SearchEnemyInCell( stcell, ~nmask, mmask, 3 );
					if ( OB )
					{
						dist = Norma( OB->RealX - RealX, OB->RealY - RealY );
						if ( dist<mindist&&dist>minR )
						{
							mindist = dist;
							DestObj = OB;
						}
					}
				}
			}
		}
		if ( DestObj )
		{
			int t0 = GetTopology( RealX >> 4, RealY >> 4 );

			if ( t0 >= 0xFFFE )
			{
				return;
			}

			int t1 = GetTopology( DestObj->RealX >> 4, DestObj->RealY >> 4 );

			if ( t0 == t1 || LinksDist[t1*NAreas + t0] < 40 )
			{
				AttackObj( DestObj->Index, 1 );
			}
		}
		return;
	}

	if ( PeaceMode )
	{
		return;
	}

	int cell = ( ( RealY >> 11 ) << VAL_SHFCX ) + ( RealX >> 11 );
	int rr = NM->VisRange;
	int rx1 = ( rr >> 11 ) + 1;
	byte nmask = ~NMask;
	byte Priest = NM->Priest;

	if ( Priest )
	{
		nmask = NMask;
		Priest = 1;
	}
	else
	{
		if ( !( NM->Capture || LockType || NewBuilding ) )
		{
			Priest = 2;
		}
	}

	byte mmask = NM->KillMask;
	OneObject* DestObj = nullptr;
	int mindist = 10000000;
	int dist;

	if ( rx1 <= 5 )
	{
		//short range search
		int rx2 = rx1 + rx1 + 1;
		int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
		byte* bpt = NPresence + stcell;
		for ( int nx = 0; nx < rx2; nx++ )
		{
			for ( int ny = 0; ny < rx2; ny++ )
			{
				if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
				{
					if ( bpt[0] & nmask )
					{
						OneObject*OB = SearchEnemyInCell( stcell, ~nmask, mmask, Priest );
						if ( OB )
						{
							dist = Norma( OB->RealX - RealX, OB->RealY - RealY );
							if ( dist<mindist&&dist>minR )
							{
								mindist = dist;
								DestObj = OB;
							}
						}
					}
				}
				stcell++;
				bpt++;
			}
			stcell += VAL_MAXCX - rx2;
			bpt += VAL_MAXCX - rx2;
		}
	}
	else
	{
		//near nose search
		if ( Ref.General->MoreCharacter->MinR_Attack < 140 && !( StandTime > 16 && rando() > 6000 ) )
		{
			int rxx = 1;
			int rx2 = rxx + rxx + 1;
			int stcell = cell - rxx - ( rxx << VAL_SHFCX );
			byte* bpt = NPresence + stcell;
			for ( int nx = 0; nx < rx2; nx++ )
			{
				for ( int ny = 0; ny < rx2; ny++ )
				{
					if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
					{
						if ( bpt[0] & nmask )
						{
							OneObject*OB = SearchEnemyInCell( stcell, ~nmask, mmask, Priest );
							if ( OB )
							{
								dist = Norma( OB->RealX - RealX, OB->RealY - RealY );
								if ( dist<mindist&&dist>minR )
								{
									mindist = dist;
									DestObj = OB;
								}
							}
						}
					}
				}
				stcell++;
				bpt++;
			}
			stcell += VAL_MAXCX - rx2;
			bpt += VAL_MAXCX - rx2;
		}
		if ( !DestObj )
		{

			//long range search
			int rx2 = rx1 + rx1 + 1;
			int stcell;
			int NSR = 6;
			if ( NM->Usage == TowerID )
			{
				NSR = 20;
			}
			for ( int i = 0; i < 6; i++ )
			{
				int dx = ( ( rando()*rx2 ) >> 15 ) - rx1;
				int dy = ( ( rando()*rx2 ) >> 15 ) - rx1;
				stcell = cell + dx + ( dy << VAL_SHFCX );
				if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
				{
					rando();
					if ( NPresence[stcell] & nmask )
					{
						rando();
						OneObject*OB = SearchEnemyInCell( stcell, ~nmask, mmask, Priest );
						if ( OB )
						{
							rando();
							dist = Norma( OB->RealX - RealX, OB->RealY - RealY );
							if ( dist<mindist&&dist>minR )
							{
								mindist = dist;
								DestObj = OB;
							}
						}
					}
				}
			}
		}
	}
	//if(!DestObj&&!(newMons->Capture||NewBuilding||LockType)){}

	//if(DestObj)
	if ( DestObj&&mindist < rr )
	{
		int ac2 = GetUnitActivity( DestObj );
		if ( ac2 != 1 )
		{

			if ( !CheckVisibility( DestObj->RealX, DestObj->RealY, RealX, RealY, Index ) )
			{
				return;
			}

			AttackObj( DestObj->Index, 1 );
		}
	}
}

int GetTopDistance( int xa, int ya, int xb, int yb );
word SearchVictim( OneObject* OBJ, int R0, int R1 )
{
	int cell = ( ( OBJ->RealY >> 11 ) << VAL_SHFCX ) + ( OBJ->RealX >> 11 );
	NewMonster* NM = OBJ->newMons;
	int rr = R1 << 4;
	int rmin = R0 << 4;
	int rx1 = ( R1 >> 7 ) + 1;
	byte nmask = ~OBJ->NMask;
	byte mmask = NM->KillMask;
	OneObject* DestObj = nullptr;
	int mindist = 10000000;
	int dist;
	//short range search
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	byte* bpt = NPresence + stcell;
	int RealX = OBJ->RealX;
	int RealY = OBJ->RealY;
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( bpt[0] & nmask )
				{
					OneObject*OB = SearchEnemyInCell( stcell, ~nmask, mmask, 0 );
					if ( OB )
					{
						dist = Norma( OB->RealX - RealX, OB->RealY - RealY );
						if ( dist > rmin&&dist < mindist )
						{
							mindist = dist;
							DestObj = OB;
						};
					};
				};
			};
			stcell++;
			bpt++;
		};
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	};
	if ( DestObj&&mindist < rr )
	{
		if ( GetTopDistance( OBJ->RealX >> 10, OBJ->RealY >> 10, DestObj->RealX >> 10, DestObj->RealY >> 10 ) < 20 )
			return DestObj->Index;
	};
	return 0xFFFF;
};
void SetUnlimitedLink( OneObject* OB )
{
	OB->UnlimitedMotion = true;
	OB->DeleteLastOrder();
};
void OneObject::SetOrderedUnlimitedMotion( byte OrdType )
{
	if ( CheckOrderAbility() )return;
	Order1* Or1 = CreateOrder( OrdType );
	if ( !int( Or1 ) )return;
	Or1->OrderType = 57;
	Or1->OrderTime = 0;
	Or1->PrioryLevel = 0;
	Or1->DoLink = &SetUnlimitedLink;
};
bool FINDNEAREMPTY( int* x, int* y )
{
	int xx = *x;
	int yy = *y;
	for ( int r = 1; r < 30; r++ )
	{
		int N = Rarr[r].N;
		char* dx = Rarr[r].xi;
		char* dy = Rarr[r].yi;
		for ( int p = 0; p < N; p++ )
		{
			if ( !CheckBar( xx + dx[p] - 4, yy + dy[p] - 4, 9, 9 ) )
			{
				*x = xx + dx[p];
				*y = yy + dy[p];
				return 1;
			};
		};
	};
	return 0;
}

void ClearUnlimitedLink( OneObject* OB )
{
	if ( OB->StandTime < 5 )
		return;

	if ( CheckBar( OB->x - 1, OB->y - 1, 3, 3 ) )
	{
		int xx = OB->x;
		int yy = OB->y;
		if ( FINDNEAREMPTY( &xx, &yy ) )
		{
			OB->UnlimitedMotion = 0;
			OB->NewMonsterSendTo( xx << 8, yy << 8, 128 + 16, 1 );
			OB->UnlimitedMotion = 1;
			return;
		}
	}
	OB->UnlimitedMotion = false;
	word GID = OB->LocalOrder->info.BuildObj.ObjIndex;
	OB->DeleteLastOrder();
	OB->PrioryLevel = 0;
	OB->NextForceX = OB->newMons->OneStepDX[OB->RealDir];
	OB->NextForceY = OB->newMons->OneStepDY[OB->RealDir];
	if ( GID != 0xFFFF && GID < SCENINF.NUGRP )
	{
		UnitsGroup* UG = SCENINF.UGRP + GID;
		UG->IDS = (word*) realloc( UG->IDS, UG->N * 2 + 2 );
		UG->SNS = (word*) realloc( UG->SNS, UG->N * 2 + 2 );
		UG->IDS[UG->N] = OB->Index;
		UG->SNS[UG->N] = OB->Serial;
		UG->N++;
	}
}

void OneObject::ClearOrderedUnlimitedMotion( byte OrdType, word GroupID )
{
	if ( CheckOrderAbility() )
		return;

	Order1* Or1 = CreateOrder( OrdType );

	if ( !int( Or1 ) )
		return;

	Or1->OrderType = 57;
	Or1->OrderTime = 0;
	Or1->PrioryLevel = 0;
	Or1->info.BuildObj.ObjIndex = GroupID;
	Or1->DoLink = &ClearUnlimitedLink;
}

void OneObject::ClearOrderedUnlimitedMotion( byte OrdType )
{
	ClearOrderedUnlimitedMotion( OrdType, 0xFFFF );
}

OneObject* SearchCapturers( int cell, byte mmask )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )
	{
		return nullptr;
	}
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !( OB->NMask & mmask || OB->newMons->Capture || OB->NNUM == 7 ) )
			{
				return OB;
			}
		}
	}
	return nullptr;
}

OneObject* SearchProtectors( int cell, byte mmask )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )
	{
		return nullptr;
	}
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && ( OB->NMask&mmask )
				&& !( OB->newMons->Capture || OB->Sdoxlo ) )
			{
				return OB;
			}
		}
	}
	return nullptr;
}

bool CheckProtectors_Walls( int x, int y, byte mask )
{
	int LI = GetLI( x << 1, y << 1 );
	for ( int iy = 0; iy < 2; iy++ )
	{
		for ( int ix = 0; ix < 2; ix++ )
		{
			WallCell* WC = WRefs[LI];
			if ( WC && NATIONS[WC->NI].NMask&mask )
			{
				return true;
			}
			LI++;
		}
		LI += TopLx - 2;
	}
	return false;
}

int GetProtectors( int cell, byte mmask )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )
	{
		return 0;
	}
	int ofs1 = cell << SHFCELL;
	word MID;
	int N = 0;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && ( OB->NMask&mmask )
				&& !( OB->newMons->Capture || OB->Sdoxlo ) )
			{
				N++;
			}
		}
	}
	return N;
}

word GetNearestDefender( OneObject* OBJ )
{
	int cell = ( ( OBJ->RealY >> 11 ) << VAL_SHFCX ) + ( OBJ->RealX >> 11 );
	int CELL0 = cell;
	int rx1 = 3;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	byte NMASK = OBJ->NMask;
	byte* bpt = NPresence + stcell;
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( bpt[0] & NMASK )
				{
					OneObject* EOB = SearchProtectors( stcell, NMASK );
					if ( EOB )return EOB->Index;
				};
			};
			stcell++;
			bpt++;
		};
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	};
	return 0xFFFF;
}

int TestCapture( OneObject* OBJ )
{
	switch ( CaptState )
	{
	case 1:
		if ( OBJ->newMons->Peasant )return -1;
		break;
	case 2:
		if ( OBJ->newMons->Peasant || OBJ->newMons->Usage == CenterID || OBJ->newMons->Usage == MineID )return -1;
		break;
	case 3:
		if ( !OBJ->newMons->Artilery )return -1;
		break;
	};
	int cell = ( ( OBJ->RealY >> 11 ) << VAL_SHFCX ) + ( OBJ->RealX >> 11 );

	int X0 = OBJ->RealX >> 10;
	int Y0 = OBJ->RealY >> 10;

	int CELL0 = cell;
	NewMonster* NM = OBJ->newMons;
	byte nmask = ~OBJ->NMask;
	byte NMASK = OBJ->NMask;
	OneObject* DestObj = nullptr;
	int mindist = 10000000;
	int rx1 = 2;
	//X0-=rx1+rx1;
	//Y0-=rx1+rx1;
	bool Capture = true;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	byte* bpt = NPresence + stcell;
	//byte MyMask=OBJ->NMask;
	if ( !( OBJ->Wall&&OBJ->Life < OBJ->MaxLife / 3 ) )
	{
		rx1 = 3;
		rx2 = rx1 + rx1 + 1;
		stcell = CELL0 - rx1 - ( rx1 << VAL_SHFCX );
		//checking help
		bpt = NPresence + stcell;
		for ( int nx = 0; nx < rx2&&Capture; nx++ )
		{
			for ( int ny = 0; ny < rx2&&Capture; ny++ )
			{
				if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
				{
					if ( bpt[0] & NMASK )
					{
						OneObject*OB = SearchProtectors( stcell, NMASK );
						if ( OB && !OB->newMons->Capture )Capture = false;
						//if(CheckProtectors_Walls(X0+nx+nx,Y0+ny+ny,MyMask))Capture=false;
					};
				};
				stcell++;
				bpt++;
			};
			stcell += VAL_MAXCX - rx2;
			bpt += VAL_MAXCX - rx2;
		};
	};
	if ( Capture )return 0;
	else return -1;
};
extern int LastAttackDelay;
extern short AlarmSoundID;
extern int LastAttackX;
extern int LastAttackY;
extern int AlarmDelay;

void StopUpgradeInBuilding( OneObject *OB );

void CheckCapture( OneObject* OBJ )
{
	switch ( CaptState )
	{//Capture Options
	case 1://No Peasants
		if ( OBJ->newMons->Peasant )
		{
			return;
		}
		break;

	case 2://No Peasants and City Centers
		if ( OBJ->newMons->Peasant || OBJ->newMons->Usage == CenterID || OBJ->newMons->Usage == MineID )
		{
			return;
		}
		break;

	case 3://Only Artillery
		if ( !( OBJ->newMons->Artilery || OBJ->Wall ) )
		{
			return;
		}
		break;
	}

	int cell = ( ( OBJ->RealY / 2048 ) << VAL_SHFCX ) + ( OBJ->RealX / 2048 );

	int CELL0 = cell;
	NewMonster* NM = OBJ->newMons;
	byte nmask = ~OBJ->NMask;
	byte NMASK = OBJ->NMask;
	OneObject* DestObj = nullptr;
	int mindist = 10000000;
	int dist;
	int rx1 = 2;

	bool Capture = false;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	byte CapNation = 0;
	byte* bpt = NPresence + stcell;
	int NCapt = 0;
	OneObject* CAPUNIT = nullptr;

	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( bpt[0] & nmask )
				{
					OneObject*OB = SearchCapturers( stcell, NMASK );
					if ( OB && !( OB->newMons->Capture || OB->LockType ) )
					{
						dist = Norma( OB->RealX - OBJ->RealX, OB->RealY - OBJ->RealY );
						if ( dist < 250 * 16 )
						{
							Capture = true;
							CapNation = OB->NNUM;
							NCapt++;
							CAPUNIT = OB;
						}
					}
				}
			}
			stcell++;
			bpt++;
		}
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	}

	int npro = 0;

	if ( Capture )
	{
		int ac1 = GetUnitActivity( OBJ );
		if ( ac1 == 1 && !CAPUNIT )
		{
			return;
		}
		if ( CAPUNIT )
		{
			//int ac2 = GetUnitActivity( CAPUNIT );
			if ( ac1 == 1 )//FUNNY: Not a typo! Correcting to 'ac2 == 1' reverts PT capture rules O_o
			{
				CAPUNIT->Die();
				return;
			}
		}
	}

	if ( Capture && !( OBJ->Wall && OBJ->Life < OBJ->MaxLife / 3 ) )
	{//Unit can be captured and isn't an incomplete Wall cell
		if ( !OBJ->newMons->Peasant )
		{
			OBJ->delay = 100;
			OBJ->MaxDelay = 100;
		}

		rx1 = 3;
		rx2 = rx1 + rx1 + 1;

		stcell = CELL0 - rx1 - ( rx1 << VAL_SHFCX );

		//Checking for guards
		bpt = NPresence + stcell;
		for ( int nx = 0; nx < rx2; nx++ )
		{
			for ( int ny = 0; ny < rx2; ny++ )
			{
				if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
				{
					if ( bpt[0] & NMASK )
					{
						OneObject*OB = SearchProtectors( stcell, NMASK );
						if ( OB && !OB->newMons->Capture )
						{//Allied non-capturable units are near
							Capture = false;
							npro++;
						}
					}
				}
				stcell++;
				bpt++;
			}
			stcell += VAL_MAXCX - rx2;
			bpt += VAL_MAXCX - rx2;
		}

		if ( OBJ->Nat->AI_Enabled && OBJ->newMons->Artilery )
		{
			if ( npro
				&& ( NCapt >= npro && npro == 1 )
				|| ( NCapt > 3 && npro == 2 )
				|| ( NCapt > 7 && npro == 3 )
				|| ( NCapt > 10 && npro == 4 ) )
			{
				OBJ->Die();
				return;
			}
		}
	}

	if ( Capture )
	{//Someone is capturing something
		byte OldNat = OBJ->NNUM;
		bool Easy = CITY[OldNat].Difficulty < 3;

		if ( OBJ->UnlimitedMotion && !OBJ->Hidden )
		{
			OBJ->Die();
			return;
		}

		//AI checks?
		if ( OBJ->AutoKill || OBJ->Wall )
		{
			DestructBuilding( OBJ );
		}
		else
		{
			if ( NATIONS[OldNat].AI_Enabled )
			{
				int timer_mod_64 = tmtmt % 64;
				if ( OBJ->NewBuilding && timer_mod_64 > 16 )
				{
					DestructBuilding( OBJ );
				}
				else
				{
					byte use = OBJ->newMons->Usage;
					if ( use == PeasantID && !Easy )
					{
						DestructBuilding( OBJ );
					}
					else
					{
						if ( ( use == SupMortID && timer_mod_64 > 30 ) || ( use == PushkaID && timer_mod_64 > 20 ) ||
							( use == MortiraID && timer_mod_64 > 8 ) || ( use == PeasantID && timer_mod_64 > 35 ) )
						{
							DestructBuilding( OBJ );
							return;
						}
					}
				}
			}
			else
			{
				if ( NATIONS[CapNation].AI_Enabled )
				{
					if ( OBJ->newMons->Usage == PeasantID && !Easy )
					{
						DestructBuilding( OBJ );
						return;
					}
				}
			}
		}

		//Increment killed units counter for statistics
		NATIONS[OldNat].NKilled[OBJ->NIndex]++;

		//Adjust player scores
		OBJ->Nat->CITY->Account -= OBJ->newMons->Ves * 5;
		CITY[CapNation].Account += OBJ->newMons->Ves * 5;

		if ( OBJ->NNUM == MyNation && OBJ->NewBuilding )
		{//Your building has been captured, print hint
			char buf[200];
			sprintf( buf, CAPBLD, OBJ->Ref.General->Message );
			CreateTimedHintEx( buf, kImportantMessageDisplayTime, 32 );//Your %s have been captured.
		}

		if ( OBJ->NNUM == MyNation )
		{//Check if alarm sound is necessary?
			LastActionX = OBJ->RealX >> 4;
			LastActionY = OBJ->RealY >> 4;
			LastAttackX = OBJ->RealX;
			LastAttackY = OBJ->RealY;
			AlarmDelay = 60;
		}

		if ( OBJ->newMons->Usage == PeasantID )
		{//Stop previous actions for captured peasants
			OBJ->ClearOrders();
		}

		if ( OBJ->NewBuilding && nullptr != OBJ->LocalOrder )
		{//Captured object is a building and it's doing something
			//In case it's an upgrade it will be canceled
			StopUpgradeInBuilding( OBJ );
		}

		DelObject( OBJ );
		OBJ->Nat->CITY->UnRegisterNewUnit( OBJ );
		int oldst = OBJ->Stage;
		int oldmax = OBJ->Ref.General->MoreCharacter->ProduceStages;
		OBJ->Ref.General = NATIONS[CapNation].Mon[OBJ->NIndex];
		OBJ->Nat = &NATIONS[CapNation];
		OBJ->NMask = OBJ->Nat->NMask;
		OBJ->Serial ^= 1;
		OBJ->Selected = 0;
		OBJ->ImSelected = 0;
		OBJ->Zombi = false;

		if ( OBJ->NewBuilding && OldNat == MyNation )
		{//One of your buildings has been captured
			LastAttackDelay = 900;
			if ( AlarmSoundID != -1 )
			{//Play 'you are under attack' sound
				AddEffect( ( mapx * 32 ) + 500, ( mapy * 16 ) + 300, AlarmSoundID );
				LastActionX = OBJ->RealX;
				LastActionY = OBJ->RealY;
				AlarmDelay = 60;
			}
		}

		OBJ->NNUM = CapNation;

		OBJ->Nat->CITY->RegisterNewUnit( OBJ );

		AddObject( OBJ );

		if ( OBJ->NewBuilding )
		{
			OBJ->Stage = ( oldst*int( OBJ->Ref.General->MoreCharacter->ProduceStages ) ) / oldmax;
		}

		if ( OBJ->NInside )
		{
			if ( !CaptState )
			{
				word* Uni = OBJ->Inside;
				for ( int j = 0; j < OBJ->NInside; j++ )
				{
					word MID = Uni[j];
					if ( MID != 0xFFFF )
					{
						OneObject* OB = Group[MID];
						if ( OB )
						{
							OB->Nat->CITY->Account -= OB->newMons->Ves * 5;
							CITY[CapNation].Account += OB->newMons->Ves * 5;
							if ( OB->newMons->Usage == PeasantID )
							{
								OB->ClearOrders();
							}
							DelObject( OB );
							OB->Nat->CITY->UnRegisterNewUnit( OB );
							OB->Ref.General = NATIONS[CapNation].Mon[OB->NIndex];
							OB->Nat = &NATIONS[CapNation];
							OB->NMask = OB->Nat->NMask;
							OB->Serial ^= 1;
							OB->Selected = false;
							OB->ImSelected = false;
							OB->Zombi = false;
							byte OldNat = OB->NNUM;
							OB->NNUM = CapNation;
							OB->Nat->CITY->RegisterNewUnit( OB );
							AddObject( OB );
						}
					}
				}
			}
			else
			{
				DestructBuilding( OBJ );
			}
		}
	}
}

int GetAmountOfProtectors( OneObject* OBJ )
{
	int cell = ( ( OBJ->RealY >> 11 ) << VAL_SHFCX ) + ( OBJ->RealX >> 11 );
	int CELL0 = cell;
	int rx1 = 5;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	int N = 0;
	byte NMASK = OBJ->NMask;
	byte* bpt = NPresence + stcell;
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( bpt[0] & NMASK )
				{
					N += GetProtectors( stcell, NMASK );
				};
			};
			stcell++;
			bpt++;
		};
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	};
	return N;
};
extern int HISPEED;
void PlayAnimation( NewAnimation* NA, int Frame, int x, int y )
{
	if ( NA->SoundID == -1 )return;
	int HF = NA->HotFrame;
	Frame /= FrmDec;
	HF /= FrmDec;
	if ( Frame != HF )return;
	AddEffect( x, y, NA->SoundID );

};
bool CheckPlaceForShar( int x, int y )
{
	if ( CheckBar( ( x >> 4 ) - 8, ( y >> 4 ) - 8, 17, 17 ) )return false;
	int h = GetHeight( x, y );
	if ( abs( GetHeight( x - 60, y ) - h ) > 10 || abs( GetHeight( x + 60, y ) - h ) > 10 || abs( GetHeight( x, y - 60 ) - h ) > 10 || abs( GetHeight( x, y + 60 ) - h ) > 10 )return false;
	//check monsters
	return CheckMonstersInArea( x << 4, y << 4, 150 << 4 ) && CheckSpritesInArea( x << 4, y << 4, 150 << 4 );
}

//Place observation balloon
void HandleShar( Nation* NT )
{
	if ( NT->SharAllowed )
	{
		if ( NT->SharPlaceFound )
		{
			if ( NT->SharStage == 0xFF )
			{
				//processing and drawing <shar> in motion.
				//1.processing
				int x = NT->SharX >> 4;
				int y = NT->SharY >> 4;
				if ( y > ( msy * 128 ) )
				{
					y = msy * 128;
				}

				int z = GetHeight( x, y );

				int dz = ( NT->SharZ >> 4 ) - z;

				bool AllowClip = false;
				if ( dz < 100 )
				{
					NT->SharAx = 0;
					NT->SharAy = 0;
					NT->SharAz = 64;
				}
				else
				{
					NT->Vision = true;
					NT->SharAz = ( 250 - dz ) >> 5;
					if ( NT->SharAz > 64 )
						NT->SharAz = 64;
					if ( NT->SharAz < -64 )
						NT->SharAz = -64;
					//free motion;
					if ( rando() < 60 )
					{
						do
						{
							NT->SharAx = ( rando() & 64 ) - 32;
						} while ( abs( NT->SharAx ) < 16 );
						do
						{
							NT->SharAy = ( rando() & 64 ) - 32;
						} while ( abs( NT->SharAy ) < 16 );
					};
					AllowClip = true;
				}

				//Friction
				int fx = -NT->SharVx / 128;
				int fy = -NT->SharVy / 128;
				int fz = -NT->SharVz / 128;
				if ( AllowClip )
				{
					if ( x < 1200 )
						fx += 64;
					if ( y < 1200 )
						fy += 64;
					if ( x > ( msx * 32 ) - 1200 )
						fx -= 64;
					if ( y > ( msy * 32 ) - 1200 )
						fy -= 64;
				}
				NT->SharVx += NT->SharAx + fx;
				NT->SharVy += NT->SharAy + fy;
				NT->SharVz += NT->SharAz + fz;
				NT->SharX += NT->SharVx / 128;
				NT->SharY += NT->SharVy / 128;
				NT->SharZ += NT->SharVz / 128;
			}
			else
			{
				int frm = 0;
				if ( NT->SharStage > 180 )
					frm = ( NT->SharStage - 180 ) >> 1;
				NT->SharStage++;
				int dx = mapx << 5;
				int dy = mapy << 4;
				int x = NT->SharX >> 4;
				int y = NT->SharY >> 4;
				int z = GetHeight( x, y );
				if ( frm >= SharBuild->NFrames - FrmDec )
				{
					BClrBar( ( NT->SharX >> 8 ) - 3, ( NT->SharY >> 8 ) - 3, 7 );
					NT->SharStage = 0xFF;
					NT->SharZ = z << 4;
					NT->SharVx = 0;
					NT->SharVy = 0;
					NT->SharVz = 0;
					NT->SharAx = 0;
					NT->SharAy = 0;
					NT->SharAz = 0;
				}
			}
		}
		else
		{
			int xx = ( NT->SharX >> 4 ) + ( ( int( rando() )*NT->SearchRadius ) >> 15 );
			int yy = ( NT->SharY >> 4 ) + ( ( int( rando() )*NT->SearchRadius ) >> 15 );
			if ( CheckPlaceForShar( xx, yy ) )
			{
				NT->SharPlaceFound = true;
				NT->SharStage = 0;
				NT->SharX = xx << 4;
				NT->SharY = yy << 4;
				BSetBar( ( NT->SharX >> 8 ) - 3, ( NT->SharY >> 8 ) - 3, 7 );
			}
			else
			{
				NT->SearchRadius += 10;
			}
		}
	}
}

void DrawShar( Nation* NT )
{
	if ( NT->SharAllowed )
	{
		if ( NT->SharPlaceFound )
		{
			if ( NT->SharStage == 0xFF )
			{
				//processing and drawing <shar> in motion.
				//1.processing
				int x = NT->SharX >> 4;
				int y = NT->SharY >> 4;
				if ( y > ( msy << 5 ) )y = msy << 5;
				int z = GetHeight( x, y );
				int dz = ( NT->SharZ >> 4 ) - z;
				bool AllowClip = false;

				//2.drawing
				if ( Shar )
				{
					int dx = mapx << 5;
					int dy = mapy << 4;
					int x = NT->SharX >> 4;
					int y = NT->SharY >> 4;
					int z = NT->SharZ >> 4;
					int vx = NT->SharVx >> 5;
					int vy = NT->SharVy >> 5;
					z += int( sqrt( vx*vx + vy*vy ) );
					int z0 = GetHeight( x, y );
					int dz = ( z - z0 ) >> 1;
					x -= dx;
					y = ( y >> 1 ) - dy - z;
					int maxxx = smaplx << 5;
					int maxyy = smaply << 4;
					CurDrawNation = NT->NNUM;
					if ( x > -128 && y > -160 && x < maxxx + 128 && y < maxyy + 160 )
					{
						NewFrame* NF = &Shar->Frames[0];
						AddPoint( x, y + z, x + NF->dx, y + NF->dy, nullptr, NF->FileID, NF->SpriteID, 0, 0 );
					}
					//shadow drawing
					x = ( NT->SharX >> 4 ) - dz;
					y = ( NT->SharY >> 4 ) - ( dz >> 1 );
					z = GetHeight( x, y );
					x -= dx;
					y = ( y >> 1 ) - dy - z;
					if ( x > -128 && y > -160 && x < maxxx + 128 && y < maxyy + 160 )
					{
						NewFrame* NF = &Shar->Frames[1];
						CurDrawNation = NT->NNUM;
						AddPoint( x, y + z, x + NF->dx, y + NF->dy, nullptr, NF->FileID, NF->SpriteID, 0, 0 );
					}
				}
			}
			else
			{
				int frm = 0;
				if ( NT->SharStage > 180 )frm = ( NT->SharStage - 180 ) >> 1;
				int dx = mapx << 5;
				int dy = mapy << 4;
				int x = NT->SharX >> 4;
				int y = NT->SharY >> 4;
				int z = GetHeight( x, y );
				x -= dx;
				y = ( y >> 1 ) - dy - z;
				int maxxx = smaplx << 5;
				int maxyy = smaply << 4;
				if ( x > -128 && y > -160 && x < maxxx + 128 && y < maxyy + 160 )
				{
					NewFrame* NF = &SharBuild->Frames[frm];
					CurDrawNation = NT->NNUM;
					AddPoint( x, y, x + NF->dx, y + NF->dy, nullptr, NF->FileID, NF->SpriteID, 0, 0 );
				}
			}
		}
	}
}

BlockBars::BlockBars()
{
	BC = nullptr;
	NBars = 0;
	MaxBars = 0;
};
void BlockBars::Clear()
{
	if ( BC )free( BC );
	NBars = 0;
	MaxBars = 0;
	BC = nullptr;
};
BlockBars::~BlockBars()
{
	Clear();
};
bool BlockBars::Add( word x, word y )
{
	if ( NBars )
	{
		DWORD DT = x + ( y << 16 );
		DWORD* SDAT = (DWORD*) BC;
		for ( int i = 0; i < NBars; i++ )if ( SDAT[i] == DT )return false;
	};
	if ( NBars >= MaxBars )
	{
		MaxBars += 128;
		BC = (BlockCell*) realloc( BC, MaxBars * sizeof( BlockCell ) );
	};
	BC[NBars].x = x;
	BC[NBars].y = y;
	NBars++;
	return true;
};
bool BlockBars::FastAdd( word x, word y )
{
	if ( NBars >= MaxBars )
	{
		MaxBars += 128;
		BC = (BlockCell*) realloc( BC, MaxBars * sizeof( BlockCell ) );
	};
	BC[NBars].x = x;
	BC[NBars].y = y;
	NBars++;
	return true;
};
bool BlockBars::Delete( word x, word y )
{
	if ( NBars )
	{
		DWORD DT = x + ( y << 16 );
		DWORD* SDAT = (DWORD*) BC;
		int i;
		for ( i = 0; i < NBars&&SDAT[i] != DT; i++ );
		if ( i >= NBars )return false;
		if ( i < NBars - 1 )
		{
			memcpy( SDAT + i, SDAT + i + 1, ( NBars - i - 1 ) << 2 );
		};
		NBars--;
		return false;
	};
	return false;
};
void MoveAwayInCell( int cell, int x, int y )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )return;
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				int dist = Norma( OB->RealX - x, OB->RealY - y );
				if ( dist < 364 * 16 )
				{
					int dx = OB->RealX - x;
					int dy = OB->RealY - y;
					if ( dx || dy )
					{
						dx = ( 350 * 16 * dx ) / dist;
						dy = ( 350 * 16 * dy ) / dist;
						if ( !CheckBar( ( ( x + dx ) >> 8 ) - 2, ( ( y + dy ) >> 8 ) - 2, 4, 4 ) )
						{
							OB->NewMonsterSendTo( x + dx, y + dy, 128, 1 );
						};
					};
				};
			};
		};
	};
};
void MoveAway( int x, int y )
{
	int cell = ( ( y >> 11 ) << VAL_SHFCX ) + ( x >> 11 );
	int CELL0 = cell;
	int rx1 = 3;
	bool Capture = false;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				MoveAwayInCell( stcell, x, y );
			};
			stcell++;
		};
		stcell += VAL_MAXCX - rx2;
	};
};
bool MoveAwaySectorInCell( int cell, int x, int y, int r0, int r1, char Dir, int ddir )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )return false;
	int ofs1 = cell << SHFCELL;
	word MID;
	bool Pushed = false;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				int dist = Norma( OB->RealX - x, OB->RealY - y );
				if ( dist<r1&&dist>r0 )
				{
					int dx = OB->RealX - x;
					int dy = OB->RealY - y;
					if ( dx || dy )
					{
						char ndir = char( GetDir( dx, dy ) );
						if ( abs( ndir - Dir ) < ddir )
						{
							if ( OB->NewCurSprite >= OB->NewAnm->NFrames - FrmDec )
							{
								int r = TryToMove( OB, OB->RealDir, false );
								if ( r == -1 )TryToMove( OB, OB->RealDir, true );
								/*
								dx=(int(TCos[OB->RealDir])<<3);
								dy=(int(TSin[OB->RealDir])<<3);

								if(!CheckBar(((x+dx)>>8)-2,((y+dy)>>8)-2,4,4)){
									OB->NewMonsterSendTo(OB->RealX+dx,OB->RealY+dy,128+64,1);
								}else{
									OB->RealDir+=32;
									OB->GraphDir=OB->RealDir;
								};
								*/
							};
							Pushed = true;
						};
					};
				};
			};
		};
	};
	return Pushed;
};
bool MoveAwayInSector( int x, int y, int r0, int r1, char Dir, int ddir )
{
	int cell = ( ( y >> 11 ) << VAL_SHFCX ) + ( x >> 11 );
	int CELL0 = cell;
	int rx1 = 2;
	bool Capture = false;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	bool Pushed = false;
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				if ( MoveAwaySectorInCell( stcell, x, y, r0, r1, Dir, ddir ) )Pushed = true;
			};
			stcell++;
		};
		stcell += VAL_MAXCX - rx2;
	};
	return Pushed;
};
#define SectRAD (450*16)
#define SectMIN (32*16)
void GetUnitsInSector( int cell, int x, int y, char IniDir, int* My, int* Enemy, byte Mask )
{
	cell += VAL_MAXCX + 1;
	if ( cell >= 0 && cell < VAL_MAXCX*VAL_MAXCX )
	{
		int NMon = MCount[cell];
		if ( !NMon )return;
		int ofs1 = cell << SHFCELL;
		word MID;
		bool Pushed = false;
		for ( int i = 0; i < NMon; i++ )
		{
			MID = GetNMSL( ofs1 + i );
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )
				{
					int dx = OB->RealX - x;
					int dy = OB->RealY - y;
					int dist = Norma( dx, dy );
					if ( dist<SectRAD&&dist>SectMIN )
					{
						if ( dx || dy )
						{
							char ndir = char( GetDir( dx, dy ) );
							if ( abs( ndir - IniDir ) < 28 )
							{
								if ( OB->NMask&Mask )( *My )++;
								else ( *Enemy )++;
							};
						};
					};
				};
			};
		};
	};
};
bool TestUnitsInSectorForGrenade( OneObject* GRE )
{
	int x = GRE->RealX;
	int y = GRE->RealY;
	int cell = ( ( y >> 11 ) << VAL_SHFCX ) + ( x >> 11 );
	int CELL0 = cell;
	int rx1 = 4;
	bool Capture = false;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	bool Pushed = false;
	int NMY = 0;
	int NHIM = 0;
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCX*VAL_MAXCX )
			{
				GetUnitsInSector( stcell, x, y, GRE->RealDir, &NMY, &NHIM, GRE->NMask );
			};
			stcell++;
		};
		stcell += VAL_MAXCX - rx2;
	};
	return NMY < 9 && ( NMY + NMY + NMY <= NHIM );
};
int GetTopology1( int x, int y )
{
	int xc = x >> 2;
	int yc = y >> 2;
	word tr = SafeTopRef( xc, yc );
	if ( tr < 0xFFFE )return tr;
	else return 0xFFFF;
};
int GetWTopology1( int x, int y, byte LTP )
{
	int xc = x >> 2;
	int yc = y >> 2;
	if ( xc < 0 || yc < 0 || xc >= TopLx || yc >= TopLy )return 0xFFFF;
	word tr = GTOP[LTP].TopRef[xc + ( yc << TopSH )];
	if ( tr < 0xFFFE )return tr;
	else return 0xFFFF;
};
bool TopFindBestPosition( OneObject* OB, int* xd, int* yd, int R0, int Top, byte LTP )
{
	MotionField* MFI = MFIELDS + OB->LockType;
	if ( !OB )return false;
	if ( !MFI->CheckBar( ( *xd ) - 1, ( *yd ) - 1, OB->Lx + 1, OB->Lx + 1 ) )return true;
	int Lx = OB->Lx;
	int R1 = R0;
	int bx = *xd;
	int by = *yd;
	int bdist = 100000;
	int xxx = bx - 1;
	int yyy = by - 1;
	int LL = 2;
	int bx1, by1, dstn;
	int x = OB->x;
	int y = OB->y;
	while ( R1 )
	{
		for ( int i = 0; i <= LL; i++ )if ( !MFI->CheckBar( xxx + i - 1, yyy - 1, Lx, Lx ) )
		{
			bx1 = xxx + i;
			by1 = yyy;
			dstn = Norma( bx1 - x, by1 - y );
			if ( dstn < bdist&&GetWTopology1( bx1, by1, LTP ) == Top )
			{
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		for ( int i = 0; i <= LL; i++ )if ( !MFI->CheckBar( xxx + i - 1, yyy + LL - 1, Lx, Lx ) )
		{
			bx1 = xxx + i;
			by1 = yyy + LL;
			dstn = Norma( bx1 - x, by1 - y );
			if ( dstn < bdist&&GetWTopology1( bx1, by1, LTP ) == Top )
			{
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		for ( int i = 0; i < LL - 1; i++ )if ( !MFI->CheckBar( xxx - 1, yyy + i, Lx, Lx ) )
		{
			bx1 = xxx;
			by1 = yyy + i + 1;
			dstn = Norma( bx1 - x, by1 - y );
			if ( dstn < bdist&&GetWTopology1( bx1, by1, LTP ) == Top )
			{
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		for ( int i = 0; i < LL - 1; i++ )if ( !MFI->CheckBar( xxx + LL - 1, yyy + i, Lx, Lx ) )
		{
			bx1 = xxx + LL;
			by1 = yyy + i + 1;
			dstn = Norma( bx1 - x, by1 - y );
			if ( dstn < bdist&&GetWTopology1( bx1, by1, LTP ) == Top )
			{
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		if ( bdist < 100000 )
		{
			*xd = bx;
			*yd = by;
			return true;
		};
		R1--;
		LL += 2;
		xxx--;
		yyy--;
	};
	return false;
};
bool CheckDirectWay( int x0, int y0, int x1, int y1 )
{
	int bx = x0 << 10;
	int by = y0 << 10;
	int Lx = x1 - x0;
	int Ly = y1 - y0;
	int r = Norma( Lx, Ly );
	int N = ( r >> 6 ) + 1;
	Lx = ( Lx << 10 ) / N;
	Ly = ( Ly << 10 ) / N;
	for ( int i = 0; i < N; i++ )
	{
		bx += Lx;
		by += Ly;
		if ( CheckPt( bx >> 14, by >> 14 ) )return false;
	};
	return true;
};
int FindSuperSmartBestPosition( OneObject* OB, int* cx, int* cy, int dx, int dy, word Top, byte LTP )
{
	MotionField* MF = MFIELDS;;
	if ( OB )MF = MFIELDS + OB->LockType;

	word* MotionLinks = GTOP[LTP].MotionLinks;
	word* LinksDist = GTOP[LTP].LinksDist;
	int NAreas = GTOP[LTP].NAreas;
	Area* TopMap = GTOP[LTP].TopMap;
	word* TopRef = GTOP[LTP].TopRef;

	int LDX = -2;
	int LLX = 4;
	if ( OB&&OB->LockType )
	{
		LDX = -7;
		LLX = 14;
	};
	int x0 = ( *cx ) << 8;
	int y0 = ( *cy ) << 8;
	int N = ( Norma( dx, dy ) >> 5 ) + 1;
	int DX = ( dx << 8 ) / N;
	int DY = ( dy << 8 ) / N;
	int mx = msx >> 1;
	if ( !LTP )
	{
		int i;
		for ( i = 0; i < N; i++ )
		{
			x0 += DX;
			y0 += DY;
			int tx = x0 >> ( 8 + 6 );
			int ty = y0 >> ( 8 + 6 );
			if ( tx <= 0 || ty <= 0 || tx >= mx || ty >= mx )goto NextSm1;
			int ofs = tx + TopLx*ty;
			if ( TopRef[ofs] >= 0xFFFE )goto NextSm1;
		};
	NextSm1:;
		bool Prolong = true;
		if ( i != N )
		{
			x0 -= DX;
			y0 -= DY;
			Prolong = false;
		};
		N = i;
		if ( !MF->CheckBar( ( x0 >> ( 8 + 4 ) ) + LDX, ( y0 >> ( 8 + 4 ) ) + LDX, LLX, LLX ) )
		{
			*cx = x0 >> 8;
			*cy = y0 >> 8;
			//assert(!CheckBar((*cx>>4)-1,(*cy>>4)-1,3,3));
			return 1;
		};
		int xx = x0;
		int yy = y0;
		for ( int i = 0; i < N; i++ )
		{
			x0 -= DX;
			y0 -= DY;
			if ( !MF->CheckBar( ( x0 >> ( 8 + 4 ) ) + LDX, ( y0 >> ( 8 + 4 ) ) + LDX, LLX, LLX ) )
			{
				*cx = x0 >> 8;
				*cy = y0 >> 8;
				//assert(!CheckBar((*cx>>4)-1,(*cy>>4)-1,3,3));
				return 2;
			};
		};
		if ( Prolong )
		{
			x0 = xx;
			y0 = yy;
			for ( int i = 0; i < 5; i++ )
			{
				x0 += DX;
				y0 += DY;
				int tx = x0 >> ( 8 + 6 );
				int ty = y0 >> ( 8 + 6 );
				if ( tx <= 0 || ty <= 0 || tx >= mx || ty >= mx )goto NextSm2;
				int ofs = tx + TopLx*ty;
				if ( TopRef[ofs] >= 0xFFFE )goto NextSm2;
				if ( !MF->CheckBar( ( x0 >> ( 8 + 4 ) ) + LDX, ( y0 >> ( 8 + 4 ) ) + LDX, LLX, LLX ) )
				{
					*cx = x0 >> 8;
					*cy = y0 >> 8;
					//assert(!CheckBar((*cx>>4)-1,(*cy>>4)-1,3,3));
					return 3;
				};
			};
		};
	};
NextSm2:;
	if ( !OB )return false;
	int ccx = ( *cx ) >> 4;
	int ccy = ( *cy ) >> 4;
	if ( TopFindBestPosition( OB, &ccx, &ccy, 60, Top, LTP ) )
	{
		*cx = ccx << 4;
		*cy = ccy << 4;
		//assert(!CheckBar((*cx>>4)-1,(*cy>>4)-1,3,3));
		return 4;
	};
	return false;
};
int GetWTopology( int x, int y, byte LTP )
{
	if ( !GTOP[LTP].NAreas )return 0xFFFF;
	int xc = x >> 6;
	int yc = y >> 6;
	if ( xc < 0 || yc < 0 || xc >= TopLx || yc >= TopLy )return 0xFFFF;
	word* TopRef = GTOP[LTP].TopRef;
	word tr = TopRef[xc + ( yc << TopSH )];
	if ( tr < 0xFFFE )return tr;
	for ( int i = 0; i < 10; i++ )
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for ( int j = 0; j < N; j++ )
		{
			int xx = xc + xi[j];
			int yy = yc + yi[j];
			if ( xx < 0 || yy < 0 || xx >= TopLx || yy >= TopLy )tr = 0xFFFF;
			else tr = TopRef[xx + ( yy << TopSH )];
			if ( tr < 0xFFFE )return tr;
		};
	};
	return 0xFFFF;
};
int GetWTopology( int* x, int* y, byte LTP )
{
	int xc = ( *x ) >> 6;
	int yc = ( *y ) >> 6;
	if ( xc < 0 || yc < 0 || xc >= TopLx || yc >= TopLy )return 0xFFFF;
	word* TopRef = GTOP[LTP].TopRef;
	word tr = TopRef[xc + ( yc << TopSH )];
	if ( tr < 0xFFFE )return tr;
	for ( int i = 0; i < 10; i++ )
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for ( int j = 0; j < N; j++ )
		{
			int xx = xc + xi[j];
			int yy = yc + yi[j];
			if ( xx < 0 || yy < 0 || xx >= TopLx || yy >= TopLy )tr = 0xFFFF;
			else tr = TopRef[xx + ( yy << TopSH )];
			if ( tr < 0xFFFE )
			{
				*x = ( xx << 6 ) + 32;
				*y = ( yy << 6 ) + 32;
				return tr;
			};
		};
	};
	return 0xFFFF;
};
void NewMonsterSmartSendToLink( OneObject* OBJ )
{
	Order1* OR1 = OBJ->LocalOrder;
	OBJ->PrioryLevel = OR1->PrioryLevel;
	int x = OR1->info.SmartSend.x;
	int y = OR1->info.SmartSend.y;
	int dx = OR1->info.SmartSend.dx;
	int dy = OR1->info.SmartSend.dy;
	int NextX = OR1->info.SmartSend.NextX;
	int NextY = OR1->info.SmartSend.NextY;
	int NextTop = OR1->info.SmartSend.NextTop;
	byte LTP = OBJ->LockType;

	word* MotionLinks = GTOP[LTP].MotionLinks;
	word* LinksDist = GTOP[LTP].LinksDist;
	int NAreas = GTOP[LTP].NAreas;
	Area* TopMap = GTOP[LTP].TopMap;

	if ( CheckTopDirectWay( OBJ->RealX >> 4, OBJ->RealY >> 4, x + dx, y + dy, OBJ->LockType ) )
	{
		int prior = OR1->PrioryLevel;
		OBJ->DeleteLastOrder();
		OBJ->NewMonsterSendTo( ( x + dx ) << 4, ( y + dy ) << 4, prior, 1 );
		return;
	};
	if ( NextTop == 0xFFFF )
	{
		NextTop = GetWTopology( OBJ->RealX >> 4, OBJ->RealY >> 4, LTP );
		if ( NextTop == 0xFFFF )
		{
			OBJ->DeleteLastOrder();
			return;
		};
	}
	else
	{
		//if(Norma(x-(OBJ->RealX>>4),y-(OBJ->RealY>4))>64){
		//	OBJ->NewMonsterSendTo(NextX<<4,NextY<<4,OR1->PrioryLevel,1);
		//	return;
		//};
	};
	int FinalTop = GetWTopology( &x, &y, LTP );
	if ( FinalTop == 0xFFFF )
	{
		OBJ->DeleteLastOrder();
		return;
	};
	OR1->info.SmartSend.x = x;
	OR1->info.SmartSend.y = y;
	word NextNextTop = MotionLinks[FinalTop + NAreas*NextTop];
	if ( NextNextTop == FinalTop || FinalTop == NextTop )
	{
		int prio = OR1->PrioryLevel;
		OBJ->DeleteLastOrder();
		if ( !FindSuperSmartBestPosition( OBJ, &x, &y, dx, dy, FinalTop, LTP ) )
		{
			OBJ->DeleteLastOrder();
			return;
		};
		OBJ->NewMonsterSendTo( x << 4, y << 4, prio, 1 );
		return;
	};
	if ( NextNextTop != 0xFFFF )
	{
		//atttempt to optomise way
		int MaxPre = 3;
		int cox = OBJ->RealX >> 4;
		int coy = OBJ->RealY >> 4;
		do
		{
			int Next2 = MotionLinks[FinalTop + NAreas*NextNextTop];
			if ( Next2 != 0xFFFF )
			{
				Area* AR2 = TopMap + Next2;
				int nx = ( int( AR2->x ) << 6 ) + 32;
				int ny = ( int( AR2->y ) << 6 ) + 32;
				int rx = nx + dx;
				int ry = ny + dy;
				if ( CheckTopDirectWay( cox, coy, rx, ry, OBJ->LockType ) )
				{
					NextNextTop = Next2;
					MaxPre--;
				}
				else MaxPre = 0;
			}
			else MaxPre = 0;
		} while ( MaxPre );
		Area* NextAr = TopMap + NextNextTop;
		//------------------------
		int nx = ( int( NextAr->x ) << 6 ) + 32;
		int ny = ( int( NextAr->y ) << 6 ) + 32;
		int rx = nx + dx;
		int ry = ny + dy;
		//if(CheckBar((rx>>4)-2,(ry>>4)-2,4,4)){
			//search for better place
		int xx0 = nx;
		int yy0 = ny;
		if ( !FindSuperSmartBestPosition( OBJ, &xx0, &yy0, dx, dy, NextNextTop, LTP ) )
		{
			OBJ->DeleteLastOrder();
			return;
		};
		rx = xx0;
		ry = yy0;
		//};
		OR1->info.SmartSend.NextX = rx;
		OR1->info.SmartSend.NextY = ry;
		OR1->info.SmartSend.NextTop = NextNextTop;
		OBJ->NewMonsterSendTo( rx << 4, ry << 4, OR1->PrioryLevel, 1 );
		return;
	}
	else
	{
		OBJ->DeleteLastOrder();
		return;
	}
}

#undef NewMonsterSmartSendTo
void OneObject::NewMonsterSmartSendTo( int px, int py, int dx, int dy, byte Prio, byte OrdType )
{
	if ( !NAreas )
	{
		return;
	}
	if ( NewBuilding || !Ready )
	{
		return;
	}
	int RR = Norma( dx, dy );
	if ( RR > 128 )
	{
		int dx1 = ( dx * 120 ) / RR;
		int dy1 = ( dy * 90 ) / RR;
		NewMonsterSmartSendTo( px + dx - dx1, py + dy - dy1, dx1, dy1, Prio, OrdType );
		return;
	}
	if ( newMons->LockType )
	{
		//NewMonsterSendTo((x+dx)<<4,(y+dy)<<4,Prio,OrdType);
		//return;
	}
	/*
	if(BrigadeID!=0xFFFF){
		Brigade* BR=Nat->CITY->Brigs+BrigadeID;
		if(BR->WarType){
			BR->HumanGlobalSendTo(x,y,LastDirection,Prio,OrdType);
			return;
		};
	};
	*/
	if ( px < 64 )
	{
		px = 64;
	}
	if ( py < 64 )
	{
		py = 64;
	}
	if ( Prio < PrioryLevel )
	{
		return;
	}
	word Top = GetWTopology( &px, &py, LockType );
	if ( Top == 0xFFFF )
	{
		return;
	}
	Order1* OR1 = CreateOrder( OrdType );
	if ( !OR1 )
	{
		return;
	}
	OR1->DoLink = (ReportFn*) ( SmartLink - 0x7654391 );//&NewMonsterSmartSendToLink;
	OR1->PrioryLevel = Prio & 127;
	OR1->info.SmartSend.x = px;
	OR1->info.SmartSend.y = py;
	OR1->info.SmartSend.dx = dx;
	OR1->info.SmartSend.dy = dy;
	OR1->info.SmartSend.NextX = 0xFFFF;
	OR1->info.SmartSend.NextY = 0xFFFF;
	OR1->info.SmartSend.NextTop = 0xFFFF;
	if ( OrdType == 0 || OrdType == 1 )
	{
		PrioryLevel = Prio & 127;
	}
	if ( OR1->PrioryLevel == 0 )
	{
		InPatrol = true;
	}
	if ( !( Prio & 127 ) )
	{
		if ( newMons->PMotionL[0].Enabled && !newMons->MaxAIndex )
		{
			GroundState = 1;
		}
	}
	else
	{
		GroundState = 0;
	}
}

#define NewMonsterSmartSendTo(x,y,dx,dy,Prio,OrdType) NewMonsterSmartSendTo(x,y,dx,dy,Prio,OrdType);
int CurTR = 2;
int PreMID = 0xFFFF;
#define DLR 3
int CheckPointForDamageAbility( OneObject* OBJ, int x, int y, int z );

void ProcessSelectedTower()
{
	if ( NSL[MyNation] == 1 )
	{
		word MID = Selm[MyNation][0];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->newMons->Usage == TowerID )
			{
				if ( PreMID != MID )CurTR = 2;
				PreMID = MID;
				int N = Rarr[CurTR].N;
				char* xi = Rarr[CurTR].xi;
				char* yi = Rarr[CurTR].yi;
				int nn = 0;
				int xc = OB->RealX >> 10;
				int yc = OB->RealY >> 10;
				int mpdx = mapx << 5;
				int mpdy = mapy << 4;
				int MLX = smaplx << 5;
				int MLY = smaply << 4;
				for ( int p = 0; p < N; p++ )
				{
					int xx = ( ( xc + int( xi[p] ) ) << 6 ) + 32;
					int yy = ( ( yc + int( yi[p] ) ) << 6 ) + 32;
					int zz = GetHeight( xx, yy );
					if ( !CheckPointForDamageAbility( OB, xx, yy, zz + 32 ) )
					{
						nn++;
						int xs = xx - mpdx;
						int ys = ( yy >> 1 ) - mpdy - zz;
						if ( xs > 0 && ys > 0 && xs < MLX&&ys < MLY )
						{
							int ys1 = ( ( yy - DLR ) >> 1 ) - mpdy - GetHeight( xx - DLR, yy - DLR );
							int ys2 = ( ( yy - DLR ) >> 1 ) - mpdy - GetHeight( xx + DLR, yy - DLR );
							int ys3 = ( ( yy + DLR ) >> 1 ) - mpdy - GetHeight( xx - DLR, yy + DLR );
							int ys4 = ( ( yy + DLR ) >> 1 ) - mpdy - GetHeight( xx + DLR, yy + DLR );
							DrawLine( xs - DLR, ys1, xs + DLR, ys4, 255 );
							DrawLine( xs - DLR, ys3, xs + DLR, ys2, 255 );
						}
					}
				}
				if ( nn || CurTR < 12 )
				{
					CurTR++;
				}
				else
				{
					CurTR = 2;
				}
			}
		}
	}
}

//------------------------smart motion-----------------------
void CorrectTopPoint( int* cx, int* cy, int dx, int dy )
{
	if ( !( dx || dy ) )
	{
		return;
	}
	int NR = Norma( dx, dy );
	int x0 = ( *cx ) << 4;
	int y0 = ( *cy ) << 4;
	int N = ( NR >> 6 ) + 1;
	int sx = ( dx << 4 ) / N;
	int sy = ( dy << 4 ) / N;
	for ( int i = 0; i < N; i++ )
	{
		x0 += sx;
		y0 += sy;
		int tx = x0 >> 10;
		int ty = y0 >> 10;
		if ( tx > 0 && ty > 0 && tx < TopLx&&ty < TopLy )
		{
			int ofs = tx + ( ty << TopSH );
			if ( TopRef[ofs] >= 0xFFFE )
			{
				x0 -= sx;
				y0 -= sy;
				*cx = x0 >> 4;
				*cy = y0 >> 4;
				return;
			}
		}
	}
}

void CorrectBlocking( int* cx, int* cy )
{
	if ( !CheckBar( ( ( *cx ) >> 4 ) - 2, ( ( *cy ) >> 4 ) - 2, 4, 4 ) )
	{
		return;
	}
	int top = GetTopology( *cx, *cy );
	if ( top >= 0xFFFE )
	{
		//need to find free point with the same topology
		int tcx = ( *cx ) >> 6;
		int tcy = ( *cy ) >> 6;
		for ( int r = 1; r < 10; r++ )
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int N = Rarr[r].N;
			for ( int j = 0; j < N; j++ )
			{
				int tx = tcx + xi[j];
				int ty = tcy + yi[j];
				if ( tx > 0 && ty > 0 && tx < TopLx&&ty < TopLy )
				{
					int ofs = tx + ( ty << TopSH );
					if ( TopRef[ofs] == top )
					{
						if ( !CheckBar( ( tx << 2 ), ( ty << 2 ), 4, 4 ) )
						{
							*cx = ( tx << 6 ) + 32;
							*cy = ( ty << 6 ) + 32;
						}
					}
				}
			}
		}
	}
}

bool CheckTopDirectWay( int x0, int y0, int x1, int y1, byte TopType )
{
	word* TopRef = GTOP[TopType].TopRef;
	x0 <<= 8;
	y0 <<= 8;
	x1 <<= 8;
	y1 <<= 8;
	int dx = x1 - x0;
	int dy = y1 - y0;
	int N = ( Norma( dx, dy ) >> ( 8 + 6 ) ) + 1;
	dx /= N;
	dy /= N;
	for ( int i = 0; i < N; i++ )
	{
		x0 += dx;
		y0 += dy;
		int tx = x0 >> ( 8 + 6 );
		int ty = y0 >> ( 8 + 6 );
		if ( tx > 0 && ty > 0 && tx < TopLx&&ty < TopLy )
		{
			if ( TopRef[tx + ( ty << TopSH )] >= 0xFFFE )
			{
				return false;
			}
		}
		else
		{
			return false;
		}

	}
	return true;
}

void OneObject::FindNextSmartPoint()
{
	bm_NextX = -1;
	bm_NextY = -1;
	int lx = bm_DestX;
	int ly = bm_DestY;
	int FinalTop = GetTopology( &lx, &ly );
	if ( FinalTop >= 0xFFFE )
	{
		return;
	}
	int MyX = RealX >> 4;
	int MyY = RealY >> 4;
	int MyTop = GetTopology( &MyX, &MyY );
	if ( MyTop >= 0xFFFE )
	{
		return;
	}
	if ( MyTop == FinalTop )
	{
		bm_NextX = bm_DestX;
		bm_NextY = bm_DestY;
		return;
	}
	int MMTP = MyTop*NAreas;
	int NextTop = MotionLinks[MMTP + FinalTop];
	if ( NextTop >= 0xFFFE )
	{
		return;
	}
	if ( NextTop == FinalTop )
	{
		int cx = bm_DestX;
		int cy = bm_DestY;
		CorrectTopPoint( &cx, &cy, bm_dx, bm_dy );
		CorrectBlocking( &cx, &cy );
		return;
	}
	Area* AR = TopMap + NextTop;
	int NxX = ( int( AR->x ) << 6 ) + 32;
	int NxY = ( int( AR->y ) << 6 ) + 32;
	CorrectTopPoint( &NxX, &NxY, bm_dx, bm_dy );
	bm_NextX = NxX;
	bm_NextY = NxY;
	bm_NextTop = NextTop;
	if ( CheckTopDirectWay( MyX, MyY, NxX, NxY, LockType ) )
	{
		//attempt to optimise way
		int MaxPre = 3;
		while ( MaxPre && NextTop != FinalTop )
		{
			NextTop = MotionLinks[NextTop*NAreas + FinalTop];
			if ( NextTop < 0xFFFE )
			{
				if ( NextTop == FinalTop )
				{
					NxX = bm_DestX;
					NxY = bm_DestY;
				}
				else
				{
					Area* AR = TopMap + NextTop;
					NxX = ( int( AR->x ) << 6 ) + 32;
					NxY = ( int( AR->y ) << 6 ) + 32;
				}
				CorrectTopPoint( &NxX, &NxY, bm_dx, bm_dy );
				if ( CheckTopDirectWay( MyX, MyY, bm_dx, bm_dy, LockType ) )
				{
					bm_NextX = NxX;
					bm_NextY = NxY;
					bm_NextTop = NextTop;
				}
				else
				{
					int cx = bm_NextX;
					int cy = bm_NextY;
					CorrectBlocking( &cx, &cy );
					bm_NextX = cx;
					bm_NextY = cy;
					return;
				}
			}
		}
	}
	int cx = bm_NextX;
	int cy = bm_NextY;
	CorrectBlocking( &cx, &cy );
	bm_NextX = cx;
	bm_NextY = cy;
}

void OneObject::CreateSmartPath( int px, int py, int dx, int dy )
{
	if ( px == bm_DestX && py == bm_DestY && dx == bm_DestY && dy == bm_DestY )
	{

	}
	else
	{

	}
}

int EnumUnitsInCell( int cell, int x, int y, int r, word Type, byte Nation )
{
	int N = 0;
	if ( cell > 0 )
	{
		cell += VAL_MAXCX + 1;
		if ( cell >= VAL_MAXCX*VAL_MAXCX )
		{
			return 0;
		}
		int NMon = MCount[cell];
		if ( !NMon )
		{
			return 0;
		}
		int ofs1 = cell << SHFCELL;
		word MID;
		for ( int i = 0; i < NMon; i++ )
		{
			MID = GetNMSL( ofs1 + i );
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && ( !( OB->Guard != 0xFFFF || OB->Sdoxlo ) ) &&
					OB->BrigadeID == 0xFFFF &&
					OB->NNUM == Nation &&
					OB->NIndex == Type &&
					Norma( OB->RealX - x, OB->RealY - y ) < r )
				{
					N++;
				}
			}
		}
	}
	return N;
}

int EnumUnitsInRound( int x, int y, int r, word Type, byte Nation )
{
	int rx1 = ( r >> 11 ) + 1;
	int N = 0;
	int x0 = x >> 11;
	int y0 = y >> 11;
	int maxx = msx >> 2;
	for ( int R = 0; R <= rx1; R++ )
	{
		char* xi = Rarr[R].xi;
		char* yi = Rarr[R].yi;
		int NN = Rarr[R].N;
		for ( int p = 0; p < NN; p++ )
		{
			int xx = x0 + xi[p];
			int yy = y0 + yi[p];
			if ( xx >= 0 && yy >= 0 && xx < maxx&&yy < maxx )
			{
				int cell = xx + ( yy << VAL_SHFCX );
				N += EnumUnitsInCell( cell, x, y, r, Type, Nation );
			}
		}
	}
	return N;
}

int CheckShipDirection( char Dir )
{
	word MinD = 127;
	char BestD = 0;
	if ( abs( Dir - 32 ) < MinD )
	{
		BestD = 32;
		MinD = abs( Dir - 32 );
	}
	if ( abs( Dir + 32 ) < MinD )
	{
		BestD = -32;
		MinD = abs( Dir + 32 );
	}
	if ( abs( Dir - 96 ) < MinD )
	{
		BestD = 96;
		MinD = abs( Dir - 86 );
	}
	if ( abs( Dir + 96 ) < MinD )
	{
		BestD = -96;
		MinD = abs( Dir + 96 );
	}
	char D = Dir - BestD;
	if ( abs( D ) < 4 )
	{
		return 0;
	}
	if ( D > 0 )
	{
		return -2;
	}
	return 2;
}

void RotateShipAndDieLink( OneObject* OBJ )
{
	char DD = CheckShipDirection( OBJ->RealDir );
	if ( DD == 0 )
	{
		OBJ->ClearOrders();
		OBJ->MaxDelay = 2000;
		OBJ->delay = 2000;
		OBJ->Die();
		return;
	}
	OBJ->RealDir += DD;
	OBJ->GraphDir = OBJ->RealDir;
}

void RotateShipAndDie( OneObject* OBJ )
{
	Order1* OR1 = OBJ->CreateOrder( 0 );
	OR1->DoLink = &RotateShipAndDieLink;
	OR1->PrioryLevel = 255;
	OBJ->PrioryLevel = 255;
	OBJ->RealVx = 0;
	OBJ->RealVy = 0;
	OBJ->Speed = 0;
	OBJ->DestX = -1;
	OBJ->DestY = -1;
	NewMonster* NM = OBJ->newMons;
	if ( NM->Destruct.NWeap )
	{
		FogRec* FR = &NM->Destruct;
		if ( FR->NWeap )
		{
			for ( int j = 0; j < FR->NWeap; j++ )
			{
				int xp = ( OBJ->RealX >> 4 ) + ( rando() % 180 ) - 90;
				int yp = ( OBJ->RealY >> 4 ) + ( rando() % 180 ) - 90;
				int nw = j;

				Create3DAnmObject( WPLIST[FR->Weap[nw]], xp, yp, GetHeight( xp, yp ) + 60, xp, yp, 1000, nullptr, 0, 0xFFFF );
			}
		}
	}
}

///DEBUG
char LASTFILE[128];
int LastLine;
byte NatRefTBL[8] = { 0,1,2,3,4,5,6,7 };

void DoNormalTBL()
{
	for ( int i = 0; i < 8; i++ )
	{
		NatRefTBL[i] = i;
	}
}

void ProcessGuard()
{
	if ( ( tmtmt % 32 ) == 5 )
	{
		for ( int i = 0; i < MAXOBJECT; i++ )
		{
			OneObject* OB = Group[i];
			if ( OB && ( !OB->Sdoxlo ) )
			{
				word GUID = OB->Guard;
				if ( GUID != 0xFFFF )
				{
					OneObject* GUOB = Group[GUID];
					if ( GUOB && !GUOB->Sdoxlo )
					{
						int R = Norma( OB->RealX - GUOB->RealX, OB->RealY - GUOB->RealY ) >> 4;
						if ( GUOB->NewBuilding && R > 250 )
						{
							int vx = ( GUOB->RealX - OB->RealX ) >> 4;
							int vy = ( GUOB->RealY - OB->RealY ) >> 4;
							int NR = Norma( vx, vy );

							vx = ( 150 * vx ) / NR;
							vy = ( 150 * vy ) / NR;

							OB->NewMonsterSmartSendTo(
								( GUOB->RealX >> 4 ) - vx,
								( GUOB->RealY >> 4 ) - vy,
								0, 0, 16 + 128, 0 );
						}
						if ( ( !GUOB->NewBuilding ) && R > 150 )
						{
							int vx = ( GUOB->RealX - OB->RealX ) >> 4;
							int vy = ( GUOB->RealY - OB->RealY ) >> 4;
							int NR = Norma( vx, vy );

							vx = ( 64 * vx ) / NR;
							vy = ( 64 * vy ) / NR;

							OB->NewMonsterSmartSendTo(
								( GUOB->RealX >> 4 ) - vx,
								( GUOB->RealY >> 4 ) - vy,
								0, 0, 16 + 128, 0 );
						}
					}
					else
					{
						OB->Guard = 0xFFFF;
					}
				}
			}
		}
	}
}