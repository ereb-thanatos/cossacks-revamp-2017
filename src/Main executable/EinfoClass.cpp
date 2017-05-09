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
#include "Nucl.h"

#include <assert.h>
#include "Megapolis.h"

#include "MapSprites.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "Path.h"
#include "NewMon.h"
#include <math.h>
#include "StrategyResearch.h"

#include "Safety.h"
#include "ActiveScenary.h"
#include "Sort.h"
#include "ConstStr.h"
#include "EinfoClass.h"
extern int PeaceTimeLeft;
extern int SafeMLx;
extern int SafeMSH;
extern int TSH;
#define MAXPLAY 7
void EnemyInfo::Clear() {
	CloseSafeInfo();
	ClearPlaces();
	ClearIslands();
};
GlobalEnemyInfo::GlobalEnemyInfo() {
	memset(EINF, 0, sizeof(EINF));

};
GlobalEnemyInfo::~GlobalEnemyInfo() {
	Clear();
};
void ResearchIslands();
void GlobalEnemyInfo::Setup() {
	for (int i = 0; i < MAXPLAY; i++)if (EINF[i])return;

	for (int i = 0;i < MAXPLAY; i++)if (NATIONS[i].ThereWasUnit || NtNUnits[i]) {
		if (!EINF[i]) {
			byte ms = NATIONS[i].NMask;
			EnemyInfo* EIN = new EnemyInfo;
			EIN->ALLOCATE();
			EIN->Mask = ms;
			EIN->BasicNation = i;
			EIN->GAINF.Clear();
			EIN->InitInflMap();
			EIN->InitSuperMortiraCells();
			EIN->CreateEnmBuildList();
			EIN->CreateWallsList();
			EIN->CreateListOfDangerObjects();
			EIN->InitBuildSafety();
			EIN->InitSafeInfo();
			EIN->ResearchSafeCells(200, 400);
			for (int j = 0; j < MAXPLAY; j++)
				if ((NtNUnits[j] || NATIONS[j].ThereWasUnit) && ms&NATIONS[j].NMask)EINF[j] = EIN;
			ResearchIslands();
		};
	};
};
void GlobalEnemyInfo::Clear() {
	for (int i = 0; i < MAXPLAY; i++) {
		EnemyInfo* EIN = EINF[i];

		if (EIN) {
			EIN->GAINF.Clear();
			EIN->Clear();
			EIN->FREE();
			free(EIN);
			for (int j = 0; j < MAXPLAY; j++)
				if (EINF[j] == EIN)EINF[j] = NULL;
		};
	};
};
extern int tmtmt;

void GlobalEnemyInfo::Process() {
	
	int tt = tmtmt & 255;
	for (int i = 0; i < MAXPLAY; i++)if (tt == 9 + (i << 5) && EINF[i])
		EINF[i]->GAINF.ResearchArmyDistribution(EINF[i]->BasicNation);
	
	if ((tt & 31) == 9) {
		int K = tt >> 5;
		if (EINF[K])EINF[K]->ResearchHumanAttackPlaces();
	};
	
	if ((tt & 31) == 15) {
		int K = tt >> 5;
		if (EINF[K])EINF[K]->RegisterHumanShips();
	};
	
	if ((tt & 31) == 19) {
		int K = tt >> 5;
		if (EINF[K])EINF[K]->ResearchShipsGroups();
	};
	
	tt &= 63;
	for (int i = 0;i < MAXPLAY; i++)if (EINF[i]) {
		int dd = (i << 3);
		if (dd + 3 == tt)EINF[i]->CreateListOfDangerObjects();
		if (dd + 6 == tt)EINF[i]->RefreshSafeMap();
		if (dd + 4 == tt)EINF[i]->ResearchSafeCells(200, 400);
	};
	
	tt = tt & 7;
	if (tt < MAXPLAY&&EINF[tt]) {
		EINF[tt]->ProcessTow();
	};
	
};
int GetTopology(int x, int y);
extern City CITY[8];
bool GlobalEnemyInfo::FindNearestEnemy(byte NNUM, int* x, int* y, bool TowerFear, int Min, bool Defence) {
	if (PeaceTimeLeft || !(EINF[NNUM] && CITY[NNUM].CenterFound))return false;
	byte ms = EINF[NNUM]->Mask;
	int xx = *x;
	int yy = *y;
	int MyTop = GetTopology(xx, yy);
	if (MyTop >= 0xFFFE)return false;
	int TIND = MyTop*NAreas;
	int myx = xx >> 8;
	int myy = yy >> 8;
	int MINR = 0xFFFE;
	int ax = -1;
	int ay = -1;
	ArmyInfo* AINF = EINF[NNUM]->GAINF.AINF;
	int Na = EINF[NNUM]->GAINF.NArmy;
	for (int i = 0; i < Na; i++) {
		if (AINF->N >= Min) {
			int fx = (AINF->MinX + AINF->MaxX) >> 1;
			int fy = (AINF->MinY + AINF->MaxY) >> 1;
			int Top = GetTopology((fx << 8) + 128, (fy << 8) + 128);
			if (Top != 0xFFFF) {
				int dis = LinksDist[Top + TIND];
				if (dis < MINR) {
					MINR = dis;
					ax = fx;
					ay = fy;
				};
			};
		};
		AINF++;
	};
	if (ax == -1) {
		int bx = 0;
		int by = 0;
		int bdis = 0xFFFF;
		int TIND = MyTop*NAreas;
		for (int i = 0; i < MAXOBJECT; i++) {
			OneObject* OB = Group[i];
			if (OB && (!(OB->NMask&ms)) && !OB->Sdoxlo) {
				NewMonster* NM = OB->newMons;
				if ((!(NM->Building || NM->LockType || NM->Wall)) || NM->Capture || !OB->Ready) {
					int Top = GetTopology(OB->RealX >> 4, OB->RealY >> 4);
					if (Top != 0xFFFF) {
						int dis = LinksDist[Top + TIND];
						if (dis < bdis) {
							//*********Tmpporarely deleted************//
							//if((!TowerFear)||CheckPositionForDanger(OB->RealX>>4,OB->RealY>>4,OB->RZ+32)<1){
							bdis = dis;
							bx = OB->RealX >> 4;
							by = OB->RealY >> 4;
							//};
						};
					};
				};
			};
		};
		if (bdis < 0xFFFF) {
			*x = bx;
			*y = by;
			return true;
		}
		else return false;
	};
	if (Defence) {
		int cx = CITY[NNUM].CenterX >> 1;
		int cy = CITY[NNUM].CenterY >> 1;
		int R = Norma(ax - cx, ay - cy);
		if (R > 16) {
			int NN = 0;
			int MX = 30 << ADDSH;
			int xxx, yyy;
			do {
				xxx = cx - (rando() & 7) + 3;
				yyy = cy - (rando() & 7) + 3;
				NN++;
			} while (NN < 20 && (xxx < 0 || xxx >= MX || yyy < 0 || yyy >= MX));
			*x = (xxx << 8) + 128;
			*y = (yyy << 8) + 128;
			return NN < 20;
		};
	};
	*x = (ax << 8) + 128;
	*y = (ay << 8) + 128;
	return 1;
};
GlobalEnemyInfo GNFO;









void EnemyInfo::CreateListOfDangerObjects() {
	CreateWallsList();
	NDINF = 0;
	byte msk = Mask;
	DangerInfo* DF = DINF;
	for (int MID = 0; MID < MAXOBJECT; MID++) {
		OneObject* OB = Group[MID];
		if (OB&&OB->Ready && !(OB->Sdoxlo || OB->NMask&msk)) {
			if (OB->newMons->Usage == TowerID) {
				DF->ID = MID;
				DF->SN = OB->Serial;
				DF->Kind = 0;
				DF->Life = OB->Life;
				DF->MaxLife = OB->MaxLife;
				DF->UpgradeLevel = OB->SingleUpgLevel;
				AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
				DF->MinR = ADC->AttackRadius1[0];
				DF->MaxR = ADC->AttackRadius2[0];
				DF->x = OB->RealX >> 4;
				DF->y = OB->RealY >> 4;
				DF->z = OB->RZ;
				DF++;
				NDINF++;
				if (NDINF >= 40)return;
			};
		};
	};
};
void EnemyInfo::InitSuperMortiraCells() {
	memset(SupMortBestID, 0xFF, MAXCIOFS * 2);
	memset(SupMortLastTime, 0, MAXCIOFS * 2);
};
word EnemyInfo::ResearchCellForSupermortira(int cell, int MinDistance, int MaxDistance) {
	word ct = word(tmtmt);
	MinDistance <<= 4;
	MaxDistance <<= 4;
	int x0 = (int(cell&(VAL_MAXCX - 1)) << 11) + (64 * 16);
	int y0 = (int(cell >> VAL_SHFCX) << 11) + (64 * 16);
	int BESTR = 100000 * 16;
	word BESTMID = 0xFFFF;
	if (ct - SupMortLastTime[cell] > 129) {
		//need the new research
		//search the tower
		for (int i = 0; i < NDINF; i++) {
			word MID = DINF[i].ID;
			word SN = DINF[i].SN;
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB&&OB->Serial == SN && !OB->Sdoxlo) {
					int R = Norma(OB->RealX - x0, OB->RealY - y0);
					if (R > MinDistance&&R < MaxDistance&&R < BESTR) {
						BESTR = R;
						BESTMID = OB->Index;
					};
				};
			};
		};
		if (BESTMID == 0xFFFF) {
			//search other building
			for (int i = 0; i < NEnmBuild; i++) {
				word MID = EnmBuildList[i];
				word SN = EnmBuildSN[i];
				if (MID != 0xFFFF) {
					OneObject* OB = Group[MID];
					if (OB&&OB->Serial == SN && !OB->Sdoxlo) {
						byte use = OB->newMons->Usage;
						if (use != SkladID&&use != MelnicaID) {
							int R = Norma(OB->RealX - x0, OB->RealY - y0);
							if (R > MinDistance&&R < MaxDistance&&R < BESTR) {
								BESTR = R;
								BESTMID = OB->Index;
							};
						};
					};
				};
			};
			for (int p = 0; p < NEnWalls; p++) {
				int xx1 = (int(WallsX[p]) << 10) + 32 * 16;
				int yy1 = (int(WallsY[p]) << 10) + 32 * 16;
				int dr = Norma(xx1 - x0, yy1 - y0);
				if (dr > MinDistance&&dr < MaxDistance&&dr < BESTR) {
					int LI = GetLI(WallsX[p], WallsY[p]);
					WallCell* WC = WRefs[LI];
					if (WC&&WC->Visible) {
						OneObject* OB = Group[WC->OIndex];
						if (OB&&OB->Life < 3000) {
							BESTR = dr;
							BESTMID = OB->Index;
						};
					};
				};
			};
		};
		SupMortBestID[cell] = BESTMID;
		SupMortLastTime[cell] = ct;
		return BESTMID;
	}
	else {
		BESTMID = SupMortBestID[cell];
		if (BESTMID != 0xFFFF) {
			OneObject* OB = Group[BESTMID];
			if (OB&&OB->NewBuilding && !OB->Sdoxlo)return BESTMID;
			else SupMortBestID[cell] = 0xFFFF;
		};
		return 0xFFFF;
	};
};
int GetTopDistance(int xa, int ya, int xb, int yb);
word EnemyInfo::SearchDangerousPlaceForEnemy(int* utx, int* uty, int MINR, int MAXR) {
	if (!NAreas)return 0xFFFF;
	int tx = *utx;
	int ty = *uty;
	int cx = tx >> 1;
	int cy = ty >> 1;
	int maxx = msx >> 1;

	for (int r = 1; r < 7; r++) {
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		for (int j = 0; j < N; j++) {
			int xx = cx + xi[j];
			int yy = cy + yi[j];
			if (xx >= 0 && xx < maxx&&yy >= 0 && yy < maxx) {
				int cell1 = xx + (yy << VAL_SHFCX);
				if (ProtectionMap[cell1] & 128) {
					//it is safe for me
					word MID = ResearchCellForSupermortira(cell1, MINR, MAXR);
					if (MID != 0xFFFF) {
						//checking blocking
						int cx2 = xx << 3;
						int cy2 = yy << 3;
						int dx = -1;
						int dy = 0;
						if (!CheckBar(cx2, cy2, 8, 8)) {
							dx = rando() & 1;
							dy = rando() & 1;
						}
						else
							if (!CheckBar(cx2, cy2, 4, 4)) {
								dx = 0;
								dy = 0;
							}
							else
								if (!CheckBar(cx2 + 4, cy2, 4, 4)) {
									dx = 1;
									dy = 0;
								}
								else
									if (!CheckBar(cx2, cy2 + 4, 4, 4)) {
										dx = 0;
										dy = 1;
									}
									else
										if (!CheckBar(cx2 + 4, cy2 + 4, 4, 4)) {
											dx = 1;
											dy = 1;
										};
						if (dx != -1) {
							//checking the topological distance
							if (GetTopDistance(tx, ty, xx + xx + dx, yy + yy + dy) < 20) {
								*utx = xx + xx + dx;
								*uty = yy + yy + dy;
								return MID;
							};
						};
					};
				};
			};
		};
	};
	return 0xFFFF;
};
bool EnemyInfo::FindSafePlace(int* utx, int* uty) {
	int tx = *utx;
	int ty = *uty;
	int cx = tx >> 1;
	int cy = ty >> 1;
	int maxx = msx >> 1;

	for (int r = 1; r < 12; r++) {
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		for (int j = 0; j < N; j++) {
			int xx = cx + xi[j];
			int yy = cy + yi[j];
			if (xx >= 0 && xx < maxx&&yy >= 0 && yy < maxx) {
				int cell1 = xx + (yy << VAL_SHFCX);
				if (ProtectionMap[cell1] & 128) {
					//it is safe for me
					//checking blocking
					int cx2 = xx << 3;
					int cy2 = yy << 3;
					int dx = -1;
					int dy = 0;
					if (!CheckBar(cx2, cy2, 8, 8)) {
						dx = rando() & 1;
						dy = rando() & 1;
					}
					else
						if (!CheckBar(cx2, cy2, 4, 4)) {
							dx = 0;
							dy = 0;
						}
						else
							if (!CheckBar(cx2 + 4, cy2, 4, 4)) {
								dx = 1;
								dy = 0;
							}
							else
								if (!CheckBar(cx2, cy2 + 4, 4, 4)) {
									dx = 0;
									dy = 1;
								}
								else
									if (!CheckBar(cx2 + 4, cy2 + 4, 4, 4)) {
										dx = 1;
										dy = 1;
									};
					if (dx != -1) {
						//checking the topological distance
						if (GetTopDistance(tx, ty, xx + xx + dx, yy + yy + dy) < 30) {
							*utx = xx + xx + dx;
							*uty = yy + yy + dy;
							return true;
						};
					};
				};
			};
		};
	};
	return false;
};
word EnemyInfo::SearchBestEnemyAndPlaceForSupermortira(OneObject* OB, int* BestX, int* BestY) {
	int cx = OB->RealX >> 11;
	int cy = OB->RealY >> 11;
	if (cx < 0 || cx >= VAL_MAXCX || cy < 0 || cy >= VAL_MAXCX) {
		*BestX = OB->RealX;
		*BestY = OB->RealY;
		return 0xFFFF;
	};
	int maxx = msx >> 2;
	int cell = cx + (cy << VAL_SHFCX);
	//is the positon dangerous?
	bool SAFETY = 1;//(ProtectionMap[cell]&128)!=0;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	int MINR = ADC->AttackRadius1[0];
	int MAXR = ADC->AttackRadius2[0];
	//Can I shot now?
	if (SAFETY) {
		word MID = ResearchCellForSupermortira(cell, MINR, MAXR - 64);
		if (MID != 0xFFFF) {
			*BestX = OB->RealX;
			*BestY = OB->RealY;
			return MID;
		};
	};
	int tx = OB->RealX >> 10;
	int ty = OB->RealY >> 10;
	//need to search safe for me and dangerous for the enemy position
	word MID = SearchDangerousPlaceForEnemy(&tx, &ty, MINR, MAXR - 64);
	if (MID != 0xFFFF) {
		*BestX = (tx << 10) + (rando() & 1023);
		*BestY = (ty << 10) + (rando() & 1023);
		return MID;
	};
	//*BestX=OB->RealX;
	//*BestY=OB->RealY;
	return 0xFFFF;
};
void EnemyInfo::CreateEnmBuildList() {
	NEnmBuild = 0;
	byte msk = Mask;
	for (int MID = 0; MID < MAXOBJECT&&NEnmBuild < 128; MID++) {
		OneObject* OB = Group[MID];
		if (OB && (!(OB->NMask&msk)) && OB->NewBuilding && !OB->Wall) {
			NewMonster* NM = OB->newMons;
			if (!(NM->Farm || NM->Usage == TowerID)) {
				EnmBuildList[NEnmBuild] = OB->Index;
				EnmBuildSN[NEnmBuild] = OB->Serial;
				NEnmBuild++;
			};
		};
	};
};
void EnemyInfo::CreateProtectionMap() {
	byte MASK = 0xFE;
	memset(ProtectionMap, 0, MAXCIOFS);
	for (int i = 0; i < MAXOBJECT; i++) {
		OneObject* OB = Group[i];
		if (OB&&OB->NMask&MASK && !(OB->Sdoxlo || OB->NewBuilding || OB->newMons->Capture || OB->newMons->Behavior == 2)) {
			int cell = (OB->RealX >> 11) + (((OB->RealY) >> 11) << VAL_SHFCX);
			if (cell >= 0 && cell < MAXCIOFS) {
				if (ProtectionMap[cell] < 127)
					ProtectionMap[cell]++;
			};
		};
	};
	for (int i = 0;i < MAXCIOFS; i++) {
		int N = 0;
		if (i > 0)N += ProtectionMap[i - 1] & 127;
		if (i < MAXCIOFS - 1)N += ProtectionMap[i + 1] & 127;
		if (i > VAL_MAXCX)N += ProtectionMap[i - VAL_MAXCX] & 127;
		if (i < MAXCIOFS - VAL_MAXCX)N += ProtectionMap[i + VAL_MAXCX] & 127;
		if (N > 5) {
			ProtectionMap[i] |= 128;
			if (i > 0)ProtectionMap[i - 1] |= 128;
			if (i < MAXCIOFS - 1)ProtectionMap[i + 1] |= 128;
			if (i > VAL_MAXCX)ProtectionMap[i - VAL_MAXCX] |= 128;
			if (i < MAXCIOFS - VAL_MAXCX)ProtectionMap[i + VAL_MAXCX] |= 128;
		};
	};
};

void EnemyInfo::CreateWallsList() {
	NEnWalls = 0;
	int N1 = WSys.NClusters;
	for (int i = 0; i < N1; i++) {
		WallCluster* WCL = WSys.WCL[i];
		if (WCL && !((1 << WCL->NI)&Mask)) {
			int N2 = WCL->NCells;
			WallCell* CL = WCL->Cells;
			for (int j = 0; j < N2; j++) {
				if (CL->Visible) {
					int x = CL->x;
					int y = CL->y;
					int LI = GetLI(x, y);
					int TP_L = TopRef[LI - 1];
					int TP_R = TopRef[LI + 1];
					int TP_U = TopRef[LI - MaxLIX];
					int TP_D = TopRef[LI + MaxLIX];
					OneObject* WOB = NULL;
					if (CL->OIndex < 0xFFFE)WOB = Group[CL->OIndex];
					bool NeedDel = false;
					if (TP_L < 0xFFFE && TP_R < 0xFFFE) {
						//if(TP_L==0xFFFE||TP_R==0xFFFE)NeedDel=true;
						//else 
						if (TP_L != TP_R&&LinksDist[TP_L + TP_R*NAreas] > 47)NeedDel = true;
					}
					else
						if (TP_U < 0xFFFE && TP_D < 0xFFFE) {
							//if(TP_U==0xFFFE||TP_D==0xFFFE)NeedDel=true;
							//else 
							if (TP_U != TP_D&&LinksDist[TP_U + TP_D*NAreas] > 47)NeedDel = true;
						};
					if (NeedDel) {
						if (NEnWalls < 200) {
							WallsX[NEnWalls] = x;
							WallsY[NEnWalls] = y;
							NEnWalls++;
						};
						if (WOB)WOB->SafeWall = 0;
					}
					else if (WOB)WOB->SafeWall = 1;
				};
				CL++;
			};
		};
	};
};
int NTows = MAXTOW;
bool EnemyInfo::CheckTowersPresence() {
	for (int i = 0; i < MAXTOW; i++) {
		if (TowsID[i] != 0xFFFF)return true;
	};
	return false;
};
//[0..7:Damage][8..31:Influence mask]
void EnemyInfo::InitInflMap() {
	memset(InflMap, 0, TopLx*TopLx * 4);
	NTows = MAXTOW;
	memset(TowsID, 0xFF, sizeof TowsID);
	memset(TowsSN, 0xFF, sizeof TowsSN);
	memset(TowsMaxR, 0, sizeof TowsMaxR);
	CurTow = 0;
	CurR = 0;
	CurXip = 0;
	MaxTowR = 0;
};
void EnemyInfo::ClearTow(OneObject* OB) {
	//return;
	word MID = OB->Index;
	int maxtx = msx >> 1;
	int maxty = msy >> 1;
	for (int i = 0; i < NTows; i++) {
		if (TowsID[i] == MID&&TowsSN[i] == OB->Serial) {
			int cx = OB->RealX >> 10;
			int cy = OB->RealY >> 10;
			DWORD msk = 1 << (i + 8);
			DWORD amsk = ~msk;
			amsk &= 0xFFFFFF00;
			/*
			int LL=TopLx*TopLx;
			for(int j=0;j<LL;j++){
				InflMap[ofst]=InflMap[ofst]&amask;
			};
			*/
			int rmax = TowsMaxR[i];
			int ddang = TowDanger[i];
			for (int j = 0; j < rmax; j++) {
				int N = Rarr[j].N;
				char* xi = Rarr[j].xi;
				char* yi = Rarr[j].yi;
				for (int p = 0; p < N; p++) {
					int x1 = cx + xi[p];
					int y1 = cy + yi[p];
					if (x1 >= 0 && y1 >= 0 && x1 < maxtx&&y1 < maxty) {
						int ofst = x1 + (y1 << TopSH);
						DWORD dat = InflMap[ofst];
						if (dat&msk) {
							InflMap[ofst] = ((dat & 255) - ddang) + (dat&amsk);
						};
					};
				};
			};
			TowsID[i] = 0xFFFF;
			TowsSN[i] = 0xFFFF;
			TowDanger[i] = 0;
			TowsMaxR[i] = 0;
			//return;
		};
	};
	CheckSafeInfo();
};
void EnemyInfo::AddTow(OneObject* OB) {
	//return;
	if (OB->NMask&Mask)return;
	for (int i = 0; i < NTows; i++) {
		if (TowsID[i] == OB->Index)return;
	};
	for (int i = 0;i < NTows; i++) {
		if (TowsID[i] == 0xFFFF) {
			TowsID[i] = OB->Index;
			TowsSN[i] = OB->Serial;
			TowsMaxR[i] = 0;
			TowDanger[i] = 0;
			//checking for existance
			word idx = OB->Index;
			int N = 0;
			for (int j = 0; j < NTows; j++) {
				if (TowsID[i] == idx)N++;
			};

			return;
		};
	};
};
int CheckPointForDamageAbility(OneObject* OBJ, int x, int y, int z);
void EnemyInfo::ProcessTow() {
	rando();
	//return;
	if (TowsID[CurTow] == 0xFFFF) {
		CurTow++;
		if (CurTow >= NTows)CurTow = 0;
		return;
	};
	OneObject* OB = Group[TowsID[CurTow]];
	if (!(OB&&OB->newMons->Usage == TowerID)) {
		TowsID[CurTow] = 0xFFFF;
		
		return;
	};
	int xc = OB->RealX >> 10;
	int yc = OB->RealY >> 10;
	if (!CurR)MaxTowR = 12;
	int nn = 0;
	int maxtx = msx >> 1;
	int maxty = msy >> 1;

	DWORD msk = 1 << (CurTow + 8);
	DWORD amsk = (~msk) & 0xFFFFFF00;
	int ddam = TowDanger[CurTow];
	int ncp = 0;
	do {
		int N = Rarr[CurR].N;
		char* xi = Rarr[CurR].xi;
		char* yi = Rarr[CurR].yi;
		for (int j = CurXip; j < N; j++) {
			int x1 = xc + xi[j];
			int y1 = yc + yi[j];
			if (x1 >= 0 && y1 >= 0 && x1 < maxtx&&y1 < maxty) {
				int ofst = x1 + (y1 << TopSH);
				int dat = InflMap[ofst];
				int xx1 = (x1 << 6) + 32;
				int yy1 = (y1 << 6) + 32;
				int ccc = CheckPointForDamageAbility(OB, xx1, yy1, GetHeight(xx1, yy1) + 27);
				if (ccc) {
					if (dat&msk) {
						InflMap[ofst] = ((dat & 0xFF) - ddam) + (dat&amsk);
					};
				}
				else {
					if (!(dat&msk)) {
						InflMap[ofst] = ((dat & 0xFF) + ddam) + ((dat & 0xFFFFFF00) | msk);
					};
					if (MaxTowR < CurR + 4)MaxTowR = CurR + 4;
				};
				ncp++;
				if (ncp > 20) {
					CurXip = j + 1;
					
					return;
				};
			};
		};
		CurR++;
		CurXip = 0;
	} while (MaxTowR > CurR);
	//if(MaxTowR<=CurR){
	int CurDang = (5 * 100) / OB->PersonalDelay;
	if (CurDang != ddam) {
		for (int k = 0; k < MaxTowR; k++) {
			int N = Rarr[k].N;
			char* xi = Rarr[k].xi;
			char* yi = Rarr[k].yi;
			for (int p = 0; p < N; p++) {
				int x1 = xc + xi[p];
				int y1 = yc + yi[p];
				if (x1 >= 0 && y1 >= 0 && x1 < maxtx&&y1 < maxty) {
					int ofst = x1 + (y1 << TopSH);
					DWORD dat = InflMap[ofst];
					if (dat&msk) {
						InflMap[ofst] = ((dat & 255) + CurDang - ddam) + (dat & 0xFFFFFF00);
					};
				};
			};
		};
		TowDanger[CurTow] = CurDang;
	};
	TowsMaxR[CurTow] = MaxTowR;
	CurTow++;
	CurR = 0;
	if (CurTow >= NTows)CurTow = 0;
	if (rando() < 512) {
		int N = TopLx*TopLx;
		for (int i = 0; i < N; i++) {
			DWORD C = InflMap[i];
			if ((C & 255) == C)InflMap[i] = 0;
		};
	};
	//};
	
};
void GlobalEnemyInfo::AddTow(OneObject* OB) {
	for (int i = 0; i < MAXPLAY; i++)if (EINF[i])EINF[i]->AddTow(OB);
};
void GlobalEnemyInfo::ClearTow(OneObject* OB) {
	for (int i = 0; i < MAXPLAY; i++)if (EINF[i])EINF[i]->ClearTow(OB);
};



void EnemyInfo::InitBuildSafety() {
	LastBuildSafetyTime = 0;
	CreateBuildSafetyMap();
};
void EnemyInfo::CreateBuildSafetyMap() {
	memset(SafeMAP, 0, SafeMLx*SafeMLx);
	byte msk = Mask;
	for (int i = 0; i < MAXOBJECT; i++) {
		OneObject* OB = Group[i];
		if (OB && (!(OB->Sdoxlo || OB->NMask&msk || OB->Wall))) {
			int x = OB->RealX >> (4 + 10);
			int y = OB->RealY >> (4 + 10);
			if (x >= 0 && y >= 0 && x < SafeMLx - 1 && y < SafeMLx - 1) {
				SafeMAP[x + (y << SafeMSH)] = 1;
			};
		};
	};
	for (int ix = 1; ix < SafeMLx - 1; ix++) {
		for (int iy = 1; iy < SafeMLx - 1; iy++) {
			int ofs = ix + (iy << SafeMSH);
			if (SafeMAP[ofs] == 1) {
				if (!SafeMAP[ofs - 1])SafeMAP[ofs - 1] = 2;
				if (!SafeMAP[ofs + 1])SafeMAP[ofs + 1] = 2;
				if (!SafeMAP[ofs - SafeMLx])SafeMAP[ofs - SafeMLx] = 2;
				if (!SafeMAP[ofs + SafeMLx])SafeMAP[ofs + SafeMLx] = 2;
				if (!SafeMAP[ofs - SafeMLx + 1])SafeMAP[ofs - SafeMLx + 1] = 2;
				if (!SafeMAP[ofs + SafeMLx + 1])SafeMAP[ofs + SafeMLx + 1] = 2;
				if (!SafeMAP[ofs - SafeMLx - 1])SafeMAP[ofs - SafeMLx - 1] = 2;
				if (!SafeMAP[ofs + SafeMLx - 1])SafeMAP[ofs + SafeMLx - 1] = 2;
			};
		};
	};
};
extern int tmtmt;

void EnemyInfo::RefreshSafeMap() {
	if (LastBuildSafetyTime == 0)LastBuildSafetyTime = tmtmt;
	if (tmtmt - LastBuildSafetyTime > 256) {
		LastBuildSafetyTime = tmtmt;
		CreateBuildSafetyMap();
	};
};
byte EnemyInfo::GetSafeVal(int x, int y) {
	int x0 = x;
	int y0 = y;
	x >>= 3;
	y >>= 3;
	if (x >= 0 && y >= 0 && x < SafeMLx&&y < SafeMLx)return SafeMAP[x + (y << SafeMSH)];
	return 0;
};
void EnemyInfo::ShowSafetyInfo(int x, int y) {

};





void EnemyInfo::AddSafePoint(int x, int y, word Index, word SN, word Prio) {
	if (x > 0 && y > 0 && x < TopLx&&y < TopLx) {
		int idx = (x >> SafeCellSH) + ((y >> SafeCellSH) << SafeSX);
		SafeCellInfo* SCI = SCINF[idx];
		if (!SCI) {
			SCI = new SafeCellInfo[64];
			memset(SCI, 0xFFFF, sizeof(SafeCellInfo)*SafeCN);
			SCINF[idx] = SCI;
		};
		SCI += (x&SafeMask) + ((y&SafeMask) << SafeCellSH);
		if (SCI->Index == 0xFFFF || SCI->Prio > Prio) {
			SCI->Prio = Prio;
			SCI->Index = Index;
			SCI->SN = SN;
		};
	};
};
void EnemyInfo::ClearSafePoint(int x, int y, word Index) {
	if (x > 0 && y > 0 && x < TopLx&&y < TopLx) {
		int idx = (x >> SafeCellSH) + ((y >> SafeCellSH) << SafeSX);
		SafeCellInfo* SCI = SCINF[idx];
		if (!SCI)return;
		SCI += (x&SafeMask) + ((y&SafeMask) << SafeCellSH);
		if (SCI->Index == Index)SCI->Index = 0xFFFF;
	};
};
void EnemyInfo::InitSafeInfo() {
	memset(SCINF, 0, sizeof SCINF);
	CurRStage = 0;
};
void EnemyInfo::CloseSafeInfo() {
	for (int i = 0; i < SafeSize; i++) {
		if (SCINF[i])free(SCINF[i]);
	};
	InitSafeInfo();
};
void EnemyInfo::CheckSafeInfo() {
	
	SafeCellInfo** lpSCI = SCINF;
	int x0 = 0;
	int y0 = 0;
	for (int i = 0; i < SafeSize; i++) {
		if (lpSCI[0]) {
			SafeCellInfo* SCI = lpSCI[0];
			if (SCI) {
				
				for (int dy = 0; dy < SafeCLX; dy++) {
					for (int dx = 0; dx < SafeCLX; dx++) {
						
						if (SCI->Index != 0xFFFF) {
							OneObject* OB = Group[SCI->Index];
							
							if (OB && !(OB->Sdoxlo || OB->SafeWall)) {
								
								if (OB->Serial != SCI->SN) {
									SCI->Index = 0xFFFF;
									SCI->BusyIndex = 0xFFFF;
									
								}
								else {
									
									//check danger
									int xx1 = (x0 << SafeCellSH) + dx;
									int yy1 = (y0 << SafeCellSH) + dy;
									if (CheckBar((xx1 << 2), (yy1 << 2), 4, 4) || (InflMap[xx1 + (yy1 << TopSH)] & 255)) {
										SCI->Index = 0xFFFF;
										SCI->BusyIndex = 0xFFFF;
										
									}
									else {
										
										SCI->Prio = 32768 + (OB->Life >> 1);
									};
								};
							}
							else {
								SCI->Index = 0xFFFF;
								SCI->BusyIndex = 0xFFFF;
								
							};

						};
						if (SCI->BusyIndex != 0xFFFF) {
							OneObject* OB = Group[SCI->BusyIndex];
							if (OB && !(OB->Sdoxlo || OB->SafeWall)) {
								if (OB->Serial != SCI->BusySN) {
									SCI->BusyIndex = 0xFFFF;
									SCI->BusySN = 0xFFFF;
									
								};
							}
							else {
								SCI->BusyIndex = 0xFFFF;
								SCI->BusySN = 0xFFFF;
								
							};
						};
						SCI++;
					};
				};
			};
		};
		lpSCI++;
		x0++;
		if (x0 >= SafeLX) {
			x0 = 0;
			y0++;
		};
	};
};
SafeCellInfo* EnemyInfo::GetCellInfo(int x, int y) {
	if (x > 0 && y > 0 && x < TopLx&&y < TopLx) {
		int idx = (x >> SafeCellSH) + ((y >> SafeCellSH) << SafeSX);
		SafeCellInfo* SCI = SCINF[idx];
		if (SCI) {
			SCI += (x&SafeMask) + ((y&SafeMask) << SafeCellSH);
			if (SCI->Index == 0xFFFF)return NULL;
			else return SCI;

		}
		else return NULL;
	}
	else return NULL;
};
void EnemyInfo::RegisterSafePoint(OneObject* OB, int x, int y) {
	if (x > 0 && y > 0 && x < TopLx&&y < TopLx) {
		int idx = (x >> SafeCellSH) + ((y >> SafeCellSH) << SafeSX);
		SafeCellInfo* SCI = SCINF[idx];
		if (SCI) {
			SCI += (x&SafeMask) + ((y&SafeMask) << SafeCellSH);
			if (SCI->Index == 0xFFFF)return;
			else {
				SCI->BusyIndex = OB->Index;
				SCI->BusySN = OB->Serial;
				
			};
		};
	};
};
void EnemyInfo::InitResearchSafeCell() {
	CurRStage = 0;
};
void EnemyInfo::ResearchSafeObject(OneObject* OB, int MinR, int MaxR, int pstart) {
	int x0 = OB->RealX >> 4;
	int y0 = OB->RealY >> 4;
	int xx = x0 >> 6;
	int yy = y0 >> 6;
	if (xx <= 0 || yy <= 0 || xx >= TopLx - 1 || yy >= TopLy - 1)return;
	int tofs = xx + (yy << TopSH);

	int TU = TopRef[tofs - TopLx];
	int TD = TopRef[tofs + TopLx];
	int TR = TopRef[tofs + 1];
	int TL = TopRef[tofs - 1];
	//bool Clear=0;
	//if(!(TU<0xFFFE&&TD<0xFFFE&&LinksDist[TU+NAreas*TD]>30&&TU!=TD)){
	//	if(!(TR<0xFFFE&&TL<0xFFFE&&LinksDist[TL+NAreas*TR]>30&&TL!=TR))Clear=1;
	//};
//AddSafePt:
	int r0 = (MinR >> 6) - 1;
	if (r0 < 2)r0 = 2;
	int r1 = (MaxR >> 6) + 1;
	for (int rr = r0; rr <= r1; rr++) {
		char* xi = Rarr[rr].xi;
		char* yi = Rarr[rr].yi;
		int N = Rarr[rr].N;
		for (int p = 0; p < N; p++) {
			int xc = xx + xi[p];
			int yc = yy + yi[p];
			if (xc > 0 && yc > 0 && xc < TopLx&&yc < TopLx) {
				int ofst = xc + (yc << TopSH);
				if (TopRef[ofst] < 0xFFFE && !((InflMap[ofst] & 255) || CheckBar((xc << 2), (yc << 2), 4, 4))) {
					int rad = Norma(x0 - (xc << 6), y0 - (yc << 6));
					if (rad > MinR&&rad < MaxR) {
						AddSafePoint(xc, yc, OB->Index, OB->Serial, pstart + (OB->Life >> 1));
					};
				};
			};
		};
	};
};
void EnemyInfo::ResearchSafeCells(int MinR, int MaxR) {
	//COUNTER=GetTickCount();
	if (rando() < 4096)CheckSafeInfo();
	else {
		if (CurRStage < NEnWalls) {
			//walls
			WallCell* WC = WRefs[int(WallsX[CurRStage]) + (int(WallsY[CurRStage]) << TopSH)];
			if (WC) {
				OneObject* OB = Group[WC->OIndex];
				if (OB) {
					ResearchSafeObject(OB, MinR, MaxR, 32768);
				};
			};
		}
		else {
			//towers
			if (CurRStage < NEnWalls + NDINF) {
				DangerInfo* DIN = DINF + CurRStage - NEnWalls;
				OneObject* OB = Group[DIN->ID];
				if (OB&&DIN->SN == OB->Serial) {
					ResearchSafeObject(OB, MinR, MaxR, 32768);
				};
			}
			else {
				CurRStage = -1;
			};
		};
		CurRStage++;
	};
	//COUNTER=GetTickCount()-COUNTER;
};
void EnemyInfo::ResearchHumanAttackPlaces() {
	byte MASK = Mask;
	memset(TMAP, 0, TSX*TSX);
	memset(NUN, 0, TSX*TSX);
	for (int MID = 0; MID < MAXOBJECT; MID++) {
		OneObject* OB = Group[MID];
		if (OB && (!(OB->Sdoxlo || OB->NewBuilding || OB->Wall || OB->LockType || OB->NMask&MASK))) {
			int ofs = (OB->RealX >> TSSHIFT) + ((OB->RealY >> TSSHIFT) << TSH);
			if (ofs >= 0 && ofs < TSX*TSX) {
				if (OB->newMons->Artilery)NUN[ofs] += 8;
				NUN[ofs]++;
			};
		};
	};

	for (int MID = 0; MID < MAXOBJECT; MID++) {
		OneObject* OB = Group[MID];
		if (OB&&OB->NewBuilding && !(OB->Sdoxlo || OB->NMask&MASK)) {
			byte USE = OB->newMons->Usage;
			if (USE == TowerID) {
				int ofs = (OB->RealX >> TSSHIFT) + ((OB->RealY >> TSSHIFT) << TSH);
				if (ofs >= 0 && ofs < TSX*TSX) {
					TMAP[ofs] = 5;
					TMAP[ofs + 1] = 5;
					TMAP[ofs - 1] = 5;
					TMAP[ofs + TSX] = 5;
					TMAP[ofs + TSX] = 5;
				};
			};
		};
	};

	for (int x = 1; x < TSX - 1; x++) {
		for (int y = 1; y < TSX - 1; y++) {
			int ofs = x + (y << TSH);
			if (NUN[ofs] > 8) {
				TMAP[ofs] = 5;
				TMAP[ofs + 1] = 5;
				TMAP[ofs - 1] = 5;
				TMAP[ofs + TSX] = 5;
				TMAP[ofs + TSX] = 5;
			};
		};
	};

	for (int p = 5; p >= 2; p--) {
		int q = p - 1;
		for (int x = 1; x < TSX - 1; x++) {
			for (int y = 1; y < TSX - 1; y++) {
				int ofs = x + (y << TSH);
				if (TMAP[ofs] == p) {
					if (TMAP[ofs - 1] == 0)TMAP[ofs - 1] = q;
					if (TMAP[ofs + 1] == 0)TMAP[ofs + 1] = q;
					if (TMAP[ofs - TSX] == 0)TMAP[ofs - TSX] = q;
					if (TMAP[ofs + TSX] == 0)TMAP[ofs + TSX] = q;

					if (p > 3) {
						if (TMAP[ofs - 1 - TSX] == 0)TMAP[ofs - 1 - TSX] = q;
						if (TMAP[ofs + 1 - TSX] == 0)TMAP[ofs + 1 - TSX] = q;
						if (TMAP[ofs - 1 + TSX] == 0)TMAP[ofs - 1 + TSX] = q;
						if (TMAP[ofs + 1 + TSX] == 0)TMAP[ofs + 1 + TSX] = q;
					};

				};
			};
		};
	};

	NWTopNear = 0;
	for (int i = 0; i < WNAreas&&NWTopNear < 64; i++) {
		Area* AR = WTopMap + i;
		int xx = AR->x >> 2;
		int yy = AR->y >> 2;
		if (TMAP[xx + (yy << TSH)]) {
			WTopNear[NWTopNear] = i;
			NWTopNear++;
		};
	};
};

int GetWTopology(int x, int y, byte LTP);


int GetShipForce(byte Usage) {
	switch (Usage) {
	case IaxtaID:
		return 2;
	case ShebekaID:
	case FregatID:
		return 5;
	case LinkorID:
		return 12;
	case GaleraID:
		return 5;
	};
	return 0;
};
void EnemyInfo::ClearPlaces() {
	NWTopNear = 0;
};
void EnemyInfo::RegisterHumanShips() {
	if (!NAreas)return;
	NHSHIPS = 0;
	ShipsForce = 0;
	if (TopAreasDanger) {
		memset(TopAreasDanger, 0, WNAreas * 2);
	};
	for (int MID = 0; MID < MAXOBJECT&&NHSHIPS < 128; MID++) {
		OneObject* OB = Group[MID];
		if (OB && (!OB->Sdoxlo) && (!(OB->NMask&Mask)) && OB->LockType) {
			byte Usage = OB->newMons->Usage;
			int F = GetShipForce(Usage);
			if (OB->Transport&&OB->NInside > 10)F = 10;
			if (F) {
				SHIPS[NHSHIPS].ID = OB->Index;
				SHIPS[NHSHIPS].SN = OB->Serial;
				SHIPS[NHSHIPS].x = OB->RealX;
				SHIPS[NHSHIPS].y = OB->RealY;
				SHIPS[NHSHIPS].Force = F;
				ShipsForce += F;
				SHIPS[NHSHIPS].Life = OB->Life;
				int Top = GetWTopology(OB->RealX >> 4, OB->RealY >> 4, 1);
				if (Top >= 0 && Top < 0xFFFE && TopAreasDanger) {
					TopAreasDanger[Top] += F;
					Area* AR = WTopMap + Top;
					int N1 = AR->NLinks;
					for (int j = 0; j < N1; j++)
						TopAreasDanger[AR->Link[j + j]] += F;
				};
				NHSHIPS++;
			};
		};
	};
};
int EnemyInfo::GetMaxForceOnTheWay(int TopStart, int FinalTop) {
	if (!WNAreas)return 0;
	int MaxF = 0;
	while (TopStart != FinalTop) {
		int NextTop = WMotionLinks[FinalTop + TopStart*WNAreas];
		if (NextTop == 0xFFFF)return 0;
		int F = TopAreasDanger[NextTop];
		if (F > MaxF)MaxF = F;
		TopStart = NextTop;
	};
	if (MaxF > 30)MaxF = 30;
	return MaxF;
};
void EnemyInfo::ResearchShipsGroups() {
	NSGRP = 0;
	for (int i = 0; i < NHSHIPS; i++) {
		if (!NSGRP) {
			HShipsGroup* SGR = SGRP;
			HumanShip* HS = SHIPS + i;
			int xx = HS->x;
			int yy = HS->y;
			SGR->xs = xx;
			SGR->ys = yy;
			SGR->xL = xx;
			SGR->yL = yy;
			SGR->xR = xx;
			SGR->yR = yy;
			SGR->Ns = 1;
			SGR->Force = HS->Force;
			NSGRP++;
		}
		else {
			HumanShip* HS = SHIPS + i;
			int xx = HS->x;
			int yy = HS->y;
			HShipsGroup* SGR = SGRP;
			int j;
			for (j = 0; j < NSGRP; j++) {
				int xb = -1;
				int yb = -1;
				if (xx < SGR->xL)xb = SGR->xL;
				if (xx > SGR->xR)xb = SGR->xR;
				if (yy < SGR->yL)yb = SGR->yL;
				if (yy > SGR->yR)yb = SGR->yR;
				if (xb == -1 || Norma(xx - xb, yy - yb) < 1000 * 16) {
					SGR->xs += xx;
					SGR->ys += yy;
					if (xx < SGR->xL)SGR->xL = xx;
					if (yy < SGR->yL)SGR->yL = yy;
					if (xx > SGR->xR)SGR->xR = xx;
					if (yy > SGR->yR)SGR->yR = yy;
					SGR->Ns++;
					SGR->Force += HS->Force;
					j = NSGRP + 32;
				};
				SGR++;
			};
			if (j <= NSGRP + 32 && NSGRP < 64) {
				//need to create new group
				SGR = SGRP + NSGRP;
				SGR->xs = xx;
				SGR->ys = yy;
				SGR->xL = xx;
				SGR->yL = yy;
				SGR->xR = xx;
				SGR->yR = yy;
				SGR->Ns = 1;
				SGR->Force += HS->Force;
				NSGRP++;
			};
		};
	};
	HShipsGroup* SGR = SGRP;
	for (int i = 0;i < NSGRP; i++) {
		SGR->xs /= SGR->Ns;
		SGR->ys /= SGR->Ns;
		SGR->CTop = GetWTopology(SGR->xs >> 4, SGR->ys >> 4, 1);
		SGR++;
	};
};
int EnemyInfo::GetShipsForce(int x, int y, int r) {
	int F = 0;
	for (int i = 0; i < NHSHIPS; i++) {
		if (Norma(x - SHIPS[i].x, y - SHIPS[i].y) < r)F += SHIPS[i].Force;
	};
	return F;
};
extern int NIslands;
void EnemyInfo::ClearIslands() {
	NIslands = 0;
	NHSHIPS = 0;
	ShipsForce = 0;
};
