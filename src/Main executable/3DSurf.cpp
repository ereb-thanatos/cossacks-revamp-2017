//3D surface rendering
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
#include <crtdbg.h>

#define MediaMax 8
#define CompoMax 8
#define RepTileMax 4
#define MaxStackSize 32768

static int StackSize = 8192;

short HiMap[256][256];//Height of surface
word RefMap[256][256];//Reference to cell being rendered

byte RXCoor[MaxStackSize];
byte RYCoor[MaxStackSize];

static int Theta = 20;
static int Phi = 100;

int STHead;
int STTile;

static char* RendSurf = nullptr;

void InitRenderStack()
{
	memset(RXCoor, 0, sizeof RXCoor);
	memset(RYCoor, 0, sizeof RYCoor);
	memset(&RefMap[0][0], 255, sizeof RefMap);
	STHead = 0;
	STTile = 0;
}

void ReRenderMap()
{
	memset(&RefMap[0][0], 255, sizeof RefMap);
	STHead = 0;
	STTile = 0;
}

void SetStackSize(int STSize)
{
	StackSize = STSize;
}

short xGetHeight(int x, int y)
{
	int xx = x;
	int yy = y;
	if (xx < 0)xx = 0;
	if (yy < 0)yy = 0;
	if (xx > msx)xx = msx;
	if (yy > msy)yy = msy;
	return HiMap[y][x];
}

int LightDX;
int LightDY;
int LightDZ;
void ClearRender();

void SetLight(int Ldx, int Ldy, int Ldz)
{
	int Ab = sqrt(Ldx*Ldx + Ldy*Ldy + Ldz*Ldz);
	LightDX = div(Ldx << 8, Ab).quot;
	LightDY = div(Ldy << 8, Ab).quot;
	LightDZ = div(Ldz << 8, Ab).quot;

	ClearRender();
}

void SetAngLight(int Theta, int Phi)
{
	double teta = Theta*3.1415 / 180;
	double phi = Phi*3.1415 / 180;
	SetLight(100 * sin(teta)*cos(phi), 100 * sin(teta)*cos(phi), 100 * cos(teta));
}

void RotateTeta()
{
	Theta += 2;
	SetAngLight(Theta, Phi);
}

void RotateTetaI() 
{
	Theta -= 2;
	SetAngLight(Theta, Phi);
}

void RotatePhi()
{
	Phi += 2;
	SetAngLight(Theta, Phi);
}

void RotatePhiI()
{
	Phi -= 2;
	SetAngLight(Theta, Phi);
}

int GetLight(int x, int y)
{
	int dy = xGetHeight(x + 1, y) - xGetHeight(x - 1, y);
	int dx = xGetHeight(x, y + 1) - xGetHeight(x, y - 1);
	int lig = 16 + (div(dx*LightDX + dy*LightDY + 64 * LightDZ, sqrt(dx*dx + dy*dy + 64 * 64)).quot >> 4);
	if (lig < 2)lig = 2;
	if (lig > 31)lig = 31;
	return 32 - lig;
}

int mul3(int);
extern byte mtiles[16][4096];

void AddHeight(int x, int y, int dh)
{
	if (x >= 0 && y >= 0 && x <= msx && y <= msy)
	{
		HiMap[y][x] += dh;
	}
}

void CreateBlob(int x, int y, int h, int r)
{
	int x0 = x - (r << 1);
	int y0 = y - (r << 1);
	int x1 = x + (r << 1);
	int y1 = y + (r << 1);
	int r2 = r*r;

	for (int i = x0; i < x1; i++)
	{
		for (int j = y0; j < y1; j++)
		{
			AddHeight(i, j, h*exp(-double((x - i)*(x - i) + (y - j)*(y - j)) / r2));
		}
	}
}

void InitRenderMap()
{
	InitRenderStack();

	SetLight(0, 20, 30);

	memset(&HiMap[0][0], 0, sizeof HiMap);
}
