#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mouse.h"
#include "MapDiscr.h"
#include <assert.h>
#include "sort.h"
#include "math.h"
#include <crtdbg.h>
#include "NewMon.h"
#include "Megapolis.h"

word GetDir(int, int);
SortClass::SortClass() {
	NUids = 0;
	MaxUID = 0;
	Uids = NULL;
	//Usn=NULL;
	Parms = NULL;
};
SortClass::~SortClass() {
	if (MaxUID) {
		free(Uids);
		free(Parms);
		//free(Usn);
	};
};
void SortClass::Sort() {
	if (NUids < 2)return;
	int uids = int(Uids);
	//int sns=int(Usn);
	//int snsn;
	int parms = int(Parms);
	int nuids = NUids - 1;
	byte fault;

	__asm {
		push    esi
		push    edi
		mov     esi, uids
		mov     edi, parms
		mov     ecx, nuids
		lpp1 :
		//mov     eax,sns
		//mov     snsn,eax
		xor     eax, eax
			mov     esi, uids
			mov     edi, parms
			mov     ecx, nuids
			mov     ecx, nuids
			mov     fault, 0
			lpp3:
		mov     ebx, [edi]
			mov     edx, [edi + 4]
			cmp     ebx, edx
			jle     lpp2
			mov[edi], edx
			mov[edi + 4], ebx
			//xchg    ebx,snsn
			//mov     dx,[ebx]
			//xchg    dx,[ebx+2]
			//mov     [ebx],dx
			//xchg    ebx,snsn
			mov     bx, [esi]
			xchg    bx, [esi + 2]
			mov[esi], bx
			mov     fault, 1
			lpp2:   add     esi, 2
			add     edi, 4
			//add     snsn,2
			dec     ecx
			jnz     lpp3
			cmp     fault, 0
			jnz      lpp1
			pop     esi
			pop     edi
	};
};
void SortClass::CheckSize(int Size) {
	if (!Size)return;
	if (Size > MaxUID) {
		if (MaxUID) {
			free(Uids);
			free(Parms);
			//free(Usn);
		};
		Uids = new word[Size];
		Parms = new int[Size];
		//Usn=new word[Size];
		MaxUID = Size;
	};
};
word SortClass::CreateByX(word* UIDS, int NUIDS, bool Direction) {
	CheckSize(NUIDS);
	word realn = 0;
	if (Direction) {
		for (int i = 0; i < NUIDS; i++) {
			word MID = UIDS[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB && !OB->Sdoxlo) {
					Uids[realn] = MID;
					//Usn[realn]=USN[i];
					UIDS[realn] = MID;
					Parms[realn] = OB->RealX;
					realn++;
				};
			};
		};
	}
	else {
		for (int i = 0; i < NUIDS; i++) {
			word MID = UIDS[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB && !OB->Sdoxlo) {
					Uids[realn] = MID;
					//Usn[realn]=USN[i];
					UIDS[realn] = MID;
					Parms[realn] = -OB->RealX;
					realn++;
				};
			};
		};
	};
	NUids = realn;
	return realn;
};
word SortClass::CreateByY(word* UIDS, int NUIDS, bool Direction) {
	CheckSize(NUIDS);
	word realn = 0;
	if (Direction) {
		for (int i = 0; i < NUIDS; i++) {
			word MID = UIDS[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB && !OB->Sdoxlo) {
					Uids[realn] = MID;
					//Usn[realn]=USN[i];
					UIDS[realn] = MID;
					Parms[realn] = OB->RealY;
					realn++;
				};
			};
		};
	}
	else {
		for (int i = 0; i < NUIDS; i++) {
			word MID = UIDS[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB && !OB->Sdoxlo) {
					Uids[realn] = MID;
					//Usn[realn]=USN[i];
					UIDS[realn] = MID;
					Parms[realn] = -OB->RealY;
					realn++;
				};
			};
		};
	};
	NUids = realn;
	return realn;
};
word SortClass::CreateByR(word* UIDS, int NUIDS, bool Direction, int x, int y) {
	CheckSize(NUIDS);
	word realn = 0;
	if (Direction) {
		for (int i = 0; i < NUIDS; i++) {
			word MID = UIDS[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB && !OB->Sdoxlo) {
					Uids[realn] = MID;
					//Usn[realn]=USN[i];
					UIDS[realn] = MID;
					Parms[realn] = Norma(OB->RealX - x, OB->RealY - y);
					realn++;
				};
			};
		};
	}
	else {
		for (int i = 0; i < NUIDS; i++) {
			word MID = UIDS[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB && !OB->Sdoxlo) {
					Uids[realn] = MID;
					//Usn[realn]=USN[i];
					UIDS[realn] = MID;
					Parms[realn] = -Norma(OB->RealX - x, OB->RealY - y);
					realn++;
				};
			};
		};
	};
	NUids = realn;
	return realn;
};
word SortClass::CreateByLine(word* UIDS, int NUIDS, int dx, int dy) {
	CheckSize(NUIDS);
	word realn = 0;
	for (int i = 0; i < NUIDS; i++) {
		word MID = UIDS[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			if (OB && !OB->Sdoxlo) {
				Uids[realn] = MID;
				//Usn[realn]=USN[i];
				UIDS[realn] = MID;
				Parms[realn] = (OB->RealX >> 5)*dx + (OB->RealY >> 5)*dy;
				realn++;
			};
		};
	};
	NUids = realn;
	return realn;
};
word SortClass::CreateByLineForShips(word* UIDS, int NUIDS, int dx, int dy) {
	CheckSize(NUIDS);
	word realn = 0;
	for (int i = 0; i < NUIDS; i++) {
		word MID = UIDS[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			if (OB && !OB->Sdoxlo) {
				Uids[realn] = MID;
				//Usn[realn]=USN[i];
				UIDS[realn] = MID;
				Parms[realn] = (OB->RealX >> 5)*dx + (OB->RealY >> 5)*dy;
				if (OB->newMons->Usage == GaleraID)Parms[realn] -= 100000;
				realn++;
			};
		};
	};
	NUids = realn;
	return realn;
};
void SortClass::CopyToDst(word* Dst, word* Sns) {
	memcpy(Dst, Uids, NUids << 1);
	for (int i = 0; i < NUids; i++) {
		OneObject* OB = Group[Uids[i]];
		if (OB)Sns[i] = OB->Serial;
	};
};
void SortClass::Copy(word* Dst) {
	memcpy(Dst, Uids, NUids << 1);
};
//-----------------------Usage of the SortClass----------------------
SortClass UNISORT;
word SortUnitsByR(word* urefs, word* snrefs, int nu, int x, int y) {
	int n = UNISORT.CreateByR(urefs, nu, true, x, y);
	UNISORT.Sort();
	UNISORT.CopyToDst(urefs, snrefs);
	return n;
};
//--------------------Selected groups management---------------------
//1.Creating abstract group: square group without the place on map.
//  It means, that group will be sorted specially:
//  1.a. Sorting by Y
//  1.b. Sort Lx elements by X
//  1.c. Sort next Lx elements by X
//  ....
//2.Placing on the map:
//???

PositionOrder::PositionOrder() {
	NUnits = 0;
	MaxUnit = 0;
	Ids = NULL;
	px = NULL;
	py = NULL;
};
PositionOrder::~PositionOrder() {
	if (MaxUnit) {
		free(Ids);
		free(px);
		free(py);
	};
	NUnits = 0;
	MaxUnit = 0;
	Ids = NULL;
	px = NULL;
	py = NULL;
};
void PositionOrder::CheckSize(int Size) {
	if (Size > MaxUnit) {
		if (MaxUnit > 0) {
			free(Ids);
			free(px);
			free(py);
		};
		Ids = new word[Size];
		px = new int[Size];
		py = new int[Size];
		MaxUnit = Size;
	};
	NUnits = Size;
};
void PositionOrder::Create(word* IDS, int NIDS) {
	CheckSize(NIDS);
	memcpy(Ids, IDS, NIDS << 1);
};
word PositionOrder::CreateLinearPositions(int x, int y, word* IDS, int NIDS, int dx, int dy) {
	Create(IDS, NIDS);
	UNISORT.CreateByLine(Ids, NUnits, dy >> 4, -dx >> 4);
	UNISORT.Sort();
	NIDS = UNISORT.NUids;
	CheckSize(NIDS);
	UNISORT.Copy(Ids);
	if (!(dx || dy))dx = 1;
	int N = Norma(dx, dy);
	int dx1 = (dx * 96 * 16) / N;
	int dy1 = (dy * 96 * 16) / N;
	int x0 = x - ((dy1*NIDS) >> 1);
	int y0 = y + ((dx1*NIDS) >> 1);
	for (int i = 0; i < NIDS; i++) {
		px[i] = x0;
		py[i] = y0;
		x0 += dy1;
		y0 -= dx1;
	};
	return N;
};
word PositionOrder::CreateRotatedPositions(int x, int y, word* IDS, int NIDS, int dx, int dy) {
	//special sorting
	dx >>= 4;
	dy >>= 4;
	if (!(dx || dy))dx = 1;
	Create(IDS, NIDS);
	//CheckSize(NIDS);
	int Lx = int(sqrt(NIDS));
	int Ly = Lx << 1;
	Lx >>= 1;
	if (NIDS < 4) {
		Lx = 1;
		Ly = NIDS;
	};
	int dd = dx;
	dx = dy;
	dy = -dd;
	int nn = Lx*Ly;
	if (nn < NIDS) {
		if (nn + Lx >= NIDS)Ly++;
		else {
			if (nn + Ly >= NIDS)Lx++;
			else {
				Ly++;
				Lx++;
			};
		};
	};
	nn = Lx*Ly;
	if (nn < NIDS) {
		if (nn + Lx >= NIDS)Ly++;
		else {
			if (nn + Ly >= NIDS)Lx++;
			else {
				Ly++;
				Lx++;
			};
		};
	};
	UNISORT.CreateByLine(Ids, NUnits, dx >> 4, dy >> 4);
	UNISORT.Sort();
	int NU = UNISORT.NUids;
	CheckSize(NU);
	UNISORT.Copy(Ids);
	int Px = 0;
	int Lx1;
	for (int iy = 0; iy < Ly; iy++) {
		Lx1 = NU - Px;
		if (Lx1 > Lx)Lx1 = Lx;
		if (Lx1 > 0) {
			UNISORT.CreateByLine(Ids + Px, Lx1, -dy >> 4, dx >> 4);
			UNISORT.Sort();
			UNISORT.Copy(Ids + Px);
			Px += Lx1;
		};
	};
	//Sorting is finished now
	//Getting of the maximal size of Unit
	int maxR = 0;
	word* IDE = Ids;
	word MID;
	int rr;
	OneObject* OB;
	for (int iy = 0; iy < NU; iy++) {
		MID = IDE[iy];
		OB = Group[MID];
		if (OB&&OB->NewMonst) {
			rr = OB->newMons->Radius2;
			if (rr > maxR) {
				maxR = rr;
			};
		};
	};
	//Creating the position
	//if(maxR<516)maxR+=512;
	//else maxR+=1024;
	maxR <<= 2;
	int Maxx = (msx << 9) - 768;
	int Maxy = (msy << 9) - 768;
	//int BLx=Lx*maxR;
	//int BLy=Ly*maxR;
	//int SX=x-(((Lx-1)*maxR)>>1);
	//int SY=y-(((Ly-1)*maxR)>>1);
	//if(SX<768)SX=768;
	//if(SY<768)SY=768;
	//if(SX+BLx>Maxx)SX=Maxx-BLx;
	//if(SY+BLy>Maxy)SY=Maxy-BLy;
	//int UX=SX;
	//int UY=SY;
	int pos = 0;
	int NR = Norma(dx, dy);
	int vx = (dx*maxR) / NR;
	int vy = (dy*maxR) / NR;
	int Dx = (-(Lx - 1)*vy + (Ly - 1)*vx) >> 1;
	int Dy = ((Lx - 1)*vx + (Ly - 1)*vy) >> 1;
	for (int iy = 0; iy < Ly; iy++) {
		for (int ix = 0; ix < Lx; ix++) {
			if (pos < NU) {
				px[pos] = x - ix*vy + iy*vx - Dx;
				py[pos] = y + ix*vx + iy*vy - Dy;
			};
			pos++;
		};
	};
	return NU;
};
word PositionOrder::CreateRotatedPositions2(int x, int y, word* IDS, int NIDS, int dx, int dy) {
	//special sorting
	dx >>= 4;
	dy >>= 4;
	if (!(dx || dy))dx = 1;
	Create(IDS, NIDS);
	//CheckSize(NIDS);
	int Lx = int(sqrt(NIDS));
	int Ly = Lx << 2;
	Lx >>= 2;
	if (NIDS < 10) {
		Lx = 1;
		Ly = NIDS;
	};
	int dd = dx;
	dx = dy;
	dy = -dd;
	int nn = Lx*Ly;
	if (nn < NIDS) {
		if (nn + Lx >= NIDS)Ly++;
		else {
			if (nn + Ly >= NIDS)Lx++;
			else {
				Ly++;
				Lx++;
			};
		};
	};
	nn = Lx*Ly;
	if (nn < NIDS) {
		if (nn + Lx >= NIDS)Ly++;
		else {
			if (nn + Ly >= NIDS)Lx++;
			else {
				Ly++;
				Lx++;
			};
		};
	};
	UNISORT.CreateByLine(Ids, NUnits, dx >> 4, dy >> 4);
	UNISORT.Sort();
	int NU = UNISORT.NUids;
	CheckSize(NU);
	UNISORT.Copy(Ids);
	int Px = 0;
	int Lx1;
	for (int iy = 0; iy < Ly; iy++) {
		Lx1 = NU - Px;
		if (Lx1 > Lx)Lx1 = Lx;
		if (Lx1 > 0) {
			UNISORT.CreateByLine(Ids + Px, Lx1, -dy >> 4, dx >> 4);
			UNISORT.Sort();
			UNISORT.Copy(Ids + Px);
			Px += Lx1;
		};
	};
	//Sorting is finished now
	//Getting of the maximal size of Unit
	int maxR = 0;
	word* IDE = Ids;
	word MID;
	int rr;
	OneObject* OB;
	for (int iy = 0; iy < NU; iy++) {
		MID = IDE[iy];
		OB = Group[MID];
		if (OB&&OB->NewMonst) {
			rr = OB->newMons->Radius2;
			if (rr > maxR) {
				maxR = rr;
			};
		};
	};
	//Creating the position
	//if(maxR<516)maxR+=512;
	//else maxR+=1024;
	maxR <<= 2;
	int Maxx = (msx << 9) - 768;
	int Maxy = (msy << 9) - 768;
	//int BLx=Lx*maxR;
	//int BLy=Ly*maxR;
	//int SX=x-(((Lx-1)*maxR)>>1);
	//int SY=y-(((Ly-1)*maxR)>>1);
	//if(SX<768)SX=768;
	//if(SY<768)SY=768;
	//if(SX+BLx>Maxx)SX=Maxx-BLx;
	//if(SY+BLy>Maxy)SY=Maxy-BLy;
	//int UX=SX;
	//int UY=SY;
	int pos = 0;
	int NR = Norma(dx, dy);
	int vx = (dx*maxR) / NR;
	int vy = (dy*maxR) / NR;
	int Dx = (-(Lx - 1)*vy + (Ly - 1)*vx) >> 1;
	int Dy = ((Lx - 1)*vx + (Ly - 1)*vy) >> 1;
	for (int iy = 0; iy < Ly; iy++) {
		for (int ix = 0; ix < Lx; ix++) {
			if (pos < NU) {
				px[pos] = x - ix*vy + iy*vx - Dx;
				py[pos] = y + ix*vx + iy*vy - Dy;
			};
			pos++;
		};
	};
	return NU;
};
word PositionOrder::CreatePositions(int x, int y, word* IDS, int NIDS) {
	//special sorting
	Create(IDS, NIDS);
	//CheckSize(NIDS);
	int Lx = int(sqrt(NIDS));
	int Ly = Lx;
	int nn = Lx*Ly;
	if (nn < NIDS) {
		if (nn + Lx >= NIDS)Ly++;
		else {
			if (nn + Ly >= NIDS)Lx++;
			else {
				Ly++;
				Lx++;
			};
		};
	};
	UNISORT.CreateByY(Ids, NUnits, true);
	UNISORT.Sort();
	int NU = UNISORT.NUids;
	CheckSize(NU);
	UNISORT.Copy(Ids);
	int Px = 0;
	int Lx1;
	for (int iy = 0; iy < Ly; iy++) {
		Lx1 = NU - Px;
		if (Lx1 > Lx)Lx1 = Lx;
		if (Lx1 > 0) {
			UNISORT.CreateByX(Ids + Px, Lx1, true);
			UNISORT.Sort();
			UNISORT.Copy(Ids + Px);
			Px += Lx1;
		};
	};
	//Sorting is finished now
	//Getting of the maximal size of Unit
	int maxR = 0;
	word* IDE = Ids;
	word MID;
	int rr;
	OneObject* OB;
	for (int iy = 0; iy < NU; iy++) {
		MID = IDE[iy];
		OB = Group[MID];
		if (OB&&OB->NewMonst) {
			rr = OB->newMons->Radius2;
			if (rr > maxR) {
				maxR = rr;
			};
		};
	};
	//Creating the position
	//if(maxR<516)maxR+=512;
	//else maxR+=1024;
	maxR <<= 2;
	maxR = 768;//????????
	int Maxx = (msx << 9) - 768;
	int Maxy = (msy << 9) - 768;
	int BLx = Lx*maxR;
	int BLy = Ly*maxR;
	int SX = x - (((Lx - 1)*maxR) >> 1);
	int SY = y - (((Ly - 1)*maxR) >> 1);
	if (SX < 768)SX = 768;
	if (SY < 768)SY = 768;
	if (SX + BLx > Maxx)SX = Maxx - BLx;
	if (SY + BLy > Maxy)SY = Maxy - BLy;
	int UX = SX;
	int UY = SY;
	int pos = 0;
	for (int iy = 0; iy < Ly; iy++) {
		for (int ix = 0; ix < Lx; ix++) {
			if (pos < NU) {
				px[pos] = UX;
				py[pos] = UY;
				UX += maxR;
			};
			pos++;
		};
		UX = SX;
		UY += maxR;
	};
	return NU;
};
extern bool CmdDone[8192];
void RotUnit(OneObject* OB, char Dir, byte OrdType);
void PositionOrder::SendToPosition(byte Prio, byte OrdType) {
	word MID;
	int cx = CenterX >> 4;
	int cy = CenterY >> 4;
	for (int i = 0; i < NUnits; i++) {
		MID = Ids[i];
		OneObject* OB = Group[MID];
		if (OB && !CmdDone[MID]) {
			OB->StandGround = 0;
			//OB->NewMonsterSendTo(px[i],py[i],Prio,OrdType);
			if (LastDirection < 256) {
				if (OB->BrigadeID == 0xFFFF) {
					switch (OrdType) {
					case 0:
						OB->NewMonsterSmartSendTo(cx, cy, (px[i] - CenterX) >> 4, (py[i] - CenterY) >> 4, Prio, 0);
						if (!OB->newMons->Transport)RotUnit(OB, LastDirection, 2);
						break;
					case 1:
						if (!OB->newMons->Transport)RotUnit(OB, LastDirection, 1);
						OB->NewMonsterSmartSendTo(cx, cy, (px[i] - CenterX) >> 4, (py[i] - CenterY) >> 4, Prio, 1);
						break;
					case 2:
						OB->NewMonsterSmartSendTo(cx, cy, (px[i] - CenterX) >> 4, (py[i] - CenterY) >> 4, Prio, 2);
						if (!OB->newMons->Transport)RotUnit(OB, LastDirection, 2);
						break;
					};

				}
				else {
					OB->NewMonsterSmartSendTo(cx, cy, (px[i] - CenterX) >> 4, (py[i] - CenterY) >> 4, Prio, OrdType);
				};
			}
			else {
				OB->NewMonsterSmartSendTo(cx, cy, (px[i] - CenterX) >> 4, (py[i] - CenterY) >> 4, Prio, OrdType);
			};
		};
	};
};
void PositionOrder::Patrol() {
	word MID;
	int cx = CenterX >> 4;
	int cy = CenterY >> 4;
	for (int i = 0; i < NUnits; i++) {
		MID = Ids[i];
		OneObject* OB = Group[MID];
		if (OB && !CmdDone[MID]) {
			OB->Patrol(OB->RealX >> 4, OB->RealY >> 4, cx + ((px[i] - CenterX) >> 4), cy + ((py[i] - CenterY) >> 4), 0);
		};
	};
};
extern City CITY[8];
PositionOrder PORD;
void SetStandState(Brigade* BR, byte State) {
	if (BR->Strelki)return;
	int N = BR->NMemb;
	word* Memb = BR->Memb;
	word* MembSN = BR->MembSN;
	int addD = 0;
	int addS = 0;
	if (State) {
		for (int i = 0; i < N; i++) {
			word MID = Memb[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB&&OB->Serial == MembSN[i]) {
					if (OB->newMons->PMotionL[0].Enabled) {
						OB->GroundState = State;
						if (!OB->LocalOrder) {
							OB->NewState = 1;
						};
					};
				};
			};
		};
	}
	else {
		for (int i = 0; i < N; i++) {
			word MID = Memb[i];
			if (MID != 0xFFFF) {
				OneObject* OB = Group[MID];
				if (OB&&OB->Serial == MembSN[i]) {
					OB->GroundState = 0;
				};
			};
		};
	};
};
extern int tmtmt;
void CancelStandGroundAnyway(Brigade* BR);
bool GetBrCenter(Brigade* BR, int* x, int* y);
extern bool OneDirection;
void GroupSendSelectedTo(byte NI, int x, int y, byte Prio, byte OrdType) {
	int Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* MSN = SerN[NI];
	//checking for pushka
	word* SSEL = new word[NSL[NI]];
	bool AllPus = true;
	int xc = 0;
	int yc = 0;
	int Np = 0;
	int Nu = 0;
	int BRIDX[32];
	int BRX[32];
	int BRY[32];
	int NBRG = 0;
	int ADX = 0;
	int ADY = 0;
	byte RealNI = NatRefTBL[NI];
	for (int i = 0; i < Nsel; i++) {
		word MID = SMon[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			if (OB&&OB->Serial == MSN[i]) {
				if (!OB->InArmy) {
					byte Usage = OB->newMons->Usage;
					if (Usage != PushkaID)AllPus = false;
					SSEL[Np] = MID;
					Np++;
					Nu++;
					xc += OB->RealX;
					yc += OB->RealY;
				}
				else {
					xc += OB->RealX;
					yc += OB->RealY;
					Nu++;
					if (OB->NNUM == RealNI&&OB->BrigadeID != 0xFFFF && NBRG < 32) {
						Brigade* BR = CITY[RealNI].Brigs + OB->BrigadeID;
						if (BR->ID < 8192 && BR->NMemb) {
							BRIDX[NBRG] = BR->ID;
							NBRG++;
							BR->ID += 8192;
							if (BR->WarType) {
								int vx = TCos[byte(BR->Direction)] >> 2;
								int vy = TSin[byte(BR->Direction)] >> 2;
								ADX += vx;
								ADY += vy;
							};
						};
					};
				};
			};
		};
	};
	char AverageDir = GetDir(ADX, ADY);
	int BCX = 0;
	int BCY = 0;
	for (int i = 0; i < NBRG; i++) {
		Brigade* BR = CITY[RealNI].Brigs + BRIDX[i];
		BR->ID -= 8192;
		GetBrCenter(BR, BRX + i, BRY + i);
		BCX += BRX[i];
		BCY += BRY[i];
	};
	int rdx = 16;
	int rdy = 16;
	if (LastDirection < 256) {
		rdx = int(TCos[byte(LastDirection)]) << 4;
		rdy = int(TSin[byte(LastDirection)]) << 4;
	}
	else {
		if (Nu) {
			xc /= Nu;
			yc /= Nu;
			rdx = xc - x;
			rdy = yc - y;
			LastDirection = GetDir(-rdx, -rdy);
			char DRR = LastDirection;
			if (abs(DRR - AverageDir) < 16)LastDirection = AverageDir;
		};
	};
	char ddir = LastDirection - AverageDir;
	//if(ddir>64||ddir<-63)ddir+=128;
	int SIN = TSin[byte(ddir)];
	int COS = TCos[byte(ddir)];
	if (NBRG) {
		BCX /= NBRG;
		BCY /= NBRG;
		for (int i = 0; i < NBRG; i++) {
			int dx = BRX[i] - BCX;
			int dy = BRY[i] - BCY;
			int Nr = Norma(dx, dy);
			if (Nr > 1000) {
				dx = (dx * 256) / Nr;
				dy = (dy * 256) / Nr;
			};
			BRX[i] = (x >> 4) + ((dx*COS - dy*SIN) >> 8);
			BRY[i] = (y >> 4) + ((dx*SIN + dy*COS) >> 8);
			Brigade* BR = CITY[RealNI].Brigs + BRIDX[i];
			//int Dir=BR->Direction;
			//if(NBRG==1)Dir=LastDirection;
			char Dir = BR->Direction - AverageDir + LastDirection;
			if (OneDirection)Dir = LastDirection;
			if (Prio & 127) {
				if (BR->LastOrderTime != tmtmt) {
					SetStandState(BR, 0);
					BR->AttEnm = 0;
				};
				BR->HumanGlobalSendTo(BRX[i], BRY[i], Dir, 126 + 16, OrdType);
			}
			else {
				if (!BR->Strelki) {
					BR->HumanGlobalSendTo(BRX[i], BRY[i], Dir, 126 + 16, OrdType);
					SetStandState(BR, 1);
					BR->AttEnm = true;
				}
				else {
					//BR->LastOrderTime=tmtmt;
					BR->ClearBOrders();
					BR->HumanGlobalSendTo(BRX[i], BRY[i], Dir, 128, OrdType);
					BR->LastOrderTime = tmtmt;
				};
			};
			CancelStandGroundAnyway(BR);
		};
	};
	if (Np) {
		PORD.CreateRotatedPositions(x, y, SSEL, Np, rdx, rdy);
	}
	else return;
	if (AllPus) {
		if (Np)PORD.CreateRotatedPositions2(x, y, SSEL, Np, rdx, rdy);
		//PORD.CreateLinearPositions(x,y,SSEL,Np,rdx,rdy);
	}
	else { //PORD.CreatePositions(x,y,SMon,Nsel);
		if (Np) {
			PORD.CreateRotatedPositions(x, y, SSEL, Np, rdx, rdy);
		}
		else return;
	};
	if (Np) {
		PORD.CenterX = x;
		PORD.CenterY = y;
		PORD.SendToPosition(Prio, OrdType);
	};
	free(SSEL);
};
void GroupAttackSelectedBrigadesTo(byte NI, int x, int y, byte Prio, byte OrdType) {
	int Nsel = NSL[NI];
	word* SMon = Selm[NI];
	word* MSN = SerN[NI];
	//checking for pushka
	word* SSEL = new word[NSL[NI]];
	bool AllPus = true;
	int xc = 0;
	int yc = 0;
	int Np = 0;
	int BRIDX[32];
	int BRX[32];
	int BRY[32];
	int NBRG = 0;
	for (int i = 0; i < Nsel; i++) {
		word MID = SMon[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			if (OB&&OB->Serial == MSN[i]) {
				if (!OB->InArmy) {
					byte Usage = OB->newMons->Usage;
					if (Usage != SupMortID&&Usage != PushkaID&&Usage != MortiraID)AllPus = false;
					SSEL[Np] = MID;
					Np++;
					xc += OB->RealX;
					yc += OB->RealY;
					OB->StandGround = false;
				}
				else {
					if (OB->NNUM == NI&&OB->BrigadeID != 0xFFFF && NBRG < 32) {
						Brigade* BR = CITY[NI].Brigs + OB->BrigadeID;
						if (BR->ID < 8192) {
							BRIDX[NBRG] = BR->ID;
							NBRG++;
							BR->ID += 8192;
						};
					};
				};
			};
		};
	};
	int BCX = 0;
	int BCY = 0;
	for (int i = 0; i < NBRG; i++) {
		Brigade* BR = CITY[NI].Brigs + BRIDX[i];
		BR->ID -= 8192;
		BR->GetCenter(BRX + i, BRY + i);
		BCX += BRX[i];
		BCY += BRY[i];
	};
	if (NBRG) {
		BCX /= NBRG;
		BCY /= NBRG;
		for (int i = 0; i < NBRG; i++) {
			int dx = BRX[i] - BCX;
			int dy = BRY[i] - BCY;
			int Nr = Norma(dx, dy);
			if (Nr > 1000) {
				dx = (dx * 256) / Nr;
				dy = (dy * 256) / Nr;
			};
			BRX[i] = (x >> 4) + dx;
			BRY[i] = (y >> 4) + dy;
			Brigade* BR = CITY[NI].Brigs + BRIDX[i];
			int Dir = BR->Direction;
			if (NBRG == 1)Dir = LastDirection;
			if (!BR->Strelki) {
				BR->HumanGlobalSendTo(BRX[i], BRY[i], Dir, 126 + 16, OrdType);
				SetStandState(BR, 1);
				BR->AttEnm = true;
			}
			else {
				BR->LastOrderTime = tmtmt;
				BR->ClearBOrders();
			};
			CancelStandGroundAnyway(BR);
		};
	};
	free(SSEL);
};
void GroupPatrolSelected(byte NI, int x, int y, byte Prio) {
	int Nsel = NSL[NI];
	word* SMon = Selm[NI];
	//checking for pushka
	bool AllPus = true;
	int xc = 0;
	int yc = 0;
	int Np = 0;
	for (int i = 0; i < Nsel; i++) {
		word MID = SMon[i];
		if (MID != 0xFFFF) {
			OneObject* OB = Group[MID];
			if (OB) {
				if (OB->newMons->Usage != SupMortID&&OB->newMons->Usage != PushkaID&&OB->newMons->Usage != MortiraID)AllPus = false;
				Np++;
				xc += OB->RealX;
				yc += OB->RealY;
			};
		};
	};
	int rdx = 16;
	int rdy = 16;
	if (LastDirection < 256) {
		rdx = int(TCos[byte(LastDirection)]) << 4;
		rdy = int(TSin[byte(LastDirection)]) << 4;
	}
	else {
		if (Np) {
			xc /= Np;
			yc /= Np;
			rdx = xc - x;
			rdy = yc - y;
			LastDirection = GetDir(-rdx, -rdy);
		};
	};
	if (AllPus&&Np) {
		PORD.CreateLinearPositions(x, y, SMon, Nsel, rdx, rdy);
	}
	else { //PORD.CreatePositions(x,y,SMon,Nsel);
		if (Np) {
			PORD.CreateRotatedPositions(x, y, SMon, Nsel, rdx, rdy);
		}
		else return;
	};
	PORD.CenterX = x;
	PORD.CenterY = y;
	PORD.Patrol();
};
word PositionOrder::CreateOrdPos(int x, int y, byte dir, int NIDS, word* IDS, OrderDescription* ODS) {
	int dx = TCos[dir];
	int dy = TSin[dir];
	Create(IDS, NIDS);
	//CheckSize(NIDS);
	UNISORT.CreateByLine(Ids, NUnits, dx >> 4, dy >> 4);
	UNISORT.Sort();
	int NU = UNISORT.NUids;
	CheckSize(NU);
	UNISORT.Copy(Ids);
	int Px = 0;
	int Lx1;
	int Ly = ODS->NLines;
	for (int iy = 0; iy < Ly; iy++) {
		int Nx = ODS->LineNU[iy];
		if (Nx) {
			Lx1 = NU - Px;
			if (Lx1 > Nx)Lx1 = Nx;
			UNISORT.CreateByLine(Ids + Px, Lx1, -dy >> 4, dx >> 4);
			UNISORT.Sort();
			UNISORT.Copy(Ids + Px);
			Px += Lx1;
		};
	};
	//Sorting is finished now
	//Getting of the maximal size of Unit
	int maxR = 512;
	word* IDE = Ids;
	//word MID;
	//int rr;
	//OneObject* OB;
	/*
	for(iy=0;iy<NU;iy++){
		MID=IDE[iy];
		OB=Group[MID];
		if(OB&&OB->NewMonst){
			rr=OB->newMons->Radius2;
			if(rr>maxR){
				maxR=rr;
			};
		};
	};
	*/
	//Creating the position
	maxR <<= 2;
	int Maxx = (msx << 9) - 768;
	int Maxy = (msy << 9) - 768;
	int pos = 0;
	int NR = Norma(dx, dy);
	int vx = (dx*maxR) / NR;
	int vy = (dy*maxR) / NR;
	int Dy = ODS->YShift;
	for (int iy = 0; iy < Ly; iy++) {
		int Nx = ODS->LineNU[iy];
		short* shx = ODS->Lines[iy];
		for (int ix = 0; ix < Nx; ix++) {
			if (pos < NU) {
				int sx = shx[ix];
				px[pos] = x - OScale((sx*vy + (Dy - iy - iy)*vx) >> 3);
				py[pos] = y + OScale((sx*vx + (iy + iy - Dy)*vy) >> 3);
			};
			pos++;
		};
	};
	return NU;
};
word PositionOrder::CreateSimpleOrdPos(int x, int y, byte dir, int NIDS, word* IDS, OrderDescription* ODS) {
	int dx = TCos[dir];
	int dy = TSin[dir];
	if (IDS) {
		for (int i = 0; i < NIDS; i++) {
			if (IDS[i] == 0xFFFF)return CreateOrdPos(x, y, dir, NIDS, IDS, ODS);
		};
		Create(IDS, NIDS);
	}
	else {
		CheckSize(NIDS);
	};
	//Getting of the maximal size of Unit
	int maxR = 512;
	//word* IDE=Ids;
	//word MID;
	int NU = NIDS;
	//int rr;
	//OneObject* OB;
	/*
	for(int iy=0;iy<NU;iy++){
		MID=IDE[iy];
		OB=Group[MID];
		if(OB&&OB->NewMonst){
			rr=OB->newMons->Radius2;
			if(rr>maxR){
				maxR=rr;
			};
		};
	};
	//Creating the position
	maxR<<=2;
	*/
	maxR <<= 2;
	int Maxx = (msx << 9) - 768;
	int Maxy = (msy << 9) - 768;
	int pos = 0;
	int NR = Norma(dx, dy);
	int vx = (dx*maxR) / NR;
	int vy = (dy*maxR) / NR;
	int Dy = ODS->YShift;
	int Ly = ODS->NLines;
	for (int iy = 0; iy < Ly; iy++) {
		int Nx = ODS->LineNU[iy];
		short* shx = ODS->Lines[iy];
		for (int ix = 0; ix < Nx; ix++) {
			if (pos < NU) {
				int sx = shx[ix];
				px[pos] = x - OScale((sx*vy + (Dy - iy - iy)*vx) >> 3);
				py[pos] = y + OScale((sx*vx + (iy + iy - Dy)*vy) >> 3);
			};
			pos++;
		};
	};
	return NU;
};