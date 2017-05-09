#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "ActiveScenary.h"
#include "fog.h"
#include "Megapolis.h"

#include <assert.h>
#include "walls.h"
#include "mode.h"
#include "GSound.h"
#include "MapSprites.h"
#include "NewMon.h"
#include "Math.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "NewUpgrade.h"
#include "ZBuffer.h"
#include "Path.h"
#include "Transport.h"
#include "3DBars.h"
#include "CDirSnd.h"
#include "NewAI.h"
#include "3DMapEd.h"
#include "TopoGraf.h"

#include "Fonts.h"
#include "ConstStr.h"

extern const int kImportantMessageDisplayTime;
extern const int kMinorMessageDisplayTime;

bool CheapMode;
void PerformAction( OneObject* OB, int x, int y );
extern const int drr[9] = { 7,6,5,0,0,4,1,2,3 };
extern word LastObject;
void CreateTimedHint( char* s, int time );

//return 0-unable
//       n-busy(n=number of tasks)
//		 -1-ready
bool ApplyCost( byte NI, word NIndex );

//Returns the number of queued unit productions in this object
//word ID: id of unit
//const bool running_production: only count queued productions, not all available
int OneObject::CheckAbility( word ID, const bool running_production )
{
	if ( !NewBuilding )
	{
		return 0;
	}

	Nation* NT = Nat;
	word NInd = NIndex;

	int p = NT->PACount[NInd];

	int i;
	for ( i = 0; i < p; i++ )
	{
		if ( NT->PAble[NInd][i] == ID )
		{
			break;
		}
	}
	if ( i >= p )
	{
		return 0;
	}

	int retval = 0;
	Order1* order = LocalOrder;
	while ( nullptr != order )
	{
		if ( 13 == order->OrderType &&
			( !running_production || ID == order->info.Produce.ObjIndex ) )
		{//BUGFIX: Simultaneous queue management for groups of buildings
			//Upon cancelling queues count not all queued orders for unit
			//production, but only those orders with matching object indexes
			retval++;
		}
		order = order->NextOrder;
	}

	if ( 0 == retval )
	{
		retval = -1;
	}

	return retval;
}

extern MotionField UnitsField;
void MoveAwayShipInCell( int cell, int x, int y )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )return;
	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->LockType && ( !OB->Sdoxlo ) && !( OB->RealVx || OB->RealVy ) )
			{
				int NAtt = 40;
				do
				{
					NAtt--;
					int dx = ( rando() & 1023 ) - 512;
					int dy = ( rando() & 1023 ) - 512;
					int N = Norma( dx, dy );
					if ( N > 300 && N < 700 )
					{
						int xx = ( x >> 8 ) + ( dx >> 4 ) - 8;
						int yy = ( y >> 8 ) + ( dy >> 4 ) - 8;
						if ( !( MFIELDS[1].CheckBar( xx, yy, 16, 16 ) ||
							UnitsField.CheckBar( xx, yy, 16, 16 ) ) )
						{
							OB->NewMonsterSmartSendTo( ( OB->RealX >> 4 ) + dx, ( OB->RealY >> 4 ) + dy, 0, 0, 128, 0 );
							NAtt = -4;
						};
					};
				} while ( NAtt&&NAtt != -4 );
				if ( !NAtt )
				{
					NAtt = 40;
					do
					{
						NAtt--;
						int dx = ( rando() & 1023 ) - 512;
						int dy = ( rando() & 1023 ) - 512;
						int N = Norma( dx, dy );
						if ( N > 300 && N < 700 )
						{
							int xx = ( x >> 8 ) + ( dx >> 4 ) - 8;
							int yy = ( y >> 8 ) + ( dy >> 4 ) - 8;
							if ( !MFIELDS[1].CheckBar( xx, yy, 16, 16 ) )
							{
								OB->NewMonsterSmartSendTo( ( OB->RealX >> 4 ) + dx, ( OB->RealY >> 4 ) + dy, 0, 0, 128, 0 );
								NAtt = 0;
							};
						};
					} while ( NAtt );
				};
			};
		};
	};
};
void MoveAwayShip( int x, int y )
{
	int cell = ( ( y >> 11 ) << VAL_SHFCX ) + ( x >> 11 );
	int CELL0 = cell;
	int rx1 = 2;
	bool Capture = false;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCIOFS )
			{
				MoveAwayShipInCell( stcell, x, y );
			};
			stcell++;
		};
		stcell += VAL_MAXCX - rx2;
	};
};
void ProduceObjLink( OneObject* OBJ );
extern City CITY[8];
void CheckArmies( City* );
void OneObject::Produce( word ID )
{
	Produce( ID, 0xFFFF );
};
//extern int RESRC[8][8];
void GetUnitCost( byte NI, word NIndex, int* Cost, word Power );
void OneObject::Produce( word ID, word GroupID )
{
	bool PREVIEW = 0;
	if ( CheckOrderAbility() )return;
	if ( UnlimitedMotion )return;
	NewMonster* NM = newMons;
	if ( !( NM->Building&&Ready ) )return;
	int FAST = ID >> 13;
	ID &= 8191;
	//checking for infinity
	Order1* OR2 = LocalOrder;
	while ( OR2 )
	{
		if ( OR2->DoLink == &ProduceObjLink&&OR2->info.Produce.ObjIndex == ID
			&&OR2->info.Produce.ID == 0xFFFE )return;
		OR2 = OR2->NextOrder;
	};
	if ( GroupID == 0xFFFE )
	{
		OR2 = LocalOrder;
		Order1* ORP = NULL;
		while ( OR2 )
		{
			if ( OR2->DoLink == &ProduceObjLink&&OR2->info.Produce.ObjIndex == ID
				&&OR2->info.Produce.ID != 0xFFFE )
			{
				if ( ORP )
				{
					ORP->NextOrder = OR2->NextOrder;
				}
				else
				{
					LocalOrder = OR2->NextOrder;
				};
				Order1* OR3 = OR2->NextOrder;
				if ( OR2->info.Produce.Power != 0xFFFF )
				{
					if ( newMons->Port )
					{
						if ( OR2->info.Produce.Progress )
						{
							MFIELDS[1].BClrBar( WallX - 7, WallY - 7, 15 );
						};
					};
					GeneralObject* GO = Nat->Mon[ID];
					int Cost[8];
					GetUnitCost( NNUM, ID, Cost, OR2->info.Produce.Power );
					for ( int p = 0; p < 8; p++ )
					{
						AddXRESRC( NNUM, p, Cost[p] );
					};
				};
				FreeOrdBlock( OR2 );
				OR2 = OR3;
			}
			else
			{
				ORP = OR2;
				OR2 = OR2->NextOrder;
			};
		};
	};
	GeneralObject* GG = Nat->Mon[ID];
	Nation* NT = Nat;
	int Power = CITY[NNUM].UnitAmount[ID];
	if ( ( XRESRC( NNUM, FoodID ) < 500 && GroupID == 0xFFFE ) || !ApplyCost( NNUM, ID ) )
	{
		if ( GroupID == 0xFFFE )
		{
			PREVIEW = 1;
		}
		else return;
	}

	Order1* Or1 = CreateOrder( 2 );
	Or1->PrioryLevel = 96;
	Or1->OrderType = 13;//Produce
	Or1->OrderTime = 0;
	Or1->DoLink = &ProduceObjLink;
	Or1->info.Produce.ObjIndex = ID;
	Or1->info.Produce.PStep = 1;
	Or1->info.Produce.Progress = 0;
	Or1->info.Produce.NStages = Nat->Mon[ID]->MoreCharacter->ProduceStages;
	if ( FAST&&Or1->info.Produce.NStages > 30 * FAST )Or1->info.Produce.Progress = Or1->info.Produce.NStages - 30 * FAST;
	Or1->info.Produce.ID = GroupID;
	if ( PREVIEW )Or1->info.Produce.Power = 0xFFFF;
	else Or1->info.Produce.Power = Power;
}

bool ShowProducedShip( OneObject* Port, int CX, int CY )
{
	if ( Port )
	{
		Order1* OR1 = Port->LocalOrder;
		if ( OR1 && OR1->DoLink == &ProduceObjLink )
		{
			NewMonster* NM = Port->newMons;

			if ( !NM->Port )
			{
				return false;
			}

			GeneralObject* GO = NATIONS[Port->NNUM].Mon[OR1->info.Produce.ObjIndex];
			NewMonster* SNM = GO->newMons;
			int prog = OR1->info.Produce.Progress;

			if ( !prog )
			{
				return false;
			}

			int stage = div( prog << 2, OR1->info.Produce.NStages ).quot;
			if ( stage > 3 )
			{
				stage = 3;
			}

			NewAnimation* NA = &SNM->PMotionL[stage];
			if ( NA->Enabled )
			{
				NewFrame* NF = &NA->Frames[0];
				AddPoint( CX, CY, CX + NF->dx, CY + NF->dy, nullptr, NF->FileID, NF->SpriteID, 0, 0 );
				if ( NA->NFrames > 1 )
				{
					//reflection
					NF = &NA->Frames[1];
					AddOptPoint( ZBF_LO, CX, CY, CX + NF->dx, CY + NF->dy, nullptr, NF->FileID, NF->SpriteID, 256 + 512 );
				}
			}
			return true;
		}
	}
	return false;
}

extern MotionField UnitsField;
bool SubCost = false;
byte DetermineResource( int x, int y );
void GotoFinePosition( OneObject* OB );
int GetHeight( int, int );
void FindUnitPosition( int* x, int *y, NewMonster* NM );
extern int DiffP[4];
void UnProduce( OneObject* OB, word Type );
extern word* BLDList;
void GetRect( OneObject* ZZ, int* x, int* y, int* Lx, int* Ly );
bool PInside( int x, int y, int x1, int y1, int xp, int yp );
word CheckBuildingInCell( int cx, int cy, int dstx, int dsty )
{
	if ( cx >= 0 && cy >= 0 && cx < VAL_MAXCX&&cy < VAL_MAXCX )
	{
		word MID = BLDList[cx + ( cy << VAL_SHFCX )];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->NewBuilding&&OB->newMons->UnitAbsorber )
			{
				int xs, ys, xs1, ys1, LY;
				GetRect( OB, &xs, &ys, &xs1, &LY );
				ys1 = ys + LY - 1;
				xs1 += xs - 1;
				if ( PInside( xs, ys, xs1, ys1, dstx, dsty ) )return MID;
			};
		};
	};
	return 0xFFFF;
};
void ProduceObjLink( OneObject* OBJ )
{
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	NewMonster* NM = OBJ->newMons;
	Nation* NT = OBJ->Nat;
	if ( NT->NFarms < NT->NGidot + 1 )return;

	word OI = OBJ->LocalOrder->info.Produce.ObjIndex;
	word GID = OBJ->LocalOrder->info.Produce.ID;
	if ( OBJ->LocalOrder->info.Produce.Power == 0xFFFF )
	{
		OBJ->DeleteLastOrder();
		OBJ->Produce( OI, GID );
		return;
	};
	//if(int(NT->NFarms)*15<NT->NGidot+1){
	//	if(NT->NNUM==MyNation){
	//		sprintf(Prompt,"Недостаточно ферм.");
	//		PromptTime=100;
	//	};
	//	return;
	//};
	GeneralObject* PGO = NT->Mon[OI];
	AdvCharacter* PADC = PGO->MoreCharacter;
	NewMonster* ONM = PGO->newMons;
	if ( ONM->ArtSet&&NT->NArtUnits[ONM->ArtSet - 1] >= NT->NArtdep*ONM->NInArtDepot )
	{
		UnProduce( OBJ, OI );
		return;
	};
	byte NI = NT->NNUM;
	//int xcost=NT->Mon[OI]->cost;
	byte step = OBJ->LocalOrder->info.Produce.PStep << SpeedSh;
	int xxx = OBJ->LocalOrder->info.Produce.Progress;
	int nxxx = OBJ->LocalOrder->info.Produce.NStages;
	int pst = PADC->ProduceStages;
	if ( NT->AI_Enabled&&NT->CITY->Difficulty != 3 )
	{
		pst = ( pst*DiffP[NT->CITY->Difficulty] ) >> 8;
	};
	if ( nxxx != pst )
	{
		xxx = ( pst*xxx ) / nxxx;
		nxxx = pst;
		OBJ->LocalOrder->info.Produce.NStages = pst;
		OBJ->LocalOrder->info.Produce.Progress = xxx;
	};
	if ( NM->Port )
	{
		if ( !xxx )
		{
			if ( MFIELDS[1].CheckBar( OBJ->WallX - 7, OBJ->WallY - 7, 15, 15 ) ||
				UnitsField.CheckBar( OBJ->WallX - 7, OBJ->WallY - 7, 15, 15 ) )
			{
				MoveAwayShip( int( OBJ->WallX ) << 8, int( OBJ->WallY ) << 8 );
				return;
			};
			MFIELDS[1].BSetBar( OBJ->WallX - 7, OBJ->WallY - 7, 15 );
		};
	};
	if ( !PGO->Enabled )
	{
		if ( NM->Port )
		{
			MFIELDS[1].BClrBar( OBJ->WallX - 7, OBJ->WallY - 7, 15 );
		};
		OBJ->DeleteLastOrder();
		return;
	};
	//	byte NI=OBJ->NNUM;
	xxx += step;
	OBJ->LocalOrder->info.Produce.Progress = xxx;
	if ( xxx < nxxx )return;
	int xx, yy;
	OBJ->GetCornerXY( &xx, &yy );
	//if(!NM->NBornPt){
	//	OBJ->DeleteLastOrder();
	//};
	if ( NM->Port )
	{
		MFIELDS[1].BClrBar( OBJ->WallX - 7, OBJ->WallY - 7, 15 );
		SubCost = true;
		int ID = NT->CreateNewMonsterAt( int( OBJ->WallX ) << 8, int( OBJ->WallY ) << 8, OI, true );
		SubCost = false;
		if ( ID != -1 )
		{
			OneObject* OB = Group[ID];
			if ( OB )
			{
				OB->RealDir = 96;
				OB->GraphDir = OB->RealDir;
				if ( OBJ->DstX > 0 )
					OB->NewMonsterSmartSendTo( ( OBJ->DstX + ( ( rando() - 16384 ) >> 2 ) ) >> 4, ( OBJ->DstY + ( ( rando() - 16384 ) >> 2 ) ) >> 4, 0, 0, 128, 2 );
			};
			if ( GID <= 0xFFFE && GID < SCENINF.NUGRP )
			{
				UnitsGroup* UG = SCENINF.UGRP + GID;
				UG->IDS = (word*) realloc( UG->IDS, UG->N * 2 + 2 );
				UG->SNS = (word*) realloc( UG->SNS, UG->N * 2 + 2 );
				UG->IDS[UG->N] = OB->Index;
				UG->SNS[UG->N] = OB->Serial;
				UG->N++;
			};
		}
		else
		{
			OBJ->DeleteLastOrder();
			CreateTimedHintEx( UNILIMIT, kImportantMessageDisplayTime, 32 );//Unit limit has been reached!
			return;
		};

		OBJ->DeleteLastOrder();
		if ( GID == 0xFFFE )
		{
			OBJ->Produce( OI, 0xFFFE );
		};
		if ( OBJ->NNUM == MyNation && !OBJ->LocalOrder )
		{
			char ccc[200];
			sprintf( ccc, FINBLD, OBJ->Ref.General->Message );
			CreateTimedHint( ccc, kMinorMessageDisplayTime );//%s has finished creating units
			LastActionX = OBJ->RealX >> 4;
			LastActionY = OBJ->RealY >> 4;
		};
		return;
	};
	SubCost = true;
	int ID = NT->CreateNewMonsterAt( ( xx + NM->BornPtX[0] ) << 8, ( yy + NM->BornPtY[0] ) << 8, OI, true );

	SubCost = false;
	if ( ID != -1 )
	{
		OneObject* OB = Group[ID];
		NewMonster* NM1 = OB->newMons;
		{
			OB->SetOrderedUnlimitedMotion( 0 );
			if ( NM1->BornSoundID != -1 )AddEffect( OB->RealX >> 4, ( OB->RealY >> 5 ) - GetHeight( OB->RealX >> 4, OB->RealY >> 4 ), NM1->BornSoundID );
			int j;
			for ( j = 1; j < NM->NBornPt; j++ )
			{
				if ( j == NM->NBornPt - 1 )
				{
					OB->NewMonsterSendTo( ( ( xx + NM->BornPtX[j] ) << 8 ), ( ( yy + NM->BornPtY[j] ) << 8 ), 16, 2 );
				}
				else OB->NewMonsterSendTo( ( ( xx + NM->BornPtX[j] ) << 8 ), ( ( yy + NM->BornPtY[j] ) << 8 ), 16, 2 );
			};
			if ( GID < 0xFFFE )OB->ClearOrderedUnlimitedMotion( 2, GID );
			else OB->ClearOrderedUnlimitedMotion( 2, 0xFFFF );
			OB->NewMonsterSendTo( ( ( xx + NM->BornPtX[j - 1] ) << 8 ), ( ( yy + NM->BornPtY[j - 1] ) << 8 ), 16, 2 );

			if ( OBJ->DstX > 0 )
			{
				int DSTX = OBJ->DstX >> 4;
				int DSTY = OBJ->DstY >> 4;
				//determining if it possible to go inside
				int cx = DSTX >> 7;
				int cy = DSTY >> 7;
				word INS = 0xFFFF;
				int dyy = ( DSTY >> 1 ) - GetHeight( DSTX, DSTY );
				for ( int r = 0; r < 4; r++ )
				{
					char* xi = Rarr[r].xi;
					char* yi = Rarr[r].yi;
					int N = Rarr[r].N;
					for ( int j = 0; j < N; j++ )
					{
						word mid = CheckBuildingInCell( cx + xi[j], cy + yi[j], DSTX, dyy );
						if ( mid != 0xFFFF )INS = mid;
					};
				};
				if ( INS != 0xFFFF )
				{
					OB->GoToMine( INS, 16 + 128, 2 );
				}
				else
				{
					byte rk = DetermineResource( DSTX, DSTY );
					if ( rk < 0xFE && OB->newMons->Peasant )
					{
						int NN = 15;
						byte RK1 = 0xFE;
						int XXX, YYY;
						do
						{
							XXX = DSTX + ( ( rando() & 255 ) - 128 );
							YYY = DSTY + ( ( rando() & 255 ) - 128 );
							if ( XXX > 0 && YYY > 0 )
							{
								RK1 = DetermineResource( XXX, YYY );
							};
						} while ( NN > 0 && RK1 != rk );
						if ( RK1 == rk )OB->TakeResource( XXX, YYY, rk, 16, 2 );
						else OB->TakeResource( DSTX, DSTY, rk, 16, 2 );
					}
					else
					{
						int dx = OBJ->DstX;
						int dy = OBJ->DstY;
						FindUnitPosition( &dx, &dy, OB->newMons );
						OB->NewMonsterSmartSendTo( ( dx ) >> 4, ( dy ) >> 4, 0, 0, 128, 2 );
					};
				};
			};
			GotoFinePosition( OB );;
			if ( !OB->LockType )OB->UnlimitedMotion = true;
			OB->GroundState = 0;
		};
	}
	else
	{
		OBJ->DeleteLastOrder();
		CreateTimedHintEx( UNILIMIT, kImportantMessageDisplayTime, 32 );//Unit limit has been reached!
		return;
	}
	OBJ->DeleteLastOrder();
	if ( GID == 0xFFFE )
	{
		OBJ->Produce( OI, 0xFFFE );
	}

	if ( OBJ->NNUM == MyNation && !OBJ->LocalOrder )
	{
		char ccc[200];
		sprintf( ccc, FINBLD, OBJ->Ref.General->Message );
		CreateTimedHint( ccc, kMinorMessageDisplayTime );//%s has finished creating units.
		LastActionX = OBJ->RealX >> 4;
		LastActionY = OBJ->RealY >> 4;
	};
};
void GotoFinePositionLink( OneObject* OB )
{
	if ( OB->LockType )
	{
		OB->DeleteLastOrder();
		return;
	};
	if ( OB->BrigadeID != 0xFFFF )
	{
		OB->DeleteLastOrder();
		return;
	};
	if ( OB->NewAnm->NFrames - FrmDec <= OB->NewCurSprite )
	{
		OB->Sdoxlo = true;
		int xx = OB->RealX;
		int yy = OB->RealY;
		int xx0 = xx;
		int yy0 = yy;
		FindUnitPosition( &xx, &yy, OB->newMons );
		OB->Sdoxlo = 0;
		if ( xx == xx0&&yy == yy0 )
		{
			OB->DeleteLastOrder();
			return;
		}
		else
		{
			OB->NewMonsterSendTo( xx, yy, OB->PrioryLevel, 1 );
		};
	};
};
void GotoFinePosition( OneObject* OB )
{
	if ( OB->NewBuilding || OB->Wall )return;
	if ( OB->UnlimitedMotion )return;
	if ( OB->BrigadeID != 0xFFFF )return;
	Order1* OR1 = OB->CreateOrder( 2 );
	OR1->DoLink = &GotoFinePositionLink;
	OR1->PrioryLevel = OB->PrioryLevel;
};
struct ProCash
{
	word ID;
	byte NI;
	int Amount;
	int SubMoney[6];
};
ProCash UNICASH[32];
int CSSIZE = 0;
void GetUnitCost( byte NI, word NIndex, int* Cost );
//extern int RESRC[8][8];
void GetCorrectMoney( byte NI, int* MONEY );
void TakeUnitFromCash( byte NI, word ID )
{
	for ( int i = 0; i < CSSIZE; i++ )if ( UNICASH[i].ID == ID&&UNICASH[i].NI == NI )
	{
		if ( UNICASH[i].Amount > 1 )
		{
			UNICASH[i].Amount--;
		}
		else
		{
			if ( i < CSSIZE - 1 )
			{
				memcpy( UNICASH + i, UNICASH + i + 1, ( CSSIZE - i - 1 ) * sizeof ProCash );
			};
			CSSIZE--;
			i--;
		};
	};
};
bool AddUnitsToCash( byte NI, word ID )
{
	int cps = -1;
	int addc = 0;
	for ( int i = 0; i < CSSIZE&&cps == -1; i++ )if ( UNICASH[i].ID == ID&&UNICASH[i].NI == NI )cps = i;
	if ( cps == -1 )
	{
		if ( CSSIZE >= 32 )return true;
		cps = CSSIZE;
		CSSIZE++;
		addc = 1;
		memset( UNICASH + cps, 0, sizeof ProCash );
	};
	int COST[8];
	int CMON[8];
	GetUnitCost( NI, ID & 8191, COST );
	GetCorrectMoney( NI, CMON );
	for ( int i = 0; i < 6; i++ )if ( CMON[i] < COST[i] )
	{
		CSSIZE -= addc;
		return false;
	};
	for ( int i = 0; i < 6; i++ )UNICASH[cps].SubMoney[i] = COST[i];
	UNICASH[cps].ID = ID;
	UNICASH[cps].NI = NI;
	UNICASH[cps].Amount++;
	return true;
};
void ClearUniCash()
{
	CSSIZE = 0;
};
void GetCorrectMoney( byte NI, int* MONEY )
{
	for ( int i = 0; i < 6; i++ )MONEY[i] = XRESRC( NI, i );
	for ( int i = 0; i < CSSIZE; i++ )
	{
		if ( UNICASH[i].NI == NI )
		{
			for ( int j = 0; j < 6; j++ )
			{
				MONEY[j] -= UNICASH[i].SubMoney[j];
				if ( MONEY[j] < 0 )MONEY[j] = 0;
			};
		};
	};
};
int GetAmount( word ID )
{
	word CID = 0;
	word Nsel = ImNSL[MyNation];
	word* SMon = ImSelm[MyNation];
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OBJ = Group[MID];
			if ( int( OBJ ) )
			{
				Order1* OR1 = OBJ->LocalOrder;
				while ( int( OR1 ) )
				{
					if ( OR1->OrderType == 13 &&
						OR1->info.Produce.ObjIndex == ID )
					{
						if ( OR1->info.Produce.ID == 0xFFFE )CID += 1200;
						else CID++;
					};
					OR1 = OR1->NextOrder;
				};
			};
		};
	};
	for ( int i = 0; i < CSSIZE; i++ )
	{
		if ( UNICASH[i].NI == MyNation && UNICASH[i].ID == ID )
		{
			CID += UNICASH[i].Amount;
		}
	}
	return CID;
}

int GetProgress( word ID, int* MaxProgress )
{
	word CID = 0;
	word Nsel = ImNSL[MyNation];
	word* SMon = ImSelm[MyNation];
	for ( int i = 0; i < Nsel; i++ )
	{
		word MID = SMon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OBJ = Group[MID];
			if ( int( OBJ ) )
			{
				Order1* OR1 = OBJ->LocalOrder;
				if ( int( OR1 ) && OR1->OrderType == 13 &&
					OR1->info.Produce.ObjIndex == ID&&
					OR1->info.Produce.Progress > CID )
				{
					CID = OR1->info.Produce.Progress;
					*MaxProgress = OR1->info.Produce.NStages;
				};
			};
		};
	};
	return CID;
};