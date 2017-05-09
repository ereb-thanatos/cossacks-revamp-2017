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
#include "Transport.h"
#include "3DBars.h"
#include "CDirSnd.h"
#include "NewAI.h"
#include "3DMapEd.h"
#include "TopoGraf.h"

#include "Fonts.h"
#include "Safety.h"
#include "3DGraph.h"
#include "Nature.h"
int NDOBJ;
word DOBJ[512];
word DOBJSN[512];
int DOBJLastTime = 0;
extern int DangLx;
extern int DangSH;
byte* DANGMAP;
word* DCHTIME;
void InitDANGER() {
	NDOBJ = 0;
	DOBJLastTime = 0;
	memset(DOBJ, 0, sizeof DOBJ);
	memset(DOBJSN, 0, sizeof DOBJSN);
	memset(DANGMAP, 0, sizeof DANGMAP);
	memset(DCHTIME, 0, sizeof DCHTIME);
};
byte OBJDANG[48] = { 0,0,0,0,0,0,0,0, 0,1,1,0,0,0,6,0,
				   0,0,0,0,0,0,0,0, 0,3,0,2,1,0,1,0,
				   0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
byte OBJDTYPE[48] = { 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,2,0,
				   0,0,0,0,0,0,0,0, 0,2,0,1,1,0,1,0,
				   0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
extern int tmtmt;
int CheckDamageAbility(OneObject* OB, int x, int y, int z, byte Nation, int Soft);
void CheckDOBJS() {
	if (tmtmt - DOBJLastTime > 256 + (rando() & 63)) {
		NDOBJ = 0;
		for (int i = 0; i < MAXOBJECT&&NDOBJ < 511; i++) {
			OneObject* OB = Group[i];
			if (OB&&OB->NNUM == 0 && !OB->Sdoxlo) {
				byte USE = OB->newMons->Usage;
				if (OBJDANG[USE]) {
					DOBJ[NDOBJ] = OB->Index;
					DOBJSN[NDOBJ] = OB->Serial;
					NDOBJ++;
				};
			};
		};
		DOBJLastTime = tmtmt;
	};
};
byte GetDangValue(int x, int y) {
	CheckDOBJS();
	int ofs = x + (y << DangSH);
	word tm = word(tmtmt);
	if (tm - DCHTIME[ofs] > 255) {
		int xx = (x << 7) + 64;
		int yy = (y << 7) + 64;
		int zz = GetHeight(xx, yy) + 32;
		int dam = 0;
		int wat = 0;
		DCHTIME[ofs] = tm - (rando() & 127);
		for (int i = 0; i < NDOBJ; i++) {
			OneObject* OB = Group[DOBJ[i]];
			if (OB&&OB->Serial == DOBJSN[i]) {
				int USE = OB->newMons->Usage;
				int w = OBJDTYPE[USE];
				int w1 = w;
				if (!OB->Selected)w1 = 0;
				if (CheckDamageAbility(OB, xx, yy, zz, OB->NNUM, 1 + w1) >= 0) {
					if (w)wat = 128;
					dam += OBJDANG[USE];
				};
			};
		};
		return DANGMAP[ofs] = dam | wat;
	};
	return DANGMAP[ofs];
};
