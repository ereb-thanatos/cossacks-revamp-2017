class VirtualScreen
{
public:
	int RealVLx;
	int RealVLy;

	int CellSX;
	int CellSY;
	int CellNX;
	int CellNY;
	int NCells;
	int ShiftsPerCellX;
	int ShiftsPerCellY;
	byte* CellQuotX;
	byte* CellQuotY;
	byte* CellFlags;
	int* TriangMap;
	int* LoTriMap;
	int MaxTMX;
	int MaxTMY;
	int Lx;
	int Ly;
	byte* MarkedX;
	bool Grids;
	byte* VirtualScreenPointer;
	VirtualScreen();
	~VirtualScreen();
	void CopyVSPart( int vx, int vy, int sx, int sy, int SizeX, int SizeY );
	void CopyVSPartMMX( int vx, int vy, int sx, int sy, int SizeX, int SizeY );
	void SetVSParameters( int Lx, int Ly );
	void CopyVSToScreen();
	void CreateTrianglesMapping();
	void CreateVerticalTrianglesMapping( int VertSet );
	void RenderVerticalSet( int QuotX, int QuotY, int cx, int cy, int cly );
	void ShowVerticalGrids( int QuotX, int QuotY, int cx, int cy, int cly );
	void RenderVSPart( int QuotX, int QuotY, int cx, int cy, int clx, int cly );
	void RefreshSurface();
	int  ShowLimitedSector( int i, bool Mode3D, int HiLine, int LoLine, int QuotX, int QuotY );
	void Execute();
	void Zero();
	void RefreshTriangle( int i );
	void RefreshScreen();
	void Sequrity();
	void SetSize( int Lx, int Ly );
	void CheckVLINE( int Vert );
};
extern VirtualScreen SVSC;