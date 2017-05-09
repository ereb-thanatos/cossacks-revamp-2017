
struct MineRec{
	char* Name;
	int Ng,Ni,Nc;
};
struct ResRec{
	char* Name;
	int RES[8];
};
struct PlRec{
	int NPlayers;
	char* name;
};
struct StyleRec{
	char* Name;
	char* Style;
	int NPl;
	int AI_Style;
	PlRec* Players;
};
class RandomMapDesc{
public:
	int NMINES;
	int MINES_DEF;
	MineRec* MINES;
	int NRES;
	int RES_DEF;
	ResRec* RES;
	int NRelief;
	int Relief_DEF;
	char** Relief;
	int NSTY;
	int STY_DEF;
	StyleRec* STY;
	RandomMapDesc();
	~RandomMapDesc();
	void Close();
	void Load(char* Name);
};
class GlobalProgress{
public:
	int NWeights;
	int StageID[64];
	int StageWeight[64];
	int StagePositions[64];
	int CurStage;
	int CurPosition;
	int MaxPosition;
	void Setup();
	void AddPosition(int ID,int Weight,int Max);
	void SetCurrentStage(int ID);
	void SetCurrentPosition(int Pos);
	int GetCurProgress();
	GlobalProgress();
};
extern GlobalProgress GPROG;