struct BlockCell{
	word x;
	word y;
};

class BlockBars{
public:
	int NBars;
	int MaxBars;
	BlockCell* BC;
	BlockBars();
	~BlockBars();
	void Clear();
	bool Add(word x,word y);
	bool FastAdd(word x,word y);
	bool Delete(word x,word y);
};
void AddLockBar(word x,word y);
void AddUnLockbar(word x,word y);
void AddDefaultBar(word x,word y);
void SetTexturedRound(int x,int y,int r,byte Tex);
int ConvScrY(int x,int y);
extern int NCurves;
void AddPointToCurve(int x,int y,bool Final,byte Type);
extern word TexList[128];
extern word NTextures;