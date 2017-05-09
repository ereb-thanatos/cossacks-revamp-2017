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
void ProcessBlobs();
class Blob {
public:
	short Lx;
	short Ly;
	short N;
	short H;
	byte* Data;
	Blob();
	~Blob();
	void Load(char* Name);
	void Show(int x, int y, int N);
};
Blob::Blob() {
	Data = NULL;
	Lx = 0;
	Ly = 0;
};
Blob::~Blob() {
	if (Data)free(Data);
};
void ErrM(char*);
void Blob::Load(char* Name) {
	ResFile f = RReset(Name);
	if (f == INVALID_HANDLE_VALUE) {
		char ccc[128];
		sprintf(ccc, "Could not load: %s", Name);
		ErrM(ccc);
		return;
	};
	RBlockRead(f, &Lx, 2);
	RBlockRead(f, &Ly, 2);
	RBlockRead(f, &N, 2);
	RBlockRead(f, &H, 2);
	Data = new byte[Lx*Ly*N];
	RBlockRead(f, Data, Lx*Ly*N);
	RClose(f);
};
byte TraceGrd[4096];
void ShowBlob(int x, int y, byte* Blob, int Lx, int Ly) {
	if (x > WindX1 || y > WindY1)return;
	byte* bof = Blob;
	int BLX = Lx;
	int BLY = Ly;
	if (x < WindX) {
		if (x + Lx <= WindX)return;
		bof -= x - WindX;
		BLX += x - WindX;
		x = 0;
	};
	if (y < WindY) {
		if (y + Ly <= WindY)return;
		bof -= (y - WindY)*Lx;
		BLY += y - WindY;
		y = 0;
	};
	if (x + Lx > WindX1 + 1) {
		BLX -= x + Lx - WindX1 - 1;
	};
	if (y + Ly > WindY1 + 1) {
		BLY -= y + Ly - WindY1 - 1;
	};
	if (BLX <= 0 || BLY <= 0) {
		//assert(BLX>0&&BLY>0);
		return;
	};
	int sof = int(ScreenPtr) + x + y*ScrWidth;
	int bufo = int(bof);
	int scadd = ScrWidth - BLX;
	int badd = Lx - BLX;
	if (BLX & 3) {
		__asm {
			push	esi
			push	edi
			mov		esi, bufo
			mov		edi, sof
			mov		ch, byte ptr BLY
			xor		eax, eax
			lab1 :
			mov		cl, byte ptr BLX
				lab2 :
			mov		ah, [esi]
				mov		al, [edi]
				mov		al, [TraceGrd + eax]
				mov[edi], al
				inc		esi
				inc		edi
				dec		cl
				jnz		lab2
				add		esi, badd
				add		edi, scadd
				dec		ch
				jnz		lab1
				pop		edi
				pop		esi
		};
	}
	else {
		BLX >>= 2;
		__asm {
			push	esi
			push	edi
			mov		esi, bufo
			mov		edi, sof
			mov		ch, byte ptr BLY
			xor		eax, eax
			lab1o :
			mov		cl, byte ptr BLX
				lab2o :
			mov		ebx, [esi]
				mov		edx, [edi]
				mov		ah, bl
				mov		al, dl
				shr		ebx, 8
				mov		al, [TraceGrd + eax]
				mov		dl, al
				mov		ah, bl
				ror		edx, 8
				mov		al, dl

				shr		ebx, 8
				mov		al, [TraceGrd + eax]
				mov		dl, al
				mov		ah, bl
				ror		edx, 8
				mov		al, dl

				add		edi, 4

				shr		ebx, 8
				mov		al, [TraceGrd + eax]
				mov		dl, al
				mov		ah, bl
				ror		edx, 8
				mov		al, dl

				add		esi, 4

				mov		al, [TraceGrd + eax]
				mov		dl, al
				ror		edx, 8
				dec		cl

				mov[edi - 4], edx

				jnz		lab2o
				add		esi, badd
				add		edi, scadd
				dec		ch
				jnz		lab1o
				pop		edi
				pop		esi
		};
	};
};
void Blob::Show(int x, int y, int m) {
	if (m >= N || m < 0)return;
	ShowBlob(x - (Lx >> 1), y - (Ly >> 1), Data + Lx*Ly*m, Lx, Ly);
};
//------------------------LOADING&TEST-----------------

Blob Blob1;
void LoadBlobs()
{
	ResFile f = RReset("wave.grd");
	RBlockRead(f, TraceGrd, 4096);
	RClose(f);
	Blob1.Load("Wave1.pix");
}

extern int tmtmt;
void TestBlob() {
	ProcessBlobs();
};
//-------------------Traces system--------------------
#define MaxBlob 1024
int BlobX[MaxBlob];
int BlobY[MaxBlob];
int BlobVx[MaxBlob];
int BlobVy[MaxBlob];
byte BlobTime[MaxBlob];
byte BlobVisible[MaxBlob];
byte BlobOpt[MaxBlob];
int NBlobs;
int CurBlob;
extern int time3;
void ProcessBlobs() {
	int tm = GetTickCount();
	if (!NBlobs)return;
	int MinX = (mapx << 9) - 32 * 16;
	int MinY = (mapy << 9) - 32 * 16;
	int MaxX = ((mapx + smaplx) << 9) + 32 * 16;
	int MaxY = ((mapy + smaply) << 9) + 32 * 16;
	memset(BlobVisible, 0, MaxBlob);
	int ddd = !(tmtmt & 3);
	__asm {
		push	esi
		push	edi
		mov		esi, NBlobs;
		xor		ecx, ecx
			mov		ebx, MinX
			mov		edx, MaxX
			mov		edi, MinY
			lpp1 : mov		al, [BlobTime + ecx]
			or al, al
			jnz		lpp2
			inc		ecx
			dec		esi
			jnz		lpp1
			jmp     final
			lpp2:	mov		eax, [BlobVx + ecx * 4]
			add[BlobX + ecx * 4], eax
			mov		eax, [BlobVy + ecx * 4]
			add[BlobY + ecx * 4], eax
			dec		byte ptr[BlobTime + ecx]
			mov		eax, [BlobX + ecx * 4]
			cmp		eax, ebx
			jl		NoShow
			cmp		eax, edx
			jg		NoShow
			mov		eax, [BlobY + ecx * 4]
			cmp		eax, edi
			jl		NoShow
			cmp		eax, MaxY
			jg		NoShow
			mov		byte ptr[BlobVisible + ecx], 1
			NoShow: inc		ecx
			dec		esi
			jnz		lpp1
			final:
		pop		edi
			pop		esi
	}

	MinX = mapx * 32;
	MinY = mapy * 16;

	for (int i = 0; i < NBlobs; i++)
	{
		if (BlobVisible[i])
		{
			int p = BlobTime[i];
			byte op = BlobOpt[i];
			if (p)
			{
				int n = Blob1.N;

				if (p < n)
				{
					p = n - p;
				}
				else
				{
					p -= n;
				}

				if (p < 0)
					p = 0;

				if (p >= n)
					p = n - 1;

				if (op)
				{
					p = 20 + (i % 18);
				}

				Blob1.Show((BlobX[i] >> 4) - MinX, (BlobY[i] >> 5) - MinY, p);
			}
		}
		else
		{
			BlobTime[i] = 0;
		}
	}

	time3 = GetTickCount() - tm;
}

short BDVX[256];
short BDVY[256];
extern short TSin[257];
extern short TCos[257];

void InitBlobs()
{
	for (int i = 0; i < 256; i++)
	{
		BDVX[i] = (TCos[i]) >> 4;
		BDVY[i] = (TSin[i]) >> 4;
	}
	NBlobs = 0;
	CurBlob = 0;
}

void AddBlob(int x, int y, byte Dir, bool dir2)
{
	int Cur;
	if (NBlobs < MaxBlob)
	{
		Cur = NBlobs;
		NBlobs++;
		CurBlob++;
		if (NBlobs == MaxBlob)
		{
			CurBlob = 0;
		}
	}
	else
	{
		__asm
		{
			mov		eax, CurBlob
			mov		ecx, MaxBlob
			sub		ecx, eax
			lpp1 : cmp		byte ptr[BlobTime + eax], 0
				   je		lpp2
				   inc		eax
				   dec		ecx
				   jnz		lpp1
				   mov		Cur, -1
				   jmp		lpp3
				   lpp2 : mov		Cur, eax
						  inc		eax
						  lpp3 : mov		CurBlob, eax
		}
	}

	if (CurBlob >= MaxBlob)
	{
		CurBlob = 0;
	}

	if (Cur == -1)
	{
		return;
	}

	BlobX[Cur] = x;
	BlobY[Cur] = y;
	BlobVx[Cur] = BDVX[Dir];
	BlobVy[Cur] = BDVY[Dir];

	if (dir2)
	{
		BlobTime[Cur] = (Blob1.N) << 1;
	}
	else
	{
		BlobTime[Cur] = (Blob1.N);
	}

	BlobOpt[Cur] = dir2;
}

void AddBlob(int x, int y, byte Dir)
{
	AddBlob(x, y, Dir, 0);
}

int srando();
extern int RealLx;
extern int RealLy;
int GetRiverDir(int x, int y);

void AddRandomBlobs()
{
	int x0 = (mapx << 5);
	int y0 = (mapy << 5);
	for (int i = 0; i < 80; i++)
	{
		int x = x0 + ((srando()*RealLx) >> 15);
		int y = y0 + ((srando()*RealLy) >> 14);
		int dir = GetRiverDir(x, y);
		if (dir != -1)
		{
			int ofst = (x >> 5) + (MAPSX >> 1)*(y >> 5);
			AddBlob(x << 4, y << 4, dir, 1);
		}
	}
}

extern int tmtmt;

void AddTrace(int x, int y, byte Dir)
{
	AddBlob(x, y, Dir + 64);
	AddBlob(x, y, Dir - 64);
}

int srando();

void CreateTrace(OneObject* OB)
{
	if (tmtmt & 3)
	{
		return;
	}

	NewMonster* NM = OB->newMons;
	byte ANGL = OB->RealDir - 64;

	int dz = NM->WaveDZ;
	int SIN = TSin[ANGL];
	int COS = TCos[ANGL];

	if (NM->NWaves)
	{
		int N = NM->NWaves;
		for (int i = 0; i < N; i++)
		{
			int x = NM->WavePoints[i + i];
			int y = NM->WavePoints[i + i + 1];
			int xx = (x*COS - y*SIN) >> 4;
			int yy = ((y*COS + x*SIN) >> 4) + (dz << 5);
			xx += srando() >> 6;
			yy += srando() >> 7;
			AddTrace(OB->RealX + xx, OB->RealY + yy, OB->RealDir);
		}
	}
}