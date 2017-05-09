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
#include "TopoGraf.h"
#include <CrtDbg.h>
#include "fonts.h"
MediaTop GTOP[2];
Radio Rarr[RRad];
void CreateWTopMap();
void CreateRadio() {
	for (int ix = -RRad; ix <= RRad; ix++) {
		for (int iy = -RRad; iy <= RRad; iy++) {
			int r = int(sqrt(ix*ix + iy*iy));
			if (r < RRad)Rarr[r].N++;
		};
	};
	for (int i = 0; i < RRad; i++) {
		if (Rarr[i].N) {
			Rarr[i].xi = new char[Rarr[i].N];
			Rarr[i].yi = new char[Rarr[i].N];
			Rarr[i].N = 0;
		};
	};
	for (int ix = -RRad; ix <= RRad; ix++) {
		for (int iy = -RRad; iy <= RRad; iy++) {
			int r = int(sqrt(ix*ix + iy*iy));
			if (r < RRad) {
				int N = Rarr[r].N;
				Rarr[r].xi[N] = ix;
				Rarr[r].yi[N] = iy;
				Rarr[r].N++;
			};
		};
	};
}
void CreateLinkInfo();
word* TopRef;
Area* TopMap;
int   NAreas;
int MaxArea;
word* MotionLinks;
word* LinksDist;

bool AddArea(short x, short y, byte Sliv)
{
	if (NAreas > 1016)
	{
		return false;
	}

	int MinNorm = 0;
	switch (ADDSH)
	{
	case 1:
		MinNorm = 7;
		break;
	case 2:
		MinNorm = 10;
		break;
	case 3:
		MinNorm = 15;
		break;
	}

	if (Sliv != 2)
	{
		for (int i = 0; i < NAreas; i++)
		{
			if (Norma(TopMap[i].x - x, TopMap[i].y - y) <= MinNorm)
			{
				if (Sliv)
				{
					TopMap[i].x = (TopMap[i].x + x) >> 1;
					TopMap[i].y = (TopMap[i].y + y) >> 1;
				}
				return false;
			}
		}

		if (NAreas >= MaxArea)
		{
			MaxArea += 120;
			TopMap = (Area*)realloc(TopMap, MaxArea * sizeof(Area));
		}

		Area* AR = TopMap + NAreas;
		AR->x = x;
		AR->y = y;
		AR->Importance = 0;
		AR->NTrees = 0;
		AR->NStones = 0;
		AR->MinesIdx = nullptr;
		AR->NLinks = 0;
		AR->NMines = 0;
		AR->MaxLink = 6;
		AR->Link = new word[AR->MaxLink << 1];
		NAreas++;
		return true;
	}
}

void AddLink(int N1, int N2) {
	Area* AR = &TopMap[N1];
	int N = AR->NLinks;
	for (int i = 0; i < N; i++) {
		if (AR->Link[i + i] == N2)return;
	};
	if (AR->NLinks == AR->MaxLink) {
		AR->MaxLink += 4;
		AR->Link = (word*)realloc(AR->Link, AR->MaxLink << 2);
	};
	N = AR->NLinks;
	AR->Link[N + N] = N2;
	AR->Link[N + N + 1] = Norma(AR->x - TopMap[N2].x, AR->y - TopMap[N2].y);
	AR->NLinks++;
};
extern int MAXSPR;
void EraseAreas();
bool GetTCStatus(int x, int y) {
	int xxx = x << 2;
	int yyy = y << 2;
	if (!MFIELDS->CheckBar(xxx, yyy, 4, 4))return true;
	else {
		//accurate check
		if (((!CheckPt(xxx + 1, yyy)) || (!CheckPt(xxx + 2, yyy + 1)) || (!CheckPt(xxx + 3, yyy + 2))) ||
			((!CheckPt(xxx, yyy + 1)) || (!CheckPt(xxx + 1, yyy + 2)) || (!CheckPt(xxx + 2, yyy + 3))) ||
			((!CheckPt(xxx + 2, yyy)) || (!CheckPt(xxx + 1, yyy + 1)) || (!CheckPt(xxx, yyy + 2))) ||
			((!CheckPt(xxx + 3, yyy + 1)) || (!CheckPt(xxx + 2, yyy + 2)) || (!CheckPt(xxx + 1, yyy + 3)))
			)return true;
	};
	return false;
}

void ResearchIslands();
void CreateCostPlaces();
void StopDynamicalTopology();
bool ProcessMessages();
void CreateAreas() {
	StopDynamicalTopology();
	EraseAreas();
	MaxArea = 200;
	rpos = 0;
	TopMap = new Area[MaxArea];
	NAreas = 0;
	//creating ref. map
	memset(TopRef, 0xFFFF, TopLx*TopLx * 2);
	int mmx = msx >> 1;
	int mmy = msy >> 1;
	for (int x = 0; x < mmx; x++)
		for (int y = 0; y < mmy; y++) {
			if (GetTCStatus(x, y))TopRef[x + y*TopLx] = 0xFFFE;
		};
	/*
	for(x=0;x<mmx;x++)
		for(int y=0;y<mmy;y++){
			if(x>0&&y>0&&x<mmx-1&&y<mmx-1&&!MFIELDS[1].CheckBar(x<<2,y<<2,4,4)){
				int ofs=x+y*TopLx;
				TopRef[ofs]=0xFFFD;
				if(TopRef[ofs-1]==0xFFFE)TopRef[ofs-1]=0xFFFF;
				if(TopRef[ofs+1]==0xFFFE)TopRef[ofs+1]=0xFFFF;
				if(TopRef[ofs-TopLx]==0xFFFE)TopRef[ofs-TopLx]=0xFFFF;
				if(TopRef[ofs+TopLx]==0xFFFE)TopRef[ofs+TopLx]=0xFFFF;
				if(TopRef[ofs-TopLx]==0xFFFE)TopRef[ofs-TopLx]=0xFFFF;
				if(TopRef[ofs-TopLx+1]==0xFFFE)TopRef[ofs-TopLx+1]=0xFFFF;
				if(TopRef[ofs-TopLx-1]==0xFFFE)TopRef[ofs-TopLx-1]=0xFFFF;
				if(TopRef[ofs+TopLx+1]==0xFFFE)TopRef[ofs+TopLx+1]=0xFFFF;
				if(TopRef[ofs+TopLx-1]==0xFFFE)TopRef[ofs+TopLx-1]=0xFFFF;
			};
		};
	for(x=0;x<mmx;x++)
		for(int y=0;y<mmy;y++){
			int ofs=x+y*TopLx;
			if(TopRef[ofs]==0xFFFD)TopRef[ofs]=0xFFFE;
		};
	*/
	int N1 = 0;
	//assert(_CrtCheckMemory());
	for (int i = 0; i < MAXSPR; i++) {
		OneSprite* OS = &Sprites[i];
		if (OS->Enabled) {
			SprGroup* SG = OS->SG;
			ObjCharacter* OC = &SG->ObjChar[OS->SGIndex];
			if (SG == &TREES&&OS->SGIndex == SG->NSpr - 1)AddArea(OS->x >> 6, OS->y >> 6, 2);
		};
	};
	/*
	for(i=0;i<MAXSPR;i++){
		OneSprite* OS=&Sprites[i];
		if(OS->Enabled){
			SprGroup* SG=OS->SG;
			ObjCharacter* OC=&SG->ObjChar[OS->SGIndex];
			if(OC->IntResType<8)AddArea(OS->x>>6,OS->y>>6,0);
		};
	};
	*/
	do {
		int x = (int(rando())*(mmx - 4)) >> 15;
		int y = (int(rando())*(mmy - 4)) >> 15;
		bool Empty = true;
		for (int dx = 0; dx <= 2 && Empty; dx++)
			for (int dy = 0; dy <= 2 && Empty; dy++) {
				if (TopRef[x + dx + (y + dy)*TopLx] != 0xFFFE)Empty = false;
			};
		if (Empty) {
			if (AddArea(x + 1, y + 1, 0))N1 = 0;
			else N1++;
		};
	} while (N1 < 50);
	ProcessMessages();
	//assert(_CrtCheckMemory());
	//creating linking between areas
	for (int i = 0; i < NAreas; i++)TopRef[TopMap[i].x + int(TopMap[i].y)*TopLx] = i;
	bool Change = false;
	//word TempTop[TopLx*TopLy];
	Change = true;
	for (int i = 1; i < RRad&&Change; i++) {
		//memcpy(TempTop,TopRef,sizeof(TopRef));
		Change = false;
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for (int j = 0; j < NAreas; j++) {
			int xx = TopMap[j].x;
			int yy = TopMap[j].y;
			for (int k = 0; k < N; k++) {
				int x = xx + xi[k];
				int y = yy + yi[k];
				if (x > 0 && y > 0 && x < mmx&&y < mmy) {
					int ofst = x + y*TopLx;
					word ZZ = TopRef[ofst];
					word ZU = TopRef[ofst - TopLx];
					word ZD = TopRef[ofst + TopLx];
					word ZL = TopRef[ofst - 1];
					word ZR = TopRef[ofst + 1];
					if (ZZ == 0xFFFE) {
						if (ZU == j || ZD == j || ZL == j || ZR == j ||
							TopRef[ofst - TopLx - 1] == j ||
							TopRef[ofst - TopLx + 1] == j ||
							TopRef[ofst + TopLx - 1] == j ||
							TopRef[ofst + TopLx + 1] == j) {
							TopRef[ofst] = j;
							Change = true;
							ZZ = j;
						};
					};
					if (ZZ < 0xFFFE) {
						if (ZU != ZZ&&ZU < 0xFFFE) {
							AddLink(ZZ, ZU);
							AddLink(ZU, ZZ);
						};
						if (ZD != ZZ&&ZD < 0xFFFE) {
							AddLink(ZZ, ZD);
							AddLink(ZD, ZZ);
						};
						if (ZL != ZZ&&ZL < 0xFFFE) {
							AddLink(ZZ, ZL);
							AddLink(ZL, ZZ);
						};
						if (ZR != ZZ&&ZR < 0xFFFE) {
							AddLink(ZZ, ZR);
							AddLink(ZR, ZZ);
						};
					};
				};
			};
		};
	};
	ProcessMessages();
	//creating resource references
	for (int i = 0; i < MAXSPR; i++) {
		OneSprite* OS = &Sprites[i];
		SprGroup* SG = OS->SG;
		ObjCharacter* OC = &SG->ObjChar[OS->SGIndex];
		if (OC->IntResType < 8) {
			byte imp = 0;
			int ort = OC->IntResType;
			if (ort == GoldID)imp = 1;
			else if (ort == IronID)imp = 2;
			else if (ort == CoalID)imp = 4;
			int xx = OS->x >> 6;
			int yy = OS->y >> 6;
			int ofs = xx + yy*TopLx;
			word TZ = TopRef[ofs];
			if (TZ < 4096) {
				Area* AR = TopMap + TZ;
				AR->NMines++;
				AR->Importance |= imp;
			};
		};
	};
	for (int i = 0; i < NAreas; i++)if (TopMap[i].NMines) {
		TopMap[i].MinesIdx = new word[TopMap[i].NMines];
		TopMap[i].NMines = 0;
	};
	for (int i = 0; i < MAXSPR; i++) {
		OneSprite* OS = &Sprites[i];
		SprGroup* SG = OS->SG;
		ObjCharacter* OC = &SG->ObjChar[OS->SGIndex];
		if (OC->IntResType < 8) {
			int xx = OS->x >> 6;
			int yy = OS->y >> 6;
			int ofs = xx + yy*TopLx;
			word TZ = TopRef[ofs];
			if (TZ < 4096) {
				Area* AR = TopMap + TZ;
				AR->MinesIdx[AR->NMines] = i;
				AR->NMines++;
			};
		};
	};
	ProcessMessages();
	MotionLinks = new word[NAreas*NAreas];
	LinksDist = new word[NAreas*NAreas];
	memset(MotionLinks, 0xFFFF, NAreas*NAreas * 2);
	memset(LinksDist, 0xFFFF, NAreas*NAreas * 2);
	CreateLinkInfo();
	CreateStrategyInfo();
	CreateWTopMap();
	ResearchIslands();
	CreateCostPlaces();
}

void ClearLinkInfo() {
	memset(MotionLinks, 0xFFFF, NAreas*NAreas * 2);
	memset(LinksDist, 0xFFFF, NAreas*NAreas * 2);
	for (int i = 0; i < NAreas; i++) {
		Area* AR = TopMap + i;
		int k = AR->NLinks;
		for (int j = 0; j < k; j++) {
			int ofst = AR->Link[j + j] + i*NAreas;
			MotionLinks[ofst] = AR->Link[j + j];
			LinksDist[ofst] = AR->Link[j + j + 1];
		};
	};
};
int OneIteration() {
	int N = NAreas*NAreas;
	int ofs = 0;
	int NChanges = 0;
	for (int i = 0; i < NAreas; i++)
		for (int j = 0; j < NAreas; j++) {
			if (i != j) {
				Area* AR = TopMap + i;
				int N = AR->NLinks;
				word* LINK = AR->Link;
				int CurDis = LinksDist[ofs];
				for (int k = 0; k < N; k++) {
					word N2 = LINK[k + k];
					word adds = LINK[k + k + 1];
					if (N2 != j) {
						int ofs1 = j + N2*NAreas;
						int dst = LinksDist[ofs1];
						if (dst != 0xFFFF) {
							if (dst + adds < CurDis) {
								LinksDist[ofs] = dst + adds;
								MotionLinks[ofs] = N2;
								NChanges++;
							};
						};
					};
				};
			};
			ofs++;
		};
	return NChanges;
};
void CreateRoadsNet();
extern bool MiniMade;
bool ProcessMessages();
bool NeedProcessTop;
void CreateLinkInfo() {
	ClearLinkInfo();
	int N;
	int p = 0;
	do {
		ProcessMessages();
		N = OneIteration();
		p++;
	} while (N&&p < 3);
	//CreateRoadsNet();
	NeedProcessTop = 1;
	MiniMade = false;
};
void InitAreas() {
	NAreas = 0;
};
void EraseAreas() {
	if (NAreas) {
		for (int i = 0; i < NAreas; i++) {
			Area* AR1 = &TopMap[i];
			if (AR1->MaxLink)free(AR1->Link);
			if (AR1->NMines)free(AR1->MinesIdx);
		};
		free(TopMap);
		free(MotionLinks);
		free(LinksDist);
		NAreas = 0;
		MaxArea = 0;
		TopMap = NULL;
	};
	if (!WNAreas)return;
	for (int i = 0; i < WNAreas; i++) {
		Area* AR1 = &WTopMap[i];
		if (AR1->MaxLink)free(AR1->Link);
		if (AR1->NMines)free(AR1->MinesIdx);
	};
	free(WTopMap);
	free(WMotionLinks);
	free(WLinksDist);
	WNAreas = 0;
	WMaxArea = 0;
	WTopMap = NULL;
}

extern word CurZone;

#define xTopLx (256<<2)

//Crawl the map and enumerate objects
void CreateStrategyInfo()
{
	if (!NAreas)
	{
		return;
	}

	for (int i = 0; i < NAreas; i++)
	{
		memset(&TopMap[i].SINF[0], 0, 8 * sizeof StrategyInfo);
	}

	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB)
		{
			int xx = OB->RealX >> 10;
			int yy = OB->RealY >> 10;
			int ofs = xx + (yy << TopSH);
			if (ofs >= 0 && ofs < MaxTop)
			{
				word TR = TopRef[ofs];
				if (TR < 0xFFFE)
				{
					Area* AR = TopMap + TR;
					StrategyInfo* STINF = &AR->SINF[OB->NNUM];
					byte USAGE = OB->newMons->Usage;
					switch (USAGE)
					{
					case MelnicaID:
						STINF->BuildInfo |= SI_Meln;
						break;
					case FarmID:
						STINF->BuildInfo |= SI_Farm;
						break;
					case CenterID:
						STINF->BuildInfo |= SI_Center;
						break;
					case SkladID:
						STINF->BuildInfo |= SI_Sklad;
						break;
					case TowerID:
						STINF->BuildInfo |= SI_Tower;
						STINF->NTowers++;
						break;
					case MineID:
						STINF->BuildInfo |= SI_Shahta;
						break;
					case HardHorceID:
					case LightInfID:
						if (STINF->NShortRange < 255)
							STINF->NShortRange++;
						break;
					case HorseStrelokID:
					case StrelokID:
						if (STINF->NLongRange < 255)
							STINF->NLongRange++;
						break;
					case ArcherID:
					case GrenaderID:
						if (STINF->NLongRange < 255)
							STINF->NLongRange++;
						break;
					case ArtDepoID:
						STINF->BuildInfo |= SI_ArtDepo;
						break;
					case PeasantID:
						if (STINF->NPeasants < 255)
							STINF->NPeasants++;
						break;
					case MortiraID:
						if (STINF->NMortir < 255)
							STINF->NMortir++;
						break;
					case PushkaID:
						if (STINF->NPushek < 255)
							STINF->NPushek++;
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < NAreas; i++)
	{
		Area*AR = TopMap + i;
		int N = AR->NLinks;
		StrategyInfo* ST = AR->SINF;
		if (N)
		{
			word* Lin = AR->Link;
			for (int j = 0; j < N; j++)
			{
				Area* AR2 = TopMap + Lin[j];
				StrategyInfo* ST2 = AR2->SINF;
				for (int k = 0; k < 8; k++)
				{
					if (ST[k].BuildInfo & (~SI_Near))
					{
						ST2[k].BuildInfo |= SI_Near;
					}
				}
			}
		}
	}
}

bool WasOnlyOpen;
word NChAreas;
word MaxChAreas;
word* ChAreas;
//procesing variables
int CurIStart;
int TmpChanges;
word* TmpMLinks;
word* TmpMDist;
int MinChX;
int MaxChX;
int MinChY;
int MaxChY;
int GetTopology(int x, int y);
void InitTopChange() {
	NeedProcessTop = false;
	MaxChAreas = 0;
	NChAreas = 0;
	ChAreas = NULL;
	TmpMLinks = NULL;
	TmpMDist = NULL;
	CurIStart = 0;
	MinChX = 100000;
	MaxChX = 0;
	MinChY = 100000;
	MaxChY = 0;
};
void ClearTopChange() {
	if (ChAreas)free(ChAreas);
	if (TmpMLinks) {
		free(TmpMLinks);
		free(TmpMDist);
	};
	TmpMLinks = NULL;
	TmpMDist = NULL;
	InitTopChange();
	WasOnlyOpen = true;
};
void AddChTop(word TopID) {
	for (int i = 0; i < NChAreas; i++)if (ChAreas[i] == TopID)return;
	if (NChAreas >= MaxChAreas) {
		MaxChAreas += 32;
		ChAreas = (word*)realloc(ChAreas, MaxChAreas << 1);
	};
	ChAreas[NChAreas] = TopID;
	NChAreas++;
};
void AddSurroundingAreas(word TopID) {
	if (!NAreas)return;
	AddChTop(TopID);
	Area* AR = TopMap + TopID;
	int N = AR->NLinks;
	for (int i = 0; i < N; i++)AddChTop(AR->Link[i << 1]);
};
void DynamicalLockTopCell(int x, int y) {
	NeedProcessTop = true;
	word topo = GetTopology(x << 6, y << 6);
	if (topo < 0xFFFE) {
		AddSurroundingAreas(topo);
	};
	int ofst = x + (y << TopSH);
	TopRef[ofst] = 0xFFFF;
	if (x < MinChX)MinChX = x;
	if (x > MaxChX)MaxChX = x;
	if (y < MinChY)MinChY = y;
	if (y > MaxChY)MaxChY = y;
	WasOnlyOpen = false;
};
void DynamicalUnLockTopCell(int x, int y) {
	NeedProcessTop = true;
	word topo = GetTopology(x << 6, y << 6);
	if (topo < 0xFFFE) {
		AddSurroundingAreas(topo);
	};
	int ofst = x + (y << TopSH);
	TopRef[ofst] = 0xFFFE;
	if (x < MinChX)MinChX = x;
	if (x > MaxChX)MaxChX = x;
	if (y < MinChY)MinChY = y;
	if (y > MaxChY)MaxChY = y;
};
void OnePartIteration(int Nitr) {
	if (!TmpMLinks)return;
	int N = NAreas*NAreas;
	int ofs = CurIStart*NAreas;
	if (!CurIStart)TmpChanges = 0;
	for (int i = CurIStart; i < NAreas; i++) {
		for (int j = 0; j < NAreas; j++) {
			if (i != j) {
				Area* AR = TopMap + i;
				int N = AR->NLinks;
				word* LINK = AR->Link;
				int CurDis = TmpMDist[ofs];
				//int CurDis=LinksDist[ofs];//
				for (int k = 0; k < N; k++) {
					word N2 = LINK[k + k];
					word adds = LINK[k + k + 1];
					if (N2 != j&&adds != 0xFFFF) {
						int ofs1 = j + N2*NAreas;
						int dst = TmpMDist[ofs1];
						//int dst=LinksDist[ofs1];//
						if (dst != 0xFFFF) {
							if (dst + adds < CurDis) {
								int ddd = dst + adds;
								TmpMDist[ofs] = ddd;
								//LinksDist[ofs]=ddd;//
								TmpMLinks[ofs] = N2;
								//MotionLinks[ofs]=N2;//
								if (ddd < LinksDist[ofs]) {
									LinksDist[ofs] = ddd;
									MotionLinks[ofs] = N2;
								};
								TmpChanges++;
							};
						};
					};
				};
			};
			ofs++;
		};
		CurIStart = i;
		Nitr--;
		if (!Nitr)return;
	};
	//COUNTER=TmpChanges;
	if (!TmpChanges) {
		memcpy(MotionLinks, TmpMLinks, NAreas*NAreas * 2);
		memcpy(LinksDist, TmpMDist, NAreas*NAreas * 2);
		free(TmpMLinks);
		free(TmpMDist);
		TmpMLinks = NULL;
		TmpMDist = NULL;
		CurIStart = 0;
	}
	else {
		CurIStart = 0;
	};
};
void LimitedClearBadLinks(word* AList, int Na, word* MLinks, word* MDist) {

	int Change;
	int p = 0;
	do {
		Change = 0;
		for (int i = 0; i < Na; i++) {
			int Ai = AList[i];
			Area* ARi = TopMap + Ai;
			int N = ARi->NLinks;
			word* LINK = ARi->Link;
			int ofs0 = int(AList[i])*NAreas;
			for (int j = 0; j < Na; j++) {
				if (i != j) {
					int Aj = AList[j];
					int ofs = ofs0 + Aj;
					int CurDis = MDist[ofs];
					if (CurDis != 0xFFFF) {
						for (int k = 0; k < N; k++) {
							word N2 = LINK[k + k];
							//word adds=LINK[k+k+1];
							if (N2 != Aj) {
								int ofs1 = Aj + N2*NAreas;
								int dst = TmpMDist[ofs1];
								if (dst == 0xFFFF) {
									MLinks[ofs] = 0xFFFF;
									MDist[ofs] = 0xFFFF;
									Change++;
								};
							};
						};
					};
				};

			};
		};
		p++;
	} while (Change&&p < 100);
};
void TotalClearBadLinks(word* MLinks, word* MDist) {

	int Change;
	int p = 0;
	do {
		Change = 0;
		for (int i = 0; i < NAreas; i++) {
			int Ai = i;
			Area* ARi = TopMap + Ai;
			int N = ARi->NLinks;
			word* LINK = ARi->Link;
			int ofs0 = int(i)*NAreas;
			for (int j = 0; j < NAreas; j++) {
				if (i != j) {
					int Aj = j;
					int ofs = ofs0 + Aj;
					int CurDis = MDist[ofs];
					if (CurDis != 0xFFFF) {
						for (int k = 0; k < N; k++) {
							word N2 = LINK[k + k];
							//word adds=LINK[k+k+1];
							if (N2 != Aj) {
								int ofs1 = Aj + N2*NAreas;
								int dst = TmpMDist[ofs1];
								if (dst == 0xFFFF) {
									MLinks[ofs] = 0xFFFF;
									MDist[ofs] = 0xFFFF;
									Change++;
								};
							};
						};
					};
				};

			};
		};
		p++;
	} while (Change&&p < 100);
};
void LimitedSetNewLinks(word* AList, int Na, word* MLinks, word* MDist) {

	byte* aropt = new byte[NAreas];
	memset(aropt, 0, NAreas);
	for (int i = 0; i < Na; i++)aropt[AList[i]] = 1;

	int Change;
	int p = 0;
	do {
		Change = 0;
		for (int i = 0; i < Na; i++) {
			int Ai = AList[i];
			Area* ARi = TopMap + Ai;
			int N = ARi->NLinks;
			word* LINK = ARi->Link;
			int ofs0 = int(AList[i])*NAreas;
			for (int j = 0; j < Na; j++) {
				if (i != j) {
					int Aj = AList[j];
					int ofs = ofs0 + Aj;
					int CurDis = MDist[ofs];
					for (int k = 0; k < N; k++) {
						word N2 = LINK[k + k];
						if (aropt[N2]) {
							word adds = LINK[k + k + 1];
							if (N2 != Aj&&adds != 0xFFFF) {
								int ofs1 = Aj + N2*NAreas;
								int dst = MDist[ofs1];
								if (dst != 0xFFFF) {
									if (dst + adds < CurDis) {
										int ddd = dst + adds;
										MDist[ofs] = ddd;
										MLinks[ofs] = N2;
										if (ddd < LinksDist[ofs]) {
											LinksDist[ofs] = ddd;
											MotionLinks[ofs] = N2;
										};
										Change++;
									};
								};
							};
						};
					};
				};
			};
		};
		p++;
	} while (Change&&p < 100);
	free(aropt);
};
void FastReduceWays(word* MLinks, word* MDist, word* StartChange, int NStartChange) {
	word Changes[1000];
	word PrevChanges[1000];
	memcpy(PrevChanges, StartChange, NStartChange << 1);
	int NPrev = NStartChange;
	int Change = 0;
	int p = 0;
	int NChanges;
	do {
		memcpy(Changes, PrevChanges, NPrev << 1);
		NChanges = NPrev;
		NPrev = 0;
		//extend changes
		byte ChMap[1000];
		memset(ChMap, 0, NAreas);
		byte PrevMap[1000];
		memset(PrevMap, 0, NAreas);
		for (int i = 0; i < NChanges; i++)ChMap[Changes[i]] = 1;
		int N1 = NChanges;
		for (int i = 0; i < N1; i++) {
			Area* AR = TopMap + Changes[i];
			int N = AR->NLinks;
			word* ids = AR->Link;
			for (int j = 0; j < N; j++) {
				word id = ids[j];
				if (!ChMap[id]) {
					ChMap[id] = 1;
					Changes[NChanges] = id;
					NChanges++;
				};
			};
		};
		//processing
		Change = 0;
		for (int i = 0; i < NChanges; i++) {
			int Ai = Changes[i];
			Area* ARi = TopMap + Ai;
			int N = ARi->NLinks;
			word* LINK = ARi->Link;
			int ofs0 = int(Changes[i])*NAreas;
			for (int j = 0; j < NChanges; j++) {
				if (i != j) {
					int Aj = Changes[j];
					int ofs = ofs0 + Aj;
					int CurDis = MDist[ofs];
					for (int k = 0; k < N; k++) {
						word N2 = LINK[k + k];
						//if(aropt[N2]){
						word adds = LINK[k + k + 1];
						if (N2 != Aj&&adds != 0xFFFF) {
							int ofs1 = Aj + N2*NAreas;
							int dst = MDist[ofs1];
							if (dst != 0xFFFF) {
								if (dst + adds < CurDis) {
									int ddd = dst + adds;
									MDist[ofs] = ddd;
									MLinks[ofs] = N2;
									//if(ddd<LinksDist[ofs]){
									//	LinksDist[ofs]=ddd;
									//	MotionLinks[ofs]=N2;
									//};
									Change++;
									if (!PrevMap[Ai]) {
										PrevChanges[NPrev] = Ai;
										PrevMap[Ai] = 1;
										NPrev++;
									};
									if (!PrevMap[Aj]) {
										PrevChanges[NPrev] = Aj;
										PrevMap[Aj] = 1;
										NPrev++;
									};
								};
							};
						};
						//};
					};
				};
			};
		};
		p++;
	} while (Change&&p < 200);
};
void OnePartIterationNew(int Nitr);
void RemakeGroupOfAreas(word* Grp, int Na);
int NeedITR = 100;

void ProcessDynamicalTopology()
{
	if (NChAreas)
	{
		RemakeGroupOfAreas(ChAreas, NChAreas);
		NChAreas = 0;
		NeedITR = 3200;
	}

	if (MinChX <= MaxChX)
	{
		word LimAreas[200];
		int xc = (MaxChX + MinChX) >> 1;
		int yc = (MaxChY + MinChY) >> 1;
		word NLim = 0;
		for (int i = 0; i < NAreas; i++)
		{
			Area* AR1 = TopMap + i;
			if (Norma(xc - int(AR1->x), yc - int(AR1->y)) < 70)
			{
				LimAreas[NLim] = i;
				NLim++;
			}
		}

		for (int i = 0; i < NAreas; i++)
		{
			Area* AR = TopMap + i;
			int k = AR->NLinks;
			for (int j = 0; j < k; j++)
			{
				int ofst = int(AR->Link[j + j]) + i*NAreas;
				MotionLinks[ofst] = AR->Link[j + j];
				LinksDist[ofst] = AR->Link[j + j + 1];
			}
		}
		LimitedSetNewLinks(LimAreas, NLim, MotionLinks, LinksDist);
	}

	MinChX = 100000;
	MaxChX = 0;
	MinChY = 100000;
	MaxChY = 0;

	OnePartIterationNew(NeedITR);

	if (!TmpChanges)
	{
		NeedITR = 1600;
	}
	else
	{
		int NeedITR = 4500;
	}
}

void StopDynamicalTopology() {
	if (TmpMLinks)free(TmpMLinks);
	if (TmpMDist)free(TmpMDist);
	TmpMLinks = NULL;
	TmpMDist = NULL;
	NeedProcessTop = false;
	NChAreas = 0;
	if (MaxChAreas)free(ChAreas);
	ChAreas = NULL;
	CurIStart = 0;
	TmpChanges = 0;
	MaxChAreas = 0;
};

//------------------Roads tecnology------------------
class Road;
class Road {
public:
	Road();
	~Road();
	int MaxSegs;
	int NSegs;
	int CheckSumm;
	word* Segs;
	void Erase();
	void CreateCheckSumm();
	void CheckSize(int N);
	void CreateRandomWay(int Start, int Finish, int N);
	void EraseCrossing();
	void ErasePart(int Start, int Finish);
	bool Connect(int StartIndex, int EndIndex);
	void Simplify();
	//void CheckDanger();
	void CreateMostSafeWay();
	bool Compare(Road* ROAD);
	void View(int x, int y, byte c);
};
Road::Road() {
	MaxSegs = 0;
	NSegs = 0;
	Segs = NULL;
};
Road::~Road() {
	if (Segs)free(Segs);
	MaxSegs = 0;
	NSegs = 0;
	Segs = NULL;
};
void Road::Erase() {
	NSegs = 0;
};
void Road::CheckSize(int N) {
	if (N >= MaxSegs) {
		MaxSegs += 64;
		Segs = (word*)realloc(Segs, MaxSegs << 1);
	};
};
void Road::CreateRandomWay(int Start, int Finish, int N) {
	NSegs = 0;
	if (LinksDist[Start + Finish*NAreas] == 0xFFFF)return;
	CheckSize(1);
	Segs[0] = Start;
	NSegs = 1;
	int PreTop = -1;
	for (int i = 0; i < N; i++) {
		word Top = 0xFFFF;
		while (Top == 0xFFFF) {
			Top = (int(rando())*NAreas) >> 15;
			if (LinksDist[Start + Top*NAreas] == 0xFFFF || PreTop == Top || Top == Start || Top == Finish)Top = 0xFFFF;
		};
		CheckSize(NSegs + 1);
		Segs[NSegs] = Top;
		NSegs++;
		Connect(NSegs - 2, NSegs - 1);
		PreTop = Top;
	};
	CheckSize(NSegs + 1);
	Segs[NSegs] = Finish;
	NSegs++;
	Connect(NSegs - 2, NSegs - 1);
	EraseCrossing();
	Simplify();
	CreateCheckSumm();
};
bool Road::Connect(int Start, int End) {
	int N = 1;
	int CTop = Segs[Start];
	int ETop = Segs[End];
	bool Change = false;
	do {
		CTop = MotionLinks[ETop + CTop*NAreas];
		N++;
	} while (CTop != ETop&&CTop != 0xFFFF);
	if (CTop == 0xFFFF)return false;
	if (N != (End - Start + 1)) {
		//Need to expand
		CheckSize(NSegs + N - End + Start - 1);
		memmove(Segs + Start + N - 1, Segs + End, (NSegs - End) << 1);
		Change = true;
		NSegs += N - End + Start - 1;
	};
	CTop = Segs[Start];
	int ps = Start + 1;
	do {
		CTop = MotionLinks[ETop + CTop*NAreas];
		if (Segs[ps] != CTop) {
			Segs[ps] = CTop;
			Change = true;
		};
		ps++;
	} while (CTop != ETop&&CTop != 0xFFFF);
	return Change;
};
void Road::ErasePart(int Start, int Finish) {
	if (Segs&&Finish < NSegs - 1) {
		memmove(Segs + Start, Segs + Finish + 1, (NSegs - Finish - 1) << 1);
	};
	NSegs -= Finish - Start + 1;
};
void Road::EraseCrossing() {
	word NCross[512];
	assert(NAreas < 512);
STRT:
	memset(NCross, 0xFFFF, NAreas << 1);
	for (int i = 0; i < NSegs; i++) {
		int sg = Segs[i];
		if (NCross[sg] != 0xFFFF) {
			//erasing
			ErasePart(NCross[sg] + 1, i);
			goto STRT;
		}
		else NCross[sg] = i;
	};
};
void Road::CreateCheckSumm() {
	CheckSumm = NSegs << 19;
	for (int i = 0; i < NSegs; i++) {
		CheckSumm += Segs[i] << (i & 15);
	};
};
void Road::Simplify() {
	CreateCheckSumm();
	int PreSumm = CheckSumm;
	do {
		PreSumm = CheckSumm;
		for (int i = 0; i < NSegs - 4; i++) {
			Connect(i, i + 4);
		};
		CreateCheckSumm();
	} while (CheckSumm != PreSumm);
};
void Road::View(int x, int y, byte c) {
	int prevx = -1;
	int prevy = -1;
	for (int i = 0; i < NSegs - 1; i++) {
		Area* AR = TopMap + Segs[i];
		if (prevx != -1) {
			DrawLine(prevx, prevy, x + AR->x, y + AR->y, c);
		};
		prevx = x + AR->x;
		prevy = y + AR->y;
	};
};

extern int tmtmt;

//Water topology
word* WTopRef;
Area* WTopMap;
int   WNAreas;
int WMaxArea;
word* WMotionLinks;
word* WLinksDist;
#define MinWNorm (8<<(ADDSH-1))
bool AddWArea(short x, short y) {
	for (int i = 0; i < WNAreas; i++) {
		if (Norma(WTopMap[i].x - x, WTopMap[i].y - y) <= MinWNorm) {
			return false;
		};
	};
	if (WNAreas >= WMaxArea) {
		WMaxArea += 120;
		WTopMap = (Area*)realloc(WTopMap, WMaxArea * sizeof(Area));
	};
	Area* AR = WTopMap + WNAreas;
	AR->x = x;
	AR->y = y;
	AR->Importance = 0;
	AR->NTrees = 0;
	AR->NStones = 0;
	AR->MinesIdx = NULL;
	AR->NLinks = 0;
	AR->NMines = 0;
	AR->MaxLink = 6;
	AR->Link = new word[AR->MaxLink << 1];
	WNAreas++;
	return true;
};
void AddWLink(int N1, int N2) {
	Area* AR = &WTopMap[N1];
	int N = AR->NLinks;
	for (int i = 0; i < N; i++) {
		if (AR->Link[i + i] == N2)return;
	};
	if (AR->NLinks == AR->MaxLink) {
		AR->MaxLink += 4;
		AR->Link = (word*)realloc(AR->Link, AR->MaxLink << 2);
	};
	N = AR->NLinks;
	AR->Link[N + N] = N2;
	AR->Link[N + N + 1] = Norma(AR->x - WTopMap[N2].x, AR->y - WTopMap[N2].y);
	AR->NLinks++;
};
int WOneIteration() {
	int N = WNAreas*WNAreas;
	int ofs = 0;
	int NChanges = 0;
	for (int i = 0; i < WNAreas; i++)
		for (int j = 0; j < WNAreas; j++) {
			if (i != j) {
				Area* AR = WTopMap + i;
				int N = AR->NLinks;
				word* LINK = AR->Link;
				int CurDis = WLinksDist[ofs];
				for (int k = 0; k < N; k++) {
					word N2 = LINK[k + k];
					word adds = LINK[k + k + 1];
					if (N2 != j) {
						int ofs1 = j + N2*WNAreas;
						int dst = WLinksDist[ofs1];
						if (dst != 0xFFFF) {
							if (dst + adds < CurDis) {
								WLinksDist[ofs] = dst + adds;
								WMotionLinks[ofs] = N2;
								NChanges++;
							};
						};
					};
				};
			};
			ofs++;
		};
	return NChanges;
};
void ClearWLinkInfo() {
	memset(WMotionLinks, 0xFFFF, WNAreas*WNAreas * 2);
	memset(WLinksDist, 0xFFFF, WNAreas*WNAreas * 2);
	for (int i = 0; i < WNAreas; i++) {
		Area* AR = WTopMap + i;
		int k = AR->NLinks;
		for (int j = 0; j < k; j++) {
			int ofst = AR->Link[j + j] + i*WNAreas;
			WMotionLinks[ofst] = AR->Link[j + j];
			WLinksDist[ofst] = AR->Link[j + j + 1];
		};
	};
};
void CreateWLinkInfo() {
	ClearWLinkInfo();
	int N;
	int p = 0;
	do {
		ProcessMessages();
		N = WOneIteration();
		p++;
	} while (N&&p < 500);
};
void CreateWTopMap() {

	WNAreas = 0;
	memset(WTopRef, 0xFFFF, WTopLx*WTopLx * 2);
	for (int ix = 0; ix < WTopLx; ix++)
		for (int iy = 0; iy < WTopLx; iy++) {
			if (!MFIELDS[1].CheckBar((ix << WCLSH), (iy << WCLSH), WCLLX, WCLLX)) {
				WTopRef[ix + (iy << WTopSH)] = 0xFFFE;
			};
		};
	int mmx = msx >> (WCLSH - 1);
	int mmy = msy >> (WCLSH - 1);
	int N1 = 0;
	int N2 = 0;
	do {
		int x = (int(rando())*(mmx - 4)) >> 15;
		int y = (int(rando())*(mmy - 4)) >> 15;
		if (x > 4 && y > 4 && x < WTopLx - 4 && y < WTopLx - 4) {
			if (WTopRef[x + y*WTopLx] == 0xFFFE && WTopRef[x + 3 + y*WTopLx] == 0xFFFE
				&& WTopRef[x - 3 + y*WTopLx] == 0xFFFE && WTopRef[x + (y + 3)*WTopLx] == 0xFFFE
				&& WTopRef[x + (y - 3)*WTopLx] == 0xFFFE && WTopRef[x + 3 + (y + 3)*WTopLx] == 0xFFFE
				&& WTopRef[x - 3 + (y + 3)*WTopLx] == 0xFFFE && WTopRef[(x + 3) + (y - 3)*WTopLx] == 0xFFFE
				&& WTopRef[x - 3 + (y - 3)*WTopLx] == 0xFFFE) {
				N2 = 0;
				if (AddWArea(x + 1, y + 1))N1 = 0;
				else N1++;
			}
			else N2++;
		}
		else N2++;
	} while (N1 < 50 && N2 < 5000);
	//splitting...
	for (int i = 0; i < WNAreas; i++)WTopRef[WTopMap[i].x + int(WTopMap[i].y)*WTopLx] = i;
	bool Change = false;
	//word TempTop[TopLx*TopLy];
	Change = true;
	for (int i = 1; i < RRad&&Change; i++) {
		//memcpy(TempTop,TopRef,sizeof(TopRef));
		Change = false;
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for (int j = 0; j < WNAreas; j++) {
			int xx = WTopMap[j].x;
			int yy = WTopMap[j].y;
			for (int k = 0; k < N; k++) {
				int x = xx + xi[k];
				int y = yy + yi[k];
				if (x > 0 && y > 0 && x < mmx&&y < mmy) {
					int ofst = x + y*WTopLx;
					word ZZ = WTopRef[ofst];
					word ZU = WTopRef[ofst - WTopLx];
					word ZD = WTopRef[ofst + WTopLx];
					word ZL = WTopRef[ofst - 1];
					word ZR = WTopRef[ofst + 1];
					if (ZZ == 0xFFFE) {
						if (ZU == j || ZD == j || ZL == j || ZR == j ||
							WTopRef[ofst - WTopLx - 1] == j ||
							WTopRef[ofst - WTopLx + 1] == j ||
							WTopRef[ofst + WTopLx - 1] == j ||
							WTopRef[ofst + WTopLx + 1] == j) {
							WTopRef[ofst] = j;
							Change = true;
							ZZ = j;
						};
					};
					if (ZZ < 0xFFFE) {
						if (ZU != ZZ&&ZU < 0xFFFE) {
							AddWLink(ZZ, ZU);
							AddWLink(ZU, ZZ);
						};
						if (ZD != ZZ&&ZD < 0xFFFE) {
							AddWLink(ZZ, ZD);
							AddWLink(ZD, ZZ);
						};
						if (ZL != ZZ&&ZL < 0xFFFE) {
							AddWLink(ZZ, ZL);
							AddWLink(ZL, ZZ);
						};
						if (ZR != ZZ&&ZR < 0xFFFE) {
							AddWLink(ZZ, ZR);
							AddWLink(ZR, ZZ);
						};
					};
				};
			};
		};
	};
	ProcessMessages();
	if (WNAreas) {
		WMotionLinks = new word[WNAreas*WNAreas];
		WLinksDist = new word[WNAreas*WNAreas];
		memset(WMotionLinks, 0xFFFF, WNAreas*WNAreas * 2);
		memset(WLinksDist, 0xFFFF, WNAreas*WNAreas * 2);
		CreateWLinkInfo();
	}
	else {
		WMotionLinks = NULL;
		WLinksDist = NULL;
	};
	GTOP[0].LinksDist = LinksDist;
	GTOP[0].MotionLinks = MotionLinks;
	GTOP[0].NAreas = NAreas;
	GTOP[0].TopMap = TopMap;
	GTOP[0].TopRef = TopRef;
	GTOP[1].LinksDist = WLinksDist;
	GTOP[1].MotionLinks = WMotionLinks;
	GTOP[1].NAreas = WNAreas;
	GTOP[1].TopMap = WTopMap;
	GTOP[1].TopRef = WTopRef;
}

extern word TopAreasDanger[2048];
extern word WTopNear[64];
extern int NWTopNear;
void CBar(int x, int y, int Lx, int Ly, byte c);
extern int NIslands;

#define MaxIsl 64

extern int IslandX[MaxIsl];
extern int IslandY[MaxIsl];

extern word* TopIslands;
extern word* NearWater;

//Dynamic zones remarking
int SearchID(word* Set, int N, word ID)
{
	for (int i = 0; i < N; i++)
	{
		if (Set[i] == ID)
		{
			return i;
		}
	}
	return -1;
}

bool Stop = 0;
void EraseAllLinks(int Strat, int Fin);
struct OLDLINK {
	int  NAr;
	word* Links[2048];
	word NLinks[2048];
	word LinkBuf[40000];
	int LBPOS;
};
OLDLINK* OLN = NULL;
void SymmetrizeLinks() {
	for (int i = 0; i < NAreas; i++) {
		Area* AR = TopMap + i;
		for (int j = 0; j < AR->NLinks; j++) {
			int L = AR->Link[j + j];
			AddLink(L, i);
			AddLink(i, L);
		};
	};
}

void RemakeGroupOfAreas(word* Grp, int Na)
{
	if (!Na)
	{
		return;
	}

	byte* LSET = new byte[Na*Na];
	memset(LSET, 0, Na*Na);

	for (int q = 0; q < Na; q++)
	{
		int ID = Grp[q];
		Area* AR = TopMap + ID;
		int NL = AR->NLinks;
		for (int j = 0; j < NL; j++)
		{
			int ID1 = AR->Link[j + j];
			int qq = SearchID(Grp, Na, ID1);
			if (qq != -1)
			{
				LSET[qq + q*Na] = 1;
				LSET[qq*Na + q] = 1;
			}
		}
	}

	OLN = new OLDLINK;
	OLN->LBPOS = 0;

	for (int q = 0; q < NAreas; q++)
	{
		Area* AR = TopMap + q;
		OLN->NLinks[q] = AR->NLinks;
		OLN->Links[q] = OLN->LinkBuf + OLN->LBPOS;
		OLN->LBPOS += AR->NLinks;
		for (int j = 0; j < AR->NLinks; j++)
		{
			OLN->Links[q][j] = AR->Link[j + j];
		}
	}
	//clear
	int mmx = msx >> 1;
	int mmy = msy >> 1;
	for (int i = 0; i < Na; i++)
	{
		word ID = Grp[i];
		int minr = 1000;
		Area* AR = TopMap + ID;
		int x0 = AR->x;
		int y0 = AR->y;
		int Nclr = 0;
		for (int r = 0; r < RRad && r < minr + 3; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int N = Rarr[r].N;
			for (int j = 0; j < N; j++)
			{
				int x1 = x0 + xi[j];
				int y1 = y0 + yi[j];
				if (x1 > 0 && y1 > 0 && x1 < mmx&&y1 < mmy)
				{
					int ofst = x1 + (y1 << TopSH);
					if (TopRef[ofst] == ID) {
						TopRef[ofst] = 0xFFFE;
						minr = r;
					}
				}
			}
		}
		//start point
		int ofst = x0 + (y0 << TopSH);
		if (TopRef[ofst] == 0xFFFE)TopRef[ofst] = ID;
		//Erase links
		AR->NLinks = 0;
	}

	//filling areas and creating new links
	bool Change = true;
	for (int i = 1; i < RRad&&Change; i++) {
		Change = false;
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for (int m = 0; m < Na; m++) {
			int j = Grp[m];
			int xx = TopMap[j].x;
			int yy = TopMap[j].y;
			for (int k = 0; k < N; k++) {
				int x = xx + xi[k];
				int y = yy + yi[k];
				if (x > 0 && y > 0 && x < mmx&&y < mmy) {
					int ofst = x + y*TopLx;
					word ZZ = TopRef[ofst];
					word ZU = TopRef[ofst - TopLx];
					word ZD = TopRef[ofst + TopLx];
					word ZL = TopRef[ofst - 1];
					word ZR = TopRef[ofst + 1];
					if (ZZ == 0xFFFE) {
						if (ZU == j || ZD == j || ZL == j || ZR == j ||
							(ZU != 0xFFFF && ZL != 0xFFFF && TopRef[ofst - TopLx - 1] == j) ||
							(ZU != 0xFFFF && ZR != 0xFFFF && TopRef[ofst - TopLx + 1] == j) ||
							(ZD != 0xFFFF && ZL != 0xFFFF && TopRef[ofst + TopLx - 1] == j) ||
							(ZD != 0xFFFF && ZR != 0xFFFF && TopRef[ofst + TopLx + 1] == j)) {
							TopRef[ofst] = j;
							Change = true;
							ZZ = j;
						};
					};
					if (ZZ < 0xFFFE) {
						if (ZU != ZZ&&ZU < 0xFFFE) {
							AddLink(ZZ, ZU);
							AddLink(ZU, ZZ);
						};
						if (ZD != ZZ&&ZD < 0xFFFE) {
							AddLink(ZZ, ZD);
							AddLink(ZD, ZZ);
						};
						if (ZL != ZZ&&ZL < 0xFFFE) {
							AddLink(ZZ, ZL);
							AddLink(ZL, ZZ);
						};
						if (ZR != ZZ&&ZR < 0xFFFE) {
							AddLink(ZZ, ZR);
							AddLink(ZR, ZZ);
						};
					};
				};
			};
		};
	};
	SymmetrizeLinks();
	//setting new links
	//determining erased links
	for (int q = 0; q < Na; q++) {
		int ID = Grp[q];
		Area* AR = TopMap + ID;
		int NL = AR->NLinks;
		for (int j = 0; j < NL; j++) {
			int ID1 = AR->Link[j + j];
			int qq = SearchID(Grp, Na, ID1);
			if (qq != -1) {
				LSET[qq + q*Na] = 2;
				LSET[qq*Na + q] = 2;
			};
		};
	}

	int NBadLinks = 0;
	word BadLinks[1024];

	for (int q = 0; q < Na; q++) {
		for (int p = 0; p < q; p++) {
			if (LSET[q*Na + p] == 1 && NBadLinks < 512) {
				BadLinks[NBadLinks + NBadLinks] = Grp[q];
				BadLinks[NBadLinks + NBadLinks + 1] = Grp[p];
				NBadLinks++;
			};
		};
	};
	free(LSET);
	int NBAD = 0;

	for (int t = 0; t < NBadLinks; t++)
	{
		int C = BadLinks[t + t];
		int D = BadLinks[t + t + 1];
		EraseAllLinks(C, D);
		EraseAllLinks(D, C);
	}

	int NN = NAreas*NAreas;
	for (int q = 0; q < NN; q++)
	{
		if (LinksDist[q] == 0xFFFF)
		{
			MotionLinks[q] = 0xFFFF;
		}
	}
	free(OLN);
}

void OnePartIterationNew(int Nitr)
{
	if (Stop)
	{
		return;
	}

	int N = NAreas*NAreas;
	int ofs = CurIStart*NAreas;

	if (!CurIStart)
	{
		TmpChanges = 0;
	}

	for (int i = CurIStart; i < NAreas; i++)
	{
		for (int j = 0; j < NAreas; j++)
		{
			if (i != j)
			{
				Area* AR = TopMap + i;
				int N = AR->NLinks;
				word* LINK = AR->Link;
				int CurDis = LinksDist[ofs];
				for (int k = 0; k < N; k++)
				{
					word N2 = LINK[k + k];
					word adds = LINK[k + k + 1];
					if (N2 != j&&adds != 0xFFFF)
					{
						int ofs1 = j + N2*NAreas;
						int dst = LinksDist[ofs1];
						if (dst != 0xFFFF)
						{
							if (dst + adds < CurDis)
							{
								int ddd = dst + adds;
								LinksDist[ofs] = ddd;
								MotionLinks[ofs] = N2;
								TmpChanges++;
							}
						}
					}
				}
				Nitr--;
			}
			ofs++;
		}
		CurIStart = i;
		Nitr--;

		if (Nitr <= 0)
		{
			return;
		}
	}

	if (CurIStart == NAreas - 1)
	{
		CurIStart = 0;
	}
}

void EraseLinks(int X, int A) {
	int N = OLN->NLinks[A];
	word* LINKS = OLN->Links[A];
	for (int j = 0; j < N; j++) {
		int W = LINKS[j];
		int ofs = W*NAreas + X;
		int Next = MotionLinks[ofs];
		if (Next == A) {
			LinksDist[ofs] = 0xFFFF;
			EraseLinks(X, W);
		};
	};
};
void EraseAllLinks(int Start, int Fin) {
	int NS = Start*NAreas;
	LinksDist[Fin + NS] = 0xFFFF;
	EraseLinks(Fin, Start);
	for (int j = 0; j < NAreas; j++) {
		int Next = MotionLinks[NS + j];
		if (Next == Fin) {
			LinksDist[j + NS] = 0xFFFF;
			EraseLinks(j, Start);
		};
	};
	LinksDist[Start*NAreas + Fin] = 0xFFFF;
};
