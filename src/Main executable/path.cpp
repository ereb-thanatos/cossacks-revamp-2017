#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "MapDiscr.h"
#include <assert.h>
#include "mode.h"
#include "NewMon.h"
#include <math.h>
#include "Path.h"
#include "TopoGraf.h"

bool AllowPathDelay;

MotionBrush::MotionBrush()
{
	Data = nullptr;
	Lx = 0; Ly = 0;
	Dx = 0; Dy = 0;
}

MotionBrush::~MotionBrush()
{
	if (Data)
	{
		free(Data);
		Data = nullptr;
		Lx = 0; Ly = 0;
		Dx = 0; Dy = 0;
	}
}

void MotionBrush::CreateRoundBrush(int d)
{
	Data = new int[d];
	memset(Data, 0, d << 2);
	for (int y = 0; y < d; y++)
	{
		for (int x = 0; x < d; x++)
		{
			int rr = int(sqrt((x + x - d)*(x + x - d) + (y + y - d)*(y + y - d)));
			if (rr < d + 1)
			{
				Data[y] |= 1 << x;
			}
		}
	}
	Lx = d;
	Ly = d;
	Dx = d >> 1;
	Dy = d >> 1;
}

void MotionBrush::CreateSquareBrush(int d)
{
	Data = new int[d];
	memset(Data, 0, d * 4);
	for (int y = 0; y < d; y++)
	{
		for (int x = 0; x < d; x++)
		{
			Data[y] |= 1 << x;
		}
	}
	Lx = d;
	Ly = d;
	Dx = d >> 1;
	Dy = d >> 1;
}


//-------------------------------------------------------//
//                                                       //
//                    ОБХОД ПРЕПЯТСТВИЙ                  //
//                                                       //
//-------------------------------------------------------//

const int Power2[24] = {
		0x00000001,0x00000003,0x00000007,0x0000000F,
		0x0000001F,0x0000003F,0x0000007F,0x000000FF,
		0x000001FF,0x000003FF,0x000007FF,0x00000FFF,
		0x00001FFF,0x00003FFF,0x00007FFF,0x0000FFFF,
		0x0001FFFF,0x0003FFFF,0x0007FFFF,0x000FFFFF,
		0x001FFFFF,0x003FFFFF,0x007FFFFF,0x00FFFFFF
};

//Zero out MapV
void MotionField::ClearMaps()
{
	memset(MapV, 0, MAPSY*BMSX);//1024*128
}

void MotionField::BSetPt(int x, int y)
{
	int MAPV = int(MapV);
	if (x >= 0 && x < MAPSX&&y >= 0 && y < MAPSY) {
		switch (ADDSH) {
		case 1:
			__asm {
				//Horisontal
				/*
				mov		eax,x
				mov		ecx,eax
				and		ecx,7
				shr		eax,3
				mov		ebx,y
				shl		ebx,7//MAPSHF
				add		ebx,eax
				mov		al,1
				shl		al,cl
				add		ebx,MAPH
				or		[ebx],al
				*/
				//Vertical
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 7//MAPSHF
				add		ebx, eax
				mov		al, 1
				shl		al, cl
				add		ebx, MAPV
				or [ebx], al
			};
			break;
		case 2:
			__asm {
				//Horisontal
				/*
				mov		eax,x
				mov		ecx,eax
				and		ecx,7
				shr		eax,3
				mov		ebx,y
				shl		ebx,8//MAPSHF
				add		ebx,eax
				mov		al,1
				shl		al,cl
				add		ebx,MAPH
				or		[ebx],al
				*/
				//Vertical
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 8//MAPSHF
				add		ebx, eax
				mov		al, 1
				shl		al, cl
				add		ebx, MAPV
				or [ebx], al
			};
			break;
		case 3:
			__asm {
				//Horisontal
				/*
				mov		eax,x
				mov		ecx,eax
				and		ecx,7
				shr		eax,3
				mov		ebx,y
				shl		ebx,9//MAPSHF
				add		ebx,eax
				mov		al,1
				shl		al,cl
				add		ebx,MAPH
				or		[ebx],al
				*/
				//Vertical
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 9//MAPSHF
				add		ebx, eax
				mov		al, 1
				shl		al, cl
				add		ebx, MAPV
				or [ebx], al
			};
			break;
		};
	};
}

void MotionField::BClrPt(int x, int y)
{
	int MAPV = int(MapV);
	if (x >= 0 && x < MAPSX&&y >= 0 && y < MAPSY) {
		switch (ADDSH) {
		case 1:
			__asm {
				//Horisontal
				/*
				mov		eax,x
				mov		ecx,eax
				and		ecx,7
				shr		eax,3
				mov		ebx,y
				shl		ebx,7//MAPSHF
				add		ebx,eax
				mov		al,254
				rol		al,cl
				add		ebx,MAPH
				and		[ebx],al
				*/
				//Vertical
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 7//MAPSHF
				add		ebx, eax
				mov		al, 254
				rol		al, cl
				add		ebx, MAPV
				and[ebx], al
			};
			break;
		case 2:
			__asm {
				//Horisontal
				/*
				mov		eax,x
				mov		ecx,eax
				and		ecx,7
				shr		eax,3
				mov		ebx,y
				shl		ebx,8//MAPSHF
				add		ebx,eax
				mov		al,254
				rol		al,cl
				add		ebx,MAPH
				and		[ebx],al
				*/
				//Vertical
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 8//MAPSHF
				add		ebx, eax
				mov		al, 254
				rol		al, cl
				add		ebx, MAPV
				and[ebx], al
			};
			break;
		case 3:
			__asm {
				//Horisontal
				/*
				mov		eax,x
				mov		ecx,eax
				and		ecx,7
				shr		eax,3
				mov		ebx,y
				shl		ebx,9//MAPSHF
				add		ebx,eax
				mov		al,254
				rol		al,cl
				add		ebx,MAPH
				and		[ebx],al
				*/
				//Vertical
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 9//MAPSHF
				add		ebx, eax
				mov		al, 254
				rol		al, cl
				add		ebx, MAPV
				and[ebx], al
			};
			break;
		};

	};
}

void MotionField::BSetBar(int x, int y, int Lx) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Lx; iy++)
		{
			BSetPt(ix, iy);
		}
	}
}

void MotionField::BClrBar(int x, int y, int Lx) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Lx; iy++)
		{
			BClrPt(ix, iy);
		}
	}
}

void MotionField::BSetSQ(int x, int y, int Lx, int Ly) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Ly; iy++)
		{
			BSetPt(ix, iy);
		}
	}
}

void MotionField::BClrSQ(int x, int y, int Lx, int Ly) {
	for (int ix = x; ix < x + Lx; ix++)
	{
		for (int iy = y; iy < y + Ly; iy++)
		{
			BClrPt(ix, iy);
		}
	}
}

//Checks coordinates against MotionField::MapV
//Possible return values: 0, 1, 2, 4, 8, 16, 32, 64, 128
int MotionField::CheckPt(int x, int y)
{
	int retval = 0;
	int MAPV = int(MapV);
	if (x >= 0 && x < MAPSX&&y >= 0 && y < MAPSY) 
	{
		switch (ADDSH) 
		{
		case 1:
			__asm 
			{
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 7//MAPSHF
				add		ebx, eax
				mov		al, 1
				shl		al, cl
				add		ebx, MAPV
				and		al, [ebx]
				and eax, 0xFF
				mov		retval, eax
			}
			break;

		case 2:
			__asm 
			{
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 8//MAPSHF
				add		ebx, eax
				mov		al, 1
				shl		al, cl
				add		ebx, MAPV
				and		al, [ebx]
				and eax, 0xFF
				mov		retval, eax
			}
			break;

		case 3:
			__asm 
			{
				mov		eax, y
				mov		ecx, eax
				and		ecx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 9//MAPSHF
				add		ebx, eax
				mov		al, 1
				shl		al, cl
				add		ebx, MAPV
				and		al, [ebx]
				and eax, 0xFF
				mov		retval, eax
			}
			break;
		}
		return retval;
	}
	else
	{
		return 1;
	}
}

int MotionField::CheckHLine(int x, int y, int Lx)
{
	for (int i = 0; i < Lx; i++)
	{
		if (CheckPt(x + i, y))
		{
			return 1;
		}
	}
	return 0;
}

int MotionField::CheckVLine(int x, int y, int Lx)
{
	int retval = 0;
	int MAPV = int(MapV);
	if (x > 0 && y > 0 && y + Lx - 1 < MAPSY && x < MAPSX && Lx <= 24)
	{
		switch (ADDSH)
		{
		case 1:
			__asm
			{
				mov		eax, y
				mov		edx, eax
				and		edx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 7//MAPSHF
				add		ebx, eax
				mov		ecx, Lx
				mov		eax, 1
				shl		eax, cl
				dec		eax
				mov		cl, dl
				rol		eax, cl
				add		ebx, MAPV
				and		eax, dword ptr[ebx]
				mov		retval, eax
			}
			break;

		case 2:
			__asm
			{
				mov		eax, y
				mov		edx, eax
				and		edx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 8//MAPSHF
				add		ebx, eax
				mov		ecx, Lx
				mov		eax, 1
				shl		eax, cl
				dec		eax
				mov		cl, dl
				rol		eax, cl
				add		ebx, MAPV
				and		eax, dword ptr[ebx]
				mov		retval, eax
			}
			break;

		case 3:
			__asm
			{
				mov		eax, y
				mov		edx, eax
				and		edx, 7
				shr		eax, 3
				mov		ebx, x
				shl		ebx, 9//MAPSHF
				add		ebx, eax
				mov		ecx, Lx
				mov		eax, 1
				shl		eax, cl
				dec		eax
				mov		cl, dl
				rol		eax, cl
				add		ebx, MAPV
				and		eax, dword ptr[ebx]
				mov		retval, eax
			}
			break;
		}
		return retval;
	}
	else
	{
		return 1;
	}
}

bool MotionField::CheckBar(int x, int y, int Lx, int Ly)
{
	for (int ix = 0; ix < Lx; ix++)
	{
		if (CheckVLine(x + ix, y, Ly))
		{
			return true;
		}
	}
	return false;
}

extern byte NewCirc[16];

void MotionField::FillNewCirc(int x, int y, int Lx, int Ly)
{
	int* xx = (int*)NewCirc;
	xx[0] = 0;
	xx[1] = 0;

	if (CheckHLine(x, y - 1, Lx))
	{
		NewCirc[0] = 1;
	}
	if (CheckHLine(x, y + Ly, Lx))
	{
		NewCirc[4] = 1;
	}
	if (CheckVLine(x + Lx, y, Ly))
	{
		NewCirc[2] = 1;
	}
	if (CheckVLine(x - 1, y, Ly))
	{
		NewCirc[6] = 1;
	}

	if (Ly < 2)
	{
		if (CheckPt(x - 1, y - 1))
			NewCirc[7] = 1;

		if (CheckPt(x + Lx, y - 1))
			NewCirc[1] = 1;

		if (CheckPt(x - 1, y + Ly))
			NewCirc[5] = 1;

		if (CheckPt(x + Lx, y + Ly))
			NewCirc[3] = 1;
	}
	else
	{
		if (Ly == 2)
		{
			if (CheckHLine(x + 1, y - 1, Lx) && CheckPt(x + Lx, y))
				NewCirc[1] = 1;

			if (CheckHLine(x + 1, y + Ly, Lx) && CheckPt(x + Lx, y + 1))
				NewCirc[3] = 1;

			if (CheckHLine(x - 1, y + Ly, Lx) && CheckPt(x - 1, y + 1))
				NewCirc[5] = 1;

			if (CheckHLine(x - 1, y - 1, Lx) && CheckPt(x - 1, y))
				NewCirc[7] = 1;
		}
		else
		{
			if (CheckHLine(x + 1, y - 1, Lx) || CheckVLine(x + Lx, y, Ly - 1))
				NewCirc[1] = 1;

			if (CheckHLine(x + 1, y + Ly, Lx) || CheckVLine(x + Lx, y + 1, Ly - 1))
				NewCirc[3] = 1;

			if (CheckHLine(x - 1, y + Ly, Lx) || CheckVLine(x - 1, y + 1, Ly - 1))
				NewCirc[5] = 1;

			if (CheckHLine(x - 1, y - 1, Lx) || CheckVLine(x - 1, y, Ly - 1))
				NewCirc[7] = 1;
		}
	}
	xx[2] = xx[0];
	xx[3] = xx[1];
}

//------------------------------------------------//
//          KERNEL OF THE MOTION ENGINE           //
//            Search for the best way             //
//------------------------------------------------//
#define MaxP (4096)
extern word pxx[MaxP + 160];
extern word pyy[MaxP + 160];
extern word RVIS[MaxP + 160];
extern word LVIS[MaxP + 160];
int GetLAngle(int dx, int dy, int Angle);
int GetRAngle(int dx, int dy, int Angle);
const int ndrr[9] = { 7,6,5,0,0,4,1,2,3 };
const int nidrx[8] = { 0,1,1,1,0,-1,-1,-1 };
const int nidry[8] = { -1,-1,0,1,1,1,0,-1 };

MotionField MFIELDS[2];//0-Land,1-Water

bool FindBestPosition(OneObject* OB, int* xd, int* yd, int R0)
{
	MotionField* MFI = &MFIELDS[OB->LockType];

	if (!OB)
		return false;

	if (!MFI->CheckBar((*xd) - 1, (*yd) - 1, OB->Lx + 2, OB->Lx + 2))
		return true;

	int Lx = OB->Lx + 2;
	int R1 = R0;
	int bx = *xd;
	int by = *yd;
	int bdist = 100000;
	int xxx = bx - 1;
	int yyy = by - 1;
	int LL = 2;
	int bx1, by1, dstn;
	int x = OB->x;
	int y = OB->y;
	while (R1)
	{
		for (int i = 0; i <= LL; i++)if (!MFI->CheckBar(xxx + i - 1, yyy - 1, Lx, Lx)) {
			bx1 = xxx + i;
			by1 = yyy;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		for (int i = 0; i <= LL; i++)if (!MFI->CheckBar(xxx + i - 1, yyy + LL - 1, Lx, Lx)) {
			bx1 = xxx + i;
			by1 = yyy + LL;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		for (int i = 0; i < LL - 1; i++)if (!MFI->CheckBar(xxx - 1, yyy + i, Lx, Lx)) {
			bx1 = xxx;
			by1 = yyy + i + 1;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		for (int i = 0; i < LL - 1; i++)if (!MFI->CheckBar(xxx + LL - 1, yyy + i, Lx, Lx)) {
			bx1 = xxx + LL;
			by1 = yyy + i + 1;
			dstn = Norma(bx1 - x, by1 - y);
			if (dstn < bdist) {
				bx = bx1;
				by = by1;
				bdist = dstn;
			};
		};
		if (bdist < 100000) {
			*xd = bx;
			*yd = by;
			return true;
		};
		R1--;
		LL += 2;
		xxx--;
		yyy--;
	};
	return false;
}

extern int COUNTER;

void CreateFullPath(int x1, int y1, OneObject* OB)
{
	OB->DeletePath();
	OB->NIPoints = 0;
	int xx1 = x1;
	int yy1 = y1;

	if (!FindBestPosition(OB, &xx1, &yy1, 40))
		return;

	bool pc;
	int NN = 0;
	AllowPathDelay = false;
	do
	{
		int t0 = GetTickCount();

		pc = OB->CreatePrePath(xx1, yy1);
		t0 = GetTickCount() - t0;

		if (pc) {
			xx1 = OB->PathX[OB->NIPoints - 1];
			yy1 = OB->PathY[OB->NIPoints - 1];
		};
		NN++;
		rando();
	} while (pc&&NN < 4);

	if (AllowPathDelay)
	{
		OB->PathDelay = (rando() & 7);
	}
}

#define sssh 1
extern int COUNTER;
bool OneObject::CreatePrePath(int x1, int y1)
{
	AllowPathDelay = false;
	if (LockType == 1) {
		return CreatePrePath2(x1, y1);
	};
	if (PathDelay)return false;

	AllowPathDelay = true;
	if (abs(x - x1) < 2 && abs(y - y1) < 2)return false;
	MotionField* MFI = &MFIELDS[LockType];
	if (GLock)MFI->BClrBar(x, y, Lx);
	bool InLocked = MFI->CheckBar(x, y, Lx, Lx);
	if (InLocked&&PathX) {
		if (GLock)MFI->BSetBar(x, y, Lx);
		return false;
	};
	int sdx = x1 - x;
	int	sdy = y1 - y;
	int	Cum = 0;
	int Pps = 0;
	int sx = (sdx > 0) ? 1 : -1;
	int sy = (sdy > 0) ? 1 : -1;
	int dx = abs(sdx);
	int dy = abs(sdy);
	int	Mdx = dx;
	int	Mdy = dy;
	int	Mx = x;
	int	My = y;
	int	xx1 = x;
	int yy1 = y;
	int rx = sx;
	int ry = sy;
	if (dx > dy)ry = 0;
	if (dy > dx)rx = 0;
	int Angle0 = GetLAngle(x1 - x, y1 - y, 0);
	int Angle;
	int ddx, ddy;
	int Lvp = 0;
	int Rvp = 0;
	bool LvpLast = false;
	bool RvpLast = false;
	int RvpDist = 100000;
	int LvpDist = 100000;

	CurIPoint = 0;
	NeedPath = false;

	//соединяем линией начальную и конечную точки. 
	//Оптимизация только по скорости
	__asm
	{
		mov		ax, word ptr Mdx
		mov		bx, word ptr Mdy
		xor		edx, edx  //Pps
		xor		ecx, ecx  //Cum
		mov		si, word ptr Mx
		mov		di, word ptr My
		cmp		bx, ax
		jae		Lp5xx
		//dx>dy
		mov		word ptr[pxx + edx], si
		mov		word ptr[pyy + edx], di
		add		edx, 2
		or ax, ax
		jz		LoopsEnd
		cmp		sy, 0
		jl		Lp3xx
		cmp		sx, 0
		jl		Lp2begin
		//dx>dy,sx>0,sy>0
		Lp1begin :
		inc		si	//x++
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp1_1
			sub		cx, word ptr Mdx
			inc		di  //y++
			Lp1_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp1begin
			jmp		LoopsEnd
			Lp2begin : //dx>dy,sx<0,sy>0
		dec		si	//x--
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp2_1
			sub		cx, word ptr Mdx
			inc		di //y++
			Lp2_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp2begin
			jmp		LoopsEnd
			Lp3xx :	//dy<0
		cmp		sx, 0
			jl		Lp4begin
			Lp3begin : //dx>dy,sx>0,sy<0
		inc		si	//x++
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp3_1
			sub		cx, word ptr Mdx
			dec		di //y--
			Lp3_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp3begin
			jmp		LoopsEnd
			Lp4begin : //dx>dy,sx<0,sy<0
		dec		si	//x--
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp4_1
			sub		cx, word ptr Mdx
			dec		di //y--
			Lp4_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp4begin
			jmp		LoopsEnd
			Lp5xx :	//dx<dy
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			or bx, bx
			jz		LoopsEnd
			cmp		sx, 0
			jl		Lp7xx
			cmp		sy, 0
			jl		Lp6begin
			Lp5Begin :
		inc		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp5_1
			sub		cx, word ptr dy
			inc		si	//x++
			Lp5_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp5begin
			jmp		LoopsEnd
			Lp6Begin ://sx>0,sy<0
		dec		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp6_1
			sub		cx, word ptr dy
			inc		si	//x++
			Lp6_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp6begin
			jmp		LoopsEnd
			Lp7xx :	//dx<0
		cmp		sy, 0
			jl		Lp8begin
			Lp7Begin ://dx<0,dy>0
		inc		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp7_1
			sub		cx, word ptr dy
			dec		si	//x--
			Lp7_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp7begin
			jmp		LoopsEnd
			Lp8Begin ://dx<0,dy<0
		dec		di	//y--
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp8_1
			sub		cx, word ptr dy
			dec		si	//x--
			Lp8_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp8begin
			loopsEnd :
		shr		edx, 1
			mov		Pps, edx
	}

	Pps--;

	if (InLocked)
	{
		int uu;
		for (uu = 0; uu < Pps&&MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
		if (uu >= Pps)
		{
			return false;
		}

		PathX = new short;
		PathY = new short;
		PathX[0] = pxx[uu];
		PathY[0] = pyy[uu];
		NIPoints = 1;
		return true;
	}

	int RVisPos = 0;
	int LVisPos = 0;
	bool RightPrefer = true;
	int Rtx;//current point 
	int Rty;
	int Ltx;
	int Lty;
	word Rpx[MaxP];//right path
	word Rpy[MaxP];
	word Lpx[MaxP];//left path
	word Lpy[MaxP];
	int Rpp = 1;//index of current point
	bool LDoing;//=true if last point reached
	bool RDoing;
	byte Rdirc;//currend direction
	byte Ldirc;
	int Rmaxalt;//maximum alteration,right path
	int Lmaxalt;//maximum alteration,left path
	int Rcum = 0;
	int Rcum1 = 0;
	int Lcum = 0;
	int Lcum1 = 0;
	bool Rvis, Lvis;
	//Проверяем прямую проходимость
	int uu;
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
	if (uu == Pps)return false;
	//Идем, пока не упремся в стенку
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu] - 1, pyy[uu] - 1, Lx + 2, Lx + 2); uu++);
	Rpp = uu;
	Rtx = pxx[Rpp];
	Rty = pyy[Rpp];
	int Rppm = uu;
	int Lppm = uu;
	// Если dx>dy,то на каждом шагу dx изменяетя строго на 1
	if (Rtx != x1 || Rty != y1) {
		//LLock[y][x]=false;
		if (Rpp > 0)Rpp -= 1;
		Rtx = pxx[Rpp];
		Rty = pyy[Rpp];
		Ltx = pxx[Rpp];
		Lty = pyy[Rpp];
		int Ppi = Rpp + 1;
		LDoing = true;
		RDoing = true;
		//Ищем, пока находимся в занятой зоне
		while (MFI->CheckBar(pxx[Ppi] - 1, pyy[Ppi] - 1, Lx + 2, Lx + 2) && Ppi < Pps)Ppi++;
		if (Ppi > Pps)LDoing = false;//Кон.точка недостижима
		int Xls = pxx[Ppi - 1];
		int Yls = pyy[Ppi - 1];
		//Уперлись...Вычисляем направление движения
		Rdirc = ndrr[(pxx[Rpp + 1] - pxx[Rpp] + 1) * 3 + pyy[Rpp + 1] - pyy[Rpp] + 1];
		Ldirc = Rdirc;
		//Выбираем начальное направление-right
		MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
		int dirc1 = (Rdirc + 1) & 7;
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1++, z++);
		if (!NewCirc[6 + (dirc1 & 7)])dirc1++;
		Rdirc = dirc1 & 7;
		//-left
		dirc1 = 8 + ((Ldirc + 7) & 7);
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1--, z++);
		if (!NewCirc[(dirc1 & 7) + 2])dirc1--;
		Ldirc = dirc1 & 7;
		//Идем по правому краю до тех пор пока вновь не пересе-
		//чемся с прямой линией, соединяющей начальную и конечную
		//точки
		Rmaxalt = 0;
		Lmaxalt = 0;
		while (Rpp < MaxP - 8 && LDoing&&RDoing) {
			//пытаемся повернуть направо
			MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
			int dirc1 = (Rdirc + 6) & 7;
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1++, z++);
			Rdirc = dirc1 & 7;
			Rpp++;
			int Tdx = nidrx[Rdirc];
			int Tdy = nidry[Rdirc];
			Rcum -= sdy*Tdx;
			Rcum += sdx*Tdy;
			Rtx += Tdx;
			Rty += Tdy;
			Rpx[Rpp] = Rtx; Rpy[Rpp] = Rty;
			Angle = GetLAngle(Rtx - x, Rty - y, Angle0);
			if (Angle > Rmaxalt) {
				Rmaxalt = Angle;
				Rppm = Rpp;
			};
			//проверяем условие прямой видимости
			ddx = x1 - Rtx;
			ddy = y1 - Rty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			int dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			int dirmo = (ndrr[Tdx + Tdx + Tdx + Tdy + 4] - dirvi) & 7;
			if ((dirmo == 1 || dirmo == 7 || dirmo == 0) && !MFI->CheckBar(Rtx + ddx + ddx - 1, Rty + ddy + ddy - 1, Lx + 2, Lx + 2)) {
				//if(!NewCirc[dirvi]){
					//direct vision
				RVIS[RVisPos] = Rpp;
				RVisPos++;
			};
			//the same, but left
			MFI->FillNewCirc(Ltx - 1, Lty - 1, Lx + 2, Lx + 2);
			dirc1 = 8 + ((Ldirc + 2) & 7);
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1--, z++);
			Ldirc = dirc1 & 7;
			Tdx = nidrx[Ldirc];
			Tdy = nidry[Ldirc];
			Lcum += sdy*Tdx;
			Lcum -= sdx*Tdy;
			Ltx += Tdx;
			Lty += Tdy;
			Lpx[Rpp] = Ltx; Lpy[Rpp] = Lty;
			Angle = GetRAngle(Ltx - x, Lty - y, Angle0);
			if (Angle > Lmaxalt) {
				Lmaxalt = Angle;
				Lppm = Rpp;
			};
			//проверяем условие прямой видимости
			ddx = x1 - Ltx;
			ddy = y1 - Lty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			dirmo = (ndrr[Tdx + Tdx + Tdx + Tdy + 4] - dirvi) & 7;
			if ((dirmo == 1 || dirmo == 7 || dirmo == 0) && !MFI->CheckBar(Ltx + ddx + ddx - 1, Lty + ddy + ddy - 1, Lx + 2, Lx + 2)) {
				//if(!NewCirc[dirvi]){
				LVIS[LVisPos] = Rpp;
				LVisPos++;
			};
			//Вычисляем значение y на прямой линии, соотв. 
			//данному x
			if (Rcum <= 0 && Rcum1 >= 0) {
				if (!Rvis)Rvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Rtx >= Xls)RDoing = false;
					}
					else if (Rtx <= Xls)RDoing = false;
				}
				else {
					if (sy > 0) {
						if (Rty >= Yls)RDoing = false;
					}
					else if (Rty <= Yls)RDoing = false;
				};
				if (!RDoing)RightPrefer = true;
			};
			Rcum1 = Rcum;
			//the same for left
			if (Lcum <= 0 && Lcum1 >= 0) {
				if (!Lvis)Lvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Ltx >= Xls)LDoing = false;
					}
					else if (Ltx <= Xls)LDoing = false;
				}
				else {
					if (sy > 0) {
						if (Lty >= Yls)LDoing = false;
					}
					else if (Lty <= Yls)LDoing = false;
				};
				if (!LDoing)RightPrefer = false;
			};
			Lcum1 = Lcum;
		};
		//LLock[y][x]=true;
		if (Rpp < MaxP - 9)
		{
			if (RightPrefer)
			{
				Pps = Rppm;
				//search for direct vision point
				int i;
				for (i = 0; i < RVisPos&&RVIS[i] < Pps; i++);

				if (i < RVisPos)
				{
					Rvp = RVIS[i];
				}
				else
				{
					Rvp = Rpp - 1;
				}
				if (Rvp < 0)
				{
					Rvp = 0;
				}
			}
			else
			{
				Pps = Lppm;
				//Rvp=Lvp;
				//search for direct vision point
				int i;
				for (i = 0; i < LVisPos&&LVIS[i] < Pps; i++);

				if (i < LVisPos)
				{
					Rvp = LVIS[i];
				}
				else
				{
					Rvp = Rpp - 1;
				}
				if (Rvp < 0)
				{
					Rvp = 0;
				}
			}
			if (Rvp < Rpp)Rvp++;
			if (Rvp < Pps)Rvp = Pps;
			int addNIPoints = Rvp - Pps + 1;
			int maxp = addNIPoints;
			CurIPoint = 0;
			if (addNIPoints > 2) {
				addNIPoints = 2 + ((addNIPoints - 2) >> sssh);
			};
			short* OldPathX = PathX;
			short* OldPathY = PathY;
			PathX = new short[NIPoints + addNIPoints];
			PathY = new short[NIPoints + addNIPoints];
			if (OldPathX) {
				memcpy(PathX, OldPathX, NIPoints << 1);
				memcpy(PathY, OldPathY, NIPoints << 1);
				free(OldPathX);
				free(OldPathY);
			};
			if (RightPrefer) {
				if (maxp <= 2) {
					PathX[NIPoints] = Rpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Rpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Rpx[Pps];
						PathY[NIPoints + 1] = Rpy[Pps];
					};
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Rpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Rpy[Pps + (nn << sssh)];
					};
					PathX[NIPoints] = Rpx[Pps + maxp - 1];
					PathY[NIPoints] = Rpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
				};
			}
			else {
				if (maxp <= 2) {
					PathX[NIPoints] = Lpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Lpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Lpx[Pps];
						PathY[NIPoints + 1] = Lpy[Pps];
					};
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Lpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Lpy[Pps + (nn << sssh)];
					};
					PathX[NIPoints] = Lpx[Pps + maxp - 1];
					PathY[NIPoints] = Lpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
				};
			};
			NeedPath = true;
			if (GLock)MFI->BSetBar(x, y, Lx);
			return true;
		}
		else {
			if (GLock)MFI->BSetBar(x, y, Lx);
			PathDelay = 200 + (rando() & 15);
			return false;
		};
	};
	if (GLock)MFI->BSetBar(x, y, Lx);
	return false;
};
#undef sssh
#define sssh 2
bool OneObject::CreatePrePath2(int x1, int y1) {
	if (PathDelay)return false;
	if (abs(x - x1) < 2 && abs(y - y1) < 2)return false;
	MotionField* MFI = &MFIELDS[LockType];
	if (GLock)MFI->BClrBar(x, y, Lx);
	bool InLocked = MFI->CheckBar(x, y, Lx, Lx);
	if (InLocked&&PathX) {
		if (GLock)MFI->BClrBar(x, y, Lx);
		return false;
	};
	int sdx = x1 - x;
	int	sdy = y1 - y;
	int	Cum = 0;
	int Pps = 0;
	int sx = (sdx > 0) ? 1 : -1;
	int sy = (sdy > 0) ? 1 : -1;
	int dx = abs(sdx);
	int dy = abs(sdy);
	int	Mdx = dx;
	int	Mdy = dy;
	int	Mx = x;
	int	My = y;
	int	xx1 = x;
	int yy1 = y;
	int rx = sx;
	int ry = sy;
	if (dx > dy)ry = 0;
	if (dy > dx)rx = 0;
	int Angle0 = GetLAngle(x1 - x, y1 - y, 0);
	int Angle;
	int ddx, ddy;
	int Lvp = 0;
	int Rvp = 0;
	bool LvpLast = false;
	bool RvpLast = false;
	int RvpDist = 100000;
	int LvpDist = 100000;
	//if(PathX)free(PathX);
	//if(PathY)free(PathY);
	//PathX=NULL;
	//PathY=NULL;
	//NIPoints=0;
	CurIPoint = 0;
	NeedPath = false;
	//соединяем линией начальную и конечную точки. 
	//Оптимизация только по скорости
	__asm {
		mov		ax, word ptr Mdx
		mov		bx, word ptr Mdy
		xor		edx, edx  //Pps
		xor		ecx, ecx  //Cum
		mov		si, word ptr Mx
		mov		di, word ptr My
		cmp		bx, ax
		jae		Lp5xx
		//dx>dy
		mov		word ptr[pxx + edx], si
		mov		word ptr[pyy + edx], di
		add		edx, 2
		or ax, ax
		jz		LoopsEnd
		cmp		sy, 0
		jl		Lp3xx
		cmp		sx, 0
		jl		Lp2begin
		//dx>dy,sx>0,sy>0
		Lp1begin :
		inc		si	//x++
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp1_1
			sub		cx, word ptr Mdx
			inc		di  //y++
			Lp1_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp1begin
			jmp		LoopsEnd
			Lp2begin : //dx>dy,sx<0,sy>0
		dec		si	//x--
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp2_1
			sub		cx, word ptr Mdx
			inc		di //y++
			Lp2_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp2begin
			jmp		LoopsEnd
			Lp3xx :	//dy<0
		cmp		sx, 0
			jl		Lp4begin
			Lp3begin : //dx>dy,sx>0,sy<0
		inc		si	//x++
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp3_1
			sub		cx, word ptr Mdx
			dec		di //y--
			Lp3_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp3begin
			jmp		LoopsEnd
			Lp4begin : //dx>dy,sx<0,sy<0
		dec		si	//x--
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp4_1
			sub		cx, word ptr Mdx
			dec		di //y--
			Lp4_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp4begin
			jmp		LoopsEnd
			Lp5xx :	//dx<dy
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			or bx, bx
			jz		LoopsEnd
			cmp		sx, 0
			jl		Lp7xx
			cmp		sy, 0
			jl		Lp6begin
			Lp5Begin :
		inc		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp5_1
			sub		cx, word ptr dy
			inc		si	//x++
			Lp5_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp5begin
			jmp		LoopsEnd
			Lp6Begin ://sx>0,sy<0
		dec		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp6_1
			sub		cx, word ptr dy
			inc		si	//x++
			Lp6_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp6begin
			jmp		LoopsEnd
			Lp7xx :	//dx<0
		cmp		sy, 0
			jl		Lp8begin
			Lp7Begin ://dx<0,dy>0
		inc		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp7_1
			sub		cx, word ptr dy
			dec		si	//x--
			Lp7_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp7begin
			jmp		LoopsEnd
			Lp8Begin ://dx<0,dy<0
		dec		di	//y--
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp8_1
			sub		cx, word ptr dy
			dec		si	//x--
			Lp8_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp8begin
			loopsEnd :
		shr		edx, 1
			mov		Pps, edx
	};
	Pps--;
	//assert(Pps<MaxP);
	if (InLocked) {
		int uu;
		for (uu = 0; uu < Pps&&MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
		if (uu >= Pps)return false;
		PathX = new short;
		PathY = new short;
		PathX[0] = pxx[uu];
		PathY[0] = pyy[uu];
		NIPoints = 1;
		return true;
	};
	bool RightPrefer = true;
	int Rtx;//current point 
	int Rty;
	int Ltx;
	int Lty;
	word Rpx[MaxP];//right path
	word Rpy[MaxP];
	word Lpx[MaxP];//left path
	word Lpy[MaxP];
	int Rpp = 1;//index of current point
	bool LDoing;//=true if last point reached
	bool RDoing;
	byte Rdirc;//currend direction
	byte Ldirc;
	int Rmaxalt;//maximum alteration,right path
	int Lmaxalt;//maximum alteration,left path
	int Rcum = 0;
	int Rcum1 = 0;
	int Lcum = 0;
	int Lcum1 = 0;
	bool Rvis, Lvis;
	//Проверяем прямую проходимость
	int uu;
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
	if (uu == Pps)return false;
	//Идем, пока не упремся в стенку
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu] - 1, pyy[uu] - 1, Lx + 2, Lx + 2); uu++);
	Rpp = uu;
	Rtx = pxx[Rpp];
	Rty = pyy[Rpp];
	int Rppm = uu;
	int Lppm = uu;
	// Если dx>dy,то на каждом шагу dx изменяетя строго на 1
	if (Rtx != x1 || Rty != y1) {
		//LLock[y][x]=false;
		if (Rpp > 0)Rpp -= 1;
		Rtx = pxx[Rpp];
		Rty = pyy[Rpp];
		Ltx = pxx[Rpp];
		Lty = pyy[Rpp];
		int Ppi = Rpp + 1;
		LDoing = true;
		RDoing = true;
		//Ищем, пока находимся в занятой зоне
		while (MFI->CheckBar(pxx[Ppi] - 1, pyy[Ppi] - 1, Lx + 2, Lx + 2) && Ppi < Pps)Ppi++;
		if (Ppi > Pps)LDoing = false;//Кон.точка недостижима
		int Xls = pxx[Ppi - 1];
		int Yls = pyy[Ppi - 1];
		//Уперлись...Вычисляем направление движения
		Rdirc = ndrr[(pxx[Rpp + 1] - pxx[Rpp] + 1) * 3 + pyy[Rpp + 1] - pyy[Rpp] + 1];
		Ldirc = Rdirc;
		//Выбираем начальное направление-right
		MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
		int dirc1 = (Rdirc + 1) & 7;
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1++, z++);
		if (!NewCirc[6 + (dirc1 & 7)])dirc1++;
		Rdirc = dirc1 & 7;
		//-left
		dirc1 = 8 + ((Ldirc + 7) & 7);
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1--, z++);
		if (!NewCirc[(dirc1 & 7) + 2])dirc1--;
		Ldirc = dirc1 & 7;
		//Идем по правому краю до тех пор пока вновь не пересе-
		//чемся с прямой линией, соединяющей начальную и конечную
		//точки
		Rmaxalt = 0;
		Lmaxalt = 0;
		while (Rpp < MaxP - 8 && LDoing&&RDoing) {
			//пытаемся повернуть направо
			MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
			int dirc1 = (Rdirc + 6) & 7;
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1++, z++);
			Rdirc = dirc1 & 7;
			Rpp++;
			int Tdx = nidrx[Rdirc];
			int Tdy = nidry[Rdirc];
			Rcum -= sdy*Tdx;
			Rcum += sdx*Tdy;
			Rtx += Tdx;
			Rty += Tdy;
			Rpx[Rpp] = Rtx; Rpy[Rpp] = Rty;
			Angle = GetLAngle(Rtx - x, Rty - y, Angle0);
			if (Angle > Rmaxalt) {
				Rmaxalt = Angle;
				Rppm = Rpp;
			};
			//проверяем условие прямой видимости
			ddx = x1 - Rtx;
			ddy = y1 - Rty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			int dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			if (!NewCirc[dirvi]) {
				Rvis = true;
				if (!RvpLast) {
					//int distant=Norma(x1-Rtx,y1-Rty);
					//if(RvpDist>distant){
					Rvp = Rpp;
					RvpLast = true;
					//	RvpDist=distant;
					//}else{
					//	RvpLast=true;
					//};
				};
			}
			else {
				RvpLast = false;
				Rvis = false;
			};
			//the same, but left
			MFI->FillNewCirc(Ltx - 1, Lty - 1, Lx + 2, Lx + 2);
			dirc1 = 8 + ((Ldirc + 2) & 7);
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1--, z++);
			Ldirc = dirc1 & 7;
			Tdx = nidrx[Ldirc];
			Tdy = nidry[Ldirc];
			Lcum += sdy*Tdx;
			Lcum -= sdx*Tdy;
			Ltx += Tdx;
			Lty += Tdy;
			Lpx[Rpp] = Ltx; Lpy[Rpp] = Lty;
			Angle = GetRAngle(Ltx - x, Lty - y, Angle0);
			if (Angle > Lmaxalt) {
				Lmaxalt = Angle;
				Lppm = Rpp;
			};
			//проверяем условие прямой видимости
			ddx = x1 - Ltx;
			ddy = y1 - Lty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			if (!NewCirc[dirvi]) {
				Lvis = true;
				if (!LvpLast) {
					//int distant=Norma(x1-Ltx,y1-Lty);
					//if(LvpDist>distant){
					Lvp = Rpp;
					LvpLast = true;
					//	LvpDist=distant;
					//}else LvpLast=true;
				};
			}
			else {
				LvpLast = false;
				Lvis = false;
			};
			//Вычисляем значение y на прямой линии, соотв. 
			//данному x
			if (Rcum <= 0 && Rcum1 >= 0) {
				if (!Rvis)Rvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Rtx >= Xls)RDoing = false;
					}
					else if (Rtx <= Xls)RDoing = false;
				}
				else {
					if (sy > 0) {
						if (Rty >= Yls)RDoing = false;
					}
					else if (Rty <= Yls)RDoing = false;
				};
				if (!RDoing)RightPrefer = true;
			};
			Rcum1 = Rcum;
			//the same for left
			if (Lcum <= 0 && Lcum1 >= 0) {
				if (!Lvis)Lvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Ltx >= Xls)LDoing = false;
					}
					else if (Ltx <= Xls)LDoing = false;
				}
				else {
					if (sy > 0) {
						if (Lty >= Yls)LDoing = false;
					}
					else if (Lty <= Yls)LDoing = false;
				};
				if (!LDoing)RightPrefer = false;
			};
			Lcum1 = Lcum;
		};
		//LLock[y][x]=true;
		if (Rpp < MaxP - 9) {
			if (RightPrefer) {
				//if(Rppm+1<Rpp)Rppm+=1;else Rppm=Rpp;
				//memcpy(xx,Rpx,(Rppm+1));
				//memcpy(yy,Rpy,(Rppm+1));
				Pps = Rppm;
			}
			else {
				//if(Lppm+1<Rpp)Lppm+=1;else Lppm=Rpp;
				//memcpy(xx,Lpx,(Lppm+1));
				//memcpy(yy,Lpy,(Lppm+1));
				Pps = Lppm;
				Rvp = Lvp;
				//for(int G=Pps;G<=Rpp;G++)assert(Lpx[G]<400&&Lpx[G]>0);
			};
			if (Rvp < Rpp)Rvp++;
			if (Rvp < Pps)Rvp = Pps;
			int addNIPoints = Rvp - Pps + 1;
			int maxp = addNIPoints;
			CurIPoint = 0;
			if (addNIPoints > 2) {
				addNIPoints = 2 + ((addNIPoints - 2) >> sssh);
			};
			short* OldPathX = PathX;
			short* OldPathY = PathY;
			PathX = new short[NIPoints + addNIPoints];
			PathY = new short[NIPoints + addNIPoints];
			if (OldPathX) {
				memcpy(PathX, OldPathX, NIPoints << 1);
				memcpy(PathY, OldPathY, NIPoints << 1);
				free(OldPathX);
				free(OldPathY);
			};
			if (RightPrefer) {
				if (maxp <= 2) {
					PathX[NIPoints] = Rpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Rpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Rpx[Pps];
						PathY[NIPoints + 1] = Rpy[Pps];
					};
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Rpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Rpy[Pps + (nn << sssh)];
					};
					PathX[NIPoints] = Rpx[Pps + maxp - 1];
					PathY[NIPoints] = Rpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
				};
			}
			else {
				if (maxp <= 2) {
					PathX[NIPoints] = Lpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Lpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Lpx[Pps];
						PathY[NIPoints + 1] = Lpy[Pps];
					};
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Lpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Lpy[Pps + (nn << sssh)];
					};
					PathX[NIPoints] = Lpx[Pps + maxp - 1];
					PathY[NIPoints] = Lpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
				};
			};
			NeedPath = true;
			if (GLock)MFI->BSetBar(x, y, Lx);
			return true;
		}
		else {
			if (GLock)MFI->BSetBar(x, y, Lx);
			PathDelay = 64 + (rando() & 15);
			return false;
		};
	};
	if (GLock)MFI->BSetBar(x, y, Lx);
	return false;
};
#undef sssh
#define sssh 3
bool OneObject::CreatePrePath4(int x1, int y1) {
	if (PathDelay)return false;
	if (abs(x - x1) < 2 && abs(y - y1) < 2)return false;
	MotionField* MFI = &MFIELDS[LockType];
	if (GLock)MFI->BClrBar(x, y, Lx);
	bool InLocked = MFI->CheckBar(x, y, Lx, Lx);
	if (InLocked&&PathX) {
		if (GLock)MFI->BClrBar(x, y, Lx);
		return false;
	};
	int sdx = x1 - x;
	int	sdy = y1 - y;
	int	Cum = 0;
	int Pps = 0;
	int sx = (sdx > 0) ? 1 : -1;
	int sy = (sdy > 0) ? 1 : -1;
	int dx = abs(sdx);
	int dy = abs(sdy);
	int	Mdx = dx;
	int	Mdy = dy;
	int	Mx = x;
	int	My = y;
	int	xx1 = x;
	int yy1 = y;
	int rx = sx;
	int ry = sy;
	if (dx > dy)ry = 0;
	if (dy > dx)rx = 0;
	int Angle0 = GetLAngle(x1 - x, y1 - y, 0);
	int Angle;
	int ddx, ddy;
	int Lvp = 0;
	int Rvp = 0;
	bool LvpLast = false;
	bool RvpLast = false;
	int RvpDist = 100000;
	int LvpDist = 100000;
	//if(PathX)free(PathX);
	//if(PathY)free(PathY);
	//PathX=NULL;
	//PathY=NULL;
	//NIPoints=0;
	CurIPoint = 0;
	NeedPath = false;
	//соединяем линией начальную и конечную точки. 
	//Оптимизация только по скорости
	__asm {
		mov		ax, word ptr Mdx
		mov		bx, word ptr Mdy
		xor		edx, edx  //Pps
		xor		ecx, ecx  //Cum
		mov		si, word ptr Mx
		mov		di, word ptr My
		cmp		bx, ax
		jae		Lp5xx
		//dx>dy
		mov		word ptr[pxx + edx], si
		mov		word ptr[pyy + edx], di
		add		edx, 2
		or ax, ax
		jz		LoopsEnd
		cmp		sy, 0
		jl		Lp3xx
		cmp		sx, 0
		jl		Lp2begin
		//dx>dy,sx>0,sy>0
		Lp1begin :
		inc		si	//x++
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp1_1
			sub		cx, word ptr Mdx
			inc		di  //y++
			Lp1_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp1begin
			jmp		LoopsEnd
			Lp2begin : //dx>dy,sx<0,sy>0
		dec		si	//x--
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp2_1
			sub		cx, word ptr Mdx
			inc		di //y++
			Lp2_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp2begin
			jmp		LoopsEnd
			Lp3xx :	//dy<0
		cmp		sx, 0
			jl		Lp4begin
			Lp3begin : //dx>dy,sx>0,sy<0
		inc		si	//x++
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp3_1
			sub		cx, word ptr Mdx
			dec		di //y--
			Lp3_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp3begin
			jmp		LoopsEnd
			Lp4begin : //dx>dy,sx<0,sy<0
		dec		si	//x--
			add		cx, bx
			cmp		cx, word ptr Mdx
			jb		Lp4_1
			sub		cx, word ptr Mdx
			dec		di //y--
			Lp4_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		ax
			jnz		Lp4begin
			jmp		LoopsEnd
			Lp5xx :	//dx<dy
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			or bx, bx
			jz		LoopsEnd
			cmp		sx, 0
			jl		Lp7xx
			cmp		sy, 0
			jl		Lp6begin
			Lp5Begin :
		inc		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp5_1
			sub		cx, word ptr dy
			inc		si	//x++
			Lp5_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp5begin
			jmp		LoopsEnd
			Lp6Begin ://sx>0,sy<0
		dec		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp6_1
			sub		cx, word ptr dy
			inc		si	//x++
			Lp6_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp6begin
			jmp		LoopsEnd
			Lp7xx :	//dx<0
		cmp		sy, 0
			jl		Lp8begin
			Lp7Begin ://dx<0,dy>0
		inc		di	//y++
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp7_1
			sub		cx, word ptr dy
			dec		si	//x--
			Lp7_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp7begin
			jmp		LoopsEnd
			Lp8Begin ://dx<0,dy<0
		dec		di	//y--
			add		cx, ax
			cmp		cx, word ptr dy
			jb		Lp8_1
			sub		cx, word ptr dy
			dec		si	//x--
			Lp8_1 :
		mov		word ptr[pxx + edx], si
			mov		word ptr[pyy + edx], di
			add		edx, 2
			dec		bx
			jnz		Lp8begin
			loopsEnd :
		shr		edx, 1
			mov		Pps, edx
	};
	Pps--;
	//assert(Pps<MaxP);
	if (InLocked)
	{
		int uu;
		for (uu = 0; uu < Pps&&MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
		if (uu >= Pps)return false;
		PathX = new short;
		PathY = new short;
		PathX[0] = pxx[uu];
		PathY[0] = pyy[uu];
		NIPoints = 1;
		return true;
	};
	bool RightPrefer = true;
	int Rtx;//current point 
	int Rty;
	int Ltx;
	int Lty;
	word Rpx[MaxP];//right path
	word Rpy[MaxP];
	word Lpx[MaxP];//left path
	word Lpy[MaxP];
	int Rpp = 1;//index of current point
	bool LDoing;//=true if last point reached
	bool RDoing;
	byte Rdirc;//currend direction
	byte Ldirc;
	int Rmaxalt;//maximum alteration,right path
	int Lmaxalt;//maximum alteration,left path
	int Rcum = 0;
	int Rcum1 = 0;
	int Lcum = 0;
	int Lcum1 = 0;
	bool Rvis, Lvis;
	//Проверяем прямую проходимость
	int uu;
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu], pyy[uu], Lx, Lx); uu++);
	if (uu == Pps)return false;
	//Идем, пока не упремся в стенку
	for (uu = 1; uu < Pps && !MFI->CheckBar(pxx[uu] - 1, pyy[uu] - 1, Lx + 2, Lx + 2); uu++);
	Rpp = uu;
	Rtx = pxx[Rpp];
	Rty = pyy[Rpp];
	int Rppm = uu;
	int Lppm = uu;
	// Если dx>dy,то на каждом шагу dx изменяетя строго на 1
	if (Rtx != x1 || Rty != y1) {
		//LLock[y][x]=false;
		if (Rpp > 0)Rpp -= 1;
		Rtx = pxx[Rpp];
		Rty = pyy[Rpp];
		Ltx = pxx[Rpp];
		Lty = pyy[Rpp];
		int Ppi = Rpp + 1;
		LDoing = true;
		RDoing = true;
		//Ищем, пока находимся в занятой зоне
		while (MFI->CheckBar(pxx[Ppi] - 1, pyy[Ppi] - 1, Lx + 2, Lx + 2) && Ppi < Pps)Ppi++;
		if (Ppi > Pps)LDoing = false;//Кон.точка недостижима
		int Xls = pxx[Ppi - 1];
		int Yls = pyy[Ppi - 1];
		//Уперлись...Вычисляем направление движения
		Rdirc = ndrr[(pxx[Rpp + 1] - pxx[Rpp] + 1) * 3 + pyy[Rpp + 1] - pyy[Rpp] + 1];
		Ldirc = Rdirc;
		//Выбираем начальное направление-right
		MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
		int dirc1 = (Rdirc + 1) & 7;
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1++, z++);
		if (!NewCirc[6 + (dirc1 & 7)])dirc1++;
		Rdirc = dirc1 & 7;
		//-left
		dirc1 = 8 + ((Ldirc + 7) & 7);
		for (int z = 0; NewCirc[dirc1] && z < 7; dirc1--, z++);
		if (!NewCirc[(dirc1 & 7) + 2])dirc1--;
		Ldirc = dirc1 & 7;
		//Идем по правому краю до тех пор пока вновь не пересе-
		//чемся с прямой линией, соединяющей начальную и конечную
		//точки
		Rmaxalt = 0;
		Lmaxalt = 0;
		while (Rpp < MaxP - 8 && LDoing&&RDoing) {
			//пытаемся повернуть направо
			MFI->FillNewCirc(Rtx - 1, Rty - 1, Lx + 2, Lx + 2);
			int dirc1 = (Rdirc + 6) & 7;
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1++, z++);
			Rdirc = dirc1 & 7;
			Rpp++;
			int Tdx = nidrx[Rdirc];
			int Tdy = nidry[Rdirc];
			Rcum -= sdy*Tdx;
			Rcum += sdx*Tdy;
			Rtx += Tdx;
			Rty += Tdy;
			Rpx[Rpp] = Rtx; Rpy[Rpp] = Rty;
			Angle = GetLAngle(Rtx - x, Rty - y, Angle0);
			if (Angle > Rmaxalt) {
				Rmaxalt = Angle;
				Rppm = Rpp;
			};
			//проверяем условие прямой видимости
			ddx = x1 - Rtx;
			ddy = y1 - Rty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			int dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			if (!NewCirc[dirvi]) {
				Rvis = true;
				if (!RvpLast) {
					//int distant=Norma(x1-Rtx,y1-Rty);
					//if(RvpDist>distant){
					Rvp = Rpp;
					RvpLast = true;
					//	RvpDist=distant;
					//}else{
					//	RvpLast=true;
					//};
				};
			}
			else {
				RvpLast = false;
				Rvis = false;
			};
			//the same, but left
			MFI->FillNewCirc(Ltx - 1, Lty - 1, Lx + 2, Lx + 2);
			dirc1 = 8 + ((Ldirc + 2) & 7);
			for (int z = 0; z < 6 && NewCirc[dirc1]; dirc1--, z++);
			Ldirc = dirc1 & 7;
			Tdx = nidrx[Ldirc];
			Tdy = nidry[Ldirc];
			Lcum += sdy*Tdx;
			Lcum -= sdx*Tdy;
			Ltx += Tdx;
			Lty += Tdy;
			Lpx[Rpp] = Ltx; Lpy[Rpp] = Lty;
			Angle = GetRAngle(Ltx - x, Lty - y, Angle0);
			if (Angle > Lmaxalt) {
				Lmaxalt = Angle;
				Lppm = Rpp;
			};
			//проверяем условие прямой видимости
			ddx = x1 - Ltx;
			ddy = y1 - Lty;
			if (ddx > 1)ddx = 1;
			if (ddx < -1)ddx = -1;
			if (ddy > 1)ddy = 1;
			if (ddy < -1)ddy = -1;
			dirvi = ndrr[ddx + ddx + ddx + ddy + 4];
			if (!NewCirc[dirvi]) {
				Lvis = true;
				if (!LvpLast) {
					//int distant=Norma(x1-Ltx,y1-Lty);
					//if(LvpDist>distant){
					Lvp = Rpp;
					LvpLast = true;
					//	LvpDist=distant;
					//}else LvpLast=true;
				};
			}
			else {
				LvpLast = false;
				Lvis = false;
			};
			//Вычисляем значение y на прямой линии, соотв. 
			//данному x
			if (Rcum <= 0 && Rcum1 >= 0) {
				if (!Rvis)Rvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Rtx >= Xls)RDoing = false;
					}
					else if (Rtx <= Xls)RDoing = false;
				}
				else {
					if (sy > 0) {
						if (Rty >= Yls)RDoing = false;
					}
					else if (Rty <= Yls)RDoing = false;
				};
				if (!RDoing)RightPrefer = true;
			};
			Rcum1 = Rcum;
			//the same for left
			if (Lcum <= 0 && Lcum1 >= 0) {
				if (!Lvis)Lvp = Rpp;
				if (dx > dy) {
					if (sx > 0) {
						if (Ltx >= Xls)LDoing = false;
					}
					else if (Ltx <= Xls)LDoing = false;
				}
				else {
					if (sy > 0) {
						if (Lty >= Yls)LDoing = false;
					}
					else if (Lty <= Yls)LDoing = false;
				};
				if (!LDoing)RightPrefer = false;
			};
			Lcum1 = Lcum;
		};
		//LLock[y][x]=true;
		if (Rpp < MaxP - 9) {
			if (RightPrefer) {
				//if(Rppm+1<Rpp)Rppm+=1;else Rppm=Rpp;
				//memcpy(xx,Rpx,(Rppm+1));
				//memcpy(yy,Rpy,(Rppm+1));
				Pps = Rppm;
			}
			else {
				//if(Lppm+1<Rpp)Lppm+=1;else Lppm=Rpp;
				//memcpy(xx,Lpx,(Lppm+1));
				//memcpy(yy,Lpy,(Lppm+1));
				Pps = Lppm;
				Rvp = Lvp;
				//for(int G=Pps;G<=Rpp;G++)assert(Lpx[G]<400&&Lpx[G]>0);
			};
			if (Rvp < Rpp)Rvp++;
			if (Rvp < Pps)Rvp = Pps;
			int addNIPoints = Rvp - Pps + 1;
			int maxp = addNIPoints;
			CurIPoint = 0;
			if (addNIPoints > 2) {
				addNIPoints = 2 + ((addNIPoints - 2) >> sssh);
			};
			short* OldPathX = PathX;
			short* OldPathY = PathY;
			PathX = new short[NIPoints + addNIPoints];
			PathY = new short[NIPoints + addNIPoints];
			if (OldPathX) {
				memcpy(PathX, OldPathX, NIPoints << 1);
				memcpy(PathY, OldPathY, NIPoints << 1);
				free(OldPathX);
				free(OldPathY);
			};
			if (RightPrefer) {
				if (maxp <= 2) {
					PathX[NIPoints] = Rpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Rpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Rpx[Pps];
						PathY[NIPoints + 1] = Rpy[Pps];
					};
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Rpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Rpy[Pps + (nn << sssh)];
					};
					PathX[NIPoints] = Rpx[Pps + maxp - 1];
					PathY[NIPoints] = Rpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
				};
			}
			else {
				if (maxp <= 2) {
					PathX[NIPoints] = Lpx[Pps + addNIPoints - 1];
					PathY[NIPoints] = Lpy[Pps + addNIPoints - 1];
					if (maxp == 2) {
						PathX[NIPoints + 1] = Lpx[Pps];
						PathY[NIPoints + 1] = Lpy[Pps];
					};
					NIPoints += addNIPoints;
				}
				else {
					int np = ((maxp - 2) >> sssh) + 1;
					for (int nn = 0; nn < np; nn++) {
						PathX[NIPoints + np - nn] = Lpx[Pps + (nn << sssh)];
						PathY[NIPoints + np - nn] = Lpy[Pps + (nn << sssh)];
					};
					PathX[NIPoints] = Lpx[Pps + maxp - 1];
					PathY[NIPoints] = Lpy[Pps + maxp - 1];
					NIPoints += addNIPoints;
				};
			};
			NeedPath = true;
			if (GLock)MFI->BSetBar(x, y, Lx);
			return true;
		}
		else {
			if (GLock)MFI->BSetBar(x, y, Lx);
			PathDelay = 64 + (rando() & 15);
			return false;
		};
	};
	if (GLock)MFI->BSetBar(x, y, Lx);
	return false;
};
/*
bool OneObject::CreatePrePathBordered(int x1,int y1,int Border){
	if(abs(x-x1)<2&&abs(y-y1)<2)return false;
	MotionField* MFI=&MFIELDS[LockType];
	bool InLocked=MFI->CheckBar(x,y,Lx,Lx);
	if(InLocked&&PathX)return false;
	int sdx=x1-x;
	int	sdy=y1-y;
	int	Cum=0;
	int Pps=0;
	int sx=(sdx>0)?1:-1;
	int sy=(sdy>0)?1:-1;
	int dx=abs(sdx);
	int dy=abs(sdy);
	int	Mdx=dx;
	int	Mdy=dy;
	int	Mx=x;
	int	My=y;
	int	xx1=x;
	int yy1=y;
	int rx=sx;
	int ry=sy;
	if(dx>dy)ry=0;
	if(dy>dx)rx=0;
	int Angle0=GetLAngle(x1-x,y1-y,0);
	int Angle;
	int ddx,ddy;
	int Lvp=0;
	int Rvp=0;
	bool LvpLast=false;
	bool RvpLast=false;
	int RvpDist=100000;
	int LvpDist=100000;
	//if(PathX)free(PathX);
	//if(PathY)free(PathY);
	//PathX=NULL;
	//PathY=NULL;
	//NIPoints=0;
	CurIPoint=0;
	NeedPath=false;
	//соединяем линией начальную и конечную точки.
	//Оптимизация только по скорости
	__asm{
		mov		ax,word ptr Mdx
		mov		bx,word ptr Mdy
		xor		edx,edx  //Pps
		xor		ecx,ecx  //Cum
		mov		si,word ptr Mx
		mov		di,word ptr My
		cmp		bx,ax
		jae		Lp5xx
		//dx>dy
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		or		ax,ax
		jz		LoopsEnd
		cmp		sy,0
		jl		Lp3xx
		cmp		sx,0
		jl		Lp2begin
		//dx>dy,sx>0,sy>0
Lp1begin:
		inc		si	//x++
		add		cx,bx
		cmp		cx,word ptr Mdx
		jb		Lp1_1
		sub		cx,word ptr Mdx
		inc		di  //y++
Lp1_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		ax
		jnz		Lp1begin
		jmp		LoopsEnd
Lp2begin: //dx>dy,sx<0,sy>0
		dec		si	//x--
		add		cx,bx
		cmp		cx,word ptr Mdx
		jb		Lp2_1
		sub		cx,word ptr Mdx
		inc		di //y++
Lp2_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		ax
		jnz		Lp2begin
		jmp		LoopsEnd
Lp3xx:	//dy<0
		cmp		sx,0
		jl		Lp4begin
Lp3begin: //dx>dy,sx>0,sy<0
		inc		si	//x++
		add		cx,bx
		cmp		cx,word ptr Mdx
		jb		Lp3_1
		sub		cx,word ptr Mdx
		dec		di //y--
Lp3_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		ax
		jnz		Lp3begin
		jmp		LoopsEnd
Lp4begin: //dx>dy,sx<0,sy<0
		dec		si	//x--
		add		cx,bx
		cmp		cx,word ptr Mdx
		jb		Lp4_1
		sub		cx,word ptr Mdx
		dec		di //y--
Lp4_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		ax
		jnz		Lp4begin
		jmp		LoopsEnd
Lp5xx:	//dx<dy
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		or		bx,bx
		jz		LoopsEnd
		cmp		sx,0
		jl		Lp7xx
		cmp		sy,0
		jl		Lp6begin
Lp5Begin:
		inc		di	//y++
		add		cx,ax
		cmp		cx,word ptr dy
		jb		Lp5_1
		sub		cx,word ptr dy
		inc		si	//x++
Lp5_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		bx
		jnz		Lp5begin
		jmp		LoopsEnd
Lp6Begin://sx>0,sy<0
		dec		di	//y++
		add		cx,ax
		cmp		cx,word ptr dy
		jb		Lp6_1
		sub		cx,word ptr dy
		inc		si	//x++
Lp6_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		bx
		jnz		Lp6begin
		jmp		LoopsEnd
Lp7xx:	//dx<0
		cmp		sy,0
		jl		Lp8begin
Lp7Begin://dx<0,dy>0
		inc		di	//y++
		add		cx,ax
		cmp		cx,word ptr dy
		jb		Lp7_1
		sub		cx,word ptr dy
		dec		si	//x--
Lp7_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		bx
		jnz		Lp7begin
		jmp		LoopsEnd
Lp8Begin://dx<0,dy<0
		dec		di	//y--
		add		cx,ax
		cmp		cx,word ptr dy
		jb		Lp8_1
		sub		cx,word ptr dy
		dec		si	//x--
Lp8_1:
		mov		word ptr[pxx+edx],si
		mov		word ptr[pyy+edx],di
		add		edx,2
		dec		bx
		jnz		Lp8begin
loopsEnd:
		shr		edx,1
		mov		Pps,edx
	};
	Pps--;
	assert(Pps<MaxP);
	if(InLocked){
		for(int uu=0;uu<Pps&&MFI->CheckBar(pxx[uu],pyy[uu],Lx,Lx);uu++);
		if(uu>=Pps)return false;
		PathX=new short;
		PathY=new short;
		PathX[0]=pxx[uu];
		PathY[0]=pyy[uu];
		NIPoints=1;
		return true;
	};
	bool RightPrefer=true;
	int Rtx;//current point
	int Rty;
	int Ltx;
	int Lty;
	word Rpx[MaxP];//right path
	word Rpy[MaxP];
	word Lpx[MaxP];//left path
	word Lpy[MaxP];
	int Rpp=1;//index of current point
	bool LDoing;//=true if last point reached
	bool RDoing;
	byte Rdirc;//currend direction
	byte Ldirc;
	int Rmaxalt;//maximum alteration,right path
	int Lmaxalt;//maximum alteration,left path
	int Rppm=1;
	int Lppm=1;
	int Rcum=0;
	int Rcum1=0;
	int Lcum=0;
	int Lcum1=0;
	bool Rvis,Lvis;
	//Проверяем прямую проходимость
	for(int uu=1;uu<Pps&&!MFI->CheckBar(pxx[uu],pyy[uu],Lx,Lx);uu++);
	if(uu==Pps)return false;
	//Идем, пока не упремся в стенку
	for(uu=1;uu<Pps&&!MFI->CheckBar(pxx[uu],pyy[uu],Lx,Lx);uu++);
	Rpp=uu;
	Rtx=pxx[Rpp];
	Rty=pyy[Rpp];

	// Если dx>dy,то на каждом шагу dx изменяетя строго на 1
	if(Rtx!=x1||Rty!=y1){
		//LLock[y][x]=false;
		if(Rpp>0)Rpp-=1;
		Rtx=pxx[Rpp];
		Rty=pyy[Rpp];
		Ltx=pxx[Rpp];
		Lty=pyy[Rpp];
		int Ppi=Rpp+1;
		LDoing=true;
		RDoing=true;
		//Ищем, пока находимся в занятой зоне
		while(MFI->CheckBar(pxx[Ppi],pyy[Ppi],Lx,Lx)&&Ppi<Pps)Ppi++;
		if(Ppi>Pps)LDoing=false;//Кон.точка недостижима
		int Xls=pxx[Ppi-1];
		int Yls=pyy[Ppi-1];
		//Уперлись...Вычисляем направление движения
		Rdirc=ndrr[(pxx[Rpp+1]-pxx[Rpp]+1)*3+pyy[Rpp+1]-pyy[Rpp]+1];
		Ldirc=Rdirc;
		//Выбираем начальное направление-right
		MFI->FillNewCirc(Rtx,Rty,Lx,Lx);
		int dirc1=(Rdirc+1)&7;
		for(int z=0;NewCirc[dirc1]&&z<7;dirc1++,z++);
		if(!NewCirc[6+(dirc1&7)])dirc1++;
		Rdirc=dirc1&7;
		//-left
		dirc1=8+((Ldirc+7)&7);
		for(z=0;NewCirc[dirc1]&&z<7;dirc1--,z++);
		if(!NewCirc[(dirc1&7)+2])dirc1--;
		Ldirc=dirc1&7;
		//Идем по правому краю до тех пор пока вновь не пересе-
		//чемся с прямой линией, соединяющей начальную и конечную
		//точки
		Rmaxalt=0;
		Lmaxalt=0;
		while(Rpp<MaxP-8&&LDoing&&RDoing){
			//пытаемся повернуть направо
			MFI->FillNewCirc(Rtx,Rty,Lx,Lx);
			int dirc1=(Rdirc+6)&7;
			for(int z=0;z<6&&NewCirc[dirc1];dirc1++,z++);
			Rdirc=dirc1&7;
			Rpp++;
			int Tdx=nidrx[Rdirc];
			int Tdy=nidry[Rdirc];
			Rcum-=sdy*Tdx;
			Rcum+=sdx*Tdy;
			Rtx+=Tdx;
			Rty+=Tdy;
			Rpx[Rpp]=Rtx;Rpy[Rpp]=Rty;
			Angle=GetLAngle(Rtx-x,Rty-y,Angle0);
			if(Angle>Rmaxalt){
				Rmaxalt=Angle;
				Rppm=Rpp;
			};
			//проверяем условие прямой видимости
			ddx=x1-Rtx;
			ddy=y1-Rty;
			if(ddx>1)ddx=1;
			if(ddx<-1)ddx=-1;
			if(ddy>1)ddy=1;
			if(ddy<-1)ddy=-1;
			int dirvi=ndrr[ddx+ddx+ddx+ddy+4];
			if(!NewCirc[dirvi]){
				Rvis=true;
				if(!RvpLast){
					//int distant=Norma(x1-Rtx,y1-Rty);
					//if(RvpDist>distant){
						Rvp=Rpp;
						RvpLast=true;
					//	RvpDist=distant;
					//}else{
					//	RvpLast=true;
					//};
				};
			}else{
				RvpLast=false;
				Rvis=false;
			};
			//the same, but left
			MFI->FillNewCirc(Ltx,Lty,Lx,Lx);
			dirc1=8+((Ldirc+2)&7);
			for(z=0;z<6&&NewCirc[dirc1];dirc1--,z++);
			Ldirc=dirc1&7;
			Tdx=nidrx[Ldirc];
			Tdy=nidry[Ldirc];
			Lcum+=sdy*Tdx;
			Lcum-=sdx*Tdy;
			Ltx+=Tdx;
			Lty+=Tdy;
			Lpx[Rpp]=Ltx;Lpy[Rpp]=Lty;
			Angle=GetRAngle(Ltx-x,Lty-y,Angle0);
			if(Angle>Lmaxalt){
				Lmaxalt=Angle;
				Lppm=Rpp;
			};
			//проверяем условие прямой видимости
			ddx=x1-Ltx;
			ddy=y1-Lty;
			if(ddx>1)ddx=1;
			if(ddx<-1)ddx=-1;
			if(ddy>1)ddy=1;
			if(ddy<-1)ddy=-1;
			dirvi=ndrr[ddx+ddx+ddx+ddy+4];
			if(!NewCirc[dirvi]){
				Lvis=true;
				if(!LvpLast){
					//int distant=Norma(x1-Ltx,y1-Lty);
					//if(LvpDist>distant){
						Lvp=Rpp;
						LvpLast=true;
					//	LvpDist=distant;
					//}else LvpLast=true;
				};
			}else{
				LvpLast=false;
				Lvis=false;
			};
			//Вычисляем значение y на прямой линии, соотв.
			//данному x
			if(Rcum<=0&&Rcum1>=0){
				if(!Rvis)Rvp=Rpp;
				if(dx>dy){
					if(sx>0){
						if(Rtx>=Xls)RDoing=false;
					}else if(Rtx<=Xls)RDoing=false;
				}else{
					if(sy>0){
						if(Rty>=Yls)RDoing=false;
					}else if(Rty<=Yls)RDoing=false;
				};
				if(!RDoing)RightPrefer=true;
			};
			Rcum1=Rcum;
			//the same for left
			if(Lcum<=0&&Lcum1>=0){
				if(!Lvis)Lvp=Rpp;
				if(dx>dy){
					if(sx>0){
						if(Ltx>=Xls)LDoing=false;
					}else if(Ltx<=Xls)LDoing=false;
				}else{
					if(sy>0){
						if(Lty>=Yls)LDoing=false;
					}else if(Lty<=Yls)LDoing=false;
				};
				if(!LDoing)RightPrefer=false;
			};
			Lcum1=Lcum;
		};
		//LLock[y][x]=true;
		if(Rpp<MaxP-9){
			if(RightPrefer){
				//if(Rppm+1<Rpp)Rppm+=1;else Rppm=Rpp;
				//memcpy(xx,Rpx,(Rppm+1));
				//memcpy(yy,Rpy,(Rppm+1));
				Pps=Rppm;
			}else{
				//if(Lppm+1<Rpp)Lppm+=1;else Lppm=Rpp;
				//memcpy(xx,Lpx,(Lppm+1));
				//memcpy(yy,Lpy,(Lppm+1));
				Pps=Lppm;
				Rvp=Lvp;
			};
			if(Rvp<Rpp)Rvp++;
			if(Rvp<Pps)Rvp=Pps;
			int addNIPoints=Rvp-Pps+1;
			int maxp=addNIPoints;
			CurIPoint=0;
			if(addNIPoints>2){
				addNIPoints=2+((addNIPoints-2)>>sssh);
			};
			short* OldPathX=PathX;
			short* OldPathY=PathY;
			PathX=new short [NIPoints+addNIPoints];
			PathY=new short [NIPoints+addNIPoints];
			if(OldPathX){
				memcpy(PathX,OldPathX,NIPoints<<1);
				memcpy(PathY,OldPathY,NIPoints<<1);
				free(OldPathX);
				free(OldPathY);
			};
			if(RightPrefer){
				if(maxp<=2){
					PathX[NIPoints]=Rpx[Pps+addNIPoints-1];
					PathY[NIPoints]=Rpy[Pps+addNIPoints-1];
					if(maxp==2){
						PathX[NIPoints+1]=Rpx[Pps];
						PathY[NIPoints+1]=Rpy[Pps];
					};
					NIPoints+=addNIPoints;
				}else{
					int np=((maxp-2)>>sssh)+1;
					for(int nn=0;nn<np;nn++){
						PathX[NIPoints+np-nn]=Rpx[Pps+(nn<<sssh)];
						PathY[NIPoints+np-nn]=Rpy[Pps+(nn<<sssh)];
					};
					PathX[NIPoints]=Rpx[Pps+maxp-1];
					PathY[NIPoints]=Rpy[Pps+maxp-1];
					NIPoints+=addNIPoints;
				};
			}else{
				if(maxp<=2){
					PathX[NIPoints]=Lpx[Pps+addNIPoints-1];
					PathY[NIPoints]=Lpy[Pps+addNIPoints-1];
					if(maxp==2){
						PathX[NIPoints+1]=Lpx[Pps];
						PathY[NIPoints+1]=Lpy[Pps];
					};
					NIPoints+=addNIPoints;
				}else{
					int np=((maxp-2)>>sssh)+1;
					for(int nn=0;nn<np;nn++){
						PathX[NIPoints+np-nn]=Lpx[Pps+(nn<<sssh)];
						PathY[NIPoints+np-nn]=Lpy[Pps+(nn<<sssh)];
					};
					PathX[NIPoints]=Lpx[Pps+maxp-1];
					PathY[NIPoints]=Lpy[Pps+maxp-1];
					NIPoints+=addNIPoints;
				};
			};
			NeedPath=true;
			return true;
		}else return false;
	};
	return false;
};
*/
void CorrectLockPosition(OneObject* OB) {
	MotionField* MFI = MFIELDS + OB->LockType;
	int x0 = OB->x;
	int y0 = OB->y;
	int Lx = OB->Lx;
	for (int r = 1; r < 40; r++) {
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		for (int j = 0; j < N; j++) {
			int x1 = x0 + xi[j];
			int y1 = y0 + yi[j];
			if (!MFI->CheckBar(x1, y1, Lx, Lx)) {
				OB->x = x1;
				OB->y = y1;
				OB->RealX = (x1 << 8) + (Lx << 7);
				OB->RealY = (y1 << 8) + (Lx << 7);
				return;
			};
		};
	};
};
#undef CreatePath
void OneObject::CreatePath(int x1, int y1) {


	if (UnlimitedMotion) {
		DestX = (x1 << 8) + ((Lx) << 7);
		DestY = (y1 << 8) + ((Lx) << 7);
		CPdestX = (word)-1;
		CPdestY = (word)-1;
		return;
	};
	MotionField* MFI = &MFIELDS[LockType];
	if (CPdestX == x1&&CPdestY == y1) {
		if (PathX) {
			if (NIPoints) {
				//if(MFI->CheckBar(x,y,Lx,Lx)){
					//BClrBar(x,y,Lx);         //???CHECK???
				//	CorrectLockPosition(this);
				//};
				int cx = PathX[NIPoints - 1];
				int cy = PathY[NIPoints - 1];
				int dis = DistTo(cx, cy);
				//if(dis<2&&NIPoints>1)NIPoints--;
				//else if((!dis))NIPoints--;
				if (dis < 2)NIPoints--;
				if (NIPoints) {
					cx = PathX[NIPoints - 1];
					cy = PathY[NIPoints - 1];
					if (MFI->CheckBar(cx, cy, Lx, Lx)) {
						NIPoints = 0;
						free(PathX);
						free(PathY);
						PathX = NULL;
						PathY = NULL;

						CreateFullPath(x1, y1, this);
					}
					else {
						DestX = (cx << 8) + ((Lx) << 7);
						DestY = (cy << 8) + ((Lx) << 7);
					};
				}
				else {
					free(PathX);
					free(PathY);
					PathX = NULL;
					PathY = NULL;

					CreateFullPath(x1, y1, this);
					if (NIPoints == 1) {
						int cx1 = PathX[0];
						int cy1 = PathY[0];
						if (Norma(cx1 - x, cy1 - y) < 3) {
							free(PathX);
							free(PathY);
							PathX = NULL;
							PathY = NULL;
							NIPoints = 0;
						};
					};
				};
			}
			else {

				CreateFullPath(x1, y1, this);
			};
		}
		else {
			DestX = (x1 << 8) + ((Lx) << 7);
			DestY = (y1 << 8) + ((Lx) << 7);
		};
	}
	else {
		if (PathX) {
			free(PathX);
			free(PathY);
			NIPoints = 0;
			PathX = NULL;
			PathY = NULL;
		};

		CreateFullPath(x1, y1, this);
	};
	CPdestX = x1;
	CPdestY = y1;
};
//void OneObject::CreateSimplePath(int x,int y){
//
//};

