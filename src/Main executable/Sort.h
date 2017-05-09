class SortClass{
public:
    word* Uids;
    //word* Usn;
    int*  Parms;
    int NUids;
    int MaxUID;
    SortClass();
    ~SortClass();
    void Sort();
    void CheckSize(int Size); 
    word CreateByX(word* UIDS,int NUIDS,bool Direction);
    word CreateByY(word* UIDS,int NUIDS,bool Direction);
    word CreateByR(word* UIDS,int NUIDS,bool Direction,int x,int y);
	word CreateByLine(word* UIDS,int NUIDS,int dx,int dy);
	word CreateByLineForShips(word* UIDS,int NUIDS,int dx,int dy);
    void CopyToDst(word* Dst,word* Sns);
    void Copy(word* Dst);
};
extern SortClass UNISORT;
word SortUnitsByR(word* urefs,word* usn,int nu,int x,int y);
class PositionOrder{
public:
    int NUnits;
    int MaxUnit;
	int CenterX;
	int CenterY;
    word* Ids;
    int* px;
    int* py;
    PositionOrder();
    ~PositionOrder();
    void CheckSize(int Size);
    void Create(word* IDS,int NIDS);
    word CreatePositions(int x,int y,word* IDS,int NIDS);
	word CreateLinearPositions(int x,int y,word* IDS,int NIDS,int dx,int dy);
	word CreateRotatedPositions(int x,int y,word* IDS,int NIDS,int dx,int dy);
	word CreateRotatedPositions2(int x,int y,word* IDS,int NIDS,int dx,int dy);
	word CreateOrdPos(int x,int y,byte dir,int NIDS,word* IDS,OrderDescription* ODS);
	word CreateSimpleOrdPos(int x,int y,byte dir,int NIDS,word* IDS,OrderDescription* ODS);
    void SendToPosition(byte Prio,byte OrdType);
	void Patrol();
};
void GroupSendSelectedTo(byte NI,int x,int y,byte Prio,byte OrdType);
extern PositionOrder PORD;