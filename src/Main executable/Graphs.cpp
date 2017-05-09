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
#include "Dialogs.h"
#include "fonts.h"
#include "dpchat.h"
#include "dplobby.h"
#include "GSound.h"
#include "3DGraph.h"
#include "3DMapEd.h"
#include "MapSprites.h"
#include <assert.h>
#include <math.h>
#include "NewMon.h"
#include "IconTool.h"
#include "GP_Draw.h"
#include "3DRandMap.h"
#include <crtdbg.h>
#include "ActiveScenary.h"
#include "DrawForm.h"
#include "Conststr.h"
#include <Process.h>
#include "MPlayer.h"
#include "Recorder.h"
#include "GSINC.H"
#define NOGRAF
class  OneGraph {
public:
	int Color;
	int* T;
	int* V;
	int  N;
	int  NMax;
	//------------
	int MinT;
	int MaxT;
	int MinV;
	int MaxV;
	OneGraph();
	~OneGraph();
	void AddTV(int T, int V);
	void Clear();

};
class Graph {
public:
	char* Header;
	OneGraph GRP[16];
	Graph();
	void Add(int t, int v, byte c);
	void Clear();
	void Draw(int x0, int y0, int Lx, int Ly);
};
Graph::Graph() {
	Header = NULL;
};
OneGraph::OneGraph() {
	T = NULL;
	V = NULL;
	NMax = 0;
	N = 0;
	Color = -1;
};
void OneGraph::Clear() {
	N = 0;
	Color = -1;
	if (NMax) {
		free(T);
		free(V);
	};
	T = NULL;
	V = NULL;
	NMax = 0;
};
OneGraph::~OneGraph() {
	Clear();
};
void OneGraph::AddTV(int t, int v) {
	if (N >= NMax) {
		NMax += 1024;
		T = (int*)realloc(T, NMax * 4);
		V = (int*)realloc(V, NMax * 4);
	};
	T[N] = t;
	V[N] = v;
	if (!N) {
		MinT = t;
		MaxT = t;
		MinV = v;
		MaxV = v;
	}
	else {
		if (t < MinT)MinT = t;
		if (t > MaxT)MaxT = t;
		if (v < MinV)MinV = v;
		if (v > MaxV)MaxV = v;
	};
	N++;
};
void Graph::Add(int t, int v, byte c) {
	for (int i = 0; i < 16; i++)if (GRP[i].Color == c) {
		GRP[i].AddTV(t, v);
		return;
	};
	for (int i = 0; i < 16; i++)if (GRP[i].Color == -1) {
		GRP[i].Color = c;
		GRP[i].AddTV(t, v);
		return;
	};
};
void Graph::Clear() {
	for (int i = 0; i < 16; i++)GRP[i].Clear();
};
void xLine(int x, int y, int x1, int y1, byte c);
void Graph::Draw(int x0, int y0, int Lx, int Ly) {
	int MaxT = -2147483647; //BUGFIX: -2147483648 causes C4146
	int MinT = 2147483647;
	int MaxV = -2147483647;
	int MinV = 2147483647;
	for (int i = 0; i < 16; i++) {
		OneGraph* OG = GRP + i;
		if (OG->N) {
			if (OG->MinT < MinT)MinT = OG->MinT;
			if (OG->MaxT > MaxT)MaxT = OG->MaxT;
			if (OG->MinV < MinV)MinV = OG->MinV;
			if (OG->MaxV > MaxV)MaxV = OG->MaxV;
		};
	};
	if (MinT < MaxT && MinV < MaxV) {
		OneGraph* OG = GRP;
		for (int i = 0; i < 16; i++) {
			if (OG->N) {
				int NP = OG->N;
				int* V = OG->V;
				int* T = OG->T;
				int px = 0;
				int py = 0;
				for (int j = 0; j < NP; j++) {
					int x = x0 + ((T[j] - MinT)*Lx) / (MaxT - MinT);
					int y = y0 + Ly - ((V[j] - MinV)*Ly) / (MaxV - MinV);
					if (j) {
						xLine(px, py, x, y, OG->Color);
					};
					px = x;
					py = y;
				};
			};
			OG++;
		};
		Xbar(x0, y0 - 1, Lx, Ly + 2, 255);
		char cc[32];
		sprintf(cc, "%d", MaxV);
		ShowString(x0 + 2, y0 + 2, cc, &SmallWhiteFont);
		sprintf(cc, "%d", MinV);
		ShowString(x0 + 2, y0 + Ly - 12, cc, &SmallWhiteFont);
		if (Header)
			ShowString(x0 + (Lx - GetRLCStrWidth(Header, &SmallWhiteFont)) / 2, y0 + 2, Header, &SmallWhiteFont);
	};
};
extern int RealLx;
char* GHDR[10] = { "Add Time","Current Step Time","Ping","Input stream","Output stream","Execute Buffer Size","Max send block","Send Frequency","MaxPingTime","?" };
Graph GRPS[10];
void DrawAllGrp()
{
#ifndef NOGRAF
	if (GetKeyState(VK_SCROLL)) {
		for (int i = 0; i < 10; i++) {
			GRPS[i].Header = GHDR[i];
			GRPS[i].Draw(10 + (i / 5) * 400, 50 + (i % 5) * 110, 390, 100);
		};
	};
#endif
}

void ADDGR(int g, int t, int v, byte c)
{
#ifndef NOGRAF
	GRPS[g].Add(t, v, c);
#endif
}

void CLRGR()
{
	for (int i = 0; i < 10; i++)GRPS[i].Clear();
}