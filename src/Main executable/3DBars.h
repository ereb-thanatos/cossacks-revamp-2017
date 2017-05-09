struct OneBar
{
	int x0,y0,x1,y1;
	short MinBX,MinBY,MaxBX,MaxBY;
	short H;
	word Owner;
	word ID;
};

void DrawBar3D(OneBar* ONB);
extern OneBar* OBARS[8192];
extern int NBars;
#define B3SHIFT 8
extern int B3SX;
extern int B3SY;
extern int B3SZ;

extern word** Obj3Map;
extern word*  NObj3;

void InitObjs3();
void ClearObjs3();
word Add3DBar(int X0,int Y0,int X1,int Y1,short H,int ID,word Owner);
void Delete3DBar(word ID);
int GetBar3DHeight(int x,int y);
int GetBar3DOwner(int x,int y);