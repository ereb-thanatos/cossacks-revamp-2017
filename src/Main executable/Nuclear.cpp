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
extern short TSin[257];
extern short TCos[257];
extern short TAtg[257];
extern int NUCLUSE[4];
class HugeExplosion {
public:
	bool Enabled;
	int x;
	int y;
	int Radius1;
	int Radius2;
	int r1;
	int r2;
	int vr1;
	int vr2;
	int NR1;
	int NR2;
	HugeExplosion();
	void CreateExpl(int x, int y, int r1, int r2, int time);
	void HandleExpl();
};
HugeExplosion::HugeExplosion() {
	Enabled = false;
};
void HugeExplosion::CreateExpl(int ex, int ey, int er1, int er2, int time) {
	x = ex << 7;
	y = ey << 7;
	Radius1 = er1;
	Radius2 = er2;
	NR1 = 5;
	Radius1 = er1;
	Radius2 = er2;
	r1 = 32;
	r2 = 64;
	vr1 = div(er1 - 32, time).quot;
	vr2 = div(er2 - 64, time).quot;
	Enabled = true;
};
void HugeExplosion::HandleExpl() {

};
//USAGE
static HugeExplosion HUGEX[64];
void AddHugExp(int ex, int ey, int er1, int er2, int time)
{
	int i;
	for (i = 0; HUGEX[i].Enabled&&i < 64; i++);

	if (i < 64)
	{
		HUGEX[i].CreateExpl(ex, ey, er1, er2, time);
	}
}

void HandleHExpls()
{
	for (int i = 0; i < 64; i++)HUGEX[i].HandleExpl();

}
