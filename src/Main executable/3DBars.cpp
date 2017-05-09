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
#include "3DBars.h"

OneBar* OBARS[8192];
int NBars;
word** Obj3Map;
word*  NObj3;
//Zero 3D Bars variables (?)
void InitObjs3()
{
	memset( OBARS, 0, sizeof OBARS );
	memset( Obj3Map, 0, B3SZ * 4 );
	memset( NObj3, 0, B3SZ * 2 );
	NBars = 0;
}

void ClearObjs3()
{
	for (int i = 0; i < NBars; i++)
	{
		if (OBARS[i])
		{
			free( OBARS[i] );
			OBARS[i] = NULL;
		};
	};
	for (int i = 0; i < B3SZ; i++)
	{
		if (Obj3Map[i])
		{
			free( Obj3Map[i] );
		};
	};
	InitObjs3();
};
void AddObj3( int CX, int CY, word BarID )
{
	if (CX < 0 || CY < 0 || CX >= B3SX || CY >= B3SY)return;
	int Cell = CX + ( CY*B3SX );
	word* CEMAP = Obj3Map[Cell];
	if (CEMAP)
	{
		int NCE = NObj3[Cell];
		for (int j = 0; j < NCE; j++)
		{
			if (CEMAP[j] == BarID)return;
		};
	};
	Obj3Map[Cell] = (word*) realloc( Obj3Map[Cell], ( NObj3[Cell] + 1 ) << 1 );
	Obj3Map[Cell][NObj3[Cell]] = BarID;
	NObj3[Cell]++;
	OneBar* ONB = OBARS[BarID];
	if (ONB)
	{
		if (CX < ONB->MinBX)ONB->MinBX = CX;
		if (CX > ONB->MaxBX)ONB->MaxBX = CX;
		if (CY < ONB->MinBY)ONB->MinBY = CY;
		if (CY > ONB->MaxBY)ONB->MaxBY = CY;
	};
};
void DelObj3( int CX, int CY, word BarID )
{
	if (CX < 0 || CY < 0 || CX >= B3SX || CY >= B3SY)return;
	int Cell = CX + ( CY*B3SX );
	word* CEMAP = Obj3Map[Cell];
	if (CEMAP)
	{
STARTA:;
		int NCE = NObj3[Cell];
		for (int j = 0; j < NCE; j++)
		{
			if (CEMAP[j] == BarID)
			{
				if (j < NCE - 1)memcpy( CEMAP + j, CEMAP + j + 1, ( NCE - j - 1 ) << 1 );
				NObj3[Cell]--;
				goto STARTA;
			};
		};
		if (Obj3Map[Cell] && !NObj3[Cell])
		{
			free( Obj3Map[Cell] );
			Obj3Map[Cell] = NULL;
		};
	};
};
word Add3DBar( int X0, int Y0, int X1, int Y1, short H, int ID, word Owner )
{
	word BarID = 0xFFFF;
	for (int i = 0; i < NBars&&BarID == 0xFFFF; i++)
	{
		if (!OBARS[i])BarID = i;
	};
	if (BarID == 0xFFFF)
	{
		if (NBars >= 8192)return 0xFFFF;
		BarID = NBars;
		NBars++;
	};
	OneBar* ONB = new OneBar;
	OBARS[BarID] = ONB;
	ONB->MaxBX = -1000;
	ONB->MaxBY = -1000;
	ONB->MinBX = 1000;
	ONB->MinBY = 1000;
	ONB->H = H;
	ONB->x0 = X0 - Y0;
	ONB->y0 = Y0 + X0;
	ONB->x1 = X1 - Y1;
	ONB->y1 = Y1 + X1;
	ONB->Owner = Owner;
	int D2 = ( Y1 - Y0 + X1 - X0 ) >> 1;
	int D1 = ( X1 - X0 - Y1 + Y0 ) >> 1;
	int D12 = D1 >> 1;
	int D22 = D2 >> 1;
	AddObj3( X0 >> B3SHIFT, Y0 >> B3SHIFT, BarID );
	AddObj3( X1 >> B3SHIFT, Y1 >> B3SHIFT, BarID );
	AddObj3( ( X0 + D1 ) >> B3SHIFT, ( Y0 - D1 ) >> B3SHIFT, BarID );
	AddObj3( ( X0 + D2 ) >> B3SHIFT, ( Y0 + D2 ) >> B3SHIFT, BarID );
	AddObj3( ( X0 + X1 ) >> ( B3SHIFT + 1 ), ( Y0 + Y1 ) >> ( B3SHIFT + 1 ), BarID );
	AddObj3( ( X0 + D12 ) >> B3SHIFT, ( Y0 - D12 ) >> B3SHIFT, BarID );
	AddObj3( ( X1 - D12 ) >> B3SHIFT, ( Y1 - D12 ) >> B3SHIFT, BarID );
	AddObj3( ( X0 + D22 ) >> B3SHIFT, ( Y0 + D22 ) >> B3SHIFT, BarID );
	AddObj3( ( X0 - D12 ) >> B3SHIFT, ( Y1 + D12 ) >> B3SHIFT, BarID );
	if (ID == 0xFFFF)ID = BarID;
	ONB->ID = ID;
	return BarID;
};
void CheckBars3D();
void CureBars3D()
{
	for (int x = 0; x < B3SX; x++)
		for (int y = 0; y < B3SY; y++)
		{
			int Cell = x + ( y*B3SX );
			word NBR = NObj3[Cell];
			if (NBR)
			{
				int xx = x - y;
				int yy = y + x;
				word* CMAP = Obj3Map[Cell];
				for (int i = 0; i < NBR; i++)
				{
					if (!OBARS[CMAP[i]])
					{
						DelObj3( x, y, CMAP[i] );
					};
				};
			};
		};
};
void Delete3DBar( word ID )
{
	//CheckBars3D();
	//assert(ID!=11);
	for (int i = 0; i < NBars; i++)
	{
		OneBar* ONB = OBARS[i];
		if (ONB&&ONB->ID == ID)
		{
			OBARS[i] = NULL;
			for (int ix = ONB->MinBX; ix <= ONB->MaxBX; ix++)
				for (int iy = ONB->MinBY; iy <= ONB->MaxBY; iy++)
				{
					DelObj3( ix, iy, i );
				};
			free( ONB );
		};
	};
	CureBars3D();
	CheckBars3D();
};
word OWNER;
int GetBar3DHeight( int x, int y )
{
	int CX = x >> B3SHIFT;
	int CY = y >> B3SHIFT;
	int MaxH = 0;
	OWNER = 0xFFFF;
	if (CX >= 0 && CX < B3SX&&CY >= 0 && CY < B3SY)
	{
		int Cell = CX + ( CY*B3SX );
		word NBR = NObj3[Cell];
		if (NBR)
		{
			int xx = x - y;
			int yy = y + x;
			word* CMAP = Obj3Map[Cell];
			for (int i = 0; i < NBR; i++)
			{
				OneBar* ONB = OBARS[CMAP[i]];
				if (xx >= ONB->x0&&xx <= ONB->x1&&yy >= ONB->y0&&yy <= ONB->y1)
				{
					if (MaxH < ONB->H)
					{
						MaxH = ONB->H;
						OWNER = ONB->Owner;
					};
				};
			};
		};
	};
	return MaxH;
};
void CheckBars3D()
{
	for (int x = 0; x < B3SX; x++)
		for (int y = 0; y < B3SY; y++)
		{
			int Cell = x + ( y*B3SX );
			word NBR = NObj3[Cell];
			if (NBR)
			{
				int xx = x - y;
				int yy = y + x;
				word* CMAP = Obj3Map[Cell];
				for (int i = 0; i < NBR; i++)
				{
					//					assert(OBARS[CMAP[i]]);
				};
			};
		};
};
int GetBar3DOwner( int x, int y )
{
	int CX = x >> B3SHIFT;
	int CY = y >> B3SHIFT;
	word Owner = 0xFFFF;
	if (CX >= 0 && CX < B3SX&&CY >= 0 && CY < B3SY)
	{
		int Cell = CX + ( CY*B3SX );
		word NBR = NObj3[Cell];
		int MaxH = 0;
		if (NBR)
		{
			int xx = x - y;
			int yy = y + x;
			word* CMAP = Obj3Map[Cell];
			for (int i = 0; i < NBR; i++)
			{
				OneBar* ONB = OBARS[CMAP[i]];
				if (xx >= ONB->x0&&xx <= ONB->x1&&yy >= ONB->y0&&yy <= ONB->y1)
				{
					if (MaxH < ONB->H)
					{
						MaxH = ONB->H;
						Owner = ONB->Owner;
					};
				};
			};
		};
	};
	return Owner;
}

void CBar( int x, int y, int Lx, int Ly, byte c );

__declspec( dllexport ) void xLine( int x, int y, int x1, int y1, byte c )
{
	int DD = int( sqrt( ( x - x1 )*( x - x1 ) + ( y - y1 )*( y - y1 ) ) );
	if (!DD)
	{
		return;
	}
	int NP = DD + 1;
	for (int i = 0; i <= NP; i++)
	{
		CBar( x + div( ( x1 - x )*i, NP ).quot, y + div( ( y1 - y )*i, NP ).quot, 1, 1, c );
	}
}

extern int RealLx;
extern int RealLy;

void DrawBar3D( OneBar* ONB )
{
	int x0 = ( ONB->x0 + ONB->y0 ) >> 1;
	int y0 = ( ONB->y0 - ONB->x0 ) >> 1;
	int x1 = ( ONB->x1 + ONB->y1 ) >> 1;
	int y1 = ( ONB->y1 - ONB->x1 ) >> 1;
	int L1 = ( ONB->x1 - ONB->x0 ) >> 1;
	int L2 = ( ONB->y1 - ONB->y0 ) >> 1;
	int xc = ( x0 + x1 ) >> 1;
	int yc = ( y0 + y1 ) >> 1;
	int zz = GetHeight( xc, yc );
	int sdx = ( mapx << 5 );
	int sdy = ( mapy << 4 ) + zz;
	x0 -= sdx;
	y0 = ( y0 >> 1 ) - sdy;
	x1 -= sdx;
	y1 = ( y1 >> 1 ) - sdy;
	xc -= sdx;
	yc = ( yc >> 1 ) - sdy;
	xc = ( x0 + x1 ) >> 1;
	yc = ( y0 + y1 ) >> 1;
	int x2 = x0 + L2;
	int y2 = y0 + ( L2 >> 1 );
	int x3 = x0 + L1;
	int y3 = y0 - ( L1 >> 1 );
	int H = ONB->H;
	if (xc > -128 && yc > -128 && xc < RealLx + 128 && yc < RealLy + 128)
	{
		Vline( x0, y0, y0 - H, 255 );
		Vline( x2, y2, y2 - H, 255 );
		Vline( x1, y1, y1 - H, 255 );
		xLine( x0, y0, x2, y2, 255 );
		xLine( x1, y1, x2, y2, 255 );
		xLine( x0, y0 - H, x2, y2 - H, 255 );
		xLine( x1, y1 - H, x2, y2 - H, 255 );
		xLine( x0, y0 - H, x3, y3 - H, 255 );
		xLine( x1, y1 - H, x3, y3 - H, 255 );
	};
};
void ShowAllBars()
{
	for (int i = 0; i < NBars; i++)
	{
		OneBar* ONB = OBARS[i];
		if (ONB)
		{
			DrawBar3D( ONB );
		};
	};
};