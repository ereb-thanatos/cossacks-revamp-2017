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
#include "NewMon.h"
#include "ZBuffer.h"
#include "TopoGraf.h"
#include "MapSprites.h"
int MaxTex;
void ErrM(char* s);
//#include "3DLow.h"
bool Mode3D;
void CreateRandomHMap();
extern int mul3(int);
void CBar(int x, int y, int Lx, int Ly, byte c);
int Buf3Dofs;
int Buf3DLx;
int Buf3DLy;
int x03D;
int x13D;
int y03D;
int y13D;
extern int LightDX;
extern int LightDY;
extern int LightDZ;
#define Max3DLy 1024
extern byte darkfog[40960];
word VertBuf[Max3DLy * 16];
//word x1  -->Dx
//word x2  -->Lx
//int res         BMX
//int bmxy        BMY
//int fog
//byte TempDest[800000];
static int curXT;
static int curYT;
static int curRX;
static int curRY;
static int curScrOfs;
static int CurFog;
static int FogDx;
static int FogDy;
static int BMxy;
static int BMdy;
static int BMdx;
static int OffsetX;
static int OffsetY;
bool PrepareToRender(int NLines, int startX, int startY) {
	word curx;
	__asm {
		mov		ax, VertBuf[0]
		mov		bx, VertBuf[2]
		cmp		ax, 0xFFFF
		je		uu1
		cmp		bx, 0xFFFF
		je		uu3
		cmp		ax, bx
		jb		uu3
		mov		curx, bx
		jmp		uu5
		uu3 : mov		curx, ax
			  jmp		uu5
			  uu1 : mov		curx, 0xFFFF
					uu5 :
	};
	OffsetX = -curx + startX;
	OffsetY = -startY;
	if (curx == 0xFFFF)return false;
	int bmdxx = (BMdx & 0xFFFF);
	int bmdxy = (BMdy & 0xFFFF);
	int bmx0 = ((BMxy & 0xFFFF) + bmdxx*(curx - startX) - bmdxy*startY) & 16382;
	int bmdyy = (BMdy >> 16);
	int bmdyx = (BMdx >> 16);
	int bmy0 = ((BMxy >> 16) + bmdyx*(curx - startX) - bmdyy*startY) & 16382;
	int bmstart = bmx0 + (bmy0 << 16);//BMxy-BMdx*(curx-startX)-BMdy*startY;
	int	fogstart = CurFog + FogDx*(curx - startX) - FogDy*startY;
	__asm {
		push	edi
		push	esi
		pushf
		mov		esi, offset VertBuf
		mov		ecx, NLines
		mov		bx, curx
		mov		edi, bmstart
		per1 : mov		bx, curx
			   mov		dx, [esi]
			   mov		ax, [esi + 2]
			   cmp		ax, 0xFFFF
			   jne		per2
			   mov		ax, dx
			   per2 : cmp		dx, ax
					  jb		per3
					  xchg	dx, ax	//now dx<=ax
					  per3 : sub		ax, dx
							 mov[esi + 2], ax  //now ax is free
							 cmp		dx, bx		//dx=x1,bx=curx
							 je		x1eqCurx
							 jae		x1moreCurx
							 //need to move left,x1<curx
							 mov		ax, bx
							 sub		ax, dx
							 mov		ebx, fogstart
							 per4 : sub		edi, BMdx
									//and     edi,00111111111111110011111111111111b
									sub		ebx, FogDx
									dec		ax
									jnz		per4
									mov		fogstart, ebx
									jmp		PrepareToNextLine
									x1moreCurx :
		mov		ax, dx
			sub		ax, bx
			mov		ebx, fogstart
			per5 : add		edi, BMdx
			//and     edi,00111111111111110011111111111111b
			add		ebx, FogDx
			dec		ax
			jnz		per5
			mov		fogstart, ebx
			x1eqCurx :
	PrepareToNextLine:
		mov		ebx, fogstart
			mov[esi + 12], ebx
			add		ebx, FogDy
			mov		fogstart, ebx
			mov		bx, curx
			mov		curx, dx
			sub		dx, bx
			mov[esi], dx
			mov[esi + 8], edi
			add		edi, BMdy   //edi=BMxy
			//and     edi,00111111111111110011111111111111b
			add		esi, 16
			dec		ecx
			jnz		per1
			popf
			pop		esi
			pop		edi
	};
	//Now we are ready to render trrriangle !!!!!!!!!!!!
	return true;
};
void addLine(int x, int y) {
	int sdx;
	int sdy = 16;
	int dy = abs(y - curYT) + 1;
	if (y < curYT)sdy = -16;
	if (y != curYT) sdx = div((x - curXT) << 16, abs(y - curYT)).quot;
	__asm {
		push	edi
		push	esi
		pushf
		mov		ebx, curYT
		mov		ecx, dy
		mov		eax, sdx
		mov		edi, sdy
		shl		ebx, 4
		add		ebx, offset VertBuf
		mov		esi, curXT
		shl		esi, 16
		add		esi, 32768
		lp1:	mov		edx, esi
				shr		edx, 16
				cmp		word ptr[ebx], 0xFFFF
				jne		lp2
				mov[ebx], dx
				lp3 : add		esi, eax
					  add		ebx, edi
					  dec		ecx
					  jnz		lp1
					  jmp		lp_end
					  lp7 : cmp		dx, word ptr[ebx]
							je		lp3
							cmp		dx, word ptr[ebx + 2]
							je		lp3
							mov[ebx + 2], dx
							add		esi, eax
							add		ebx, edi
							dec		ecx
							jnz		lp1
							lp2 : cmp		word ptr[ebx + 2], 0xFFFF
								  jne		lp7
								  mov[ebx + 2], dx
								  add		esi, eax
								  add		ebx, edi
								  dec		ecx
								  jnz		lp1
								  lp_end : popf
										   pop		esi
										   pop		edi
	};
	curXT = x;
	curYT = y;
};
//RENDERING THE TRIANGLE. 
//It is really important. Before this oeration
//VertBuf must be filled by values of x1,Lx,BMxy,Fog
//OUTPUT:
// word  NLines
// For every line:
// word  Dx
// short Lx 
// byte  data[Lx]
// ...
int BPtemp;
void RenderTriangle64(int NLines, byte* Dest, byte* Bitmap) {
	((word*)Dest)[0] = NLines;
	int Startscan;
	int ScanSize;
	int VertPos;
	((int*)Dest)[1] = OffsetX;
	((int*)Dest)[2] = OffsetY;
	int VBpos = int(VertBuf);
	__asm {
		push	esi
		push	edi
		pushf
		mov		esi, Bitmap
		mov		edi, Dest
		add		edi, 12
		mov		ebx, VBpos
		cld
		StartLine :
		//Rendering the linear transformation
		mov		ecx, [ebx]
			mov[edi], ecx
			add		edi, 4     //edi points to Dest
			shr		ecx, 16    //Length of the scan line
			mov		ScanSize, ecx
			mov		Startscan, edi  //Storing for shadowing
			mov		edx, [ebx + 8]    //BMxy
			mov		VertPos, ebx
			mov		ebx, BMdx
			//and     ebx,0011 1111 1111 1110 0011 1111 1111 1110b
			jcxz	StartShad
			StartLinear :
		mov		eax, edx     //1
			shr		eax, 16      //1
			mov		al, dh       //1
			mov		al, [esi + eax]//?
			add		edx, ebx     //0
			dec		cx          //0
			stosb               //0  could be optimized to stosd
			jnz		StartLinear //1
	//Shadow processing
			StartShad :
		xchg	edi, Startscan
			mov		esi, VertPos
			mov		ecx, ScanSize
			mov		ebx, [esi + 12]   //fog
			mov		edx, FogDx
			//mov		edi,Startscan
			jcxz	endfog
			StartFog4 :      //Not optimal now! 32 bit reading coulde performed
		mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jnz		StartFog4
			endfog : mov		edi, Startscan
			mov		ebx, esi
			mov		esi, Bitmap
			add		ebx, 16
			dec		NLines
			jnz		StartLine
			popf
			pop		esi
			pop		edi
	};
};
int GetMax(int z1, int z2, int z3) {
	if (z1 > z2) {
		if (z3 > z1)return z3;
		else return z1;
	}
	else {
		if (z3 > z2)return z3;
		else return z2;
	};
};
int GetMin(int z1, int z2, int z3) {
	if (z1 < z2) {
		if (z3 < z1)return z3;
		else return z1;
	}
	else {
		if (z3 < z2)return z3;
		else return z2;
	};
};
void PreRenderTri64(int x1, int y1,
	int x2, int y2,
	int x3, int y3,
	int bmxy, int bmdx,
	int bmdy, int fog, int fogdx, int fogdy,
	byte* bitm,
	byte* Dest) {
	int z1 = GetMin(y1, y2, y3);
	int z2 = GetMax(y1, y2, y3);
	curScrOfs = x1 + y1*Buf3DLx;
	CurFog = fog;
	curXT = x1;
	curYT = y1 - z1;
	FogDx = fogdx;
	FogDy = fogdy;
	BMxy = bmxy;
	BMdx = bmdx;
	BMdy = bmdy;
	int VBpos = int(VertBuf);
	//Инициализация VertBuf
	__asm {
		push	edi
		pushf
		cld
		mov		edi, VBpos
		mov		ecx, z2
		sub		ecx, z1
		inc		ecx
		uux1 : mov		dword ptr[edi], 0xFFFFFFFF;
		add		edi, 16
			dec		ecx
			jnz		uux1
			popf
			pop		edi
	};
	addLine(x2, y2 - z1);
	addLine(x3, y3 - z1);
	addLine(x1, y1 - z1);
	PrepareToRender(z2 - z1 + 1, x1, y1 - z1);
	RenderTriangle64(z2 - z1 + 1, Dest, bitm);
};
void ShowTriangle(int x, int y, byte* data) {
	int dx = ((int*)data)[1];
	int dy = ((int*)data)[2];
	int ofst = int(ScreenPtr) + (y + dy)*ScrWidth + x - dx;
	int dofs = int(data);
	__asm {
		push	esi
		push	edi
		pushf
		mov		edi, ofst
		mov		esi, dofs
		cld
		mov		bx, [esi]
		add		esi, 12
		xor ecx, ecx
		xor		eax, eax
		lpp1 :	//mov		TempDI,edi
		xor		eax, eax
			mov		ax, [esi]
			test	ax, 8000h
			jz		uu1
			or eax, 0xFFFF0000
			uu1 :
			add		edi, eax
			mov		cx, [esi + 2]
			add		esi, 4
			mov		edx, ecx
			shr		ecx, 2
			rep		movsd
			mov		ecx, edx
			and		ecx, 3
			rep		movsb
			//mov		edi,TempDI
			sub		edi, edx
			add		edi, ScrWidth
			dec		bx
			jnz		lpp1
			popf
			pop		edi
			pop		esi
	};
};
void ShowClippedTriangle(int x, int y, byte* data) {
	int dx = ((int*)data)[1];
	int dy = ((int*)data)[2];
	int x1 = x + dx;
	int y1 = y + dy;
	int dofs = int(data);
	int realof = dofs + 12;
	int NY = (((int*)data)[0]) & 65535;
	int DOfst = 0;
	if (y1 < WindY) {
		int rdy = WindY - y1;
		if (NY <= rdy)return;
		NY -= rdy;
		y1 = WindY;
		__asm {
			mov		ecx, rdy
			mov		ebx, realof
			mov		edx, DOfst
			lopp1 : mov		ax, [ebx]
					movsx	eax, ax
					add		edx, eax
					xor		eax, eax
					mov		ax, [ebx + 2]
					add		ebx, eax
					add		ebx, 4
					dec		ecx
					jnz		lopp1
					//test	dx,0x8000
					//jz		lopp2
					//or		edx,0xFFFF0000
					mov		DOfst, edx
					mov		realof, ebx
		};
	};
	if (y1 + NY > WindY1)NY = WindY1 - y1 + 1;
	if (NY <= 0)return;
	int ofst = int(ScreenPtr) + y1*ScrWidth + x - dx;
	int TempDI;
	int MinOfst = int(ScreenPtr) + y1*ScrWidth + WindX;
	int MaxOfst = MinOfst + WindLx;
	int  FinalESI;
	__asm {
		push	esi
		push	edi
		pushf
		mov		edi, ofst
		add		edi, DOfst
		mov		esi, dofs
		cld
		mov		bx, word ptr NY
		mov		esi, realof
		xor		ecx, ecx
		xor		eax, eax
		lpp1 :	//mov		TempDI,edi
		xor		eax, eax
			mov		ax, [esi]
			test	ax, 8000h
			jz		uu1
			or eax, 0xFFFF0000
			uu1 :
			add		edi, eax
			xor		ecx, ecx
			mov		cx, [esi + 2]
			add		esi, 4
			mov		FinalESI, esi
			add		FinalESI, ecx
			mov		edx, ecx
			add		edx, edi
			mov		TempDI, edi
			cmp		edi, MinOfst
			jae		uuu3
			sub		edi, MinOfst
			sub		esi, edi
			mov		edi, MinOfst
			uuu3 : cmp		edx, MaxOfst
			jbe		uuu4

			mov		edx, MaxOfst
			uuu4 : sub		edx, edi
			mov		ecx, edx
			cmp		ecx, 0
			jle		ttr
			shr		ecx, 2
			rep		movsd
			mov		ecx, edx
			and		ecx, 3
			rep		movsb
			ttr :
		mov		esi, FinalESI
			mov		edi, TempDI
			//sub		edi,edx
			add		edi, ScrWidth
			mov		eax, MinOfst
			add		eax, ScrWidth
			mov		MinOfst, eax
			mov		eax, MaxOfst
			add		eax, ScrWidth
			mov		MaxOfst, eax
			dec		bx
			jnz		lpp1
			popf
			pop		edi
			pop		esi
	};
};
void ShowClippedTriangleMMX(int x, int y, byte* data) {
	int dx = ((int*)data)[1];
	int dy = ((int*)data)[2];
	int x1 = x + dx;
	int y1 = y + dy;
	int dofs = int(data);
	int realof = dofs + 12;
	int NY = (((int*)data)[0]) & 65535;
	int DOfst = 0;
	if (y1 < WindY) {
		int rdy = WindY - y1;
		if (NY <= rdy)return;
		NY -= rdy;
		y1 = WindY;
		__asm {
			mov		ecx, rdy
			mov		ebx, realof
			mov		edx, DOfst
			lopp1 : mov		ax, [ebx]
					movsx	eax, ax
					add		edx, eax
					xor		eax, eax
					mov		ax, [ebx + 2]
					add		ebx, eax
					add		ebx, 4
					dec		ecx
					jnz		lopp1
					//test	dx,0x8000
					//jz		lopp2
					//or		edx,0xFFFF0000
					mov		DOfst, edx
					mov		realof, ebx
		};
	};
	if (y1 + NY >= WindY1)NY = WindY1 - y1 + 1;
	if (NY <= 0)return;
	int ofst = int(ScreenPtr) + y1*ScrWidth + x - dx;
	int TempDI;
	int MinOfst = int(ScreenPtr) + y1*ScrWidth + WindX;
	int MaxOfst = MinOfst + WindLx;
	int  FinalESI;
	__asm {
		push	esi
		push	edi
		pushf
		mov		edi, ofst
		add		edi, DOfst
		mov		esi, dofs
		cld
		mov		bx, word ptr NY
		mov		esi, realof
		xor		ecx, ecx
		xor		eax, eax
		lpp1 :	//mov		TempDI,edi
		xor		eax, eax
			mov		ax, [esi]
			test	ax, 8000h
			jz		uu1
			or eax, 0xFFFF0000
			uu1 :
			add		edi, eax
			xor		ecx, ecx
			mov		cx, [esi + 2]
			add		esi, 4
			mov		FinalESI, esi
			add		FinalESI, ecx
			mov		edx, ecx
			add		edx, edi
			mov		TempDI, edi
			cmp		edi, MinOfst
			jae		uuu3
			sub		edi, MinOfst
			sub		esi, edi
			mov		edi, MinOfst
			uuu3 : cmp		edx, MaxOfst
			jbe		uuu4

			mov		edx, MaxOfst
			uuu4 : sub		edx, edi
			mov		ecx, edx
			cmp		ecx, 0
			jle		ttr
			shr		ecx, 3
			jcxz	noMMX
			MMXLoop :
		movq	mm1, [esi]
			add		esi, 8
			movq[edi], mm1
			add		edi, 8
			dec		cx
			jnz		MMXLoop
			//		rep		movsd
			noMMX :
		mov		ecx, edx
			and		ecx, 7
			rep		movsb
			ttr :
		mov		esi, FinalESI
			mov		edi, TempDI
			//sub		edi,edx
			add		edi, ScrWidth
			mov		eax, MinOfst
			add		eax, ScrWidth
			mov		MinOfst, eax
			mov		eax, MaxOfst
			add		eax, ScrWidth
			mov		MaxOfst, eax
			dec		bx
			jnz		lpp1
			popf
			pop		edi
			pop		esi
			EMMS
	};
};
//--------------------------------------------------------------
void RenderSmartTriangle64(int xs1, int ys1,
	int xs2, int ys2,
	int xs3, int ys3,
	int xb1, int yb1,
	int xb2, int yb2,
	int xb3, int yb3,
	int f1, int f2, int f3,
	byte * Dest, byte* Bitm) {
	int dxb2 = xb2 - xb1;
	int dxb3 = xb3 - xb1;
	int dyb2 = yb2 - yb1;
	int dyb3 = yb3 - yb1;
	int dxs2 = xs2 - xs1;
	int dxs3 = xs3 - xs1;
	int dys2 = ys2 - ys1;
	int dys3 = ys3 - ys1;

	int D = dxs2*dys3 - dys2*dxs3;
	if (!D) {
		Dest = NULL;
		return;
	};
	int Axx = dys3*dxb2 - dxb3*dys2;
	int Axy = dxs2*dxb3 - dxb2*dxs3;
	int Ayx = dys3*dyb2 - dyb3*dys2;
	int Ayy = dxs2*dyb3 - dyb2*dxs3;

	Axx = div(Axx << 8, D).quot;
	Ayy = div(Ayy << 8, D).quot;
	Axy = div(Axy << 8, D).quot;
	Ayx = div(Ayx << 8, D).quot;
	int bmxy = ((xb1 & 63) << 8) + ((yb1 & 63) << 24);
	int bmdx = word(Axx) + (word(Ayx) << 16);
	int bmdy = word(Axy) + (word(Ayy) << 16);
	//Fogging
	int DScr = dys2*dxs3 - dxs2*dys3;
	int FDx = 0;
	int FDy = 0;
	if (DScr) {
		FDx = div((f3*dys2 - dys3*f2) << 16, DScr).quot;
		FDy = div((f2*dxs3 - dxs2*f3) << 16, DScr).quot;
	};
	PreRenderTri64(xs1, ys1, xs2, ys2, xs3, ys3, bmxy, bmdx, bmdy, f1 << 16, FDx, FDy,
		Bitm, Dest);

};
//----------SLOW, BUT PRECISE VARIANT OF 3D GRAPHICS------------
int BMX;
int BMY;
int BMDXX;
int BMDYY;
int BMDXY;
int BMDYX;
static int TempEBP;
static int BMXStart;
static int BMYStart;
static int	fogstart;
static word curx;
static int FOG1;
bool PrecPrepareToRender(int NLines, int startX, int startY) {
	__asm {
		mov		ax, VertBuf[0]
		mov		bx, VertBuf[2]
		cmp		ax, 0xFFFF
		je		uu1
		cmp		bx, 0xFFFF
		je		uu3
		cmp		ax, bx
		jb		uu3
		mov		curx, bx
		jmp		uu5
		uu3 : mov		curx, ax
			  jmp		uu5
			  uu1 : mov		curx, 0xFFFF
					uu5 :
	};
	OffsetX = -curx + startX;
	OffsetY = -startY;
	if (curx == 0xFFFF)return false;
	BMXStart = BMX + BMDXX*(curx - startX) - BMDXY*startY;
	BMYStart = BMY + BMDYX*(curx - startX) - BMDYY*startY;
	fogstart = FOG1 + FogDx*(curx - startX) - FogDy*startY;
	__asm {
		push	edi
		push	esi
		pushf
		mov		esi, offset VertBuf
		mov		ecx, NLines
		mov		bx, curx
		mov		edi, BMXStart
		mov		TempEBP, EBP
		mov		ebp, BMYStart
		per1 : mov		bx, curx
			   mov		dx, [esi]
			   mov		ax, [esi + 2]
			   cmp		ax, 0xFFFF
			   jne		per2
			   mov		ax, dx
			   per2 : cmp		dx, ax
					  jb		per3
					  xchg	dx, ax	//now dx<=ax
					  per3 : sub		ax, dx
							 mov[esi + 2], ax  //now ax is free
							 cmp		dx, bx		//dx=x1,bx=curx
							 je		x1eqCurx
							 jae		x1moreCurx
							 //need to move left,x1<curx
							 mov		ax, bx
							 sub		ax, dx
							 mov		ebx, fogstart
							 per4 : sub		edi, BMDXX
									sub		ebp, BMDYX
									//and     edi,00111111111111110011111111111111b
									sub		ebx, FogDx
									dec		ax
									jnz		per4
									mov		fogstart, ebx
									jmp		PrepareToNextLine
									x1moreCurx :
		mov		ax, dx
			sub		ax, bx
			mov		ebx, fogstart
			per5 : add		edi, BMDXX
			add		ebp, BMDYX
			//and     edi,00111111111111110011111111111111b
			add		ebx, FogDx
			dec		ax
			jnz		per5
			mov		fogstart, ebx
			x1eqCurx :
	PrepareToNextLine:
		mov		ebx, fogstart
			mov[esi + 12], ebx
			add		ebx, FogDy
			mov		fogstart, ebx
			mov		bx, curx
			mov		curx, dx
			sub		dx, bx
			mov[esi], dx
			mov[esi + 4], edi
			mov[esi + 8], ebp
			add		edi, BMDXY   //edi=BMxy
			add		ebp, BMDYY
			//and     edi,00111111111111110011111111111111b
			add		esi, 16
			dec		ecx
			jnz		per1
			mov		EBP, TempEBP
			popf
			pop		esi
			pop		edi
	};
	//Now we are ready to render trrriangle !!!!!!!!!!!!
	return true;
};
static word STRTX;
bool AbsolutePrecPrepareToRender(int NLines, int startX, int startY) {
	__asm {
		mov		ax, VertBuf[0]
		mov		bx, VertBuf[2]
		cmp		ax, 0xFFFF
		je		uu1
		cmp		bx, 0xFFFF
		je		uu3
		cmp		ax, bx
		jb		uu3
		mov		curx, bx
		jmp		uu5
		uu3 : mov		curx, ax
			  jmp		uu5
			  uu1 : mov		curx, 0xFFFF
					uu5 :
	};
	STRTX = startX;
	OffsetX = -curx + startX;
	OffsetY = -startY;
	if (curx == 0xFFFF)return false;
	BMXStart = BMX + BMDXX*(curx - startX) - BMDXY*startY;
	BMYStart = BMY + BMDYX*(curx - startX) - BMDYY*startY;
	fogstart = FOG1 + FogDx*(curx - startX) - FogDy*startY;
	__asm {
		push	edi
		push	esi
		pushf
		mov		esi, offset VertBuf
		mov		ecx, NLines
		mov		bx, curx
		mov		edi, BMXStart
		mov		TempEBP, EBP
		mov		ebp, BMYStart
		per1 : mov		bx, curx
			   mov		dx, [esi]
			   mov		ax, [esi + 2]
			   cmp		ax, 0xFFFF
			   jne		per2
			   mov		ax, dx
			   per2 : cmp		dx, ax
					  jb		per3
					  xchg	dx, ax	//now dx<=ax
					  per3 : sub		ax, dx
							 mov[esi + 2], ax  //now ax is free
							 cmp		dx, bx		//dx=x1,bx=curx
							 je		x1eqCurx
							 jae		x1moreCurx
							 //need to move left,x1<curx
							 mov		ax, bx
							 sub		ax, dx
							 mov		ebx, fogstart
							 per4 : sub		edi, BMDXX
									sub		ebp, BMDYX
									//and     edi,00111111111111110011111111111111b
									sub		ebx, FogDx
									dec		ax
									jnz		per4
									mov		fogstart, ebx
									jmp		PrepareToNextLine
									x1moreCurx :
		mov		ax, dx
			sub		ax, bx
			mov		ebx, fogstart
			per5 : add		edi, BMDXX
			add		ebp, BMDYX
			//and     edi,00111111111111110011111111111111b
			add		ebx, FogDx
			dec		ax
			jnz		per5
			mov		fogstart, ebx
			x1eqCurx :
	PrepareToNextLine:
		mov		ebx, fogstart
			mov[esi + 12], ebx
			add		ebx, FogDy
			mov		fogstart, ebx
			mov		bx, curx
			mov		curx, dx
			sub		dx, STRTX
			mov[esi], dx
			mov[esi + 4], edi
			mov[esi + 8], ebp
			add		edi, BMDXY   //edi=BMxy
			add		ebp, BMDYY
			//and     edi,00111111111111110011111111111111b
			add		esi, 16
			dec		ecx
			jnz		per1
			mov		EBP, TempEBP
			popf
			pop		esi
			pop		edi
	};
	//Now we are ready to render trrriangle !!!!!!!!!!!!
	return true;
};
//RENDERING THE TRIANGLE. 
//It is really important. Before this oeration
//VertBuf must be filled by values of x1,Lx,BMxy,Fog
//OUTPUT:
// word  NLines
// For every line:
// word  Dx
// short Lx 
// byte  data[Lx]
// ...
int PrecRenderTriangle64(int NLines, byte* Dest, byte* Bitmap) {
	((int*)Dest)[0] = NLines;
	int Startscan;
	int ScanSize;
	int VertPos;
	((int*)Dest)[1] = OffsetX;
	((int*)Dest)[2] = OffsetY;
	int VBpos = int(VertBuf);
	__asm {
		push	esi
		push	edi
		pushf
		mov		esi, Bitmap
		mov		edi, Dest
		add		edi, 12
		mov		ebx, VBpos
		cld
		StartLine :
		//Rendering the linear transformation
		mov		ecx, [ebx]
			mov[edi], ecx
			add		edi, 4     //edi points to Dest
			shr		ecx, 16    //Length of the scan line
			mov		ScanSize, ecx
			mov		Startscan, edi  //Storing for shadowing
			mov		edx, [ebx + 4]    //BMX
			mov		VertPos, ebx
			mov		ebx, [ebx + 8]    //BMY
			//and     ebx,0011 1111 1111 1110 0011 1111 1111 1110b
			jcxz	StartShad
			StartLinear :
		mov		eax, edx     //1
			ror		ebx, 8       //0
			shr		eax, 16      //1
			mov		ah, bh       //1
			and		ax, 0011111100111111b
			mov		al, [esi + eax]//?
			rol		ebx, 8       //0
			add		edx, BMDXX   //0
			add		ebx, BMDYX   //1
			dec		cx          //0
			stosb               //0  could be optimized to stosd
			jnz		StartLinear //1
	//Shadow processing
			StartShad :
		xchg	edi, Startscan
			mov		esi, VertPos
			mov		ecx, ScanSize
			mov		ebx, [esi + 12]   //fog
			mov		edx, FogDx
			//mov		edi,Startscan
			jcxz	endfog
			StartFog4 :      //Not optimal now! 32 bit reading coulde performed
		mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jnz		StartFog4
			endfog : mov		edi, Startscan
			mov		ebx, esi
			mov		esi, Bitmap
			add		ebx, 16
			dec		NLines
			jnz		StartLine
			sub		edi, Dest
			mov		eax, edi
			popf
			pop		esi
			pop		edi
	};
	return 0;
};
int PrecRenderTriangle128(int NLines, byte* Dest, byte* Bitmap) {
	((word*)Dest)[0] = NLines;
	int Startscan;
	int ScanSize;
	int VertPos;
	((int*)Dest)[1] = OffsetX;
	((int*)Dest)[2] = OffsetY;
	int VBpos = int(VertBuf);
	__asm {
		push	esi
		push	edi
		pushf
		mov		esi, Bitmap
		mov		edi, Dest
		add		edi, 12
		mov		ebx, VBpos
		cld
		StartLine :
		//Rendering the linear transformation
		mov		ecx, [ebx]
			mov[edi], ecx
			add		edi, 4     //edi points to Dest
			shr		ecx, 16    //Length of the scan line
			mov		ScanSize, ecx
			mov		Startscan, edi  //Storing for shadowing
			mov		edx, [ebx + 4]    //BMX
			mov		VertPos, ebx
			mov		ebx, [ebx + 8]    //BMY
			//and     ebx,0011 1111 1111 1110 0011 1111 1111 1110b
			jcxz	StartShad
			StartLinear :
		mov		eax, edx     //1
			ror		ebx, 8       //0
			shr		eax, 16      //1
			mov		ah, bh       //1
			and		ax, 0111111101111111b
			mov		al, [esi + eax]//?
			rol		ebx, 8       //0
			add		edx, BMDXX   //0
			add		ebx, BMDYX   //1
			dec		cx          //0
			stosb               //0  could be optimized to stosd
			jnz		StartLinear //1
	//Shadow processing
			StartShad :
		xchg	edi, Startscan
			mov		esi, VertPos
			mov		ecx, ScanSize
			mov		ebx, [esi + 12]   //fog
			mov		edx, FogDx
			//mov		edi,Startscan
			jcxz	endfog
			StartFog4 :      //Not optimal now! 32 bit reading coulde performed
		mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jnz		StartFog4
			endfog : mov		edi, Startscan
			mov		ebx, esi
			mov		esi, Bitmap
			add		ebx, 16
			dec		NLines
			jnz		StartLine
			sub		edi, Dest
			mov		eax, edi
			popf
			pop		esi
			pop		edi
	};
	return 0;
};
int PrecRenderTriangle64Dithering(int NLines, byte* Dest, byte* Bitmap) {
	((int*)Dest)[0] = NLines;
	int Startscan;
	int ScanSize;
	int VertPos;
	((int*)Dest)[1] = OffsetX;
	((int*)Dest)[2] = OffsetY;
	int VBpos = int(VertBuf);
	__asm {
		push	esi
		push	edi
		pushf
		mov		esi, Bitmap
		mov		edi, Dest
		add		edi, 12
		mov		ebx, VBpos
		cld
		StartLine :
		//Rendering the linear transformation
		mov		ecx, [ebx]
			mov[edi], ecx
			add		edi, 4     //edi points to Dest
			shr		ecx, 16    //Length of the scan line
			mov		ScanSize, ecx
			mov		Startscan, edi  //Storing for shadowing
			mov		edx, [ebx + 4]    //BMX
			mov		VertPos, ebx
			mov		ebx, [ebx + 8]    //BMY
			//and     ebx,0011 1111 1111 1110 0011 1111 1111 1110b
			jcxz	StartShad
			StartLinear :
		mov		eax, edx     //1
			ror		ebx, 8       //0
			shr		eax, 16      //1
			mov		ah, bh       //1
			and		ax, 0011111100111111b
			mov		al, [esi + eax]//?
			rol		ebx, 8       //0
			add		edx, BMDXX   //0
			add		ebx, BMDYX   //1
			dec		cx          //0
			stosb               //0  could be optimized to stosd
			jnz		StartLinear //1

	//Shadow processing
			StartShad :
		xchg	edi, Startscan
			mov		esi, VertPos
			mov		ecx, ScanSize
			mov		ebx, [esi + 12]   //fog
			test	NLines, 1
			jz		uuu2
			add		ebx, 16384
			uuu2:
		mov		edx, FogDx
			//mov		edi,Startscan
			jcxz	endfog
			StartFog4 :      //Not optimal now! 32 bit reading coulde performed
		mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			add		ebx, 32768
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jz		endfog
			mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			sub		ebx, 32768
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jnz		StartFog4
			endfog : mov		edi, Startscan
			mov		ebx, esi
			mov		esi, Bitmap
			add		ebx, 16
			dec		NLines
			jnz		StartLine
			sub		edi, Dest
			mov		eax, edi
			popf
			pop		esi
			pop		edi
	};
	return 0;
};
int PrecPreRenderTri64(int x1, int y1,
	int x2, int y2,
	int x3, int y3,
	int bmx, int bmy,
	int bmdxx, int bmdyx, int bmdxy, int bmdyy,
	int fog, int fogdx, int fogdy,
	byte* bitm,
	byte* Dest) {
	int z1 = GetMin(y1, y2, y3);
	int z2 = GetMax(y1, y2, y3);
	curScrOfs = x1 + y1*Buf3DLx;
	CurFog = fog;
	curXT = x1;
	curYT = y1 - z1;
	FogDx = fogdx;
	FogDy = fogdy;
	BMX = bmx;
	BMY = bmy;
	BMDXX = bmdxx;
	BMDYX = bmdyx;
	BMDXY = bmdxy;
	BMDYY = bmdyy;
	FOG1 = fog;
	int VBpos = int(VertBuf);
	//Инициализация VertBuf
	__asm {
		push	edi
		pushf
		cld
		mov		edi, VBpos
		mov		ecx, z2
		sub		ecx, z1
		inc		ecx
		uux1 : mov		dword ptr[edi], 0xFFFFFFFF;
		add		edi, 16
			dec		ecx
			jnz		uux1
			popf
			pop		edi
	};
	addLine(x2, y2 - z1);
	addLine(x3, y3 - z1);
	addLine(x1, y1 - z1);
	PrecPrepareToRender(z2 - z1 + 1, x1, y1 - z1);
	return PrecRenderTriangle64Dithering(z2 - z1 + 1, Dest, bitm);//Dithering(z2-z1+1,Dest,bitm);
};
int PrecPreRenderTri128(int x1, int y1,
	int x2, int y2,
	int x3, int y3,
	int bmx, int bmy,
	int bmdxx, int bmdyx, int bmdxy, int bmdyy,
	int fog, int fogdx, int fogdy,
	byte* bitm,
	byte* Dest) {
	int z1 = GetMin(y1, y2, y3);
	int z2 = GetMax(y1, y2, y3);
	curScrOfs = x1 + y1*Buf3DLx;
	CurFog = fog;
	curXT = x1;
	curYT = y1 - z1;
	FogDx = fogdx;
	FogDy = fogdy;
	BMX = bmx;
	BMY = bmy;
	BMDXX = bmdxx;
	BMDYX = bmdyx;
	BMDXY = bmdxy;
	BMDYY = bmdyy;
	FOG1 = fog;
	int VBpos = int(VertBuf);
	//Инициализация VertBuf
	__asm {
		push	edi
		pushf
		cld
		mov		edi, VBpos
		mov		ecx, z2
		sub		ecx, z1
		inc		ecx
		uux1 : mov		dword ptr[edi], 0xFFFFFFFF;
		add		edi, 16
			dec		ecx
			jnz		uux1
			popf
			pop		edi
	};
	addLine(x2, y2 - z1);
	addLine(x3, y3 - z1);
	addLine(x1, y1 - z1);
	PrecPrepareToRender(z2 - z1 + 1, x1, y1 - z1);
	return PrecRenderTriangle128(z2 - z1 + 1, Dest, bitm);
};
int RenderBestTriangle64(int xs1, int ys1,
	int xs2, int ys2,
	int xs3, int ys3,
	int xb1, int yb1,
	int xb2, int yb2,
	int xb3, int yb3,
	int f1, int f2, int f3,
	byte * Dest, byte* Bitm) {
	int dxb2 = xb2 - xb1;
	int dxb3 = xb3 - xb1;
	int dyb2 = yb2 - yb1;
	int dyb3 = yb3 - yb1;
	int dxs2 = xs2 - xs1;
	int dxs3 = xs3 - xs1;
	int dys2 = ys2 - ys1;
	int dys3 = ys3 - ys1;

	int D = dxs2*dys3 - dys2*dxs3;
	if (!D) {
		Dest = NULL;
		return -1;
	};
	int Axx = dys3*dxb2 - dxb3*dys2;
	int Axy = dxs2*dxb3 - dxb2*dxs3;
	int Ayx = dys3*dyb2 - dyb3*dys2;
	int Ayy = dxs2*dyb3 - dyb2*dxs3;

	Axx = div(Axx << 16, D).quot;
	Ayy = div(Ayy << 16, D).quot;
	Axy = div(Axy << 16, D).quot;
	Ayx = div(Ayx << 16, D).quot;

	int bmxy = ((xb1 & 63) << 8) + ((yb1 & 63) << 24);
	int bmdx = word(Axx) + (word(Ayx) << 16);
	int bmdy = word(Axy) + (word(Ayy) << 16);
	//Fogging
	int DScr = dys2*dxs3 - dxs2*dys3;
	int FDx = 0;
	int FDy = 0;
	if (DScr) {
		FDx = div(((f3 - f1)*dys2 - dys3*(f2 - f1)) << 16, DScr).quot;
		FDy = div(((f2 - f1)*dxs3 - dxs2*(f3 - f1)) << 16, DScr).quot;
	};
	return PrecPreRenderTri64(xs1, ys1, xs2, ys2, xs3, ys3,
		((xb1 & 255) << 16) + 32768, ((yb1 & 255) << 16) + 32768, Axx, Ayx, Axy, Ayy,
		(f1 << 16) + 32768, FDx, FDy, Bitm, Dest);

};
int RenderBestTriangle128(int xs1, int ys1,
	int xs2, int ys2,
	int xs3, int ys3,
	int xb1, int yb1,
	int xb2, int yb2,
	int xb3, int yb3,
	int f1, int f2, int f3,
	byte * Dest, byte* Bitm) {
	int dxb2 = xb2 - xb1;
	int dxb3 = xb3 - xb1;
	int dyb2 = yb2 - yb1;
	int dyb3 = yb3 - yb1;
	int dxs2 = xs2 - xs1;
	int dxs3 = xs3 - xs1;
	int dys2 = ys2 - ys1;
	int dys3 = ys3 - ys1;

	int D = dxs2*dys3 - dys2*dxs3;
	if (!D) {
		Dest = NULL;
		return -1;
	};
	int Axx = dys3*dxb2 - dxb3*dys2;
	int Axy = dxs2*dxb3 - dxb2*dxs3;
	int Ayx = dys3*dyb2 - dyb3*dys2;
	int Ayy = dxs2*dyb3 - dyb2*dxs3;

	Axx = div(Axx << 16, D).quot;
	Ayy = div(Ayy << 16, D).quot;
	Axy = div(Axy << 16, D).quot;
	Ayx = div(Ayx << 16, D).quot;

	int bmxy = ((xb1 & 63) << 8) + ((yb1 & 63) << 24);
	int bmdx = word(Axx) + (word(Ayx) << 16);
	int bmdy = word(Axy) + (word(Ayy) << 16);
	//Fogging
	int DScr = dys2*dxs3 - dxs2*dys3;
	int FDx = 0;
	int FDy = 0;
	if (DScr) {
		FDx = div(((f3 - f1)*dys2 - dys3*(f2 - f1)) << 16, DScr).quot;
		FDy = div(((f2 - f1)*dxs3 - dxs2*(f3 - f1)) << 16, DScr).quot;
	};
	return PrecPreRenderTri128(xs1, ys1, xs2, ys2, xs3, ys3,
		((xb1 & 255) << 16) + 32768, ((yb1 & 255) << 16) + 32768, Axx, Ayx, Axy, Ayy,
		f1 << 16, FDx, FDy, Bitm, Dest);

};
int DirectRenderTriangle64Dithering(int NLines, int StartLine, int EndLine, int DestSizeX, byte* Dest, byte* Bitmap) {
	if (StartLine >= NLines || EndLine < 0)return 0;
	int RealStartLine = StartLine;
	if (RealStartLine < 0)RealStartLine = 0;
	int RealEndLine = EndLine;
	if (RealEndLine >= NLines)RealEndLine = NLines - 1;
	RealEndLine -= RealStartLine - 1;
	int Startscan;
	int ScanSize;
	int VertPos;
	//((int*)Dest)[1]=OffsetX;
	//((int*)Dest)[2]=OffsetY;
	int VBpos = int(VertBuf) + (RealStartLine << 4);
	int StartEDI = int(Dest) + RealStartLine*DestSizeX;
	__asm {
		push	esi
		push	edi
		pushf
		mov		esi, Bitmap
		add		edi, 12
		mov		ebx, VBpos
		cld
		StartLineCode :
		//Rendering the linear transformation
		mov		edi, StartEDI
			mov		ecx, [ebx]
			//mov		[edi],ecx
			xor eax, eax
			mov		ax, cx
			test	cx, 0x8000
			jz		ululuk
			or eax, 0xFFFF0000
			ululuk:
		add		edi, eax   //edi points to Dest
			shr		ecx, 16    //Length of the scan line
			mov		ScanSize, ecx
			mov		Startscan, edi  //Storing for shadowing
			mov		edx, [ebx + 4]    //BMX
			mov		VertPos, ebx
			mov		ebx, [ebx + 8]    //BMY
			//and     ebx,0011 1111 1111 1110 0011 1111 1111 1110b
			jcxz	StartShad
			StartLinear :
		mov		eax, edx     //1
			ror		ebx, 8       //0
			shr		eax, 16      //1
			mov		ah, bh       //1
			and		ax, 0011111100111111b
			mov		al, [esi + eax]//?
			rol		ebx, 8       //0
			add		edx, BMDXX   //0
			add		ebx, BMDYX   //1
			dec		cx          //0
			stosb               //0  could be optimized to stosd
			jnz		StartLinear //1

	//Shadow processing
			StartShad :
		mov		edi, Startscan
			mov		esi, VertPos
			mov		ecx, ScanSize
			mov		ebx, [esi + 12]   //fog
			test	NLines, 1
			jz		uuu2
			add		ebx, 16384
			uuu2:
		mov		edx, FogDx
			//mov		edi,Startscan
			jcxz	endfog
			StartFog4 :      //Not optimal now! 32 bit reading coulde performed
		mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			add		ebx, 32768
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jz		endfog
			mov		eax, ebx
			sar		eax, 8
			add		ebx, edx
			sub		ebx, 32768
			mov		al, [edi]
			mov		al, [darkfog + 16384 + eax]
			mov[edi], al
			inc		edi
			dec		cx
			jnz		StartFog4
			endfog : mov		edi, StartEDI
			add		edi, DestSizeX
			mov		StartEDI, edi
			mov		ebx, esi
			mov		esi, Bitmap
			add		ebx, 16
			dec		RealEndLine
			jnz		StartLineCode
			popf
			pop		esi
			pop		edi
	};
	return 0;
};
int DirectPreRenderTri64(int x1, int y1,
	int x2, int y2,
	int x3, int y3,
	int bmx, int bmy,
	int bmdxx, int bmdyx, int bmdxy, int bmdyy,
	int fog, int fogdx, int fogdy,
	byte* bitm,
	byte* Dest,
	int StartLine, int EndLine, int ScanSize) {
	int z1 = GetMin(y1, y2, y3);
	int z2 = GetMax(y1, y2, y3);
	int MyStart = StartLine + y1 - z1;
	int MyEnd = EndLine + y1 - z1;
	curScrOfs = x1 + y1*Buf3DLx;
	CurFog = fog;
	curXT = x1;
	curYT = y1 - z1;
	FogDx = fogdx;
	FogDy = fogdy;
	BMX = bmx;
	BMY = bmy;
	BMDXX = bmdxx;
	BMDYX = bmdyx;
	BMDXY = bmdxy;
	BMDYY = bmdyy;
	FOG1 = fog;
	int VBpos = int(VertBuf);
	//Инициализация VertBuf
	__asm {
		push	edi
		pushf
		cld
		mov		edi, VBpos
		mov		ecx, z2
		sub		ecx, z1
		inc		ecx
		uux1 : mov		dword ptr[edi], 0xFFFFFFFF;
		add		edi, 16
			dec		ecx
			jnz		uux1
			popf
			pop		edi
	};
	addLine(x2, y2 - z1);
	addLine(x3, y3 - z1);
	addLine(x1, y1 - z1);
	AbsolutePrecPrepareToRender(z2 - z1 + 1, x1, y1 - z1);
	DirectRenderTriangle64Dithering(z2 - z1 + 1, MyStart, MyEnd, ScanSize, Dest + (z1 - y1)*ScanSize, bitm);//Dithering(z2-z1+1,Dest,bitm);
	return 0;
};
void DirectRenderTriangle64(int xs1, int ys1,
	int xs2, int ys2,
	int xs3, int ys3,
	int xb1, int yb1,
	int xb2, int yb2,
	int xb3, int yb3,
	int f1, int f2, int f3,
	byte * Dest, byte* Bitm,
	int StartLine, int EndLine, int ScanSize) {
	StartLine -= ys1;
	EndLine -= ys1;
	Dest += xs1 + ys1*ScanSize;
	int dxb2 = xb2 - xb1;
	int dxb3 = xb3 - xb1;
	int dyb2 = yb2 - yb1;
	int dyb3 = yb3 - yb1;
	int dxs2 = xs2 - xs1;
	int dxs3 = xs3 - xs1;
	int dys2 = ys2 - ys1;
	int dys3 = ys3 - ys1;

	int D = dxs2*dys3 - dys2*dxs3;
	if (!D) {
		Dest = NULL;
		return;
	};
	int Axx = dys3*dxb2 - dxb3*dys2;
	int Axy = dxs2*dxb3 - dxb2*dxs3;
	int Ayx = dys3*dyb2 - dyb3*dys2;
	int Ayy = dxs2*dyb3 - dyb2*dxs3;

	Axx = div(Axx << 16, D).quot;
	Ayy = div(Ayy << 16, D).quot;
	Axy = div(Axy << 16, D).quot;
	Ayx = div(Ayx << 16, D).quot;

	int bmxy = ((xb1 & 63) << 8) + ((yb1 & 63) << 24);
	int bmdx = word(Axx) + (word(Ayx) << 16);
	int bmdy = word(Axy) + (word(Ayy) << 16);
	//Fogging
	int DScr = dys2*dxs3 - dxs2*dys3;
	int FDx = 0;
	int FDy = 0;
	if (DScr) {
		FDx = div(((f3 - f1)*dys2 - dys3*(f2 - f1)) << 16, DScr).quot;
		FDy = div(((f2 - f1)*dxs3 - dxs2*(f3 - f1)) << 16, DScr).quot;
	};
	DirectPreRenderTri64(xs1, ys1, xs2, ys2, xs3, ys3,
		((xb1 & 255) << 16) + 32768, ((yb1 & 255) << 16) + 32768, Axx, Ayx, Axy, Ayy,
		(f1 << 16) + 32768, FDx, FDy, Bitm, Dest, StartLine, EndLine, ScanSize);

};
//---------------------NEW 3D MAP RENDERING!--------------------

byte *tex1;
byte TexColors[256];

//----------------------NEW MAP WITH CASHING--------------------
//
//
//    /|\ /|\ /|\ /|\
//   |/|\|/|\|/|\|/|\|
//   |/|\|/|\|/|\|/|\|
//   
//
//
//
//
/*
#define MaxSector 128      //Amount of sectors  /|\
						  //                  |/ \| in line
#define MaxTH (MaxSector*2)  //128 - Nunber of strips /|\ /|\ /|\...
							 //                      |/ \|/ \|/ \...
#define VertInLine (MaxSector+MaxSector+1)  //   Number of vertices in line    .     .     .
											//                              .     .     .     .
#define SectInLine MaxSector*6
#define TIBufSize 8192000
#define MaxTIRef 8192
*/

short* THMap;// Map of heights in vertices
byte* TexMap;//Map of textures in vertices
byte* SectMap;//Map of sections on lines

//int CurTIRef;
//int CurTIBufPos;
//int MaxExistingTIRef;
RLCTable SimpleMask;
RLCTable SimpleMaskA;
RLCTable SimpleMaskB;
RLCTable SimpleMaskC;
RLCTable SimpleMaskD;
extern byte ExtTex[256][4];
extern short randoma[8192];
int GetVTex(int i) {
	//if(RoadOpt[i]==1)return RoadTex[TexMap[i]];
	//else 
	return TexMap[i];
};
void Init3DMapSystem() {
	memset(THMap, 0, (MaxTH + 1)*MaxTH * 2);

	CreateRandomHMap();
	//SVSC.CreateTrianglesMapping();
	//CurTIRef=0;
	//MaxExistingTIRef=0;
	//CurTIBufPos=0;
	LoadRLC("ms.msk", &SimpleMask);
	LoadRLC("ms_a.msk", &SimpleMaskA);
	LoadRLC("ms_b.msk", &SimpleMaskB);
	LoadRLC("ms_c.msk", &SimpleMaskC);
	LoadRLC("ms_d.msk", &SimpleMaskD);
	Mode3D = true;
	if (SectMap)for (int i = 0; i < MaxSector*MaxTH * 6; i++)SetSectMap(i, div(rand(), 11000).quot);
};
void ClearRender() {
	SVSC.RefreshScreen();
};
void Reset3D() {
	Mode3D = !Mode3D;
	ClearRender();
};
//#define TriUnit		32 //24
//#define TriUnit34	24 //18
int prp34(int i) {
	//return mul3(i)>>2;
	return i >> 1;
};
//---------------------------------------------
int GetHi(int i) {
	if (i<0 || i>MaxTH*(MaxTH + 1))return 0;
	return THMap[i];
};
int GetTriX(int i) {
	if (i<0 || i>MaxTH*MaxTH * 2)return -1000000;
	return div(i, VertInLine).rem*(TriUnit + TriUnit);
};
int GetTriY(int i) {
	div_t uu = div(i, VertInLine);
	if (uu.rem & 1) {
		return uu.quot*(TriUnit + TriUnit) - TriUnit;
	}
	else {
		return uu.quot*(TriUnit + TriUnit);
	};
};
void SetHi(int i, int h) {
	if (i<0 || i>MaxTH*(MaxTH + 1))return;
	THMap[i] = h;
};
void CreateTriBlob(int x, int y, int h, int r) {
	int sx = x << 5;
	int sy = y << 5;
	int dvd = (TriUnit * 2) >> 5;
	int utx = div(x << 5, TriUnit * 2).quot;
	int uty = div(y << 5, TriUnit * 2).quot;
	double r2 = r*r * 512;
	for (int tx = -r; tx < r; tx++)
		for (int ty = -r; ty < r; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				vx = GetTriX(vert);
				vy = GetTriY(vert);
				int dh = 0;
				double sss = -double((vx - sx)*(vx - sx) + (vy - sy)*(vy - sy)) / r2;
				if (abs(int(sss)) < 10) dh = int(double(h)*exp(sss));
				SetHi(vert, GetHi(vert) + dh);
			};
		};
};
void CreateEffect(int x, int y, int r, HiCallback* HCB) {
	int dvd = (TriUnit * 2) >> 5;
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit + TriUnit).quot + 3;
	double r2 = r*r * 512;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				vx = GetTriX(vert);
				vy = GetTriY(vert);
				SetHi(vert, HCB(vx, vy, GetHi(vert)));
			};
		};
};
void MarkPointToDraw(int i);
void CreateAveragePlane(int x, int y, int r) {
	int h = 0;
	int np = 0;
	int dvd = (TriUnit * 2) >> 5;
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = (r >> 5) + 3;
	//double r2=r*r*512;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				vx = GetTriX(vert);
				vy = GetTriY(vert);
				int  rr = int(sqrt((vx - x)*(vx - x) + (vy - y)*(vy - y)));
				if (rr < r) {
					h += GetHi(vert);
					np++;
				};
			};
		};
	h = div(h, np).quot;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				vx = GetTriX(vert);
				vy = GetTriY(vert);
				int  rr = int(sqrt((vx - x)*(vx - x) + (vy - y)*(vy - y)));
				if (rr < r) {
					SetHi(vert, h);
					MarkPointToDraw(vert);
				}
				else if (rr < r + 128) {
					int hh1 = GetHi(vert);
					hh1 = h + (((hh1 - h)*(rr - r)) >> 7);
					SetHi(vert, hh1);
					MarkPointToDraw(vert);
				};
			};
		};
};
void CreateRandomHMap() {
	//CreateTriBlob(40,40,100,10);
	/*
	for(int i=0;i<400;i++){
	CreateTriBlob(rand()&255,rand()&255,100,10);
	//eateTriBlob(rand()&255,rand()&255,10,5);
		CreateTriBlob(rand()&255,rand()&255,16+(rand()&31),1+(rand()&1));
	};
	*/
	memset(TexMap, 0, sizeof TexMap);
	/*for(i=0;i<7550;i++){
		int p=rand();
		if(p<sizeof TexMap)TexMap[p]=14;
	};
	for(i=0;i<7550;i++){
		int p=rand();
		if(p<sizeof TexMap)TexMap[p]=5;
	};
	*/
	//for(i=0;i<sizeof SectMap;i++)SectMap[i]=div(rand(),11000).quot;
};
//const int HardLight[32]={0,1,2,3,4,5,6,8,10,12,14,16,18,20,22,24,26,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30};
const int HardLight[32] = { 0,1,1,3,5,7,9,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13 };
int GetLighting(int i) {
	byte tex = GetVTex(i);
	word tf = TexFlags[tex];
	if (tf&TEX_NOLIGHT)return 0;
	int h1, h2, h3, h4, h5, h6;
	if (i & 1) {
		h1 = GetHi(i + VertInLine);
		h2 = GetHi(i + VertInLine + 1);
		h3 = GetHi(i + 1);
		h4 = GetHi(i - VertInLine);
		h5 = GetHi(i - 1);
		h6 = GetHi(i + VertInLine - 1);
	}
	else {
		h1 = GetHi(i + VertInLine);
		h2 = GetHi(i + 1);
		h3 = GetHi(i - VertInLine + 1);
		h4 = GetHi(i - VertInLine);
		h5 = GetHi(i - VertInLine - 1);
		h6 = GetHi(i - 1);
	};
	int dy = h4 - h1;
	int dx = (h2 - h5 + h3 - h6) >> 1;
	int dz = TriUnit + TriUnit + TriUnit + TriUnit;
	int lig = 16 + (div(dx*LightDX + dy*LightDY + dz*LightDZ, int(sqrt(dx*dx + dy*dy + dz*dz))).quot >> 4);
	//if(tf&TEX_HARDLIGHT){
	if (lig < 1)lig = 1;
	if (lig > 31)lig = 31;
	lig = 32 - HardLight[32 - lig];
	//};
	//lig-=AddTHMap(i)>>2;
	//if(lig>30)lig=30;
	if (lig < 1)lig = 1;
	if (lig > 31)lig = 31;
	return 32 - lig;
};
//---------------------------------------------

void ShowVisualLess(int yend);
void ProcessWaveFrames();
void ShowRelief() {

};
//-----------------------------TESTING--------------------------
word TexFlags[256];
byte TexMedia[256];
word RoadTex[256];
byte ExtTex[256][4];
char* TexNames[256];
void NLine(GFILE*);
void ClearIntersectionBuffer();
byte TileMap[256];
extern int TEXARR[8];

void Loadtextures()
{
	memset(TileMap, 0, sizeof TileMap);
	ResFile f1 = RReset("tiles3.bmp");//("dmbmp.bpx");//("textures.bpx");
	RSeek(f1, 0x436);
	tex1 = new byte[RFileSize(f1) - 0x436];
	int ntex = RFileSize(f1) >> 12;
	MaxTex = ntex;
	RBlockRead(f1, tex1, RFileSize(f1) - 0x436);
	for (int j = 0; j < ntex; j++) {
		TexColors[j] = (tex1 + GetBmOfst(j))[0];
	};
	RClose(f1);
	GFILE* FF = Gopen("Tiling.txt", "r");
	if (FF) {
		char ccc[128];
		for (int i = 0; i < 16; i++) {
			Gscanf(FF, "%s", ccc);
			for (int j = 0; j < 16; j++) {
				TileMap[i * 16 + j] = ccc[j] - '0';
			};
		};
		Gclose(FF);
		FF = Gopen("fract_set.txt", "r");
		if (FF) {
			for (int i = 0; i < 8; i++) {
				int n;
				Gscanf(FF, "%d", &n);
				TEXARR[i] = n;
			};
			Gclose(FF);
		};
	};
	memset(TexFlags, 0, 256);
	memset(TexMedia, 0, 256);
	memset(TexNames, 0, 1024);
	for (int i = 0; i < 256; i++) {
		RoadTex[i] = i;
	};
	GFILE* f = Gopen("textures.lst", "r");

	char gx[128];
	char gy[128];
	char gz[128];
	int nte;
	int z;
	for (int j = 0; j < 256; j++) {
		ExtTex[j][0] = j;
		ExtTex[j][1] = j;
		ExtTex[j][2] = j;
		ExtTex[j][3] = j;
	};
	if (f) {
		do {
			z = Gscanf(f, "%s", gx);
			if (!strcmp(gx, "#MULTI")) {
				if (z > 0) {
					int t1, t2, t3, t4;
					z = Gscanf(f, "%d%d%d%d", &t1, &t2, &t3, &t4);
					if (z == 4 && t1 < 256 && t2 < 256 && t3 < 256 && t4 < 256) {
						ExtTex[t1][0] = t1;
						ExtTex[t1][1] = t2;
						ExtTex[t1][2] = t3;
						ExtTex[t1][3] = t4;
					}
					else ErrM("Textures.lst:invalid #MULTI directive");
				};
				NLine(f);
			}
			else
				if (!strcmp(gx, "#ROAD")) {
					if (z > 0) {
						int t1, t2;
						z = Gscanf(f, "%d%d", &t1, &t2);
						if (z == 2 && t1 < 256 && t2 < 256) {
							RoadTex[t1] = t2;
						}
						else ErrM("Textures.lst:invalid #ROAD directive");
					};
					NLine(f);
				}
				else
					if (z > 0 && gx[0] != '/'&&gx[0] != 0) {
						z = Gscanf(f, "%d%s", &nte, gy);
						TexNames[nte] = new char[strlen(gx) + 1];
						strcpy(TexNames[nte], gx);
						if (strchr(gy, 'W'))TexFlags[nte] |= TEX_ALWAYS_WATER_UNLOCK;
						if (strchr(gy, 'L'))TexFlags[nte] |= TEX_ALWAYS_LAND_LOCK;
						if (strchr(gy, 'U'))TexFlags[nte] |= TEX_ALWAYS_LAND_UNLOCK;
						if (strchr(gy, 'P'))TexFlags[nte] |= TEX_PLAIN;
						if (strchr(gy, 'N'))TexFlags[nte] |= TEX_NORMALPUT;
						if (strchr(gy, 'H'))TexFlags[nte] |= TEX_HARD;
						if (strchr(gy, 'R'))TexFlags[nte] |= TEX_HARDLIGHT;
						if (strchr(gy, 'B'))TexFlags[nte] |= TEX_NOLIGHT;
						if (strchr(gy, '#')) {
							//Media description is present
							z = Gscanf(f, "%s", gy);
							if (z != 1) {
								sprintf(gz, "textures.lst: %s : Media description must present.", gx);
								ErrM(gz);
							};
							int temed = GetExMedia(gy);
							if (temed == -1) {
								sprintf(gx, "textures.lst : Unknown media type : %s", gy);
								ErrM(gx);
							};
							TexMedia[nte] = temed;
						};
					};
			NLine(f);
		} while (z > 0);
	}
	else {
		ErrM("Could not load textures properties: Textures.lst");
	}
	Gclose(f);
};

extern int COUNTER;

void CBar(int x, int y, int Lx, int Ly, byte c);

void TestGP();

void TestTriangle() 
{
	SVSC.Execute();
	TestGP();
}

extern int VertLx;

struct VertOver 
{
	short xs;
	short ys;
	short xz;
	int   v;
	bool Visible;
	byte* Data;
};

class OverTriangle 
{
public:
	VertOver** TRIANG;
	word*      NTRIANG;
	byte** Buffer;
	int MaxElm;
	int CurElm;

	OverTriangle();
	~OverTriangle();

	void Clear();
	void AddTriangle(int i);
	void CreateFullMap();
	void Show();
	void ShowElement(int Sq, int idx);
};

OverTriangle::OverTriangle()
{
	Buffer = nullptr;
	MaxElm = 0;
	CurElm = 0;
}

void OverTriangle::Clear() 
{
	for (int i = 0; i < VertLx*VertLx; i++) 
	{
		if (TRIANG[i])
		{
			free(TRIANG[i]);
		}

		TRIANG[i] = nullptr;
		NTRIANG[i] = 0;
	}

	for (int i = 0; i < CurElm; i++)
	{
		free(Buffer[i]);
	}

	if (MaxElm)
	{
		free(Buffer);
	}

	Buffer = nullptr;
	MaxElm = 0;
	CurElm = 0;
}

//Calls Clear()
OverTriangle::~OverTriangle() 
{
	Clear();
}

void OverTriangle::AddTriangle(int i) 
{
	int TriType;
	int x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0, x3 = 0, y3 = 0, z3 = 0, yy1 = 0, yy2 = 0, yy3 = 0;
	int TriStartY = div((i >> 2), MaxSector).quot;
	int StartVertex = TriStartY + ((i >> 2) << 1);
	int StartSide = (i >> 2) * 6;
	TriType = i & 3;
	int TriStartX = ((i >> 2) - TriStartY*MaxSector)*(4 * TriUnit);
	TriStartY = TriStartY*(TriUnit + TriUnit);
	bool Visible = true;
	switch (TriType) {
	case 0:
		x1 = x2 = TriStartX;
		y1 = TriStartY;
		yy1 = prp34(y1);
		y2 = TriStartY + TriUnit + TriUnit;
		yy2 = prp34(y2);
		x3 = TriStartX + TriUnit + TriUnit;
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex] - AddTHMap(StartVertex);
			z2 = yy2 - THMap[StartVertex + VertInLine] - AddTHMap(StartVertex + VertInLine);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z1 >= z2)Visible = false;
		break;
	case 1:
		x1 = TriStartX;
		y1 = TriStartY;
		yy1 = prp34(y1);
		x2 = x3 = TriStartX + TriUnit + TriUnit;
		y2 = TriStartY - TriUnit;
		yy2 = prp34(y2);
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex] - AddTHMap(StartVertex);
			z2 = yy2 - THMap[StartVertex + 1] - AddTHMap(StartVertex + 1);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z2 >= z3)Visible = false;
		break;
	case 2:
		x1 = TriStartX + TriUnit + TriUnit + TriUnit + TriUnit;
		y1 = TriStartY;
		yy1 = prp34(y1);
		x2 = x3 = TriStartX + TriUnit + TriUnit;
		y2 = TriStartY - TriUnit;
		yy2 = prp34(y2);
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex + 2] - AddTHMap(StartVertex + 2);
			z2 = yy2 - THMap[StartVertex + 1] - AddTHMap(StartVertex + 1);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z2 >= z3)Visible = false;
		break;
	case 3:
		x1 = x2 = TriStartX + TriUnit + TriUnit + TriUnit + TriUnit;
		y1 = TriStartY;
		yy1 = prp34(y1);
		y2 = TriStartY + TriUnit + TriUnit;
		yy2 = prp34(y2);
		x3 = TriStartX + TriUnit + TriUnit;
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex + 2] - AddTHMap(StartVertex + 2);
			z2 = yy2 - THMap[StartVertex + VertInLine + 2] - AddTHMap(StartVertex + VertInLine + 2);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z1 >= z2)Visible = false;
	};
	if (Visible) {
		int zmin = z1;
		int zmax = z1;
		int ymin = y1;
		if (ymin > y2)ymin = y2;
		if (ymin > y3)ymin = y3;

		if (zmin > z2)zmin = z2;
		if (zmin > z3)zmin = z3;
		if (zmax < z2)zmax = z2;
		if (zmax < z3)zmax = z3;

		int x = (x1 + x2 + x3) / 3;
		int px = x >> 9;
		int py = (zmin + zmax) >> 9;
		if (px < 0)px = 0;
		if (py < 0)py = 0;
		if (px >= VertLx)px = VertLx - 1;
		if (py >= VertLx)py = VertLx - 1;
		int v = px + py*VertLx;
		TRIANG[v] = (VertOver*)realloc(TRIANG[v], int(NTRIANG[v] + 1) * sizeof VertOver);
		VertOver* VO = TRIANG[v] + NTRIANG[v];
		NTRIANG[v]++;
		VO->Data = NULL;
		VO->xs = x;
		VO->ys = (zmin + zmax) >> 1;
		VO->xz = ymin;
		VO->v = i;
		VO->Visible = true;
	};
};
int TotalTriangles = 0;
int TotalTriSize = 0;
void OverTriangle::CreateFullMap() {
	int LX = msx >> 1;
	TotalTriangles = 0;
	TotalTriSize = 0;
	int ymax0 = -100000;
	int ymax1 = -100000;
	int ymax2 = -100000;
	int ymax3 = -100000;
	int Vert0 = 0;
	int StTri = 0;
	int yy = 0;
	for (int ix = 0; ix < LX; ix++) {
		Vert0 = 0;
		StTri = 0;
		yy = 0;
		ymax0 = -100000;
		ymax1 = -100000;
		ymax2 = -100000;
		int Count0 = 0;
		int Count1 = 0;
		int Count2 = 0;
		int Count3 = 0;
		for (int iy = 0; iy < msy; iy++) {
			int StartVertex = (ix << 1) + Vert0;
			if (!iy) {
				THMap[StartVertex] = THMap[StartVertex + VertInLine];
				THMap[StartVertex + 1] = THMap[StartVertex + VertInLine + 1];
				THMap[StartVertex + 2] = THMap[StartVertex + VertInLine + 2];
			};
			int StartTri = (ix << 2) + StTri;
			int Y00 = yy - THMap[StartVertex];//-AddTHMap[StartVertex];
			int Y01 = yy + 16 - THMap[StartVertex + VertInLine];//-AddTHMap[StartVertex+VertInLine];
			int Y10 = yy - 8 - THMap[StartVertex + 1];//-AddTHMap[StartVertex+1];
			int Y11 = yy + 8 - THMap[StartVertex + VertInLine + 1];//-AddTHMap[StartVertex+VertInLine+1];
			int Y20 = yy - THMap[StartVertex + 2];//-AddTHMap[StartVertex+2];
			int Y21 = yy + 16 - THMap[StartVertex + VertInLine + 2];//-AddTHMap[StartVertex+VertInLine+2];
			//Locking--------//
			//if(Y00>=Y01+1||Y10>=Y11+1||Y20>=Y21+1){
			//	if(!GetL3Height((ix<<6)+32,(iy<<5)+16))BSetBar(ix<<2,(iy>>1)<<2,4);
			//};
			//--------------//
			if (Count0)Count0--;
			if (Count1)Count1--;
			if (Count2)Count2--;
			if (Count3)Count3--;
			bool CN1 = Y00 < ymax0 || Y11 < ymax1;
			bool CN2 = Y10 < ymax1 || Y00 < ymax0;
			bool CN3 = Y10 < ymax1 || Y20 < ymax2;
			bool CN4 = Y20 < ymax2 || Y11 < ymax1;
			if (Y01 > Y00 && (Count0 || CN1)) {
				AddTriangle(StartTri);
				TotalTriangles++;
				if (CN1)Count0 = 2;
			};
			if (Y11 > Y10) {
				if (Count1 || CN2) {
					AddTriangle(StartTri + 1);
					TotalTriangles++;
					if (CN2)Count1 = 2;
				};
				if (Count2 || CN3) {
					AddTriangle(StartTri + 2);
					TotalTriangles++;
					if (CN3)Count2 = 2;
				};
			};
			if (Y21 > Y20 && (Count3 || CN4)) {
				AddTriangle(StartTri + 3);
				TotalTriangles++;
				if (CN4)Count3 = 2;
			};
			if (Y00 > ymax0)ymax0 = Y00;
			if (Y01 > ymax0)ymax0 = Y01;

			if (Y10 > ymax1)ymax1 = Y10;
			if (Y11 > ymax1)ymax1 = Y11;

			if (Y20 > ymax2)ymax2 = Y20;
			if (Y21 > ymax2)ymax2 = Y21;

			Vert0 += VertInLine;
			StTri += MaxTH * 2;
			yy += 16;
		};
	};
}

int RenderSector(int i, bool Mode3D, byte* DST, bool NeedRender)
{
	if (i >= MaxTH*MaxTH * 2)
	{
		return 0;
	}

	//determination of the parameters of the triangle
	//1.Coordinates&type 
	int x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0, x3 = 0, y3 = 0, z3 = 0, yy1 = 0, yy2 = 0, yy3 = 0;
	int TriStartY = div((i / 4), MaxSector).quot;
	int StartVertex = TriStartY + ((i / 4) * 2);
	int StartSide = (i / 4) * 6;
	int TriType = i % 4;
	int Tex1 = 0, Tex2 = 0, Tex3 = 0;
	int TriStartX = ((i / 4) - TriStartY*MaxSector)*(4 * TriUnit);
	TriStartY = TriStartY*(TriUnit + TriUnit);
	bool Visible = true;

	switch (TriType)
	{
	case 0:
		x1 = x2 = TriStartX;
		y1 = TriStartY;
		yy1 = prp34(y1);
		y2 = TriStartY + TriUnit + TriUnit;
		yy2 = prp34(y2);
		x3 = TriStartX + TriUnit + TriUnit;
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex] - AddTHMap(StartVertex);
			z2 = yy2 - THMap[StartVertex + VertInLine] - AddTHMap(StartVertex + VertInLine);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z1 >= z2)Visible = false;
		break;
	case 1:
		x1 = TriStartX;
		y1 = TriStartY;
		yy1 = prp34(y1);
		x2 = x3 = TriStartX + TriUnit + TriUnit;
		y2 = TriStartY - TriUnit;
		yy2 = prp34(y2);
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex] - AddTHMap(StartVertex);
			z2 = yy2 - THMap[StartVertex + 1] - AddTHMap(StartVertex + 1);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z2 >= z3)Visible = false;
		break;
	case 2:
		x1 = TriStartX + TriUnit + TriUnit + TriUnit + TriUnit;
		y1 = TriStartY;
		yy1 = prp34(y1);
		x2 = x3 = TriStartX + TriUnit + TriUnit;
		y2 = TriStartY - TriUnit;
		yy2 = prp34(y2);
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex + 2] - AddTHMap(StartVertex + 2);
			z2 = yy2 - THMap[StartVertex + 1] - AddTHMap(StartVertex + 1);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z2 >= z3)Visible = false;
		break;
	case 3:
		x1 = x2 = TriStartX + TriUnit + TriUnit + TriUnit + TriUnit;
		y1 = TriStartY;
		yy1 = prp34(y1);
		y2 = TriStartY + TriUnit + TriUnit;
		yy2 = prp34(y2);
		x3 = TriStartX + TriUnit + TriUnit;
		y3 = TriStartY + TriUnit;
		yy3 = prp34(y3);
		if (Mode3D) {
			z1 = yy1 - THMap[StartVertex + 2] - AddTHMap(StartVertex + 2);
			z2 = yy2 - THMap[StartVertex + VertInLine + 2] - AddTHMap(StartVertex + VertInLine + 2);
			z3 = yy3 - THMap[StartVertex + VertInLine + 1] - AddTHMap(StartVertex + VertInLine + 1);
		}
		else {
			z1 = yy1;
			z2 = yy2;
			z3 = yy3;
		};
		if (z1 >= z2)Visible = false;
	};
	int xt1 = -(x1 >> 1) - y1;
	int yt1 = (((x1 + x1 + x1) >> 1) - y1) >> 1;
	int xt2 = -(x2 >> 1) - y2;
	int yt2 = (((x2 + x2 + x2) >> 1) - y2) >> 1;
	int xt3 = -(x3 >> 1) - y3;
	int yt3 = (((x3 + x3 + x3) >> 1) - y3) >> 1;

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
	int xs0 = mapx << 5;
	int ys0 = mul3(mapy) << 3;
	int xs1 = xs0 + (smaplx << 5);
	int ys1 = ys0 + (mul3(smaply) << 3);
	if ((!Visible) ||
		(x1 < xs0&&x2 < xs0&&x3 < xs0) ||
		(x1 > xs1&&x2 > xs1&&x3 > xs1) ||
		(z1 < ys0&&z2 < ys0&&z3 < ys0) ||
		(z1 > ys1&&z2 > ys1&&z3 > ys1))return 0;
	int RSIZE = 0;
	if (NeedRender) {
		//Now we are ready to render
		int f1 = 0, f2 = 0, f3 = 0;
		switch (TriType) {
		case 0:
			f1 = GetLighting(StartVertex);
			f2 = GetLighting(StartVertex + VertInLine);
			f3 = GetLighting(StartVertex + VertInLine + 1);
			Tex1 = GetVTex(StartVertex);
			Tex2 = GetVTex(StartVertex + VertInLine);
			Tex3 = GetVTex(StartVertex + VertInLine + 1);
			break;
		case 1:
			f1 = GetLighting(StartVertex);
			f2 = GetLighting(StartVertex + 1);
			f3 = GetLighting(StartVertex + VertInLine + 1);
			Tex1 = GetVTex(StartVertex);
			Tex2 = GetVTex(StartVertex + 1);
			Tex3 = GetVTex(StartVertex + VertInLine + 1);
			break;
		case 2:
			f1 = GetLighting(StartVertex + 2);
			f2 = GetLighting(StartVertex + 1);
			f3 = GetLighting(StartVertex + VertInLine + 1);
			Tex1 = GetVTex(StartVertex + 2);
			Tex2 = GetVTex(StartVertex + 1);
			Tex3 = GetVTex(StartVertex + VertInLine + 1);
			break;
		case 3:
			f1 = GetLighting(StartVertex + 2);
			f2 = GetLighting(StartVertex + VertInLine + 2);
			f3 = GetLighting(StartVertex + VertInLine + 1);
			Tex1 = GetVTex(StartVertex + 2);
			Tex2 = GetVTex(StartVertex + VertInLine + 2);
			Tex3 = GetVTex(StartVertex + VertInLine + 1);
		}

		if ((Tex1 == Tex2 && Tex1 == Tex3) || (TriType == 7))
		{
			RSIZE = RenderBestTriangle64(1024 + x1, 1024 + z1, 1024 + x2, 1024 + z2, 1024 + x3, 1024 + z3,
				xt1, yt1, xt2, yt2, xt3, yt3,
				f1, f2, f3, DST,
				tex1 + GetBmOfst(Tex1));
		}
		else
		{
			switch (TriType) {
			case 0:
				PrepareIntersection1(Tex2, Tex1, Tex3, xt2, yt2,
					SECTMAP(StartSide),
					SECTMAP(StartSide + 2),
					SECTMAP(StartSide + SectInLine + 1),
					SimpleMask, tex1);
				RSIZE = RenderBestTriangle64(1024 + x1, 1024 + z1, 1024 + x3, 1024 + z3, 1024 + x2, 1024 + z2,
					30, 15, 0, 31, 0, 0,
					f1, f3, f2, DST,
					ResultMask);
				break;
			case 1:
				PrepareIntersection2(Tex1, Tex3, Tex2, xt3, yt3,
					SECTMAP(StartSide + 2),
					SECTMAP(StartSide + 3),
					SECTMAP(StartSide + 1),
					SimpleMask, tex1);
				RSIZE = RenderBestTriangle64(1024 + x1, 1024 + z1, 1024 + x3, 1024 + z3, 1024 + x2, 1024 + z2,
					30, 1, 1, 14, 30, 30,
					f1, f3, f2, DST,
					ResultMask);
				break;
			case 2:
				PrepareIntersection1(Tex3, Tex2, Tex1, xt3, yt3,
					SECTMAP(StartSide + 3),
					SECTMAP(StartSide + 4),
					SECTMAP(StartSide + 5),
					SimpleMask, tex1);
				RSIZE = RenderBestTriangle64(1024 + x1, 1024 + z1, 1024 + x3, 1024 + z3, 1024 + x2, 1024 + z2,
					1, 30, 1, 1, 30, 14,
					f1, f3, f2, DST,
					ResultMask);
				break;
			case 3:
				PrepareIntersection2(Tex3, Tex2, Tex1, xt2, yt2,
					SECTMAP(StartSide + SectInLine + 4),
					SECTMAP(StartSide + 6),
					SECTMAP(StartSide + 5),
					SimpleMask, tex1);
				RSIZE = RenderBestTriangle64(1024 + x1, 1024 + z1, 1024 + x3, 1024 + z3, 1024 + x2, 1024 + z2,
					30, 30, 30, 1, 1, 14,
					f1, f3, f2, DST,
					ResultMask);
				break;
			};
		};
	};
	ShowClippedTriangle(smapx + x1 - xs0, smapy + z1 - ys0, DST);
	return RSIZE;
};

void OverTriangle::ShowElement(int Sq, int idx) {
	//	assert(Sq>=0);
	VertOver* VO = TRIANG[Sq];
	if (VO&&idx < NTRIANG[Sq]) {
		VO += idx;
		if (!VO->Data) {
			byte TMP[8192];
			int SZ = RenderSector(VO->v, Mode3D, TMP, true);
			//			assert(SZ<8192);
			if (SZ) {
				VO->Data = new byte[SZ];
				memcpy(VO->Data, TMP, SZ);
				TotalTriSize += SZ;
			};
		}
		else RenderSector(VO->v, Mode3D, VO->Data, false);
	};
};
void OverTriangle::Show() {
	int sx0 = (mapx >> 4) - 1;
	int sy0 = (mapy >> 4) - 1;
	int sx1 = ((mapx + smaplx) >> 4) + 2;
	int sy1 = ((mapy + smaply) >> 4) + 2;
	if (sx0 < 0)sx0 = 0;
	if (sy0 < 0)sy0 = 0;
	int LX = msx >> 4;
	if (sx1 >= LX)sx1 = LX - 1;
	if (sy1 >= LX)sy1 = LX - 1;
	int x0 = mapx << 5;
	int y0 = mapy << 4;
	int xs0 = (mapx << 5) - 32;
	int xs1 = ((mapx + smaplx) << 5) + 32;
	int ys0 = (mapy << 4) - 32;
	int ys1 = ((mapy + smaply) << 4) + 32;
	for (int ix = sx0; ix <= sx1; ix++) {
		for (int iy = sy0; iy <= sy1; iy++) {
			int sq = ix + iy*VertLx;
			VertOver* VO = TRIANG[sq];
			int N = NTRIANG[sq];
			if (VO) {
				for (int p = 0; p < N; p++) {
					if (VO->Visible) {
						int dy = 0;//(rand()&3)-1;
						int xs = VO->xs;
						int ys = VO->ys;
						if (xs >= xs0&&xs <= xs1&&ys >= ys0&&ys <= ys1) {
							AddPoint(VO->xs - x0, (VO->xz >> 1) - y0 + dy - 16, 0, 0, NULL, sq, p, 0xFFFF, 0xFFFF);
						};
					};
					VO++;
				};

			};
		};
	};
};
//--------------------(}:)        ;
OverTriangle OTRI;
void ShowTrianglesOwerUnits() {
	OTRI.Show();
};
void DrawTriangleElement(int sq, int idx) {
	OTRI.ShowElement(sq, idx);
};
void ClearTrianglesSystem() {
	OTRI.Clear();
};
void CreateTrianglesSystem() {
	OTRI.CreateFullMap();
};
bool CheckPointInside(int x, int y, short* xi, short* yi, int Np) {
	int NCross = 0;
	for (int i = 1; i < Np; i++) {
		int x0 = xi[i - 1];
		int x1 = xi[i];
		int y0 = yi[i - 1];
		int y1 = yi[i];
		if (x1 < x0) {
			int t = x1;
			x1 = x0;
			x0 = t;
			t = y1;
			y1 = y0;
			y0 = t;
		};
		if (x0 <= x&&x1 > x && (y0 > y || y1 > y)) {
			if (x0 != x1) {
				int yy = y0 + ((y1 - y0)*(x - x0)) / (x1 - x0);
				if (yy > y)NCross++;
			};
		};
	};
	return NCross & 1;
};
void SetTrianglesState(int xc, int yc, short* xi, short* yi, int NP, bool State) {
	int x0 = (xc >> 9) - 1;
	int y0 = (yc >> 8) - 1;
	int x1 = x0 + 2;
	int y1 = y0 + 2;
	int mx = msx >> 4;
	if (x0 < 0)x0 = 0;
	if (y0 < 0)y0 = 0;
	if (x1 >= mx)x1 = mx - 1;
	if (y1 >= mx)y1 = mx - 1;
	for (int ix = x0; ix <= x1; ix++) {
		for (int iy = 0; iy < y1; iy++) {
			int ofs = ix + iy*VertLx;
			VertOver* VO = OTRI.TRIANG[ofs];
			int N = OTRI.NTRIANG[ofs];
			for (int i = 0; i < N; i++) {
				int xs = VO[i].xs - xc;
				int ys = VO[i].ys - yc;
				if (CheckPointInside(xs, ys, xi, yi, NP)) {
					VO[i].Visible = State;
				};
			};
		};
	};
};