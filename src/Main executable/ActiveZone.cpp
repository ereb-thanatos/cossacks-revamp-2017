#include "ddini.h"
#include <stdlib.h>
#include "ResFile.h"
#include "Fastdraw.h"
#include "MapDiscr.h"
#include "mouse.h"
#include "mode.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <crtdbg.h>
#include <math.h>

#include "Megapolis.h"

#include "fonts.h"
#include "WeaponID.h"

#include "3DSurf.h"
#include "GSound.h"
#include <assert.h>
#include "3DMapEd.h"
#include "MapSprites.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "ZBuffer.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "StrategyResearch.h"
#include "Curve.h"
#include "NewMon.h"
#include "ActiveZone.h"

ActiveGroup* AGroups = NULL;
int NAGroups = 0;
int MaxAGroups = 0;

ActiveZone* AZones = NULL;
int NAZones = 0;
int MaxAZones = 0;

void DrawPlaneLine( int x0, int y0, int x1, int y1, byte c );
void Draw3DRound( int x, int y, int r )
{
	int N = ( ( r * 6 ) >> 5 ) + 5;
	double Rd = r;
	int xp = x + r;
	int yp = y;
	for (int i = 1; i <= N; i++)
	{
		double Angle = i * 2 * 3.1415926535 / N;
		int x1 = x + int( Rd*cos( Angle ) );
		int y1 = y + int( Rd*sin( Angle ) );
		DrawPlaneLine( xp, yp, x1, y1, clrYello );
		xp = x1;
		yp = y1;
	};
};
extern int RealLx;
extern int RealLy;
void ActiveZone::Draw()
{
	int xx = x - ( mapx << 5 );
	int yy = ( y >> 1 ) - ( mapy << 4 ) - GetHeight( x, y );
	if (xx<-R || yy<-R || xx>RealLx + R || yy>RealLy + R)return;
	int L = GetRLCStrWidth( Name, &BigYellowFont );
	int L2 = L >> 1;
	int H = GetRLCHeight( BigYellowFont.RLC, 'W' );
	Draw3DRound( x, y, R );

	DrawLine( xx - 1, yy, xx - 1, yy - 48, clrWhite );
	DrawLine( xx, yy, xx, yy - 48, clrWhite );
	DrawLine( xx + 1, yy, xx + 1, yy - 48, clrWhite );

	DrawLine( xx - L2 - 8, yy - 48, xx + L2 + 8, yy - 48, clrWhite );
	DrawLine( xx - L2 - 8, yy - 48 - H - 10, xx + L2 + 8, yy - 48 - H - 10, clrWhite );
	DrawLine( xx - L2 - 8, yy - 48, xx - L2 - 8, yy - 48 - H - 10, clrWhite );
	DrawLine( xx + L2 + 8, yy - 48, xx + L2 + 8, yy - 48 - H - 10, clrWhite );

	ShowString( xx - L2, yy - 3 - 48 - H, Name, &BigYellowFont );
};
void DrawAGroups();
void DrawAZones()
{
	for (int i = 0; i < NAZones; i++)AZones[i].Draw();
	DrawAGroups();
};
char ZoneName[64] = "Zone1";
char GroupName[64] = "Group1";
int CurrentZone = -1;
int ZActionType = 0;
int CreateNewZone( char* Name, int x, int y, int R )
{
	if (NAZones >= MaxAZones)
	{
		MaxAZones += 32;
		AZones = (ActiveZone*) realloc( AZones, MaxAZones * sizeof ActiveZone );
	};
	AZones[NAZones].x = x;
	AZones[NAZones].y = y;
	AZones[NAZones].Name = new char[strlen( Name ) + 1];
	strcpy( AZones[NAZones].Name, Name );
	AZones[NAZones].R = R;
	NAZones++;
	return NAZones - 1;
};
bool DeleteActiveGroup();
void DeleteAZone( int id )
{
	if (id < NAZones)
	{
		free( AZones[id].Name );
		if (id < NAZones - 1)
		{
			memcpy( AZones + id, AZones + id + 1, ( NAZones - id - 1 ) * sizeof ActiveZone );
		};
		NAZones--;
	};
};
bool DelCurrentAZone()
{
	if (!DeleteActiveGroup())
	{
		if (CurrentZone != -1)
		{
			DeleteAZone( CurrentZone );
			CurrentZone = -1;
			return true;
		};
		return false;
	}
	else return true;
};
extern bool KeyPressed;
extern int LastKey;
extern bool realLpressed;
bool EnterStr( char * str, char* Message );
void ProcessAGroups( int x, int y );
void CreateNewActiveGroup( char* gName );

int ZoneCmd = 0;
bool WasUnpress = 0;
void UnPress();
void ClearMStack();
void ControlAZones( int x, int y )
{
	if (!GetKeyState( VK_SCROLL ))
	{
		ZoneCmd = 0;
		return;
	};
	ProcessAGroups( x, y );
	if (( KeyPressed&&LastKey == 13 && ( GetKeyState( VK_CONTROL ) & 0x8000 ) ) || ZoneCmd)
	{
		if (NSL[MyNation] && ZoneCmd != 1)
		{
			if (EnterStr( GroupName, "New Group" ))
			{
				bool Err = false;
				for (int i = 0; i < NAGroups; i++)if (!strcmp( AGroups[i].Name, GroupName ))
				{
					Err = true;
				}
				if (!Err)
					CreateNewActiveGroup( GroupName );
			};
		};
		if (ZoneCmd != 2 && !NSL[MyNation])
		{
			if (EnterStr( ZoneName, "New Zone" ))
			{
				bool Err = false;

				for (int i = 0; i < NAZones; i++)if (!strcmp( AZones[i].Name, ZoneName ))
				{
					Err = true;
				}

				Lpressed = 0;
				UnPress();
				ClearMStack();
				if (!Err)
				{
					WasUnpress = 1;
					//if(ZoneCmd){
					//	CurrentZone=CreateNewZone(ZoneName,mouseX,mouseY,0);
					//}else{
					CurrentZone = CreateNewZone( ZoneName, x, y, 0 );
					//};
				};
				ZActionType = 0;
			};
		};
		KeyPressed = false;
	};
	ZoneCmd = 0;
	if (!realLpressed)WasUnpress = 0;
	if (WasUnpress)return;
	if (realLpressed && !WasUnpress)
	{
		if (CurrentZone != -1)
		{
			switch (ZActionType)
			{
			case 0:
				ZActionType = 1;
				Lpressed = false;
				break;
			case 1:
			{
				int x0 = AZones[CurrentZone].x;
				int y0 = AZones[CurrentZone].y;
				AZones[CurrentZone].R = int( sqrt( ( x - x0 )*( x - x0 ) + ( y - y0 )*( y - y0 ) ) );
				Lpressed = false;
			};
			break;
			case 2:
				AZones[CurrentZone].x = x;
				AZones[CurrentZone].y = y;
				Lpressed = false;
				break;
			};
		}
		else
		{
			//search for zone
			for (int i = 0; i < NAZones; i++)
			{
				ActiveZone* AZ = AZones + i;
				int r = int( sqrt( ( x - AZ->x )*( x - AZ->x ) + ( y - AZ->y )*( y - AZ->y ) ) );
				if (abs( r - AZ->R ) < 16)
				{
					CurrentZone = i;
					ZActionType = 1;
					Lpressed = false;
				}
				else if (r < 16)
				{
					CurrentZone = i;
					ZActionType = 2;
					Lpressed = false;
				};
			};
		};
	}
	else
	{
		if (CurrentZone != -1)
		{
			switch (ZActionType)
			{
			case 0:
				AZones[CurrentZone].x = x;
				AZones[CurrentZone].y = y;
				break;
			case 1:
			case 2:
				CurrentZone = -1;
				break;
			};
		};
	};

};
int CurrentAGroup = -1;
void ActiveGroup::Draw( byte c )
{
	MinX = 10000000;
	MinY = 10000000;
	MaxX = 0;
	MaxY = 0;

	AminX = 10000000;
	AminY = 10000000;
	AmaxX = 0;
	AmaxY = 0;
	for (int i = 0; i < N; i++)
	{
		word MID = Units[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB&&OB->Serial == Serials[i])
			{
				int x = OB->RealX >> 4;
				int y = ( OB->RealY >> 5 ) - OB->RZ;
				if (x < MinX)MinX = x;
				if (x > MaxX)MaxX = x;
				if (y < MinY)MinY = y;
				if (y > MaxY)MaxY = y;
				x = OB->RealX >> 10;
				y = OB->RealY >> 10;
				if (x < AminX)AminX = x;
				if (x > AmaxX)AmaxX = x;
				if (y < AminY)AminY = y;
				if (y > AmaxY)AmaxY = y;
			};
		};
	};
	if (MinX <= MaxX)
	{
		MinX -= 40;
		MaxX += 40;
		MinY -= 40;
		MaxY += 40;
		int dx = mapx << 5;
		int dy = mapy << 4;
		int x0 = MinX - dx;
		int y0 = MinY - dy;
		int x1 = MaxX - dx;
		int y1 = MaxY - dy;
		DrawLine( x0, y0, x1, y0, c );
		DrawLine( x1, y0, x1, y1, c );
		DrawLine( x1, y1, x0, y1, c );
		DrawLine( x0, y0, x0, y1, c );

		int L = GetRLCStrWidth( Name, &BigYellowFont );
		int xc = ( x0 + x1 ) >> 1;
		int yc = y0 - 40;
		if (xc > -64 && xc<RealLx + 64 && yc>-64 && yc < RealLy + 64)
		{
			ShowString( xc - ( L >> 1 ), yc, Name, &BigYellowFont );
		};
		Visible = true;
	}
	else Visible = false;
};
void DrawAGroups()
{
	for (int i = 0; i < NAGroups; i++)
	{
		if (i == CurrentAGroup)AGroups[i].Draw( clrWhite );
		else AGroups[i].Draw( clrYello );
	};
};
void CreateNewActiveGroup( char* gName )
{
	if (NAGroups >= MaxAGroups)
	{
		MaxAGroups += 32;
		AGroups = (ActiveGroup*) realloc( AGroups, MaxAGroups * sizeof ActiveGroup );
	};
	ActiveGroup* AG = AGroups + NAGroups;
	NAGroups++;
	int NU = NSL[MyNation];
	word* ids = Selm[MyNation];
	AG->Units = new word[NU];
	AG->Serials = new word[NU];
	AG->N = 0;
	AG->Name = new char[strlen( gName ) + 1];
	strcpy( AG->Name, gName );
	for (int i = 0; i < NU; i++)
	{
		word MID = ids[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB)
			{
				AG->Units[AG->N] = MID;
				AG->Serials[AG->N] = OB->Serial;
				AG->N++;
			};
		};
	};
	AG->Visible = false;
	AG->Draw( 0 );
};

void ProcessAGroups( int x, int y )
{
	CurrentAGroup = -1;
	int xs = x;
	int ys = ( y >> 1 ) - GetHeight( x, y );
	for (int i = 0; i < NAGroups; i++)
	{
		ActiveGroup* AG = AGroups + i;
		if (AG->Visible&&xs >= AG->MinX&&xs <= AG->MaxX&&ys >= AG->MinY&&ys <= AG->MaxY)
		{
			CurrentAGroup = i;
		};
	};
	RefreshAGroups();
};
bool DeleteActiveGroup()
{
	if (CurrentAGroup == -1)return false;
	free( AGroups[CurrentAGroup].Name );
	if (CurrentAGroup < NAGroups - 1)
	{
		memcpy( AGroups + CurrentAGroup, AGroups + CurrentAGroup + 1, ( NAGroups - CurrentAGroup - 1 ) * sizeof ActiveGroup );
	};
	NAGroups--;
	CurrentAGroup = -1;
	return true;
};
void DrawMinAZones( int x, int y )
{
	for (int i = 0; i < NAZones; i++)
	{
		int dx = x + ( AZones[i].x >> 6 );
		int dy = y + ( AZones[i].y >> 6 );
		int R = AZones[i].R >> 6;
		if (R < 1)R = 1;
		DrawLine( dx - R, dy, dx + R, dy, 255 );
		DrawLine( dx, dy - R, dx, dy + R, 255 );
	};
	for (int i = 0; i < NAGroups; i++)
	{
		ActiveGroup* AG = AGroups + i;
		if (AG->Visible)
		{
			Xbar( x + AG->AminX - 1, y + AG->AminY - 1, AG->AmaxX - AG->AminX + 4, AG->AmaxY - AG->AminY + 4, clrYello );
		};
	};
};
void DrawMinAZonesVis( int x, int y )
{
	for (int i = 0; i < NAZones; i++)if (AZones[i].Dir)
	{
		int dx = x + ( AZones[i].x >> 6 );
		int dy = y + ( AZones[i].y >> 6 );
		int R = AZones[i].R >> 6;
		if (R < 1)R = 1;
		DrawLine( dx - R, dy, dx + R, dy, 255 );
		DrawLine( dx, dy - R, dx, dy + R, 255 );
	};
};
void ClearAllZones()
{
	if (AZones)
	{
		for (int i = 0; i < NAZones; i++)
		{
			free( AZones[i].Name );
		};
		free( AZones );
		AZones = NULL;
		NAZones = 0;
		MaxAZones = 0;
	};
	if (AGroups)
	{
		for (int i = 0; i < NAGroups; i++)
		{
			free( AGroups[i].Name );
			if (AGroups[i].Units)
			{
				free( AGroups[i].Units );
				free( AGroups[i].Serials );
			};
		};
		free( AGroups );
		AGroups = NULL;
		NAGroups = 0;
		MaxAGroups = 0;
	};
	CurrentZone = -1;
	CurrentAGroup = -1;
};
void RefreshGroup( ActiveGroup* AG )
{
	for (int i = 0; i < AG->N; i++)
	{
		bool DelThis = false;
		word MID = AG->Units[i];
		if (MID == 0xFFFF)DelThis = true;
		else
		{
			OneObject* OB = Group[MID];
			if (!( OB && ( !OB->Sdoxlo ) && OB->Serial == AG->Serials[i] ))DelThis = true;
		};
		if (DelThis)
		{
			if (i < AG->N - 1)
			{
				memcpy( AG->Units + i, AG->Units + i + 1, ( AG->N - i - 1 ) * 2 );
				memcpy( AG->Serials + i, AG->Serials + i + 1, ( AG->N - i - 1 ) * 2 );
			};
			AG->N--;
			i--;
		};
	};
};
void RefreshAGroups()
{
	for (int i = 0; i < NAGroups; i++)
	{
		ActiveGroup* AG = AGroups + i;
		if (!AG->Visible)
		{
			CurrentAGroup = i;
			DeleteActiveGroup();
			i--;
		}
		else
		{
			RefreshGroup( AG );
		};
	};
};
