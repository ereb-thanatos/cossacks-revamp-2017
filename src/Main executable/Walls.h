class WallCharacter
{
public:
	char* Name;
	word RIndex;
	word NBuild;
	word Ndamage;
	short dx, dy;
	word GateFile;
	short GateDx;
	short GateDy;
	short UpgradeGateIcon;
	short OpenGateIcon;
	short CloseGateIcon;
	short UpgradeTower1;
	short UpgradeTower2;
	word  GateCost[8];
	short TexRadius;
	short NTex;
	short TexPrec;
	short* Tex;
	short OpenGateSoundID;
	short CloseGateSoundID;
	byte  OpenKeyFrame;
	byte  CloseKeyFrame;
};

class WallCluster;

class WallCell
{
public:
	short x;
	short y;
	byte DirMask;
	byte Type;
	byte NI;
	byte Stage;
	byte MaxStage;
	word Health;
	word MaxHealth;
	byte Sprite;
	byte SprBase;
	word ClusterIndex;
	word OIndex;
	word GateIndex;
	int Locks;
	bool Visible;
	bool CheckPosition();
	int GetLockStatus();
	void SetLockStatus();
	bool StandOnLand( WallCluster* WC );
	void CreateLocking( WallCluster* WC );
	void Landing( WallCluster* WC );
	void ClearLocking();
};

class WallSystem;

class WallCluster
{
public:
	byte Type;
	int NCornPt;
	word* CornPt;
	int NCells;
	WallCell* Cells;
	WallSystem* WSys;
	short LastX;
	short LastY;
	short FinalX;
	short FinalY;
	NewMonster* NM;
	word  NIndex;
	byte  NI;
	//------------------//
	WallCluster();
	~WallCluster();
	void SetSize( int N );
	void ConnectToPoint( short x, short y );
	void ConnectToPoint( short x, short y, bool Vis );
	void UndoSegment();
	void SetPreviousSegment();
	void KeepSegment();
	void CreateSprites();
	void AddPoint( short x, short y, bool Vis );
	void Preview();
	void View();
	int  CreateData( word* Data, word Health );
	void CreateByData( word* Data );
	void SendSelectedToWork( byte NI, byte OrdType );
};

class WallSystem
{
public:
	int NClusters;
	WallCluster** WCL;
	//-------------------//
	WallSystem();
	~WallSystem();
	void AddCluster( WallCluster* );
	void WallSystem::Show();
};

void WallHandleDraw();
void WallHandleMouse();
void SetWallBuildMode( byte NI, word NIndex );
void LoadAllWalls();

inline int GetLI( int x, int y )
{
	return x + ( y << ( VAL_SHFCX + 1 ) );
}

extern int MaxLI;
extern int MaxLIX;
extern int MaxLIY;
extern int NChar;

extern WallCharacter WChar[32];

//extern WallCell* WRefs[MAXCX*MAXCY*4];

extern WallCell** WRefs;
extern WallSystem WSys;

void DetermineWallClick( int x, int y );
void SetLife( WallCell* WC, int Health );

//gates
#define NGOpen 9

class Gate
{
public:
	short x;
	short y;
	byte NI;
	byte NMask;
	byte State;
	byte delay;
	byte Locked;
	byte CharID;
};

extern Gate* Gates;
extern int NGates;
extern int MaxGates;

void SetupGates();
void InitGates();
int AddGate( short x, short y, byte NI );
void DelGate( int ID );