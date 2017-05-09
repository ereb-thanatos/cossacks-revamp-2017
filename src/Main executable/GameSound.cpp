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

#include "3DSurf.h"
#include "CWave.h"
#include "CDirSnd.h"
int FieldDelay = 0;
extern int WarSound;
extern int WorkSound;
extern int OrderSound;
extern int MidiSound;
static bool SoundOK;
CDirSound* CDS;
char* SoundID[MaxSnd];
word SndTable[MaxSnd][16];
byte SnDanger[MaxSnd];
word NSnd[MaxSnd];
word NSounds;
int NoMineSound = -1;
void Errs(LPCSTR s)
{
	MessageBox(hwnd, s, "Sound loading failed...", MB_ICONWARNING | MB_OK);
	assert(false);
};
extern short randoma[8192];
int srpos;
int srando() {
	srpos++;
	srpos &= 8191;
	return randoma[srpos];
};
bool IsSound;
void normstr(char* str);
void NLine(GFILE* f);
void LoadSounds(char* fn) {
	char sss[128];
	char idn[128];
	int srpos = 0;
	int nsn, z;
	NSounds = 0;
	SoundOK = CDS->DirectSoundOK() != 0;
	GFILE* f1 = Gopen(fn, "rt");

	char ccpr[128];
	ccpr[0] = 0;
	if (f1 != INVALID_HANDLE_VALUE) {
		do {
		uuux:	int Extent = 0;
			z = Gscanf(f1, "%s", idn);
			if (idn[0] == '/') {
				NLine(f1);
				goto uuux;
			};
			if (idn[0] == '#')Extent = 1;
			z = Gscanf(f1, "%d", &nsn);
			NSnd[NSounds] = nsn;
			SoundID[NSounds] = new char[strlen(idn) + 1 - Extent];
			strcpy(SoundID[NSounds], idn + Extent);
			if (z) {
				NLine(f1);
				SnDanger[NSounds] = 0;
				for (int i = 0; i < nsn; i++) {
					int Vol = 0;
				uuu:			if (Extent) {
					z = Gscanf(f1, "%s%d", sss, &Vol);
				}
								else {
									z = Gscanf(f1, "%s", sss);
								};
								if (sss[0] == '/') {
									NLine(f1);
									goto uuu;
								};
								if (!strcmp(sss, "DANGER")) {
									SnDanger[NSounds] = 1;
									goto uuu;
								};
								normstr(sss);
								if (SoundOK) {
									if (!strcmp(ccpr, sss)) {
										SndTable[NSounds][i] = CDS->DuplicateSoundBuffer(CDS->m_currentBufferNum);
										CDS->SetLastVolume(Vol);
									}
									else {
										CWave CW(sss);

										int sdid;
										if (CW.WaveOK()) {
											sdid = CDS->CreateSoundBuffer(&CW);
											CDS->SetLastVolume(Vol);
											if (CDS->DirectSoundOK()) {
												CDS->CopyWaveToBuffer(&CW, sdid);
											}
											else {
												sprintf(idn, "Could not create sound buffer : %s", sss);
												Errs(idn);
											};
											SndTable[NSounds][i] = sdid;
											strcpy(ccpr, sss);
										}
										else {
											//sprintf(idn,"Could not load sound : %s",sss);
											//Errs(idn);
											nsn--;
											NSnd[NSounds]--;
											i--;
										};
									};
								};
				};
			};
			NSounds++;
		} while (z);
	}
	else {
		sprintf(sss, "Could not open sounds list : %s", fn);
		Errs(sss);
	};
	//NSounds=0;
	Gclose(f1);
	//if(!CDS->DirectSoundOK()){
	//	NSounds=0;
	//};
	//NoMineSound=GetSound("NOFREEMINES");
};
__declspec(dllexport)
void PlayEffect(int n, int pan, int vol) {
	if (!SoundOK)return;
	vol -= (100 - WarSound) * 40;
	if (n < NSounds) {
		if (SnDanger[n])FieldDelay = 400;
		if (NSnd[n] > 0) {
			int maxsnd = NSnd[n];
			int u = maxsnd;
			int nnn = (srando()*maxsnd) >> 15;
			bool sndmade = true;
			do {
				int sid = SndTable[n][nnn];
				bool poss = CDS->IsPlaying(sid);
				if (!poss) {
					CDS->SetVolume(sid, vol/*+CDS->Volume[sid]*/);
					CDS->SetPan(sid, pan);
					CDS->PlaySound(sid);
					sndmade = false;
				}
				else {
					u--;
					nnn++;
					if (nnn >= maxsnd)nnn = 0;
				};
			} while (sndmade&&u);
			/*
			if(sndmade&&srando()<200){
				int nnn=(srando()*maxsnd)>>15;
				CDS->SetVolume(SndTable[n][nnn],vol);
				CDS->SetPan(SndTable[n][nnn],pan);
				CDS->PlaySound(SndTable[n][nnn]);
			};
			*/
		};
	};
};
void PlayCoorEffect(int n, int x, int y, int pan, int vol) {
	if (!SoundOK)return;
	vol -= (100 - WarSound) * 40;
	if (n < NSounds) {
		if (SnDanger[n])FieldDelay = 400;
		if (NSnd[n] > 0) {
			int maxsnd = NSnd[n];
			int u = maxsnd;
			int nnn = (srando()*maxsnd) >> 15;
			bool sndmade = true;
			do {
				int sid = SndTable[n][nnn];
				bool poss = CDS->IsPlaying(sid);
				if (!poss) {
					CDS->SetVolume(sid, vol/*+CDS->Volume[sid]*/);
					CDS->SetPan(sid, pan);
					CDS->PlayCoorSound(sid, x, y);
					sndmade = false;
				}
				else {
					u--;
					nnn++;
					if (nnn >= maxsnd)nnn = 0;
				};
			} while (sndmade&&u);
			/*
			if(sndmade&&srando()<200){
				int nnn=(srando()*maxsnd)>>15;
				CDS->SetVolume(SndTable[n][nnn],vol);
				CDS->SetPan(SndTable[n][nnn],pan);
				CDS->PlaySound(SndTable[n][nnn]);
			};
			*/
		};
	};
};
void PlaySingleEffect(int n, int pan, int vol) {
	if (!SoundOK)return;
	vol -= (100 - WarSound) * 40;
	if (n < NSounds) {
		if (SnDanger[n])FieldDelay = 400;
		if (NSnd[n] > 0) {
			int maxsnd = NSnd[n];
			for (int i = 0; i < maxsnd; i++) {
				int sid = SndTable[n][i];
				if (CDS->IsPlaying(sid))return;
			};

			int u = maxsnd;
			int nnn = (srando()*maxsnd) >> 15;
			bool sndmade = true;
			do {
				int sid = SndTable[n][nnn];
				bool poss = CDS->IsPlaying(sid);
				if (!poss) {
					CDS->SetVolume(sid, vol);
					CDS->SetPan(sid, pan);
					CDS->PlaySound(sid);
					sndmade = false;
				}
				else {
					u--;
					nnn++;
					if (nnn >= maxsnd)nnn = 0;
				};
			} while (sndmade&&u);
			/*
			if(sndmade&&srando()<200){
				int nnn=(srando()*maxsnd)>>15;
				CDS->SetVolume(SndTable[n][nnn],vol);
				CDS->SetPan(SndTable[n][nnn],pan);
				CDS->PlaySound(SndTable[n][nnn]);
			};
			*/
		};
	};
};

int SMinX, SMaxX, LoMinX, LoMaxX, SMinY, SMaxY, LoMinY, LoMaxY, CenterY;
int CenterX;
void PrepareSound() {
	if (FieldDelay)FieldDelay--;
	SMinX = mapx << 5;
	SMaxX = (mapx + smaplx) << 5;
	CenterX = (SMinX + SMaxX) >> 1;
	LoMinX = SMinX - 1024;
	LoMaxX = SMaxX + 1024;

	SMinY = mapy << 4;
	SMaxY = (mapy + smaply) << 4;
	CenterY = (SMinY + SMaxY) >> 1;
	LoMinY = SMinY - 512;
	LoMaxY = SMaxY + 512;
};

extern int FogMode;
word GetFog(int x, int y);

void AddEffectV(int x, int y, int vx, int id) {
	if (!SoundOK)return;
	if (id < 0)return;
	if (x<LoMinX || x>LoMaxX || y<LoMinY || y>LoMaxY)return;
	//fog checking;
	if (FogMode && (!NATIONS[MyNation].Vision) && GetFog(x, y << 1) < 900)return;

	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	//int maxp=div(8000,smaplx).quot;
	int pan = (x - CenterX) << 1;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	if (x >= SMinX&&y >= SMinY&&x < SMaxX&&y < SMaxY) {
		PlayCoorEffect(id, x, vx, pan, 0);
	}
	else {
		PlayCoorEffect(id, x, vx, pan, -800);
	};
};
void AddEffect(int x, int y, int id) {
	AddEffectV(x, y, 0, id);
};
void AddUnlimitedEffect(int x, int y, int id) {
	if (!SoundOK)return;
	if (id < 0)return;
	//fog checking;
	if (GetFog(x, y << 1) < 900 && FogMode)return;

	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	//int maxp=div(8000,smaplx).quot;
	int pan = (x - CenterX) << 1;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	PlayEffect(id, pan, 0);
};
void AddSingleEffect(int x, int y, int id) {
	if (!SoundOK)return;
	if (id < 0)return;
	if (x<LoMinX || x>LoMaxX || y<LoMinY || y>LoMaxY)return;
	//fog checking;
	if (GetFog(x, y << 1) < 900 && FogMode)return;

	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	//int maxp=div(8000,smaplx).quot;
	int pan = (x - CenterX) << 1;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	if (x >= SMinX&&y >= SMinY&&x < SMaxX&&y < SMaxY) {
		PlaySingleEffect(id, pan, 0);
	}
	else {
		PlaySingleEffect(id, pan, -800);
	};
};
void AddWarEffect(int x, int y, int id) {
	if (!SoundOK)return;
	if (GetV_fmap(x, y) < 2000)return;
	if (id < 0)return;
	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	int maxp = div(8000, smaplx).quot;
	int pan = (x - (smaplx >> 1) - mapx)*maxp;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	if (x >= mapx&&y >= mapy&&x < mapx + smaplx&&y < mapy + smaply) {
		PlayEffect(id, pan, WarSound);
	}
	else {
		if (x > mx0&&y > my0&&x < mx1&&y < my1) {
			PlayEffect(id, pan, WarSound - 400);
		};
	};
};
void AddWorkEffect(int x, int y, int id) {
	if (!SoundOK)return;
	if (GetV_fmap(x, y) < 2000)return;
	if (id < 0)return;
	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	int maxp = div(8000, smaplx).quot;
	int pan = (x - (smaplx >> 1) - mapx)*maxp;
	if (pan < -4000)pan = -4000;
	if (pan > 4000)pan = 4000;
	//int pan=-9999;
	if (x >= mapx&&y >= mapy&&x < mapx + smaplx&&y < mapy + smaply) {
		PlayEffect(id, pan, WorkSound - 200);
	}
	else {
		if (x > mx0&&y > my0&&x < mx1&&y < my1) {
			PlayEffect(id, pan, WorkSound - 600);
		};
	};
}

void AddOrderEffect(int x, int y, int id) 
{
	if (!SoundOK)
	{
		return;
	}
	if (GetV_fmap(x >> 2, y >> 2) < 2000)
	{
		return;
	}

	if (id < 0)
	{
		return;
	}

	int mx0 = mapx - 8;
	int my0 = mapy - 8;
	int mx1 = mapx + smaplx + 16;
	int my1 = mapy + smaply + 16;
	int maxp = div(8000, smaplx).quot;
	int pan = (x - (smaplx / 2) - mapx)*maxp;

	if (pan < -4000)
	{
		pan = -4000;
	}

	if (pan > 4000)
	{
		pan = 4000;
	}

	if (x >= mapx && y >= mapy && x < mapx + smaplx && y < mapy + smaply) {
		PlayEffect(id, pan, OrderSound);
	}
	else 
	{
		if (x > mx0 && y > my0 && x < mx1 && y < my1) 
		{
			PlayEffect(id, pan, OrderSound - 400);
		}
	}
}
