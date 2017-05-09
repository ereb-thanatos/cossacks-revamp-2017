#pragma pack(1)
struct Radio{
	char* xi;
	char* yi;
	word N;
};
#define RRad 90
extern Radio Rarr[RRad];

void CreateAreas();
void InitAreas();
void EraseAreas();
extern int TopLx;
extern int TopLy;
extern int MaxTop;
extern int TopSH;
extern word* TopRef;

#define SI_Center	1
#define SI_Meln		2
#define SI_Sklad	4
#define SI_Shahta	8
#define SI_Tower	16
#define SI_ArtDepo	32
#define SI_House	64
#define SI_Barrack  128
#define SI_Farm		256
#define SI_Other	512
#define SI_Near     1024
#define SI_FarLand  8192
#define SI_Okrug    16384
#define SI_Central  32768
struct StrategyInfo{
public:
	word BuildInfo;
	byte NPeasants;
	byte NShortRange;
	byte NLongRange;
	byte NMortir;
	byte NTowers;
	byte NPushek;
};
struct Area{
	short x;
	short y;
	byte  Importance;
	byte NTrees;
	byte NStones;
	//01-gold
	//02-iron
	//04-coal
	word  NMines;
	word* MinesIdx;
	word  NLinks;
	word* Link;
	word MaxLink;
	StrategyInfo SINF[8];
};
struct MediaTop{
	word* MotionLinks;
	word* LinksDist;
	word* TopRef;
	int NAreas;
	Area* TopMap;
};
extern MediaTop GTOP[2];
extern Area* TopMap;
extern int   NAreas;
extern int MaxArea;
extern word* MotionLinks;
extern word* LinksDist;
void CreateStrategyInfo();
//---------
#define WTopLx (TopLx)
#define WTopSH (TopSH)
#define WCLSH 2
#define WCLLX 4
extern word* WTopRef;
extern Area* WTopMap;
extern int   WNAreas;
extern int WMaxArea;
extern word* WMotionLinks;
extern word* WLinksDist;
//---------
void InitTopChange();
void ClearTopChange();
void AddChTop(word TopID);
void AddSurroundingAreas(word TopID);
void DynamicalLockTopCell(int x,int y);
void DynamicalUnLockTopCell(int x,int y);
void ProcessDynamicalTopology();