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
#include <math.h>

extern int RivNX;
extern int RivSH;

byte* RivDir = nullptr;
byte* RivVol = nullptr;

void InitRiv()
{
	memset(RivDir, 0, RivNX*RivNX);
	memset(RivVol, 0, RivNX*RivNX);
}

extern int RealLx;
extern int RealLy;

void CBar(int x, int y, int Lx, int Ly, byte c);

void xLine(int x, int y, int x1, int y1, byte c);

word GetDir(int dx, int dy);

bool RiverEditMode = 0;

extern bool EditMapMode;

void ProcessRivEditor() 
{
	if (!EditMapMode)
	{
		return;
	}

	if (RiverEditMode) 
	{
		int x0 = -((mapx % 4) << 5) - 128;
		int y0 = -((mapy % 4) << 4) - 64;
		int vx0 = (mapx >> 2) - 1;
		int vy0 = (mapy >> 2) - 1;
		int Nx = (smaplx >> 2) + 2;
		int Ny = (smaply >> 2) + 2;

		for (int ix = 0; ix < Nx; ix++)
		{
			Vline(x0 + (ix << 7), 0, RealLy, 0xFD);
		}

		for (int iy = 0; iy < Ny; iy++)
		{
			Hline(0, y0 + (iy << 6), RealLx, 0xFD);
		}

		for (int ix = 0; ix < Nx; ix++) 
		{
			for (int iy = 0; iy < Ny; iy++) 
			{
				int ofs = ix + vx0 + ((iy + vy0) << RivSH);
				if (RivVol[ofs]) 
				{
					byte dir = RivDir[ofs];
					int vol = RivVol[ofs];
					int dx = (int(TCos[dir])*vol) >> 8;
					int dy = (int(TSin[dir])*vol) >> 9;
					int xx = ((ix + vx0) << 7) - (mapx << 5) + 64;
					int yy = ((iy + vy0) << 6) - (mapy << 4) + 32;
					CBar(xx - 3, yy - 3, 6, 6, 0xFE);
					xLine(xx, yy, xx + dx, yy + dy, 0xFE);
				}
			}
		}

		if (Lpressed || Rpressed) 
		{
			int px = mouseX + (mapx << 5);
			int py = (mouseY << 1) + (mapy << 5);
			int nx = (px >> 7);
			int ny = (py >> 7);
			if (nx >= 0 && ny >= 0) 
			{
				px -= (nx << 7) + 64;
				py -= (ny << 7) + 64;
				byte dir = GetDir(px, py);
				int vol;

				if (Rpressed)
				{
					vol = 0;
				}
				else
				{
					vol = int(sqrt(px*px + py*py));
				}

				int ofs = nx + (ny << RivSH);
				RivVol[ofs] = vol;
				RivDir[ofs] = dir;
			}
		}
	}
}

int GetRiverDir(int x, int y) 
{
	x >>= 7;
	y >>= 7;
	if (x >= 0 && y >= 0) 
	{
		int ofs = x + (y << RivSH);
		if (RivVol[ofs]) 
		{
			return RivDir[ofs];
		}
	}
	return -1;
}
