class OneIcon{
public:
	word FileID;
	word SpriteID;
	//Type:
	//0  -simple icon
	//1  -icon with colored rectangle
	//2  -icon with Health bar
	//4  -icon with stage bar
	//8  -icon with integer value
	//16 -icon with hint
	//32 -right button active
	//64 -left  button active
    //128-memory for hint was allocated
	byte  Color;
	int   Health;
	int   MaxHealth;
	int   Stage;
	int   MaxStage;
	int   IntVal;
	byte  Level;
	bool  Disabled:1;
	bool  Selected:1;
	word MoreSprite;
    word Type;
	bool Visible:1;
	bool NoPress:1;
	char* Hint;
	char* HintLo;
    HandlePro* LPro;
    HandlePro* RPro;
	HandlePro* MoveOver;
    int   LParam;
    int   RParam;
	int   MoveParam;
	int   IntParam;

    void InitIcon();
    void AssignIcon(word FileID,word SpriteID);
    void AssignHint(char* str);
    void CreateHint(char* str);
	void CreateHintLo(char* str);
    void AssignColor(byte c);
    void AssignHealth(int Health,int MaxHeath);
    void AssignStage(int Stage,int MaxStage);
    void AssignIntVal(int ival);
    void AssignLeft(HandlePro* Lpro,int param);
    void AssignRight(HandlePro* Hpro,int param);
	void AssignMoveOver(HandlePro* Hpro,int param);
	void AssignLevel(byte Level);
	void AssignIntParam(int i);
	void AssignRedPulse();
	void SelectIcon();
	void Disable();
    void Draw(int x,int y);
};
class IconSet{
public:
    int Space;
    OneIcon* Icons;
    int NIcons;
    IconSet();
    ~IconSet();
    void InitIconSet();
    void ClearIconSet();
	OneIcon* AddIconFixed(word FileID,word SpriteID,int Index);
    OneIcon* AddIcon(word FileID,word SpriteID);
    OneIcon* AddIcon(word FileID,word SpriteID,char* Hint);
    OneIcon* AddIcon(word FileID,word SpriteID,char* Hint,byte Color);
    OneIcon* AddIcon(word FileID,word SpriteID,byte Color,char* Hint);
	int GetMaxX(int Lx);
    void DrawIconSet(int x,int y,int Nx,int Ny,int NyStart);
};