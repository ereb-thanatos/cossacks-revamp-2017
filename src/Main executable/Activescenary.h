struct GAMEOBJ{
	word Index;
	word Serial;
	int  Type;
};
struct UnitsGroup{
	word* IDS;
	word* SNS;
	int N;
};
struct UnitsPosition{
	word* Type;
	int*  coor;
	int N;
};
struct ZonesGroup{
	word* ZoneID;
	int N;
};
typedef void StdVoid();
struct GTimer{
	int Time;
	bool Used;
	bool First;
};
struct LightSpot{
	int x,y,Type;
};
class ScenaryInterface{
public:
	void** SaveZone;
	int *  SaveSize;
	int    NSaves;
	int    MaxSaves;
	HINSTANCE hLib;
	char*  DLLName;

	UnitsGroup* UGRP;
	int    NUGRP;
	int    MaxUGRP;
	
	UnitsPosition* UPOS;
	int    NUPOS;
	int    MaxUPOS;

	ZonesGroup* ZGRP;
	int    NZGRP;
	int    MaxZGRP;

	char** Messages;
	int    NMess;
	int    MaxMess;

	char** Sounds;
	int    NSnd;
	int    MaxSnds;

	int NErrors;

	//char*  MissText;
	//int    TextSize;

	int NPages;
	int MaxPages;
	char** Page;
	int*   PageSize;
	char** PageID;
	char** PageBMP;

	bool   TextDisable[52];
	
	bool   StandartVictory;
	bool   Victory;
	char*  VictoryText;

	bool   LooseGame;
	char*  LooseText;

	char*  CentralText;
	int CTextTime;

	GTimer TIME[32];
	word   TRIGGER[512];
	LightSpot LSpot[64];
	StdVoid* ScenaryHandler;
	ScenaryInterface();
	~ScenaryInterface();
	void Load(char* Name,char* Text);
	void UnLoading();
};
extern ScenaryInterface SCENINF;
class SingleMission{
public:
	char* ID;
	char* DLLPath;
	char* MapName;
	char* Name;
	char* Description;
	int NIntro;
	char** Intro;
};
class MissPack{
public:
	SingleMission* MISS;
	int NMiss;
	int MaxMiss;
	int CurrentMission;
	int* SingleMS;
	int MSMiss;
	void LoadMissions();
	MissPack();
	~MissPack();
};
struct SingleCampagin{
	char* CampMessage;
	char* CampText;
	char* CampBmp;
	int NMiss;
	int* Miss;
	DWORD* OpenIndex;
};
class CampaginPack{
public:
	int NCamp;
	SingleCampagin* SCamp;
	CampaginPack();
	~CampaginPack();
};
class OneBattle{
public:
	char* ID;
	char* Map;
	char* Text;
	char* Brief;
	char* BigMap;
	char* MiniMap;
	char* BigHeader;
	char* SmallHeader;
	char* Date;
	char* RedUnits;
	char* BlueUnits;
	int NHints;
	char** Hints;
	int* Coor;
};
class OneWar{
public:
	char* Name;
	char* Date;
	char* Text;
	int NBatles;
	int* BattleList;
};
class WarPack{
public:
	int NWars;
	OneWar* Wars;
	int NBattles;
	OneBattle* Battles;
	WarPack();
	~WarPack();
};
extern WarPack WARS;
extern MissPack MISSLIST;
extern CampaginPack CAMPAGINS;

extern byte CurAINation;
extern City* CCIT;
extern Nation* CNAT;
extern bool AiIsRunNow;
extern int CurrentCampagin;
extern int CurrentMission;
void SFLB_LoadPlayerData();
void SavePlayerData();