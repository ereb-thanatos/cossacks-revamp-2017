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
#include "Nucl.h"

#include "Megapolis.h"
#include "mode.h"
#include "MapSprites.h"
#include <assert.h>
#include "GP_Draw.h"
int mul3( int );

void GetRect( OneObject* ZZ, int* x, int* y, int* Lx, int* Ly );

void OneObject::SetDstPoint( int xd, int yd )
{
	if (!Ref.General->CanDest)
	{
		return;
	}

	DstX = xd;
	DstY = yd;
}

int DestnX[512];
int DestnY[512];
int NDestn;

//NDestn = 0
void InitDestn()
{
	NDestn = 0;
}

void AddDestn( int x, int y )
{
	if (NDestn < 512)
	{
		DestnX[NDestn] = x;
		DestnY[NDestn] = y;
		NDestn++;
	}
}

extern RLCTable RCross;
extern RLCTable mRCross;
extern int tmtmt;
void miniShowDestn()
{
	if (( tmtmt & 7 ) < 3)return;
	int smx = mapx + smaplx;
	int smy = mapy + smaply;
	byte x, y;
	for (int i = 0; i < NDestn; i++)
	{
		x = DestnX[i];
		y = DestnY[i];
		if (x >= mapx&&y > mapy&&x < smx&&y < smy)
		{
			ShowRLCItem( smapx + ( int( x - mapx ) << 4 ), smapy + ( int( y - mapy ) << 4 ), &RCross, 0, NatRefTBL[MyNation] );
		};
	};
};
void ShowDestn()
{
	if (MiniMode)
	{
		miniShowDestn();
		return;
	};
	if (( tmtmt & 7 ) < 3)return;
	int smx = mapx + smaplx;
	int smy = mapy + smaply;
	int x, y, y1;
	int dx = mapx << 5;
	int dy = mul3( mapy ) << 3;
	int Lx = smaplx << 5;
	int Ly = mul3( smaply ) << 3;
	for (int i = 0; i < NDestn; i++)
	{
		x = DestnX[i];
		y1 = DestnY[i];
		y = mul3( DestnY[i] ) >> 2;
		y -= GetHeight( x, y1 ) + dy;
		x -= dx;
		if (x >= 0 && y >= 0 && x <= Lx&&y <= Ly)
		{
			ShowRLCItem( smapx + x - 16, smapy + y - 16, &RCross, 0, NatRefTBL[MyNation] );
		};
	};
};
extern int MiniLx;
extern int MiniLy;
extern int MiniX, MiniY;

void ShowMiniDestn()
{
	if (( tmtmt % 8 ) < 5)
	{
		return;
	}

	int x, y;
	for (int i = 0; i < NDestn; i++)
	{
		x = minix + ( DestnX[i] >> ( 5 + ADDSH ) ) - MiniX;
		y = miniy + ( DestnY[i] >> ( 5 + ADDSH ) ) - MiniY;
		Hline( x, y, x, clrYello );
		Hline( x - 1, y - 1, x - 1, clrYello );
		Hline( x + 1, y + 1, x + 1, clrYello );
		Hline( x + 1, y - 1, x + 1, clrYello );
		Hline( x - 1, y + 1, x - 1, clrYello );
	}
}

class GreenPulse
{
public:
	word GreenObjs[256];
	word GreenTime[256];
	byte ObjColor[256];
	byte ObjOpt[256];
	void AddObj( word ID, byte Color );
	void AddXY( int x, int y );
	GreenPulse();
	void Clear();
	void Show();
};

GreenPulse::GreenPulse()
{
	memset( GreenObjs, 0xFF, 512 );
	memset( GreenTime, 0, 512 );
}

void GreenPulse::AddObj( word ID, byte Color )
{
	int i;
	for (i = 0; i < 256 && GreenObjs[i] != 0xFFFF; i++);

	if (i < 256)
	{
		GreenObjs[i] = ID;
		GreenTime[i] = 24;
		ObjColor[i] = Color;
		ObjOpt[i] = 255;
	}
}

void GreenPulse::AddXY( int x, int y )
{
	int i;
	for (i = 0; i < 256 && GreenObjs[i] != 0xFFFF; i++);

	if (i < 256)
	{
		GreenObjs[i] = x;
		GreenTime[i] = y;
		ObjColor[i] = 0;
		ObjOpt[i] = 0;
	}
}

void GreenPulse::Clear()
{
	memset( GreenObjs, 0xFF, 512 );
	memset( GreenObjs, 0xFF, 512 );
	memset( ObjOpt, 0, 256 );
}

extern int AnmCursorGP;
void DrawColorMarker( OneObject* OB, byte cl );
extern bool Mode3D;

void GreenPulse::Show()
{
	bool show = 1;
	if (( tmtmt >> 2 ) & 1)show = 0;
	for (int i = 0; i < 256; i++)
	{
		if (ObjOpt[i] == 255)
		{
			word ID = GreenObjs[i];
			if (ID != 0xFFFF && show)
			{
				GreenTime[i]--;
				if (GreenTime[i])
				{
					OneObject* OB = Group[ID];
					if (OB)
					{
						DrawColorMarker( OB, ObjColor[i] );
					};
				}
				else GreenObjs[i] = 0xFFFF;
			};
		}
		else
		{
			int x = GreenObjs[i];
			int y = GreenTime[i];
			if (!GPS.GPH[AnmCursorGP])GPS.LoadGP( AnmCursorGP );
			if (x&&y)
			{
				if (ObjOpt[i] < GPS.GPNFrames[AnmCursorGP])
				{
					int spr = ObjOpt[i];
					int dy = 0;
					if (Mode3D)dy = GetHeight( x, y );
					GPS.ShowGP( x - ( mapx << 5 ) - 32, ( y >> 1 ) - ( mapy << 4 ) - 35 - dy, AnmCursorGP, spr, 0 );
					ObjOpt[i]++;
				}
				else
				{
					GreenObjs[i] = 0xFFFF;
					GreenTime[i] = 0;
					ObjOpt[i] = 0xFF;
				}
			}
		}
	}
}

GreenPulse PULSAR;

void InitPulse()
{
	PULSAR.Clear();
}

void AddPulse( word ObjID, byte c )
{
	PULSAR.AddObj( ObjID, c );
}

void AddXYPulse( int x, int y )
{
	PULSAR.AddXY( x, y );
}

void ShowPulse()
{
	PULSAR.Show();
}

//-----------------------------------National list---------------------------------
word* NatList[8];
int   NtNUnits[8];
int   NtMaxUnits[8];
byte NUnitsCHK[8192];
bool HealDoubleUnits()
{
	for (int i = 0; i < 8; i++)
	{
		int N = NtNUnits[i];
		word* units = NatList[i];
		for (int j = 0; j < N; j++)
		{
			OneObject* OB = Group[units[j]];
			if (( !OB ) || OB->NNUM != i)
			{
				if (j < N - 1)
				{
					memcpy( units + j, units + j + 1, ( N - j - 1 ) * 2 );
				};
				NtNUnits[i]--;
				return true;
			}
		};
	};
	return false;
};

void SetupNatList()
{
	for (int i = 0; i < 8; i++)
	{
		NtNUnits[i] = 0;
		NtMaxUnits[i] = 256;
		NatList[i] = new word[256];
	}
}

void InitNatList()
{
	for (int i = 0; i < 8; i++)
	{
		free( NatList[i] );
	}
	SetupNatList();
}

void AddObject( OneObject* OB )
{
	word ID = OB->Index;
	word NNUM = OB->NNUM;

	if (NtNUnits[NNUM] == NtMaxUnits[NNUM])
	{
		NtMaxUnits[NNUM] += 1024;
		NatList[NNUM] = (word*) realloc( NatList[NNUM], 2 * NtMaxUnits[NNUM] );
	}

	NatList[NNUM][NtNUnits[NNUM]] = ID;
	NtNUnits[NNUM]++;
}

void DelObject( OneObject* OB )
{
	word ID = OB->Index;
	for (int NNUM = 0; NNUM < 8; NNUM++)
	{
		int N = NtNUnits[NNUM];
		word* UList = NatList[NNUM];
		for (int j = 0; j < N; j++)
		{
			if (UList[j] == ID)
			{
				if (j < N - 1)
				{
					memcpy( UList + j, UList + j + 1, ( N - j - 1 ) * 2 );
				}
				NtNUnits[NNUM]--;
				return;
			}
		}
	}
}
