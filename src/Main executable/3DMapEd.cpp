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
#include "VirtScreen.h"
#include "NewMon.h"
#include "Path.h"
#include "RealWater.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "3DMapEd.h"
#include "mode.h"
#include "Curve.h"
#include <crtdbg.h>
void CheckGP();
BlockBars LockBars;
BlockBars UnLockBars;
extern int TM_Angle0;
extern int TM_Angle1;
extern int TM_Height0;
extern int TM_Height1;
extern bool TexMapMod;
extern int TM_Tan0;
extern int TM_Tan1;
int Prop43(int y);
word GetTexture();
void SetTexture(int Vert, int nm);
//returns planar y from(screen coordinates (x,y)
extern int mul3(int);
void AddLockBar(word x, word y) {
	LockBars.Add(x, y);
	UnLockBars.Delete(x, y);
	BSetBar(x << 2, y << 2, 4);
};
void FastAddLockBar(word x, word y) {
	LockBars.FastAdd(x, y);
	//UnLockBars.Delete(x,y);
	//BSetBar(x<<2,y<<2,4);
};
void FastAddUnLockBar(word x, word y) {
	//LockBars.FastAdd(x,y);
	UnLockBars.FastAdd(x, y);
	//UnLockBars.Delete(x,y);
	//BSetBar(x<<2,y<<2,4);
};
void AddUnLockbar(word x, word y) {
	LockBars.Delete(x, y);
	UnLockBars.Add(x, y);
	BClrBar(x << 2, y << 2, 4);
};
void AddDefaultBar(word x, word y) {
	LockBars.Delete(x, y);
	UnLockBars.Delete(x, y);
};
int GetTerrAngle(int x, int y);
int GetPSY(int i) {
	return (mul3(GetTriY(i)) >> 2) - THMap[i];
};
int GetPLY(int i) {
	return (mul3(GetTriY(i)) >> 2);
};
int ConvScrY(int x, int y) {
	div_t uu = div(x, TriUnit + TriUnit);
	int vertex = uu.quot + VertInLine*(MaxTH - 1);
	int besty = -10000;
	int yy2 = 0;
	while (vertex >= 0) {
		if (vertex < MaxPointIndex - 1) {
			int xp1 = GetTriX(vertex);
			int yp1 = GetPSY(vertex);
			int xp2 = GetTriX(vertex + 1);
			int yp2 = GetPSY(vertex + 1);
			int yy1 = yp1 + div((yp2 - yp1)*(x - xp1), xp2 - xp1).quot;
			if (yy1 < y) {
				if (vertex + VertInLine < MaxPointIndex - 1) {
					yp1 = GetPLY(vertex);
					yp2 = GetPLY(vertex + 1);
					besty = yp1 + div((yp2 - yp1)*(x - xp1), xp2 - xp1).quot;
					if (yy1 == yy2)return besty;
					else return besty + div((TriUnit)*(y - yy1), yy2 - yy1).quot;
				}
				else return -10000;
			};
			vertex -= VertInLine;
			yy2 = yy1;
		}
		else return -10000;
	};
	return -10000;
};
struct SelectionRequest {
	int x, y, r;
	byte TileType;
};
int Norm(int dx, int dy) {
	return int(sqrt(dx*dx + dy*dy));
};
void MarkTriangleToDraw(int i) {
	if (i < 0 || i >= MaxTH*MaxTH * 2)return;
	SVSC.RefreshTriangle(i);

};
void MarkPointToDraw(int i) {
	if (i<0 || i>(MaxTH + 1)*MaxTH)return;
	div_t uu = div(i, VertInLine);
	int Sector = uu.quot*(MaxTH + MaxTH) + ((uu.rem >> 1) << 2);
	if (uu.rem & 1) {
		MarkTriangleToDraw(Sector - MaxTH - MaxTH);
		MarkTriangleToDraw(Sector - MaxTH - MaxTH + 1);
		MarkTriangleToDraw(Sector - MaxTH - MaxTH + 2);
		MarkTriangleToDraw(Sector - MaxTH - MaxTH + 3);
		MarkTriangleToDraw(Sector + 1);
		MarkTriangleToDraw(Sector + 2);
	}
	else {
		//Sector-=4;
		MarkTriangleToDraw(Sector);
		MarkTriangleToDraw(Sector + 1);
		MarkTriangleToDraw(Sector - 1);
		MarkTriangleToDraw(Sector - 2);
		MarkTriangleToDraw(Sector - MaxTH - MaxTH);
		MarkTriangleToDraw(Sector - MaxTH - MaxTH - 1);
	};
};
void MarkLineToDraw(int i) {
	div_t uu = div(i, 6);
	int LineType = uu.rem;
	int StartTri = uu.quot << 2;
	switch (LineType) {
	case 0:
		MarkTriangleToDraw(StartTri);
		MarkTriangleToDraw(StartTri - 1);
		break;
	case 1:
		MarkTriangleToDraw(StartTri + 1);
		MarkTriangleToDraw(StartTri + MaxTH + MaxTH);
		break;
	case 2:
		MarkTriangleToDraw(StartTri);
		MarkTriangleToDraw(StartTri + 1);
		break;
	case 3:
		MarkTriangleToDraw(StartTri + 1);
		MarkTriangleToDraw(StartTri + 2);
		break;
	case 4:
		MarkTriangleToDraw(StartTri + 2);
		MarkTriangleToDraw(StartTri + MaxTH + MaxTH + 3);
		break;
	case 5:
		MarkTriangleToDraw(StartTri + 2);
		MarkTriangleToDraw(StartTri + 3);
		break;
	};
};
//returns division of line by cicle in CurDiv
void GetDivPoint(int x1, int y1, int x2, int y2, byte* CurDiv, SelectionRequest* SR) {
	int rd = SR->r;
	int cdv = *CurDiv;
	int r0 = Norm(x1 - SR->x, y1 - SR->y);
	int r1 = Norm(x2 - SR->x, y2 - SR->y);
	if ((r0 > rd&&r1 > rd) || (r0 < rd&&r1 < rd)) {
		if (r0 < rd&&r1 < rd)*CurDiv = div(rand(), 11000).quot;
		return;
	};
	int xp0 = x1 + ((x2 - x1) >> 2);
	int yp0 = y1 + ((y2 - y1) >> 2);
	int xp1 = x1 + ((x2 - x1) >> 1);
	int yp1 = y1 + ((y2 - y1) >> 1);
	int xp2 = x2 - ((x2 - x1) >> 2);
	int yp2 = y2 - ((y2 - y1) >> 2);
	int rp[3];
	rp[0] = Norm(xp0 - SR->x, yp0 - SR->y);
	rp[1] = Norm(xp1 - SR->x, yp1 - SR->y);
	rp[2] = Norm(xp2 - SR->x, yp2 - SR->y);
	if (r0 < rd) {
		if (rp[cdv] > rd)return;
		if (rp[0] < rd)cdv = 0;
		if (rp[1] < rd)cdv = 1;
		if (rp[2] < rd)cdv = 2;
		*CurDiv = cdv;
	};
	if (r1 < rd) {
		if (rp[cdv] > rd)return;
		if (rp[2] < rd)cdv = 2;
		if (rp[1] < rd)cdv = 1;
		if (rp[0] < rd)cdv = 0;
		*CurDiv = cdv;
	};
};
void PutTexInPoint(int i, SelectionRequest* SR) {
	if (i<0 || i>MaxTH*MaxTH * 2)return;
	//definition coordinates and type of point 
	int xp, yp;
	div_t uu = div(i, VertInLine);
	if (uu.rem & 1) {
		yp = uu.quot*(TriUnit + TriUnit) - TriUnit;
	}
	else {
		yp = uu.quot*(TriUnit + TriUnit);
	};
	xp = uu.rem*(TriUnit + TriUnit);
	//yp=Prop43(yp);
	if (Norm(xp - SR->x, yp - SR->y) > SR->r)return;
	int vind[6];
	int Lines[6];
	int StartLine = (MaxSector * 6)*uu.quot + (uu.rem >> 1) * 6;
	for (int j = 0; j < 6; j++) {
		vind[j] = -1;
		Lines[j] = -1;
	};
	if (uu.rem & 1) {
		//odd (upper) point
		if (uu.quot) {
			vind[0] = i - VertInLine;
			vind[1] = i - VertInLine + 1;
			vind[5] = i - VertInLine - 1;
			Lines[0] = StartLine - SectInLine + 3;
			Lines[1] = StartLine - SectInLine + 5;
			Lines[5] = StartLine - SectInLine + 2;
		};
		if (uu.quot < MaxTH - 1) {
			vind[3] = i + VertInLine;
			Lines[3] = StartLine + 3;
		};
		vind[2] = i + 1;
		vind[4] = i - 1;
		Lines[2] = StartLine + 4;
		Lines[4] = StartLine + 1;
	}
	else {
		//not odd (lower) point
		if (uu.quot) {
			vind[0] = i - VertInLine;
			Lines[0] = StartLine - SectInLine;
		};
		if (uu.rem < VertInLine - 1) {
			vind[1] = i + 1;
			Lines[1] = StartLine + 1;
		};
		if (uu.rem) {
			vind[5] = i - 1;
			Lines[5] = StartLine - 2;
		};
		if (uu.quot < MaxTH - 1) {
			if (uu.rem < VertInLine - 1) {
				vind[2] = i + VertInLine + 1;
				Lines[2] = StartLine + 2;
			};
			if (uu.rem) {
				vind[4] = i + VertInLine - 1;
				Lines[4] = StartLine - 1;
			};
			vind[3] = i + VertInLine;
			Lines[3] = StartLine;
		};
	};
	//Processing vertices
	//if(TexMap[i]!=SR->TileType){
	for (int j = 0; j < 6; j++) {
		if (vind[j] > 0 && vind[j] < MaxPointIndex&&Lines[j] < MaxLineIndex) {
			if (j & 1)SetSectMap(Lines[j], 2);
			else SetSectMap(Lines[j], 0);
		};
	};
	SetTexture(i, GetTexture());//SR->TileType);
	//TexMap[i]=SR->TileType;
	MarkPointToDraw(i);
	//};
	for (int j = 0; j < 6; j++) {
		if (vind[j] > 0 && vind[j] < MaxPointIndex&&Lines[j] < MaxLineIndex) {
			if (j & 1) {
				byte sm = GetSectMap(Lines[j]);
				byte sm1 = sm;
				GetDivPoint(GetTriX(vind[j]), GetTriY(vind[j]), xp, yp, &sm, SR);
				if (sm != sm1) {
					int LI = Lines[j];
					if (LI >= 0 && LI < MaxLineIndex) {
						MarkLineToDraw(LI);
						if (SectMap)SectMap[LI] = sm;
					};
				};
			}
			else {
				int LI = Lines[j];
				if (LI >= 0 && LI < MaxLineIndex) {
					byte sm = SECTMAP(Lines[j]);
					byte sm1 = sm;
					GetDivPoint(xp, yp, GetTriX(vind[j]), GetTriY(vind[j]), &sm, SR);
					if (sm != sm1) {
						MarkLineToDraw(LI);
						if (SectMap)SectMap[LI] = sm;
					};
				};
			};
		};
	};
};
void SetTexturedRound(int x, int y, int rx, byte Tex) {
	int r = rx;
	int yy = y;
	if (Mode3D)yy = ConvScrY(x, y);
	yy = Prop43(yy);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(yy, TriUnit * 2).quot;
	SelectionRequest SR;
	SR.x = x;
	//if(Mode3D)yy=ConvScrY(x,y);
	if (yy < 0)return;
	SR.y = yy;
	SR.r = r;
	SR.TileType = Tex;
	r = (r >> 4) + 4;
	for (int tx = -r; tx < r; tx++)
		for (int ty = -r; ty < r; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0 && vy < MaxTH) {
				int vert = vx + vy*VertInLine;
				PutTexInPoint(vert, &SR);
			};
		};
};
extern byte darkfog[40960];
extern byte minimap[maxmap][maxmap];
void SetTexture(int Vert, int nm) {
	if (Vert < 0 || Vert >= MaxPointIndex)return;
	if (TexMapMod) {
		int Hi = THMap[Vert];
		if (Hi<TM_Height0 || Hi>TM_Height1)return;
		int xp, yp;
		div_t uu = div(Vert, VertInLine);
		if (uu.rem & 1) {
			yp = uu.quot*(TriUnit + TriUnit) - TriUnit;
		}
		else {
			yp = uu.quot*(TriUnit + TriUnit);
		};
		xp = uu.rem*(TriUnit + TriUnit);
		int ANG = GetTerrAngle(xp, yp);
		if (ANG<TM_Tan0 || ANG>TM_Tan1)return;

	};
	div_t uu = div(Vert, VertInLine);
	int ix = uu.rem;
	int iy = uu.quot;
	int Lit = GetLighting(Vert);
	byte c = TexColors[nm];
	c = darkfog[16384 + c + (Lit << 8)];
	if (TriUnit == 16) {
		int xx = ix >> ADDSH;
		int yy = iy >> ADDSH;
		if (xx >= 0 && xx < maxmap&&yy >= 0 && yy < maxmap)minimap[yy][xx] = c;
	}
	else {
		if (ix >= 0 && ix < maxmap&&iy >= 0 && iy < maxmap)minimap[iy][ix] = c;
	};
	TexMap[Vert] = nm;
	word tf = TexFlags[nm];
	/*
	if(tf&TEX_PLAIN)AddTHMap[Vert]=0;
	else if(tf&TEX_HARD)AddTHMap[Vert]=rand()&15;
	else AddTHMap[Vert]=rand()&7;
	*/
};
extern bool MiniMade;
extern int WMPSIZE;
extern int MaxWX;
extern int MaxWY;

extern byte* WaterDeep;//cell size=32x16
void CreateMiniMapPart(int x0, int y0, int x1, int y1)
{
	int msx2 = msx >> 1;
	int msy2 = msy >> 1;

	if (x0 < 0)
		x0 = 0;

	if (y0 < 0)
		y0 = 0;

	if (x1 >= msx2)
		x1 = msx2 - 1;

	if (y1 >= msy2)
		y1 = msy2 - 1;

	MiniMade = true;
	if (TriUnit == 16)
	{
		for (int ix = x0; ix <= x1; ix++)
		{
			for (int iy = y0; iy <= y1; iy++)
			{
				int Vert = VertInLine*(iy + iy) + ix + ix;
				int Lit = GetLighting(Vert);
				byte c = TexColors[GetVTex(Vert)];
				c = darkfog[16384 + c + (Lit << 8)];
				int ixx = (ix >> ADDSH) - 1;
				int iyy = (iy >> ADDSH) - 1;
				if (ixx >= 0 && ixx < maxmap && iyy >= 0 && iyy < maxmap)
				{
					minimap[iyy][ixx] = c;
				}
			}
		}
	}
	else
	{
		for (int ix = x0; ix <= x1; ix++)
		{
			for (int iy = y0; iy <= y1; iy++)
			{
				int Vert = VertInLine*iy + ix;
				int Lit = GetLighting(Vert);
				byte c = TexColors[GetVTex(Vert)];
				c = darkfog[16384 + c + (Lit << 8)];
				int ixx = ix >> ADDSH;
				int iyy = ix >> ADDSH;
				if (ixx >= 0 && ixx < maxmap && iyy >= 0 && iyy < maxmap)
				{
					minimap[iyy][ixx] = c;
				}
			}
		}
	}

	x0 <<= 1;
	y0 <<= 1;
	x1 <<= 1;
	y1 <<= 1;
	for (int i = x0; i <= x1; i += 2)
	{
		for (int j = y0; j <= y1; j += 2)
		{
			int ix = i >> (ADDSH);
			int iy = j >> (ADDSH);
			if (ix >= 0 && ix < maxmap && iy >= 0 && iy < maxmap)
			{
				if (WaterDeep[i + j*MaxWX] > 128)
				{
					minimap[iy][ix] = 0xB7;
					minimap[iy][ix] = 0xA4;
				}
			}
		}
	}
}

int IMGMAX;
int ImgLx;
int ImgLy;
byte* ImgDat;

int* PrpX1;
int* PrpMul;

void PutMPPoint(int x, int y, byte c)
{
	if (x < 0 || x >= IMGMAX || y < 0 || y >= IMGMAX)
		return;

	ImgDat[PrpX1[x] + PrpMul[y]] = c;
}

int srando();

void CreateMapPreview(byte* Data, int Lx, int Ly)
{
	int PX1[480 << 3];
	int PM1[480 << 3];
	IMGMAX = 240 << ADDSH;
	for (int i = 0; i < IMGMAX; i++)
	{
		PX1[i] = (i*Lx) / IMGMAX;
		PM1[i] = Lx*((i*Ly) / IMGMAX);
	}
	PrpX1 = PX1;
	PrpMul = PM1;

	ImgLx = Lx;
	ImgLy = Ly;
	ImgDat = Data;

	int msx2 = msx;
	int msy2 = msy;
	int stp = 1 << (ADDSH - 1);
	for (int ix = 0; ix < msx2; ix += stp)
	{
		for (int iy = 0; iy < msy2 + 4; iy += stp)
		{
			int Vert = VertInLine*(iy)+ix;
			int Lit = GetLighting(Vert);
			byte c = TexColors[GetVTex(Vert)];
			c = darkfog[16384 + c + (Lit << 8)];
			int dy = iy - (GetHeight(ix << 5, iy << 5) >> 4);
			PutMPPoint(ix, dy, c);
			PutMPPoint(ix, dy + 1, c);
			PutMPPoint(ix, dy + 2, c);
			PutMPPoint(ix, dy + 3, c);
			PutMPPoint(ix, dy + 4, c);
			PutMPPoint(ix, dy + 5, c);
		}
	}
	for (int i = 0; i < MaxSprt; i++)
	{
		if (Sprites[i].Enabled)
		{
			int x = Sprites[i].x;
			int y = Sprites[i].y;
			SprGroup* SGG = Sprites[i].SG;
			if (SGG == &TREES)
			{
				//PutMPPoint(x>>5,(y>>5)-(GetHeight(x,y)>>4),0x9B);
			}
			else
			{
				/* if(SGG==&STONES)*/
				PutMPPoint(x >> 5, (y >> 5) - (GetHeight(x, y) >> 4), 0x4E);
			}
		}
	}
	int stp1 = stp << 1;
	for (int i = 0; i < msx; i += stp)
	{
		for (int j = 0; j < msy; j += stp1)
		{
			int wd = WaterDeep[i + j*MaxWX];
			byte cc = 0;
			if (wd > 200)
			{
				cc = 0xB4;
			}
			else
			{
				if (wd > 190)
				{
					cc = 0xB5;
				}
				else
				{
					if (wd > 160)
					{
						cc = 0xB6;
					}
					else
					{
						if (wd > 128)
						{
							cc = 0xB7;
						}
					}
				}
			}

			if (cc)
			{
				PutMPPoint(i, j, cc);
				PutMPPoint(i + 1, j, cc);
				PutMPPoint(i, j + 1, cc);
				PutMPPoint(i + 1, j + 1, cc);
			}
		}
	}
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB && !OB->Sdoxlo)
		{
			int xx = OB->RealX >> 9;
			int yy = (OB->RealY >> 9) - (OB->RZ >> 5);
			byte c = 0xD0 + (OB->NNUM << 2);
			PutMPPoint(xx - 1, yy, c);
			PutMPPoint(xx + 1, yy, c);
			PutMPPoint(xx, yy - 1, c);
			PutMPPoint(xx, yy + 1, c);
			PutMPPoint(xx, yy, c);
		}
	}
}

extern byte* WaterBright;

void CreateMiniMap()
{
	int msx2 = msx >> ADDSH;
	int msy2 = msy >> ADDSH;
	MiniMade = true;
	if (TriUnit == 16)
	{
		for (int ix = 0; ix < msx2; ix++)
		{
			for (int iy = 0; iy < msy2; iy++)
			{
				int Vert = VertInLine*(iy << ADDSH) + (ix << ADDSH);
				int Lit = GetLighting(Vert);
				byte c = TexColors[GetVTex(Vert)];
				c = darkfog[16384 + c + (Lit << 8)];
				if (ix >= 0 && ix < maxmap && iy >= 0 && iy < maxmap)
				{
					minimap[iy][ix] = c;
				}
			}
		}
	}
	else
	{
		for (int ix = 0; ix < msx2; ix++)
		{
			for (int iy = 0; iy < msy2; iy++)
			{
				int Vert = VertInLine*iy + ix;
				int Lit = GetLighting(Vert);
				byte c = TexColors[GetVTex(Vert)];
				c = darkfog[16384 + c + (Lit << 8)];
				if (ix >= 0 && ix < maxmap && iy >= 0 && iy < maxmap)
				{
					minimap[iy][ix] = c;
				}
			}
		}
	}
	for (int i = 0; i < msx; i += 2)
	{
		for (int j = 0; j < msy; j += 2)
		{
			int ofs = i + j * MaxWX;
			if (WaterDeep[ofs] > 128)
			{
				//IMPORTANT: change water coloring in minimap for better visibility
				//current = black;// grey;//blue (orignial)
				int B = WaterBright[ofs];
				byte c = 0x00;// 0x6A;//0xB3
				if (B > (6 * 16 - 8))
				{//Lighter
					c = 0x00;// 0x6E;//0xB7
				}
				else
				{
					if (B > (5 * 16 - 8))
					{
						c = 0x01;// 0x6D;//0xB6
					}
					else
					{
						if (B > (3 * 16 - 8))
						{
							c = 0xE7;// 0x6C;//0xB5
						}
						else
						{
							if (B > (1 * 16 - 8))
							{//Darker
								c = 0xEF;// 0x6B;//0xB4
							}
						}
					}
				}
				minimap[j >> ADDSH][i >> ADDSH] = c;
			}
		}
	}

	for (int i = 0; i < MaxSprt; i++)
	{
		if (Sprites[i].Enabled)
		{
			int x = Sprites[i].x >> (5 + ADDSH);
			int y = Sprites[i].y >> (5 + ADDSH);
			SprGroup* SGG = Sprites[i].SG;
			if (SGG == &TREES)
			{
				//if(x>=0&&x<maxmap&&y>=0&&y<maxmap)
				//	minimap[y][x]=0x9B;
			}
			else/* if(SGG==&STONES)*/
			{
				if (x >= 0 && x < maxmap && y >= 0 && y < maxmap)
					minimap[y][x] = 0x4E;
			}
		}
	}
}

void CBar(int x, int y, int Lx, int Ly, byte c);
extern int tmtmt;
void ShowPen(int x, int y1, int r, byte ptc) {
	SetRLCWindow(smapx, smapy, smaplx << 5, mul3(smaply) << 3, ScrWidth);
	int y = Prop43(y1);
	int npt = r >> 1;
	int tm = GetTickCount()&(0xFFFF);
	double angle0 = double(tm) / 20 / r;
	for (int i = 0; i < npt; i++) {
		double angl = (i * 2 * 3.1415 / npt) + angle0;
		int xp = x + int(r*sin(angl));
		int yp = y + int(r*cos(angl));
		int h = 0;
		if (Mode3D)h = GetHeight(xp, yp);
		xp -= (mapx << 5);
		yp -= (mapy << 5);
		yp = mul3(yp) >> 2;
		CBar(smapx + xp - 1, smapy + yp - h - 1, 3, 3, ptc);
		Xbar(smapx + xp - 2, smapy + yp - h - 2, 5, 5, 0);

	};
};
void AddHi(int x, int y1, int r, int h) {
	int y = Prop43(y1);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit * 2).quot + 2;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				int xx = GetTriX(vert);
				int yy = GetTriY(vert);
				int dst = int(sqrt((xx - x)*(xx - x) + (yy - y)*(yy - y)));
				if (dst < r) {
					int dh = (h*(r - dst)) / r;
					SetHi(vert, GetHi(vert) + dh);
				};
				if (dst < r + TriUnit * 3)MarkPointToDraw(vert);
			};
		};
};
void AddHiPlanar(int x, int y1, int r, int h) {
	int y = Prop43(y1);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit * 2).quot + 2;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				int xx = GetTriX(vert);
				int yy = GetTriY(vert);
				int dst = int(sqrt((xx - x)*(xx - x) + (yy - y)*(yy - y)));
				if (dst < r) {
					SetHi(vert, GetHi(vert) + h);
				};
				if (dst < r + TriUnit * 3)MarkPointToDraw(vert);
			};
		};
};
void CreatePlane(int x, int y1, int r) {
	int y = Prop43(y1);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit * 2).quot + 2;
	int h = GetHeight(x, y);
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				int xx = GetTriX(vert);
				int yy = GetTriY(vert);
				int dst = int(sqrt((xx - x)*(xx - x) + (yy - y)*(yy - y)));
				if (dst < r) {
					SetHi(vert, h);
				};
				if (dst < r + TriUnit * 3)MarkPointToDraw(vert);
			};
		};
};
void PlanarHi(int x, int y1, int r) {
	int y = Prop43(y1);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit * 2).quot + 2;
	int h0 = GetHeight(x, y);
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				int xx = GetTriX(vert);
				int yy = GetTriY(vert);
				int dst = int(sqrt((xx - x)*(xx - x) + (yy - y)*(yy - y)));
				if (dst < r) {
					int h1 = GetHeight(xx, yy);
					int dh = h1 + div((h0 - h1)*(r - dst), r).quot;
					SetHi(vert, dh);
				};
				if (dst < r + TriUnit * 3)MarkPointToDraw(vert);
			};
		};
};
void AverageHi(int x, int y1, int r) {
	int y = Prop43(y1);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit * 2).quot + 2;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				int xx = GetTriX(vert);
				int yy = GetTriY(vert);
				int dst = int(sqrt((xx - x)*(xx - x) + (yy - y)*(yy - y)));
				if (dst < r) {
					int dh = (GetHeight(xx - 8, yy) +
						GetHeight(xx + 8, yy) +
						GetHeight(xx, yy - 8) +
						GetHeight(xx, yy + 8)) >> 2;
					SetHi(vert, dh);
				};
				if (dst < r + TriUnit * 3)MarkPointToDraw(vert);
			};
		};
};
void RandomHi(int x, int y1, int r, int ms, int fnd) {
	int y = Prop43(y1);
	int utx = div(x, TriUnit * 2).quot;
	int uty = div(y, TriUnit * 2).quot;
	int r1 = div(r, TriUnit * 2).quot + 2;
	for (int tx = -r1; tx < r1; tx++)
		for (int ty = -r1; ty < r1; ty++) {
			int vx = utx + tx;
			int vy = uty + ty;
			if (vx >= 0 && vx < VertInLine&&vy>0) {
				int vert = vx + vy*VertInLine;
				int xx = GetTriX(vert);
				int yy = GetTriY(vert);
				int dst = int(sqrt((xx - x)*(xx - x) + (yy - y)*(yy - y)));
				if (dst < r) {
					int dh = ((rando()&ms) << 1) - fnd + GetHeight(xx, yy);
					SetHi(vert, dh);
				};
				if (dst < r + TriUnit * 3)MarkPointToDraw(vert);
			};
		};
};
int iab(int i) {
	if (i < 0)return -i;
	else return i;
};
void BSetSQ(int x, int y, int Lx, int Ly);
void BClrSQ(int x, int y, int Lx, int Ly);
void ClearMaps();
struct VECTOR3D {
	int Dx;
	int Dy;
	int Dz;
};
void GetNormal(int x, int y, VECTOR3D* V3) {
	int Dz = 32 << 4;
	int Dx = (GetHeight(x - 16, y) - GetHeight(x + 16, y)) << 4;
	int Dy = (GetHeight(x, y - 16) - GetHeight(x, y + 16)) << 4;
	int norm = int(sqrt(Dz*Dz + Dx*Dx + Dy*Dy));
	V3->Dx = div(Dx << 8, norm).quot;
	V3->Dy = div(Dy << 8, norm).quot;
	V3->Dz = div(Dz << 8, norm).quot;
};
int GetALP(VECTOR3D* V) {
	int VXY = int(sqrt((V->Dx*V->Dx + V->Dy*V->Dy)));
	if (!V->Dz)return 10000;
	return div(VXY << 8, V->Dz).quot;
};
int GetTerrAngle(int x, int y) {
	VECTOR3D V;
	GetNormal(x, y, &V);
	return GetALP(&V);
};
int GetCoef(int i, int j) {
	return 64 - ((i + j + 1) << 4) + (i + i + 1)*(j + j + 1);
};
void CreateFishMap();
void SetLockMask(int x, int y, char* mask) {
	int pos = 0;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			if (mask[pos] == '*')BSetPt(x + j, y + i);
			else BClrPt(x + j, y + i);
			pos++;
		};
};
void CreateLandLocking(int TAlp, bool ForVision) {
	int mxx = msx >> 1;
	int myy = msy >> 1;
	for (int ix = 0; ix < mxx; ix++) {
		int ZLmin = 1000000;
		int ZRmin = 1000000;
		for (int iy = myy - 1; iy >= 0; iy--) {
			int x0 = ix << 6;
			int y0 = iy << 6;
			int ppx = ix << 2;
			int ppy = iy << 2;
			int ZZ = (iy << 5) - iy - iy;
			int z0 = GetUnitHeight(x0, y0);
			int z1 = GetUnitHeight(x0 + 64, y0);
			int z2 = GetUnitHeight(x0, y0 + 64);
			int z3 = GetUnitHeight(x0 + 64, y0 + 64);
			int ZL0 = ZZ - z0;
			int ZL1 = ZZ - z2 + 30;
			int ZR0 = ZZ - z1;
			int ZR1 = ZZ - z3 + 30;
			if (ZL1 < ZL0) {
				ZL0 += ZL1;
				ZL1 = ZL0 - ZL1;
				ZL0 -= ZL1;
			};
			if (ZR1 < ZR0) {
				ZR0 += ZR1;
				ZR1 = ZR0 - ZR1;
				ZR0 -= ZR1;
			};
			//if(ZL1>ZLmin||ZR1>ZRmin){
				//BSetBar(ppx,ppy,4);
			//}else{
			int za = (z0 + z1 + z2 + z3) >> 2;
			if (abs(z0 - za) >= 20 || abs(z1 - za) >= 20 || abs(z2 - za) >= 20 || abs(z3 - za) >= 16) {
				BSetBar(ppx, ppy, 4);
			}
			else BClrBar(ppx, ppy, 4);
			//};
			if (ZL0 < ZLmin)ZLmin = ZL0;
			if (ZR0 < ZRmin)ZRmin = ZR0;

		};
	};
	//CreateWaterLocking(0,0,(MaxWX)-1,(MaxWX)-1);
	int N = UnLockBars.NBars;
	BlockCell* BC = UnLockBars.BC;
	for (int i = 0; i < N; i++) {
		BClrBar(int(BC[i].x) << 2, int(BC[i].y) << 2, 4);
	};
	N = LockBars.NBars;
	BC = LockBars.BC;
	for (int i = 0; i < N; i++) {
		BSetBar(int(BC[i].x) << 2, int(BC[i].y) << 2, 4);
	};
	if (ForVision) {
		ClearTrianglesSystem();
		CreateTrianglesSystem();
	};
	MotionField* TMF = new MotionField;
	TMF->Allocate();
	//memcpy(TMF->MapH,MFIELDS->MapH,MAPSY*BMSX);
	memcpy(TMF->MapV, MFIELDS->MapV, MAPSY*BMSX);
	for (int ix = 1; ix < mxx; ix++)
		for (int iy = 1; iy < myy; iy++) {
			int ppx = ix << 2;
			int ppy = iy << 2;
			byte c = 0;
			if (TMF->CheckBar(ppx + 1, ppy + 1 - 4, 2, 2))c |= 1;
			if (TMF->CheckBar(ppx + 1 + 4, ppy + 1, 2, 2))c |= 2;
			if (TMF->CheckBar(ppx + 1, ppy + 1 + 4, 2, 2))c |= 4;
			if (TMF->CheckBar(ppx + 1 - 4, ppy + 1, 2, 2))c |= 8;
			if (!TMF->CheckBar(ppx + 1, ppy + 1, 2, 2)) {
				switch (c) {
				case 3:
					SetLockMask(ppx, ppy, "**** ***  **   *");
					break;
				case 6:
					SetLockMask(ppx, ppy, "   *  ** *******");
					break;
				case 12:
					SetLockMask(ppx, ppy, "*   **  *** ****");
					break;
				case 9:
					SetLockMask(ppx, ppy, "******* **  *   ");
					break;
				case 7:
				case 14:
				case 13:
				case 11:
					BSetBar(ppx, ppy, 4);
					break;
				};
			};
		};
	//memcpy(TMF,MFIELDS,sizeof MotionField);
	//memcpy(TMF->MapH,MFIELDS->MapH,MAPSY*BMSX);
	memcpy(TMF->MapV, MFIELDS->MapV, MAPSY*BMSX);
	for (int ix = 1; ix < mxx; ix++)
		for (int iy = 1; iy < myy; iy++) {
			int ppx = ix << 2;
			int ppy = iy << 2;
			byte c = 0;
			if (TMF->CheckBar(ppx + 1, ppy + 1 - 4, 2, 2))c |= 1;
			if (TMF->CheckBar(ppx + 1 + 4, ppy + 1, 2, 2))c |= 2;
			if (TMF->CheckBar(ppx + 1, ppy + 1 + 4, 2, 2))c |= 4;
			if (TMF->CheckBar(ppx + 1 - 4, ppy + 1, 2, 2))c |= 8;
			if (TMF->CheckBar(ppx + 1, ppy + 1, 2, 2)) {
				switch (c) {
				case 0:
					//BSetBar(ppx,ppy,0);
					break;
				case 3:
					SetLockMask(ppx, ppy, "**** ***  **   *");
					break;
				case 6:
					SetLockMask(ppx, ppy, "   *  ** *******");
					break;
				case 12:
					SetLockMask(ppx, ppy, "*   **  *** ****");
					break;
				case 9:
					SetLockMask(ppx, ppy, "******* **  *   ");
					break;
				};
			};
		};
	TMF->FreeAlloc();
	free(TMF);
};

void CreateMapLocking() {
	int maxx = msx << 1;
	int maxy = msy << 1;
	ClearMaps();
	//BSetBar(0,0,MAPSX);
	//BClrBar(4,4,(msx-2)<<1);
	/*
	for(int ix=0;ix<maxx;ix++)
		for(int iy=0;iy<maxy;iy++){
			int xx=(ix<<4)+8;
			int yy=(iy<<4)+8;
			int dh1=iab(GetHeight(xx+16,yy)-GetHeight(xx-16,yy));
			if(dh1>35){
				//BSetSQ(ix-2,iy-1,5,3);
				//BSetSQ(ix-1,iy-2,3,1);
				BSetSQ(ix-1,iy-1,3,3);
			}
			else{
				dh1=iab(GetHeight(xx,yy+16)-GetHeight(xx,yy-16));
				if(dh1>35){
					//BSetSQ(ix-2,iy-1,5,3);
					//BSetSQ(ix-1,iy-2,3,1);
					BSetSQ(ix-1,iy-1,3,3);


				};
			};
		};
	//tiles locking checking
	*/
	CreateLandLocking(120, true);
	CreateWaterLocking(0, 0, (MaxWX)-1, (MaxWX)-1);
	CreateFishMap();
	//Border setting
	int L1 = 32 << ADDSH;
	for (int i = 0; i < 2; i++) {
		MFIELDS[i].BSetSQ(0, 0, maxx, 4);
		MFIELDS[i].BSetSQ(0, maxy, maxx, L1);
		MFIELDS[i].BSetSQ(0, 4, 4, maxy - 4);
		MFIELDS[i].BSetSQ(maxx, 0, L1, 512 << ADDSH);
	};
	int mxx = msx >> 1;
	int myy = msy >> 1;
	for (int ix = 0; ix < mxx; ix++) {
		for (int iy = 1; iy < 25; iy++) {
			if ((iy << 5) - GetHeight((ix << 6) + 32, (iy << 6)) < 16) {
				MFIELDS[0].BSetBar(ix << 2, iy << 2, 4);
				MFIELDS[1].BSetBar(ix << 2, iy << 2, 4);
				//MFIELDS[2].BSetBar(ix<<2,iy<<2,4);
			};
		};
	};
};
void CreateUnitsLocking() {
	for (int i = 0; i < MAXOBJECT; i++) {
		OneObject* OB = Group[i];
		if (OB&&OB->NewBuilding) {
			int xx, yy;
			OB->GetCornerXY(&xx, &yy);
			NewMonster* NM = OB->newMons;
			int nn;
			byte* LockX;
			byte* LockY;
			if (OB->Stage < OB->Ref.General->MoreCharacter->ProduceStages&&NM->NBLockPt) {
				nn = NM->NBLockPt;
				LockX = NM->BLockX;
				LockY = NM->BLockY;
			}
			else {
				nn = NM->NLockPt;
				LockX = NM->LockX;
				LockY = NM->LockY;
			};
			for (int i = 0; i < nn; i++)BSetPt(xx + LockX[i], yy + LockY[i]);
		};
	};
	for (int i = 0; i < WSys.NClusters; i++) {
		WallCluster* WCL = WSys.WCL[i];
		if (WCL) {
			int ncl = WCL->NCells;
			for (int j = 0; j < ncl; j++) {
				if (WCL->Cells[j].Visible)WCL->Cells[j].CreateLocking(WCL);
			};
		};
	};
}

void CreateTotalLocking() {
	rando();
	CreateMapLocking();
	rando();
	if (!NAreas)CreateAreas();
	CreateUnitsLocking();
	CreateInfoMap();
};
void EraseAreas();
void CreateFastLocking() {
	EraseAreas();
	CreateMapLocking();
	CreateUnitsLocking();
	CreateInfoMap();
};

//---------------------Drawing curves-------------------
int CurveX[512];
int CurveY[512];
int NCurves = 0;
extern int LastMx;
extern int LastMy;
bool CheckCurve() {
	return NCurves != 0;
};
int PlatoHi = 128;
int SurfType = 1;
void SetTextureInCurve();
void SetPlatoInCurve();
void SetHiInRegion(byte H);
bool EnterVal(int * val, char* Message);
int EnterHi(int * val, int Type);
void SoftRegion();
void InterpolateCurve();
void SetRoadInCurve(int);
bool CheckCurveLinked(int x, int y) {
	if (NCurves > 1) {
		int L0 = 0;
		for (int i = 0; i < NCurves - 1; i++)L0 += Norma(CurveX[i + 1] - CurveX[i], CurveY[i + 1] - CurveY[i]);
		if (L0 < 30)return false;
		if (Norma(x - CurveX[0], y - CurveY[0]) < 10)return true;
		return false;
	}
	else return false;
};
void AddPointToCurve(int x, int y, bool Final, byte Type) {
	if (NCurves && !Final) {
		if (Norma(x - CurveX[NCurves - 1], y - CurveY[NCurves - 1]) < 4)return;
	};
	if (NCurves < 510) {
		CurveX[NCurves] = x;
		CurveY[NCurves] = y;
		NCurves++;
	};
	//if(Type&&NCurves>3&&!Final){
	//	InterpolateCurve();
	//};
	if (Final)
	{
		CurveX[NCurves] = CurveX[0];
		CurveY[NCurves] = CurveY[0];
		NCurves++;
		if (Type == 1)
		{
			SetTextureInCurve();
		}
		if (Type == 2)
		{
			int p = EnterHi(&PlatoHi, SurfType);
			if (p)
			{
				SurfType = p;
				if (p == 3)
				{
					SoftRegion();
				}
				else
				{
					if (p >= 4)
					{
						SetRoadInCurve(p);
					}
					else
					{
						SetPlatoInCurve();
					}
				}
			}
		}
		if (Type >= 127)
		{
			SetRoadInCurve(Type - 127);
		}
		NCurves = 0;
	}
}

void DrawCurves()
{
	if (!NCurves)
	{
		return;
	}
	int xp = 0;
	int yp = 0;
	int dx = mapx << 5;
	int dy = mapy << 5;
	int N = NCurves;
	if (LastMx > 0 && LastMx < msx << 5 && LastMy>0 && LastMy < msy << 5) {
		CurveX[NCurves] = LastMx;
		CurveY[NCurves] = LastMy;
		N++;
	};
	for (int i = 0; i < N; i++) {
		int x = CurveX[i] - dx;
		int y = ((CurveY[i] - dy) >> 1);
		if (Mode3D)y -= GetHeight(CurveX[i], CurveY[i]);
		if (i > 0) {
			DrawLine(xp, yp, x, y, clrYello);
		};
		xp = x;
		yp = y;
	};

};
void DrawCurves1() {
	int CurveXt[512];
	int CurveYt[512];
	if (!NCurves)return;

	int NC = NCurves;
	memcpy(CurveXt, CurveX, sizeof CurveX);
	memcpy(CurveYt, CurveY, sizeof CurveY);

	if (NCurves >= 3)InterpolateCurve();

	int xp = 0;
	int yp = 0;
	int dx = mapx << 5;
	int dy = mapy << 5;
	int N = NCurves;
	if (LastMx > 0 && LastMx < msx << 5 && LastMy>0 && LastMy < msy << 5) {
		CurveX[NCurves] = LastMx;
		CurveY[NCurves] = LastMy;
		N++;
	};
	for (int i = 0; i < N; i++) {
		int x = CurveX[i] - dx;
		int y = ((CurveY[i] - dy) >> 1);
		if (Mode3D)y -= GetHeight(CurveX[i], CurveY[i]);
		if (i > 0) {
			DrawLine(xp, yp, x, y, clrYello);
		};
		xp = x;
		yp = y;
	};

	NCurves = NC;
	memcpy(CurveX, CurveXt, sizeof CurveX);
	memcpy(CurveY, CurveYt, sizeof CurveY);

};
bool CheckPointInside(int x, int y) {
	int NCross = 0;
	for (int i = 1; i < NCurves; i++) {
		int x0 = CurveX[i - 1];
		int x1 = CurveX[i];
		int y0 = CurveY[i - 1];
		int y1 = CurveY[i];
		if (x1 < x0) {
			int t = x1;
			x1 = x0;
			x0 = t;
			t = y1;
			y1 = y0;
			y0 = t;
		};
		if (x0 <= x&&x1 > x && (y0 > y || y1 > y)) {
			if (x0 != x1) {
				int yy = y0 + ((y1 - y0)*(x - x0)) / (x1 - x0);
				if (yy > y)NCross++;
			};
		};
	};
	return NCross & 1;
};
int GetCurveDistance(int x, int y) {
	int NCross = 0;
	int rmin = 100000;
	for (int i = 1; i < NCurves; i++) {
		int x0 = CurveX[i - 1];
		int x1 = CurveX[i];
		int y0 = CurveY[i - 1];
		int y1 = CurveY[i];
		if (i == 0) {
			rmin = Norma(x - x0, y - y0);
		};
		int r = Norma(x - x1, y - y1);
		if (r < rmin) {
			rmin = r;
		};
		if (x1 < x0) {
			int t = x1;
			x1 = x0;
			x0 = t;
			t = y1;
			y1 = y0;
			y0 = t;
		};
		if (x0 <= x&&x1 > x && (y0 > y || y1 > y)) {
			if (x0 != x1) {
				int yy = y0 + ((y1 - y0)*(x - x0)) / (x1 - x0);
				if (yy > y)NCross++;
			};
		};
	};
	if (NCross & 1)return -rmin;
	else return rmin;
};
int GetCurveDistance1(int x, int y) {
	int NCross = 0;
	int rmin = 100000;
	for (int i = 1; i < NCurves; i++) {
		int x0 = CurveX[i - 1];
		int x1 = CurveX[i];
		int y0 = CurveY[i - 1];
		int y1 = CurveY[i];
		if (x1 < x0) {
			int t = x1;
			x1 = x0;
			x0 = t;
			t = y1;
			y1 = y0;
			y0 = t;
		};
		int Dx = x1 - x0;
		int Dy = y1 - y0;
		int D = Dx*Dx + Dy*Dy;
		if (D) {
			int N = (((x - x0)*Dx + (y - y0)*Dy) << 8) / D;
			if (N <= 256 && N >= 0) {
				int r = abs(int(Dy*(x - x0) - Dx*(y - y0))) / int(sqrt(D));
				if (r < rmin)rmin = r;
			};
		};
		Dx = x - x0;
		Dy = y - y0;
		int r1 = int(sqrt(Dx*Dx + Dy*Dy));
		if (r1 < rmin)rmin = r1;
		if (x0 <= x&&x1 > x && (y0 > y || y1 > y)) {
			if (x0 != x1) {
				int yy = y0 + ((y1 - y0)*(x - x0)) / (x1 - x0);
				if (yy > y)NCross++;
			};
		};
	};
	if (NCross & 1)return -rmin;
	else return rmin;
};
int GetCurveDistance0(int x, int y) {
	int NCross = 0;
	int rmin = 100000;
	for (int i = 1; i < NCurves - 1; i++) {
		int x0 = CurveX[i - 1];
		int x1 = CurveX[i];
		int y0 = CurveY[i - 1];
		int y1 = CurveY[i];
		if (x1 < x0) {
			int t = x1;
			x1 = x0;
			x0 = t;
			t = y1;
			y1 = y0;
			y0 = t;
		};
		int Dx = x1 - x0;
		int Dy = y1 - y0;
		int D = Dx*Dx + Dy*Dy;
		if (D) {
			int N = (((x - x0)*Dx + (y - y0)*Dy) << 8) / D;
			if (N <= 256 && N >= 0) {
				int r = abs(int(Dy*(x - x0) - Dx*(y - y0))) / int(sqrt(D));
				if (r < rmin)rmin = r;
			};
		};
		Dx = x - x0;
		Dy = y - y0;
		int r1 = int(sqrt(Dx*Dx + Dy*Dy));
		if (r1 < rmin)rmin = r1;
		if (x0 <= x&&x1 > x && (y0 > y || y1 > y)) {
			if (x0 != x1) {
				int yy = y0 + ((y1 - y0)*(x - x0)) / (x1 - x0);
				if (yy > y)NCross++;
			};
		};
	};
	if (NCross & 1)return -rmin;
	else return rmin;
};
//return value: 0..4096;-1-no crossing
int GetCrossProportion(int x0, int y0, int x1, int y1) {
	if (x0 == x1 || y0 == y1)return -1;
	for (int i = 1; i < NCurves; i++) {
		int xc0 = CurveX[i - 1];
		int xc1 = CurveX[i];
		int yc0 = CurveY[i - 1];
		int yc1 = CurveY[i];
		if (xc1 < xc0) {
			int t = xc1;
			xc1 = xc0;
			xc0 = t;
			t = yc1;
			yc1 = yc0;
			yc0 = t;
		};
		if ((x0 <= xc0&&xc1 > x0) || (xc0 <= x1&&xc1 > x1)) {
			int D1 = x1 - x0;
			int D2 = xc1 - xc0;
			int B1 = y1 - y0;
			int B2 = yc1 - yc0;
			int A1 = y0*x1 - x0*y1;
			int A2 = yc0*xc1 - xc0*yc1;
			int DET = B2*D1 - B1*D2;
			if (DET) {
				if (abs(D1) > abs(B1)) {
					int x = (A1*D2 - A2*D1) / DET;
					int T = ((x - x0) << 12) / (x1 - x0);
					if (T >= 0 && T <= 4096)return T;
				}
				else {
					int y = (A1*B2 - A2*B1) / DET;
					int T = ((y - y0) << 12) / (y1 - y0);
					if (T >= 0 && T <= 4096)return T;
				};
			};
		};
	};
	return -1;
};
void PutTexInPointWithCurve(int i) {
	if (i<0 || i>MaxTH*MaxTH * 2)return;
	//definition coordinates and type of point 
	int xp, yp;
	div_t uu = div(i, VertInLine);
	if (uu.rem & 1) {
		yp = uu.quot*(TriUnit + TriUnit) - TriUnit;
	}
	else {
		yp = uu.quot*(TriUnit + TriUnit);
	};
	xp = uu.rem*(TriUnit + TriUnit);
	//yp=Prop43(yp);
	if (!CheckPointInside(xp, yp))return;
	int vind[6];
	int Lines[6];
	int StartLine = (MaxSector * 6)*uu.quot + (uu.rem >> 1) * 6;
	for (int j = 0; j < 6; j++) {
		vind[j] = -1;
		Lines[j] = -1;
	};
	if (uu.rem & 1) {
		//odd (upper) point
		if (uu.quot) {
			vind[0] = i - VertInLine;
			vind[1] = i - VertInLine + 1;
			vind[5] = i - VertInLine - 1;
			Lines[0] = StartLine - SectInLine + 3;
			Lines[1] = StartLine - SectInLine + 5;
			Lines[5] = StartLine - SectInLine + 2;
		};
		if (uu.quot < MaxTH - 1) {
			vind[3] = i + VertInLine;
			Lines[3] = StartLine + 3;
		};
		vind[2] = i + 1;
		vind[4] = i - 1;
		Lines[2] = StartLine + 4;
		Lines[4] = StartLine + 1;
	}
	else {
		//not odd (lower) point
		if (uu.quot) {
			vind[0] = i - VertInLine;
			Lines[0] = StartLine - SectInLine;
		};
		if (uu.rem < VertInLine - 1) {
			vind[1] = i + 1;
			Lines[1] = StartLine + 1;
		};
		if (uu.rem) {
			vind[5] = i - 1;
			Lines[5] = StartLine - 2;
		};
		if (uu.quot < MaxTH - 1) {
			if (uu.rem < VertInLine - 1) {
				vind[2] = i + VertInLine + 1;
				Lines[2] = StartLine + 2;
			};
			if (uu.rem) {
				vind[4] = i + VertInLine - 1;
				Lines[4] = StartLine - 1;
			};
			vind[3] = i + VertInLine;
			Lines[3] = StartLine;
		};
	};
	//Processing vertices
	//if(TexMap[i]!=SR->TileType){
	for (int j = 0; j < 6; j++) {
		if (vind[j] > 0 && vind[j] < MaxPointIndex&&Lines[j] < MaxLineIndex) {
			int LI = Lines[j];
			if (LI >= 0 && LI < MaxLineIndex) {
				if (j & 1) {
					if (SectMap)SectMap[LI] = 2;
				}
				else {
					if (SectMap)SectMap[LI] = 0;
				};
			};
		};
	};
	SetTexture(i, GetTexture());//SR->TileType);
	//TexMap[i]=SR->TileType;
	MarkPointToDraw(i);
	//};
	for (int j = 0; j < 6; j++) {
		if (vind[j] > 0 && vind[j] < MaxPointIndex&&Lines[j] < MaxLineIndex) {
			if (j & 1) {
				int LI = Lines[j];
				if (LI >= 0 && LI < MaxLineIndex) {
					byte sm = SECTMAP(LI);
					byte sm1 = sm;
					int cp = GetCrossProportion(GetTriX(vind[j]), GetTriY(vind[j]), xp, yp);
					//int cp=GetCrossProportion(xp,yp,GetTriX(vind[j]),GetTriY(vind[j]));
					if (cp != -1) {
						sm = cp / 1366;
						if (sm != sm1) {
							MarkLineToDraw(Lines[j]);
							//							assert(sm<=2&&sm>=0);
							if (SectMap)SectMap[Lines[j]] = sm;
						};

					};
				};
			}
			else {
				int LI = Lines[j];
				if (LI >= 0 && LI < MaxLineIndex) {
					byte sm = SECTMAP(Lines[j]);
					byte sm1 = sm;
					int cp = GetCrossProportion(xp, yp, GetTriX(vind[j]), GetTriY(vind[j]));
					//int cp=GetCrossProportion(GetTriX(vind[j]),GetTriY(vind[j]),xp,yp);
					if (cp != -1) {
						sm = cp / 1366;
						if (sm != sm1) {
							MarkLineToDraw(Lines[j]);
							if (SectMap)SectMap[Lines[j]] = sm;
							//							assert(sm<=2&&sm>=0);
						};

					};
				};
			};
		};
	};
};
void SetSectionsInPoint(int i, int h, byte* VertHi) {
	if (i<0 || i>MaxTH*MaxTH * 2)return;
	int H0 = VertHi[i];
	if (H0 > h)return;
	//definition coordinates and type of point 
	div_t uu = div(i, VertInLine);
	int vind[6];
	int Lines[6];
	int StartLine = (MaxSector * 6)*uu.quot + (uu.rem >> 1) * 6;
	for (int j = 0; j < 6; j++) {
		vind[j] = -1;
		Lines[j] = -1;
	};
	if (uu.rem & 1) {
		//odd (upper) point
		if (uu.quot) {
			vind[0] = i - VertInLine;
			vind[1] = i - VertInLine + 1;
			vind[5] = i - VertInLine - 1;
			Lines[0] = StartLine - SectInLine + 3;
			Lines[1] = StartLine - SectInLine + 5;
			Lines[5] = StartLine - SectInLine + 2;
		};
		if (uu.quot < MaxTH - 1) {
			vind[3] = i + VertInLine;
			Lines[3] = StartLine + 3;
		};
		vind[2] = i + 1;
		vind[4] = i - 1;
		Lines[2] = StartLine + 4;
		Lines[4] = StartLine + 1;
	}
	else {
		//not odd (lower) point
		if (uu.quot) {
			vind[0] = i - VertInLine;
			Lines[0] = StartLine - SectInLine;
		};
		if (uu.rem < VertInLine - 1) {
			vind[1] = i + 1;
			Lines[1] = StartLine + 1;
		};
		if (uu.rem) {
			vind[5] = i - 1;
			Lines[5] = StartLine - 2;
		};
		if (uu.quot < MaxTH - 1) {
			if (uu.rem < VertInLine - 1) {
				vind[2] = i + VertInLine + 1;
				Lines[2] = StartLine + 2;
			};
			if (uu.rem) {
				vind[4] = i + VertInLine - 1;
				Lines[4] = StartLine - 1;
			};
			vind[3] = i + VertInLine;
			Lines[3] = StartLine;
		};
	};
	int H3 = (h - H0) * 3;
	for (int j = 0; j < 6; j++) {
		int vi = vind[j];
		int Li = Lines[j];
		if (vi > 0 && vi < MaxPointIndex&&Li >= 0 && Li < MaxLineIndex) {
			int H1 = VertHi[vi];
			if (H1 >= h&&H1 != H0) {
				int s = H3 / (H1 - H0);
				if (s >= 3)s = 2;
				if (j & 1) {
					if (SectMap)SectMap[Li] = 2 - s;
				}
				else {
					if (SectMap)SectMap[Li] = s;
				};
			};
		};
	};
};
void SetTextureInCurve() {
	int xmin = 100000;
	int ymin = 100000;
	int xmax = -1;
	int ymax = -1;
	for (int i = 0; i < NCurves; i++) {
		int x = CurveX[i];
		int y = CurveY[i];
		if (x < xmin)xmin = x;
		if (x > xmax)xmax = x;
		if (y < ymin)ymin = y;
		if (y > ymax)ymax = y;
	};

	int utx0 = div(xmin, TriUnit * 2).quot - 1;
	int uty0 = div(ymin, TriUnit * 2).quot - 1;
	int utx1 = div(xmax, TriUnit * 2).quot + 1;
	int uty1 = div(ymax, TriUnit * 2).quot + 1;

	if (utx0 < 0)utx0 = 0;
	if (uty0 < 0)uty0 = 0;

	if (utx1 >= VertInLine)utx0 = VertInLine - 1;
	if (uty1 >= MaxTH)uty0 = MaxTH - 1;

	for (int tx = utx0; tx <= utx1; tx++)
		for (int ty = uty0; ty <= uty1; ty++) {
			int vert = tx + ty*VertInLine;
			PutTexInPointWithCurve(vert);
		};
};
int FUN1(int r) {
	if (r > 64) {
		if (r < 128) {
			return (int(rand())*(128 - r)) >> 16;
		};
		r = 64;
	};
	if (r < -64)r = -64;
	//if(r<0)return 
	//else return ((64-r)*(64-r))>>4;
	return 512 - 12 * r + ((r*r*r) >> 10);
};
int FUN2(int r) {
	if (r > 64) {
		if (r < 128) {
			return (int(rand())*(128 - r)) >> 16;
		};
		r = 64;
	};
	if (r < -64)r = -64;
	return ((64 - r)*(64 - r)) >> 4;
};
typedef int FUNType(int);
void SetPlatoInCurve() {
	FUNType* FTP3;
	if (SurfType == 1)FTP3 = &FUN1;
	else if (SurfType == 2)FTP3 = &FUN2;
	else return;
	int xmin = 100000;
	int ymin = 100000;
	int xmax = -1;
	int ymax = -1;
	for (int i = 0; i < NCurves; i++) {
		int x = CurveX[i];
		int y = CurveY[i];
		if (x < xmin)xmin = x;
		if (x > xmax)xmax = x;
		if (y < ymin)ymin = y;
		if (y > ymax)ymax = y;
	};

	int utx0 = div(xmin, TriUnit * 2).quot - 5;
	int uty0 = div(ymin, TriUnit * 2).quot - 5;
	int utx1 = div(xmax, TriUnit * 2).quot + 5;
	int uty1 = div(ymax, TriUnit * 2).quot + 5;

	if (utx0 < 0)utx0 = 0;
	if (uty0 < 0)uty0 = 0;

	if (utx1 >= VertInLine)utx0 = VertInLine - 1;
	if (uty1 >= MaxTH)uty0 = MaxTH - 1;

	for (int tx = utx0; tx <= utx1; tx++)
		for (int ty = uty0; ty <= uty1; ty++) {
			int vert = tx + ty*VertInLine;
			int x = GetTriX(vert);
			int y = GetTriY(vert);
			int r = GetCurveDistance1(x, y);
			int A = (*FTP3)(r);
			/*
			if(r>128)A=0;
			else if(r<-128)A=1024;
			else A=(128-r)<<2;
			*/
			int H = THMap[vert];
			int H1 = (H*(1024 - A) + PlatoHi*A) >> 10;
			if (H1 != THMap[vert]) {
				THMap[vert] = H1;
				MarkPointToDraw(vert);
			};
		};
};
byte RTARR0[16] = { 81,81,81,11,4 ,5,6,0,0,0,0,0,0,0,0,0 };
byte RTARR1[16] = { 81,11,4 ,5 ,6 ,0,0,0,0,0,0,0,0,0,0,0 };
byte RTARR2[16] = { 11,4 ,5 ,6 ,0 ,0,0,0,0,0,0,0,0,0,0,0 };
byte RoadPrio[256] = {
	0x00,0x0C,0x0C,0x10,0x0D,0x0C,0x0B,0x10,0xFF,0xFF,//00
	0xFF,0x0E,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//10
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//20
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//30
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//40
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//50
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//60
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//70
	0xFF,0x10,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//80
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//90
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//100
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//110
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//120
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//130
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//140
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//150
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//160
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//170
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//180
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//190
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//200
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//210
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//220
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//230
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//240
	0xFF,0xFF,0xFF,0xFF,0xFF };                        //250


void SetRoadInCurve(int tp) {
	byte* RTARR;
	if (tp == 4)RTARR = RTARR0;
	else if (tp == 5)RTARR = RTARR1;
	else RTARR = RTARR2;
	int xmin = 100000;
	int ymin = 100000;
	int xmax = -1;
	int ymax = -1;
	for (int i = 0; i < NCurves; i++) {
		int x = CurveX[i];
		int y = CurveY[i];
		if (x < xmin)xmin = x;
		if (x > xmax)xmax = x;
		if (y < ymin)ymin = y;
		if (y > ymax)ymax = y;
	};

	int utx0 = div(xmin, TriUnit * 2).quot - 2;
	int uty0 = div(ymin, TriUnit * 2).quot - 2;
	int utx1 = div(xmax, TriUnit * 2).quot + 2;
	int uty1 = div(ymax, TriUnit * 2).quot + 2;

	if (utx0 < 0)utx0 = 0;
	if (uty0 < 0)uty0 = 0;

	if (utx1 >= VertInLine)utx1 = VertInLine - 1;
	if (uty1 >= MaxTH)uty1 = MaxTH - 1;

	for (int tx = utx0; tx <= utx1; tx++)
		for (int ty = uty0; ty <= uty1; ty++) {
			int vert = tx + ty*VertInLine;
			int x = GetTriX(vert);
			int y = GetTriY(vert);
			int r = abs(GetCurveDistance0(x, y));
			if (r < 128) {
				int tidx = r * 10 / PlatoHi;
				if (tidx < 0)tidx = 0;
				if (tidx > 14)tidx = 14;
				int tx = RTARR[tidx];
				int t0 = TexMap[vert];
				if (RoadPrio[tx] > RoadPrio[t0]) {
					TexMap[vert] = tx;
					MarkPointToDraw(vert);
				};
			};
		};
};
void SoftRegion() {
	int NVert = 0;
	int MaxVert = 0;
	int* Vert = NULL;
	int* VertHi = NULL;
	int xmin = 100000;
	int ymin = 100000;
	int xmax = -1;
	int ymax = -1;
	for (int i = 0; i < NCurves; i++) {
		int x = CurveX[i];
		int y = CurveY[i];
		if (x < xmin)xmin = x;
		if (x > xmax)xmax = x;
		if (y < ymin)ymin = y;
		if (y > ymax)ymax = y;
	};

	int utx0 = div(xmin, TriUnit * 2).quot - 1;
	int uty0 = div(ymin, TriUnit * 2).quot - 1;
	int utx1 = div(xmax, TriUnit * 2).quot + 1;
	int uty1 = div(ymax, TriUnit * 2).quot + 1;

	if (utx0 < 0)utx0 = 0;
	if (uty0 < 0)uty0 = 0;

	if (utx1 >= VertInLine)utx0 = VertInLine - 1;
	if (uty1 >= MaxTH)uty0 = MaxTH - 1;

	for (int tx = utx0; tx <= utx1; tx++)
		for (int ty = uty0; ty <= uty1; ty++) {
			int vert = tx + ty*VertInLine;
			int x = GetTriX(vert);
			int y = GetTriY(vert);
			if (CheckPointInside(x, y)) {
				if (NVert >= MaxVert) {
					MaxVert += 256;
					Vert = (int*)realloc(Vert, MaxVert * 4);
					VertHi = (int*)realloc(VertHi, MaxVert * 4);
				};
				Vert[NVert] = vert;
				NVert++;
			};
		};
	int NC = 0;
	int maxdif = 0;
	do {
		maxdif = 0;
		for (int i = 0; i < NVert; i++) {
			int vert = Vert[i];
			int Lin = vert / VertInLine;
			int Lpos = vert%VertInLine;
			if (Lin > 0 && Lpos > 0 && Lpos < VertInLine - 1 && Lin < MaxTH - 1) {
				if (Lpos & 1) {
					VertHi[i] = (THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
						THMap[vert + 1] + THMap[vert - VertInLine + 1] + THMap[vert - VertInLine - 1]) / 6;
				}
				else {
					VertHi[i] = (THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
						THMap[vert + 1] + THMap[vert + VertInLine + 1] + THMap[vert + VertInLine - 1]) / 6;
				};
			};
			//int x=GetTriX(vert);
			//int y=GetTriY(vert);
			//int hav=(GetHeight(x-32,y)+GetHeight(x+32,y)+GetHeight(x,y-32)+GetHeight(x,y+32))>>2;
			//assert(hav>=0);
			//VertHi[i]=hav;
		};
		for (int i = 0; i < NVert; i++) {
			int DH = abs(VertHi[i] - THMap[Vert[i]]);
			if (DH > maxdif)maxdif = DH;
			THMap[Vert[i]] = VertHi[i];
		};
		NC++;
	} while (NC < 80);
	for (int i = 0; i < NVert; i++) {
		MarkPointToDraw(Vert[i]);
	};
	if (NVert) {
		free(Vert);
		free(VertHi);
	};
};
void ClearCurve() {
	NCurves = 0;
};
void GetBezierVect(int* xi, int* yi, int np, int i, int* dx, int* dy) {
	int pidx = i - 1;
	if (pidx < 0)pidx = np - 2;
	int nidx = i + 1;
	if (nidx >= np)nidx = 1;
	*dx = (xi[nidx] - xi[pidx]);
	*dy = (yi[nidx] - yi[pidx]);

};
void Normalise(int* dx, int* dy, int N) {
	int NR = Norma(*dx, *dy);
	if (NR) {
		*dx = (N*(*dx)) / NR;
		*dy = (N*(*dy)) / NR;
	};
};
void InterpolateCurve() {
	int* CurX = new int[NCurves];
	int* CurY = new int[NCurves];
	int NCur = NCurves;
	NCurves = 0;
	for (int i = 0; i < NCur; i++) {
		CurX[i] = CurveX[i];
		CurY[i] = CurveY[i];
	};
	for (int i = 1; i < NCur; i++) {
		double px0 = CurX[i - 1];
		double py0 = CurY[i - 1];
		double px3 = CurX[i];
		double py3 = CurY[i];
		double px1, py1, px2, py2;
		int Nr = 2 * Norma(CurX[i] - CurX[i - 1], CurY[i] - CurY[i - 1]) / 5;
		int dx, dy;
		GetBezierVect(CurX, CurY, NCur, i - 1, &dx, &dy);
		Normalise(&dx, &dy, Nr);
		px1 = px0 + dx;
		py1 = py0 + dy;
		GetBezierVect(CurX, CurY, NCur, i, &dx, &dy);
		Normalise(&dx, &dy, Nr);
		px2 = px3 - dx;
		py2 = py3 - dy;
		int Np = int(sqrt((px1 - px0)*(px1 - px0) + (py1 - py0)*(py1 - py0))) / 80;
		if (Np > 1) {
			Np += 3;
			for (int j = 0; j < Np; j++) {
				double U = double(j) / Np;
				double B0 = (1 - U)*(1 - U)*(1 - U);
				double B1 = 3 * U*(1 - U)*(1 - U);
				double B2 = 3 * U*U*(1 - U);
				double B3 = U*U*U;
				int    x = int(B0*px0 + B1*px1 + B2*px2 + B3*px3);
				int    y = int(B0*py0 + B1*py1 + B2*py2 + B3*py3);
				AddPointToCurve(x, y, false, 0);
			};
		}
		else {
			AddPointToCurve(CurX[i - 1], CurY[i - 1], false, 0);
		};
		if (i == NCur - 1) {
			AddPointToCurve(CurX[i], CurY[i], false, 0);
		};
	};
	free(CurX);
	free(CurY);
};
//------------------Special functions for random generation and texturing---------------
void SetPlatoInCurve(byte* VertHi, byte* VertType, byte Type) {
	FUNType* FTP3;
	if (SurfType == 1)FTP3 = &FUN1;
	else if (SurfType == 2)FTP3 = &FUN2;
	else return;
	int xmin = 100000;
	int ymin = 100000;
	int xmax = -1;
	int ymax = -1;
	for (int i = 0; i < NCurves; i++) {
		int x = CurveX[i];
		int y = CurveY[i];
		if (x < xmin)xmin = x;
		if (x > xmax)xmax = x;
		if (y < ymin)ymin = y;
		if (y > ymax)ymax = y;
	};

	int utx0 = div(xmin, TriUnit * 2).quot - 5;
	int uty0 = div(ymin, TriUnit * 2).quot - 5;
	int utx1 = div(xmax, TriUnit * 2).quot + 5;
	int uty1 = div(ymax, TriUnit * 2).quot + 5;

	if (utx0 < 0)utx0 = 0;
	if (uty0 < 0)uty0 = 0;

	if (utx1 >= VertInLine)utx0 = VertInLine - 1;
	if (uty1 >= MaxTH)uty0 = MaxTH - 1;

	for (int tx = utx0; tx <= utx1; tx++)
		for (int ty = uty0; ty <= uty1; ty++) {
			int vert = tx + ty*VertInLine;
			int x = GetTriX(vert);
			int y = GetTriY(vert);
			int r = GetCurveDistance1(x, y);
			int A = (*FTP3)(r);
			int B = A >> 3;
			if (B > VertHi[vert])VertHi[vert] = B;
			if (B)VertType[vert] = Type;
			/*
			if(r>128)A=0;
			else if(r<-128)A=1024;
			else A=(128-r)<<2;
			*/
			int H = THMap[vert];
			int H1 = (H*(1024 - A) + PlatoHi*A) >> 10;
			if (H1 != THMap[vert]) {
				THMap[vert] = H1;
			};
		};
};
void SetHiInRegion(byte H) {
	/*
	int xmin=100000;
	int ymin=100000;
	int xmax=-1;
	int ymax=-1;
	for(int i=0;i<NCurves;i++){
		int x=CurveX[i];
		int y=CurveY[i];
		if(x<xmin)xmin=x;
		if(x>xmax)xmax=x;
		if(y<ymin)ymin=y;
		if(y>ymax)ymax=y;
	};
	if(xmin<xmax){
		xmin>>=4;
		ymin>>=4;
		xmax>>=4;
		ymax>>=4;
		for(int dx=xmin;dx<=xmax;dx++)
			for(int dy=ymin;dy<=ymax;dy++)
				if(CheckPointInside(dx<<4,dy<<4))SetL3Point(dx,dy,H);
	};
	*/
};
void SoftRegion(byte* MpVertHi) {
	int NVert = 0;
	int MaxVert = 0;
	int* Vert = NULL;
	int* VertHi = NULL;
	int* TmVertHi = NULL;
	int xmin = 100000;
	int ymin = 100000;
	int xmax = -1;
	int ymax = -1;
	for (int i = 0; i < NCurves; i++) {
		int x = CurveX[i];
		int y = CurveY[i];
		if (x < xmin)xmin = x;
		if (x > xmax)xmax = x;
		if (y < ymin)ymin = y;
		if (y > ymax)ymax = y;
	};

	int utx0 = div(xmin, TriUnit * 2).quot - 1;
	int uty0 = div(ymin, TriUnit * 2).quot - 1;
	int utx1 = div(xmax, TriUnit * 2).quot + 1;
	int uty1 = div(ymax, TriUnit * 2).quot + 1;

	if (utx0 < 0)utx0 = 0;
	if (uty0 < 0)uty0 = 0;

	if (utx1 >= VertInLine)utx0 = VertInLine - 1;
	if (uty1 >= MaxTH)uty0 = MaxTH - 1;

	for (int tx = utx0; tx <= utx1; tx++)
		for (int ty = uty0; ty <= uty1; ty++) {
			int vert = tx + ty*VertInLine;
			int x = GetTriX(vert);
			int y = GetTriY(vert);
			if (CheckPointInside(x, y)) {
				if (NVert >= MaxVert) {
					MaxVert += 256;
					Vert = (int*)realloc(Vert, MaxVert * 4);
					VertHi = (int*)realloc(VertHi, MaxVert * 4);
					TmVertHi = (int*)realloc(TmVertHi, MaxVert * 4);
				};
				Vert[NVert] = vert;
				NVert++;
			};
		};
	int NC = 0;
	int maxdif = 0;
	do {
		maxdif = 0;
		for (int i = 0; i < NVert; i++) {
			int vert = Vert[i];
			int Lin = vert / VertInLine;
			int Lpos = vert%VertInLine;
			if (Lin > 0 && Lpos > 0 && Lpos < VertInLine - 1 && Lin < MaxTH - 1) {
				if (Lpos & 1) {
					VertHi[i] = (THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
						THMap[vert + 1] + THMap[vert - VertInLine + 1] + THMap[vert - VertInLine - 1]) / 6;
					TmVertHi[i] = (MpVertHi[vert + VertInLine] + MpVertHi[vert - VertInLine] + MpVertHi[vert - 1] +
						MpVertHi[vert + 1] + MpVertHi[vert - VertInLine + 1] + MpVertHi[vert - VertInLine - 1]) / 6;
				}
				else {
					VertHi[i] = (THMap[vert + VertInLine] + THMap[vert - VertInLine] + THMap[vert - 1] +
						THMap[vert + 1] + THMap[vert + VertInLine + 1] + THMap[vert + VertInLine - 1]) / 6;
					TmVertHi[i] = (MpVertHi[vert + VertInLine] + MpVertHi[vert - VertInLine] + MpVertHi[vert - 1] +
						MpVertHi[vert + 1] + MpVertHi[vert + VertInLine + 1] + MpVertHi[vert + VertInLine - 1]) / 6;
				};
			};
			//int x=GetTriX(vert);
			//int y=GetTriY(vert);
			//int hav=(GetHeight(x-32,y)+GetHeight(x+32,y)+GetHeight(x,y-32)+GetHeight(x,y+32))>>2;
			//assert(hav>=0);
			//VertHi[i]=hav;
		};
		for (int i = 0; i < NVert; i++) {
			int DH = abs(VertHi[i] - THMap[Vert[i]]);
			if (DH > maxdif)maxdif = DH;
			THMap[Vert[i]] = VertHi[i];
			MpVertHi[Vert[i]] = TmVertHi[i];
		};
		NC++;
	} while (NC < 80);
	for (int i = 0; i < NVert; i++) {
		MarkPointToDraw(Vert[i]);
	};
	if (NVert) {
		free(Vert);
		free(VertHi);
		free(TmVertHi);
	};
};
const int R14[10] = { 0,1,2,2,3,4,4,5,5,6 };
int mrand();
void PaintAllMap(byte* VertHi, byte* VertType, PaintHills* PHL, int NTypes) {
	for (int i = 0; i < MaxPointIndex; i++) {
		word VType = VertType[i];
		if (VType > 0 && VType <= NTypes) {
			PaintHills* PH = PHL + VType - 1;
			int vx = i%VertInLine;
			int vy = i / VertInLine;
			if (vx > 0 && vx < VertInLine - 1 && vy>1 && vx < MaxTH - 1) {
				int H = VertHi[i];
				int HU = VertHi[i - VertInLine];
				int HD = VertHi[i + VertInLine];
				int HRU, HRD, HLU, HLD;
				if (vx & 1) {
					HRU = VertHi[i - VertInLine + 1];
					HLU = VertHi[i - VertInLine - 1];
					HRD = VertHi[i + 1];
					HLD = VertHi[i - 1];
				}
				else {
					HRD = VertHi[i + VertInLine + 1];
					HLD = VertHi[i + VertInLine - 1];
					HRU = VertHi[i + 1];
					HLU = VertHi[i - 1];
				};
				int HL = (HLU + HLD) >> 1;
				int HR = (HRU + HRD) >> 1;
				int DH = Norma(HR - HL, HU - HD);
				bool UNFND = true;
				int L = 0;
				for (int j = 0; j < PH->NLayers&&UNFND; j++) {
					if (H >= PH->TexStartHi[j] && H <= PH->TexEndHi[j] && DH >= PH->TexStartAng[j] && DH <= PH->TexEndAng[j]) {
						int ct = PH->CondType[j];
						L = j;
						if (ct) {
							int r, rd;
							int xx = GetTriX(i) >> 4;
							int yy = GetTriY(i) >> 4;
							int r0 = PH->Param1[j];
							int r1 = PH->Param1[j];
							if (r0 != r1) {
								r = r0 + (((r1 - r0 + 1)*int(mrand())) >> 15);
							}
							else r = r0;
							if (r < 10)rd = R14[r]; else rd = 0;
							switch (ct) {
							case 1://L
								if (CheckPt(xx, yy)) {
									UNFND = false;
									if (r&&CheckPt(xx - r, yy) && CheckPt(xx + r, yy) && CheckPt(xx, yy - r) && CheckPt(xx, yy + r) &&
										CheckPt(xx - rd, yy - rd) && CheckPt(xx + rd, yy - rd) && CheckPt(xx - rd, yy + rd) && CheckPt(xx + rd, yy + rd))UNFND = true;
								};
								break;
							case 2://U
								if (!CheckPt(xx, yy)) {
									UNFND = false;
									if (r && !(CheckPt(xx - r, yy) || CheckPt(xx + r, yy) || CheckPt(xx, yy - r) || CheckPt(xx, yy + r) ||
										CheckPt(xx - rd, yy - rd) || CheckPt(xx + rd, yy - rd) || CheckPt(xx - rd, yy + rd) || CheckPt(xx + rd, yy + rd)))UNFND = true;
								};
								break;

							};
						};
					};
					if (!UNFND) {
						int ti = (int(rand())*PH->TexAmount[L]) >> 15;
						TexMap[i] = byte(PH->Texs[L][ti]);
					};
				};
				for (int p = 0; p < PH->NHiSections; p++)SetSectionsInPoint(i, PH->HiSect[p], VertHi);
			};
		};
	};
};
void GenerateRandomRoad(int idx) {
	int cnt = 0;
	SurfType = 6;
	PlatoHi = 100;
	do {
		cnt++;
		int x0 = (mrand()*(480 << ADDSH)) >> 15;
		int y0 = (mrand()*(480 << ADDSH)) >> 15;
		if (!CheckBar(x0 - 6, y0 - 6, 12, 12)) {
			AddPointToCurve(x0 << 4, y0 << 4, 0, idx);
			byte dir0 = mrand() & 255;
			int L = 400;
			int L0 = 6;
			do {
				int maxd = 32;
				int x1, y1;
				byte d1;
				do {
					int dd = ((mrand()*maxd) >> 14) - maxd;
					d1 = dir0 + dd;
					x1 = x0 + ((10 * int(TCos[d1])) >> 8);
					y1 = y0 + ((10 * int(TSin[d1])) >> 8);
					maxd += 10;
				} while (maxd < 100 && CheckBar(x1 - 6, y1 - 6, 12, 12));
				if (maxd < 100 && L>3) {
					if (L0 < 1) {
						AddPointToCurve(x1 << 4, y1 << 4, 1, 127 + SurfType);
						L0 = 6;
					};
					L0--;
					AddPointToCurve(x1 << 4, y1 << 4, 0, 127 + SurfType);
					x0 = x1;
					y0 = y1;
					dir0 = d1;
				}
				else {
					L = 1;
					AddPointToCurve(x1 << 4, y1 << 4, 1, 127 + SurfType);
					return;
				};
				L--;
			} while (L);
			return;
		};
	} while (cnt < 200);
};
extern int RealLx;
extern int RealLy;
void xLine(int x, int y, int x1, int y1, byte c);
void SHOWBARS(BlockBars* BB, byte c) {
	int dx = mapx << 5;
	int dy = mapy << 4;
	for (int i = 0; i < BB->NBars; i++) {
		int xr = (BB->BC[i].x << 6) + 32;
		int yr = (BB->BC[i].y << 6) + 32;

		int x = (xr)-dx;
		int y = (yr >> 1) - dy;
		if (Mode3D)y -= GetHeight(xr, yr);
		if (x > -32 && y > -32 && x < RealLx + 32 && y < RealLy - 32) {
			xLine(x, y - 8, x, y + 8, c);
			xLine(x - 16, y, x + 16, y, c);
		};
	};
};
void ShowLockBars() {
	SHOWBARS(&UnLockBars, 0x9B);
	SHOWBARS(&LockBars, 0x0D);
};
char* GSU_file;
void GSU_Error(char* Nation) {
	char ccc[100];
	sprintf(ccc, "Invalid data for %s in %s", Nation, GSU_file);
	MessageBox(hwnd, ccc, "ERROR!", 0);
};
extern HWND hwnd;
bool ReadWinString(GFILE* F, char* STR, int Max);
int ADDRES[8];
bool GenerateStartUnits(char* NationID, byte NI, int x, int y, int GenIndex) {
	memset(ADDRES, 0, 8 * 4);
	if (!GenIndex)return false;
	char ccc[128];
	sprintf(ccc, "Data\\Start%d.dat", GenIndex);
	GSU_file = ccc;
	GFILE* F = Gopen(ccc, "r");
	bool READOK = 1;
	if (F) {
		char ccc[128];
		do {
			ccc[0] = 0;
			READOK = ReadWinString(F, ccc, 128);
			while (ccc[0] && ccc[strlen(ccc) - 1] == ' ')ccc[strlen(ccc) - 1] = 0;
			if (ccc[0] == '#' && !strcmp(ccc + 1, NationID)) {
				//found!
				word UnitsIDS[64];
				char UnitMark[64];
				int NUIDS = 0;
				ccc[0] = 0;
				ReadWinString(F, ccc, 128);
				int z = sscanf(ccc, "%d%d%d%d%d%d", ADDRES + TreeID, ADDRES + FoodID, ADDRES + StoneID, ADDRES + GoldID, ADDRES + IronID, ADDRES + CoalID);
				if (z != 6) {
					memset(ADDRES, 0, 8 * 4);
					GSU_Error(NationID);
					Gclose(F);
					return false;
				};
				ccc[0] = 0;
				int N;
				ReadWinString(F, ccc, 128);
				z = sscanf(ccc, "%d", &N);
				if (z == 1) {
					for (int j = 0; j < N; j++) {
						ReadWinString(F, ccc, 128);
						char ccx[128];
						char ccy[128];
						int z = sscanf(ccc, "%s%s", ccx, ccy);
						if (z == 2) {
							//search for the monster
							GeneralObject** GOS = NATIONS[0].Mon;
							int NGO = NATIONS[0].NMon;
							for (int q = 0; q < NGO; q++)if (!strcmp(GOS[q]->MonsterID, ccx)) {
								UnitsIDS[NUIDS] = q;
								UnitMark[NUIDS] = ccy[0];
								NUIDS++;
								q = NGO + 100;
							};
						}
						else {
							GSU_Error(NationID);
							Gclose(F);
							return false;
						};
					};
					int dx = 0;
					int dy = 0;
					ccc[0] = 0;
					ReadWinString(F, ccc, 128);
					if (!ccc[0]) {
						GSU_Error(NationID);
						Gclose(F);
						return false;
					};
					int z = sscanf(ccc, "%d%d", &dx, &dy);
					if (z == 2) {
						//ready to read data
						int Y0 = 0;
						do {
							ccc[0] = '#';
							ccc[1] = 0;
							ReadWinString(F, ccc, 128);
							if (ccc[0] != '#') {
								int L = strlen(ccc);
								for (int v = 0; v < L; v++) {
									char c = ccc[v];
									if (c != ' ') {
										for (int u = 0; u < NUIDS; u++)if (UnitMark[u] == c) {
											NATIONS[NI].CreateNewMonsterAt((x << 4) + ((v - dx) << 9), (y << 4) + ((Y0 - dy) << 9), UnitsIDS[u], 1);
											u = NUIDS;
										};
									};
								};
							};
							Y0++;
						} while (ccc[0] != '#');
					}
					else {
						GSU_Error(NationID);
						Gclose(F);
						return false;
					};
				}
				else {
					GSU_Error(NationID);
					Gclose(F);
					return false;
				};
				Gclose(F);
				return true;
			};
		} while (READOK);
		Gclose(F);
		return false;
	}
	else return false;
};
