#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "ActiveScenary.h"
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
#include "3DMapEd.h"
#include "TopoGraf.h"

#include "Fonts.h"
#include "Safety.h"
#include "3DGraph.h"
#include "Nature.h"
#include "ConstStr.h"
class NetSample {
public:
	word* Danger;
	word* Pretty;
	int NZ;
	int LastUpdate;
	void CreateDiversantMap(byte NI);
	void CreateGrenadersMap(byte NI);
	NetSample();
	~NetSample();
};
class SafeNet {
public:
	NetSample Diversant;
	NetSample Grenader;
	/*
	int LastUpdate;
	byte NI;
	int NZ;
	word* CellDanger;
	word* CellPretty;
	*/
	word FindNextCell(int F, int Start, NetSample* Net);
	word FindWayTo(int F, int Start, int Fin, NetSample* Net);
	//void CreateMaps();
	SafeNet();
	~SafeNet();

};
NetSample::NetSample() {
	NZ = 0;
	Danger = NULL;
	Pretty = NULL;
	LastUpdate = 0;
};
NetSample::~NetSample() {
	if (Danger)free(Danger);
	if (Pretty)free(Pretty);
	NZ = 0;
	Danger = NULL;
	Pretty = NULL;
};
SafeNet::SafeNet() {
};
SafeNet::~SafeNet() {
};
extern int tmtmt;
void NetSample::CreateDiversantMap(byte NI) {
	if (tmtmt - LastUpdate < 20 && NZ == NAreas)return;
	if (NZ != NAreas) {
		Danger = (word*)realloc(Danger, 2 * NAreas);
		Pretty = (word*)realloc(Pretty, 2 * NAreas);
		NZ = NAreas;
	};
	int N2 = 2 * NAreas;
	word addDam[2048];
	memset(Danger, 0, N2);
	memset(Pretty, 0, N2);
	memset(addDam, 0, N2);
	byte Mask = 1 << NI;
	for (int i = 0; i < MAXOBJECT; i++) {
		OneObject* OB = Group[i];
		if (OB && !(OB->Sdoxlo || OB->NMask&Mask)) {
			NewMonster* NM = OB->newMons;
			byte Use = NM->Usage;
			int Pret = 0;
			int Dang = 0;
			if (NM->Capture) {
				if (NM->Building) {
					if (NM->Producer)Pret = 40;
					else {
						switch (Use) {
						case CenterID:
							Pret = 40;
							break;
						case MelnicaID:
							Pret = 20;
							break;
						case SkladID:
							Pret = 0;
							break;
						default:
							Pret = 20;
						};
					};
				};
			}
			else {
				switch (Use) {
				case HardHorceID:
					Dang = 3;
					break;
				case FastHorseID:
					Dang = 2;
					break;
				case PeasantID:
					Pret = 1;
					break;
				default:
					if (OB->AddDamage > 5)Dang = 2;
					else Dang = 1;
				};
			};
			if (Dang || Pret) {
				int cx = OB->RealX >> 10;
				int cy = OB->RealY >> 10;
				int ofs = cx + (cy << TopSH);
				if (ofs >= 0 || ofs < TopLx*TopLy) {
					word Top = TopRef[ofs];
					if (Top < 0xFFFE) {
						Danger[Top] += Dang;
						Pretty[Top] += Pret;
					};
				};
			};
		};
	};
	Area* AR = TopMap;
	for (int i = 0; i < NAreas; i++) {
		if (Danger[i] > 2) {
			int NL = AR->NLinks;
			for (int j = 0; j < NL; j++) {
				int id = AR->Link[j + j];
				int dam = Danger[id];
				if (dam) {
					addDam[i] += (dam >> 1) + (dam >> 2);
				};
			};
		};
		AR++;
	};
	for (int i = 0; i < NAreas; i++)Danger[i] += addDam[i];
};
void NetSample::CreateGrenadersMap(byte NI) {
	if (tmtmt - LastUpdate < 20 && NZ == NAreas)return;
	if (NZ != NAreas) {
		Danger = (word*)realloc(Danger, 2 * NAreas);
		Pretty = (word*)realloc(Pretty, 2 * NAreas);
		NZ = NAreas;
	};
	int N2 = 2 * NAreas;
	word addDam[2048];
	memset(Danger, 0, N2);
	memset(Pretty, 0, N2);
	memset(addDam, 0, N2);
	byte Mask = 1 << NI;
	for (int i = 0; i < MAXOBJECT; i++) {
		OneObject* OB = Group[i];
		int Pret = 0;
		int Dang = 0;
		if (OB && (!OB->Sdoxlo) && !(OB->NMask&Mask)) {
			NewMonster* NM = OB->newMons;
			byte Use = NM->Usage;
			if (OB->Wall) {
				if (!(OB->Ready&&OB->SafeWall)) {
					int cx = OB->RealX >> 10;
					int cy = OB->RealY >> 10;
					if (cx > 0 && cx < TopLx - 1 && cy>0 && cy < TopLy - 1) {
						int ofs = cx + (cy << TopSH);
						word Top = TopRef[ofs + 1];
						if (Top < 0xFFFE)Pretty[Top] += 10;
						Top = TopRef[ofs - 1];
						if (Top < 0xFFFE)Pretty[Top] += 10;
						Top = TopRef[ofs - TopLx];
						if (Top < 0xFFFE)Pretty[Top] += 10;
						Top = TopRef[ofs + TopLx];
						if (Top < 0xFFFE)Pretty[Top] += 10;
					};
				};
			}
			else {
				if (Use == TowerID)Pret = 20;
				else
					if (Use == StrelokID || Use == HorseStrelokID || Use == HardHorceID || Use == FastHorseID )Dang = 3;
					else {
						if (NM->Capture) {
							Pret = 5;
						};
						if (Use == LightInfID)Dang = 1;
					};
					if (Dang || Pret) {
						int cx = OB->RealX >> 10;
						int cy = OB->RealY >> 10;
						int ofs = cx + (cy << TopSH);
						if (ofs >= 0 || ofs < TopLx*TopLy) {
							word Top = TopRef[ofs];
							if (Top < 0xFFFE) {
								Danger[Top] += Dang;
								Pretty[Top] += Pret;
							};
						};
					};
			};
		};
	};
};
#define MAXAR 700
word SafeNet::FindNextCell(int F, int Cell, NetSample* Net) {
	int NZ = Net->NZ;
	if (Cell < NZ) {
		int F0 = F >> 2;
		int F1 = F >> 1;
		int F2 = F0 + F1;
		int F3 = F + F0;
		word* CellDanger = Net->Danger;
		word* CellPretty = Net->Pretty;
		word AddPrett[2048];
		memcpy(AddPrett, CellPretty, NZ << 1);
		for (int i = 0; i < NAreas; i++) {
			word cdn = CellDanger[i];
			if (cdn <= F1&&cdn > 3) {
				AddPrett[i] += 20;
			};
		};
		if (AddPrett[Cell])return Cell;
		byte CantGo[MAXAR];
		for (int i = 0; i < NAreas; i++) {
			word dang = CellDanger[i];
			CantGo[i] = dang > F1;
		};
		Area* AR = TopMap;
		for (int i = 0; i < NAreas; i++) {
			if (CantGo[i] == 1) {
				int NL = AR->NLinks;
				for (int j = 0; j < NL; j++) {
					word id = AR->Link[j + j];
					if (!CantGo[id])CantGo[id] = 2;
				};
			};
			AR++;
		};

		//now we are ready to find a way

		word DistArr[2048];
		word DistID[2048];
		byte Checked[2048];

		memset(DistArr, 0, NAreas << 1);
		memset(Checked, 0, NAreas);
		word BoundCells[200];
		int NBound = 0;
		word NewBound[200];
		int NNewB = 0;

		NBound = 1;
		BoundCells[0] = Cell;
		DistArr[Cell] = 1;
		word LastPrettyID = 0xFFFF;
		do {
			NNewB = 0;
			for (int i = 0; i < NBound; i++)Checked[BoundCells[i]] = 1;
			for (int i = 0; i < NBound; i++) {
				int stp = BoundCells[i];
				Area* BA = TopMap + stp;
				int N = BA->NLinks;
				int L0 = DistArr[i];
				for (int j = 0; j < N; j++) {
					word id = BA->Link[j + j];
					if (!(CantGo[id] || Checked[id] == 1)) {
						int d = L0 + BA->Link[j + j + 1];
						if (DistArr[id]) {
							if (d < DistArr[id]) {
								DistID[id] = stp;
								DistArr[id] = d;
								if (AddPrett[id]) {
									LastPrettyID = id;
								};
							};
						}
						else {
							DistID[id] = stp;
							DistArr[id] = d;
							if (AddPrett[id]) {
								LastPrettyID = id;
							};
						};
						if (!Checked[id]) {
							Checked[id] = 2;
							NewBound[NNewB] = id;
							NNewB++;
						};
					};
				};
			};
			memcpy(BoundCells, NewBound, NNewB << 1);
			NBound = NNewB;
		} while (LastPrettyID == 0xFFFF && NNewB);
		if (LastPrettyID != 0xFFFF) {
			int PreCell = LastPrettyID;
			while (LastPrettyID != Cell) {
				PreCell = LastPrettyID;
				LastPrettyID = DistID[LastPrettyID];
			};
			return PreCell;
		};
		return 0xFFFF;
	};
	return 0xFFFF;
};

word SafeNet::FindWayTo(int F, int Cell, int Fin, NetSample* Net) {
	//CreateMaps();
	if (Cell < Net->NZ) {
		word* CellDanger = Net->Danger;
		int F0 = F >> 2;
		int F1 = F >> 1;
		int F2 = F0 + F1;
		int F3 = F + F0;
		byte CantGo[MAXAR];
		for (int i = 0; i < NAreas; i++) {
			word dang = CellDanger[i];
			CantGo[i] = dang > F1;
		};

		//now we are ready to find a way

		word DistArr[2048];
		word DistID[2048];
		byte Checked[2048];

		memset(DistArr, 0, NAreas << 1);
		memset(Checked, 0, NAreas);
		word BoundCells[200];
		int NBound = 0;
		word NewBound[200];
		int NNewB = 0;

		NBound = 1;
		BoundCells[0] = Cell;
		DistArr[Cell] = 1;
		word LastPrettyID = 0xFFFF;
		do {
			NNewB = 0;
			for (int i = 0; i < NBound; i++)Checked[BoundCells[i]] = 1;
			for (int i = 0; i < NBound; i++) {
				int stp = BoundCells[i];
				Area* BA = TopMap + stp;
				int N = BA->NLinks;
				int L0 = DistArr[i];
				for (int j = 0; j < N; j++) {
					word id = BA->Link[j + j];
					if (!(CantGo[id] || Checked[id] == 1)) {
						int d = L0 + BA->Link[j + j + 1];
						if (DistArr[id]) {
							if (d < DistArr[id]) {
								DistID[id] = stp;
								DistArr[id] = d;
								if (id == Fin) {
									LastPrettyID = id;
								};
							};
						}
						else {
							DistID[id] = stp;
							DistArr[id] = d;
							if (id == Fin) {
								LastPrettyID = id;
							};
						};
						if (!Checked[id]) {
							Checked[id] = 2;
							NewBound[NNewB] = id;
							NNewB++;
						};
					};
				};
			};
			memcpy(BoundCells, NewBound, NNewB << 1);
			NBound = NNewB;
		} while (LastPrettyID == 0xFFFF && NNewB);
		if (LastPrettyID != 0xFFFF) {
			int PreCell = LastPrettyID;
			while (LastPrettyID != Cell) {
				PreCell = LastPrettyID;
				LastPrettyID = DistID[LastPrettyID];
			};
			return PreCell;
		};
		return 0xFFFF;
	};
	return 0xFFFF;
};
SafeNet SAFNET[8];
word GetNextSafeCell(byte NI, int F, int start, int Fin) {
	SAFNET[NI].Diversant.CreateDiversantMap(NI);
	return SAFNET[NI].FindWayTo(F, start, Fin, &SAFNET[NI].Diversant);
};
word GetNextDivCell(byte NI, int F, int Start) {
	SAFNET[NI].Diversant.CreateDiversantMap(NI);
	return SAFNET[NI].FindNextCell(F, Start, &SAFNET[NI].Diversant);
};
word GetNextGreCell(byte NI, int F, int Start) {
	SAFNET[NI].Grenader.CreateGrenadersMap(NI);
	return SAFNET[NI].FindNextCell(F, Start, &SAFNET[NI].Grenader);
};