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
#include "VirtScreen.h"
#include "NewMon.h"
#include "Path.h"
#include "RealWater.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "3DMapEd.h"
#include "mode.h"
#include "Sort.h"
#include "Curve.h"
#include "Process.h"
#include "3DRandmap.h"
#include "bmptool.h"
#include "GP_Draw.h"
#include "DrawForm.h"
#include "Fonts.h"

int MinLinkDist = 9;
int StartHillHeight = 90;
int MaxHillHeight = 110;
int HillWidth = 250;
void Save3DMap( char* Map );
void InitHillAmp1();
extern short randoma[8192];
int mrpos = 0;
word XXP = 0;

int mrand()
{
	mrpos++;
	if ( mrpos > 8191 )
	{
		XXP += 0x3571;
	}
	mrpos &= 8191;
	return ( randoma[mrpos] ^ XXP ) & 32767;
}

void CheckFirstLine()
{
	int N = VertInLine + VertInLine;
	for ( int i = 0; i < N; i++ )if ( THMap[i] < 0 )THMap[i] = 0;
}

struct GenArea
{
	short x;
	short y;
	byte State;
	byte Nation;
	byte Zone;
	byte Usage;
	byte Type;
	word  NLinks;
	word* Link;
	word MaxLink;
};

PaintHills PHILL[1];

class GenMap
{
public:
	word*  GTopRef;//[TopLx*TopLy];
	GenArea* GTopMap;
	int   NGAreas;
	int   MaxGArea;
	byte*  VertHi;//[MaxPointIndex];
	byte*  VertType;//[MaxPointIndex];
	int SIZE;
	void CreateNet();
	bool AddGArea( int x, int y );
	void AddGLink( int N1, int N2 );
	void SetStateAround( int N, byte State );
	GenMap();
	~GenMap();
	void Clear();
	void Show( int x, int y );
	void MakeLinearHill( int N1, int N2 );
	void MakeSoftArea( int N1, int N2 );
	void MakeHillSpot( int x, int y, int r0, byte* Height );
	bool CheckWebPoint( int i );
	bool GenerateHillLink( int N );
	bool GenerateHillLink( int N, int Thick );
	void Allocate();
	void Free();
};

void GenMap::Allocate()
{
	GTopRef = new word[TopLx*TopLy];
	VertHi = new byte[MaxPointIndex];
	VertType = new byte[MaxPointIndex];
	SIZE = MaxPointIndex;
}

void GenMap::Free()
{
	free( GTopRef );
	free( VertHi );
	free( VertType );
	VertHi = NULL;
	VertType = NULL;
	GTopRef = NULL;
}

void GenMap::SetStateAround( int N, byte State )
{
	GenArea* GA = GTopMap + N;
	for ( int p = 0; p < GA->NLinks; p++ )GTopMap[GA->Link[p + p]].State = State;
}

bool GenMap::AddGArea( int x, int y )
{
	for ( int i = 0; i < NGAreas; i++ )
	{
		GenArea* GAR = GTopMap + i;
		if ( Norma( GAR->x - x, GAR->y - y ) < MinLinkDist )return false;
	};
	if ( NGAreas >= MaxGArea )
	{
		MaxGArea += 256;
		GTopMap = (GenArea*) realloc( GTopMap, MaxGArea * sizeof GenArea );
	};
	GenArea* GAR = GTopMap + NGAreas;
	GAR->x = x;
	GAR->y = y;
	GAR->Link = NULL;
	GAR->MaxLink = 0;
	GAR->NLinks = 0;
	GAR->State = 0;
	GAR->Nation = 0xFF;
	GAR->Zone = 0xFF;
	GAR->Type = 0xFF;
	GAR->Usage = 0;
	NGAreas++;
	return true;
}

void GenMap::AddGLink( int N1, int N2 )
{
	GenArea* AR = &GTopMap[N1];
	int N = AR->NLinks;
	for ( int i = 0; i < N; i++ )
	{
		if ( AR->Link[i + i] == N2 )return;
	};
	if ( AR->NLinks == AR->MaxLink )
	{
		AR->MaxLink += 4;
		AR->Link = (word*) realloc( AR->Link, AR->MaxLink << 2 );
	};
	N = AR->NLinks;
	AR->Link[N + N] = N2;
	int dst = Norma( AR->x - GTopMap[N2].x, AR->y - GTopMap[N2].y );
	//assert(dst<40);
	AR->Link[N + N + 1] = dst;
	AR->NLinks++;
};
void CheckNet( GenMap* GM )
{
	for ( int i = 0; i < GM->NGAreas; i++ )
	{
		word* LINK = GM->GTopMap[i].Link;
		int NL = GM->GTopMap[i].NLinks;
		int x = GM->GTopMap[i].x;
		int y = GM->GTopMap[i].y;
		for ( int j = 0; j < NL; j++ )
		{
			int id = LINK[j + j];
			int ds = LINK[j + j + 1];
			GenArea* GA1 = GM->GTopMap + id;
			int dst = Norma( GA1->x - x, GA1->y - y );
			//assert(dst==ds&&dst<40);
		};
	};
};
word GetDir( int, int );
bool CheckDeep( int x, int y )
{
	int x1 = x << 1;
	int y1 = y << 1;
	if ( x > 0 && x < msx&&y>0 && y < msx )return WaterDeep[x1 + ( y << MapShift )] > 128;
	return false;
};
void GenMap::CreateNet()
{
	memset( GTopRef, 0xFF, TopLx*TopLy * 2 );
	int LX = 240 << ( ADDSH - 1 );
	int LL = ( LX - 1 ) << 1;
	int LY = 240 << ( ADDSH - 1 );
	int mmx = LX;
	int mmy = LY;
	for ( int y = 1; y < LY; y++ )
	{
		memset( GTopRef + 1 + y*TopLx, 0xEE, LL );
	};
	GTopMap = NULL;
	MaxGArea = 0;
	NGAreas = 0;
	int N3 = 0;
	int L3 = LX - 10;
	do
	{
		int x = 5 + ( ( int( mrand() )*L3 ) >> 15 );
		int y = 5 + ( ( int( mrand() )*L3 ) >> 15 );
		if ( AddGArea( x, y ) )N3 = 0;
		else N3++;
	} while ( N3 < 50 );

	for ( int i = 0; i < NGAreas; i++ )GTopRef[GTopMap[i].x + int( GTopMap[i].y )*TopLx] = i;
	bool Change = false;

	Change = true;
	for ( int i = 1; i < RRad && Change; i++ )
	{
		//memcpy(TempTop,TopRef,sizeof(TopRef));
		Change = false;
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for ( int j = 0; j < NGAreas; j++ )
		{
			int xx = GTopMap[j].x;
			int yy = GTopMap[j].y;
			for ( int k = 0; k < N; k++ )
			{
				int x = xx + xi[k];
				int y = yy + yi[k];
				if ( x > 0 && y > 0 && x < mmx&&y < mmy )
				{
					int ofst = x + y*TopLx;
					word ZZ = GTopRef[ofst];
					word ZU = GTopRef[ofst - TopLx];
					word ZD = GTopRef[ofst + TopLx];
					word ZL = GTopRef[ofst - 1];
					word ZR = GTopRef[ofst + 1];
					if ( ZZ == 0xEEEE )
					{
						if ( ZU == j || ZD == j || ZL == j || ZR == j ||
							GTopRef[ofst - TopLx - 1] == j ||
							GTopRef[ofst - TopLx + 1] == j ||
							GTopRef[ofst + TopLx - 1] == j ||
							GTopRef[ofst + TopLx + 1] == j )
						{
							GTopRef[ofst] = j;
							Change = true;
							ZZ = j;
						};
					};
					if ( ZZ < 0xEEEE )
					{
						if ( ZU != ZZ&&ZU < 0xEEEE )
						{
							AddGLink( ZZ, ZU );
							AddGLink( ZU, ZZ );
						};
						if ( ZD != ZZ&&ZD < 0xEEEE )
						{
							AddGLink( ZZ, ZD );
							AddGLink( ZD, ZZ );
						};
						if ( ZL != ZZ&&ZL < 0xEEEE )
						{
							AddGLink( ZZ, ZL );
							AddGLink( ZL, ZZ );
						};
						if ( ZR != ZZ&&ZR < 0xEEEE )
						{
							AddGLink( ZZ, ZR );
							AddGLink( ZR, ZZ );
						};
					};
				};
			};
		};
	};
	CheckNet( this );
	//sorting links
	for ( int i = 0; i < NGAreas; i++ )
	{
		GenArea* GA = GTopMap + i;
		int x0 = GA->x;
		int y0 = GA->y;
		UNISORT.CheckSize( GA->NLinks );
		int Nga = GA->NLinks;
		for ( int p = 0; p < Nga; p++ )
		{
			word LI = GA->Link[p + p];
			GenArea* GA1 = GTopMap + LI;
			byte Dir = byte( GetDir( GA1->x - x0, GA1->y - y0 ) );
			UNISORT.Uids[p] = LI;
			UNISORT.Parms[p] = Dir;
		}

		UNISORT.NUids = Nga;
		UNISORT.Sort();
		byte CDr = 0;
		for ( int p = 0; p < Nga; p++ )
		{
			word LI = UNISORT.Uids[p];
			GenArea* GA2 = GTopMap + LI;
			GA->Link[p + p] = LI;
			GA->Link[p + p + 1] = Norma( GA->x - GA2->x, GA->y - GA2->y );
			int dx = GA2->x - x0;
			int dy = GA2->y - y0;
			if ( dx > 0 )CDr |= 1;
			if ( dx < 0 )CDr |= 2;
			if ( dy > 0 )CDr |= 4;
			if ( dy < 0 )CDr |= 8;

			if ( dx > 0 && dy > 0 )CDr |= 16;
			if ( dx < 0 && dy>0 )CDr |= 32;
			if ( dx > 0 && dy < 0 )CDr |= 64;
			if ( dx < 0 && dy < 0 )CDr |= 128;
		};
		int x = GA->x;
		int y = GA->y;
		if ( CDr != 255 )
		{
			if ( x<20 || y<20 || x>LX - 20 || y>LX - 20 )GA->State = 1;
		};
		if ( CheckDeep( x, y ) )GA->State = 2;
		else if ( CheckDeep( x - 2, y ) || CheckDeep( x - 4, y ) || CheckDeep( x + 2, y ) || CheckDeep( x + 4, y ) ||
			CheckDeep( x, y - 2 ) || CheckDeep( x, y - 4 ) || CheckDeep( x, y + 2 ) || CheckDeep( x, y + 4 ) ||
			CheckDeep( x - 3, y - 3 ) || CheckDeep( x - 3, y + 3 ) || CheckDeep( x + 3, y - 3 ) || CheckDeep( x + 3, y + 3 ) )
			GA->State = 2;



	}

	CheckNet( this );
}

void ErrM( char* s );

void InvHill( char* Name )
{
	char ccc[128];
	sprintf( ccc, "Invalid file: %s", Name );
	ErrM( ccc );
}

bool LoadTerrain( char* Name )
{
	GFILE* F = Gopen( Name, "r" );
	if ( F )
	{
		int x, y, z, N;
		char str[128];
		z = Gscanf( F, "%d", &N );
		PHILL[0].NHiSections = N;
		for ( int p = 0; p < N; p++ )
		{
			z = Gscanf( F, "%d", &PHILL[0].HiSect[p] );
		};
		z = Gscanf( F, "%s%d", str, &N );
		if ( z != 2 || strcmp( str, "NLayers" ) )
		{
			InvHill( Name );
		};
		PHILL[0].NLayers = N;
		Gscanf( F, "%s", str );
		for ( int i = 0; i < N; i++ )
		{
			z = Gscanf( F, "%s%d", str, &x );
			if ( z != 2 || strcmp( str, "TexAmount" ) )InvHill( Name );
			PHILL[0].TexAmount[i] = x;
			for ( int j = 0; j < x; j++ )
			{
				z = Gscanf( F, "%d", &y );
				if ( z != 1 )InvHill( Name );
				if ( y < 0 )PHILL[0].Texs[i][j] = 128 - y;
				else PHILL[0].Texs[i][j] = y;
			};
			z = Gscanf( F, "%s%d%d", str, &x, &y );
			if ( z != 3 )InvHill( Name );
			PHILL[0].TexStartHi[i] = x;
			PHILL[0].TexEndHi[i] = y;
			z = Gscanf( F, "%s%d%d", str, &x, &y );
			if ( z != 3 )InvHill( Name );
			PHILL[0].TexStartAng[i] = x;
			PHILL[0].TexEndAng[i] = y;
			z = Gscanf( F, "%s", str );
			if ( str[0] == '#' )
			{
				switch ( str[1] )
				{
				case 'L':
					PHILL[0].CondType[i] = 1;
					PHILL[0].Param1[i] = str[2] - '0';
					PHILL[0].Param2[i] = str[3] - '0';
					break;
				case 'U':
					PHILL[0].CondType[i] = 2;
					PHILL[0].Param1[i] = str[2] - '0';
					PHILL[0].Param2[i] = str[3] - '0';
					break;
				};
				Gscanf( F, "%s", str );
			}
		}

		Gclose( F );
		return true;
	}

	return false;
}

void LoadRandomHills()
{
	int NAttm = 100;
	do
	{
		char ccc[64];
		sprintf( ccc, "Terrain\\Hills%d.dat", mrand() & 7 );
		if ( LoadTerrain( ccc ) )
		{
			return;
		}
		NAttm--;
	} while ( NAttm );
}

GenMap::GenMap()
{
	NGAreas = 0;
	MaxGArea = 0;
	GTopMap = NULL;
	//memset(VertHi,0,sizeof VertHi);
	//memset(VertType,0,sizeof VertType);
	//CreateNet();
	memset( PHILL, 0, sizeof PHILL[0] );
	PHILL[0].NLayers = 11;
	//-----------------------------
	PHILL[0].TexAmount[0] = 4;

	PHILL[0].Texs[0][0] = 56 + 128;
	PHILL[0].Texs[0][1] = 57 + 128;
	PHILL[0].Texs[0][2] = 58 + 128;
	PHILL[0].Texs[0][3] = 59 + 128;

	PHILL[0].TexStartHi[0] = 0;
	PHILL[0].TexEndHi[0] = 10;
	PHILL[0].TexStartAng[0] = 0;
	PHILL[0].TexEndAng[0] = 1024;
	//-----------------------------
	PHILL[0].TexAmount[1] = 4;

	PHILL[0].Texs[1][0] = 60 + 128;
	PHILL[0].Texs[1][1] = 61 + 128;
	PHILL[0].Texs[1][2] = 62 + 128;
	PHILL[0].Texs[1][3] = 63 + 128;

	PHILL[0].TexStartHi[1] = 10;
	PHILL[0].TexEndHi[1] = 100;
	PHILL[0].TexStartAng[1] = 40;
	PHILL[0].TexEndAng[1] = 1024;
	//-----------------------------
	PHILL[0].TexAmount[2] = 1;

	PHILL[0].Texs[2][0] = 64 + 128;

	PHILL[0].TexStartHi[2] = 100;
	PHILL[0].TexEndHi[2] = 126;
	PHILL[0].TexStartAng[2] = 25;
	PHILL[0].TexEndAng[2] = 300;
	//-----------------------------
	PHILL[0].TexAmount[3] = 3;

	PHILL[0].Texs[3][0] = 85 + 128;
	PHILL[0].Texs[3][1] = 86 + 128;
	PHILL[0].Texs[3][2] = 87 + 128;

	PHILL[0].TexStartHi[3] = 127;
	PHILL[0].TexEndHi[3] = 129;
	PHILL[0].TexStartAng[3] = 39;
	PHILL[0].TexEndAng[3] = 1024;
	//-----------------------------
	PHILL[0].TexAmount[4] = 2;

	PHILL[0].Texs[4][0] = 4;
	PHILL[0].Texs[4][1] = 11;

	PHILL[0].TexStartHi[4] = 127;
	PHILL[0].TexEndHi[4] = 129;
	PHILL[0].TexStartAng[4] = 0;
	PHILL[0].TexEndAng[4] = 99;
	//-----------------------------
	PHILL[0].TexAmount[5] = 2;

	PHILL[0].Texs[5][0] = 11;
	PHILL[0].Texs[5][1] = 4;

	PHILL[0].TexStartHi[5] = 110;
	PHILL[0].TexEndHi[5] = 127;
	PHILL[0].TexStartAng[5] = 0;
	PHILL[0].TexEndAng[5] = 99;
	//-----------------------------
	PHILL[0].TexAmount[6] = 1;

	PHILL[0].Texs[6][0] = 4;

	PHILL[0].TexStartHi[6] = 90;
	PHILL[0].TexEndHi[6] = 110;
	PHILL[0].TexStartAng[6] = 0;
	PHILL[0].TexEndAng[6] = 99;
	//-----------------------------
	PHILL[0].TexAmount[7] = 2;

	PHILL[0].Texs[7][0] = 4;
	PHILL[0].Texs[7][1] = 5;

	PHILL[0].TexStartHi[7] = 70;
	PHILL[0].TexEndHi[7] = 90;
	PHILL[0].TexStartAng[7] = 0;
	PHILL[0].TexEndAng[7] = 99;
	//-----------------------------
	PHILL[0].TexAmount[8] = 1;

	PHILL[0].Texs[8][0] = 5;

	PHILL[0].TexStartHi[8] = 55;
	PHILL[0].TexEndHi[8] = 70;
	PHILL[0].TexStartAng[8] = 0;
	PHILL[0].TexEndAng[8] = 99;
	//-----------------------------
	PHILL[0].TexAmount[9] = 2;

	PHILL[0].Texs[9][0] = 5;
	PHILL[0].Texs[9][1] = 6;

	PHILL[0].TexStartHi[9] = 45;
	PHILL[0].TexEndHi[9] = 55;
	PHILL[0].TexStartAng[9] = 0;
	PHILL[0].TexEndAng[9] = 99;
	//-----------------------------
	PHILL[0].TexAmount[10] = 2;

	PHILL[0].Texs[10][0] = 6;
	PHILL[0].Texs[10][1] = 0;

	PHILL[0].TexStartHi[10] = 30;
	PHILL[0].TexEndHi[10] = 45;
	PHILL[0].TexStartAng[10] = 0;
	PHILL[0].TexEndAng[10] = 99;

};
//-------------------------------Drawing Linear segment------------------------------//
struct BiSegment
{
	double x0;
	double y0;
	double x1;
	double y1;
	double Dx;
	double Dy;
	double R0;
	double Ang0;
	double MaxPhase;
};
#define PI 3.14159265358979
void InitBiSegment( BiSegment* BS, int x0, int y0, int x1, int y1 )
{
	BS->x0 = x0;
	BS->y0 = y0;
	BS->x1 = x1;
	BS->y1 = y1;
	BS->Dx = x1 - x0;
	BS->Dy = y1 - y0;
	BS->R0 = sqrt( BS->Dx*BS->Dx + BS->Dy*BS->Dy );
	BS->Ang0 = atan2( BS->Dy, BS->Dx );
};

double GetBiFunction( double x, double y, double* Phase, BiSegment* BS )
{

	double Pdx = x - BS->x0;
	double Pdy = y - BS->y0;
	double R = BS->R0;
	double Dx = BS->Dx;
	double Dy = BS->Dy;
	double R2 = 2 * R;
	double R1 = sqrt( Pdx*Pdx + Pdy*Pdy );
	double scl = ( Pdx*Dx + Pdy*Dy );
	if ( R1 )scl /= R;
	else scl = 0;
	double Rd, phase;
	if ( scl <= 0 )
	{
		double Ang = atan2( y - BS->y0, x - BS->x0 ) - BS->Ang0;
		if ( Ang < 0 )Ang += PI * 2;
		if ( Ang > PI * 2 )Ang -= PI * 2;
		Rd = R1;
		phase = ( R2 + Rd*( PI / 2 + Ang ) );
	}
	else
		if ( scl > R )
		{
			double Ang = atan2( y - BS->y1, x - BS->x1 ) - BS->Ang0 + PI;
			if ( Ang < 0 )Ang += PI * 2;
			if ( Ang > PI * 2 )Ang -= PI * 2;
			Pdx = x - BS->x1;
			Pdy = y - BS->y1;
			Rd = sqrt( Pdx*Pdx + Pdy*Pdy );
			phase = ( R + ( Ang - PI / 2 )*Rd );
		}
		else
		{
			Rd = ( Dy*Pdx - Dx*Pdy ) / R;
			if ( Rd > 0 )
			{
				phase = scl;
			}
			else
			{
				Rd = -Rd;
				phase = ( R2 + Rd*PI - scl );
			};
		};
	double L = 2 * ( PI*Rd + R );
	phase = ( phase + Rd*PI / 2 ) / L;
	if ( phase > 1 )phase -= 1;
	*Phase = phase;
	return Rd;
};

//-----------------------------------------------------------------------------------//
void GenMap::Clear()
{
	for ( int p = 0; p < NGAreas; p++ )
	{
		GenArea* GA = GTopMap + p;
		if ( GA->MaxLink )free( GA->Link );
	};
	if ( GTopMap )
	{
		free( GTopMap );
		GTopMap = NULL;
	};
	MaxGArea = 0;
	NGAreas = 0;
	if ( VertHi )memset( VertHi, 0, SIZE );
	if ( VertType )memset( VertType, 0, SIZE );
}

GenMap::~GenMap()
{
	Clear();
}

void GenMap::Show( int x, int y )
{
	for ( int i = 0; i < NGAreas; i++ )
	{
		GenArea* GA = GTopMap + i;
		int x0 = GA->x;
		int y0 = GA->y;
		for ( int j = 0; j < GA->NLinks; j++ )
		{
			GenArea* GA1 = GTopMap + GA->Link[j + j];
			DrawLine( x0 + x, y0 + y, GA1->x + x, GA1->y + y, clrRed );
		}
		if ( GA->State == 1 )
		{
			DrawLine( x0 - 2, y0, x0 + 2, y0, clrYello );
			DrawLine( x0, y0 - 2, x0, y0 + 2, clrYello );
		}
		else
		{
			if ( GA->State == 2 )
			{
				DrawLine( x0 - 2, y0, x0 + 2, y0, 0xF9 );
				DrawLine( x0, y0 - 2, x0, y0 + 2, 0xF9 );
			}
			else
			{
				DrawLine( x0 - 2, y0, x0 + 2, y0, 255 );
				DrawLine( x0, y0 - 2, x0, y0 + 2, 255 );
			}
		}
	}
}

void InterpolateCurve();
void UnLockLine( int x0, int y0, int x1, int y1 )
{
	int N = Norma( x1 - x0, y1 - y0 ) + 1;
	x0 <<= 16;
	y0 <<= 16;
	x1 <<= 16;
	y1 <<= 16;
	int r = Norma( x1 - x0, y1 - y0 );
	int dx = ( x1 - x0 ) / N;
	int dy = ( y1 - y0 ) / N;
	int x = x0;
	int y = y0;
	for ( int i = 0; i < N; i++ )
	{
		int xs = x >> 16;
		int ys = y >> 16;
		AddUnLockbar( xs, ys );

		AddUnLockbar( xs + 1, ys );
		AddUnLockbar( xs - 1, ys );
		AddUnLockbar( xs, ys + 1 );
		AddUnLockbar( xs, ys - 1 );

		AddUnLockbar( xs + 1, ys + 1 );
		AddUnLockbar( xs - 1, ys + 1 );
		AddUnLockbar( xs + 1, ys - 1 );
		AddUnLockbar( xs - 1, ys - 1 );
		x += dx;
		y += dy;
	};
};
void GenMap::MakeSoftArea( int N1, int N2 )
{
	//return;
	GenArea* GA1 = GTopMap + N1;
	GenArea* GA2 = GTopMap + N2;
	//search for common link
	int CL1 = -1;
	int CL2 = -1;
	for ( int i = 0; i < GA1->NLinks; i++ )
	{
		if ( GA1->Link[i + i] == N2 )CL1 = i;
	};
	for ( int i = 0; i < GA2->NLinks; i++ )
	{
		if ( GA2->Link[i + i] == N1 )CL2 = i;
	};
	if ( CL1 == -1 || CL2 == -1 )return;
	//enumerating key points
	int NKey = 0;
	int KeyX[32];
	int KeyY[32];
	int x0 = GA1->x;
	int y0 = GA1->y;
	GA1->State = 2;
	GA2->State = 2;
	for ( int i = 0; i < GA1->NLinks; i++ )
	{
		if ( i != CL1 )
		{
			word LI = GA1->Link[i + i];
			GenArea* GA3 = GTopMap + LI;
			int xa1 = ( x0 + GA3->x * 2 ) / 3;
			int ya1 = ( y0 + GA3->y * 2 ) / 3;
			KeyX[NKey] = xa1;
			KeyY[NKey] = ya1;
			NKey++;
		};
	};
	x0 = GA2->x;
	y0 = GA2->y;
	for ( int i = 0; i < GA2->NLinks; i++ )
	{
		if ( i != CL2 )
		{
			word LI = GA2->Link[i + i];
			GenArea* GA3 = GTopMap + LI;
			int xa1 = ( x0 + GA3->x * 2 ) / 3;
			int ya1 = ( y0 + GA3->y * 2 ) / 3;
			KeyX[NKey] = xa1;
			KeyY[NKey] = ya1;
			NKey++;
		};
	};
	//sorting key points by angle
	x0 = ( GA1->x + GA2->x ) >> 1;
	y0 = ( GA1->y + GA2->y ) >> 1;
	UNISORT.CheckSize( NKey );
	for ( int i = 0; i < NKey; i++ )
	{
		UNISORT.Uids[i] = i;
		UNISORT.Parms[i] = GetDir( KeyX[i] - x0, KeyY[i] - y0 );
	};
	UNISORT.NUids = NKey;
	UNISORT.Sort();
	ClearCurve();
	x0 = GA1->x;
	y0 = GA1->y;
	NKey = 0;
	for ( int i = 0; i < GA1->NLinks; i++ )
	{
		if ( i != CL1 )
		{
			word LI = GA1->Link[i + i];
			GenArea* GA3 = GTopMap + LI;
			int xa1 = ( x0 * 2 + GA3->x ) / 3;
			int ya1 = ( y0 * 2 + GA3->y ) / 3;
			KeyX[NKey] = xa1;
			KeyY[NKey] = ya1;
			NKey++;
		};
	};
	x0 = GA2->x;
	y0 = GA2->y;
	for ( int i = 0; i < GA2->NLinks; i++ )
	{
		if ( i != CL2 )
		{
			word LI = GA2->Link[i + i];
			GenArea* GA3 = GTopMap + LI;
			int xa1 = ( x0 + GA3->x * 3 ) / 4;
			int ya1 = ( y0 + GA3->y * 3 ) / 4;
			KeyX[NKey] = xa1;
			KeyY[NKey] = ya1;
			NKey++;
		};
	};
	for ( int i = 0; i < NKey; i++ )
	{
		int idx = UNISORT.Uids[i];
		AddPointToCurve( KeyX[idx] << 6, KeyY[idx] << 6, false, 0 );
	};
	int idx = UNISORT.Uids[0];
	AddPointToCurve( KeyX[idx] << 6, KeyY[idx] << 6, false, 0 );
	//InterpolateCurve();
	SurfType = 1;
	//if(rand()<16384)SurfType=2;
	//PlatoHi=128;
	SoftRegion( VertHi );
	UnLockLine( GA1->x, GA1->y, GA2->x, GA2->y );
	ClearCurve();
};
void InitHillAmp();
double GetHillHi( double r, double phase, double r0, double Width );
int SqInt( byte* REND, int x, int y );
void GenMap::MakeHillSpot( int x, int y, int r0, byte* Height )
{
	r0 <<= ADDSH - 1;
	int rrx = ( r0 + 320 ) >> 5;
	int x0 = ( x >> 5 ) - rrx;
	int y0 = ( y >> 5 ) - rrx;
	int x1 = ( x >> 5 ) + rrx;
	int y1 = ( y >> 5 ) + rrx;

	if ( x0 < 0 )x0 = 0;
	if ( y0 < 0 )y0 = 0;
	if ( x1 >= VertInLine )x1 = VertInLine - 1;
	if ( y1 >= MaxTH )y1 = MaxTH - 1;

	InitHillAmp1();
	HillWidth = 300;
	double RR = r0;
	for ( int vx = x0; vx < x1; vx++ )
	{
		for ( int vy = y0; vy < y1; vy++ )
		{
			int v = vx + VertInLine*vy;
			int xv = GetTriX( v );
			int yv = GetTriY( v );
			int H = SqInt( Height, xv, yv );
			double phase = atan2( xv - x, yv - y ) / 3.14159265;
			if ( phase < 0 )phase += 1;
			if ( phase > 1 )phase -= 1;

			double r = sqrt( ( x - xv )*( x - xv ) + ( y - yv )*( y - yv ) );
			int Wi = ( HillWidth*H ) >> 12;
			if ( Wi < 120 )Wi = 120;
			double f = GetHillHi( r, phase, RR, Wi );
			int h = int( 128.0*f );
			int Hi = int( double( H )*f*180.0 / 4096.0 );
			if ( vx == x0 || vy == y0 || vx == x1 - 1 || vy == y1 - 1 )
			{
				//assert(Hi==0);
			};
			if ( h )
			{
				VertType[v] = 1;
				if ( VertHi[v] < h )VertHi[v] = h;
				if ( THMap[v] < Hi )THMap[v] = Hi;
			}
			else VertType[v] = 1;
		};
	};
};
int GetFractalVal( int x, int y );
bool UseFractal = 0;
void GenMap::MakeLinearHill( int N1, int N2 )
{
	GenArea* GA1 = GTopMap + N1;
	GenArea* GA2 = GTopMap + N2;
	//search for common link
	int CL1 = -1;
	int CL2 = -1;
	for ( int i = 0; i < GA1->NLinks; i++ )
	{
		if ( GA1->Link[i + i] == N2 )CL1 = i;
	};
	for ( int i = 0; i < GA2->NLinks; i++ )
	{
		if ( GA2->Link[i + i] == N1 )CL2 = i;
	};
	if ( CL1 == -1 || CL2 == -1 )return;
	int MinR = 0;
	int NS = 0;
	for ( int p = 0; p < GA1->NLinks; p++ )
	{
		int r = GA1->Link[p + p + 1];
		MinR += r;
		NS++;
	};
	for ( int p = 0; p < GA2->NLinks; p++ )
	{
		int r = GA2->Link[p + p + 1];
		MinR += r;
		NS++;
	};
	MinR /= NS;
	BiSegment BS;

	InitBiSegment( &BS, int( GA1->x ) << 6, int( GA1->y ) << 6, int( GA2->x ) << 6, int( GA2->y ) << 6 );

	int x0 = ( int( GA1->x ) << 1 );
	int y0 = ( int( GA1->y ) << 1 );
	int x1 = ( int( GA2->x ) << 1 );
	int y1 = ( int( GA2->y ) << 1 );
	if ( x0 > x1 )
	{
		x0 += x1;
		x1 = x0 - x1;
		x0 = x0 - x1;
	};
	if ( y0 > y1 )
	{
		y0 += y1;
		y1 = y0 - y1;
		y0 = y0 - y1;
	};
	x0 -= MinR + 20;
	y0 -= MinR + 20;
	x1 += MinR + 20;
	y1 += MinR + 20;

	if ( x0 < 0 )x0 = 0;
	if ( y0 < 0 )y0 = 0;
	if ( x1 >= VertInLine )x1 = VertInLine - 1;
	if ( y1 >= MaxTH )y1 = MaxTH - 1;

	int r0 = MinR << 5;
	r0 = ( r0 * 14 ) >> 4;
	InitHillAmp();
	for ( int vx = x0; vx < x1; vx++ )
	{
		for ( int vy = y0; vy < y1; vy++ )
		{
			int v = vx + VertInLine*vy;
			int x = GetTriX( v );
			int y = GetTriY( v );
			double phase;
			double r = GetBiFunction( x, y, &phase, &BS );
			double f = GetHillHi( r, phase, r0, HillWidth );
			if ( f < 0 )f = 0;
			int h;//=int(128.0*f);
			if ( UseFractal )
			{
				int v = GetFractalVal( x, y ) + 256;
				f = 1;
				f = ( f*v ) / 512;
				double re = r / 650;
				double f0 = 0;
				if ( re > 15 )f = 0;
				else
				{
					f0 = exp( -re*re ) - 0.05;
					if ( f0 < 0 )f0 = 0;
					f *= f0;
					//f0=f;
				};
				//h=int(120.0*f0);
				h = int( double( PlatoHi )*f ) >> 1;
			}
			else h = int( 128.0*f );
			int Hi = int( double( PlatoHi )*f );
			if ( vx == x0 || vy == y0 || vx == x1 - 1 || vy == y1 - 1 )
			{
				//assert(Hi==0);
			};
			if ( h )
			{
				VertType[v] = 1;
				if ( VertHi[v] < h )VertHi[v] = h;
				if ( THMap[v] < Hi )THMap[v] = Hi;
			}
			else VertType[v] = 1;
		};
	};
	GA1->State = 2;
	GA2->State = 2;
	/*
	//enumerating key points
	int NKey=0;
	int KeyX[32];
	int KeyY[32];
	int x0=GA1->x;
	int y0=GA1->y;
	GA1->State=2;
	GA2->State=2;
	for(i=0;i<GA1->NLinks;i++){
		if(i!=CL1){
			word LI=GA1->Link[i+i];
			GenArea* GA3=GTopMap+LI;
			int xa1=(x0+GA3->x*2)/3;
			int ya1=(y0+GA3->y*2)/3;
			KeyX[NKey]=xa1;
			KeyY[NKey]=ya1;
			NKey++;
		};
	};
	x0=GA2->x;
	y0=GA2->y;
	for(i=0;i<GA2->NLinks;i++){
		if(i!=CL2){
			word LI=GA2->Link[i+i];
			GenArea* GA3=GTopMap+LI;
			int xa1=(x0+GA3->x*2)/3;
			int ya1=(y0+GA3->y*2)/3;
			KeyX[NKey]=xa1;
			KeyY[NKey]=ya1;
			NKey++;
		};
	};
	//sorting key points by angle
	x0=(GA1->x+GA2->x)>>1;
	y0=(GA1->y+GA2->y)>>1;
	UNISORT.CheckSize(NKey);
	for(i=0;i<NKey;i++){
		UNISORT.Uids[i]=i;
		UNISORT.Parms[i]=GetDir(KeyX[i]-x0,KeyY[i]-y0);
	};
	UNISORT.NUids=NKey;
	UNISORT.Sort();
	ClearCurve();
	x0=GA1->x;
	y0=GA1->y;
	NKey=0;
	for(i=0;i<GA1->NLinks;i++){
		if(i!=CL1){
			word LI=GA1->Link[i+i];
			GenArea* GA3=GTopMap+LI;
			int xa1=(x0+GA3->x)>>1;
			int ya1=(y0+GA3->y)>>1;
			KeyX[NKey]=xa1;
			KeyY[NKey]=ya1;
			NKey++;
		};
	};
	x0=GA2->x;
	y0=GA2->y;
	for(i=0;i<GA2->NLinks;i++){
		if(i!=CL2){
			word LI=GA2->Link[i+i];
			GenArea* GA3=GTopMap+LI;
			int xa1=(x0+GA3->x)>>1;
			int ya1=(y0+GA3->y)>>1;
			KeyX[NKey]=xa1;
			KeyY[NKey]=ya1;
			NKey++;
		};
	};
	for(i=0;i<NKey;i++){
		int idx=UNISORT.Uids[i];
		AddPointToCurve(KeyX[idx]<<6,KeyY[idx]<<6,false,0);
	};
	int idx=UNISORT.Uids[0];
	AddPointToCurve(KeyX[idx]<<6,KeyY[idx]<<6,false,0);
	InterpolateCurve();
	SurfType=1;
	if(rand()<16384)SurfType=2;
	//PlatoHi=128;
	SetPlatoInCurve(VertHi,VertType,1);
	ClearCurve();
	*/
};
bool GenMap::CheckWebPoint( int i )
{
	GenArea* GA = GTopMap + i;
	if ( GA->State )return false;
	for ( int j = 0; j < GA->NLinks; j++ )
	{
		GenArea* GA1 = GTopMap + GA->Link[j + j];
		if ( GA1->State > 1 )return false;
	};
	return true;
};
bool GenMap::GenerateHillLink( int N )
{
	return GenerateHillLink( N, 0 );
};
bool GenMap::GenerateHillLink( int N, int Thick )
{
	int Hills[96];
	int NHills = 0;
	do
	{
		int idx = ( int( mrand() )*NGAreas ) >> 15;
		if ( CheckWebPoint( idx ) )
		{
			Hills[0] = idx;
			NHills = 1;

			for ( int k = 0; k < N; k++ )
			{
				int NAttm = 0;
				int NextLink = -1;
				for ( int p = 0; p < NHills - 1; p++ )
				{
					GTopMap[Hills[p]].State += 16;
				};
				GenArea* GA0 = GTopMap + idx;
				int nLink = GA0->NLinks;
				do
				{
					int L = ( int( mrand() )*nLink ) >> 15;
					int idx1 = GA0->Link[L + L];
					if ( CheckWebPoint( idx1 ) )
					{
						NextLink = idx1;
					};
					NAttm++;
				} while ( NAttm < 15 && NextLink == -1 );
				for ( int p = 0; p < NHills - 1; p++ )
				{
					GTopMap[Hills[p]].State -= 16;
				};
				if ( NextLink != -1 )
				{
					Hills[NHills] = NextLink;
					NHills++;
					idx = NextLink;
				}
				else return false;
			};
			for ( int k = 1; k < NHills; k++ )
			{
				PlatoHi = StartHillHeight + ( ( int( mrand() )*( MaxHillHeight - StartHillHeight ) ) >> 15 );
				MakeLinearHill( Hills[k], Hills[k - 1] );
			};
			int np = 4;
			if ( Thick )np = 7;
			//expanding Hills
			for ( int k = 0; k < NHills; k++ )
			{
				GTopMap[Hills[k]].State = 1;
			};
			for ( int p = 0; p < Thick; p++ )
			{
				int NH = NHills;
				for ( int m = 0; m < NH; m++ )
				{
					GenArea* GA = GTopMap + Hills[m];
					for ( int q = 0; q < GA->NLinks; q++ )
					{
						int idx = GA->Link[q + q];
						if ( CheckWebPoint( idx ) )
						{
							Hills[NHills] = idx;
							MakeLinearHill( Hills[m], idx );
							GTopMap[idx].State = 1;
							GA->State = 1;
							NHills++;
						};
					};
				};
			};
			for ( int k = 0; k < NHills; k++ )
			{
				GTopMap[Hills[k]].State = 2;
			};
			if ( !UseFractal )
			{
				for ( int k = 0; k < np; k++ )
				{
					int hil = ( int( mrand() )*NHills ) >> 15;
					int nattm = 0;
					int aidx = Hills[hil];
					GenArea* GA4 = GTopMap + aidx;
					int nLink = GA4->NLinks;
					int GX4 = GA4->x;
					int GY4 = GA4->y;
					do
					{
						int LI = ( int( mrand() )*nLink ) >> 15;
						int mm = GA4->Link[LI + LI];
						GenArea* GA5 = GTopMap + mm;
						if ( GA5->State < 2 )
						{
							int GX5 = GA5->x;
							int GY5 = GA5->y;
							if ( GetHeight( ( GX5 << 6 ) + 32, ( GY5 << 6 ) + 32 ) < 3 )
							{
								MakeSoftArea( aidx, mm );
								GA5->State = 2;
								nattm = 10000;
							};
						};
						nattm++;
					} while ( nattm < 1000 );
				};
			};
			for ( int k = 0; k < NHills; k++ )
			{
				SetStateAround( Hills[k], 2 );
			};


			return true;
		};
		N++;
	} while ( N < 10 );
	return false;
};
GenMap GMAP;
void ShowNatDeal();

//calls ShowNatDeal()
void GenShow()
{
	ShowNatDeal();
};

#define NATLX (TopLx>>1)
#define NATSH (TopSH-1)
byte* NatDeals;//[NATLX*NATLX];
double HillR[256];
double HillW[256];
#define NFUR 10
void InitHillAmp()
{
	double HR[NFUR];
	double PH[NFUR];
	double HRW[NFUR];
	double PHW[NFUR];
	for ( int i = 0; i < NFUR; i++ )
	{
		HR[i] = double( mrand() ) / 32768.0 / ( i + 4 );
		PH[i] = double( mrand() ) / 10768.0;
		HRW[i] = double( mrand() ) / 32768.0 / ( i + 4 );
		PHW[i] = double( mrand() ) / 10768.0;
	};
	double smax = 0;
	double smin = 0;
	double smaxw = 0;
	double sminw = 0;
	for ( int i = 0; i < 256; i++ )
	{
		double Ang = ( PI*i ) / 128;
		double s = 0;
		for ( int j = 0; j < NFUR; j++ )s += sin( ( j + 4 )*Ang + PH[j] )*HR[j];
		if ( s > smax )smax = s;
		if ( s < smin )smin = s;
		HillR[i] = s;
		s = 0;
		for ( int j = 0; j < NFUR; j++ )s += sin( ( j + 4 )*Ang + PH[j] )*HR[j];
		if ( s > smaxw )smaxw = s;
		if ( s < sminw )sminw = s;
		HillW[i] = s;
	};
	if ( smax != smin )
	{
		double amp = smax - smin;
		double ampw = smaxw - sminw;
		for ( int i = 0; i < 256; i++ )
		{
			HillR[i] = ( HillR[i] - smin ) / amp;
			HillW[i] = 0.8 + ( ( HillW[i] - sminw )*0.4 / ampw );
		};
	};
};
void InitHillAmp1()
{
	double HR[NFUR];
	double PH[NFUR];
	double HRW[NFUR];
	double PHW[NFUR];
	int NFR = 5;
	for ( int i = 0; i < NFR; i++ )
	{
		HR[i] = double( mrand() ) / 32768.0 / ( i + 2 );
		PH[i] = double( mrand() ) / 10768.0;
		HRW[i] = double( mrand() ) / 32768.0 / ( i + 2 );
		PHW[i] = double( mrand() ) / 10768.0;
	};
	double smax = 0;
	double smin = 0;
	double smaxw = 0;
	double sminw = 0;
	for ( int i = 0; i < 256; i++ )
	{
		double Ang = ( PI*i ) / 128;
		double s = 0;
		for ( int j = 0; j < NFR; j++ )s += sin( ( j + 2 )*Ang + PH[j] )*HR[j];
		if ( s > smax )smax = s;
		if ( s < smin )smin = s;
		HillR[i] = s;
		s = 0;
		for ( int j = 0; j < NFR; j++ )s += sin( ( j + 2 )*Ang + PH[j] )*HR[j];
		if ( s > smaxw )smaxw = s;
		if ( s < sminw )sminw = s;
		HillW[i] = s;
	};
	if ( smax != smin )
	{
		double amp = smax - smin;
		double ampw = smaxw - sminw;
		for ( int i = 0; i < 256; i++ )
		{
			HillR[i] = ( 1 + ( HillR[i] - smin ) / amp ) / 2;
			HillW[i] = 0.6 + ( ( HillW[i] - sminw )*0.4 / ampw );
		};
	};
};
//double HILLHI[32]={0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,
//                  0.00,0.01,0.02,0.11,0.21,0.27,0.35,0.41,
//				  0.50,0.64,0.82,0.93,0.98,1.00,1.00,1.00,
//				  1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00};
double HILLHI[32] = { 0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,
				  0.00,0.01,0.01,0.03,0.08,0.13,0.23,0.36,
				  0.50,0.64,0.78,0.88,0.94,0.97,1.00,1.00,
				  1.00,1.00,1.00,1.00,1.00,1.00,1.00,1.00 };

double GetHillHi( double r, double phase, double r0, double Width )
{
	int pidx = int( phase * 255 );
	if ( pidx < 0 )pidx = 0;
	if ( pidx > 255 )pidx = 255;
	double ang = PI * 2 * phase;
	double mul = 0.7 + HillR[pidx] * 0.6;
	Width *= HillW[pidx];
	//r*=mul;
	r0 *= mul;
	double dr = r - r0;
	if ( dr > Width )return 0;
	else if ( dr < -Width )return 1;
	int idx = int( 16 * ( 1 - dr / Width ) );
	if ( idx < 0 )idx = 0;
	if ( idx > 30 )idx = 30;

	return HILLHI[idx];
};

void CreateMiniMap();
void ClearRender();

void GenerateByStyle( char* Style );

bool CreateRandomTerrain( char* Name, int NPlay, int MountStyle, int Ng, int NI, int Nc, word* Units, int ADD_PARAM );

void GenerateHill( int Thick )
{
	int N = 0;
	do
	{
		if ( GMAP.GenerateHillLink( 4, Thick ) )
		{
			return;
		};
		N++;
	} while ( N < 5000 );
	N = 0;
	do
	{
		if ( GMAP.GenerateHillLink( 3, Thick ) )
		{
			return;
		};
		N++;
	} while ( N < 5000 );
	N = 0;
	do
	{
		if ( GMAP.GenerateHillLink( 2, Thick ) )
		{
			return;
		};
		N++;
	} while ( N < 1000 );
	do
	{
		if ( GMAP.GenerateHillLink( 1, Thick ) )
		{
			return;
		};
		N++;
	} while ( N < 1000 );
};
void LimitZones();
bool ProcessMessages();
void CreateLandLocking( int TAlp, bool ForVision );
void CreateFractal();
byte* REND;

void GenerateRelief( int Style )
{
	GMAP.Allocate();
	memset( THMap, 0, ( MaxTH + 1 )*MaxTH * 2 );
	GMAP.Clear();
	ProcessMessages();
	GMAP.CreateNet();
	LimitZones();
	int Thick, N, H;
	UseFractal = 0;
	switch ( Style )
	{
	case 0:
		Thick = 0;
		N = 3;
		H = 128;
		HillWidth = 250;
		break;
	case 1:
		Thick = 0;
		N = 8;
		H = 128;
		HillWidth = 250;
		break;
	case 2:
		CreateFractal();
		UseFractal = 1;
		Thick = 0;
		N = 8;
		H = 180;
		HillWidth = 360;
		break;
	case 3:
		Thick = 0;
		N = 28;
		H = 128;
		HillWidth = 250;
		break;
	default:
		Thick = 1;
		N = 28;
		H = 128;
		HillWidth = 250;
	};
	StartHillHeight = H;
	MaxHillHeight = H + 20;
	for ( int p = 0; p < N; p++ )
	{
		char cc[128];
		sprintf( cc, "Generating hill %d", p + 1 );
		SetNextInfStage( cc );
		GenerateHill( Thick );
		ProcessMessages();
	};
	int maxx = ( msx << 5 ) + 64;
	for ( int v = 0; v < MaxPointIndex; v++ )
	{
		int x = GetTriX( v );
		int y = GetTriY( v );
		if ( x > 0 && y > 0 && x < maxx&&y < maxx )
		{
			int VAL = SqInt( REND, x, y ) - 2048;
			if ( VAL < 0 )VAL = 0;
			THMap[v] = ( int( THMap[v] )*VAL ) >> 11;
			GMAP.VertHi[v] = ( int( GMAP.VertHi[v] )*VAL ) >> 11;
		};
	};
	SetNextInfStage( "Texturing" );
	MFIELDS[0].ClearMaps();
	CreateLandLocking( 120, false );
	ProcessMessages();
	PaintAllMap( GMAP.VertHi, GMAP.VertType, PHILL, 1 );
	ProcessMessages();
	GMAP.GTopRef = new word[TopLx*TopLy];
	GMAP.VertHi = new byte[MaxPointIndex];
	GMAP.VertType = new byte[MaxPointIndex];
	GMAP.Clear();
	//GMAP.CreateNet();
	//NatDeals=new byte[NATLX*NATLX];
	InitHillAmp();
	GMAP.Free();
};
void RandomMapDialog( char* Result );
extern int RES[8][8];
extern int NNations;
void CreateNationalMaskForRandomMap( char* Name )
{
	char cc2[32] = "00000000";
	word NatUnit[8] = { 0,0,0,0,0,0,0,0 };
	/*
	char cc1[128];
	int x1, x2;
	sscanf( Name, "%s%x%x%s", cc1, &x1, &x2, cc2 );
	*/
	sscanf( Name, "%*s %*s %*s %s", cc2 );//BUGFIX: proper parsing
	LOADNATMASK = 0;
	for ( int i = 0; i < 8; i++ )
	{
		char c = cc2[7 - i];
		int id = 0;
		if ( c >= '0'&&c <= '9' )
		{
			id = c - '0';
		}
		else
		{
			if ( c >= 'A'&&c <= 'Z' )
			{
				id = c - 'A' + 10;
			}
		}
		if ( id )
		{
			id--;
			char* pname = GlobalAI.Ai[id].Peasant_ID;
			int ntt = -1;
			for ( int j = 0; j < NNations; j++ )
			{
				if ( strstr( pname, NatCharLo[j] ) || strstr( pname, NatCharHi[j] ) )
				{
					ntt = j;
				}
			}
			if ( ntt != -1 )
			{
				LOADNATMASK |= 1 << ntt;
			}
		}
	}
}

extern int RM_LandType;
extern int RM_Resstart;
extern int RM_Restot;
char CurTerrName[48];
extern int ADDRES[8];
void SetupArrays();
void MakeDesert();

void CreateMapByName( char* Name )
{
	//Set additional bitshift for large and huge maps
	//1 = normal maps
	//2 = large (2x) maps
	//3 = huge (4x) maps
	ADDSH = Name[2] - '0' + 1;

	SetupArrays();

	char cc1[128];
	char cc2[32] = "00000000";
	word NatUnit[8] = { 0,0,0,0,0,0,0,0 };
	int x1, x2;
	int ADD_PARAM = 0;

	sscanf( Name, "%s%x%x%s%d", cc1, &x1, &x2, cc2, &ADD_PARAM );

	for ( int i = 0; i < 8; i++ )
	{
		char c = cc2[7 - i];
		int id = 0;
		if ( c >= '0' && c <= '9' )
		{
			id = c - '0';
		}
		else
		{
			if ( c >= 'A' && c <= 'Z' )
			{
				id = c - 'A' + 10;
			}
		}

		if ( id )
		{
			id--;
			char* pname = GlobalAI.Ai[id].Peasant_ID;
			int nu = NATIONS[i].NMon;
			GeneralObject** GOS = NATIONS[i].Mon;

			int k;
			for ( k = 0; k < nu && strcmp( GOS[k]->MonsterID, pname ); k++ );
			if ( k < nu )
			{
				NatUnit[i] = k;
			}
			else
			{
				NatUnit[k] = 0;
				sprintf( cc1, "Creating map: Unknown unit: %s", pname );
				ErrM( cc1 );
			}
		}
		else
		{
			NatUnit[i] = 0xFFFF;
		}
	}

	RandomMapDesc RD;
	RD.Load( "Terrain\\res.dat" );
	int Style = x2 & 15;
	int Play = ( x2 >> 4 ) & 15;
	int Mount = ( x2 >> 8 ) & 15;
	int Resst = ( x2 >> 12 ) & 15;
	int Resmn = ( x2 >> 16 ) & 15;
	for ( int i = 0; i < 8; i++ )
	{
		for ( int p = 0; p < 8; p++ )
		{
			RES[i][p] = RD.RES[Resst].RES[p];
		}
	}

	RM_LandType = Style;
	RM_Resstart = Resst;
	RM_Restot = Resmn;
	mrpos = x1 & 8191;
	XXP = 0;
	sprintf( cc1, "Terrain\\%s", RD.STY[Style].Players[Play].name );
	CurTerrName[0] = 0;

	GFILE* F = Gopen( "Terrain\\terrlist.dat", "r" );
	if ( F )
	{
		int n1, n2, z;
		char ccc[48];
		do
		{
			z = Gscanf( F, "%d%d%s", &n1, &n2, ccc );
			if ( z == 3 )
			{
				if ( n1 == ADDSH && n2 == Mount )
				{
					strcpy( CurTerrName, ccc );
				}
			}
		} while ( z == 3 );
		Gclose( F );
	}

	CreateRandomTerrain( cc1, RD.STY[Style].Players[Play].NPlayers,
		Mount, RD.MINES[Resmn].Ng, RD.MINES[Resmn].Ni,
		RD.MINES[Resmn].Nc, NatUnit, ADD_PARAM );

	if ( Mount == RD.NRelief - 1 )
	{
		MakeDesert();
	}

	for ( int i = 0; i < 8; i++ )
	{
		for ( int p = 0; p < 8; p++ )
		{
			RES[i][p] += ADDRES[p];
		}
	}
}

void CreateRandomMap( char* Name )
{
	GPROG.Setup();
	int S0 = 1 << ( ADDSH + ADDSH );
	GPROG.AddPosition( 'WATG', 30, 30 );
	GPROG.AddPosition( 'WBRI', 8 * S0, 1 );
	GPROG.AddPosition( 'COST', 3 * S0, 1 );
	GPROG.AddPosition( 'NTDL', 15 * S0, 1 );
	GPROG.AddPosition( 'STRS', 50, 1 );
	GPROG.AddPosition( 'TERR', 20 * S0, 1 );
	GPROG.AddPosition( 'FTEX', 5 * S0, 1 );
	GPROG.AddPosition( 'LINK', 10 * S0, 1 );
	GPROG.AddPosition( 'TRES', 60, 1 );
	GPROG.AddPosition( 'CORD', 10 * sqrt( S0 ), 1 );
	GPROG.AddPosition( 'SETH', 20 * S0, 1 );
	GPROG.AddPosition( 'NATR', 10 * S0, 1 );
	GPROG.AddPosition( 'SAVM', 30 * S0, 1 );
	GPROG.AddPosition( 'LOAD', 6 * S0, 1 );
	CreateMapByName( Name );
	ProcessMessages();
	GPROG.SetCurrentStage( 'SAVM' );
	ProcessMessages();
	Save3DMap( Name );
	GPROG.SetCurrentStage( 'LOAD' );
	ProcessMessages();
	GPROG.Setup();
}

void CreateNationalMaskForRandomMap( char* );

void UnLoading();

void CheckMapName( char* Name )
{
	if ( Name[0] == 'R' && Name[1] == 'N' && Name[3] == ' ' )
	{
		ResFile F = RReset( Name );
		if ( F == INVALID_HANDLE_VALUE )
		{
			UnLoading();

			CreateNationalMaskForRandomMap( Name );

			for ( int i = 0; i < 8; i++ )
			{
				LoadAllNations( i );
			}

			CreateRandomMap( Name );
			UnLoading();
		}
		else
		{
			RClose( F );
		}
	}
}

void CheckMapNameForStart( char* Name )
{
	if ( Name[0] == 'R' && Name[1] == 'N' && Name[3] == ' ' )
	{
		ResFile F = RReset( Name );
		if ( F == INVALID_HANDLE_VALUE )
		{
			UnLoading();
			CreateNationalMaskForRandomMap( Name );
			for ( int i = 0; i < 8; i++ )
			{
				LoadAllNations( i );
			}
			CreateRandomMap( Name );
		}
		else
		{
			RClose( F );
		}
	}
}

//----------------------------Undo---------------------------
class UndoSegment
{
public:
	byte* Data;
	int MaxData;
	int Size;
	int pos;
	UndoSegment();
	~UndoSegment();
	void Close();
	void Add( byte val );
	void Add( short val );
	void Add( int val );
	bool Read( byte* val );
	bool Read( short* val );
	bool Read( int* val );
};
UndoSegment::UndoSegment()
{
	Data = NULL;
	MaxData = 0;
	Size = 0;
	pos = 0;
};
void UndoSegment::Close()
{
	if ( Data )free( Data );
	Data = NULL;
	MaxData = 0;
	Size = 0;
	pos = 0;
};
UndoSegment::~UndoSegment()
{
	Close();
};
void UndoSegment::Add( byte val )
{
	if ( Size + 6 >= MaxData )
	{
		MaxData += 16384;
		Data = (byte*) realloc( Data, MaxData );
	};
	Data[Size] = val;
	Size++;
};
void UndoSegment::Add( short val )
{
	if ( Size + 6 >= MaxData )
	{
		MaxData += 16384;
		Data = (byte*) realloc( Data, MaxData );
	};
	( (short*) ( Data + Size ) )[0] = val;
	Size += 2;
};
void UndoSegment::Add( int val )
{
	if ( Size + 6 >= MaxData )
	{
		MaxData += 16384;
		Data = (byte*) realloc( Data, MaxData );
	};
	( (int*) ( Data + Size ) )[0] = val;
	Size += 4;
};
bool UndoSegment::Read( byte* val )
{
	if ( pos >= Size )return false;
	*val = Data[pos];
	pos++;
	return true;
};
bool UndoSegment::Read( short* val )
{
	if ( pos >= Size )return false;
	*val = ( (short*) ( Data + pos ) )[0];
	pos += 2;
	return true;
};
bool UndoSegment::Read( int* val )
{
	if ( pos >= Size )return false;
	*val = ( (int*) ( Data + pos ) )[0];
	pos += 4;
	return true;
};
UndoSegment UNDO1;
void SetTexture( int Vert, int nm );
void MarkPointToDraw( int i );
void EraseSprite( int Index );
void MakeUndo()
{
	bool DoUndo = true;
	while ( DoUndo )
	{
		byte Type;
		DoUndo = UNDO1.Read( &Type );
		if ( DoUndo )
		{
			int vint;
			byte vbyte;
			short vshort;
			switch ( Type )
			{
			case 0://sect
				if ( UNDO1.Read( &vint ) && UNDO1.Read( &vbyte ) )
				{
					if ( vint >= 0 && vint < MaxLineIndex )
					{
						if ( SectMap )SectMap[vint] = vbyte;
					};
				};
				break;
			case 1://height&texure
				if ( UNDO1.Read( &vint ) && UNDO1.Read( &vshort ) && UNDO1.Read( &vbyte ) )
				{
					SetTHMap( vint, vshort );
					SetTexMap( vint, vbyte );
					MarkPointToDraw( vint );
				};
				break;
			case 2://object;
			{
				short ID;
				UNDO1.Read( &ID );
				OneObject* OB = Group[ID];
				if ( OB )
				{
					OB->delay = 6000;
					if ( OB->LockType )OB->RealDir = 32;
					OB->Die();
					OB = Group[ID];
					if ( OB )OB->Sdoxlo = 2500;
				};
			};
			break;
			case 3://sprite
			{
				int Index;
				UNDO1.Read( &Index );
				EraseSprite( Index );
			};
			break;
			};
		};
	};
	UNDO1.Close();
}

void ChangeSect( int Sect, byte val )
{
	if ( Sect < 0 || Sect >= MaxLineIndex )
	{
		return;
	}

	if ( SECTMAP( Sect ) != val )
	{
		UNDO1.Add( byte( 0 ) );
		UNDO1.Add( Sect );
		UNDO1.Add( byte( SECTMAP( Sect ) ) );
	}
}

void ChangeVert( int Vert )
{
	if ( Vert < 0 || Vert >= MaxPointIndex )
	{
		return;
	}

	UNDO1.Add( byte( 1 ) );
	UNDO1.Add( Vert );
	UNDO1.Add( THMap[Vert] );
	UNDO1.Add( TexMap[Vert] );
}

void ChangeObject( short Index )
{
	UNDO1.Add( byte( 2 ) );
	UNDO1.Add( Index );
}

void ChangeSprite( int Index )
{
	UNDO1.Add( byte( 3 ) );
	UNDO1.Add( Index );
}

//--------------------Including samples----------------
//SMP-file format
//SMPF
//int Lx,Ly (pix)
//Blocks:
//int BlockID
//int size
//.....
//int -1

//vertex format:
//"VERT"
//int Size
//int NVert
//-------------------
//short dx
//short dy
//byte texture
//byte s1,s2,s3
//short Height
bool ImmVis = false;
bool AllowUndo = false;
bool CheckVertTex( int vx, int v, int H0 )
{
	if ( THMap[v] != H0 || TexMap[v] )return true;
	if ( vx & 1 )
	{
		return TexMap[v - VertInLine] || TexMap[v + VertInLine] || TexMap[v - 1] || TexMap[v + 1] ||
			TexMap[v - VertInLine + 1] || TexMap[v - VertInLine - 1];
	}
	else
	{
		return TexMap[v - VertInLine] || TexMap[v + VertInLine] || TexMap[v - 1] || TexMap[v + 1] ||
			TexMap[v + VertInLine + 1] || TexMap[v + VertInLine - 1];
	};
};
void RM_SaveVertices( ResFile F, int x0, int y0, int Lx, int Ly, int* KeyX, int* KeyY )
{
	int Vx0 = ( x0 - 128 ) / ( TriUnit + TriUnit );
	int Vx1 = ( x0 + Lx + 128 ) / ( TriUnit + TriUnit );
	int Vy0 = ( y0 - 256 ) / ( TriUnit );
	int Vy1 = ( y0 + Ly + 256 ) / ( TriUnit );
	int Nv = 0;
	int vxs = 0;
	int vys = 0;
	int NVER = 0;
	int HI0 = 4096;
	int Nvc = 0;
	for ( int ix = Vx0; ix < Vx1; ix++ )
	{
		for ( int iy = Vy0; iy < Vy1; iy++ )
		{
			if ( ix > 0 && ix < VertInLine - 1 && iy>0 && iy < MaxTH - 1 )
			{
				int v = ix + iy*VertInLine;
				int x = GetTriX( v );
				int y = ( GetTriY( v ) >> 1 ) - GetHi( v );
				if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
				{
					vxs += ix;
					vys += iy;
					Nvc++;
					if ( HI0 == 4096 )
					{
						HI0 = THMap[v];
					};
					if ( CheckVertTex( ix, v, HI0 ) )
					{
						Nv++;
					};
				};
			};
		};
	};
	if ( Nv )
	{
		vxs /= Nvc;
		vys /= Nvc;
		vxs &= ( 0xFFFFFFFE );
		vys &= ( 0xFFFFFFFE );
	}
	else return;
	int sign = 'TREV';
	RBlockWrite( F, &sign, 4 );
	sign = Nv * 10 + 12;
	RBlockWrite( F, &sign, 4 );
	RBlockWrite( F, &Nv, 4 );
	for ( int ix = Vx0; ix < Vx1; ix++ )
	{
		for ( int iy = Vy0; iy < Vy1; iy++ )
		{
			if ( ix > 0 && ix < VertInLine - 1 && iy>0 && iy < MaxTH - 1 )
			{
				int v = ix + iy*VertInLine;
				if ( CheckVertTex( ix, v, HI0 ) )
				{
					int x = GetTriX( v );
					int y = ( GetTriY( v ) >> 1 ) - GetHi( v );
					if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
					{
						short xx = ix - vxs;
						RBlockWrite( F, &xx, 2 );
						xx = iy - vys;
						RBlockWrite( F, &xx, 2 );
						RBlockWrite( F, TexMap + v, 1 );
						int StartSect = ( ix >> 1 ) * 6 + iy*SectInLine;
						byte s;
						if ( ix & 1 )
						{
							s = SECTMAP( StartSect + 3 );
							RBlockWrite( F, &s, 1 );
							s = SECTMAP( StartSect + 5 );
							RBlockWrite( F, &s, 1 );
							s = SECTMAP( StartSect + 2 );
							RBlockWrite( F, &s, 1 );
						}
						else
						{
							s = SECTMAP( StartSect );
							RBlockWrite( F, &s, 1 );
							s = SECTMAP( StartSect + 1 );
							RBlockWrite( F, &s, 1 );
							s = SECTMAP( StartSect - 2 );
							RBlockWrite( F, &s, 1 );
						};
						xx = THMap[v] - HI0;
						RBlockWrite( F, &xx, 2 );
					};
				};
			};
		};
	};
	*KeyX = vxs;
	*KeyY = vys;
};
void SetTexture( int Vert, int nm );
void MarkPointToDraw( int i );
bool CheckHI = 0;
void RM_LoadVertices( ResFile F, int Vsx, int Vsy )
{
	int Nv;
	bool NPOS = 0;
	bool NNEG = 0;
	RBlockRead( F, &Nv, 4 );
	int VSMX = 0;
	int VSMY = 0;
	for ( int i = 0; i < Nv; i++ )
	{
		short vx;
		short vy;
		byte tex;
		byte s1, s2, s3;
		short Height;
		RBlockRead( F, &vx, 2 );
		RBlockRead( F, &vy, 2 );
		VSMX += vx;
		VSMY += vy;
		vx += Vsx;
		vy += Vsy;
		RBlockRead( F, &tex, 1 );
		RBlockRead( F, &s1, 1 );
		RBlockRead( F, &s2, 1 );
		RBlockRead( F, &s3, 1 );
		RBlockRead( F, &Height, 2 );
		if ( Height > 0 )NPOS++;
		if ( Height < 0 )NNEG++;
		if ( vx > 0 && vy > 0 && vx < VertInLine - 1 && vy < MaxTH - 1 )
		{
			int StartSect = ( vx >> 1 ) * 6 + vy*SectInLine;
			int v = vx + vy*VertInLine;
			if ( vx & 1 )
			{
				if ( AllowUndo )
				{
					ChangeSect( StartSect + 3, s1 );
					ChangeSect( StartSect + 5, s2 );
					ChangeSect( StartSect + 2, s3 );
				};
				SetSectMap( StartSect + 3, s1 );
				SetSectMap( StartSect + 5, s2 );
				SetSectMap( StartSect + 2, s3 );
			}
			else
			{
				if ( AllowUndo )
				{
					ChangeSect( StartSect, s1 );
					ChangeSect( StartSect + 1, s2 );
					ChangeSect( StartSect - 2, s3 );
				};
				SetSectMap( StartSect, s1 );
				SetSectMap( StartSect + 1, s2 );
				SetSectMap( StartSect - 2, s3 );
			};
			if ( AllowUndo )
			{
				ChangeVert( v );

				if ( v >= 0 && v < MaxPointIndex )
				{
					THMap[v] += Height;
					if ( tex )
					{
						TexMap[v] = tex;
					}
				}
				MarkPointToDraw( v );
			}
			else
			{
				if ( v >= 0 && v < MaxPointIndex )
				{
					if ( CheckHI )
					{
						if ( NPOS < NNEG )
						{
							if ( THMap[v] >= Height )
							{
								THMap[v] = Height;
								if ( tex != 0 )
								{
									if ( ImmVis )
									{
										SetTexture( v, tex );
									}
									else TexMap[v] = tex + 128;
								};
							};
						}
						else
						{
							if ( THMap[v] <= Height )
							{
								THMap[v] = Height;
								if ( tex != 0 )
								{
									if ( ImmVis )
									{
										SetTexture( v, tex );
									}
									else TexMap[v] = tex + 128;
								};
							};
						};
					}
					else
					{
						THMap[v] += Height;
						if ( tex != 0 )
						{
							if ( ImmVis )
							{
								SetTexture( v, tex );
							}
							else TexMap[v] = tex + 128;
						};
						if ( ImmVis )MarkPointToDraw( v );
					};
				};
			};
		};
	};
	if ( Nv )
	{
		VSMX /= Nv;
		VSMY /= Nv;
	};
	CheckFirstLine();
};
void RM_LoadVerticesPos( ResFile F, short** VX, short** VY, int* lpN )
{
	int Nv;
	RBlockRead( F, &Nv, 4 );

	*lpN = Nv;
	*VX = new short[Nv];
	*VY = new short[Nv];
	short* Vx = *VX;
	short* Vy = *VY;

	for ( int i = 0; i < Nv; i++ )
	{
		short vx;
		short vy;
		byte tex;
		byte s1, s2, s3;
		short Height;
		RBlockRead( F, &vx, 2 );
		RBlockRead( F, &vy, 2 );
		Vx[i] = vx;
		Vy[i] = vy;
		RBlockRead( F, &tex, 1 );
		RBlockRead( F, &s1, 1 );
		RBlockRead( F, &s2, 1 );
		RBlockRead( F, &s3, 1 );
		RBlockRead( F, &Height, 2 );
	};
};
void RM_CalcVerticesPos( ResFile F, short* VX, short* VY )
{
	int Nv;
	RBlockRead( F, &Nv, 4 );
	int SVX = 0;
	int SVY = 0;
	int NT = 0;
	for ( int i = 0; i < Nv; i++ )
	{
		short vx;
		short vy;
		byte tex;
		byte s1, s2, s3;
		short Height;
		RBlockRead( F, &vx, 2 );
		RBlockRead( F, &vy, 2 );
		RBlockRead( F, &tex, 1 );
		if ( tex )
		{
			SVX += vx;
			SVY += vy;
			NT++;
		};
		RBlockRead( F, &s1, 1 );
		RBlockRead( F, &s2, 1 );
		RBlockRead( F, &s3, 1 );
		RBlockRead( F, &Height, 2 );
	};
	if ( NT )
	{
		SVX /= NT;
		SVY /= NT;
	};
	*VX = SVX;
	*VY = SVY;
};
//Save sprites
//short dx,dy
//byte SprGroup
//word Index

//-----------------------------------------------------------
void addSpriteAnyway( int x, int y, SprGroup* SG, word id );
void RM_SaveSprites( ResFile F, int x0, int y0, int Lx, int Ly, int xc, int yc )
{
	int sign = 'TRPS';
	RBlockWrite( F, &sign, 4 );
	int ns = 0;
	for ( int i = 0; i < MaxSprt; i++ )
	{
		OneSprite* OS = Sprites + i;
		if ( OS->Enabled )
		{
			int x = OS->x;
			int y = ( OS->y >> 1 ) - OS->z;
			if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )ns++;
		};
	};
	int i = ( ns * 12 ) + 12;
	RBlockWrite( F, &i, 4 );
	RBlockWrite( F, &ns, 4 );

	word j = 'GA';
	for ( int i = 0; i < MaxSprt; i++ )
	{
		OneSprite* OS = &Sprites[i];
		if ( OS->Enabled )
		{
			int x = OS->x;
			int y = ( OS->y >> 1 ) - OS->z;
			if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
			{
				j = 0;
				if ( OS->SG == &TREES )j = 'GA'; else
					if ( OS->SG == &STONES )j = 'TS'; else
						if ( OS->SG == &HOLES )j = 'OH'; else
							if ( OS->SG == &COMPLEX )j = 'OC';
				RBlockWrite( F, &j, 2 );
				int xx = OS->x - xc;
				RBlockWrite( F, &xx, 4 );
				xx = OS->y - yc;
				RBlockWrite( F, &xx, 4 );
				RBlockWrite( F, &OS->SGIndex, 2 );
			};
		};
	};
};
extern bool SpriteSuccess;
extern int LastAddSpr;
void RM_LoadSprites( ResFile F, int xc, int yc )
{
	int ns;
	RBlockRead( F, &ns, 4 );
	int maxx = ( msx << 5 ) - 32;
	int maxy = ( msy << 5 ) - 32;
	for ( int i = 0; i < ns; i++ )
	{
		int x, y;
		word sign, id;
		RBlockRead( F, &sign, 2 );
		RBlockRead( F, &x, 4 );
		RBlockRead( F, &y, 4 );
		RBlockRead( F, &id, 2 );
		x += xc;
		y += yc;
		if ( x > 0 && y > 0 && x < maxx&&y < maxy )
		{
			SpriteSuccess = 0;
			if ( sign == 'GA' )
			{
				addSpriteAnyway( x, y, &TREES, id );
			}
			else if ( sign == 'TS' )
			{
				addSpriteAnyway( x, y, &STONES, id );
			}
			else if ( sign == 'OH' )
			{
				addSpriteAnyway( x, y, &HOLES, id );
			}
			else if ( sign == 'OC' )
			{
				addSpriteAnyway( x, y, &COMPLEX, id );
			};
			if ( AllowUndo&&SpriteSuccess )ChangeSprite( LastAddSpr );
		};
	};
};
void RM_SaveObj( ResFile F, int x0, int y0, int Lx, int Ly, int xc, int yc )
{
	int sign = 'SJBO';
	RBlockWrite( F, &sign, 4 );
	int ns = 0;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !OB->Sdoxlo )
		{
			int x = OB->RealX >> 4;
			int y = ( OB->RealY >> 5 ) - OB->RZ;
			if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )ns++;
		};
	};
	char ONAME[31];
	int i = ns*( 32 + 4 + 4 ) + 12;
	RBlockWrite( F, &i, 4 );
	RBlockWrite( F, &ns, 4 );
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneSprite* OS = &Sprites[i];
		OneObject* OB = Group[i];
		if ( OB && !OB->Sdoxlo )
		{
			int x = OB->RealX >> 4;
			int y = ( OB->RealY >> 5 ) - OB->RZ;
			if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
			{
				memset( ONAME, 0, 31 );
				strcpy( ONAME, OB->Ref.General->MonsterID );
				int xx = ( OB->RealX >> 4 ) - xc;
				RBlockWrite( F, &xx, 4 );
				xx = ( OB->RealY >> 4 ) - yc;
				RBlockWrite( F, &xx, 4 );
				RBlockWrite( F, &OB->NNUM, 1 );
				RBlockWrite( F, ONAME, 31 );

			};
		};
	};
};
void CreateNewUnitAt( byte NI, int x, int y, word Type, word Life, word Stage );
extern int LASTADDID;
void RM_LoadObj( ResFile F, int xc, int yc )
{
	int ns;
	RBlockRead( F, &ns, 4 );
	char ONAME[32];
	for ( int i = 0; i < ns; i++ )
	{
		int xm, ym;
		RBlockRead( F, &xm, 4 );
		RBlockRead( F, &ym, 4 );
		byte NT;
		RBlockRead( F, &NT, 1 );
		NT = MyNation;
		RBlockRead( F, ONAME, 31 );
		GeneralObject** GO = NATIONS[NT].Mon;
		int Nmon = NATIONS[NT].NMon;
		for ( int p = 0; p < Nmon; p++ )
		{
			if ( !strcmp( GO[p]->MonsterID, ONAME ) )
			{
				if ( !GO[p]->newMons->Wall )
				{
					CreateNewUnitAt( NT, ( xc + xm ) << 4, ( yc + ym ) << 4, p, GO[p]->MoreCharacter->Life, GO[p]->MoreCharacter->ProduceStages );
					if ( LASTADDID != 0xFFFF && AllowUndo )
					{
						ChangeObject( LASTADDID );
					};
					goto ttt;
				};
			};
		};
	ttt:;
	};
};
extern BlockBars LockBars;
extern BlockBars UnLockBars;
void RM_SaveLock( ResFile F, int x0, int y0, int Lx, int Ly, int xc, int yc )
{
	int NL = 0;
	int NU = 0;
	for ( int i = 0; i < LockBars.NBars; i++ )
	{
		int x = ( int( LockBars.BC[i].x ) << 6 ) + 32;
		int y = ( int( LockBars.BC[i].y ) << 5 ) + 16;
		if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
		{
			NL++;
		};
	};
	for ( int i = 0; i < UnLockBars.NBars; i++ )
	{
		int x = ( int( UnLockBars.BC[i].x ) << 6 ) + 32;
		int y = ( int( UnLockBars.BC[i].y ) << 5 ) + 16;
		if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
		{
			NU++;
		};
	};
	int i = 'KCOL';
	RBlockWrite( F, &i, 4 );
	i = 16 + 2 * ( NU + NL );
	RBlockWrite( F, &i, 4 );
	RBlockWrite( F, &NL, 4 );
	RBlockWrite( F, &NU, 4 );
	for ( int i = 0; i < LockBars.NBars; i++ )
	{
		int xb = LockBars.BC[i].x;
		int yb = LockBars.BC[i].y;
		int x = ( int( xb ) << 6 ) + 32;
		int y = ( int( yb ) << 5 ) + 16;
		if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
		{
			xb -= xc;
			yb -= yc;
			RBlockWrite( F, &xb, 2 );
			RBlockWrite( F, &yb, 2 );
		};
	};
	for ( int i = 0; i < UnLockBars.NBars; i++ )
	{
		int xb = UnLockBars.BC[i].x;
		int yb = UnLockBars.BC[i].y;
		int x = ( int( xb ) << 6 ) + 32;
		int y = ( int( yb ) << 5 ) + 16;
		if ( x > x0&&y > y0&&x < x0 + Lx&&y < y0 + Ly )
		{
			xb -= xc;
			yb -= yc;
			RBlockWrite( F, &xb, 2 );
			RBlockWrite( F, &yb, 2 );
		};
	};
};
void RM_LoadLock( ResFile F, int xc, int yc )
{
	int NL, NU;
	RBlockRead( F, &NL, 4 );
	RBlockRead( F, &NU, 4 );
	for ( int i = 0; i < NL; i++ )
	{
		short x, y;
		RBlockRead( F, &x, 2 );
		RBlockRead( F, &y, 2 );
		x += xc;
		y += yc;
		if ( !AllowUndo )AddLockBar( x, y );
	};
	for ( int i = 0; i < NU; i++ )
	{
		short x, y;
		RBlockRead( F, &x, 2 );
		RBlockRead( F, &y, 2 );
		x += xc;
		y += yc;
		if ( !AllowUndo )AddUnLockbar( x, y );
	};
};
void SaveSMSInPieces( ResFile F, int keyX, int keyY, int x0, int y0, int x1, int y1 );
void RM_Save( char* Name, int x0, int y0, int x1, int y1 )
{
	if ( x1 < x0 )
	{
		x1 += x0;
		x0 = x1 - x0;
		x1 = x1 - x0;
	};
	if ( y1 < y0 )
	{
		y1 += y0;
		y0 = y1 - y0;
		y1 = y1 - y0;
	};
	ResFile F = RRewrite( Name );
	if ( F != INVALID_HANDLE_VALUE )
	{
		int kx = -1000;
		int ky;
		int i = 'PMAS';
		RBlockWrite( F, &i, 4 );
		RM_SaveVertices( F, x0, y0, x1 - x0, y1 - y0, &kx, &ky );
		if ( kx != -1000 )
		{
			RM_SaveSprites( F, x0, y0, x1 - x0, y1 - y0, kx << 5, ky << 5 );
			RM_SaveLock( F, x0, y0, x1 - x0, y1 - y0, kx >> 1, ky >> 1 );
			RM_SaveObj( F, x0, y0, x1 - x0, y1 - y0, kx << 5, ky << 5 );
		};
		i = -1;
		RBlockWrite( F, &i, 4 );
		RClose( F );
		char ccc[64];
		sprintf( ccc, "%s.lnk", Name );
		F = RRewrite( ccc );
		if ( F != INVALID_HANDLE_VALUE )
		{
			SaveSMSInPieces( F, kx << 5, ky << 5, x0, y0, x1, y1 );
			RClose( F );
		};
	};
};
void RM_Load( char* Name, int x, int y )
{
	ResFile F = RReset( Name );
	if ( F != INVALID_HANDLE_VALUE )
	{
		int SIGN, SIZE;
		RBlockRead( F, &SIGN, 4 );
		if ( SIGN == 'PMAS' )
		{
			while ( SIGN != -1 )
			{
				SIGN = -1;
				RBlockRead( F, &SIGN, 4 );
				RBlockRead( F, &SIZE, 4 );
				switch ( SIGN )
				{
				case 'TREV':
					RM_LoadVertices( F, ( x >> 6 ) << 1, ( y >> 6 ) << 1 );
					break;
				case 'TRPS':
					RM_LoadSprites( F, ( x >> 6 ) << 6, ( y >> 6 ) << 6 );
					break;
				case 'KCOL':
					RM_LoadLock( F, x >> 6, y >> 6 );
					break;
				case 'SJBO':
					RM_LoadObj( F, ( x >> 6 ) << 6, ( y >> 6 ) << 6 );
					break;
				};
			};
		};
		RClose( F );
	};
};
bool GetCenter( char* Name, short* dx, short* dy )
{
	ResFile F = RReset( Name );
	*dx = 0;
	*dy = 0;
	if ( F != INVALID_HANDLE_VALUE )
	{
		int SIGN, SIZE;
		RBlockRead( F, &SIGN, 4 );
		if ( SIGN == 'PMAS' )
		{
			while ( SIGN != -1 )
			{
				SIGN = -1;
				RBlockRead( F, &SIGN, 4 );
				RBlockRead( F, &SIZE, 4 );
				switch ( SIGN )
				{
				case 'TREV':
					RM_CalcVerticesPos( F, dx, dy );
					break;
				case 'TRPS':
					SIGN = -1;
					break;
				case 'KCOL':
					SIGN = -1;
					break;
				};
			};
		};
		RClose( F );
		return true;
	}
	else return false;
};
void RM_LoadVerticesOnly( char* Name, short** vx, short** vy, int * N )
{
	ResFile F = RReset( Name );
	if ( F != INVALID_HANDLE_VALUE )
	{
		int SIGN, SIZE;
		RBlockRead( F, &SIGN, 4 );
		if ( SIGN == 'PMAS' )
		{
			while ( SIGN != -1 )
			{
				SIGN = -1;
				RBlockRead( F, &SIGN, 4 );
				RBlockRead( F, &SIZE, 4 );
				switch ( SIGN )
				{
				case 'TREV':
					RM_LoadVerticesPos( F, vx, vy, N );
					break;
				case 'TRPS':
					SIGN = -1;
					break;
				case 'KCOL':
					SIGN = -1;
					break;
				};
			};
		};
		RClose( F );
	};

};
int xRMp = -10000;
int yRMp = -10000;
void RM_PreLoadOpen()
{
	xRMp = -10000;
	yRMp = -10000;
};
void RM_PreLoadClose()
{
	MakeUndo();
	UNDO1.Close();
};
void RM_PreLoad( char* Name, int x, int y )
{
	//if((x>>6)==xRMp&&(y>>6)==yRMp)return;
	MakeUndo();


	AllowUndo = true;
	ImmVis = true;
	ResFile F = RReset( Name );
	if ( F != INVALID_HANDLE_VALUE )
	{
		int SIGN, SIZE;
		RBlockRead( F, &SIGN, 4 );
		if ( SIGN == 'PMAS' )
		{
			while ( SIGN != -1 )
			{
				SIGN = -1;
				RBlockRead( F, &SIGN, 4 );
				RBlockRead( F, &SIZE, 4 );
				switch ( SIGN )
				{
				case 'TREV':
					RM_LoadVertices( F, ( x >> 6 ) << 1, ( y >> 6 ) << 1 );
					break;
				case 'KCOL':
					RM_LoadLock( F, x >> 6, y >> 6 );
					break;
				case 'TRPS':
					RM_LoadSprites( F, ( x >> 6 ) << 6, ( y >> 6 ) << 6 );
					break;
				case 'SJBO':
					RM_LoadObj( F, ( x >> 6 ) << 6, ( y >> 6 ) << 6 );
					break;
				};
				/*
				switch(SIGN){
				case 'TREV':
					RM_LoadVertices(F,(x>>6)<<1,(y>>6)<<1);
					break;
				case 'TRPS':
					RClose(F);
					AllowUndo=false;
					ImmVis=false;
					return;
					//RM_LoadSprites(F,(x>>6)<<6,(y>>6)<<6);
				case 'KCOL':
					RClose(F);
					AllowUndo=false;
					ImmVis=false;
					return;
					//RM_LoadLock(F,x>>6,y>>6);
				};
				*/

			};
		};
		RClose( F );
	};
	AllowUndo = false;
	ImmVis = false;
};
char RN_FName[64] = "sample";

//Removes extension from RN_FName
void DelExt()
{
	char* cc = strchr( RN_FName, '.' );
	if ( cc )
	{
		cc[0] = 0;
	}
}

bool EnterStr( char * str, char* Message );
bool TexPieceMode = false;

void ProcessRM_Load()
{
	ImmVis = true;

	if ( EnterStr( RN_FName, "Load sample" ) )
	{
		DelExt();
		char cc[128];
		sprintf( cc, "UserPieces\\%s.smp", RN_FName );
		RM_PreLoadOpen();
		TexPieceMode = true;
	}

	ImmVis = false;
}

void ProcessRM_LoadEx( char* Name )
{
	MakeUndo();

	RM_PreLoadClose();

	TexPieceMode = false;
	ImmVis = true;

	strcpy( RN_FName, Name );

	DelExt();

	char cc[128];
	sprintf( cc, "UserPieces\\%s.smp", RN_FName );

	RM_PreLoadOpen();

	TexPieceMode = true;
	ImmVis = false;
}

void ProcessRM_Save( int x0, int y0, int x1, int y1 )
{
	if ( EnterStr( RN_FName, "Save sample" ) )
	{
		DelExt();
		char cc[128];
		sprintf( cc, "UserPieces\\%s.smp", RN_FName );
		RM_Save( cc, x0, y0, x1, y1 );
	}
}

extern int LastKey;
extern bool KeyPressed;
void ProcessTexPiece( int x, int y, bool press )
{
	if ( KeyPressed&&LastKey == 27 )
	{
		MakeUndo();
		RM_PreLoadClose();
		TexPieceMode = false;
	}

	if ( TexPieceMode )
	{
		DelExt();
		char cc[128];
		sprintf( cc, "UserPieces\\%s.smp", RN_FName );
		if ( press )
		{
			RM_PreLoadClose();
			ImmVis = true;
			short dx;
			short dy;
			GetCenter( cc, &dx, &dy );
			RM_Load( cc, x - ( dx << 5 ), y - ( dy << 5 ) );
			ImmVis = false;
			TexPieceMode = false;
		}
		else
		{
			short dx;
			short dy;
			if ( ( x >> 6 ) == xRMp && ( y >> 6 ) == yRMp )
			{
				return;
			}
			xRMp = x >> 6;
			yRMp = y >> 6;
			GetCenter( cc, &dx, &dy );
			RM_PreLoad( cc, x - ( dx << 5 ), y - ( dy << 5 ) );
		}
	}
}

void ClearAllRounds();
void SetRandomPiece( char* Name )
{
	short* vx = NULL;
	short* vy = NULL;
	int Nv = 0;
	int t = GetTickCount();
	RM_LoadVerticesOnly( Name, &vx, &vy, &Nv );
	t = GetTickCount() - t;
	if ( Nv )
	{
		//search for the place
		int NAttm = 0;
		int xmax = msx >> 1;
		int vmax = msx - 3;
		//xmax-=20;
		//int xmax=msx>>2;
		//int vmax=msx>>1;
		while ( NAttm < 100 )
		{
			int x = ( int( mrand() )*xmax ) >> 15;
			int y = ( int( mrand() )*xmax ) >> 15;
			//checking
			x <<= 1;
			y <<= 1;
			bool Good = true;
			for ( int j = 0; j < Nv&&Good; j++ )
			{
				int vrx = x + vx[j];
				int vry = y + vy[j];
				if ( vrx > 2 && vrx < vmax&&vry>2 && vry < vmax )
				{
					int v = vrx + vry*VertInLine;
					if ( TexMap[v] >= 128 )Good = false;
				}
				else Good = false;
				j += 2;
			};
			if ( Good )
			{
				RM_Load( Name, x << 5, y << 5 );
				NAttm = 10000;
			};
			NAttm++;
		};
		free( vx );
		free( vy );
	};
};
void SetPieceInPoints( char* Name, int* xi, int* yi, int N, byte* Used, int NATT )
{
	short* vx = NULL;
	short* vy = NULL;
	int Nv = 0;
	int t = GetTickCount();
	RM_LoadVerticesOnly( Name, &vx, &vy, &Nv );
	t = GetTickCount() - t;
	if ( Nv )
	{
		//search for the place
		int NAttm = 0;
		int xmax = msx >> 1;
		int vmax = msx - 3;
		//xmax-=20;
		//int xmax=msx>>2;
		//int vmax=msx>>1;
		while ( NAttm < NATT )
		{
			int NP = ( int( rand() )*N ) >> 15;
			if ( !Used[NP] )
			{
				int x = xi[NP] >> 6;
				int y = yi[NP] >> 6;
				//checking
				x <<= 1;
				y <<= 1;
				bool Good = true;
				for ( int j = 0; j < Nv&&Good; j++ )
				{
					int vrx = x + vx[j];
					int vry = y + vy[j];
					if ( vrx > 2 && vrx < vmax&&vry>2 && vry < vmax )
					{
						int v = vrx + vry*VertInLine;
						if ( TexMap[v] >= 128 )Good = false;
					}
					else Good = false;
					j += 2;
				};
				if ( Good )
				{
					RM_Load( Name, x << 5, y << 5 );
					NAttm = 10000;
					Used[NP] = 1;
				};
			};
			NAttm++;
		};
		free( vx );
		free( vy );
	};
}

extern int LastAddSpr;

void GenerateByStyle( char* Style )
{
	GFILE* f = Gopen( Style, "r" );
	if ( f )
	{
		char cc[128];
		int N1, N2;
		int z;
		do
		{
			z = Gscanf( f, "%s%d%d", cc, &N1, &N2 );
			if ( z == 3 )
			{
				int N = N1 + ( ( int( mrand() )*( N2 - N1 ) ) >> 15 );
				N <<= ADDSH + ADDSH - 2;
				int NNames = 0;
				int MaxNames = 0;
				char** Names = nullptr;
				GFILE* F1 = Gopen( cc, "r" );
				if ( F1 )
				{
					int q;
					char cc1[128];
					do
					{
						q = Gscanf( F1, "%s", cc1 );
						if ( q == 1 )
						{
							if ( NNames >= MaxNames )
							{
								MaxNames += 512;
								Names = (char**) realloc( Names, MaxNames << 2 );
							};
							Names[NNames] = new char[strlen( cc1 ) + 1];
							strcpy( Names[NNames], cc1 );
							NNames++;
						};
					} while ( q == 1 );
					Gclose( F1 );
					if ( NNames )
					{
						for ( int j = 0; j < N; j++ )
						{
							SetRandomPiece( Names[( int( mrand() )*NNames ) >> 15] );
							ProcessMessages();
						};
						for ( int j = 0; j < NNames; j++ )
						{
							free( Names[j] );
						}
						free( Names );
					};
				};
			};
			if ( z == 1 )
			{
				if ( !strcmp( cc, "TEXTURESONLY" ) )
				{
					ClearAllRounds();
					ProcessMessages();
					z = 3;
				};
			};
		} while ( z == 3 );
		Gclose( f );
	}
}

int NNamesXX = 0;
void GenerateInPoints( char* Piece, int* ObjX, int* ObjY, int NObj, int NATT )
{
	int NNamesXX = 0;
	int MaxNames = 0;
	char** Names = NULL;
	GFILE* F1 = Gopen( Piece, "r" );
	if ( F1 )
	{
		int q;
		char cc1[128];
		do
		{
			q = Gscanf( F1, "%s", cc1 );
			if ( q == 1 )
			{
				if ( NNamesXX >= MaxNames )
				{
					MaxNames += 512;
					Names = (char**) realloc( Names, MaxNames << 2 );
				};
				Names[NNamesXX] = new char[strlen( cc1 ) + 1];
				strcpy( Names[NNamesXX], cc1 );
				NNamesXX++;
			};
		} while ( q == 1 );
		Gclose( F1 );
		if ( NNamesXX )
		{
			byte Used[4096];
			memset( Used, 0, sizeof Used );
			for ( int j = 0; j < NATT; j++ )
			{
				SetPieceInPoints( Names[( int( rand() )*NNamesXX ) >> 15], ObjX, ObjY, NObj, Used, 200 );
			}
			for ( int j = 0; j < NNamesXX; j++ )
			{
				free( Names[j] );
			}
			free( Names );
		};
	};
};
//---------------------------------------------------------------
void UnregisterSprite( int N );
void DeleteInArea( int x0, int y0, int x1, int y1 )
{
	//erasing vertices
	int Vx0 = ( x0 - 128 ) / ( TriUnit + TriUnit );
	int Vx1 = ( x1 + 128 ) / ( TriUnit + TriUnit );
	int Vy0 = ( y0 - 256 ) / ( TriUnit );
	int Vy1 = ( y1 + 256 ) / ( TriUnit );
	int Nv = 0;
	int vxs = 0;
	int vys = 0;
	int NVER = 0;
	int HI0 = 4096;
	int Nvc = 0;
	for ( int ix = Vx0; ix < Vx1; ix++ )
	{
		for ( int iy = Vy0; iy < Vy1; iy++ )
		{
			if ( ix > 0 && ix < VertInLine - 1 && iy>0 && iy < MaxTH - 1 )
			{
				int v = ix + iy*VertInLine;
				int x = GetTriX( v );
				int y = ( GetTriY( v ) >> 1 ) - GetHi( v );
				if ( x > x0&&y > y0&&x < x1&&y < y1 )
				{
					vxs += ix;
					vys += iy;
					Nvc++;
					if ( HI0 == 4096 )
					{
						HI0 = THMap[v];
					};
					THMap[v] = HI0;
					SetTexture( v, 0 );
					MarkPointToDraw( v );
				};
			};
		};
	};
	//erasing objects
	for ( int i = 0; i < MaxSprt; i++ )
	{
		OneSprite* OS = Sprites + i;
		if ( OS->Enabled )
		{
			int x = OS->x;
			int y = ( OS->y >> 1 ) - OS->z;
			if ( x > x0&&y > y0&&x < x1&&y < y1 )
			{
				OS->Enabled = false;
				UnregisterSprite( OS->Index );
				ObjTimer.Del( OS->Index, 0 );
			};
		};
	};
	//erasing locking
	for ( int i = 0; i < LockBars.NBars; i++ )
	{
		int xb = LockBars.BC[i].x;
		int yb = LockBars.BC[i].y;
		int x = ( int( xb ) << 6 ) + 32;
		int y = ( int( yb ) << 5 ) + 16;
		if ( x > x0&&y > y0&&x < x1&&y < y1 )
		{
			BClrBar( xb << 2, yb << 2, 4 );
			AddDefaultBar( xb, yb );
		};
	};
	for ( int i = 0; i < UnLockBars.NBars; i++ )
	{
		int xb = UnLockBars.BC[i].x;
		int yb = UnLockBars.BC[i].y;
		int x = ( int( xb ) << 6 ) + 32;
		int y = ( int( yb ) << 5 ) + 16;
		if ( x > x0&&y > y0&&x < x1&&y < y1 )
		{
			BClrBar( xb << 2, yb << 2, 4 );
			AddDefaultBar( xb, yb );
		}
	}
}

bool CheckInArea( int x0, int y0, int x1, int y1 )
{
	int Vx0 = ( x0 - 128 ) / ( TriUnit + TriUnit );
	int Vx1 = ( x1 + 128 ) / ( TriUnit + TriUnit );
	int Vy0 = ( y0 - 256 ) / ( TriUnit );
	int Vy1 = ( y1 + 256 ) / ( TriUnit );
	int HI0 = 4096;
	for ( int ix = Vx0; ix < Vx1; ix++ )
	{
		for ( int iy = Vy0; iy < Vy1; iy++ )
		{
			if ( ix > 0 && ix < VertInLine - 1 && iy>0 && iy < MaxTH - 1 )
			{
				int v = ix + iy*VertInLine;
				int x = GetTriX( v );
				int y = ( GetTriY( v ) >> 1 ) - GetHi( v );
				if ( x > x0&&y > y0&&x < x1&&y < y1 )
				{
					if ( TexMap[v] )return true;
				}
			}
		}
	}
	return false;
}

void SaveInSquares( int x0, int y0, int Lx, int Ly, int Nx, int Ny, char* Name )
{
	char cc[120];
	sprintf( cc, "Pieces\\%s.lst", Name );
	FILE* f = fopen( cc, "w" );
	for ( int iy = 0; iy < Ny; iy++ )
	{
		for ( int ix = 0; ix < Nx; ix++ )
		{
			int xx0 = x0 + ix*Lx;
			int yy0 = y0 + iy*Ly;
			if ( CheckInArea( xx0 + 4, yy0 + 4, xx0 + Lx - 4, yy0 + Ly - 4 ) )
			{
				char tri[5];
				int N = ix + iy*Nx;
				if ( N < 10 )
				{
					sprintf( tri, "00%d", N );
				}
				else
				{
					if ( N < 100 )
					{
						sprintf( tri, "0%d", N );
					}
					else
					{
						sprintf( tri, "%d", N );
					}
				}
				sprintf( cc, "Pieces\\%s%s.smp", Name, tri );
				fprintf( f, "%s\n", cc );
				RM_Save( cc, xx0 + 4, yy0 + 4, xx0 + Lx - 4, yy0 + Ly - 4 );
			}
		}
	}
	fclose( f );
}

char GSaveName[64] = "";
extern int GridX;
extern int GridY;
extern int GridLx;
extern int GridLy;
extern int GridNx;
extern int GridNy;
extern int GridMode;

void ProcessSaveInSquares()
{
	if ( GridMode&&EnterStr( GSaveName, "Save all pieces." ) )
	{
		SaveInSquares( GridX * 64, GridY * 32, GridLx * 64, GridLy * 32, GridNx, GridNy, GSaveName );
	}
}
//--------------------------TERRAINS--------------------------
struct TerrainData
{
	short Lx, Ly;
	byte  NPlaces;
	byte  NIDS;
	word  PLACES[8];
	byte  PlayerID[8];
	int   CompSize;
	byte  Data[32];
};
//SqInt params
int SqMaxX = ( 480 << ( ADDSH - 1 ) ) * 32;
int SqMaxY = ( 480 << ( ADDSH - 1 ) ) * 32;
int SqDX = ( 480 << ( ADDSH - 1 ) ) * 32 / 256;
int SqDY = ( 480 << ( ADDSH - 1 ) ) * 32 / 256;
//------------
int SqInt( byte* REND, int x, int y )
{
	if ( x < 0 )x = 0;
	if ( y < 0 )y = 0;
	if ( x >= SqMaxX )x = SqMaxX - 1;
	if ( y >= SqMaxY )y = SqMaxY - 1;
	int cx = ( x << 8 ) / SqMaxX;
	int cy = ( y << 8 ) / SqMaxY;
	if ( cx >= 255 )cx = 254;
	if ( cy >= 255 )cy = 254;
	int x0 = ( cx*SqMaxX ) >> 8;
	int y0 = ( cy*SqMaxY ) >> 8;
	int dx = x - x0;
	int dy = y - y0;
	int pos = cx + ( cy << 8 );
	int v0 = REND[pos] << 4;
	int v1 = REND[pos + 1] << 4;
	int v2 = REND[pos + 256] << 4;
	int v3 = REND[pos + 257] << 4;
	return v0 + ( ( v1 - v0 )*dx ) / SqDX + ( ( v2 - v0 )*dy + ( ( v3 + v0 - v1 - v2 )*dx*dy ) / SqDX ) / SqDY;
}

extern int MaxWX;
extern int MaxWY;
extern byte* WaterDeep;
extern byte* WaterBright;

void ErrM( char* s );

void DeleteAllSprites();

void ClearGoodDeepSpot( int x, int y, int r, int dr, int dh );

void CreateInfoMap();

void GenerateNationalResources( char* Mines, char* Trees, char* Stones,
	int NGold, int NIron, int NCoal, int NPlay, word* Units, int );

void CreateNatDealing( int N, byte* Nats, short* Nx, short* Ny );

bool ProcessMessages();

extern int RM_LandType;
extern int RM_Resstart;
extern int RM_Restot;

void GenerateRandomRoad( int idx );

extern int LastAddSpr;

void GenerateWithStyle( char* terr );

extern int PeaceTimeLeft;
extern int MaxPeaceTime;
extern int PeaceTimeStage;

byte BalloonState = 0;
byte CannonState = 0;
byte NoArtilleryState = 0;
byte XVIIIState = 0;
byte CaptState = 0;
byte SaveState = 2;
byte DipCentreState = 0;
byte ShipyardState = 0;
byte MarketState = 0;

int DecodeOptionsToGameSettings(const int number);

bool CreateRandomTerrain( char* Name, int NPlay, int MountStyle,
	int Ng, int NI, int Nc, word * Units, int options )
{
	LastAddSpr = 0;

	LockBars.Clear();

	UnLockBars.Clear();

	DeleteAllSprites();

	for ( int u = 0; u < MAXOBJECT; u++ )
	{
		OneObject* OB = Group[u];
		if ( OB )
		{
			OB->Die();
			if ( OB )
			{
				OB->Sdoxlo = 6000;
			}
		}
	}

	GPROG.SetCurrentStage( 'WATG' );

	ResFile RF = RReset( Name );
	if ( RF != INVALID_HANDLE_VALUE )
	{
		int chunk, N;
		RBlockRead( RF, &chunk, 4 );
		RBlockRead( RF, &N, 4 );
		if ( !N || 'TSOC' != chunk ) //'TSOC' = 0x54534f43
		{
			RClose( RF );
			return false;
		}

		chunk = ( (int) mrpos  * N ) >> 15;

		RSeek( RF, 8 + chunk * 4 );

		int file_offset_1;
		int file_offset_2;
		RBlockRead( RF, &file_offset_1, 4 );
		RBlockRead( RF, &file_offset_2, 4 );
		TerrainData* TERR = (TerrainData*) malloc( file_offset_2 - file_offset_1 );

		RSeek( RF, file_offset_1 );
		RBlockRead( RF, TERR, file_offset_2 - file_offset_1 );
		RClose( RF );

		int DAT = 0;
		int RPOS = 0;
		REND = new byte[65537];
		REND[65536] = 0xE3;//for integrity testing
		REND[65535] = 0xE3;

		file_offset_1 = TERR->CompSize;
		byte* Data = &TERR->Data[0];

		for ( int i = 0; i < file_offset_1; i++ )
		{
			file_offset_2 = Data[i];
			if ( file_offset_2 )
			{
				memset( REND + RPOS, DAT, file_offset_2 );
			}
			DAT = ~DAT;
			RPOS += file_offset_2;
		}

		//softing the terrain
		byte* REND1 = new byte[65537];
		memcpy( REND1, REND, 65537 );

		int NN = 30 >> ( ADDSH - 1 );
		for ( int j = 0; j < NN; j++ )
		{
			ProcessMessages();
			int Pos = 257;
			for ( int y = 1; y < 255; y++ )
			{
				for ( int x = 1; x < 255; x++ )
				{
					REND1[Pos] = ( ( REND[Pos] << 3 ) + ( REND[Pos] << 2 )
						+ REND[Pos - 1] + REND[Pos + 1] + REND[Pos - 256]
						+ REND[Pos + 256] ) >> 4;

					Pos++;
				}

				Pos += 2;
			}

			memcpy( REND, REND1, 65537 );

			for ( int y = 0; y < 256; y++ )
			{
				REND[y] = REND[y + 256];
				REND[y + 256 * 255] = REND[y + 256 * 254];

				int p = y << 8;

				REND[p] = REND[p + 1];
				REND[p + 255] = REND[p + 254];
			}

			GPROG.SetCurrentPosition( j + 1 );
		}

		byte* REND2 = new byte[65537];
		memcpy( REND2, REND, 65537 );

		//creating water
		int B0 = 15;
		int B2 = ( 120 - B0 )*( 120 - B0 );

		memset( WaterBright, 120, WMPSIZE );

		GPROG.SetCurrentStage( 'WBRI' );

		for ( int y = 0; y <= msy + 1; y++ )
		{
			for ( int x = 0; x <= msx + 1; x++ )
			{
				int pos = x + y*MaxWX;
				int VAL = SqInt( REND, x << 5, y << 5 ) - 2048;
				int VAL1 = SqInt( REND2, x << 5, y << 5 ) - 2048;
				int Deep = 120 - ( VAL >> 4 );
				int BRI = 120 + ( VAL1 / 20 ) + ( mrand() & 7 ) - 3;
				if ( BRI < 0 )BRI = 0;
				if ( BRI > 120 )BRI = 120;
				BRI = 120 - ( 120 - BRI )*( 120 - BRI ) * 120 / B2;
				if ( BRI > 120 )BRI = 120;
				if ( BRI < 0 )BRI = 0;
				if ( Deep < 0 )Deep = 0;
				if ( Deep > 255 )Deep = 255;
				if ( ADDSH == 3 )
				{
					if ( Deep < 67 )
						Deep = 67;
					if ( Deep > 128 + 62 )
						Deep = 128 + 62;
					WaterDeep[pos] = ( ( Deep - 128 ) << 1 ) + 128;
				}
				else
				{
					WaterDeep[pos] = ( ( Deep - 128 ) >> ( 2 - ADDSH ) ) + 128;
				}

				WaterBright[pos] = BRI;
			}
		}

		ProcessMessages();

		//creating cost line //grammar? - coastline
		byte CostTex[40][8];
		byte NTex[40];
		int NTLines;
		int Divisor;

		GPROG.SetCurrentStage( 'COST' );

		GFILE* f = Gopen( "Terrain\\cost.dat", "r" );
		if ( f )
		{
			int z = Gscanf( f, "%d%d", &NTLines, &Divisor );
			if ( z == 2 )
			{
				for ( int j = 0; j < NTLines; j++ )
				{
					int a;
					z = Gscanf( f, "%d", &a );
					if ( z != 1 )ErrM( "Invalid file Terrain\\cost.dat" );
					NTex[j] = a;
					int b;
					for ( int p = 0; p < a; p++ )
					{
						Gscanf( f, "%d", &b );
						CostTex[j][p] = b;
					}
				}
			}

			Gclose( f );

			int maxx = ( msx << 5 ) + 64;

			for ( int v = 0; v < MaxPointIndex; v++ )
			{
				int x = GetTriX( v );
				int y = GetTriY( v );
				if ( x > 0 && y > 0 && x < maxx&&y < maxx )
				{
					int VAL = SqInt( REND, x, y ) - 2048;
					int tid = VAL / Divisor;
					if ( tid < 0 )tid = 0;
					if ( tid < NTLines )
					{
						TexMap[v] = CostTex[tid][( int( mrand() )*int( NTex[tid] ) ) >> 15] + 128;
					}
				}
			}
		}

		ProcessMessages();

		//creating initial resources
		int NNat = 0;
		byte Nats[8] = { 0,1,2,3,4,5,6,7 };
		int NPlc[8] = { 0,0,0,0,0,0,0,0 };
		short Ntx[8];
		short Nty[8];

		for ( int s = 0; s < TERR->NPlaces; s++ )
		{
			NPlc[TERR->PlayerID[s]]++;
		}

		int SUBTB[8] = { 0,1,2,3,4,5,6,7 };

		for ( int q = 0; q < 20; q++ )
		{
			int v1 = ( int( mrand() )*int( TERR->NIDS ) ) >> 15;
			int v2 = ( int( mrand() )*int( TERR->NIDS ) ) >> 15;
			if ( v1 != v2 )
			{
				int t = SUBTB[v1];
				SUBTB[v1] = SUBTB[v2];
				SUBTB[v2] = t;
			}
		}

		// Calculate spawn Points ?
		for ( int s = 0; s < TERR->NIDS; s++ )
		{
			int pid = ( int( mrand() )*NPlc[s] ) >> 15;
			for ( int u = 0; u < TERR->NPlaces && pid != -1; u++ )
			{
				if ( TERR->PlayerID[u] == SUBTB[s] )
				{
					if ( !pid )
					{
						word PXY = TERR->PLACES[u];
						Ntx[s] = ( int( PXY & 255 )*msx ) >> 10;
						Nty[s] = ( int( PXY >> 8 )*msy ) >> 10;

					}

					pid--;
				}
			}
		}

		GPROG.SetCurrentStage( 'NTDL' );

		if ( TERR->NPlaces )
		{
			CreateNatDealing( TERR->NIDS, Nats, Ntx, Nty );

			ProcessMessages();
		}

		//--------------------------
		if ( 0 != MountStyle )
		{//IMPORTANT: Do not create any hills at all for 'plain' style
			if ( 2 == MountStyle )
			{
				LoadTerrain( "Terrain\\Mount.dat" );
			}
			else
			{
				LoadRandomHills();
			}
		}

		//Decode 7-digit number into game settings
		int GenIndex = DecodeOptionsToGameSettings(options);

		GenerateNationalResources( "Pieces\\MINA0.LST",
			"Pieces\\TRSS.LST", "Pieces\\STNN0.LST",
			Ng, NI, Nc, NPlay, Units, GenIndex );

		int maxx = msx << 5;
		int HH = 24;

		GPROG.SetCurrentStage( 'SETH' );

		for ( int v = 0; v < MaxPointIndex; v++ )
		{
			int x = GetTriX( v );
			int y = GetTriY( v );
			int VAL = SqInt( REND, x, y ) - 2048;
			if ( VAL > 32 )
			{
				THMap[v] += ( ( VAL*VAL ) >> 14 ) * HH / 100;
			}
		}

		ProcessMessages();

		free( REND );
		free( REND1 );
		free( REND2 );
		free( TERR );

		SetNextInfStage( "Generating nature" );

		GPROG.SetCurrentStage( 'NATR' );

		for ( int v = 0; v < MaxPointIndex; v++ )
		{
			TexMap[v] &= 127;
		}

		int NRR = 25 << ( ADDSH + ADDSH - 2 );

		ClearRender();

		CreateMiniMap();

		for ( int v = 0; v < MaxSprt; v++ )
		{
			OneSprite* OS = Sprites + v;
			if ( OS->Enabled )
			{
				OS->z = GetHeight( OS->x, OS->y );
			}
		}

		CreateInfoMap();

		NATIONS[7].Geology = true;

		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------REPORT ABOUT GENERATING------------------
extern int CurPalette;
int LastPalette = -1;
LocalGP ReportPanel;

char InfMessage[128];
char CurMessage[128];
int LastCallTime;
int InitInfTime;
int LastStageTime;
//SQPicture GPANEL;

void SetupInfMessage( char* Header )
{
	//if(!GPANEL.PicPtr)GPANEL.LoadPicture("y288x128.bpx");
	strcpy( InfMessage, Header );
	InitInfTime = GetTickCount();
}

void SetNextInfStage( char* Message )
{
	strcpy( CurMessage, Message );
	LastStageTime = GetTickCount();
}

extern int RealLx;
extern int RealLy;
bool NotInGame = false;
bool ProcessMessages();

void ShowProgressBar( char* Message, int N, int NMax )
{
	if ( LastPalette != CurPalette )
	{
		ReportPanel.UnLoad();
		if ( CurPalette )
		{
			//ReportPanel.Load("Interface\\Loading1");
		}
		else
		{
			ReportPanel.Load( "Interface\\Palettes\\Loading0" );
		}
		LastPalette = CurPalette;
	}
	int x0 = RealLx >> 1;
	int y0 = RealLy >> 1;
	int LX = GPS.GetGPWidth( ReportPanel.GPID, 0 );
	int LY = GPS.GetGPHeight( ReportPanel.GPID, 0 );
	if ( !CurPalette )
	{
		LX = 347;
		LY = 165;
		DrawStdBar( x0 - ( LX >> 1 ) + 16, y0 - ( LY >> 1 ) + 16, x0 + ( LX >> 1 ) - 16, y0 + ( LY >> 1 ) - 16 );
	}
	else
	{
		GPS.ShowGP( x0 - ( LX >> 1 ), y0 - ( LY >> 1 ), ReportPanel.GPID, 0, 0 );
	}
	int L1 = GetRLCStrWidth( Message, &BigRedFont ) >> 1;
	ShowString( x0 - L1, y0 - 64 + 20, Message, &BigRedFont );
	char cc[32];
	sprintf( cc, "%d%%", N * 100 / NMax );
	L1 = GetRLCStrWidth( cc, &BigWhiteFont ) >> 1;
	ShowString( x0 - L1, y0 - 64 + 48 + 10, cc, &BigYellowFont );
}

void ShowTable( char* Message, char* Hint )
{
	if ( LastPalette != CurPalette )
	{
		ReportPanel.UnLoad();
		if ( CurPalette )
		{
			ReportPanel.Load( "Interface\\Loading1" );
		}
		else
		{
			ReportPanel.Load( "Interface\\Palettes\\Loading0" );
		}
		LastPalette = CurPalette;
	}
	int x0 = RealLx >> 1;
	int y0 = RealLy >> 1;
	int LX = GPS.GetGPWidth( ReportPanel.GPID, 0 );
	int LY = GPS.GetGPHeight( ReportPanel.GPID, 0 );
	if ( !CurPalette )
	{
		LX = 347;
		LY = 165;
		DrawStdBar( x0 - ( LX >> 1 ) + 16, y0 - ( LY >> 1 ) + 16, x0 + ( LX >> 1 ) - 16, y0 + ( LY >> 1 ) - 16 );
	}
	else
	{
		GPS.ShowGP( x0 - ( LX >> 1 ), y0 - ( LY >> 1 ), ReportPanel.GPID, 0, 0 );
	}
	int L1 = GetRLCStrWidth( Message, &BigRedFont ) >> 1;
	ShowString( x0 - L1, y0 - 64 + 25, Message, &BigRedFont );
	L1 = GetRLCStrWidth( Hint, &BlackFont ) >> 1;
	ShowString( x0 - L1, y0 - 64 + 48 + 30, Hint, &BlackFont );
}
//------------------------------NATIONAL DIVISION OF THE TERRITORY------------------//

int NatX[8];
int NatY[8];
byte NatPres[8];
int NdMaxX = 120;
void InitNatDeal()
{
	NdMaxX = msx >> 2;
	NatDeals = new byte[NATLX*NATLX];
	memset( NatDeals, 0xFF, NATLX*NATLX );
	for ( int i = 1; i < NdMaxX; i++ )memset( NatDeals + ( i << NATSH ) + 1, 0xEE, NdMaxX - 1 );
	memset( NatPres, 0, sizeof NatPres );

	for ( int x = 1; x < NdMaxX; x++ )
		for ( int y = 1; y < NdMaxX; y++ )
		{  
			int ofs = ( x << 2 ) + 1 + ( ( ( y << 2 ) + 1 )*MaxWX );
			if ( WaterDeep[ofs] > 128 )NatDeals[x + ( y << NATSH )] = 0xFF;
		};

};
void SetNatDealPoint( int x, int y, byte Nat )
{
	if ( x > 0 && y > 0 && x < NdMaxX&&y < NdMaxX )
	{
		NatX[Nat] = x;
		NatY[Nat] = y;
		NatPres[Nat] = 1;
		NatDeals[x + ( y << NATSH )] = Nat << 4;
	};

};
int NFreeIsl = 0;
short FreeIsX[768];
short FreeIsY[768];
void ExtendNatDealing()
{

	int MAX = ( 60 << ADDSH );
	bool change;
	int SH0 = 6 + ADDSH;
	byte* TMP = new byte[NATLX*NATLX];
	memcpy( TMP, NatDeals, NATLX*NATLX );
	int CenterX[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int CenterY[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	for ( int iy = 2; iy < MAX; iy++ )
	{
		for ( int ix = 2; ix < MAX; ix++ )
		{
			int ofs = ix + ( iy << SH0 );
			byte c = TMP[ofs];
			// 128dec = 0x80
			if ( c < 128 )
			{
				c >>= 4;
				CenterX[c] = ix;
				CenterY[c] = iy;
			};
		};
	};

	do
	{
		memcpy( TMP, NatDeals, NATLX*NATLX );
		change = 0;
		for ( int iy = 0; iy < MAX; iy++ )
		{
			for ( int ix = 0; ix < MAX; ix++ )
			{
				int ofs = ix + ( iy << SH0 );
				byte cxx = TMP[ofs];
				byte c;
				if ( cxx == 0xEE )
				{
					if ( ( c = TMP[ofs + 1] ) < 0xEE )
					{
						c >>= 4;
						int q = Norma( ix - CenterX[c], iy - CenterY[c] ) >> 2;
						if ( q > 15 )q = 15;
						NatDeals[ofs] = q + ( c << 4 );
						change = 1;
					}
					else
						if ( ix > 0 && ( c = TMP[ofs - 1] ) < 0xEE )
						{
							c >>= 4;
							int q = Norma( ix - CenterX[c], iy - CenterY[c] ) >> 2;
							if ( q > 15 )q = 15;
							NatDeals[ofs] = q + ( c << 4 );
							change = 1;
						}
						else
							if ( ( c = TMP[ofs + NATLX] ) < 0xEE )
							{
								c >>= 4;
								int q = Norma( ix - CenterX[c], iy - CenterY[c] ) >> 2;
								if ( q > 15 )q = 15;
								NatDeals[ofs] = q + ( c << 4 );
								change = 1;
							}
							else
								if ( iy > 0 && ( c = TMP[ofs - NATLX] ) < 0xEE )
								{
									c >>= 4;
									int q = Norma( ix - CenterX[c], iy - CenterY[c] ) >> 2;
									if ( q > 15 )q = 15;
									NatDeals[ofs] = q + ( c << 4 );
									change = 1;
								}
				}
			}
		}

		ProcessMessages();

	} while ( change );

	free( TMP );

	NFreeIsl = 0;
	for ( int x = 1; x < NATLX - 1; x += 2 )
	{
		for ( int y = 1; y < NATLX - 1; y += 2 )
		{
			int ofs = x + ( y << NATSH );
			if ( NatDeals[ofs] == 0xEE && NFreeIsl < 768 )
			{
				FreeIsX[NFreeIsl] = x;
				FreeIsY[NFreeIsl] = y;
				NFreeIsl++;
			}
		}
	}
}

bool GetRandomPoint( short* x, short* y, byte TypeStart, byte TypeEnd )
{
	if ( TypeStart == 0xFF )
	{
		if ( !NFreeIsl )return 0;
		int idx = ( int( mrand() )*NFreeIsl ) >> 15;
		*x = FreeIsX[idx];
		*y = FreeIsY[idx];
		return true;
	};
	byte Nat = TypeStart >> 4;
	int R = TypeStart & 15;
	int RM = TypeEnd & 15;
	int R0 = R << 2;
	int R1 = ( ( RM + 1 ) << 2 ) - 1;
	int NATT = 0;
	int x0 = *x;
	int y0 = *y;
	GenArea* GAS = GMAP.GTopMap;
	int NG = GMAP.NGAreas;
	do
	{
		/*
		int ar=(int(rand())*NG)>>15;
		GenArea* GA=GAS+ar;
		if(GA->Type>=TypeStart&&GA->Type<=TypeEnd){
			*x=GA->x>>1;
			*y=GA->y>>1;
			return true;
		};
		*/

		R = R0 + ( ( int( mrand() )*( R1 - R0 ) ) >> 15 );
		int N = Rarr[R].N;
		int idx = ( int( mrand() )*N ) >> 15;
		int xs = x0 + Rarr[R].xi[idx];
		int ys = y0 + Rarr[R].yi[idx];
		if ( xs > 5 && ys > 5 && xs < NdMaxX - 4 && ys < NdMaxX - 4 )
		{
			byte NDE = NatDeals[xs + ( ys << NATSH )];
			if ( NDE >= TypeStart&&NDE <= TypeEnd )
			{
				*x = xs;
				*y = ys;
				return true;
			};
		};

		NATT++;
	} while ( NATT < 400 );
	return false;
};
bool ConditionalSetRandomPiece( char* Name, byte StartType, byte EndType, int NAttempts )
{
	short* vx = NULL;
	short* vy = NULL;
	int Nv = 0;
	int Nat = StartType >> 4;
	short xst = NatX[Nat];
	short yst = NatY[Nat];
	bool Done = false;
	RM_LoadVerticesOnly( Name, &vx, &vy, &Nv );
	if ( Nv )
	{
		//search for the place
		int NAttm = 0;
		int xmax = msx >> 1;
		int vmax = msx - 3;
		xmax -= 20;
		//int xmax=msx>>2;
		//int vmax=msx>>1;
		while ( NAttm < NAttempts )
		{
			short x = xst;
			short y = yst;
			if ( GetRandomPoint( &x, &y, StartType, EndType ) )
			{
				//checking
				x <<= 2;
				y <<= 2;
				bool Good = true;
				for ( int j = 0; j < Nv&&Good; j++ )
				{
					int vrx = x + vx[j];
					int vry = y + vy[j];
					if ( vrx > 2 && vrx < vmax&&vry>2 && vry < vmax )
					{
						int v = vrx + vry*VertInLine;
						if ( TexMap[v] > 127 )Good = false;
					}
					else Good = false;
				};
				if ( Good )
				{
					RM_Load( Name, x << 5, y << 5 );
					NAttm = 10000;
					Done = true;
				};
			};
			NAttm++;
		};
		free( vx );
		free( vy );
	};
	return Done;
};
bool ProcessMessages();
bool ConditionalSetRandomPiece1( char* Name, byte StartType, byte EndType, int NAttempts )
{
	if ( StartType == 0xFF )
	{
		if ( ConditionalSetRandomPiece( Name, 0xFF, 0xFF, NAttempts ) )return true;
		return false;
	};
	for ( int p = StartType; p < EndType; p++ )
	{
		ProcessMessages();
		if ( ConditionalSetRandomPiece( Name, p, p, NAttempts ) )return true;
	};
	return false;
};
class PieceList
{
public:
	char** Names;
	int NNames;
	int MaxNames;
	PieceList();
	void Load( char* Name );
	PieceList( char* Name );
	~PieceList();
	void Clear();
};
PieceList::PieceList()
{
	Names = NULL;
	NNames = 0;
	MaxNames = 0;
};
PieceList::~PieceList()
{
	Clear();
};
void PieceList::Clear()
{
	for ( int i = 0; i < NNames; i++ )free( Names[i] );
	if ( Names )free( Names );
	Names = NULL;
	NNames = 0;
	MaxNames = 0;
};
void PieceList::Load( char* Name )
{
	Clear();
	GFILE* F1 = Gopen( Name, "r" );
	if ( F1 )
	{
		int q;
		char cc1[128];
		do
		{
			q = Gscanf( F1, "%s", cc1 );
			if ( q == 1 )
			{
				if ( NNames >= MaxNames )
				{
					MaxNames += 512;
					Names = (char**) realloc( Names, MaxNames << 2 );
				};
				Names[NNames] = new char[strlen( cc1 ) + 1];
				strcpy( Names[NNames], cc1 );
				NNames++;
			};
		} while ( q == 1 );
		Gclose( F1 );
	};
};
PieceList::PieceList( char* Name )
{
	Names = NULL;
	NNames = 0;
	MaxNames = 0;
	Load( Name );
};
/**
	Creates the National Division Dealing of territory for all nations
	@param N Amount of possible nations on this map
	@param N Constant array, containing 0-7
	@param Nx Starting X of the Nation Deal ?
	@param Ny Starting Y of the Nation Deal ?
*/
void CreateNatDealing( int N, byte* Nats, short* Nx, short* Ny )
{
	InitNatDeal();

	for ( int i = 0; i < N; i++ )
	{
		SetNatDealPoint( Nx[i], Ny[i], Nats[i] );
	}

	//ExtendNatDealing();
}

void LimitZones()
{
	//Marking zones
	for ( int i = 0; i < GMAP.NGAreas; i++ )
	{
		GenArea* GA = GMAP.GTopMap + i;
		int ofs = ( GA->x >> 1 ) + ( ( GA->y >> 1 ) << NATSH );
		byte host = NatDeals[ofs];
		if ( host < 128 )
		{
			GA->Nation = ( host >> 7 >> 1 );
			GA->Zone = host & 15;
			GA->Type = host;
			if ( GA->Zone < 3 && !GA->State )GA->State = 1;
		};
	};
};
void CreateCenterRound( int x, int y, int R )
{
	int DR = ( R >> 5 ) + 3;
	int vx0 = ( x >> 5 ) - DR;
	int vy0 = ( y >> 5 ) - DR;
	int vx1 = ( x >> 5 ) + DR;
	int vy1 = ( y >> 5 ) + DR;
	if ( vx0 < 0 )vx0 = 0;
	if ( vx0 >= VertInLine )vx0 = VertInLine - 1;
	if ( vx1 < 0 )vx1 = 0;
	if ( vx1 >= VertInLine )vx1 = VertInLine - 1;
	if ( vy0 < 0 )vy0 = 0;
	if ( vy0 > MaxTH - 1 )vy0 = MaxTH - 1;
	if ( vy1 < 0 )vy1 = 0;
	if ( vy1 > MaxTH - 1 )vy1 = MaxTH - 1;
	for ( int ix = vx0; ix <= vx1; ix++ )
		for ( int iy = vy0; iy <= vy1; iy++ )
		{
			int v = ix + iy*VertInLine;
			if ( !TexMap[v] )
			{
				int xc = GetTriX( v );
				int yc = GetTriY( v );
				if ( Norma( x - xc, y - yc ) < R )TexMap[v] = 255;
			};
		};
};
void ClearCenterRound( int x, int y, int R )
{
	int DR = ( R >> 5 ) + 3;
	int vx0 = ( x >> 5 ) - DR;
	int vy0 = ( y >> 5 ) - DR;
	int vx1 = ( x >> 5 ) + DR;
	int vy1 = ( y >> 5 ) + DR;
	if ( vx0 < 0 )vx0 = 0;
	if ( vx0 >= VertInLine )vx0 = VertInLine - 1;
	if ( vx1 < 0 )vx1 = 0;
	if ( vx1 >= VertInLine )vx1 = VertInLine - 1;
	if ( vy0 < 0 )vy0 = 0;
	if ( vy0 > MaxTH - 1 )vy0 = MaxTH - 1;
	if ( vy1 < 0 )vy1 = 0;
	if ( vy1 > MaxTH - 1 )vy1 = MaxTH - 1;
	for ( int ix = vx0; ix <= vx1; ix++ )
		for ( int iy = vy0; iy <= vy1; iy++ )
		{
			int v = ix + iy*VertInLine;
			if ( TexMap[v] == 255 )TexMap[v] = 0;
		};
};
int NCRound;
int CRoundX[8];
int CRoundY[8];
bool GenerateStartUnits( char* NationID, byte NI, int x, int y, int GenIndex );
void PictureCordons();
byte CordonIDX[8];
int PeaceTimeLeft = 300;
int MaxPeaceTime = 0;
int PeaceTimeStage = 0;

int GetUnitActivity( OneObject* OB )
{
	if ( !( PeaceTimeLeft && NatDeals ) )
	{
		return 0;
	}

	if ( OB )
	{
		int xm = OB->RealX >> 11;
		int ym = OB->RealY >> 11;
		if ( xm <= 0 || ym <= 0 || xm >= NATLX || ym >= NATLX )
			return 0;
		byte Deal = NatDeals[xm + ( ym << ( TopSH - 1 ) )];
		if ( Deal >= 128 )
			return 0;
		Deal >>= 4;
		if ( CordonIDX[Deal] == 255 )
			return 0;
		if ( OB->NMask & NATIONS[CordonIDX[Deal]].NMask )
			return 1;
	}
	return -1;
}

bool CheckBuildPossibility( byte NI, int x, int y )
{
	if ( !( PeaceTimeLeft&&NatDeals ) )
	{
		return true;
	}
	int xm = x >> 7;
	int ym = y >> 7;
	if ( xm <= 0 || ym <= 0 || xm >= NATLX || ym >= NATLX )
	{
		return false;
	}
	byte Deal = NatDeals[xm + ( ym << ( TopSH - 1 ) )];
	if ( Deal >= 128 )
	{
		return true;
	}
	Deal >>= 4;
	if ( CordonIDX[Deal] == 255 )
	{
		return true;
	}
	bool retval = ( NATIONS[CordonIDX[Deal]].NMask & NATIONS[NI].NMask ) != 0;
	return retval;
}

void GenerateNationalResources( char* Mines, char* Trees, char* Stones,
	int NGold, int NIron, int NCoal, int NPlay, word* Units, int GenIndex )
{
	byte NatStart[8];
	NCRound = 0;
	byte NIDX[8];
	for ( int i = 0; i < 8; i++ )NatStart[i] = NatPres[i];
	int NP = 0;
	int nattm = 0;
	while ( NP < NPlay&&nattm < 1000 )
	{
		int p = mrand() & 7;
		if ( NatStart[p] == 1 )
		{
			NatStart[p] = 2;
			NIDX[p] = NP;
			NP++;
		};
		nattm++;
	};
	int curN = 0;
	ProcessMessages();
	GPROG.SetCurrentStage( 'STRS' );
	ProcessMessages();
	for ( int Nat = 0; Nat < 8; Nat++ )
	{
		ProcessMessages();
		CordonIDX[Nat] = 0xFF;
		if ( NatPres[Nat] )
		{
			byte v = Nat << 4;
			//1.Generating trees
			PieceList OBJS( Trees );
			if ( OBJS.NNames )
			{
				int id = ( int( mrand() )*OBJS.NNames ) >> 15;
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 1, v + 14, 50 );
			};
			//2.Generating stones
			OBJS.Load( Stones );
			if ( OBJS.NNames )
			{
				int id = ( int( mrand() )*OBJS.NNames ) >> 15;
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 1, v + 14, 50 );
				id = ( int( mrand() )*OBJS.NNames ) >> 15;
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 1, v + 14, 50 );
			};
			if ( NatStart[Nat] == 2 )
			{
				int x0 = int( NatX[Nat] ) << 7;
				int y0 = int( NatY[Nat] ) << 7;
				CreateCenterRound( x0, y0, 750 );
				CRoundX[NCRound] = x0;
				CRoundY[NCRound] = y0;
				NCRound++;
				while ( Units[curN] == 0xFFFF && curN < 7 )curN++;
				if ( Units[curN] != 0xFFFF )
				{
					CordonIDX[Nat] = curN;
					if ( !GenerateStartUnits( NATIONS[0].Mon[Units[curN]]->MonsterID, curN, x0, y0, GenIndex ) )
					{
						for ( int p = 0; p < 18; p++ )
						{
							int dx = ( int( mrand() ) * 128 ) >> 15;
							int dy = ( int( mrand() ) * 128 ) >> 15;
							int ID = NATIONS[curN].CreateNewMonsterAt( ( x0 + dx ) << 4, ( y0 + dy ) << 4, Units[curN], 1 );
						};
					};
					curN++;
				};
			};
		};
	};
	ProcessMessages();
	GPROG.SetCurrentStage( 'TERR' );
	ProcessMessages();
	GenerateWithStyle( CurTerrName );
	//GenerateWithStyle("Pieces\\forest.terr");
	ProcessMessages();
	GPROG.SetCurrentStage( 'TRES' );
	ProcessMessages();
	for ( int Nat = 0; Nat < 8; Nat++ )
	{
		if ( NatPres[Nat] )
		{
			byte v = Nat << 4;
			//1.Generating trees
			PieceList OBJS( Trees );

			//3.Generating mines
			OBJS.Load( Mines );
			if ( OBJS.NNames )
			{
				int NMT = OBJS.NNames / 3;
				//Gold mine
				int id = 3 * ( ( int( mrand() )*NMT ) >> 15 );
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 2, v + 14, 600 );
				//Iron mine
				id = 1 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 2, v + 14, 600 );
				//Coal mine
				id = 2 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 2, v + 14, 600 );
				id = 3 * ( ( int( mrand() )*NMT ) >> 15 );
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 2, v + 14, 600 );
				//Iron mine
				id = 1 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 2, v + 14, 600 );
				//Coal mine
				id = 2 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
				ConditionalSetRandomPiece1( OBJS.Names[id], v + 2, v + 14, 600 );
				//setting other mines
				for ( int j = 2; j < NGold || j < NIron || j < NCoal; j++ )
				{
					if ( j < NGold )
					{
						id = 3 * ( ( int( mrand() )*NMT ) >> 15 );
						ConditionalSetRandomPiece( OBJS.Names[id], v + 3, v + 14, 400 );
					};
					if ( j < NIron )
					{
						id = 1 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
						ConditionalSetRandomPiece( OBJS.Names[id], v + 3, v + 14, 400 );
					};
					if ( j < NCoal )
					{
						id = 2 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
						ConditionalSetRandomPiece( OBJS.Names[id], v + 3, v + 14, 400 );
					};
				};
			};
		};
	};
	if ( NFreeIsl )
	{
		PieceList OBJS( Mines );
		int NMT = OBJS.NNames / 3;
		for ( int i = 0; i < 30; i++ )
		{
			//Gold mine
			int id = 3 * ( ( int( mrand() )*NMT ) >> 15 );
			ConditionalSetRandomPiece1( OBJS.Names[id], 0xFF, 0xFF, 20 );
			//Iron mine
			id = 1 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
			ConditionalSetRandomPiece1( OBJS.Names[id], 0xFF, 0xFF, 20 );
			//Coal mine
			id = 2 + 3 * ( ( int( mrand() )*NMT ) >> 15 );
			ConditionalSetRandomPiece1( OBJS.Names[id], 0xFF, 0xFF, 20 );
		};
		OBJS.Load( Stones );
		NMT = OBJS.NNames / 3;
		for ( int i = 0; i < 20; i++ )
		{
			int id = ( int( mrand() )*OBJS.NNames ) >> 15;
			ConditionalSetRandomPiece1( OBJS.Names[id], 0xFF, 0xFF, 20 );
		};
		OBJS.Load( Trees );
		NMT = OBJS.NNames / 3;
		for ( int i = 0; i < 20; i++ )
		{
			int id = ( int( mrand() )*OBJS.NNames ) >> 15;
			ConditionalSetRandomPiece1( OBJS.Names[id], 0xFF, 0xFF, 20 );
		};
	};
	ProcessMessages();
	GPROG.SetCurrentStage( 'CORD' );
	ProcessMessages();
	PictureCordons();
};
void ClearAllRounds()
{
	for ( int i = 0; i < NCRound; i++ )
	{
		ClearCenterRound( CRoundX[i], CRoundY[i], 750 );
	};
	NCRound = 0;
}

#define C_NATLX 128
void ShowNatDeal()
{
	return;
	int adx = ScrWidth - NATLX;
	int sptr = int( ScreenPtr ) + ScrWidth * 100;

	__asm
	{
		push	esi
		push	edi
		mov		edi, sptr
		mov		esi, NatDeals
		cld
		mov		ebx, C_NATLX
		lpp1 : mov     edx, C_NATLX
			   lpp2 : lodsb
					  shr		al, 4
					  shl		al, 2
					  add		al, 0xD0
					  //mov		ecx,C_NATLX>>2
					  stosb
					  dec		edx
					  jnz     lpp2
					  //rep		movsd
					  add		edi, adx
					  dec		ebx
					  jnz		lpp1
					  pop		edi
					  pop		esi
	}
}

void SetBrightSpot( int x, int y, int Brightness, bool dir );

void DrawCordonPoint( int x, int y )
{
	if ( WaterDeep[( x >> 5 ) + ( ( y >> 5 ) << ( TopSH + 1 ) )] > 127 )
	{
		SetBrightSpot( x, y >> 1, 6, 1 );
	}
	else
	{
		addSpriteAnyway( x + ( mrand() % 7 ) - 3, y + ( mrand() % 7 ) - 3, &HOLES, mrand() % 6 );
	}
}

void PictureCordons()
{
	if ( !PeaceTimeLeft )return;
	//1. extending on water
	int MAX = ( 60 << ADDSH ) - 2;
	bool change;
	int SH0 = 6 + ADDSH;
	byte* TMP = new byte[NATLX*NATLX];
	memcpy( TMP, NatDeals, NATLX*NATLX );
	do
	{
		memcpy( TMP, NatDeals, NATLX*NATLX );
		change = 0;
		for ( int iy = 2; iy < MAX; iy++ )
		{
			for ( int ix = 2; ix < MAX; ix++ )
			{
				int ofs = ix + ( iy << SH0 );
				byte c = TMP[ofs];
				if ( c == 0xFF )
				{
					if ( TMP[ofs + 1] != 0xFF )
					{
						NatDeals[ofs] = TMP[ofs + 1];
						change = 1;
					}
					else
						if ( TMP[ofs - 1] != 0xFF )
						{
							NatDeals[ofs] = TMP[ofs - 1];
							change = 1;
						}
						else
							if ( TMP[ofs + NATLX] != 0xFF )
							{
								NatDeals[ofs] = TMP[ofs + NATLX];
								change = 1;
							}
							else
								if ( TMP[ofs - NATLX] != 0xFF )
								{
									NatDeals[ofs] = TMP[ofs - NATLX];
									change = 1;
								};
				};
			};
		};
		ProcessMessages();
	} while ( change );
	free( TMP );
	//
	int ofs = 0;
	int MaxX = NATLX - 2;
	int Stolb = 47;
	for ( int y = 0; y < NATLX; y++ )
	{
		for ( int x = 0; x < NATLX; x++ )
		{
			if ( x > 0 && x < MaxX&&y>0 && y < MaxX )
			{
				int x0 = ( x << 7 ) + 64;
				int y0 = ( y << 7 ) + 64;
				byte c = NatDeals[ofs];
				if ( c != 0xFF )
				{
					c >>= 4;
					int cr = NatDeals[ofs + 1];
					if ( cr != 0xFF )
					{
						cr >>= 4;
						if ( cr != c )
						{
							for ( int k = -1; k < 3; k++ )
								DrawCordonPoint( x0 + 64, y0 - 16 + 32 * k );
						}
					}
					cr = NatDeals[ofs + NATLX];
					if ( cr != 0xFF )
					{
						cr >>= 4;
						if ( cr != c )
						{
							for ( int k = -1; k < 3; k++ )
								DrawCordonPoint( x0 - 16 + 32 * k, y0 + 64 );
						};
					};

				};
			};
			ofs++;
		};
	};
}

RandomMapDesc::RandomMapDesc()
{
	NMINES = 0;
	MINES = NULL;
	NRES = 0;
	RES = NULL;
	NRelief = 0;
	Relief = NULL;
	NSTY = 0;
	STY = NULL;
}

void RandomMapDesc::Close()
{
	if ( NMINES )
		free( MINES );

	MINES = NULL;
	NMINES = 0;
	if ( NRES )free( RES );
	RES = NULL;
	NRES = 0;
	if ( NRelief )
	{
		for ( int p = 0; p < NRelief; p++ )
		{
			free( Relief[p] );
		};
		free( Relief );
	};
	Relief = NULL;
	NRelief = 0;
	if ( NSTY )
	{
		for ( int p = 0; p < NSTY; p++ )
		{
			int N2 = STY[p].NPl;
			for ( int q = 0; q < N2; q++ )
			{
				free( STY[p].Players[q].name );
			};
			free( STY[p].Players );
		};
		free( STY );
	};
	STY = NULL;
	NSTY = 0;
}

RandomMapDesc::~RandomMapDesc()
{
	Close();
}

void ERRMP( char* name, char* Sect )
{
	char cc[128];
	sprintf( cc, "File %s is corrupted in section %s", name, Sect );
	ErrM( cc );
}

void normstr( char* str );

char* GetTextByID( char* ID );

void RandomMapDesc::Load( char* name )
{
	Close();
	GFILE* F = Gopen( name, "r" );
	if ( F )
	{
		char cc[128];
		int z = Gscanf( F, "%d%d", &NMINES, &MINES_DEF );
		if ( z != 2 )
		{
			ERRMP( name, "MINES" );
		}

		MINES = new MineRec[NMINES];

		for ( int p = 0; p < NMINES; p++ )
		{
			int n1, n2, n3;
			z = Gscanf( F, "%s%d%d%d", cc, &n1, &n2, &n3 );
			if ( z != 4 )ERRMP( name, "MINES" );
			char* ust = GetTextByID( cc );
			MINES[p].Name = new char[strlen( ust ) + 1];
			strcpy( MINES[p].Name, ust );
			MINES[p].Ng = n1;
			MINES[p].Ni = n2;
			MINES[p].Nc = n3;
		}

		z = Gscanf( F, "%d%d", &NRES, &RES_DEF );
		if ( z != 2 )
		{
			ERRMP( name, "STARTRES" );
		}

		RES = new ResRec[NRES];

		for ( int p = 0; p < NRES; p++ )
		{
			int Nr;
			z = Gscanf( F, "%s%d", cc, &Nr );
			char* ust = GetTextByID( cc );
			if ( z != 2 )
			{
				ERRMP( name, "STARTRES" );
			}

			memset( RES[p].RES, 0, sizeof RES[p].RES );
			RES[p].Name = new char[strlen( ust ) + 1];
			strcpy( RES[p].Name, ust );

			for ( int q = 0; q < Nr; q++ )
			{
				int n1;
				z = Gscanf( F, "%d", &n1 );
				if ( z != 1 )
				{
					ERRMP( name, "STARTRES" );
				}

				RES[p].RES[q] = n1;
			}
		}

		z = Gscanf( F, "%d%d", &NRelief, &Relief_DEF );
		if ( z != 2 )
		{
			ERRMP( name, "RELIEF" );
		}

		Relief = (char**) malloc( NRelief * 4 );

		for ( int p = 0; p < NRelief; p++ )
		{
			z = Gscanf( F, "%s", cc );
			char* ust = GetTextByID( cc );
			//normstr(cc);
			if ( z != 1 )
			{
				ERRMP( name, "RELIEF" );
			}

			Relief[p] = new char[strlen( ust ) + 1];
			strcpy( Relief[p], ust );
		}

		z = Gscanf( F, "%d%d", &NSTY, &STY_DEF );
		if ( z != 2 )
		{
			ERRMP( name, "STYLES" );
		}

		STY = new StyleRec[NSTY];

		for ( int p = 0; p < NSTY; p++ )
		{
			StyleRec* SR = STY + p;
			Gscanf( F, "%s", cc );
			//normstr(cc);
			int aitp = p;
			if ( cc[0] == '#' )
			{
				aitp = cc[1] - '0';
				Gscanf( F, "%s", cc );
			}

			char* ust = GetTextByID( cc );
			SR->Name = new char[strlen( ust ) + 1];
			strcpy( SR->Name, ust );

			Gscanf( F, "%s", cc );
			SR->Style = new char[strlen( cc ) + 1];
			strcpy( SR->Style, cc );

			z = Gscanf( F, "%d", &SR->NPl );
			SR->Players = new PlRec[SR->NPl];

			SR->AI_Style = aitp;

			for ( int q = 0; q < SR->NPl; q++ )
			{
				int n1;
				Gscanf( F, "%d%s", &n1, cc );

				SR->Players[q].name = new char[strlen( cc ) + 1];
				strcpy( SR->Players[q].name, cc );

				SR->Players[q].NPlayers = n1;
			}
		}

		Gclose( F );
	}
}

//-------------------------Generating map by the bitmap for missions-------------------//
void GenerateMapForMission( char* Relief, char* Ground, char* Mount, char* Soft );

void TestGenMap()
{
	//GenerateMapForMission("MsTools\\Bitmaps\\Relief.bmp",
	//					  "MsTools\\Bitmaps\\Ground.bmp",
	//					  "MsTools\\Bitmaps\\Mount.bmp",
	//					  "MsTools\\Bitmaps\\Soft.bmp");
}

void DelWhitePt( byte* Data, int x, int y )
{
	if ( x >= 0 && y >= 0 && x < 256 && y < 256 )
	{
		int pos = ( x + ( y << 8 ) ) * 3;
		int R = Data[pos];
		int G = Data[pos + 1];
		int B = Data[pos + 2];
		if ( R == G&&R == B&&R > 10 )
		{
			Data[pos] = 0;
			Data[pos + 1] = 0;
			Data[pos + 2] = 0;
		}
	}
}

void ReadHiMap( char* Name, byte** Data, byte Default )
{
	BMPformat BM;
	byte* Data24;
	*Data = new byte[65536];
	memset( *Data, 0, 65536 );
	if ( ReadBMP24( Name, &BM, &Data24 ) )
	{
		int pos = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			( *Data )[i] = Data24[pos + 2];
			pos += 3;
		}
		free( Data24 );
	}
}

void DelWhiteSpot( byte* Data, int x, int y )
{
	DelWhitePt( Data, x, y );

	DelWhitePt( Data, x - 1, y );
	DelWhitePt( Data, x + 1, y );
	DelWhitePt( Data, x, y - 1 );
	DelWhitePt( Data, x, y + 1 );

	DelWhitePt( Data, x - 1, y - 1 );
	DelWhitePt( Data, x - 1, y + 1 );
	DelWhitePt( Data, x + 1, y - 1 );
	DelWhitePt( Data, x + 1, y + 1 );

	DelWhitePt( Data, x - 2, y );
	DelWhitePt( Data, x + 2, y );
	DelWhitePt( Data, x, y - 2 );
	DelWhitePt( Data, x, y + 2 );

	DelWhitePt( Data, x - 2, y - 1 );
	DelWhitePt( Data, x - 2, y + 1 );
	DelWhitePt( Data, x + 2, y - 1 );
	DelWhitePt( Data, x + 2, y + 1 );
	DelWhitePt( Data, x - 1, y - 2 );
	DelWhitePt( Data, x + 1, y - 2 );
	DelWhitePt( Data, x - 1, y + 2 );
	DelWhitePt( Data, x + 1, y + 2 );
}

void DelBorder( byte* Data )
{
	int pos = 0;
	byte* Data1 = new byte[256 * 256 * 3];
	memcpy( Data1, Data, 256 * 256 * 3 );
	for ( int i = 0; i < 65536; i++ )
	{
		byte B = Data1[pos];
		byte G = Data1[pos + 1];
		byte R = Data1[pos + 2];
		if ( !( R || G || B ) )DelWhiteSpot( Data, i & 255, i >> 8 );
		pos += 3;
	};
	free( Data1 );
}

void SoftZones( char* Name, byte* VHI )
{
	byte* Data;
	ReadHiMap( Name, &Data, 0 );
	int* Vertices = NULL;
	int NVert = 0;
	int MaxVert = 0;

	for ( int v = 0; v < MaxPointIndex; v++ )
	{
		int x = GetTriX( v );
		int y = GetTriY( v );
		if ( x > 64 && y > 64 )
		{
			int H = SqInt( Data, x, y );
			if ( H > 2000 )
			{
				if ( NVert >= MaxVert )
				{
					MaxVert += 4096;
					Vertices = (int*) realloc( Vertices, MaxVert * 4 );
				};
				Vertices[NVert] = v;
				NVert++;
			}
		}
	}

	free( Data );

	//softing
	if ( NVert )
	{
		short* VertHi = new short[NVert];
		byte*  VHIT = new byte[NVert];
		for ( int j = 0; j < 50; j++ )
		{
			for ( int i = 0; i < NVert; i++ )
			{
				int vert = Vertices[i];
				int Lin = vert / VertInLine;
				int Lpos = vert%VertInLine;
				if ( Lin > 0 && Lpos > 0 && Lpos < VertInLine - 1 && Lin < MaxTH - 1 )
				{
					if ( Lpos & 1 )
					{
						VertHi[i] = ( THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
							THMap[vert + 1] + THMap[vert - VertInLine + 1] + THMap[vert - VertInLine - 1] ) / 6;
						VHIT[i] = ( VHI[vert + VertInLine] + VHI[vert - VertInLine] + VHI[vert - 1] +
							VHI[vert + 1] + VHI[vert - VertInLine + 1] + VHI[vert - VertInLine - 1] ) / 6;
					}
					else
					{
						VertHi[i] = ( THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
							THMap[vert + 1] + THMap[vert + VertInLine + 1] + THMap[vert + VertInLine - 1] ) / 6;
						VHIT[i] = ( VHI[vert + VertInLine] + VHI[vert - VertInLine] + VHI[vert - 1] +
							VHI[vert + 1] + VHI[vert + VertInLine + 1] + VHI[vert + VertInLine - 1] ) / 6;
					}
				}
			}

			for ( int i = 0; i < NVert; i++ )
			{
				int vi = Vertices[i];
				THMap[vi] = VertHi[i];
				VHI[vi] = VHIT[i];
			}
		}
		if ( Vertices )
		{
			free( Vertices );
		}
	}
}

void SoftAllMap()
{
	for ( int vert = 0; vert < MaxPointIndex; vert++ )
	{
		int Lin = vert / VertInLine;
		int Lpos = vert%VertInLine;
		if ( Lin > 0 && Lpos > 0 && Lpos < VertInLine - 1 && Lin < MaxTH - 1 )
		{
			int VHU = THMap[vert - VertInLine];
			int VH = THMap[vert];
			if ( VHU < VH )
			{
				if ( Lpos & 1 )
				{
					THMap[vert] = ( THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
						THMap[vert + 1] + THMap[vert - VertInLine + 1] + THMap[vert - VertInLine - 1] ) / 6;
				}
				else
				{
					THMap[vert] = ( THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
						THMap[vert + 1] + THMap[vert + VertInLine + 1] + THMap[vert + VertInLine - 1] ) / 6;
				}
			}
		}
	}
}

void SetFractalTexture();

void ClearAllZones();

void GenerateMapForMission( char* Relief, char* Ground, char* Mount, char* Soft )
{
	GMAP.Allocate();
	LockBars.Clear();
	UnLockBars.Clear();
	SetupInfMessage( "Generating map..." );
	SetNextInfStage( "Creating water" );
	DeleteAllSprites();
	ClearAllZones();

	for ( int u = 0; u < MAXOBJECT; u++ )
	{
		OneObject* OB = Group[u];
		if ( OB )
		{
			OB->Die();
			if ( OB )
			{
				OB->Sdoxlo = 6000;
			}
		}
	}

	byte* BData;
	BMPformat BM;

	memset( TexMap, 0, ( MaxTH + 1 )*MaxTH );

	if ( ReadBMP24( Relief, &BM, &BData ) )
	{
		byte* REND = new byte[65537];
		int pos = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			byte B = BData[pos];
			byte G = BData[pos + 1];
			byte R = BData[pos + 2];
			if ( R < 10 && G < 10 && B>240 )
			{
				REND[i] = 0;
			}
			else
			{
				REND[i] = 255;
			}
			pos += 3;
		}

		//softing the terrain
		byte* REND1 = new byte[65537];
		memcpy( REND1, REND, 65537 );
		int NN = 30 >> ( ADDSH - 1 );
		for ( int j = 0; j < NN; j++ )
		{
			int Pos = 257;
			for ( int y = 1; y < 255; y++ )
			{
				for ( int x = 1; x < 255; x++ )
				{
					//REND1[Pos]=((REND[Pos]<<2)+REND[Pos-1]+REND[Pos+1]+REND[Pos-256]+REND[Pos+256])>>3;
					REND1[Pos] = ( ( REND[Pos] << 3 ) + ( REND[Pos] << 2 ) + REND[Pos - 1] + REND[Pos + 1] + REND[Pos - 256] + REND[Pos + 256] ) >> 4;
					Pos++;
				}
				Pos += 2;
			}

			memcpy( REND, REND1, 65537 );

			for ( int y = 0; y < 256; y++ )
			{
				REND[y] = REND[y + 256];
				REND[y + 256 * 255] = REND[y + 256 * 254];
				int p = y << 8;
				REND[p] = REND[p + 1];
				REND[p + 255] = REND[p + 254];
			}
		}

		//creating water
		int B0 = 15;
		int B2 = ( 120 - B0 )*( 120 - B0 );
		memset( WaterBright, 120, WMPSIZE );
		for ( int y = 0; y <= msy + 1; y++ )
		{
			for ( int x = 0; x <= msx + 1; x++ )
			{
				int pos = x + y*MaxWX;
				int VAL = SqInt( REND, x << 5, y << 5 ) - 2048;
				//int VAL1=SqInt(REND2,x<<5,y<<5)-2048;
				int Deep = 120 - ( VAL >> 4 );
				int BRI = 120 + ( VAL / 20 ) + ( mrand() & 7 ) - 3;
				if ( BRI < 0 )BRI = 0;
				if ( BRI > 120 )BRI = 120;
				BRI = 120 - ( 120 - BRI )*( 120 - BRI ) * 120 / B2;
				//if(BRI>60)BRI=60+(BRI>>1);
				//else BRI=BRI*2-60;
				if ( BRI > 120 )BRI = 120;
				if ( BRI < 0 )BRI = 0;
				if ( Deep < 0 )Deep = 0;
				if ( Deep > 255 )Deep = 255;
				if ( ADDSH == 3 )
				{
					if ( Deep < 67 )
						Deep = 67;
					if ( Deep > 128 + 62 )
						Deep = 128 + 62;
					WaterDeep[pos] = ( ( Deep - 128 ) << 1 ) + 128;
				}
				else
				{
					WaterDeep[pos] = ( ( Deep - 128 ) >> ( 2 - ADDSH ) ) + 128;
				}
				WaterBright[pos] = BRI;
			}
		}

		//creating cost line
		byte CostTex[40][8];
		byte NTex[40];
		int NTLines;
		int Divisor;
		if ( !( GetKeyState( VK_CONTROL ) & 0x8000 ) )
			SetFractalTexture();
		SetNextInfStage( "Creating cost" );
		GFILE* f = Gopen( "Terrain\\cost.dat", "r" );
		if ( f )
		{
			int z = Gscanf( f, "%d%d", &NTLines, &Divisor );
			if ( z == 2 )
			{
				//				assert(NTLines<40);
				for ( int j = 0; j < NTLines; j++ )
				{
					int a;
					z = Gscanf( f, "%d", &a );
					if ( z != 1 )ErrM( "Invalid file Terrain\\cost.dat" );
					NTex[j] = a;
					int b;
					for ( int p = 0; p < a; p++ )
					{
						Gscanf( f, "%d", &b );
						CostTex[j][p] = b;
					};
				};
			};
			Gclose( f );
			int maxx = ( msx << 5 ) + 64;
			for ( int v = 0; v < MaxPointIndex; v++ )
			{
				int x = GetTriX( v );
				int y = GetTriY( v );
				if ( x > 0 && y > 0 && x < maxx&&y < maxx )
				{
					int VAL = SqInt( REND, x, y ) - 2048;
					int tid = VAL / Divisor;
					if ( tid < 0 )tid = 0;
					if ( tid < NTLines )
					{
						TexMap[v] = CostTex[tid][( int( mrand() )*int( NTex[tid] ) ) >> 15] + 128;
						//MarkPointToDraw(v);
					}
					else
					{
						//TexMap[v]=0;
					};
				};
			};
		};
		//Creating mountains
		LoadRandomHills();
		SetNextInfStage( "Generating hills" );
		DelBorder( BData );
		memset( THMap, 0, ( MaxTH + 1 )*MaxTH * 2 );
		GMAP.Clear();
		byte* HiMap;
		ReadHiMap( Mount, &HiMap, 255 );
		for ( int i = 0; i < 300000; i++ )
		{
			int tm = GetTickCount() & 1023;
			int x = ( rand() + tm ) & 255;
			int y = ( rand() + tm ) & 255;
			if ( x > 3 && y > 3 && x < 251 && y < 251 )
			{
				pos = ( x + ( y << 8 ) ) * 3;
				int R = BData[pos];
				int G = BData[pos + 1];
				int B = BData[pos + 2];
				if ( R == B&&R == G&&R > 10 )
				{
					int xv = ( x*msx * 32 ) >> 8;
					int yv = ( y*msx * 32 ) >> 8;
					GMAP.MakeHillSpot( xv, yv, 180, HiMap );
					DelWhitePt( BData, x, y );
					DelWhitePt( BData, x - 1, y );
					DelWhitePt( BData, x + 1, y );
					DelWhitePt( BData, x, y - 1 );
					DelWhitePt( BData, x, y + 1 );

					DelWhitePt( BData, x - 1, y - 1 );
					DelWhitePt( BData, x - 1, y + 1 );
					DelWhitePt( BData, x + 1, y - 1 );
					DelWhitePt( BData, x + 1, y + 1 );

				};
			};
		};

		free( HiMap );
		SetNextInfStage( "Softing" );
		SoftZones( Soft, GMAP.VertHi );
		SetNextInfStage( "Texturing" );
		MFIELDS[0].ClearMaps();
		CreateLandLocking( 120, false );
		PaintAllMap( GMAP.VertHi, GMAP.VertType, PHILL, 1 );
		int LesX[4096];
		int LesY[4096];
		int NLes = 0;
		pos = 0;
		for ( int i = 0; i < 65536; i++ )
		{
			int pos3 = pos + pos + pos;
			int B = BData[pos3];
			int G = BData[pos3 + 1];
			int R = BData[pos3 + 2];
			if ( B < 10 && R < 10 && G>100 && NLes < 4096 )
			{
				int x = ( ( pos & 255 )*msx * 32 ) >> 8;
				int y = ( ( pos >> 8 )*msx * 32 ) >> 8;
				LesX[NLes] = x;
				LesY[NLes] = y;
				NLes++;
			};
			pos = ( pos + 33 ) & 65535;
		};
		GenerateInPoints( "Pieces\\LESU0.LST", LesX, LesY, NLes, 200 );
		SetNextInfStage( "Generating background" );

		GenerateByStyle( "Pieces\\Empty.sty" );

		for ( int i = 0; i < MaxPointIndex; i++ )
		{
			TexMap[i] &= 127;
		};
		byte* HiGround;
		ReadHiMap( Ground, &HiGround, 0 );
		int maxx = ( msx << 5 ) + 64;
		for ( int v = 0; v < MaxPointIndex; v++ )
		{
			int x = GetTriX( v );
			int y = GetTriY( v );
			//if(x>0&&y>0&&x<maxx&&y<maxx){
			THMap[v] += ( SqInt( HiGround, x, y ) * 150 ) >> 12;
			int VAL = SqInt( REND, x, y ) - 2048;
			if ( VAL > 32 )
			{
				THMap[v] += ( ( VAL*VAL ) >> 14 ) * 24 / 100;
			};
			//};
		};
		free( HiGround );
		SoftAllMap();
		SoftAllMap();
		SetNextInfStage( "Preparing to render" );
		ClearRender();
		CreateMiniMap();
		for ( int i = 0; i < MaxSprt; i++ )
		{
			OneSprite* OS = Sprites + i;
			if ( OS->Enabled )
			{
				OS->z = GetHeight( OS->x, OS->y );
			};
		};
		CreateInfoMap();
		NATIONS[7].Geology = true;
		free( BData );
		free( REND );
		free( REND1 );
	};
	GMAP.Free();

};
//----------------------------SAMPLES ENGINE-------------------//
struct SampleRoot
{
	byte RootType;
	byte Direction;
	short x;
	short y;
	DWORD AttachMask;
	DWORD Reserved;
};
struct SampleSrc
{
	char Name[32];
	int dx;
	int dy;
	int NRoots;
	SampleRoot* ROOT;
	short* xi;
	short* yi;
	int Npt;
};
class SamplesSet
{
public:
	int NSmp;
	SampleSrc* SSET;
	char CurrentSet[32];
	bool Passive;

	void Draw();
	void NewSet( char* Name );
	void SetActiveSet( char* Name );
	void AddSquare( int x, int y );
	void DelSquare( int x, int y );
	void EditRootParam( int x, int y );
	void ClearAll();
	SamplesSet();
};
SamplesSet::SamplesSet()
{
	NSmp = 0;
	SSET = NULL;
	Passive = 0;
	CurrentSet[0] = 0;
};
void SamplesSet::ClearAll()
{
	for ( int i = 0; i < NSmp; i++ )
	{
		if ( SSET[i].xi )free( SSET[i].xi );
		if ( SSET[i].yi )free( SSET[i].yi );
		if ( SSET[i].ROOT )free( SSET[i].ROOT );
	};
	NSmp = 0;
	SSET = NULL;
};
void SamplesSet::AddSquare( int x, int y )
{
	if ( CurrentSet[0] )
	{
		for ( int i = 0; i < NSmp; i++ )if ( !_stricmp( SSET[i].Name, CurrentSet ) )
		{
			int n = SSET[i].Npt;
			for ( int j = 0; j < n; j++ )if ( SSET[i].xi[j] == x&&SSET[i].yi[j] == y )return;
			SSET[i].Npt++;
			SSET[i].xi = (short*) realloc( SSET[i].xi, SSET[i].Npt << 1 );
			SSET[i].yi = (short*) realloc( SSET[i].yi, SSET[i].Npt << 1 );
			SSET[i].xi[SSET[i].Npt - 1] = x;
			SSET[i].yi[SSET[i].Npt - 1] = y;
			return;
		};
	};
};
void SamplesSet::DelSquare( int x, int y )
{
	if ( CurrentSet[0] )
	{
		for ( int i = 0; i < NSmp; i++ )if ( !_stricmp( SSET[i].Name, CurrentSet ) )
		{
			int n = SSET[i].Npt;
			for ( int j = 0; j < n; j++ )if ( SSET[i].xi[j] == x&&SSET[i].yi[j] == y )
			{
				if ( j < n - 1 )
				{
					memcpy( SSET[i].xi + j, SSET[i].xi + j + 1, ( n - j - 1 ) << 1 );
					memcpy( SSET[i].yi + j, SSET[i].yi + j + 1, ( n - j - 1 ) << 1 );
				};
				SSET[i].Npt--;
			};
			n = SSET[i].NRoots;
			for ( int j = 0; j < n; j++ )if ( SSET[i].ROOT[j].x == x&&SSET[i].ROOT[j].y == y )
			{
				if ( j < n - 1 )
				{
					memcpy( SSET[i].ROOT + j, SSET[i].ROOT + j + 1, ( n - j - 1 ) * sizeof SampleRoot );
				};
				SSET[i].NRoots--;

			};
			return;
		};
	};
};
#define WNDLX 450
#define WNDLY 300
extern int ItemChoose;
void StdKeys();
int prevRoot = 0;
void SamplesSet::EditRootParam( int x, int y )
{
	DialogsSystem DSS( ( RealLx - WNDLX ) >> 1, ( RealLy - WNDLY ) >> 1 );
	int x0 = 150;
	int y0 = 150;
	int DD = 40;

	TextButton* U = DSS.addTextButton( NULL, x0, y0 - DD, "U", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* D = DSS.addTextButton( NULL, x0, y0 + DD, "D", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* L = DSS.addTextButton( NULL, x0 - DD, y0, "L", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* R = DSS.addTextButton( NULL, x0 + DD, y0, "R", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* LU = DSS.addTextButton( NULL, x0 - DD, y0 - DD, "LU", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* RU = DSS.addTextButton( NULL, x0 + DD, y0 - DD, "RU", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* LD = DSS.addTextButton( NULL, x0 - DD, y0 + DD, "LD", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* RD = DSS.addTextButton( NULL, x0 + DD, y0 + DD, "RD", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* DIRSET[8] = { U,RU,R,RD,D,LD,L,LU };

	x0 = 300;
	TextButton* mU = DSS.addTextButton( NULL, x0, y0 - DD, "U", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mD = DSS.addTextButton( NULL, x0, y0 + DD, "D", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mL = DSS.addTextButton( NULL, x0 - DD, y0, "L", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mR = DSS.addTextButton( NULL, x0 + DD, y0, "R", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mLU = DSS.addTextButton( NULL, x0 - DD, y0 - DD, "LU", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mRU = DSS.addTextButton( NULL, x0 + DD, y0 - DD, "RU", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mLD = DSS.addTextButton( NULL, x0 - DD, y0 + DD, "LD", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mRD = DSS.addTextButton( NULL, x0 + DD, y0 + DD, "RD", &WhiteFont, &YellowFont, &YellowFont, 1 );
	TextButton* mSET[8] = { mU,mRU,mR,mRD,mD,mLD,mL,mLU };

	TextButton* HDR = DSS.addTextButton( NULL, WNDLX / 2, 20, CurrentSet, &YellowFont, &YellowFont, &YellowFont, 1 );
	TextButton* Type = DSS.addTextButton( NULL, WNDLX / 2, WNDLY - 35, "Type:", &YellowFont, &YellowFont, &YellowFont, 2 );
	char CRT[32] = "0";
	InputBox* IBT = DSS.addInputBox( NULL, WNDLX / 2, WNDLY - 35, CRT, 8, 80, 20, &YellowFont, &WhiteFont );
	ItemChoose = -1;
	DWORD MASK = 0;
	byte CurDir = 0;
	byte CurType = prevRoot;
	//search for existing root
	for ( int i = 0; i < NSmp; i++ )if ( !_stricmp( SSET[i].Name, CurrentSet ) )
	{
		int n = SSET[i].NRoots;
		for ( int j = 0; j < n; j++ )if ( SSET[i].ROOT[j].x == x&&SSET[i].ROOT[j].y == y )
		{
			MASK = SSET[i].ROOT[j].AttachMask;
			CurDir = SSET[i].ROOT[j].Direction;
			CurType = SSET[i].ROOT[j].RootType;
		};
	};
	sprintf( CRT, "%d", CurType );
	do
	{
		DrawStdBar( ( RealLx - WNDLX ) >> 1, ( RealLy - WNDLY ) >> 1, ( RealLx + WNDLX ) >> 1, ( RealLy + WNDLY ) >> 1 );
		for ( int i = 0; i < 8; i++ )
		{
			DIRSET[i]->PassiveFont = CurDir == i ? &RedFont : &YellowFont;
			mSET[i]->PassiveFont = MASK&( 1 << i ) ? &RedFont : &YellowFont;
			if ( mSET[i]->MouseOver&&Lpressed )
			{
				if ( MASK&( 1 << i ) )
				{
					MASK &= ~( 1 << i );
				}
				else MASK |= 1 << i;
				Lpressed = 0;
			};
			if ( DIRSET[i]->MouseOver&&Lpressed )
			{
				CurDir = i;
				Lpressed = 0;
			};
		};
		ProcessMessages();
		StdKeys();
		DSS.ProcessDialogs();
		DSS.RefreshView();
		CurType = atoi( CRT );
	} while ( ItemChoose == -1 );
	if ( ItemChoose == mcmOk )
	{
		for ( int i = 0; i < NSmp; i++ )if ( !_stricmp( SSET[i].Name, CurrentSet ) )
		{
			int n = SSET[i].NRoots;
			for ( int j = 0; j < n; j++ )if ( SSET[i].ROOT[j].x == x&&SSET[i].ROOT[j].y == y )
			{
				SSET[i].ROOT[j].AttachMask = MASK;
				SSET[i].ROOT[j].Direction = CurDir;
				SSET[i].ROOT[j].RootType = CurType;
				return;
			};
			//root not found!
			SSET[i].ROOT = (SampleRoot*) realloc( SSET[i].ROOT, ( SSET[i].NRoots + 1 ) * sizeof SampleRoot );
			int cur = SSET[i].NRoots;
			SSET[i].NRoots++;
			SSET[i].ROOT[cur].AttachMask = MASK;
			SSET[i].ROOT[cur].Direction = CurDir;
			SSET[i].ROOT[cur].Reserved = 0;
			SSET[i].ROOT[cur].RootType = CurType;
			prevRoot = CurType;
			SSET[i].ROOT[cur].x = x;
			SSET[i].ROOT[cur].y = y;
			return;
		};
	};
};
int DDDXX[8] = { 0,1,1,1,0,-1,-1,-1 };
int DDDYY[8] = { -1,-1,0,1,1,1,0,-1 };
void CBar( int x, int y, int Lx, int Ly, byte c );
void xLine( int x, int y, int x1, int y1, byte c );
void SamplesSet::Draw()
{
	int dx = mapx << 5;
	int dy = mapy << 4;
	for ( int i = 0; i < NSmp; i++ )
	{
		SampleSrc* SS = SSET + i;
		bool CURR = !_stricmp( CurrentSet, SS->Name );
		for ( int j = 0; j < SS->Npt; j++ )
		{
			int x0 = ( SS->xi[j] << 8 ) - dx;
			int y0 = ( SS->yi[j] << 7 ) - dy;
			Xbar( x0, y0, 256, 128, 0x9B );
			if ( CURR )Xbar( x0 + 1, y0 + 1, 254, 126, 0x9B );
			ShowString( x0 + 4, y0 + 4, SS->Name, &SmallWhiteFont );
		};
		for ( int j = 0; j < SS->NRoots; j++ )
		{
			int x0 = ( SS->ROOT[j].x << 8 ) - dx;
			int y0 = ( SS->ROOT[j].y << 7 ) - dy;
			DWORD MSK = SS->ROOT[j].AttachMask;
			for ( int q = 0; q < 8; q++ )if ( MSK&( 1 << q ) )
			{
				int dx = DDDXX[q];
				int dy = DDDYY[q];
				xLine( x0 + 128, y0 + 64, x0 + 128 + dx * 32, y0 + 64 + dy * 16, 0xBF );
			};
			CBar( x0 + 128 - 4, y0 + 64 - 4, 8, 8, 0x9B );
			ShowString( x0 + 4, y0 + 20, SS->Name, &SmallRedFont );
			if ( SS->ROOT[j].Direction < 8 )
			{
				byte dd = SS->ROOT[j].Direction;
				int dx = DDDXX[dd];
				int dy = DDDYY[dd];
				xLine( x0 + 128, y0 + 64, x0 + 128 + dx * 32, y0 + 64 + dy * 16, 0x9B );
			};
		};
	};
};
void SamplesSet::NewSet( char* name )
{
	for ( int i = 0; i < NSmp; i++ )if ( !_stricmp( SSET[i].Name, name ) )return;
	SSET = (SampleSrc*) realloc( SSET, ( NSmp + 1 ) * sizeof SampleSrc );
	SampleSrc* SS = SSET + NSmp;
	strcpy( SS->Name, name );
	strcpy( CurrentSet, name );
	SS->NRoots = 0;
	SS->ROOT = NULL;
	SS->Npt = 0;
	SS->xi = NULL;
	SS->yi = NULL;
	NSmp++;
}

void SamplesSet::SetActiveSet( char* name )
{
	//for(int i=0;i<NSmp;i++)if(!_stricmp(SSET[i].Name,name))return;
	strcpy( CurrentSet, name );
}

SamplesSet SAMSET;
bool SamSetMode = 0;
bool EditRoot = 0;

void HandleSMSChat( char* Mess )
{
	if ( !_stricmp( Mess, "::dosmp" ) )
	{
		SamSetMode = !SamSetMode;
		SAMSET.Passive = 0;
	}
	else
	{
		if ( !_stricmp( Mess, "::root" ) )
		{
			EditRoot = !EditRoot;
			SAMSET.Passive = 0;
		}
		else
		{
			if ( !_stricmp( Mess, "::pass" ) )
			{
				SAMSET.Passive = !SAMSET.Passive;
			}
			else
			{
				char *cmd = new char[128];
				char *par = new char[128];
				int z = sscanf( Mess, "%s%s", cmd, par );
				if ( z == 2 )
				{
					if ( !_stricmp( cmd, "::goto" ) )
					{
						SAMSET.SetActiveSet( par );
					}
					else
					{
						if ( !_stricmp( cmd, "::new" ) )
						{
							SAMSET.NewSet( par );
						}
					}//::goto
				}
				delete[] cmd;
				delete[] par;
			}//::pass
		}//::root
	}//::dosmp
}

void HandlwSMSMouse()
{
	if ( SAMSET.Passive )
	{
		return;
	}

	if ( SamSetMode )
	{
		int xx = ( ( mapx * 32 ) + mouseX ) / 256;
		int yy = ( ( mapy * 16 ) + mouseY ) / 128;

		if ( SAMSET.CurrentSet[0] )
		{
			if ( Lpressed )
			{
				if ( EditRoot )
				{
					SAMSET.EditRootParam( xx, yy );
				}
				else
				{
					SAMSET.AddSquare( xx, yy );
				}
				Lpressed = 0;
			}

			if ( Rpressed )
			{
				SAMSET.DelSquare( xx, yy );
				Rpressed = 0;
			}
		}
	}
}

void AutoSMSSet()
{
	if ( SAMSET.Passive )
	{
		return;
	}

	if ( SamSetMode )
	{
		int N = ( MaxTH + 1 )*MaxTH;
		for ( int i = 0; i < N; i++ )
		{
			if ( TexMap[i] )
			{
				int x = GetTriX( i ) / 256;
				int y = GetTriY( i ) / 256;
				SAMSET.AddSquare( x, y );
			}
		}
	}
}

void DrawSMS()
{
	if ( SamSetMode )
	{
		SAMSET.Draw();
	}
}

void SaveSMSInPieces( ResFile F, int keyX, int keyY, int x0, int y0, int x1, int y1 )
{
	int Nsq = 0;
	int Nrt = 0;

	for ( int i = 0; i < SAMSET.NSmp; i++ )
	{
		SampleSrc* SS = SAMSET.SSET + i;

		for ( int j = 0; j < SS->Npt; j++ )
		{
			int xx = ( SS->xi[j] * 256 ) + 128;
			int yy = ( SS->yi[j] * 128 ) + 64;
			if ( xx > x0 && yy > y0 && xx < x1 && yy < y1 )
			{
				Nsq++;
			}
		}

		for ( int j = 0; j < SS->NRoots; j++ )
		{
			int xx = ( SS->ROOT[j].x * 256 ) + 128;
			int yy = ( SS->ROOT[j].y * 128 ) + 64;
			if ( xx > x0 && yy > y0 && xx < x1 && yy < y1 )
			{
				Nrt++;
			}
		}
	}

	int i = 'SMSP';
	RBlockWrite( F, &i, 4 );
	int sz = 4 + 4 + 4 + Nsq * 2 + Nrt * sizeof( SampleRoot );
	RBlockWrite( F, &sz, 4 );
	RBlockWrite( F, &Nsq, 2 );
	RBlockWrite( F, &Nrt, 2 );
	int dx = keyX & 255;
	RBlockWrite( F, &dx, 2 );
	int dy = keyY & 255;
	RBlockWrite( F, &dy, 2 );
	keyX = ( keyX - dx ) >> 8;
	keyY = ( keyY - dy ) >> 8;
	for ( int i = 0; i < SAMSET.NSmp; i++ )
	{
		SampleSrc* SS = SAMSET.SSET + i;
		for ( int j = 0; j < SS->Npt; j++ )
		{
			int xx = ( SS->xi[j] << 8 ) + 128;
			int yy = ( SS->yi[j] << 7 ) + 64;
			if ( xx > x0&&yy > y0&&xx < x1&&yy < y1 )
			{
				dx = SS->xi[j] - keyX;
				RBlockWrite( F, &dx, 1 );
				dx = SS->yi[j] - keyY;
				RBlockWrite( F, &dx, 1 );
			};
		};
	};
	for ( int i = 0; i < SAMSET.NSmp; i++ )
	{
		SampleSrc* SS = SAMSET.SSET + i;
		for ( int j = 0; j < SS->NRoots; j++ )
		{
			int xx = ( SS->ROOT[j].x << 8 ) + 128;
			int yy = ( SS->ROOT[j].y << 7 ) + 64;
			if ( xx > x0&&yy > y0&&xx < x1&&yy < y1 )
			{
				SampleRoot SR = SS->ROOT[j];
				SR.x -= keyX;
				SR.y -= keyY;
				RBlockWrite( F, &SR, sizeof SR );
			};
		};
	};
}

void SaveSMSInMap( ResFile F )
{
	int sz = 4 + 4;
	for ( int i = 0; i < SAMSET.NSmp; i++ )
	{
		SampleSrc* SS = SAMSET.SSET + i;
		sz += 32 + 4 + 4 + 4 * SS->Npt + SS->NRoots * sizeof( SampleRoot );
	};
	int i = 'SMSP';
	RBlockWrite( F, &i, 4 );
	RBlockWrite( F, &sz, 4 );
	RBlockWrite( F, &SAMSET.NSmp, 4 );
	for ( int i = 0; i < SAMSET.NSmp; i++ )
	{
		RBlockWrite( F, SAMSET.SSET[i].Name, 32 );
		RBlockWrite( F, &SAMSET.SSET[i].Npt, 4 );
		RBlockWrite( F, &SAMSET.SSET[i].NRoots, 4 );
		for ( int j = 0; j < SAMSET.SSET[i].Npt; j++ )
		{
			RBlockWrite( F, &SAMSET.SSET[i].xi[j], 2 );
			RBlockWrite( F, &SAMSET.SSET[i].yi[j], 2 );
		};
		for ( int j = 0; j < SAMSET.SSET[i].NRoots; j++ )
		{
			RBlockWrite( F, SAMSET.SSET[i].ROOT + j, sizeof SampleRoot );
		};
	};
};
void ClearSMS()
{
	SAMSET.ClearAll();
	memset( &SAMSET, 0, sizeof SAMSET );
};
void LoadSMSInMap( ResFile F )
{
	ClearSMS();
	RBlockRead( F, &SAMSET.NSmp, 4 );
	if ( SAMSET.NSmp )
	{
		SAMSET.SSET = new SampleSrc[SAMSET.NSmp];
		memset( SAMSET.SSET, 0, SAMSET.NSmp * sizeof SampleSrc );
	};
	for ( int i = 0; i < SAMSET.NSmp; i++ )
	{
		RBlockRead( F, SAMSET.SSET[i].Name, 32 );
		RBlockRead( F, &SAMSET.SSET[i].Npt, 4 );
		RBlockRead( F, &SAMSET.SSET[i].NRoots, 4 );
		if ( SAMSET.SSET[i].Npt )
		{
			SAMSET.SSET[i].xi = new short[SAMSET.SSET[i].Npt];
			SAMSET.SSET[i].yi = new short[SAMSET.SSET[i].Npt];
		};
		if ( SAMSET.SSET[i].NRoots )
		{
			SAMSET.SSET[i].ROOT = new SampleRoot[SAMSET.SSET[i].NRoots];
		};
		for ( int j = 0; j < SAMSET.SSET[i].Npt; j++ )
		{
			RBlockRead( F, &SAMSET.SSET[i].xi[j], 2 );
			RBlockRead( F, &SAMSET.SSET[i].yi[j], 2 );
		};
		for ( int j = 0; j < SAMSET.SSET[i].NRoots; j++ )
		{
			RBlockRead( F, SAMSET.SSET[i].ROOT + j, sizeof SampleRoot );
		};
	};
};
#define MaxType 32
struct SampleStamp
{
	char Name[64];
	int NS;
	int maxS;

	int * Coor;
};
struct OneLinkStart
{
	int x;
	int y;
	DWORD Mask;
	byte Type;
};

class LinkSys
{
public:
	SamplesSet LINKS;
	word* Map;
	word* Map2;
	int Nx;
	SampleStamp* STM;
	int NStm;
	OneLinkStart* LIS;
	int NLis;
	int MaxLis;


	word NSMP[MaxType][8];
	word* SMP[MaxType][8];
	byte* SMIDX[MaxType][8];
	void LoadOneLink( char* Name );
	void Init( int Nx, int Ny );
	void Close();
	void LoadSSet( char* list );
	bool GenerateLink( int x, int y, DWORD Mask, byte Type, int BreakProb );
	bool GenerateRandomLink( int BreakProb );
	void SetLockInMap( int x0, int y0, int SmID, int Root );
	bool CheckLockInMap( int x0, int y0, int SmID, int Root );
	void SetLockPt( int x, int y, word Val );
	word GetLockPt( int x, int y );
	void AddStamp( char* Name, int x, int y );
	void ExecuteStamps();
	void ClearStamps();
	void AddLinkStart( int x, int y, DWORD Mask, byte Type );
};
void LinkSys::AddLinkStart( int x, int y, DWORD Mask, byte Type )
{
	if ( NLis >= MaxLis )
	{
		MaxLis += 32;
		LIS = (OneLinkStart*) realloc( LIS, MaxLis * sizeof OneLinkStart );
	};
	LIS[NLis].x = x;
	LIS[NLis].y = y;
	LIS[NLis].Mask = Mask;
	LIS[NLis].Type = Type;
	NLis++;
}

void LinkSys::Init( int x, int y )
{
	memset( &NSMP, 0, sizeof NSMP );
	memset( &SMP, 0, sizeof SMP );
	memset( &SMIDX, 0, sizeof SMIDX );
	Nx = x;
	Map = new word[Nx*Nx];
	memset( Map, 0, Nx*Nx * 2 );
	Map2 = new word[Nx*Nx];
	memset( Map2, 0, Nx*Nx * 2 );
	STM = NULL;
	NStm = 0;
	LIS = NULL;
	NLis = 0;
	MaxLis = 0;
}

void  LinkSys::LoadOneLink( char* Name )
{
	char ccx[64];
	sprintf( ccx, "%s.lnk", Name );
	ResFile F = RReset( ccx );
	if ( F != INVALID_HANDLE_VALUE )
	{
		int i;

		RBlockRead( F, &i, 4 );
		RBlockRead( F, &i, 4 );
		word Nsq, Nrt;
		RBlockRead( F, &Nsq, 2 );
		RBlockRead( F, &Nrt, 2 );
		LINKS.SSET = (SampleSrc*) realloc( LINKS.SSET, ( LINKS.NSmp + 1 ) * sizeof( SampleSrc ) );
		LINKS.NSmp++;
		int cur = LINKS.NSmp - 1;
		short dx, dy;

		RBlockRead( F, &dx, 2 );
		RBlockRead( F, &dy, 2 );
		SampleSrc* SS = LINKS.SSET + cur;
		SS->dx = dx;
		SS->dy = dy;
		SS->Npt = Nsq;
		SS->NRoots = Nrt;
		strcpy( SS->Name, Name );
		SS->ROOT = new SampleRoot[SS->NRoots];
		SS->xi = new short[Nsq];
		SS->yi = new short[Nsq];

		for ( int j = 0; j < SS->Npt; j++ )
		{
			char xx;
			RBlockRead( F, &xx, 1 );
			SS->xi[j] = xx;
			RBlockRead( F, &xx, 1 );
			SS->yi[j] = xx;
		}

		for ( int j = 0; j < Nrt; j++ )
		{
			RBlockRead( F, SS->ROOT + j, sizeof SampleRoot );
			int rt = SS->ROOT[j].RootType;
			int k = SS->ROOT[j].Direction;
			int nn = NSMP[rt][k];
			SMP[rt][k] = (word*) realloc( SMP[rt][k], ( nn + 1 ) * 2 );
			SMIDX[rt][k] = (byte*) realloc( SMIDX[rt][k], nn + 1 );
			SMP[rt][k][nn] = cur;
			SMIDX[rt][k][nn] = j;
			NSMP[rt][k]++;
		}
		RClose( F );
	}
}

int TypeIDX[64];
int NTypeIDX = 0;

void LinkSys::LoadSSet( char* list )
{
	GFILE* F = Gopen( list, "r" );
	if ( F )
	{
		char cc1[128];
		int z;
		do
		{
			int nn = 0;
			z = Gscanf( F, "%s", cc1 );
			if ( z == 1 )
			{
				LoadOneLink( cc1 );
			}
		} while ( z == 1 );

		Gclose( F );
	}
}

#define NRLPAR 4

struct RLPAR
{
	bool Busy;
	int x0, y0, tt, idx, type;
	int Prev_x0, Prev_y0, Prev_tt, Prev_idx;
	DWORD NEXTMASK;
};

bool LinkSys::GenerateRandomLink( int rr )
{
	//int mrpos=1665;
	int x0 = mrand() % Nx;
	int y0 = mrand() % Nx;
	int nn = mrand() % NTypeIDX;
	return GenerateLink( x0, y0, 0xFF, TypeIDX[nn + nn], TypeIDX[nn + nn + 1] );
}

bool LinkSys::GenerateLink( int param_x0, int param_y0, DWORD Mask, byte Type, int rr )
{
	RLPAR THR[NRLPAR];
	memset( THR, 0xFF, sizeof THR );
	for ( int k = 0; k < NRLPAR; k++ )
	{
		THR[k].Busy = 0;
		THR[k].NEXTMASK = 0;
		THR[k].type = Type;
	}
	THR[0].Busy = 1;
	THR[0].NEXTMASK = 0xFF;
	THR[0].x0 = param_x0;
	THR[0].y0 = param_y0;

	bool First = 1;
	int PrevIDX = -1;
	do
	{
		for ( int p = 0; p < NRLPAR; p++ )if ( THR[p].Busy )
		{
			int NATT = 0;
			bool NoDone = 1;
			do
			{
				int dir = mrand() & 7;
				DWORD NEXTMASK = THR[p].NEXTMASK;
				if ( NEXTMASK&( 1 << dir ) )
				{
					int nn = NSMP[THR[p].type][dir];
					if ( nn )
					{
						int vv = ( mrand()*nn ) >> 15;
						int tt = SMP[THR[p].type][dir][vv];
						int idx = SMIDX[THR[p].type][dir][vv];
						if ( idx != PrevIDX&&CheckLockInMap( THR[p].x0, THR[p].y0, tt, idx ) )
						{
							First = 0;
							CheckHI = 1;
							int x0 = THR[p].x0;
							int y0 = THR[p].y0;

							AddStamp( LINKS.SSET[tt].Name, ( ( x0 - LINKS.SSET[tt].ROOT[idx].x ) << 8 ) + LINKS.SSET[tt].dx,
								( ( y0 - LINKS.SSET[tt].ROOT[idx].y ) << 8 ) + LINKS.SSET[tt].dy );
							CheckHI = 0;
							PrevIDX = idx;
							//search for final index;
							int ifin;
							int nr = LINKS.SSET[tt].NRoots;
							if ( nr == 1 )
							{
								/*
								if(THR[p].Prev_tt!=-1){
									SetLockInMap(THR[p].Prev_x0,THR[p].Prev_y0,THR[p].Prev_tt,THR[p].Prev_idx);
								};
								*/
								if ( tt != -1 )
								{
									SetLockInMap( x0, y0, tt, idx );
								}
								ifin = -1;
							}
							else
							{
								if ( nr == 2 )
								{
									ifin = 1 - idx;
								}
								else
								{
									//search for ifin!=idx<16
									ifin = -1;
									for ( int t = 0; t < nr&&ifin == -1; t++ )
									{
										if ( t != idx&&LINKS.SSET[tt].ROOT[t].RootType < 15 )
										{
											ifin = t;
										}
									}
									int IFIN[3] = { -2,-3,-4 };
									for ( int t = 0; t < nr&&IFIN[0] == -1; t++ )
									{
										if ( t != idx&&t != ifin&&LINKS.SSET[tt].ROOT[t].RootType < 15 )
										{
											IFIN[0] = t;
										}
									}
									for ( int t = 0; t < nr&&IFIN[1] == -1; t++ )
									{
										if ( t != idx&&t != ifin&&t != IFIN[0] && LINKS.SSET[tt].ROOT[t].RootType < 15 )
										{
											IFIN[1] = t;
										}
									}
									for ( int t = 0; t < nr&&IFIN[2] == -1; t++ )
									{
										if ( t != idx&&t != ifin&&t != IFIN[0] && t != IFIN[1] && LINKS.SSET[tt].ROOT[t].RootType < 15 )
										{
											IFIN[2] = t;
										}
									}
									for ( int vx = 0; vx < 3; vx++ )if ( IFIN[vx] >= 0 )
									{
										for ( int s = 0; s < NRLPAR; s++ )if ( !THR[s].Busy )
										{
											THR[s].Busy = 1;
											THR[s].NEXTMASK = LINKS.SSET[tt].ROOT[IFIN[vx]].AttachMask;
											THR[s].x0 = x0 + LINKS.SSET[tt].ROOT[IFIN[vx]].x - LINKS.SSET[tt].ROOT[idx].x;
											THR[s].y0 = y0 + LINKS.SSET[tt].ROOT[IFIN[vx]].y - LINKS.SSET[tt].ROOT[idx].y;
											THR[s].Prev_x0 = -1;
											THR[s].Prev_y0 = -1;
											THR[s].Prev_tt = -1;
											THR[s].type = LINKS.SSET[tt].ROOT[IFIN[vx]].RootType;
											s = NRLPAR;
										}
									}
								}
							}
							for ( int tv = 0; tv < nr; tv++ )if ( LINKS.SSET[tt].ROOT[tv].RootType > 15 )
							{
								AddLinkStart( x0 + LINKS.SSET[tt].ROOT[tv].x - LINKS.SSET[tt].ROOT[idx].x,
									y0 + LINKS.SSET[tt].ROOT[tv].y - LINKS.SSET[tt].ROOT[idx].y,
									LINKS.SSET[tt].ROOT[tv].AttachMask, LINKS.SSET[tt].ROOT[tv].RootType );
							}
							if ( ifin != -1 )
							{
								int tx = x0;
								int ty = y0;
								THR[p].x0 = x0 + LINKS.SSET[tt].ROOT[ifin].x - LINKS.SSET[tt].ROOT[idx].x;
								THR[p].y0 = y0 + LINKS.SSET[tt].ROOT[ifin].y - LINKS.SSET[tt].ROOT[idx].y;

								NoDone = 0;
								THR[p].NEXTMASK = LINKS.SSET[tt].ROOT[ifin].AttachMask;
								if ( THR[p].Prev_tt != -1 )
								{
									SetLockInMap( THR[p].Prev_x0, THR[p].Prev_y0, THR[p].Prev_tt, THR[p].Prev_idx );
								};

								THR[p].Prev_tt = tt;
								THR[p].Prev_idx = idx;
								THR[p].Prev_x0 = x0;
								THR[p].Prev_y0 = y0;
							}
							else
							{
								return true;
							}

						}
						else
						{
							if ( First )
							{
								return false;
							}
						}
					}
				}
				NATT++;
			} while ( NATT < 50 && NoDone );
		}
		rr--;
	} while ( rr > 0 );
	for ( int p = 0; p < NRLPAR; p++ )if ( THR[p].Busy&&THR[p].Prev_tt != -1 )
	{
		SetLockInMap( THR[p].Prev_x0, THR[p].Prev_y0, THR[p].Prev_tt, THR[p].Prev_idx );
	}
	return true;
};

//LinkSys LSS;
void TEST_GM()
{

}

word LIDX = 1;
void LinkSys::SetLockInMap( int x0, int y0, int SmID, int Root )
{
	SampleRoot* SR = LINKS.SSET[SmID].ROOT + Root;
	x0 -= SR->x;
	y0 -= SR->y;
	int N = LINKS.SSET[SmID].Npt;
	short* xi = LINKS.SSET[SmID].xi;
	short* yi = LINKS.SSET[SmID].yi;
	for ( int i = 0; i < N; i++ )SetLockPt( x0 + xi[i], y0 + yi[i], SmID );
	LIDX++;
};
bool LinkSys::CheckLockInMap( int x0, int y0, int SmID, int Root )
{
	SampleRoot* SR = LINKS.SSET[SmID].ROOT + Root;
	x0 -= SR->x;
	y0 -= SR->y;
	int N = LINKS.SSET[SmID].Npt;
	short* xi = LINKS.SSET[SmID].xi;
	short* yi = LINKS.SSET[SmID].yi;
	for ( int i = 0; i < N; i++ )if ( GetLockPt( x0 + xi[i], y0 + yi[i] ) )return false;
	return true;
}

void LinkSys::SetLockPt( int x, int y, word Val )
{
	if ( x < 0 || x >= Nx || y < 0 || y >= Nx )
	{
		return;
	}

	if ( Map[x + y*Nx] )
	{
		Map2[x + y*Nx] = LIDX;
		return;
	}
	else
	{
		Map[x + y*Nx] = LIDX;//=Val;
		Map2[x + y*Nx] = LIDX;
	}
}

word LinkSys::GetLockPt( int x, int y )
{
	if ( x < 0 || x >= Nx || y < 0 || y >= Nx )
	{
		return 1;
	}

	return Map[x + y*Nx];
}

void Draw_GRASS();

void GenerateWithStyle( char* terr )
{
	GPROG.SetCurrentStage( 'FTEX' );

	SetFractalTexture();

	LinkSys LSS;

	int NY = 30 << ADDSH;

	LSS.Init( NY, NY );

	int VIDX[512];
	for ( int i = 0; i < NY; i++ )
	{
		VIDX[i] = i * NY;
	}

	//1.
	int vrt = 0;
	for ( int j = 0; j < MaxTH; j++ )
	{
		for ( int i = 0; i < VertInLine; i++ )
		{
			byte tex = TexMap[vrt];
			if ( tex > 127 )
			{
				LSS.SetLockPt( i >> 3, j >> 3, 1 );
			}
			vrt++;
		}
	}

	GPROG.SetCurrentStage( 'LINK' );
	GFILE* F = Gopen( terr, "r" );
	if ( F )
	{
		char cc[128];
		int N;
		int z;
		do
		{
			z = Gscanf( F, "%s%d%d", cc, &N, &NTypeIDX );

			N <<= ADDSH - 1;

			if ( z == 3 )
			{
				for ( int j = 0; j < NTypeIDX; j++ )
				{
					Gscanf( F, "%d%d", &TypeIDX[j + j], &TypeIDX[j + j + 1] );
				}

				//Load all *.smp pieces in given *.lst file
				LSS.LoadSSet( cc );

				for ( int i = 0; i < N; i++ )
				{
					LSS.GenerateRandomLink( 10 );
				}

				for ( int i = 0; i < LSS.NLis; i++ )
				{
					LSS.GenerateLink( LSS.LIS[i].x, LSS.LIS[i].y, LSS.LIS[i].Mask, LSS.LIS[i].Type, 50 );
				}

				LSS.LINKS.ClearAll();
			}
		} while ( z == 3 );

		Gclose( F );

		LSS.ExecuteStamps();
	}
}

void LinkSys::AddStamp( char* Name, int x, int y )
{
	for ( int i = 0; i < NStm; i++ )if ( !_stricmp( Name, STM[i].Name ) )
	{
		if ( STM[i].NS >= STM[i].maxS )
		{
			STM[i].maxS += 32;
			STM[i].Coor = (int*) realloc( STM[i].Coor, STM[i].maxS << 3 );
		};
		int p = STM[i].NS;
		STM[i].NS++;
		STM[i].Coor[p + p] = x;
		STM[i].Coor[p + p + 1] = y;
		return;
	};
	STM = (SampleStamp*) realloc( STM, ( NStm + 1 ) * sizeof SampleStamp );
	strcpy( STM[NStm].Name, Name );
	STM[NStm].Coor = new int[8 * 32];
	STM[NStm].NS = 1;
	STM[NStm].maxS = 32;
	STM[NStm].Coor[0] = x;
	STM[NStm].Coor[1] = y;
	NStm++;
}

void LinkSys::ClearStamps()
{
	for ( int i = 0; i < NStm; i++ )
	{
		free( STM[i].Coor );
	};
	if ( STM )free( STM );
	NStm = 0;
	STM = NULL;
	if ( LIS )free( LIS );
	NLis = 0;
	MaxLis = 0;
}

class SaveBuf
{
public:
	byte* Buf;
	int Pos;
	int Size;
	int RealSize;
	SaveBuf();
	void Init();
	~SaveBuf();
	void Clear();
	void SaveToFile( ResFile f1 );
	void LoadFromFile( ResFile f1 );
};

void FAST_RM_Load( SaveBuf* SB, int x, int y );

byte IsMount;

void LinkSys::ExecuteStamps()
{
	for ( int i = 0; i < NStm; i++ )
	{
		IsMount = 1;
		SaveBuf SB;

		SB.Init();

		_strupr( STM[i].Name );

		if ( strstr( STM[i].Name, "LOW_" ) )
		{
			IsMount = 0;
		}

		ResFile F = RReset( STM[i].Name );
		if ( F != INVALID_HANDLE_VALUE )
		{
			SB.LoadFromFile( F );
			int n = STM[i].NS;
			for ( int j = 0; j < n; j++ )
			{
				CheckHI = 1;
				FAST_RM_Load( &SB, STM[i].Coor[j + j], STM[i].Coor[j + j + 1] );
				CheckHI = 0;
			}

			SB.Clear();

			RClose( F );
		}
	}

	ClearStamps();

	//checking sprite objects
	for ( int i = 0; i < MaxSprt; i++ )
	{
		OneSprite* OS = Sprites + i;
		if ( OS->Enabled )
		{
			if ( OS->z < GetHeight( OS->x, OS->y ) )
			{
				EraseSprite( i );
			}
		}
	}
}

//-----------------OPTIMIZING----------------//

void xBlockRead( SaveBuf* SB, void* Data, int Size );
inline int GET_INT( SaveBuf* SB )
{
	SB->Pos += 4;
	return *( (int*) ( SB->Buf + SB->Pos - 4 ) );
};
inline int GET_DWORD( SaveBuf* SB )
{
	SB->Pos += 4;
	return *( (DWORD*) ( SB->Buf + SB->Pos - 4 ) );
};
inline int GET_BYTE( SaveBuf* SB )
{
	SB->Pos++;
	return *( (byte*) ( SB->Buf + SB->Pos - 1 ) );
};
inline int GET_CHAR( SaveBuf* SB )
{
	SB->Pos++;
	return *( (char*) ( SB->Buf + SB->Pos - 1 ) );
};
inline int GET_SHORT( SaveBuf* SB )
{
	SB->Pos += 2;
	return *( (short*) ( SB->Buf + SB->Pos - 2 ) );
};
inline int GET_WORD( SaveBuf* SB )
{
	SB->Pos += 2;
	return *( (word*) ( SB->Buf + SB->Pos - 2 ) );
}

int FAST_RM_LoadVertices( SaveBuf* SB, int Vsx, int Vsy, int* VIDX, int MaxVIDX )
{
	int Nv;
	bool NPOS = 0;
	bool NNEG = 0;
	int CIdx = 0;

	Nv = GET_INT( SB );

	for ( int i = 0; i < Nv; i++ )
	{
		short vx;
		short vy;
		byte tex;
		byte s1, s2, s3;
		short Height;

		vx = GET_SHORT( SB );
		vy = GET_SHORT( SB );
		vx += Vsx;
		vy += Vsy;

		tex = GET_BYTE( SB );
		s1 = GET_BYTE( SB );
		s2 = GET_BYTE( SB );
		s3 = GET_BYTE( SB );
		Height = GET_SHORT( SB );

		if ( Height > 0 )
		{
			NPOS++;
		}

		if ( Height < 0 )
		{
			NNEG++;
		}

		if ( vx > 0 && vy > 0 && vx < VertInLine - 1 && vy < MaxTH - 1 )
		{
			int v = vx + vy * VertInLine;

			if ( SectMap )
			{
				int StartSect = ( vx >> 1 ) * 6 + vy * SectInLine;

				if ( vx & 1 )
				{
					SectMap[StartSect + 3] = s1;
					SectMap[StartSect + 5] = s2;
					SectMap[StartSect + 2] = s3;
				}
				else
				{
					SectMap[StartSect] = s1;
					SectMap[StartSect + 1] = s2;
					SectMap[StartSect - 2] = s3;
				}
			}

			if ( v >= 0 && v < MaxPointIndex )
			{
				if ( CheckHI )
				{
					if ( !IsMount )
					{
						if ( THMap[v] >= Height )
						{
							if ( v < VertInLine )
							{
								if ( Height < 0 )THMap[v] = 0;
								THMap[v] = Height;
							}
							else
							{
								THMap[v] = Height;
							}

							if ( tex != 0 )
							{
								TexMap[v] = tex + 128;
							}

							if ( CIdx < MaxVIDX )
							{
								VIDX[CIdx] = v;
								CIdx++;
							}
						}
					}
					else
					{
						if ( THMap[v] <= Height )
						{
							if ( v < VertInLine )
							{
								if ( Height < 0 )THMap[v] = 0;
								THMap[v] = Height;
							}
							else
							{
								THMap[v] = Height;
							}

							if ( tex != 0 )
							{
								TexMap[v] = tex + 128;
							}

							if ( CIdx < MaxVIDX )
							{
								VIDX[CIdx] = v;
								CIdx++;
							}
						}
					}
				}
				else
				{
					THMap[v] += Height;
					if ( tex != 0 )
					{
						TexMap[v] = tex + 128;
					}
				}
			}
		}
	}
	return CIdx;
}

void FAST_RM_LoadSprites( SaveBuf*SB, int xc, int yc )
{
	int ns;
	ns = GET_INT( SB );
	int maxx = ( msx << 5 ) - 32;
	int maxy = ( msy << 5 ) - 32;
	for ( int i = 0; i < ns; i++ )
	{
		int x, y;
		word sign, id;
		sign = GET_WORD( SB );
		x = GET_INT( SB );
		y = GET_INT( SB );
		id = GET_WORD( SB );

		x += xc;
		y += yc;

		if ( x > 0 && y > 0 && x < maxx&&y < maxy )
		{
			int z = GetHeight( x, y );
			if ( z > 1024 )
			{
				if ( sign == 'GA' )
					addSpriteAnyway( x, y, &TREES, id );
				else
					if ( sign == 'TS' )
						addSpriteAnyway( x, y, &STONES, id );
					else
						if ( sign == 'OH' )
							addSpriteAnyway( x, y, &HOLES, id );
						else
							if ( sign == 'OC' )
								addSpriteAnyway( x, y, &COMPLEX, id );
			}
		}
	}
}

void FastAddLockBar( word x, word y );
void FastAddUnLockBar( word x, word y );
void FAST_RM_LoadLock( SaveBuf* SB, int xc, int yc )
{
	int NL, NU;
	NL = GET_INT( SB );
	NU = GET_INT( SB );
	for ( int i = 0; i < NL; i++ )
	{
		short x, y;
		x = GET_SHORT( SB );
		y = GET_SHORT( SB );
		x += xc;
		y += yc;
		FastAddLockBar( x, y );
	};
	for ( int i = 0; i < NU; i++ )
	{
		short x, y;
		x = GET_SHORT( SB );
		y = GET_SHORT( SB );
		x += xc;
		y += yc;
		FastAddUnLockBar( x, y );
	};
};
void FAST_RM_LoadObj( SaveBuf* SB, int xc, int yc )
{
	int ns = GET_INT( SB );
	char ONAME[32];
	for ( int i = 0; i < ns; i++ )
	{
		int xm, ym;
		xm = GET_SHORT( SB );
		ym = GET_SHORT( SB );
		byte NT = GET_BYTE( SB );
		NT = MyNation;
		xBlockRead( SB, ONAME, 31 );
		GeneralObject** GO = NATIONS[NT].Mon;
		int Nmon = NATIONS[NT].NMon;
		for ( int p = 0; p < Nmon; p++ )
		{
			if ( !strcmp( GO[p]->MonsterID, ONAME ) )
			{
				if ( !GO[p]->newMons->Wall )
				{
					CreateNewUnitAt( NT, ( xc + xm ) << 4, ( yc + ym ) << 4, p, GO[p]->MoreCharacter->Life, GO[p]->MoreCharacter->ProduceStages );
					goto ttt;
				}
			}
		}
	ttt:;
	}
}

void FAST_RM_Load( SaveBuf* SB, int x, int y )
{
	int TMP_VERTEX[8192];
	int NVert = 0;
	int tt0 = GetTickCount();
	int tt1, tt2, tt3, tt4;
	SB->Pos = 0;
	int SIGN, SIZE;
	SIGN = GET_INT( SB );

	if ( SIGN == 'PMAS' )
	{
		while ( SIGN != -1 )
		{
			SIGN = -1;
			SIGN = GET_INT( SB );
			SIZE = GET_INT( SB );

			switch ( SIGN )
			{
			case 'TREV':
				tt1 = GetTickCount();
				NVert = FAST_RM_LoadVertices( SB, ( x >> 6 ) << 1, ( y >> 6 ) << 1, TMP_VERTEX, 8192 );
				tt1 = GetTickCount() - tt1;
				break;

			case 'TRPS':
				tt2 = GetTickCount();
				{
					for ( int i = 0; i < NVert; i++ )
					{
						THMap[TMP_VERTEX[i]] += 2048;
					}
					FAST_RM_LoadSprites( SB, ( x >> 6 ) << 6, ( y >> 6 ) << 6 );
					for ( int i = 0; i < NVert; i++ )
					{
						THMap[TMP_VERTEX[i]] -= 2048;
					}
				}
				tt2 = GetTickCount() - tt2;
				break;

			case 'KCOL':
				tt3 = GetTickCount();
				FAST_RM_LoadLock( SB, x >> 6, y >> 6 );
				tt3 = GetTickCount() - tt3;
				break;

			case 'SJBO':
				tt4 = GetTickCount();
				FAST_RM_LoadObj( SB, ( x >> 6 ) << 6, ( y >> 6 ) << 6 );
				tt4 = GetTickCount() - tt4;
				break;
			}
		}
	}

	tt0 = GetTickCount() - tt0;
	tt0 = 0;
}

extern byte trans4[65536];
extern byte trans8[65536];
byte carr[39] = { 47,241,85,85,45,45,47,87,45,87,47,85,241,85,85,45,47,241,85,85,45,47,85,45,45,47,85,91,45,47,85 };
void Draw1( int x, int y )
{
	if ( x<0 || x>RealLx || y<4 || y>RealLy )return;
	int ofs = int( ScreenPtr ) + x + y*ScrWidth;
	int xx1 = int( trans8 );
	byte c = carr[randoma[ofs & 8191] % 10];
	__asm {
		mov eax, ofs
		xor ebx, ebx
		mov bl, c
		mov bh, byte ptr[eax]
		mov dl, [trans8 + ebx]
		mov[eax], dl
		add eax, ScrWidth
		mov bh, byte ptr[eax]
		mov dl, byte ptr[trans8 + ebx]
		mov[eax], dl
		add eax, ScrWidth
		mov bh, byte ptr[eax]
		mov dl, byte ptr[trans8 + ebx]
		mov[eax], dl
		add eax, ScrWidth
		mov bh, byte ptr[eax]
		mov dl, byte ptr[trans8 + ebx]
		mov[eax], dl
		mov bh, 0
		inc eax
		mov bl, byte ptr[eax]
		mov dl, byte ptr[trans4 + ebx]
		mov[eax], dl
		inc eax
		sub eax, ScrWidth
		mov bl, byte ptr[eax]
		mov dl, byte ptr[trans4 + ebx]
		mov[eax], dl
	};
};
void Draw_GRASS()
{
	/*
	int pp=0;
	for(int iy=0;iy<400;iy+=2)
		for(int ix=0;ix<400;ix++){
			int r=200-Norma(ix-200,iy-200);
			int p=randoma[pp&8191]%200;
			if(p<r)Draw1(ix+200,(iy>>1)+200);
			pp++;
		};
	*/
};
byte GrassMask[256] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,0,1,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,1, 1,1,1,1, 1,1,1,1, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,1,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };

void ProlongGrass( int x, int y )
{
	if ( x<0 || x>RealLx || y<4 || y>RealLy )return;
	int ofs = int( ScreenPtr ) + x + y*ScrWidth;
	__asm {
		push edx

		mov edx, ScrWidth
		xor eax, eax
		mov ebx, ofs
		mov al, [ebx]
		cmp byte ptr[GrassMask + eax], 0
		je xsd


		sub ebx, edx
		mov ah, [ebx];
		mov cl, [trans8 + eax]
			mov[ebx], cl
			//mov [ebx],ah


			sub ebx, edx
			mov al, [ebx];
		mov cl, [trans8 + eax]
			//mov [ebx],cl
			mov[ebx], ah

			sub ebx, edx
			mov ah, [ebx];
		mov cl, [trans8 + eax]
			mov[ebx], cl
			//mov [ebx],ah

			sub ebx, edx
			mov ah, [ebx];
		mov cl, [trans8 + eax]
			mov[ebx], cl
			//mov [ebx],ah

			sub ebx, edx
			mov ah, [ebx];
		mov cl, [trans8 + eax]
			mov[ebx], cl
			//mov [ebx],ah

			sub ebx, edx
			mov ah, [ebx];
		mov cl, [trans8 + eax]
			mov[ebx], cl
			//mov [ebx],ah
			xsd :
		pop edx
	};
};
int dx_p[11] = { -5,-4,-3, 2,-1, 0, 1, 2, 3, 4, 5 };
int dy_p[11] = { 1, 3, 2, 0, 2, 4, 2, 3, 1, 2, 3 };
int sprs;

int getr()
{
	sprs = ( sprs + 1 ) & 8191;
	return randoma[sprs];
}

void DrawGrassNearUnit( int x, int y )
{
}

extern int LastMx;
extern int LastMy;

void DrawWaysToSel()
{
}

void GlobalProgress::Setup()
{
	memset( this, 0, sizeof GlobalProgress );
};
void GlobalProgress::AddPosition( int ID, int Weight, int Max )
{
	if ( NWeights < 64 )
	{
		StageID[NWeights] = ID;
		StageWeight[NWeights] = Weight;
		StagePositions[NWeights] = Max;
		MaxPosition += Weight;
		NWeights++;
	}
}

void GlobalProgress::SetCurrentStage( int ID )
{
	CurStage = 0;
	for ( int i = 0; i < NWeights; i++ )
	{
		if ( StageID[i] == ID )
		{
			CurStage = i;
		}
	}
	CurPosition = 0;
}

void GlobalProgress::SetCurrentPosition( int Pos )
{
	CurPosition = Pos;
	if ( CurPosition > StagePositions[CurStage] )CurPosition = StagePositions[CurStage] - 1;
	if ( CurPosition < 0 )CurPosition = 0;
};
int GlobalProgress::GetCurProgress()
{
	int S = 0;
	int i;
	for ( i = 0; i < CurStage; i++ )
	{
		S += StageWeight[i];
	}
	S += ( CurPosition*StageWeight[i] ) / StagePositions[i];
	if ( MaxPosition )
	{
		return ( S * 100 ) / MaxPosition;
	}
	else return 100;
};
GlobalProgress::GlobalProgress()
{
	Setup();
};
GlobalProgress GPROG;
byte ExchangeTex( byte tex )
{
	switch ( tex )
	{
	case 0:return 17;
	case 3:
	case 7:return mrand() & 1 ? 56 : 57;
	case 8:
	case 9:
	case 10:return mrand() & 1 ? 58 : 59;
	case 6:
	case 1:return 16;
	case 5:
	case 2:return 12;
	case 4:return 13;
	case 45:
	case 11:return mrand() & 1 ? 23 : 13;
	};
	return tex;
};
void MakeDesert()
{
	int N = ( MaxTH + 1 )*MaxTH;
	for ( int i = 0; i < N; i++ )
	{
		TexMap[i] = ExchangeTex( TexMap[i] );
	};
	int TRDES[8] = { 79,80,81,82,84,79,80,82 };
	for ( int i = 0; i < MaxSprt; i++ )
	{
		OneSprite* OS = Sprites + i;
		if ( OS->Enabled&&OS->SG == &TREES )
		{
			OS->SGIndex = TRDES[mrand() & 7];
			OS->OC = OS->SG->ObjChar + OS->SGIndex;
		};
	};
};