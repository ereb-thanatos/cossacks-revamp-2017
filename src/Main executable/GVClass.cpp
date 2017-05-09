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
#include "DrawForm.h"
void PreDrawGameProcess();
void PostDrawGameProcess();
SimpleDialog* DialogsSystem::addViewPort(int x, int y, int Lx, int Ly) {
	int i;
	for (i = 0; i < 512 && DSS[i] != NULL; i++);
	if (i < 512) {
		SimpleDialog* SD = new SimpleDialog;
		DSS[i] = SD;
		SD->x = x + BaseX;
		SD->y = y + BaseY;
		SD->x1 = SD->x + Lx;
		SD->y1 = SD->y + Ly;
		SD->Enabled = 1;
		SD->Parent = NULL;
		return SD;
	};
	return NULL;
};
extern int RealLx;
extern int RealLy;
bool MakeClipping(SimpleDialog* SD) {
	if (!SD->Enabled)return false;
	WindX = 0;
	WindY = 0;
	WindX1 = RealLx - 1;
	WindY1 = RealLx - 1;
	WindLx = RealLx;
	WindY1 = RealLy;
	IntersectWindows(SD->x, SD->y, SD->x1, SD->y1);
	return false;
};
SimpleDialog* DialogsSystem::addClipper(int x0, int y0, int x1, int y1) {
	int i;
	for (i = 0; i < 512 && DSS[i] != NULL; i++);
	if (i < 512) {
		SimpleDialog* SD = new SimpleDialog;
		DSS[i] = SD;
		SD->x = x0;
		SD->y = y0;
		SD->x1 = x1;
		SD->y1 = y1;
		SD->Enabled = 1;
		SD->Visible = 0;
		SD->Parent = NULL;
		SD->OnDraw = &MakeClipping;
		return SD;
	};
	return NULL;

};