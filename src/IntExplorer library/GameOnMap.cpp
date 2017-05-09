#include "../Main executable/common.h"
#include "IntExplorer.h"
#include "ParseRQ.h"
#include <crtdbg.h>
#include <assert.h>
#include "bmptool.h"
#include "GameonMap.h"
OneMap::OneMap()
{
	memset( this, 0, sizeof *this );
};

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
__declspec( dllimport ) byte GetPaletteColor( int r, int g, int b );
__declspec( dllimport ) void GetPalColor( byte idx, byte* r, byte* g, byte* b );
void SendSmartRequest( sicExplorer* SXP, char* Str );
bool ReadWinStringF( FILE* F, char* STR, int Max )
{
	STR[0] = 0;
	int cc = 0;
	//int z1=0;
	int nn = 0;
	while (!( cc == 0x0A || cc == EOF ))
	{
		cc = fgetc( F );
		if (Max > 2 && !( cc == 0x0A || cc == EOF ))
		{
			STR[nn] = (char) cc;
			nn++;
			Max--;
		};
	};
	STR[nn] = 0;
	return ( cc != EOF ) || STR[0];
	//DosToWin(STR);
};
void OneMap::LoadMapData( sicExplorer* SXP )
{
	//loading colors
	char ccc[64];
	sprintf( ccc, "Internet\\Cash\\%s.colors", Title );
	FILE* F = fopen( ccc, "r" );
	if (F)
	{
		int z;
		int idx = 0;
		do
		{
			int r0, g0, b0;
			z = fscanf( F, "%d%d%d", &r0, &g0, &b0 );
			if (z == 3)
			{
				sprintf( ccc, "Internet\\Cash\\colors_%d_%d_%d.dat", r0, g0, b0 );
				ResFile F = RReset( ccc );
				COLORS[idx].r = (byte) r0;
				COLORS[idx].g = (byte) g0;
				COLORS[idx].b = (byte) b0;
				CORIG[idx] = GetPaletteColor( r0, g0, b0 );
				if (F != INVALID_HANDLE_VALUE)
				{
					RBlockRead( F, ColorTBL + ( idx << 8 ), 256 );
					RClose( F );
				}
				else
				{
					F = RRewrite( ccc );
					if (F != INVALID_HANDLE_VALUE)
					{
						for (int i = 0; i < 255; i++)
						{
							byte r, g, b;
							GetPalColor( (byte) i, &r, &g, &b );
							int v = ( int( r ) * 228 + int( g ) * 450 + int( b ) * 58 ) / 768;
							if (v > 255)v = 255;
							ColorTBL[( idx << 8 ) + i] = GetPaletteColor( ( v*r0 ) / 255, ( v*g0 ) / 255, ( v*b0 ) / 255 );
						};
						RBlockWrite( F, ColorTBL + ( idx << 8 ), 256 );
						RClose( F );
					};
				};
			};
			idx++;
		} while (z == 3 && idx < 255);
		COLORS[255].r = 255;
		COLORS[255].g = 255;
		COLORS[255].b = 255;
		fclose( F );
		for (int i = 0; i < 256; i++)
		{
			ColorTBL[255 * 256 + i] = (byte) i;
		}
	};
	//loading set of maps
	int i;
	for (i = 0; i <= 100; i++)
	{
		sprintf( ccc, "Internet\\Cash\\%s_%d.bmp", Title, i );
		BMPformat BM;
		byte* Data;
		if (ReadBMP8( ccc, &BM, &Data ))
		{
			SCALE = (OneScale*) realloc( SCALE, ( NScales + 1 ) * sizeof OneScale );
			SCALE[NScales].Data = Data;
			SCALE[NScales].Loaded = 1;
			SCALE[NScales].Lx = BM.biWidth;
			SCALE[NScales].Ly = BM.biHeight;
			SCALE[NScales].Scale = i;
			SCALE[NScales].Colored = 0;
			SCALE[NScales].ColoredData = NULL;
			SCALE[NScales].FONT[0] = 0;
			NScales++;
		};
	};
	sprintf( ccc, "Internet\\Cash\\%s.fonts", Title );
	F = fopen( ccc, "r" );
	if (F)
	{
		for (int i = 0; i < NScales; i++)
		{
			int z = fscanf( F, "%s", SCALE[i].FONT );
			if (z)
			{
				SCALE[i].Font = NULL;
				SCALE[i].Font = SXP->GetFontByName( SCALE[i].FONT );
			};
		};
	};
	//loading base countries distribution
	sprintf( ccc, "Internet\\Cash\\%s_nations.bmp", Title );
	BMPformat BM;
	byte* Data;
	memset( XSumm, 0, sizeof XSumm );
	memset( YSumm, 0, sizeof YSumm );
	memset( NPoints, 0, sizeof NPoints );

	if (ReadBMP8( ccc, &BM, &Data ))
	{
		CountryMap = Data;
		CountryLx = BM.biWidth;
		CountryLy = BM.biHeight;
		int ofs = 0;
		int MAXLX[256];
		int CY[256];
		int CX[256];
		memset( MAXLX, 0, sizeof MAXLX );
		byte prevc = 255;
		int cx = 0;
		int cx1 = 0;
		int CLX = 0;
		for (int iy = 0; iy < CountryLy - 7; iy++)
		{
			for (int ix = 0; ix < CountryLx; ix++)
			{
				byte c = Data[ofs];
				byte c1 = Data[ofs + CountryLx];
				byte c2 = Data[ofs + CountryLx * 2];
				byte c3 = Data[ofs + CountryLx * 3];
				byte c4 = Data[ofs + CountryLx * 4];
				byte c5 = Data[ofs + CountryLx * 5];
				byte c6 = Data[ofs + CountryLx * 6];
				byte c7 = Data[ofs + CountryLx * 7];
				if (!( c == c1&&c == c2&&c == c3&&c == c4&&c == c5&&c == c6&&c == c7 ))c = 255;
				if (c == prevc&&c != 255)
				{
					CLX++;
					cx1 = ix;
				}
				else
				{
					if (prevc != 255)
					{
						if (MAXLX[prevc] < CLX)
						{
							MAXLX[prevc] = CLX;
							CX[prevc] = ( cx + cx1 ) >> 1;
							CY[prevc] = iy + 4;
						};
					};
					prevc = c;
					cx = ix;
					cx1 = ix;
					CLX = 1;
				};
				ofs++;
			};
		};
		for (i = 0; i < 256; i++)if (MAXLX[i])
		{
			XSumm[i] = CX[i];
			YSumm[i] = CY[i];
			NPoints[i] = 1;
		};
	};
	UpdateData = 0;
	sprintf( ccc, "Internet\\Cash\\%s.date", Title );
	F = fopen( ccc, "r" );
	if (F)
	{
		fscanf( F, "%u", &UpdateData );
		fclose( F );
		sprintf( ccc, "GW|mupdate&%s\\00&%d\\00", Title, UpdateData );
		SendSmartRequest( SXP, ccc );
	};
	sprintf( ccc, "Internet\\Cash\\%s.nations", Title );
	F = fopen( ccc, "r" );
	memset( Names, 0, sizeof Names );
	if (F)
	{
		char ccc[256];
		int p = 0;
		while (ReadWinStringF( F, ccc, 255 ))
		{
			Names[p] = (char*) malloc( strlen( ccc ) + 1 );
			strcpy( Names[p], ccc );
			p++;
		};
	};
};
#define VTYPE4
void OneMap::ApplyColorsToScaleLayer( int Idx )
{
	if (!CountryMap)return;
	if (Idx >= 0 && Idx < NScales)
	{
		OneScale* OSC = SCALE + Idx;
		if (!OSC->ColoredData)
		{
			OSC->ColoredData = (byte*) malloc( OSC->Lx*OSC->Ly );
			int SCDX = ( CountryLx * 65536 ) / OSC->Lx;
			int SCDY = ( CountryLy * 65536 ) / OSC->Ly;
			int NX = OSC->Lx;
			int NY = OSC->Ly;
			byte* CMAP = CountryMap;
			byte* CDT = OSC->ColoredData;
			byte* SData = OSC->Data;
			byte* COLTBL = ColorTBL;
			int CLX = CountryLx;
			int RY = 0;
			int ofs = 0;
			for (int iy = 0; iy < NY; iy++)
			{
				int RX = 0;
				int RRY = RY >> 16;
				for (int ix = 0; ix < NX; ix++)
				{
					int RRX = RX >> 16;
					int MOFF = RRX + RRY*CLX;
					byte CC = CMAP[MOFF];
#ifdef VTYPE1
					if (CC == 255/*||(iy+ix)&1*/)
					{
						CDT[ofs] = SData[ofs];
					}
					else
					{
						CDT[ofs] = CORIG[CC];//COLTBL[SData[ofs]+(CC<<8)];
					};
#endif
#ifdef VTYPE2
					if (CC == 255 || ( iy + ix ) & 1)
					{
						CDT[ofs] = SData[ofs];
					}
					else
					{
						CDT[ofs] = CORIG[CC];//COLTBL[SData[ofs]+(CC<<8)];
					};
#endif
#ifdef VTYPE3
					if (ix > 3 && iy > 3 && ix < NX - 3 && iy < NY - 3)
					{
						CDT[ofs] = SData[ofs];
						if (CC != 255)
						{
							if (CC != CMAP[( ( RX - SCDX ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[( ( RX + SCDX ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[RRX + ( ( RY - SCDY ) >> 16 )*CLX] ||
								CC != CMAP[RRX + ( ( RY + SCDY ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX - SCDX2 ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[( ( RX + SCDX2 ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[RRX + ( ( RY - SCDY2 ) >> 16 )*CLX] ||
								CC != CMAP[RRX + ( ( RY + SCDY2 ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX - SCDX3 ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[( ( RX + SCDX3 ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[RRX + ( ( RY - SCDY3 ) >> 16 )*CLX] ||
								CC != CMAP[RRX + ( ( RY + SCDY3 ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX - SCDX4 ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[( ( RX + SCDX4 ) >> 16 ) + RRY*CLX] ||
								CC != CMAP[RRX + ( ( RY - SCDY4 ) >> 16 )*CLX] ||
								CC != CMAP[RRX + ( ( RY + SCDY4 ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX - SCDX ) >> 16 ) + ( ( RY - SCDX ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX + SCDX ) >> 16 ) + ( ( RY - SCDX ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX - SCDX ) >> 16 ) + ( ( RY + SCDX ) >> 16 )*CLX] ||
								CC != CMAP[( ( RX + SCDX ) >> 16 ) + ( ( RY + SCDX ) >> 16 )*CLX])
							{
								CDT[ofs] = CORIG[CC];
							};
						};
					};
#endif
#ifdef VTYPE4
					if (CC == 255/*||(iy+ix)&1*/)
					{
						CDT[ofs] = SData[ofs];
					}
					else
					{
						CDT[ofs] = COLTBL[SData[ofs] + ( CC << 8 )];
					};
#endif
					ofs++;
					RX += SCDX;
				};
				RY += SCDY;
			};
		};
	};
};
void OneMap::ShowMapPart()
{
	if (CurScale < 0 || CurScale >= NScales)return;
	OneScale* OSC = SCALE + CurScale;
	OneScale* MOSC = SCALE + NScales - 1;
	if (!OSC->ColoredData)ApplyColorsToScaleLayer( CurScale );
	if (!OSC->ColoredData)return;
	NormalizeCoordinates();
	int SX = WX;
	int SY = WY;
	int SX1 = WX + CurLx - 1;
	int SY1 = WY + CurLy - 1;
	int X0 = CurX;
	int Y0 = CurY;
	int X1 = CurX + CurLx - 1;
	int Y1 = CurY + CurLy - 1;
	if (SX < WindX)
	{
		X0 += WindX - SX;
		SX = WindX;
	};
	if (SY < WindY)
	{
		Y0 += WindY - SY;
		SY = WindY;
	};
	if (SX1 > WindX1)
	{
		X1 -= SX1 - WindX1;
		SX1 = WindX1;
	};
	if (SY1 > WindY1)
	{
		Y1 -= SY1 - WindY1;
		SY1 = WindY1;
	};
	if (SX > SX1 || SY > SY1)return;

	int SRCWIDTH = OSC->Lx;
	int SRCLX = X1 - X0 + 1;
	byte* SRC = OSC->ColoredData + X0 + Y0*SRCWIDTH;

	int DSTWIDTH = ScrWidth;
	byte* DST = (byte*) ScreenPtr + SX + SY*DSTWIDTH;
	for (int i = SY; i <= SY1; i++)
	{
		memcpy( DST, SRC, SRCLX );
		SRC += SRCWIDTH;
		DST += DSTWIDTH;
	};
	int SCAL = OSC->Lx * 100 / MOSC->Lx;
	int LX = SCALE[CurScale].Lx;
	int LY = SCALE[CurScale].Ly;
	TempWindow TW;
	PushWindow( &TW );
	IntersectWindows( WX, WY, WX + WLX - 1, WY + WLY - 1 );
	for (int i = 0; i < NMapPix; i++)
	{
		OneMapPicture* OMP = MapPix + i;
		int x = WX + ( OMP->x*LX / CountryLx ) - CurX;
		int y = WY + ( OMP->x*LY / CountryLy ) - CurY;
		int BID = -1;
		int DS = 100;
		for (int j = 0; j < OMP->NScales; j++)
		{
			int D = abs( OMP->SCALE[j] - SCAL );
			if (D < DS)
			{
				DS = D;
				BID = OMP->GPID[j];
			};
		};
		OMP->Drawn = 0;
		if (BID != -1)
		{
			GPS.ShowGP( x - OMP->dx, y - OMP->dy, BID, OMP->Sprite, 0 );
			OMP->Drawn = 1;
			OMP->LastX = x - OMP->dx;
			OMP->LastY = y - OMP->dy;
			OMP->LastGPID = BID;
		};
	};
	//show text
	RLCFont* FNT = SCALE[CurScale].Font;
	if (FNT)
	{
		int H = GetRLCHeight( FNT->RLC, 'W' );
		for (int i = 0; i < 255; i++)if (NPoints[i] && Names[i])
		{
			int L = GetRLCStrWidth( Names[i], FNT );
			int x = WX + ( XSumm[i] * LX / CountryLx ) - CurX;
			int y = WY + ( YSumm[i] * LY / CountryLy ) - CurY;
			ShowString( x - ( L / 2 ), y - ( H / 2 ), Names[i], FNT );
		};
	};
	PopWindow( &TW );
};
void OneMap::SetCenterCoor( int x, int y )
{
	VCenterX = x;
	VCenterY = y;
	NormalizeCoordinates();
};
void OneMap::RelativeMove( int dx, int dy )
{
	if (!NScales)return;
	OneScale* CUR = SCALE + CurScale;
	OneScale* MAX = SCALE + NScales - 1;
	VCenterX += ( MAX->Lx*dx ) / CUR->Lx;
	VCenterY += ( MAX->Ly*dy ) / CUR->Lx;
	NormalizeCoordinates();
};
void OneMap::ChangeScale( int Direction )
{
	if (!NScales)return;
	CurScale += Direction;
	NormalizeCoordinates();
};
void OneMap::NormalizeCoordinates()
{
	if (!NScales)return;
	if (CurScale >= NScales)CurScale = NScales - 1;
	if (CurScale < 0)CurScale = 0;
	OneScale* MAX = SCALE + NScales - 1;
	OneScale* OSC = SCALE + CurScale;
	CurX = ( VCenterX*OSC->Lx / MAX->Lx ) - WLX / 2;
	CurY = ( VCenterY*OSC->Ly / MAX->Ly ) - WLY / 2;
	if (CurX < 0)
	{
		CurX = 0;
		VCenterX = WLX*MAX->Lx / OSC->Lx / 2;
	};
	if (CurY < 0)
	{
		CurY = 0;
		VCenterY = WLY*MAX->Ly / OSC->Ly / 2;
	};
	CurLx = WLX;
	CurLy = WLY;
	if (CurX + CurLx >= OSC->Lx)
	{
		CurX = OSC->Lx - CurLx;
		if (CurX < 0)
		{
			CurX = 0;
			CurLx = OSC->Lx;
		};
		VCenterX = ( CurX + WLX / 2 )*MAX->Lx / OSC->Lx;
	};
	if (CurY + CurLy >= OSC->Ly)
	{
		CurY = OSC->Ly - CurLy;
		if (CurY < 0)
		{
			CurY = 0;
			CurLy = OSC->Ly;
		};
		VCenterY = ( CurY + WLY / 2 )*MAX->Ly / OSC->Ly;
	};
};
void OneMap::ClearAll()
{
	for (int i = 0; i < NScales; i++)
	{
		OneScale* OSC = SCALE + i;
		if (OSC->Data)free( OSC->Data );
		if (OSC->ColoredData)free( OSC->ColoredData );
	};
	if (SCALE)free( SCALE );
	if (MapPix)free( MapPix );
	if (CountryMap)free( CountryMap );
	for (int i = 0; i < 255; i++)if (Names[i])free( Names[i] );
	memset( this, 0, sizeof *this );
};
OneMap::~OneMap()
{
	ClearAll();
};
//LW_mapdata&bindata
//bindata:
//DWORD StartUpdateData
//DWORD CurrentUpdateData
//DWORD NPoints
//for every point:
//WORD  x
//WORD  y
//BYTE  color 
int NPT = 0;
int NUP = 0;
void OneMap::UpdateMapData( byte* Date )
{
	NUP++;
	DWORD StartDate = *( (DWORD*) Date );
	Date += 4;
	DWORD CurrentDate = *( (DWORD*) Date );
	Date += 4;
	//FILE* F1=fopen("Internet\\Cash\\map.log","a");
	//fprintf(F1,"UPDATE: %d->%d (TOTAL: pts: %d  upd: %d)\n",StartDate,CurrentDate,NPT,NUP);
	if (UpdateData == StartDate)
	{
		char ccc[256];
		sprintf( ccc, "Internet\\Cash\\%s.date", Title );
		FILE* F = fopen( ccc, "w" );
		if (F)
		{
			fprintf( F, "%u", CurrentDate );
			fclose( F );
			UpdateData = CurrentDate;
			int NPt = *( (DWORD*) Date );
			Date += 4;
			int i;
			for (i = 0; i < NPt; i++)
			{
				int x = *( (word*) Date );
				Date += 2;
				int y = *( (word*) Date );
				Date += 2;
				byte c = *Date;
				Date++;
				if (x < CountryLx&&y < CountryLy)
				{
					CountryMap[x + y*CountryLx] = c;
				};
				NPT++;
			};
			sprintf( ccc, "Internet\\Cash\\%s_nations.bmp", Title );
			SaveToBMP8( ccc, CountryLx, CountryLy, CountryMap, COLORS );
		};
		for (int i = 0; i < NScales; i++)
		{
			if (SCALE[i].ColoredData)free( SCALE[i].ColoredData );
			SCALE[i].ColoredData = NULL;
			SCALE[i].Colored = 0;
		};
	};
};
OneMap BIGMAP;
void OneMap::AddMapPicture( char* Name, int x, int y, int dx, int dy, int Sprite )
{
	if (NMapPix >= MaxMapPix)
	{
		MaxMapPix += 16;
		MapPix = (OneMapPicture*) realloc( MapPix, MaxMapPix * sizeof OneMapPicture );
	};
	OneMapPicture* OMP = MapPix + NMapPix;
	memset( OMP, 0, sizeof OneMapPicture );
	OMP->x = x;
	OMP->y = y;
	OMP->dx = dx;
	OMP->dy = dy;
	OMP->Sprite = Sprite;
	int NF = 0;
	char cc[128];
	sprintf( cc, "Internet\\Cash\\%s_*.gp", Name );
	int L = strlen( Name );
	WIN32_FIND_DATA FD;
	HANDLE H = FindFirstFile( cc, &FD );
	if (H != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf( cc, "Internet\\Cash\\%s", FD.cFileName );
			cc[strlen( cc ) - 3] = 0;
			int scale = atoi( FD.cFileName + L + 1 );
			int GPI = GPS.PreLoadGPImage( cc );
			if (GPI != -1)
			{
				GPS.LoadGP( GPI );
				if (Sprite < GPS.GPNFrames[GPI])
				{
					OMP->SCALE[NF] = scale;
					OMP->GPID[NF] = GPI;
					NF++;
					OMP->NScales = NF;
				};
			};
		} while (FindNextFile( H, &FD ) && NF < 15);
	};
	if (NF)NMapPix++;
};
void OneMap::ClearMapPictures()
{
	NMapPix = 0;
};