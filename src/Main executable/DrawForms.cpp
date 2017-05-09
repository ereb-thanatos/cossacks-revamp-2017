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

DWORD GP_OPT;

extern int WindX;
extern int WindY;
extern int WindLx;
extern int WindLy;
extern int WindX1;
extern int WindY1;
extern int ScrWidth;

void PushWindow( TempWindow* W )
{
	W->ScrWidth = ScrWidth;
	W->WindLx = WindLx;
	W->WindLy = WindLy;
	W->WindX = WindX;
	W->WindX1 = WindX1;
	W->WindY = WindY;
	W->WindY1 = WindY1;
};
void PopWindow( TempWindow* W )
{
	ScrWidth = W->ScrWidth;
	WindLx = W->WindLx;
	WindLy = W->WindLy;
	WindX = W->WindX;
	WindX1 = W->WindX1;
	WindY = W->WindY;
	WindY1 = W->WindY1;
};
void IntersectWindows( int x0, int y0, int x1, int y1 )
{
	if (x0 > WindX)WindX = x0;
	if (y0 > WindY)WindY = y0;
	if (x1 < WindX1)WindX1 = x1;
	if (y1 < WindY1)WindY1 = y1;
	if (WindX1 < WindX)
	{
		WindX1 = 0;
		WindX = 0;
	};
	if (WindY1 < WindY)
	{
		WindY1 = 0;
		WindY = 0;
	};
	WindLx = WindX1 - WindX + 1;
	WindLy = WindY1 - WindY + 1;
};
void DrawRect( int x0, int y0, int x1, int y1, Corners* CR, int GP_File )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	IntersectWindows( x0, y0 - 32, x1, y1 + 32 );
	int N = ( x1 - x0 + 1 ) >> 6;
	for (int i = 0; i <= N; i++)
	{
		if (CR->LU != -1)GPS.ShowGPLayers( x0 + ( i << 6 ), y0 - 32, GP_File, CR->LU, MyNation, GP_OPT );
		if (CR->LD != -1)GPS.ShowGPLayers( x0 + ( i << 6 ), y1 - 32, GP_File, CR->LD, MyNation, GP_OPT );
	};
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, y0, x1 + 32, y1 );
	N = ( y1 - y0 + 1 ) >> 6;
	for (int i = 0; i <= N; i++)
	{
		if (CR->LL != -1)GPS.ShowGPLayers( x0 - 32, y0 + ( i << 6 ), GP_File, CR->LL, MyNation, GP_OPT );
		if (CR->LR != -1)GPS.ShowGPLayers( x1 - 32, y0 + ( i << 6 ), GP_File, CR->LR, MyNation, GP_OPT );
	};
	PopWindow( &TEMP );
	if (CR->CLU != -1)GPS.ShowGPLayers( x0 - 32, y0 - 32, GP_File, CR->CLU, MyNation, GP_OPT );
	if (CR->CRU != -1)GPS.ShowGPLayers( x1 - 32, y0 - 32, GP_File, CR->CRU, MyNation, GP_OPT );
	if (CR->CLD != -1)GPS.ShowGPLayers( x0 - 32, y1 - 32, GP_File, CR->CLD, MyNation, GP_OPT );
	if (CR->CRD != -1)GPS.ShowGPLayers( x1 - 32, y1 - 32, GP_File, CR->CRD, MyNation, GP_OPT );
	PopWindow( &TEMP );
};
void DrawRect1( int x0, int y0, int x1, int y1, Corners* CR, int GP_File )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	IntersectWindows( x0 + 32, y0 - 32, x1 - 33, y1 + 32 );
	int N = ( x1 - x0 + 1 - 64 ) >> 6;
	for (int i = 0; i <= N; i++)
	{
		if (CR->LU != -1)GPS.ShowGPLayers( x0 + ( i << 6 ) + 32, y0 - 32, GP_File, CR->LU, MyNation, GP_OPT );
		if (CR->LD != -1)GPS.ShowGPLayers( x0 + ( i << 6 ) + 32, y1 - 32, GP_File, CR->LD, MyNation, GP_OPT );
	};
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, y0 + 32, x1 + 32, y1 - 33 );
	N = ( y1 - y0 + 1 - 64 ) >> 6;
	for (int i = 0; i <= N; i++)
	{
		if (CR->LL != -1)GPS.ShowGPLayers( x0 - 32, y0 + ( i << 6 ) + 32, GP_File, CR->LL, MyNation, GP_OPT );
		if (CR->LR != -1)GPS.ShowGPLayers( x1 - 32, y0 + ( i << 6 ) + 32, GP_File, CR->LR, MyNation, GP_OPT );
	};
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, y0 - 32, x1 + 32, ( y1 + y0 ) / 2 - 1 );
	if (CR->CLU != -1)GPS.ShowGPLayers( x0 - 32, y0 - 32, GP_File, CR->CLU, MyNation, GP_OPT );
	if (CR->CRU != -1)GPS.ShowGPLayers( x1 - 32, y0 - 32, GP_File, CR->CRU, MyNation, GP_OPT );
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, ( y1 + y0 ) / 2, x1 + 32, y1 + 32 );
	if (CR->CLD != -1)GPS.ShowGPLayers( x0 - 32, y1 - 32, GP_File, CR->CLD, MyNation, GP_OPT );
	if (CR->CRD != -1)GPS.ShowGPLayers( x1 - 32, y1 - 32, GP_File, CR->CRD, MyNation, GP_OPT );
	PopWindow( &TEMP );
};
void DrawRect3( int x0, int y0, int x1, int y1, Corners* CR, int GP_File )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	IntersectWindows( x0 + 32, y0 - 32, x1 - 33, y1 + 32 );
	int N = ( x1 - x0 + 1 - 64 ) >> 6;
	for (int i = 0; i <= N; i++)
	{
		if (CR->LU != -1)GPS.ShowGP( x0 + ( i << 6 ) + 32, y0 - 32, GP_File, CR->LU, MyNation );
		if (CR->LD != -1)GPS.ShowGP( x0 + ( i << 6 ) + 32, y1 - 32, GP_File, CR->LD, MyNation );
	};
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, y0 + 32, x1 + 32, y1 - 33 );
	N = ( y1 - y0 + 1 - 64 ) >> 6;
	for (int i = 0; i <= N; i++)
	{
		if (CR->LL != -1)GPS.ShowGP( x0 - 32, y0 + ( i << 6 ) + 32, GP_File, CR->LL, MyNation );
		if (CR->LR != -1)GPS.ShowGP( x1 - 32, y0 + ( i << 6 ) + 32, GP_File, CR->LR, MyNation );
	};
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, y0 - 32, x1 + 32, ( y1 + y0 ) / 2 - 1 );
	if (CR->CLU != -1)GPS.ShowGP( x0 - 32, y0 - 32, GP_File, CR->CLU, MyNation );
	if (CR->CRU != -1)GPS.ShowGP( x1 - 32, y0 - 32, GP_File, CR->CRU, MyNation );
	PopWindow( &TEMP );
	IntersectWindows( x0 - 32, ( y1 + y0 ) / 2, x1 + 32, y1 + 32 );
	if (CR->CLD != -1)GPS.ShowGP( x0 - 32, y1 - 32, GP_File, CR->CLD, MyNation );
	if (CR->CRD != -1)GPS.ShowGP( x1 - 32, y1 - 32, GP_File, CR->CRD, MyNation );
	PopWindow( &TEMP );
}

void DrawRect4( int x0, int y0, int x1, int y1, Corners* CR, int GP_File )
{
	TempWindow TEMP;

	PushWindow( &TEMP );

	int ULLX = GPS.GetGPWidth( GP_File, CR->CLU );
	if (ULLX < 64)
	{
		ULLX = 64;
	}
	int LX2 = ULLX / 2;

	int DLLX = GPS.GetGPWidth( GP_File, CR->CLD );
	if (DLLX < 64)
	{
		DLLX = 64;
	}
	int LX3 = DLLX / 2;

	int UPLX = GPS.GetGPWidth( GP_File, CR->LU );
	if (UPLX < 64)
	{
		UPLX = 64;
	}

	int DNLX = GPS.GetGPWidth( GP_File, CR->LD );
	if (DNLX < 64)
	{
		DNLX = 64;
	}

	int LSLY = GPS.GetGPHeight( GP_File, CR->CLU );
	int LLY2 = LSLY / 2;

	int LEFTLY = GPS.GetGPHeight( GP_File, CR->LL );
	if (LEFTLY < 64)
	{
		LEFTLY = 64;
	}

	int RIGHTLY = GPS.GetGPHeight( GP_File, CR->LR );
	if (RIGHTLY < 64)
	{
		RIGHTLY = 64;
	}

	//horizontal lines
	IntersectWindows( x0 + LX2, y0 - LLY2, x1 - LX2 - 1, y1 + LLY2 );
	int N = ( x1 - x0 + 1 - ULLX ) / UPLX;
	for (int i = 0; i <= N + 2; i++)
	{
		if (CR->LU != -1)
		{
			GPS.ShowGPLayers( x0 + i*UPLX + LX2, y0 - LLY2, GP_File, CR->LU, MyNation, GP_OPT );
		}
	}

	PopWindow( &TEMP );

	IntersectWindows( x0 + LX3, y0 - LLY2, x1 - LX3 - 1, y1 + LLY2 );

	N = ( x1 - x0 + 1 - ULLX ) / DNLX;
	for (int i = 0; i <= N + 2; i++)
	{
		if (CR->LD != -1)
		{
			GPS.ShowGPLayers( x0 + i*DNLX + LX3, y1 - LX3, GP_File, CR->LD, MyNation, GP_OPT );
		}
	}

	//vertical lines
	PopWindow( &TEMP );

	IntersectWindows( x0 - LX3, y0 + LLY2, x1 + LX3, y1 - LLY2 );

	N = ( y1 - y0 + 1 - LSLY ) / LEFTLY;
	for (int i = 0; i <= N + 1; i++)
	{
		if (CR->LL != -1)
		{
			GPS.ShowGPLayers( x0 - LX3, y0 + i*LEFTLY + LLY2, GP_File, CR->LL, MyNation, GP_OPT );
		}
	}

	N = ( y1 - y0 + 1 - LSLY ) / RIGHTLY;
	for (int i = 0; i <= N + 1; i++)
	{
		if (CR->LR != -1)
		{
			GPS.ShowGPLayers( x1 - LX3, y0 + i*RIGHTLY + LLY2, GP_File, CR->LR, MyNation, GP_OPT );
		}
	}

	//corners
	PopWindow( &TEMP );

	IntersectWindows( x0 - LX2, y0 - LLY2, ( x1 + x0 ) / 2 - 1, ( y1 + y0 ) / 2 - 1 );

	if (CR->CLU != -1)
	{
		GPS.ShowGPLayers( x0 - LX2, y0 - LLY2, GP_File, CR->CLU, MyNation, GP_OPT );
	}

	PopWindow( &TEMP );

	IntersectWindows( ( x0 + x1 ) / 2, y0 - LLY2, x1 + LX2, ( y1 + y0 ) / 2 - 1 );

	if (CR->CRU != -1)
	{
		GPS.ShowGPLayers( x1 - LX2, y0 - LLY2, GP_File, CR->CRU, MyNation, GP_OPT );
	}

	PopWindow( &TEMP );

	IntersectWindows( x0 - LX3, ( y1 + y0 ) / 2, ( x0 + x1 ) / 2 - 1, y1 + LLY2 );

	if (CR->CLD != -1)
	{
		GPS.ShowGPLayers( x0 - LX3, y1 - LLY2, GP_File, CR->CLD, MyNation, GP_OPT );
	}

	PopWindow( &TEMP );

	IntersectWindows( ( x0 + x1 ) / 2, ( y1 + y0 ) / 2, x1 + LX3, y1 + LLY2 );

	if (CR->CRD != -1)
	{
		GPS.ShowGPLayers( x1 - LX3, y1 - LLY2, GP_File, CR->CRD, MyNation, GP_OPT );
	}

	PopWindow( &TEMP );
}

void DrawFilledRect( int x0, int y0, int x1, int y1, Corners* CR, int GP_File, int StartFill, int NFill )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	IntersectWindows( x0, y0, x1, y1 );
	int nx = ( x1 - x0 ) >> 6;
	int ny = ( y1 - y0 ) >> 6;
	for (int ix = 0; ix <= nx; ix++)
		for (int iy = 0; iy <= ny; iy++)
			GPS.ShowGPLayers( x0 + ( ix << 6 ), y0 + ( iy << 6 ), GP_File, StartFill + ( ( ix*ix + iy*iy*iy ) % NFill ), MyNation, GP_OPT );
	PopWindow( &TEMP );
	DrawRect4( x0, y0, x1, y1, CR, GP_File );
}

void DrawFilledRect1( int x0, int y0, int x1, int y1, Corners* CR, int GP_File, int StartFill, int NFill )
{
	TempWindow TEMP;

	PushWindow( &TEMP );

	IntersectWindows( x0, y0, x1, y1 );

	int nx = ( x1 - x0 ) >> 6;
	int ny = ( y1 - y0 ) >> 6;

	//Background fill for various UI elements
	for (int ix = 0; ix <= nx; ix++)
	{
		for (int iy = 0; iy <= ny; iy++)
		{
			GPS.ShowGPLayers( x0 + ( ix << 6 ), y0 + ( iy << 6 ),
				GP_File, StartFill + ( ( ix*ix + iy*iy*iy ) % NFill ),
				MyNation, GP_OPT );
		}
	}

	PopWindow( &TEMP );

	DrawRect4( x0, y0, x1, y1, CR, GP_File );
}

__declspec( dllexport ) void DrawFilledRect3( int x0, int y0, int x1, int y1, Corners* CR, int GP_File, int StartFill, int NFill )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	IntersectWindows( x0, y0, x1, y1 );
	int nx = ( x1 - x0 ) >> 6;
	int ny = ( y1 - y0 ) >> 6;
	for (int ix = 0; ix <= nx; ix++)
		for (int iy = 0; iy <= ny; iy++)
			GPS.ShowGP( x0 + ( ix << 6 ), y0 + ( iy << 6 ), GP_File, StartFill + ( ( ix*ix + iy*iy*iy ) % NFill ), MyNation );
	PopWindow( &TEMP );
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, CR, GP_File );
}

void DrawHeader( int x0, int y0, int Lx, int GP_File, int Frame )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	if (Lx < 64)Lx = 64;
	if (Lx > 64)
	{
		Lx >>= 1;
		IntersectWindows( x0 - Lx + 32, y0 - 32, x0 + Lx - 32, y0 + 32 );
		int N = ( Lx >> 5 ) + 1;
		for (int i = 0; i < N; i++)GPS.ShowGPLayers( x0 - Lx + ( i << 6 ), y0 - 32, GP_File, Frame + 1, MyNation, GP_OPT );
	}
	else Lx >>= 1;
	PopWindow( &TEMP );
	GPS.ShowGPLayers( x0 - 32 - Lx, y0 - 32, GP_File, Frame, MyNation, GP_OPT );
	GPS.ShowGPLayers( x0 - 32 + Lx, y0 - 32, GP_File, Frame + 2, MyNation, GP_OPT );
};
void DrawVScroller( int x, int y, int Ly, int GP_File, int Up, int Down, int Center, int Ncenter )
{
	TempWindow TEMP;
	PushWindow( &TEMP );
	int ULY = GPS.GetGPHeight( GP_File, Up ) / 2;
	IntersectWindows( x - 64, y + ULY, x + 64, y + Ly - 1 - ULY );
	int CLY = GPS.GetGPHeight( GP_File, Center );

	if (CLY)
	{
		int N = Ly / CLY;
		for (int i = 0; i <= N; i++)GPS.ShowGP( x, y + i*CLY + ULY, GP_File, Center + ( i%Ncenter ), 0 );
	};
	PopWindow( &TEMP );
	GPS.ShowGP( x, y, GP_File, Up, 0 );
	GPS.ShowGP( x, y + Ly - GPS.GetGPHeight( GP_File, Down ), GP_File, Down, 0 );
};
//---------------------Using----------------------//
Corners StdRect = { 0,2,6,8,12,14,9,11 };
Corners LeftRect = { 0,-1,6,-1,12,-1,9,11 };
Corners RightRect = { 1,2,7,8,13,14,9,11 };
Corners UpRect = { 0,2,-1,-1,12,14,9,-1 };
Corners DnRect = { 3,5,6,8,12,14,10,11 };
Corners Table = { 17,18,19,20,21,22,23,24 };
Corners UpTable = { 17,18,-1,-1,12,14,23,-1 };
Corners UpTable1 = { 17,87,-1,-1,12,36,23,-1 };
Corners DnTable = { 3,5,6,8,12,14,10,11 };
Corners DnTable1 = { 3,22,6,24,12,36,10,11 };
Corners DnTable2 = { 3,86,6,8,12,14,10,11 };
Corners Paper = { 43,44,45,46,49,50,47,48 };
Corners HintPan = { 58,59,60,61,62,63,64,65 };
Corners UpTable3 = { 0,2,-1,-1,12,14,9,-1 };
Corners DnTable3 = { 3,5,6,8,12,14,10,11 };
Corners HdrTbl = { 17,18,6,8,12,14,23,11 };

//Corners for the statistics panel
Corners ST_Top = { 17,18,3,5,12,14,23,10 };

//Corners ST_Middle = { -1,-1,68,69,12,14,-1,70 };
//Corners ST_Bottom = { -1,-1,6,8,12,14,-1,11 };
Corners ST_Middle = { 0,-1,68,69,12,14,-1,70 };//BUGFIX: -1 caused wrong coordinates
Corners ST_Bottom = { 0,-1,6,8,12,14,-1,11 };

Corners ST_Left = { 17,-1,6,-1,12,-1,23,11 };
Corners ST_Right = { 23/*wrong!*/,18,7,8,13,14,23,11 };

//-----PALETTE 2---------//
Corners P2_NORMAL = { 0,1,2,3,4,5,6,7 };

int BordGP2 = -1;

void DrawHdrTable( int x0, int y0, int x1, int y1 )
{
	DrawFilledRect1( x0, y0, x1, y1, &HdrTbl, BordGP, 28, 3 );
}

void DrawStdRect2( int x0, int y0, int x1, int y1, int GP )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, &P2_NORMAL, GP );
}

__declspec( dllexport ) void DrawStdBar2( int x0, int y0, int x1, int y1, int GP )
{
	GP_OPT = 1023;
	DrawFilledRect1( x0, y0, x1, y1, &P2_NORMAL, GP, 11, 3 );
}

void DrawStdBar12( int x0, int y0, int x1, int y1, int GP )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, y1, &P2_NORMAL, GP, 11, 3 );
}

int BordGP = -1;

void DrawStdHeader( int xc, int yc, int Lx )
{
	GP_OPT = 1023 + 1024;
	DrawHeader( xc, yc, Lx, BordGP, 25 );
}

void DrawStdRect( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, &StdRect, BordGP );
}

void DrawStdBar( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023;
	DrawFilledRect1( x0, y0, x1, y1, &StdRect, BordGP, 28, 3 );
}

void DrawStdBar1( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, y1, &StdRect, BordGP, 28, 3 );
}

void DrawLeftRect( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, &LeftRect, BordGP );
}

void DrawRightRect( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, &RightRect, BordGP );
}

void DrawUpRect( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, &UpRect, BordGP );
}

void DrawDnRect( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, y1, &DnRect, BordGP );
}

void DrawTable( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, y1, &Table, BordGP, 28, 3 );
	DrawHeader( ( x0 + x1 ) >> 1, y0, 150, BordGP, 25 );
}

void DrawDoubleTable( int x0, int y0, int x1, int y1, int ym )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, ym, &UpTable, BordGP, 28, 3 );
	DrawFilledRect1( x0, ym, x1, y1, &DnTable, BordGP, 28, 3 );
	//DrawHeader((x0+x1)>>1,y0,150,BordGP,25);
}

void DrawDoubleTable2( int x0, int y0, int x1, int y1, int ym )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, ym, &UpTable3, BordGP, 28, 3 );
	DrawFilledRect1( x0, ym, x1, y1, &DnTable3, BordGP, 28, 3 );
	//DrawHeader((x0+x1)>>1,y0,150,BordGP,25);
}

void DrawScrollDoubleTable( int x0, int y0, int x1, int y1, int ym )
{
	GP_OPT = 1023 + 1024;
	DrawRect4( x0, y0, x1, ym, &UpTable, BordGP );
	DrawFilledRect1( x0, ym, x1, y1, &DnTable1, BordGP, 28, 3 );
	//DrawHeader((x0+x1)>>1,y0,150,BordGP,25);
}

void DrawScrollDoubleTableF( int x0, int y0, int x1, int y1, int ym )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, ym, &UpTable, BordGP, 28, 3 );
	DrawFilledRect1( x0, ym, x1, y1, &DnTable1, BordGP, 28, 3 );
	//DrawHeader((x0+x1)>>1,y0,150,BordGP,25);
}

void DrawScrollDoubleTable1( int x0, int y0, int x1, int y1, int ym )
{
	GP_OPT = 1023 + 1024;
	DrawFilledRect1( x0, y0, x1, ym, &UpTable1, BordGP, 28, 3 );
	DrawRect4( x0, ym, x1, y1, &DnTable2, BordGP );
	//DrawHeader((x0+x1)>>1,y0,150,BordGP,25);
}

void DrawPaperPanel( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023;
	DrawFilledRect1( x0, y0, x1, y1, &Paper, BordGP, 51, 3 );
}

void DrawPaperPanelShadow( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1024;
	DrawRect4( x0, y0, x1, y1, &Paper, BordGP );
}

void CBar( int x, int y, int Lx, int Ly, byte c );

void DrawHintPattern( int x0, int y0, int x1, int y1 )
{
	GP_OPT = 1023 + 1024;
	CBar( x0, y0, x1 - x0 + 1, y1 - y0 + 1, 0xB9 );
	DrawRect( x0, y0, x1, y1, &HintPan, BordGP );
	GPS.ShowGPLayers( ( x0 + x1 ) / 2 - 32, y0 - 32, BordGP, 66, 0, GP_OPT );
	GPS.ShowGPLayers( ( x0 + x1 ) / 2 - 32, y1 - 32, BordGP, 67, 0, GP_OPT );
}

//Fills background for pages 1 and 2 of stats screen
void DrawStatTable( int x0, int y0, int Lx, int GraphLy, int LineLy, int NLines )
{
	GP_OPT = 1023 + 1024;

	int y1 = y0 + GraphLy + LineLy * NLines;

	if (NLines)
	{
		//Borders, upgrade timelines backgrounds
		DrawFilledRect1( x0, y1 - LineLy, x0 + Lx - 1, y1, &ST_Bottom, BordGP, 28, 3 );
		y1 -= LineLy;
		for (int i = 1; i < NLines; i++)
		{
			DrawFilledRect1( x0, y1 - LineLy, x0 + Lx - 1, y1, &ST_Middle, BordGP, 28, 3 );
			y1 -= LineLy;
		}

		//Graph background
		CBar( x0, y0 + 18, Lx, y1 - y0 + 1 - 18, 0x38 );

		//Add distortion effect to graph background
		//DrawFilledRect1( x0, y0, x0 + Lx - 1, y1, &ST_Top, BordGP, 51, 1 );

		//Header caption background
		DrawRect4( x0, y0, x0 + Lx - 1, y1, &ST_Top, BordGP );
	}
}

//Fills background for page 3 of stats screen
void DrawStatTable1( int x0, int y0, int x1, int y1, int xm )
{
	DrawFilledRect1( x0, y0, xm, y1, &ST_Left, BordGP, 28, 3 );
	DrawFilledRect1( xm, y0, x1, y1, &ST_Right, BordGP, 28, 3 );
}

void DrawStHeader( int x, int y, int Lx )
{
	GP_OPT = 1023 + 1024;
	DrawHeader( x, y, Lx, BordGP, 71 );
}

void LoadBorders()
{
	BordGP = GPS.PreLoadGPImage( "Border", 0 );
	BordGP2 = GPS.PreLoadGPImage( "Interface\\Bor2", 0 );
}
