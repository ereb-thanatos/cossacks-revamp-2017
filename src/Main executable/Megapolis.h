#define N_KINDS 5
#define OrdDist 320
class AI_Army;
class Cell8x8
{
public:
	union
	{
		struct foo
		{
			byte Peasants;//кол-во крестьян на этой клетке
			byte Towers;//------//------башен
			byte Warriors;
			byte Farms;
			byte Buildings;
			byte Fly;
			byte WaterPeas;
			byte WaterObj;
		};
		byte UnitsAmount[8];
	};
	//byte P_MoveUp;//кол-во пересечений верхней границы
	//byte P_MoveDn;
	//byte P_MoveLf;
	//byte P_MoveRi;
	//byte Protection;  //=4*Towers+Warriors
	//byte Agressivity; //=Warriors
	//byte Importance;  //=Farms+Buildings
	word Neighbor;    //(0x001)  bit0 - UP
					  //(0x002)  bit1 - UP-RIGHT
					  //(0x004)  bit2 - RIGHT
					  //(0x008)  bit3 - DN-RIGHT
					  //(0x010)  bit4 - DN
					  //(0x020)  bit5 - DN-LEFT
					  //(0x040)  bit6 - LEFT
					  //(0x080)  bit7 - UP-LEFT
					  //(0x100)  bit8 - UP+2
					  //(0x200)  bit9 - RIGHT+2
					  //(0x400)  bit10- DN+2
					  //(0x800)  bit11- RIGHT+2
	//byte Resrv[3];
	//word Enemy;
	//word EnemyTime;
	Cell8x8();
};
typedef Cell8x8 CellsInfo[64][64];
typedef CellsInfo TotalCInfo[8];
//extern TotalCInfo  TCInf;
class CityProject
{
public:

};
class ProposedProject
{
public:
	byte PKind;//=0-Monster,==1-Upgrade
	word NIndex;//номер типа монстра/Upgrade index
	word ProducerIndex;//индекс свободного производителя
	word Percent;//процент денег, употребляемый на производство
	//int Cost;//цена 
	//int Useful[NBRANCH];
};
struct BuildProject
{
	bool Used : 1;
	bool PlaceFound : 1;
	bool Founded : 1;
	bool PeasantsCalled : 1;
	byte Usage;
	short NearX;
	short NearY;
	int  x, y;
	int Options;
	word AttemptsToStand;
	word AttemptsToFindApprPlace;
	word Index;
	word NIndex;
	byte MaxPeasants;
	byte MinPeasants;
	byte NPeasantsCalled;
};
class CityCell
{
public:
	int x, y;
	word WorkTimes;//==0xFFFF if inside the city
	CityCell* NextCell;
};
class CityCluster
{
public:
	CityCell* FirstCell;
	void AddCell( int x, int y );
};
typedef int Functional( int* x, int* y, int Lx, int Ly, byte NI );
typedef int CheckWorkFn( OneObject* OB, City* CT );
class SmartGroup;
class Brigade;
class City;
typedef void BrigadeLink( Brigade* Brig );
class BrigadeOrder;
class BrigadeOrder
{
public:
	byte OrdType;
	byte Prio;
	word Size;
	char* Message;
	BrigadeOrder* Next;
	BrigadeLink* BLink;
};
struct BrigMemb
{
	word Peons;
	word Infantry;
	word Strelkov;
	word Mortir;
	word Pushek;
	word Grenaderov;
	word Other;
	word reserv;
};
int GetBMIndex( OneObject* OB );
class Brigade
{
public:
	City* CT;
	word* Memb;
	word* MembSN;
	word  SN;
	int* posX;
	int* posY;
	word NMemb;
	word MaxMemb;
	word MembID;
	char AddDamage;
	char AddShield;
	byte WarType;
	byte Direction;
	word ID;
	word BrigDelay;
	word LastTopology;
	int LastOrderTime;
	word ArmyID;
	word ErasureTime;
	bool Enabled : 1;
	bool PosCreated : 1;
	bool Precise : 1;
	bool Strelki : 1;
	bool AttEnm : 1;
	BrigMemb BM;
	CheckWorkFn* CFN;
	BrigadeOrder* BOrder;
	word LastEnemyID;
	word LastEnemySN;


	void SetIndex();
	void Init( City* CT, word ID );
	void AddObject( OneObject* OB );
	void CheckMembers( City* CT );
	void RemoveObjects( int NObj, Brigade* Dest );
	bool RemoveOne( int Index, Brigade* Dest );
	void RemovePeasants( Brigade* Dest );
	void FreeMember( int Idx );
	void DeleteAll();
	void CreateSquare();
	void CreateConvoy( byte Type );
	int AddInRadius( int x, int y, int r, BrigMemb* BMem, Brigade* Dest );
	int AddInRadius( int x, int y, int r, BrigMemb* BMemb );
	BrigadeOrder* CreateOrder( byte OrdType, int Size );
	int SelectPeasants( byte NI );
	void Rospusk();
	bool GetCenter( int* x, int* y );
	//----------------------ORDERS----------------------//
	bool LocalSendTo( int x, int y, byte prio, byte OrdType );
	bool LinearLocalSendTo( int x, int y, byte prio, byte OrdType );
	bool WideLocalSendTo( int x, int y, byte prio, byte OrdType );
	bool GlobalSendTo( int x, int y, byte prio, byte OrdType );
	bool CaptureMine( int SID, byte prio, byte OrdType );
	bool AttackEnemy( int x, int y, byte prio, byte OrdType );
	void MakeBattle();
	void ProtectFarMines();
	//-----------------Human functions------------------//
	bool CreateNearOfficer( OneObject* OB, word Type, int ODIndex );
	void CreateOrderedPositions( int x, int y, char dir );
	void CreateSimpleOrderedPositions( int x, int y, char dir );
	void HumanCheckUnits();
	void HumanLocalSendTo( int x, int y, short Dir, byte Prio, byte OrdType );
	void HumanGlobalSendTo( int x, int y, short Dir, byte Prio, byte OrdType );
	void KeepPositions( byte OrdType, byte Prio );
	void Bitva();
	//--------------------------------------------------//
	void ClearBOrders();
	void DeleteBOrder();
};
typedef void GroupMaker( SmartGroup* GS );
class GroupOrder;
class GroupOrder
{
public:
	byte		OrderType;
	GroupOrder* NextOrder;
	GroupMaker* GroupLink;
	void*       GroupData;
	int         GDataSize;
};

class Idea;
typedef void IdeaBrain( Idea* IDEA );
typedef void FreeIdea( Idea* ID );
class Idea
{
public:
	City* CT;
	Idea* PrevIdea;
	Idea* NextIdea;
	IdeaBrain* Brain;
	void* IdeaData;
	FreeIdea* FI;
	int   DataSize;
	void ClearIdea();
};
class Inform;
//typedef void I_Save(SaveBuf* SB,Inform* Inf);
//typedef void I_Load(SaveBuf* SB,Inform** Inf);
typedef void I_Clear( Inform* Inf );
class Inform
{
public:
	word ID;
	word Essence;
	Inform* Next;
	Inform* Previous;
	City*   CT;
	I_Clear* IClr;
	word Size;
};
class MineBase :public Inform
{
public:
	word   M_ID;
	word   M_SN;
	short  topx;
	short  topy;
	byte   ResKind;
	byte   UpgLevel;
	byte   PSend;
	word   DefendersID;
	word   DefendersSN;
	word   MinersID;
	word   MinersSN;
	/*
	word   TowID;
	word   TowSN;
	word   NWalls;
	short* Walls;
	word   NGateProj;
	short* GateProj;
	word   NGates;
	short* Gates;
	*/
};
class PRM_Info :public Inform
{
public:
	int NBrigs;
	word BrigsID[5];
	word BrigsSN[5];
};
class SmartGroup : public Brigade
{
	bool Enabled;
	word Index;
	word NFarInf;
	word NNearInf;
	word NFarCav;
	word NNearCav;
	short CenterX;
	short CenterY;
	GroupOrder* Order;
};
#define MBR4    16
#define MaxBrig (MBR4*32)
#define MaxArm 128
class DefendInfo
{
public:
	byte  x;
	byte  y;
	byte  Importance;
	word  NDefenders;
	word  MaxDefs;
	word* Def;
	word* DefSN;
	void AddUnit( OneObject* OB );
};
class AI_Army;
//----------------------ARMY--------------------//

#define PolkSize   36
#define NBrigTypes 6
class ExtendedBrigade
{
public:
	byte BrigadeType;
	//0-Short range infantry
	//1-Long range infantry
	//2-Fast cavalry
	//3-Hard cavalry
	//4-Long range cavalry
	//5-Artillery
	//6-Sapers
	//7-Builders
	int Force;
	int NeedMembers;
	word NextBrigade;
	Brigade* Brig;
};
typedef void ArmyLink( AI_Army* ARM );
class ArmyOrder;
class ArmyOrder
{
public:
	byte OrdType;
	byte Prio;
	word Size;
	char* Message;
	ArmyOrder* Next;
	ArmyLink*  ALink;
};
struct FormInfo
{
	word ComID;
	word ComSN;
	word BarID;
	word BarSN;
};
class AI_Army
{
public:
	int NExBrigs;
	int MaxExBrigs;
	bool Enabled : 1;
	bool SpecialOrder : 1;
	int TopPos;
	int x, y;
	int Force;
	byte Spec;
	word ArmyID;
	ExtendedBrigade* ExBrigs;
	//Additional information
	City* CT;
	Nation* NT;
	byte NI;
	int LastBitvaTime;
	word FirstBrig;
	//Order information
	ArmyOrder* AOrder;
	//Formation info;
	word ComID[8];
	word ComSN[8];
	word NCom;
	word BarID[8];
	word BarSN[8];
	word NBar;
	word NFreeUnits;
	//Functions
	void InitArmy( City* CT );
	void ClearArmy();
	void CreateMinimalArmyFromBrigade( Brigade* BR, int Type );
	void AddBrigade( Brigade* BR );
	void AddUnit( OneObject* OB );
	void CloseArmy();
	void MoveArmyTo( int x, int y );
	void CreateBrigLink();
	void AttackArea( int x, int y );
	ArmyOrder* CreateOrder( byte OrdType, int Size );
	void DeleteAOrder();
	void ClearAOrders();
	void LocalSendTo( int x, int y, byte Prio, byte OrdType );
	int  GetArmyDanger( int x, int y );
	void SendToMostSafePosition();
	void WideLocalSendTo( int x, int y, byte Prio, byte OrdType );
	void Parad();
	void ConnectToArmy( int ID, byte Prio, byte OrdType );
	void MakeBattle();
	void Bitva();
	void Diversia();
	void SetZombi();
	void ClearZombi();
	bool CheckArtillery();
	int  GetAmountOfBrigs( byte Type );
	void SendGrenaders();
};
class City
{
public:
	Nation* Nat;
	byte NI;
	//int InX,InY,InX1,InY1,OutX,OutY,OutX1,OutY1;
	//int TryTimes;
	//int ExpandSize;
	int Amount;
	//int GoldPeasPart;
	//int WoodPeasPart;
	//int BuildPeasPart;
	//word KindMax[N_KINDS];//макс. кол-во тварей типа Kind
	//word *MKList[N_KINDS];//массив указателей на массивы с индексами монстров данного типа
	//Functional* FNC[N_KINDS];//массив функционалов для выбора места, если место вообще не подходит, то возвр. -1
	//word BranchPart[NBRANCH];
	int AutoEraseTime;
	word BestProj[128];
	word NBestProjects;
	byte NWoodSklads;
	byte NStoneSklads;
	word WoodSkladID[8];
	word WoodSkladSN[8];
	word StoneSkladID[8];
	word StoneSkladSN[8];
	char AIID[48];
	byte StartRes;
	byte ResOnMap;
	byte Difficulty;
	byte LandType;
	word TransportID;
	word TransportSN;

	byte NMeln;
	word MelnID[4];
	word MelnSN[4];

	int  MelnCoorX[4];
	int  MelnCoorY[4];
	int  NMelnCoor;

	int  FieldsID[512];
	word FieldsSN[512];
	word NFields;

	word Construction[64];
	word ConstructionSN[64];
	word ConstructionTime[64];
	word NConstructions;

	word FarmsUnderConstruction;
	int Account;

	bool FieldReady;
	bool AllowWood;
	bool AllowStone;
	bool AllowMine;
	bool DefenceStage;

	word NeedOnField;
	word NeedOnStone;
	word NeedOnWood;

	word** GroupsSet;
	word*  NGroupsInSet;
	int    NGroups;

	Brigade Brigs[MaxBrig - 11];
	Brigade Settlers;
	Brigade Defenders;
	Brigade Guards;
	Brigade Agressors;
	Brigade OnField;
	Brigade OnStone;
	Brigade OnWood;
	Brigade Builders;
	Brigade Free;
	Brigade InMines;
	Brigade FreeArmy;
	byte N_mineUp[10];
	word MaxFields;
	word BestOnField;
	word WaterBrigs[128];
	//------------------ARMIES-------------------//
	AI_Army ARMS[MaxArm];
	word* DefArms;
	int NDefArms;
	int MaxDefArms;
	int NDivr;
	byte LockUpgrade[8];
	byte MyIsland;
	//-------------Peasants balance--------------//
	short NPeas;
	short NeedPF;
	short NeedPW;
	short NeedPS;
	short FreePS;
	int   WoodAmount;
	int   StoneAmount;
	int   FoodAmount;
	int   LastTime;
	int  WoodSpeed;
	int  StoneSpeed;
	int  FoodSpeed;
	//-------------------------------------------//
	int   LFarmIndex;
	int   LFarmRadius;
	short FarmLX;
	short FarmLY;
	//-------------------------------------------//
	word  MAX_WORKERS;
	//-------------------------------------------//
	bool AI_DEVELOPMENT : 1;
	bool AI_WATER_BATTLE : 1;
	bool AI_LAND_BATTLE : 1;
	bool AI_MINES_CPTURE : 1;
	bool AI_TOWN_DEFENCE : 1;
	bool AI_MINES_UPGRADE : 1;
	bool AI_FAST_DIVERSION : 1;
	//-----------DEFENDING THE BASE--------------//
	DefendInfo* DefInf;
	int NDefn;
	int MaxDefn;
	int MaxGuards;
	int AbsMaxGuards;
	//-------------------------------------------//
	word NP;
	word NAgressors;
	word MinType;
	word MaxType;
	word UnitAmount[2048];//Amount of units of some kind
	word ReadyAmount[2048];
	word PRPIndex[2048];//index of project to produce this type of monster,if not proposed,0xFFFF
	word UPGIndex[2048];
	word UnBusyAmount[2048];//сколько модулей простаивает
	word Producer[2048];
	bool PresentProject : 1;
	bool IntellectEnabled : 1;
	bool CenterFound : 1;
	short CenterX;
	short CenterY;
	BuildProject BPR;
	word NProp;
	ProposedProject Prop[100];
	Idea* IDEA;
	Inform* INFORM;
	Idea* AddIdea( IdeaBrain* IBR, bool Dup );
	void AddInform( Inform* Inf, I_Clear* ICL );
	void DelInform();
	void DelInform( Inform* Inf );
	Inform* SearchInform( word ID, word Essence, Inform* inf );
	Inform* SearchInform( word ID, Inform* inf );
	void HandleIdeas();
	void DelIdeas();
	word EnemyList[32];
	void CreateCity( byte NI );
	bool CheckTZone( int x, int y, int Lx, int Ly );
	bool TryToFindPlace( int* x, int* y, int Lx, int Ly, byte Kind );
	void EnumUnits();
	void AddProp( word NIN, GeneralObject* GO, word prop, word per );
	void AddUpgr( word NIN, word prod, word per );
	void EnumProp();
	void ProcessCreation();
	void HelpMe( word ID );
	void HandleFly();
	word FindNeedProject();
	void RefreshAbility();
	bool FindApproximateBuildingPlace( GeneralObject* GO );
	bool FindPreciseBuildingPlace( GeneralObject* GO );
	void AddConstruction( OneObject* OB );
	void HandleConstructions();
	void MarkUnusablePlace();
	void CloseCity();
	void HandleFields();
	void MakeZasev();
	void MakeSlowZasev();
	void HandleBrigades( int NP );
	void RegisterNewUnit( OneObject* OB );
	int  GetFreeBrigade();
	void ExecuteBrigades();
	void CalculateBalance();
	void GetNextPlaceForFarm( int* x, int* y );
	void UnRegisterNewUnit( OneObject* OB );
	void RegisterWaterBrigade( int ID );
	void UnRegisterWaterBrigade( int ID );
	//---------------Armies--------------
	int GetFreeArmy();

	//--------------Ideas----------------
	void TakeNewMine();
	void UpgradeSomeMine();
	void CreateWallNearMine();
	void BuildTower();
	void UpgradeMines();
	void SendAgressors();
	void ProtectMine();
	void BuildWallsNearMines();
	//----------------defending--------------//
	void HandleDefending();
	int  CheckDefending( byte x, byte y );
	void AddDefending( byte x, byte y, byte Importance );
	void AddUnitDefender( OneObject* OB );
};
class Forces
{
public:
	int MinX;
	int MinY;
	int MaxX;
	int MaxY;
	int Damage;
	int Life;
	int Shield;
	int Protection;
	int Population;
	int StrongFactor;
	int CenterX;
	int CenterY;
	int NBuild;
	void SetDefaults();
};
#define SendPTime 10000
class SendPInform :public Inform
{
public:
	int time;
};
struct DangerInfo
{
	word ID;
	word SN;
	word Kind;
	word UpgradeLevel;
	word Life;
	word MaxLife;
	int x, y, z;
	int MinR;
	int MaxR;
};

extern Forces NForces[8];
extern int BestForce;
extern int BestNat;
extern int NextBest;
void OutCInf( byte x, byte y );
extern int COUNTER;
extern DWORD BMASK[MBR4];
void InitSuperMortiraCells();
void CreateEnmBuildList();
void CreateProtectionMap();
struct CostPlace
{
	int xw, yw;
	int xc, yc;
	byte Island;
	byte R;
	byte Access;
	word Transport;
};
extern CostPlace* COSTPL;
extern int NCost;
extern int MaxCost;