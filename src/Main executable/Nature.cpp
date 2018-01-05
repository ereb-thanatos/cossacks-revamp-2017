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

#include <assert.h>

#include "3DSurf.h"
#include "Cdirsnd.h"
#include "GSound.h"
#include "NewMon.h"
#include "Nature.h"
#include <crtdbg.h>
#include "ConstStr.h"
#include "GP_Draw.h"
#include "DrawForm.h"
#include "Fonts.h"
//void FindEmptyPoint(byte* x,byte* y);
byte rtmap[256][256]; //resource type map
byte ramap[256][256]; //resource amount map
int URESRC[8][8];
int RESADD[8][8];
char* ResNames[8];
const int drrn[9] = {7, 6, 5, 0, 0, 4, 1, 2, 3};
extern int NWeaponIDS;
extern char* WeaponIDS[32];
byte WeaponFlags[32];
short WeaponIcn[32];
short ProtectIcn[32];
char* UnitKindID[64];
int NUnitKinds;
char* MediaID[64];
int NMedias;
int SGenIcon = 6;
int FREE_ORDER_ICON = -1;
int STAND_GROUND_ICON = -1;
int NOT_STAND_GROUND_ICON = -1;
int SET_DEST_ICON = -1;
int SDS_X = 8;
int SDS_Y = 2;
int STAND_GRUND_ORDER_ICON = -1;
int ORDERS_LIST_ICON = -1;
int EATT_X = 4;
int EATT_Y = 2;
int EATT_Icon = -1;
int DATT_X = 5;
int DATT_Y = 2;
int DATT_Icon = -1;
int GATT_X = 8;
int GATT_Y = 2;
int GATT_Icon = -1;
int PATR_X = 9;
int PATR_Y = 2;
int PATR_Icon = -1;
int STOP_ICON = -1;
int UNLOAD_ICON = -1;
int IDLE_PICON = -1;
int IDLE_MICON = -1;
int IDLE_PX = 2;
int IDLE_PY = 0;
int IDLE_MX = 3;
int IDLE_MY = 0;
int GOAT_Icon = -1;
int GOAT_PX = 3;
int GOAT_PY = 2;
int HISTORY_Icon = -1;
int FILLFORM_ICON = -1;
int DOGUARD_ICON = -1;
int NOGUARD_ICON = -1;
int PATROL_ICON = -1;
int ATTGR_ICON = -1;
int AttGrPos = 12;
int ADDRANGE = 200;
int ADDPAUSE = 150;
int UNGROUP_ICON = -1;
int DOGROUP_ICON = -1;
int GroupIconPos = 12;
extern int DecOrdID;
extern int IncOrdID;
//------Nature sounds&alarm------
short AlarmSoundID = -1;

short ForestFreq;
short* ForestSounds;
byte NForestSounds = 0;

short FieldFreq;
short* FieldSounds;
byte NFieldSounds = 0;

short SeaFreq;
short* SeaSounds;
byte NSeaSounds = 0;
//-------------------------------
ResDiscr::ResDiscr()
{
	Enabled = false;
	Name[0] = 0;
	ResIDS[0] = 0;
};
ResDiscr RDS[8];

void Errr(LPCSTR s)
{
	MessageBox(hwnd, s, "Nature loading failed...", MB_ICONWARNING | MB_OK);
	assert( false );
};

int GetUnitKind(char* Name)
{
	for (int i = 0; i < NUnitKinds; i++)if (!strcmp(UnitKindID[i], Name))return i;
	return -1;
};

int GetExMedia(char* Name)
{
	for (int i = 0; i < NMedias; i++)if (!strcmp(MediaID[i], Name))return i;
	return -1;
};
int UnitsPerFarm;
int ResPerUnit;
int EatenRes;
int GetResByName(char* gy);
FormGroupDescription FormGrp;
void ErrM(char* s);
extern char* SoundID[MaxSnd];
extern word NSounds;
OrderClassDescription OrderDesc[16];
int NOClasses = 0;
OrderDescription ElementaryOrders[128];
int NEOrders = 0;
int SearchStr(char** Res, char* s, int count);
int GetIconByName(char* Name);
void LoadOrders();
char* GetTextByID(char* ID);
extern bool ProtectionMode;

void LoadRDS()
{
	for (int p = 0; p > 8; p++)
	{
		for (int q = 0; q > 8; q++)
		{
			SetXRESRC( p, q, 0 );
		};
	};
	memset(&RESADD[0][0], 0, 64 * 4);
	for (int h = 0; h < 8; h++)
	{
		SetXRESRC( h, 1, 5000 );
	};

	ProtectionMode = 1;
	GFILE* f = Gopen("nres.dat", "r");
	ProtectionMode = 0;

	if (!int(f))
	{
		Errr("Could not find NRES.DAT");
		return;
	};
	NOClasses = 0;
	NWeaponIDS = 0;
	NUnitKinds = 0;
	NMedias = 0;
	bool eoff = false;
	char s1[128];
	char s2[128];
	char s3[128];
	int z0, z1, z2, z3, z5, z6, z7;
	int mode = 0;
	z5 = 0;
	char sss[64];
	char* ccx;
	do
	{
		Gscanf(f, "%s", s1);
		if (!strcmp(s1, "[RESTYPES]"))mode = 1;
		else if (!strcmp(s1, "[WEAPONS]"))mode = 2;
		else if (!strcmp(s1, "[UNITKINDS]"))mode = 3;
		else if (!strcmp(s1, "[MEDIA]"))mode = 4;
		else if (!strcmp(s1, "[CONST]"))mode = 5;
		else if (!strcmp(s1, "[SOUNDS]"))mode = 6;
		else if (!strcmp(s1, "[ORDERICONS]"))mode = 7;
		else if (!strcmp(s1, "[END]"))mode = 255;
		else
		{
			switch (mode)
			{
			case 1: //RESTYPES
				z6 = Gscanf(f, "%d%d", &z0, &z1);
				z7 = Gscanf(f, "%s", RDS[z0].ResIDS);
				if (z6 + z7 != 3)
				{
					sprintf(sss, "NRES.DAT:[RESTYPE]:Invalid parameters.");
					Errr(sss);
				};
				for (z2 = 0; z2 < 8; z2++)SetXRESRC( z2, z0, z1 );
				RDS[z0].Enabled = true;
				ccx = GetTextByID(s1);
				strcpy(RDS[z0].Name, ccx);
				break;
			case 2: //WEAPONS
				if (!strcmp(s1, "#GENSHIELD"))
				{
					z1 = Gscanf(f, "%d", &z2);
					SGenIcon = z2;
				}
				else
				{
					char ccc[16];
					z1 = Gscanf(f, "%d%d%s", &z2, &z3, ccc);
					WeaponIcn[NWeaponIDS] = z2;
					ProtectIcn[NWeaponIDS] = z3;
					WeaponIDS[NWeaponIDS] = new char[strlen(s1) + 1];
					strcpy(WeaponIDS[NWeaponIDS], s1);
					WeaponFlags[NWeaponIDS] = 0;
					if (strchr(ccc, 'H'))WeaponFlags[NWeaponIDS] |= 1; //Height-dependent
					if (strchr(ccc, 'R'))WeaponFlags[NWeaponIDS] |= 2; //random(1/25) killing
					if (strchr(ccc, 'O'))WeaponFlags[NWeaponIDS] |= 4; //additional damage in formation
					NWeaponIDS++;
				};
				break;
			case 3: //UNITKIND
				UnitKindID[NUnitKinds] = new char[strlen(s1) + 1];
				strcpy(UnitKindID[NUnitKinds], s1);
				NUnitKinds++;
				break;
			case 4:
				MediaID[NMedias] = new char[strlen(s1) + 1];
				strcpy(MediaID[NMedias], s1);
				NMedias++;
				break;
			case 5:
				if (!strcmp(s1, "UNITS/FARM"))
				{
					Gscanf(f, "%d", &UnitsPerFarm);
				}
				else if (!strcmp(s1, "RES/UNIT"))
				{
					Gscanf(f, "%s%d", s2, &ResPerUnit);
					EatenRes = GetResByName(s2);
					if (EatenRes < 0 || EatenRes > 8)
					{
						ErrM("NRES.DAT: Invalid resource value for RES/UNIT");
					};
				}
				else if (!strcmp(s1, "FREE_ORDER"))
				{
					char c[64];
					Gscanf(f, "%s", c);
					FREE_ORDER_ICON = GetIconByName(c);
				}
				else if (!strcmp(s1, "STAND_GROUND"))
				{
					char c[64];
					Gscanf(f, "%s", c);
					STAND_GROUND_ICON = GetIconByName(c);
				}
				else if (!strcmp(s1, "NOT_STAND_GROUND"))
				{
					char c[64];
					Gscanf(f, "%s", c);
					NOT_STAND_GROUND_ICON = GetIconByName(c);
				}
				else if (!strcmp(s1, "ORDERS_LIST"))
				{
					char c[64];
					Gscanf(f, "%s", c);
					ORDERS_LIST_ICON = GetIconByName(c);
				}
				else if (!strcmp(s1, "STAND_GROUND_ORDER"))
				{
					char c[64];
					Gscanf(f, "%s", c);
					STAND_GRUND_ORDER_ICON = GetIconByName(c);
				}
				else if (!strcmp(s1, "SET_DEST"))
				{
					char c[64];
					char cc[128];
					Gscanf(f, "%d%d%s%s", &SDS_X, &SDS_Y, c, cc);
					SET_DEST_ICON = GetIconByName(c);
				}
				else if (!strcmp(s1, "ENABLE_ATTACK"))
				{
					char cc[128];
					Gscanf(f, "%d%d%s", &EATT_X, &EATT_Y, cc);
					EATT_Icon = GetIconByName(cc);
				}
				else if (!strcmp(s1, "DISABLE_ATTACK"))
				{
					char cc[128];
					Gscanf(f, "%d%d%s", &DATT_X, &DATT_Y, cc);
					DATT_Icon = GetIconByName(cc);
				}
				else
					/*
									if(!strcmp(s1,"PATROL")){
										char cc[128];
										Gscanf(f,"%s",&PATR_X,&PATR_Y,cc);
										PATR_Icon=GetIconByName(cc);
									}else
					*/

					if (!strcmp(s1, "GOATTACK"))
					{
						char cc[128];
						Gscanf(f, "%d%d%s", &GATT_X, &GATT_Y, cc);
						GATT_Icon = GetIconByName(cc);
					}
					else if (!strcmp(s1, "STOP_ICON"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						STOP_ICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "UNLOAD_ICON"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						UNLOAD_ICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "IDLE_PEASANT"))
					{
						char cc[128];
						Gscanf(f, "%d%d%s", &IDLE_PX, &IDLE_PY, cc);
						IDLE_PICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "IDLE_MINE"))
					{
						char cc[128];
						Gscanf(f, "%d%d%s", &IDLE_MX, &IDLE_MY, cc);
						IDLE_MICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "GO&ATTACK"))
					{
						char cc[128];
						Gscanf(f, "%d%d%s", &GOAT_PX, &GOAT_PY, cc);
						GOAT_Icon = GetIconByName(cc);
					}
					else if (!strcmp(s1, "HISTORY"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						HISTORY_Icon = GetIconByName(cc);
					}
					else if (!strcmp(s1, "FILLFORM"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						FILLFORM_ICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "DECFORM"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						DecOrdID = GetIconByName(cc);
					}
					else if (!strcmp(s1, "INCFORM"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						IncOrdID = GetIconByName(cc);
					}
					else if (!strcmp(s1, "ADDRANGE"))
					{
						Gscanf(f, "%d", &ADDRANGE);
					}
					else if (!strcmp(s1, "ADDPAUSE"))
					{
						Gscanf(f, "%d", &ADDPAUSE);
					}
					else if (!strcmp(s1, "DOGUARD"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						DOGUARD_ICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "NOGUARD"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						NOGUARD_ICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "PATROL"))
					{
						char cc[128];
						Gscanf(f, "%s", cc);
						PATROL_ICON = GetIconByName(cc);
					}
					else if (!strcmp(s1, "UN?GROUP"))
					{
						char cc[128];
						char cc1[128];
						Gscanf(f, "%d%s%s", &GroupIconPos, cc, cc1);
						UNGROUP_ICON = GetIconByName(cc);
						DOGROUP_ICON = GetIconByName(cc1);
					}
					else if (!strcmp(s1, "ATTACKGROUND"))
					{
						char cc[128];
						Gscanf(f, "%s%d", cc, &AttGrPos);
						ATTGR_ICON = GetIconByName(cc);
					}
					else
					{
						char c[128];
						sprintf(c, "nres.dat:[CONST]:Unknown constant: %s", s1);
						ErrM(c);
					};
				break;
			case 6:
				if (!strcmp(s1, "ALARM"))
				{
					Gscanf(f, "%s", s2);
					AlarmSoundID = SearchStr(SoundID, s2, NSounds);
					if (AlarmSoundID == -1)
					{
						ErrM("NRES.DAT: Invalid sound for ALARM");
					};
				}
				else if (!strcmp(s1, "FOREST"))
				{
					z2 = Gscanf(f, "%d%d", &z0, &z1);
					if (z2 != 2)ErrM("NRES.DAT: Invalid parameters for FOREST");
					ForestFreq = z0;
					NForestSounds = z1;
					ForestSounds = new short[z1];
					for (z2 = 0; z2 < z1; z2++)
					{
						z3 = Gscanf(f, "%s", s2);
						z3 = SearchStr(SoundID, s2, NSounds);
						if (z3 == -1)
						{
							sprintf(s3, "NRES.DAT-:Unknown sound %s for FOREST", s2);
							ErrM(s3);
						};
						ForestSounds[z2] = z3;
					};
				}
				else if (!strcmp(s1, "FIELD"))
				{
					z2 = Gscanf(f, "%d%d", &z0, &z1);
					if (z2 != 2)ErrM("NRES.DAT: Invalid parameters for FOREST");
					FieldFreq = z0;
					NFieldSounds = z1;
					FieldSounds = new short[z1];
					for (z2 = 0; z2 < z1; z2++)
					{
						z3 = Gscanf(f, "%s", s2);
						z3 = SearchStr(SoundID, s2, NSounds);
						if (z3 == -1)
						{
							sprintf(s3, "NRES.DAT:Unknown sound %s for FIELD", s2);
							ErrM(s3);
						};
						FieldSounds[z2] = z3;
					};
				}
				else if (!strcmp(s1, "SEA"))
				{
					z2 = Gscanf(f, "%d%d", &z0, &z1);
					if (z2 != 2)ErrM("NRES.DAT: Invalid parameters for SEA");
					SeaFreq = z0;
					NSeaSounds = z1;
					SeaSounds = new short[z1];
					for (z2 = 0; z2 < z1; z2++)
					{
						z3 = Gscanf(f, "%s", s2);
						z3 = SearchStr(SoundID, s2, NSounds);
						if (z3 == -1)
						{
							sprintf(s3, "NRES.DAT:Unknown sound %s for SEA", s2);
							ErrM(s3);
						};
						SeaSounds[z2] = z3;
					};
				}
				else
				{
					ErrM("NRES.DAT:Unknown command for [SOUNDS].");
				};
				break;
			case 7: //[ORDERICONS]
				Gscanf(f, "%d%d%s%s", &z2, &z3, s2, s3);
				if (NOClasses >= 16)
				{
					ErrM("NRES.DAT:[ORDERICONS]:Too many fundamental orders.");
				}
				else
				{
					OrderClassDescription* ODE = OrderDesc + NOClasses;
					NOClasses++;
					ODE->ID = new char[strlen(s1) + 1];
					strcpy(ODE->ID, s1);
					ODE->Message = GetTextByID(s3);
					if (!strcmp(s3, ODE->Message))
					{
						sprintf(s1, "Text not found:%s", s3);
						ErrM(s1);
					};
					//strcpy(ODE->Message,s3);
					ODE->IconPos = z3 * 12 + z2;
					ODE->IconID = GetIconByName(s2);
				};
				break;
			};
		};
	}
	while (mode != 255);
	Gclose(f);
	LoadOrders();
	//assert(_CrtCheckMemory());
};

int GetResID(char* Name)
{
	for (int i = 0; i < 8; i++)
	{
		if (RDS[i].Enabled)
		{
			if (!strcmp(RDS[i].ResIDS, Name))return i;
		};
	};
	return -1;
};
int xit[6] = {126, 531, 382, 257, 663, 793};
extern int RealLx;

bool AddUnitsToCash(byte NI, word ID);
void ClearUniCash();
void GetCorrectMoney(byte NI, int* MONEY);
extern byte PlayGameMode;

void ShowRMap()
{
	if (PlayGameMode == 1)return;
	int x = smapx + smaplx * 32 - 140;
	if (MiniMode)x = smapx + (smaplx << 4) - 140;
	int y = smapy;
	char gg[100];
	int x0 = (RealLx - 800) >> 1;
	GPS.ShowGP(x0, 0, BordGP, 16, MyNation);
	int N = (RealLx - 800) / 224;
	if ((RealLx - 800) % 112)N++;
	for (int i = 0; i < N; i++)
	{
		GPS.ShowGP((RealLx >> 1) - 400 - 112 - i * 112, 0, BordGP, 15, MyNation);
		GPS.ShowGP((RealLx >> 1) + 400 + i * 112, 0, BordGP, 15, MyNation);
	};
	int MON[8];

	GetCorrectMoney(NatRefTBL[MyNation], MON);
	for (int i = 0; i < 6; i++)
	{
		sprintf(gg, "%d", MON[i]);
		ShowString(x0 + xit[i] - GetRLCStrWidth(gg, &SmallWhiteFont), 3 + 1, gg, &SmallWhiteFont);
	};
	return;
};

//------------------Nature----------------
int srando();
extern byte* WaterDeep;
extern int FieldDelay;
void AddEffectV(int x, int y, int vx, int id);

void ProcessNature()
{
	//search for the sea;
	for (int i = 0; i < 10; i++)
	{
		int x = mapx + ((int(srando()) * smaplx) >> 15);
		int y = mapy + ((int(srando()) * smaply) >> 15);
		if (x > 0 && x < (MaxWX) && y > 0 && y < (MAPSY >> 1))
		{
			if (WaterDeep[x + y * (VAL_MAPSX >> 1)] > 128)
			{
				//play sea sounds
				if (srando() < SeaFreq && NSeaSounds)
				{
					AddEffectV(x << 5, y << 4, (srando() - 16384) / 100, SeaSounds[(int(srando()) * NSeaSounds) >> 15]);
				};
				return;
			};
		};
	};
	if (FieldDelay)return;
	if (srando() < FieldFreq && NFieldSounds)
	{
		AddEffect((mapx << 5) + ((int(srando()) * smaplx) >> 10), (mapy << 4) + ((int(srando()) * smaply) >> 11),
		          FieldSounds[(int(srando()) * NFieldSounds) >> 15]);
	};
};

OrderClassDescription::OrderClassDescription()
{
	ID = NULL;
	Message = NULL;
};

OrderClassDescription::~OrderClassDescription()
{
	if (ID)free(ID);
	//if(Message)free(Message);
};

OrderDescription::OrderDescription()
{
	ID = NULL;
	NLines = 0;
	Lines = NULL;
	LineNU = NULL;
	NCom = 0;
	ComX = NULL;
	ComY = NULL;
};

OrderDescription::~OrderDescription()
{
	if (ID)free(ID);
	if (NLines)
	{
		for (int i = 0; i < NLines; i++)
		{
			if (Lines[i])free(Lines[i]);
		};
		free(Lines);
		free(LineNU);
	};
	if (NCom)
	{
		free(ComX);
		free(ComY);
	};
};

bool ReadFLine(GFILE* f, char* c)
{
	int s;
	bool eol;
	do
	{
		s = Ggetch(f);
		eol = !(s == 0x0A || s == 0x0D || s == -1);
		if (eol)
		{
			c[0] = s;
			c++;
		};
	}
	while (eol);
	c[0] = 0;
	return (s == -1);
};

FormGroupDescription::FormGroupDescription()
{
	NGrp = 0;
	Grp = NULL;
};

FormGroupDescription::~FormGroupDescription()
{
	if (NGrp)
	{
		for (int i = 0; i < NGrp; i++)
		{
			if (Grp[i].NForms)free(Grp[i].Forms);
			if (Grp[i].NCommon)free(Grp[i].IDX);
		};
		NGrp = 0;
		free(Grp);
	};
};

void FGError()
{
	ErrM("Invalid file ord_groups.lst");
};

void FormGroupDescription::Load(GFILE* f)
{
	int z = Gscanf(f, "%d", &NGrp);
	if (z != 1)FGError();
	Grp = new SingleGroup[NGrp];
	int x;
	char cc[64];
	for (int j = 0; j < NGrp; j++)
	{
		z = Gscanf(f, "%s%d", cc, &Grp[j].NCommon);
		if (z != 2)FGError();
		int cidx = -1;
		for (int q = 0; q < NOClasses && cidx == -1; q++)
		{
			if (!strcmp(cc, OrderDesc[q].ID))cidx = q;
		};
		if (cidx == -1)FGError();
		Grp[j].ClassIndex = cidx;
		Grp[j].IDX = new byte[Grp[j].NCommon];
		for (int q = 0; q < Grp[j].NCommon; q++)
		{
			z = Gscanf(f, "%d", &x);
			if (z != 1)FGError();
			Grp[j].IDX[q] = x;
		};
		z = Gscanf(f, "%d", &Grp[j].NForms);
		if (z != 1)FGError();
		Grp[j].Forms = new word[Grp[j].NForms];
		for (int q = 0; q < Grp[j].NForms; q++)
		{
			z = Gscanf(f, "%s", cc);
			if (z != 1)FGError();
			cidx = -1;
			for (int p = 0; p < NEOrders && cidx == -1; p++)
			{
				if (!strcmp(ElementaryOrders[p].ID, cc))cidx = p;
			};
			if (cidx == -1)FGError();
			Grp[j].Forms[q] = cidx;
			ElementaryOrders[cidx].GroupID = j;
		};
	};
}

void LoadOrders()
{
	GFILE* f = Gopen("orders.lst", "r");
	int curOT = -1;
	if (f)
	{
		bool EndOfFile = false;
		char str[256];
		while (!EndOfFile)
		{
			EndOfFile = ReadFLine(f, str);
			if (!EndOfFile)
			{
				if (str[0] == '#')
				{
					int p;
					while (str[p = strlen(str) - 1] == ' ')
					{
						str[p] = 0;
					}
					if (!strcmp(str, "#EXIT"))
					{
						EndOfFile = true;
					}
					else
					{
						if (!strcmp(str, "#END"))
						{
							if (curOT != -1)
							{
								//centering
								int NU = 0;
								int xs = 0;
								int ys = 0;

								int xmin = 10000;
								int ymin = 10000;
								int xmax = 0;
								int ymax = 0;

								OrderDescription* ODE = ElementaryOrders + curOT;
								ODE->GroupID = 0xFF;
								for (int p = 0; p < ODE->NLines; p++)
								{
									int nn = ODE->LineNU[p];
									if (nn)
									{
										for (int q = 0; q < nn; q++)
										{
											if (p < ymin)
											{
												ymin = p;
											}
											if (p > ymax)
											{
												ymax = p;
											}
											int xx = ODE->Lines[p][q];
											if (xx < xmin)
											{
												xmin = xx;
											}
											if (xx > xmax)
											{
												xmax = xx;
											}
											xs += xx;
											ys += p * 2;
											NU++;
										}
									}
								}
								if (NU)
								{
									xs /= NU;
									ys /= NU;
									ODE->BarX0 = xmin - xs - 1;
									ODE->BarX1 = xmax - xs + 1;
									ODE->BarY0 = ymin * 2 - ys - 1;
									ODE->BarY1 = ymax * 2 - ys + 1;
									for (int p = 0; p < ODE->NLines; p++)
									{
										int nn = ODE->LineNU[p];
										if (nn)
										{
											for (int q = 0; q < nn; q++)
											{
												ODE->Lines[p][q] -= xs;
											}
										}
									}
									for (int p = 0; p < ODE->NCom; p++)
									{
										ODE->ComX[p] -= xs;
										ODE->ComY[p] -= ys;
									}
									ODE->YShift = ys;
									ODE->NUnits = NU;
									//symmetry checking
									if (ODE->SymInv)
									{
										//symmetry checking
										//1.Creating index collection
										word* Idc[100];
										int MaxX = -1000;
										int MinX = 1000;
										int MinY = 1000;
										int MaxY = -1000;
										for (int p = 0; p < ODE->NLines; p++)
										{
											int N1 = ODE->LineNU[p];
											if (N1)
											{
												if (p < MinY)
												{
													MinY = p;
												}
												if (p > MaxY)
												{
													MaxY = p;
												}
												for (int t = 0; t < N1; t++)
												{
													int x = ODE->Lines[p][t];
													if (x < MinX)
													{
														MinX = x;
													}
													if (x > MaxX)
													{
														MaxX = x;
													}
												}
											}
										}
										int MaxLx = MaxX - MinX + 1;
										int MaxLy = MaxY - MinY + 1;
										bool Good = true;
										int id = 0;
										for (int p = 0; p < MaxLy; p++)
										{
											Idc[p] = new word[MaxLx];
											memset(Idc[p], 0xFFFF, MaxLx * 2);
											int N1 = ODE->LineNU[p + MinY];
											for (int q = 0; q < N1; q++)
											{
												Idc[p][ODE->Lines[p + MinY][q] - MinX] = id;
												id++;
											}
										}
										Good = true;
										id = 0;
										ODE->SymInv = new word[ODE->NUnits];
										for (int p = 0; p < ODE->NLines; p++)
										{
											int N1 = ODE->LineNU[p];
											for (int q = 0; q < N1; q++)
											{
												int x = ODE->Lines[p][q] - MinX;
												int y1 = p - MinY;
												int NN = 0;
												for (int z = 0; z < y1; z++)
												{
													if (Idc[z][x] != 0xFFFF)NN++;
												}
												NN++;
												int x1 = MaxLx - x - 1;
												bool FOUND = 0;
												for (int z = MaxLy - 1; z >= 0 && NN; z--)
												{
													if (Idc[z][x1] != 0xFFFF)
													{
														NN--;
														if (!NN)
														{
															ODE->SymInv[id] = Idc[z][x1];
															FOUND = 1;
														}
													}
												}
												if (!FOUND)
												{
													Good = false;
												}
												id++;
											}
										}
										//4.Closing index collection
										for (int p = 0; p < MaxLy; p++)
										{
											free(Idc[p]);
										}
										if (!Good)
										{
											char c[128];
											sprintf(c, "orders.lst:Order %s has no symmetry SYM2", ODE->ID);
											ErrM(c);
											ODE->SymInv = NULL;
										}
										/*
										//1.checking
										bool Good=true;
										int N=ODE->NLines-1;
										for(int q=0;q<=N;q++){
											if(ODE->LineNU[q]==ODE->LineNU[N-q]){
												int N1=ODE->LineNU[q];
												for(int m=0;m<N1;m++)
													if(ODE->Lines[q][m]!=ODE->Lines[N-q][m])Good=false;
											}else Good=false;
										};
										if(!Good){
											char c[128];
											sprintf(c,"orders.lst:Order %s has no symmetry SYM2.",ODE->ID);
											ErrM(c);
											ODE->SymInv=NULL;
										}else{
											//accepting
											//1.Creating index collection
											word* Idc[200];
											memset(Idc,0,4*ODE->NLines);
											int id=0;
											for(int p=0;p<ODE->NLines;p++){
												if(ODE->LineNU[p]){
													int N1=ODE->LineNU[p];
													Idc[p]=new word[N1];
													for(int k=0;k<N1;k++){
														Idc[p][k]=id;
														id++;
													};
												};
											};
											//2.Reordering
											ODE->SymInv=new word[ODE->NUnits];
											id=0;
											for(p=ODE->NLines-1;p>=0;p--){
												if(Idc[p]){
													int N1=ODE->LineNU[p];
													for(int k=N1-1;k>=0;k--){
														ODE->SymInv[id]=Idc[p][k];
														id++;
													};
												};
											};
											//3.Free index collection
											for(p=0;p<ODE->NLines;p++)if(Idc[p])free(Idc[p]);
										};
										*/
									}
									if (ODE->Sym4f)
									{
										//symmetry checking
										//1.Creating index collection
										word* Idc[100];
										int MaxX = -1000;
										int MinX = 1000;
										int MinY = 1000;
										int MaxY = -1000;
										for (int p = 0; p < ODE->NLines; p++)
										{
											int N1 = ODE->LineNU[p];
											if (N1)
											{
												if (p < MinY)MinY = p;
												if (p > MaxY)MaxY = p;
												for (int t = 0; t < N1; t++)
												{
													int x = ODE->Lines[p][t];
													if (x < MinX)MinX = x;
													if (x > MaxX)MaxX = x;
												}
											}
										}
										int MaxLx = (MaxX - MinX + 2) >> 1;
										int MaxLy = MaxY - MinY + 1;
										bool Good = true;
										if (MaxLx == MaxLy)
										{
											int id = 0;
											for (int p = 0; p < MaxLy; p++)
											{
												Idc[p] = new word[MaxLx];
												memset(Idc[p], 0xFFFF, MaxLx * 2);
												int N1 = ODE->LineNU[p + MinY];
												for (int q = 0; q < N1; q++)
												{
													Idc[p][(ODE->Lines[p + MinY][q] - MinX) >> 1] = id;
													id++;
												}
											}
											//2.Checking
											for (int p = 0; p < MaxLy; p++)
											{
												int N1 = ODE->LineNU[p + MinY];
												for (int q = 0; q < N1; q++)
												{
													int x = (ODE->Lines[p + MinY][q] - MinX) >> 1;
													if (Idc[p][x] == 0xFFFF || Idc[x][p] == 0xFFFF || Idc[MaxLy - p - 1][MaxLx - x - 1] == 0xFFFF)Good =
														false;
												}
											}
											//3.Accepting
											if (Good)
											{
												id = 0;
												ODE->Sym4f = new word[ODE->NUnits];
												ODE->Sym4i = new word[ODE->NUnits];
												for (int p = 0; p < ODE->NLines; p++)
												{
													int N1 = ODE->LineNU[p];
													for (int q = 0; q < N1; q++)
													{
														int x = (ODE->Lines[p][q] - MinX) >> 1;
														//assert((ODE->Sym4f[id]=Idc[MaxLx-x-1][p-MinY])!=0xFFFF);
														//assert((ODE->Sym4i[id]=Idc[x][MaxY-p])!=0xFFFF);
														ODE->Sym4f[id] = Idc[MaxLx - x - 1][p - MinY];
														ODE->Sym4i[id] = Idc[x][MaxY - p];
														id++;
													}
												}
											}
											//4.Closing index collection
											for (int p = 0; p < MaxLy; p++)
											{
												free(Idc[p]);
											}
										}
										else
										{
											Good = false;
										}
										if (!Good)
										{
											char c[128];
											sprintf(c, "orders.lst:Order %s has no symmetry SYM4", ODE->ID);
											ErrM(c);
											ODE->Sym4f = NULL;
											ODE->Sym4i = NULL;
										}
									}
								}
							}
							curOT = -1;
						}
						else
						{
							if (NEOrders >= 128)
							{
								ErrM("orders.lst:Too many order types.");
								Gclose(f);
								return;
							}
							else
							{
								curOT = NEOrders;
								char ccc[128];
								char SY[32];
								SY[0] = 0;
								int p1, p2, p3, p4;
								OrderDescription* ODE = ElementaryOrders + curOT;
								int z = sscanf(str, "%s%s%d%d%d%d", ccc, SY, &p1, &p3, &p2, &p4);
								if (z != 4)
								{
									char c[128];
									sprintf(c, "orders.lst:Invalid order headline for %s", ccc);
								}
								ODE->ID = new char[strlen(ccc) + 1];
								strcpy(ElementaryOrders[curOT].ID, ccc);
								if (SY[0])
								{
									if (!strcmp(SY, "SYM2"))
									{
										ODE->SymInv = (word*)0xFFFF;
									}
									else
									{
										if (!strcmp(SY, "SYM4"))
										{
											ODE->SymInv = (word*)0xFFFF;
											ODE->Sym4i = (word*)0xFFFF;
											ODE->Sym4f = (word*)0xFFFF;
										}
										else
										{
											if (!strcmp(SY, "SYM0"))
											{
												ODE->SymInv = NULL;
												ODE->Sym4i = NULL;
												ODE->Sym4f = NULL;
											}
											else
											{
												char c[128];
												sprintf(c, "orders.lst:%s:Invalid symmetry specification:%s", ccc, SY);
												ErrM(c);
											}
										}
									}
								}
								else
								{
									ODE->SymInv = NULL;
									ODE->Sym4i = NULL;
									ODE->Sym4f = NULL;
								}
								ODE->AddDamage1 = p1;
								ODE->AddDamage2 = p3;
								ODE->AddShield1 = p2;
								ODE->AddShield2 = p4;
								NEOrders++;
							}
						}
					}
				}
				else
				{
					if (str[0] != '/')
					{
						if (curOT != -1)
						{
							OrderDescription* ODE = ElementaryOrders + curOT;
							int nz = 0;
							int nc = 0;
							int L = strlen(str);
							for (int p = 0; p < L; p++)
							{
								switch (str[p])
								{
								case '*':
									nz++;
									break;
								case 'C':
									nc++;
									break;
								}
							}
							int nL = ODE->NLines;
							ODE->NLines++;
							ODE->LineNU = (word*)realloc(ODE->LineNU, (nL + 1) << 2);
							ODE->Lines = (short**)realloc(ODE->Lines, (nL + 1) << 2);
							ODE->LineNU[nL] = nz;
							ODE->Lines[nL] = NULL;
							if (nz)
							{
								ODE->Lines[nL] = new short[nz];
								nz = 0;
								for (int p = 0; p < L; p++)
								{
									if (str[p] == '*')
									{
										ODE->Lines[nL][nz] = p * 2;
										nz++;
									}
								}
							}
							if (nc)
							{
								ODE->ComX = (short*)realloc(ODE->ComX, (ODE->NCom + nc) * 2);
								ODE->ComY = (short*)realloc(ODE->ComY, (ODE->NCom + nc) * 2);
								int nc0 = ODE->NCom;
								ODE->NCom += nc;
								for (int p = 0; p < L; p++)
								{
									if (str[p] == 'C')
									{
										ODE->ComX[nc0] = p * 2;
										ODE->ComY[nc0] = nL * 2;
										nc0++;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		ErrM("Could not open orders.lst");
	}
	//reading groups of orders
	f = Gopen("ord_groups.lst", "r");
	if (f)
	{
		FormGrp.Load(f);
		Gclose(f);
	}
	else
	{
		ErrM("Could not open ord_groups.lst");
	}
}

int DiffP[4];
AI_Description GlobalAI;

void InvAI()
{
	ErrM("Invalid file AI\\Ai.dat");
}

void LoadMessages();

AI_Description::AI_Description()
{
	LoadMessages();
	NAi = 0;
	GFILE* f = Gopen("Ai\\Ai.dat", "r");
	char cc[128];
	if (f)
	{
		int z = Gscanf(f, "%d %d", &NAi, &NComp);
#ifdef INETTESTVERSION
		NAi = 1;
		NComp = 1;
#endif
		if (z != 2)InvAI();
		Ai = new SingleAI_Desc[NAi];
		for (int i = 0; i < NAi; i++)
		{
			Gscanf(f, "%s", cc);
			Ai[i].NationID = new char[strlen(cc) + 1];
			strcpy(Ai[i].NationID, cc);
			char CC4[64];
			sprintf(CC4, "#%s", cc);
			Gscanf(f, "%s", cc);
			Ai[i].Message = new char[strlen(GetTextByID(CC4)) + 1];
			strcpy(Ai[i].Message, GetTextByID(CC4));
			Gscanf(f, "%s", cc);
			Ai[i].Peasant_ID = new char[strlen(cc) + 1];
			strcpy(Ai[i].Peasant_ID, cc);

			z = Gscanf(f, "%d%d%d", &Ai[i].NPeas, &Ai[i].NLandAI, &Ai[i].NWaterAI);
			if (z != 3)InvAI();
			Ai[i].LandAI = (char**)malloc(4 * Ai[i].NLandAI);
			Ai[i].WaterAI = (char**)malloc(4 * Ai[i].NWaterAI);
			for (int j = 0; j < Ai[i].NLandAI; j++)
			{
				Gscanf(f, "%s", cc);
				Ai[i].LandAI[j] = new char[strlen(cc) + 1];
				strcpy(Ai[i].LandAI[j], cc);
			};
			for (int j = 0; j < Ai[i].NWaterAI; j++)
			{
				Gscanf(f, "%s", cc);
				Ai[i].WaterAI[j] = new char[strlen(cc) + 1];
				strcpy(Ai[i].WaterAI[j], cc);
			};
		};
	}
	else
	{
		ErrM("Could not open Ai\\Ai.dat");
	};
	f = Gopen("Ai\\df.dat", "r");
	if (f)
	{
		Gscanf(f, "%d%d%d%d", DiffP, DiffP + 1, DiffP + 2, DiffP + 3);
		Gclose(f);
	}
	else ErrM("Could not open Ai\\df.dat");
};

AI_Description::~AI_Description()
{
	if (!NAi)return;
	for (int i = 0; i < NAi; i++)
	{
		for (int j = 0; j < Ai[i].NLandAI; j++)free(Ai[i].LandAI[j]);
		for (int j = 0; j < Ai[i].NWaterAI; j++)free(Ai[i].WaterAI[j]);
		free(Ai[i].LandAI);
		free(Ai[i].WaterAI);
		free(Ai[i].Message);
		free(Ai[i].NationID);
		free(Ai[i].Peasant_ID);
	};
	free(Ai);
};
