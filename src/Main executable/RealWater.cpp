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

#include <cstring>

void CreateMiniMapPart(int x0, int y0, int x1, int y1);

#define WaveLx 260
#define WaveLy 260
#define WaveSize (WaveLx*WaveLy)
short Wave0[WaveSize * 2];
short Wave1[WaveSize * 2];
short Wave2[WaveSize * 2];

byte NSegments[WaveLy];
short Segments[WaveLy * 16];
short CostLine[4096];
short NCostPoints;
int Colors4[2048];
int CurStage;
int OldMapx;
int OldMapy;
void InitWatt();

#define WXShift 8
#define WYShift 8

short ScWave0[65536];
short ScWave1[65536];
short ScWave2[65536];

extern int tmtmt;
extern byte WaterCost[65536];

#define SetWAVE(x) x[pos]=dh;\
	               x[pos+1]=dh2;\
				   x[pos-1]=dh2;\
				   x[pos+WaveLx]=dh2;\
				   x[pos-WaveLx]=dh2;

#define WaterLevel 0

void ExtrapolateCell(int x, int y) 
{
	int xn, yn;
	do 
	{
		xn = rand() & 63;
		yn = rand() & 63;
	} while (!(xn > 1 && yn > 1 && xn < smaplx - 1 && yn < smaply - 1 && x != xn&&y != yn));
	//copy
	int xxx = (x + mapx) & 31;
	int yyy = (y + mapy) & 31;
	int src = (xxx << 4) + ((yyy << 9));
	int dst = (8 + (x << 3) + ((y << 3) + 8)*WaveLx) << 1;
	int DWx = WaveLx - 16;
	int SW0 = int(ScWave0);
	int W0 = int(Wave0);
	int SW1 = int(ScWave1);
	int W1 = int(Wave1);
	int SW2 = int(ScWave2);
	int W2 = int(Wave2);
	__asm {
		push	esi
		push	edi
		pushf
		mov		esi, SW0
		add		esi, src
		mov		edi, W0
		add		esi, dst
		mov		ecx, 8
		uuu1:
		mov		eax, [esi]
			mov		edx, [esi + 4]
			mov[edi], eax
			mov[edi + 4], edx
			mov		eax, [esi + 8]
			mov		edx, [esi + 12]
			mov[edi + 8], eax
			mov[edi + 12], edx
			add		esi, WaveLx * 2
			add		edi, 512
			dec		ecx
			jnz		uuu1

			mov		esi, SW1
			add		esi, src
			mov		edi, W1
			add		esi, dst
			mov		ecx, 8
			uuu2:
		mov		eax, [esi]
			mov		edx, [esi + 4]
			mov[edi], eax
			mov[edi + 4], edx
			mov		eax, [esi + 8]
			mov		edx, [esi + 12]
			mov[edi + 8], eax
			mov[edi + 12], edx
			add		esi, WaveLx * 2
			add		edi, 512
			dec		ecx
			jnz		uuu2

			mov		esi, SW2
			add		esi, src
			mov		edi, W2
			add		esi, dst
			mov		ecx, 8
			uuu3:
		mov		eax, [esi]
			mov		edx, [esi + 4]
			mov[edi], eax
			mov[edi + 4], edx
			mov		eax, [esi + 8]
			mov		edx, [esi + 12]
			mov[edi + 8], eax
			mov[edi + 12], edx
			add		esi, WaveLx * 2
			add		edi, 512
			dec		ecx
			jnz		uuu3

			popf
			pop		edi
			pop		esi

	};
	//for(int i=0;i<8;i++){
		/*
		memcpy(Wave0+dst,Wave0+src,16);
		memcpy(Wave1+dst,Wave1+src,16);
		memcpy(Wave2+dst,Wave2+src,16);
		if(i<2||i>5){
			for(int j=0;j<8;j++){
				Wave0[dst+j]=(Wave0[dst+j+1]+Wave0[dst+j-1]+Wave0[dst+j-WaveLx]+Wave0[dst+j+WaveLx])>>2;
				Wave1[dst+j]=(Wave1[dst+j+1]+Wave1[dst+j-1]+Wave1[dst+j-WaveLx]+Wave1[dst+j+WaveLx])>>2;
				Wave2[dst+j]=(Wave2[dst+j+1]+Wave2[dst+j-1]+Wave2[dst+j-WaveLx]+Wave2[dst+j+WaveLx])>>2;
			};
		};
		Wave0[dst]=(Wave0[dst+1]+Wave0[dst-1])>>1;
		Wave1[dst]=(Wave1[dst+1]+Wave1[dst-1])>>1;
		Wave2[dst]=(Wave2[dst+1]+Wave2[dst-1])>>1;
		Wave0[dst+7]=(Wave0[dst+8]+Wave0[dst+6])>>1;
		Wave1[dst+7]=(Wave1[dst+8]+Wave1[dst+6])>>1;
		Wave2[dst+7]=(Wave2[dst+8]+Wave2[dst+6])>>1;
		*/
		//src+=WaveLx;
		//dst+=WaveLx;
	//};
};
void ExtrapolateSurface(int x, int y, int Nx, int Ny) {
	for (int ix = 0; ix < Nx; ix++)
		for (int iy = 0; iy < Ny; iy++) {
			ExtrapolateCell(x + ix, y + iy);
		};
};
void GenerateSurface(int x, int y, int Lx, int Ly) {
	int pos = x + y*WaveLx;
	int WaveAdd = WaveLx - Lx;
	for (int yy = 0; yy < Ly; yy++) {
		/*
		memset(Wave0+pos,0,Lx<<1);
		memset(Wave1+pos,0,Lx<<1);
		memset(Wave2+pos,0,Lx<<1);
		*/
		for (int xx = 0; xx < Lx; xx++) {
			int dh = ((rand() & 511) - 256) * 110 / 10;
			int dh2 = dh >> 1;
			int dh3 = (dh + dh + dh) >> 2;
			int dh4 = dh >> 2;
			SetWAVE(Wave0);
			SetWAVE(Wave1);
			SetWAVE(Wave2);
			pos++;
		};
		pos += WaveAdd;
	};
	pos = x + y*WaveLx;
	for (int yy = 0; yy < Ly; yy++) {
		for (int xx = 0; xx < Lx; xx++) {
			Wave0[pos] = (Wave0[pos + 1] + Wave0[pos - 1] + Wave0[pos - WaveLx] + Wave0[pos + WaveLx]) >> 2;
			Wave1[pos] = (Wave1[pos + 1] + Wave1[pos - 1] + Wave1[pos - WaveLx] + Wave1[pos + WaveLx]) >> 2;
			Wave2[pos] = (Wave2[pos + 1] + Wave2[pos - 1] + Wave2[pos - WaveLx] + Wave2[pos + WaveLx]) >> 2;
			pos++;
		};
		pos += WaveAdd;
	};

	pos = x + y*WaveLx;
	for (int yy = 0; yy < Ly; yy++) {
		for (int xx = 0; xx < Lx; xx++) {
			Wave0[pos] = (Wave0[pos + 1] + Wave0[pos - 1] + Wave0[pos - WaveLx] + Wave0[pos + WaveLx]) >> 2;
			Wave1[pos] = (Wave1[pos + 1] + Wave1[pos - 1] + Wave1[pos - WaveLx] + Wave1[pos + WaveLx]) >> 2;
			Wave2[pos] = (Wave2[pos + 1] + Wave2[pos - 1] + Wave2[pos - WaveLx] + Wave2[pos + WaveLx]) >> 2;
			pos++;
		};
		pos += WaveAdd;
	};
	/*
	for(yy=0;yy<Ly;yy++){
		for(int xx=0;xx<Lx;xx++){
			Wave0[pos]=(Wave0[pos+1]+Wave0[pos-1]+Wave0[pos-WaveLx]+Wave0[pos+WaveLx])>>2;
			Wave1[pos]=(Wave1[pos+1]+Wave1[pos-1]+Wave1[pos-WaveLx]+Wave1[pos+WaveLx])>>2;
			Wave2[pos]=(Wave2[pos+1]+Wave2[pos-1]+Wave2[pos-WaveLx]+Wave2[pos+WaveLx])>>2;
			pos++;
		};
		pos+=WaveAdd;
	};*/
};
void SetPoint(short* Wave, int x, int y, int r, int h) {
	int r0 = r;
	int r1 = r0 * 2;
	double rrr = r0*r0 / 3;
	if (x <= r0 || y <= r0 || x >= WaveLx - r0 || y >= WaveLy - r0)return;
	int pos = x + y*WaveLx;
	for (int i = 0; i < r1; i++)
		for (int j = 0; j < r1; j++) {
			int rr = sqrt((i - r0)*(i - r0) + (j - r0)*(j - r0));
			if (rr < r0) {
				Wave[pos + i - 5 + (j - 5)*WaveLx] += h*exp(-double(rr*rr) / rrr);
			};
		};

}

void SetSpot(int x, int y)
{
	int h = (rand() & 4096);
	SetPoint(Wave0, x >> 2, y >> 1, 2, h);
	SetPoint(Wave1, x >> 2, y >> 1, 2, h);
	SetPoint(Wave2, x >> 2, y >> 1, 2, h);
}

#define ash 6

void InitWater()
{
	for (int i = 0; i < 2048; i++)
	{
		if (i < 1024 + ash - 12)
		{
			Colors4[i] = 0xB0B0B0B0;
		}
		else
		{
			if (i >= 1024 + ash + 10)
			{
				int dcc = (i - 1024 - ash - 10);
				if (dcc < 2)
				{
					Colors4[i] = 0xBBBBBBBB;
				}
				else
				{
					Colors4[i] = 0xBABABABA;
				}
			}
			else
			{
				int c1 = 0xB0 + ((i - 1024 - ash + 12) >> 1);
				if (i & 1)
				{
					Colors4[i] = c1 + ((c1 + 1) << 8) + (c1 << 16) + ((c1 + 1) << 24);
				}
				else
				{
					Colors4[i] = c1 + ((c1) << 8) + (c1 << 16) + ((c1) << 24);
				}
			}
		}
	}
	memset(Wave0, WaterLevel, sizeof Wave0);
	memset(Wave1, WaterLevel, sizeof Wave1);
	memset(Wave2, WaterLevel, sizeof Wave2);
	InitWatt();
	OldMapx = 0;
	OldMapy = 0;
	GenerateSurface(2, 2, WaveLx - 4, WaveLy - 4);
	CurStage = 0;
}

void ProcessWaves1(short* wave0, short* wave1, short* wave2, int xx, int yy, int Lx, int Ly) {
	int StWave = (xx + yy*WaveLx) << 1;
	int DWave = (WaveLx - Lx) << 1;
	int cyc = Ly;
	int cyc1;
	__asm {
		push	esi
		push	edi
		mov		esi, wave0
		mov		edi, wave1
		mov		ebx, wave2
		mov		edx, StWave

		lpp1 : mov		ecx, Lx
			   mov		cyc1, ecx
			   lpp2 : mov		ax, [edi + edx + 2]
					  add		ax, [edi + edx - 2]
					  add		ax, [edi + edx - WaveLx * 2]
					  add		ax, [edi + edx + WaveLx * 2]
					  mov		cx, [edi + edx]
					  sal		cx, 1
					  sub		ax, cx
					  sub		ax, cx

					  sar		ax, 3
					  add		ax, cx
					  sub		ax, [esi + edx]
					  mov		cx, ax
					  sar		cx, 16
					  sub		ax, cx
					  mov[ebx + edx], ax
					  add		edx, 2
					  dec		cyc1
					  jnz		lpp2
					  add		edx, DWave
					  dec		cyc
					  jnz		lpp1
					  pop		edi
					  pop		esi
	};
};
void ProcessWaves(short* wave0, short* wave1, short* wave2, int xx, int yy, int Lx, int Ly) {
	ProcessWaves1(wave0, wave1, wave2, xx, yy, Lx, Ly);
	return;
	int StWave = (xx + yy*WaveLx) << 1;
	int DWave = (WaveLx - Lx) << 1;
	int cyc = Ly;
	int cyc1;
	__asm {
		push	esi
		push	edi
		mov		esi, wave0
		mov		edi, wave1
		mov		ebx, wave2
		mov		edx, StWave

		lpp1 : mov		ecx, Lx
			   mov		cyc1, ecx
			   lpp2 : mov		ax, [edi + edx + 2]
					  add		ax, [edi + edx - 2]
					  add		ax, [edi + edx - WaveLx * 2]
					  add		ax, [edi + edx + WaveLx * 2]
					  mov		cx, [edi + edx]
					  sal		cx, 1
					  sub		ax, cx
					  sub		ax, cx
					  sar		ax, 4
					  add		ax, cx
					  sub		ax, [esi + edx]
					  mov[ebx + edx], ax
					  add		edx, 2
					  dec		cyc1
					  jnz		lpp2
					  add		edx, DWave
					  dec		cyc
					  jnz		lpp1
					  pop		edi
					  pop		esi
	};
};

void ProcessWaveFrame(short* Wave, int x, int y, int x1, int y1) {
	int pos1 = x + y*WaveLx;
	int dh = 0;
	for (int i = x; i <= x1; i++) {
		dh = ((dh + dh + dh) >> 2) + (rand() & 63) - 31;
		Wave[pos1] = Wave[pos1 + WaveLx] + dh;
		pos1++;
	};
	pos1 = x + y1*WaveLx;
	dh = 0;
	for (int i = x; i <= x1; i++) {
		dh = ((dh + dh + dh) >> 2) + (rand() & 63) - 31;
		Wave[pos1] = Wave[pos1 - WaveLx] + dh;
		pos1++;
	};
	pos1 = x + y*WaveLx + WaveLx;
	dh = 0;
	for (int i = y + 1; i <= y1 - 1; i++) {
		dh = ((dh + dh + dh) >> 2) + (rand() & 63) - 31;
		Wave[pos1] = Wave[pos1 + 1] + dh;
		pos1 += WaveLx;
	};
	pos1 = x1 + y*WaveLx + WaveLx;
	dh = 0;
	for (int i = y + 1; i <= y1 - 1; i++) {
		dh = ((dh + dh + dh) >> 2) + (rand() & 63) - 31;
		Wave[pos1] = Wave[pos1 - 1] + dh;
		pos1 += WaveLx;
	};
};

//FINAL WATER MAP
extern int WMPSIZE;
extern int MaxWX;
extern int MaxWY;
byte* WaterDeep;//cell size=32x16
byte* WaterBright;
word  CostPos[32 * 32];
short CostHi[32 * 32];
#define MaxCVal  30*256
#define CSpeed 2*128
#define CInvSpeed 128

int GOF(int x, int y)
{
	return (x & 31) + ((y & 31) << 5);
}

void InitCost()
{
	memset(WaterBright, 0, WMPSIZE);
	short PreCost[32 * 32];
	for (int i = 0; i < 32 * 32; i++)
	{
		PreCost[i] = rand()&(MaxCVal - 1);
		CostHi[i] = CostPos[i] - (MaxCVal >> 1);
	}

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			CostPos[GOF(x, y)] = (int(PreCost[GOF(x + 1, y)])
				+ int(PreCost[GOF(x - 1, y)])
				+ int(PreCost[GOF(x, y - 1)])
				+ int(PreCost[GOF(x, y + 1)])) >> 2;
		}
	}

	for (int p = 0; p < 10; p++)
	{
		int xx = rand() & 31;
		int yy = rand() & 31;
		for (int dx = 0; dx < 5; dx++)
		{
			for (int dy = 0; dy < 5; dy++)
			{
				if ((dx - 2)*(dx - 2) + (dy - 2)*(dy - 2) < 6)
				{
					CostPos[GOF(xx + dx, yy + dy)] |= 32768;
				}
			}
		}
	}
}

double ExSin(double x)
{
	return sin(x + sin(x));
}

void ProcessCost()
{
	for (int i = 0; i < 32 * 32; i++)
	{
		CostHi[i] = 0;
		word CP = CostPos[i];
		if (CP & 32768) {
			if (CP < 32768 + MaxCVal)CP += CSpeed;
			else CP &= 32767;
		}
		else {
			if (CP >= CInvSpeed)CP -= CInvSpeed;
			else CP |= 32768;
		};
		CostPos[i] = CP;
		CostHi[i] = ((CP & 32767) - (MaxCVal >> 1)) >> 2;
	}
}

//Zeroes WaterDeep
void ClearWater()
{
	memset(WaterDeep, 0, WMPSIZE);
}

void CreateMiniMap();

void SetDeepSpot(int x, int y, int r, int dr, int dh)
{
	int xx = x >> 5;
	int yy = y >> 4;
	int rr = (r + dr) >> 5;
	int x0 = xx - rr - 3;
	int y0 = yy - rr - 3;
	int x1 = xx + rr + 3;
	int y1 = yy + rr + 3;
	if (x0 < 0)x0 = 0;
	if (y0 < 0)y0 = 0;
	if (x0 >= MaxWX)x0 = MaxWX - 1;
	if (y0 >= MaxWY)y0 = MaxWY - 1;
	if (x1 < 0)x1 = 0;
	if (y1 < 0)y1 = 0;
	if (x1 >= MaxWX)x1 = MaxWX - 1;
	if (y1 >= MaxWY)y1 = MaxWY - 1;
	y <<= 1;
	bool Change = false;

	for (int xa = x0; xa <= x1; xa++)
	{
		for (int ya = y0; ya < y1; ya++)
		{
			int dx = x - (xa << 5);
			int dy = y - (ya << 5);
			int ra = sqrt(dx*dx + dy*dy);
			int dhi = 0;

			if (ra < r)
			{
				dhi = dh;
			}
			else
			{
				if (ra >= r + dr)
				{
					dhi = 0;
				}
				else
				{
					dhi = div(dh*(r + dr - ra), dr).quot;
				}
			}

			int pos = xa + ya*MaxWX;

			if (dhi > 255)
			{
				dhi = 255;
			}

			if (WaterDeep[pos] < dhi)
			{
				WaterDeep[pos] = dhi;
				Change = true;
			}
		}
	}
	if (Change)
	{
		CreateMiniMapPart((x0 >> 1) - 2, (y0 >> 1) - 2, (x1 >> 1) + 2, (y1 >> 1) + 2);
	}
}

void SetBrightSpot(int x, int y, int Brightness, bool dir) {
	if (Brightness > 15)Brightness = 15;
	int xx = x >> 5;
	int yy = y >> 4;
	int BRI16 = Brightness << 4;
	int Bri2 = Brightness*(Brightness + 1);
	if (dir) {
		//increase brightess
		for (int dx = -Brightness; dx <= Brightness; dx++)
			for (int dy = -Brightness; dy <= Brightness; dy++) {
				int rr = int(sqrt((dx*dx + dy*dy) << 8));
				int r = rr >> 4;
				int x1 = xx + dx;
				int y1 = yy + dy;
				if (r <= Brightness&&x1 >= 0 && y1 >= 0 && x1 < (MaxWX) && y1 < (MAPSY >> 1)) {
					int bri = BRI16 - rr;
					int ofst = x1 + (y1*(MaxWX));
					if (WaterBright[ofst] < bri)WaterBright[ofst] = bri;
				};
			};
	}
	else {
		//decrease brightess
		int MBright = 16 + Brightness;
		for (int dx = -MBright; dx <= MBright; dx++)
			for (int dy = -MBright; dy <= MBright; dy++) {
				int rr = int(sqrt((dx*dx + dy*dy) << 8));
				int r = rr >> 4;
				int x1 = xx + dx;
				int y1 = yy + dy;
				if (r <= MBright&&x1 >= 0 && y1 >= 0 && x1 < (MaxWX) && y1 < (MAPSY >> 1)) {
					int ofst = x1 + (y1*(MaxWX));
					if (r < Brightness)WaterBright[ofst] = 0;
					else {
						int bri = rr - BRI16;
						if (WaterBright[ofst] > bri)WaterBright[ofst] = bri;
					};
				};
			};
	};
};
void SetGoodDeepSpot(int x, int y, int r, int dr, int dh) {
	int dr1 = div(dr*(dh - 128), (143 - 128)).quot;
	if (dr1 >= r) {
		dh = 128 + div((dh - 128)*dr1, r).quot;
		dr = r;
	};
	int r0 = r - dr1;
	int r1 = r + div(dr1 << 7, (dh - 128)).quot;
	SetDeepSpot(x, y, r0, r1 - r0 + 1, dh);
};
void ClearDeepSpot(int x, int y, int r, int dr, int dh) {
	int xx = x >> 5;
	int yy = y >> 4;
	int rr = (r + dr) >> 5;
	int x0 = xx - rr - 3;
	int y0 = yy - rr - 3;
	int x1 = xx + rr + 3;
	int y1 = yy + rr + 3;
	if (x0 < 0)x0 = 0;
	if (y0 < 0)y0 = 0;
	if (x0 >= MaxWX)x0 = MaxWX - 1;
	if (y0 >= MaxWY)y0 = MaxWY - 1;
	if (x1 < 0)x1 = 0;
	if (y1 < 0)y1 = 0;
	if (x1 >= MaxWX)x1 = MaxWX - 1;
	if (y1 >= MaxWY)y1 = MaxWY - 1;
	y <<= 1;
	bool Change = false;
	for (int xa = x0; xa <= x1; xa++) {
		for (int ya = y0; ya < y1; ya++) {
			int dx = x - (xa << 5);
			int dy = y - (ya << 5);
			int ra = sqrt(dx*dx + dy*dy);
			int dhi = 0;
			if (ra < r)dhi = dh;
			else if (ra >= r + dr)dhi = 0;
			else dhi = div(dh*(r + dr - ra), dr).quot;
			int pos = xa + ya*MaxWX;
			//dhi+=WaterDeep[pos];
			if (dhi > 255)dhi = 255;
			dhi = 255 - dhi;
			if (WaterDeep[pos] > dhi) {
				WaterDeep[pos] = dhi;
				Change = true;
			};
		};
	};
	if (Change) {
		CreateMiniMapPart((x0 >> 1) - 2, (y0 >> 1) - 2, (x1 >> 1) + 2, (y1 >> 1) + 2);
	};
};
void ClearGoodDeepSpot(int x, int y, int r, int dr, int dh) {
	if (r <= 0)return;
	int dr1 = div(dr*(dh - 128), (143 - 128)).quot;
	if (dr1 >= r) {
		dh = 128 + div((dh - 128)*dr1, r).quot;
		dr = r;
	};
	int r0 = r - dr1;
	int r1 = r + div(dr1 << 7, (dh - 128)).quot;
	ClearDeepSpot(x, y, r0, r1 - r0 + 1, dh);
};
#define ashift 7
void DrawCost(int x, int y, short* Wave, int z1, int z2, int z3, int z4, int Msh) {
	int ASHI = 1024 + Msh;
	if (z1 < 128 && z2 < 128 && z3 < 128 && z4 < 128)return;
	if (z1 > 143 && z2 > 143 && z3 > 143 && z4 > 143) {
		//simple copy
		int ofst = int(ScreenPtr) + x + y*ScrWidth;
		__asm {
			push	esi
			push	edi
			pushf
			cld
			mov		esi, Wave
			mov		edi, ofst
			mov		cx, 0x0808
			mov		edx, ScrWidth
			lpp2 : xor		ebx, ebx
				   mov		bx, [esi + WaveLx * 2]
				   sub		bx, [esi - WaveLx * 2]
				   test	bx, 0x8000
				   jz		lpp3
				   or ebx, 0xFFFF0000
				   lpp3 : sar		ebx, ashift
						  //add		ebx,1024
						  add		ebx, ASHI
						  mov		eax, [Colors4 + ebx * 4]
						  stosd
						  mov[edi + edx - 2], eax
						  add		esi, 2
						  dec		cl
						  jnz		lpp2
						  add		edi, edx
						  add		esi, (WaveLx * 2) - 16
						  add		edi, edx
						  mov		cl, 8

						  sub		edi, 32
						  dec		ch
						  jnz		lpp2
						  popf
						  pop		edi
						  pop		esi
		};
	}
	else {
		;
		int ofst = int(ScreenPtr) + x + y*ScrWidth;
		z1 <<= 16;
		z2 <<= 16;
		z3 <<= 16;
		z4 <<= 16;
		int a = z1;
		int b = (z2 - z1) >> 5;
		int a0 = a;
		int c = (z3 - z1) >> 3;
		int d = (z1 + z4 - z2 - z3) >> 8;
		int cyc2 = 8;
		int cyc1;
		__asm {
			//curved clipping
			push	esi
			push	edi
			pushf
			cld
			mov		esi, Wave
			mov		edi, ofst
			mov		edx, ScrWidth
			lpp1C : mov		cyc1, 8
					lpp2C : xor		ebx, ebx
							mov		bx, [esi + WaveLx * 2]
							sub		bx, [esi - WaveLx * 2]
							test	bx, 0x8000
							jz		lpp3C
							or ebx, 0xFFFF0000
							lpp3C : sar		ebx, ashift
									//add		ebx,1024
									add		ebx, ASHI
									mov		eax, [Colors4 + ebx * 4]
									mov		ecx, eax
									sub		cl, 0xB0
									and cl, 0xF
									//transparency
									xor eax, eax
									mov		ebx, a
									cmp		ebx, 0x00800000
									jb		lpp4_1
									cmp		ebx, 0x00900000
									jae		lpp4_2
									//transparent
									mov		eax, ebx
									sub		eax, 0x00800000
									sar		eax, 12
									and eax, 0xF0
									or al, cl
									mov		ah, [edi]
									mov		al, [WaterCost + eax]
									stosb
									mov[edi + edx + 1], al
									jmp		lpp4_3
									lpp4_2 : mov[edi], ch
											 mov[edi + edx + 2], ch
											 lpp4_1 : inc		edi
													  lpp4_3 : add		ebx, b
															   add		ebx, 0x10000
															   cmp		ebx, 0x00800000
															   jb		lpp5_1
															   cmp		ebx, 0x00900000
															   jae		lpp5_2
															   //transparent
															   mov		eax, ebx
															   sub		eax, 0x00800000
															   sar		eax, 12
															   and eax, 0xF0
															   or al, cl
															   mov		ah, [edi]
															   mov		al, [WaterCost + eax]
															   stosb
															   mov[edi + edx + 1], al
															   jmp		lpp5_3
															   lpp5_2 : mov[edi], ch
																		mov[edi + edx + 2], ch
																		lpp5_1 : inc		edi
																				 lpp5_3 : add		ebx, b
																						  sub		ebx, 0x10000
																						  cmp		ebx, 0x00800000
																						  jb		lpp6_1
																						  cmp		ebx, 0x00900000
																						  jae		lpp6_2
																						  //transparent
																						  mov		eax, ebx
																						  sub		eax, 0x00800000
																						  sar		eax, 12
																						  and eax, 0xF0
																						  or al, cl
																						  mov		ah, [edi]
																						  mov		al, [WaterCost + eax]
																						  stosb
																						  mov[edi + edx + 1], al
																						  jmp		lpp6_3
																						  lpp6_2 : mov[edi], ch
																								   mov[edi + edx + 2], ch
																								   lpp6_1 : inc		edi
																											lpp6_3 : add		ebx, b
																													 add		ebx, 0x10000
																													 cmp		ebx, 0x00800000
																													 jb		lpp7_1
																													 cmp		ebx, 0x00900000
																													 jae		lpp7_2
																													 //transparent
																													 mov		eax, ebx
																													 sub		eax, 0x00800000
																													 sar		eax, 12
																													 and eax, 0xF0
																													 or al, cl
																													 mov		ah, [edi]
																													 mov		al, [WaterCost + eax]
																													 stosb
																													 mov[edi + edx + 1], al
																													 jmp		lpp7_3
																													 lpp7_2 : mov[edi], ch
																															  mov[edi + edx + 2], ch
																															  lpp7_1 : inc		edi
																																	   lpp7_3 : add		ebx, b
																																				sub		ebx, 0x10000
																																				mov		a, ebx
																																				add		esi, 2
																																				dec		cyc1
																																				jnz		lpp2C
																																				add		edi, edx
																																				add		esi, (WaveLx * 2) - 16
																																				add		edi, edx
																																				mov		ebx, a0
																																				add		ebx, c
																																				mov		a, ebx
																																				mov		a0, ebx
																																				mov		ebx, b
																																				add		ebx, d
																																				mov		b, ebx
																																				sub		edi, 32
																																				dec		cyc2
																																				jnz		lpp1C
																																				popf
																																				pop		edi
																																				pop		esi
		}
	}
}

#define CostCL 0x3D

void DrawCost2(int x, int y, short* Wave, int z1, int z2, int z3, int z4, int Msh, int Msh1) 
{
	int ASHI = 1024 + Msh;
	int ASHI1 = 1024 + Msh1;
	if (z1 < 128 && z2 < 128 && z3 < 128 && z4 < 128)
	{
		return;
	}

	if (z1 > 143 && z2 > 143 && z3 > 143 && z4 > 143) 
	{
		//simple copy
		int ofst = int(ScreenPtr) + x + y*ScrWidth;
		__asm {
			push	esi
			push	edi
			pushf
			cld
			mov		esi, Wave
			mov		edi, ofst
			mov		cx, 0x0408
			mov		edx, ScrWidth
			lpp2 : xor		ebx, ebx
				   mov		bx, [esi + WaveLx * 2]
				   sub		bx, [esi - WaveLx * 2]
				   test	bx, 0x8000
				   jz		lpp3
				   or ebx, 0xFFFF0000
				   lpp3 : sar		ebx, ashift
						  //add		ebx,1024
						  add		ebx, ASHI
						  mov		eax, [Colors4 + ebx * 4]
						  stosd
						  mov[edi + edx - 2], eax
						  add		esi, 2
						  dec		cl
						  jnz		lpp2
						  add		edi, edx
						  add		esi, (WaveLx * 2) - 16
						  add		edi, edx
						  mov		cl, 4
						  sub		edi, 32

						  lpp2_1:		xor		ebx, ebx
										mov		bx, [esi + WaveLx * 2]
										sub		bx, [esi - WaveLx * 2]
										test	bx, 0x8000
										jz		lpp3_1
										or ebx, 0xFFFF0000
										lpp3_1 : sar		ebx, ashift
												 //add		ebx,1024
												 add		ebx, ASHI
												 mov		eax, [Colors4 + ebx * 4]
												 stosd
												 mov[edi + edx - 2], eax
												 add		esi, 2
												 dec		cl
												 jnz		lpp2_1
												 //add		edi,edx
												 //add		esi,(WaveLx*2)-16
												 //add		edi,edx
												 mov		cl, 4

												 lpp2_2:		xor		ebx, ebx
																mov		bx, [esi + WaveLx * 2]
																sub		bx, [esi - WaveLx * 2]
																test	bx, 0x8000
																jz		lpp3_2
																or ebx, 0xFFFF0000
																lpp3_2 : sar		ebx, ashift
																		 //add		ebx,1024
																		 add		ebx, ASHI1
																		 mov		eax, [Colors4 + ebx * 4]
																		 stosd
																		 mov[edi + edx - 2], eax
																		 add		esi, 2
																		 dec		cl
																		 jnz		lpp2_2
																		 add		edi, edx
																		 add		esi, (WaveLx * 2) - 16
																		 add		edi, edx
																		 mov		cl, 8
																		 sub		edi, 32

																		 dec		ch
																		 jnz		lpp2
																		 popf
																		 pop		edi
																		 pop		esi
		}
	}
	else 
	{
		int ofst = int(ScreenPtr) + x + y*ScrWidth;
		z1 <<= 16;
		z2 <<= 16;
		z3 <<= 16;
		z4 <<= 16;
		int a = z1;
		int b = (z2 - z1) >> 5;
		int a0 = a;
		int c = (z3 - z1) >> 3;
		int d = (z1 + z4 - z2 - z3) >> 8;
		int cyc2 = 8;
		int cyc1;
		__asm 
		{
			//curved clipping
			push	esi
			push	edi
			pushf
			cld
			mov		esi, Wave
			mov		edi, ofst
			mov		edx, ScrWidth
			lpp1C : mov		cyc1, 8
					lpp2C : xor		ebx, ebx
							mov		bx, [esi + WaveLx * 2]
							sub		bx, [esi - WaveLx * 2]
							test	bx, 0x8000
							jz		lpp3C
							or ebx, 0xFFFF0000
							lpp3C : sar		ebx, ashift
									//add		ebx,1024
									add		ebx, ASHI
									mov		eax, [Colors4 + ebx * 4]
									mov		ecx, eax
									sub		cl, 0xB0
									and cl, 0xF
									//transparency
									xor eax, eax
									mov		ebx, a
									cmp		ebx, 0x00800000
									jb		lpp4_1
									cmp		ebx, 0x00900000
									jae		lpp4_2
									//transparent
									mov		eax, ebx
									sub		eax, 0x00800000
									sar		eax, 12
									and eax, 0xF0
									or al, cl
									mov		ah, CostCL//[edi]
									mov		al, [WaterCost + eax]
									stosb
									mov[edi + edx + 1], al
									jmp		lpp4_3
									lpp4_2 : mov[edi], ch
											 mov[edi + edx + 2], ch
											 lpp4_1 : inc		edi
													  lpp4_3 : add		ebx, b
															   add		ebx, 0x10000
															   cmp		ebx, 0x00800000
															   jb		lpp5_1
															   cmp		ebx, 0x00900000
															   jae		lpp5_2
															   //transparent
															   mov		eax, ebx
															   sub		eax, 0x00800000
															   sar		eax, 12
															   and eax, 0xF0
															   or al, cl
															   mov		ah, CostCL//[edi]
															   mov		al, [WaterCost + eax]
															   stosb
															   mov[edi + edx + 1], al
															   jmp		lpp5_3
															   lpp5_2 : mov[edi], ch
																		mov[edi + edx + 2], ch
																		lpp5_1 : inc		edi
																				 lpp5_3 : add		ebx, b
																						  sub		ebx, 0x10000
																						  cmp		ebx, 0x00800000
																						  jb		lpp6_1
																						  cmp		ebx, 0x00900000
																						  jae		lpp6_2
																						  //transparent
																						  mov		eax, ebx
																						  sub		eax, 0x00800000
																						  sar		eax, 12
																						  and eax, 0xF0
																						  or al, cl
																						  mov		ah, CostCL//[edi]
																						  mov		al, [WaterCost + eax]
																						  stosb
																						  mov[edi + edx + 1], al
																						  jmp		lpp6_3
																						  lpp6_2 : mov[edi], ch
																								   mov[edi + edx + 2], ch
																								   lpp6_1 : inc		edi
																											lpp6_3 : add		ebx, b
																													 add		ebx, 0x10000
																													 cmp		ebx, 0x00800000
																													 jb		lpp7_1
																													 cmp		ebx, 0x00900000
																													 jae		lpp7_2
																													 //transparent
																													 mov		eax, ebx
																													 sub		eax, 0x00800000
																													 sar		eax, 12
																													 and eax, 0xF0
																													 or al, cl
																													 mov		ah, CostCL//[edi]
																													 mov		al, [WaterCost + eax]
																													 stosb
																													 mov[edi + edx + 1], al
																													 jmp		lpp7_3
																													 lpp7_2 : mov[edi], ch
																															  mov[edi + edx + 2], ch
																															  lpp7_1 : inc		edi
																																	   lpp7_3 : add		ebx, b
																																				sub		ebx, 0x10000
																																				mov		a, ebx
																																				add		esi, 2
																																				dec		cyc1
																																				jnz		lpp2C
																																				add		edi, edx
																																				add		esi, (WaveLx * 2) - 16
																																				add		edi, edx
																																				mov		ebx, a0
																																				add		ebx, c
																																				mov		a, ebx
																																				mov		a0, ebx
																																				mov		ebx, b
																																				add		ebx, d
																																				mov		b, ebx
																																				sub		edi, 32
																																				dec		cyc2
																																				jnz		lpp1C
																																				popf
																																				pop		edi
																																				pop		esi
		}
	}
}

void DrawCost1(int x, int y, short* Wave, int z1, int z2, int z3, int z4, int Msh, int Msh1) 
{
	int ASHI = 1024 + Msh;
	int ASHI1 = 1024 + Msh1;
	if (z1 < 128 && z2 < 128 && z3 < 128 && z4 < 128)
	{
		return;
	}

	if (z1 > 143 && z2 > 143 && z3 > 143 && z4 > 143) 
	{
		//simple copy
		int ofst = int(ScreenPtr) + x + y*ScrWidth;
		__asm 
		{
			push	esi
			push	edi
			pushf
			cld
			mov		esi, Wave
			mov		edi, ofst
			mov		cx, 0x0408
			mov		edx, ScrWidth
			lpp2 : xor		ebx, ebx
				   mov		bx, [esi + WaveLx * 2]
				   sub		bx, [esi - WaveLx * 2]
				   test	bx, 0x8000
				   jz		lpp3
				   or ebx, 0xFFFF0000
				   lpp3 : sar		ebx, ashift
						  add		ebx, ASHI
						  mov		eax, [Colors4 + ebx * 4]
						  stosd
						  mov[edi + edx - 2], eax
						  add		esi, 2
						  dec		cl
						  jnz		lpp2
						  add		edi, edx
						  add		esi, (WaveLx * 2) - 16
						  add		edi, edx
						  mov		cl, 4
						  sub		edi, 32

						  lpp2_1:		xor		ebx, ebx
										mov		bx, [esi + WaveLx * 2]
										sub		bx, [esi - WaveLx * 2]
										test	bx, 0x8000
										jz		lpp3_1
										or ebx, 0xFFFF0000
										lpp3_1 : sar		ebx, ashift
												 add		ebx, ASHI
												 mov		eax, [Colors4 + ebx * 4]
												 stosd
												 mov[edi + edx - 2], eax
												 add		esi, 2
												 dec		cl
												 jnz		lpp2_1
												 mov		cl, 4

												 lpp2_2:		xor		ebx, ebx
																mov		bx, [esi + WaveLx * 2]
																sub		bx, [esi - WaveLx * 2]
																test	bx, 0x8000
																jz		lpp3_2
																or ebx, 0xFFFF0000
																lpp3_2 : sar		ebx, ashift
																		 add		ebx, ASHI1
																		 mov		eax, [Colors4 + ebx * 4]
																		 stosd
																		 mov[edi + edx - 2], eax
																		 add		esi, 2
																		 dec		cl
																		 jnz		lpp2_2
																		 add		edi, edx
																		 add		esi, (WaveLx * 2) - 16
																		 add		edi, edx
																		 mov		cl, 8
																		 sub		edi, 32

																		 dec		ch
																		 jnz		lpp2
																		 popf
																		 pop		edi
																		 pop		esi
		}
	}
	else 
	{
		int ofst = int(ScreenPtr) + x + y*ScrWidth;
		z1 <<= 16;
		z2 <<= 16;
		z3 <<= 16;
		z4 <<= 16;
		int a = z1;
		int b = (z2 - z1) >> 5;
		int a0 = a;
		int c = (z3 - z1) >> 3;
		int d = (z1 + z4 - z2 - z3) >> 8;
		int cyc2 = 8;
		int cyc1;
		__asm 
		{
			//curved clipping
			push	esi
			push	edi
			pushf
			cld
			mov		esi, Wave
			mov		edi, ofst
			mov		edx, ScrWidth
			lpp1C : mov		cyc1, 8
					lpp2C : xor		ebx, ebx
							mov		bx, [esi + WaveLx * 2]
							sub		bx, [esi - WaveLx * 2]
							test	bx, 0x8000
							jz		lpp3C
							or ebx, 0xFFFF0000
							lpp3C : sar		ebx, ashift
									//add		ebx,1024
									add		ebx, ASHI
									mov		eax, [Colors4 + ebx * 4]
									mov		ecx, eax
									sub		cl, 0xB0
									and cl, 0xF
									//transparency
									xor eax, eax
									mov		ebx, a
									cmp		ebx, 0x00800000
									jb		lpp4_1
									cmp		ebx, 0x00900000
									jae		lpp4_2
									//transparent
									mov		eax, ebx
									sub		eax, 0x00800000
									sar		eax, 12
									and eax, 0xF0
									or al, cl
									mov		ah, [edi]
									mov		al, [WaterCost + eax]
									stosb
									mov[edi + edx + 1], al
									jmp		lpp4_3
									lpp4_2 : mov[edi], ch
											 mov[edi + edx + 2], ch
											 lpp4_1 : inc		edi
													  lpp4_3 : add		ebx, b
															   add		ebx, 0x10000
															   cmp		ebx, 0x00800000
															   jb		lpp5_1
															   cmp		ebx, 0x00900000
															   jae		lpp5_2
															   //transparent
															   mov		eax, ebx
															   sub		eax, 0x00800000
															   sar		eax, 12
															   and eax, 0xF0
															   or al, cl
															   mov		ah, [edi]
															   mov		al, [WaterCost + eax]
															   stosb
															   mov[edi + edx + 1], al
															   jmp		lpp5_3
															   lpp5_2 : mov[edi], ch
																		mov[edi + edx + 2], ch
																		lpp5_1 : inc		edi
																				 lpp5_3 : add		ebx, b
																						  sub		ebx, 0x10000
																						  cmp		ebx, 0x00800000
																						  jb		lpp6_1
																						  cmp		ebx, 0x00900000
																						  jae		lpp6_2
																						  //transparent
																						  mov		eax, ebx
																						  sub		eax, 0x00800000
																						  sar		eax, 12
																						  and eax, 0xF0
																						  or al, cl
																						  mov		ah, [edi]
																						  mov		al, [WaterCost + eax]
																						  stosb
																						  mov[edi + edx + 1], al
																						  jmp		lpp6_3
																						  lpp6_2 : mov[edi], ch
																								   mov[edi + edx + 2], ch
																								   lpp6_1 : inc		edi
																											lpp6_3 : add		ebx, b
																													 add		ebx, 0x10000
																													 cmp		ebx, 0x00800000
																													 jb		lpp7_1
																													 cmp		ebx, 0x00900000
																													 jae		lpp7_2
																													 //transparent
																													 mov		eax, ebx
																													 sub		eax, 0x00800000
																													 sar		eax, 12
																													 and eax, 0xF0
																													 or al, cl
																													 mov		ah, [edi]
																													 mov		al, [WaterCost + eax]
																													 stosb
																													 mov[edi + edx + 1], al
																													 jmp		lpp7_3
																													 lpp7_2 : mov[edi], ch
																															  mov[edi + edx + 2], ch
																															  lpp7_1 : inc		edi
																																	   lpp7_3 : add		ebx, b
																																				sub		ebx, 0x10000
																																				mov		a, ebx
																																				add		esi, 2
																																				dec		cyc1
																																				jnz		lpp2C
																																				add		edi, edx
																																				add		esi, (WaveLx * 2) - 16
																																				add		edi, edx
																																				mov		ebx, a0
																																				add		ebx, c
																																				mov		a, ebx
																																				mov		a0, ebx
																																				mov		ebx, b
																																				add		ebx, d
																																				mov		b, ebx
																																				sub		edi, 32
																																				dec		cyc2
																																				jnz		lpp1C
																																				popf
																																				pop		edi
																																				pop		esi
		}
	}
}

inline void Wspot(short* wave, int x, int y, int h) 
{
	if (x > 1 && y > 1 && x < WaveLx - 2 && y < WaveLy - 2) 
	{
		wave[x + y*WaveLx] += h;
	}
}

void GenerateHi(short* Wave, int x, int y, int h) 
{
	if (x > 4 && y > 4 && x < WaveLx - 5 && y < WaveLy - 5) 
	{
		int ofst = x + y*WaveLx;
		Wave[ofst] += h;
		h = (h + h + h) >> 2;
		Wave[ofst + 1] += h;
		Wave[ofst - 1] += h;
		Wave[ofst + WaveLx] += h;
		Wave[ofst - WaveLx] += h;
	}
}

void GenerateWave(int x, int y, int vx, int vy, int h) 
{
	short* wave1 = nullptr;
	short* wave2 = nullptr;
	switch (CurStage) 
	{
	case 0:
		wave1 = Wave1;
		wave2 = Wave2;
		break;
	case 1:
		wave1 = Wave2;
		wave2 = Wave0;
		break;
	case 2:
		wave1 = Wave0;
		wave2 = Wave1;
		break;
	}

	GenerateHi(wave1, x - vx, y - vy, h);
	GenerateHi(wave2, x, y, h);
}

void DrawAllWater() 
{
	int cpos = mapx + mapy*MaxWX;
	int acpos = MaxWX - smaplx;
	short* Wave = nullptr;

	switch (CurStage) 
	{
	case 0:Wave = Wave1;
		break;
	case 1:Wave = Wave2;
		break;
	case 2:Wave = Wave0;
	}

	for (int i = 0; i < smaply; i++) 
	{
		for (int j = 0; j < smaplx; j++) 
		{
			int xx = j + mapx;
			int yy = i + mapy;
			int z1 = WaterDeep[cpos];
			int z2 = WaterDeep[cpos + 1];
			int z3 = WaterDeep[cpos + MaxWX];
			int z4 = WaterDeep[cpos + MaxWX + 1];
			if (z1 > 40 && z1 < 190) {
				z1 += CostHi[(xx & 31) + ((yy & 31) << 5)] >> 7;
			};
			if (z2 > 40 && z2 < 190) {
				z2 += CostHi[((xx + 1) & 31) + ((yy & 31) << 5)] >> 7;
			};
			if (z3 > 40 && z3 < 190) {
				z3 += CostHi[(xx & 31) + (((yy + 1) & 31) << 5)] >> 7;
			};
			if (z4 > 40 && z4 < 190) {
				z4 += CostHi[((xx + 1) & 31) + (((yy + 1) & 31) << 5)] >> 7;
			};
			int wx = (j + mapx) & 31;
			int wy = (i + mapy) & 31;
			int asha = WaterBright[cpos] >> 4;
			int asha1 = WaterBright[cpos + 1] >> 4;
			DrawCost1(j << 5, (i << 4) + smapy, Wave + (wx << 3) + 2 + ((wy << 3) + 2)*WaveLx, z1, z2, z3, z4, asha, asha1);
			cpos++;
		}
		cpos += acpos;
	}
}

void InitWatt()
{
	memset(WaterDeep, 0, WMPSIZE);
	InitCost();
}

void CopyWaves(short* wave, int SrcOfs, int DstOfs, int Lx, int Ly) 
{
	if (SrcOfs > DstOfs) 
	{
		//direct copy
		int addo = (WaveLx - Lx) << 1;
		int Lx1 = Lx >> 1;
		__asm 
		{
			push	esi
			push	edi
			pushf
			cld
			mov		esi, wave
			add		esi, SrcOfs
			mov		edi, wave
			add		edi, DstOfs
			mov		edx, Ly
			ppp1 : mov		ecx, Lx1
				   rep		movsd
				   add		esi, addo
				   add		edi, addo
				   dec		edx
				   jnz		ppp1
				   popf
				   pop		edi
				   pop		esi
		}
	}
	else 
	{
		//inverse copy
		int addo = (WaveLx - Lx) << 1;
		int add1 = (Lx + Ly*WaveLx - 1) << 1;
		int Lx1 = Lx >> 1;
		__asm 
		{
			push	esi
			push	edi
			pushf
			std
			mov		esi, wave
			add		esi, SrcOfs
			add		esi, add1
			mov		edi, wave
			add		edi, DstOfs
			add		edi, add1
			mov		edx, Ly
			ppp2 : mov		ecx, Lx1
				   rep		movsd
				   sub		esi, addo
				   sub		edi, addo
				   dec		edx
				   jnz		ppp2
				   popf
				   pop		edi
				   pop		esi
		}
	}
}

bool INSI(int x, int y) 
{
	if (x >= 0 && y >= 0 && x < smaplx && y < smaply)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void HandleShift() 
{
	if (mapx == OldMapx&&mapy == OldMapy)
	{
		return;
	}
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
	int Lx0 = 0;
	int Ly0 = 0;
	int Lx1 = 0;
	int Ly1 = 0;
	int Lx2 = 0;
	int Ly2 = 0;
	int xs = 0;
	int ys = 0;
	int Dx, Dy;
	bool Is0, Is1, Is2;
	if (mapx + smaplx <= OldMapx || OldMapx + smaplx <= mapx || mapy + smaply <= OldMapy || OldMapy + smaply <= mapy) 
	{
		Is0 = false;
		Is1 = true;
		Is2 = false;
		x1 = 0;
		y1 = 0;
		Lx1 = smaplx;
		Ly1 = smaply;
	}
	else 
	{
		if (mapx > OldMapx) 
		{
			Is0 = true;
			y0 = 0;
			Lx0 = mapx - OldMapx;
			x0 = smaplx - Lx0;
			Ly0 = smaply;
			if (mapy == OldMapy) 
			{
				Is1 = false;
				Is2 = true;
				x2 = 0;
				y2 = 0;
				Lx2 = x0;
				Ly2 = smaply;
				xs = Lx0;
				ys = 0;
			}
			else 
			{
				if (mapy < OldMapy) 
				{
					Is1 = true;
					Is2 = true;
					x1 = 0;
					y1 = 0;
					Lx1 = x0;
					Ly1 = OldMapy - mapy;
					x2 = 0;
					y2 = Ly1;
					Lx2 = Lx1;
					Ly2 = smaply - Ly1;
					xs = Lx0;
					ys = 0;
				}
				else 
				{
					Is1 = true;
					Is2 = true;
					x1 = 0;
					y1 = OldMapy + smaply - mapy;
					Lx1 = x0;
					Ly1 = mapy - OldMapy;
					x2 = 0;
					y2 = 0;
					Lx2 = Lx1;
					Ly2 = y1;
					xs = Lx0;
					ys = Ly1;
				}
			}
		}
		else 
		{
			Is0 = true;
			x0 = 0;
			y0 = 0;
			Lx0 = OldMapx - mapx;
			Ly0 = smaply;
			if (mapy == OldMapy) 
			{
				Is1 = false;
				Is2 = true;
				x2 = Lx0;
				y2 = 0;
				Lx2 = smaplx - Lx0;
				Ly2 = smaply;
				xs = 0;
				ys = 0;
			}
			else 
			{
				if (mapy < OldMapy) 
				{
					Is1 = true;
					Is2 = true;
					x1 = Lx0;
					y1 = 0;
					Ly1 = OldMapy - mapy;
					Lx2 = smaplx - Lx0;
					Ly2 = smaply - Ly1;
					Lx1 = Lx2;
					x2 = x1;
					y2 = Ly1;
					xs = 0;
					ys = 0;
				}
				else 
				{
					Is1 = true;
					Is2 = true;
					x1 = Lx0;
					Ly1 = mapy - OldMapy;
					y1 = smaply - Ly1;
					Lx2 = smaplx - Lx0;
					Ly2 = y1;
					Lx1 = Lx2;
					x2 = x1;
					y2 = 0;
					xs = 0;
					ys = Ly1;
				}
			}
		}
	}

	Dx = mapy - OldMapy;
	Dy = mapx - OldMapx;

	if (Is2)
	{
		x2++;
		y2++;
		xs++;
		ys++;
		x2 <<= 3;
		y2 <<= 3;
		xs <<= 3;
		ys <<= 3;
		int DstOfs = ((x2 + y2*WaveLx) << 1);
		int SrcOfs = ((xs + ys*WaveLx) << 1);

		CopyWaves(Wave0, SrcOfs, DstOfs, Lx2 << 3, Ly2 << 3);
		CopyWaves(Wave1, SrcOfs, DstOfs, Lx2 << 3, Ly2 << 3);
		CopyWaves(Wave2, SrcOfs, DstOfs, Lx2 << 3, Ly2 << 3);
	}

	if (Is1&&Lx1&&Ly1) 
	{
		ExtrapolateSurface(x1, y1, Lx1, Ly1);
	}

	if (Is0&&Lx0&&Ly0) 
	{
		ExtrapolateSurface(x0, y0, Lx0, Ly0);
	}

	OldMapx = mapx;
	OldMapy = mapy;
}

void ConditionalProcessWaves(short* Wave0, short* Wave1, short* Wave2, int x0, int y0, int Lx, int Ly) 
{
	int SX = MaxWX;
	byte* copos = WaterDeep + x0 + mapx + (y0 + mapy)*SX;
	int dpos = SX - Lx;
	for (int iy = 0; iy < Ly; iy++) 
	{
		for (int ix = 0; ix < Lx; ix++) 
		{
			if (copos[0] > 100 && copos[1] > 100 && copos[MaxWX] > 100 && copos[(MaxWX)+1] > 100) 
			{
				ProcessWaves(Wave0, Wave1, Wave2, ((x0 + ix) << 3) + 10, ((y0 + iy) << 3) + 10, 8, 8);
			}
			copos++;
		}
		copos += dpos;
	}
}

void FastProcess1(short* Wave0, short* Wave1, short* Wave2) 
{
	int cyc = WaveLy - 2;
	int	cyc1;
	__asm 
	{
		push	esi
		push	edi
		mov		esi, Wave0
		mov		edi, Wave1
		mov		ebx, Wave2
		mov		edx, (WaveLx + 1) * 2
		lpp1:	mov		cyc1, WaveLx - 2
				lpp2 : mov		ax, [edi + edx + 2]
					   add		ax, [edi + edx - 2]
					   add		ax, [edi + edx - WaveLx * 2]
					   add		ax, [edi + edx + WaveLx * 2]
					   mov		cx, [edi + edx]
					   sal     cx, 1
					   sub     ax, cx
					   sub     ax, cx
					   sar     ax, 6
					   add     ax, cx
					   sub     ax, [esi + edx]
					   mov     cx, ax
					   sar     cx, 16
					   mov[ebx + edx], ax
					   add     edx, 2
					   dec     cyc1
					   jnz     lpp2
					   add		edx, 4
					   dec		cyc
					   jnz		lpp1
					   pop		edi
					   pop		esi
	}
}

void FastProcess1_0(short* Wave0, short* Wave1, short* Wave2) 
{
	int cyc = (WaveLy - 2) >> 1;
	int	cyc1;
	__asm 
	{
		push	esi
		push	edi
		mov		esi, Wave0
		mov		edi, Wave1
		mov		ebx, Wave2
		mov		edx, (WaveLx + 1) * 2
		lpp1:	mov		cyc1, WaveLx - 2
				lpp2 : mov		ax, [edi + edx + 2]
					   add		ax, [edi + edx - 2]
					   add		ax, [edi + edx - WaveLx * 2]
					   add		ax, [edi + edx + WaveLx * 2]
					   mov		cx, [edi + edx]
					   sal     cx, 1
					   sub     ax, cx
					   sub     ax, cx
					   sar     ax, 4
					   add     ax, cx
					   sub     ax, [esi + edx]
					   mov[ebx + edx], ax
					   add     edx, 2
					   dec     cyc1
					   jnz     lpp2
					   add		edx, 4
					   dec		cyc
					   jnz		lpp1
					   pop		edi
					   pop		esi
	}
}

void FastProcess1_1(short* Wave0, short* Wave1, short* Wave2) 
{
	int cyc = (WaveLy - 2) >> 1;
	int	cyc1;
	__asm 
	{
		push	esi
		push	edi
		mov		esi, Wave0
		mov		edi, Wave1
		mov		ebx, Wave2
		mov		edx, WaveLx*WaveLy + 2
		lpp1:	mov		cyc1, WaveLx - 2
				lpp2 : mov		ax, [edi + edx + 2]
					   add		ax, [edi + edx - 2]
					   add		ax, [edi + edx - WaveLx * 2]
					   add		ax, [edi + edx + WaveLx * 2]
					   mov		cx, [edi + edx]
					   sal     cx, 1
					   sub     ax, cx
					   sub     ax, cx
					   sar     ax, 4
					   add     ax, cx
					   sub     ax, [esi + edx]
					   mov[ebx + edx], ax
					   add     edx, 2
					   dec     cyc1
					   jnz     lpp2
					   add		edx, 4
					   dec		cyc
					   jnz		lpp1
					   pop		edi
					   pop		esi
	}
}

static int tttx;

void ProcessWaves(short* Wave0, short* Wave1, short* Wave2) 
{
	int ppp = WaveLx * 2;
	for (int yy = 4; yy < WaveLy; yy++) 
	{
		Wave0[ppp + 1] = Wave0[ppp + WaveLx - 3];
		Wave1[ppp + 1] = Wave1[ppp + WaveLx - 3];
		Wave0[ppp + WaveLx - 2] = Wave0[ppp + 2];
		Wave1[ppp + WaveLx - 2] = Wave1[ppp + 2];
		ppp += WaveLx;
	}

	memcpy(Wave0, Wave0 + WaveLx*(WaveLy - 4), WaveLx * 2);
	memcpy(Wave0 + WaveLx, Wave0 + WaveLx*(WaveLy - 3), WaveLx * 2);
	memcpy(Wave1, Wave1 + WaveLx*(WaveLy - 4), WaveLx * 2);
	memcpy(Wave1 + WaveLx, Wave1 + WaveLx*(WaveLy - 3), WaveLx * 2);
	memcpy(Wave0 + WaveLx*(WaveLy - 2), Wave0 + WaveLx * 2, WaveLx * 2);
	memcpy(Wave0 + WaveLx*(WaveLy - 1), Wave0 + WaveLx * 3, WaveLx * 2);
	memcpy(Wave1 + WaveLx*(WaveLy - 2), Wave1 + WaveLx * 2, WaveLx * 2);
	memcpy(Wave1 + WaveLx*(WaveLy - 1), Wave1 + WaveLx * 3, WaveLx * 2);

	FastProcess1_0(Wave0, Wave1, Wave2);

	FastProcess1_1(Wave0, Wave1, Wave2);

	tttx = !tttx;
}

void DisturbWater(short* Wave) 
{
	int ofst = int(Wave + ((1 + WaveLx) << 1));
	__asm {
		push	esi
		push	edi

		pop		esi
		pop		edi
	};
}

void CorrectLeftWaves();

void HandleWater() {
	int WLX = smaplx + 2;
	int WLY = smaply + 2;
	int han = tmtmt & 1;
	int w1 = 0;
	int w2 = WLY >> 2;
	int w3 = WLY >> 1;
	int w4 = w2 + w3;
	int w5 = WLY - 1;
	w2 = w3;
	w3 = w5;
	han = 2;

	switch (han) 
	{
	case 0:
		switch (CurStage) 
		{
		case 0:
			ConditionalProcessWaves(Wave0, Wave1, Wave2, 0, w1, WLX, w2 - w1);
			break;

		case 1:
			ConditionalProcessWaves(Wave1, Wave2, Wave0, 0, w1, WLX, w2 - w1);
			break;

		case 2:
			ConditionalProcessWaves(Wave2, Wave0, Wave1, 0, w1, WLX, w2 - w1);
			break;
		}
		break;

	case 1:
		switch (CurStage) 
		{
		case 0:
			ConditionalProcessWaves(Wave0, Wave1, Wave2, 0, w2, WLX, w3 - w2);
			CurStage = 1;
			break;

		case 1:
			ConditionalProcessWaves(Wave1, Wave2, Wave0, 0, w2, WLX, w3 - w2);
			CurStage = 2;
			break;

		case 2:
			ConditionalProcessWaves(Wave2, Wave0, Wave1, 0, w2, WLX, w3 - w2);
			CurStage = 0;
			break;
		}
		break;

	case 2:
		switch (CurStage) 
		{
		case 0:
			ProcessWaves(Wave0, Wave1, Wave2);
			if (!tttx)
				CurStage = 1;
			break;

		case 1:
			ProcessWaves(Wave1, Wave2, Wave0);
			if (!tttx)
				CurStage = 2;
			break;

		case 2:
			ProcessWaves(Wave2, Wave0, Wave1);
			if (!tttx)
				CurStage = 0;
			break;
		}
		break;
	}

	ProcessCost();

	DrawAllWater();

	CorrectLeftWaves();
}

//EDITOR FUNCTIONS
void SetWaterSpot(int x, int y, int r) 
{
	int dr = CostThickness << 4;
	if (!r)
		r = 1;
	SetGoodDeepSpot(x, y, r << 5, dr, 170);
}

void EraseWaterSpot(int x, int y, int r) 
{
	int dr = CostThickness << 4;
	ClearGoodDeepSpot(x, y, r << 5, dr, 170);
}

int Spx0, Spy0, SpLx, SpLy;
void SetupSpot() 
{
	Spx0 = mapx << 3;
	Spy0 = mapy << 3;
	SpLx = smaplx << 3;
	SpLy = smaply << 3;
}

extern short TSin[257];
extern short TCos[257];
void SpotByUnit(int x, int y, int r, byte dir) {
	int xx = (x >> 6) - Spx0;
	int yy = (y >> 6) - Spy0;
	if (xx < SpLx&&yy < SpLy) {
		int dx = (r*TCos[dir]) >> 8;
		int dy = (r*TSin[dir]) >> 8;
		int h = (rand() & 1023);
		SetPoint(Wave0, xx + dx + 8, yy + dy + 8, 2, h);
		SetPoint(Wave1, xx + dx + 8, yy + dy + 8, 2, h);
		SetPoint(Wave2, xx + dx + 8, yy + dy + 8, 2, h);
		h = (rand() & 2048) - 1024;
		SetPoint(Wave0, xx - dx + 8, yy - y + 8, 2, h);
		SetPoint(Wave1, xx - dx + 8, yy - y + 8, 2, h);
		SetPoint(Wave2, xx - dx + 8, yy - y + 8, 2, h);
	};
};
int mul3(int);
int m3(int x) {
	return x + x + x;
};
int GetZ(int z1, int z2, int z3, int z4) {
	return (m3(m3(z1) + z2 + z3) + z4) >> 4;
};
void CreateWaterLocking(int x, int y, int x1, int y1) {
	int ofst = x + (MaxWX)*y;
	int addofs = (MaxWX)+x - x1;
	for (int iy = x; iy < y1; iy++) {
		for (int ix = x; ix < x1; ix++) {
			int z1 = WaterDeep[ofst];
			int z2 = WaterDeep[ofst + 1];
			int z3 = WaterDeep[ofst + (MaxWX)];
			int z4 = WaterDeep[ofst + (MaxWX)+1];
			int D1 = GetZ(z1, z2, z3, z4);
			int D2 = GetZ(z2, z4, z1, z3);
			int D3 = GetZ(z3, z4, z1, z2);
			int D4 = GetZ(z4, z3, z2, z1);
			/*
			int Z1=WaterBright[ofst];
			int Z2=WaterBright[ofst+1];
			int Z3=WaterBright[ofst+(MaxWX)];
			int Z4=WaterBright[ofst+(MaxWX)+1];
			int B1=GetZ(Z1,Z2,Z3,Z4);
			int B2=GetZ(Z2,Z4,Z1,Z3);
			int B3=GetZ(Z3,Z4,Z1,Z2);
			int B4=GetZ(Z4,Z3,Z2,Z1);
			*/
			int xx = ix + ix;
			int yy = iy + iy;
			if (D1 > 130)MFIELDS[1].BClrPt(xx, yy); else MFIELDS[1].BSetPt(xx, yy);
			if (D2 > 130)MFIELDS[1].BClrPt(xx + 1, yy); else MFIELDS[1].BSetPt(xx + 1, yy);
			if (D3 > 130)MFIELDS[1].BClrPt(xx, yy + 1); else MFIELDS[1].BSetPt(xx, yy + 1);
			if (D4 > 130)MFIELDS[1].BClrPt(xx + 1, yy + 1); else MFIELDS[1].BSetPt(xx + 1, yy + 1);
			if (D1 > 128)MFIELDS->BSetPt(xx, yy);
			if (D2 > 128)MFIELDS->BSetPt(xx + 1, yy);
			if (D3 > 128)MFIELDS->BSetPt(xx, yy + 1);
			if (D4 > 128)MFIELDS->BSetPt(xx + 1, yy + 1);
			ofst++;
		};
		ofst += addofs;
	};
};
extern int RealLy;
void CorrectLeftWaves() {
	byte* Scr = ((byte*)ScreenPtr);
	int N = RealLy >> 1;
	for (int i = 0; i < N; i++) {
		byte c = Scr[0];
		if (c >= 0xB0 && c <= 0xBC) {
			Scr[ScrWidth] = Scr[0];
			Scr[ScrWidth + 1] = Scr[1];
		};
		Scr += ScrWidth + ScrWidth;
	};
};