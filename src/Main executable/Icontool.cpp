#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "mode.h"
#include "Nature.h"
#include "fonts.h"

#include "walls.h"
#include "Megapolis.h"
#include "GSound.h"
#include "IconTool.h"
#include "GP_Draw.h"
#include <math.h>
int CreateRZone( int x, int y, int lx, int ly, HandlePro* HPro, HandlePro* RHPro, int Index, char* Hint, char* HintLo );
int CreateZone( int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint, char* HintLo );
extern int IconLx;
extern int IconLy;
extern int AddIconLx;
extern int AddIconLy;
extern byte GraySet[256];
void CBar( int x, int y, int Lx, int Ly, byte c );
extern byte KeyCodes[512][2];
#define NKEYS 61
extern char* KeyNames[NKEYS];
extern byte ScanKeys[NKEYS];
void OneIcon::InitIcon()
{
	FileID = 0xFFFF;
	SpriteID = 0xFFFF;
	Type = 0;
	Disabled = false;
	Hint = NULL;
	HintLo = NULL;
	RPro = NULL;
	LPro = NULL;
	MoveOver = NULL;
	Level = 0;
	Selected = false;
	MoreSprite = 0xFFFF;
};
void OneIcon::AssignIcon( word FID, word SID )
{
	Type = 0;
	Disabled = false;
	RPro = NULL;
	LPro = NULL;
	MoveOver = NULL;
	Level = 0;

	FileID = FID;
	SpriteID = SID;
	Selected = false;
	MoreSprite = 0xFFFF;
};
void OneIcon::AssignHint( char* str )
{
	Type |= 16;
	Hint = str;
};
void OneIcon::SelectIcon()
{
	Selected = true;
};
extern bool EditMapMode;
void OneIcon::CreateHint( char* str )
{
	Type |= 16 + 128;
	if (Hint)
	{
		//free(Hint);
		//Hint=NULL;
	};
	//Hint=new char[strlen(str)+1];
	if (!Hint)Hint = new char[512];
	strcpy( Hint, str );
	if (HintLo)
	{
		free( HintLo );
		HintLo = NULL;
	};
	if (( !EditMapMode ) && SpriteID != -1 && KeyCodes[SpriteID][0])
	{
		strcat( Hint, " (" );
		byte Code = KeyCodes[SpriteID][0];
		byte State = KeyCodes[SpriteID][1];
		if (State & 1)strcat( Hint + strlen( Hint ), "CTRL+" );
		if (State & 2)strcat( Hint + strlen( Hint ), "ALT+" );
		if (State & 4)strcat( Hint + strlen( Hint ), "SHIFT+" );
		strcat( Hint + strlen( Hint ), KeyNames[Code] );
		strcat( Hint, ")" );
	};
};
void OneIcon::CreateHintLo( char* str )
{
	Type |= 16 + 128;
	if (HintLo)
	{
		//free(HintLo);
		//HintLo=NULL;
	};
	//HintLo=new char[strlen(str)+1];
	if (!HintLo)HintLo = new char[512];
	strcpy( HintLo, str );

};
void OneIcon::AssignColor( byte c )
{
	Type |= 1;
	Color = c;
};
void OneIcon::AssignHealth( int H, int MH )
{
	Type |= 2;
	Health = H;
	MaxHealth = MH;
};
void OneIcon::AssignStage( int S, int MS )
{
	Type |= 4;
	Stage = S;
	MaxStage = MS;
}

void OneIcon::AssignLeft( HandlePro* Lpro, int param )
{
	Type |= 32;
	LPro = Lpro;
	LParam = param;
}

void OneIcon::AssignMoveOver( HandlePro* pro, int param )
{
	MoveOver = pro;
	MoveParam = param;
}

void OneIcon::AssignRight( HandlePro* Rpro, int param )
{
	Type |= 64;
	RPro = Rpro;
	RParam = param;
}

void OneIcon::AssignIntVal( int ival )
{
	if (ival < 1)return;
	Type |= 8;
	IntVal = ival;
}

void OneIcon::AssignIntParam( int i )
{
	Type |= 256;
	IntParam = i;
};
void OneIcon::AssignRedPulse()
{
	Type |= 512;
};
void OneIcon::Disable()
{
	Disabled = true;
	Type &= ~( 32 | 64 );
	LPro = NULL;
	RPro = NULL;
	MoveOver = NULL;
};
void OneIcon::AssignLevel( byte L )
{
	Level = L;
};
extern byte graysc[256];
extern bool realLpressed;
bool CheckSpritePressed( int sp );
void xLine( int x, int y, int x1, int y1, byte c );
void DrawInf( int x0, int y0, byte c )
{
	xLine( x0 + 2, y0, x0 + 4, y0, c );
	xLine( x0 + 4, y0, x0 + 10, y0 + 6, c );
	xLine( x0 + 10, y0 + 6, x0 + 12, y0 + 6, c );
	xLine( x0 + 12, y0 + 6, x0 + 14, y0 + 4, c );
	xLine( x0 + 14, y0 + 4, x0 + 14, y0 + 2, c );
	xLine( x0 + 14, y0 + 2, x0 + 12, y0, c );
	xLine( x0 + 12, y0, x0 + 10, y0, c );
	xLine( x0 + 10, y0, x0 + 4, y0 + 6, c );
	xLine( x0 + 4, y0 + 6, x0 + 2, y0 + 6, c );
	xLine( x0 + 2, y0 + 6, x0, y0 + 4, c );
	xLine( x0, y0 + 4, x0, y0 + 2, c );
	xLine( x0, y0 + 2, x0 + 2, y0, c );
};
void OneIcon::Draw( int x, int y )
{
	if (( !NoPress ) && ( ( realLpressed&&mouseX<x + IconLx&&mouseY<y + IconLy&&mouseX >= x&&mouseX>x&&mouseY>y ) ||
		CheckSpritePressed( SpriteID ) ))
	{
		x++;
		y++;
	};
	if (FileID != 0xFFFF && SpriteID != 0xFFFF)
	{
		if (Disabled)GPS.ShowGPTransparent( x, y, FileID, SpriteID, NatRefTBL[MyNation] );//.ShowGPPal(x,y,FileID,SpriteID,MyNation,GraySet);
		else
		{
			if (Type & 512)GPS.ShowGPPal( x, y, FileID, SpriteID, NatRefTBL[MyNation],/*9+int(8.0*sin(float(GetTickCount())/100)),*/graysc );
			else GPS.ShowGP( x, y, FileID, SpriteID, NatRefTBL[MyNation] );
		};
	}
	else
	{
		CBar( x, y, IconLx, IconLy, 0 );
	};
	if (Type & 1)
	{//Rectangle
		Xbar( x, y, IconLx + 1, IconLy + 1, Color );
		if (( Type & 2 ) && Health > 0)Hline( x + 1, y + IconLy - 3, x + IconLx - 1, Color );
	};
	if (Selected)
	{
		Xbar( x - 1, y - 1, IconLx + 1 + 2, IconLy + 1 + 2, clrYello );
	};
	if (( Type & 2 ) && MaxHealth > 0 && Health > 0)
	{//Health bar
		int L = div( ( IconLx - 2 )*Health, MaxHealth ).quot;
		Hline( x + 1, y + IconLy - 2, x + L + 1, clrWhite );
		Hline( x + 1, y + IconLy - 1, x + L + 1, clrWhite );
		if (Health < MaxHealth)
		{
			Hline( x + L + 2, y + IconLy - 2, x + IconLx - 1, 0 );
			Hline( x + L + 2, y + IconLy - 1, x + IconLx - 1, 0 );
		};

	};
	if (( Type & 4 ) && MaxStage > 0 && Stage > 0)
	{//Stage bar
		int L = div( ( IconLx - 2 )*Stage, MaxStage ).quot;
		Hline( x + 1, y + IconLy - 5, x + L + 1, clrWhite );
		Hline( x + 1, y + IconLy - 4, x + L + 1, clrWhite );
		if (Stage < MaxStage)
		{
			Hline( x + L + 2, y + IconLy - 5, x + IconLx - 1, 0 );
			Hline( x + L + 2, y + IconLy - 4, x + IconLx - 1, 0 );
		};
	};
	if (Type & 256)
	{
		char ss[16];
		sprintf( ss, "%d", IntParam );
		int L = GetRLCStrWidth( ss, &WhiteFont );
		int H = GetRLCHeight( WhiteFont.RLC, 'W' );
		ShowString( x + IconLx - 4 - L + 1, y + IconLy - H + 1, ss, &BlackFont );
		ShowString( x + IconLx - 4 - L - 1, y + IconLy - H - 1, ss, &BlackFont );
		ShowString( x + IconLx - 4 - L, y + IconLy - H, ss, &WhiteFont );
	};
	if (Type & 8)
	{//integer value
		if (IntVal >= 1200)
		{
			byte c = 0xA5;
			int x0 = x + 3;
			int y0 = y + 3;
			DrawInf( x0 + 2, y0 + 2, 0 );
			//DrawInf(x0,y0,0xAE);
			//DrawInf(x0+1,y0,0xAE);
			//DrawInf(x0,y0+1,0xAE);
			DrawInf( x0 + 1, y0 + 1, 0xAE );
			int N = IntVal / 1200;
			if (N > 1)
			{
				char ss[16];
				sprintf( ss, "%d", N );
				int L = GetRLCStrWidth( ss, &YellowFont );
				ShowString( x + IconLx - 2 - L + 1, y + 1 + 1, ss, &BlackFont );
				ShowString( x + IconLx - 2 - L - 1, y + 1 - 1, ss, &BlackFont );
				ShowString( x + IconLx - 2 - L, y + 1, ss, &YellowFont );
			};
		}
		else
		{
			char ss[16];
			sprintf( ss, "%d", IntVal );
			ShowString( x + 1 + 1, y + 1 + 1, ss, &BlackFont );
			ShowString( x + 1 - 1, y + 1 - 1, ss, &BlackFont );
			ShowString( x + 1, y + 1, ss, &WhiteFont );
		};
	};
	if (Level > 1)
	{
		char cc[2];
		cc[1] = 0;
		cc[0] = Level;
		ShowString( x + IconLx - GetRLCStrWidth( cc, &fn10 ) - 2, y + IconLy - 11, cc, &fn10 );
	};
	if (MoreSprite != 0xFFFF)GPS.ShowGP( x, y, FileID, MoreSprite, NatRefTBL[MyNation] );
};
IconSet::IconSet()
{
	NIcons = 0;
	Space = 64;
	Icons = new OneIcon[Space];
	for (int i = 0; i < Space; i++)
	{
		Icons[i].Visible = false;
		Icons[i].Hint = NULL;
		Icons[i].HintLo = NULL;
	};
};
IconSet::~IconSet()
{
	ClearIconSet();
	if (Space)free( Icons );
	NIcons = 0;
	Space = 0;
};
void IconSet::InitIconSet()
{
	NIcons = 0;
};
void IconSet::ClearIconSet()
{
	for (int i = 0; i < NIcons; i++)
	{
		OneIcon* OI = Icons + i;
		if (OI->Hint)
		{
			//free(OI->Hint);
			//OI->Hint=NULL;
		};
		if (OI->HintLo)
		{
			//free(OI->HintLo);
			//OI->HintLo=NULL;
		};
		OI->Visible = false;
	};
	NIcons = 0;
}

OneIcon* IconSet::AddIcon( word FileID, word SpriteID )
{
	int Index = -1;
	for (int i = 0; i < NIcons; i++)
	{
		if (Index == -1 && !Icons[i].Visible)
		{
			Index = i;
		}
	}

	if (Index == -1)
	{
		if (NIcons >= Space)
		{
			OneIcon* Ico = new OneIcon[NIcons + 8];
			if (Space > 0)
			{
				memcpy( Ico, Icons, NIcons * sizeof OneIcon );
				free( Icons );
			}
			Space = NIcons + 8;
			Icons = Ico;
		}
		Index = NIcons;
		NIcons++;
	}

	OneIcon* OI = Icons + Index;
	OI->AssignIcon( FileID, SpriteID );
	OI->Visible = true;
	return OI;
}

OneIcon* IconSet::AddIconFixed( word FileID, word SpriteID, int Index )
{
	if (Index == -1)
	{
		return AddIcon( FileID, SpriteID );
	}

	OneIcon* OI = Icons + Index;

	if (Index >= NIcons)
	{
		NIcons = Index + 1;
	}

	OI->AssignIcon( FileID, SpriteID );
	OI->Visible = true;
	return OI;
}

OneIcon* IconSet::AddIcon( word FileID, word SpriteID, char* Hint )
{
	OneIcon* OI = AddIcon( FileID, SpriteID );
	OI->CreateHint( Hint );
	return OI;
}

OneIcon* IconSet::AddIcon( word FileID, word SpriteID, char* Hint, byte Color )
{
	OneIcon* OI = AddIcon( FileID, SpriteID, Hint );
	OI->AssignColor( Color );
	return OI;
};
OneIcon* IconSet::AddIcon( word FileID, word SpriteID, byte Color, char* Hint )
{
	OneIcon* OI = AddIcon( FileID, SpriteID, Hint );
	OI->AssignColor( Color );
	return OI;
};
void AssignMovePro( int i, HandlePro* HPro, int id );
void AssignKeys( int i, byte Scan, byte State );
extern IconSet PrpPanel;
void IconSet::DrawIconSet( int x, int y, int Nx, int Ny, int NyStart )
{
	bool DOKEYS = this != &PrpPanel || NIcons != 1;
	int start = NyStart*Nx;
	if (start >= NIcons)return;
	int n = NIcons - start;
	if (n > Nx*Ny)n = Nx*Ny;
	int xx = 0;
	int yy = 0;
	int x1 = x;
	int y1 = y;
	for (int i = 0; i < n; i++)
	{
		OneIcon* OI = Icons + i;
		if (OI->Visible)
		{
			OI->Draw( x1, y1 );
			int p = -1;
			if (OI->LPro && !OI->RPro)p = CreateZone( x1, y1, IconLx, IconLy, OI->LPro, OI->LParam, OI->Hint, OI->HintLo );
			if (OI->RPro)p = CreateRZone( x1, y1, IconLx, IconLy, OI->LPro, OI->RPro, OI->RParam, OI->Hint, OI->HintLo );
			if (p != -1 && OI->MoveOver)
			{
				AssignMovePro( p, OI->MoveOver, OI->MoveParam );
			};
			if (DOKEYS&&p != -1 && OI->SpriteID != -1 && KeyCodes[OI->SpriteID][0])
			{
				AssignKeys( p, ScanKeys[KeyCodes[OI->SpriteID][0]], KeyCodes[OI->SpriteID][1] );
			};
			OI->NoPress = !DOKEYS;
		};
		xx++;
		x1 += IconLx + AddIconLx;
		if (xx >= Nx)
		{
			xx = 0;
			x1 = x;
			y1 += IconLy + AddIconLy;
			yy++;
		};
	};
};
int IconSet::GetMaxX( int Lx )
{
	int Max = -1;
	for (int i = 0; i < NIcons; i++)if (Icons[i].Visible)
	{
		int x = i%Lx;
		if (x > Max)Max = x;
	};
	return Max + 1;
};