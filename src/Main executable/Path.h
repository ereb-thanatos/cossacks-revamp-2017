class MotionBrush{
public:
	int* Data;
	int Lx,Ly;
	int Dx;
	int Dy;
	MotionBrush();
	~MotionBrush();
	void CreateRoundBrush(int d);
	void CreateSquareBrush(int L);
};
class MotionField{
public:
	//byte* MapH;//(x,y)-bit matrix(1point=16x12 pix)
	byte* MapV;//(y,x)-T bit matrix //1024*128=131072 bytes big
	//----Methods-----
	void Allocate();
	void FreeAlloc();
	void ClearMaps();
	void BSetPt(int x,int y);
	void BClrPt(int x,int y);
	void BSetBar(int x,int y,int Lx);
	void BClrBar(int x,int y,int Lx);
	void BSetSQ(int x,int y,int Lx,int Ly);
	void BClrSQ(int x,int y,int Lx,int Ly);
	int  CheckPt(int x,int y);
	int  CheckHLine(int x,int y,int Lx);
	int  CheckVLine(int x,int y,int Lx);
	bool CheckBar(int x,int y,int Lx,int Ly);
	void FillNewCirc(int x,int y,int Lx,int Ly);


	/*
	MotionField();
	~MotionField();
	void CreateField(int Lx,int Ly);
	void DeleteField();
	void DrawPoint(int x,int y,bool Color);//Color=0,1
	void DrawBar(int x,int y,int Lx,int Ly,bool Color);
	void DrawBrush(int x,int y,MotionBrush* Brush,bool Color);
	bool CheckBrush(int x,int y,MotionBrush* MB);
	int FindWay(int x,int y,int x1,int y1,MotionBrush* Brush,short** WayX,short** WayY,int Step,int Border);
	*/
};
extern MotionField MFIELDS[2];//0-Land,1-Water