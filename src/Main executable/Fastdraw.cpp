/*               Fast raster graphics routine
 *
 *  This module presents several useful procedures for fast output,
 * using RLC graphics format (see the discription below). Before the
 * wor you must call SetRlcWindow to set properly the screen
 * capabilities.After this you mast call SetScreenPtr to set the
 * output distanation. Uou can use LoadRLC to load the set of RLC
 * pictures into the RLCTable structure. Then you can draw the
 * picture on the screen wit the procedure ShowRLCItem
 */
#include "ddini.h"
#include "ResFile.h"
#include "mode.h"
#include <stdio.h>
#include "Fastdraw.h"
#include "GP_Draw.h"
#include "assert.h"
extern int SCRSizeX;
extern int SCRSizeY;
extern int RSCRSizeX;
extern int RSCRSizeY;
extern int COPYSizeX;
extern int LOADED;
__declspec( dllexport )
void ShowCharUNICODE( int x, int y, byte* strptr, lpRLCFont lpr );
byte PAL1[256];
byte PAL2[256];
byte PAL3[256];
byte PAL4[256];
byte PAL5[256];
byte PAL6[256];
byte PAL7[256];
void ErrM( char* s );
extern void* offScreenPtr;

typedef RLCHeader* lpRLCHeader;

typedef xRLCTable* RLCTable;
typedef RLCTable* lpRLCTable;
RLCFont::RLCFont( int GP_Index )
{
	RLC = (RLCTable) GP_Index;
	FirstSymbol = 0;
	LastSymbol = 0;
	Options = 0;
	ShadowGP = -1;
};
void RLCFont::SetGPIndex( int n )
{
	RLC = (RLCTable) n;
	FirstSymbol = 0;
	LastSymbol = 0;
	Options = 0;
};
RLCFont::RLCFont()
{
	RLC = nullptr;
	FirstSymbol = 0;
	LastSymbol = 0;
	Options = 0;
	ShadowGP = -1;
};
RLCFont::~RLCFont()
{
	if (RLC&&int( RLC ) > 4096)free( RLC );
	RLC = nullptr;
};
typedef RLCFont* lpRLCFont;
//Window parameters
int WindX = 0;
int WindY = 0;
int WindLx;
int WindLy;
int WindX1;
int WindY1;
int ScrWidth;
int ScrHeight;
int BytesPerPixel = 1;
void* ScreenPtr = nullptr;
void* RealScreenPtr = nullptr;
extern int RealLx;
extern int RealLy;
void InitRLCWindows()
{
	WindLx = RealLx;
	WindLy = RealLy;
	WindX1 = RealLx - 1;
	WindY1 = RealLy - 1;
	ScrWidth = SCRSizeX;
	ScrHeight = SCRSizeY;
};
/*  Creating window
 */
bool ProcessMessages();
static int cntr;
static bool InCycle;

int ST_WX;
int ST_WY;
int ST_WLX;
int ST_WLY;
int ST_SCW;

__declspec( dllexport )
void SetRLCWindow( int x, int y, int lx, int ly, int slx );
void StoreWindow()
{
	ST_WX = WindX;
	ST_WY = WindY;
	ST_WLX = WindX1 - WindX + 1;
	ST_WLY = WindY1 - WindY + 1;
	ST_SCW = ScrWidth;
};
void RestoreWindow()
{
	SetRLCWindow( ST_WX, ST_WY, ST_WLX, ST_WLY, ST_SCW );
};
void SetWind( int x, int y, int Lx, int Ly )
{
	SetRLCWindow( x, y, Lx, Ly, ScrWidth );
};

__declspec( dllexport ) void SetRLCWindow( int x, int y, int lx, int ly, int slx )
{
	WindX = x;
	WindY = y;
	WindX1 = x + lx - 1;
	WindY1 = y + ly - 1;
	ScrWidth = slx;
	if (WindX < 0)
		WindX = 0;
	if (WindY < 0)
		WindY = 0;
	if (WindX1 >= RealLx)
		WindX1 = RealLx - 1;
	if (WindY1 >= RealLy)
		WindY1 = RealLy - 1;
	WindLx = WindX1 - WindX + 1;
	WindLy = WindY1 - WindY + 1;
}

//  Setting proper value of the screen pointer
void SetScreenPtr( void )
{
	ScreenPtr = offScreenPtr;
	RealScreenPtr = ddsd.lpSurface;
}

void ClearScreen();
/*
{
	if(DDError) return;
	int sz=ScrWidth*ScrHeight*BytesPerPixel/4;
	__asm
	{
		push	edi
		push	esi
		mov		edi,ScreenPtr
		xor		eax,eax
		cld
		mov		ecx,sz
		rep		stosd
		pop		esi
		pop		edi
	}
}
*/

//Showing RLC image with clipping
void ShowRLC( int x, int y, void* PicPtr )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					xor ebx, ebx
					inc		esi
					or dl, dl
					jz		NextLine1
					mov		bx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		bx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		bx, 0
					jle		ok1
					cmp		bx, cx
					jl		hdraw1
					//nothing to draw
					sub		bx, cx
					add		esi, ecx
					add		edi, ecx
					dec		dl
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, bx
					mov		ax, bx
					xor		bx, bx
					add		esi, eax
					add		edi, eax
					ok1 :
				mov		eax, ecx
					shr		ecx, 2
					//jcxz	Lx11
					rep		movsd
					mov		ecx, eax
					and		ecx, 3
					//jcxz	Lx21
					rep		movsb
					dec		dl
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					inc		esi
					xor     ebx, ebx
					or dl, dl
					jz		NextLine2
					mov		bx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		bx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		bx, cx
					jge		ok2
					//clipping
					cmp		bx, 0
					jle		ntd2
					//partial drawing
					sub		cx, bx
					mov		part, ecx
					mov		cx, bx
					mov		eax, ecx
					shr		ecx, 2
					//jcxz	Lx11_1
					rep		movsd
					mov		ecx, eax
					and		ecx, 3
					//jcxz	Lx2
					rep		movsb
					add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		dl
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		dl
					jnz		ntd22
					jmp		NextLine2
					ok2 : sub		bx, cx
					mov		eax, ecx
					shr		ecx, 2
					//jcxz	Lx11
					rep		movsd
					mov		ecx, eax
					and		ecx, 3
					//jcxz	Lx22
					rep		movsb
					dec		dl
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		bl, [esi]
				inc		esi
				or bl, bl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				add		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				mov		eax, ecx
				shr		ecx, 2
				jcxz	Lx1
				rep		movsd
				Lx1 : mov		ecx, eax
				and		ecx, 3
				//jcxz	Lx2
				rep		movsb
				//			xor		eax,eax
				//rrr:		lodsb
				//			mov		al,[precomp+eax]
				//	        stosb
				//			loop	rrr
				//			rep		movsb
				dec		ebx
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}
//End of RLC with clipping
//Showing inverse RLC image with clipping
void ShowRLCi( int x, int y, void* PicPtr )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					inc		esi
					or dl, dl
					jz		NextLine1
					xor     ebx, ebx
					mov		bx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		bx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		bx, 0
					jle		ok1_1
					cmp		bx, cx
					jl		hdraw1
					//nothing to draw
					sub		bx, cx
					add		esi, ecx
					sub		edi, ecx
					dec		dl
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, bx
					mov		ax, bx
					xor		bx, bx
					add		esi, eax
					sub		edi, eax
					ok1_1 : jcxz    Lx21
					ok1 :
				movsb
					sub		edi, 2
					dec		cl
					jnz		ok1
					Lx21 : dec		dl
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		ebx, ebx
					mov		bx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		bx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		bx, cx
					jge		ok2
					//clipping
					cmp		bx, 0
					jle		ntd2
					//partial drawing
					sub		cx, bx
					mov		part, ecx
					mov		cx, bx
					jcxz    lxsd1_1
					lxsd1 : movsb
					sub		edi, 2
					dec		cl
					jnz		lxsd1
					lxsd1_1 : add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		dl
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		dl
					jnz		ntd22
					jmp		NextLine2
					ok2 : sub		bx, cx
					jcxz    Lx22
					lkfr1 : movsb
					sub		edi, 2
					dec		cl
					jnz		lkfr1
					Lx22 : dec		dl
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		bl, [esi]
				inc		esi
				or bl, bl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				sub		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				jcxz	Lx2
				ghte : movsb
				sub		edi, 2
				dec		cl
				jnz		ghte
				Lx2 : dec		ebx
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}

//End of RLC with clipping & with palette
void ShowRLCpal( int x, int y, void* PicPtr, byte* pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	byte Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine1
					xor		edx, edx
					xor     ecx, ecx
					mov		dx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		dx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		dx, 0
					jle		ok1_1
					cmp		dx, cx
					jl		hdraw1
					//nothing to draw
					sub		dx, cx
					add		esi, ecx
					add		edi, ecx
					dec		Acm
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, dx
					mov		ax, dx
					xor		dx, dx
					add		esi, eax
					add		edi, eax
					ok1_1 : jcxz    Lx21
					ok1 :
				lodsb
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		ok1
					Lx21 : dec		Acm
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		edx, edx
					xor     ecx, ecx
					mov		dx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		dx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		dx, cx
					jge		ok2
					//clipping
					cmp		dx, 0
					jle		ntd2
					//partial drawing
					sub		cx, dx
					mov		part, ecx
					mov		cx, dx
					jcxz	kkj1_2
					kkj1 : lodsb
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		kkj1
					kkj1_2 : add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		Acm
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		Acm
					jnz		ntd22
					jmp		NextLine2
					ok2 : sub		dx, cx
					jcxz	Lx22
					kkj2 : lodsb
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		kkj2
					Lx22 : dec		Acm
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			xor		eax, eax
			mov		ebx, pal
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		dl, [esi]
				inc		esi
				or dl, dl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				add		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				jcxz	Lx2
				hgaw : lodsb
				mov		al, [ebx + eax]
				stosb
				dec		cl
				jnz		hgaw
				Lx2 : dec		dl
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}

//End of RLC with clipping & encoding
//Showing inverse RLC image with clipping & encodint
void ShowRLCipal( int x, int y, void* PicPtr, byte* pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	byte Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine1
					xor     edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		dx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		dx, 0
					jle		ok1_1
					cmp		dx, cx
					jl		hdraw1
					//nothing to draw
					sub		dx, cx
					add		esi, ecx
					sub		edi, ecx
					dec		Acm
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, dx
					mov		ax, dx
					xor		edx, edx
					add		esi, eax
					sub		edi, eax
					ok1_1 : jcxz	Lx21
					ok1 : lodsb
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		ok1
					Lx21 : dec		Acm
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		dx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		dx, cx
					jge		ok2
					//clipping
					cmp		dx, 0
					jle		ntd2
					//partial drawing
					sub		cx, dx
					mov		part, ecx
					mov		cx, dx
					jcxz    lxsd1_1
					lxsd1 : lodsb
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cx
					jnz		lxsd1
					lxsd1_1 : add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		Acm
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		Acm
					jnz		ntd22
					jmp		NextLine2
					ok2 : sub		dx, cx
					jcxz	Lx22
					lkfr1 : lodsb
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		lkfr1
					Lx22 : dec		Acm
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			xor		eax, eax
			mov		ebx, pal
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		dl, [esi]
				inc		esi
				or dl, dl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				sub		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				jcxz	Lx2
				ghte : lodsb
				mov		al, [eax + ebx]
				mov[edi], al
				dec		edi
				dec		cl
				jnz		ghte
				Lx2 : dec		dl
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}
//End of inverted RLC with clipping & encoding

//End of RLC with clipping & with palette->fon
void ShowRLCfonpal( int x, int y, void* PicPtr, byte* pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	if (!PicPtr)return;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	byte Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine1
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		dx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		dx, 0
					jle		okk1
					cmp		dx, cx
					jl		hdraw1
					//nothing to draw
					sub		dx, cx
					add		esi, ecx
					add		edi, ecx
					dec		Acm
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, dx
					mov		ax, dx
					xor		edx, edx
					add		esi, eax
					add		edi, eax
					okk1 : jcxz	Lx21
					add		esi, ecx
					ok1 : mov		al, [edi]
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		ok1
					Lx21 : dec		Acm
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		dx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		dx, cx
					jge		ok2
					//clipping
					cmp		dx, 0
					jle		ntd2
					//partial drawing
					sub		cx, dx
					mov		part, ecx
					mov		cx, dx
					add		esi, ecx
					jcxz    kkj1_1
					kkj1 : mov		al, [edi]
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		kkj1
					kkj1_1 : add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		Acm
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		Acm
					jnz		ntd22
					jmp		NextLine2
					ok2 : sub		dx, cx
					add		esi, ecx
					jcxz    Lx22
					kkj3 : mov		al, [edi]
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		kkj3
					Lx22 : dec		Acm
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			xor		eax, eax
			mov		ebx, pal
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		dl, [esi]
				inc		esi
				or dl, dl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				add		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				add		esi, ecx
				jcxz    Lx2
				hgaw : mov		al, [edi]
				mov		al, [ebx + eax]
				stosb
				dec		cl
				jnz		hgaw
				Lx2 : dec		dl
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}
//End of RLC with clipping & encoding
//Showing inverse RLC image with clipping & encodint
void ShowRLCifonpal( int x, int y, void* PicPtr, byte* pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	byte Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine1
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		dx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		dx, 0
					jle		okk1
					cmp		dx, cx
					jl		hdraw1
					//nothing to draw
					sub		dx, cx
					add		esi, ecx
					sub		edi, ecx
					dec		Acm
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, dx
					mov		ax, dx
					xor		edx, edx
					add		esi, eax
					sub		edi, eax
					okk1 : jcxz	Lx21
					add		esi, ecx
					//or		cl,cl
					//jz		Lx21
					ok1 : mov		al, [edi]
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		ok1
					Lx21 : dec		Acm
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		dx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		dx, cx
					jge		ok2
					//clipping
					cmp		dx, 0
					jle		ntd2
					//partial drawing
					sub		cx, dx
					mov		part, ecx
					mov		cx, dx
					add		esi, ecx
					lxsd1 : mov		al, [edi]
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		lxsd1
					add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		Acm
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		Acm
					jnz		ntd22
					jmp		NextLine2
					ok2 : jcxz	Lx22
					sub		dx, cx
					add		esi, ecx
					lkfr1 : mov		al, [edi]
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		lkfr1
					Lx22 : dec		Acm
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			xor		eax, eax
			mov		ebx, pal
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		dl, [esi]
				inc		esi
				or dl, dl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				sub		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				add		esi, ecx
				jcxz	Lx2
				ghte : mov		al, [edi]
				mov		al, [eax + ebx]
				mov[edi], al
				dec		edi
				dec		cl
				jnz		ghte
				Lx2 : dec		dl
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}
//End of inverted RLC with clipping & encoding->fon

//End of RLC with clipping & with palette(half-transparent fog)
void ShowRLChtpal( int x, int y, void* PicPtr, byte* pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x + PLX <= WindX ) || ( x > WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	byte Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x < WindX)
		{
			int roff = WindX - x;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine1
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		dx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		dx, 0
					jle		ok1_1
					cmp		dx, cx
					jl		hdraw1
					//nothing to draw
					sub		dx, cx
					add		esi, ecx
					add		edi, ecx
					dec		Acm
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				sub		cx, dx
					xor		eax, eax
					mov		ax, dx
					xor		edx, edx
					add		esi, eax
					add		edi, eax
					ok1_1 : jcxz	Lx21
					ok1 :
				mov		ah, [esi]
					inc		esi
					mov		al, [edi]
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		ok1
					Lx21 : dec		Acm
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x + PLX >= WindX1)
		{
			int roff = WindX1 - x + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		dx, cx
					add		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		dx, cx
					jge		ok2
					//clipping
					cmp		dx, 0
					jle		ntd2
					//partial drawing
					sub		cx, dx
					mov		part, ecx
					mov		cx, dx
					jcxz    kkj1_1
					kkj1 : mov		ah, [esi]
					inc		esi
					mov		al, [edi]
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		kkj1
					kkj1_1 : add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		Acm
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		Acm
					jnz		ntd22
					jmp		NextLine2
					ok2 : jcxz	Lx22
					sub		dx, cx
					kkj4 : mov		ah, [esi]
					inc		esi
					mov		al, [edi]
					mov		al, [ebx + eax]
					stosb
					dec		cl
					jnz		kkj4
					Lx22 : dec		Acm
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			xor		eax, eax
			mov		ebx, pal
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		dl, [esi]
				inc		esi
				or dl, dl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				add		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				jcxz    Lx2
				hgaw : mov		ah, [esi]
				inc		esi
				mov		al, [edi]
				mov		al, [ebx + eax]
				stosb
				dec		cl
				jnz		hgaw
				Lx2 : dec		dl
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}
//End of RLC with clipping & encoding
//Showing inverse RLC image with clipping & encodint(half-transparent fog)
void ShowRLCihtpal( int x, int y, void* PicPtr, byte* pal )
{
	//for(int i=0;i<256;i++) precomp[i]=i;
	int ScrOfst = int( ScreenPtr ) + y*ScrWidth + x;
	int addofs = 0;
	int subline = 0;
	int PLY = ( lpRLCHeader( PicPtr )->SizeY ) & 65535;
	int PLX = ( lpRLCHeader( PicPtr )->SizeX ) & 65535;
	if (( y + PLY - 1 < WindY ) | ( y > WindY1 ) ||
		( ( x < WindX ) || ( x - PLX + 1 >= WindX1 ) || !PLY )) return;
	if (y < WindY)
	{
		__asm
		{
			mov		edx, PicPtr
			add		edx, 4
			xor eax, eax
			mov		ecx, WindY
			sub		ecx, y
			xor		eax, eax
			xor		ebx, ebx
			Loop1xx1 : mov		al, [edx]
					   inc		edx
					   or eax, eax
					   jz		Loop1xx3
					   Loop1xx2 : mov		bl, [edx + 1]
								  add		edx, ebx
								  add		edx, 2
								  dec		eax
								  jnz		Loop1xx2
								  Loop1xx3 : dec		cx
											 jnz		Loop1xx1
											 sub		edx, PicPtr
											 sub		edx, 4
											 mov		addofs, edx
		}
		subline = WindY - y;
		ScrOfst = int( ScreenPtr ) + WindY*ScrWidth + x;
	}
	if (WindY1 < y + PLY - 1) subline += y + PLY - 1 - WindY1;
	addofs += 4;
	byte Acm;
	PLY -= subline;
	if (PLY > 0)
	{
		if (x > WindX1)
		{
			int roff = x - WindX1;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop1 :
				cmp		PLY, 0
					je		ScanLineLoopEnd1
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine1
					xor		edx, edx
					xor		ecx, ecx
					mov		dx, word ptr roff
					BeginLine1 : mov		cl, [esi]
					sub		dx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					//sub		bh,cl
					add		esi, 2
					//clipping left code
					cmp		dx, 0
					jle		ok1_1
					cmp		dx, cx
					jl		hdraw1
					//nothing to draw
					sub		dx, cx
					add		esi, ecx
					sub		edi, ecx
					dec		Acm
					jnz		BeginLine1
					pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					hdraw1 :			//draw only small part of line
				xor		eax, eax
					sub		cx, dx
					mov		ax, dx
					xor		edx, edx
					add		esi, eax
					sub		edi, eax
					ok1_1 : jcxz	Lx21
					ok1 : mov		ah, [esi]
					inc		esi
					mov		al, [edi]
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		ok1
					Lx21 : dec		Acm
					jnz		BeginLine1
					NextLine1 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop1
					ScanLineLoopEnd1 :
				pop		edi
					pop		esi
			};
		}
		else if (x - PLX + 1 < WindX)
		{
			int roff = x - WindX + 1;
			int part;
			__asm {
				push	esi
				push	edi
				mov		edi, ScrOfst
				mov		esi, PicPtr
				add		esi, addofs
				xor		ecx, ecx
				xor		eax, eax
				mov		ebx, pal
				cld
				ScanLineLoop2 :
				cmp		PLY, 0
					je		ScanLineLoopEnd2
					push	edi
					mov		dl, [esi]
					mov		Acm, dl
					inc		esi
					or dl, dl
					jz		NextLine2
					xor		ecx, ecx
					xor		edx, edx
					mov		dx, word ptr roff
					BeginLine2 : mov		cl, [esi]
					sub		dx, cx
					sub		edi, ecx
					mov		cl, [esi + 1]
					add		esi, 2
					//clipping right code
					cmp		dx, cx
					jge		ok2
					//clipping
					cmp		dx, 0
					jle		ntd2
					//partial drawing
					sub		cx, dx
					mov		part, ecx
					mov		cx, dx
					jcxz	lxsd1_1
					lxsd1 : mov		ah, [esi]
					inc		esi
					mov		al, [edi]
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		lxsd1
					lxsd1_1 : add		esi, part
					jmp		ntd4
					ntd2 :			//scanning to the next line
				add		esi, ecx
					ntd4 : dec		Acm
					jz		NextLine2
					ntd22 : mov		cl, [esi + 1]
					add		esi, 2
					add		esi, ecx
					dec		Acm
					jnz		ntd22
					jmp		NextLine2
					ok2 : jcxz	Lx22
					sub		dx, cx
					lkfr1 : mov		ah, [esi]
					inc		esi
					mov		al, [edi]
					mov		al, [ebx + eax]
					mov[edi], al
					dec		edi
					dec		cl
					jnz		lkfr1
					Lx22 : dec		Acm
					jnz		BeginLine2
					NextLine2 : pop		edi
					add		edi, ScrWidth
					dec     PLY
					jmp		ScanLineLoop2
					ScanLineLoopEnd2 :
				pop		edi
					pop		esi
			};
		}
		else
			__asm
		{
			push	esi
			push	edi
			mov		edi, ScrOfst
			mov		esi, PicPtr
			add		esi, addofs
			xor		ecx, ecx
			xor		ebx, ebx
			xor		eax, eax
			mov		ebx, pal
			cld
			ScanLineLoop :
			cmp		PLY, 0
				je		ScanLineLoopEnd
				push	edi
				mov		dl, [esi]
				inc		esi
				or dl, dl
				jz		NextLine
				BeginLine : mov		cl, [esi]
				sub		edi, ecx
				mov		cl, [esi + 1]
				add		esi, 2
				jcxz	Lx2
				ghte : mov		ah, [esi]
				inc		esi
				mov		al, [edi]
				mov		al, [eax + ebx]
				mov[edi], al
				dec		edi
				dec		cl
				jnz		ghte
				Lx2 : dec		dl
				jnz		BeginLine
				NextLine : pop		edi
				add		edi, ScrWidth
				dec     PLY
				jmp		ScanLineLoop
				ScanLineLoopEnd :
			pop		edi
				pop		esi
		}
	}
}
//End of inverted RLC with clipping & encoding(half-transparent fog)

void ShowRLCp1( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL1 );
}
void ShowRLCp2( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL2 );
}
void ShowRLCp3( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL3 );
}
void ShowRLCp4( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL4 );
}
void ShowRLCp5( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL5 );
}
void ShowRLCp6( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL6 );
}
void ShowRLCp7( int x, int y, void* PicPtr )
{
	ShowRLCpal( x, y, PicPtr, PAL7 );
}
void ShowRLCip1( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL1 );
}
void ShowRLCip2( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL2 );
}
void ShowRLCip3( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL3 );
}
void ShowRLCip4( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL4 );
}
void ShowRLCip5( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL5 );
}
void ShowRLCip6( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL6 );
}
void ShowRLCip7( int x, int y, void* PicPtr )
{
	ShowRLCipal( x, y, PicPtr, PAL7 );
}

extern byte fog[8192 + 1024];
extern byte wfog[8192];
extern byte yfog[8192];
extern byte rfog[8192];
extern byte trans8[65536];

void ShowRLCShadow( int x, int y, void* PicPtr )
{
	ShowRLCfonpal( x, y, PicPtr, fog + 4096 );
}
void ShowRLCiShadow( int x, int y, void* PicPtr )
{
	ShowRLCifonpal( x, y, PicPtr, fog + 4096 );
}
void ShowRLCWhite( int x, int y, void* PicPtr )
{
	ShowRLCfonpal( x, y, PicPtr, wfog + 1024 );
}
void ShowRLCDarkN( int x, int y, void* PicPtr, int N )
{
	ShowRLCpal( x, y, PicPtr, wfog + ( N << 8 ) );
};
void ShowRLCiDarkN( int x, int y, void* PicPtr, int N )
{
	ShowRLCipal( x, y, PicPtr, wfog + ( N << 8 ) );
};
void ShowRLCRedN( int x, int y, void* PicPtr, int N )
{
	ShowRLCpal( x, y, PicPtr, yfog + ( N << 8 ) );
};
void ShowRLCiRedN( int x, int y, void* PicPtr, int N )
{
	ShowRLCipal( x, y, PicPtr, yfog + ( N << 8 ) );
};
void ShowRLCItemDarkN( int x, int y, lpRLCTable lprt, int n, int Ints )
{
	if (n < 4096)
	{
		ShowRLCDarkN( x, y, (void*) ( ( *lprt )->OfsTable[n] ), Ints );
	}
	else
	{
		ShowRLCiDarkN( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ), Ints );
	};
};
void ShowRLCItemPal( int x, int y, lpRLCTable lprt, int n, byte* Pal )
{
	if (n < 4096)
	{
		ShowRLCpal( x, y, (void*) ( ( *lprt )->OfsTable[n] ), Pal );
	}
	else
	{
		ShowRLCipal( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ), Pal );
	};
};
void ShowRLCItemGrad( int x, int y, lpRLCTable lprt, int n, byte* Pal )
{
	if (n < 4096)
	{
		ShowRLChtpal( x, y, (void*) ( ( *lprt )->OfsTable[n] ), Pal );
	}
	else
	{
		ShowRLCihtpal( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ), Pal );
	};
};
void ShowRLCItemRedN( int x, int y, lpRLCTable lprt, int n, int Ints )
{
	if (n < 4096)
	{
		ShowRLCRedN( x, y, (void*) ( ( *lprt )->OfsTable[n] ), Ints );
	}
	else
	{
		ShowRLCiRedN( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ), Ints );
	};
};
void ShowRLCWFog( int x, int y, void* PicPtr )
{
	ShowRLChtpal( x, y, PicPtr, wfog + 1024 );
}
void ShowRLCiWhite( int x, int y, void* PicPtr )
{
	ShowRLCifonpal( x, y, PicPtr, wfog + 1024 );
}
void ShowRLCiWFog( int x, int y, void* PicPtr )
{
	ShowRLCihtpal( x, y, PicPtr, wfog + 1024 );
}
void ShowRLCiTrans8( int x, int y, void* PicPtr )
{
	ShowRLCihtpal( x, y, PicPtr, trans8 );
}
void ShowRLCTrans8( int x, int y, void* PicPtr )
{
	ShowRLChtpal( x, y, PicPtr, trans8 );
}
void ShowRLCDark( int x, int y, void* PicPtr )
{
	ShowRLChtpal( x, y, PicPtr, fog + 1024 );
}
void ShowRLCiDark( int x, int y, void* PicPtr )
{
	ShowRLCihtpal( x, y, PicPtr, fog + 1024 );
}
void ShowRLCBlue( int x, int y, void* PicPtr )
{
	ShowRLChtpal( x, y, PicPtr, rfog + 2048 );
}
void ShowRLCiBlue( int x, int y, void* PicPtr )
{
	ShowRLCihtpal( x, y, PicPtr, rfog + 2048 );
}
void ShowRLCFire( int x, int y, void* PicPtr )
{
	ShowRLChtpal( x, y, PicPtr, yfog );
}
void ShowRLCiFire( int x, int y, void* PicPtr )
{
	ShowRLCihtpal( x, y, PicPtr, yfog );
}

//Load rlc file, allocate and fill provided RLCTable
bool LoadRLC( LPCSTR lpFileName, RLCTable *RLCtbl )
{
	ResFile f1 = RReset( lpFileName );
	if (INVALID_HANDLE_VALUE != f1)
	{
		DWORD fsz = RFileSize( f1 );
		LOADED += fsz;

		*RLCtbl = (RLCTable) malloc( fsz + 4 );
		( *RLCtbl )->Size = fsz + 4;

		RBlockRead( f1, &( ( *RLCtbl )->sign ), fsz );
		RClose( f1 );

		int shft = int( *RLCtbl ) + 4;
		int cnt = ( ( *RLCtbl )->SCount & 65535 );
		for (int i = 0; i < cnt; i++)
		{
			( *RLCtbl )->OfsTable[i] += shft;
		}

		return true;
	}

	char gg[128];
	sprintf( gg, "Could not load RLC file: %s", lpFileName );
	ErrM( gg );

	return false;
}

void ShowRLCItem( int x, int y, lpRLCTable lprt, int n, byte nt )
{
	cntr++;
	if (cntr > 64 && !InCycle)
	{
		InCycle = true;
		ProcessMessages();
		InCycle = false;
		cntr = 0;
	}

	int GPID = int( *lprt );

	if (GPID < 4096)
	{
		GPS.ShowGP( x, y, GPID, n, nt );
		return;
	}

	if (n < 4096)
	{
		switch (nt)
		{
		case 1:
			ShowRLCp1( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 2:
			ShowRLCp2( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 3:
			ShowRLCp3( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 4:
			ShowRLCp4( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 5:
			ShowRLCp5( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 6:
			ShowRLCp6( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		case 7:
			ShowRLCp7( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
			break;
		default:
			ShowRLC( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
		}
	}
	else
	{
		switch (nt)
		{
		case 1:
			ShowRLCip1( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 2:
			ShowRLCip2( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 3:
			ShowRLCip3( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 4:
			ShowRLCip4( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 5:
			ShowRLCip5( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 6:
			ShowRLCip6( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		case 7:
			ShowRLCip7( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
			break;
		default:
			ShowRLCi( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
		}
	}
}

void ShowRLCItemTrans8( int x, int y, lpRLCTable lprt, int n )
{
	if (n < 4096)
	{
		ShowRLCTrans8( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
	}
	else
	{
		ShowRLCiTrans8( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
	};
};

void ShowRLCItemMutno( int x, int y, lpRLCTable lprt, int n )
{
	if (n < 4096)
	{
		ShowRLCWFog( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
	}
	else
	{
		ShowRLCiWFog( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
	};
};
void ShowRLCItemDark( int x, int y, lpRLCTable lprt, int n )
{
	if (n < 4096)
	{
		ShowRLCDark( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
	}
	else
	{
		ShowRLCiDark( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
	};
};
void ShowRLCItemBlue( int x, int y, lpRLCTable lprt, int n )
{
	if (n < 4096)
	{
		ShowRLCBlue( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
	}
	else
	{
		ShowRLCiBlue( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
	};
};
void ShowRLCItemFired( int x, int y, lpRLCTable lprt, int n )
{
	if (n < 4096)
	{
		ShowRLCFire( x, y, (void*) ( ( *lprt )->OfsTable[n] ) );
	}
	else
	{
		ShowRLCiFire( x, y, (void*) ( ( *lprt )->OfsTable[n - 4096] ) );
	};
};
int GetRLCWidth( RLCTable lpr, byte n )
{
	int GPID = int( lpr );
	if (GPID < 4096)
	{
		if (n == 32)
		{
			return GPS.GetGPWidth( GPID, 'c' );
		}
		return GPS.GetGPWidth( GPID, n );
	};
	if (n < lpr->SCount)
	{
		return ( *( (lpRLCHeader) ( (void*) ( lpr->OfsTable[n] ) ) ) ).SizeX;
	}
	else return 0;
}
int GetCHEX( byte c )
{
	if (c >= '0'&&c <= '9')return c - '0';
	if (c >= 'a'&&c <= 'z')return c + 10 - 'a';
	if (c >= 'A'&&c <= 'Z')return c + 10 - 'A';
	return 0;
}

int GetRLCWidthUNICODE( RLCTable lpr, byte* strptr, int* L )
{
	if (strptr[0] == SIGNBYTE)//FUNNY: Funny shit, Sherlock. DEL, srsly?!
	{
		if (L)*L = 5;
		int idx = ( GetCHEX( strptr[1] ) << 12 ) +
			( GetCHEX( strptr[2] ) << 8 ) +
			( GetCHEX( strptr[3] ) << 4 ) +
			GetCHEX( strptr[4] );
		int GPID = int( lpr );
		if (GPID < 4096)
		{
			UNICODETABLE* UT = GPS.UNITBL[GPID];
			if (!UT)return 0;
			for (int j = 0; j < UT->NTables; j++)
			{
				if (idx >= UT->USET[j].Start)
				{
					int v = idx - UT->USET[j].Start;
					if (v < UT->USET[j].NSymbols)
					{
						v += UT->USET[j].GP_Start;
						return GPS.GetGPWidth( UT->USET[j].GPID, v );
					}
				}
			}
			return 0;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (L)
		{
			*L = 1;
		}
		return GetRLCWidth( lpr, strptr[0] );
	}
}

__declspec( dllexport ) int GetRLCHeight( RLCTable lpr, byte n )
{
	int GPID = int( lpr );
	if (GPID < 4096)
	{
		return GPS.GetGPHeight( GPID, n );
	}
	if (n < lpr->SCount)
	{
		return ( *( (lpRLCHeader) ( (void*) ( lpr->OfsTable[n] ) ) ) ).SizeY;
	}
	else
	{
		return 0;
	}
}

void RegisterRLCFont( lpRLCFont lrf, RLCTable lpr, int fir )
{
	lrf->FirstSymbol = fir;
	lrf->LastSymbol = lpr->SCount + fir - 1;
	lrf->RLC = lpr;
}
void CheckFontColor( lpRLCFont lpf )
{
	int GPID = int( lpf->RLC );
	if (GPID < 4096)GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpf->Options;
};
void ShowChar( int x, int y, char c, lpRLCFont lpf )
{
	int GPID = int( lpf->RLC );
	if (GPID < 4096)
	{
		GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpf->Options;
		GPS.ShowGP( x, y, GPID, byte( c ), 0 );
		return;
	};
	ShowRLCItem( x, y, &( lpf->RLC ), c, 0 );
}

__declspec( dllexport ) void ShowCharUNICODE( int x, int y, byte* strptr, lpRLCFont lpr )
{
	if (strptr[0] == SIGNBYTE)
	{
		int idx = ( GetCHEX( strptr[1] ) << 12 ) +
			( GetCHEX( strptr[2] ) << 8 ) +
			( GetCHEX( strptr[3] ) << 4 ) +
			GetCHEX( strptr[4] );
		int GPID = int( lpr->RLC );
		if (GPID < 4096)
		{
			UNICODETABLE* UT = GPS.UNITBL[GPID];
			if (UT)
			{
				for (int j = 0; j < UT->NTables; j++)
				{
					if (idx >= UT->USET[j].Start)
					{
						int v = idx - UT->USET[j].Start;
						if (v < UT->USET[j].NSymbols)
						{
							v += UT->USET[j].GP_Start;
							GPID = UT->USET[j].GPID;
							if (lpr->ShadowGP != -1)
							{
								GPS.ImageType[lpr->ShadowGP] = 1;
								GPS.ShowGP( x + UT->USET[j].DX + lpr->ShadowDx, y + UT->USET[j].DY + lpr->ShadowDy, GPID, v, 0 );
							}
							if (UT->USET[j].UseColor)
							{
								GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpr->Options;
							}
							else
							{
								GPS.ImageType[GPID] = 1;
							}
							GPS.ShowGP( x + UT->USET[j].DX, y + UT->USET[j].DY, GPID, v, 0 );
							return;
						}
					}
				}
			}
		}
	}
	else
	{
		if (lpr->ShadowGP != -1)
		{
			GPS.ImageType[lpr->ShadowGP] = 1;
			GPS.ShowGP( x + lpr->ShadowDx, y + lpr->ShadowDy, lpr->ShadowGP, strptr[0], 0 );
		}
		ShowChar( x, y, strptr[0], lpr );
	}
}

void ShowString( int x, int y, LPCSTR lps, lpRLCFont lpf )
{
	if (nullptr == lps)
	{
		return;
	}

	int GPID = int( lpf->RLC );

	if (GPID < 4096)
	{
		GPS.ImageType[GPID] = ( GPS.ImageType[GPID] & 7 ) | lpf->Options;
		byte ch;
		int i = 0;
		do
		{
			ch = lps[i];
			if (ch != 0)
			{
				ShowCharUNICODE( x, y, (byte*) ( lps + i ), lpf );
				int L = 1;
				x += GetRLCWidthUNICODE( lpf->RLC, (byte*) ( lps + i ), &L );
				i += L - 1;
			}
			i++;
		} while (ch);

		return;
	}

	byte ch;
	int i = 0;
	do
	{
		ch = lps[i];
		if (ch != 0)
		{
			ShowRLCItem( x, y, &( lpf->RLC ), lpf->FirstSymbol + ch, 0 );
			x += GetRLCWidth( lpf->RLC, lpf->FirstSymbol + ch );
		}
		i++;
	} while (ch);
}

int GetRLCStrWidth( char* str, lpRLCFont lpf )
{
	if (!str)return 0;
	int L = 0;
	int sl = strlen( str );
	for (int i = 0; i < sl; i++)
	{
		int LL = 1;
		L += GetRLCWidthUNICODE( lpf->RLC, (byte*) ( str + i ), &LL );
		i += LL - 1;
	};
	return L;
};
void ShowShadString( int x, int y, LPCSTR lps, lpRLCFont lpf )
{
	if (lps == nullptr) return;
	byte	ch;
	int		i = 0;
	do
	{
		ch = lps[i];
		if (ch != 0)
		{
			ShowRLCItem( x, y, &( lpf->RLC ), lpf->FirstSymbol + ch, 0 );
			int LL = 1;
			x += GetRLCWidthUNICODE( lpf->RLC, (byte*) ( lps + i ), &LL );
			i += LL - 1;
		}
		i++;
	} while (ch - 0);
}

void LoadPalettes()//IMPORTANT: load color palettes for ???
{
	ResFile f1 = RReset( "pal1.dat" );
	RBlockRead( f1, PAL1, 256 );
	RClose( f1 );
	f1 = RReset( "pal2.dat" );
	RBlockRead( f1, PAL2, 256 );
	RClose( f1 );
	f1 = RReset( "pal3.dat" );
	RBlockRead( f1, PAL3, 256 );
	RClose( f1 );
	f1 = RReset( "pal4.dat" );
	RBlockRead( f1, PAL4, 256 );
	RClose( f1 );
	f1 = RReset( "pal5.dat" );
	RBlockRead( f1, PAL5, 256 );
	RClose( f1 );
	f1 = RReset( "pal6.dat" );
	RBlockRead( f1, PAL6, 256 );
	RClose( f1 );
	f1 = RReset( "pal7.dat" );
	RBlockRead( f1, PAL7, 256 );
	RClose( f1 );
}

RLCFont::RLCFont( char* Name )
{
	int LOLD = LOADED;
	LoadRLC( Name, &RLC );
	RegisterRLCFont( this, RLC, 0 );
	LOADED = LOLD;
	Options = 0;
	ShadowGP = -1;
}

void RLCFont::SetRedColor()
{
	Options = 32;
	//SetStdShadow();
}

void RLCFont::SetWhiteColor()
{
	Options = 16;
	//SetStdShadow();
}

void RLCFont::SetBlackColor()
{
	Options = 0;
}

void RLCFont::SetColorTable( int n )
{
	Options = n << 4;
	//SetStdShadow();
}
/* End of Graphics routine */