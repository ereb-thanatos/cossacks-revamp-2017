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
#include "Topograf.h"
extern MotionField UnitsField;
int SIGN(int x) {
	if (x < 0)return -1;
	if (x > 0)return 1;
	return 0;
};
word GetDir(int, int);
char GetBestDirection(MotionField* MFI, int xd, int yd, int LX, char CurDir) {
	int resdir = 0;
	int NDir = 0;
	int tdx = 0;
	int tdy = 0;
	for (int D0 = 0; D0 < 256; D0 += 8) {
		int DY = TSin[D0] >> 5;
		int DX = TCos[D0] >> 5;
		if (MFI->CheckBar(xd + DX, yd + DY, LX, LX)) {
			tdx += DX;
			tdy += DY;
			NDir++;
		};
	};
	if (!NDir)return CurDir;
	resdir = GetDir(tdx, tdy);
	char dd = resdir - CurDir;
	//if(abs(dd)>64)return CurDir;
	//else 
	return resdir;

};
bool CheckUnloadProcess(OneObject* OB);
void CreateUnloadProcess(OneObject* OB, int x, int y);
void RetLock(OneObject* OB) {
	int LX = OB->Lx;
	MotionField* MFI = MFIELDS + OB->LockType;
	if (OB->GLock)MFI->BSetBar(OB->x, OB->y, LX);
	if (OB->LLock)UnitsField.BSetBar(OB->x, OB->y, LX);
};
bool FindCostPosition(OneObject* OB, int* XDest, int* YDest, char* Direction) {
	int LX = OB->Lx;
	MotionField* MFI = MFIELDS + OB->LockType;
	if (OB->GLock)MFI->BClrBar(OB->x, OB->y, LX);
	if (OB->LLock)UnitsField.BClrBar(OB->x, OB->y, LX);
	int LX2 = (LX) >> 1;
	int yd = (*YDest) - LX2;
	int xd = (*XDest) - LX2;
	int xs = OB->x;
	int ys = OB->y;
	int adx = abs(xs - xd);
	int ady = abs(ys - yd);
	int sx = SIGN(xs - xd);
	int sy = SIGN(ys - yd);
	if (!(adx || ady)) {
		RetLock(OB);
		return false;
	};
	//if(!MFI->CheckBar(xd,yd,LX,LX))return false;
	char DIRC = GetDir(xd - xs, yd - ys);
	if (adx > ady) {
		int cumm = -adx >> 1;
		while (xd != xs) {
			if (!MFI->CheckBar(xd, yd, LX, LX)) {
				*XDest = xd + LX2;
				*YDest = yd + LX2;
				*Direction = GetBestDirection(MFI, xd, yd, LX, DIRC);
				RetLock(OB);
				return true;
			};
			xd += sx;
			cumm += ady;
			if (cumm >= 0) {
				yd += sy;
				cumm -= adx;
			};
		};
	}
	else {
		int cumm = -ady >> 1;
		while (yd != ys) {
			if (!MFI->CheckBar(xd, yd, LX, LX)) {
				*XDest = xd + LX2;
				*YDest = yd + LX2;
				*Direction = GetBestDirection(MFI, xd, yd, LX, DIRC);
				RetLock(OB);
				return true;
			};
			yd += sy;
			cumm += adx;
			if (cumm >= 0) {
				xd += sx;
				cumm -= ady;
			};
		};
	};
	if (!MFI->CheckBar(xs, ys, LX, LX)) {
		*XDest = xd + LX2;
		*YDest = yd + LX2;
		*Direction = GetBestDirection(MFI, xd, yd, LX, DIRC);
		RetLock(OB);
		return true;
	};
	RetLock(OB);
	return false;
};
void WaterNewMonsterSendToLink(OneObject* OB);
bool SearchTransportPlace(int* x, int* y, int Lx) {
	int x0 = *x;
	int y0 = *y;
	for (int r = 0; r < 40; r++) {
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int   N = Rarr[r].N;
		for (int p = 0; p < N; p++) {
			int xx = x0 + xi[p];
			int yy = y0 + yi[p];
			if (!MFIELDS[1].CheckBar(xx, yy, Lx, Lx)) {
				*x = xx;
				*y = yy;
				return true;
			};
		};
	};
	return false;
};
bool ParkWaterNewMonster(OneObject* OB, int x, int y, byte Prio, byte OrdType)
{
	//if(OB->CheckOrderAbility(0x1234,0))return true;
	OB->DstX = -1;
	NewMonster* NM = OB->newMons;
	if (NM->Building)
	{
		return false;
	}
	if (OB->LockType != 1)
	{
		return false;
	}
	MotionField* MFI = MFIELDS + OB->LockType;
	int LX = OB->Lx;
	int xx = (x >> 8) - (LX >> 1);
	int yy = (y >> 8) - (LX >> 1);
	int xx1 = xx;
	int yy1 = yy;
	if (OB->GLock)
	{
		MFI->BClrBar(OB->x, OB->y, LX);
	}
	if (!MFI->CheckBar(xx1 - 1, yy1 - 1, LX + 2, LX + 2))
	{
		if (OB->GLock)
		{
			MFI->BSetBar(OB->x, OB->y, LX);
		}
		return false;
	}
	if (!SearchTransportPlace(&xx1, &yy1, LX))
	{
		return true;
	}
	if (OB->PrioryLevel > Prio)
	{
		return false;
	}

	Order1* Or1 = OB->CreateOrder(OrdType);
	if (!int(Or1))
	{
		return false;
	}

	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 12;
	Or1->OrderTime = 0;
	Or1->DoLink = &WaterNewMonsterSendToLink;
	Or1->info.MoveToXY.x = xx1;
	Or1->info.MoveToXY.y = yy1;

	Or1->info.MoveToXY.PrevDist = 0;
	Order1* LOR = OB->LocalOrder;

	OB->UnBlockUnit();

	MakeOrderSound(OB, 2);

	return true;
}

void RunLeftVeslo(OneObject* OB, bool State);
void RunRightVeslo(OneObject* OB, bool State);
void WaterNewMonsterSendToLink(OneObject* OB) {
	OB->PrioryLevel = OB->LocalOrder->PrioryLevel;
	OB->NewState = 0;
	OB->DstX = 0;
	int xx1 = OB->LocalOrder->info.MoveToXY.x;
	int yy1 = OB->LocalOrder->info.MoveToXY.y;
	int tt = OB->LocalOrder->info.MoveToXY.PrevDist;
	int distan = OB->DistTo(xx1, yy1);
	int LX = OB->Lx;
	if (tt) {
		int MRot = 2;//OB->newMons->MinRotator;
		//need only to rotate
		char dir1 = GetBestDirection(MFIELDS + 1, xx1, yy1, LX, OB->RealDir);
		char ddir = dir1 - OB->RealDir;
		if (abs(ddir) <= MRot) {
			OB->RealDir = dir1;
			OB->GraphDir = OB->RealDir;
			OB->DeleteLastOrder();
			if (OB->newMons->Transport) {
				int xx1 = OB->RealX + int(TCos[OB->RealDir]) * 16;
				int yy1 = OB->RealY + int(TSin[OB->RealDir]) * 16;
				if (!MFIELDS->CheckBar((xx1 >> 8) - 2, (yy1 >> 8) - 2, 4, 4)) {
					OB->DstX = xx1;
					OB->DstY = yy1;
				};
			};
			return;
		}
		else {
			if (ddir > 0)OB->RealDir += MRot;
			else OB->RealDir -= MRot;
		};	OB->GraphDir = OB->RealDir;
	}
	else {
		int xxx = OB->x;
		int yyy = OB->y;
		if (OB->GLock)MFIELDS[1].BClrBar(xxx, yyy, LX);
		if (distan < 8 && MFIELDS[1].CheckBar(xxx - 2, yyy - 2, LX + 4, LX + 4)) {
			OB->DeletePath();
			OB->DestX = -1;
			OB->LocalOrder->info.MoveToXY.PrevDist = 1;
		}
		else {
			OB->CreatePath(xx1, yy1);
		}
		if (OB->GLock)MFIELDS[1].BSetBar(xxx, yyy, LX);
	};
	/*
	int distan=OB->DistTo(xx1,yy1);
	int LX2=OB->Lx>>1;
	MotionField* MFI=MFIELDS+OB->LockType;
	if(distan<12&&tt){
		//if(CheckUnloadProcess(OB))CreateUnloadProcess(OB,xx1,yy1);
		if(OB->GLock)MFI->BClrBar(OB->x,OB->y,OB->Lx);
		bool LOCKR=MFI->CheckBar(OB->x-1,OB->y-1,OB->Lx+2,OB->Lx+2);
		if(OB->GLock)MFI->BSetBar(OB->x,OB->y,OB->Lx);
		if(LOCKR){
			OB->LocalOrder->info.MoveToXY.PrevDist=0;
			tt=0;
			OB->DeletePath();
			OB->DestX=0;
			//OB->Speed=0;
			char dirr=OB->LocalOrder->info.MoveToXY.Times;
			dirr=GetBestDirection(MFI,OB->x,OB->y,OB->Lx,dirr);
			OB->LocalOrder->info.MoveToXY.Times=dirr;
			return;
		};
	};
	if(OB->DistTo(xx1,yy1)<=1){
		OB->LocalOrder->info.MoveToXY.PrevDist=0;
		char dirr=OB->LocalOrder->info.MoveToXY.Times;
		char Ddir=dirr-OB->RealDir;
		if(abs(Ddir)<4){
			RunLeftVeslo(OB,false);
			RunRightVeslo(OB,false);
			if(int(OB->LocalOrder)){
				Order1* Loc1=OB->LocalOrder->NextOrder;
				OB->FreeOrdBlock(OB->LocalOrder);
				OB->LocalOrder=Loc1;
			};
			//Is it on the cost?
			if(OB->newMons->Transport){
				int xx1=OB->RealX+int(TCos[OB->RealDir])*16;
				int yy1=OB->RealY+int(TSin[OB->RealDir])*16;
				if(!MFIELDS->CheckBar((xx1>>8)-2,(yy1>>8)-2,5,5)){
					OB->DstX=xx1;
					OB->DstY=yy1;
				};
			};
		}else{
			if(Ddir>0){
				OB->RealDir+=3;
				OB->GraphDir=OB->RealDir;
				RunLeftVeslo(OB,false);
				RunRightVeslo(OB,true);
			}else{
				OB->RealDir-=3;
				OB->GraphDir=OB->RealDir;
				RunLeftVeslo(OB,true);
				RunRightVeslo(OB,false);
			};
		};
	}else OB->CreatePath(xx1-LX2,yy1-LX2);
	*/
};
//transporting of units
#define TR_LASY      0
#define TR_GOTO_COST 1
#define TR_LOADING   2
char* TransHint = "Прервать перевозку.";
class TransProcess {
public:
	word* Units;
	word* Serials;
	byte* OnTheWay;
	int   Reserv;
	int   NUnits;
	word  TransportID;
	word  TransportSN;
	byte  Nation;
	int   FinX;
	int   FinY;
	int   StartX;
	int   StartY;
	int   ConcX;
	int   ConcY;
	int   ConcRadius;
	int   MidPX;
	int   MidPY;
	int   FinMidX;
	int   FinMidY;
	int   FinConcX;
	int   FinConcY;
	bool  MidPresent : 1;
	bool  ConcPresent : 1;
	bool  NearCost : 1;
	bool  StartPointFound : 1;
	bool  MovingToFinalCost : 1;
	bool  MovingToCost : 1;
	bool  FinalPointPresent : 1;
	bool  AbsorbUnits : 1;
	bool  ExtractUnits : 1;
	bool  FinMidPresent : 1;
	bool  Waiting : 1;
	bool  ReadyToDrive : 1;
	bool  Used : 1;
	GOrder* GLOB;
	//---------------------------methods------------------//
	TransProcess();
	void Init(word TransportID);
	void AddUnit(word UnitID);
	bool ExcludeUnit(word UnitID);
	bool CheckUnit(word UnitID);
	void RefreshUnits();
	void Process();
	void Demobilisation();
	bool FindStartPoint();
	bool FindConcPoint();
	bool CheckNearCost();
	bool SetFinalPoint(int x, int y);
	int FindNearestUnitIndex(int x, int y);
	bool CheckUnloadAbility();
	bool CheckAbsorbAbility();
	//-------------checking&disconnection routines
	bool CheckDisconnectionAbility(OneObject* OB, int LParam, int RParam);
	bool Disconnect(OneObject* OB, int LParam, int RParam);
	void KillAnyway();
};

bool TransProcess::CheckDisconnectionAbility(OneObject* OB, int LParam, int RParam) {
	if (OB->Index == TransportID) {
		if (OB->delay)return false;
		int OnWay = 0;
		for (int i = 0; i < NUnits; i++) {
			word MID = Units[i];
			if (MID&&OnTheWay[i])OnWay++;
		};
		if (OnWay || ExtractUnits)return false;
		else return true;
	}
	else {
		word uind = 0xFFFF;
		for (int i = 0; i < NUnits; i++) {
			word MID = Units[i];
			if (MID == OB->Index)uind = i;
		};
		if (uind != 0xFFFF) {
			if (OnTheWay[uind])return false;
		};
		return true;
	};
};
bool TransProcess::Disconnect(OneObject* OB, int LParam, int RParam) {
	if (LParam == 0x1234)return true;
	if (OB->Index == TransportID) {
		KillAnyway();
	}
	else {
		word uind = 0xFFFF;
		for (int i = 0; i < NUnits; i++) {
			word MID = Units[i];
			if (MID == OB->Index)uind = i;
		};
		if (uind != 0xFFFF) {
			Units[uind] = 0xFFFF;
		};
	};
	return true;
};
void TransProcess::KillAnyway() {
	Used = false;
	int R = ConcRadius << 8;
	int R2 = R >> 1;
	int CX = ConcX << 8;
	int CY = ConcY << 8;
	for (int i = 0; i < NUnits; i++) {
		word MID = Units[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			OB->GlobalOrder = NULL;
			if (OnTheWay[i]) {
				OB->GlobUnlock();
				int xi = CX + ((R*rando()) >> 15) - R2;
				int yi = CY + ((R*rando()) >> 15) - R2;
				OB->NewMonsterSendTo(xi, yi, 0, 0);
				OB->ClearOrderedUnlimitedMotion(2);
				OB->NewMonsterSendTo(xi + (rando() & 7) - 3, yi + (rando() & 7) - 3, 0, 2);
				OB->GlobLock();
			};
		};
	};
	if (TransportID != 0xFFFF) {
		OneObject* TRA = Group[TransportID];
		TRA->GlobalOrder = NULL;
	};
	GLOB->Data = NULL;
	free(GLOB);
};
bool TRP_CheckDisconnectionAbility(OneObject* OB, GOrder* GOR, int LParam, int RParam) {
	TransProcess* TRP = (TransProcess*)GOR->Data;
	return TRP->CheckDisconnectionAbility(OB, LParam, RParam);
};
bool TRP_Disconnect(OneObject* OB, GOrder* GOR, int LParam, int RParam) {
	TransProcess* TRP = (TransProcess*)GOR->Data;
	return TRP->Disconnect(OB, LParam, RParam);
};
bool TRP_KillOrder(OneObject* OB, GOrder* GOR, int LParam, int RParam) {
	TransProcess* TRP = (TransProcess*)GOR->Data;
	TRP->KillAnyway();
	return true;
};
TransProcess::TransProcess() {
	Used = false;
	GLOB = false;
};
void TRP_LHandle(int Param) {
	TransProcess* TRP = (TransProcess*)Param;
	if (TRP->Used)TRP->KillAnyway();
};
int TRP_IconInfo(GOrder* GOR, int IcoIndex, OneObject* OB, GlobalIconInfo* GIN) {
	TransProcess* TRP = (TransProcess*)GOR->Data;
	if (OB->Index == TRP->TransportID) {
		GIN->IconSpriteID = -1;
		GIN->HPLeft = &TRP_LHandle;
		GIN->HPRight = NULL;
		GIN->LParam = int(TRP);
		GIN->RParam = 0;
		GIN->Hint = TransHint;
		return 1;
	}
	else return 0;
};
void TransProcess::Init(word TransID) {
	TransportID = TransID;
	OneObject* OB = Group[TransportID];
	if (OB) {
		TransportSN = OB->Serial;
		Nation = OB->NNUM;
		Units = new word[8];
		Serials = new word[8];
		OnTheWay = new byte[8];
		Reserv = 8;
		NUnits = 0;
		FinalPointPresent = false;
		MidPresent = false;
		ConcPresent = false;
		NearCost = false;
		MovingToFinalCost = false;
		AbsorbUnits = false;
		MovingToCost = false;
		ReadyToDrive = false;
		ExtractUnits = false;
		Waiting = false;
		FinMidPresent = false;
		Used = true;
		StartPointFound = false;
		GLOB = new GOrder;
		GLOB->Data = this;
		GLOB->CheckDisconnectionAbility = &TRP_CheckDisconnectionAbility;
		GLOB->Disconnect = &TRP_Disconnect;
		GLOB->KillOrder = &TRP_KillOrder;
		GLOB->GetIcon = &TRP_IconInfo;
		OB->GlobalOrder = GLOB;
	};
};
void TransProcess::AddUnit(word UnitID) {
	OneObject* OB = Group[UnitID];
	if ((!OB) || OB->NNUM != Nation)return;
	NewMonster* NM = OB->newMons;
	if (NM->LockType || NM->Building)return;
	if (CheckUnit(UnitID))return;
	for (int i = 0; i < NUnits; i++) {
		if (Units[i] == 0xFFFF) {
			Units[i] = UnitID;
			Serials[i] = OB->Serial;
			OB->GlobalOrder = GLOB;
			OnTheWay[i] = 0;
			return;
		};
	};
	if (NUnits >= Reserv) {
		Reserv += 8;
		Units = (word*)realloc(Units, Reserv << 1);
		Serials = (word*)realloc(Serials, Reserv << 1);
		OnTheWay = (byte*)realloc(OnTheWay, Reserv);
		OB->GlobalOrder = GLOB;
	};
	Units[NUnits] = UnitID;
	Serials[NUnits] = OB->Serial;
	OnTheWay[NUnits] = 0;
	NUnits++;
};
bool TransProcess::ExcludeUnit(word UnitID) {
	bool exc = false;
	for (int i = 0; i < NUnits; i++) {
		if (Units[i] == UnitID) {
			Units[i] = 0xFFFF;
			exc = true;
		};
	};
	if (NUnits&&Units[NUnits - 1])NUnits--;
	return exc;
};
bool TransProcess::CheckUnit(word UnitID) {
	for (int i = 0; i < NUnits; i++) {
		if (Units[i] == UnitID)return true;
	};
	return false;
};
int TransProcess::FindNearestUnitIndex(int x, int y) {
	int mindis = 10000000;
	int IDI = -1;
	for (int i = 0; i < NUnits; i++) {
		word ID = Units[i];
		if (ID != 0xFFFF) {
			OneObject* OB = Group[ID];
			if (OB) {
				int dista = Norma(OB->RealX - x, OB->RealY - y);
				if (dista < mindis) {
					mindis = dista;
					IDI = i;
				};
			};
		};
	};
	return IDI;
};
void TransportLink(OneObject* OB);
void TransProcess::RefreshUnits() {
	int poss = 0;
	for (int i = 0; i < NUnits; i++) {
		word ID = Units[i];
		word SN = Serials[i];
		Units[poss] = ID;
		Serials[poss] = SN;
		OnTheWay[poss] = OnTheWay[i];
		if (ID != 0xFFFF) {
			OneObject* OB = Group[ID];
			if (OB) {
				if (OB->Serial != SN || OB->Hidden) {
					if (OB->GlobalOrder&&OB->GlobalOrder->Disconnect)OB->GlobalOrder->Disconnect(OB, OB->GlobalOrder, 0, 0);
					OB->GlobalOrder = NULL;
					Units[poss] = 0xFFFF;
				}
				else poss++;
			}
			else Units[i] = 0xFFFF;
		};
	};
	NUnits = poss;
};
void TransProcess::Demobilisation() {
	for (int i = 0; i < NUnits; i++) {
		word MID = Units[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			if (OB) {
				OB->GlobUnlock();
				OB->ClearOrders();
				OB->GlobLock();
			};
		};
	};
	if (TransportID != 0xFFFF) {
		OneObject* OB = Group[TransportID];
		OB->ClearOrders();
	};
	NUnits = 0;
	Used = false;
};
bool TransProcess::FindStartPoint() {
	OneObject* OBTR = Group[TransportID];
	if (NUnits&&OBTR) {
		int neu = FindNearestUnitIndex(OBTR->RealX, OBTR->RealY);
		if (neu != -1) {
			word MID = Units[neu];
			OneObject* OB = Group[MID];
			int xx = OB->x;
			int yy = OB->y;
			OBTR->Lx -= 2;
			char Dirc = OBTR->RealDir;

			if (FindCostPosition(OBTR, &xx, &yy, &Dirc)) {
				StartX = xx + (OBTR->Lx >> 1);
				StartY = yy + (OBTR->Lx >> 1);
				StartPointFound = true;
				OBTR->Lx += 2;
				MovingToCost = true;
				OBTR->GlobUnlock();
				ParkWaterNewMonster(OBTR, StartX << 8, StartY << 8, 0, 0);
				OBTR->GlobLock();
				return true;
			};
		};
		OBTR->Lx += 2;
	};
	return false;
};
bool TransProcess::FindConcPoint() {
	OneObject* OB = Group[TransportID];
	if (OB) {
		int LX2 = (OB->Lx >> 1) + 12;
		int DX = (TCos[OB->RealDir] * LX2) >> 8;
		int DY = (TSin[OB->RealDir] * LX2) >> 8;
		int CX = OB->RealX >> 8;
		int CY = OB->RealY >> 8;
		if (!CheckBar(CX + DX - 2, CY + DY - 2, 5, 5)) {
			//middle point found;
			MidPX = CX + DX;
			MidPY = CY + DY;
			ConcX = CX + DX + (DX >> 1);
			ConcY = CY + DY + (DY >> 1);
			ConcRadius = 3 + int(sqrt(NUnits));
			MidPresent = true;
			ConcPresent = true;
			return true;
		};
	};
	MidPresent = false;
	return false;
};
bool TransProcess::CheckUnloadAbility() {
	OneObject* OB = Group[TransportID];
	if (OB) {
		if (OB->LocalOrder)return false;
		MotionField* MFI = MFIELDS + OB->LockType;
		int LX2 = (OB->Lx >> 1) + 15;
		int DX = (TCos[OB->RealDir] * LX2) >> 8;
		int DY = (TSin[OB->RealDir] * LX2) >> 8;
		int CX = OB->RealX >> 8;
		int CY = OB->RealY >> 8;
		if (OB->GLock) {
			MFI->BClrBar(OB->x, OB->y, OB->Lx);
		};
		char NDir = GetBestDirection(MFI, OB->x + (OB->Lx >> 1), OB->y + (OB->Lx >> 1), OB->Lx, OB->RealDir);
		bool NEARCOST = MFI->CheckBar(OB->x - 2, OB->y - 2, OB->Lx + 4, OB->Lx + 4);
		if (OB->GLock) {
			MFI->BClrBar(OB->x, OB->y, OB->Lx);
		};
		char DDIR = NDir - OB->RealDir;
		if (NEARCOST&&abs(DDIR > 32)) {
			if (DDIR > 0) {
				OB->RealDir += 2;
			}
			else OB->RealDir -= 2;
			OB->GraphDir = OB->RealDir;
			return false;
		};
		return NEARCOST && !CheckBar(CX + DX - 2, CY + DY - 2, 5, 5);
	};
	return false;
};
bool TransProcess::CheckAbsorbAbility() {
	OneObject* OB = Group[TransportID];
	NewMonster* NM = OB->newMons;
	MotionField* MFI = MFIELDS + NM->LockType;
	int LX = OB->Lx;
	if (OB->GLock) {
		MFI->BClrBar(OB->x, OB->y, LX);
	};
	bool NEARCOST = MFI->CheckBar(OB->x - 2, OB->y - 2, LX + 4, LX + 4);
	char NDir = GetBestDirection(MFI, OB->x + (LX >> 1), OB->y + (LX >> 1), LX, OB->RealDir);
	if (OB->GLock) {
		MFI->BSetBar(OB->x, OB->y, LX);
	};
	if (!NEARCOST)return false;
	char DDIR = NDir - OB->RealDir;
	if (abs(DDIR) > 32) {
		if (DDIR > 0)OB->RealDir += 2;
		else OB->RealDir -= 2;
		OB->GraphDir = OB->RealDir;
		return false;
	};
	int LX2 = (OB->Lx >> 1) + 12;
	int DX = (TCos[OB->RealDir] * LX2) >> 8;
	int DY = (TSin[OB->RealDir] * LX2) >> 8;
	int CX = OB->RealX >> 8;
	int CY = OB->RealY >> 8;
	if (!CheckBar(CX + DX - 2, CY + DY - 2, 5, 5)) {
		//middle point found;
		MidPX = CX + DX;
		MidPY = CY + DY;
		ConcX = CX + DX + (DX >> 1);
		ConcY = CY + DY + (DY >> 1);
		ConcRadius = 3 + int(sqrt(NUnits));
		MidPresent = true;
		ConcPresent = true;
		return true;
	}
	else return false;
};
bool TransProcess::SetFinalPoint(int xx, int yy) {
	OneObject* OBTR = Group[TransportID];
	if (OBTR) {
		int LXX = OBTR->Lx;
		OBTR->Lx -= 1;
		char Dirc = OBTR->RealDir;
		if (FindCostPosition(OBTR, &xx, &yy, &Dirc)) {
			FinX = xx + (OBTR->Lx >> 1);
			FinY = yy + (OBTR->Lx >> 1);
			FinalPointPresent = true;
			//MovingToFinalCost=true;
			OBTR->Lx = LXX;
			return true;
		};
		OBTR->Lx = LXX;
	};
	return false;
};
void AbsorbObjectLink(OneObject* OB);
void AbsorbObject(OneObject* OB, OneObject* Absorber, byte OrdType) {
	if (OB->CheckOrderAbility())return;
	NewMonster* NM = Absorber->newMons;
	if (Absorber->NInside >= NM->MaxInside)return;
	Order1* OR1 = OB->CreateOrder(OrdType);
	if (!OR1)return;
	OR1->DoLink = &AbsorbObjectLink;
	OR1->info.MoveToObj.ObjIndex = Absorber->Index;
	OR1->info.MoveToObj.SN = Absorber->Serial;
	OB->PrioryLevel = 0;
};
void AbsorbObjectLink(OneObject* OB) {
	word AbIndex = OB->LocalOrder->info.MoveToObj.ObjIndex;
	word SN = OB->LocalOrder->info.MoveToObj.SN;
	OneObject* Absorber = Group[AbIndex];
	if (Absorber) {
		NewMonster* ANM = Absorber->newMons;
		if (Absorber->NInside < ANM->MaxInside) {
			Absorber->Inside = (word*)realloc(Absorber->Inside, (Absorber->NInside + 1) << 1);
			Absorber->Inside[Absorber->NInside] = OB->Index;
			Absorber->NInside++;
			OB->GlobUnlock();
			OB->HideMe();
			OB->GlobLock();
		}
		else OB->DeleteLastOrder();
	}
	else OB->DeleteLastOrder();
};
void LeaveShipLink(OneObject* OB);
void LeaveShip(OneObject* OB, byte OrdType) {
	if (OB->CheckOrderAbility())return;
	if (!OB->NInside)return;
	NewMonster* NM = OB->newMons;
	if (NM->LockType != 1)return;
	Order1* OR1 = OB->CreateOrder(OrdType);
	if (!OR1)return;
	OR1->DoLink = &LeaveShipLink;
	OB->PrioryLevel = 0;
};
void LeaveShipLink(OneObject* OB) {
	if (OB->delay)return;
	if (OB->NInside) {
		int L2 = OB->Lx + 6;
		int DX = (TCos[OB->RealDir] * L2) >> 8;
		int DY = (TSin[OB->RealDir] * L2) >> 8;
		int CX = OB->RealX >> 8;
		int CY = OB->RealY >> 8;
		if (!CheckBar(CX + DX - 1, CY + DY - 1, 3, 3)) {
			word ID = OB->Inside[OB->NInside - 1];
			OB->NInside--;
			if (!OB->NInside) {
				free(OB->Inside);
				OB->Inside = NULL;
			};
			OneObject* INs = Group[ID];
			INs->GlobUnlock();
			INs->RealX = OB->RealX;
			INs->RealY = OB->RealY;
			INs->ShowMe();
			INs->UnlimitedMotion = false;
			INs->SetOrderedUnlimitedMotion(0);
			INs->NewMonsterSendTo((CX + DX) << 8, (CY + DY) << 8, 0, 2);
			INs->ClearOrderedUnlimitedMotion(2);
			INs->NewMonsterSendTo((CX + DX + (DX >> 2) + (rando() & 3) - 1) << 8, (CY + DY + (DY >> 2) + (rando() & 3) - 1) << 8, 0, 2);
			INs->UnlimitedMotion = true;
			INs->GlobLock();
			OB->delay = 10;//40;
			OB->MaxDelay = 0;
			OB->DeleteLastOrder();
		};
	}
	else OB->DeleteLastOrder();
};
void TotalLeaveShip(OneObject* OB) {
	for (int i = 0; i < OB->NInside; i++)LeaveShip(OB, 2);
};
void NewMonsterSendToLink(OneObject* OB);

//Transport ships logic
void TransProcess::Process() 
{
	RefreshUnits();

	//1.Existing of concentration square
	if (!StartPointFound) 
	{
		FindStartPoint();
	}

	if (TransportID != 0xFFFF) 
	{
		OneObject* TRA = Group[TransportID];
		NewMonster* NM = TRA->newMons;
		AdvCharacter* ADC = TRA->Ref.General->MoreCharacter;
		if (TRA && TRA->Serial == TransportSN) 
		{
			if (MovingToCost) {
				Order1* OR1 = TRA->LocalOrder;
				if ((!OR1) && CheckAbsorbAbility())
				{//parking is performed
					if (!MidPresent)
						FindConcPoint();
					MovingToCost = false;
					AbsorbUnits = true;
				}
			}

			if (AbsorbUnits && MidPresent) 
			{
				//moving units to ship
				if (!TRA->delay) 
				{
					int OnWay = 0;
					for (int i = 0; i < NUnits; i++) 
					{
						word MID = Units[i];
						if (MID != 0xFFFF) 
						{
							OneObject* OB = Group[MID];
							if (!OB->Hidden)
							{
								if (OnTheWay[i])
								{
									OnWay++;
								}
							}
						}
					}

					if (TRA->NInside + OnWay < ADC->MaxInside) 
					{
						//search for the nearest unit
						int mindis = 1000000;
						OneObject* NEA = nullptr;
						int MUIndex = 0;
						int MIX = MidPX << 8;
						int MIY = MidPY << 8;

						for (int i = 0; i < NUnits; i++) 
						{
							word ID = Units[i];
							if (ID != 0xFFFF && !OnTheWay[i]) 
							{
								OneObject* OB = Group[ID];
								if (!OB->Hidden) 
								{
									int dis = Norma(OB->RealX - MIX, OB->RealY - MIY);
									if (dis < mindis) 
									{
										mindis = dis;
										NEA = OB;
										MUIndex = i;
									}
								}
							}
						}

						if (NEA) 
						{
							//sending to transport
							OnTheWay[MUIndex] = 1;
							NEA->GlobUnlock();
							NEA->NewMonsterSendTo(MidPX << 8, MidPY << 8, 0, 0);
							NEA->SetOrderedUnlimitedMotion(2);
							NEA->NewMonsterSendTo(TRA->RealX, TRA->RealY, 0, 2);
							AbsorbObject(NEA, TRA, 2);
							NEA->GlobLock();
							TRA->delay = 30;
							TRA->MaxDelay = 0;
						}
					}
					else 
					{
						TRA->delay = 20;
						TRA->MaxDelay = 0;
					}
				}

				if (TRA->NInside >= ADC->MaxInside || (NUnits == 0 && TRA->NInside)) 
				{
					AbsorbUnits = false;
					ReadyToDrive = true;
				}
			}

			if (ConcPresent) 
			{
				//preparing to moving to the ship
				//search for units beyond the circle
				int R = ConcRadius << 8;
				int R2 = R >> 1;
				int CX = ConcX << 8;
				int CY = ConcY << 8;
				for (int i = 0; i < NUnits; i++) 
				{
					word ID = Units[i];
					if (ID != 0xFFFF && !OnTheWay[i]) 
					{
						OneObject* OB = Group[ID];
						Order1* OR1 = OB->LocalOrder;
						bool MotionToCircle = false;
						if (OR1&&OR1->DoLink == &NewMonsterSendToLink) 
						{
							int xd = int(OR1->info.MoveToXY.x) << 8;
							int yd = int(OR1->info.MoveToXY.y) << 8;
							if (Norma(xd - CX, yd - CY) < R)MotionToCircle = true;
						}

						if (!MotionToCircle&&Norma(OB->RealX - CX, OB->RealY - CY) > R) 
						{
							OB->GlobUnlock();
							OB->NewMonsterSendTo(CX + ((R*rando()) >> 15) - R2, CY + ((R*rando()) >> 15) - R2, 0, 0);
							OB->GlobLock();
						}
					}
				}
			}

			if (ReadyToDrive && FinalPointPresent && !MovingToFinalCost) 
			{
				MovingToFinalCost = true;
				TRA->GlobUnlock();
				ParkWaterNewMonster(TRA, FinX << 8, FinY << 8, 0, 0);
				TRA->GlobLock();
			}

			if (MovingToFinalCost)//IMPORTANT: parking of transport ship
			{
				Order1* OR1 = TRA->LocalOrder;
				if ((!OR1) && CheckUnloadAbility()) 
				{
					MovingToFinalCost = false;
					ExtractUnits = true;
					ReadyToDrive = false;
				}
			}

			if (ExtractUnits) 
			{
				if (TRA->NInside && (!TRA->delay) && CheckUnloadAbility()) 
				{
					TRA->GlobUnlock();
					LeaveShip(TRA, 0);
					TRA->GlobLock();
					TRA->delay = 30;
					TRA->MaxDelay = 0;
				}
				else 
				{
					ParkWaterNewMonster(TRA, FinX << 8, FinY << 8, 0, 0);
					ExtractUnits = false;
					MovingToFinalCost = true;
				}

				if (!TRA->NInside) 
				{
					TRA->delay = 100;
					TRA->MaxDelay = 0;
					ExtractUnits = false;
					Waiting = true;
				}
			}

			if (!TRA->delay && Waiting) 
			{
				bool Need = false;
				for (int i = 0; i < NUnits; i++) 
				{
					if (Units[i] != 0xFFFF)Need = true;
				}

				if (Need) 
				{
					Waiting = false;
					MovingToCost = true;
					TRA->GlobUnlock();
					ParkWaterNewMonster(TRA, StartX << 8, StartY << 8, 0, 0);
					TRA->GlobLock();
				}
				else
				{
					KillAnyway();
				}
			}
		}
	}
}

#define MaxTP 16

TransProcess TRAPR[MaxTP];

bool CheckTransportOnParking(OneObject* TRA, int x, int y) 
{
	for (int i = 0; i < MaxTP; i++) 
	{
		TransProcess* TR = TRAPR + i;
		if (TR->TransportID == TRA->Index) 
		{
			TR->SetFinalPoint(x, y);
			return true;
		}
	}
	return false;
}

bool CheckUnloadProcess(OneObject* OB) 
{
	if (OB) 
	{
		return (!OB->GlobalOrder) && OB->Inside;
	}
	return false;
}

//Calls TransProcess::Process() in every TRAPR[]
void HandleTransport() 
{
	for (int i = 0; i < MaxTP; i++) 
	{
		if (TRAPR[i].Used)
		{
			TRAPR[i].Process();
		}
	}
}

GOrder::GOrder() {
	CheckDisconnectionAbility = NULL;
	Disconnect = NULL;
	KillOrder = NULL;
	Data = NULL;
};
GOrder::~GOrder() {
};
bool OneObject::CheckOrderAbility(int LParam, int RParam) {
	if (!InternalLock)return false;
	if (GlobalOrder) {
		if (GlobalOrder->CheckDisconnectionAbility) {
			if (GlobalOrder->CheckDisconnectionAbility(this, GlobalOrder, LParam, RParam)) {
				if (GlobalOrder->Disconnect)
					return !GlobalOrder->Disconnect(this, GlobalOrder, LParam, RParam);
			}
			else return true;
		}
		else return true;
	};
	return false;
};
bool OneObject::CheckOrderAbility() {
	return CheckOrderAbility(0, 0);
};
void OneObject::GlobLock() {
	InternalLock = true;
};
void OneObject::GlobUnlock() {
	InternalLock = false;
};
//------------------------New transport algoritm---------------------
void EraseBrigade(Brigade* BR);
void GoToTransportLink(OneObject* OB);
bool CheckTransportEntering(OneObject* OB) {
	if (!OB)return false;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	return OB->newMons->Transport&&OB->DstX&&OB->Ready&&OB->NInside < ADC->MaxInside + OB->AddInside;
};
bool OneObject::GoToTransport(word ID, byte Prio) {
	if (InArmy&&BrigadeID != 0xFFFF) {
		Brigade* BR = Nat->CITY->Brigs + BrigadeID;
		if (BR->Enabled) {
			OneObject* OB1 = NULL;
			OneObject* OB2 = NULL;
			if (BR->WarType) {
				word M1 = BR->Memb[0];
				word M2 = BR->Memb[1];
				if (M1 != 0xFFFF) {
					OB1 = Group[M1];
					if (OB1&&OB1->Serial == BR->MembSN[0]) {
						//if(!OB1->Selected)OB1=NULL;
					}
					else OB1 = NULL;
				};
				if (M2 != 0xFFFF) {
					OB2 = Group[M2];
					if (OB2&&OB2->Serial == BR->MembSN[1]) {
						//if(!OB2->Selected)OB2=NULL;
					}
					else OB2 = NULL;
				};
			};
			EraseBrigade(BR);
			if (OB1)OB1->GoToTransport(ID, Prio);
			if (OB2)OB2->GoToTransport(ID, Prio);
		};
	};
	if (CheckOrderAbility())return false;
	NewMonster* NM = newMons;
	OneObject* OB = Group[ID];
	if (!(OB->newMons->Transport&&OB->DstX))return false;
	if (!OB->Ready)return false;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	NewMonster* MiNM = OB->newMons;
	if (!(NewMonst && (!LockType) && (!NewBuilding) && (MiNM->Transport)))return false;
	if (OB->NInside >= ADC->MaxInside + OB->AddInside)return false;
	if (PrioryLevel > Prio)return false;
	if (LocalOrder&&LocalOrder->OrderType == 243
		&& LocalOrder->info.BuildObj.ObjIndex == ID)return true;
	Order1* Or1 = CreateOrder(0);
	if (!int(Or1))return false;
	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 243;
	Or1->OrderTime = 0;
	Or1->DoLink = &GoToTransportLink;
	int xm, ym;
	OB->GetCornerXY(&xm, &ym);
	Or1->info.BuildObj.ObjIndex = ID;
	Or1->info.BuildObj.SN = OB->Serial;
	Or1->info.BuildObj.ObjX = OB->DstX >> 4;
	Or1->info.BuildObj.ObjY = OB->DstY >> 4;
	//PrioryLevel=Prio&127;
	UnBlockUnit();
	MakeOrderSound(this, 2);
	return true;
};
void GoToTransportLink(OneObject* OBJ) {
	//int xx=OBJ->LocalOrder->info.BuildObj.ObjX;
	//int yy=OBJ->LocalOrder->info.BuildObj.ObjY;
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	word OID = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	word OSN = OBJ->LocalOrder->info.BuildObj.SN;
	OBJ->NewState = OBJ->GroundState;
	OneObject* OB = Group[OID];

	if ((OB && (OB->DstX <= 0 || OB->Sdoxlo)) || (!OB)) 
	{
		OBJ->DeleteLastOrder();
		if (OBJ->UnlimitedMotion) 
		{
			OBJ->Die();
		}
		return;
	}

	int xx, yy;

	NewMonster* NM = OBJ->newMons;
	NewMonster* MiNM = OB->newMons;
	xx = OB->DstX;
	yy = OB->DstY;
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	if (OB&&OB->Serial == OSN&&OB->NInside < ADC->MaxInside + OB->AddInside) {
		int dst = Norma(OBJ->RealX - xx, OBJ->RealY - yy);
		if (Norma(OBJ->RealX - OB->RealX, OBJ->RealY - OB->RealY) < 768) {
			//hiding;
			word* Ins = new word[OB->NInside + 1];
			if (OB->NInside) {
				memcpy(Ins, OB->Inside, OB->NInside << 1);
				free(OB->Inside);
			};
			OB->Inside = Ins;
			Ins[OB->NInside] = OBJ->Index;
			OBJ->HideMe();
			OBJ->ClearOrders();
			OB->NInside++;
			return;
		};
		if (dst < 4 * 16 * 16) {

			OBJ->UnlimitedMotion = false;
			OBJ->NewMonsterSendTo(OB->RealX, OB->RealY, 16, 1);
			OBJ->NewMonsterSendTo((OB->RealX * 3 + xx) >> 2, (OB->RealY * 3 + yy) >> 2, 16, 1);
			OBJ->NewMonsterSendTo((OB->RealX + xx) >> 1, (OB->RealY + yy) >> 1, 16, 1);
			OBJ->NewMonsterSendTo((OB->RealX + xx * 3) >> 2, (OB->RealY + yy * 3) >> 2, 16, 1);

			OBJ->UnlimitedMotion = true;
			return;
		}
		else OBJ->CreatePath(xx >> 8, yy >> 8);
	}
	else {
		if (OBJ->UnlimitedMotion) {
			int XX = int(OBJ->LocalOrder->info.BuildObj.ObjX) << 4;
			int YY = int(OBJ->LocalOrder->info.BuildObj.ObjY) << 4;
			OBJ->ClearOrders();
			OBJ->UnlimitedMotion = false;
			OBJ->NewMonsterSendTo((XX + OB->RealX) >> 1, (YY + OB->RealY) >> 1, 16, 0);
			OBJ->ClearOrderedUnlimitedMotion(2);
			OBJ->NewMonsterSendTo(XX + (rando() >> 6), YY + (rando() >> 6), 16, 2);
			//OBJ->ClearOrderedUnlimitedMotion(2);
			OBJ->UnlimitedMotion = true;
		}
		else {
			OBJ->DeleteLastOrder();
			OBJ->UnlimitedMotion = false;
		};
	};
};

void LeaveTransportLink(OneObject* OB);
void LeaveAll(OneObject* OB) {
	if (!OB->NInside)return;
	for (int i = 0; i < OB->NInside; i++) {
		OneObject* INS = Group[OB->Inside[i]];
		OB->LeaveMine(INS->NIndex);
	};
};
void OneObject::LeaveTransport(word Type) {
	if (!(NInside&&DstX))return;
	int xx1 = RealX + int(TCos[RealDir]) * 16;
	int yy1 = RealY + int(TSin[RealDir]) * 16;
	if (!MFIELDS->CheckBar((xx1 >> 8) - 2, (yy1 >> 8) - 2, 4, 4)) {
		DstX = xx1;
		DstY = yy1;
	}
	else {
		DstX = -1;
		return;
	};
	int ni = 0;
	for (int i = 0; i < NInside; i++) {
		OneObject* OB = Group[Inside[i]];
		if (OB&&OB->NIndex == Type)ni++;
	};
	if (!ni)return;
	Order1* OR1 = CreateOrder(1);
	if (!OR1)return;
	OR1->info.BuildObj.ObjIndex = Type;
	OR1->info.BuildObj.ObjX = 10 >> SpeedSh;
	OR1->DoLink = &LeaveTransportLink;
	OR1->PrioryLevel = 0;
};
void LeaveTransportLink(OneObject* OBJ) {
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	if (OBJ->LocalOrder->info.BuildObj.ObjX) {
		OBJ->LocalOrder->info.BuildObj.ObjX--;
		return;
	};
	if (!(OBJ->NInside&&OBJ->DstX)) {
		OBJ->DeleteLastOrder();
		return;
	};
	OneObject* OB = NULL;
	word p;
	word Type = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	for (int i = 0; i < OBJ->NInside && !OB; i++) {
		p = OBJ->Inside[i];
		OB = Group[p];
		if (OB&&OB->NIndex == Type) {
			if (i + 1 < OBJ->NInside)memcpy(OBJ->Inside + i, OBJ->Inside + i + 1, (OBJ->NInside - i - 1) << 1);
			OBJ->NInside--;
		}
		else OB = NULL;
	};
	if (!OB) {
		OBJ->DeleteLastOrder();
		return;
	};
	if (!OBJ->NInside) {
		free(OBJ->Inside);
		OBJ->Inside = NULL;
		//return;
	};
	OB->ShowMe();
	NewMonster* MiNM = OBJ->newMons;

	OB->RealX = OBJ->RealX;
	OB->RealY = OBJ->RealY;
	OB->UnlimitedMotion = 0;
	OB->SetOrderedUnlimitedMotion(0);
	OB->NewMonsterSendTo((OBJ->DstX * 3 + OBJ->RealX) >> 2, (OBJ->DstY * 3 + OBJ->RealY) >> 2, 16, 2);
	OB->ClearOrderedUnlimitedMotion(2);
	OB->NewMonsterSendTo(OBJ->DstX + (rando() & 511) - 256, OBJ->DstY + (rando() & 511) - 256, 16, 2);
	OB->UnlimitedMotion = true;
	OBJ->DeleteLastOrder();

};
//------------------Fishing----------------

#define FishSX 3
byte* FishMap;
int FishLx;

//Make *FishMap a null pointer
void InitFishMap() 
{
	FishMap = nullptr;
}

void CreateFishMap() {
	FishLx = msx >> FishSX;
	FishMap = new byte[FishLx*FishLx];
	memset(FishMap, 0, FishLx*FishLx);
	for (int x = 0; x < FishLx; x++)
		for (int y = 0; y < FishLx; y++) {
			if (MFIELDS[1].CheckBar(x << (FishSX + 1), y << (FishSX + 1), 2 << FishSX, 2 << FishSX))FishMap[x + y*FishLx] = 2;
		};
};
byte TestFishMap(int x, int y) {
	if (x >= 0 && y >= 0 && x < FishLx&&y < FishLx) {
		//if(FishMap[x+y*FishLx])return 2;
		if (UnitsField.CheckBar(x << (FishSX + 1), y << (FishSX + 1), 2 << FishSX, 2 << FishSX) ||
			MFIELDS[1].CheckBar(x << (FishSX + 1), y << (FishSX + 1), 2 << FishSX, 2 << FishSX))return 1;
	};
	return 0;
};
void FishingLink(OneObject* OB);
void OneObject::Fishing() {
	if (LocalOrder)return;
	PrioryLevel = 0;
	Order1* OR1 = CreateOrder(0);
	if (OR1) {
		OR1->DoLink = &FishingLink;
		OR1->OrderType = 221;
		OR1->PrioryLevel = 0;
		OR1->info.BuildObj.ObjX = -1;
		OR1->info.BuildObj.ObjY = -1;
		OR1->info.BuildObj.ObjIndex = 0xFFFF;
		OR1->info.BuildObj.SN = 0;
	};
};
extern int tmtmt;
//extern int RESRC[8][8];
extern int FoodID;
int GetShipDanger1(int x, int y);
int GetShipDanger(int x, int y);
bool GetMostSafePlace(int *x, int *y) {
	int x0 = *x;
	int y0 = *y;
	int MIND = 100000000;
	int xd = x0;
	int yd = y0;
	int cx = x0 >> 4;
	int cy = y0 >> 4;
	if (!(MFIELDS[1].CheckBar(cx + 16 - 3, cy - 3, 6, 6) || UnitsField.CheckBar(cx + 16 - 3, cy - 3, 6, 6))) {
		int D = GetShipDanger1(x0 + 256, y0);
		if (D < MIND) {
			MIND = D;
			xd = x0 + 256;
			yd = y0;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx - 16 - 3, cy - 3, 6, 6) || UnitsField.CheckBar(cx - 16 - 3, cy - 3, 6, 6))) {
		int D = GetShipDanger1(x0 - 256, y0);
		if (D < MIND) {
			MIND = D;
			xd = x0 - 256;
			yd = y0;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx - 3, cy + 16 - 3, 6, 6) || UnitsField.CheckBar(cx - 3, cy + 16 - 3, 6, 6))) {
		int D = GetShipDanger1(x0, y0 + 256);
		if (D < MIND) {
			MIND = D;
			xd = x0;
			yd = y0 + 256;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx - 3, cy - 16 - 3, 6, 6) || UnitsField.CheckBar(cx - 3, cy - 16 - 3, 6, 6))) {
		int D = GetShipDanger1(x0, y0 - 256);
		if (D < MIND) {
			MIND = D;
			xd = x0;
			yd = y0 - 256;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx - 16 - 3, cy - 16 - 3, 6, 6) || UnitsField.CheckBar(cx - 16 - 3, cy - 16 - 3, 6, 6))) {
		int D = GetShipDanger1(x0 - 256, y0 - 256);
		if (D < MIND) {
			MIND = D;
			xd = x0 - 256;
			yd = y0 - 256;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx + 16 - 3, cy - 16 - 3, 6, 6) || UnitsField.CheckBar(cx + 16 - 3, cy - 16 - 3, 6, 6))) {
		int D = GetShipDanger1(x0 + 256, y0 - 256);
		if (D < MIND) {
			MIND = D;
			xd = x0 + 256;
			yd = y0 - 256;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx - 16 - 3, cy + 16 - 3, 6, 6) || UnitsField.CheckBar(cx - 16 - 3, cy + 16 - 3, 6, 6))) {
		int D = GetShipDanger1(x0 - 256, y0 + 256);
		if (D < MIND) {
			MIND = D;
			xd = x0 - 256;
			yd = y0 + 256;
		};
	};
	if (!(MFIELDS[1].CheckBar(cx + 16 - 3, cy + 16 - 3, 6, 6) || UnitsField.CheckBar(cx + 16 - 3, cy + 16 - 3, 6, 6))) {
		int D = GetShipDanger1(x0 + 256, y0 + 256);
		if (D < MIND) {
			MIND = D;
			xd = x0 + 256;
			yd = y0 + 256;
		};
	};
	*x = xd;
	*y = yd;
	return MIND < 100000000;
};
void FishingLink(OneObject* OBJ) {
	int cxx = OBJ->RealX >> 4;
	int cyy = OBJ->RealY >> 4;
	if (GetShipDanger(cxx, cyy) && GetMostSafePlace(&cxx, &cyy)) {
		OBJ->NewMonsterSendTo(cxx << 4, cyy << 4, 128 + 16, 0);
		return;
	};
	AdvCharacter* ADC = OBJ->Ref.General->MoreCharacter;
	word ObjIndex = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	int MinDis = OBJ->LocalOrder->info.BuildObj.SN;
	int ObjX = OBJ->LocalOrder->info.BuildObj.ObjX;
	int ObjY = OBJ->LocalOrder->info.BuildObj.ObjY;
	if (OBJ->GLock) {
		OBJ->GLock = false;
		MFIELDS[1].BClrBar(OBJ->x, OBJ->y, OBJ->Lx);
	};
	if (OBJ->Ustage >= ADC->FishAmount) {
		//go to home
		if (ObjIndex == 0xFFFF) {
			//search for the base
			byte NN = OBJ->NNUM;
			int LoX = OBJ->RealX;
			int LoY = OBJ->RealY;
			int Mindis = 3000000;
			word OBJID = 0xFFFF;
			for (int j = 0; j < MAXOBJECT; j++) {
				OneObject* OB = Group[j];
				if (OB&&OB->NNUM == NN) {
					if (OB->newMons->Port) {
						int pds = Norma(OB->RealX - LoX, OB->RealY - LoY);
						if (pds < Mindis) {
							Mindis = pds;
							OBJID = j;
						};
					};
				};
			};

			if (OBJID != 0xFFFF) {
				ObjIndex = OBJID;
				OneObject* OB = Group[OBJID];
				OBJ->LocalOrder->info.BuildObj.ObjIndex = OBJID;
				OBJ->LocalOrder->info.BuildObj.ObjX = OB->WallX;
				OBJ->LocalOrder->info.BuildObj.ObjY = OB->WallY;
			};
		};
		if (ObjIndex != 0xFFFF) {
			//I know the way to my home !
			OneObject* OB = Group[ObjIndex];
			if ((!OB) || OB->Sdoxlo) {
				OBJ->LocalOrder->info.BuildObj.ObjIndex = 0xFFFF;
			}
			else {
				NewMonster* PNM = OB->newMons;
				int BX = int(OB->WallX) << 8;
				int BY = int(OB->WallY) << 8;
				if (OB && (!OB->Sdoxlo) && PNM->Port) {
					int dist = Norma(BX - OBJ->RealX, BY - OBJ->RealY) >> 4;
					if (dist < 400 && ((tmtmt & 15) == 4)) {
						//if(MFIELDS[1].CheckBar(OBJ->x-2,OBJ->y-2,OBJ->Lx+4,OBJ->Lx+4)){
						AddXRESRC(OB->NNUM, FoodID, OBJ->Ustage);
						NATIONS[OB->NNUM].ResTotal[FoodID] += OBJ->Ustage;
						NATIONS[OB->NNUM].AddResource(FoodID, OBJ->Ustage);
						OBJ->Nat->CITY->FoodAmount += OBJ->Ustage;
						OBJ->Ustage = 0;
						OBJ->delay = 60;
						OBJ->MaxDelay = 60;
						OBJ->LocalOrder->info.BuildObj.ObjIndex = 0xFFFF;
						OBJ->DeletePath();
						OBJ->DestX = -1;
						OBJ->LocalOrder->info.BuildObj.ObjX = -1;
						//};
					}
					else OBJ->CreatePath(ObjX, ObjY);
				}
				else {
					OBJ->LocalOrder->info.BuildObj.ObjIndex = 0xFFFF;
					return;
				};
			};
		}
		else {
			OBJ->DeleteLastOrder();
			return;
		};
	}
	else {
		AdvCharacter* NM = OBJ->Ref.General->MoreCharacter;
		OBJ->NUstages += ADC->FishSpeed << SpeedSh;
		OBJ->Ustage += OBJ->NUstages >> 7;
		OBJ->NUstages &= 127;
		if (OBJ->delay)return;
		//search for the field
		if (ObjX < 0) {
			int xx = OBJ->x;
			int yy = OBJ->y;
			for (int f = 0; f < 20; f++) {
				int x1 = xx + (rando() & 63) - 32;
				int y1 = yy + (rando() & 63) - 32;
				if (!MFIELDS[1].CheckBar(x1 - 4, y1 + 6, 8, 8)) {
					OBJ->LocalOrder->info.BuildObj.ObjX = x1;
					OBJ->LocalOrder->info.BuildObj.ObjY = y1;
					return;
				};
			};
		}
		else {
			if (Norma(ObjX - OBJ->x, ObjY - OBJ->y) < 10) {
				OBJ->LocalOrder->info.BuildObj.ObjX = -1;
				return;
			}
			else OBJ->CreatePath(ObjX, ObjY);
		};
	};
}

void ProcessFishing() 
{
	if (!FishMap)
	{
		return;
	}

	if (37 == tmtmt % 51) 
	{
		for (int j = 0; j < MAXOBJECT; j++) 
		{
			OneObject* OB = Group[j];
			if (OB) 
			{
				NewMonster* NM = OB->newMons;
				if (NM->FishAmount)
				{
					OB->Fishing();
				}
			}
		}
	}
}