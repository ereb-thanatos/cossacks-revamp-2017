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
#include "dialogs.h"
#include <assert.h>
#include <math.h>
#include "Masks.h"
#include "fonts.h"
#include "3DGraph.h"
#include "MapSprites.h"
#include "3DmapEd.h"
#include "NewMon.h"
#include "ZBuffer.h"
#include <crtdbg.h>
#include "TopoGraf.h"

int GetTopology( int x, int y );
bool CheckSpritesInArea( int x, int y, int r );
int MAXSPR;
word GetDir( int dx, int dy );
void UnregisterSprite( int N );
int Prop43( int y );
SprGroup TREES;
SprGroup STONES;
SprGroup HOLES;
SprGroup SPECIAL;
SprGroup COMPLEX;

//returns GetHeight(x,y)
int GetUnitHeight( int x, int y )
{
	return GetHeight( x, y );
}

byte* NSpri;//16384

int** SpRefs;
OneSprite* Sprites = nullptr;

word OneObject::FindNearestBase()
{
	OneObject* OB;
	byte ResID = RType;
	byte msk = 1 << ResID;
	int dist = 10000000;
	word OID = 0xFFFF;

	for (int i = 0; i < MAXOBJECT; i++)
	{
		OB = Group[i];
		if (OB && OB->Ready && ( OB->AbRes&msk ) && OB->NNUM == NNUM)
		{
			int dist1 = Norma( RealX - OB->RealX, RealY - OB->RealY );
			if (dist1 < dist)
			{
				OID = i;
				dist = dist1;
			}
		}
	}
	return OID;
}

int LastAddSpr = 0;

void DeleteAllSprites()
{
	for (int i = 0; i < MaxSprt; i++)
	{
		Sprites[i].Enabled = false;
	}
	memset( NSpri, 0, VAL_SPRSIZE );
	int nn = VAL_SPRSIZE;
	for (int i = 0; i < nn; i++)
	{
		if (SpRefs[i])
		{
			free( SpRefs[i] );
			SpRefs[i] = nullptr;
		}
	}
	MAXSPR = 0;
	LastAddSpr = 0;

	if (Sprites)
		free( Sprites );

	Sprites = nullptr;
	MaxSprt = 0;
	ObjTimer.NMembers = 0;
}

void CHKS();

void RegisterSprite( int N )
{
	OneSprite* OSP = &Sprites[N];
	if (OSP->x < 0)OSP->x = 0;
	if (OSP->y < 0)OSP->y = 0;
	int nn = ( OSP->x >> 7 ) + ( ( OSP->y >> 7 ) << SprShf );
	if (SpRefs[nn])
	{
		//search for existing reference
		int nsp = NSpri[nn];
		int* SPR = SpRefs[nn];
		for (int i = 0; i < nsp; i++)if (SPR[i] == N)return;
		if (( nsp & 0xFFFFF0 ) == nsp)
		{
			int* spp = new int[nsp + 16];
			memcpy( spp, SPR, nsp << 2 );
			free( SPR );
			SPR = spp;
			SpRefs[nn] = spp;
		}
		SPR[nsp] = N;
		NSpri[nn]++;
	}
	else
	{
		SpRefs[nn] = new int[16];
		SpRefs[nn][0] = N;
		NSpri[nn]++;
	}
}

void UnregisterSprite( int N )
{
	OneSprite* OSP = &Sprites[N];
	int nn = ( OSP->x >> 7 ) + ( ( OSP->y >> 7 ) << SprShf );
	if (SpRefs[nn])
	{
		//search for existing reference
		int nsp = NSpri[nn];
		int* SPR = SpRefs[nn];
		for (int i = 0; i < nsp; i++)if (SPR[i] == N)
		{
			if (i < nsp - 1)
			{
				memcpy( SPR + i, SPR + i + 1, ( nsp - i - 1 ) << 2 );
			}
			NSpri[nn]--;
			if (!NSpri[nn])
			{
				free( SPR );
				SpRefs[nn] = nullptr;
			};
			return;
		}
	}
}

void InitSprites()
{
	Sprites = nullptr;
	MaxSprt = 0;
	memset( NSpri, 0, VAL_SPRSIZE );//16384
	TREES.LoadSprites( "treelist" );
	STONES.LoadSprites( "stonlist" );
	HOLES.LoadSprites( "holelist" );
	COMPLEX.LoadSprites( "complex" );
}

int GetMinDist( int x, int y )
{
	int md = 10000;
	for (int i = 0; i < MaxSprt; i++)if (Sprites[i].Enabled)
	{
		int dst = abs( x - Sprites[i].x ) + abs( y - Sprites[i].y );

		if (dst < md)
			md = dst;
	}
	return md;
}

bool CheckDist( int x, int y, word r )
{
	if (CheckBar( x / 16, y / 16, ( r / 16 ) + 3, ( r / 16 ) + 3 ))
		return false;

	for (int i = 0; i < MaxSprt; i++)
	{
		OneSprite* SPR = &Sprites[i];
		if (SPR->Enabled)
		{
			int dst = abs( x - SPR->x ) + abs( y - SPR->y );

			if (dst < r + SPR->Radius)
				return false;
		}
	}
	return true;
}

bool SpriteSuccess;
int LastSpriteIndex;
void CHKS();
struct T_Curve
{
	int x0;
	int y0;
	int N;
	short* xi;
	short* yi;
};
bool CheckTPointInside( int x, int y, T_Curve* TC )
{
	int ncr = 0;
	for (int i = 0; i < TC->N; i++)
	{
		int x1 = TC->x0 + TC->xi[i];
		int y1 = TC->y0 + TC->yi[i];
		int i1 = ( i + 1 ) % TC->N;
		int x2 = TC->x0 + TC->xi[i1];
		int y2 = TC->y0 + TC->yi[i1];
		if (x1 > x2)
		{
			x1 += x2;
			x2 = x1 - x2;
			x1 -= x2;
			y1 += y2;
			y2 = y1 - y2;
			y1 -= y2;
		};
		if (x1 <= x&&x < x2)
		{
			int y0 = y1 + ( ( y2 - y1 )*( x - x1 ) ) / ( x2 - x1 );
			if (y0 > y)ncr++;
		};
	};
	return ncr & 1;
};
bool AddLockPts( int x0, int y0, int NPT, short* xi, short* yi, byte add )
{
	T_Curve TC;
	TC.x0 = x0;
	TC.y0 = y0;
	TC.N = NPT;
	TC.xi = xi;
	TC.yi = yi;
	int minx = 100000;
	int miny = 100000;
	int maxx = -100000;
	int maxy = -100000;
	for (int i = 0; i < NPT; i++)
	{
		int x = x0 + xi[i];
		int y = y0 + yi[i];
		if (x < minx)minx = x;
		if (y < miny)miny = y;
		if (x > maxx)maxx = x;
		if (y > maxy)maxy = y;
	};
	minx = ( minx >> 4 ) - 4;
	miny = ( miny >> 3 ) - 4;
	maxx = ( maxx >> 4 ) + 4;
	maxy = ( maxy >> 3 ) + 4;
	for (int dx = minx; dx <= maxx; dx++)
	{
		for (int dy = miny; dy <= maxy; dy++)
		{
			if (CheckTPointInside( ( dx << 4 ) + 8, ( dy << 3 ) + 4, &TC ))
			{
				if (add == 1)
				{
					BSetPt( dx, dy );
				}
				else
					if (add == 0)
					{
						BClrPt( dx, dy );
					}
					else
					{
						if (CheckPt( dx, dy ))return true;
					};
			};
		};
	};
	return false;
};
void addSprite( int x, int y, SprGroup* SG, word id )
{
	SpriteSuccess = false;
	LastSpriteIndex = -1;
	int i = LastAddSpr;
	if (LastAddSpr + 4 >= MaxSprt)
	{
		int psp = MaxSprt;
		MaxSprt += 32768;
		Sprites = (OneSprite*) realloc( Sprites, MaxSprt * sizeof OneSprite );
		memset( Sprites + psp, 0, ( MaxSprt - psp ) * sizeof OneSprite );
	};
	//int i=0;
	while (Sprites[i].Enabled&&i < MaxSprt)i++;
	if (i >= MaxSprt)
	{
		LastAddSpr = 0;
		return;
	};
	LastAddSpr = i;
	OneSprite* OSP = &Sprites[i];
	if (i > MAXSPR)MAXSPR = i;
	int cx = x >> 7;
	int cy = y >> 7;
	if (cx >= VAL_SPRNX || cy >= VAL_SPRNX)return;
	if (!CheckSpritesInArea( x << 4, y << 4, int( SG->Radius[id] ) << 4 ))return;
	ObjCharacter* OC = SG->ObjChar + id;
	if (OC->NLockPt)
	{
		if (AddLockPts( x - SG->Dx[id], ( y >> 1 ) - SG->Dy[id], OC->NLockPt, OC->LockX, OC->LockY, 2 ))return;
	};
	//if(GetMinDist(x,y)<48)return;
	int offs = cx + cy*VAL_SPRNX;
	//if(NSpri[offs]>=SprInCell)return;
	OSP->x = x;
	OSP->y = y;
	OSP->z = GetHeight( x, y );
	OSP->Enabled = true;
	//SPRMap[offs*SprInCell+NSpri[offs]]=i;
	//NSpri[offs]++;
	OSP->SG = SG;
	OSP->SGIndex = id;
	OSP->Index = i;
	RegisterSprite( i );
	OSP->WorkOver = 0;
	OSP->TimePassed = 0;
	OSP->Damage = 0;
	OSP->Radius = SG->Radius[id];
	OSP->OC = &SG->ObjChar[id];
	if (OSP->OC->TimeAmount)ObjTimer.Add( i, 0 );
	SpriteSuccess = true;
	LastSpriteIndex = i;

	if (OC->NLockPt)
	{
		AddLockPts( OSP->x - SG->Dx[id], ( OSP->y >> 1 ) - SG->Dy[id], OC->NLockPt, OC->LockX, OC->LockY, 1 );
	};
};
void addSpriteAnyway( int x, int y, SprGroup* SG, word id )
{
	SpriteSuccess = false;
	int i = LastAddSpr;
	if (LastAddSpr + 4 >= MaxSprt)
	{
		int psp = MaxSprt;
		MaxSprt += 32768;
		Sprites = (OneSprite*) realloc( Sprites, MaxSprt * sizeof OneSprite );
		memset( Sprites + psp, 0, ( MaxSprt - psp ) * sizeof OneSprite );
	};
	while (Sprites[i].Enabled&&i < MaxSprt)i++;
	if (i >= MaxSprt)
	{
		LastAddSpr = 0;
		return;
	};
	LastAddSpr = i;
	OneSprite* OSP = &Sprites[i];
	if (i > MAXSPR)MAXSPR = i;
	int cx = x >> 7;
	int cy = y >> 7;
	if (cx >= VAL_SPRNX || cy >= VAL_SPRNX)return;
	int offs = cx + cy*VAL_SPRNX;
	OSP->x = x;
	OSP->y = y;
	OSP->z = GetHeight( x, y );
	if (OSP->z > 1024)OSP->z -= 2048;
	OSP->Enabled = true;
	OSP->SG = SG;
	OSP->SGIndex = id;
	OSP->Index = i;
	RegisterSprite( i );
	OSP->WorkOver = 0;
	OSP->TimePassed = 0;
	OSP->Damage = 0;
	OSP->Radius = SG->Radius[id];
	OSP->OC = &SG->ObjChar[id];
	if (OSP->OC->TimeAmount)ObjTimer.Add( i, 0 );
	SpriteSuccess = true;
	ObjCharacter* OC = SG->ObjChar + id;
	if (OC->NLockPt)
	{
		AddLockPts( OSP->x - SG->Dx[id], ( OSP->y >> 1 ) - SG->Dy[id], OC->NLockPt, OC->LockX, OC->LockY, 1 );
	};
};

#define MSXH (240<<ADDSH)

int GetHeight( int x, int y )
{
	if (x < 0)
		x = 0;

	if (y < 32)
		y = 32;

	if (x > MSXH * 32)
		x = MSXH * 32;

	if (y > MSXH * 32)
		y = MSXH * 32;

	int nx = x / 32;

	if (nx & 1)
	{
		int dd = ( x & 31 );
		int dy = dd / 2;
		int oy = 15 - dy;
		int y1 = ( y + oy ) / 32;
		int dy1 = ( y + oy ) % 32;
		if (dy1 > 32 - dd)
		{
			int Vert2 = nx + y1*VertInLine + 1;
			int Vert3 = Vert2 + VertInLine;
			int Vert1 = Vert3 - 1;
			int H1 = THMap[Vert1];
			int H2 = THMap[Vert2];
			int H3 = THMap[Vert3];
			int x0 = ( nx * 32 );
			int y0 = ( y1 * 32 ) + 16;
			int HH = H1 + ( ( ( x - x0 ) * ( ( ( H2 + H3 ) / 2 ) - H1 ) ) / 32 ) + ( ( ( y - y0 )*( H3 - H2 ) ) / 32 );
			return HH;
		}
		else
		{
			int Vert2 = nx + y1*VertInLine;
			int Vert3 = Vert2 + VertInLine;
			int Vert1 = Vert2 + 1;
			int H1 = THMap[Vert1];
			int H2 = THMap[Vert2];
			int H3 = THMap[Vert3];
			int x0 = ( nx << 5 ) + 32;
			int y0 = ( y1 << 5 );
			int HH = H1 - ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 5 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 5 );
			return HH;
		}
	}
	else
	{
		int dd = ( x & 31 );
		int dy = dd >> 1;
		int oy = 15 - dy;
		int y1 = ( y + dy ) >> 5;
		int dy1 = ( y + dy ) & 31;
		if (dy1 < dd)
		{
			int Vert1 = nx + y1*VertInLine;
			int Vert2 = Vert1 + 1;
			int Vert3 = Vert2 + VertInLine;
			int H1 = THMap[Vert1];
			int H2 = THMap[Vert2];
			int H3 = THMap[Vert3];
			int x0 = ( nx << 5 );
			int y0 = ( y1 << 5 );
			int HH = H1 + ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 5 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 5 );
			return HH;
		}
		else
		{
			int Vert2 = nx + y1*VertInLine;
			int Vert3 = Vert2 + VertInLine;
			int Vert1 = Vert3 + 1;
			int H1 = THMap[Vert1];
			int H2 = THMap[Vert2];
			int H3 = THMap[Vert3];
			int x0 = ( nx << 5 ) + 32;
			int y0 = ( y1 << 5 ) + 16;
			int HH = H1 - ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 5 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 5 );
			return HH;
		}
	}
}

#define CLB (64<<4)
#define CLB1 ((64<<4)-1)
#define CLB2 (64<<3)
#define CLB21 ((64<<3)-1)

int GetPrecHeight( int x, int y )
{
	int nx = x >> ( 6 + 4 );
	if (nx & 1)
	{
		int dd = ( x&CLB1 );
		int dy = dd >> 1;
		int oy = CLB21 - dy;
		int y1 = ( y + oy ) >> ( 6 + 4 );
		int dy1 = ( y + oy )&CLB1;
		if (dy1 > CLB - dd)
		{
			int Vert2 = nx + y1*VertInLine + 1;
			int Vert3 = Vert2 + VertInLine;
			int Vert1 = Vert3 - 1;
			int H1 = THMap[Vert1] << 4;
			int H2 = THMap[Vert2] << 4;
			int H3 = THMap[Vert3] << 4;
			int x0 = ( nx << ( 6 + 4 ) );
			int y0 = ( y1 << ( 6 + 4 ) ) + CLB2;
			return H1 + ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 10 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 10 );
		}
		else
		{
			int Vert2 = nx + y1*VertInLine;
			int Vert3 = Vert2 + VertInLine;
			int Vert1 = Vert2 + 1;
			int H1 = THMap[Vert1] << 4;
			int H2 = THMap[Vert2] << 4;
			int H3 = THMap[Vert3] << 4;
			int x0 = ( nx << 10 ) + CLB;
			int y0 = ( y1 << 10 );
			return H1 - ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 10 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 10 );
		};
	}
	else
	{
		int dd = ( x&CLB1 );
		int dy = dd >> 1;
		int oy = CLB21 - dy;
		int y1 = ( y + dy ) >> 10;
		int dy1 = ( y + dy )&CLB1;
		if (dy1 < dd)
		{
			int Vert1 = nx + y1*VertInLine;
			int Vert2 = Vert1 + 1;
			int Vert3 = Vert2 + VertInLine;
			int H1 = THMap[Vert1] << 4;
			int H2 = THMap[Vert2] << 4;
			int H3 = THMap[Vert3] << 4;
			int x0 = ( nx << 10 );
			int y0 = ( y1 << 10 );
			return H1 + ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 10 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 10 );
		}
		else
		{
			int Vert2 = nx + y1*VertInLine;
			int Vert3 = Vert2 + VertInLine;
			int Vert1 = Vert3 + 1;
			int H1 = THMap[Vert1] << 4;
			int H2 = THMap[Vert2] << 4;
			int H3 = THMap[Vert3] << 4;
			int x0 = ( nx << 10 ) + CLB;
			int y0 = ( y1 << 10 ) + CLB2;
			return H1 - ( ( ( x - x0 )*( ( ( H2 + H3 ) >> 1 ) - H1 ) ) >> 6 ) + ( ( ( y - y0 )*( H3 - H2 ) ) >> 6 );
		};
	};
};
int mul3( int );
extern int tmtmt;
void OldPreShowSprites()
{
	int x0 = mapx << 5;
	int y0 = mul3( mapy ) << 3;
	int Lx = smaplx << 5;
	int Ly = mul3( smaply ) << 3;
	OneSprite* OS = Sprites;
	for (int i = 0; i <= MAXSPR; i++)
	{
		if (OS->Enabled)
		{
			int z = OS->z;//GetHeight(OS->x,OS->y);
			if (!Mode3D)z = 0;
			int ry = ( mul3( OS->y ) >> 2 ) - y0;
			int ry1 = ry - z;
			int rx = OS->x - x0;
			if (ry1 > -128 && ry1<Ly + 128 && rx>-128 && rx < Lx + 128)
			{
				ObjCharacter* OC = OS->OC;
				SprGroup* SG = OS->SG;
				if (OC->Stand)
				{
					int tm1 = div( tmtmt, OC->Delay ).quot;
					int fr = div( tm1, OC->Frames ).rem;
					int spr = fr*OC->Parts;
					int z0 = ry + OC->Z0 - z;
					int XX = rx - SG->Dx[OS->SGIndex];
					int YY = ry - SG->Dy[OS->SGIndex] - z;
					NewAnimation* NA = OC->Stand;
					for (int p = 0; p < OC->Parts; p++)
					{
						NewFrame* OF = &NA->Frames[spr + p];
						AddPoint( rx, ry, XX, YY, NULL, OF->FileID, OF->SpriteID, 0, 0 );
						z0 += OC->DZ;
					};
				}
				else
				{
					if (SG != &TREES)
						AddSuperLoPoint( rx - SG->Dx[OS->SGIndex], ry1 - SG->Dy[OS->SGIndex], NULL, SG->FileID, SG->Items[OS->SGIndex], 0, 0 );
					else
						AddPoint( rx, ry, rx - SG->Dx[OS->SGIndex], ry1 - SG->Dy[OS->SGIndex], NULL, SG->FileID, SG->Items[OS->SGIndex], 0, 0 );
				};
			};
		};
		OS++;
	};
};
void HideFlags()
{
	for (int i = 0; i < MaxSprt; i++)
	{
		OneSprite* OS = Sprites + i;
		if (OS->Enabled&&OS->SG == &TREES&&TREES.NSpr == OS->SGIndex + 1)
		{
			OS->Enabled = false;
			ObjTimer.Del( OS->Index, 0 );
			UnregisterSprite( OS->Index );
		};
	};
}

void PreShowSprites()
{
	int spx0 = ( mapx - 2 ) >> 2;
	int spx1 = ( mapx + smaplx + 2 ) >> 2;
	int spy0 = ( mapy - 2 ) >> 2;

	if (spx0 < 0)
	{
		spx0 = 0;
	}
	else
	{
		if (spx0 >= VAL_SPRNX)
		{
			spx0 = VAL_SPRNX - 1;
		}
	}

	if (spy0 < 0)
	{
		spy0 = 0;
	}
	else
	{
		if (spy0 >= VAL_SPRNX)
		{
			spy0 = VAL_SPRNX - 1;
		}
	}

	if (spx1 < 0)
	{
		spx1 = 0;
	}
	else
	{
		if (spx1 >= VAL_SPRNX)
		{
			spx1 = VAL_SPRNX - 1;
		}
	}

	int x0 = mapx << 5;
	int y0 = mul3( mapy ) << 3;
	int Lx = smaplx << 5;
	int Ly = mul3( smaply ) << 3;
	int x1 = x0 + Lx;
	int y1 = y0 + Ly;

	for (int spx = spx0; spx <= spx1; spx++)
	{
		int ofst = spx + ( spy0 << SprShf );
		int spy = spy0;
		int maxy;
		int xx = ( spx << 7 ) + 64;
		do
		{
			int N = NSpri[ofst];
			int* List = SpRefs[ofst];
			if (N && List)
			{
				for (int i = 0; i < N; i++)
				{
					OneSprite* OS = Sprites + List[i];
					if (OS->Enabled)
					{
						int z = OS->z;//GetHeight(OS->x,OS->y);
						if (!Mode3D)
						{
							z = 0;
						}
						int ry = ( mul3( OS->y ) >> 2 ) - y0;
						int ry1 = ry - z;
						int rx = OS->x - x0;
						if (ry1 > -128 && ry1<Ly + 128 && rx>-128 && rx < Lx + 128)
						{
							ObjCharacter* OC = OS->OC;
							SprGroup* SG = OS->SG;
							if (OC->Stand)
							{
								int tm1 = div( tmtmt, OC->Delay ).quot;
								int fr = div( tm1 + OS->x * 47 + OS->y * 83, OC->Frames ).rem;
								int spr = fr*OC->Parts;
								int z0 = ry + OC->Z0;
								int XX = rx - SG->Dx[OS->SGIndex];
								int YY = ry - SG->Dy[OS->SGIndex] - z;
								NewAnimation* NA = OC->Stand;
								for (int p = 0; p < OC->Parts; p++)
								{
									NewFrame* OF = &NA->Frames[spr + p];
									AddPoint( rx, z0, XX, YY, nullptr, OF->FileID, OF->SpriteID, 0, 0 );
									z0 += OC->DZ;
								}
							}
							else
							{
								if (SG != &TREES)
								{
									AddSuperLoPoint( rx - SG->Dx[OS->SGIndex], ry1 - SG->Dy[OS->SGIndex], 
										nullptr, SG->FileID, SG->Items[OS->SGIndex], 0, 0 );
								}
								else
								{
									AddPoint( rx, ry, rx - SG->Dx[OS->SGIndex], ry1 - SG->Dy[OS->SGIndex], 
										nullptr, SG->FileID, SG->Items[OS->SGIndex], 0, 0 );
								}
							}
						}
					}
				}
			}

			spy++;
			ofst += VAL_SPRNX;
			maxy = spy << 6;
			if (Mode3D)
			{
				maxy -= GetHeight( xx, maxy << 1 );
			}
		} while (spy < VAL_SPRNX&&maxy < ( y1 + 100 ));
	}
}

void addScrSprite( int x, int y, SprGroup* SG, word id )
{
	int yy = y;
	if (Mode3D)yy = ConvScrY( x, y );
	if (yy < 0)return;
	yy = div24( yy << 5 );
	addSprite( x, yy, SG, id );
};
int GetTreeItem();
extern int InteriorType;
void addTree( int x, int y )
{
	switch (InteriorType)
	{
	case 0:addScrSprite( x, y, &TREES, GetTreeItem() );
		break;
	case 1:addScrSprite( x, y, &STONES, GetTreeItem() );
		break;
	case 2:addScrSprite( x, y, &HOLES, GetTreeItem() );
		break;
	};
};
void addLogTree( int x, int y )
{
	switch (InteriorType)
	{
	case 0:addSprite( x, y, &TREES, GetTreeItem() );
		break;
	case 1:addSprite( x, y, &STONES, GetTreeItem() );
		break;
	case 2:addSprite( x, y, &COMPLEX, GetTreeItem() );
		break;
	case 3:addSprite( x, y, &COMPLEX, GetTreeItem() );
		break;

	};
};
void addTrees( int x, int y1, int r )
{
	int y = Prop43( y1 );
	if (r == 16)
	{
		addLogTree( x, y );
	}
	else
	{
		int n = ( r*r ) >> 8;
		int r2 = r*r;
		for (int i = 0; i < n; i++)
		{
			int dx = ( ( rando() - 16384 )*r ) >> 14;
			int dy = ( ( rando() - 16384 )*r ) >> 14;
			if (dx*dx + dy*dy <= r2)addLogTree( x + dx, y + dy );
		};
	};
};
void delTrees( int x, int y1, int r )
{
	int y = Prop43( y1 );
	int r2 = r*r;
	for (int i = 0; i < MaxSprt; i++)
	{
		OneSprite* OS = &Sprites[i];
		if (OS->Enabled)
		{
			int ox = OS->x - x;
			int oy = OS->y - y;
			if (ox*ox + oy*oy <= r2)
			{
				OS->Enabled = false;
				UnregisterSprite( OS->Index );
				ObjTimer.Del( OS->Index, 0 );
			};
		};
	};
};
//searces place for peasant near the resource
//px,py-coordinates of peasant(unit: 16*unit=1pix)
//tx,ty-coordintes of resource(unit : pixel)
//r-best work distance (unit : pixel)
bool FindPlaceForPeasant( int* px, int* py,
	int tx, int ty, int r )
{
	int pix = ( ( *px ) ) >> 8;
	int piy = ( ( *py ) ) >> 8;
	int treex = ( tx >> 4 ) - 1;
	int treey = ( ty >> 4 ) - 1;
	int rad = r >> 4;
	int radx = ( r - ( r >> 2 ) ) >> 4;
	int minr = 10000;
	int bx = 0;
	int by = 0;
	int rr;
	int x1 = treex + rad;
	int y1 = treey;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		minr = rr;
		bx = x1 + 1;
		by = y1 + 1;
	};
	y1 = treey + rad;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	y1 = treey - rad;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	x1 = treex;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	y1 = treey + rad;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	x1 = treex - rad;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	y1 = treey;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	y1 = treey - rad;
	if (!CheckBar( x1, y1, 3, 3 ))
	{
		rr = Norma( x1 - pix, y1 - piy );
		if (rr < minr)
		{
			minr = rr;
			bx = x1 + 1;
			by = y1 + 1;
		};
	};
	if (minr < 1000)
	{
		*px = bx << 4;
		*py = by << 4;
		return true;
	}
	else return false;
};
//-------------SEARCH FOR RESOURCES ROUTINES----------
//searcing for the resource in the cell.
//New Resource - resource,that was not use before;
//Old resource - already partially used resource
class CellSearch
{
public:
	int xd, yd;//unit : pixels
	int BestNew;
	int BestOld;
	int NewDist;//unit : pix
	int OldDist;//unit : pix
	byte ResType;
	void InitCSR();
};
void CellSearch::InitCSR()
{
	NewDist = 10000;
	OldDist = 10000;
	BestNew = INITBEST;
	BestOld = INITBEST;
};
void FindResInCell( CellSearch* CSR, int cell, int Top )
{
	if (cell < 0 || cell >= VAL_SPRSIZE)return;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	int TOP1 = Top*NAreas;
	if (!( CEL&&NCEL ))return;
	CSR->NewDist = 10000;
	CSR->OldDist = 10000;
	int dist;
	for (int i = 0; i < NCEL; i++)
	{
		OneSprite* OS = &Sprites[CEL[i]];
		//assert(OS->Enabled);
		ObjCharacter* OC = OS->OC;
		if (OC->ResType == CSR->ResType && !OS->Surrounded)
		{
			int tx = OS->x >> 6;
			int ty = OS->y >> 6;
			int STop = SafeTopRef( tx, ty );
			if (STop < 0xFFFE && ( Top == STop || LinksDist[TOP1 + STop] < 30 ))
			{
				if (OC->ResPerWork)
				{
					//old resource
					dist = Norma( OS->x - CSR->xd, OS->y - CSR->yd );
					if (dist < CSR->OldDist)
					{
						CSR->OldDist = dist;
						CSR->BestOld = CEL[i];
					};
				}
				else
				{
					//new resource
					dist = Norma( OS->x - CSR->xd, OS->y - CSR->yd );
					if (dist < CSR->NewDist)
					{
						CSR->NewDist = dist;
						CSR->BestNew = CEL[i];
					};
				};
			};
		};
	};
};
bool CheckSpritesInCell( int cell, int x, int y, int r )
{
	if (cell < 0 || cell >= VAL_SPRSIZE)return true;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	if (!( CEL&&NCEL ))return true;
	for (int i = 0; i < NCEL; i++)
	{
		OneSprite* OS = &Sprites[CEL[i]];
		//assert(OS->Enabled);
		ObjCharacter* OC = OS->OC;
		if (OC->ResType != 0xFE)
		{
			int dr = OS->Radius;
			if (OC->IntResType > 8)dr = 0;
			if (Norma( OS->x - x, OS->y - y ) < ( r + dr ))return false;
		};
	};
	return true;
};
//x: 16units=1 pix
//y: 16units=1 pix
bool CheckSpritesInArea( int x, int y, int r )
{
	x >>= 4;
	y >>= 4;
	r >>= 4;
	int nr = ( r >> 7 ) + 3;
	int nr1 = nr + nr + 1;
	int cell = ( x >> 7 ) - nr + ( ( ( y >> 7 ) - nr ) << SprShf );
	for (int ix = 0; ix < nr1; ix++)
		for (int iy = 0; iy < nr1; iy++)
			if (!CheckSpritesInCell( cell + ix + ( iy << SprShf ), x, y, r ))return false;
	return true;
};
bool CheckSpritesInCellNew( int cell, int x, int y, int r )
{
	if (cell < 0 || cell >= VAL_SPRSIZE)return true;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	if (!( CEL&&NCEL ))return true;
	for (int i = 0; i < NCEL; i++)
	{
		OneSprite* OS = &Sprites[CEL[i]];
		//assert(OS->Enabled);
		ObjCharacter* OC = OS->OC;
		if (OC->ResType != 0xFE && OS->SG != &TREES)
		{
			int dr = OS->Radius;
			if (OC->IntResType > 8)dr = 0;
			if (Norma( OS->x - x, OS->y - y ) < ( r + dr ))return false;
		};
	};
	return true;
};
void EraseSprite( int Index )
{
	OneSprite* OS = &Sprites[Index];
	if (OS&&OS->Enabled)
	{
		int id = OS->SGIndex;
		ObjCharacter* OC = OS->SG->ObjChar + id;
		if (OC->NLockPt)
		{
			AddLockPts( OS->x - OS->SG->Dx[id], ( OS->y >> 1 ) - OS->SG->Dy[id], OC->NLockPt, OC->LockX, OC->LockY, 0 );
		};
		OS->Enabled = 0;
		ObjTimer.Del( Index, 0 );
		UnregisterSprite( Index );
	};
};
void EraseTreesInPoint( int x, int y )
{
	int cell = ( x >> 3 ) + ( ( y >> 3 ) << SprShf );
	if (cell < 0 || cell >= VAL_SPRSIZE)return;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	if (!( CEL&&NCEL ))return;
	for (int i = 0; i < NCEL; i++)
	{
		OneSprite* OS = &Sprites[CEL[i]];
		//assert(OS->Enabled);
		ObjCharacter* OC = OS->OC;
		if (OC->ResType == 0xFE || OS->SG == &TREES)
		{
			if (x == ( OS->x >> 4 ) && y == ( OS->y >> 4 ))
			{
				OS->Enabled = false;
				ObjTimer.Del( OS->Index, 0 );
				UnregisterSprite( OS->Index );
			};
		};
	};
};
bool CheckSpritesInAreaNew( int x, int y, int r, bool Erase )
{
	x >>= 4;
	y >>= 4;
	r >>= 4;
	int nr = ( r >> 7 ) + 3;
	int nr1 = nr + nr + 1;
	int cell = ( x >> 7 ) - nr + ( ( ( y >> 7 ) - nr ) << SprShf );
	if (Erase)
	{
		for (int ix = 0; ix < nr1; ix++)
			for (int iy = 0; iy < nr1; iy++)
				if (!CheckSpritesInCellNew( cell + ix + ( iy << SprShf ), x, y, r ))return false;
	}
	else
	{
		for (int ix = 0; ix < nr1; ix++)
			for (int iy = 0; iy < nr1; iy++)
				if (!CheckSpritesInCell( cell + ix + ( iy << SprShf ), x, y, r ))return false;
	};
	return true;
}

int CheckSpritesInCellForMine( int cell, int* xi, int* yi, int r, byte rmask )
{
	if (cell < 0 || cell >= VAL_SPRSIZE)return true;
	int x = *xi;
	int y = *yi;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	if (!( CEL&&NCEL ))return -2;
	byte rm;
	int ID = -1;
	for (int i = 0; i < NCEL; i++)
	{
		OneSprite* OS = &Sprites[CEL[i]];
		ObjCharacter* OC = OS->OC;
		if (OC->IntResType < 8)
		{
			rm = 1 << OC->IntResType;
			if (rm&rmask && ( Norma( OS->x - x, OS->y - y ) >> 2 ) < OC->IntWorkRadius)
			{
				ID = CEL[i];
				*xi = OS->x;
				*yi = OS->y;
			}
		}
	}

	if (ID == -1)
		return -2;

	return ID;
};

//x: 16 units = 1 pix
//y: 16 units = 1 pix
int CheckMinePosition( NewMonster* NM, int* xi, int* yi, int r )
{
	int x = ( *xi ) >> 4;
	int y = ( *yi ) >> 4;
	r >>= 4;
	byte RMask = NM->ProdType;
	int nr = ( r >> 7 ) + 1;
	int nr1 = nr + nr + 1;
	int cell = ( ( x - nr ) >> 7 ) + ( ( ( y - nr ) >> 7 ) << SprShf );
	int ID = -1;
	int ID1;
	for (int ix = 0; ix < nr1; ix++)
		for (int iy = 0; iy < nr1; iy++)
		{
			ID1 = CheckSpritesInCellForMine( cell + ix + ( iy << SprShf ), &x, &y, r, RMask );
			if (ID1 == -1)return -1;
			if (ID1 >= 0)
			{
				*xi = x << 4;
				*yi = y << 4;
				return ID1;
			};
		};
	return -1;
}

int GetCell( int xs, int ys )
{
	return ( xs >> 7 ) + ( ( ys >> 7 ) << SprShf );
}

//searches for resource object. 
//xd,yd-coordinates of the researching point(pix)
//returns required coordinates of peasant
//xp,yp-coordinates of peasant(min units)
//RType-resource type
//-----Notes-------
int FindResourceObject( int* xd, int* yd, int xp, int yp, byte RType )
{
	CellSearch CSR1;
	CSR1.xd = *xd;
	CSR1.yd = *yd;
	CSR1.ResType = RType;
	CSR1.InitCSR();
	int tx = ( *xd ) >> 6;
	int ty = ( *yd ) >> 6;
	if (tx <= 0 || ty <= 0)return INITBEST;
	int DstTop = SafeTopRef( tx, ty );
	if (DstTop >= 0xFFFE)return INITBEST;
	int pix, piy;
	int STCL = GetCell( CSR1.xd, CSR1.yd );
	bool ResFound = false;
	bool EndCyc = false;
	int rr = 0;
	int subcsr = 0;
	do
	{
		do
		{
			if (!rr)FindResInCell( &CSR1, STCL, DstTop );
			else
			{
				int nn = rr + rr + 1;
				for (int i = 0; i < nn; i++)
					FindResInCell( &CSR1, STCL - subcsr - rr + i, DstTop );
				for (int i = 0; i < nn; i++)
					FindResInCell( &CSR1, STCL + subcsr - rr + i, DstTop );
				nn -= 2;
				int subb = VAL_SPRNX - subcsr - rr;
				for (int i = 0; i < nn; i++)
				{
					FindResInCell( &CSR1, STCL + subb, DstTop );
					subb += VAL_SPRNX;
				};
				subb = VAL_SPRNX - subcsr + rr;
				for (int i = 0; i < nn; i++)
				{
					FindResInCell( &CSR1, STCL + subb, DstTop );
					subb += VAL_SPRNX;
				};
			};
			if (CSR1.BestOld != INITBEST&&CSR1.BestNew != INITBEST)
			{
				if (CSR1.NewDist < CSR1.OldDist + CSR1.OldDist)
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestNew];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC[OSP->SGIndex].WorkRadius ))
					{
						*xd = pix;
						*yd = piy;

						return CSR1.BestNew;
					}
					else
					{
						OSP->Surrounded = true;
						OSP = &Sprites[CSR1.BestOld];
						if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC[OSP->SGIndex].WorkRadius ))
						{
							*xd = pix;
							*yd = piy;

							return CSR1.BestOld;
						}
						else
						{
							OSP->Surrounded = true;
							EndCyc = false;
							CSR1.InitCSR();
						};
					};
				}
				else
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestOld];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC[OSP->SGIndex].WorkRadius ))
					{
						*xd = pix;
						*yd = piy;

						return CSR1.BestOld;
					}
					else
					{
						OSP->Surrounded = true;
						OSP = &Sprites[CSR1.BestNew];
						if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC[OSP->SGIndex].WorkRadius ))
						{
							*xd = pix;
							*yd = piy;

							return CSR1.BestNew;
						}
						else
						{
							OSP->Surrounded = true;
							EndCyc = false;
							CSR1.InitCSR();
						};
					};
				};
			}
			else
			{
				EndCyc = true;
				if (CSR1.BestOld != INITBEST)
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestOld];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
					{
						*xd = pix;
						*yd = piy;

						return CSR1.BestOld;
					}
					else
					{
						OSP->Surrounded = true;
						EndCyc = false;
						CSR1.InitCSR();
					};
				};
				if (CSR1.BestNew != INITBEST)
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestNew];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
					{
						*xd = pix;
						*yd = piy;

						return CSR1.BestNew;
					}
					else
					{
						OSP->Surrounded = true;
						EndCyc = false;
						CSR1.InitCSR();
					};
				};
			};
		} while (!EndCyc);
		EndCyc = false;
		rr++;
		subcsr += VAL_SPRNX;
		CSR1.InitCSR();
	} while (rr < 12);
	rando();
	return INITBEST;
};
void FindLimResInCell( CellSearch* CSR, int cell, word Lim, int Top )
{
	if (cell < 0 || cell >= VAL_SPRSIZE)return;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	if (!( CEL&&NCEL ))return;
	CSR->NewDist = 10000;
	CSR->OldDist = 10000;
	int TOP1 = Top*NAreas;
	int dist;
	for (int i = 0; i < NCEL; i++)
	{
		int RS = CEL[i];
		if (RS != Lim)
		{
			OneSprite* OS = &Sprites[CEL[i]];
			//assert(OS->Enabled);
			ObjCharacter* OC = OS->OC;
			if (OC->ResType == CSR->ResType && !OS->Surrounded)
			{
				int tx = OS->x >> 6;
				int ty = OS->y >> 6;
				int FTop = SafeTopRef( tx, ty );
				if (FTop < 0xFFFE && ( LinksDist[FTop + TOP1] < 20 || FTop == Top ))
				{
					if (OC->ResPerWork)
					{
						//old resource
						dist = Norma( OS->x - CSR->xd, OS->y - CSR->yd );
						if (dist < CSR->OldDist)
						{
							CSR->OldDist = dist;
							CSR->BestOld = CEL[i];
						};
					}
					else
					{
						//new resource
						dist = Norma( OS->x - CSR->xd, OS->y - CSR->yd );
						if (dist < CSR->NewDist)
						{
							CSR->NewDist = dist;
							CSR->BestNew = CEL[i];
						};
					};
				};
			};
		};
	};
};

int FindLimResourceObject( int* xd, int* yd, int xp, int yp, byte RType, word Lim )
{




	int SrcTop = GetTopology( xp >> 4, yp >> 4 );
	int DstTop = GetTopology( *xd, *yd );
	if (DstTop >= 0xFFFE)return INITBEST;
	CellSearch CSR1;
	CSR1.xd = *xd;
	CSR1.yd = *yd;
	CSR1.ResType = RType;
	CSR1.InitCSR();
	int pix, piy;
	int STCL = ( CSR1.xd >> 7 ) + ( ( CSR1.yd >> 7 ) << SprShf );
	bool ResFound = false;
	bool EndCyc = false;
	int rr = 0;
	int subcsr = 0;
	do
	{
		do
		{
			if (!rr)FindLimResInCell( &CSR1, STCL, Lim, DstTop );
			else
			{
				int nn = rr + rr + 1;
				for (int i = 0; i < nn; i++)
					FindLimResInCell( &CSR1, STCL - subcsr - rr + i, Lim, DstTop );
				for (int i = 0; i < nn; i++)
					FindLimResInCell( &CSR1, STCL + subcsr - rr + i, Lim, DstTop );
				nn -= 2;
				int subb = VAL_SPRNX - subcsr - rr;
				for (int i = 0; i < nn; i++)
				{
					FindLimResInCell( &CSR1, STCL + subb, Lim, DstTop );
					subb += VAL_SPRNX;
				};
				subb = VAL_SPRNX - subcsr + rr;
				for (int i = 0; i < nn; i++)
				{
					FindLimResInCell( &CSR1, STCL + subb, Lim, DstTop );
					subb += VAL_SPRNX;
				};
			};
			if (CSR1.BestOld != INITBEST&&CSR1.BestNew != INITBEST)
			{
				if (CSR1.NewDist < CSR1.OldDist + CSR1.OldDist)
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestNew];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
					{
						*xd = pix;
						*yd = piy;
						return CSR1.BestNew;
					}
					else
					{
						OSP->Surrounded = true;
						OSP = &Sprites[CSR1.BestOld];
						if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
						{
							*xd = pix;
							*yd = piy;
							return CSR1.BestOld;
						}
						else
						{
							OSP->Surrounded = true;
							EndCyc = false;
							CSR1.InitCSR();
						};
					};
				}
				else
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestOld];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
					{
						*xd = pix;
						*yd = piy;
						return CSR1.BestOld;
					}
					else
					{
						OSP->Surrounded = true;
						OSP = &Sprites[CSR1.BestNew];
						if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
						{
							*xd = pix;
							*yd = piy;
							return CSR1.BestNew;
						}
						else
						{
							OSP->Surrounded = true;
							EndCyc = false;
							CSR1.InitCSR();
						};
					};
				};
			}
			else
			{
				EndCyc = true;
				if (CSR1.BestOld != INITBEST)
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestOld];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
					{
						*xd = pix;
						*yd = piy;
						return CSR1.BestOld;
					}
					else
					{
						OSP->Surrounded = true;
						EndCyc = false;
						CSR1.InitCSR();
					};
				};
				if (CSR1.BestNew != INITBEST)
				{
					pix = xp;
					piy = yp;
					OneSprite* OSP = &Sprites[CSR1.BestNew];
					if (FindPlaceForPeasant( &pix, &piy, OSP->x, OSP->y, OSP->OC->WorkRadius ))
					{
						*xd = pix;
						*yd = piy;
						return CSR1.BestNew;
					}
					else
					{
						OSP->Surrounded = true;
						EndCyc = false;
						CSR1.InitCSR();
					};
				};
			};
		} while (!EndCyc);
		EndCyc = false;
		rr++;
		subcsr += VAL_SPRNX;
		CSR1.InitCSR();
	} while (rr < 4);
	return INITBEST;
};
int OneSprite::PerformWork( word Effect )
{
	ObjCharacter* OCR = OC;
	WorkOver++;
	int Wamount = ( int( OCR->WorkAmount )*Effect ) / 100;
	if (Wamount <= WorkOver)
	{
		//transformation to the next stage
		SGIndex = OCR->WNextObj;
		if (SGIndex != 0xFFFF)
		{
			OC = &SG->ObjChar[SGIndex];
			WorkOver = 0;
			TimePassed = 0;
		}
		else
		{
			Enabled = false;
			ObjTimer.Del( Index, 0 );
			UnregisterSprite( Index );
		};
	};
	return OCR->ResPerWork;
};
int OneSprite::PerformIntWork( int work )
{
	ObjCharacter* OCR = OC;
	WorkOver += work;
	if (OCR->WorkAmount <= WorkOver)
	{
		//transformation to the next stage
		SGIndex = OCR->WNextObj;
		OC = &SG->ObjChar[SGIndex];
		WorkOver = 0;
	};
	return work*OCR->IntResPerWork;
};
void OneSprite::PerformDamage( int Dam )
{
	ObjCharacter* OCR = OC;
	if (Dam > 1)Dam = Dam >> 1;
	Damage += Dam;
	if (OCR->DamageAmount <= Damage)
	{
		//transformation to the next stage
		SGIndex = OCR->DNextObj;
		OC = &SG->ObjChar[SGIndex];
		Damage = 0;
	};
};
//word FindResBase
void TakeResLink( OneObject* OBJ );

int OneObject::TakeResource( int px, int py, byte ResID, int Prio, byte OrdType )
{
	if (UnlimitedMotion)
		return false;

	if (CheckOrderAbility())
		return false;

	if (Prio < PrioryLevel)
		return false;

	if (UnlimitedMotion)
		return false;

	//checking ability
	NewMonster* NM = newMons;
	switch (ResID)
	{
	case 0://wood
		if (!NM->WorkTree.Enabled)return INITBEST;
		break;
	case 1://gold
	case 2://stone
		if (!NM->WorkStone.Enabled)return INITBEST;
		break;
	case 3:
		if (!NM->WorkField.Enabled)return INITBEST;
		break;
	default:
		return  INITBEST;
	}
	if (RType != ResID)
	{
		RAmount = 0;
		RType = 0xFF;
	}
	int xx = px;
	int yy = py;
	int DObj = FindResourceObject( &xx, &yy, RealX, RealY, ResID );

	if (DObj == INITBEST)
		return INITBEST;

	RStage = 0;
	Order1* Or1 = CreateOrder( OrdType );

	if (!int( Or1 ))
		return INITBEST;

	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 238;//Take resource
	Or1->OrderTime = 0;
	Or1->DoLink = &TakeResLink;
	Or1->info.TakeRes.x = xx;
	Or1->info.TakeRes.y = yy;
	Or1->info.TakeRes.ResID = ResID;
	Or1->info.TakeRes.SprObj = DObj;
	//PrioryLevel=Prio&127;
	NothingTime = 0;
	NearBase = 0xFFFF;
	PrioryLevel = LocalOrder->PrioryLevel;
	MakeOrderSound( this, 2 );

	return DObj;
}

extern bool FastMode;
int GOINDEX = 0;

//Process gathering of resources
void TakeResLink( OneObject* OBJ )
{
	if (OBJ->NewAnm->NFrames - FrmDec > OBJ->NewCurSprite)
	{
		return;
	}

	rando();
	GOINDEX = OBJ->Index;
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	OBJ->UnBlockUnit();
	NewMonster* NM = OBJ->newMons;
	int DObj = OBJ->LocalOrder->info.TakeRes.SprObj;
	int xx = OBJ->LocalOrder->info.TakeRes.x;
	int yy = OBJ->LocalOrder->info.TakeRes.y;
	byte ResType = OBJ->LocalOrder->info.TakeRes.ResID;

	if (ResType != 0xFF && OBJ->RAmount >= NM->MaxResPortion[ResType])
	{
FindNow:
		if (OBJ->NearBase == 0xFFFF)
		{
			OBJ->RType = ResType;
			OBJ->NearBase = OBJ->FindNearestBase();
		}
		if (OBJ->NearBase == 0xFFFF)
		{
			Order1* OR1 = OBJ->LocalOrder;
			OBJ->LocalOrder = OR1->NextOrder;
			OBJ->FreeOrdBlock( OR1 );
			OBJ->UnBlockUnit();
			return;
		}
		OneObject* OB = Group[OBJ->NearBase];
		if (!OB)
		{
			OBJ->NearBase = 0xFFFF;
			goto FindNow;
		}
		if (!OB->AbRes)
		{
			OBJ->NearBase = 0xFFFF;
			if (OBJ->NearBase)
			{
				OBJ->NearBase = OBJ->FindNearestBase();
			}
			OBJ->NearBase = 0xFFFF;
			if (OBJ->NearBase)
			{
				Order1* OR1 = OBJ->LocalOrder;
				OBJ->LocalOrder = OR1->NextOrder;
				OBJ->FreeOrdBlock( OR1 );
				OBJ->UnBlockUnit();
				return;
			}
		}
		NewMonster* NM1 = OB->newMons;
		short dsx = 0;
		short dsy = 0;
		if (NM1->NConcPt)
		{
			int PicSX = NM1->PicDx << 4;
			int PicSY = Prop43( NM1->PicDy ) << 4;
			int x0 = ( ( OB->RealX + PicSX ) >> 8 );
			int y0 = ( ( OB->RealY + PicSY ) >> 8 );
			int np = NM1->NConcPt;
			int Nr = 1000000;
			for (int j = 0; j < np; j++)
			{
				int dsx0 = x0 + NM1->ConcPtX[j];
				int dsy0 = y0 + NM1->ConcPtY[j];
				int NRM = Norma( dsx0 - OBJ->x, dsy0 - OBJ->y );
				if (NRM < Nr)
				{
					dsx = dsx0;
					dsy = dsy0;
					Nr = NRM;
				}
			}
		}
		else
		{
			dsx = OB->RealX >> 8;
			dsy = OB->RealY >> 8;
		}

		if (Norma( OBJ->x - dsx, OBJ->y - dsy ) < 6)
		{
			AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
			int ramn = div( int( OBJ->RAmount )*ADC->ResEff, 100 ).quot;
			if (ResType == TreeID)
			{
				ramn = ( ramn*int( NATIONS[OBJ->NNUM].WoodEff ) ) / 100;
				OBJ->Nat->CITY->WoodAmount += ramn;
			}
			else
			{
				if (ResType == StoneID)
				{
					ramn = ( ramn * int( NATIONS[OBJ->NNUM].StoneEff ) ) / 100;
					OBJ->Nat->CITY->StoneAmount += ramn;
				}
				else
				{

					if (ResType == FoodID)
					{
						int eff = ( NATIONS[OBJ->NNUM].FoodEff ) % 100;
						ramn = ( ramn*eff ) / 10;
						OBJ->Nat->CITY->FoodAmount += ramn;
					}
				}
			}
			AddXRESRC( OBJ->NNUM, ResType, ramn );
			NATIONS[OBJ->NNUM].AddResource( ResType, ramn );
			NATIONS[OBJ->NNUM].ResTotal[ResType] += ramn;
			OBJ->RAmount = 0;
			OBJ->RType = 0xFF;
			OBJ->NewState = 0;
		}
		else
		{
			if (OBJ->NothingTime > 8)
			{
				if (OBJ->CheckBlocking())
				{
					OBJ->DeleteBlocking();
					return;
				}
				else
				{
					OBJ->CreatePath( dsx, dsy );
				}
			}
			else
			{
				OBJ->CreatePath( dsx, dsy );
			}
			return;
		}
	}
	if (DObj == INITBEST)
	{
		//Resource still not found!
		DObj = FindResourceObject( &xx, &yy, OBJ->RealX, OBJ->RealY, ResType );

		if (DObj == INITBEST)
		{
			//Resource does not exists
			Order1* OR1 = OBJ->LocalOrder;
			OBJ->LocalOrder = OR1->NextOrder;
			OBJ->FreeOrdBlock( OR1 );
			OBJ->UnBlockUnit();
			return;
		}
		OBJ->LocalOrder->info.TakeRes.SprObj = DObj;
		OBJ->LocalOrder->info.TakeRes.x = xx;
		OBJ->LocalOrder->info.TakeRes.y = yy;
	}

	OneSprite* OS = &Sprites[DObj];
	if (!OS->Enabled)
	{
		OBJ->LocalOrder->info.TakeRes.SprObj = INITBEST;
		return;
	}

	ObjCharacter* OC = OS->OC;
	if (OBJ->NothingTime > 10)
	{
		OBJ->UnBlockUnit();
		DObj = FindLimResourceObject( &xx, &yy, OBJ->RealX, OBJ->RealY, ResType, DObj );

		if (DObj != INITBEST)
		{
			OBJ->LocalOrder->info.TakeRes.SprObj = DObj;
			OBJ->LocalOrder->info.TakeRes.x = xx;
			OBJ->LocalOrder->info.TakeRes.y = yy;
			OS = &Sprites[DObj];
			OBJ->NothingTime = 0;
			OC = OS->OC;
		}
	}

	if (!( OC->ResType == ResType&&OS->Enabled ))
	{
		OBJ->UnBlockUnit();
		DObj = FindResourceObject( &xx, &yy, OBJ->RealX, OBJ->RealY, ResType );
		if (DObj == INITBEST)
		{
			//Resource does not exists
			Order1* OR1 = OBJ->LocalOrder;
			OBJ->LocalOrder = OR1->NextOrder;
			OBJ->FreeOrdBlock( OR1 );
			return;
		}
		OBJ->LocalOrder->info.TakeRes.SprObj = DObj;
		OBJ->LocalOrder->info.TakeRes.x = xx;
		OBJ->LocalOrder->info.TakeRes.y = yy;
		OS = &Sprites[DObj];
		OC = OS->OC;
	}

	OC = OS->OC;
	int odx = ( OBJ->RealX >> 4 ) - OS->x;
	int ody = ( OBJ->RealY >> 4 ) - OS->y;
	int Rdist = Norma( odx, ody );

	if (Rdist < OC->WorkRadius)
	{
		OBJ->DeletePath();
		OBJ->DestX = -1;
		char dirc = char( GetDir( -odx, -ody ) );
		char ddir = dirc - OBJ->RealDir;
		if (ddir < 16)
		{
			OBJ->RealDir = dirc;
			OBJ->GraphDir = dirc;
			OBJ->NewCurSprite = 0;

			switch (ResType)
			{
			case 0://wood
				OBJ->NewAnm = &NM->WorkTree;
				break;
			case 1://stone
			case 2://gold
				OBJ->NewAnm = &NM->WorkStone;
				break;
			case 3:
				OBJ->NewAnm = &NM->WorkField;
			}

			rando();
			OBJ->DeletePath();
			OBJ->InMotion = false;
			OBJ->BackMotion = false;
			OBJ->RType = ResType;
			if (ResType == FoodID)
			{
				OBJ->RAmount += OS->PerformWork( int( NATIONS[OBJ->NNUM].FoodEff / 100 ) * 10 );
			}
			else
			{
				OBJ->RAmount += OS->PerformWork( 100 );
			}
			OBJ->BlockUnit();
			OBJ->NothingTime = 0;
			OBJ->NearBase = 0xFFFF;
		}
		else
		{
			if (ddir > 0)
			{
				OBJ->RealDir += 16;
			}
			else
			{
				OBJ->RealDir -= 16;
			}
			OBJ->GraphDir = OBJ->RealDir;
		}
	}
	else
	{
		if (Rdist < 128)
		{
			//checking for place to stand
			if (CheckBar( ( xx >> 4 ) - 1, ( yy >> 4 ) - 1, 3, 3 ))
			{
				//Place for peasant is oocupied by other!
				//Let us find new place
				if (FindPlaceForPeasant( &xx, &yy, OS->x, OS->y, OC->WorkRadius ))
				{
					OBJ->LocalOrder->info.TakeRes.x = xx;
					OBJ->LocalOrder->info.TakeRes.y = yy;
					OBJ->CreatePath( OS->x >> 4, OS->y >> 4 );
				}
				else
				{
					OBJ->UnBlockUnit();
					DObj = FindResourceObject( &xx, &yy, OBJ->RealX, OBJ->RealY, ResType );
					if (DObj == INITBEST)
					{
						//Resource does not exists
						Order1* OR1 = OBJ->LocalOrder;
						OBJ->LocalOrder = OR1->NextOrder;
						OBJ->FreeOrdBlock( OR1 );
						return;
					}
					OBJ->LocalOrder->info.TakeRes.SprObj = DObj;
					OBJ->LocalOrder->info.TakeRes.x = xx;
					OBJ->LocalOrder->info.TakeRes.y = yy;
					TryToStand( OBJ, false );
				}
				return;
			}
			else
			{
				if (OBJ->NothingTime > 8)
				{
					if (OBJ->CheckBlocking())
					{
						OBJ->DeleteBlocking();
					}
					else
					{
						OBJ->CreatePath( OS->x >> 4, OS->y >> 4 );
					}
				}
				else
				{
					OBJ->CreatePath( OS->x >> 4, OS->y >> 4 );
				}
			}
		}
		else
		{
			if (OBJ->NothingTime > 8)
			{
				if (OBJ->CheckBlocking())
				{
					OBJ->DeleteBlocking();
				}
				else
				{
					OBJ->CreatePath( OS->x >> 4, OS->y >> 4 );
				}
			}
			else
			{
				OBJ->CreatePath( OS->x >> 4, OS->y >> 4 );
			}
		}
	}
}

byte FindAnyResInCell( int x, int y, int cell, int* Dist, byte Res )
{
	if (cell < 0 || cell >= VAL_SPRSIZE)return Res;
	int* CEL = SpRefs[cell];
	int   NCEL = NSpri[cell];
	if (!( CEL&&NCEL ))return Res;
	int dst;
	int dist = *Dist;
	byte rsr = Res;
	for (int i = 0; i < NCEL; i++)
	{
		OneSprite* OS = &Sprites[CEL[i]];
		//assert(OS->Enabled);
		dst = Norma( x - OS->x, y - OS->y );
		if (dst < dist)
		{
			ObjCharacter* OC = OS->OC;
			if (OC->ResType < 0xFE)
			{
				dist = dst;
				rsr = OC->ResType;
			};
		};
	};
	*Dist = dist;
	return rsr;
};
byte DetermineResource( int x, int y )
{
	int cell = ( x >> 7 ) + ( ( y >> 7 ) << SprShf );
	int DistR = 10000;
	byte res = 0xFF;
	res = FindAnyResInCell( x, y, cell, &DistR, res );
	res = FindAnyResInCell( x, y, cell + 1, &DistR, res );
	res = FindAnyResInCell( x, y, cell - 1, &DistR, res );
	res = FindAnyResInCell( x, y, cell + VAL_SPRNX, &DistR, res );
	res = FindAnyResInCell( x, y, cell - VAL_SPRNX, &DistR, res );
	res = FindAnyResInCell( x, y, cell + VAL_SPRNX + 1, &DistR, res );
	res = FindAnyResInCell( x, y, cell + VAL_SPRNX - 1, &DistR, res );
	res = FindAnyResInCell( x, y, cell - VAL_SPRNX + 1, &DistR, res );
	res = FindAnyResInCell( x, y, cell - VAL_SPRNX - 1, &DistR, res );
	if (DistR < 64)return res;
	else return 0xFF;
};

extern int tmtmt;

//Sprites[].Surrounded = false
void ProcessSprites()
{
	if (0 == tmtmt % 8)
	{
		for (int i = 0; i < MaxSprt; i++)
		{
			Sprites[i].Surrounded = false;
		}
	}
}

bool OneObject::CheckLocking( int dx, int dy )
{
	return CheckBar( x + dx - 1, y + dy - 1, Lx + 2, Lx + 2 );
}

void OneObject::SetDestCoor( int px, int py )
{
	DestX = ( px << 8 ) + ( ( Lx ) << 7 );
	DestY = ( py << 8 ) + ( ( Lx ) << 7 );
}

void OneObject::EscapeLocking()
{
	//round 1
	if (!CheckLocking( -1, 0 ))
	{
		DeletePath();
		SetDestCoor( x - 1, y );
		return;
	};
	if (!CheckLocking( 1, 0 ))
	{
		DeletePath();
		SetDestCoor( x + 1, y );
		return;
	};
	if (!CheckLocking( 0, -1 ))
	{
		DeletePath();
		SetDestCoor( x, y - 1 );
		return;
	};
	if (!CheckLocking( 0, 1 ))
	{
		DeletePath();
		SetDestCoor( x, y + 1 );
		return;
	};
	if (!CheckLocking( -1, -1 ))
	{
		DeletePath();
		SetDestCoor( x - 1, y - 1 );
		return;
	};
	if (!CheckLocking( 1, 1 ))
	{
		DeletePath();
		SetDestCoor( x + 1, y + 1 );
		return;
	};
	if (!CheckLocking( 1, -1 ))
	{
		DeletePath();
		SetDestCoor( x + 1, y - 1 );
		return;
	};
	if (!CheckLocking( -1, 1 ))
	{
		DeletePath();
		SetDestCoor( x - 1, y + 1 );
		return;
	};
	if (!CheckLocking( -2, 0 ))
	{
		DeletePath();
		SetDestCoor( x - 2, y );
		return;
	};
	if (!CheckLocking( 2, 0 ))
	{
		DeletePath();
		SetDestCoor( x + 2, y );
		return;
	};
	if (!CheckLocking( 0, 2 ))
	{
		DeletePath();
		SetDestCoor( x, y + 2 );
		return;
	};
	if (!CheckLocking( 0, -2 ))
	{
		DeletePath();
		SetDestCoor( x, y - 2 );
		return;
	};
};
//-----------------Time handler-------------------//

TimeReq::TimeReq()
{
	NMembers = 0;
	MaxMembers = 0;
	IDS = NULL;
	Kinds = NULL;
};
TimeReq::~TimeReq()
{
	if (IDS)free( IDS );
	if (Kinds)free( Kinds );
	IDS = NULL;
	Kinds = NULL;
	NMembers = 0;
	Kinds = 0;
	MaxMembers = 0;
};
void TimeReq::Add( int ID, byte kind )
{
	if (!MaxMembers)
	{
		MaxMembers = 32;
		IDS = new int[MaxMembers];
		Kinds = new byte[MaxMembers];
	}
	else
	{
		;
		if (NMembers >= MaxMembers)
		{
			MaxMembers += 32;
			IDS = (int*) realloc( (void*) IDS, MaxMembers << 2 );
			Kinds = (byte*) realloc( (void*) Kinds, MaxMembers );
		};
	};
	IDS[NMembers] = ID;
	Kinds[NMembers] = kind;
	NMembers++;
};
void TimeReq::Del( int ID, byte Kind )
{
	for (int i = 0; i < NMembers; i++)
	{
		if (IDS[i] == ID&&Kinds[i] == Kind)
		{
			if (i < NMembers - 1)
			{
				memcpy( IDS + i, IDS + i + 1, ( NMembers - i - 1 ) << 2 );
				memcpy( Kinds + i, Kinds + i + 1, NMembers - i - 1 );
			};
			NMembers--;
		};
	};
};
void TimeReq::Handle()
{
	for (int i = 0; i < NMembers; i++)
	{
		int ID = IDS[i];
		byte Kind = Kinds[i];
		if (!Kind )
		{
			OneSprite* OS = &Sprites[ID];
			if ( nullptr == OS->OC )//BUGFIX: access violation
			{
				continue;
			}
			ObjCharacter* OC = OS->OC;
			SprGroup* SG = OS->SG;
			OS->TimePassed += FrmDec;
			if (OS->TimePassed >= OC->TimeAmount)
			{
				if (OC->TNextObj != 0xFFFF)
				{
					if (OS->SGIndex != OC->TNextObj)OS->WorkOver = 0;
					OS->SGIndex = OC->TNextObj;
					OS->OC = &SG->ObjChar[OS->SGIndex];
				}
				else
				{
					OS->Enabled = false;
					Del( OS->Index, 0 );
					UnregisterSprite( OS->Index );
				};
				OS->TimePassed = 0;
			};
		};
	};
};
TimeReq ObjTimer;
int MaxSprt;
