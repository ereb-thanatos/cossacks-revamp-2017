extern int StratLx;
extern int StratLy;
extern int StratSH;
class CellInfo{
public:
	word NSword;
	word NStrel;
	word NCaval;
	word NPush;
	word NMort;
	word NTow;
	word Total;
	byte x;
	byte y;
	word ArmyID;
	byte Changed;
};
class ArmyInfo{
public:
	word NSword;
	word NStrel;
	word NCaval;
	word NPush;
	word NMort;
	word NTow;
	//word NPeon;
	byte MinX;
	byte MinY;
	byte MaxX;
	byte MaxY;
	word N;
};
class GlobalArmyInfo{
public:
	word* ArmDistr;
	CellInfo* CIN;
	int NCIN;
	int MaxCIN;
	ArmyInfo* AINF;
	int NArmy;
	int MaxArmy;
	GlobalArmyInfo();
	~GlobalArmyInfo();
	void Clear();
	void ResearchArmyDistribution(byte NI);
	void Show(int x,int y);
};
