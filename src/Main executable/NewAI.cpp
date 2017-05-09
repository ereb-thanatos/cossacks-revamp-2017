
#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
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
#include "TopoGraf.h"
#include "Danger.h"
#include "StrategyResearch.h"
#include "Safety.h"
#include "EinfoClass.h"
class BuildingAI
{
public:
	word ID;
	word SN;
	short sx;
	short sy;
	int MoreInfo;
};
class BuildingProject
{
public:
	short apprX;
	short apprY;
	byte Stage;//0-search place,1-removing units,2-search for builders
	int Time;
	int PrecX;
	int PrecY;
	int PushOutRadius;
	int Razbros;
};
class Kingdom
{
public:
	word NSklad;
	word MaxSklad;
	BuildingAI* Sklads;
	word NFarms;
	word MaxFarm;
	word OnLandPeasants[1024];
	word NPeasants;
	bool BuildProjectPresent;
	BuildingProject BPR;
};
//01-wood(trees>2)
//02-stone(stones>5)
//04-full empty
//08-empty from locking
//16-plane zone
//32-high plate
byte* InfoMap;
//01-sklad
//02-farm
//04-tower
//08-building
//16-melnica
byte* CantBuild;

int* MineList;
word NMines;
word MaxMine;
void CreateInfoMap()
{
	memset( CantBuild, 0, VAL_SPRNX*VAL_SPRNX );
	int mmax = msx >> 2;
	int mmay = msy >> 2;
	for (int y = 0; y <= mmay; y++)
	{
		for (int x = 0; x <= mmax; x++)
		{
			byte ms = 0;
			if (!CheckBar( x << 3, y << 3, 8, 8 ))ms |= 8;
			int x0 = x << 7;
			int y0 = y << 7;
			int H1 = GetHeight( x0, y0 );
			int H2 = GetHeight( x0 + 128, y0 );
			int H3 = GetHeight( x0, y0 + 128 );
			int H4 = GetHeight( x0, y0 + 128 );
			int HH = ( H1 + H2 + H3 + H4 ) >> 2;
			if (abs( H1 - HH ) < 20 && abs( H2 - HH ) < 20 && abs( H3 - HH ) < 20 && abs( H4 - HH ) < 20)ms |= 16;
			//resource checking;
			int ofst = x + y*VAL_SPRNX;
			int N = NSpri[ofst];
			int NTrees = 0;
			int NStones = 0;
			if (( ms & 8 ) && ( ms & 16 ) && !N)ms |= 4;
			int* spr = SpRefs[ofst];
			for (int i = 0; i < N; i++)
			{
				word SID = spr[i];
				OneSprite* OS = &Sprites[SID];
				SprGroup* SG = OS->SG;
				ObjCharacter* OC = &SG->ObjChar[OS->SGIndex];
				if (OC->ResType == TreeID)NTrees++;
				if (OC->ResType == StoneID)NStones++;
			};
			if (NTrees > 2)ms |= 1;
			if (NStones > 5)ms |= 2;
			if (NTrees || NStones)ms &= ~4;
			InfoMap[ofst] = ms;
		};
	};
	if (MineList)free( MineList );
	NMines = 0;
	MineList = new int[256];
	MaxMine = 256;
	for (int i = 0; i < MaxSprt; i++)
	{
		OneSprite* OS = &Sprites[i];
		if (OS->Enabled)
		{
			SprGroup* SG = OS->SG;
			ObjCharacter* OC = &SG->ObjChar[OS->SGIndex];
			if (OC->IntResType < 8)
			{
				if (NMines >= MaxMine)
				{
					MaxMine += 256;
					MineList = (int*) realloc( MineList, MaxMine << 2 );
				};
				MineList[NMines] = i;
				NMines++;
			};
		};
	};
};
//x,y-pixel coordinates
void CHKS();
void RenewInfoMap( int x, int y )
{
	x = ( x >> 7 ) - 1;
	y = ( y >> 7 ) - 1;
	for (int dx = 0; dx < 3; dx++)
	{
		for (int dy = 0; dy < 3; dy++)
		{
			int sx = x + dx;
			int sy = y + dy;
			if (sx >= 0 && sy >= 0)
			{
				int ofst = sx + sy*VAL_SPRNX;
				byte ms = InfoMap[ofst];
				ms &= 16;
				if (!CheckBar( sx << 3, sy << 3, 8, 8 ))ms |= 8;
				int N = NSpri[ofst];
				int NTrees = 0;
				int NStones = 0;
				if (( ms & 8 ) && ( ms & 16 ) && !N)ms |= 4;
				int* spr = SpRefs[ofst];
				for (int i = 0; i < N; i++)
				{
					word SID = spr[i];
					OneSprite* OS = &Sprites[SID];
					SprGroup* SG = OS->SG;
					ObjCharacter* OC = &SG->ObjChar[OS->SGIndex];
					if (OC->ResType == TreeID)NTrees++;
					if (OC->ResType == StoneID)NStones++;
				};
				if (NTrees)ms |= 1;
				if (NStones)ms |= 2;
				InfoMap[ofst] = ms;
			};
		};
	};
};
void FreeInfoMap()
{
	free( MineList );
};
//special search procedures
//1.Melnica&field

int SsMaxX;
int SsMaxY;
#define FieldSX 2
bool CheckMelnica( int x, int y )
{
	if (x <= FieldSX || y <= FieldSX || x >= SsMaxX - FieldSX || y >= SsMaxY - FieldSX)return false;
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Melnica)return false;
	ofst -= FieldSX + VAL_SPRNX*FieldSX;
	for (int dy = -FieldSX; dy <= FieldSX; dy++)
	{
		for (int dx = -FieldSX; dx <= FieldSX; dx++)
		{
			if (!( InfoMap[ofst] & 4 ))return false;
			ofst++;
		};
		ofst += VAL_SPRNX - FieldSX - FieldSX - 1;
	};
	return true;
};
bool CheckStoneSklad( int x, int y )
{
	if (x <= 1 || y <= 1 || x >= SsMaxX - 1 || y >= SsMaxY - 1)return false;
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Sklad)return false;
	if (!( InfoMap[ofst] & 4 ))return false;
	if (( InfoMap[ofst - VAL_SPRNX] & 2 ) || ( InfoMap[ofst + VAL_SPRNX] & 2 ) ||
		( InfoMap[ofst + 1] & 2 ) || ( InfoMap[ofst - 1] & 2 ) ||
		( InfoMap[ofst - VAL_SPRNX - 1] & 2 ) || ( InfoMap[ofst - VAL_SPRNX + 1] & 2 ) ||
		( InfoMap[ofst + VAL_SPRNX - 1] & 2 ) || ( InfoMap[ofst - VAL_SPRNX + 1] & 2 ))return true;
	return false;
};
bool CheckWoodSklad( int x, int y )
{
	if (x <= 1 || y <= 1 || x >= SsMaxX - 1 || y >= SsMaxY - 1)return false;
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Sklad)return false;
	if (!( InfoMap[ofst] & 4 ))return false;
	if (( InfoMap[ofst - VAL_SPRNX] & 1 ) || ( InfoMap[ofst + VAL_SPRNX] & 1 ) ||
		( InfoMap[ofst + 1] & 1 ) || ( InfoMap[ofst - 1] & 1 ) ||
		( InfoMap[ofst - VAL_SPRNX - 1] & 1 ) || ( InfoMap[ofst - VAL_SPRNX + 1] & 1 ) ||
		( InfoMap[ofst + VAL_SPRNX - 1] & 1 ) || ( InfoMap[ofst - VAL_SPRNX + 1] & 1 ))return true;
	return false;
};
bool CheckStoneWoodSklad( int x, int y )
{
	if (x <= 2 || y <= 2 || x >= SsMaxX - 2 || y >= SsMaxY - 2)return false;
	int ofst = ( x - 2 ) + ( y - 2 )*VAL_SPRNX;
	int of1 = ofst + 2 + ( 2 << SprShf );
	if (CantBuild[of1] & CB_Sklad)return false;
	if (!( InfoMap[of1] & 4 ))return false;
	bool tpresent = false;
	for (int ix = 0; ix < 5; ix++)
	{
		for (int iy = 0; iy < 5; iy++)
		{
			if (InfoMap[ofst + ix + ( iy << SprShf )] & 2)tpresent = true;
		};
	};
	if (!tpresent)return false;
	for (int ix = 0; ix < 5; ix++)
	{
		for (int iy = 0; iy < 5; iy++)
		{
			if (InfoMap[ofst + ix + ( iy << SprShf )] & 1)return true;
		};
	};
	/*
	if(!((InfoMap[ofst-VAL_SPRNX]&2)||(InfoMap[ofst+VAL_SPRNX]&2)||
	   (InfoMap[ofst+1]&2)||(InfoMap[ofst-1]&2)||
	   (InfoMap[ofst-VAL_SPRNX-1]&2)||(InfoMap[ofst-VAL_SPRNX+1]&2)||
	   (InfoMap[ofst+VAL_SPRNX-1]&2)||(InfoMap[ofst-VAL_SPRNX+1]&2)))return false;
	if((InfoMap[ofst-VAL_SPRNX]&1)||(InfoMap[ofst+VAL_SPRNX]&1)||
	   (InfoMap[ofst+1]&1)||(InfoMap[ofst-1]&1)||
	   (InfoMap[ofst-VAL_SPRNX-1]&1)||(InfoMap[ofst-VAL_SPRNX+1]&1)||
	   (InfoMap[ofst+VAL_SPRNX-1]&1)||(InfoMap[ofst-VAL_SPRNX+1]&1))return true;
	*/
	return false;
}

bool CheckFarm( int x, int y )
{
	if (x <= 1 || y <= 1 || x >= SsMaxX - 1 || y >= SsMaxY - 1)
	{
		return false;
	}
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Farm)
	{
		return false;
	}
	bool retval = ( InfoMap[ofst] & 4 ) != 0;
	return retval;
}

bool CheckBuilding( int x, int y )
{
	if (x <= 1 || y <= 1 || x >= SsMaxX - 1 || y >= SsMaxY - 1)return false;
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Building)return false;
	return ( InfoMap[ofst] & 4 ) && ( InfoMap[ofst + 1] & 4 ) && ( InfoMap[ofst + VAL_SPRNX] & 4 ) && ( InfoMap[ofst + VAL_SPRNX + 1] & 4 );
}

byte NPORTS;
short PORTSX[32];
short PORTSY[32];
bool CheckTower( int x, int y )
{
	for (int i = 0; i < NPORTS; i++)
	{
		if (Norma( int( PORTSX[i] ) - x, int( PORTSY[i] ) - y ) < 9)return false;
	};
	if (x <= 1 || y <= 1 || x >= SsMaxX - 1 || y >= SsMaxY - 1)return false;
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Building)return false;
	return ( InfoMap[ofst] & 4 ) && ( InfoMap[ofst + 1] & 4 ) && ( InfoMap[ofst + VAL_SPRNX] & 4 ) && ( InfoMap[ofst + VAL_SPRNX + 1] & 4 );
};
bool CheckPort( int x, int y )
{
	if (x <= 5 || y <= 5 || x >= SsMaxX - 5 || y >= SsMaxY - 5)return false;
	for (int i = 0; i < NPORTS; i++)
	{
		if (Norma( int( PORTSX[i] ) - x, int( PORTSY[i] ) - y ) < 9)return false;
	};
	int ofst = x + y*VAL_SPRNX;
	if (CantBuild[ofst] & CB_Port)return false;
	x <<= 2;
	y <<= 2;
	int ofsw = x + 2 + ( y + 2 )*( MaxWX );
	int DY = MAPSX * 10;
	int DX = 20;
	return !( WaterDeep[ofsw - DY] < 128 && WaterDeep[ofsw + DY] < 128 && WaterDeep[ofsw - DX] < 128 && WaterDeep[ofsw + DX] < 128 &&
		WaterDeep[ofsw - DY + DX] < 128 && WaterDeep[ofsw - DY - DX] < 128 && WaterDeep[ofsw + DY - DX] < 128 && WaterDeep[ofsw + DY + DX] < 128 );
};
extern int CURRENTAINATION;
int GetTopology( int x, int y );
bool SearchPlace( int* xx1, int* yy1, SearchFunction* SFN, int r )
{
	SsMaxX = msx >> 2;
	SsMaxY = msy >> 2;
	int x = *xx1;
	int y = *yy1;
	if (SFN( x, y ))
	{
		//assert(y<17648);
		return true;
	};
	int MyTop = GetTopology( x << 7, y << 7 );
	if (MyTop >= 0xFFFE)return false;
	int REALMYTOP = MyTop;
	MyTop *= NAreas;
	int PrevTopDst = 0;
	for (int i = 1; i < r; i++)
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for (int j = 0; j < N; j++)
		{
			int xx = x + xi[j];
			int yy = y + yi[j];
			if (xx > 0 && yy > 0 && xx < SsMaxX&&yy < SsMaxY)
			{
				if (!GNFO.EINF[CURRENTAINATION]->GetSafeVal( xx, yy ))
				{
					int top = TopRef[xx + xx + ( yy << ( TopSH + 1 ) )];
					if (top < 0xFFFE)
					{
						int TDST = LinksDist[MyTop + top];
						if (top == REALMYTOP)TDST = 0;
						if (TDST - PrevTopDst < 24)
						{
							if (TDST > PrevTopDst)PrevTopDst = TDST;
							if (SFN( xx, yy ))
							{
								*xx1 = xx;
								*yy1 = yy;
								return true;
							};
						};
					};
				};
			};
		};
	};
	return false;
};
bool SearchTowerPlace( int* xx1, int* yy1, SearchFunction* SFN, int r, int xc, int yc, int xe, int ye )
{
	if (NPORTS > 1)return SearchPlace( xx1, yy1, SFN, r );
	SsMaxX = msx >> 2;
	SsMaxY = msy >> 2;
	int x = *xx1;
	int y = *yy1;
	if (SFN( x, y ))
	{
		//assert(y<17648);
		return true;
	};
	int MyTop = GetTopology( x << 7, y << 7 );
	if (MyTop >= 0xFFFE)return false;
	MyTop *= NAreas;
	int PrevTopDst = 0;
	for (int i = 6; i < r; i++)
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for (int j = 0; j < N; j++)
		{
			int xx = x + xi[j];
			int yy = y + yi[j];
			if (( xx - xc )*( xe - xc ) + ( yy - yc )*( ye - yc ) > 0 && xx > 0 && yy > 0 && xx < SsMaxX&&yy < SsMaxY)
			{
				if (!GNFO.EINF[CURRENTAINATION]->GetSafeVal( xx, yy ))
				{
					int top = TopRef[xx + xx + ( yy << ( TopSH + 1 ) )];
					if (top < 0xFFFE)
					{
						int TDST = LinksDist[MyTop + top];
						if (TDST - PrevTopDst < 14)
						{
							if (TDST > PrevTopDst)PrevTopDst = TDST;
							if (SFN( xx, yy ))
							{
								*xx1 = xx;
								*yy1 = yy;
								return true;
							};
						};
					};
				};
			};
		};
	};
	return false;
};
bool FindCenter( int* xx, int *yy, byte NI )
{
	int N = 0;
	int x = 0;
	int y = 0;
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB&&OB->NNUM == NI&&OB->newMons->Peasant)
		{
			x += OB->RealX >> 4;
			y += OB->RealY >> 4;
			N++;
		};
	};
	if (N)
	{
		x = ( div( x, N ).quot ) >> 7;
		y = ( div( y, N ).quot ) >> 7;
		*xx = x;
		*yy = y;
		return true;
	}
	else return false;
};
void SetUnusable( int x, int y, byte Mask )
{
	if (x < 0 || y < 0 || x >= SsMaxX || y >= SsMaxY)return;
	CantBuild[x + y*VAL_SPRNX] |= Mask;
};
