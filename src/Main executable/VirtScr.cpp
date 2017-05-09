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
#include "VirtScreen.h"
#include "MapSprites.h"
#include "NewMon.h"
#include <crtdbg.h>

extern byte *tex1;
extern RLCTable SimpleMask;

int mul3( int );
int prp34( int i );

//class that is used for fast cashing of the 3D surface
VirtualScreen SVSC;

VirtualScreen::VirtualScreen()
{
	CellQuotX = nullptr;
}

void VirtualScreen::SetSize( int scLx, int scLy )
{
	if (CellQuotX)
	{
		free( VirtualScreenPointer - RealVLx - RealVLx );
		free( CellQuotX );
		free( CellQuotY );
		free( CellFlags );
		free( MarkedX );
		free( TriangMap );
		free( LoTriMap );
	}

	CellSX = TriUnit * 2;
	CellSY = TriUnit34 * 8;

	int N = scLx / CellSX;
	if (N & 1)
	{
		N += 3;
	}
	else
	{
		N += 2;
	}

	MaxSizeX = N*CellSX;
	N = scLy / CellSY;
	MaxSizeY = ( N + 2 )*CellSY;

	RealVLx = MaxSizeX;
	RealVLy = MaxSizeY;

	ShiftsPerCellX = div( CellSX, 32 ).quot;
	ShiftsPerCellY = div( CellSY, mul3( 8 ) ).quot;

	CellNX = div( RealVLx, CellSX ).quot;
	CellNY = div( RealVLy, CellSY ).quot;

	RealVLx = CellNX*CellSX;
	RealVLy = CellNY*CellSY;
	NCells = CellNX*CellNY;

	CellQuotX = new byte[NCells];
	CellQuotY = new byte[NCells];
	CellFlags = new byte[NCells];

	memset( CellQuotX, 0, NCells );
	memset( CellQuotY, 0, NCells );
	memset( CellFlags, 0, NCells );

	Lx = 128 << ADDSH;
	Ly = 128 << ADDSH;

	VirtualScreenPointer = new byte[RealVLx*( RealVLy + 4 )] + RealVLx + RealVLx;
	memset( VirtualScreenPointer - ( RealVLx << 1 ), 255, RealVLx << 1 );
	memset( VirtualScreenPointer + ( RealVLx*RealVLy ), 255, RealVLx << 1 );
	MaxTMX = div( 256 << ADDSH, ShiftsPerCellX ).quot;
	MaxTMY = div( 256 << ADDSH, ShiftsPerCellY ).quot + 1;
	TriangMap = new int[MaxTMX*MaxTMY];
	LoTriMap = new int[MaxTMX*MaxTMY];
	MarkedX = new byte[MaxTMX];
	memset( MarkedX, 1, MaxTMX );
	Grids = false;
}

VirtualScreen::~VirtualScreen()
{
	free( VirtualScreenPointer - RealVLx - RealVLx );
	free( CellQuotX );
	free( CellQuotY );
	free( CellFlags );
	free( MarkedX );
	free( TriangMap );
	free( LoTriMap );
}

void VirtualScreen::SetVSParameters( int sLx, int sLy )
{
	Lx = sLx;
	Ly = sLy;
}

void VirtualScreen::CopyVSPart( int vx, int vy, int sx, int sy, int SizeX, int SizeY )
{
	if (!( SizeY && SizeY ))
	{
		return;
	}

	//debugging control
	//assert(sx>=0&&sy>=0&&sx+SizeX<=SCRSizeX&&sy+SizeY<=SCRSizeY);
	//-----------------
	int vsofs = int( VirtualScreenPointer ) + vx + vy*RealVLx;
	int scofs = int( ScreenPtr ) + sx + sy*SCRSizeX;
	int szx4 = SizeX >> 2;
	int szx1 = SizeX & 3;
	int addvs = RealVLx - SizeX;
	int addsc = SCRSizeX - SizeX;
	__asm 
	{
		push	esi
		push	edi
		pushf
		cld
		mov		edi, scofs
		mov		esi, vsofs
		mov		edx, SizeY
		lpp1 : mov		ecx, szx4
			   rep		movsd
			   mov		ecx, szx1
			   rep		movsb
			   add		esi, addvs
			   add		edi, addsc
			   dec		edx
			   jnz		lpp1
			   popf
			   pop		edi
			   pop		esi
	}
}

void VirtualScreen::CopyVSPartMMX( int vx, int vy, int sx, int sy, int SizeX, int SizeY )
{
	if (!( SizeY&&SizeY ))return;
	//debugging control
	//assert(sx>=0&&sy>=0&&sx+SizeX<=SCRSizeX&&sy+SizeY<=SCRSizeY);
	//-----------------
	int vsofs = int( VirtualScreenPointer ) + vx + vy*RealVLx;
	int scofs = int( ScreenPtr ) + sx + sy*SCRSizeX;
	int szx4 = SizeX >> 3;
	int szx1 = SizeX & 7;
	int addvs = RealVLx - SizeX;
	int addsc = SCRSizeX - SizeX;
	__asm {
		push	esi
		push	edi
		pushf
		cld
		mov		edi, scofs
		mov		esi, vsofs
		mov		edx, SizeY
		lpp1 : mov		ecx, szx4
			   jcxz	lpp3
			   lpp2 : movq	mm0, [esi]
					  add		esi, 8;
		movq[edi], mm0
			add		edi, 8
			dec		cx
			jnz		lpp2
			lpp3 : mov		ecx, szx1
			rep		movsb
			add		esi, addvs
			add		edi, addsc
			dec		edx
			jnz		lpp1
			popf
			pop		edi
			pop		esi
			emms
	};
};
void VirtualScreen::CopyVSToScreen()
{
	int TotLx = smaplx << 5;
	int TotLy = mul3( smaply ) << 3;
	int vsx = div( mapx << 5, RealVLx ).rem;
	int vsy = div( mul3( mapy ) << 3, RealVLy ).rem;
	int Lx0, Ly0, Lx1, Ly1;
	if (vsx + TotLx <= RealVLx)
	{
		Lx0 = TotLx;
		Lx1 = 0;
	}
	else
	{
		Lx0 = RealVLx - vsx;
		Lx1 = TotLx - Lx0;
	};
	if (vsy + TotLy <= RealVLy)
	{
		Ly0 = TotLy;
		Ly1 = 0;
	}
	else
	{
		Ly0 = RealVLy - vsy;
		Ly1 = TotLy - Ly0;
	};
	CopyVSPart( vsx, vsy, smapx, smapy, Lx0, Ly0 );
	CopyVSPart( 0, vsy, smapx + Lx0, smapy, Lx1, Ly0 );
	CopyVSPart( vsx, 0, smapx, smapy + Ly0, Lx0, Ly1 );
	CopyVSPart( 0, 0, smapx + Lx0, smapy + Ly0, Lx1, Ly1 );
}

void VirtualScreen::RenderVSPart( int QuotX, int QuotY, int cx, int cy, int clx, int cly )
{
	if (!( clx && cly ))
	{
		return;
	}

	int spos = cx + cy*CellNX;
	int StartCellY = 0;
	int NCellY = 0;
	bool CellStart = false;
	for (int px = 0; px < clx; px++)
	{
		int pos = spos + px;
		for (int py = 0; py < cly; py++)
		{
			if (( !CellFlags[pos] ) || CellQuotX[pos] != QuotX || CellQuotY[pos] != QuotY)
			{
				//need to be rendered
				if (CellStart)NCellY++;
				else
				{
					CellStart = true;
					StartCellY = py + cy;
					NCellY = 1;
				};
			}
			else
			{
				if (CellStart)
				{
					RenderVerticalSet( QuotX, QuotY, px + cx, StartCellY, NCellY );
					CellStart = false;
				};
			};
			pos += CellNX;
		};
		if (CellStart)
		{
			RenderVerticalSet( QuotX, QuotY, px + cx, StartCellY, NCellY );
			CellStart = false;
		};
	};
}

void VirtualScreen::RefreshSurface()
{
	//calculating starting cell
	int scx = div( mapx, ShiftsPerCellX ).quot;
	int scy = div( mapy, ShiftsPerCellY ).quot;
	int scx1 = div( mapx + smaplx - 1, ShiftsPerCellX ).quot;
	int scy1 = div( mapy + smaply - 1, ShiftsPerCellY ).quot;
	int clnx = scx1 - scx + 1;
	int clny = scy1 - scy + 1;
	int vsx = div( scx, CellNX ).rem;
	int vsy = div( scy, CellNY ).rem;
	int Lx0, Ly0, Lx1, Ly1;
	if (vsx + clnx <= CellNX)
	{
		Lx0 = clnx;
		Lx1 = 0;
	}
	else
	{
		Lx0 = CellNX - vsx;
		Lx1 = clnx - Lx0;
	};
	if (vsy + clny <= CellNY)
	{
		Ly0 = clny;
		Ly1 = 0;
	}
	else
	{
		Ly0 = CellNY - vsy;
		Ly1 = clny - Ly0;
	};
	int QuotX = div( scx, CellNX ).quot;
	int QuotY = div( scy, CellNY ).quot;
	RenderVSPart( QuotX, QuotY, vsx, vsy, Lx0, Ly0 );
	RenderVSPart( QuotX + 1, QuotY, 0, vsy, Lx1, Ly0 );
	RenderVSPart( QuotX, QuotY + 1, vsx, 0, Lx0, Ly1 );
	RenderVSPart( QuotX + 1, QuotY + 1, 0, 0, Lx1, Ly1 );
}

extern byte ExtTex[256][4];
extern short randoma[8192];

byte DTX( byte v, int t )
{
	return ExtTex[v][randoma[t & 8191] & 3];
}

extern byte TileMap[256];

int VirtualScreen::ShowLimitedSector( int i, bool Mode3D, int HiLine, int LoLine, int QuotX, int QuotY )
{
	if (i >= MaxTH*MaxTH * 2)
	{
		return 0;
	}

	//determination of the parameters of the triangle
	//1.Coordinates&type 
	int TriType;
	int x1 = 0;
	int y1 = 0;
	int z1 = 0;
	int x2 = 0;
	int y2 = 0;
	int z2 = 0;
	int x3 = 0;
	int y3 = 0;
	int z3 = 0;
	int yy1 = 0;
	int yy2 = 0;
	int yy3 = 0;
	int TriStartY = div( ( i >> 2 ), MaxSector ).quot;
	int StartVertex = TriStartY + ( ( i >> 2 ) << 1 );
	int StartSide = ( i >> 2 ) * 6;
	int Tex1 = 0, Tex2 = 0, Tex3 = 0;
	TriType = i & 3;
	int TriStartX = ( ( i >> 2 ) - TriStartY*MaxSector )*( 4 * TriUnit );
	TriStartY = TriStartY*( TriUnit + TriUnit );
	bool Visible = true;
	switch (TriType)
	{
	case 0:
		x1 = x2 = TriStartX;
		y1 = TriStartY;
		yy1 = prp34( y1 );
		y2 = TriStartY + TriUnit + TriUnit;
		yy2 = prp34( y2 );
		x3 = TriStartX + TriUnit + TriUnit;
		y3 = TriStartY + TriUnit;
		yy3 = prp34( y3 );
		if (Mode3D)
		{
			z1 = yy1 - THMap[StartVertex] - AddTHMap( StartVertex );
			z2 = yy2 - THMap[StartVertex + VertInLine] - AddTHMap( StartVertex + VertInLine );
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
		}
		else
		{
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z1 >= z2)Visible = false;
		break;
	case 1:
		x1 = TriStartX;
		y1 = TriStartY;
		yy1 = prp34( y1 );
		x2 = x3 = TriStartX + TriUnit + TriUnit;
		y2 = TriStartY - TriUnit;
		yy2 = prp34( y2 );
		y3 = TriStartY + TriUnit;
		yy3 = prp34( y3 );
		if (Mode3D)
		{
			z1 = yy1 - THMap[StartVertex] - AddTHMap( StartVertex );
			z2 = yy2 - THMap[StartVertex + 1] - AddTHMap( StartVertex + 1 );
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
		}
		else
		{
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z2 >= z3)Visible = false;
		break;
	case 2:
		x1 = TriStartX + TriUnit + TriUnit + TriUnit + TriUnit;
		y1 = TriStartY;
		yy1 = prp34( y1 );
		x2 = x3 = TriStartX + TriUnit + TriUnit;
		y2 = TriStartY - TriUnit;
		yy2 = prp34( y2 );
		y3 = TriStartY + TriUnit;
		yy3 = prp34( y3 );
		if (Mode3D)
		{
			z1 = yy1 - THMap[StartVertex + 2] - AddTHMap( StartVertex + 2 );
			z2 = yy2 - THMap[StartVertex + 1] - AddTHMap( StartVertex + 1 );
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
		}
		else
		{
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z2 >= z3)Visible = false;
		break;
	case 3:
		x1 = x2 = TriStartX + TriUnit + TriUnit + TriUnit + TriUnit;
		y1 = TriStartY;
		yy1 = prp34( y1 );
		y2 = TriStartY + TriUnit + TriUnit;
		yy2 = prp34( y2 );
		x3 = TriStartX + TriUnit + TriUnit;
		y3 = TriStartY + TriUnit;
		yy3 = prp34( y3 );
		if (Mode3D)
		{
			z1 = yy1 - THMap[StartVertex + 2] - AddTHMap( StartVertex + 2 );
			z2 = yy2 - THMap[StartVertex + VertInLine + 2] - AddTHMap( StartVertex + VertInLine + 2 );
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
		}
		else
		{
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z1 >= z2)Visible = false;
	};

	if (!Visible)
	{
		return 0;
	}

	//Now we are ready to render
	int f1 = 0, f2 = 0, f3 = 0;
	switch (TriType)
	{
	case 0:
		f1 = GetLighting( StartVertex );
		f2 = GetLighting( StartVertex + VertInLine );
		f3 = GetLighting( StartVertex + VertInLine + 1 );
		Tex1 = GetVTex( StartVertex );
		Tex2 = GetVTex( StartVertex + VertInLine );
		Tex3 = GetVTex( StartVertex + VertInLine + 1 );
		break;
	case 1:
		f1 = GetLighting( StartVertex );
		f2 = GetLighting( StartVertex + 1 );
		f3 = GetLighting( StartVertex + VertInLine + 1 );
		Tex1 = GetVTex( StartVertex );
		Tex2 = GetVTex( StartVertex + 1 );
		Tex3 = GetVTex( StartVertex + VertInLine + 1 );
		break;
	case 2:
		f1 = GetLighting( StartVertex + 2 );
		f2 = GetLighting( StartVertex + 1 );
		f3 = GetLighting( StartVertex + VertInLine + 1 );
		Tex1 = GetVTex( StartVertex + 2 );
		Tex2 = GetVTex( StartVertex + 1 );
		Tex3 = GetVTex( StartVertex + VertInLine + 1 );
		break;
	case 3:
		f1 = GetLighting( StartVertex + 2 );
		f2 = GetLighting( StartVertex + VertInLine + 2 );
		f3 = GetLighting( StartVertex + VertInLine + 1 );
		Tex1 = GetVTex( StartVertex + 2 );
		Tex2 = GetVTex( StartVertex + VertInLine + 2 );
		Tex3 = GetVTex( StartVertex + VertInLine + 1 );
	};
	int xt1, yt1, xt2, yt2, xt3, yt3;
	if (( TexFlags[Tex1] & TEX_NORMALPUT ) || ( TexFlags[Tex2] & TEX_NORMALPUT ) || ( TexFlags[Tex3] & TEX_NORMALPUT ))
	{
		xt1 = -( x1 >> 1 ) - y1;
		yt1 = ( ( ( x1 + x1 + x1 ) >> 1 ) - y1 ) >> 1;
		xt2 = -( x2 >> 1 ) - y2;
		yt2 = ( ( ( x2 + x2 + x2 ) >> 1 ) - y2 ) >> 1;
		xt3 = -( x3 >> 1 ) - y3;
		yt3 = ( ( ( x3 + x3 + x3 ) >> 1 ) - y3 ) >> 1;
	}
	else
	{
		xt1 = ( ( x1 + y1 ) * 3 ) >> 2;
		yt1 = ( ( y1 - x1 ) * 3 ) >> 2;
		xt2 = ( ( x2 + y2 ) * 3 ) >> 2;
		yt2 = ( ( y2 - x2 ) * 3 ) >> 2;
		xt3 = ( ( x3 + y3 ) * 3 ) >> 2;
		yt3 = ( ( y3 - x3 ) * 3 ) >> 2;
	};


	//-------  Del this
	//xt1-=xt1>>2;
	//yt1-=yt1>>2;
	//xt2-=xt2>>2;
	//yt2-=yt2>>2;
	//xt3-=xt3>>2;
	//yt3-=yt3>>2;
	//-------
	int xmin = xt1;
	if (xt2 < xmin)xmin = xt2;
	if (xt3 < xmin)xmin = xt3;
	int ymin = yt1;
	if (yt2 < ymin)ymin = yt2;
	if (yt3 < ymin)ymin = yt3;
	xmin -= xmin & 63;
	ymin -= ymin & 63;
	xt1 -= xmin;
	xt2 -= xmin;
	xt3 -= xmin;
	yt1 -= ymin;
	yt2 -= ymin;
	yt3 -= ymin;
	int RSIZE = 0;
	int DXX = QuotX*RealVLx;
	int DYY = QuotY*RealVLy;
	x1 = x1 - DXX;
	z1 = z1 - DYY;
	x2 = x2 - DXX;
	z2 = z2 - DYY;
	x3 = x3 - DXX;
	z3 = z3 - DYY;
	if (( z1 < HiLine&&z2 < HiLine&&z3 < HiLine ) || ( z1 > LoLine&&z2 > LoLine&&z3 > LoLine ))return 0;
	if (( Tex1 == Tex2&&Tex1 == Tex3 ) || ( TriType == 7 ))
	{
		if (TileMap[Tex1])
		{
			int dx = randoma[( ( i * 23 ) / 17 ) & 8191] & 63;
			int dy = randoma[( ( i * 29 ) / 17 ) & 8191] & 63;
			xt1 += dx;
			xt2 += dx;
			xt3 += dx;
			yt1 += dy;
			yt2 += dy;
			yt3 += dy;
		}
		else
			if (ExtTex[Tex1][0] != ExtTex[Tex1][1])
			{
				int dx = randoma[( ( i * 23 ) / 17 ) & 8191] & 63;
				xt1 += dx;
				xt2 += dx;
				xt3 += dx;
			};
		DirectRenderTriangle64( x1, z1, x2, z2, x3, z3,
			xt1, yt1, xt2, yt2, xt3, yt3,
			f1, f2, f3, VirtualScreenPointer,
			tex1 + GetBmOfst( DTX( Tex1, ( i * 19 ) / 17 ) ),
			HiLine, LoLine, RealVLx );
	}
	else
	{
		switch (TriType)
		{
		case 0:
			PrepareIntersection1( Tex2, Tex1, Tex3, xt2, yt2,
				SECTMAP( StartSide ),
				SECTMAP( StartSide + 2 ),
				SECTMAP( StartSide + SectInLine + 1 ),
				SimpleMask, tex1 );
			DirectRenderTriangle64( x1, z1, x3, z3, x2, z2,
				//63,31,0,63,0,0,
				30, 15, 0, 31, 0, 0,
				f1, f3, f2, VirtualScreenPointer,
				ResultMask,
				HiLine, LoLine, RealVLx );
			break;
		case 1:
			PrepareIntersection2( Tex1, Tex3, Tex2, xt3, yt3,
				SECTMAP( StartSide + 2 ),
				SECTMAP( StartSide + 3 ),
				SECTMAP( StartSide + 1 ),
				SimpleMask, tex1 );
			DirectRenderTriangle64( x1, z1, x3, z3, x2, z2,
				//63,0,0,31,63,63,
				30, 1, 1, 14, 30, 30,
				f1, f3, f2, VirtualScreenPointer,
				ResultMask,
				HiLine, LoLine, RealVLx );
			break;
		case 2:
			PrepareIntersection1( Tex3, Tex2, Tex1, xt3, yt3,
				SECTMAP( StartSide + 3 ),
				SECTMAP( StartSide + 4 ),
				SECTMAP( StartSide + 5 ),
				SimpleMask, tex1 );
			DirectRenderTriangle64( x1, z1, x3, z3, x2, z2,
				//0,63,0,0,63,31,
				1, 30, 1, 1, 30, 14,
				f1, f3, f2, VirtualScreenPointer,
				ResultMask,
				HiLine, LoLine, RealVLx );
			break;
		case 3:
			PrepareIntersection2( Tex3, Tex2, Tex1, xt2, yt2,
				SECTMAP( StartSide + SectInLine + 4 ),
				SECTMAP( StartSide + 6 ),
				SECTMAP( StartSide + 5 ),
				SimpleMask, tex1 );
			DirectRenderTriangle64( x1, z1, x3, z3, x2, z2,
				//63,63,63,0,0,31,
				30, 30, 30, 1, 1, 14,
				f1, f3, f2, VirtualScreenPointer,
				ResultMask,
				HiLine, LoLine, RealVLx );
			break;
		};
	};
	return 1;
}

void CheckFirstLine();

void VirtualScreen::RenderVerticalSet( int QuotX, int QuotY, int cx, int cy, int cly )
{
	//debugging part
	//assert(cy>=0&&cy+cly<=CellNY);
	//--------------
	int NRend = 0;
	int HiLine = cy*CellSY;
	int LoLine = ( cy + cly )*CellSY - 1;
	int cost = QuotX*CellNX + cx + ( QuotY*CellNY + cy )*MaxTMX;
	CheckFirstLine();
	CheckVLINE( cost );
	int Tstart = TriangMap[cost];
	//assert(Tstart!=4166);
	if (Tstart == -1)
	{
		int cost1 = cost;
		cost1 -= MaxTMX;
		while (cost1 >= 0 && Tstart == -1)
		{
			CheckVLINE( cost1 );
			Tstart = TriangMap[cost1];
			cost1 -= MaxTMX;
		};
		if (Tstart == -1)
		{
			if (cx & 1)Tstart = ( cx << 1 ) + 1;
			else Tstart = ( cx << 1 );
		};
	};
	int TStart0 = Tstart;
	//assert(Tstart!=4166);
	cost += (cly) *MaxTMX;
	CheckVLINE( cost );
	int Tend = LoTriMap[cost];
	if (Tend == -1)
	{
		int cmax = MaxTMX*MaxTMY;
		cost += MaxTMX;
		while (Tend == -1 && cost < cmax)
		{
			CheckVLINE( cost );
			Tend = LoTriMap[cost];
			cost += MaxTMX;
		};
		if (Tend == -1)
		{
			if (cx & 1)Tend = ( cx << 1 ) + 1 + ( MaxTH - 1 )*MaxTH * 2;
			else Tend = ( cx << 1 ) + ( MaxTH - 1 )*MaxTH * 2;
		};
	};
	int pos = cx + CellNX*cy;
	for (int j = 0; j < cly; j++)
	{
		CellQuotX[pos] = QuotX;
		CellQuotY[pos] = QuotY;
		CellFlags[pos] = 1;
		pos += CellNX;
	};
	if (cx & 1)
	{
		while (Tstart != Tend&&Tstart < Tend + 2)
		{
			NRend += ShowLimitedSector( Tstart, Mode3D, HiLine, LoLine, QuotX, QuotY );
			if (Tstart & 1)Tstart += MaxTH + MaxTH - 1;
			else Tstart += 1;
		};
		if (TStart0 != Tend)
			NRend += ShowLimitedSector( Tend, Mode3D, HiLine, LoLine, QuotX, QuotY );
	}
	else
	{
		while (Tstart != Tend&&Tstart < Tend + 2)
		{
			NRend += ShowLimitedSector( Tstart, Mode3D, HiLine, LoLine, QuotX, QuotY );
			if (Tstart & 1)Tstart -= 1;
			else Tstart += MaxTH + MaxTH + 1;
		};
		if (TStart0 != Tend)
			NRend += ShowLimitedSector( Tend, Mode3D, HiLine, LoLine, QuotX, QuotY );
	};
	//return;
	//assert(NRend);
	if (Grids)
	{
		//marking grids
		int xxx = cx*CellSX;
		int miny = 100000;
		int cdx = cx + QuotX*CellNX;
		int vend = VertInLine*div( Tend, MaxTH + MaxTH ).quot + cdx;
		int vstart = VertInLine*div( TStart0, MaxTH + MaxTH ).quot + cdx;
		int dy0 = QuotY*RealVLy;
		for (int v = vend; v >= vstart; v -= VertInLine)
		{
			int yy = ( mul3( GetTriY( v ) ) >> 2 ) - dy0;
			if (Mode3D)yy -= THMap[v] + AddTHMap( v );
			if (yy < miny)
			{
				miny = yy;
				if (yy >= HiLine&&yy <= LoLine)
				{
					int ofst = yy*RealVLx + xxx;
					VirtualScreenPointer[ofst] = 254;
					VirtualScreenPointer[ofst + 1] = 254;
					if (yy < LoLine)
					{
						ofst += RealVLx;
						VirtualScreenPointer[ofst] = 254;
						VirtualScreenPointer[ofst + 1] = 254;
					};
				};
			};
		};
	};
	Sequrity();
}

void VirtualScreen::CreateTrianglesMapping()
{
	int NELM = MaxTMX*MaxTMY;
	memset( TriangMap, 0xFF, NELM << 2 );
	memset( LoTriMap, 0xFF, NELM << 2 );
	/*
	for(int nx=0;nx<MaxTH*2;nx++){
		CreateVerticalTrianglesMapping(nx);
	};
	*/
}

int GetMaxTriY( int StartTri, int InsTri, bool Minimax )
{
	div_t ddt = div( StartTri, MaxTH * 2 );
	int StartVertex = ddt.quot*VertInLine + ( ddt.rem >> 1 );
	int y1 = 0, y2 = 0, y3 = 0;
	if (Mode3D)
	{
		switch (InsTri)
		{
		case 0:
			y1 = ( mul3( GetTriY( StartVertex ) ) >> 2 ) - THMap[StartVertex] - AddTHMap( StartVertex );
			y2 = ( mul3( GetTriY( StartVertex + VertInLine ) ) >> 2 ) - THMap[StartVertex + VertInLine] - AddTHMap( StartVertex + VertInLine );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 ) - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
			break;
		case 1:
			y1 = ( mul3( GetTriY( StartVertex ) ) >> 2 ) - THMap[StartVertex] - AddTHMap( StartVertex );
			y2 = ( mul3( GetTriY( StartVertex + 1 ) ) >> 2 ) - THMap[StartVertex + 1] - AddTHMap( StartVertex + 1 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 ) - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
			break;
		case 2:
			y1 = ( mul3( GetTriY( StartVertex + 1 ) ) >> 2 ) - THMap[StartVertex + 1] - AddTHMap( StartVertex + 1 );
			y2 = ( mul3( GetTriY( StartVertex + 2 ) ) >> 2 ) - THMap[StartVertex + 2] - AddTHMap( StartVertex + 2 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 ) - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
			break;
		case 3:
			y1 = ( mul3( GetTriY( StartVertex + 2 ) ) >> 2 ) - THMap[StartVertex + 2] - AddTHMap( StartVertex + 2 );
			y2 = ( mul3( GetTriY( StartVertex + VertInLine + 2 ) ) >> 2 ) - THMap[StartVertex + VertInLine + 2] - AddTHMap( StartVertex + VertInLine + 2 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 ) - THMap[StartVertex + VertInLine + 1] - AddTHMap( StartVertex + VertInLine + 1 );
		};
	}
	else
	{
		switch (InsTri)
		{
		case 0:
			y1 = ( mul3( GetTriY( StartVertex ) ) >> 2 );
			y2 = ( mul3( GetTriY( StartVertex + VertInLine ) ) >> 2 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 );
			break;
		case 1:
			y1 = ( mul3( GetTriY( StartVertex ) ) >> 2 );
			y2 = ( mul3( GetTriY( StartVertex + 1 ) ) >> 2 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 );
			break;
		case 2:
			y1 = ( mul3( GetTriY( StartVertex + 1 ) ) >> 2 );
			y2 = ( mul3( GetTriY( StartVertex + 2 ) ) >> 2 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 );
			break;
		case 3:
			y1 = ( mul3( GetTriY( StartVertex + 2 ) ) >> 2 );
			y2 = ( mul3( GetTriY( StartVertex + VertInLine + 2 ) ) >> 2 );
			y3 = ( mul3( GetTriY( StartVertex + VertInLine + 1 ) ) >> 2 );
		};
	};
	if (Minimax)
	{
		if (y1 > y2&&y1 > y3)return y1;
		if (y2 > y1&&y2 > y3)return y2;
		return y3;
	}
	else
	{
		if (y1 < y2&&y1 < y3)return y1;
		if (y2 < y1&&y2 < y3)return y2;
		return y3;
	};
}

void VirtualScreen::CreateVerticalTrianglesMapping( int VertSet )
{
	if (VertSet >= MaxTMX)return;
	int pos = VertSet;
	for (int i = 0; i < MaxTMY; i++)
	{
		TriangMap[pos] = -1;
		LoTriMap[pos] = -1;
		pos += MaxTMX;
	};
	int VStart = ( ( VertSet & 65534 ) << 1 ) + ( MaxTH - 2 )*MaxTH * 2;
	int y1, yind;
	if (VertSet & 1)
	{
		for (int ny = MaxTH - 2; ny >= 0; ny--)
		{
			y1 = GetMaxTriY( VStart, 3, true );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)TriangMap[yind*MaxTMX + VertSet] = VStart + 3;
			y1 = GetMaxTriY( VStart, 2, true );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)TriangMap[yind*MaxTMX + VertSet] = VStart + 2;
			VStart -= MaxTH * 2;
		};
		VStart += MaxTH * 2;
		for (int ny = MaxTH - 2; ny >= 0; ny--)
		{
			y1 = GetMaxTriY( VStart, 2, false );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)LoTriMap[yind*MaxTMX + VertSet] = VStart + 2;
			y1 = GetMaxTriY( VStart, 3, false );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)LoTriMap[yind*MaxTMX + VertSet] = VStart + 3;
			VStart += MaxTH * 2;
		};
	}
	else
	{
		for (int ny = MaxTH - 2; ny >= 0; ny--)
		{
			y1 = GetMaxTriY( VStart, 0, true );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)TriangMap[yind*MaxTMX + VertSet] = VStart;
			y1 = GetMaxTriY( VStart, 1, true );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)TriangMap[yind*MaxTMX + VertSet] = VStart + 1;
			VStart -= MaxTH * 2;
		};
		VStart += MaxTH * 2;
		for (int ny = MaxTH - 2; ny >= 0; ny--)
		{
			y1 = GetMaxTriY( VStart, 1, false );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)LoTriMap[yind*MaxTMX + VertSet] = VStart + 1;
			y1 = GetMaxTriY( VStart, 1, false );
			yind = div( y1, CellSY ).quot;
			if (yind < MaxTMY&&yind >= 0)LoTriMap[yind*MaxTMX + VertSet] = VStart;
			VStart += MaxTH * 2;
		};
	};
	Sequrity();
}

void VirtualScreen::CheckVLINE( int V )
{
	int v = V%MaxTMX;
	if (MarkedX[v])
	{
		CreateVerticalTrianglesMapping( v );
		MarkedX[v] = 0;
	};
}

void VirtualScreen::Execute()
{
	RefreshSurface();
	CopyVSToScreen();
}

void VirtualScreen::Zero()
{
	memset( VirtualScreenPointer, 0xCD, RealVLx*RealVLy );
	CreateTrianglesMapping();
}

int GetHiDiff( int xx, int yy )
{
	int x = ( xx << 4 ) + 8;
	int y = ( yy << 4 ) + 8;
	int hi1 = abs( GetHeight( x - 16, y ) - GetHeight( x + 16, y ) );
	int hi2 = abs( GetHeight( x, y - 16 ) - GetHeight( x, y + 16 ) );
	if (abs( hi1 ) > abs( hi2 ))return hi1; else return hi2;
}

int GetBigHiDiff( int xx, int yy )
{
	int x = ( xx << 4 ) + 8;
	int y = ( yy << 4 ) + 8;
	int hi1 = abs( GetHeight( x - 32, y ) - GetHeight( x + 32, y ) );
	int hi2 = abs( GetHeight( x, y - 32 ) - GetHeight( x, y + 32 ) );
	if (abs( hi1 ) > abs( hi2 ))return hi1; else return hi2;
}

void SetLockPoint( int xx, int yy )
{
	int ddif = GetHiDiff( xx, yy );
	if (ddif > 14 && GetBigHiDiff( xx, yy ) > 14)BSetPt( xx, yy );
	else BClrPt( xx, yy );
}

void VirtualScreen::RefreshTriangle( int i )
{
	int ost = i % 4096;
	div_t ddt = div( i, MaxTH * 2 );
	int sx = ddt.rem >> 1;
	MarkedX[sx] = 1;
	div_t sxdt = div( sx, CellNX );
	int Miny = div( GetMaxTriY( i & 0xFFFFFFFC, i & 3, false ), CellSY ).quot;
	int Maxy = div( GetMaxTriY( i & 0xFFFFFFFC, i & 3, true ), CellSY ).quot - Miny + 1;
	if (Miny < 0)Miny = 0;
	div_t sydt = div( Miny, CellNY );
	sx = sxdt.rem;
	int ofst = sx + sydt.rem*CellNX;
	int maxo = NCells;
	for (int dsy = 0; dsy < Maxy; dsy++)
	{
		if (CellQuotX[ofst] == sxdt.quot&&CellQuotY[ofst] == sydt.quot)
		{
			if (ofst >= 0 && ofst < maxo)CellFlags[ofst] = 0;
		};
		sydt.rem++;
		if (sydt.rem >= CellNY)
		{
			ofst = sx;
			sydt.rem = 0;
			sydt.quot++;
		}
		else ofst += CellNX;
	}
}

int CheckPt( int x, int y );

int GetNP( int x, int y )
{
	int np = 0;
	if (CheckPt( x - 1, y ))np++;
	if (CheckPt( x + 1, y ))np++;
	if (CheckPt( x, y - 1 ))np++;
	if (CheckPt( x, y + 1 ))np++;
	if (CheckPt( x - 1, y - 1 ))np++;
	if (CheckPt( x - 1, y + 1 ))np++;
	if (CheckPt( x + 1, y - 1 ))np++;
	if (CheckPt( x + 1, y + 1 ))np++;
	return np;
}

void PrepareLandLocking()
{
	int maxx = MaxTH << 1;
	int xx, yy;
	for (int ix = 0; ix < MaxTH; ix++)
		for (int iy = 0; iy < MaxTH; iy++)
		{
			xx = iy + iy;
			yy = ix + ix;
			if (CheckPt( xx, yy ))
			{
				int np = GetNP( xx, yy );
				if (np <= 3)BClrPt( xx, yy );
			};
		};
	for (int ix = 0; ix < MaxTH; ix++)
		for (int iy = 0; iy < MaxTH; iy++)
		{
			xx = iy + iy + 1;
			yy = ix + ix;
			if (CheckPt( xx, yy ))
			{
				int np = GetNP( xx, yy );
				if (np <= 3)BClrPt( xx, yy );
			};
		};
	for (int ix = 0; ix < MaxTH; ix++)
		for (int iy = 0; iy < MaxTH; iy++)
		{
			xx = iy + iy;
			yy = ix + ix + 1;
			if (CheckPt( xx, yy ))
			{
				int np = GetNP( xx, yy );
				if (np <= 3)BClrPt( xx, yy );
			};
		};
	for (int ix = 0; ix < MaxTH; ix++)
		for (int iy = 0; iy < MaxTH; iy++)
		{
			xx = iy + iy + 1;
			yy = ix + ix + 1;
			if (CheckPt( xx, yy ))
			{
				int np = GetNP( xx, yy );
				if (np <= 3)BClrPt( xx, yy );
			};
		};
}

void VirtualScreen::RefreshScreen()
{
	memset( MarkedX, 1, MaxTMX );
	memset( CellFlags, 0, CellNX*CellNY );
	memset( CellQuotX, 0, NCells );
	memset( CellQuotY, 0, NCells );
	memset( CellFlags, 0, NCells );

	int maxx = MaxTH << 1;
}

void VirtualScreen::Sequrity()
{
	int* tt = (int*) ( VirtualScreenPointer - ( RealVLx << 1 ) );
	int nn = RealVLx >> 1;
	tt = (int*) ( VirtualScreenPointer + RealVLx*RealVLy );
}

int AddTHMap( int i )
{
	return ( TexFlags[TexMap[i]] & 8 ? 0 : word( randoma[word( i % 8133 )] ) & 7 );
}