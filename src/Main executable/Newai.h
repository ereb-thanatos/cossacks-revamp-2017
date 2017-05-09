//01-wood(trees>2)
//02-stone(stones>5)
//04-full empty
//08-empty from locking
//16-plane zone
//32-high plate
extern byte* InfoMap;
//01-sklad
//02-farm
//04-tower
//08-building
//16-melnica
#define CB_Sklad	1
#define CB_Farm		2
#define CB_Tower	4
#define CB_Building 8
#define CB_Melnica  16
#define CB_Port     32
extern byte* CantBuild;

extern int* MineList;
extern word NMines;
extern word MaxMine;
void CreateInfoMap();
void RenewInfoMap(int x,int y);
typedef bool SearchFunction(int xx,int yy);
bool CheckMelnica(int x,int y);
bool CheckStoneSklad(int x,int y);
bool CheckWoodSklad(int x,int y);
bool CheckStoneWoodSklad(int x,int y);
bool CheckFarm(int x,int y);
bool CheckBuilding(int x,int y);
bool CheckPort(int x,int y);
bool SearchPlace(int* xx,int* yy,SearchFunction* SFN,int r);
bool FindCenter(int* xx,int *yy,byte NI);
void SetUnusable(int x,int y,byte Mask);
