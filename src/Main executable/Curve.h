bool CheckCurve();
void AddPointToCurve(int x, int y, bool Final, byte Type);
void DrawCurves();
void AddPointToCurve(int x, int y, bool Final, byte Type);
bool CheckPointInside(int x, int y);
int GetCurveDistance1(int x, int y);
int GetCrossProportion(int x0, int y0, int x1, int y1);
void SetTextureInCurve();
void SetPlatoInCurve();
void SoftRegion();
void ClearCurve();
extern int PlatoHi;
extern int SurfType;

class PaintHills
{
public:
	word NLayers;
	word TexAmount[32];
	word Texs[32][8];
	word TexStartHi[32];
	word TexEndHi[32];
	word TexStartAng[32];
	word TexEndAng[32];
	byte CondType[32];
	byte Param1[32];
	byte Param2[32];
	int NHiSections;
	int HiSect[8];
};

void PaintAllMap(byte* VertHi, byte* VertType, PaintHills* PHL, int NTypes);
void SetPlatoInCurve(byte* VertHi, byte* VertType, byte Type);
void SoftRegion(byte * MpVertHi);
