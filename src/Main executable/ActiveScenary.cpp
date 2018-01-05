#include "ddini.h"
#include <stdlib.h>
#include "ResFile.h"
#include "Fastdraw.h"
#include "MapDiscr.h"
#include "mouse.h"
#include "mode.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <crtdbg.h>
#include <math.h>

#include "Megapolis.h"

#include "fonts.h"
#include "WeaponID.h"

#include "3DSurf.h"
#include "GSound.h"
#include <assert.h>
#include "3DMapEd.h"
#include "MapSprites.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "ZBuffer.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "StrategyResearch.h"
#include "Curve.h"
#include "NewMon.h"
#include "Sort.h"
#include "ActiveZone.h"
#include "ActiveScenary.h"
#include "DrawForm.h"
#include "recorder.h"
#include "Safety.h "
#include "EInfoClass.h"

#include "PlayerInfo.h"
extern PlayerInfo PINFO[8];

extern bool NOPAUSE;
extern bool LockPause;
extern int CurrentAnswer;
byte AssignTBL[8] = {0, 1, 2, 3, 4, 5, 6, 7};
void AIER(char* Mess);
bool AiIsRunNow = false;

ScenaryInterface SCENINF;

ScenaryInterface::ScenaryInterface()
{
	memset(this, 0, sizeof ScenaryInterface);
}

ScenaryInterface::~ScenaryInterface()
{
	UnLoading();
}

void HandleMission()
{
	if (SCENINF.ScenaryHandler)SCENINF.ScenaryHandler();
	if (SCENINF.CTextTime > 0)
	{
		SCENINF.CTextTime -= 8 << SpeedSh;
		if (SCENINF.CTextTime < 0)SCENINF.CTextTime = 0;
	}
	GTimer* GTM = SCENINF.TIME;
	for (int i = 0; i < 32; i++)
	{
		if (GTM->Time)
		{
			GTM->Time -= 8 << SpeedSh;
			if (GTM->Time <= 0)
			{
				GTM->Time = 0;
				GTM->First = 1;
				GTM->Used = 1;
			}
		}
		GTM++;
	}
}

void FogSpot(int x, int y);

void ScenaryLights()
{
	for (int i = 0; i < 64; i++)
	{
		int xx = SCENINF.LSpot[i].x;
		int yy = SCENINF.LSpot[i].y;
		int Type = SCENINF.LSpot[i].Type;
		if (xx)
		{
			int rr = 200;
			int rr1 = rr - (rr >> 2);
			yy -= (GetHeight(xx, yy) << 1);
			switch (Type)
			{
			case 0:
				FogSpot(xx, yy);
				break;
			case 1:
				FogSpot(xx + 128, yy);
				FogSpot(xx - 128, yy);
				FogSpot(xx, yy + 128);
				FogSpot(xx, yy - 128);
				break;
			case 2:
				FogSpot(xx + 128, yy + 128);
				FogSpot(xx - 128, yy + 128);
				FogSpot(xx + 128, yy - 128);
				FogSpot(xx - 128, yy - 128);
				break;
			case 3:
				FogSpot(xx + 2 * 128, yy);
				FogSpot(xx - 2 * 128, yy);
				FogSpot(xx, yy - 2 * 128);
				FogSpot(xx, yy + 2 * 128);
				FogSpot(xx + 128, yy + 128);
				FogSpot(xx - 128, yy + 128);
				FogSpot(xx + 128, yy - 128);
				FogSpot(xx - 128, yy - 128);
				break;
			case 4:
				FogSpot(xx + 3 * 128, yy);
				FogSpot(xx - 3 * 128, yy);
				FogSpot(xx, yy + 3 * 128);
				FogSpot(xx, yy - 3 * 128);
				FogSpot(xx + 2 * 128, yy + 2 * 128);
				FogSpot(xx - 2 * 128, yy + 2 * 128);
				FogSpot(xx + 2 * 128, yy - 2 * 128);
				FogSpot(xx - 2 * 128, yy - 2 * 128);
				break;
			case 5:
				FogSpot(xx + 4 * 128, yy);
				FogSpot(xx - 4 * 128, yy);
				FogSpot(xx, yy + 4 * 128);
				FogSpot(xx, yy - 4 * 128);
				FogSpot(xx + 3 * 128, yy + 3 * 128);
				FogSpot(xx - 3 * 128, yy + 3 * 128);
				FogSpot(xx + 3 * 128, yy - 3 * 128);
				FogSpot(xx - 3 * 128, yy - 3 * 128);
				break;
			case 6:
				FogSpot(xx + 5 * 128, yy);
				FogSpot(xx - 5 * 128, yy);
				FogSpot(xx, yy + 5 * 128);
				FogSpot(xx, yy - 5 * 128);
				FogSpot(xx + 4 * 128, yy + 4 * 128);
				FogSpot(xx - 4 * 128, yy + 4 * 128);
				FogSpot(xx + 4 * 128, yy - 4 * 128);
				FogSpot(xx - 4 * 128, yy - 4 * 128);
				break;
			case 7:
				FogSpot(xx + 6 * 128, yy);
				FogSpot(xx - 6 * 128, yy);
				FogSpot(xx, yy + 6 * 128);
				FogSpot(xx, yy - 6 * 128);
				FogSpot(xx + 4 * 128, yy + 4);
				FogSpot(xx - 4 * 128, yy + 4 * 128);
				FogSpot(xx + 4 * 128, yy - 4 * 128);
				FogSpot(xx - 4 * 128, yy - 4 * 128);
				break;
			default:
				FogSpot(xx + 7 * 128, yy);
				FogSpot(xx - 7 * 128, yy);
				FogSpot(xx, yy + 7 * 128);
				FogSpot(xx, yy - 7 * 128);
				FogSpot(xx + 5 * 128, yy + 3);
				FogSpot(xx - 5 * 128, yy + 5 * 128);
				FogSpot(xx + 5 * 128, yy - 5 * 128);
				FogSpot(xx - 5 * 128, yy - 5 * 128);
			}
		}
	}
}

void ScenaryInterface::UnLoading()
{
	for (int i = 0; i < 8; i++)AssignTBL[i] = i;
	if (MaxSaves)
	{
		free(SaveZone);
		free(SaveSize);
	}
	if (DLLName)free(DLLName);
	if (MaxUGRP)
	{
		for (int i = 0; i < NUGRP; i++)
		{
			if (UGRP[i].N)
			{
				free(UGRP[i].IDS);
				free(UGRP[i].SNS);
			}
		}
		free(UGRP);
	}
	if (MaxUPOS)
	{
		for (int i = 0; i < NUPOS; i++)
		{
			if (UPOS[i].N)
			{
				free(UPOS[i].coor);
				free(UPOS[i].Type);
			}
		}
		free(UPOS);
	}
	if (MaxZGRP)
	{
		for (int i = 0; i < NZGRP; i++)
		{
			if (ZGRP[i].N)
			{
				free(ZGRP[i].ZoneID);
			}
		}
		free(ZGRP);
	}
	if (MaxMess)
	{
		free(Messages);
	}
	if (MaxSnds)
	{
		for (int i = 0; i < NSnd; i++)free(Sounds[i]);
		free(Sounds);
	}
	//if(MissText)free(MissText);
	if (MaxPages)
	{
		for (int i = 0; i < NPages; i++)
		{
			free(Page[i]);
			free(PageID[i]);
			free(PageBMP[i]);
		}
		free(Page);
		free(PageID);
		free(PageBMP);
		free(PageSize);
	}
	if (hLib)FreeLibrary(hLib);
	memset(this, 0, sizeof ScenaryInterface);
}

void ScenErr(char* Mess)
{
	ResFile FFF = RReset("ERR.LOG");
	if (FFF != INVALID_HANDLE_VALUE)
	{
		RClose(FFF);
		MessageBox(NULL, Mess, "Scenary error...", MB_TOPMOST);
		assert( false );
	}
}

void ScenErr(char* Mess, char* Par1)
{
	char ccc[160];
	sprintf(ccc, Mess, Par1);
	ScenErr(ccc);
}

void ScenErr(char* Mess, char* Par1, char* Par2)
{
	char ccc[160];
	sprintf(ccc, Mess, Par1, Par2);
	ScenErr(ccc);
}

int GetCPos(char C)
{
	if (C >= 'A' && C <= 'Z')
	{
		return C - 'A';
	}
	if (C >= 'a' && C <= 'z')
	{
		return 26 + C - 'a';
	}
	return 0;
}

void CreateMissText()
{
	int TextSize = 0; //SCENINF.TextSize;
	char* MissText = NULL; //SCENINF.MissText;
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(SCENINF.PageID[i], "#BRIEFING"))
		{
			MissText = SCENINF.Page[i];
			TextSize = SCENINF.PageSize[i];
		}
	}
	if (TextSize && MissText)
	{
		char* TmpText = new char[TextSize];
		int sz = 0;
		bool Visible = true;
		for (int i = 0; i < TextSize; i++)
		{
			char c = MissText[i];
			if (c != '{')
			{
				if (Visible)
				{
					TmpText[sz] = c;
					sz++;
				}
			}
			else
			{
				int pos = GetCPos(MissText[i + 1]);
				if (pos >= 0 && pos < 52)
				{
					Visible = !SCENINF.TextDisable[pos];
				}
				i += 2;
			}
		}
		ResFile RF = RRewrite("Miss.txt");
		if (RF != INVALID_HANDLE_VALUE)
		{
			RBlockWrite(RF, TmpText, sz);
			RClose(RF);
		}
		free(TmpText);
	}
	else
	{
		ResFile RF = RRewrite("Miss.txt");
		if (RF != INVALID_HANDLE_VALUE)
		{
			RClose(RF);
		}
	}
}

int ReadStr(char* Dst, char* Src, char c)
{
	int L = 0;
	while (Src[L] != c && Src[L] != 0)
	{
		Dst[L] = Src[L];
		L++;
	}
	Dst[L] = 0;
	return L;
}

extern bool ProtectionMode;

extern word COMPSTART[8];

void ScenaryInterface::Load(char* Name, char* Text)
{
	for (int i = 0; i < 8; i++)AssignTBL[i] = i;
	//ProtectionMode=1;
	memset(COMPSTART, 0, sizeof COMPSTART);
	ResFile RF = RReset(Text);
	//ProtectionMode=0;
	if (RF != INVALID_HANDLE_VALUE)
	{
		int sz = RFileSize(RF);
		if (sz)
		{
			char* TMP = new char[sz + 1];
			char* TMP1 = new char[sz + 1];
			RBlockRead(RF, TMP1, sz);
			char* STR0 = TMP1;
			TMP1[sz] = 0;
			bool exit = false;
			do
			{
				int L = ReadStr(TMP, STR0, ' ');
				if (STR0[0] == '#')
				{
					if (NPages >= MaxPages)
					{
						MaxPages += 32;
						Page = (char**)realloc(Page, 4 * MaxPages);
						PageID = (char**)realloc(PageID, 4 * MaxPages);
						PageBMP = (char**)realloc(PageBMP, 4 * MaxPages);
						PageSize = (int*)realloc(PageSize, 4 * MaxPages);
					}
					STR0 += L + 1;
					PageID[NPages] = new char[strlen(TMP) + 1];
					strcpy(PageID[NPages], TMP);
					L = ReadStr(TMP, STR0, 0x0D);
					STR0 += L;
					PageBMP[NPages] = new char[strlen(TMP) + 1];
					strcpy(PageBMP[NPages], TMP);
					if (STR0[0] == 0x0D)STR0 += 2;
					L = ReadStr(TMP, STR0, '#');
					Page[NPages] = new char[strlen(TMP) + 1];
					PageSize[NPages] = L;
					strcpy(Page[NPages], TMP);
					NPages++;
					STR0 += L;
				}
				else exit = true;
			}
			while (!exit);
			free(TMP);
			free(TMP1);
			//MissText=new char[sz];
			//TextSize=sz;
			//RBlockRead(RF,MissText,sz);
		}
		CreateMissText();
		RClose(RF);
	}
	char cc3[200];
	strcpy(cc3, Name);
	_strupr(cc3);
	if (strstr(cc3, ".CMS"))
	{
		FILE* F = fopen("UserMissions\\start.dat", "w");
		if (F)
		{
			fprintf(F, "%s", cc3);
			fclose(F);
			hLib = LoadLibrary("UserMissions\\CMS_start.dll");
		}
	}
	else
	{
		if (hLib)FreeLibrary(hLib);
		hLib = LoadLibrary(Name);
	}
	ScenaryHandler = NULL;
	if (hLib == NULL)
	{
		ScenErr("Can't load DLL:%s", Name);
	}
	else
	{
		NErrors = 0;
		ScenaryHandler = (StdVoid*)GetProcAddress(hLib, "ProcessScenary");
		if (!ScenaryHandler)
		{
			ScenErr("%s: can't find function:void ProcessScenary()", Name);
		}
		if (NErrors)
		{
			ScenErr("%s:Errors was found during initialisation.", Name);
			ScenaryHandler = NULL;
		}
	}
}

//-------------Registration commands--------------
void IntErr(char* Mess)
{
	ResFile FFF = RReset("ERR.LOG");
	if (FFF != INVALID_HANDLE_VALUE)
	{
		RClose(FFF);
		if (AiIsRunNow)
		{
			MessageBox(NULL, Mess, "AI registration error...", MB_TOPMOST);
		}
		else
		{
			MessageBox(NULL, Mess, SCENINF.DLLName, MB_TOPMOST);
		}
	}
}

void IntErr(char* Mess, char* s1)
{
	char ccc[256];
	sprintf(ccc, Mess, s1);
	IntErr(ccc);
}

void TestUnitsGroup(int id)
{
	UnitsGroup* AG = SCENINF.UGRP + id;
	int N = AG->N;
	word* idx = AG->IDS;
	word* sns = AG->SNS;
	for (int i = 0; i < N; i++)
	{
		word MID = idx[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (!OB->Sdoxlo) && (OB->Serial & 0xFFFE) == (sns[i] & 0xFFFE))
			{
				sns[i] = OB->Serial;
			}
		}
	}
}

extern "C" __declspec( dllexport ) void AssignNation(byte Src, byte Dst)
{
	if (Src < 8 && Dst < 8)AssignTBL[Src] = Dst;
}

extern "C" __declspec( dllexport ) bool RegisterUnits(GAMEOBJ* GOBJ, char* Name)
{
	for (int i = 0; i < NAGroups; i++)
		if (!strcmp(AGroups[i].Name, Name))
		{
			GOBJ->Type = 'UNIT';
			if (SCENINF.NUGRP >= SCENINF.MaxUGRP)
			{
				SCENINF.MaxUGRP += 32;
				SCENINF.UGRP = (UnitsGroup*)realloc(SCENINF.UGRP, SCENINF.MaxUGRP * sizeof UnitsGroup);
			}
			GOBJ->Index = SCENINF.NUGRP;
			UnitsGroup* UG = SCENINF.UGRP + SCENINF.NUGRP;
			UG->IDS = new word[AGroups[i].N];
			UG->SNS = new word[AGroups[i].N];
			UG->N = AGroups[i].N;
			memcpy(UG->IDS, AGroups[i].Units, 2 * UG->N);
			memcpy(UG->SNS, AGroups[i].Serials, 2 * UG->N);
			SCENINF.NUGRP++;
			GOBJ->Serial = 0;
			return true;
		}
	IntErr("RegisterUnits : Unknown ID of group : %s", Name);
	SCENINF.NErrors++;
	return false;
}

//Do not remove: Needed elsewhere, loaded at runtime, causes error if not defined
extern "C" __declspec( dllexport ) bool RegisterUnitsForm(GAMEOBJ* GOBJ, char* Name)
{
	return false;
}

char* GetTextByID(char* ID);

extern "C" __declspec( dllexport ) bool RegisterString(GAMEOBJ* GOBJ, char* ID)
{
	char* id = GetTextByID(ID);
	if (!strcmp(id, ID))
	{
		IntErr("RegisterString : Unknown string ID : %s ,(see COMMENT.TXT)", ID);
		SCENINF.NErrors++;
	}

	GOBJ->Type = 'STRI';
	GOBJ->Index = SCENINF.NMess;
	GOBJ->Serial = 0;
	if (SCENINF.NMess >= SCENINF.MaxMess)
	{
		SCENINF.MaxMess += 64;
		SCENINF.Messages = (char**)realloc(SCENINF.Messages, SCENINF.MaxMess * 4);
	}

	SCENINF.Messages[SCENINF.NMess] = id;
	SCENINF.NMess++;
	return true;
}

extern "C" __declspec( dllexport ) bool RegisterSound(GAMEOBJ* GOBJ, char* Name)
{
	GOBJ->Type = 'SOUN';
	GOBJ->Index = SCENINF.NSnd;
	GOBJ->Serial = 0;
	if (SCENINF.NSnd >= SCENINF.MaxSnds)
	{
		SCENINF.MaxSnds += 32;
		SCENINF.Sounds = (char**)realloc(SCENINF.Sounds, 4 * SCENINF.MaxSnds);
	}

	SCENINF.Sounds[SCENINF.NSnd] = new char[strlen(Name) + 1];
	strcpy(SCENINF.Sounds[SCENINF.NSnd], Name);
	SCENINF.NSnd++;
	return true;
}

extern "C" __declspec( dllexport ) void RegisterVar(void* Var, int size)
{
	if (SCENINF.NSaves >= SCENINF.MaxSaves)
	{
		SCENINF.MaxSaves += 32;
		SCENINF.SaveZone = (void**)realloc(SCENINF.SaveZone, 4 * SCENINF.MaxSaves);
		SCENINF.SaveSize = (int*)realloc(SCENINF.SaveSize, 4 * SCENINF.MaxSaves);
	}

	SCENINF.SaveSize[SCENINF.NSaves] = size;
	SCENINF.SaveZone[SCENINF.NSaves] = Var;
	SCENINF.NSaves++;
}

extern "C" __declspec( dllexport ) void RegisterZone(GAMEOBJ* GOBJ, char* Name)
{
	int NZON = 0;
	word ZIDS[64];
	for (int i = 0; i < NAZones; i++)
	{
		if (!strcmp(Name, AZones[i].Name))
		{
			if (NZON < 64)
			{
				ZIDS[NZON] = i;
				NZON++;
				AZones[i].Dir = 0;
			}
		}
	}

	if (NZON)
	{
		if (SCENINF.NZGRP >= SCENINF.MaxZGRP)
		{
			SCENINF.MaxZGRP += 16;
			SCENINF.ZGRP = (ZonesGroup*)realloc(SCENINF.ZGRP, SCENINF.MaxZGRP * sizeof ZonesGroup);
		}

		SCENINF.ZGRP[SCENINF.NZGRP].N = NZON;
		SCENINF.ZGRP[SCENINF.NZGRP].ZoneID = new word[NZON * 2];
		memcpy(SCENINF.ZGRP[SCENINF.NZGRP].ZoneID, ZIDS, NZON * 2);
		GOBJ->Index = SCENINF.NZGRP;
		GOBJ->Serial = 0;
		GOBJ->Type = 'ZONE';
		SCENINF.NZGRP++;
	}
	else
	{
		IntErr("RegisterZone : Unknown zone name : %s", Name);
		SCENINF.NErrors++;
	}
}

extern "C" __declspec( dllexport ) void RegisterVisibleZone(GAMEOBJ* GOBJ, char* Name)
{
	int NZON = 0;
	word ZIDS[64];
	for (int i = 0; i < NAZones; i++)
	{
		if (!strcmp(Name, AZones[i].Name))
		{
			if (NZON < 64)
			{
				ZIDS[NZON] = i;
				NZON++;
				AZones[i].Dir = 1;
			}
		}
	}

	if (NZON)
	{
		if (SCENINF.NZGRP >= SCENINF.MaxZGRP)
		{
			SCENINF.MaxZGRP += 16;
			SCENINF.ZGRP = (ZonesGroup*)realloc(SCENINF.ZGRP, SCENINF.MaxZGRP * sizeof ZonesGroup);
		}

		SCENINF.ZGRP[SCENINF.NZGRP].N = NZON;
		SCENINF.ZGRP[SCENINF.NZGRP].ZoneID = new word[NZON * 2];
		memcpy(SCENINF.ZGRP[SCENINF.NZGRP].ZoneID, ZIDS, NZON * 2);
		GOBJ->Index = SCENINF.NZGRP;
		GOBJ->Serial = 0;
		GOBJ->Type = 'ZONE';
		SCENINF.NZGRP++;
	}
	else
	{
		IntErr("RegisterZone : Unknown zone name : %s", Name);
		SCENINF.NErrors++;
	}
}

extern "C" __declspec( dllexport ) bool RegisterUnitType(GAMEOBJ* GOBJ, char* Name)
{
	GeneralObject** GOS = NATIONS[0].Mon;
	int N = NATIONS[0].NMon;
	for (int i = 0; i < N; i++)
	{
		if (!strcmp(GOS[i]->MonsterID, Name))
		{
			GOBJ->Index = i;
			GOBJ->Serial = 0;
			GOBJ->Type = 'UTYP';
			return true;
		}
	}

	IntErr("RegisterUnitType : Unknown unit type : %s", Name);
	SCENINF.NErrors++;
	return false;
}

extern "C" __declspec( dllexport ) bool RegisterUpgrade(GAMEOBJ* GOBJ, char* Name)
{
	NewUpgrade** NUP = NATIONS[0].UPGRADE;
	int N = NATIONS[0].NUpgrades;
	for (int i = 0; i < N; i++)
	{
		if (!strcmp(NUP[i]->Name, Name))
		{
			GOBJ->Index = i;
			GOBJ->Serial = 0;
			GOBJ->Type = 'UPGR';
			return true;
		}
	}
	IntErr("RegisterUpgrade : Unknown upgrade : %s", Name);
	SCENINF.NErrors++;
	return false;
}

extern "C" __declspec( dllexport ) bool SafeRegisterUpgrade(GAMEOBJ* GOBJ, char* Name)
{
	NewUpgrade** NUP = NATIONS[0].UPGRADE;
	int N = NATIONS[0].NUpgrades;
	for (int i = 0; i < N; i++)
	{
		if (!strcmp(NUP[i]->Name, Name))
		{
			GOBJ->Index = i;
			GOBJ->Serial = 0;
			GOBJ->Type = 'UPGR';
			return true;
		}
	}

	GOBJ->Index = 0;
	GOBJ->Serial = 0;
	GOBJ->Type = 'SAFE';
	return true;
}

void PerformNewUpgrade(Nation* NT, int UIndex, OneObject* OB);

extern "C" __declspec( dllexport ) void InitialUpgrade(char* Grp, char* Upgrade)
{
	for (int i = 0; i < NAGroups; i++)
	{
		if (!strcmp(AGroups[i].Name, Grp))
		{
			GAMEOBJ UP;
			if (RegisterUpgrade(&UP, Upgrade))
			{
				int Nu = AGroups[i].N;
				word* SNS = AGroups[i].Serials;
				word* UNI = AGroups[i].Units;
				for (int j = 0; j < Nu; j++)
				{
					word MID = UNI[j];
					if (MID != 0xFFFF)
					{
						OneObject* OB = Group[MID];
						if (OB && OB->Serial == SNS[j] && !OB->Sdoxlo)
						{
							PerformNewUpgrade(OB->Nat, UP.Index, OB);
							NewUpgrade* NU = OB->Nat->UPGRADE[UP.Index];
							if (!(NU->Individual || NU->StageUp))
							{
								NU->Done = true;
								NU->PermanentEnabled = 0;
								NU->Enabled = 0;
							}
							return;
						}
					}
				}
			}
			return;
		}
	}

	IntErr("InitialUpgrade : Unknown <Group>");
}

extern "C" __declspec( dllexport ) void DisableUpgrade(byte Nat, GAMEOBJ* Upg)
{
	Nat = AssignTBL[Nat];
	if (Nat < 8)
	{
		if (Upg->Type != 'UPGR')
		{
			IntErr("DisableUpgrade : Incorrect parameter <Upgrade>");
			SCENINF.NErrors++;
			return;
		}
		NewUpgrade* NU = NATIONS[Nat].UPGRADE[Upg->Index];
		NU->ManualDisable = 1;
	}
}

extern "C" __declspec( dllexport ) void EnableUpgrade(byte Nat, GAMEOBJ* Upg)
{
	Nat = AssignTBL[Nat];
	if (Nat < 8)
	{
		if (Upg->Type != 'UPGR')
		{
			IntErr("DisableUpgrade : Incorrect parameter <Upgrade>");
			SCENINF.NErrors++;
			return;
		}
		NewUpgrade* NU = NATIONS[Nat].UPGRADE[Upg->Index];
		NU->ManualDisable = 0;
	}
}

extern "C" __declspec( dllexport ) void EnableUnit(byte Nat, GAMEOBJ* Type, bool State)
{
	Nat = AssignTBL[Nat];
	if (Nat < 8)
	{
		if (Type->Type != 'UTYP')
		{
			IntErr("EnableUnit : Incorrect parameter <Type>");
			SCENINF.NErrors++;
			return;
		}
		GeneralObject* GO = NATIONS[Nat].Mon[Type->Index];
		GO->ManualDisable = !State;
		GO->MoreCharacter->Changed = 1;
		if (State)
		{
			GO->Enabled = 1;
		}
	}
}

extern "C" __declspec( dllexport ) bool RegisterFormation(GAMEOBJ* GOBJ, char* Name)
{
	for (int i = 0; i < NEOrders; i++)
	{
		if (!strcmp(ElementaryOrders[i].ID, Name))
		{
			GOBJ->Index = i;
			GOBJ->Serial = 0;
			GOBJ->Type = 'FORM';
			return true;
		}
	}
	IntErr("RegisterFormation : Unknown formation type :%s  (see ORDERS.LST)", Name);
	SCENINF.NErrors++;
	return false;
}

//---------------Checking commands----------------
//1.In zones
extern "C" __declspec( dllexport ) int GetUnitsAmount0(GAMEOBJ* Zone, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (Nation >= 8)
	{
		IntErr("GetUnitsAmount(Zone,Nation) : Incorrect parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}

	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		int R0 = Zone->Type & 0x00FFFFFF;
		int R = (R0 >> 7) + 2;
		int cx = Zone->Index >> 7;
		int cy = Zone->Serial >> 7;

		int zx = int(Zone->Index) << 4;
		int zy = int(Zone->Serial) << 4;
		int Rz = R0 << 4;

		int mxx = msx >> 2;
		int myy = msy >> 2;
		int NU = 0;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->NNUM == Nation && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
								{
									NU++;
								}
							}
						}
					}
				}
			}
		}
		return NU;
	}

	if ((!Zone) || Zone->Type != 'ZONE')
	{
		IntErr("GetUnitsAmount(Zone,Nation) : Incorrect parameter <Zone>");
		SCENINF.NErrors++;
		return 0;
	}

	ZonesGroup* ZGRP = SCENINF.ZGRP + Zone->Index;
	assert( SCENINF.NZGRP >= Zone->Index );
	int mxx = msx >> 2;
	int myy = msy >> 2;
	int NU = 0;
	for (int j = 0; j < ZGRP->N; j++)
	{
		ActiveZone* AZ = AZones + ZGRP->ZoneID[j];
		int R = (AZ->R >> 7) + 2;
		int cx = AZ->x >> 7;
		int cy = AZ->y >> 7;

		int zx = AZ->x << 4;
		int zy = AZ->y << 4;
		int Rz = AZ->R << 4;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->NNUM == Nation && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
								{
									NU++;
								}
							}
						}
					}
				}
			}
		}
	}
	return NU;
}

//1.In zones
void AddUnitToSelected(byte NI, OneObject* OB)
{
	if (OB->Selected & GM( NI ))
	{
		return;
	}

	int N = NSL[NI];
	Selm[NI] = (word*)realloc(Selm[NI], (N + 1) << 1);
	SerN[NI] = (word*)realloc(SerN[NI], (N + 1) << 1);
	NSL[NI]++;
	Selm[NI][N] = OB->Index;
	SerN[NI][N] = OB->Serial;
}

void CopyReIm(byte NI);

extern "C" __declspec( dllexport ) void ClearSelection(byte Nat);

extern short AlarmSoundID;
extern int AlarmDelay;

extern "C" __declspec( dllexport ) void ShowAlarm(GAMEOBJ* Zone)
{
	int x = 0;
	int y = 0;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		x = int(Zone->Index);
		y = int(Zone->Serial);
	}
	else
	{
		if (Zone->Type == 'ZONE')
		{
			ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
			x = AZ->x;
			y = AZ->y;
		}
		else
		{
			IntErr("ShowAlarm : Invalid <Zone>");
			SCENINF.NErrors++;
			return;
		}
	}

	AddEffect((mapx << 5) + 500, (mapy << 4) + 300, AlarmSoundID);
	LastActionX = x << 4;
	LastActionY = y << 4;
	AlarmDelay = 60;
}

extern "C" __declspec( dllexport ) void SelectUnitsInZone(GAMEOBJ* Zone, byte Nation, bool add)
{
	Nation = AssignTBL[Nation];
	if (Nation >= 8)
	{
		IntErr("GetUnitsAmount(Zone,Nation) : Incorrect parameter <Nation>");
		SCENINF.NErrors++;
		return;
	}

	if (!add)
	{
		ClearSelection(Nation);
	}

	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		int R0 = Zone->Type & 0x00FFFFFF;
		int R = (R0 >> 7) + 2;
		int cx = Zone->Index >> 7;
		int cy = Zone->Serial >> 7;

		int zx = int(Zone->Index) << 4;
		int zy = int(Zone->Serial) << 4;
		int Rz = R0 << 4;

		int mxx = msx >> 2;
		int myy = msy >> 2;
		int NU = 0;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->NNUM == Nation && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
								{
									AddUnitToSelected(Nation, OB);
								}
							}
						}
					}
				}
			}
		}
		CopyReIm(Nation);
		return;
	}

	if ((!Zone) || Zone->Type != 'ZONE')
	{
		IntErr("GetUnitsAmount(Zone,Nation) : Incorrect parameter <Zone>");
		SCENINF.NErrors++;
		return;
	}

	ZonesGroup* ZGRP = SCENINF.ZGRP + Zone->Index;
	assert( SCENINF.NZGRP >= Zone->Index );
	int mxx = msx >> 2;
	int myy = msy >> 2;
	int NU = 0;
	for (int j = 0; j < ZGRP->N; j++)
	{
		ActiveZone* AZ = AZones + ZGRP->ZoneID[j];
		int R = (AZ->R >> 7) + 2;
		int cx = AZ->x >> 7;
		int cy = AZ->y >> 7;

		int zx = AZ->x << 4;
		int zy = AZ->y << 4;
		int Rz = AZ->R << 4;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->NNUM == Nation && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
								{
									AddUnitToSelected(Nation, OB);
								}
							}
						}
					}
				}
			}
		}
	}

	CopyReIm(Nation);
	return;
}

extern "C" __declspec( dllexport ) void ChangeUnitParam(GAMEOBJ* Type,
                                                        byte NI, int AddDamage, int DamType, int AddShield)
{
	if (NI >= 8 || DamType > 4)
	{
		return;
	}
	if (Type->Type != 'UTYP')
	{
		IntErr("ChangeUnitParam(Type,Nation,...) : Incorrect Type");
		SCENINF.NErrors++;
		return;
	}
	NATIONS[NI].Mon[Type->Index]->MoreCharacter->Shield += AddShield;
	NATIONS[NI].Mon[Type->Index]->MoreCharacter->MaxDamage[DamType] += AddDamage;
	NATIONS[NI].Mon[Type->Index]->MoreCharacter->Changed = 1;
}

extern "C" __declspec( dllexport ) void SelectTypeOfUnitsInZone(GAMEOBJ* Zone,
                                                                GAMEOBJ* Type, byte Nation, bool add)
{
	Nation = AssignTBL[Nation];
	if (Nation >= 8)
	{
		IntErr("GetTypeOfUnitsAmount(Zone,Type,Nation) : Incorrect parameter <Nation>");
		SCENINF.NErrors++;
		return;
	}
	if (Type->Type != 'UTYP')
	{
		IntErr("GetTypeOfUnitsAmount(Zone,Type,Nation) : Incorrect parameter <Type>");
		SCENINF.NErrors++;
		return;
	}
	if (!add)
	{
		ClearSelection(Nation);
	}
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		int R0 = Zone->Type & 0x00FFFFFF;
		int R = (R0 >> 7) + 2;
		int cx = Zone->Index >> 7;
		int cy = Zone->Serial >> 7;

		int zx = int(Zone->Index) << 4;
		int zy = int(Zone->Serial) << 4;
		int Rz = R0 << 4;

		int mxx = msx >> 2;
		int myy = msy >> 2;
		int NU = 0;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->NNUM == Nation && OB->NIndex == Type->Index && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
								{
									AddUnitToSelected(Nation, OB);
								}
							}
						}
					}
				}
			}
		}
		CopyReIm(Nation);
		return;
	}

	if ((!Zone) || Zone->Type != 'ZONE')
	{
		IntErr("GetTypeOfUnitsAmount(Zone,Type,Nation) : Incorrect parameter <Zone>");
		SCENINF.NErrors++;
		return;
	}

	ZonesGroup* ZGRP = SCENINF.ZGRP + Zone->Index;
	assert( SCENINF.NZGRP >= Zone->Index );
	int mxx = msx >> 2;
	int myy = msy >> 2;
	int NU = 0;
	for (int j = 0; j < ZGRP->N; j++)
	{
		ActiveZone* AZ = AZones + ZGRP->ZoneID[j];
		int R = (AZ->R >> 7) + 2;
		int cx = AZ->x >> 7;
		int cy = AZ->y >> 7;

		int zx = AZ->x << 4;
		int zy = AZ->y << 4;
		int Rz = AZ->R << 4;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->NNUM == Nation && OB->NIndex == Type->Index && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
									AddUnitToSelected(Nation, OB);
							}
						}
					}
				}
			}
		}
	}
	CopyReIm(Nation);
	return;
}

extern "C" __declspec( dllexport ) int GetUnitsAmount1(GAMEOBJ* Zone, GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("GetUnitsAmount(Zone,Units) : Incorrect parameter <Units>");
		return 0;
	}
	TestUnitsGroup(Units->Index);
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		int R0 = Zone->Type & 0x00FFFFFF;
		int cx = int(Zone->Index) << 4;
		int cy = int(Zone->Serial) << 4;
		int R = R0 << 4;
		word* UIDS = SCENINF.UGRP[Units->Index].IDS;
		word* SIDS = SCENINF.UGRP[Units->Index].SNS;
		int Nu = SCENINF.UGRP[Units->Index].N;
		int NU = 0;
		for (int p = 0; p < Nu; p++)
		{
			word MID = UIDS[p];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && (OB->Hidden || !OB->Sdoxlo) && OB->Serial == SIDS[p] && Norma(OB->RealX - cx, OB->RealY - cy) < R)NU++;
			}
		}
		return NU;
	}
	if (Zone->Type != 'ZONE')
	{
		IntErr("GetUnitsAmount(Zone,Units) : Incorrect parameter <Zone>");
		return 0;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	ZonesGroup* ZGRP = SCENINF.ZGRP + Zone->Index;
	assert( SCENINF.NZGRP >= Zone->Index );
	int NU = 0;
	for (int j = 0; j < ZGRP->N; j++)
	{
		ActiveZone* AZ = AZones + ZGRP->ZoneID[j];
		int R = (AZ->R) << 4;
		int cx = AZ->x << 4;
		int cy = AZ->y << 4;
		for (int p = 0; p < Nu; p++)
		{
			word MID = UIDS[p];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && (!OB->Sdoxlo) && OB->Serial == SIDS[p] && Norma(OB->RealX - cx, OB->RealY - cy) < R)NU++;
			}
		}
	}
	return NU;
}

extern "C" __declspec( dllexport ) int GetUnitsAmount2(GAMEOBJ* Zone, GAMEOBJ* UnitType, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (Nation >= 8)
	{
		IntErr("GetUnitsAmount(Zone,UnitType,Nation) : Incorrect parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	if (UnitType->Type != 'UTYP')
	{
		IntErr("GetUnitsAmount(Zone,UnitType,Nation) : Incorrect parameter <UnitType>");
		SCENINF.NErrors++;
		return 0;
	}
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		GeneralObject* GO = NATIONS[Nation].Mon[UnitType->Index];
		int R0 = Zone->Type & 0x00FFFFFF;
		int R = (R0 >> 7) + 2;
		int cx = Zone->Index >> 7;
		int cy = Zone->Serial >> 7;

		int zx = int(Zone->Index) << 4;
		int zy = int(Zone->Serial) << 4;
		int Rz = R0 << 4;

		int mxx = msx >> 2;
		int myy = msy >> 2;
		int NU = 0;
		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->Ref.General == GO && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
									NU++;
							}
						}
					}
				}
			}
		}
		return NU;
	}
	if (Zone->Type != 'ZONE')
	{
		IntErr("GetUnitsAmount(Zone,UnitType,Nation) : Incorrect parameter <Zone>");
		SCENINF.NErrors++;
		return 0;
	}
	GeneralObject* GO = NATIONS[Nation].Mon[UnitType->Index];
	ZonesGroup* ZGRP = SCENINF.ZGRP + Zone->Index;
	assert( SCENINF.NZGRP >= Zone->Index );
	int mxx = msx >> 2;
	int myy = msy >> 2;
	int NU = 0;
	for (int j = 0; j < ZGRP->N; j++)
	{
		ActiveZone* AZ = AZones + ZGRP->ZoneID[j];
		int R = (AZ->R >> 7) + 2;
		int cx = AZ->x >> 7;
		int cy = AZ->y >> 7;

		int zx = AZ->x << 4;
		int zy = AZ->y << 4;
		int Rz = AZ->R << 4;

		for (int r = 0; r < R; r++)
		{
			char* xi = Rarr[r].xi;
			char* yi = Rarr[r].yi;
			int Np = Rarr[r].N;
			for (int p = 0; p < Np; p++)
			{
				int xp = cx + xi[p];
				int yp = cy + yi[p];
				if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
				{
					int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
					int NMon = MCount[cell];
					if (NMon)
					{
						int ofs1 = cell << SHFCELL;
						word MID;
						for (int i = 0; i < NMon; i++)
						{
							MID = GetNMSL(ofs1 + i);
							if (MID != 0xFFFF)
							{
								OneObject* OB = Group[MID];
								if (OB && OB->Ref.General == GO && (!OB->Sdoxlo) &&
									Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
									NU++;
							}
						}
					}
				}
			}
		}
	}
	return NU;
}

//2.total
extern "C" __declspec( dllexport ) int GetTotalAmount0(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("GetTotalAmount(Units) : Invalid parameter <Units>");
		SCENINF.NErrors++;
		return 0;
	}
	TestUnitsGroup(Units->Index);
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NU = 0;
	for (int i = 0; i < Nu; i++)
	{
		word MID = UIDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && OB->Serial == SIDS[i])NU++;
		}
	}
	return NU;
}

extern "C" __declspec( dllexport ) int GetTotalAmount1(GAMEOBJ* UnitType, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (UnitType->Type != 'UTYP')
	{
		IntErr("GetTotalAmount(UnitType,Nation) : Invalid parameter <UnitType>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Nation >= 8)
	{
		IntErr("GetTotalAmount(UnitType,Nation) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nation].CITY->UnitAmount[UnitType->Index];
}

extern "C" __declspec( dllexport ) int GetTotalAmount2(GAMEOBJ* Units, GAMEOBJ* UnitType, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (UnitType->Type != 'UTYP')
	{
		IntErr("GetTotalAmount(UnitType,Nation) : Invalid parameter <UnitType>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Units->Type != 'UNIT')
	{
		IntErr("GetTotalAmount(Units) : Invalid parameter <Units>");
		SCENINF.NErrors++;
		return 0;
	}
	TestUnitsGroup(Units->Index);
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NU = 0;
	int uid = UnitType->Index;
	for (int i = 0; i < Nu; i++)
	{
		word MID = UIDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && OB->Serial == SIDS[i] && OB->NIndex == uid)NU++;
		}
	}
	return NU;
}

extern "C" __declspec( dllexport ) int GetReadyAmount(GAMEOBJ* UnitType, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (UnitType->Type != 'UTYP')
	{
		IntErr("GetReadyAmount(UnitType,Nation) : Invalid parameter <UnitType>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Nation >= 8)
	{
		IntErr("GetReadyAmount(UnitType,Nation) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nation].CITY->ReadyAmount[UnitType->Index];
}

extern "C" __declspec( dllexport ) int GetAmountOfWarriors(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("GetAmountOfWarriors : Invalid <Nat>");
		return 0;
	}
	word* Mons = NatList[Nat];
	int N = NtNUnits[Nat];
	int NU = 0;
	for (int i = 0; i < N; i++)
	{
		OneObject* OB = Group[i];
		if (OB && !OB->Sdoxlo)
		{
			if (!(OB->NewBuilding || OB->newMons->Capture || OB->LockType))
			{
				NU++;
			}
		}
	}
	return NU;
}

//3.Upgrades
extern "C" __declspec( dllexport ) bool IsUpgradeDoing(GAMEOBJ* Upgrade, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (Upgrade->Type != 'UPGR')
	{
		IntErr("IsUpgradeDoung(Upgrade,Nation) : Incorrect parameter <Upgrade>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Nation >= 8)
	{
		IntErr("IsUpgradeDoing(Upgrade,Nation) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nation].UPGRADE[Upgrade->Index]->IsDoing;
}

extern "C" __declspec( dllexport ) bool IsUpgradeDone(GAMEOBJ* Upgrade, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (Upgrade->Type != 'UPGR')
	{
		IntErr("IsUpgradeDone(Upgrade,Nation) : Incorrect parameter <Upgrade>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Nation >= 8)
	{
		IntErr("IsUpgradeDone(Upgrade,Nation) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nation].UPGRADE[Upgrade->Index]->Done;
}

extern "C" __declspec( dllexport ) bool IsUpgradeEnabled(GAMEOBJ* Upgrade, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (Upgrade->Type != 'UPGR')
	{
		IntErr("IsUpgradeEnabled(Upgrade,Nation) : Incorrect parameter <Upgrade>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Nation >= 8)
	{
		IntErr("IsUpgradeEnabled(Upgrade,Nation) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nation].UPGRADE[Upgrade->Index]->Enabled;
}

//4.Died units
extern "C" __declspec( dllexport ) int GetDied(GAMEOBJ* UnitType, byte Nation)
{
	Nation = AssignTBL[Nation];
	if (UnitType->Type != 'UNIT')
	{
		IntErr("GetDied(UnitType,Nation) : Invalid parameter <UnitType>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Nation >= 8)
	{
		IntErr("GetDied(UnitType,Nation) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nation].NKilled[UnitType->Index];
}

//---------------Creating/Erasure commands--------------
int CreateNewTerrMons2(byte NI, int x, int y, word Type);

extern "C" __declspec( dllexport ) bool CreateObject0(GAMEOBJ* DstObj, GAMEOBJ* Form, GAMEOBJ* UnitType, byte NatID,
                                                      GAMEOBJ* Zone, byte Direction)
{
	NatID = AssignTBL[NatID];
	if (NatID >= 8)
	{
		IntErr("CreateObject(DstObj,Formation,UnitType,Nation,Zone,Direction) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return false;
	}
	if (Form->Type != 'FORM')
	{
		IntErr("CreateObject(DstObj,Formation,UnitType,Nation,Zone,Direction) : Invalid parameter <Formation>");
		SCENINF.NErrors++;
		return false;
	}
	if (UnitType->Type != 'UTYP')
	{
		IntErr("CreateObject(DstObj,Formation,UnitType,Nation,Zone,Direction) : Invalid parameter <UnitType>");
		SCENINF.NErrors++;
		return false;
	}
	int xc, yc;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		xc = Zone->Index << 4;
		yc = Zone->Serial << 4;
	}
	else if (Zone->Type != 'ZONE')
	{
		IntErr("CreateObject(DstObj,Formation,UnitType,Nation,Zone,Direction) : Invalid parameter <Zone>");
		SCENINF.NErrors++;
		return false;
	}
	else
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
		xc = AZ->x << 4;
		yc = AZ->y << 4;
	}
	OrderDescription* ODS = ElementaryOrders + Form->Index;
	int N = ODS->NUnits;
	PORD.CreateSimpleOrdPos(xc, yc, Direction, ODS->NUnits, NULL, ODS);
	Nation* Nat = NATIONS + NatID;
	word NewIds[1024];
	int NU = 0;
	for (int j = 0; j < N; j++)
	{
		int ID = CreateNewTerrMons2(NatID, PORD.px[j], PORD.py[j], UnitType->Index);
		if (ID != -1 && NU < 1024)
		{
			NewIds[NU] = ID;
			NU++;
		}
	}
	if (SCENINF.NUGRP >= SCENINF.MaxUGRP)
	{
		SCENINF.MaxUGRP += 10;
		SCENINF.UGRP = (UnitsGroup*)realloc(SCENINF.UGRP, SCENINF.MaxUGRP * sizeof UnitsGroup);
	}
	UnitsGroup* UG = SCENINF.UGRP + SCENINF.NUGRP;
	SCENINF.NUGRP++;
	if (NU)
	{
		UG->IDS = new word[NU];
		UG->SNS = new word[NU];
		UG->N = NU;
		for (int j = 0; j < NU; j++)
		{
			UG->IDS[j] = NewIds[j];
			UG->SNS[j] = Group[NewIds[j]]->Serial;
		}
	}
	else
	{
		UG->IDS = new word;
		UG->SNS = new word;
		UG->N = 0;
	}
	DstObj->Index = SCENINF.NUGRP - 1;
	DstObj->Serial = 0;
	DstObj->Type = 'UNIT';
	return true;
}

//---------------------Action commands----------------------
void ImClearSelection(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("ClearSelection(Nat) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return;
	}
	word* SMon = ImSelm[Nat];
	word* SN = ImSerN[Nat];
	int N = ImNSL[Nat];
	if (N)
	{
		for (int i = 0; i < N; i++)
		{
			word MID = SMon[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == SN[i])OB->ImSelected &= ~GM( Nat );
			}
		}
		free(SMon);
		free(SN);
		ImSelm[Nat] = NULL;
		ImSerN[Nat] = NULL;
		ImNSL[Nat] = NULL;
	}
	ImSelm[Nat] = NULL;
	ImSerN[Nat] = NULL;
	ImNSL[Nat] = NULL;
}

void ReClearSelection(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("ClearSelection(Nat) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return;
	}
	word* SMon = Selm[Nat];
	word* SN = SerN[Nat];
	int N = NSL[Nat];
	if (N)
	{
		for (int i = 0; i < N; i++)
		{
			word MID = SMon[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == SN[i])OB->Selected &= ~GM( Nat );
			}
		}
		free(SMon);
		free(SN);
		Selm[Nat] = NULL;
		SerN[Nat] = NULL;
		NSL[Nat] = NULL;
	}
	Selm[Nat] = NULL;
	SerN[Nat] = NULL;
	NSL[Nat] = NULL;
}

extern "C" __declspec( dllexport ) void ClearSelection(byte Nat)
{
	ReClearSelection(Nat);
	ImClearSelection(Nat);
}

void CopyReIm(byte NI)
{
	int Ni = ImNSL[NI];
	if (Ni)
	{
		word* mid = ImSelm[NI];
		word* msn = ImSerN[NI];
		for (int i = 0; i < Ni; i++)
		{
			word MID = mid[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == msn[i])
				{
					OB->ImSelected &= ~GM( NI );
				}
			}
		}
	}
	int N = NSL[NI];
	if (N)
	{
		ImNSL[NI] = NSL[NI];
		N *= 2;
		ImSelm[NI] = (word*)realloc(ImSelm[NI], N);
		ImSerN[NI] = (word*)realloc(ImSerN[NI], N);
		memcpy(ImSelm[NI], Selm[NI], N);
		memcpy(ImSerN[NI], SerN[NI], N);
	}
	else
	{
		if (ImSelm)
		{
			free(ImSelm[NI]);
			free(ImSerN[NI]);
			ImSelm[NI] = NULL;
			ImSerN[NI] = NULL;
			ImNSL[NI] = 0;
		}
	}
	Ni = ImNSL[NI];
	if (Ni)
	{
		word* mid = ImSelm[NI];
		word* msn = ImSerN[NI];
		for (int i = 0; i < Ni; i++)
		{
			word MID = mid[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == msn[i])
				{
					OB->ImSelected |= 1 << NI;
				}
			}
		}
	}
}

extern "C" __declspec( dllexport ) void SelectUnits(GAMEOBJ* Units, bool Add)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("SelectUnits(Units,Add) : Invalid parameter <Units>");
		SCENINF.NErrors++;
		return;
	}
	TestUnitsGroup(Units->Index);
	int Nat = -1;
	UnitsGroup* UG = SCENINF.UGRP + Units->Index;
	int NU = 0;
	for (int i = 0; i < UG->N; i++)
	{
		word MID = UG->IDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == UG->SNS[i])
			{
				if (Nat == -1)Nat = OB->NNUM;
				if (OB->NNUM == Nat)NU++;
			}
		}
	}
	if (Nat != -1)
	{
		if (!Add)ClearSelection(Nat);
		if (NU)
		{
			int N = NSL[Nat];
			Selm[Nat] = (word*)realloc(Selm[Nat], (N + NU) * 2);
			SerN[Nat] = (word*)realloc(SerN[Nat], (N + NU) * 2);

			word* SMon = Selm[Nat];
			word* SN = SerN[Nat];
			NSL[Nat] += NU;

			NU = 0;
			for (int i = 0; i < UG->N; i++)
			{
				word MID = UG->IDS[i];
				if (MID != 0xFFFF)
				{
					OneObject* OB = Group[MID];
					if (OB && OB->Serial == UG->SNS[i])
					{
						if (Nat == -1)Nat = OB->NNUM;
						if (OB->NNUM == Nat)
						{
							Selm[Nat][N + NU] = OB->Index;
							SerN[Nat][N + NU] = OB->Serial;
							OB->Selected |= GM( Nat );
							NU++;
						}
					}
				}
			}
		}
		CopyReIm(Nat);
	}
}

extern "C" __declspec( dllexport ) void SelectUnits1(byte Nat, GAMEOBJ* Units, bool Add)
{
	Nat = AssignTBL[Nat];
	if (Nat > 7)return;
	if (Units->Type != 'UNIT')
	{
		IntErr("SelectUnits1(Nat,Units,Add) : Invalid parameter <Units>");
		SCENINF.NErrors++;
		return;
	}
	TestUnitsGroup(Units->Index);
	UnitsGroup* UG = SCENINF.UGRP + Units->Index;
	int NU = 0;
	if (!Add)ClearSelection(Nat);
	for (int i = 0; i < UG->N; i++)
	{
		word MID = UG->IDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == UG->SNS[i] && OB->NNUM == Nat && !(OB->Sdoxlo || OB->Selected & GM( Nat )))NU++;
		}
	}
	if (NU)
	{
		int N = NSL[Nat];
		Selm[Nat] = (word*)realloc(Selm[Nat], (N + NU) * 2);
		SerN[Nat] = (word*)realloc(SerN[Nat], (N + NU) * 2);
		word* SMon = Selm[Nat];
		word* SN = SerN[Nat];
		NSL[Nat] += NU;

		NU = 0;
		for (int i = 0; i < UG->N; i++)
		{
			word MID = UG->IDS[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == UG->SNS[i] && OB->NNUM == Nat && !(OB->Sdoxlo || OB->Selected & GM( Nat )))
				{
					Selm[Nat][N + NU] = OB->Index;
					SerN[Nat][N + NU] = OB->Serial;
					OB->Selected |= GM( Nat );
					NU++;
				}
			}
		}
	}
	CopyReIm(Nat);
}

extern "C" __declspec( dllexport ) void SelectUnitsType(GAMEOBJ* UnitsType, byte Nat, bool Add)
{
	Nat = AssignTBL[Nat];
	if (UnitsType->Type != 'UTYP')
	{
		IntErr("SelectUnitsType(UnitsType,Nation,Add) : Invalid parameter <UnitsType>");
		SCENINF.NErrors++;
		return;
	}
	if (Nat >= 8)
	{
		IntErr("SelectUnitsType(UnitsType,Nation,Add) : Invalid parameter <Nation>");
		SCENINF.NErrors++;
		return;
	}
	int NU = 0;
	GeneralObject* GO = NATIONS[Nat].Mon[UnitsType->Index];
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB && (!OB->Sdoxlo) && OB->Ref.General == GO && OB->NNUM == Nat)NU++;
	}
	if (!Add)ClearSelection(Nat);
	if (NU)
	{
		int N = NSL[Nat];
		Selm[Nat] = (word*)realloc(Selm[Nat], (N + NU) * 2);
		SerN[Nat] = (word*)realloc(SerN[Nat], (N + NU) * 2);

		word* SMon = Selm[Nat];
		word* SN = SerN[Nat];
		NSL[Nat] += NU;

		NU = 0;
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && (!OB->Sdoxlo) && OB->Ref.General == GO && OB->NNUM == Nat)
			{
				Selm[Nat][N + NU] = OB->Index;
				SerN[Nat][N + NU] = OB->Serial;
				OB->Selected |= GM( Nat );
				NU++;
			}
		}
	}
	CopyReIm(Nat);
}

void DieSelected(byte NI);
void EraseSelected(byte NI);

extern "C" __declspec( dllexport ) bool SelDie(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("SelDie(Nat) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	DieSelected(Nat);
	return true;
}

extern bool BreefUInfo;

extern "C" __declspec( dllexport ) void DoMessagesBrief()
{
	BreefUInfo = 1;
}

extern "C" __declspec( dllexport ) void SelErase(byte NI)
{
	if (NI >= 8)
	{
		IntErr("SelDie(Nat) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return;
	}
	EraseSelected(NI);
}

extern "C" __declspec( dllexport ) void SelAttackGroup(byte Nat, GAMEOBJ* Enemy)
{
	Nat = AssignTBL[Nat];
	if (Enemy->Type != 'UNIT')
	{
		IntErr("SelAttackGroup : Invalid <Enemy>");
		SCENINF.NErrors++;
		return;
	}
	if (Nat >= 8)
	{
		IntErr("SelAttackGroup : Invalid <Nat>");
		SCENINF.NErrors++;
		return;
	}
	TestUnitsGroup(Enemy->Index);
	UnitsGroup* AG = SCENINF.UGRP + Enemy->Index;
	int N = AG->N;
	word* Uids = AG->IDS;
	word* Sns = AG->SNS;
	word* MyUnits = Selm[Nat];
	word* MySN = SerN[Nat];
	int NU = NSL[Nat];
	if (!NSL)return;
	for (int i = 0; i < N; i++)
	{
		word MID = Uids[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (!OB->Sdoxlo) && OB->Serial == Sns[i])
			{
				for (int j = 0; j < NU; j++)
				{
					word MyMid = MyUnits[j];
					if (MyMid != 0xFFFF)
					{
						OneObject* MyObj = Group[MyMid];
						if (MyObj && MyObj->Serial == MySN[j] && !(MyObj->Sdoxlo || MyObj->EnemyID != 0xFFFF)/*MyObj->LocalOrder*/)
						{
							MyObj->AttackObj(MID, 128 + 16, 0);
						}
					}
				}
			}
		}
	}
}

void ComOpenGates(byte NI);
void ComCloseGates(byte NI);

extern "C" __declspec( dllexport ) bool SelOpenGates(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("SelOpenGate(Nat) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	ComOpenGates(Nat);
	return true;
}

extern "C" __declspec( dllexport ) int GetNInside(byte Nat)
{
	Nat = AssignTBL[Nat];
	int Ni = 0;
	int N = NSL[Nat];
	word* SM = Selm[Nat];
	word* SR = SerN[Nat];
	for (int i = 0; i < N; i++)
	{
		word MID = SM[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == SR[i] && !OB->Sdoxlo)
			{
				Ni += OB->NInside;
			}
		}
	}
	return Ni;
}

extern "C" __declspec( dllexport ) int GetMaxInside(byte Nat)
{
	Nat = AssignTBL[Nat];
	int Ni = 0;
	int N = NSL[Nat];
	word* SM = Selm[Nat];
	word* SR = SerN[Nat];
	for (int i = 0; i < N; i++)
	{
		word MID = SM[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == SR[i] && !OB->Sdoxlo)
			{
				Ni += OB->Ref.General->MoreCharacter->MaxInside + OB->AddInside;
			}
		}
	}
	return Ni;
}

extern "C" __declspec( dllexport ) void PushUnitAway(byte Nat)
{
	Nat = AssignTBL[Nat];
	int N = NSL[Nat];
	word* SM = Selm[Nat];
	word* SR = SerN[Nat];
	for (int i = 0; i < N; i++)
	{
		word MID = SM[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == SR[i] && OB->NInside && !(OB->Sdoxlo || OB->LocalOrder))
			{
				if (OB->Transport)OB->LeaveTransport(Group[OB->Inside[0]]->NIndex);
				else OB->LeaveMine(Group[OB->Inside[0]]->NIndex);
			}
		}
	}
}

void LeaveAll(OneObject* OB);

extern "C" __declspec( dllexport ) void PushAllUnitsAway(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (NSL[Nat])
	{
		int N = NSL[Nat];
		for (int i = 0; i < N; i++)
		{
			word MID = Selm[Nat][i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == SerN[Nat][i])
				{
					if (OB->Transport)LeaveAll(OB);
				}
			}
		}
	}
}

extern "C" __declspec( dllexport ) void SendUnitsToTransport(byte Nat)
{
	Nat = AssignTBL[Nat];
	word TransID = 0xFFFF;
	if (NSL[Nat])
	{
		int N = NSL[Nat];
		for (int i = 0; i < N; i++)
		{
			word MID = Selm[Nat][i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && OB->Serial == SerN[Nat][i] && !OB->Sdoxlo)
				{
					if (OB->Transport)TransID = MID;
				}
			}
		}
		if (TransID != 0xFFFF)
		{
			for (int i = 0; i < N; i++)
			{
				word MID = Selm[Nat][i];
				if (MID != 0xFFFF)
				{
					OneObject* OB = Group[MID];
					if (OB && OB->Serial == SerN[Nat][i] && !OB->Sdoxlo)
					{
						if (!OB->Transport)OB->GoToTransport(TransID, 128 + 16);
					}
				}
			}
		}
	}
}

extern "C" __declspec( dllexport ) bool CheckLeaveAbility(byte Nat)
{
	Nat = AssignTBL[Nat];
	int N = NSL[Nat];
	word* SM = Selm[Nat];
	word* SR = SerN[Nat];
	for (int i = 0; i < N; i++)
	{
		word MID = SM[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == SR[i] && OB->Transport && OB->DstX > 0)return true;
		}
	}
	return false;
}

word SearchUnitInZone(int zx, int zy, int Rz, byte Nat)
{
	Nat = AssignTBL[Nat];
	int R = (Rz >> 11) + 2;
	int cx = zx >> 11;
	int cy = zy >> 11;

	int mxx = msx >> 2;
	int myy = msy >> 2;
	int NU = 0;

	for (int r = 0; r < R; r++)
	{
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int Np = Rarr[r].N;
		for (int p = 0; p < Np; p++)
		{
			int xp = cx + xi[p];
			int yp = cy + yi[p];
			if (xp >= 0 && yp >= 0 && xp < mxx && yp < myy)
			{
				int cell = xp + 1 + ((yp + 1) << VAL_SHFCX);
				int NMon = MCount[cell];
				if (NMon)
				{
					int ofs1 = cell << SHFCELL;
					word MID;
					for (int i = 0; i < NMon; i++)
					{
						MID = GetNMSL(ofs1 + i);
						if (MID != 0xFFFF)
						{
							OneObject* OB = Group[MID];
							if (OB && OB->NNUM == Nat && (!OB->Sdoxlo) &&
								Norma(OB->RealX - zx, OB->RealY - zy) < Rz)
								return MID;
						}
					}
				}
			}
		}
	}

	return 0xFFFF;
}

extern "C" __declspec( dllexport ) void AttackZoneByArtillery(GAMEOBJ* ArtGroup, GAMEOBJ* Zone, byte Nat)
{
	Nat = AssignTBL[Nat];
	int xc, yc, R;

	if (ArtGroup->Type != 'UNIT')
	{
		IntErr("AttackZoneByArtillery : Invalid <ArtGroup>");

		return;
	}

	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		xc = int(Zone->Index) << 4;
		yc = int(Zone->Serial) << 4;
		R = int(Zone->Type & 0xFFFFFF) << 4;
	}
	else
	{
		if (Zone->Type == 'ZONE')
		{
			ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
			xc = AZ->x << 4;
			yc = AZ->y << 4;
			R = AZ->R << 4;
		}
		else
		{
			IntErr("AttackZoneByArtillery : Invalid <Zone>");
			SCENINF.NErrors++;

			return;
		}
	}

	word MID = SearchUnitInZone(xc, yc, R, Nat);

	if (MID != 0xFFFF)
	{
		OneObject* Enm = Group[MID];
		int EnX = Enm->RealX >> 4;
		int EnY = Enm->RealY >> 4;
		int EnZ = GetHeight(EnX, EnY) + 32;
		word* AMID = SCENINF.UGRP[ArtGroup->Index].IDS;
		word* AUSN = SCENINF.UGRP[ArtGroup->Index].SNS;
		int AUN = SCENINF.UGRP[ArtGroup->Index].N;

		for (int j = 0; j < AUN; j++)
		{
			word MMID = AMID[j];
			if (MMID != 0xFFFF)
			{
				OneObject* OB = Group[MMID];
				if (OB && (!OB->Sdoxlo) && OB->Serial == AUSN[j] && OB->newMons->Artilery)
				{
					if (!OB->delay)
					{
						OB->AttackPoint(EnX, EnY, EnZ, 1, 4, 0);
					}
				}
			}
		}
	}
}

word FindShipInZone(int x0, int y0, byte Nat)
{
	Nat = AssignTBL[Nat];
	word* Uni = NatList[Nat];
	int N = NtNUnits[Nat];
	word UNIT = 0xFFFF;
	int RMIN = 1000000;

	for (int i = 0; i < N; i++)
	{
		OneObject* OB = Group[Uni[i]];
		if (OB && OB->LockType && !OB->Sdoxlo)
		{
			byte Usage = OB->newMons->Usage;
			if (Usage != FisherID)
			{
				int R0 = Norma(OB->RealX - x0, OB->RealY - y0);
				if (R0 < RMIN)
				{
					RMIN = R0;
					UNIT = OB->Index;
				}
			}
		}
	}

	return UNIT;
}

OneObject* SearchUnitInCell(int cell, byte nmask)
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if (NMon < 3)return NULL;
	int ofs1 = cell << SHFCELL;
	word MID;
	for (int i = 0; i < NMon; i++)
	{
		MID = GetNMSL(ofs1 + i);
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->NMask & nmask && !OB->LockType)return OB;
		}
	}
	return NULL;
}

OneObject* SearchShipInCell(int cell, byte nmask)
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if (NMon < 3)return NULL;
	int ofs1 = cell << SHFCELL;
	word MID;
	for (int i = 0; i < NMon; i++)
	{
		MID = GetNMSL(ofs1 + i);
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->NMask & nmask && OB->LockType && OB->newMons->Usage != FisherID)return OB;
		}
	}
	return NULL;
}

OneObject* SearchBuildingInCell(int cell, byte nmask)
{
	cell += VAL_MAXCX + 1;
	word MID = BLDList[cell];
	if (MID != 0xFFFF)
	{
		OneObject* OB = Group[MID];
		if (OB->NMask & nmask && !OB->Sdoxlo)return OB;
	}
	return NULL;
}

bool CheckVisibility(int x1, int y1, int x2, int y2, word MyID);
void SearchEnemyForAIArtillery(OneObject* OB);
void AIArtilleryAgainstTowers(OneObject* OB);
void SearchEnemyForMortira(OneObject* OB);
void EraseBrigade(Brigade* BR);

OneObject* TryToFindEnemy(int x, int y, int r0, int r1, byte mask)
{
	int cell = ((y >> 11) << VAL_SHFCX) + (x >> 11);
	int mindx = -(x >> 11);
	int maxdx = (msx >> 2) + mindx;
	int mindy = -(y >> 11);
	int maxdy = (msx >> 2) + mindy;
	int rx1 = (r1 >> 7) + 1;
	byte nmask = ~mask;
	OneObject* DestObj = NULL;
	OneObject* DestShip = NULL;
	OneObject* DestBld = NULL;
	int mindist = 10000000;
	int minSdist = 10000000;
	int minBdist = 10000000;
	int dist;
	//long range search
	int rx2 = rx1 + rx1 + 1;
	int stcell;
	r0 <<= 4;
	r1 <<= 4;
	for (int i = 0; i < 50; i++)
	{
		int dx = ((rando() * rx2) >> 15) - rx1;
		int dy = ((rando() * rx2) >> 15) - rx1;
		int rr = Norma(dx, dy);
		if (dx <= maxdx && dx >= mindx && dy >= mindx && dy <= maxdx)
		{
			stcell = cell + dx + (dy << VAL_SHFCX);
			if (stcell >= 0 && stcell < VAL_MAXCIOFS)
			{
				if (NPresence[stcell] & nmask)
				{
					OneObject* OB = SearchShipInCell(stcell, nmask);
					if (OB)
					{
						dist = Norma(OB->RealX - x, OB->RealY - y);
						if (dist > r0 && dist < r1 && dist < mindist)
						{
							minSdist = dist;
							DestShip = OB;
						}
					}
					else
					{
						OB = SearchBuildingInCell(stcell, nmask);
						if (OB)
						{
							dist = Norma(OB->RealX - x, OB->RealY - y);
							if (dist > r0 && dist < r1 && dist < mindist)
							{
								minBdist = dist;
								DestBld = OB;
							}
						}
						else if (rr < 20)
						{
							OB = SearchUnitInCell(stcell, nmask);
							if (OB)
							{
								dist = Norma(OB->RealX - x, OB->RealY - y);
								if (dist > r0 && dist < r1 && dist < mindist)
								{
									mindist = dist;
									DestObj = OB;
								}
							}
						}
					}
				}
			}
		}
	}
	if (DestShip)return DestShip;
	if (DestBld)return DestBld;
	if (DestObj)return DestObj;
	return NULL;
}

extern "C" __declspec( dllexport ) void AttackBuildingsInZone(GAMEOBJ* ArtGroup, GAMEOBJ* Zone, byte Nat)
{
	Nat = AssignTBL[Nat];
	int xc, yc, R;
	if (ArtGroup->Type != 'UNIT')
	{
		IntErr("AttackZoneByArtillery : Invalid <ArtGroup>");
		return;
	}
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		xc = int(Zone->Index) << 4;
		yc = int(Zone->Serial) << 4;
		R = int(Zone->Type & 0xFFFFFF) << 4;
	}
	else if (Zone->Type == 'ZONE')
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
		xc = AZ->x << 4;
		yc = AZ->y << 4;
		R = AZ->R << 4;
	}
	else
	{
		IntErr("AttackZoneByArtillery : Invalid <Zone>");
		SCENINF.NErrors++;
		return;
	}
	int XC = 0;
	int YC = 0;
	int NU0 = 0;
	word* AMID = SCENINF.UGRP[ArtGroup->Index].IDS;
	word* AUSN = SCENINF.UGRP[ArtGroup->Index].SNS;
	int AUN = SCENINF.UGRP[ArtGroup->Index].N;
	for (int j = 0; j < AUN; j++)
	{
		word MID = AMID[j];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (!OB->Sdoxlo) && OB->Serial == AUSN[j])
			{
				if (OB->EnemyID == 0xFFFF)
				{
					AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
					OneObject* EOB = TryToFindEnemy(OB->RealX, OB->RealY, ADC->MinR_Attack, ADC->MaxR_Attack, OB->NMask);
					if (EOB)OB->AttackObj(EOB->Index, 16 + 128, 1, 10);
				}
			}
		}
	}
}

void ProduceObject(byte NI, word Type);

extern "C" __declspec( dllexport ) void ProduceOneUnit(byte Nat, GAMEOBJ* UnitType)
{
	Nat = AssignTBL[Nat];
	if (UnitType->Type != 'UTYP')
	{
		IntErr("ProduceOneUnit : Invalid <UnitType>");
		return;
	}
	ProduceObject(Nat, UnitType->Index);
}

extern "C" __declspec( dllexport ) bool SelCloseGates(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("SelCloseGate(Nat) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	ComCloseGates(Nat);
	return true;
}

void SendSelectedToXY(byte NI, int xx, int yy, short Dir, byte Prio, byte Type);

extern "C" __declspec( dllexport ) bool SelSendTo(byte Nat, GAMEOBJ* Zone, byte Dir, byte Type)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("SelSendTo(Nat,Zone,Dir,Type) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		SendSelectedToXY(Nat, int(Zone->Index) << 4, int(Zone->Serial) << 4, Dir, 16, Type);
		return true;
	}
	if (Zone->Type != 'ZONE')
	{
		IntErr("SelSendTo(Nat,Zone,Dir,Type) : Invalid parameter <Zone>");
		SCENINF.NErrors++;
		return false;
	}
	ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
	SendSelectedToXY(Nat, AZ->x << 4, AZ->y << 4, Dir, 16, Type);
	return true;
}

extern "C" __declspec( dllexport ) bool SelSendAndKill(byte Nat, GAMEOBJ* Zone, byte Dir, byte Type)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("SelSendAndKill(Nat,Zone,Dir,Type) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		SendSelectedToXY(Nat, int(Zone->Index) << 4, int(Zone->Serial) << 4, Dir, 128, Type);
		return true;
	}
	if (Zone->Type != 'ZONE')
	{
		IntErr("SelSendAndKill(Nat,Zone,Dir,Type) : Invalid parameter <Zone>");
		SCENINF.NErrors++;
		return false;
	}
	ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
	SendSelectedToXY(Nat, AZ->x << 4, AZ->y << 4, Dir, 128, Type);
	return true;
}

void PatrolGroup(byte NI, int x1, int y1, byte Dir);

extern "C" __declspec( dllexport ) bool Patrol(byte Nat, GAMEOBJ* Zone, byte Dir)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("Patrol(Nat,Zone,Dir) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		PatrolGroup(Nat, int(Zone->Index) << 4, int(Zone->Serial) << 4, Dir);
		return true;
	}
	if (Zone->Type != 'ZONE')
	{
		IntErr("Patrol(Nat,Zone,Dir) : Invalid parameter <Zone>");
		SCENINF.NErrors++;
		return false;
	}
	ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
	PatrolGroup(Nat, AZ->x << 4, AZ->y << 4, Dir);
	return true;
}

extern "C" __declspec( dllexport ) void ChangeFriends(byte Nat, byte Flags)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("ChangeFriends(Nat,Flags) : Invalid parameter <Nat>");
		SCENINF.NErrors++;
		return;
	}
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB && OB->NNUM == Nat)
		{
			OB->NMask = Flags;
		}
	}
	NATIONS[Nat].NMask = Flags;
}

extern "C" __declspec( dllexport ) void SelChangeNation(byte SrcNat, byte DstNat)
{
	if (SrcNat >= 8 || DstNat >= 8)
	{
		IntErr("SelChangeNation(SrcNat,DstNat) : Invalid parameters");
		SCENINF.NErrors++;
		return;
	}
	if (SrcNat == DstNat)
	{
		IntErr("SelChangeNation(SrcNat,DstNat) : SrcNat and DstNat must be different");
		SCENINF.NErrors++;
		return;
	}
	int N = NSL[SrcNat];
	word* SMon = Selm[SrcNat];
	word* SN = SerN[SrcNat];
	Nation* DNat = NATIONS + DstNat;
	for (int i = 0; i < N; i++)
	{
		word MID = SMon[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB)
			{
				DelObject(OB);
				OB->Nat->CITY->UnRegisterNewUnit(OB);
				OB->Ref.General = NATIONS[DstNat].Mon[OB->NIndex];
				OB->Nat = &NATIONS[DstNat];
				OB->NMask = OB->Nat->NMask;
				OB->NNUM = DstNat;
				OB->Nat->CITY->RegisterNewUnit(OB);
				if (OB->Wall)
				{
					int li = GetLI(OB->WallX, OB->WallY);
					WallCell* WC = WRefs[li];
					if (WC)WC->NI = OB->NNUM;
				}
				//OB->Selected=0;
				//OB->ImSelected=0;
				AddObject(OB);
			}
		}
	}
	ClearSelection(SrcNat);
}

extern "C" __declspec( dllexport ) void SelAutoKill(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("SelAutoKill(Nat) : Invalid parameters");
		SCENINF.NErrors++;
		return;
	}
	int N = NSL[Nat];
	word* SMon = Selm[Nat];
	word* SN = SerN[Nat];
	for (int i = 0; i < N; i++)
	{
		word MID = SMon[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB)
			{
				OB->AutoKill = true;
			}
		}
	}
}

void MakeStandGround(byte NI);
void CancelStandGround(byte NI);
void SetSearchVictim(byte NI, byte Val);

extern "C" __declspec( dllexport ) void SetStandGround(byte Nat, byte val)
{
	Nat = AssignTBL[Nat];
	if (Nat > 7)
	{
		IntErr("SetStandGround(Nat,val) : Invalid <Nat>");
		SCENINF.NErrors++;
		return;
	}
	if (val)MakeStandGround(Nat);
	else CancelStandGround(Nat);
}

extern "C" __declspec( dllexport ) void AllowAttack(byte Nat, byte val)
{
	Nat = AssignTBL[Nat];
	if (Nat > 7)
	{
		IntErr("AllowAttack(Nat,val) : Invalid <Nat>");
		SCENINF.NErrors++;
		return;
	}
	if (val)SetSearchVictim(Nat, 2);
	else SetSearchVictim(Nat, 3);
}

//-----------------------INFORMATION COMMANDS----------------------------//
extern "C" __declspec( dllexport ) void HINT(GAMEOBJ* Hint, int time)
{
	if (Hint->Type != 'STRI')
	{
		IntErr("HINT(Hint,Time) : invalid parameter <Hint>");
		SCENINF.NErrors++;
		return;
	}

	char* cc = SCENINF.Messages[Hint->Index];
	if (cc)
	{
		CreateTimedHint(cc, time);
	}
}

extern "C" __declspec( dllexport ) void DisableMission(char MISSID)
{
	SCENINF.TextDisable[GetCPos(MISSID)] = 1;
	CreateMissText();
}

extern "C" __declspec( dllexport ) void EnableMission(char MISSID)
{
	SCENINF.TextDisable[GetCPos(MISSID)] = 0;
	CreateMissText();
}

extern "C" __declspec( dllexport ) void SetVictoryText(char* ID)
{
	char* txt = GetTextByID(ID);
	if (strcmp(txt, ID))
	{
		SCENINF.VictoryText = ID;
	}
	else
	{
		IntErr("SetVictoryText(ID) : Unknown ID : %s  (see COMMENT.TXT)", ID);
		SCENINF.NErrors++;
	}
}

extern "C" __declspec( dllexport ) void SetLooseText(char* ID)
{
	char* txt = GetTextByID(ID);
	if (strcmp(txt, ID))
	{
		SCENINF.LooseText = ID;
	}
	else
	{
		IntErr("SetLoosText(ID) : Unknown ID : %s  (see COMMENT.TXT)", ID);
		SCENINF.NErrors++;
	}
}

extern bool ShowStat;
void CmdEndGame(byte NI, byte state, byte cause);
extern int HISPEED;

extern "C" __declspec( dllexport ) void ShowVictory()
{
	SCENINF.Victory = true;
	HISPEED = 0;
	NOPAUSE = 0;
	LockPause = 1;
	ShowStat = 0;
	if (SCENINF.StandartVictory)
	{
		CmdEndGame(MyNation, 2, 100);
	}
}

extern "C" __declspec( dllexport ) void LooseGame()
{
	SCENINF.LooseGame = true;
	HISPEED = 0;
	NOPAUSE = 0;
	LockPause = 1;
	ShowStat = 0;
	if (SCENINF.hLib)
	{
		SCENINF.StandartVictory = 0;
	}
}

extern "C" __declspec( dllexport ) void ShowCentralText(char* ID, int time)
{
	SCENINF.CentralText = GetTextByID(ID);
	SCENINF.CTextTime = time;
}

void ProcessMissionText(char* Bmp, char* Text);

void AddHistory(Nation* NAT, char* Name)
{
	NAT->History = (char**)realloc(NAT->History, 4 * NAT->NHistory + 4);
	NAT->History[NAT->NHistory] = new char[strlen(Name) + 1];
	strcpy(NAT->History[NAT->NHistory], Name);
	NAT->NHistory++;
}

extern "C" __declspec( dllexport ) void ShowPage(char* Name)
{
	int NP = 0;
	AddHistory(NATIONS + MyNation, Name);
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(Name, SCENINF.PageID[i]))
		{
			ProcessMissionText(SCENINF.PageBMP[i], SCENINF.Page[i]);
			NP++;
		}
	}

	if (!NP)
	{
		IntErr("ShowPage : Unknown page: %s", Name);
		SCENINF.NErrors++;
	}
}

extern "C" __declspec( dllexport ) void ShowPageParam(char* Name, ...)
{
	va_list va;
	va_start( va, Name );
	char cc2[4096];
	char cc3[4096];
	int NP = 0;
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(Name, SCENINF.PageID[i]))
		{
			vsprintf(cc2, SCENINF.Page[i], va);
			sprintf(cc3, "%s @ %s", SCENINF.PageBMP[i], cc2);
			AddHistory(NATIONS + MyNation, cc3);
			ProcessMissionText(SCENINF.PageBMP[i], cc2);
			NP++;
		}
	}
	va_end( va );
	if (!NP)
	{
		IntErr("ShowPage : Unknown page: %s", Name);
		SCENINF.NErrors++;
	}
}

extern byte PlayGameMode;
bool AskMissionQuestion(char* Bmp, char* Text);

extern "C" __declspec( dllexport ) bool AskQuestion(char* Name)
{
	if (PlayGameMode)RGAME.Extract();
	int NP = 0;
	AddHistory(NATIONS + MyNation, Name);
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(Name, SCENINF.PageID[i]))
		{
			int t = AskMissionQuestion(SCENINF.PageBMP[i], SCENINF.Page[i]);
			if (RecordMode)
			{
				RGAME.AddByte(3);
				RGAME.AddByte(t);
			}
			return 0 != t;
		}
	}
	IntErr("ShowPage : Unknown page: %s", Name);
	SCENINF.NErrors++;
	return 0;
}

int ProcessComplexQuestion(int Nx, char* Bmp1, byte or1, char* Text1, char* Bmp2, byte or2, char* Text2, char* Quest);
int _pr_q1 = -1;
int _pr_q2 = -1;
int _pr_Nx = -1;
byte _pr_or1;
byte _pr_or2;

extern "C" __declspec( dllexport ) int AskComplexQuestion(int Nx, char* Name1, byte or1, char* Name2, byte or2,
                                                          char* Quest)
{
	if (PlayGameMode)RGAME.Extract();
	int q1 = -1;
	int q2 = -1;
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(Name1, SCENINF.PageID[i]))
		{
			q1 = i;
			goto qq1;
		}
	}
	IntErr("AskComplexQuestion : Unknown page : %s", Name1);
qq1:
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(Name2, SCENINF.PageID[i]))
		{
			q2 = i;
			goto qq2;
		}
	}
	IntErr("AskComplexQuestion : Unknown page : %s", Name2);
qq2:
	_pr_q1 = q1;
	_pr_q2 = q2;
	_pr_Nx = Nx;
	_pr_or1 = or1;
	_pr_or2 = or2;
	int t = ProcessComplexQuestion(Nx, SCENINF.PageBMP[q1], or1, SCENINF.Page[q1], SCENINF.PageBMP[q2], or2,
	                               SCENINF.Page[q2], Quest);

	if (RecordMode)
	{
		RGAME.AddByte(3);
		RGAME.AddByte(t);
	}

	return t;
}

void RetryCQuest()
{
	if (_pr_Nx != -1)
	{
		ProcessComplexQuestion(_pr_Nx, SCENINF.PageBMP[_pr_q1], _pr_or1, SCENINF.Page[_pr_q1], SCENINF.PageBMP[_pr_q2],
		                       _pr_or2, SCENINF.Page[_pr_q2], GetTextByID("TUTCLOS"));
	}
}

int ShowHistryItem(char* Bmp, char* Text);

void ShowHistory()
{
	int res = 0;
	Nation* Nat = NATIONS + MyNation;
	int Cur = Nat->NHistory - 1;

	if (Cur < 0)
	{
		return;
	}

	do
	{
		char* Name = Nat->History[Cur];
		if (Name[0] == '#')
		{
			int cp = -1;

			for (int i = 0; i < SCENINF.NPages; i++)
			{
				if (!strcmp(Name, SCENINF.PageID[i]))
				{
					cp = i;
				}
			}

			if (cp == -1)
			{
				return;
			}

			res = ShowHistryItem(SCENINF.PageBMP[cp], SCENINF.Page[cp]);
		}
		else
		{
			char CC3[4096];
			strcpy(CC3, Name);
			char* cc = strstr(CC3, "@");
			if (cc)
			{
				cc[-1] = 0;
				res = ShowHistryItem(CC3, cc + 1);
			}
			else
			{
				return;
			}
		}

		if (res == 1 && Cur > 0)
		{
			Cur--;
		}

		if (res == 2 && Cur < Nat->NHistory - 1)
		{
			Cur++;
		}
	}
	while (res != 3);
}

//-----------------------------Resource functions------------------------//
extern "C" __declspec( dllexport ) int GetResource(byte Nat, byte ID)
{
	Nat = AssignTBL[Nat];
	if (Nat < 8 && ID < 6)
	{
		return XRESRC( Nat, ID );
	}
	else
	{
		IntErr("GetResource : Invalid parameters");
		SCENINF.NErrors++;
	}
	return 0;
}

extern "C" __declspec( dllexport ) void AddResource(byte Nat, byte ID, int Amount)
{
	Nat = AssignTBL[Nat];
	if (Nat < 8 && ID < 6)
	{
		AddXRESRC( Nat, ID, Amount );
		if (XRESRC( Nat, ID ) < 0)SetXRESRC( Nat, ID, 0 );
	}
	else
	{
		IntErr("AddResource : Invalid parameters");
		SCENINF.NErrors++;
	}
}

extern "C" __declspec( dllexport ) void SetResource(byte Nat, byte ID, int Amount)
{
	Nat = AssignTBL[Nat];
	if (Nat < 8 && ID < 6)
	{
		SetXRESRC( Nat, ID, Amount );
		if (XRESRC( Nat, ID ) < 0)SetXRESRC( Nat, ID, 0 );
	}
	else
	{
		IntErr("SetResource : Invalid parameters");
		SCENINF.NErrors++;
	}
}

void GetUnitCost(byte NI, word NIndex, int* Cost);

extern "C" __declspec( dllexport ) int GetUnitCost(byte Nat, GAMEOBJ* UnitType, byte ResID)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("GetUnitCost(Nat,UnitType,ResID) : Invalid <Nat>");
		SCENINF.NErrors++;
		return 0;
	}
	if (UnitType->Type != 'UTYP')
	{
		IntErr("GetUnitCost(Nat,UnitType,ResID) : Invalid <UnitType>");
		SCENINF.NErrors++;
		return 0;
	}
	if (ResID >= 6)
	{
		IntErr("GetUnitCost(Nat,UnitType,ResID) : Invalid <ResID>");
		SCENINF.NErrors++;
		return 0;
	}
	int Cost[8];
	GetUnitCost(Nat, UnitType->Index, Cost);
	return Cost[ResID];
}

extern "C" __declspec( dllexport ) int GetUpgradeCost(byte Nat, GAMEOBJ* Upgrade, byte ResID)
{
	Nat = AssignTBL[Nat];
	if (Nat >= 8)
	{
		IntErr("GetUpgradeCost(Nat,Upgrade,ResID) : Invalid <Nat>");
		SCENINF.NErrors++;
		return 0;
	}
	if (Upgrade->Type != 'UPGR')
	{
		IntErr("GetUpgradeCost(Nat,Upgrade,ResID) : Invalid <Upgrade>");
		SCENINF.NErrors++;
		return 0;
	}
	if (ResID >= 6)
	{
		IntErr("GetUpgradeCost(Nat,Upgrade,ResID) : Invalid <ResID>");
		SCENINF.NErrors++;
		return 0;
	}
	return NATIONS[Nat].UPGRADE[Upgrade->Index]->Cost[ResID];
}

//----------------------------TRIGGER FUNCTION---------------------------//
extern "C" __declspec( dllexport ) byte Trigg(byte ID)
{
	if (ID > 511)
	{
		IntErr("Trigg(ID) : Invalid ID");
		SCENINF.NErrors++;
		return 0;
	}
	return ~SCENINF.TRIGGER[ID];
}

extern "C" __declspec( dllexport ) void SetTrigg(byte ID, byte Val)
{
	if (ID > 511)
	{
		IntErr("SetTrigg(ID) : Invalid ID");
		SCENINF.NErrors++;
		return;
	}
	SCENINF.TRIGGER[ID] = ~Val;
}

extern "C" __declspec( dllexport ) word WTrigg(byte ID)
{
	if (ID > 511)
	{
		IntErr("Trigg(ID) : Invalid ID");
		SCENINF.NErrors++;
		return 0;
	}
	return ~SCENINF.TRIGGER[ID];
}

extern "C" __declspec( dllexport ) void SetWTrigg(byte ID, word Val)
{
	if (ID > 511)
	{
		IntErr("SetTrigg(ID) : Invalid ID");
		SCENINF.NErrors++;
		return;
	}
	SCENINF.TRIGGER[ID] = ~Val;
}

//------------------------------AI Functions-----------------------------//
void LoadAIFromDLL(byte Nat, char* Name);

extern "C" __declspec( dllexport ) void RunAI(byte Nat)
{
	Nat = AssignTBL[Nat];
	if (Nat < 1 || Nat > 7)
	{
		IntErr("RunAI(Nat) :  Invalid <Nat>");
		SCENINF.NErrors++;
		return;
	}
	Nation* NT = &NATIONS[Nat];
	if (!NT->AI_Enabled)
	{
		NT->AI_Enabled = true;
		//NT->NMask=0x7E;
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && OB->NNUM == Nat && OB->newMons->Usage == PeasantID)
			{
				for (int j = 0; j < GlobalAI.NAi; j++)
				{
					if (!strcmp(GlobalAI.Ai[j].Peasant_ID, OB->Ref.General->MonsterID))
					{
						LoadAIFromDLL(Nat, GlobalAI.Ai[j].LandAI[0]);
						goto RegUni;
					}
				}
			}
		}
	RegUni:;
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && OB->NNUM == Nat && OB->BrigadeID == 0xFFFF)
			{
				NT->CITY->RegisterNewUnit(OB);
				//OB->NMask=0x7E;
			}
		}
	}
}

extern "C" __declspec( dllexport ) void RunAIWithPeasants(byte Nat, char* P_Name)
{
	Nat = AssignTBL[Nat];
	if (Nat < 1 || Nat > 7)
	{
		IntErr("RunAI(Nat) :  Invalid <Nat>");
		SCENINF.NErrors++;
		return;
	}
	Nation* NT = &NATIONS[Nat];
	if (!NT->AI_Enabled)
	{
		NT->AI_Enabled = true;
		//NT->NMask=0x7E;
		for (int j = 0; j < GlobalAI.NAi; j++)
		{
			if (!strcmp(GlobalAI.Ai[j].Peasant_ID, P_Name))
			{
				LoadAIFromDLL(Nat, GlobalAI.Ai[j].LandAI[0]);
			}
		}
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && OB->NNUM == Nat && OB->BrigadeID == 0xFFFF)
			{
				NT->CITY->RegisterNewUnit(OB);
				//OB->NMask=0x7E;
			}
		}
	}
}

//-----------------------------Timer commands----------------------------//
extern "C" __declspec( dllexport ) void RunTimer(byte ID, int Long)
{
	if (ID >= 32)
	{
		IntErr("RunTimer : Invalid timer index");
		SCENINF.NErrors++;
		return;
	}
	GTimer* TM = SCENINF.TIME + ID;
	TM->Time = Long;
	TM->Used = true;
	TM->First = false;
}

extern "C" __declspec( dllexport ) bool TimerDone(byte ID)
{
	if (ID >= 32)
	{
		IntErr("TimerDone : Invalid timer index");
		SCENINF.NErrors++;
		return false;
	}
	GTimer* TM = SCENINF.TIME + ID;
	return TM->Time == 0 && TM->Used;
}

extern "C" __declspec( dllexport ) bool TimerDoneFirst(byte ID)
{
	if (ID >= 32)
	{
		IntErr("TimerDoneFirst : Invalid timer index");
		SCENINF.NErrors++;
		return false;
	}
	GTimer* TM = SCENINF.TIME + ID;
	if (TM->Time == 0 && TM->Used && TM->First)
	{
		TM->First = 0;
		return true;
	}
	else return false;
}

extern "C" __declspec( dllexport ) bool TimerIsEmpty(byte ID)
{
	if (ID >= 32)
	{
		IntErr("TimerIsEmpty : Invalid timer index");
		SCENINF.NErrors++;
		return false;
	}
	GTimer* TM = SCENINF.TIME + ID;
	return !TM->Used;
}

extern "C" __declspec( dllexport ) void FreeTimer(byte ID)
{
	if (ID >= 32)
	{
		IntErr("FreeTimer : Invalid timer index");
		SCENINF.NErrors++;
		return;
	}
	GTimer* TM = SCENINF.TIME + ID;
	TM->First = 0;
	TM->Time = 0;
	TM->Used = 0;
}

extern "C" __declspec( dllexport ) int GetTime(byte ID)
{
	if (ID >= 32)
	{
		IntErr("GeRunTimer : Invalid timer index");
		SCENINF.NErrors++;
		return 0;
	}
	GTimer* TM = SCENINF.TIME + ID;
	return TM->Time;
}

extern int tmtmt;

extern "C" __declspec( dllexport ) int GetGlobalTime()
{
	return REALTIME;
}

//---------------------------Dynamical zones-----------------------------//
extern "C" __declspec( dllexport ) bool UnitsCenter(GAMEOBJ* DstZone, GAMEOBJ* Units, word R)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("UnitsCenter(DstZone,Units,R) : invalid <Units>");
		SCENINF.NErrors++;
		return false;
	}
	TestUnitsGroup(Units->Index);
	int xs = 0;
	int ys = 0;
	int N = 0;
	UnitsGroup* UG = SCENINF.UGRP + Units->Index;
	for (int i = 0; i < UG->N; i++)
	{
		word MID = UG->IDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (!OB->Sdoxlo) && OB->Serial == UG->SNS[i])
			{
				xs += OB->RealX;
				ys += OB->RealY;
				N++;
			}
		}
	}
	if (N)
	{
		xs = (xs >> 4) / N;
		ys = (ys >> 4) / N;
		DstZone->Index = xs;
		DstZone->Serial = ys;
		DstZone->Type = '@   ' - 0x202020 + R;
	}
	else
	{
		DstZone->Type = -1;
		return false;
	}
	return true;
}

void ProcessScreen();
void GSYSDRAW();

extern "C" __declspec( dllexport ) void RefreshScreen()
{
	ProcessScreen();
	GSYSDRAW();
}

extern "C" __declspec( dllexport ) bool SelCenter(GAMEOBJ* DstZone, byte Nat, int R)
{
	Nat = AssignTBL[Nat];
	if (Nat > 7)
	{
		IntErr("SelCenter(DstZone,byte Nat,R) : invalid <Nat>");
		SCENINF.NErrors++;
		return false;
	}
	int xs = 0;
	int ys = 0;
	int N = 0;
	word* SNS = SerN[Nat];
	word* IDS = Selm[Nat];
	int NU = NSL[Nat];
	for (int i = 0; i < NU; i++)
	{
		word MID = IDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (!OB->Sdoxlo) && OB->Serial == SNS[i])
			{
				xs += OB->RealX;
				ys += OB->RealY;
				N++;
			}
		}
	}
	if (N)
	{
		xs = (xs >> 4) / N;
		ys = (ys >> 4) / N;
		DstZone->Index = xs;
		DstZone->Serial = ys;
		DstZone->Type = '@   ' - 0x202020 + R;
	}
	else
	{
		DstZone->Type = -1;
		return false;
	}
	return true;
}

extern "C" __declspec( dllexport ) bool CreateZoneNearUnit(GAMEOBJ* DstZone, GAMEOBJ* Zone, GAMEOBJ* UnitType, byte Nat,
                                                           int R)
{
	Nat = AssignTBL[Nat];
	if (UnitType->Type != 'UTYP')
	{
		IntErr("CreateZoneNearUnit : invalid <UnitType>");
		SCENINF.NErrors++;
		DstZone->Type = 0xFFFFFFFF;
		return false;
	}
	GeneralObject* GO = NATIONS[Nat].Mon[UnitType->Index];
	word* Mons = NatList[Nat];
	int N = NtNUnits[Nat];
	if (Zone)
	{
		int x0, y0;
		if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
		{
			x0 = int(Zone->Index) << 4;
			y0 = int(Zone->Serial) << 4;
		}
		else
		{
			if (Zone->Type == 'ZONE')
			{
				ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
				x0 = AZ->x << 4;
				y0 = AZ->y << 4;
			}
			else
			{
				IntErr("CreateZoneNearUnits : Invalid <Zone>");
				SCENINF.NErrors++;
				return false;
			}
		}
		int Rmin = 10000000;
		int xc = -1;
		int yc = -1;
		for (int i = 0; i < N; i++)
		{
			OneObject* OB = Group[Mons[i]];
			if (OB && OB->Ref.General == GO && !(OB->Sdoxlo && !OB->Hidden))
			{
				int norma = Norma(OB->RealX - x0, OB->RealY - y0);
				if (norma < Rmin)
				{
					xc = OB->RealX;
					yc = OB->RealY;
					Rmin = norma;
				}
			}
		}
		if (xc > -1)
		{
			DstZone->Index = xc >> 4;
			DstZone->Serial = yc >> 4;
			DstZone->Type = '@   ' - 0x202020 + R;
			return true;
		}
		else
		{
			DstZone->Type = 0xFFFFFFFF;
			return false;
		}
	}
	for (int i = 0; i < N; i++)
	{
		OneObject* OB = Group[Mons[i]];
		if (OB && OB->Ref.General == GO)
		{
			DstZone->Index = OB->RealX >> 4;
			DstZone->Serial = OB->RealY >> 4;
			DstZone->Type = '@   ' - 0x202020 + R;
			return true;
		}
	}
	DstZone->Type = 0xFFFFFFFF;
	return false;
}

extern "C" __declspec( dllexport ) bool CreateZoneNearGroup(GAMEOBJ* DstZone, GAMEOBJ* Zone, GAMEOBJ* Grp, int R)
{
	if (Grp->Type != 'UNIT')
	{
		IntErr("CreateZoneNearGroup : invalid <Group>");
		SCENINF.NErrors++;
		DstZone->Type = 0xFFFFFFFF;
		return false;
	}
	if (Zone)
	{
		int x0, y0;
		if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
		{
			x0 = int(Zone->Index) << 4;
			y0 = int(Zone->Serial) << 4;
		}
		else if (Zone->Type == 'ZONE')
		{
			ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
			x0 = AZ->x << 4;
			y0 = AZ->y << 4;
		}
		else
		{
			IntErr("CreateZoneNearUnits : Invalid <Zone>");
			SCENINF.NErrors++;
			return false;
		}
		int Rmin = 10000000;
		int xc = -1;
		int yc = -1;
		UnitsGroup* UG = SCENINF.UGRP + Grp->Index;
		for (int i = 0; i < UG->N; i++)
		{
			word MID = UG->IDS[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && (!OB->Sdoxlo) && OB->Serial == UG->SNS[i])
				{
					int norma = Norma(OB->RealX - x0, OB->RealY - y0);
					if (norma < Rmin)
					{
						xc = OB->RealX;
						yc = OB->RealY;
						Rmin = norma;
					}
				}
			}
		}
		if (xc > -1)
		{
			DstZone->Index = xc >> 4;
			DstZone->Serial = yc >> 4;
			DstZone->Type = '@   ' - 0x202020 + R;
			return true;
		}
		else
		{
			DstZone->Type = 0xFFFFFFFF;
			return false;
		}
	}
	return false;
}

extern "C" __declspec( dllexport ) void SetLightSpot(GAMEOBJ* Zone, int R, byte index)
{
	int x0 = 0;
	int y0 = 0;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		x0 = int(Zone->Index);
		y0 = int(Zone->Serial);
	}
	else if (Zone->Type == 'ZONE')
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
		x0 = AZ->x;
		y0 = AZ->y;
	}
	else if (Zone->Type != 'ZONE')
	{
		IntErr("SetLightSpot : Invalid  <Zone>");
		SCENINF.NErrors++;
		return;
	}
	if (index > 63)
	{
		IntErr("SetLightSpot : Invalid  <Index>");
		SCENINF.NErrors++;
		return;
	}
	SCENINF.LSpot[index].x = x0;
	SCENINF.LSpot[index].y = y0;
	SCENINF.LSpot[index].Type = R;
}

extern "C" __declspec( dllexport ) void ClearLightSpot(byte index)
{
	if (index > 63)
	{
		IntErr("ClearLightSpot : Invalid  <Index>");
		SCENINF.NErrors++;
		return;
	}
	SCENINF.LSpot[index].x = 0;
	SCENINF.LSpot[index].y = 0;
	SCENINF.LSpot[index].Type = 0;
}

extern int RealLx;
extern int RealLy;

extern "C" __declspec( dllexport ) void SetStartPoint(GAMEOBJ* Zone)
{
	int x;
	int y;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		x = int(Zone->Index);
		y = int(Zone->Serial);
	}
	else if (Zone->Type == 'ZONE')
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
		x = AZ->x;
		y = AZ->y;
	}
	else
	{
		IntErr("SetStartPoint : Invalid <Zone>");
		return;
	}
	x = (x - (RealLx >> 1)) >> 5;
	y = (y - RealLy) >> 5;
	if (x < 1)x = 1;
	if (y < 1)y = 1;
	if (x + smaplx > msx - 1)x = msx - smaplx - 1;
	if (y + smaply > msy - 1)y = msy - smaply - 1;
	mapx = x;
	mapy = y;
}

void AttackObjLink(OneObject* OBJ);
int GetTopDistance(int xa, int ya, int xb, int yb);
bool CheckVisibility(int x1, int y1, int x2, int y2, word MyID);

extern "C" __declspec( dllexport ) void AttackEnemyInZone(GAMEOBJ* Grp, GAMEOBJ* Zone, byte EnmNation)
{
	int xc, yc, R;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		xc = Zone->Index << 4;
		yc = Zone->Serial << 4;
		R = (Zone->Type & 0xFFFFFF) << 4;
	}
	else
	{
		if (Zone->Type != 'ZONE')
		{
			IntErr("AttackEnemyInZone : Invalid parameter <Zone>");
			SCENINF.NErrors++;
			return;
		}
		else
		{
			ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
			xc = AZ->x << 4;
			yc = AZ->y << 4;
			R = AZ->R << 4;
		}
	}
	word ENLIST[800];
	int NEN = 0;
	word* enm = NatList[EnmNation];
	int Nu = NtNUnits[EnmNation];
	int MinR = 100000000;
	word EMID = 0xFFFF;
	for (int i = 0; i < Nu; i++)
	{
		OneObject* OB = Group[enm[i]];
		if (OB && !(OB->Sdoxlo))
		{
			int r0 = Norma(OB->RealX - xc, OB->RealY - yc);
			if (r0 < R && NEN < 800)
			{
				ENLIST[NEN] = OB->Index;
				NEN++;
			}
		}
	}
	if (NEN)
	{
		UnitsGroup* UG = SCENINF.UGRP + Grp->Index;
		for (int i = 0; i < UG->N; i++)
		{
			word MID = UG->IDS[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && (!OB->Sdoxlo) && OB->Serial == UG->SNS[i])
				{
					if (!OB->LocalOrder)
					{
						//need to find appropriate unit;
						//1.Search for unit in the attack area
						AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
						int R0 = ADC->MinR_Attack;
						int R1 = ADC->MaxR_Attack;
						byte kmask = OB->newMons->KillMask;
						bool KBUI = OB->newMons->AttBuild;
						word EMID = 0xFFFF;
						int Dist = 10000;
						int Type = -1;
						int tx0 = OB->RealX >> 10;
						int ty0 = OB->RealY >> 10;
						for (int j = 0; j < NEN; j++)
						{
							OneObject* EOB = Group[ENLIST[j]];
							if (kmask & EOB->newMons->MathMask && !EOB->NewBuilding)
							{
								int R = Norma(EOB->RealX - OB->RealX, EOB->RealY - OB->RealY) >> 4;
								R = GetTopDistance(EOB->RealX >> 10, EOB->RealY >> 10, tx0, ty0);
								int Rv = Norma(OB->RealX - EOB->RealX, OB->RealY - EOB->RealY);
								if (Rv > R0 && Rv < R1)
								{
									if (R < Dist)
									{
										EMID = EOB->Index;
										Dist = R;
										Type = 0;
									}
								}
							}
						}
						Dist = 10000;
						if (EMID == 0xFFFF && KBUI)
						{
							for (int j = 0; j < NEN; j++)
							{
								OneObject* EOB = Group[ENLIST[j]];
								if (EOB->NewBuilding)
								{
									int R = Norma(EOB->RealX - OB->RealX, EOB->RealY - OB->RealY) >> 4;
									int Rt = GetTopDistance(EOB->RealX >> 10, EOB->RealY >> 10, tx0, ty0);
									if (R > R0 && R < R1)
									{
										if (Rt + R < Dist)
										{
											EMID = EOB->Index;
											Dist = Rt + R;
											Type = 1;
										}
									}
								}
							}
						}
						bool Vis = true;
						word OldEMID = EMID;
						if (EMID != 0xFFFF && (KBUI || R1 > 150))
						{
							OneObject* EOB = Group[EMID];
							if (!CheckVisibility(EOB->RealX, EOB->RealY, OB->RealX, OB->RealY, OB->Index))
							{
								Vis = false;
								EMID = 0xFFFF;
							}
						}
						Dist = 100000;
						if (EMID == 0xFFFF && KBUI)
						{
							//search for walls
							for (int j = 0; j < NEN; j++)
							{
								OneObject* EOB = Group[ENLIST[j]];
								if (EOB->Wall)
								{
									int R = Norma(EOB->RealX - OB->RealX, EOB->RealY - OB->RealY) >> 4;
									if (R > R0)
									{
										if (R < Dist)
										{
											EMID = EOB->Index;
											Dist = R;
											Type = 2;
										}
									}
								}
							}
						}
						if (EMID == 0xFFFF && OldEMID != 0xFFFF)
						{
							EMID = OldEMID;
						}
						if (EMID == 0xFFFF)
						{
							for (int j = 0; j < NEN; j++)
							{
								OneObject* EOB = Group[ENLIST[j]];
								if (kmask & EOB->newMons->MathMask || (KBUI && EOB->NewBuilding))
								{
									int R = Norma(EOB->RealX - OB->RealX, EOB->RealY - OB->RealY) >> 4;
									int Rt = GetTopDistance(EOB->RealX >> 10, EOB->RealY >> 10, tx0, ty0);
									if (R > R0 && Rt < 560)
									{
										if (Rt < Dist)
										{
											EMID = EOB->Index;
											Dist = Rt;
										}
									}
								}
							}
						}
						if (EMID != 0xFFFF)
						{
							OB->AttackObj(EMID, 1, 0, 0);
						}
					}
				}
			}
		}
	}
}

extern "C" __declspec( dllexport ) int GetMyNation()
{
	return MyNation;
}

int GetTopDistance(int xa, int ya, int xb, int yb);

extern "C" __declspec( dllexport ) int GetTopDst(GAMEOBJ* Z1, GAMEOBJ* Z2)
{
	int xc1, yc1, xc2, yc2;
	if ((Z1->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		xc1 = Z1->Index;
		yc1 = Z1->Serial;
	}
	else if (Z1->Type != 'ZONE')
	{
		IntErr("GetTopDst : Invalid parameter <Z1>");
		SCENINF.NErrors++;
		return 0xFFFF;
	}
	else
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Z1->Index].ZoneID[0];
		xc1 = AZ->x;
		yc1 = AZ->y;
	}
	if ((Z2->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		xc2 = Z2->Index;
		yc2 = Z2->Serial;
	}
	else if (Z2->Type != 'ZONE')
	{
		IntErr("GetTopDst : Invalid parameter <Z2>");
		SCENINF.NErrors++;
		return 0xFFFF;
	}
	else
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Z2->Index].ZoneID[0];
		xc2 = AZ->x;
		yc2 = AZ->y;
	}
	return GetTopDistance(xc1 >> 6, yc1 >> 6, xc2 >> 6, yc2 >> 6);
}

extern City CITY[8];
extern GlobalEnemyInfo GNFO;
void ResearchCurrentIsland(byte Nat);
void StartAIEx(byte Nat, char* Name, int Land, int Money, int ResOnMap, int Difficulty);

extern "C" __declspec( dllexport ) void StartAI(byte Nat, char* Name, int Land, int Money, int ResOnMap, int Difficulty)
{
	byte MSKS[8];
	for (int i = 0; i < 8; i++)
	{
		MSKS[i] = NATIONS[i].NMask;
		if (i)
		{
			NATIONS[i].NMask = 0x7E;
		}
		else
		{
			NATIONS[i].NMask = 1;
		}
	}
	GNFO.Clear();
	GNFO.Setup();
	for (int i = 0; i < 8; i++)
	{
		NATIONS[i].NMask = MSKS[i];
	}
	StartAIEx(Nat, Name, Land, Money, ResOnMap, Difficulty);
}

void StartAIEx(byte Nat, char* Name, int Land, int Money, int ResOnMap, int Difficulty)
{
	Nat = AssignTBL[Nat];
	LoadAIFromDLL(Nat, GetTextByID(Name));
	if (Difficulty != -1)CITY[Nat].Difficulty = Difficulty;
	CITY[Nat].StartRes = Money;
	CITY[Nat].LandType = Land;
	CITY[Nat].ResOnMap = ResOnMap;
	strcpy(CITY[Nat].AIID, Name);
	NATIONS[Nat].AI_Enabled = 1;
	for (int i = 0; i < 8; i++)ResearchCurrentIsland(i);
	word* Units = NatList[Nat];
	int N = NtNUnits[Nat];
	int NML = 0;
	int cxi[4];
	int cyi[4];
	for (int i = 0; i < N; i++)
	{
		OneObject* OB = Group[Units[i]];
		if (OB)
		{
			byte Usage = OB->newMons->Usage;
			if (Usage == MelnicaID && NML < 4)
			{
				if (CITY[Nat].NMeln < 4)
				{
					CITY[Nat].MelnID[NML] = OB->Index;
					CITY[Nat].MelnSN[NML] = OB->Serial;
					cxi[NML] = OB->RealX >> 4;
					cyi[NML] = OB->RealY >> 4;
					NML++;
				}
			}
			if (/*Usage==CenterID||*/Usage == SkladID)
			{
				if (CITY[Nat].NStoneSklads < 8)
				{
					CITY[Nat].StoneSkladID[CITY[Nat].NStoneSklads] = OB->Index;
					CITY[Nat].StoneSkladSN[CITY[Nat].NStoneSklads] = OB->Serial;
					CITY[Nat].NStoneSklads++;
				}
				if (CITY[Nat].NWoodSklads < 8)
				{
					CITY[Nat].WoodSkladID[CITY[Nat].NWoodSklads] = OB->Index;
					CITY[Nat].WoodSkladSN[CITY[Nat].NWoodSklads] = OB->Serial;
					CITY[Nat].NWoodSklads++;
				}
			}
		}
	}
	CITY[Nat].NMeln = NML;
	CITY[Nat].NFields = 0;
	if (NML)
	{
		int NF = 0;
		OneSprite* OS = Sprites;
		for (int i = 0; i < MaxSprt && NF < 512; i++)
		{
			if (OS->Enabled && OS->SG == &COMPLEX)
			{
				int cx = OS->x;
				int cy = OS->y;
				for (int j = 0; j < NML; j++)
				{
					if (Norma(cx - cxi[j], cy - cyi[j]) < 800)
					{
						CITY[Nat].FieldsID[NF] = i;
						CITY[Nat].FieldsSN[NF] = (OS->x >> 5) + ((OS->y >> 5) << 8);
						NF++;
					}
				}
			}
			OS++;
		}
		CITY[Nat].NFields = NF;
	}
	CITY[Nat].FieldReady = 0;
	//registering storages&centers
}

extern "C" __declspec( dllexport ) void DoNotUseSelInAI(byte Nat)
{
	Nat = AssignTBL[Nat];
	word* SL = Selm[Nat];
	word* SN = SerN[Nat];
	int N = NSL[Nat];
	for (int i = 0; i < N; i++)
	{
		word MID = SL[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && !(OB->Sdoxlo || OB->NewBuilding))
			{
				OB->Borg = true;
				OB->DoNotCall = true;
				OB->NoBuilder = 1;
			}
		}
	}
}

extern bool Tutorial;
extern bool NoPress;
extern bool TutOver;
extern bool MiniActive;
void ClearMINIMENU();

extern "C" __declspec( dllexport ) void SetTutorial(bool State)
{
	Tutorial = State;
	//MiniActive=0;
	ClearMINIMENU();
}

extern "C" __declspec( dllexport ) bool GetQuestPressed()
{
	return (!NoPress) && TutOver && Tutorial;
}

#define DEVELOPMENT	   0x1001
#define WATER_BATTLE   0x1002
#define LAND_BATTLE    0x1003
#define MINES_CPTURE   0x1004
#define TOWN_DEFENCE   0x1005
#define MINES_UPGRADE  0x1006
#define FAST_DIVERSION 0x1007

extern "C" __declspec( dllexport ) void SetAIProperty(byte NAT, int Prop, int Val)
{
	City* CT = CITY + NAT;
	switch (Prop)
	{
	case DEVELOPMENT:
		CT->AI_DEVELOPMENT = 0 != Val;
		break;
	case WATER_BATTLE:
		CT->AI_WATER_BATTLE = 0 != Val;
		break;
	case LAND_BATTLE:
		CT->AI_LAND_BATTLE = 0 != Val;
		break;
	case MINES_CPTURE:
		CT->AI_MINES_CPTURE = 0 != Val;
		break;
	case TOWN_DEFENCE:
		CT->AI_TOWN_DEFENCE = 0 != Val;
		break;
	case MINES_UPGRADE:
		CT->AI_MINES_UPGRADE = 0 != Val;
		break;
	case FAST_DIVERSION:
		CT->AI_FAST_DIVERSION = 0 != Val;
		break;
	default:
		IntErr("SetAIProperty : Unknown property");
	}
}

int ProcessMultilineQuestion(int Nx, char* Bmp1, byte or1, char* Text1, char* Quest);

extern "C" __declspec( dllexport ) int AskMultilineQuestion(int Nx, char* Name1, byte or1, char* Quest)
{
	if (PlayGameMode)RGAME.Extract();
	int q1 = -1;
	int q2 = -1;
	for (int i = 0; i < SCENINF.NPages; i++)
	{
		if (!strcmp(Name1, SCENINF.PageID[i]))
		{
			q1 = i;
			goto qq2;
		}
	}
	IntErr("AskMultilineQuestion : Unknown page : %s", Name1);
	return 0;
qq2:
	int t = ProcessMultilineQuestion(Nx, SCENINF.PageBMP[q1], or1, SCENINF.Page[q1], Quest);
	if (RecordMode)
	{
		RGAME.AddByte(3);
		RGAME.AddByte(t);
	}
	return t;
}

extern "C" __declspec( dllexport ) void SetReadyState(GAMEOBJ* Units, bool State)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("TakeFood(Units) : Incorrect parameter");
		return;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p])
			{
				OB->Ready = State;
			}
		}
	}
}

extern "C" __declspec( dllexport ) void TakeFood(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("TakeFood(Units) : Incorrect parameter");
		return;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p] && OB->newMons->Peasant)
			{
				OB->TakeResource(OB->RealX >> 4, OB->RealY >> 4, FoodID, 128, 0);
			}
		}
	}
}

extern "C" __declspec( dllexport ) void TakeWood(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("TakeWood(Units) : Incorrect parameter");
		return;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p] && OB->newMons->Peasant)
			{
				OB->TakeResource(OB->RealX >> 4, OB->RealY >> 4, TreeID, 128, 0);
			}
		}
	}
}

extern "C" __declspec( dllexport ) void TakeStone(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("TakeStone(Units) : Incorrect parameter");
		return;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p] && OB->newMons->Peasant)
			{
				OB->TakeResource(OB->RealX >> 4, OB->RealY >> 4, StoneID, 128, 0);
			}
		}
	}
}

void BuildWithSelected(byte NI, word ObjID, byte OrdType);

extern "C" __declspec( dllexport ) void RepairBuildingsBySel(byte Nat, GAMEOBJ* Buildings)
{
	if (Nat > 7)return;
	if (Buildings->Type != 'UNIT')
	{
		IntErr("RepairBuildings(Nat,Buildings) : Incorrect parameter <Buildings>");
		return;
	}
	word* UIDS = SCENINF.UGRP[Buildings->Index].IDS;
	word* SIDS = SCENINF.UGRP[Buildings->Index].SNS;
	int Nu = SCENINF.UGRP[Buildings->Index].N;
	byte otp = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && OB->Serial == SIDS[p] && OB->NewBuilding)
			{
				BuildWithSelected(Nat, OB->Index, otp);
				otp = 1;
			}
		}
	}
}

extern "C" __declspec( dllexport ) bool CheckBuildingsComplete(GAMEOBJ* Buildings)
{
	if (Buildings->Type != 'UNIT')
	{
		IntErr("CheckBuildingsComplete(Buildings) : Incorrect parameter <Buildings>");
		return false;
	}
	word* UIDS = SCENINF.UGRP[Buildings->Index].IDS;
	word* SIDS = SCENINF.UGRP[Buildings->Index].SNS;
	int Nu = SCENINF.UGRP[Buildings->Index].N;
	byte otp = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p]
				&& OB->NewBuilding && OB->Stage < OB->Ref.General->MoreCharacter->ProduceStages)
				return false;
		}
	}
	return true;
}

extern "C" __declspec( dllexport ) int GetKilled(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("GetKilled(Units) : Incorrect parameter <Units>");
		return 0;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NK = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p])
			{
				NK += OB->Kills;
			}
		}
	}
	return NK;
}

extern "C" __declspec( dllexport ) int GetUnitsByNation(GAMEOBJ* Units, byte Nat)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("GetUnitsByNation(Units,Nat) : Incorrect parameter <Units>");
		return 0;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NK = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (OB->Serial >> 1) == (SIDS[p] >> 1) && OB->NNUM == Nat)NK++;
		}
	}
	return NK;
}

extern "C" __declspec( dllexport ) void ProduceUnit(GAMEOBJ* Units, GAMEOBJ* UnitType, GAMEOBJ* DestGroup)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("ProduceUnit(Units,UnitType,Dest) : Invalid parameter <Units>");
		return;
	}
	if (DestGroup->Type != 'UNIT')
	{
		IntErr("ProduceUnit(Units,UnitType,Dest) : Invalid parameter <DestGroup>");
		return;
	}
	if (UnitType->Type != 'UTYP')
	{
		IntErr("ProduceUnit(Units,UnitType,Dest) : Invalid parameter <UnitType>");
		return;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NK = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p])
			{
				OB->Produce(UnitType->Index, DestGroup->Index);
			}
		}
	}
}

extern "C" __declspec( dllexport ) void ProduceUnitFast(GAMEOBJ* Units, GAMEOBJ* UnitType, GAMEOBJ* DestGroup,
                                                        int Speed)
{
	if (Speed > 6)Speed = 6;
	if (Units->Type != 'UNIT')
	{
		IntErr("ProduceUnit(Units,UnitType,Dest) : Invalid parameter <Units>");
		return;
	}
	if (DestGroup->Type != 'UNIT')
	{
		IntErr("ProduceUnit(Units,UnitType,Dest) : Invalid parameter <DestGroup>");
		return;
	}
	if (UnitType->Type != 'UTYP')
	{
		IntErr("ProduceUnit(Units,UnitType,Dest) : Invalid parameter <UnitType>");
		return;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NK = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && OB->Serial == SIDS[p])
			{
				OB->Produce(UnitType->Index + 8192 * (Speed + 1), DestGroup->Index);
			}
		}
	}
}

void AddOneUnitToGroup(GAMEOBJ* Units, OneObject* OB)
{
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	for (int i = 0; i < Nu; i++)if (OB->Index == UIDS[i])return;
	UIDS = (word*)realloc(UIDS, (Nu + 1) << 1);
	SCENINF.UGRP[Units->Index].IDS = UIDS;
	SIDS = (word*)realloc(SIDS, (Nu + 1) << 1);
	SCENINF.UGRP[Units->Index].SNS = SIDS;
	SCENINF.UGRP[Units->Index].N = Nu + 1;
	UIDS[Nu] = OB->Index;
	SIDS[Nu] = OB->Serial;
}

extern "C" __declspec( dllexport ) void SaveSelectedUnits(byte NI, GAMEOBJ* Units, bool add)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("SaveSelectedUnits : Invalid <Units>");
		return;
	}
	if (!add)
	{
		if (SCENINF.UGRP[Units->Index].IDS)
		{
			free(SCENINF.UGRP[Units->Index].IDS);
			SCENINF.UGRP[Units->Index].IDS = NULL;
			free(SCENINF.UGRP[Units->Index].SNS);
			SCENINF.UGRP[Units->Index].SNS = NULL;
			free(SCENINF.UGRP[Units->Index].IDS);
			SCENINF.UGRP[Units->Index].N = 0;
		}
	}
	int NS = NSL[NI];
	word* uni = Selm[NI];
	word* sns = SerN[NI];
	for (int i = 0; i < NS; i++)
	{
		word MID = uni[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == sns[i])
			{
				AddOneUnitToGroup(Units, OB);
			}
		}
	}
}

extern "C" __declspec( dllexport ) bool CheckProduction(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("CheckProduction : Invalid <Units>");
		return false;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	for (int i = 0; i < Nu; i++)
	{
		word MID = UIDS[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && OB->Serial == SIDS[i] && OB->NewBuilding && OB->Ready && !(OB->Sdoxlo || OB->LocalOrder))
				return true;
		}
	}
	return false;
}

extern "C" __declspec( dllexport ) bool CreateBuilding(byte Nat, GAMEOBJ* Zone, GAMEOBJ* UnitType, GAMEOBJ* DestGroup)
{
	if (Nat > 7)return false;
	if (UnitType->Type != 'UTYP')
	{
		IntErr("CreateBuilding(Zone,UnitType,DestGroup) : Invalid <UnitType>");
		return false;
	}
	if (DestGroup->Type != 'UNIT')
	{
		IntErr("CreateBuilding(Zone,UnitType,DestGroup) : Invalid <DestGroup>");
		return false;
	}
	int zx = 0;
	int zy = 0;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		int R0 = Zone->Type & 0x00FFFFFF;
		zx = int(Zone->Index) << 4;
		zy = int(Zone->Serial) << 4;
	}
	else
	{
		if (Zone->Type == 'ZONE')
		{
			zx = AZones[SCENINF.ZGRP[Zone->Index].ZoneID[0]].x << 4;
			zy = AZones[SCENINF.ZGRP[Zone->Index].ZoneID[0]].y << 4;
		}
		else
		{
			IntErr("CreateBuilding(Zone,UnitType,DestGroup) : Invalid <Zone>");
			return false;
		}
	}
	int id = NATIONS[Nat].CreateNewMonsterAt(zx, zy, UnitType->Index, 0);
	if (id != -1)
	{
		OneObject* OB = Group[id];
		int N = SCENINF.UGRP[DestGroup->Index].N;
		SCENINF.UGRP[DestGroup->Index].IDS = (word*)realloc(SCENINF.UGRP[DestGroup->Index].IDS, (N + 1) * 2);
		SCENINF.UGRP[DestGroup->Index].SNS = (word*)realloc(SCENINF.UGRP[DestGroup->Index].SNS, (N + 1) * 2);
		SCENINF.UGRP[DestGroup->Index].IDS[N] = id;
		SCENINF.UGRP[DestGroup->Index].SNS[N] = OB->Serial;
		SCENINF.UGRP[DestGroup->Index].N++;
		return true;
	}
	return false;
}

extern "C" __declspec( dllexport ) void SetDestPoint(GAMEOBJ* Units, GAMEOBJ* Zone)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("SetDestPoint(Units,Zone) : Incorrect parameter <Units>");
		return;
	}
	int zx = 0;
	int zy = 0;
	if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		int R0 = Zone->Type & 0x00FFFFFF;
		zx = int(Zone->Index) << 4;
		zy = int(Zone->Serial) << 4;
	}
	else
	{
		if (Zone->Type == 'ZONE')
		{
			zx = AZones[SCENINF.ZGRP[Zone->Index].ZoneID[0]].x << 4;
			zy = AZones[SCENINF.ZGRP[Zone->Index].ZoneID[0]].y << 4;
		}
		else
		{
			IntErr("SetDestPoint(Units,Zone) : Incorrect parameter <Zone>");
			return;
		}
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	int NK = 0;
	for (int p = 0; p < Nu; p++)
	{
		word MID = UIDS[p];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB && (OB->Hidden || !OB->Sdoxlo) && (!OB->LocalOrder) && OB->Serial == SIDS[p] && OB->Ref.General->CanDest)
			{
				OB->DstX = zx;
				OB->DstY = zy;
			}
		}
	}
}

extern "C" __declspec( dllexport ) void RegisterDynGroup(GAMEOBJ* Units)
{
	Units->Type = 'UNIT';
	if (SCENINF.NUGRP >= SCENINF.MaxUGRP)
	{
		SCENINF.MaxUGRP += 32;
		SCENINF.UGRP = (UnitsGroup*)realloc(SCENINF.UGRP, SCENINF.MaxUGRP * sizeof UnitsGroup);
	}
	Units->Index = SCENINF.NUGRP;
	UnitsGroup* UG = SCENINF.UGRP + SCENINF.NUGRP;
	UG->IDS = NULL;
	UG->SNS = NULL;
	UG->N = 0;
	SCENINF.NUGRP++;
	Units->Serial = 0;
}

extern "C" __declspec( dllexport ) int GetNUnits(GAMEOBJ* Units)
{
	if (Units->Type != 'UNIT')
	{
		IntErr("GetNUnits(Units) : Incorrect parameter <Units>");
		return 0;
	}
	return SCENINF.UGRP[Units->Index].N;
}

struct OneUnit
{
	word Index;
	word Serial;
	word Life;
	word MaxLife;
	byte AddDamage;
	byte AddShield;
	word Stage;
	word MaxStage;
	word Kills;
	word NIndex;
	byte Usage;
	byte Building;
	int x, y;
	byte Reserved[16];
};

extern "C" __declspec( dllexport ) bool GetUnitInfo(GAMEOBJ* Units, int Index, OneUnit* Uni)
{
	Uni->Index = 0xFFFF;
	if (Units->Type != 'UNIT')
	{
		IntErr("GetUnitInfo(Units,Index,Uni) : Incorrect parameter <Units>");
		return false;
	}
	word* UIDS = SCENINF.UGRP[Units->Index].IDS;
	word* SIDS = SCENINF.UGRP[Units->Index].SNS;
	int Nu = SCENINF.UGRP[Units->Index].N;
	if (Index >= Nu)
	{
		return false;
	}
	if (UIDS[Index] != 0xFFFF)
	{
		OneObject* OB = Group[UIDS[Index]];
		if (OB && OB->Serial == SIDS[Index] && (!OB->Sdoxlo))
		{
			Uni->AddDamage = OB->AddDamage;
			Uni->AddShield = OB->AddShield;
			Uni->Building = OB->NewBuilding;
			Uni->Index = OB->Index;
			Uni->Kills = OB->Kills;
			Uni->Life = OB->Life;
			Uni->MaxLife = OB->Ref.General->MoreCharacter->Life;
			Uni->MaxStage = OB->Ref.General->MoreCharacter->ProduceStages;
			Uni->NIndex = OB->NIndex;
			Uni->Serial = OB->Serial;
			Uni->Stage = OB->Stage;
			Uni->Usage = OB->newMons->Usage;
			Uni->x = OB->RealX >> 4;
			Uni->y = OB->RealY >> 4;
			return true;
		}
	}
	return false;
}

extern "C" __declspec( dllexport ) void SetUnitInfo(OneUnit* Uni)
{
	if (Uni->Index != 0xFFFF)
	{
		OneObject* OB = Group[Uni->Index];
		if (OB && OB->Serial == Uni->Serial && (!OB->Sdoxlo))
		{
			OB->Life = Uni->Life;
			OB->AddDamage = Uni->AddDamage;
			OB->AddShield = Uni->AddShield;
			if (!OB->NewBuilding)
			{
				OB->RealX = Uni->x << 4;
				OB->RealY = Uni->y << 4;
			}
		}
	}
}

extern "C" __declspec( dllexport ) void RemoveGroup(GAMEOBJ* Source, GAMEOBJ* Dest)
{
	if (Source->Type != 'UNIT')
	{
		IntErr("RemoveGroup(Source,Dest) : Invalid <Source>");
		return;
	}
	if (Dest->Type != 'UNIT')
	{
		IntErr("RemoveGroup(Source,Dest) : Invalid <Dest>");
		return;
	}

	word* UIDS = SCENINF.UGRP[Source->Index].IDS;
	word* SIDS = SCENINF.UGRP[Source->Index].SNS;
	int Ns = SCENINF.UGRP[Source->Index].N;
	int Nd = SCENINF.UGRP[Dest->Index].N;
	if (Ns)
	{
		SCENINF.UGRP[Dest->Index].IDS = (word*)realloc(SCENINF.UGRP[Dest->Index].IDS, (Ns + Nd) * 2);
		SCENINF.UGRP[Dest->Index].SNS = (word*)realloc(SCENINF.UGRP[Dest->Index].SNS, (Ns + Nd) * 2);
		memcpy(SCENINF.UGRP[Dest->Index].IDS + Nd, SCENINF.UGRP[Source->Index].IDS, Ns * 2);
		memcpy(SCENINF.UGRP[Dest->Index].SNS + Nd, SCENINF.UGRP[Source->Index].SNS, Ns * 2);
		free(SCENINF.UGRP[Source->Index].IDS);
		free(SCENINF.UGRP[Source->Index].SNS);
		SCENINF.UGRP[Source->Index].IDS = NULL;
		SCENINF.UGRP[Source->Index].SNS = NULL;
		SCENINF.UGRP[Source->Index].N = 0;
		SCENINF.UGRP[Dest->Index].N += Ns;
	}
}

//-------------------------AI Low level functions------------------------//
const int AI_PROB[4] = {32768 / 100, 32768 / 50, 32768 / 10, 32768};
byte CurAINation = 1;
City* CCIT = NULL;
Nation* CNAT;
#define PRC(x) ((32768*##x##)/100)

int PERCONV[101] = {
	PRC( 0 ),PRC( 1 ),PRC( 2 ),PRC( 3 ),PRC( 4 ),PRC( 5 ),PRC( 6 ),PRC( 7 ),PRC( 8 ),PRC( 9 ),
	PRC( 10 ),PRC( 11 ),PRC( 12 ),PRC( 13 ),PRC( 14 ),PRC( 15 ),PRC( 16 ),PRC( 17 ),PRC( 18 ),PRC( 19 ),
	PRC( 20 ),PRC( 21 ),PRC( 22 ),PRC( 23 ),PRC( 24 ),PRC( 25 ),PRC( 26 ),PRC( 27 ),PRC( 28 ),PRC( 29 ),
	PRC( 30 ),PRC( 31 ),PRC( 32 ),PRC( 33 ),PRC( 34 ),PRC( 35 ),PRC( 36 ),PRC( 37 ),PRC( 38 ),PRC( 39 ),
	PRC( 40 ),PRC( 41 ),PRC( 42 ),PRC( 43 ),PRC( 44 ),PRC( 45 ),PRC( 46 ),PRC( 47 ),PRC( 48 ),PRC( 49 ),
	PRC( 50 ),PRC( 51 ),PRC( 52 ),PRC( 53 ),PRC( 54 ),PRC( 55 ),PRC( 56 ),PRC( 57 ),PRC( 58 ),PRC( 59 ),
	PRC( 60 ),PRC( 61 ),PRC( 62 ),PRC( 63 ),PRC( 64 ),PRC( 65 ),PRC( 66 ),PRC( 67 ),PRC( 68 ),PRC( 69 ),
	PRC( 70 ),PRC( 71 ),PRC( 72 ),PRC( 73 ),PRC( 74 ),PRC( 75 ),PRC( 76 ),PRC( 77 ),PRC( 78 ),PRC( 79 ),
	PRC( 80 ),PRC( 81 ),PRC( 82 ),PRC( 83 ),PRC( 84 ),PRC( 85 ),PRC( 86 ),PRC( 87 ),PRC( 88 ),PRC( 89 ),
	PRC( 90 ),PRC( 91 ),PRC( 92 ),PRC( 93 ),PRC( 94 ),PRC( 95 ),PRC( 96 ),PRC( 97 ),PRC( 98 ),PRC( 99 ),
	PRC( 100 )
};

char* CurAIDLL = NULL;

void AIER(char* Mess)
{
	MessageBox(NULL, Mess, CurAIDLL, MB_TOPMOST);
}

void AI_Error()
{
	AIER("ERROR: AI function must be called only from AI section.");
}

extern "C" __declspec( dllexport ) int GetAINation()
{
	if (AiIsRunNow)
	{
		return CurAINation;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern byte CannonState;

extern "C" __declspec( dllexport ) bool TryUnit(GAMEOBJ* UnitType, int Max, byte CostPercent, byte Probability)
{
	if (AiIsRunNow)
	{
		if (UnitType->Type != 'UTYP')
		{
			AIER("TryUnit : invalid <UnitType>");
			return 0;
		}
		if (Probability > 100)
		{
			AIER("TryUnit : invalid <Probability>");
			return 0;
		}
		if (CCIT->NBestProjects >= 127)
		{
			return false;
		}
		int UID = UnitType->Index;

		GeneralObject* GO = CNAT->Mon[UID];
		if (CannonState == 1)
		{
			if (GO->newMons->Artilery || GO->newMons->Usage == TowerID)
			{
				return false;
			}
		}

		GO->Branch = 0xFF;
		if (Max > CCIT->UnitAmount[UID])
		{
			word prid = CCIT->PRPIndex[UID];
			if (prid != 0xFFFF)
			{
				ProposedProject* PRP = &CCIT->Prop[prid];
				if (rando() < PERCONV[Probability] && CostPercent >= PRP->Percent)
				{
					if (CCIT->PresentProject)
					{
						if (!GO->newMons->Building)
						{
							CCIT->BestProj[CCIT->NBestProjects] = prid;
							CCIT->NBestProjects++;
							return true;
						}
					}
					else
					{
						CCIT->BestProj[CCIT->NBestProjects] = prid;
						CCIT->NBestProjects++;
						return true;
					}
				}
			}
		}
		return false;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern "C" __declspec( dllexport ) bool TryUpgrade(GAMEOBJ* Upgrade, int CostPercent, int Probability)
{
	if (Upgrade->Type == 'SAFE')
	{
		return false;
	}
	if (AiIsRunNow)
	{
		if (Upgrade->Type != 'UPGR')
		{
			AIER("TryUpgrade : invalid <Upgrade>");
			return 0;
		}
		if (Probability > 100)
		{
			AIER("TryUpgrade : invalid <Probability>");
			return 0;
		}
		if (CCIT->NBestProjects >= 127)
		{
			return false;
		}
		int UID = Upgrade->Index;

		NewUpgrade* NU = CNAT->UPGRADE[UID];
		NU->Branch = 0xFF;
		word prid = CCIT->UPGIndex[UID];
		if (prid != 0xFFFF)
		{
			ProposedProject* PRP = &CCIT->Prop[prid];
			if (rando() < PERCONV[Probability] && CostPercent >= PRP->Percent)
			{
				CCIT->BestProj[CCIT->NBestProjects] = prid;
				CCIT->NBestProjects++;
				return true;
			}
		}
		return false;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern "C" __declspec( dllexport ) void SetMineBalanse(int N, word* Bal)
{
	if (AiIsRunNow)
	{
		CNAT->PBL = Bal;
		CNAT->NPBal = N;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void SetPDistribution(int OnFood, int OnWood, int OnStone)
{
	if (AiIsRunNow)
	{
		CNAT->POnFood = OnFood;
		CNAT->POnWood = OnWood;
		CNAT->POnStone = OnStone;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void AssignAmountOfMineUpgrades(int MU)
{
	if (AiIsRunNow)
	{
		CNAT->UGRP_MINEUP.N = MU;
		CNAT->UGRP_MINEUP.UIDS = new word[MU];
		CNAT->UGRP_MINEUP.UVAL = new word[MU];
		memset(CNAT->UGRP_MINEUP.UIDS, 0xFF, MU * 2);
		memset(CNAT->UGRP_MINEUP.UVAL, 0xFF, MU * 2);
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) bool AssignMineUpgrade(word U, char* Str, word val)
{
	if (AiIsRunNow)
	{
		if (U < CNAT->UGRP_MINEUP.N)
		{
			NewUpgrade** NUG = CNAT->UPGRADE;
			int N = CNAT->NUpgrades;
			int k;
			for (k = 0; k < N && strcmp(NUG[k]->Name, Str); k++);
			if (k >= N)
			{
				char cc[128];
				sprintf(cc, "AssignMineUpgrade : Invalid upgrade name:%s", Str);
				AIER(cc);
				return false;
			}
			else
			{
				CNAT->UGRP_MINEUP.UIDS[U] = k;
				CNAT->UGRP_MINEUP.UVAL[U] = val;
			}
		}
		else
		{
			AIER("AssignMineUpgrade : invalid index of upgrade");
			return 0;
		}
	}
	else
	{
		AI_Error();
		return 0;
	}
	return 1;
}

int GETUID(char* Name)
{
	GeneralObject** GOG = NATIONS[CurAINation].Mon;
	int N = NATIONS[CurAINation].NMon;
	int k;
	for (k = 0; k < N && strcmp(GOG[k]->MonsterID, Name); k++);
	if (k >= N)return -1;
	return k;
}

extern "C" __declspec( dllexport ) void AssignMine(char* Name)
{
	if (AiIsRunNow)
	{
		int id = GETUID(Name);
		if (id == -1)
		{
			AIER("AssignMine : Unknown ID");
		}
		else CNAT->UID_MINE = id;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void AssignPeasant(char* Name)
{
	if (AiIsRunNow)
	{
		int id = GETUID(Name);
		if (id == -1)
		{
			AIER("AssignPeasant : Unknown ID");
		}
		else CNAT->UID_PEASANT = id;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void AssignHouse(char* Name)
{
	if (AiIsRunNow)
	{
		int id = GETUID(Name);
		if (id == -1)
		{
			AIER("AssignHouse : Unknown ID");
		}
		else
		{
			CNAT->UID_HOUSE = id;
			NewMonster* NM = CNAT->Mon[CNAT->UID_HOUSE]->newMons;
			int x0 = NM->BuildX0 - NM->BuildY0;
			int x1 = NM->BuildX1 - NM->BuildY1;
			int y0 = NM->BuildX0 + NM->BuildY0;
			int y1 = NM->BuildX1 + NM->BuildY1;
			CCIT->FarmLX = (x1 - x0) + 20;
			CCIT->FarmLY = (y1 - y0) + 20;
			CCIT->LFarmIndex = 0;
			CCIT->LFarmRadius = 0;
		}
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void AssignWall(char* Name)
{
	if (AiIsRunNow)
	{
		int id = GETUID(Name);
		if (id == -1)
		{
			AIER("AssignWall : Unknown ID");
		}
		else CNAT->UID_WALL = id;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void SET_MINE_CAPTURE_RADIUS(int x)
{
	if (AiIsRunNow)
	{
		CNAT->MINE_CAPTURE_RADIUS = x;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void SET_MINE_UPGRADE1_RADIUS(int x)
{
	if (AiIsRunNow)
	{
		CNAT->MINE_UPGRADE1_RADIUS = x;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void SET_MINE_UPGRADE2_RADIUS(int x)
{
	if (AiIsRunNow)
	{
		CNAT->MINE_UPGRADE2_RADIUS = x;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void SET_DEFAULT_MAX_WORKERS(int x)
{
	if (AiIsRunNow)
	{
		CNAT->DEFAULT_MAX_WORKERS = x;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) void SET_MIN_PEASANT_BRIGADE(int x)
{
	if (AiIsRunNow)
	{
		CNAT->MIN_PBRIG = x;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport ) int GetMoney(byte id)
{
	if (AiIsRunNow)
	{
		if (id < 6)
		{
			return XRESRC( CurAINation, id );
		}
		else return 0;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern "C" __declspec( dllexport ) int GetUnits(GAMEOBJ* UnitType)
{
	if (AiIsRunNow)
	{
		if (UnitType->Type != 'UTYP')
		{
			AIER("GetUnits : Invaid unit ID");
			return 0;
		}

		return CCIT->UnitAmount[UnitType->Index];
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern "C" __declspec( dllexport )int GetUnitsByUsage(byte Nat, byte Usage)
{
	Nat = AssignTBL[Nat];
	if (AiIsRunNow)
	{
		int N = NtNUnits[Nat];
		word* Units = NatList[Nat];
		int NUN = 0;
		for (int i = 0; i < N; i++)
		{
			word MID = Units[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB && (!OB->Sdoxlo) && OB->newMons->Usage == Usage)
				{
					NUN++;
				}
			}
		}
		return NUN;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

void CmdTorg(byte NI, byte SellRes, byte BuyRes, int SellAmount);

byte INVECO[6] = {2, 5, 4, 0, 1, 3};

void PerformTorg(byte Nation, byte SellRes, byte BuyRes, int SellAmount);

//Performs market exchange for bots
extern "C" __declspec( dllexport ) void AI_Torg(byte SellRes, byte BuyRes, int SellAmount)
{
	PerformTorg(CNAT->NNUM, INVECO[SellRes], INVECO[BuyRes], SellAmount);
}

int GetTorgResultEx(byte SellRes, byte BuyRes, int SellAmount);

//Calculates market exchange results for bots
extern "C" __declspec( dllexport ) int GetTorgResult(byte SellRes, byte BuyRes, int SellAmount)
{
	return GetTorgResultEx(INVECO[SellRes], INVECO[BuyRes], SellAmount);
}

extern "C" __declspec( dllexport )int GetReadyUnits(GAMEOBJ* UnitType)
{
	if (AiIsRunNow)
	{
		if (UnitType->Type != 'UTYP')
		{
			AIER("GetReadyUnits : Invaid unit ID");
			return 0;
		}

		return CCIT->ReadyAmount[UnitType->Index];
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern "C" __declspec( dllexport )void SetUpgradeLock(byte Res, byte Val)
{
	if (Res < 8)
	{
		CCIT->LockUpgrade[Res] = Val;
	}
}

extern "C" __declspec( dllexport )void SetDefSettings(int p1, int p2)
{
	if (AiIsRunNow)
	{
		CCIT->MaxGuards = p1;
		CCIT->AbsMaxGuards = p2;
	}
	else
	{
		AI_Error();
	}
}

extern "C" __declspec( dllexport )int GetMaxPeasantsInMines()
{
	int N = NtNUnits[CNAT->NNUM];
	word* Uni = NatList[CNAT->NNUM];
	int Max = 0;
	for (int i = 0; i < N; i++)
	{
		OneObject* OB = Group[Uni[i]];
		if (OB && OB->Inside)
		{
			int Nm = OB->Ref.General->MoreCharacter->MaxInside + OB->AddInside;
			if (Nm > Max)
			{
				Max = Nm;
			}
		}
	}
	return Max;
}

extern "C" __declspec( dllexport )bool UpgIsDone(GAMEOBJ* Upgrade)
{
	if (AiIsRunNow)
	{
		if (Upgrade->Type != 'UPGR')
		{
			AIER("UpgIsDone : Invalid Upgrade ID");
			return 0;
		}

		return CNAT->UPGRADE[Upgrade->Index]->Done;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern "C" __declspec( dllexport )bool UpgIsRun(GAMEOBJ* Upgrade)
{
	if (AiIsRunNow)
	{
		if (Upgrade->Type != 'UPGR')
		{
			AIER("UpgIsDone : Invalid Upgrade ID");
			return 0;
		}
		int id = Upgrade->Index;

		return CNAT->UPGRADE[id]->Done || CNAT->UPGRADE[id]->IsDoing;
	}
	else
	{
		AI_Error();
		return 0;
	}
}

extern int* ResTBL;
extern int NInResTBL;

extern "C" __declspec( dllexport )void SetMinesBuildingRules(int* Table, int NElm)
{
	ResTBL = Table;
	NInResTBL = NElm;
}

extern "C" __declspec( dllexport )void SetMinesUpgradeRules(int* Table)
{
	if (!AiIsRunNow)
	{
		return;
	}

	Nation* Nat = CNAT;
	Nat->MU1G_PERCENT[0] = Table[0];
	Nat->MU1G_PERCENT[1] = Table[1];
	Nat->MU1G_PERCENT[2] = Table[2];

	Nat->MU1I_PERCENT[0] = Table[3];
	Nat->MU1I_PERCENT[1] = Table[4];
	Nat->MU1I_PERCENT[2] = Table[5];

	Nat->MU1C_PERCENT[0] = Table[6];
	Nat->MU1C_PERCENT[1] = Table[7];
	Nat->MU1C_PERCENT[2] = Table[8];
	//=============II==============//

	Nat->MU2G_PERCENT[0] = Table[9];
	Nat->MU2G_PERCENT[1] = Table[10];
	Nat->MU2G_PERCENT[2] = Table[11];

	Nat->MU2I_PERCENT[0] = Table[12];
	Nat->MU2I_PERCENT[1] = Table[13];
	Nat->MU2I_PERCENT[2] = Table[14];

	Nat->MU2C_PERCENT[0] = Table[15];
	Nat->MU2C_PERCENT[1] = Table[16];
	Nat->MU2C_PERCENT[2] = Table[17];
	//=============III==============//

	Nat->MU3G_PERCENT[0] = Table[18];
	Nat->MU3G_PERCENT[1] = Table[19];
	Nat->MU3G_PERCENT[2] = Table[20];

	Nat->MU3I_PERCENT[0] = Table[21];
	Nat->MU3I_PERCENT[1] = Table[22];
	Nat->MU3I_PERCENT[2] = Table[23];

	Nat->MU3C_PERCENT[0] = Table[24];
	Nat->MU3C_PERCENT[1] = Table[25];
	Nat->MU3C_PERCENT[2] = Table[26];
}

extern "C" __declspec( dllexport )bool FieldExist()
{
	return CCIT->FieldReady;
}

extern "C" __declspec( dllexport )int GetDifficulty()
{
	return CCIT->Difficulty;
}

extern "C" __declspec( dllexport )int GetStartRes()
{
	return CCIT->StartRes;
}

extern "C" __declspec( dllexport )int GetDiff(byte NI)
{
	return CITY[NI].Difficulty;
}

extern "C" __declspec( dllexport )int GetResOnMap()
{
	return CCIT->ResOnMap;
}

extern "C" __declspec( dllexport )int GetLandType()
{
	return CCIT->LandType;
}

extern "C" __declspec( dllexport )void SetStandartVictory()
{
	SCENINF.StandartVictory = 1;
}

extern "C" __declspec( dllexport )bool NationIsErased(byte Nat)
{
	if (Nat < 8)
	{
		Nat = NatRefTBL[Nat];
		int NMyPeasants = 0;
		int NMyCenters = 0;
		int MyMask = 1 << Nat;
		//memset(LockN,1,8);
		//for(int i=0;i<8;i++)if(NATIONS[i].VictState==1)LockN[i]=0;
		int NMyUnits = 0;
		//NThemUnits=0;
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && !(OB->Sdoxlo && !OB->Hidden))
			{
				byte USE = OB->newMons->Usage;
				if (USE == PeasantID)
				{
					if (OB->NNUM == Nat)NMyPeasants++;
					//else if(!(OB->NMask&MyMask))NThemPeasants++;
				}
				else if (USE == CenterID)
				{
					if (OB->NNUM == Nat)NMyCenters++;
					//else if(!(OB->NMask&MyMask))NThemCenters++;
				}
				if (!(OB->LockType || OB->NewBuilding || OB->Wall))
				{
					if (OB->NNUM == Nat)NMyUnits++;
					//else if(!(OB->NMask&MyMask))NThemUnits++;
				}
			}
		}
		if (NMyPeasants == 0 && NMyCenters == 0 && NMyUnits < 10)NMyUnits = 0;
		//if(NThemPeasants==0&&NThemCenters==0&&NThemUnits<10)NThemUnits=0;
		return NMyUnits == 0;
	}
	else return 1;
}

extern "C" __declspec( dllexport ) void AssignFormUnit(char* Name)
{
	GeneralObject** GOS = NATIONS[0].Mon;
	int N = NATIONS[0].NMon;
	for (int i = 0; i < N; i++)
		if (!strcmp(GOS[i]->MonsterID, Name))
		{
			CNAT->FormUnitID = i;
			return;
		}
	IntErr("AssignFormUnit : Unknown unit type : %s", Name);
	SCENINF.NErrors++;
}

extern "C" __declspec( dllexport ) void SetPlayerName(byte Nat, char* ID)
{
	for (int i = 0; i < 8; i++)
		if (PINFO[i].ColorID == Nat)
		{
			char cc[128];
			strcpy(cc, GetTextByID(ID));
			cc[31] = 0;
			strcpy(PINFO[i].name, cc);
			return;
		}
}

extern int MaxPeaceTime;
extern int PeaceTimeStage;

extern "C" __declspec( dllexport ) int GetMaxPeaceTime()
{
	return MaxPeaceTime / 60;
}

extern "C" __declspec( dllexport ) int GetPeaceTimeLeft()
{
	return PeaceTimeStage;
}

extern bool RecordMode;
extern byte PlayGameMode;
int PrevRand = -1;
extern word NPlayers;

extern "C" __declspec( dllexport ) int GetRandomIndex()
{
	if (PlayGameMode || RecordMode)
	{
		return 0;
	}

	if (NPlayers)
	{
		int RR = 0;
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB)
			{
				RR += (OB->RealX + OB->RealY) * int(OB->NNUM);
			}
		}

		RR += rando();
		RR >>= 3;
		return RR;
	}
	else
	{
		if (PrevRand == -1)PrevRand = (GetTickCount() / 100) & 32767;
		else PrevRand *= 177;
	}

	return PrevRand;
}

int AI_Registers[8][32];

extern "C" __declspec( dllexport ) void SetAIRegister(int Reg, int Val)
{
	if (Reg < 32 && CurAINation < 8)AI_Registers[CurAINation][Reg] = Val;
}

extern "C" __declspec( dllexport ) int GetAIRegister(int Reg)
{
	if (Reg < 32 && CurAINation < 8)return AI_Registers[CurAINation][Reg];
	else return 0;
}

extern int WasInGold[8];
extern int WasInIron[8];
extern int WasInCoal[8];

//Returns resource amount based on gathering speed
extern "C" __declspec( dllexport ) int GetExtraction(byte ResID)
{
	if (!AiIsRunNow)
	{
		return 0;
	}

	if (ResID == FoodID)
	{
		return CCIT->FoodSpeed;
	}
	else if (ResID == TreeID)
	{
		return CCIT->WoodSpeed;
	}
	else if (ResID == StoneID)
	{
		return CCIT->StoneSpeed;
	}
	else if (ResID == IronID)
	{
		return WasInIron[CNAT->NNUM];
	}
	else if (ResID == CoalID)
	{
		return WasInCoal[CNAT->NNUM];
	}
	else if (ResID == GoldID)
	{
		return WasInGold[CNAT->NNUM];
	}
	else return 0;
}

extern "C" __declspec( dllexport ) void SetDefenseState(byte State)
{
	if (AiIsRunNow)
	{
		CCIT->DefenceStage = 0 != State;
	}
}

//-----------------------------------------------------------------------//
void DosToWin(char* Str);

bool ReadWinString(GFILE* F, char* STR, int Max)
{
	STR[0] = 0;
	int cc = 0;
	//int z1=0;
	int nn = 0;
	while (!(cc == 0x0A || cc == EOF))
	{
		cc = Ggetch(F);
		if (Max > 2 && !(cc == 0x0A || cc == EOF))
		{
			STR[nn] = cc;
			nn++;
			Max--;
		}
	}
	STR[nn] = 0;
	return (cc != EOF) || STR[0];
	//DosToWin(STR);
}

extern int CurPalette;

void ErrM(char* s, char* s1)
{
	if (CurPalette == 2)LoadPalette("2\\agew_1.pal");
	else LoadPalette("0\\agew_1.pal");
	char cc2[200];
	sprintf(cc2, s, s1);
	MessageBox(hwnd, cc2, "LOADING FAILED...", MB_ICONWARNING | MB_OK);
	assert( false );
}

extern bool ProtectionMode;

void MissPack::LoadMissions()
{
	ProtectionMode = 1;
	GFILE* F = Gopen("Missions\\Missions.txt", "r");
	ProtectionMode = 0;
	if (F)
	{
		char MissID[256];
		char MissName[256];
		char DLLPath[256];
		char MapName[256];
		char MissDesc[256];
		bool MissLoad = false;
		do
		{
			MissLoad = false;
			ReadWinString(F, MissID, 256);
			ReadWinString(F, MissName, 256);
			ReadWinString(F, DLLPath, 256);
			ReadWinString(F, MapName, 256);
			ReadWinString(F, MissDesc, 256);
			if (MissID[0] && MissName[0] && DLLPath[0] && MapName[0] && MissDesc[0])
			{
				if (NMiss >= MaxMiss)
				{
					MaxMiss += 32;
					MISS = (SingleMission*)realloc(MISS, MaxMiss * sizeof SingleMission);
				}
				SingleMission* SM = MISS + NMiss;
				NMiss++;
				SM->ID = new char[strlen(MissID) + 1];
				strcpy(SM->ID, MissID);
				SM->Name = new char[strlen(MissName) + 1];
				strcpy(SM->Name, MissName);
				SM->DLLPath = new char[strlen(DLLPath) + 1];
				strcpy(SM->DLLPath, DLLPath);
				SM->MapName = new char[strlen(MapName) + 1];
				strcpy(SM->MapName, MapName);
				SM->Description = new char[strlen(MissDesc) + 1];
				strcpy(SM->Description, MissDesc);
				MissLoad = true;
				ReadWinString(F, MissDesc, 256);
				int z = sscanf(MissDesc, "%d", &SM->NIntro);
				if (z != 1)
				{
					ErrM("Invalid file Missiions\\Missions.txt");
				}
				SM->Intro = (char**)malloc(4 * SM->NIntro);
				for (int j = 0; j < SM->NIntro; j++)
				{
					ReadWinString(F, MissDesc, 256);
					SM->Intro[j] = new char[strlen(MissDesc) + 1];
					strcpy(SM->Intro[j], MissDesc);
				}
			}
		}
		while (MissLoad);
		Gclose(F);
	}
	else
	{
		MessageBox(NULL, "Could not open Missions\\Missions.txt", "Missions loading failed...", MB_TOPMOST);
	}

	ProtectionMode = 1;
	F = Gopen("Missions\\SingleMiss.txt", "r");
	ProtectionMode = 0;
	if (F)
	{
		int z = Gscanf(F, "%d", &MSMiss);
		if (z != 1)
		{
			ErrM("Invalid file Missions\\SingleMiss.txt");
			return;
		}
		char cc1[64];
		SingleMS = new int[MSMiss];
		for (int i = 0; i < MSMiss; i++)
		{
			Gscanf(F, "%s", cc1);
			SingleMS[i] = 0;
			for (int j = 0; j < NMiss; j++)
				if (!strcmp(MISS[j].ID, cc1))
				{
					SingleMS[i] = j;
					goto GG1;
				}
			ErrM("SingleMiss.txt: Unknown mission %s", cc1);
		GG1:;
		}
	}
	else
	{
		MessageBox(NULL, "Could not open Missions\\SingleMiss.txt", "Missions loading failed...", MB_TOPMOST);
	}
}

MissPack::MissPack()
{
	memset(this, 0, sizeof MissPack);
	CurrentMission = -1;
	LoadMissions();
}

MissPack::~MissPack()
{
	for (int i = 0; i < NMiss; i++)
	{
		free(MISS[i].Description);
		free(MISS[i].DLLPath);
		free(MISS[i].ID);
		free(MISS[i].MapName);
		free(MISS[i].Name);
		for (int j = 0; j < MISS[i].NIntro; j++)
		{
			free(MISS[i].Intro[j]);
		}
	}
	free(MISS);
	free(SingleMS);
}

MissPack MISSLIST;
CampaginPack CAMPAGINS;

void InvCamp()
{
	ErrM("Invalid file CAMPAIGNS.TXT");
}

void SavePlayerData(char* Name);

CampaginPack::CampaginPack()
{
	ProtectionMode = 1;
	GFILE* F = Gopen("Missions\\Campaigns.txt", "r");
	if (!F)F = Gopen("Missions\\Campagins.txt", "r");
	ProtectionMode = 0;
	if (F)
	{
		char cc[256];
		ReadWinString(F, cc, 255);
		int z = sscanf(cc, "%d", &NCamp);
		if (z != 1)InvCamp();
		SCamp = new SingleCampagin[NCamp];
		char CampMessage[256];
		char CampText[256];
		char CampBmp[256];
		for (int i = 0; i < NCamp; i++)
		{
			ReadWinString(F, CampMessage, 255);
			ReadWinString(F, CampText, 255);
			ReadWinString(F, CampBmp, 255);
			ReadWinString(F, cc, 255);
			if (cc[0] && CampMessage[0] && CampText[0] && CampBmp[0])
			{
				SCamp[i].CampMessage = new char[strlen(CampMessage) + 1];
				strcpy(SCamp[i].CampMessage, CampMessage);
				SCamp[i].CampText = new char[strlen(CampText) + 1];
				strcpy(SCamp[i].CampText, CampText);
				SCamp[i].CampBmp = new char[strlen(CampBmp) + 1];
				strcpy(SCamp[i].CampBmp, CampBmp);
				z = sscanf(cc, "%d", &SCamp[i].NMiss);
				if (z != 1)InvCamp();
				SCamp[i].Miss = new int[SCamp[i].NMiss];
				SCamp[i].OpenIndex = new DWORD[SCamp[i].NMiss];
				DWORD op0, op1, op2, op3;
				char ONOFF[32];
				char MISSID[64];
				for (int q = 0; q < SCamp[i].NMiss; q++)
				{
					int miss = -1;
					ReadWinString(F, cc, 255);
					z = sscanf(cc, "%s%s%d%d%d", MISSID, ONOFF, &op1, &op2, &op3);
					if (z == 5)
					{
						if (!(strcmp(ONOFF, "ON") && strcmp(ONOFF, "on")))op0 = 1;
						else op0 = 0;
						for (int p = 0; p < MISSLIST.NMiss; p++)if (!strcmp(MISSID, MISSLIST.MISS[p].ID))miss = p;
						if (miss == -1)
						{
							sprintf(cc, "CAMPAGINS.TXT: Uncnown mission %s", MISSID);
							ErrM(cc);
						}
						SCamp[i].OpenIndex[q] = op0 + (op1 << 8) + (op2 << 16) + (op3 << 24);
						SCamp[i].Miss[q] = miss;
					}
					else InvCamp();
				}
			}
			else InvCamp();
		}
		Gclose(F);
		SavePlayerData("standart player info");
	}
	else InvCamp();
}

CampaginPack::~CampaginPack()
{
	for (int i = 0; i < NCamp; i++)
	{
		free(SCamp[i].CampBmp);
		free(SCamp[i].CampMessage);
		free(SCamp[i].CampText);
		free(SCamp[i].Miss);
		free(SCamp[i].OpenIndex);
	}
}

void LoadAIFromDLL(byte Nat, char* Name)
{
	Nat = AssignTBL[Nat];
	Nation* NT = NATIONS + Nat;
	AiIsRunNow = true;
	CNAT = NATIONS + Nat;
	CCIT = CNAT->CITY;
	CurAINation = Nat;
	NT->hLibAI = LoadLibrary(Name);
	AiIsRunNow = false;
	if (NT->hLibAI)
	{
		NT->ProcessAIinDLL = (VoidProc*)GetProcAddress(NT->hLibAI, "ProcessAI");
		if (!NT->ProcessAIinDLL)
		{
			char cc[128];
			sprintf(cc, "%s : Could not load <void ProcessAI()>", Name);
			MessageBox(NULL, cc, "AI loadind from DLL", MB_TOPMOST);
			assert( 0 );
		}
		else
		{
			VoidProc* INITAI = (VoidProc*)GetProcAddress(NT->hLibAI, "InitAI");
			if (!INITAI)
			{
				char cc[128];
				sprintf(cc, "%s : Could not load <void InitAI()>", Name);
				MessageBox(NULL, cc, "AI loadind from DLL", MB_TOPMOST);
				assert( 0 );
			}
			AiIsRunNow = true;
			INITAI();
			AiIsRunNow = false;
			NT->DLLName = Name;
		}
	}
	else
	{
#ifndef STARFORCE
		char cc[128];
		sprintf(cc, "Could not load %s", Name);
		MessageBox(NULL, cc, "AI loadind from DLL", MB_TOPMOST);
		assert( 0 );
#endif
	}
}

extern char PlName[64];

void SavePlayerData(char* Name)
{
	char cc[200];
	sprintf(cc, "Players\\%s.txt", Name);
	GFILE* F = Gopen(cc, "w");
	if (F)
	{
		Gprintf(F, "%d ", CAMPAGINS.NCamp);
		for (int i = 0; i < CAMPAGINS.NCamp; i++)
		{
			Gprintf(F, "%d ", CAMPAGINS.SCamp[i].NMiss);
			for (int j = 0; j < CAMPAGINS.SCamp[i].NMiss; j++)
			{
				Gprintf(F, "%d ", CAMPAGINS.SCamp[i].OpenIndex[j]);
			}
		}
		Gclose(F);
	}
}

void SavePlayerData()
{
	SavePlayerData(PlName);
}

bool PLOAD = 0;

void SFLB_LoadPDATA(char* Name)
{
	PLOAD = 0;
	char cc[200];
	sprintf(cc, "Players\\%s.txt", Name);
	GFILE* F = Gopen(cc, "r");
	if (F)
	{
		int x1, x2;
		Gscanf(F, "%d ", &x1);
		if (x1 != CAMPAGINS.NCamp)
		{
			Gclose(F);
			return;
		}

		for (int i = 0; i < x1; i++)
		{
			Gscanf(F, "%d ", &x2);
			if (x2 != CAMPAGINS.SCamp[i].NMiss)
			{
				Gclose(F);
				return;
			}

			for (int j = 0; j < x2; j++)
			{
				Gscanf(F, "%d ", &CAMPAGINS.SCamp[i].OpenIndex[j]);
			}
		}

		Gclose(F);
		PLOAD = 1;
	}
}

void SFLB_LoadPlayerData()
{
	SFLB_LoadPDATA(PlName);
	if (!PLOAD)
	{
		SFLB_LoadPDATA("standart player info");
	}
}

WarPack WARS;

void InvBatt()
{
	ErrM("Invalid file: History_battl\\Battles.txt");
}

void InvWar()
{
	ErrM("Invalid file: History_battl\\Wars.txt");
}

void NLine(GFILE* f);

WarPack::WarPack()
{
	char ccc[200];
	GFILE* f = Gopen("History_battl\\Battles.txt", "r");
	if (f)
	{
		int z = Gscanf(f, "%d", &NBattles);
		if (z != 1)InvBatt();

		Battles = new OneBattle[NBattles];
		for (int i = 0; i < NBattles; i++)
		{
			OneBattle* BTL = Battles + i;

			z = Gscanf(f, "%s", ccc);
			if (z != 1)InvBatt();
			BTL->ID = new char[strlen(ccc) + 1];
			strcpy(BTL->ID, ccc);

			z = Gscanf(f, "%s", ccc);
			if (z != 1)InvBatt();
			BTL->Map = new char[strlen(ccc) + 1];
			strcpy(BTL->Map, ccc);

			z = Gscanf(f, "%s", ccc);
			if (z != 1)InvBatt();
			BTL->Text = new char[strlen(ccc) + 1];
			strcpy(BTL->Text, ccc);

			z = Gscanf(f, "%s", ccc);
			if (z != 1)InvBatt();
			BTL->Brief = new char[strlen(ccc) + 1];
			strcpy(BTL->Brief, ccc);

			z = Gscanf(f, "%s", ccc);
			if (z != 1)InvBatt();
			BTL->BigMap = new char[strlen(ccc) + 1];
			strcpy(BTL->BigMap, ccc);

			z = Gscanf(f, "%s", ccc);
			if (z != 1)InvBatt();
			BTL->MiniMap = new char[strlen(ccc) + 1];
			strcpy(BTL->MiniMap, ccc);

			NLine(f);
			ReadWinString(f, ccc, 200);
			BTL->BigHeader = new char[strlen(ccc) + 1];
			strcpy(BTL->BigHeader, ccc);

			ReadWinString(f, ccc, 200);
			BTL->Date = new char[strlen(ccc) + 1];
			strcpy(BTL->Date, ccc);

			ReadWinString(f, ccc, 200);
			BTL->SmallHeader = new char[strlen(ccc) + 1];
			strcpy(BTL->SmallHeader, ccc);

			ReadWinString(f, ccc, 200);
			BTL->RedUnits = new char[strlen(ccc) + 1];
			strcpy(BTL->RedUnits, ccc);

			ReadWinString(f, ccc, 200);
			BTL->BlueUnits = new char[strlen(ccc) + 1];
			strcpy(BTL->BlueUnits, ccc);

			z = Gscanf(f, "%d", &Battles[i].NHints);
			if (z != 1)InvBatt();

			int NH = BTL->NHints;

			BTL->Hints = (char**)malloc(4 * NH);
			BTL->Coor = new int[2 * NH];
			for (int j = 0; j < NH; j++)
			{
				int x, y;
				z = Gscanf(f, "%d%d", &x, &y);
				if (z != 2)InvBatt();
				BTL->Coor[j + j] = x;
				BTL->Coor[j + j + 1] = y;
				NLine(f);
				ReadWinString(f, ccc, 200);
				BTL->Hints[j] = new char[strlen(ccc) + 1];
				strcpy(BTL->Hints[j], ccc);
			}
		}
		Gclose(f);
	}
	else
	{
		ErrM("Could not open History_battl\\Battles.txt");
	}
	//reading wars
	f = Gopen("History_battl\\Wars.txt", "r");
	if (f)
	{
		int z = Gscanf(f, "%d", &NWars);
		if (z != 1)InvWar();
		Wars = new OneWar[NWars];
		NLine(f);
		for (int i = 0; i < NWars; i++)
		{
			OneWar* WAR = Wars + i;
			ReadWinString(f, ccc, 200);
			WAR->Name = new char[strlen(ccc) + 1];
			strcpy(WAR->Name, ccc);

			ReadWinString(f, ccc, 200);
			WAR->Date = new char[strlen(ccc) + 1];
			strcpy(WAR->Date, ccc);

			ReadWinString(f, ccc, 200);
			WAR->Text = new char[strlen(ccc) + 1];
			strcpy(WAR->Text, ccc);

			z = Gscanf(f, "%d", &WAR->NBatles);
			if (z != 1)InvWar();
			WAR->BattleList = new int[WAR->NBatles];
			for (int j = 0; j < WAR->NBatles; j++)
			{
				z = Gscanf(f, "%s", ccc);
				if (z != 1)InvWar();
				int k;
				for (k = 0; k < NBattles && strcmp(ccc, Battles[k].ID); k++);
				if (k < NBattles)
				{
					WAR->BattleList[j] = k;
				}
				else
				{
					char cc1[150];
					sprintf(cc1, "Wars.txt: Unknown battle ID: %s", ccc);
					ErrM(cc1);
					WAR->BattleList[j] = 0;
				}
			}
			NLine(f);
		}
	}
	else
	{
		ErrM("Could not open History_battl\\Wars.txt");
	}
}

WarPack::~WarPack()
{
	for (int i = 0; i < NBattles; i++)
	{
		free(Battles[i].BigHeader);
		free(Battles[i].BigMap);
		free(Battles[i].Brief);
		free(Battles[i].Coor);
		free(Battles[i].Date);
		free(Battles[i].ID);
		free(Battles[i].Map);
		free(Battles[i].MiniMap);
		for (int j = 0; j < Battles[i].NHints; j++)free(Battles[i].Hints[j]);
		free(Battles[i].Hints);
		free(Battles[i].Text);
	}
	free(Battles);
	for (int i = 0; i < NWars; i++)
	{
		free(Wars[i].BattleList);
		free(Wars[i].Date);
		free(Wars[i].Name);
		free(Wars[i].Text);
	}
	free(Wars);
}

extern "C" __declspec( dllexport ) void MissErrorMessage(char* Header, char* Message)
{
	MessageBox(hwnd, Message, Header, 0);
}

//Is the map editor running?
bool RUNMAPEDITOR = 0;

//Is the user playing a mission?
bool RUNUSERMISSION = 0;

char USERMISSPATH[128];

extern "C" __declspec( dllexport ) void RunMapeditor(char* path)
{
	RUNMAPEDITOR = 1;
	strcpy(USERMISSPATH, path);
}

extern "C" __declspec( dllexport ) void RunUserMission(char* path)
{
	RUNUSERMISSION = 1;
	strcpy(USERMISSPATH, path);
}

//------------------------NEW! FUNCTIONS----------------------//
/*
ReportFn* ORDLIST[OrdCount]={
	&BuildObjLink,
	&ProduceObjLink,
	&AttackObjLink,
	&TakeResLink,
	&PerformUpgradeLink,
	&GoToMineLink,
	&LeaveMineLink,
	&TakeResourceFromSpriteLink,
	&NewMonsterSendToLink,
	&DeleteBlockLink,
	&SetUnlimitedLink,
	&ClearUnlimitedLink,
	&WaterNewMonsterSendToLink,
	&AbsorbObjectLink,
	&LeaveShipLink,
	&BuildWallLink,
	&WaterAttackLink,
	&LeaveTransportLink,
	&GoToTransportLink,
	&FishingLink,
	&CreateGatesLink,
	&NewMonsterSmartSendToLink,
	&AI_AttackPointLink,
	&GrenaderSupermanLink,
	&GotoFinePositionLink,
	&NewMonsterPreciseSendToLink,
	&RotUnitLink,
	&PatrolLink,
	&RotateShipAndDieLink,
	&SearchArmyLink,
	&NewAttackPointLink,
	&MakeOneShotLink
}
*/
int GetOrderKind(ReportFn* RF);

struct UnitExCaps
{
	int Direction;
	int DestX;
	int DestY;
	int Type;
	char* UnitID;
	int NI;
	int StandTime;
	int OrderType;
};

extern "C" __declspec( dllexport ) bool GetUnitExCaps(int Index, UnitExCaps* CAPS, bool NeedOrderType)
{
	if (Index >= 0 && Index <= MAXOBJECT)
	{
		OneObject* OB = Group[Index];
		if (OB && !OB->Sdoxlo)
		{
			CAPS->DestX = OB->DestX >> 4;
			CAPS->DestY = OB->DestY >> 4;
			CAPS->NI = OB->NNUM;
			CAPS->Direction = OB->RealDir;
			CAPS->Type = OB->NIndex;
			CAPS->UnitID = OB->Ref.General->MonsterID;
			CAPS->StandTime = OB->StandTime;
			if (NeedOrderType)
			{
				if (OB->LocalOrder)
				{
					CAPS->OrderType = GetOrderKind(OB->LocalOrder->DoLink);
				}
				else
				{
					CAPS->OrderType = -1;
				}
			}
			else
			{
				CAPS->OrderType = -1;
			}
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return false;
	}
}

extern "C" __declspec( dllexport ) int InsertUnitToGroup(GAMEOBJ* Src, GAMEOBJ* Dst, int Index)
{
	if ((!Dst) || Dst->Type != 'UNIT')
	{
		return -1;
	}
	if (Src)
	{
		if (!(Src->Type != 'UNIT'))
		{
			return -1;
		}
		if (Src->Index < SCENINF.NUGRP)
		{
			UnitsGroup* UG = SCENINF.UGRP + Src->Index;
			if (Index < UG->N)
			{
				int Id = UG->IDS[Index];
				int Sn = UG->SNS[Index];
				UnitsGroup* DG = SCENINF.UGRP + Dst->Index;
				DG->IDS = (word*)realloc(DG->IDS, (DG->N + 1) << 1);
				DG->SNS = (word*)realloc(DG->SNS, (DG->N + 1) << 1);
				DG->IDS[DG->N] = Id;
				DG->SNS[DG->N] = Sn;
				DG->N++;
				return DG->N - 1;
			}
		}
	}
	else
	{
		if (Index >= 0 && Index <= MAXOBJECT)
		{
			OneObject* OB = Group[Index];
			if (OB && !OB->Sdoxlo)
			{
				UnitsGroup* DG = SCENINF.UGRP + Dst->Index;
				DG->IDS = (word*)realloc(DG->IDS, (DG->N + 1) << 1);
				DG->SNS = (word*)realloc(DG->SNS, (DG->N + 1) << 1);
				DG->IDS[DG->N] = OB->Index;
				DG->SNS[DG->N] = OB->Serial;
				DG->N++;
				return DG->N - 1;
			}
		}
	}
	return -1;
}

extern "C" __declspec( dllexport ) void RemoveUnitFromGroup(GAMEOBJ* Src, int Index)
{
	if (Src->Type != 'UNIT')return;
	if (Src->Index >= SCENINF.NUGRP)return;
	UnitsGroup* SG = SCENINF.UGRP + Src->Index;
	if (Index < SG->N)
	{
		if (Index < SG->N - 1)
		{
			memcpy(SG->IDS + Index, SG->IDS + Index + 1, (SG->N - Index - 1) << 1);
			memcpy(SG->SNS + Index, SG->SNS + Index + 1, (SG->N - Index - 1) << 1);
		}
		SG->N--;
	}
}

extern "C" __declspec( dllexport ) bool GetZoneCoor(GAMEOBJ* Zone, int* x, int* y)
{
	if (Zone->Type == 'ZONE' && Zone->Index < SCENINF.NZGRP)
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
		*x = AZ->x;
		*y = AZ->y;
		return true;
	}
	else if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
	{
		*x = Zone->Index;
		*y = Zone->Serial;
		return true;
	}
	else
	{
		return false;
	}
}

void RM_Load(char* Name, int x, int y);
extern bool ImmVis;

extern "C" __declspec( dllexport ) void PastePiece(GAMEOBJ* Zone, char* Name)
{
	int x, y;
	if (GetZoneCoor(Zone, &x, &y))
	{
		ImmVis = true;
		RM_Load(Name, x, y);
		ImmVis = false;
	}
}

extern "C" __declspec( dllexport ) void SelectBuildingsInZone(byte NI, GAMEOBJ* Zone, GAMEOBJ* UTP, bool Add)
{
	if (!Add)
	{
		ClearSelection(NI);
	}
	int x, y, R;
	if (Zone->Type == 'ZONE' && Zone->Index < SCENINF.NZGRP)
	{
		ActiveZone* AZ = AZones + SCENINF.ZGRP[Zone->Index].ZoneID[0];
		x = AZ->x;
		y = AZ->y;
		R = AZ->R;
	}
	else
	{
		if ((Zone->Type & 0xFF000000) == ('@   ' - 0x202020))
		{
			x = Zone->Index;
			y = Zone->Serial;
			R = Zone->Type & 0x00FFFFFF;
		}
		else
		{
			return;
		}
	}

	if (UTP)
	{
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && OB->NewBuilding && OB->NIndex == UTP->Index && OB->NNUM == NI && !OB->Sdoxlo)
			{
				int r = Norma((OB->RealX >> 4) - x, (OB->RealY >> 4) - y);
				if (r < R)
				{
					AddUnitToSelected(NI, OB);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && OB->NewBuilding && OB->NNUM == NI && !OB->Sdoxlo)
			{
				int r = Norma((OB->RealX >> 4) - x, (OB->RealY >> 4) - y);
				if (r < R)
				{
					AddUnitToSelected(NI, OB);
				}
			}
		}
	}
}
