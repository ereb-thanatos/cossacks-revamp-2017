//#define sprNx  (64<<ADDSH)
#define SprShf (6+ADDSH)
#define SprInCell 16
extern int MaxSprt;
class ObjCharacter{
public:
	byte ResType;//0-wood,1-gold,2-stone,3-food,0xFE-no resource,0xFF-removable
	byte WorkRadius;
	byte ResPerWork;
	int WorkAmount;
	word WNextObj;
	word DamageAmount;
	word DNextObj;
    word TimeAmount;
    word TNextObj;
    byte IntResType;
    word IntResPerWork;
    word IntWorkRadius;
    NewAnimation* Stand;
    NewAnimation* Shadow;
    short Z0;
    short DZ;
    byte Parts;
    byte Delay;
    byte Frames;
	byte Discret;
	byte LockRadius;
	word NLockPt;
	short* LockX;
	short* LockY;

	char* Name;
};
class SprGroup{
public:
	int  NSpr;
	word FileID;
	word* Items;
	short* Dx;
	short* Dy;
	short* Radius;
	ObjCharacter* ObjChar;
	void LoadSprites(char* fname);
	int GetIndexByName(char* Name);
};
class OneSprite{
public:
	bool Enabled:1;
	bool Surrounded:1;
	int x;
	int y;
	short z;
	word Radius;
	SprGroup* SG;
	ObjCharacter* OC;
	int Index;
	word SGIndex;
	byte WorkOver;
    word TimePassed;
	byte Damage;
	int PerformWork(word);
    int PerformIntWork(int work);
	void PerformDamage(int Dam);
};
class TimeReq{
public:
    int NMembers;
    int MaxMembers;
    int* IDS;
    byte* Kinds;
    TimeReq();
    ~TimeReq();
    void Handle();
    void Add(int ID,byte Kind);
    void Del(int ID,byte Kind);
};
extern TimeReq ObjTimer;
//extern byte NSpri[sprNx*sprNx];
extern byte* NSpri;
//extern word* SpRefs[sprNx*sprNx];
extern int** SpRefs
;
extern OneSprite* Sprites;
void InitSprites();
void addSprite(int x,int y,SprGroup* SG,word id);
int GetHeight(int x,int y);
void PreShowSprites();
void addScrSprite(int x,int y,SprGroup* SG,word id);
void addTree(int x,int y);
extern OneSprite* Sprites;
extern SprGroup TREES;
extern SprGroup STONES;
extern SprGroup HOLES;
extern SprGroup COMPLEX;
void ProcessSprites();
byte DetermineResource(int x,int y);
byte FindAnyResInCell(int x,int y,int cell,int* Dist,byte Res);
bool CheckDist(int x,int y,word r);
void HideFlags();
//-----------------------L3--------------------------
//#define L3DX (MAPSX>>4)
//#define L3DY (MAPSY>>4)
//#define L3MAX (MAPSX<<2)
//#define L3SH (ADDSH+5)
//extern byte* L3HIMap[L3DX*L3DY];
//void InitL3();
//void ClearL3();
//void SetL3Point(int x,int y,byte H);
//int GetL3Height(int x,int y);
int GetUnitHeight(int x,int y);