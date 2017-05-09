#define SafeCellSH 2
#define SafeCLX (1<<SafeCellSH)
#define SafeMask (SafeCLX-1)
#define SafeCN (SafeCLX*SafeCLX)
#define SafeLX (TopLx>>SafeCellSH)
#define SafeSX (TopSH-SafeCellSH)
#define SafeSize (SafeLX*SafeLX)
extern int CurRStage;
struct SafeCellInfo{
	word Index,SN;
	word Prio,BusyIndex,BusySN;
};
void ResearchSafeCells(int MinR,int MaxR);
void InitResearchSafeCell();
SafeCellInfo* GetCellInfo(int x,int y);
void InitSafeInfo();
void CloseSafeInfo();
void GrenaderSuperman(OneObject* OB);
