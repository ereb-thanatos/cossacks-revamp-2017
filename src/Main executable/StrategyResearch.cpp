
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
#include "StrategyResearch.h"
#include <crtdbg.h>
#include "TopoGraf.h"

#include "Safety.h"
#include "EinfoClass.h"

//#define NOSTRATINF
/*
#define DamSx (MAXCX>>1)
#define DamSy (MAXCY>>1)
#define DamShf (5+ADDSH)
#define DamSize (DamSx*DamSy)
class DamageMap(){
	word DIndex[DamSize];

};
*/
extern int COUNTER;
const byte StrCod[26] = { 0xFF,0xFF,0xFF,0xFF,0xFF,5,0xFF,0xFF,2,4,3,0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,1,2,2,0xFF,0xFF };
/*
void GlobalArmyInfo::ResearchArmyDistribution(byte NI){
#ifdef NOSTRATINF
	return;
#endif
	int tt=GetTickCount();
	int N=NtNUnits[NI];
	word* Units=NatList[NI];
	memset(ArmDistr,0xFFFF,sizeof ArmDistr);
	NCIN=0;
	NArmy=0;
	if(N){
		for(int i=0;i<N;i++){
			word MID=Units[i];
			if(MID!=0xFFFF){
				OneObject* OB=Group[MID];
				if(OB&&!OB->Sdoxlo){
					NewMonster* NM=OB->newMons;
					byte ID=StrCod[NM->Usage];
					if(ID!=0xFF){
						int xx1=OB->RealX>>10;
						int yy1=OB->RealY>>10;
						if(xx1>=0&&yy1>=0){
							int dang=0;
							if(xx1>0&&yy1>0){
								dang=InflMap[xx1+(yy1<<TopSH)];
							};
							int xx=xx1>>2;
							int yy=yy1>>2;
							int SIDX=xx+(yy<<StratSH);
							int sdx=ArmDistr[SIDX];
							CellInfo* SINFO;
							if(sdx==0xFFFF){
								if(NCIN>=MaxCIN){
									MaxCIN+=64;
									CIN=(CellInfo*)realloc(CIN,MaxCIN*sizeof CellInfo);
								};
								sdx=NCIN;
								ArmDistr[SIDX]=sdx;
								SINFO=CIN+sdx;
								memset(SINFO,0,sizeof CellInfo);
								SINFO->x=xx;
								SINFO->y=yy;
								NCIN++;
							}else SINFO=CIN+sdx;
							((word*)SINFO)[ID]++;
							SINFO->Total+=NM->Force;//++
						}else OB->Die();
					};
				};
			};
		};
		//assert(_CrtCheckMemory());
		if(NCIN){
			CellInfo* CINFO=CIN;
			for(int i=0;i<NCIN;i++){

				CINFO->ArmyID=i+8192;
				CINFO++;
			};
			bool change;
			//assert(_CrtCheckMemory());
			do{
				change=false;
				CellInfo* CINFO=CIN;
				for(int i=0;i<NCIN;i++){
					word AID=CINFO->ArmyID;
					if(AID!=0xFFFF){
						int cell=int(CINFO->x)+(int(CINFO->y)<<StratSH);
						word ARMD=ArmDistr[cell+1];
						if(ARMD!=0xFFFF){
							CellInfo* CI1=CIN+ARMD;
							if(CI1->ArmyID>AID){
								CI1->ArmyID=AID;
								change=true;
								//CI1->Changed=1;
							};
						};
						ARMD=ArmDistr[cell+StratLx];
						if(ARMD!=0xFFFF){
							CellInfo* CI1=CIN+ARMD;
							if(CI1->ArmyID>AID){
								CI1->ArmyID=AID;
								change=true;
								//CI1->Changed=1;
							};
						};
						if(cell>1){
							ARMD=ArmDistr[cell-1];
							if(ARMD!=0xFFFF){
								CellInfo* CI1=CIN+ARMD;
								if(CI1->ArmyID>AID){
									CI1->ArmyID=AID;
									change=true;
									//CI1->Changed=1;
								};
							};
						};
						if(cell>StratLx){
							ARMD=ArmDistr[cell-StratLx];
							if(ARMD!=0xFFFF){
								CellInfo* CI1=CIN+ARMD;
								if(CI1->ArmyID>AID){
									CI1->ArmyID=AID;
									change=true;
									//CI1->Changed=1;
								};
							};
						};
					};
					CINFO++;
				};
			}while(change);
			//assert(_CrtCheckMemory());
			//enumerating armies
			//COUNTER=GetTickCount()-tt;
			//return;
			NArmy=0;
			int CurArmy=0;
			for(i=0;i<NCIN;i++){
				word ARID=CIN[i].ArmyID;
				if(ARID>=8192){
					if(NArmy>=MaxArmy){
						MaxArmy+=32;
						AINF=(ArmyInfo*)realloc(AINF,MaxArmy*sizeof ArmyInfo);
					};
					ArmyInfo* Ari=AINF+CurArmy;
					memset(Ari,0,sizeof ArmyInfo);
					Ari->MinX=250;
					Ari->MinY=250;
					for(int j=0;j<NCIN;j++){
						if(CIN[j].ArmyID==ARID){
							CellInfo* CI=CIN+j;
							Ari->NCaval+=CI->NCaval;
							Ari->NMort+=CI->NMort;
							Ari->NPush+=CI->NPush;
							Ari->NStrel+=CI->NStrel;
							Ari->NSword+=CI->NSword;
							Ari->NTow+=CI->NTow;
							int xx=CI->x;
							int yy=CI->y;
							if(xx<Ari->MinX)Ari->MinX=xx;
							if(xx>Ari->MaxX)Ari->MaxX=xx;
							if(yy<Ari->MinY)Ari->MinY=yy;
							if(yy>Ari->MaxY)Ari->MaxY=yy;
							Ari->N+=CI->Total;//++;
							CI->ArmyID=CurArmy;
						};
					};
					CurArmy++;
					NArmy++;
				};
				//assert(_CrtCheckMemory());
			};
		};
	};
	//assert(_CrtCheckMemory());
	//COUNTER=GetTickCount()-tt;
};
*/
void GlobalArmyInfo::ResearchArmyDistribution(byte NI) {
#ifdef NOSTRATINF
	return;
#endif
	byte ms = 1 << NI;
	int tt = GetTickCount();
	//int N=NtNUnits[NI];
	//word* Units=NatList[NI];
	memset(ArmDistr, 0xFF, StratLx*StratLy * 2);
	if (!GNFO.EINF[NI])return;
	NCIN = 0;
	NArmy = 0;
	DWORD* __InflMap = GNFO.EINF[NI]->InflMap;
	//if(N){
	EnemyInfo* GN0 = GNFO.EINF[NI];
	for (int MID = 0; MID < MAXOBJECT; MID++) {
		OneObject* OB = Group[MID];
		if (OB && (!OB->Sdoxlo) && GN0 != GNFO.EINF[OB->NNUM]) {
			NewMonster* NM = OB->newMons;
			byte ID = StrCod[NM->Usage];
			if (ID != 0xFF) {
				int xx1 = OB->RealX >> 10;
				int yy1 = OB->RealY >> 10;
				if (xx1 >= 0 && yy1 >= 0) {
					int dang = 0;
					if (xx1 > 0 && yy1 > 0) {
						dang = __InflMap[xx1 + (yy1 << TopSH)];
					};
					int xx = xx1 >> 2;
					int yy = yy1 >> 2;
					int SIDX = xx + (yy << StratSH);
					int sdx = ArmDistr[SIDX];
					CellInfo* SINFO;
					if (sdx == 0xFFFF) {
						if (NCIN >= MaxCIN) {
							MaxCIN += 64;
							CIN = (CellInfo*)realloc(CIN, MaxCIN * sizeof CellInfo);
						};
						sdx = NCIN;
						ArmDistr[SIDX] = sdx;
						SINFO = CIN + sdx;
						memset(SINFO, 0, sizeof CellInfo);
						SINFO->x = xx;
						SINFO->y = yy;
						NCIN++;
					}
					else SINFO = CIN + sdx;
					((word*)SINFO)[ID]++;
					SINFO->Total += NM->Force;//++
				}
				else OB->Die();
			};
		};
	};
	//assert(_CrtCheckMemory());
	if (NCIN) {
		CellInfo* CINFO = CIN;
		for (int i = 0; i < NCIN; i++) {
			CINFO->ArmyID = i + 8192;
			CINFO++;
		};
		bool change;
		//assert(_CrtCheckMemory());
		do {
			change = false;
			CellInfo* CINFO = CIN;
			for (int i = 0; i < NCIN; i++) {
				word AID = CINFO->ArmyID;
				if (AID != 0xFFFF) {
					int cell = int(CINFO->x) + (int(CINFO->y) << StratSH);
					word ARMD = ArmDistr[cell + 1];
					if (ARMD != 0xFFFF) {
						CellInfo* CI1 = CIN + ARMD;
						if (CI1->ArmyID > AID) {
							CI1->ArmyID = AID;
							change = true;
							//CI1->Changed=1;
						};
					};
					ARMD = ArmDistr[cell + StratLx];
					if (ARMD != 0xFFFF) {
						CellInfo* CI1 = CIN + ARMD;
						if (CI1->ArmyID > AID) {
							CI1->ArmyID = AID;
							change = true;
							//CI1->Changed=1;
						};
					};
					if (cell > 1) {
						ARMD = ArmDistr[cell - 1];
						if (ARMD != 0xFFFF) {
							CellInfo* CI1 = CIN + ARMD;
							if (CI1->ArmyID > AID) {
								CI1->ArmyID = AID;
								change = true;
								//CI1->Changed=1;
							};
						};
					};
					if (cell > StratLx) {
						ARMD = ArmDistr[cell - StratLx];
						if (ARMD != 0xFFFF) {
							CellInfo* CI1 = CIN + ARMD;
							if (CI1->ArmyID > AID) {
								CI1->ArmyID = AID;
								change = true;
								//CI1->Changed=1;
							};
						};
					};
				};
				CINFO++;
			};
		} while (change);
		//assert(_CrtCheckMemory());
		//enumerating armies
		//COUNTER=GetTickCount()-tt;
		//return;
		NArmy = 0;
		int CurArmy = 0;
		for (int i = 0; i < NCIN; i++) {
			word ARID = CIN[i].ArmyID;
			if (ARID >= 8192) {
				if (NArmy >= MaxArmy) {
					MaxArmy += 32;
					AINF = (ArmyInfo*)realloc(AINF, MaxArmy * sizeof ArmyInfo);
				};
				ArmyInfo* Ari = AINF + CurArmy;
				memset(Ari, 0, sizeof ArmyInfo);
				Ari->MinX = 250;
				Ari->MinY = 250;
				for (int j = 0; j < NCIN; j++) {
					if (CIN[j].ArmyID == ARID) {
						CellInfo* CI = CIN + j;
						Ari->NCaval += CI->NCaval;
						Ari->NMort += CI->NMort;
						Ari->NPush += CI->NPush;
						Ari->NStrel += CI->NStrel;
						Ari->NSword += CI->NSword;
						Ari->NTow += CI->NTow;
						int xx = CI->x;
						int yy = CI->y;
						if (xx < Ari->MinX)Ari->MinX = xx;
						if (xx > Ari->MaxX)Ari->MaxX = xx;
						if (yy < Ari->MinY)Ari->MinY = yy;
						if (yy > Ari->MaxY)Ari->MaxY = yy;
						Ari->N += CI->Total;//++;
						CI->ArmyID = CurArmy;
					};
				};
				CurArmy++;
				NArmy++;
			};
			//assert(_CrtCheckMemory());
		};
	};
};
void GlobalArmyInfo::Clear() {
#ifdef NOSTRATINF
	return;
#endif
	NCIN = 0;
	NArmy = 0;
	MaxArmy = 0;
	MaxCIN = 0;
	if (CIN) {
		free(CIN);
		CIN = NULL;
	};
	if (AINF) {
		free(AINF);
		AINF = NULL;
	};
	memset(ArmDistr, 0xFF, StratLx*StratLy * 2);
};
GlobalArmyInfo::GlobalArmyInfo() {
#ifdef NOSTRATINF
	return;
#endif
	NCIN = 0;
	NArmy = 0;
	MaxArmy = 0;
	MaxCIN = 0;
	CIN = NULL;
	AINF = NULL;
	//memset(ArmDistr,0xFF,StratLx*StratLy*2);
};
GlobalArmyInfo::~GlobalArmyInfo() {
#ifdef NOSTRATINF
	return;
#endif
	Clear();
};
void GlobalArmyInfo::Show(int x, int y) {
#ifdef NOSTRATINF
	return;
#endif
	if (!NCIN)return;
	for (int i = 0; i < NCIN; i++) {
		CellInfo* CI = CIN + i;
		int col = 0xD0 + ((CI->ArmyID & 7) << 2);
		int xx = x + (int(CI->x) << 2);
		int yy = y + (int(CI->y) << 2);
		DrawLine(xx, yy, xx + 4, yy, col);
		DrawLine(xx, yy + 4, xx + 4, yy + 4, col);
		DrawLine(xx, yy, xx, yy + 4, col);
		DrawLine(xx + 4, yy, xx + 4, yy + 4, col);
	};
};