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
void ErrM(char* s);
class WaveFrame {
public:
	bool Enabled;
	int x;
	int y;
	NewAnimation* WAnm;
	byte CurSprite;
	char vx;
	char vy;
	char CheckShiftX;
	char CheckShiftY;
	byte Direction;
};
#define MaxWFrames  512
WaveFrame WAVES[MaxWFrames];
int NWaves;
NewAnimation* WaveAnm[32];
#define LoCRange 0xB0
#define HiCRange  0xBB
int MinX;
int MinY;
int MaxX;
int MaxY;
byte WSET[256];
NewAnimation* GetNewAnimationByName(char* Name);
void InitWaves() {
	for (int i = 0; i < MaxWFrames; i++)WAVES[i].Enabled = false;
};
void LoadWaveAnimations() {
	ResFile ff = RReset("water.set");
	RBlockRead(ff, WSET, 256);
	RClose(ff);
	char gx[128];
	char ann[64];
	GFILE* f1 = Gopen("WaveList.lst", "r");
	
	if (f1)
	{
		Gscanf(f1, "%d", &NWaves);
		for (int j = 0; j < NWaves; j++)
		{
			Gscanf(f1, "%s", ann);
			WaveAnm[j] = GetNewAnimationByName(ann);
			if (!WaveAnm[j])
			{
				sprintf(gx, " WaveList.lst : Unknown animation (%d) : %s", j, ann);
				ErrM(gx);
			}
		}
		Gclose(f1);
	}
	else
	{
		ErrM("Could not load WaveList.lst");
	}
}

inline bool CheckColor(int x, int y)
{
	if (x >= 0 && y >= 0 && x <= MaxX && y <= MaxY)
	{
		byte c = ((byte*)(ScreenPtr))[smapx + x + (smapy + y)*SCRSizeX];
		return(0 != WSET[c]);
	}
	else
	{
		return 0;
	}
}

void ShowRLCItemBlue(int x, int y, lpRLCTable lprt, int n);
int mul3(int);
void ProcessWaveFrames()
{
	MaxX = (smaplx << Shifter) - 1;
	MaxY = (mul3(smaply) << (Shifter - 2)) - 1;
	int scx, scy;
	int dx = mapx << 5;
	int dy = mul3(mapy) << 3;
	int NFram = 0;
	for (int i = 0; i < MaxWFrames; i++)
	{
		WaveFrame* WF = &WAVES[i];
		if (WF->Enabled) {
			NFram++;
			scx = (WF->x >> 2) - dx;
			scy = (mul3(WF->y) >> 4) - dy;
			if (!(CheckColor(scx, scy) &&
				CheckColor(scx + WF->CheckShiftX, scy + WF->CheckShiftY) &&
				CheckColor(scx + WF->CheckShiftX, scy + WF->CheckShiftY))
				)
			{
				WF->vx = 0;
				WF->vy = 0;
			}
		}
	}
	//processing creation new waves
	int cfm = 0;
	int NAttempt = 60;
	do {
		//search for empty frame
		while (cfm < MaxWFrames&&WAVES[cfm].Enabled)cfm++;
		if (cfm < MaxWFrames) {
			int x1 = (rand()*MaxX) >> 15;
			int y1 = (rand()*MaxY) >> 15;
			if (CheckColor(x1, y1) &&
				CheckColor(x1 + 16, y1 + 16) &&
				CheckColor(x1 - 16, y1 - 16)) {
				//Creating wave
				WaveFrame* WF = &WAVES[cfm];
				WF->Enabled = true;
				WF->x = (dx + x1) << 2;
				WF->y = (dy + y1) << 2;
				WF->vx = (rand() & 1);
				WF->vy = -WF->vx;
				WF->CheckShiftX = 16;
				WF->CheckShiftY = -16;
				WF->Enabled = true;
				WF->CurSprite = 0;
				WF->WAnm = WaveAnm[(rand()*NWaves) >> 15];
				cfm++;
			};
		};
		NAttempt--;
	} while (NAttempt&&cfm < MaxWFrames);
	//Processing motion
	for (int i = 0; i < MaxWFrames; i++) {
		WaveFrame* WF = &WAVES[i];
		if (WF->Enabled) {
			WF->x += WF->vx;
			WF->y += WF->vy;
			scx = smapx + (WF->x >> 2) - dx;
			scy = smapy + (mul3(WF->y) >> 4) - dy;
			//NewFrame* NF=&WF->WAnm->Frames[WF->CurSprite];
			//ShowRLCItemBlue(scx+NF->dx,scy+NF->dy,&RMImages[NF->FileID],NF->SpriteID);
			WF->CurSprite++;
			if (WF->CurSprite >= WF->WAnm->NFrames)WF->Enabled = false;
		};
	};
};