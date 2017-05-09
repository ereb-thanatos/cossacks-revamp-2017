#define MAXTOW 24
extern int NTows;
#define TSSHIFT (4+8)
struct HumanShip{
	word ID;
	word SN;
	int x,y;
	int Force;
	int Life;
	int R;
};
struct HShipsGroup{
	int xs,ys;
	int xL,yL;
	int xR,yR;
	int Ns, Force;
	int CTop;
	int R;
};
class EnemyInfo{
public:
	GlobalArmyInfo GAINF;
	byte Mask;
	byte BasicNation;

	byte* ProtectionMap;
	word* SupMortBestID;
	word* SupMortLastTime;
	word EnmBuildList[128];
	word EnmBuildSN[128];
	int  NEnmBuild;
	int  NEnWalls;
	word WallsX[200];
	word WallsY[200];
	word TowsID[MAXTOW];
	word TowsSN[MAXTOW];
	word TowsMaxR[MAXTOW];
	byte TowDanger[MAXTOW];
	word CurTow;
	word CurR;
	word CurXip;
	word MaxTowR;
	DWORD* InflMap;
	DangerInfo DINF[40];
	int NDINF;
	int LastBuildSafetyTime;
	byte* SafeMAP;
	int CurRStage;
	SafeCellInfo** SCINF;
	word WTopNear[64];
	int NWTopNear;
	word* NUN;
	byte* TMAP;
	HumanShip SHIPS[128];
	HShipsGroup SGRP[64];
	int NSGRP;
	int NHSHIPS;
	int ShipsForce;
	word TopAreasDanger[1024];
	
	void InitInflMap();
	void ClearTow(OneObject* OB);
	void AddTow(OneObject* OB);
	void ProcessTow();
	void InitSuperMortiraCells();
	word ResearchCellForSupermortira(int cell,int MinDistance,int MaxDistance);
	word SearchDangerousPlaceForEnemy(int* utx,int* uty,int MINR,int MAXR);
	bool FindSafePlace(int* utx,int* uty);
	word SearchBestEnemyAndPlaceForSupermortira(OneObject* OB,int* BestX,int* BestY);
	void CreateEnmBuildList();
	void CreateProtectionMap();
	void CreateWallsList();
	bool CheckTowersPresence();
	void CreateListOfDangerObjects();
	void ResearchArmyDistribution();
	void InitBuildSafety();
	void CreateBuildSafetyMap();
	void RefreshSafeMap();
	byte GetSafeVal(int x,int y);
	void ShowSafetyInfo(int x,int y);
	void AddSafePoint(int x,int y,word Index,word SN,word Prio);
	void ClearSafePoint(int x,int y,word Index);
	void InitSafeInfo();
	void CloseSafeInfo();
	void CheckSafeInfo();
	SafeCellInfo* GetCellInfo(int x,int y);
	void RegisterSafePoint(OneObject* OB,int x,int y);
	void InitResearchSafeCell();
	void ResearchSafeObject(OneObject* OB,int MinR,int MaxR,int pstart);
	void ResearchSafeCells(int MinR,int MaxR);
	void Clear();
	void ResearchHumanAttackPlaces();
	void ClearPlaces();
	void RegisterHumanShips();
	int  GetMaxForceOnTheWay(int TopStart,int FinalTop);
	void ResearchShipsGroups();
	int  GetShipsForce(int x,int y,int r);
	void ClearIslands();

	void ALLOCATE();
	void FREE();


};
class GlobalEnemyInfo{
public:
	EnemyInfo* EINF[8];
	void Clear();
	void Setup();
	void Process();
	void AddTow(OneObject* OB);
	void ClearTow(OneObject* OB);
	~GlobalEnemyInfo();
	GlobalEnemyInfo();
	bool FindNearestEnemy(byte NNUM,int* x,int* y,bool TowerFear,int Min,bool Defence);
};
extern GlobalEnemyInfo GNFO;