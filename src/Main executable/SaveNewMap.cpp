#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "multipl.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <time.h>
#include "Nucl.h"

#include "Megapolis.h"
#include "dialogs.h"
#include <assert.h>
#include <math.h>
#include "Masks.h"
#include "fonts.h"
#include "3DGraph.h"
#include "MapSprites.h"
#include "NewMon.h"
#include "RealWater.h"
#include "3DMapEd.h"
#include "TopoGraf.h"
#include "crtdbg.h"
#include "ActiveZone.h"

bool Loadingmap = 0;
void CreateTotalLocking();
void CreateMapLocking();
extern char MapScenaryDLL[200];

//------------Saving&Loading 3D map & textures--------

void SaveHeader( ResFile f1 )
{
	int i = 'PMD3' + ADDSH - 1;
	RBlockWrite( f1, &i, 4 );
	i = VertInLine;
	RBlockWrite( f1, &i, 4 );
	i = MaxTH;
	RBlockWrite( f1, &i, 4 );
}

void FreeArrays();
void SetupArrays();
void NewMap( int szX, int szY );
void CleanArrays();
extern byte* RivDir;

bool LoadHeader( ResFile f1 )
{
	int i;
	RBlockRead( f1, &i, 4 );
	int ADDX = i - 'PMD3' + 1;
	if ( ADDX < 1 || ADDX>3 )
	{
		return false;
	}
	RBlockRead( f1, &i, 4 );
	RBlockRead( f1, &i, 4 );
	if ( ( !RivDir ) || ADDSH != ADDX )
	{
		ADDSH = ADDX;
		//FreeArrays();
		SetupArrays();
	}
	msx = 240 << ADDSH;
	msy = 240 << ADDSH;
	return true;
}

bool xLoadHeader( ResFile f1 )
{
	int i;
	RBlockRead( f1, &i, 4 );
	int ADDX = i - 'PMD3' + 1;
	if ( ADDX < 1 || ADDX > 3 )
	{
		return false;
	}
	RBlockRead( f1, &i, 4 );
	RBlockRead( f1, &i, 4 );
	return true;
}

void SaveSurface( ResFile f1 )
{
	int i = 'FRUS';
	RBlockWrite( f1, &i, 4 );
	i = 4 + ( MaxTH + 1 )*MaxTH * 2;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, THMap, i - 4 );
}

extern int RivNX;
extern byte* RivVol;

void LoadSurface( ResFile f1 )
{
	RBlockRead( f1, THMap, ( MaxTH + 1 )*MaxTH * 2 );
	memset( RivDir, 0, RivNX*RivNX );
	memset( RivVol, 0, RivNX*RivNX );
}

extern BlockBars LockBars;
extern BlockBars UnLockBars;

void SaveLockNew( ResFile f1 )
{
	int i = '1COL';
	RBlockWrite( f1, &i, 4 );
	i = 4 + 4 + 4 + ( ( LockBars.NBars + UnLockBars.NBars ) << 2 );
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &LockBars.NBars, 4 );
	RBlockWrite( f1, &UnLockBars.NBars, 4 );
	if ( LockBars.NBars )RBlockWrite( f1, LockBars.BC, LockBars.NBars << 2 );
	if ( UnLockBars.NBars )RBlockWrite( f1, UnLockBars.BC, UnLockBars.NBars << 2 );
}

void LoadLockNew( ResFile f1 )
{
	LockBars.Clear();
	UnLockBars.Clear();
	RBlockRead( f1, &LockBars.NBars, 4 );
	RBlockRead( f1, &UnLockBars.NBars, 4 );
	LockBars.BC = new BlockCell[LockBars.NBars];
	UnLockBars.BC = new BlockCell[UnLockBars.NBars];
	RBlockRead( f1, LockBars.BC, LockBars.NBars << 2 );
	RBlockRead( f1, UnLockBars.BC, UnLockBars.NBars << 2 );
	LockBars.MaxBars = LockBars.NBars;
	UnLockBars.MaxBars = UnLockBars.NBars;
}

void SaveTiles( ResFile f1 )
{
	int i = 'ELIT';
	RBlockWrite( f1, &i, 4 );
	i = 4 + ( MaxTH + 1 )*MaxTH;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, TexMap, i - 4 );
}

void LoadTiles( ResFile f1 )
{
	int N = ( MaxTH + 1 ) * MaxTH;
	RBlockRead( f1, TexMap, N );
	for ( int i = 0; i < N; i++ )
	{
		word tf = TexFlags[TexMap[i]];
	}
}

void SaveSect( ResFile f1 )
{
	if ( SectMap )
	{
		int i = 'TCES';
		RBlockWrite( f1, &i, 4 );
		i = 4 + MaxSector*MaxTH * 6;
		RBlockWrite( f1, &i, 4 );
		RBlockWrite( f1, SectMap, i - 4 );
	}
}

void LoadSect( ResFile f1 )
{
	if ( SectMap )
	{
		RBlockRead( f1, SectMap, MaxSector*MaxTH * 6 );
	}
}

void SaveSprites( ResFile f1 )
{
	int ns = 0;
	int i;

	for ( i = 0; i < MaxSprt; i++ )
	{
		if ( Sprites[i].Enabled )
		{
			ns++;
		}
	}

	i = 'EERT';
	RBlockWrite( f1, &i, 4 );
	i = ( ns * 12 ) + 8;
	word j = 'GA';
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &ns, 4 );

	for ( int i = 0; i < MaxSprt; i++ )
	{
		OneSprite* OS = &Sprites[i];
		if ( OS->Enabled )
		{
			j = 0;
			if ( OS->SG == &TREES )
			{
				j = 'GA';
			}
			else
			{
				if ( OS->SG == &STONES )
				{
					j = 'TS';
				}
				else
				{
					if ( OS->SG == &HOLES )
					{
						j = 'OH';
					}
					else
					{
						if ( OS->SG == &COMPLEX )
						{
							j = 'OC';
						}
					}
				}
			}

			RBlockWrite( f1, &j, 2 );
			RBlockWrite( f1, &OS->x, 4 );
			RBlockWrite( f1, &OS->y, 4 );
			RBlockWrite( f1, &OS->SGIndex, 2 );
		}
	}
}

void addSpriteAnyway( int x, int y, SprGroup* SG, word id );

void DeleteAllSprites();

void LoadSprites( ResFile f1 )
{
	DeleteAllSprites();
	int ns;
	RBlockRead( f1, &ns, 4 );
	int x, y;
	word GSIND, sign;
	for ( int i = 0; i < MaxSprt; i++ )
		Sprites[i].Enabled = false;
	for ( int i = 0; i < ns; i++ )
	{
		RBlockRead( f1, &sign, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &GSIND, 2 );
		if ( sign == 'GA' )
			addSpriteAnyway( x, y, &TREES, GSIND );
		else
			if ( sign == 'TS' )
				addSpriteAnyway( x, y, &STONES, GSIND );
			else
				if ( sign == 'OH' )
					addSpriteAnyway( x, y, &HOLES, GSIND );
				else
					if ( sign == 'OC' )
						addSpriteAnyway( x, y, &COMPLEX, GSIND );
	}
}

void DeleteAllUnits()
{
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			OB->ClearOrders();
			OB->DeletePath();
			OB->Nat->CITY->UnRegisterNewUnit( OB );
			DelObject( OB );
			if ( OB->NInside )
			{
				free( OB->Inside );
			}
			Group[i] = NULL;
		}
	}
}

void SaveUnits( ResFile f1 )
{
	int i = 'TINU';
	RBlockWrite( f1, &i, 4 );
	int NU = 0;
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !OB->Wall )NU++;
	};
	i = 8 + NU * 48;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &NU, 4 );
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !OB->Wall )
		{
			OneObject* OB = Group[i];
			GeneralObject* GO = OB->Ref.General;
			NewMonster* NM = OB->newMons;
			char Name[33];
			memset( Name, 0, 33 );
			strcpy( Name, GO->MonsterID );
			RBlockWrite( f1, &OB->NNUM, 1 );
			RBlockWrite( f1, &OB->NIndex, 2 );
			RBlockWrite( f1, &OB->RealX, 4 );
			RBlockWrite( f1, &OB->RealY, 4 );
			RBlockWrite( f1, &OB->Life, 2 );
			RBlockWrite( f1, &OB->Stage, 2 );
			RBlockWrite( f1, Name, 33 );
		}
	}
}

void SaveUnits2( ResFile f1 )
{
	int i = '2INU';
	RBlockWrite( f1, &i, 4 );
	int NU = 0;
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !OB->Wall )
			NU++;
	}

	i = 8 + NU * 52;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &NU, 4 );
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !OB->Wall )
		{
			OneObject* OB = Group[i];
			GeneralObject* GO = OB->Ref.General;
			NewMonster* NM = OB->newMons;
			char Name[33];
			memset( Name, 0, 33 );
			strcpy( Name, GO->MonsterID );
			RBlockWrite( f1, &OB->NNUM, 1 );
			RBlockWrite( f1, &OB->NIndex, 2 );
			RBlockWrite( f1, &OB->RealX, 4 );
			RBlockWrite( f1, &OB->RealY, 4 );
			RBlockWrite( f1, &OB->Life, 2 );
			RBlockWrite( f1, &OB->Stage, 2 );
			RBlockWrite( f1, &OB->WallX, 2 );
			RBlockWrite( f1, &OB->WallY, 2 );
			RBlockWrite( f1, Name, 33 );
		}
	}
}

void SaveUnits3( ResFile f1 )
{
	int i = '3INU';
	RBlockWrite( f1, &i, 4 );
	int NU = 0;
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !( OB->Wall || OB->Sdoxlo ) )
			NU++;
	}
	i = 8 + NU * 54;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &NU, 4 );
	for ( int i = 0; i < ULIMIT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB && !( OB->Wall || OB->Sdoxlo ) )
		{
			OneObject* OB = Group[i];
			GeneralObject* GO = OB->Ref.General;
			NewMonster* NM = OB->newMons;
			char Name[33];
			memset( Name, 0, 33 );
			strcpy( Name, GO->MonsterID );
			RBlockWrite( f1, &OB->NNUM, 1 );
			RBlockWrite( f1, &OB->NIndex, 2 );
			RBlockWrite( f1, &OB->RealX, 4 );
			RBlockWrite( f1, &OB->RealY, 4 );
			RBlockWrite( f1, &OB->Life, 2 );
			RBlockWrite( f1, &OB->Stage, 2 );
			RBlockWrite( f1, &OB->WallX, 2 );
			RBlockWrite( f1, &OB->WallY, 2 );
			RBlockWrite( f1, &OB->RealDir, 1 );
			byte Opt = 0;
			if ( OB->StandGround )Opt |= 1;
			if ( OB->NoSearchVictim )Opt |= 2;
			RBlockWrite( f1, &Opt, 1 );
			RBlockWrite( f1, Name, 33 );
		}
	}
}

void ClearHints();

int LASTADDID;

void CreateNewUnitAt( byte NI, int x, int y, word Type, word Life, word Stage )
{
	LASTADDID = 0xFFFF;
	Nation* NT = &NATIONS[NI];
	int ID = NT->CreateNewMonsterAt( x, y, Type, true );
	if ( ID != -1 )
	{
		LASTADDID = ID;
		OneObject* OB = Group[ID];
		if ( OB->NewBuilding )
		{
			OB->Stage = 0;
			for ( int i = 0; i < Stage; i++ )OB->NextStage();
			OB->Ready = true;
		}
		else
		{
			OB->Life = Life;
			OB->Ready = true;
		}
	}
	ClearHints();
}

extern bool GroundBox;

OneObject* CreateNewUnitAt3( byte NI, int x, int y, word Type, word Life, word Stage )
{
	Nation* NT = &NATIONS[NI];
	GroundBox = 0;
	int ID = NT->CreateNewMonsterAt( x, y, Type, true );
	GroundBox = 1;
	if ( ID != -1 )
	{
		OneObject* OB = Group[ID];
		if ( OB->NewBuilding )
		{
			OB->Stage = 0;
			int NS = OB->Ref.General->MoreCharacter->ProduceStages;
			for ( int i = 0; i < NS; i++ )OB->NextStage();
			OB->Ready = true;
			ClearHints();
		}
		else
		{
			OB->Life = OB->Ref.General->MoreCharacter->Life;
			OB->Ready = true;
		}
		return OB;
	}
	return NULL;
}

void LoadUnits( ResFile f1 )
{
	DeleteAllUnits();
	int NU;
	RBlockRead( f1, &NU, 4 );
	for ( int i = 0; i < NU; i++ )
	{
		byte NI;
		word Life, NIndex, Stage;
		int x, y;
		char Name[33];
		RBlockRead( f1, &NI, 1 );
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &Life, 2 );
		RBlockRead( f1, &Stage, 2 );
		RBlockRead( f1, Name, 33 );
		//search for NIndex
		Nation* NT = &NATIONS[NI];
		i = -1;
		bool found = false;
		GeneralObject* GO;
		do
		{
			i++;
			GO = NT->Mon[i];
			if ( !strcmp( GO->MonsterID, Name ) )found = true;
		} while ( ( !found ) && i < NT->NMon - 1 );

		if ( found && !GO->newMons->Wall )
		{
			CreateNewUnitAt( NI, x, y, i, Life, Stage );
		}
	}
}

extern int NNations;

void TestUnits( ResFile f1 )
{
	int NU;
	RBlockRead( f1, &NU, 4 );
	for ( int i = 0; i < NU; i++ )
	{
		byte NI;
		word Life, NIndex, Stage;
		int x, y;
		char Name[33];
		RBlockRead( f1, &NI, 1 );
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &Life, 2 );
		RBlockRead( f1, &Stage, 2 );
		RBlockRead( f1, Name, 33 );

		for ( int j = 0; j < NNations; j++ )
		{
			if ( strstr( Name, NatCharLo[j] ) || strstr( Name, NatCharHi[j] ) )
			{
				LOADNATMASK |= 1 << j;
			}
		}
	}
}

extern int PortBuiX;
extern int PortBuiY;

void LoadUnits2( ResFile f1 )
{
	DeleteAllUnits();
	int NU;
	RBlockRead( f1, &NU, 4 );

	for ( int i = 0; i < NU; i++ )
	{
		byte NI;
		word Life, NIndex, Stage;
		int x, y;
		short wx, wy;
		char Name[33];
		RBlockRead( f1, &NI, 1 );
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &Life, 2 );
		RBlockRead( f1, &Stage, 2 );
		RBlockRead( f1, &wx, 2 );
		RBlockRead( f1, &wy, 2 );
		RBlockRead( f1, Name, 33 );
		//search for NIndex
		Nation* NT = &NATIONS[NI];
		PortBuiX = wx;
		PortBuiY = wy;
		i = -1;
		bool found = false;
		GeneralObject* GO;

		do
		{
			i++;
			GO = NT->Mon[i];
			if ( !strcmp( GO->MonsterID, Name ) )
			{
				found = true;
			}
		} while ( ( !found ) && i < NT->NMon - 1 );

		if ( found && !GO->newMons->Wall )
		{
			CreateNewUnitAt( NI, x, y, i, Life, Stage );
		}
	}
}

void TestUnits2( ResFile f1 )
{
	int NU;
	RBlockRead( f1, &NU, 4 );
	for ( int i = 0; i < NU; i++ )
	{
		byte NI;
		word Life, NIndex, Stage;
		int x, y;
		short wx, wy;
		char Name[33];
		RBlockRead( f1, &NI, 1 );
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &Life, 2 );
		RBlockRead( f1, &Stage, 2 );
		RBlockRead( f1, &wx, 2 );
		RBlockRead( f1, &wy, 2 );
		RBlockRead( f1, Name, 33 );
		for ( int j = 0; j < NNations; j++ )
		{
			if ( strstr( Name, NatCharLo[j] ) || strstr( Name, NatCharHi[j] ) )
			{
				LOADNATMASK |= 1 << j;
			}
		}
	}
}

OneObject* CreateNewUnitAt3( byte NI, int x, int y, word Type, word Life, word Stage );

void LoadUnits3( ResFile f1 )
{
	DeleteAllUnits();
	int NU;
	RBlockRead( f1, &NU, 4 );
	for ( int q = 0; q < NU; q++ )
	{
		byte NI;
		word Life, NIndex, Stage;
		int x, y;
		short wx, wy;
		char Name[33];
		RBlockRead( f1, &NI, 1 );
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &Life, 2 );
		RBlockRead( f1, &Stage, 2 );
		RBlockRead( f1, &wx, 2 );
		RBlockRead( f1, &wy, 2 );
		byte DIR, Opt;
		RBlockRead( f1, &DIR, 1 );
		RBlockRead( f1, &Opt, 1 );
		RBlockRead( f1, Name, 33 );
		//search for NIndex
		Nation* NT = &NATIONS[NI];
		PortBuiX = wx;
		PortBuiY = wy;
		int i = -1;
		bool found = false;
		GeneralObject* GO = nullptr;
		if ( NI < 8 )
		{
			do
			{
				i++;
				if ( i < NT->NMon )
				{
					GO = NT->Mon[i];
					if ( !strcmp( GO->MonsterID, Name ) )found = true;
				};
			} while ( ( !found ) && i < NT->NMon - 1 );

			if ( found && !GO->newMons->Wall )
			{
				OneObject* OB = CreateNewUnitAt3( NI, x, y, i, Life, Stage );
				if ( OB )
				{
					OB->RealDir = DIR;
					OB->GraphDir = DIR;
					OB->StandGround = 0 != ( Opt & 1 );
					OB->NoSearchVictim = 0 != ( Opt & 2 );
				}
			}
		}
	}
}

void TestUnits3( ResFile f1 )
{
	int NU;
	RBlockRead( f1, &NU, 4 );
	for ( int i = 0; i < NU; i++ )
	{
		byte NI;
		word Life, NIndex, Stage;
		int x, y;
		short wx, wy;
		char Name[33];
		RBlockRead( f1, &NI, 1 );
		RBlockRead( f1, &NIndex, 2 );
		RBlockRead( f1, &x, 4 );
		RBlockRead( f1, &y, 4 );
		RBlockRead( f1, &Life, 2 );
		RBlockRead( f1, &Stage, 2 );
		RBlockRead( f1, &wx, 2 );
		RBlockRead( f1, &wy, 2 );
		byte DIR, Opt;
		RBlockRead( f1, &DIR, 1 );
		RBlockRead( f1, &Opt, 1 );
		RBlockRead( f1, Name, 33 );
		for ( int j = 0; j < NNations; j++ )
		{
			if ( strstr( Name, NatCharLo[j] ) || strstr( Name, NatCharHi[j] ) )LOADNATMASK |= 1 << j;
		}
	}
}

void SaveNewWalls( ResFile f1 )
{
	int i = 'LLAW';
	RBlockWrite( f1, &i, 4 );
	//calculating the size of data
	int dsize = 8;
	for ( int i = 0; i < WSys.NClusters; i++ )
	{
		WallCluster* WCL = WSys.WCL[i];
		dsize += 9 + WCL->NCells * 16;
	};
	RBlockWrite( f1, &dsize, 4 );
	//saving the data
	RBlockWrite( f1, &WSys.NClusters, 4 );
	for ( int i = 0; i < WSys.NClusters; i++ )
	{
		WallCluster* WCL = WSys.WCL[i];
		RBlockWrite( f1, &WCL->Type, 1 );
		RBlockWrite( f1, &WCL->NCells, 4 );
		//Getting NM_Index
		int NM_Index = 0;
		for ( int i = 0; i < NNewMon; i++ )if ( WCL->NM == &NewMon[i] )NM_Index = i;
		RBlockWrite( f1, &NM_Index, 4 );
		for ( int j = 0; j < WCL->NCells; j++ )
		{
			WallCell* WC = &WCL->Cells[j];
			RBlockWrite( f1, &WC->x, 2 );
			RBlockWrite( f1, &WC->y, 2 );
			RBlockWrite( f1, &WC->Type, 1 );
			RBlockWrite( f1, &WC->NI, 1 );
			RBlockWrite( f1, &WC->Stage, 1 );
			RBlockWrite( f1, &WC->MaxStage, 1 );
			RBlockWrite( f1, &WC->Health, 2 );
			RBlockWrite( f1, &WC->MaxHealth, 2 );
			RBlockWrite( f1, &WC->Sprite, 1 );
			RBlockWrite( f1, &WC->SprBase, 1 );
			RBlockWrite( f1, &WC->ClusterIndex, 2 );
		}
	}
}

void LoadNewWalls( ResFile f1 )
{
	WSys.~WallSystem();
	//WSys.WallSystem();
	int NClusters;
	RBlockRead( f1, &NClusters, 4 );
	WSys.WCL = (WallCluster**) malloc( NClusters << 4 );
	WSys.NClusters = NClusters;
	for ( int i = 0; i < NClusters; i++ )
	{
		WallCluster* WCL = new WallCluster;
		WSys.WCL[i] = WCL;
		RBlockRead( f1, &WCL->Type, 1 );
		RBlockRead( f1, &WCL->NCells, 4 );
		int NM_Index;
		RBlockRead( f1, &NM_Index, 4 );
		WCL->NM = &NewMon[NM_Index];
		WCL->NIndex = 0xFFFF;
		WCL->Cells = new WallCell[WCL->NCells];
		for ( int j = 0; j < WCL->NCells; j++ )
		{
			WallCell* WC = &WCL->Cells[j];
			RBlockRead( f1, &WC->x, 2 );
			RBlockRead( f1, &WC->y, 2 );
			RBlockRead( f1, &WC->Type, 1 );
			RBlockRead( f1, &WC->NI, 1 );
			if ( WCL->NIndex == 0xFFFF )
			{
				WCL->NI = WC->NI;
				Nation* NT = &NATIONS[WC->NI];
				int j;
				for ( j = 0; j < NT->NMon&&NT->Mon[j]->newMons != WCL->NM; j++ );

				WCL->NIndex = j;
			};
			RBlockRead( f1, &WC->Stage, 1 );
			RBlockRead( f1, &WC->MaxStage, 1 );
			RBlockRead( f1, &WC->Health, 2 );
			RBlockRead( f1, &WC->MaxHealth, 2 );
			RBlockRead( f1, &WC->Sprite, 1 );
			RBlockRead( f1, &WC->SprBase, 1 );
			RBlockRead( f1, &WC->ClusterIndex, 2 );
			//creating locking&location information
			WC->Landing( WCL );//CreateLocking(WCL);
			OneObject* OB = Group[WC->OIndex];
			if ( OB )
			{
				OB->Ready = true;
			}

		}
	}
}

void SaveNewWallsV1( ResFile f1 )
{
	int i = '1LAW';
	RBlockWrite( f1, &i, 4 );
	//calculating the size of data
	int dsize = 8;
	for ( int i = 0; i < WSys.NClusters; i++ )
	{
		WallCluster* WCL = WSys.WCL[i];
		dsize += 9 + WCL->NCells * 17;
	}

	RBlockWrite( f1, &dsize, 4 );
	//saving the data
	RBlockWrite( f1, &WSys.NClusters, 4 );
	for ( int i = 0; i < WSys.NClusters; i++ )
	{
		WallCluster* WCL = WSys.WCL[i];
		RBlockWrite( f1, &WCL->Type, 1 );
		RBlockWrite( f1, &WCL->NCells, 4 );
		//Getting NM_Index
		int NM_Index = 0;
		for ( int i = 0; i < NNewMon; i++ )if ( WCL->NM == &NewMon[i] )NM_Index = i;
		RBlockWrite( f1, &NM_Index, 4 );
		for ( int j = 0; j < WCL->NCells; j++ )
		{
			WallCell* WC = &WCL->Cells[j];
			RBlockWrite( f1, &WC->x, 2 );
			RBlockWrite( f1, &WC->y, 2 );
			RBlockWrite( f1, &WC->Type, 1 );
			RBlockWrite( f1, &WC->NI, 1 );
			RBlockWrite( f1, &WC->Stage, 1 );
			RBlockWrite( f1, &WC->MaxStage, 1 );
			RBlockWrite( f1, &WC->Health, 2 );
			RBlockWrite( f1, &WC->MaxHealth, 2 );
			RBlockWrite( f1, &WC->Sprite, 1 );
			RBlockWrite( f1, &WC->SprBase, 1 );
			RBlockWrite( f1, &WC->ClusterIndex, 2 );
			RBlockWrite( f1, &WC->Visible, 1 );
		}
	}
}

void LoadNewWallsV1( ResFile f1 )
{
	WSys.~WallSystem();
	//WSys.WallSystem();
	int NClusters;
	RBlockRead( f1, &NClusters, 4 );
	WSys.WCL = (WallCluster**) malloc( NClusters << 4 );
	WSys.NClusters = NClusters;
	for ( int i = 0; i < NClusters; i++ )
	{
		WallCluster* WCL = new WallCluster;
		WSys.WCL[i] = WCL;
		RBlockRead( f1, &WCL->Type, 1 );
		RBlockRead( f1, &WCL->NCells, 4 );
		int NM_Index;
		RBlockRead( f1, &NM_Index, 4 );
		WCL->NM = &NewMon[NM_Index];
		WCL->NIndex = 0xFFFF;
		WCL->Cells = new WallCell[WCL->NCells];
		for ( int j = 0; j < WCL->NCells; j++ )
		{
			WallCell* WC = &WCL->Cells[j];
			RBlockRead( f1, &WC->x, 2 );
			RBlockRead( f1, &WC->y, 2 );
			RBlockRead( f1, &WC->Type, 1 );
			RBlockRead( f1, &WC->NI, 1 );
			if ( WCL->NIndex == 0xFFFF )
			{
				WCL->NI = WC->NI;
				Nation* NT = &NATIONS[WC->NI];
				int j;
				for ( j = 0; j < NT->NMon&&NT->Mon[j]->newMons != WCL->NM; j++ );

				WCL->NIndex = j;
			};
			RBlockRead( f1, &WC->Stage, 1 );
			RBlockRead( f1, &WC->MaxStage, 1 );
			RBlockRead( f1, &WC->Health, 2 );
			RBlockRead( f1, &WC->MaxHealth, 2 );
			RBlockRead( f1, &WC->Sprite, 1 );
			RBlockRead( f1, &WC->SprBase, 1 );
			RBlockRead( f1, &WC->ClusterIndex, 2 );
			RBlockRead( f1, &WC->Visible, 1 );
			//creating locking&location information
			if ( WC->Visible )
			{
				WC->Landing( WCL );//CreateLocking(WCL);
				OneObject* OB = Group[WC->OIndex];
				if ( OB )
				{
					OB->Ready = true;
					OB->Life = WC->Health;
				}
			}
		}
	}
}

void SaveNewWallsV2( ResFile f1 )
{
	int i = '2LAW';
	RBlockWrite( f1, &i, 4 );
	//calculating the size of data
	int dsize = 8;
	for ( int i = 0; i < WSys.NClusters; i++ )
	{
		WallCluster* WCL = WSys.WCL[i];
		dsize += 9 + WCL->NCells*( 17 + 9 );
	}

	RBlockWrite( f1, &dsize, 4 );
	//saving the data
	RBlockWrite( f1, &WSys.NClusters, 4 );
	for ( int i = 0; i < WSys.NClusters; i++ )
	{
		WallCluster* WCL = WSys.WCL[i];
		RBlockWrite( f1, &WCL->Type, 1 );
		RBlockWrite( f1, &WCL->NCells, 4 );
		//Getting NM_Index
		int NM_Index = 0;
		for ( int i = 0; i < NNewMon; i++ )if ( WCL->NM == &NewMon[i] )NM_Index = i;
		RBlockWrite( f1, &NM_Index, 4 );
		for ( int j = 0; j < WCL->NCells; j++ )
		{
			WallCell* WC = &WCL->Cells[j];
			RBlockWrite( f1, &WC->x, 2 );
			RBlockWrite( f1, &WC->y, 2 );
			RBlockWrite( f1, &WC->Type, 1 );
			RBlockWrite( f1, &WC->NI, 1 );
			RBlockWrite( f1, &WC->Stage, 1 );
			RBlockWrite( f1, &WC->MaxStage, 1 );
			RBlockWrite( f1, &WC->Health, 2 );
			RBlockWrite( f1, &WC->MaxHealth, 2 );
			RBlockWrite( f1, &WC->Sprite, 1 );
			RBlockWrite( f1, &WC->SprBase, 1 );
			RBlockWrite( f1, &WC->ClusterIndex, 2 );
			RBlockWrite( f1, &WC->Visible, 1 );

			RBlockWrite( f1, &WC->GateIndex, 2 );
			RBlockWrite( f1, &WC->Locks, 4 );
			RBlockWrite( f1, &WC->DirMask, 1 );
			RBlockWrite( f1, &WC->OIndex, 2 );
		}
	}
}

void LoadNewWallsV2( ResFile f1 )
{
	WSys.~WallSystem();
	//WSys.WallSystem();
	int NClusters;
	RBlockRead( f1, &NClusters, 4 );
	WSys.WCL = (WallCluster**) malloc( NClusters << 4 );
	WSys.NClusters = NClusters;
	for ( int i = 0; i < NClusters; i++ )
	{
		WallCluster* WCL = new WallCluster;
		WSys.WCL[i] = WCL;
		RBlockRead( f1, &WCL->Type, 1 );
		RBlockRead( f1, &WCL->NCells, 4 );
		int NM_Index;
		RBlockRead( f1, &NM_Index, 4 );
		WCL->NM = &NewMon[NM_Index];
		WCL->NIndex = 0xFFFF;
		WCL->Cells = new WallCell[WCL->NCells];
		int CurType = -1;
		int NMN = NATIONS->NMon;
		GeneralObject** GOS = NATIONS->Mon;
		for ( int j = 0; j < WCL->NCells; j++ )
		{
			WallCell* WC = &WCL->Cells[j];
			RBlockRead( f1, &WC->x, 2 );
			RBlockRead( f1, &WC->y, 2 );
			RBlockRead( f1, &WC->Type, 1 );
			RBlockRead( f1, &WC->NI, 1 );
			int wtp = WC->Type;

			if ( CurType == -1 )
			{
				for ( int i = 0; i < NMN&&CurType == -1; i++ )
				{
					NewMonster* NM = GOS[i]->newMons;
					if ( NM->Wall&&NM->Sprite == wtp )CurType = i;
				};
			};
			WCL->NI = WC->NI;
			WCL->NIndex = CurType;
			/*
			if(WCL->NIndex==0xFFFF){
				WCL->NI=WC->NI;
				Nation* NT=&NATIONS[WC->NI];
				for(int j=0;j<NT->NMon&&NT->Mon[j]->newMons!=WCL->NM;j++);
				WCL->NIndex=j;
			};
			*/
			RBlockRead( f1, &WC->Stage, 1 );
			RBlockRead( f1, &WC->MaxStage, 1 );
			RBlockRead( f1, &WC->Health, 2 );
			RBlockRead( f1, &WC->MaxHealth, 2 );
			RBlockRead( f1, &WC->Sprite, 1 );
			RBlockRead( f1, &WC->SprBase, 1 );
			RBlockRead( f1, &WC->ClusterIndex, 2 );
			RBlockRead( f1, &WC->Visible, 1 );

			RBlockRead( f1, &WC->GateIndex, 2 );
			RBlockRead( f1, &WC->Locks, 4 );
			RBlockRead( f1, &WC->DirMask, 1 );
			RBlockRead( f1, &WC->OIndex, 2 );
			WC->OIndex = CurType;
			//creating locking&location information
			if ( WC->Visible )
			{
				WC->Landing( WCL );//CreateLocking(WCL);
				OneObject* OB = Group[WC->OIndex];
				if ( OB )
				{
					OB->Ready = true;
					OB->Life = WC->Health;
				}
			}
		}
	}
}

void SaveGates( ResFile f1 )
{
	if ( !NGates )return;
	int i = '1TAG';
	RBlockWrite( f1, &i, 4 );
	i = 12 + NGates * sizeof Gate;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &NGates, 4 );
	RBlockWrite( f1, &MaxGates, 4 );
	RBlockWrite( f1, Gates, i - 12 );
};
void LoadGates( ResFile f1 )
{
	RBlockRead( f1, &NGates, 4 );
	RBlockRead( f1, &MaxGates, 4 );
	Gates = (Gate*) realloc( Gates, MaxGates * sizeof Gate );
	RBlockRead( f1, Gates, NGates * sizeof Gate );
};
extern byte* WaterBright;
void SaveWaterCost( ResFile f1 )
{
	int i = '2AES';
	RBlockWrite( f1, &i, 4 );
	int Lx = msx + 2;
	int Ly = msy + 2;
	i = 12 + Lx*Ly * 2;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &Lx, 4 );
	RBlockWrite( f1, &Ly, 4 );
	int dsx = ( MaxWX );
	int pos = 0;

	for ( int i = 0; i < Ly; i++ )
	{
		RBlockWrite( f1, WaterDeep + pos, Lx );
		RBlockWrite( f1, WaterBright + pos, Lx );
		pos += dsx;
	}
}

void LoadWaterCost( ResFile f1 )
{
	int Lx, Ly;
	RBlockRead( f1, &Lx, 4 );
	RBlockRead( f1, &Ly, 4 );
	int dsx = ( MaxWX );
	int pos = 0;
	for ( int i = 0; i < Ly; i++ )
	{
		RBlockRead( f1, WaterDeep + pos, Lx );
		RBlockRead( f1, WaterBright + pos, Lx );
		pos += dsx;
	}
}

extern int RES[8][8];
void SaveRES( ResFile f1 )
{
	int i = 'USER';
	RBlockWrite( f1, &i, 4 );
	i = sizeof( RES ) + 4;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &( RES[0][0] ), i - 4 );
}

void LoadRES( ResFile f1 )
{
	int i = sizeof( RES );
	RBlockRead( f1, &( RES[0][0] ), i );
	for ( int p = 0; p < 8; p++ )
	{
		for ( int q = 0; q < 8; q++ )
		{
			SetXRESRC( p, q, RES[p][q] );
		};
	};
	for ( int i = 0; i < 8; i++ )
	{
		Nation* NT = NATIONS + i;
		NT->SELO.Init();
		NT->ARMY.Init();
		NT->SCIENCE.Init();
		for ( int j = 0; j < 8; j++ )
		{
			NT->GENERAL.RESAM[j] = XRESRC( i, j );
			NT->GENERAL.RESRM[j] = 0;
		};
	};
};
void EraseAreas();
void SaveTopology( ResFile f1 )
{
	EraseAreas();
	rando();//!!
	CreateTotalLocking();
	int i = '1POT';
	RBlockWrite( f1, &i, 4 );
	i = 4 + 4 + NAreas * sizeof( Area ) + 4 * NAreas*NAreas + 2 * TopLx*TopLy;
	for ( int j = 0; j < NAreas; j++ )
	{
		Area* Ar1 = TopMap + j;
		i += ( Ar1->NMines + ( Ar1->NLinks << 1 ) ) << 1;
	};
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &NAreas, 4 );
	for ( int j = 0; j < NAreas; j++ )
	{
		Area Ar1 = TopMap[j];
		RBlockWrite( f1, &Ar1, sizeof Area );
		Ar1.MaxLink = Ar1.NLinks;
		if ( Ar1.NMines )RBlockWrite( f1, Ar1.MinesIdx, Ar1.NMines << 1 );
		if ( Ar1.NLinks )RBlockWrite( f1, Ar1.Link, Ar1.NLinks << 2 );
	};
	RBlockWrite( f1, MotionLinks, NAreas*NAreas * 2 );
	RBlockWrite( f1, LinksDist, NAreas*NAreas * 2 );
	RBlockWrite( f1, TopRef, 2 * TopLx*TopLy );
};
void CreateRoadsNet();
void LoadTopology( ResFile f1 )
{
	EraseAreas();
	RBlockRead( f1, &NAreas, 4 );
	MaxArea = NAreas;
	TopMap = new Area[NAreas];
	MotionLinks = new word[NAreas*NAreas];
	LinksDist = new word[NAreas*NAreas];
	for ( int j = 0; j < NAreas; j++ )
	{
		Area* Ar1 = TopMap + j;
		RBlockRead( f1, Ar1, sizeof Area );
		if ( Ar1->NMines )Ar1->MinesIdx = new word[Ar1->NMines];
		else Ar1->MinesIdx = NULL;
		if ( Ar1->NLinks )Ar1->Link = new word[Ar1->NLinks];
		else Ar1->Link = NULL;
		if ( Ar1->NMines )RBlockRead( f1, Ar1->MinesIdx, Ar1->NMines << 1 );
		if ( Ar1->NLinks )RBlockRead( f1, Ar1->Link, Ar1->NLinks << 1 );
	};
	RBlockRead( f1, MotionLinks, 2 * NAreas*NAreas );
	RBlockRead( f1, LinksDist, 2 * NAreas*NAreas );
	RBlockRead( f1, TopRef, 2 * TopLx*TopLy );
	//CreateRoadsNet();
};

void SaveWTopology( ResFile f1 )
{
	int i = 'WPOT';
	RBlockWrite( f1, &i, 4 );
	i = 4 + 4 + WNAreas * sizeof( Area ) + 4 * WNAreas*WNAreas + 2 * TopLx*TopLy;
	for ( int j = 0; j < WNAreas; j++ )
	{
		Area* Ar1 = WTopMap + j;
		i += ( Ar1->NMines + ( Ar1->NLinks << 1 ) ) << 1;
	};
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &WNAreas, 4 );
	for ( int j = 0; j < WNAreas; j++ )
	{
		Area Ar1 = WTopMap[j];
		RBlockWrite( f1, &Ar1, sizeof Area );
		Ar1.MaxLink = Ar1.NLinks;
		if ( Ar1.NMines )RBlockWrite( f1, Ar1.MinesIdx, Ar1.NMines << 1 );
		if ( Ar1.NLinks )RBlockWrite( f1, Ar1.Link, Ar1.NLinks << 2 );
	};
	RBlockWrite( f1, WMotionLinks, WNAreas*WNAreas * 2 );
	RBlockWrite( f1, WLinksDist, WNAreas*WNAreas * 2 );
	RBlockWrite( f1, WTopRef, 2 * TopLx*TopLy );
};
void LoadTopology1( ResFile f1 )
{
	EraseAreas();
	RBlockRead( f1, &NAreas, 4 );
	MaxArea = NAreas;
	TopMap = new Area[NAreas];
	MotionLinks = new word[NAreas*NAreas];
	LinksDist = new word[NAreas*NAreas];
	for ( int j = 0; j < NAreas; j++ )
	{
		Area* Ar1 = TopMap + j;
		RBlockRead( f1, Ar1, sizeof Area );
		if ( Ar1->NMines )Ar1->MinesIdx = new word[Ar1->NMines];
		else Ar1->MinesIdx = NULL;
		if ( Ar1->NLinks )Ar1->Link = new word[Ar1->MaxLink << 1];
		else Ar1->Link = NULL;
		if ( Ar1->NMines )RBlockRead( f1, Ar1->MinesIdx, Ar1->NMines << 1 );
		if ( Ar1->NLinks )RBlockRead( f1, Ar1->Link, Ar1->NLinks << 2 );
	};
	RBlockRead( f1, MotionLinks, 2 * NAreas*NAreas );
	RBlockRead( f1, LinksDist, 2 * NAreas*NAreas );
	RBlockRead( f1, TopRef, 2 * TopLx*TopLy );
	//CreateRoadsNet();
};
void ResearchIslands();
void LoadWTopology1( ResFile f1 )
{
	RBlockRead( f1, &WNAreas, 4 );
	WTopMap = new Area[WNAreas];
	WMotionLinks = new word[WNAreas*WNAreas];
	WLinksDist = new word[WNAreas*WNAreas];
	for ( int j = 0; j < WNAreas; j++ )
	{
		Area* Ar1 = WTopMap + j;
		RBlockRead( f1, Ar1, sizeof Area );
		if ( Ar1->NMines )Ar1->MinesIdx = new word[Ar1->NMines];
		else Ar1->MinesIdx = NULL;
		if ( Ar1->NLinks )Ar1->Link = new word[Ar1->MaxLink << 1];
		else Ar1->Link = NULL;
		if ( Ar1->NMines )RBlockRead( f1, Ar1->MinesIdx, Ar1->NMines << 1 );
		if ( Ar1->NLinks )RBlockRead( f1, Ar1->Link, Ar1->NLinks << 2 );
	};
	RBlockRead( f1, WMotionLinks, 2 * WNAreas*WNAreas );
	RBlockRead( f1, WLinksDist, 2 * WNAreas*WNAreas );
	RBlockRead( f1, WTopRef, 2 * TopLx*TopLy );
	GTOP[1].LinksDist = WLinksDist;
	GTOP[1].MotionLinks = WMotionLinks;
	GTOP[1].NAreas = WNAreas;
	GTOP[1].TopMap = WTopMap;
	GTOP[1].TopRef = WTopRef;
	ResearchIslands();
};
void SaveZonesAndGroups( ResFile f1 )
{
	int i = '1NOZ';
	RBlockWrite( f1, &i, 4 );
	int sz = 4 + 8 + NAZones*( sizeof ActiveZone ) + NAGroups*( sizeof ActiveGroup );
	for ( int i = 0; i < NAZones; i++ )
	{
		ActiveZone* AZ = AZones + i;
		sz += strlen( AZ->Name ) + 1 + 1;
	};
	for ( int i = 0; i < NAGroups; i++ )
	{
		ActiveGroup* AG = AGroups + i;
		sz += strlen( AG->Name ) + 1 + 1;
		sz += AG->N * 8;
	};
	RBlockWrite( f1, &sz, 4 );
	RBlockWrite( f1, &NAZones, 4 );
	RBlockWrite( f1, &NAGroups, 4 );
	for ( int i = 0; i < NAZones; i++ )
	{
		ActiveZone* AZ = AZones + i;
		RBlockWrite( f1, AZ, sizeof ActiveZone );
		sz = strlen( AZ->Name ) + 1;
		RBlockWrite( f1, &sz, 1 );
		RBlockWrite( f1, AZ->Name, sz );
	};
	for ( int i = 0; i < NAGroups; i++ )
	{
		ActiveGroup* AG = AGroups + i;
		RBlockWrite( f1, AG, sizeof ActiveGroup );
		sz = strlen( AG->Name ) + 1;
		RBlockWrite( f1, &sz, 1 );
		RBlockWrite( f1, AG->Name, sz );
		for ( int j = 0; j < AG->N; j++ )
		{
			OneObject* OB = Group[AG->Units[j]];
			RBlockWrite( f1, &OB->RealX, 4 );
			RBlockWrite( f1, &OB->RealY, 4 );
		};
	};
};
int FindUnitByCoor( int x, int y )
{
	int MinDst = 10000000;
	int OID = -1;
	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB )
		{
			int dst = Norma( OB->RealX - x, OB->RealY - y );
			if ( dst < MinDst )
			{
				MinDst = dst;
				OID = OB->Index;
			};
		};
	};
	return OID;
};
void LoadZonesAndGroups( ResFile f1 )
{
	RBlockRead( f1, &NAZones, 4 );
	RBlockRead( f1, &NAGroups, 4 );
	MaxAZones = NAZones;
	MaxAGroups = NAGroups;
	AGroups = new ActiveGroup[MaxAGroups];
	AZones = new ActiveZone[MaxAZones];
	for ( int i = 0; i < NAZones; i++ )
	{
		ActiveZone* AZ = AZones + i;
		RBlockRead( f1, AZ, sizeof ActiveZone );
		byte L;
		RBlockRead( f1, &L, 1 );
		AZ->Name = new char[L];
		RBlockRead( f1, AZ->Name, L );
	};
	for ( int q = 0; q < NAGroups; q++ )
	{
		ActiveGroup* AG = AGroups + q;
		RBlockRead( f1, AG, sizeof ActiveGroup );
		byte L;
		RBlockRead( f1, &L, 1 );
		AG->Name = new char[L];
		RBlockRead( f1, AG->Name, L );
		AG->Units = new word[AG->N];
		AG->Serials = new word[AG->N];
		for ( int i = 0; i < AG->N; i++ )
		{
			int x, y;
			RBlockRead( f1, &x, 4 );
			RBlockRead( f1, &y, 4 );
			int id = FindUnitByCoor( x, y );
			if ( id != -1 )
			{
				OneObject* OB = Group[id];
				AG->Units[i] = id;
				AG->Serials[i] = OB->Serial;
			}
			else
			{
				AG->Units[i] = 0xFFFF;
				AG->Serials[i] = 0xFFFF;
			};
		};
	};
};
extern City CITY[8];
void SaveFormations( ResFile f1 )
{
	int i = 'MROF';
	RBlockWrite( f1, &i, 4 );
	int sz = 4 + 4;
	int NB = 0;
	for ( int i = 0; i < 8; i++ )
	{
		Brigade* BR = CITY[i].Brigs;
		for ( int j = 0; j < MaxBrig; j++ )
		{
			if ( BR->Enabled&&BR->WarType )
			{
				NB++;
				sz += 8 + sizeof( BR->AddDamage ) + sizeof( BR->AddShield ) + sizeof( BR->BM )
					+ sizeof( BR->Direction ) + sizeof( BR->NMemb ) + sizeof( BR->WarType )
					+ sizeof( BR->SN ) + sizeof( BR->MembID );
				sz += 4 * BR->NMemb;
				if ( BR->NMemb )
				{
					sz += BR->NMemb * 8;
				};
			};
			BR++;
		};
	};
	RBlockWrite( f1, &sz, 4 );
	RBlockWrite( f1, &NB, 4 );
	for ( int i = 0; i < 8; i++ )
	{
		Brigade* BR = CITY[i].Brigs;
		for ( int j = 0; j < MaxBrig; j++ )
		{
			if ( BR->Enabled&&BR->WarType )
			{
				RBlockWrite( f1, &i, 4 );
				RBlockWrite( f1, &j, 4 );
				RBlockWrite( f1, &BR->AddDamage, sizeof BR->AddDamage );
				RBlockWrite( f1, &BR->AddShield, sizeof BR->AddShield );
				RBlockWrite( f1, &BR->BM, sizeof BR->BM );
				RBlockWrite( f1, &BR->Direction, sizeof BR->Direction );
				RBlockWrite( f1, &BR->NMemb, sizeof BR->NMemb );
				RBlockWrite( f1, &BR->WarType, sizeof BR->WarType );
				RBlockWrite( f1, &BR->SN, sizeof BR->SN );
				RBlockWrite( f1, &BR->MembID, sizeof BR->MembID );
				if ( BR->NMemb )
				{
					for ( int k = 0; k < BR->NMemb; k++ )RBlockWrite( f1, &BR->posX[k], 2 );
					for ( int k = 0; k < BR->NMemb; k++ )RBlockWrite( f1, &BR->posY[k], 2 );
					for ( int j = 0; j < BR->NMemb; j++ )
					{
						word MID = BR->Memb[j];
						if ( MID != 0xFFFF )
						{
							OneObject* OB = Group[MID];
							if ( OB&&OB->Serial == BR->MembSN[j] )
							{
								RBlockWrite( f1, &OB->RealX, 4 );
								RBlockWrite( f1, &OB->RealY, 4 );
							}
							else
							{
								int p = -1;
								RBlockWrite( f1, &p, 4 );
								RBlockWrite( f1, &p, 4 );
							};
						}
						else
						{
							int p = -1;
							RBlockWrite( f1, &p, 4 );
							RBlockWrite( f1, &p, 4 );
						};
					};
				};
			};
			BR++;
		};
	};
};
void LoadFormations( ResFile f1 )
{
	int NB;
	RBlockRead( f1, &NB, 4 );
	bool oldvers = 0;
	word* tmp = nullptr;
	byte* tmp1 = nullptr;
	if ( NB )
	{
		tmp = new word[NB];
		tmp1 = new byte[NB];
	};
	for ( int i = 0; i < NB; i++ )
	{
		int bid, nat;
		RBlockRead( f1, &nat, 4 );
		RBlockRead( f1, &bid, 4 );
		tmp[i] = bid;
		tmp1[i] = nat;
		Brigade* BR = CITY[nat].Brigs + bid;
		memset( BR, 0, sizeof Brigade );
		BR->Enabled = true;
		BR->ArmyID = 0xFFFF;
		RBlockRead( f1, &BR->AddDamage, sizeof BR->AddDamage );
		RBlockRead( f1, &BR->AddShield, sizeof BR->AddShield );
		RBlockRead( f1, &BR->BM, sizeof BR->BM );
		RBlockRead( f1, &BR->Direction, sizeof BR->Direction );
		RBlockRead( f1, &BR->NMemb, sizeof BR->NMemb );
		RBlockRead( f1, &BR->WarType, sizeof BR->WarType );
		RBlockRead( f1, &BR->SN, sizeof BR->SN );
		RBlockRead( f1, &BR->MembID, sizeof BR->MembID );
		if ( BR->NMemb - 2 != ElementaryOrders[BR->WarType - 1].NUnits )oldvers = 1;
		BR->MaxMemb = BR->NMemb;
		BR->CT = CITY + nat;
		BR->ID = bid;
		memset( &BR->BM, 0, sizeof BR->BM );
		if ( BR->NMemb )
		{
			BR->PosCreated = 1;
			BR->posX = new int[BR->NMemb];
			BR->posY = new int[BR->NMemb];
			BR->Memb = new word[BR->NMemb];
			BR->MembSN = new word[BR->NMemb];
			for ( int k = 0; k < BR->NMemb; k++ )
			{
				BR->posX[k] = 0;
				RBlockRead( f1, &BR->posX[k], 2 );
			};
			for ( int k = 0; k < BR->NMemb; k++ )
			{
				BR->posY[k] = 0;
				RBlockRead( f1, &BR->posY[k], 2 );
			};
			for ( int q = 0; q < BR->NMemb; q++ )
			{
				int x, y;
				RBlockRead( f1, &x, 4 );
				RBlockRead( f1, &y, 4 );
				if ( x == -1 )
				{
					BR->Memb[q] = 0xFFFF;
					BR->MembSN[q] = 0xFFFF;
				}
				else
				{
					int ID = FindUnitByCoor( x, y );
					if ( ID == -1 )
					{
						BR->Memb[q] = 0xFFFF;
						BR->MembSN[q] = 0xFFFF;
					}
					else
					{
						OneObject* OB = Group[ID];
						BR->Memb[q] = OB->Index;
						BR->MembSN[q] = OB->Serial;
						OB->InArmy = true;
						OB->BrigadeID = bid;
						OB->BrIndex = q;
						OB->AddDamage = BR->AddDamage;
						OB->AddShield = BR->AddShield;
						if ( q > 1 )
						{
							BR->MembID = OB->NIndex;
						};
						( &BR->BM.Peons )[GetBMIndex( OB )]++;
					};
				};
			};
		};
	};
	if ( oldvers )
	{
		for ( int i = 0; i < NB; i++ )
		{
			CITY[tmp1[i]].Brigs[tmp[i]].WarType--;
		};
	};
	if ( NB )
	{
		free( tmp );
		free( tmp1 );
	};
};
void SaveFormationsNew( ResFile f1 )
{
	int i = '1ROF';
	RBlockWrite( f1, &i, 4 );
	int sz = 4 + 4;
	int NB = 0;
	for ( int i = 0; i < 8; i++ )
	{
		Brigade* BR = CITY[i].Brigs;
		for ( int j = 0; j < MaxBrig; j++ )
		{
			if ( BR->Enabled&&BR->WarType )
			{
				NB++;
				sz += 8 + sizeof( BR->AddDamage ) + sizeof( BR->AddShield ) + sizeof( BR->BM )
					+ sizeof( BR->Direction ) + sizeof( BR->NMemb ) + 32
					+ sizeof( BR->SN ) + sizeof( BR->MembID );
				sz += 4 * BR->NMemb;
				if ( BR->NMemb )
				{
					sz += BR->NMemb * 8;
				};
			};
			BR++;
		};
	};
	RBlockWrite( f1, &sz, 4 );
	RBlockWrite( f1, &NB, 4 );
	char FRMNM[32];
	for ( int i = 0; i < 8; i++ )
	{
		Brigade* BR = CITY[i].Brigs;
		for ( int j = 0; j < MaxBrig; j++ )
		{
			if ( BR->Enabled&&BR->WarType )
			{
				RBlockWrite( f1, &i, 4 );
				RBlockWrite( f1, &j, 4 );
				RBlockWrite( f1, &BR->AddDamage, sizeof BR->AddDamage );
				RBlockWrite( f1, &BR->AddShield, sizeof BR->AddShield );
				RBlockWrite( f1, &BR->BM, sizeof BR->BM );
				RBlockWrite( f1, &BR->Direction, sizeof BR->Direction );
				RBlockWrite( f1, &BR->NMemb, sizeof BR->NMemb );
				memset( FRMNM, 0, 32 );
				OrderDescription* ODE = ElementaryOrders + BR->WarType - 1;
				strcpy( FRMNM, ODE->ID );
				//RBlockWrite(f1,&BR->WarType,sizeof BR->WarType);
				RBlockWrite( f1, FRMNM, 32 );
				RBlockWrite( f1, &BR->SN, sizeof BR->SN );
				RBlockWrite( f1, &BR->MembID, sizeof BR->MembID );
				if ( BR->NMemb )
				{
					for ( int k = 0; k < BR->NMemb; k++ )RBlockWrite( f1, &BR->posX[k], 2 );
					for ( int k = 0; k < BR->NMemb; k++ )RBlockWrite( f1, &BR->posY[k], 2 );
					for ( int j = 0; j < BR->NMemb; j++ )
					{
						word MID = BR->Memb[j];
						if ( MID != 0xFFFF )
						{
							OneObject* OB = Group[MID];
							if ( OB&&OB->Serial == BR->MembSN[j] )
							{
								RBlockWrite( f1, &OB->RealX, 4 );
								RBlockWrite( f1, &OB->RealY, 4 );
							}
							else
							{
								int p = -1;
								RBlockWrite( f1, &p, 4 );
								RBlockWrite( f1, &p, 4 );
							};
						}
						else
						{
							int p = -1;
							RBlockWrite( f1, &p, 4 );
							RBlockWrite( f1, &p, 4 );
						};
					};
				};
			};
			BR++;
		};
	};
};
void LoadFormationsNew( ResFile f1 )
{
	int NB;
	RBlockRead( f1, &NB, 4 );
	char FRMNM[32];
	for ( int i = 0; i < NB; i++ )
	{
		int bid, nat;
		RBlockRead( f1, &nat, 4 );
		RBlockRead( f1, &bid, 4 );
		Brigade* BR = CITY[nat].Brigs + bid;
		memset( BR, 0, sizeof Brigade );
		BR->Enabled = true;
		BR->ArmyID = 0xFFFF;
		RBlockRead( f1, &BR->AddDamage, sizeof BR->AddDamage );
		RBlockRead( f1, &BR->AddShield, sizeof BR->AddShield );
		RBlockRead( f1, &BR->BM, sizeof BR->BM );
		RBlockRead( f1, &BR->Direction, sizeof BR->Direction );
		RBlockRead( f1, &BR->NMemb, sizeof BR->NMemb );
		RBlockRead( f1, FRMNM, 32 );
		int wt = -1;
		for ( int j = 0; j < NEOrders&&wt == -1; j++ )
		{
			if ( !strcmp( FRMNM, ElementaryOrders[j].ID ) )wt = j;
		};
		//assert(wt!=-1);
		BR->WarType = wt + 1;
		//RBlockRead(f1,&BR->WarType,sizeof BR->WarType);
		RBlockRead( f1, &BR->SN, sizeof BR->SN );
		RBlockRead( f1, &BR->MembID, sizeof BR->MembID );
		BR->MaxMemb = BR->NMemb;
		BR->CT = CITY + nat;
		BR->ID = bid;
		if ( BR->NMemb )
		{
			BR->PosCreated = 1;
			BR->posX = new int[BR->NMemb];
			BR->posY = new int[BR->NMemb];
			BR->Memb = new word[BR->NMemb];
			BR->MembSN = new word[BR->NMemb];
			for ( int k = 0; k < BR->NMemb; k++ )
			{
				BR->posX[k] = 0;
				RBlockRead( f1, &BR->posX[k], 2 );
			};
			for ( int k = 0; k < BR->NMemb; k++ )
			{
				BR->posY[k] = 0;
				RBlockRead( f1, &BR->posY[k], 2 );
			};
			for ( int q = 0; q < BR->NMemb; q++ )
			{
				int x, y;
				RBlockRead( f1, &x, 4 );
				RBlockRead( f1, &y, 4 );
				if ( x == -1 )
				{
					BR->Memb[q] = 0xFFFF;
					BR->MembSN[q] = 0xFFFF;
				}
				else
				{
					int ID = FindUnitByCoor( x, y );
					if ( ID == -1 )
					{
						BR->Memb[q] = 0xFFFF;
						BR->MembSN[q] = 0xFFFF;
					}
					else
					{
						OneObject* OB = Group[ID];
						BR->Memb[q] = OB->Index;
						BR->MembSN[q] = OB->Serial;
						OB->InArmy = true;
						OB->BrigadeID = bid;
						OB->BrIndex = q;
						OB->AddDamage = BR->AddDamage;
						OB->AddShield = BR->AddShield;
						if ( q > 1 )
						{
							if ( StrelokID == OB->newMons->Usage
								|| GrenaderID == OB->newMons->Usage//BUGFIX: Non-shooting grenadiers in formations
								|| OB->newMons->Artilery )
							{
								BR->Strelki = true;
							}
							else
							{
								BR->Strelki = false;
							}



							//byte use = OB->newMons->Usage;

							//BR->Strelki = ( use == StrelokID || use == GrenaderID || OB->newMons->Artilery );
						}
						if ( q > 1 )
						{
							BR->MembID = OB->NIndex;
						}
					}
				}
			}
		}
	}
}

void SaveDLL( ResFile f1 )
{
	int i = 'LLD.';
	RBlockWrite( f1, &i, 4 );
	i = 4 + 200;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, MapScenaryDLL, 200 );
}

void LoadDLL( ResFile f1 )
{
	RBlockRead( f1, MapScenaryDLL, 200 );
}

extern int RM_LandType;
extern int RM_Resstart;
extern int RM_Restot;
void SaveAIParam( ResFile f1 )
{
	int i = 'AIIA';
	RBlockWrite( f1, &i, 4 );
	i = 4 + 12;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, &RM_LandType, 4 );
	RBlockWrite( f1, &RM_Resstart, 4 );
	RBlockWrite( f1, &RM_Restot, 4 );
}

void LoadAIParam( ResFile f1 )
{
	RBlockRead( f1, &RM_LandType, 4 );
	RBlockRead( f1, &RM_Resstart, 4 );
	RBlockRead( f1, &RM_Restot, 4 );
}

void CloseMap( ResFile f1 )
{
	int i = 'MDNE';
	RBlockWrite( f1, &i, 4 );
	RClose( f1 );
}

void CreateMapPreview( byte* Data, int Lx, int Ly );
char* MPNAME;

bool GetPreviewName( char* Name, char* ccc )
{
	if ( Name[0] == 'R'&&Name[1] == 'N'&&Name[3] == ' ' )
	{
		/*
		//char cc1[64];
		//int x1, x2, x3;
		//int z = sscanf( Name, "%s%x%x%s%d", cc1, &x1, &x2, cc4, &x3 );
		if ( z == 5 )
		{
		*/
		char cc4[64] = { 0 };
		int x2 = 0;
		int x3 = 0;
		int z = sscanf( Name, "%*s %*s %x %s %d", &x2, cc4, &x3 );//BUGFIX: proper parsing
		if ( 3 == z )
		{
			strcpy( ccc, "Preview\\" );
			ccc[8] = '0' + ( ( ( x2 >> 8 ) & 15 ) == 5 );//cc[2]
			ccc[9] = '0' + ( x2 & 15 );
			int N = 0;
			for ( int v = 0; v < 8; v++ )
			{
				if ( cc4[v] != '0' )
				{
					N++;
				}
			}
			ccc[10] = '0' + N;
			x3 = ( ( x3 / 1000 ) % 10 );
			ccc[11] = x3 ? '1' : '0';
			ccc[12] = 0;
			return true;
		}
	}
	return false;
}

void SavePreview( ResFile f1 )
{
	byte* Preview = new byte[292 * 190];
	CreateMapPreview( Preview, 292, 190 );
	int i = 'WEIV';
	RBlockWrite( f1, &i, 4 );
	i = 4 + 292 * 190;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, Preview, 292 * 190 );
	//saving example
	if ( MPNAME[0] == 'R'&&MPNAME[1] == 'N'&&MPNAME[3] == ' ' )
	{
		char ccc[128];
		GetPreviewName( MPNAME, ccc );
		ResFile F2 = RRewrite( ccc );
		if ( F2 )
		{
			SaveHeader( F2 );
			i = 'WEIV';
			RBlockWrite( F2, &i, 4 );
			i = 4 + 292 * 190;
			RBlockWrite( F2, &i, 4 );
			RBlockWrite( F2, Preview, 292 * 190 );
			CloseMap( F2 );
			//RClose(F2);
		};
	};

	free( Preview );
};
void LoadPreview( ResFile f1, byte* Data )
{
	RBlockRead( f1, Data, 292 * 190 );
};
void ClearSMS();
void LoadSMSInMap( ResFile F );
void SaveSMSInMap( ResFile F );
extern byte* NatDeals;
#define NATLX (TopLx>>1)
extern int PeaceTimeLeft;
extern byte CordonIDX[8];

extern byte BalloonState;
extern byte CannonState;
extern byte NoArtilleryState;
extern byte XVIIIState;
extern byte DipCentreState;
extern byte ShipyardState;
extern byte MarketState;

extern int MaxPeaceTime;
extern int PeaceTimeStage;

void SavePeaceTimeInfo( ResFile f1 )
{
	if ( NatDeals )
	{
		int i = 'PEAC';
		RBlockWrite( f1, &i, 4 );
		int sz = 4 + NATLX*NATLX + 4 + 8 + 3;
		RBlockWrite( f1, &sz, 4 );
		RBlockWrite( f1, NatDeals, NATLX*NATLX );
		RBlockWrite( f1, &PeaceTimeLeft, 4 );
		MaxPeaceTime = PeaceTimeLeft;
		PeaceTimeStage = PeaceTimeLeft / 60;

		RBlockWrite( f1, CordonIDX, 8 );

		RBlockWrite( f1, &BalloonState, 1 );
		RBlockWrite( f1, &CannonState, 1 );
		RBlockWrite( f1, &NoArtilleryState, 1 );
		RBlockWrite( f1, &XVIIIState, 1 );
		RBlockWrite( f1, &DipCentreState, 1 );
		RBlockWrite( f1, &ShipyardState, 1 );
		RBlockWrite( f1, &MarketState, 1 );
	};
};
extern int RivNX;
extern byte* RivVol;
void SaveRivers( ResFile f1 )
{
	int i = '1VIR';
	RBlockWrite( f1, &i, 4 );
	i = RivNX*RivNX * 2 + 4;
	RBlockWrite( f1, &i, 4 );
	RBlockWrite( f1, RivDir, RivNX*RivNX );
	RBlockWrite( f1, RivVol, RivNX*RivNX );
}

void LoadRivers( ResFile f1 )
{
	RBlockRead( f1, RivDir, RivNX*RivNX );
	RBlockRead( f1, RivVol, RivNX*RivNX );
}

void LoadPeaceTimeInfo( ResFile f1 )
{
	if ( NatDeals )
	{
		free( NatDeals );
	}

	NatDeals = new byte[NATLX*NATLX];
	RBlockRead( f1, NatDeals, NATLX*NATLX );
	RBlockRead( f1, &PeaceTimeLeft, 4 );
	RBlockRead( f1, CordonIDX, 8 );
	RBlockWrite( f1, &BalloonState, 1 );
	RBlockWrite( f1, &CannonState, 1 );
	RBlockWrite( f1, &NoArtilleryState, 1 );
	RBlockWrite( f1, &XVIIIState, 1 );
	RBlockWrite( f1, &DipCentreState, 1 );
	RBlockWrite( f1, &ShipyardState, 1 );
	RBlockWrite( f1, &MarketState, 1 );
}

void Save3DMap( char* Map )
{
	MPNAME = Map;
	EraseAreas();
	ResFile f1 = RRewrite( Map );
	//if(IOresult())return;
	SaveHeader( f1 );
	SavePreview( f1 );
	SaveSurface( f1 );
	SaveTiles( f1 );
	SaveSect( f1 );
	SaveSprites( f1 );
	SaveUnits3( f1 );
	SaveNewWallsV2( f1 );
	SaveLockNew( f1 );
	SaveWaterCost( f1 );
	SaveRES( f1 );
	SaveTopology( f1 );
	SaveWTopology( f1 );
	SaveGates( f1 );
	SaveZonesAndGroups( f1 );
	SaveFormationsNew( f1 );
	SaveDLL( f1 );
	SaveAIParam( f1 );
	SaveSMSInMap( f1 );
	SavePeaceTimeInfo( f1 );
	SaveRivers( f1 );
	CloseMap( f1 );
};
void RenameAnyway( char* src, char* dst )
{
	DeleteFile( dst );
	rename( src, dst );
};
void FastSave3DMap( char* Map );
extern bool EditMapMode;
int ME_prevtime = 0;

//Autosave in map editor every 5 min
void ProcessMapAutosave()
{
	if ( !EditMapMode )
		return;

	if ( ME_prevtime == 0 )
	{
		ME_prevtime = GetTickCount();
	}
	if ( GetTickCount() - ME_prevtime > 300000 )
	{
		RenameAnyway( "MapAutosave.old.bak.m3d", "MapAutosave.old.old.bak.m3d" );
		RenameAnyway( "MapAutosave.bak.m3d", "MapAutosave.old.bak.m3d" );
		RenameAnyway( "MapAutosave.m3d", "MapAutosave.bak.m3d" );
		FastSave3DMap( "MapAutosave.m3d" );
		ME_prevtime = GetTickCount();
	}
}

void FastSave3DMap( char* Map )
{
	MPNAME = Map;
	//EraseAreas();
	ResFile f1 = RRewrite( Map );
	//if(IOresult())return;
	SaveHeader( f1 );
	//SavePreview(f1);
	SaveSurface( f1 );
	SaveTiles( f1 );
	SaveSect( f1 );
	SaveSprites( f1 );
	SaveUnits3( f1 );
	SaveNewWallsV2( f1 );
	SaveLockNew( f1 );
	SaveWaterCost( f1 );
	SaveRES( f1 );
	//SaveTopology(f1);
	//SaveWTopology(f1);
	SaveGates( f1 );
	SaveZonesAndGroups( f1 );
	SaveFormationsNew( f1 );
	SaveDLL( f1 );
	SaveAIParam( f1 );
	SaveSMSInMap( f1 );
	SavePeaceTimeInfo( f1 );
	SaveRivers( f1 );
	CloseMap( f1 );
};
void ClearRender();
void CreateMiniMap();
void ClearMaps();
void CheckMapName( char* Name );
void ClearTrianglesSystem();
void CreateTrianglesSystem();
void CreateWTopMap();
extern word NChAreas;
void ResearchCurrentIsland( byte Nat );
void CreateCostPlaces();
void CheckMapNameForStart( char* Name );

//Loads mapdata from file
void Load3DMap( char* Map )
{
	LockBars.Clear();

	UnLockBars.Clear();

	ClearSMS();

	MapScenaryDLL[0] = 0;

	ClearAllZones();

	CheckMapNameForStart( Map );

	for ( int i = 0; i < 8; i++ )
	{
		memset( NATIONS[i].NProduced, 0, sizeof NATIONS[i].NProduced );
	}

	ResFile f1 = RReset( Map );

	ClearMaps();

	if ( f1 == INVALID_HANDLE_VALUE )
	{
		return;
	}

	if ( !LoadHeader( f1 ) )
	{
		RClose( f1 );
		return;
	}

	int posit = 12;
	int sign, size;
	Loadingmap = 1;

	do
	{
		sign = 'MDNE';
		RBlockRead( f1, &sign, 4 );
		RBlockRead( f1, &size, 4 );
		posit += 4 + size;

		switch ( sign )
		{
		case 'FRUS':
			LoadSurface( f1 );
			break;

		case 'ELIT':
			LoadTiles( f1 );
			break;

		case 'TCES':
			LoadSect( f1 );
			break;

		case 'EERT':
			LoadSprites( f1 );
			break;

		case 'TINU':
			LoadUnits( f1 );
			break;

		case 'LLAW':
			LoadNewWalls( f1 );
			break;

		case '1LAW':
			LoadNewWallsV1( f1 );
			break;

		case '2LAW':
			LoadNewWallsV2( f1 );
			break;

		case '1COL':
			LoadLockNew( f1 );
			break;

		case '2AES':
			LoadWaterCost( f1 );
			break;

		case 'USER':
			LoadRES( f1 );
			break;

		case '2INU':
			LoadUnits2( f1 );
			break;

		case '3INU':
			LoadUnits3( f1 );
			break;

		case '1TAG':
			LoadGates( f1 );
			break;

		case '1NOZ':
			LoadZonesAndGroups( f1 );
			break;

		case '1POT':
			LoadTopology1( f1 );
			break;

		case 'WPOT':
			LoadWTopology1( f1 );
			break;

		case 'MROF':
			LoadFormations( f1 );
			break;

		case '1ROF':
			LoadFormationsNew( f1 );
			break;

		case 'LLD.':
			LoadDLL( f1 );
			break;

		case 'AIIA':
			LoadAIParam( f1 );
			break;

		case 'SMSP':
			LoadSMSInMap( f1 );
			break;

		case 'PEAC':
			LoadPeaceTimeInfo( f1 );
			break;

		case '1VIR':
			LoadRivers( f1 );
			break;

		default:
			RSeek( f1, posit );
		}
	} while ( sign != 'MDNE' );

	RClose( f1 );

	//CreateMapLocking();

	rando();//!!

	CreateTotalLocking();

	ClearRender();

	CreateMiniMap();

	ClearTrianglesSystem();

	CreateTrianglesSystem();

	if ( !WTopMap )
	{
		CreateWTopMap();
	}

	GTOP[0].LinksDist = LinksDist;
	GTOP[0].MotionLinks = MotionLinks;
	GTOP[0].NAreas = NAreas;
	GTOP[0].TopMap = TopMap;
	GTOP[0].TopRef = TopRef;
	NChAreas = 0;

	for ( int i = 0; i < MAXOBJECT; i++ )
	{
		OneObject* OB = Group[i];
		if ( OB&&OB->Wall && !OB->Sdoxlo )
		{
			DynamicalLockTopCell( OB->WallX, OB->WallY );
		}
	}

	for ( int q = 0; q < 8; q++ )
	{
		ResearchCurrentIsland( q );
	}

	CreateCostPlaces();

	Loadingmap = 0;
}

void Load3DMapLandOnly( char* Map )
{
	LockBars.Clear();
	UnLockBars.Clear();
	ClearSMS();
	MapScenaryDLL[0] = 0;
	CheckMapName( Map );
	ResFile f1 = RReset( Map );
	ClearMaps();
	if ( f1 == INVALID_HANDLE_VALUE )return;
	if ( !LoadHeader( f1 ) )
	{
		RClose( f1 );
		return;
	};
	int posit = 12;
	int sign, size;
	Loadingmap = 1;
	do
	{
		RBlockRead( f1, &sign, 4 );
		RBlockRead( f1, &size, 4 );
		posit += 4 + size;
		switch ( sign )
		{
		case 'FRUS':
			LoadSurface( f1 );
			break;
		case 'ELIT':
			LoadTiles( f1 );
			break;
		case 'TCES':
			LoadSect( f1 );
			break;
		case '1VIR':
			LoadRivers( f1 );
			break;
			//case 'EERT':
			//	LoadSprites(f1);
			//	break;
			//case 'TINU':
			//	LoadUnits(f1);
			//	break;
			//case 'LLAW':
			//	LoadNewWalls(f1);
			//	break;
			//case '1LAW':
			//	LoadNewWallsV1(f1);
			//	break;
			//case 'KCOL':
			//	LoadLock(f1);
			//	break;
		case '2AES':
			LoadWaterCost( f1 );
			break;
		case '1POT':
			LoadTopology1( f1 );
			break;
		case 'WPOT':
			LoadWTopology1( f1 );
			break;
		case 'LLD.':
			LoadDLL( f1 );
			break;
		case 'AIIA':
			LoadAIParam( f1 );
			break;
		case 'PEAC':
			LoadPeaceTimeInfo( f1 );
			break;
			//case 'USER':
			//	LoadRES(f1);
			//	break;
		default:
			RSeek( f1, posit );
		};
	} while ( sign != 'MDNE' );
	if ( !WTopMap )CreateWTopMap();
	RClose( f1 );
	ClearRender();
	ClearTrianglesSystem();
	CreateTrianglesSystem();
	NChAreas = 0;
	CreateCostPlaces();
	Loadingmap = 0;
};
void CreateNationalMaskForMap( char* Name )
{
	ResFile f1 = RReset( Name );
	if ( f1 == INVALID_HANDLE_VALUE )return;
	if ( !LoadHeader( f1 ) )
	{
		RClose( f1 );
		return;
	};
	LOADNATMASK = 0;
	int posit = 12;
	int sign, size;
	do
	{
		RBlockRead( f1, &sign, 4 );
		RBlockRead( f1, &size, 4 );
		posit += 4 + size;
		switch ( sign )
		{
		case '3INU':
			TestUnits3( f1 );
			goto GGG;
		case '2INU':
			TestUnits2( f1 );
			goto GGG;
		case 'TINU':
			TestUnits( f1 );
			goto GGG;
		default:
			RSeek( f1, posit );
		};
	} while ( sign != 'MDNE' );
GGG:;
	RClose( f1 );
}

bool GetPreview( char* Name, byte* Data )
{
	ResFile f1 = RReset( Name );
	if ( f1 == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	if ( !xLoadHeader( f1 ) )
	{
		RClose( f1 );
		return false;
	}

	int posit = 12;
	int sign, size;

	do
	{
		sign = 'MDNE';
		RBlockRead( f1, &sign, 4 );
		RBlockRead( f1, &size, 4 );
		posit += 4 + size;

		if ( sign == 'FRUS' )
		{
			RClose( f1 );
			return false;
		}

		if ( sign == 'WEIV' )
		{
			LoadPreview( f1, Data );
			RClose( f1 );
			return true;
		}
		else
		{
			RSeek( f1, posit );
		}
	} while ( sign != 'MDNE' );
	RClose( f1 );
	return false;
}
