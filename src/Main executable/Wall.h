struct WallCell{
	byte x;
	byte y;
	word Stage;
	word Tile;
	word Health;
	word MaxHealth;
	word WorkerID;
	byte WorkTime;
	byte WorkNeed;
	word BuildProgress;
};
class WallCluster{
public:
	int ClusterSize;
	WallCell* Cells;
	byte OwnerID;
	byte Type;
	int Index;
	WallCluster();
void AddWall(byte x,byte y);
void PreArrangeTiles();
void ArrangeTiles();
int CheckPoint(byte x,byte y);
void ShowCluster();
void Stand();
void Init();
int	GetDataSize();
void CreateData(byte NI,byte* lpData);
int FindWorkPoint(byte x,byte y,word ID);
int FindDamagePoint(byte x,byte y,word ID);
};
class GWSys{
public:
	WallCluster* GWC;
	int MaxWall;
	GWSys();
	word AddCluster(byte* Data);
	void ProcessClusters();
	void BuildCell(byte x,byte y,byte Owner,byte Incr,word WorkerID);
	void DamageCell(byte x,byte y,byte Owner,byte Incr);
};
extern WallCluster TMPCluster;
extern bool BuildWall;
extern bool WStarted;
void SetBuildWallMode();
void WallHandleMouse(int x,int y);
void LoadWalls();
extern GWSys GWALLS;
extern word Links[256][256];
extern word LIndex[256][256];