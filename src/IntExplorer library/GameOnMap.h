#ifndef BMPTOOL
#include "bmptool.h"
#endif
struct TempWindow{
	int WindX; 
	int WindY;
	int WindLx;
	int WindLy;
	int WindX1;
	int WindY1; 
	int ScrWidth;
};
void PushWindow(TempWindow* W);
void PopWindow(TempWindow* W);
void IntersectWindows(int x0,int y0,int x1,int y1);
struct OneScale{
	byte* Data;
	byte* ColoredData;
	char FONT[16];
	RLCFont* Font;
	int Lx;
	int Ly;
	int Scale;
	bool Loaded;
	bool Colored;
};
struct OneMapPicture{
	int  x,y;
	int  dx,dy;
	int  GPID   [16];
	int  SCALE  [16];
	int  NScales;
	int  Sprite;
	bool Drawn;
	int  LastX;
	int  LastY;
	int  LastGPID;
	char URL[256];
};
class OneMap{
public:
	char Title[32];
	OneScale* SCALE;
	int NScales;
	byte* CountryMap;
	byte ColorTBL[65536];
	byte CORIG[256];
	PAL3 COLORS[256];
	char* Names[256];
	int XSumm[256];
	int YSumm[256];
	int NPoints[256];

	int CountryLx;
	int CountryLy;
	DWORD LastUpdateDate;
	DWORD UpdateData;
	int WX,WY,WLX,WLY;

	int VCenterX;
	int VCenterY;
	int CurScale;

	int CurX;
	int CurY;
	int CurLx;
	int CurLy;

	int NMapPix;
	int MaxMapPix;
	OneMapPicture* MapPix;

	void SetCenterCoor(int x,int y);
	void RelativeMove(int dx,int dy);
	void ChangeScale(int Direction);

	void NormalizeCoordinates();

	void SetTitle(char* title){strcpy(Title,title);};
	void LoadMapData(sicExplorer* SXP);
	void UpdateMapData(byte* Data);
	void SetOutput(int x,int y,int lx,int ly){WX=x;WY=y;WLX=lx;WLY=ly;};
	void ShowMapPart();
	void ApplyColorsToScaleLayer(int Idx);
	void ClearAll();
	void AddMapPicture(char* Name,int x,int y,int dx,int dy,int Sprite);
	void ClearMapPictures();
	OneMap();
	~OneMap();
};
extern OneMap BIGMAP;
extern bool HaveExComm;

