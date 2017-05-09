#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "mode.h"
#include "MapDiscr.h"
#include "multipl.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include "Nucl.h"

#include <assert.h>
#include "Megapolis.h"

#include "MapSprites.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "Path.h"
#include "NewMon.h"
#include "math.h"
#include "Sort.h"
#include "fonts.h"
#include "ActiveScenary.h"
#include "StrategyResearch.h"
#include "Safety.h"
#include "EinfoClass.h"

extern int PeaceTimeLeft;
void CheckArmies( City* CT );
void CorrectBrigadesSelection( byte NT );
void ImCorrectBrigadesSelection( byte NT );
void NewMonsterSmartSendToLink( OneObject* OBJ );
void NewMonsterSendToLink( OneObject* OBJ );
void AI_AttackPointLink( OneObject* OBJ );
extern word SelCenter[8];
int CheckCreationAbility( byte NI, NewMonster* NM, int* x2i, int* y2i, word* BLD, int NBLD );
void BuildWithSelected( byte NI, word ObjID, byte OrdType );
void GoToMineWithSelected( byte NI, word ID );
int GetBMIndex( OneObject* OB );

void Brigade::Init( City* ct, word id )
{
	memset( this, 0, sizeof Brigade );
	CT = ct;
	ID = id;
	ArmyID = 0xFFFF;
};
bool Brigade::RemoveOne( int Index, Brigade* Dest )
{
	if ( Index < 0 || Index >= NMemb )return false;
	OneObject* OB = Group[Memb[Index]];
	if ( ( !OB ) || OB->Serial != MembSN[Index] )return false;
	( &BM.Peons )[GetBMIndex( OB )]--;
	if ( Index < NMemb - 1 )
	{
		memcpy( Memb + Index, Memb + Index + 1, ( NMemb - Index - 1 ) << 1 );
		memcpy( MembSN + Index, MembSN + Index + 1, ( NMemb - Index - 1 ) << 1 );
		if ( PosCreated )
		{
			memcpy( posX + Index, posX + Index + 1, ( NMemb - Index - 1 ) << 2 );
			memcpy( posY + Index, posY + Index + 1, ( NMemb - Index - 1 ) << 2 );
		};
	};
	NMemb--;
	Dest->AddObject( OB );
	SetIndex();
	return true;
};
void Brigade::FreeMember( int Index )
{
	if ( Index < 0 || Index >= NMemb )return;
	OneObject* OB = Group[Memb[Index]];
	if ( ( !OB ) || OB->Serial != MembSN[Index] )return;
	( &BM.Peons )[GetBMIndex( OB )]--;
	if ( Index < NMemb - 1 )
	{
		memcpy( Memb + Index, Memb + Index + 1, ( NMemb - Index - 1 ) << 1 );
		memcpy( MembSN + Index, MembSN + Index + 1, ( NMemb - Index - 1 ) << 1 );
		if ( PosCreated )
		{
			memcpy( posX + Index, posX + Index + 1, ( NMemb - Index - 1 ) << 2 );
			memcpy( posY + Index, posY + Index + 1, ( NMemb - Index - 1 ) << 2 );
		};
	};
	NMemb--;
	OB->BrigadeID = 0xFFFF;
	OB->BrIndex = 0;
	SetIndex();
}

void Brigade::RemovePeasants( Brigade* Dest )
{
	int N = NMemb;
	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Memb[i]];
		if ( OB&&OB->Serial == MembSN[i] && OB->newMons->Peasant )
		{
			RemoveOne( i, Dest );
			i--;
			N--;
		}
	}
}

void Brigade::AddObject( OneObject* OB )
{
	//CheckBrIntegrity();
	/*
	byte CCC[100];
	memset(CCC,0,100);
	for(int k=0;k<NMemb;k++){
		if(Memb[k]<100){
			CCC[Memb[k]]++;
			assert(CCC[Memb[k]]<2);
		};
	};
	*/
	//CheckArmies(CT);
	if ( NMemb >= MaxMemb )
	{
		MaxMemb += 128;
		Memb = (word*) realloc( Memb, MaxMemb << 1 );
		MembSN = (word*) realloc( MembSN, MaxMemb << 1 );
	};
	OB->DoNotCall = false;
	PosCreated = false;
	Memb[NMemb] = OB->Index;
	MembSN[NMemb] = OB->Serial;
	OB->BrigadeID = ID;
	OB->BrIndex = NMemb;
	( &BM.Peons )[GetBMIndex( OB )]++;
	NMemb++;
	//CheckArmies(CT);
	/*
	memset(CCC,0,100);
	for(k=0;k<NMemb;k++){
		if(Memb[k]<100){
			CCC[Memb[k]]++;
			assert(CCC[Memb[k]]<2);
		};
	};
	*/
	MembID = OB->NIndex;
	//CheckBrIntegrity();
};
void Brigade::CheckMembers( City* pCT )
{
	if ( WarType )return;
	/*
	byte CCC[100];
	memset(CCC,0,100);
	for(int k=0;k<NMemb;k++){
		if(Memb[k]<100){
			CCC[Memb[k]]++;
			assert(CCC[Memb[k]]<2);
		};
	};
	*/
	/*
	for(int i=0;i<NMemb;i++){
		OneObject* OB=Group[Memb[i]];
		if(OB&&OB->BrIndex!=ID){
			MembSN[i]-=31;
		};
	};
	*/
	for ( int i = 0; i < NMemb; i++ )
	{
		OneObject* OB = Group[Memb[i]];
		if ( ( !OB ) || OB->Serial != MembSN[i] || OB->Sdoxlo )
		{
			( &BM.Peons )[GetBMIndex( OB )]--;
			if ( i < NMemb - 1 )
			{
				memcpy( Memb + i, Memb + i + 1, ( NMemb - i - 1 ) << 1 );
				memcpy( MembSN + i, MembSN + i + 1, ( NMemb - i - 1 ) << 1 );
				if ( PosCreated )
				{
					memcpy( posX + i, posX + i + 1, ( NMemb - i - 1 ) << 2 );
					memcpy( posY + i, posY + i + 1, ( NMemb - i - 1 ) << 2 );
				};
			};
			NMemb--;
			i--;
		};
	};
	SetIndex();
	//memset(CCC,0,100);
	if ( CFN )
	{
		for ( int i = 0; i < NMemb; i++ )
		{
			OneObject* OB = Group[Memb[i]];
			if ( OB )
			{
				int br = CFN( OB, pCT );
				if ( br != -1 )
				{
					RemoveOne( i, &pCT->Brigs[br] );
					i--;
					//NMemb--;
				};
			};
		};
	};
	/*
	memset(CCC,0,100);
	for(k=0;k<NMemb;k++){
		if(Memb[k]<100){
			CCC[Memb[k]]++;
			assert(CCC[Memb[k]]<2);
		};
	};
	*/
};
void Brigade::RemoveObjects( int NObj, Brigade* Dest )
{
	for ( int i = 0; i < NObj; i++ )
	{
		if ( NMemb )
		{
			OneObject* OB = Group[Memb[NMemb - 1]];
			if ( OB )
			{
				( &BM.Peons )[GetBMIndex( OB )]--;
				Dest->AddObject( OB );
			}
			NMemb--;
		}
	}

	SetIndex();
}

void DeleteFromGroups( byte NI, word ID );

void Brigade::DeleteAll()
{
	DeleteFromGroups( CT->NI, ID );
	ClearBOrders();
	if ( Memb )free( Memb );
	if ( MembSN )free( MembSN );
	Init( CT, ID );
}

void Brigade::SetIndex()
{
	for ( int i = 0; i < NMemb; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				OB->BrigadeID = ID;
				OB->BrIndex = i;
			}
		}
	}
}

Brigade* OneObject::GetBrigade()
{
	if ( BrigadeID == 0xFFFF )return NULL;
	Brigade* BRI = NULL;
	switch ( BrigadeID )
	{
	case 0://Free
		BRI = &Nat->CITY->Free;
		break;
	case 1://OnWood
		BRI = &Nat->CITY->OnWood;
		break;
	case 2://OnStone
		BRI = &Nat->CITY->OnStone;
		break;
	case 3:
		BRI = &Nat->CITY->OnField;
		break;
	case 4:
		BRI = &Nat->CITY->Builders;
		break;
	case 5:
		BRI = &Nat->CITY->InMines;
		break;
	case 6:
		BRI = &Nat->CITY->FreeArmy;
		break;
	};
	if ( !BRI )
	{
		BrigadeID = 0xFFFF;
		return NULL;
	};
	if ( BrIndex >= BRI->NMemb || Group[BRI->Memb[BrIndex]] != this )
	{
		BrigadeID = 0xFFFF;
		return NULL;
	};
	return BRI;
};
BrigadeOrder* Brigade::CreateOrder( byte Type, int Size )
{
	BrigadeOrder* OR1 = (BrigadeOrder*) malloc( Size );
	BrigadeOrder* OR2;
	switch ( Type )
	{
	case 1:
		OR1->Next = BOrder;
		BOrder = OR1;
		break;
	case 2:
		OR1->Next = NULL;
		if ( BOrder )
		{
			OR2 = BOrder;
			while ( OR2->Next )OR2 = OR2->Next;
			OR2->Next = OR1;
		}
		else BOrder = OR1;
		break;
	default:
		ClearBOrders();
		BOrder = OR1;
		OR1->Next = NULL;
	};
	return OR1;
};
void Brigade::DeleteBOrder()
{
	if ( BOrder )
	{
		BrigadeOrder* OR1 = BOrder->Next;
		free( BOrder );
		BOrder = OR1;
		//		if(OR1)assert(int(OR1->BLink)!=0xcdcdcdcd);
	};
};
void Brigade::ClearBOrders()
{
	while ( BOrder )DeleteBOrder();
};
void Brigade::CreateConvoy( byte Type )
{
	posX = (int*) realloc( posX, NMemb << 2 );
	posY = (int*) realloc( posY, NMemb << 2 );
	int LX = int( sqrt( NMemb ) );
	int LY = LX;
	if ( LX*LY < NMemb )LX++;
	if ( LX*LY < NMemb )LY++;
	int Ldis = 32;
	if ( Type == 1 )Ldis = 128;
	int dx = -( LX*Ldis ) >> 1;
	int dy = -( LY*Ldis ) >> 1;
	int n = 0;
	PosCreated = true;
	for ( int xx = 0; xx < LX; xx++ )
		for ( int yy = 0; yy < LY; yy++ )
		{
			posX[n] = ( xx*Ldis ) + dx;
			posY[n] = ( yy*Ldis ) + dx;
			n++;
			if ( n >= NMemb )return;
		};
};
bool CorrectOnePosition( int* x, int* y )
{
	int xx = *x;
	int yy = *y;
	for ( int r = 0; r < RRad; r++ )
	{
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		for ( int j = 0; j < N; j++ )
		{
			int x1 = xx + xi[j];
			int y1 = yy + yi[j];
			if ( !CheckBar( x1, y1, 6, 6 ) )
			{
				*x = x1;
				*y = y1;
				return true;
			};
		};
	};
	return false;
};
bool PosChanged;

void CheckTopPointPos( int xc, int yc, int* xd, int* yd )
{
	xc <<= 4;
	yc <<= 4;
	int x = ( *xd ) << 4;
	int y = ( *yd ) << 4;
	int mx = msx << 9;
	int my = msy << 9;

	if ( xc <= 0 )xc = 1;
	if ( yc <= 0 )yc = 1;
	if ( xc >= mx )xc = mx - 1;
	if ( yc >= my )yc = my - 1;

	if ( x <= 0 )x = 1;
	if ( y <= 0 )y = 1;
	if ( x >= mx )x = mx - 1;
	if ( y >= my )y = my - 1;

	int dx = xc - x;
	int dy = yc - y;
	int NRM = Norma( dx, dy );
	int N1 = ( NRM >> 9 ) + 1;
	dx /= N1;
	dy /= N1;
	for ( int j = 0; j < N1; j++ )
	{
		if ( xc > 0 && yc > 0 && xc < mx&&yc < my )
		{
			int tpx = xc >> 10;
			int tpy = yc >> 10;
			if ( SafeTopRef( tpx, tpy ) >= 0xFFFE )
			{
				xc -= dx;
				yc -= dy;
				if ( xc > 0 && yc > 0 && xc < mx&&yc < my )
				{
					*xd = xc >> 4;
					*yd = yc >> 4;
				};
				return;
			};
		};
	};
};
bool CorrectPositions( int* pos, word N )
{
	PosChanged = false;
	for ( int i = 0; i < N; i++ )
	{
		int ti = i + i + i;
		int xx = pos[ti];
		int yy = pos[ti + 1];
		int xs = ( xx - 48 ) >> 4;
		int ys = ( yy - 48 ) >> 4;
		if ( CheckBar( xs, ys, 6, 6 ) )
		{
			if ( !CorrectOnePosition( &xs, &ys ) )return false;
			pos[ti] = ( xs << 4 ) + 48;
			pos[ti + 1] = ( ys << 4 ) + 48;
			PosChanged = true;
		};
	};
	return true;
};
int Brigade::SelectPeasants( byte NI )
{
	SelCenter[NI] = 0;
	int np = 0;
	for ( int i = 0; i < NMemb; i++ )
	{
		OneObject* OB = Group[Memb[i]];
		if ( OB&&OB->Serial == MembSN[i] )
		{
			if ( OB->newMons->Peasant && !( OB->Sdoxlo || OB->Hidden ) )np++;
		};
	};
	if ( NSL[NI] )
	{
		int N = NSL[NI];
		for ( int i = 0; i < N; i++ )
		{
			word MID = Selm[NI][i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == SerN[NI][i] )OB->Selected &= ~GM( NI );
			};
		};
		free( Selm[NI] );
		free( SerN[NI] );
		Selm[NI] = NULL;
		SerN[NI] = NULL;
	};
	NSL[NI] = np;
	Selm[NI] = new word[np];
	SerN[NI] = new word[np];
	np = 0;
	for ( int i = 0; i < NMemb; i++ )
	{
		OneObject* OB = Group[Memb[i]];
		if ( OB&&OB->Serial == MembSN[i] )
		{
			if ( OB->newMons->Peasant && !( OB->Sdoxlo || OB->Hidden ) )
			{
				Selm[NI][np] = OB->Index;
				SerN[NI][np] = OB->Serial;
				OB->Selected |= GM( NI );
				np++;
			};
		};
	};
	return NSL[NI];
};
//------------------------------Local send to---------------------------//
int GetTopology( int x, int y )
{
	int xc = x >> 6;
	int yc = y >> 6;
	word tr;
	if ( xc < 0 || yc < 0 || xc >= TopLx || yc >= TopLy )tr = 0xFFFF;
	else tr = TopRef[xc + ( yc << TopSH )];
	if ( tr < 0xFFFE )return tr;
	for ( int i = 0; i < 20; i++ )
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for ( int j = 0; j < N; j++ )
		{
			int xx = xc + xi[j];
			int yy = yc + yi[j];
			if ( xx >= 0 && yy >= 0 && xx < TopLx&&yy < TopLy )
			{
				tr = TopRef[xx + ( yy << TopSH )];
				if ( tr < 0xFFFE )return tr;
			};
		};
	};
	return 0xFFFF;
};
int GetTopology( int* x, int* y )
{
	int xc = ( *x ) >> 6;
	int yc = ( *y ) >> 6;
	word tr;
	if ( xc < 0 || yc < 0 || xc >= TopLx || yc >= TopLy )tr = 0xFFFF;
	else tr = TopRef[xc + ( yc << TopSH )];
	if ( tr < 0xFFFE )return tr;
	for ( int i = 0; i < 20; i++ )
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for ( int j = 0; j < N; j++ )
		{
			int xx = xc + xi[j];
			int yy = yc + yi[j];
			if ( xx >= 0 && yy >= 0 && xx < TopLx&&yy < TopLy )
			{
				tr = TopRef[xx + ( yy << TopSH )];
				if ( tr < 0xFFFE )
				{
					*x = ( xx << 6 ) + 32;
					*y = ( yy << 6 ) + 32;
					return tr;
				};
			};
		};
	};
	return 0xFFFF;
};
class Brigade_LST :public BrigadeOrder
{
public:
	int time;
	int N;
	int ZastTime;
	int PrevSumm1;
	int PrevSumm2;
	int PrevSumm3;
	int StartTime;
	int Position[30];
};
extern int tmtmt;
void TakeResLink( OneObject* OBJ );
void AttackObjLink( OneObject* OBJ );
void B_LocalSendToLink( Brigade* BR )
{
	Brigade_LST* BLS = (Brigade_LST*) BR->BOrder;
	if ( tmtmt - BLS->StartTime > 300 )
	{
		BR->DeleteBOrder();
		return;
	};
	int* pos = BLS->Position;
	int N = BLS->N;
	int S = 0;
	for ( int i = 0; i < N; i++ )
	{
		int ti = i + i + i;
		int ID = pos[ti + 2];
		word SN = ID >> 13;
		ID &= 8191;
		OneObject* OB = Group[ID];
		if ( OB&&OB->Serial == SN && !OB->Sdoxlo )
		{
			S += OB->RealX + OB->RealY;
		};
	};
	if ( S == BLS->PrevSumm1&&S == BLS->PrevSumm2&&S == BLS->PrevSumm3&&tmtmt - BLS->StartTime > 50 )
	{
		BR->DeleteBOrder();
		return;
	};
	byte prio = BLS->Prio;
	for ( int i = 0; i < N; i++ )
	{
		int ti = i + i + i;
		int ID = pos[ti + 2];
		word SN = ID >> 13;
		ID &= 8191;
		OneObject* OB = Group[ID];
		if ( OB&&OB->Serial == SN && !OB->Sdoxlo )
		{
			if ( OB->LocalOrder )
			{
				if ( OB->EnemyID != 0xFFFF )
				{
					BR->DeleteBOrder();
					return;
				};
				if ( OB->LocalOrder->DoLink == &TakeResLink )
				{
					OB->NewMonsterSendTo( BLS->Position[ti] << 4, BLS->Position[ti + 1] << 4, ( prio & 127 ) + 128, 0 );
				};
			};
			int dstt = Norma( ( OB->RealX >> 4 ) - pos[ti], ( OB->RealY >> 4 ) - pos[ti + 1] );
			if ( dstt > 250 && !OB->LocalOrder )
			{
				OB->NewMonsterSendTo( BLS->Position[ti] << 4, BLS->Position[ti + 1] << 4, ( prio & 127 ) + 128, 0 );
			};
		};
	};
	BLS->PrevSumm3 = BLS
		->PrevSumm2;
	BLS->PrevSumm2 = BLS->PrevSumm1;
	BLS->PrevSumm1 = S;
}
char* LS_Mess = "[Local sent to]";
bool Brigade::LinearLocalSendTo( int x, int y, byte prio, byte OrdType )
{
	Brigade_LST* OR1 = (Brigade_LST*) CreateOrder( OrdType, sizeof( Brigade_LST ) - 120 + ( NMemb * 12 ) );
	OR1->Size = sizeof( Brigade_LST ) - 120 + ( NMemb * 12 );
	OR1->OrdType = 1;
	OR1->BLink = &B_LocalSendToLink;
	OR1->N = NMemb;
	OR1->time = -1;
	OR1->Message = LS_Mess;
	OR1->ZastTime = -1;
	OR1->Prio = prio;
	OR1->PrevSumm1 = 0;
	OR1->PrevSumm2 = 1;
	OR1->PrevSumm3 = 2;
	OR1->StartTime = tmtmt;
	if ( !PosCreated )CreateConvoy( 0 );
	int avx = 0;
	int avy = 0;
	int na = 0;
	for ( int i = 0; i < NMemb; i++ )
	{
		OneObject* OB = Group[Memb[i]];
		if ( OB )
		{
			avx += ( OB->RealX >> 4 );
			avy += ( OB->RealY >> 4 );
			na++;
		};
	};
	if ( !na )
	{
		DeleteBOrder();
		return false;
	};
	avx /= na;
	avy /= na;
	int DX = x - avx;
	int DY = y - avy;
	int NR = Norma( DX, DY );
	if ( !NR )
	{
		DeleteBOrder();
		return false;
	};
	int dx1 = DY * 70 / NR;
	int dy1 = -DX * 70 / NR;
	int dx0 = ( -dx1*na ) >> 1;
	int dy0 = ( -dy1*na ) >> 1;
	for ( int i = 0; i < NMemb; i++ )
	{
		int ti = i + i + i;
		OR1->Position[ti] = x + dx0;
		OR1->Position[ti + 1] = y + dy0;
		OR1->Position[ti + 2] = Memb[i] + ( int( MembSN[i] ) << 13 );
		dx0 += dx1;
		dy0 += dy1;
	};
	if ( !CorrectPositions( OR1->Position, NMemb ) )
	{
		DeleteBOrder();
		return false;
	};
	for ( int i = 0; i < NMemb; i++ )
	{
		int ti = i + i + i;
		OneObject* OB = Group[Memb[i]];
		if ( OB )OB->NewMonsterSendTo( OR1->Position[ti] << 4, OR1->Position[ti + 1] << 4, prio, 0 );
	};
	return true;
};
word GetDir( int, int );
bool Brigade::LocalSendTo( int x, int y, byte prio, byte OrdType )
{
	if ( WarType )
	{
		int N = 0;
		int xa = 0;
		int ya = 0;
		for ( int i = 2; i < NMemb; i++ )if ( Memb[i] != 0xFFFF )
		{
			OneObject* OB = Group[Memb[i]];
			if ( OB && !OB->Sdoxlo )
			{
				xa += OB->RealX >> 4;
				ya += OB->RealY >> 4;
				N++;
			};
		};
		if ( !N )return false;

		xa = div( xa, N ).quot;
		ya = div( ya, N ).quot;
		HumanLocalSendTo( x << 4, y << 4, GetDir( x - xa, y - ya ), prio, OrdType );
		return true;
	};
	//calculating center
	/*
	int N=0;
	int xa=0;
	int ya=0;
	for(int i=0;i<NMemb;i++){
		OneObject* OB=Group[Memb[i]];
		if(OB&&!OB->Sdoxlo){
			xa+=OB->RealX>>4;
			ya+=OB->RealY>>4;
			N++;
		};
	};
	if(!N)return false;

	xa=div(xa,N).quot;
	ya=div(ya,N).quot;
	*/
	//	assert(x<20000);
	Brigade_LST* OR1 = (Brigade_LST*) CreateOrder( OrdType, sizeof( Brigade_LST ) - 120 + ( NMemb * 12 ) );
	OR1->Size = sizeof( Brigade_LST ) - 120 + ( NMemb * 12 );
	OR1->OrdType = 1;
	OR1->BLink = &B_LocalSendToLink;
	OR1->N = NMemb;
	OR1->time = -1;
	OR1->Message = LS_Mess;
	OR1->ZastTime = -1;
	OR1->Prio = prio;
	OR1->PrevSumm1 = 0;
	OR1->PrevSumm2 = 1;
	OR1->PrevSumm3 = 2;
	OR1->StartTime = tmtmt;
	if ( !PosCreated )CreateConvoy( 0 );
	for ( int i = 0; i < NMemb; i++ )
	{
		int ti = i + i + i;
		OR1->Position[ti] = x + posX[i];
		OR1->Position[ti + 1] = y + posY[i];
		OR1->Position[ti + 2] = Memb[i] + ( int( MembSN[i] ) << 13 );
	};
	if ( !CorrectPositions( OR1->Position, NMemb ) )
	{
		DeleteBOrder();
		return false;
	};
	for ( int i = 0; i < NMemb; i++ )
	{
		int ti = i + i + i;
		OneObject* OB = Group[Memb[i]];
		if ( OB&&OB->Serial == MembSN[i] )OB->NewMonsterSendTo( OR1->Position[ti] << 4, OR1->Position[ti + 1] << 4, prio, 0 );
	};
	return true;
};

void B_WideLocalSendToLink( Brigade* BR )
{
	Brigade_LST* BLS = (Brigade_LST*) BR->BOrder;
	int* pos = BLS->Position;
	int N = BLS->N;
	bool moved = true;
	bool zastr = false;
	if ( BLS->time < -1 )BLS->time++;
	if ( BLS->time >= 0 )
	{
		if ( tmtmt - BLS->time > 350 )
		{
			zastr = true;
			BLS->time = -10;
		};
	};
	int NZast = 0;
	byte prio = BLS->Prio;
	for ( int i = 0; i < N; i++ )
	{
		int ti = i + i + i;
		int ID = pos[ti + 2];
		word SN = ID >> 13;
		ID &= 8191;
		OneObject* OB = Group[ID];
		if ( OB&&OB->Serial == SN && !OB->Sdoxlo )
		{
			if ( OB->EnemyID != 0xFFFF )
			{
				BR->DeleteBOrder();
				return;
			};
			int dstt = Norma( ( OB->RealX >> 4 ) - pos[ti], ( OB->RealY >> 4 ) - pos[ti + 1] );
			if ( dstt > 300 )
			{
				NZast++;
				if ( BLS->ZastTime != -1 )
				{
					if ( tmtmt - BLS->ZastTime > 600 )
					{
						if ( OB->newMons->Artilery )
						{
							BLS->ZastTime = tmtmt;
						}
						else OB->Die();
					};
				};
				moved = false;
				if ( zastr )
				{
					int xx = OB->RealX;
					int yy = OB->RealY;
					bool done = false;
					int nattm = 0;
					while ( ( !done ) && nattm < 6 )
					{
						int xx1 = xx + ( rando() & 2048 ) - 1024;
						int yy1 = yy + ( rando() & 2048 ) - 1024;
						if ( !CheckBar( ( xx1 >> 8 ) - 2, ( yy1 >> 8 ) - 2, 4, 4 ) )
						{
							done = true;
							OB->NewMonsterSendTo( xx1, yy1, prio, 0 );
						};
						nattm++;
					};
					if ( !done )OB->Die();
				};
			}
			else
			{
				if ( BLS->time == -1 )BLS->time = tmtmt;
			};
			if ( dstt >= 300 && !( OB->LocalOrder || OB->Sdoxlo ) )
			{
				OB->NewMonsterSendTo( pos[ti] << 4, pos[ti + 1] << 4, prio, 0 );
			};
		};
	};
	if ( BLS->ZastTime == -1 && NZast&&NZast < 4 )
	{
		BLS->ZastTime = tmtmt;
	};
	if ( rando() < 2000 )
	{
		Brigade_LST* OR1 = (Brigade_LST*) BR->BOrder;
		CorrectPositions( pos, N );
		int Nmx = BR->NMemb;
		if ( PosChanged )
		{
			for ( int i = 0; i < N&&i < Nmx; i++ )
			{
				int ti = i + i + i;
				OneObject* OB = Group[BR->Memb[i]];
				if ( OB&&OB->Serial == BR->MembSN[i] )OB->NewMonsterSendTo( OR1->Position[ti] << 4, OR1->Position[ti + 1] << 4, prio, 0 );
			};
		};
	};
	if ( moved )BR->DeleteBOrder();
};
char* WLS_Mess = "[Wide Local sent to]";
bool Brigade::WideLocalSendTo( int x, int y, byte prio, byte OrdType )
{
	//calculating center
	/*
	int N=0;
	int xa=0;
	int ya=0;
	for(int i=0;i<NMemb;i++){
		OneObject* OB=Group[Memb[i]];
		if(OB&&!OB->Sdoxlo){
			xa+=OB->RealX>>4;
			ya+=OB->RealY>>4;
			N++;
		};
	};
	if(!N)return false;

	xa=div(xa,N).quot;
	ya=div(ya,N).quot;
	*/
	//	assert(x<20000);
	Brigade_LST* OR1 = (Brigade_LST*) CreateOrder( OrdType, sizeof( Brigade_LST ) - 120 + ( NMemb * 12 ) );
	OR1->Size = sizeof( Brigade_LST ) - 120 + ( NMemb * 12 );
	OR1->OrdType = 1;
	OR1->BLink = &B_WideLocalSendToLink;
	OR1->N = NMemb;
	OR1->time = -1;
	OR1->Message = LS_Mess;
	OR1->ZastTime = -1;
	OR1->Prio = prio;
	if ( !PosCreated )CreateConvoy( 1 );
	for ( int i = 0; i < NMemb; i++ )
	{
		int ti = i + i + i;
		OR1->Position[ti] = x + posX[i];
		OR1->Position[ti + 1] = y + posY[i];
		OR1->Position[ti + 2] = Memb[i] + ( int( MembSN[i] ) << 13 );
	};
	if ( !CorrectPositions( OR1->Position, NMemb ) )
	{
		DeleteBOrder();
		return false;
	};
	for ( int i = 0; i < NMemb; i++ )
	{
		int ti = i + i + i;
		OneObject* OB = Group[Memb[i]];
		if ( OB&&OB->Serial == MembSN[i] )OB->NewMonsterSendTo( OR1->Position[ti] << 4, OR1->Position[ti + 1] << 4, prio, 0 );
	};
	return true;
};
int Brigade::AddInRadius( int x, int y, int r, BrigMemb* BMem, Brigade* Dest )
{
	word* IDIS = new word[NMemb];
	int NIDS = 0;
	//word* NKINDS=&BMem->Peons;
	word* NKI = &BMem->Peons;
	//SortClass SORT;
	for ( int i = 0; i < 6; i++ )
	{
		word id1 = IDIS[i];
		int n = 0;
		for ( int j = 0; j < NMemb; j++ )
		{
			OneObject* OB = Group[Memb[j]];
			if ( OB&&Norma( ( OB->RealX >> 4 ) - x, ( OB->RealY >> 4 ) - y ) < r )
			{
				int k = GetBMIndex( OB );
				if ( NKI[k] )
				{
					IDIS[NIDS] = j;
					NKI[k]--;
				};
			};
		};
	};
	int nn = 0;
	for ( int i = 0; i < NIDS; i++ )
	{
		if ( RemoveOne( IDIS[i] - nn, Dest ) )
		{
			nn++;
		}
	}
	free( IDIS );
	return BMem->Grenaderov + BMem->Infantry + BMem->Mortir + BMem->Peons + BMem->Pushek + BMem->Strelkov;
}

void BuildObjLink( OneObject* OBJ );
void GoToMineLink( OneObject* OB );

int Brigade::AddInRadius( int x, int y, int r, BrigMemb* pBM )
{
	int xc = x >> 7;
	int yc = y >> 7;
	r = ( r >> 7 ) + 1;
	if ( r >= RRad )r = RRad - 1;
	int maxx = msx >> 2;
	int maxy = msy >> 2;
	byte NI = CT->NI;
	word MyID = ID;
	int M = pBM->Grenaderov + pBM->Infantry + pBM->Mortir + pBM->Peons + pBM->Pushek + pBM->Strelkov;
	word* mem = (word*) pBM;
	for ( int i = 0; i < r; i++ )
	{
		char* xi = Rarr[i].xi;
		char* yi = Rarr[i].yi;
		int N = Rarr[i].N;
		for ( int j = 0; j < N; j++ )
		{
			int xx = xc + xi[j];
			int yy = yc + yi[j];
			if ( xx >= 0 && yy >= 0 && xx <= maxx&&yy <= maxy )
			{
				int ofst = xx + 1 + ( ( yy + 1 ) << VAL_SHFCX );
				int N1 = MCount[ofst];
				if ( N1 )
				{
					ofst <<= SHFCELL;
					for ( int k = 0; k < N1; k++ )
					{
						word MID = GetNMSL( ofst + k );
						if ( MID != 0xFFFF )
						{
							OneObject* OB = Group[MID];
							if ( OB&&OB->NNUM == NI && !( OB->DoNotCall || OB->Sdoxlo || OB->InArmy ) )
							{
								byte nn = GetBMIndex( OB );
								if ( mem[nn] )
								{
									bool Allow = true;
									Order1* OR1 = OB->LocalOrder;
									if ( OR1&&OR1->DoLink == NewMonsterSendToLink&&OR1->NextOrder )
									{
										OR1 = OR1->NextOrder;
									}
									if ( OR1&&OR1->DoLink == NewMonsterSmartSendToLink&&OR1->NextOrder )
									{
										OR1 = OR1->NextOrder;
									}
									if ( ( !OR1 ) || ( OR1->DoLink != &BuildObjLink&&OR1->DoLink != GoToMineLink ) )
									{
										if ( OB->BrigadeID != 0xFFFF )
										{
											if ( OB->BrigadeID != MyID )
											{
												Brigade* SRC = CT->Brigs + OB->BrigadeID;
												if ( SRC->Memb&&SRC->Memb[OB->BrIndex] == OB->Index )
												{
													SRC->RemoveOne( OB->BrIndex, this );
													mem[nn]--;
													M--;
												}
											}
										}
										else
										{
											AddObject( OB );
											mem[nn]--;
											M--;
										}

										if ( !M )
											return 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return M;
}

//------------------------------Global send to---------------------------//
class Brigade_GST :public BrigadeOrder
{
public:
	bool Final;
	int DestX;
	int DestY;
};
void B_GlobalSendToLink( Brigade* BR )
{
	Brigade_GST* OR1 = (Brigade_GST*) BR->BOrder;
	if ( OR1->Final )
	{
		BR->DeleteBOrder();
		return;
	};
	int xc = 0;
	int yc = 0;
	int N = 0;
	int NM = BR->NMemb;
	int Dsx = OR1->DestX;
	int Dsy = OR1->DestY;
	word Top = GetTopology( Dsx, Dsy );
	if ( Top == 0xFFFF )
	{
		BR->DeleteBOrder();
		return;
	};
	for ( int i = 0; i < NM; i++ )
	{
		OneObject* OB = Group[BR->Memb[i]];
		if ( OB&&OB->Serial == BR->MembSN[i] )
		{
			xc += OB->RealX >> 4;
			yc += OB->RealY >> 4;
			N++;
		};
	};
	if ( N )
	{
		xc = div( xc, N ).quot;
		yc = div( yc, N ).quot;
		word Top1 = GetTopology( xc, yc );
		if ( Top1 == 0xFFFF )
		{
			BR->DeleteBOrder();
			return;
		};
		word NextTop = MotionLinks[Top*NAreas + Top1];
		if ( NextTop == Top )
		{
			OR1->Final = true;
			BR->LocalSendTo( Dsx, Dsy, 128, 1 );
		}
		else
		{
			Area* AR = TopMap + Top1;
			BR->LocalSendTo( ( AR->x << 6 ) + 32, ( AR->y << 6 ) + 32, 128, 1 );
		};
	}
	else
	{
		BR->DeleteBOrder();
	};
};
char* GS_Mess = "[Global send to]";
bool Brigade::GlobalSendTo( int x, int y, byte prio, byte OrdType )
{
	Brigade_GST* OR1 = (Brigade_GST*) CreateOrder( OrdType, sizeof( Brigade_GST ) );
	OR1->DestX = x;
	OR1->DestY = y;
	OR1->Final = false;
	OR1->Message = GS_Mess;
	OR1->Size = sizeof( Brigade_GST );
	OR1->BLink = &B_GlobalSendToLink;
	return true;
};
//---------------------------Capture mine-------------------------//
class Brigade_CM :public BrigadeOrder
{
public:
	int time;
	int Stage;
	int SID;
	int  DestX;
	int  DestY;
};
void AllowBuilder( Brigade* BR )
{
	int N = BR->NMemb;
	for ( int i = 0; i < N; i++ )
	{
		word mid = BR->Memb[i];
		if ( mid != 0xFFFF )
		{
			OneObject* OB = Group[mid];
			if ( OB&&OB->Serial == BR->MembSN[i] )
			{
				OB->NoBuilder = false;
			};
		};
	};
};
OneObject* DetermineMineBySprite( int Spr );
void MoveAway( int x, int y );
void B_CaptureMineLink( Brigade* BR )
{
	Brigade_CM* OR1 = (Brigade_CM*) BR->BOrder;
	City* CT = BR->CT;
	Nation* NT = CT->Nat;
	//	assert(NT);
	if ( OR1->time )
	{
		OR1->time--;
		return;
	};
	int xc = 0;
	int yc = 0;
	int N = 0;
	int NM = BR->NMemb;
	int Dsx = OR1->DestX;
	int Dsy = OR1->DestY;
	word Top = GetTopology( Dsx, Dsy );
	if ( Top == 0xFFFF )
	{
		BR->DeleteBOrder();
		return;
	};
	for ( int i = 0; i < NM; i++ )
	{
		OneObject* OB = Group[BR->Memb[i]];
		if ( OB&&OB->Serial == BR->MembSN[i] )
		{
			xc += OB->RealX >> 4;
			yc += OB->RealY >> 4;
			N++;
			OB->NoBuilder = true;
			OB->DoNotCall = true;
			OB->DoWalls = false;
			//			assert(OB->InArmy==0);
		};
	};
	if ( N )
	{
		xc = div( xc, N ).quot;
		yc = div( yc, N ).quot;
		word Top1 = GetTopology( xc, yc );
		if ( Top1 == 0xFFFF )
		{
			AllowBuilder( BR );
			BR->DeleteBOrder();
			BR->Rospusk();
			return;
		};
		word NextTop = MotionLinks[Top1*NAreas + Top];
		if ( Top1 != Top&&NextTop >= 0xFFFE )
		{
			BR->DeleteBOrder();
			return;
		};
		if ( Top1 == Top || NextTop == Top )
		{
			OneObject* MIN = DetermineMineBySprite( OR1->SID );
			MineBase* BMS = (MineBase*) CT->SearchInform( 0x4519, OR1->SID, NULL );
			if ( MIN )
			{
				if ( Norma( ( MIN->RealX >> 4 ) - xc, ( MIN->RealY >> 4 ) - yc ) < 350 )
				{
					if ( MIN->NNUM == BR->CT->NI )
					{
						Brigade* BR1 = BR;
						if ( !BMS )
						{
							/*
							BR1=&CT->Brigs[BMS->MinersID];
							if(BR!=BR1){
								if(BR!=BR1)BR->RemovePeasants(BR1);
								Brigade* BR2=&CT->Brigs[BMS->DefendersID];
								if(BR!=BR2)BR->RemoveObjects(BR->NMemb,BR2);
								//BR->DeleteAll();
								//BR->Enabled=false;
								//BR=NULL;
							};
						}else{
						*/
							int Bpid = 0xFFFF;//BR->CT->GetFreeBrigade();
							BR1 = BR;
							//if(i==-1){
							//	BR->DeleteBOrder();
							//	return;
							//};
							//BR1=&CT->Brigs[Bpid];
							//BR1->Enabled=true;
							//BR->RemovePeasants(BR1);
							//for(int j=0;j<BR1->NMemb;j++){
							//	OneObject* OB=Group[BR1->Memb[j]];
							//	if(OB){
							//		OB->DoNotCall=true;
							//		OB->NoBuilder=true;
							//	};
							//};
							BMS = new MineBase;
							BMS->DefendersID = BR->ID;
							BMS->DefendersSN = BR->SN;
							BMS->MinersID = 0xFFFF;//BR1->ID;
							BMS->MinersSN = 0xFFFF;//BR1->SN;
							//BMS->NGates=0;
							//BMS->NGateProj=0;
							//BMS->Gates=NULL;
							//BMS->GateProj=NULL;
							BMS->M_ID = MIN->Index;
							BMS->M_SN = MIN->Serial;
							//BMS->NWalls=0;
							//BMS->Walls=NULL;
							BMS->Size = sizeof MineBase;
							BMS->topx = MIN->RealX >> 10;
							BMS->topy = MIN->RealY >> 10;
							BMS->ID = 0x4519;
							BMS->Essence = OR1->SID;
							BMS->UpgLevel = 0;
							//BMS->TowID=0xFFFF;
							//BMS->TowSN=0xFFFF;
							BMS->PSend = 0;
							OneSprite* OS = &Sprites[OR1->SID];
							ObjCharacter* OC = OS->OC;
							//SprGroup* SG=OS->SG;
							//ObjCharacter* OC=SG->ObjChar[OS->SGIndex];
							BMS->ResKind = OC->IntResType;
							CT->AddInform( BMS, NULL );
						};

						int nn = BR1->SelectPeasants( CT->NI );
						if ( nn )
						{
							if ( MIN->Ready )
							{
								int Bpid = BR->CT->GetFreeBrigade();
								if ( Bpid == -1 )
								{
									BR->DeleteBOrder();
									return;
								};
								BR1 = &CT->Brigs[Bpid];
								BR1->Enabled = true;
								BR->RemovePeasants( BR1 );
								for ( int j = 0; j < BR1->NMemb; j++ )
								{
									OneObject* OB = Group[BR1->Memb[j]];
									if ( OB )
									{
										OB->DoNotCall = true;
										OB->NoBuilder = true;
										OB->DoWalls = false;
									};
								};
								BMS->MinersID = BR1->ID;
								BMS->MinersSN = BR1->SN;
								GoToMineWithSelected( CT->NI, MIN->Index );
								SendPInform* SIF = (SendPInform*) CT->SearchInform( 0x1256, OR1->SID, NULL );
								if ( SIF )
								{
									SIF->time = tmtmt - SendPTime + 300;
								};
								BR->DeleteBOrder();
								if ( !BR->NMemb )
								{
									BR->DeleteAll();
									BR->Enabled = false;
								};
								return;
							}
							else
							{
								CmdBuildObj( CT->NI, MIN->Index );
								OR1->time = 15;
								return;
							};
						}
						else
						{
							if ( BR )
							{
								BR->DeleteBOrder();
							};
							return;
						};
					};
				}
				else BR->LocalSendTo( Dsx, Dsy, 128, 1 );;
			}
			else
			{
				//need to build
				int mineID = NT->CreateNewMonsterAt( ( Dsx + ( rando() & 63 ) - 32 ) << 4, ( Dsy + ( rando() & 63 ) - 32 ) << 4, NT->UID_MINE, false );
				if ( mineID != -1 )
				{
					return;
				}
				else
				{
					if ( rando() < 2000 )MoveAway( Dsx << 4, Dsy << 4 );
					OR1->time = 5;
					rando();
					return;
				};
				return;
			};
			BR->LocalSendTo( Dsx, Dsy, 128, 1 );
		}
		else
		{
			Area* AR = TopMap + NextTop;
			BR->LocalSendTo( ( AR->x << 6 ) + 32, ( AR->y << 6 ) + 32, 128, 1 );
		};
	}
	else
	{
		AllowBuilder( BR );
		BR->DeleteBOrder();
		if ( !BR->NMemb )
		{
			BR->DeleteAll();
			BR->Enabled = false;
		};
	};
};
char* CM_Mess = "[Capture mine]";
bool Brigade::CaptureMine( int SID, byte prio, byte OrdType )
{
	Brigade_CM* OR1 = (Brigade_CM*) CreateOrder( OrdType, sizeof( Brigade_CM ) );
	OneSprite* OS = &Sprites[SID];
	if ( !OS->Enabled )return false;
	OR1->DestX = OS->x;
	OR1->DestY = OS->y;
	OR1->Message = CM_Mess;
	OR1->SID = SID;
	OR1->time = 0;
	OR1->Stage = 0;
	OR1->Size = sizeof( Brigade_CM );
	OR1->BLink = &B_CaptureMineLink;
	return true;
};
void Brigade::Rospusk()
{
	int N = NMemb;
	for ( int i = 0; i < N; i++ )
	{
		word mid = Memb[i];
		if ( mid != 0xFFFF )
		{
			OneObject* OB = Group[mid];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				if ( OB->newMons->Peasant )
				{
					RemoveOne( i, &CT->Free );
					i--;
					N--;
				}
				else
				{
					RemoveOne( i, &CT->FreeArmy );
					i--;
					N--;
				};
			};
		};
	};
};
//----------------------------------WAR----------------------------------
class Brigade_BTL :public BrigadeOrder
{
public:
	int xdest;
	int ydest;
	int time;
	word BestEnemy;
	word EnemyList[64];
	word NEnemy;
};
void B_MakeBattleLink( Brigade* BR )
{
	Brigade_BTL* OR1 = (Brigade_BTL*) BR->BOrder;
	if ( !BR->NMemb )
	{
		BR->DeleteAll();
		BR->Enabled = false;
	};
	int xc = 0;
	int yc = 0;
	int N = 0;
	int NM = BR->NMemb;
	for ( int i = 0; i < NM; i++ )
	{
		word mid = BR->Memb[i];
		if ( mid != 0xFFFF )
		{
			OneObject* OB = Group[mid];
			if ( OB&&OB->Serial == BR->MembSN[i] )
			{
				//if(OB->LocalOrder&&OB->LocalOrder->DoLink==&AttackObjLink)return;
				if ( OB->EnemyID != 0xFFFF )return;
				OB->DoNotCall = true;
				OB->NoBuilder = true;
				OB->DoWalls = false;
				xc += OB->RealX >> 4;
				yc += OB->RealY >> 4;
				N++;
			};
		};
	};
	if ( N )
	{
		xc = div( xc, N ).quot;
		yc = div( yc, N ).quot;
	}
	else
	{
		BR->DeleteAll();
		BR->Enabled = false;
		return;
	};
	int Dsx = xc;
	int Dsy = yc;
	if ( !GNFO.FindNearestEnemy( BR->CT->NI, &Dsx, &Dsy, false, 0, 1 ) )
	{
		return;
	};
	word Top = GetTopology( Dsx, Dsy );
	if ( Top == 0xFFFF )
	{
		//BR->DeleteBOrder();
		return;
	};
	word Top1 = GetTopology( xc, yc );
	if ( Top1 == 0xFFFF )
	{
		BR->DeleteBOrder();
		return;
	};
	word NextTop = MotionLinks[Top1*NAreas + Top];
	if ( NextTop == 0xFFFF )return;
	if ( NextTop == Top )
	{
		//OR1->Final=true;
		BR->LocalSendTo( Dsx, Dsy, 128, 1 );
	}
	else
	{
		Area* AR = TopMap + NextTop;
		BR->LocalSendTo( ( AR->x << 6 ) + 32, ( AR->y << 6 ) + 32, 128, 1 );
	};
};
void SetStandState( Brigade* BR, byte State );
char* BTL_Mess = "[Make Battle]";
void Brigade::MakeBattle()
{
	Brigade_BTL* OR1 = (Brigade_BTL*) CreateOrder( 0, sizeof( Brigade_BTL ) );
	OR1->Message = BTL_Mess;
	OR1->time = 0;
	OR1->xdest = -1;
	OR1->ydest = -1;
	OR1->Size = sizeof( Brigade_BTL );
	OR1->BLink = &B_MakeBattleLink;
	return;
};
//---------------------------Protect mines--------------------------//
class  PFM_InfoSingle :public Inform
{
public:
	int time;
};
class  Brigade_PFM :public BrigadeOrder
{
public:
	word SID;
	word M_ID;
	word M_SN;
	int  xtop;
	int  ytop;
	word TowID;
	word TowSN;
	word Delay;
	word Time;
};
MineBase* CorrectMB( MineBase* MB )
{
	if ( MB->ID != 0xFFFF )
	{
		OneObject* OB = Group[MB->M_ID];
		if ( OB&&OB->Serial == MB->M_SN )return MB;
		else return NULL;
	}
	else
	{
		return NULL;
	}
}

void B_ProtectOrder( Brigade* BR )
{
}

char* PFM_Mess = "[Protect mines]";
void Brigade::ProtectFarMines()
{
	Brigade_PFM* BPF = (Brigade_PFM*) CreateOrder( 0, sizeof Brigade_PFM );
	BPF->Delay = 0;
	BPF->Time = 0;
	BPF->TowID = 0xFFFF;
	BPF->TowSN = 0xFFFF;
	BPF->M_ID = 0xFFFF;
	BPF->M_SN = 0xFFFF;
	BPF->SID = 0xFFFF;
	BPF->xtop = 0;
	BPF->ytop = 0;
	BPF->BLink = &B_ProtectOrder;
	BPF->Message = PFM_Mess;
	BPF->Prio = 0;
	BPF->Size = sizeof Brigade_PFM;
	BOrder = BPF;
}

bool Brigade::GetCenter( int* x, int* y )
{
	if ( !NMemb )return false;
	int xx = 0;
	int yy = 0;
	int N = 0;
	for ( int i = 0; i < NMemb; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MembSN[i] )
			{
				xx += ( OB->RealX >> 4 );
				yy += ( OB->RealY >> 4 );
				N++;
			};
		};
	};
	if ( N )
	{
		*x = xx / N;
		*y = yy / N;
		return true;
	}
	else return false;
};
bool GetBrCenter( Brigade* BR, int* x, int* y )
{
	if ( BR->NMemb < 2 )return false;
	if ( BR->PosCreated )
	{
		int xs = 0;
		int ys = 0;
		int Nu = BR->NMemb;
		for ( int i = 2; i < Nu; i++ )
		{
			xs += BR->posX[i];
			ys += BR->posY[i];
		};
		xs /= ( Nu - 2 );
		ys /= ( Nu - 2 );
		*x = xs;
		*y = ys;
		return true;
	};
	return BR->GetCenter( x, y );
};
//------------------Human functions--------------

int FindUnitsInCell( int cell, int x, int y, int r, int Type, byte Nation, int Need, word* Dest )
{
	int N = 0;
	if ( !Need )return 0;
	if ( cell > 0 )
	{
		cell += VAL_MAXCX + 1;
		if ( cell >= VAL_MAXCIOFS )return 0;
		int NMon = MCount[cell];
		if ( !NMon )return NULL;
		int ofs1 = cell << SHFCELL;
		word MID;
		for ( int i = 0; i < NMon; i++ )
		{
			MID = GetNMSL( ofs1 + i );
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && ( !( OB->Guard != 0xFFFF || OB->Sdoxlo ) ) && OB->BrigadeID == 0xFFFF && OB->NNUM == Nation&&OB->NIndex == Type&&Norma( OB->RealX - x, OB->RealY - y ) < r )
				{
					Dest[N] = OB->Index;
					N++;
					if ( N >= Need )return N;
				};
			};
		};
	};
	return N;
};
int FindUnits( int x, int y, int r, int Type, byte Nation, int Need, word* Dest )
{
	int rx1 = ( r >> 11 ) + 1;
	int N = 0;
	int x0 = x >> 11;
	int y0 = y >> 11;
	int maxx = msx >> 2;
	for ( int R = 0; R <= rx1; R++ )
	{
		char* xi = Rarr[R].xi;
		char* yi = Rarr[R].yi;
		int NN = Rarr[R].N;
		for ( int p = 0; p < NN; p++ )
		{
			int xx = x0 + xi[p];
			int yy = y0 + yi[p];
			if ( xx >= 0 && yy >= 0 && xx < maxx&&yy < maxx )
			{
				int cell = xx + ( yy << VAL_SHFCX );
				N += FindUnitsInCell( cell, x, y, r, Type, Nation, Need - N, Dest + N );
				if ( N >= Need )return N;
			};
		};
	};
	return N;
};
void MakeReformation( byte NI, word BrigadeID, byte FormType );
void CancelStandGroundAnyway( Brigade* BR );
/*
word GetMyOfficerInf(byte NI,word UnitID,word WarType){
	Nation* NAT=NATIONS+NI;
	int N=NAT->NMon;
	for(int i=0;i<N;i++)if(NAT->Mon[i]->OFCR){
		OfficerRecord* OFCR=NAT->Mon[i]->OFCR;
		for(int j=0;j<OFCR->NStroi;j++){
			OFCR->SDES[j].ID==UnitID&&OFCR->SDES[j].
		};
	};
};
*/
void CheckBrigadeDUP( Brigade* BR )
{
	//byte PRES[8192];
	//memset(PRES,0,8192);
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == BR->MembSN[i] )
			{
				assert( OB->BrIndex == i );
			};
		};
	};
};
void FillFormation( Brigade* BR )
{
	word ULOCAL[512];
	int N1 = 0;
	int N2 = 0;
	int xx = 0;
	int yy = 0;
	bool SELECTED = 0;
	bool ImSELECTED = 0;
	byte NI = BR->CT->NI;
	int NN = ElementaryOrders[BR->WarType - 1].NUnits + 2;
	for ( int i = 2; i < NN; i++ )
	{
		if ( BR->Memb[i] == 0xFFFF )N1++;
		else
		{
			OneObject* OB = Group[BR->Memb[i]];
			if ( OB && !OB->Sdoxlo )
			{
				xx += OB->RealX;
				yy += OB->RealY;
				N2++;
				if ( OB->Selected&GM( NI ) )SELECTED = 1;
				if ( OB->ImSelected&GM( NI ) )ImSELECTED = 1;
			}
			else N1++;
		};
	};
	if ( !( N2&&N1 ) )return;
	xx /= N2;
	yy /= N2;
	/*
	if(BR->Memb[0]==0xFFFF){
		int N=FindUnits(xx,yy,MobilR*16,BR->MembID,BR->CT->NI,N1,ULOCAL);
	};
	if(BR->Memb[1]==0xFFFF){

	};
	*/
	int N = FindUnits( xx, yy, MobilR * 16, BR->MembID, BR->CT->NI, N1, ULOCAL );
	N2 = 0;
	for ( int i = 2; i < NN&&N; i++ )
	{
		bool INS = 0;
		if ( BR->Memb[i] == 0xFFFF )INS = 1;
		else
		{
			OneObject* OB = Group[BR->Memb[i]];
			INS = !( OB && !OB->Sdoxlo );
		};
		if ( INS )
		{
			OneObject* OB = Group[ULOCAL[N2]];
			BR->Memb[i] = ULOCAL[N2];
			BR->MembSN[i] = OB->Serial;
			OB->BrigadeID = BR->ID;
			OB->BrIndex = i;
			OB->InArmy = true;
			OB->AddDamage = BR->AddDamage;
			OB->AddShield = BR->AddShield;
			OB->StandGround = 0;
			( &BR->BM.Peons )[GetBMIndex( OB )]++;
			N2++;
			N--;
		};
	};
	assert( BR->NMemb == NN );
	CancelStandGroundAnyway( BR );
	assert( BR->NMemb == NN );
	BR->CreateOrderedPositions( xx, yy, BR->Direction );
	assert( BR->NMemb == NN );
	BR->KeepPositions( 0, 128 + 16 );
	assert( BR->NMemb == NN );
	int n0 = BR->NMemb;
	if ( SELECTED )
	{
	};
	if ( ImSELECTED )
	{
	};
	BR->NMemb = NN;
	//CorrectBrigadesSelection(BR->CT->NI);
	//ImCorrectBrigadesSelection(BR->CT->NI);
};
bool Brigade::CreateNearOfficer( OneObject* OB, word Type, int ODIndex )
{
	if ( !OB->Ref.General->OFCR )return false;
	bool Dem = false;
	word BarbID = OB->Ref.General->OFCR->BarabanID;
	if ( BarbID == Type )Dem = true;
	int RX = OB->RealX;
	int RY = OB->RealY;
	OrderDescription* ODS = ElementaryOrders + ODIndex;
	word ULOCAL[512];
	int N = FindUnits( RX, RY, MobilR * 16, Type, OB->NNUM, ODS->NUnits, ULOCAL );
	if ( N != ODS->NUnits )return false;
	AddObject( OB );
	if ( Dem )
	{
		OB->BrigadeID = 0xFFFF;
		OB->BrIndex = 0xFFFF;
		OB->InArmy = false;
		Memb[0] = 0xFFFF;
		MembSN[0] = 0xFFFF;
	}
	else OB->StandGround = true;
	if ( !Dem )OB->InArmy = true;
	word BMID;
	N = FindUnits( RX, RY, MobilR * 16, OB->Ref.General->OFCR->BarabanID, OB->NNUM, 1, &BMID );
	if ( N == 1 )
	{
		OB = Group[BMID];
		AddObject( OB );
		if ( Dem )
		{
			OB->BrigadeID = 0xFFFF;
			OB->BrIndex = 0xFFFF;
			OB->InArmy = false;
			Memb[1] = 0xFFFF;
			MembSN[1] = 0xFFFF;
			memset( &BM, 0, sizeof BM );
		}
		else OB->StandGround = true;
		if ( !Dem )OB->InArmy = true;
		//OB->AddDamage=ODS->AddDamage1;
		//OB->AddShield=ODS->AddShield1;
		AddShield = ODS->AddShield2;
		AddDamage = ODS->AddDamage2;
		N = FindUnits( RX, RY, MobilR * 16, Type, OB->NNUM, ODS->NUnits, ULOCAL );
		if ( !N )return false;
		for ( int i = 0; i < N; i++ )
		{
			OneObject* OB2 = Group[ULOCAL[i]];
			AddObject( OB2 );
			OB2->InArmy = true;
			OB2->AddDamage = ODS->AddDamage2;
			OB2->AddShield = ODS->AddShield2;
			OB2->StandGround = 0;
		};
		WarType = ODIndex + 1;
		PosCreated = false;
		CreateOrderedPositions( RX, RY, 0 );
		KeepPositions( 0, 128 + 16 );
		MembID = Type;
		byte usag = NATIONS[OB->NNUM].Mon[Type]->newMons->Usage;
		if ( usag == StrelokID
			|| usag == HorseStrelokID
			|| usag == GrenaderID //BUGFIX: Non-shooting grenadiers in formations
			|| OB->newMons->Artilery )
		{
			Strelki = true;
		}
		else
		{
			Strelki = false;
		}
		//HumanLocalSendTo(OB->RealX,OB->RealY,128+16,0);
	}
	else
	{
		FreeMember( 0 );
		return 0;
	};
	return 1;
}

int FindCommandPlace( int* x, int* y, byte dir, OneObject* OB, int Indx, OrderDescription* ODS )
{
	int xx = *x;
	int yy = *y;
	int SIN = TCos[dir];
	int COS = TSin[dir];
	int N = ODS->NCom;
	int MinR = 100000000;
	int idx = -1;
	int rx = OB->RealX;
	int ry = OB->RealY;
	int bx = 0, by = 0;

	for ( int i = 0; i < N; i++ )
	{
		if ( i != Indx )
		{
			int xc = ODS->ComX[i];
			int yc = ODS->ComY[i];
			int xci = xx + OScale( -xc*COS + yc*SIN );
			int yci = yy + OScale( yc*COS + xc*SIN );
			int R = Norma( rx - xci, ry - yci );

			if ( R < MinR )
			{
				MinR = R;
				idx = i;
				bx = xci;
				by = yci;
			}
		}
	}

	if ( idx != -1 )
	{
		*x = bx;
		*y = by;
	}

	return idx;
}

void OptimiseBrigadePosition( Brigade* BR );
void Brigade::CreateOrderedPositions( int x, int y, char dir )
{
	if ( NMemb > 2 )
	{
		int NM = ElementaryOrders[WarType - 1].NUnits;
		int N = PORD.CreateOrdPos( x, y, dir, NM, Memb + 2, ElementaryOrders + WarType - 1 );
		//NMemb=N+2;
		if ( !PosCreated )
		{
			posX = new int[MaxMemb];
			posY = new int[MaxMemb];
			PosCreated = true;
		};
		for ( int i = 0; i < N; i++ )
		{
			posX[i + 2] = PORD.px[i] >> 4;
			posY[i + 2] = PORD.py[i] >> 4;
			word MID = PORD.Ids[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )
				{
					OB->BrIndex = i + 2;
					Memb[i + 2] = MID;
					MembSN[i + 2] = OB->Serial;
				}
				else
				{
					Memb[i + 2] = 0xFFFF;
					MembSN[i + 2] = 0xFFFF;
				};
			}
			else
			{
				Memb[i + 2] = 0xFFFF;
				MembSN[i + 2] = 0xFFFF;
			};
		};
		for ( int i = N; i < NM; i++ )
		{
			Memb[i + 2] = 0xFFFF;
			MembSN[i + 2] = 0xFFFF;
		};
		NMemb = NM + 2;
		int xx = x;
		int yy = y;
		word MID0 = Memb[0];
		int id1 = -1;
		if ( MID0 != 0xFFFF )
		{
			OneObject* OB = Group[MID0];
			if ( OB && !OB->Sdoxlo )
			{
				id1 = FindCommandPlace( &xx, &yy, dir, OB, -1, ElementaryOrders + WarType - 1 );
			};
			posX[0] = xx >> 4;
			posY[0] = yy >> 4;
		};
		xx = x;
		yy = y;
		MID0 = Memb[1];
		if ( MID0 != 0xFFFF )
		{
			OneObject* OB = Group[MID0];
			if ( OB && !OB->Sdoxlo )
			{
				id1 = FindCommandPlace( &xx, &yy, dir, OB, MID0, ElementaryOrders + WarType - 1 );
			};
			posX[1] = xx >> 4;
			posY[1] = yy >> 4;
		};
	};
	if ( posX )OptimiseBrigadePosition( this );
};
void Brigade::CreateSimpleOrderedPositions( int x, int y, char dir )
{
	if ( NMemb > 2 )
	{
		int N = PORD.CreateSimpleOrdPos( x, y, dir, NMemb - 2, Memb + 2, ElementaryOrders + WarType - 1 );
		//NMemb=N+2;
		memset( Memb + 2, 0xFF, NMemb * 2 - 4 );
		memset( MembSN + 2, 0xFF, NMemb * 2 - 4 );
		if ( !PosCreated )
		{
			posX = new int[MaxMemb];
			posY = new int[MaxMemb];
			PosCreated = true;
		};
		for ( int i = 0; i < N; i++ )
		{
			posX[i + 2] = PORD.px[i] >> 4;
			posY[i + 2] = PORD.py[i] >> 4;
			word MID = PORD.Ids[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )
				{
					OB->BrIndex = i + 2;
					Memb[i + 2] = MID;
					MembSN[i + 2] = OB->Serial;
				}
				else
				{
					Memb[i + 2] = 0xFFFF;
					MembSN[i + 2] = 0xFFFF;
				};
			}
			else
			{
				Memb[i + 2] = 0xFFFF;
				MembSN[i + 2] = 0xFFFF;
			};
		};
		int xx = x;
		int yy = y;
		word MID0 = Memb[0];
		int id1 = -1;
		if ( MID0 != 0xFFFF )
		{
			OneObject* OB = Group[MID0];
			if ( OB && !OB->Sdoxlo )
			{
				id1 = FindCommandPlace( &xx, &yy, dir, OB, -1, ElementaryOrders + WarType - 1 );
			};
			posX[0] = xx >> 4;
			posY[0] = yy >> 4;
		};
		xx = x;
		yy = y;
		MID0 = Memb[1];
		if ( MID0 != 0xFFFF )
		{
			OneObject* OB = Group[MID0];
			if ( OB && !OB->Sdoxlo )
			{
				id1 = FindCommandPlace( &xx, &yy, dir, OB, id1, ElementaryOrders + WarType - 1 );
			};
			posX[1] = xx >> 4;
			posY[1] = yy >> 4;
		};
	};
	OptimiseBrigadePosition( this );
};
void Brigade::HumanCheckUnits()
{
	for ( int i = 0; i < NMemb; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( !( OB&&OB->Serial == MembSN[i] && !OB->Sdoxlo ) )
			{
				Memb[i] = 0xFFFF;
				MembSN[i] = 0xFFFF;
			};
		};
	};
};
word GetDir( int, int );
class B_HSend :public BrigadeOrder
{
public:
	int x, y;
	short Dir;
	byte Prio, OrdType;
};
short FilterDir( short Dir )
{
	return ( ( ( Dir ) >> 3 ) << 3 );
};
void ApplySwap( Brigade* BR, word* swp )
{
	word tm[512];
	word tmsn[512];
	word tm1[512];
	word tmsn1[512];
	if ( BR->WarType )
	{
		int N = BR->NMemb;
		if ( N < 3 )return;
		OrderDescription* ODE = ElementaryOrders + BR->WarType - 1;
		if ( N - 2 < ODE->NUnits )
		{
			int s = ( ODE->NUnits + 2 - N ) << 1;
			memset( BR->Memb + N, 0xFFFF, s );
			memset( BR->MembSN + N, 0xFFFF, s );
			BR->NMemb = ODE->NUnits + 2;
			N = BR->NMemb;
		};
		for ( int i = 2; i < N; i++ )
		{
			word id = swp[i - 2] + 2;
			tm[i] = BR->Memb[id];
			tmsn[i] = BR->MembSN[id];
			tm1[i] = BR->Memb[i];
			tmsn1[i] = BR->MembSN[i];

		};
		for ( int i = 2; i < N; i++ )
		{
			word MID = tm[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB )
				{
					BR->Memb[i] = tm[i];
					BR->MembSN[i] = tmsn[i];
					OB->BrIndex = i;
				}
				else
				{
					BR->Memb[i] = 0xFFFF;
					BR->MembSN[i] = 0xFFFF;
				};
			}
			else
			{
				BR->Memb[i] = 0xFFFF;
				BR->MembSN[i] = 0xFFFF;
			};
		};
	};
};
void HumanLocalSendToLink( Brigade* BR )
{
	if ( BR->NMemb < 4 )
	{
		BR->DeleteBOrder();
		return;
	};
	B_HSend* BS = (B_HSend*) BR->BOrder;
	int x = BS->x;
	int y = BS->y;
	int xc = 0;
	int yc = 0;
	int N = 0;
	for ( int i = 2; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				xc += OB->RealX;
				yc += OB->RealY;
				N++;
			};
		};
	};
	if ( N )
	{
		xc /= N;
		yc /= N;
	}
	else return;
	if ( Norma( x - xc, y - yc ) < 256 )
	{
		BR->DeleteBOrder();
		return;
	};
	byte Dir;
	if ( BS->Dir > 256 )Dir = GetDir( ( x - xc ) >> 4, ( y - yc ) >> 4 );
	else Dir = BS->Dir;
	//Dir=FilterDir(Dir);
	char dd = char( Dir ) - char( BR->Direction );
	byte pri = BS->Prio;
	if ( abs( dd ) < 4 )
	{
		BR->CreateSimpleOrderedPositions( x, y, Dir );
		BR->Direction = Dir;
		BR->DeleteBOrder();
	}
	else
	{
		if ( abs( dd ) < 32 )
		{
			BR->CreateSimpleOrderedPositions( xc, yc, Dir );
			BR->Direction = Dir;
		}
		else
		{
			OrderDescription* ODE = ElementaryOrders + BR->WarType - 1;
			if ( ODE->Sym4f )
			{
				//maximal symmetry(rotation pi/4)
				if ( dd >= 32 && dd < 96 )ApplySwap( BR, ODE->Sym4f );
				else if ( dd <= -32 && dd > -96 )ApplySwap( BR, ODE->Sym4i );
				else ApplySwap( BR, ODE->SymInv );
				BR->CreateSimpleOrderedPositions( xc, yc, Dir );
				BR->Direction = Dir;
			}
			else
				if ( ODE->SymInv )
				{
					if ( dd > 64 || dd < -64 )ApplySwap( BR, ODE->SymInv );
					BR->CreateSimpleOrderedPositions( xc, yc, Dir );
					BR->Direction = Dir;
				}
				else
				{
					if ( dd > 0 )
					{
						BR->CreateSimpleOrderedPositions( xc, yc, BR->Direction + 32 );
						BR->Direction += 32;
					}
					else
					{
						BR->CreateSimpleOrderedPositions( xc, yc, BR->Direction - 32 );
						BR->Direction -= 32;
					};
				};
		};
	};
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				OB->NewMonsterPreciseSendTo( int( BR->posX[i] ) << 4, int( BR->posY[i] ) << 4, pri, 0 );
			};
		};
	};
	BR->KeepPositions( 1, pri );
};
void HumanEscapeLink( Brigade* BR )
{
	if ( BR->NMemb < 4 )
	{
		BR->DeleteBOrder();
		return;
	};
	B_HSend* BS = (B_HSend*) BR->BOrder;
	int x = BS->x;
	int y = BS->y;
	int xc = 0;
	int yc = 0;
	int N = 0;
	for ( int i = 2; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				xc += OB->RealX;
				yc += OB->RealY;
				N++;
			};
		};
	};
	if ( N )
	{
		xc /= N;
		yc /= N;
	}
	else return;
	if ( Norma( x - xc, y - yc ) < 256 )
	{
		BR->DeleteBOrder();
		return;
	};
	byte Dir;
	if ( BS->Dir > 256 )Dir = GetDir( ( x - xc ) >> 4, ( y - yc ) >> 4 );
	else Dir = BS->Dir;
	//Dir=FilterDir(Dir);
	char dd = char( Dir ) - char( BR->Direction );
	byte pri = BS->Prio;
	if ( abs( dd ) < 32 )
	{
		BR->CreateSimpleOrderedPositions( x, y, Dir );
		BR->Direction = Dir;
	}
	else
	{
		OrderDescription* ODE = ElementaryOrders + BR->WarType - 1;
		if ( ODE->Sym4f )
		{
			//maximal symmetry(rotation pi/4)
			if ( dd >= 32 && dd < 96 )ApplySwap( BR, ODE->Sym4f );
			else if ( dd <= -32 && dd > -96 )ApplySwap( BR, ODE->Sym4i );
			else ApplySwap( BR, ODE->SymInv );
			BR->CreateSimpleOrderedPositions( x, y, Dir );
			BR->Direction = Dir;
		}
		else
			if ( ODE->SymInv )
			{
				if ( dd > 64 || dd < -64 )ApplySwap( BR, ODE->SymInv );
				BR->CreateSimpleOrderedPositions( x, y, Dir );
				BR->Direction = Dir;
			}
			else
			{
				if ( dd > 0 )
				{
					BR->CreateSimpleOrderedPositions( x, y, BR->Direction + 32 );
					BR->Direction += 32;
				}
				else
				{
					BR->CreateSimpleOrderedPositions( x, y, BR->Direction - 32 );
					BR->Direction -= 32;
				};
			};
	};
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				OB->NewMonsterPreciseSendTo( int( BR->posX[i] ) << 4, int( BR->posY[i] ) << 4, pri, 0 );
			};
		};
	};
	BR->DeleteBOrder();
	BR->KeepPositions( 1, pri );
};
char* HLST_Message = "[HumanLocalSendTo]";
void Brigade::HumanLocalSendTo( int x, int y, short Dir, byte Prio, byte OrdType )
{
	if ( NMemb < 4 )return;
	B_HSend* BS = (B_HSend*) CreateOrder( OrdType, sizeof B_HSend );
	BS->x = x;
	BS->y = y;
	BS->Prio = Prio;
	BS->Dir = Dir;
	BS->Size = sizeof B_HSend;
	BS->OrdType = OrdType;
	BS->BLink = &HumanLocalSendToLink;
	BS->Message = HLST_Message;
	int N = NMemb;
	bool Attack = false;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				if ( OB->EnemyID != 0xFFFF )Attack = true;
			};
		};
	};
	//if(Attack)
	BS->BLink = &HumanEscapeLink;
};
extern MotionField UnitsField;
void RotUnitLink( OneObject* OB )
{
	int Mrot = OB->newMons->MinRotator;
	char Dir = OB->LocalOrder->info.MoveFrom.dir;
	char dd = char( OB->RealDir ) - Dir;
	if ( abs( dd ) <= Mrot )
	{
		OB->RealDir = Dir;
		OB->GraphDir = OB->RealDir;
		OB->DeleteLastOrder();
		return;
	};
	if ( dd > 0 )OB->RealDir -= Mrot;
	else OB->RealDir += Mrot;
	OB->GraphDir = OB->RealDir;
};
void RotUnit( OneObject* OB, char Dir, byte OrdType )
{
	if ( OB->Transport )return;
	Order1* Or1 = OB->CreateOrder( OrdType );
	if ( Or1 )
	{
		Or1->PrioryLevel = 0;
		Or1->info.MoveFrom.dir = Dir;
		Or1->DoLink = &RotUnitLink;
	};
};
bool RotateUnit( OneObject* OB, char Dir )
{
	int Mrot = OB->newMons->MinRotator;
	char dd = char( OB->RealDir ) - Dir;
	if ( abs( dd ) <= Mrot )return false;
	RotUnit( OB, Dir, 0 );
	return true;
};
void NewMonsterPreciseSendToLink( OneObject* OB );
void B_KeepPositionsLink( Brigade* BR )
{
	byte pri = BR->BOrder->Prio;
	if ( BR->PosCreated )
	{
		if ( BR->Precise )
		{
			bool Done = true;
			word* Memb = BR->Memb;
			int N = BR->NMemb;
			char DIRC = BR->Direction;
			for ( int i = 0; i < N; i++ )
			{
				word MID = Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && !OB->Sdoxlo )
					{
						if ( ( !OB->LocalOrder ) || OB->EnemyID != 0xFFFF )
						{
							int xi = int( BR->posX[i] ) << 4;
							int yi = int( BR->posY[i] ) << 4;
							int dd = Norma( xi - OB->RealX, yi - OB->RealY );
							int Lx = OB->Lx;
							if ( dd > 100 )
							{
								bool PDN = Done;
								Done = false;
								if ( dd < 768 )
								{
									int xx1 = ( xi - ( Lx << 7 ) ) >> 8;
									int yy1 = ( yi - ( Lx << 7 ) ) >> 8;
									if ( !UnitsField.CheckBar( xx1, yy1, Lx, Lx ) )
									{
										OB->NewMonsterPreciseSendTo( xi, yi, pri, 0 );
									}
									else
									{
										if ( RotateUnit( OB, DIRC ) )Done = false;
										else Done = PDN;
									};
								}
								else OB->NewMonsterPreciseSendTo( xi, yi, pri, 0 );
							}
							else
							{
								if ( RotateUnit( OB, DIRC ) )Done = false;
							};
						}
						else Done = false;

					};
				};
			};
			if ( Done )
			{
				BR->DeleteBOrder();
			};
		}
		else
		{
			bool Done = true;
			word* Memb = BR->Memb;
			int N = BR->NMemb;
			for ( int i = 0; i < N; i++ )
			{
				word MID = Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && !OB->Sdoxlo )
					{
						if ( OB->LocalOrder )Done = false;
					};
				};
			};
			if ( Done )
			{
				BR->DeleteBOrder();
			};
		};
	};
};
void B_LeaveAttackLink( Brigade* BR )
{
	if ( BR->PosCreated )
	{
		bool Done = true;
		word* Memb = BR->Memb;
		int N = BR->NMemb;
		for ( int i = 0; i < N; i++ )
		{
			word MID = Memb[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && !OB->Sdoxlo )
				{
					if ( OB->LocalOrder )
					{
						if ( OB->EnemyID == 0xFFFF )Done = false;
						else OB->NewMonsterSendTo( int( BR->posX[i] ) << 4, int( BR->posY[i] ) << 4, 16, 0 );
					};
				};
			};
		};
		if ( Done )
		{
			for ( int i = 0; i < N; i++ )
			{
				word MID = Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && !OB->Sdoxlo )
					{
						if ( OB->LocalOrder )
						{
							if ( OB->EnemyID != 0xFFFF )
							{
								BR->CT->UnRegisterNewUnit( OB );
							};
						};
					};
				};
			};
			BR->DeleteBOrder();
		};
	};
};
char* KP_Message = "[KeepPosition]";
void Brigade::KeepPositions( byte OrdType, byte Prio )
{
	BrigadeOrder* OR = CreateOrder( OrdType, sizeof BrigadeOrder );
	OR->Message = KP_Message;
	OR->Prio = Prio;
	OR->Size = sizeof BrigadeOrder;
	OR->BLink = &B_KeepPositionsLink;
	int N = NMemb;
	char DIRC = Direction;
	bool Attack = false;
	for ( int i = 0; i < N; i++ )
	{
		word MID = Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				if ( OB->EnemyID != 0xFFFF )Attack = true;
				int xi = int( posX[i] ) << 4;
				int yi = int( posY[i] ) << 4;
				OB->NewMonsterPreciseSendTo( xi, yi, Prio, 0 );
			};
		};
	};
	//if(Attack)OR->BLink=&B_LeaveAttackLink;
};

void DrawPlaneLine( int x0, int y0, int x1, int y1, byte c )
{
	int dx = mapx << 5;
	int dy = mapy << 4;
	int h0 = GetHeight( x0, y0 );
	int h1 = GetHeight( x1, y1 );
	x0 -= dx;
	y0 = ( y0 >> 1 ) - h0 - dy;
	x1 -= dx;
	y1 = ( y1 >> 1 ) - h1 - dy;
	DrawLine( x0, y0, x1, y1, c );
};
void DrawMultiLine( int x0, int y0, int x1, int y1, byte c )
{
	int dx = ( x1 - x0 ) << 16;
	int dy = ( y1 - y0 ) << 16;
	int r = Norma( dx, dy );
	if ( !r )return;
	int xx = 0;
	int yy = 0;
	int N = ( r >> ( 16 + 5 ) ) + 1;
	dx /= N;
	dy /= N;
	for ( int i = 0; i < N; i++ )
	{
		DrawPlaneLine( x0 + ( xx >> 16 ), y0 + ( yy >> 16 ), x0 + ( ( xx + dx ) >> 16 ), y0 + ( ( yy + dy ) >> 16 ), c );
		xx += dx;
		yy += dy;
	};
};
void DrawRotLine( int x0, int y0, int dx0, int dy0, int dx1, int dy1, int SIN, int COS, byte c )
{
	DrawMultiLine( x0 + ( ( dx0*COS - dy0*SIN ) >> 8 ), y0 + ( ( dy0*COS + dx0*SIN ) >> 8 ),
		x0 + ( ( dx1*COS - dy1*SIN ) >> 8 ), y0 + ( ( dy1*COS + dx1*SIN ) >> 8 ), c );
};
void B_BitvaLink( Brigade* BR );

void DrawBorder( Brigade* BR )
{
	if ( BR->WarType&&BR->NMemb > 2 && BR->PosCreated )
	{
		OrderDescription* ODS = ElementaryOrders + BR->WarType - 1;
		int SIN = -TSin[byte( BR->Direction )];
		int COS = TCos[byte( BR->Direction )];
		int xt = 0;
		int yt = 0;
		int Nt = 0;
		int MinX = 10000000;
		int MaxX = -10000000;
		int MinY = 10000000;
		int MaxY = -10000000;
		int N = BR->NMemb;
		bool Cross = false;
		byte cl = clrYello;
		if ( BR->BOrder )
		{
			if ( BR->BOrder->BLink == &B_KeepPositionsLink )
			{
				Cross = true;
			}
			if ( BR->BOrder->BLink == &B_BitvaLink )
			{
				cl = 0xC9;
			}
		}
		for ( int i = 2; i < N; i++ )
		{
			word MID = BR->Memb[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && !OB->Sdoxlo )
				{
					xt += OB->RealX >> 4;
					yt += OB->RealY >> 4;
					Nt++;
					int xx, yy;
					if ( Cross )
					{
						xx = BR->posX[i];
						yy = BR->posY[i];
						DrawPlaneLine( xx, yy - 2, xx, yy + 2, cl );
						DrawPlaneLine( xx - 2, yy, xx + 2, yy, cl );
					}
					xx = OB->RealX >> 4;
					yy = OB->RealY >> 4;
					DrawPlaneLine( xx - 12, yy, xx, yy - 12, cl );
					DrawPlaneLine( xx - 12, yy, xx, yy + 12, cl );
					DrawPlaneLine( xx + 12, yy, xx, yy - 12, cl );
					DrawPlaneLine( xx + 12, yy, xx, yy + 12, cl );
				}
			}
		}
	}
}

int OScale( int x )
{
	return ( x * 256 ) >> 8;
};
bool FindCorrectTopPos( int x0, int y0, int* x, int* y )
{
	int top0 = GetTopology( &x0, &y0 );
	if ( top0 == 0xFFFF )return false;
	x0 <<= 8;
	y0 <<= 8;
	int dx = ( ( *x ) << 8 ) - x0;
	int dy = ( ( *y ) << 8 ) - y0;
	int Nr = Norma( dx, dy );
	int N = ( Nr >> ( 8 + 5 ) ) + 1;
	dx /= N;
	dy /= N;
	for ( int i = 0; i <= N; i++ )
	{
		if ( CheckBar( ( x0 >> 12 ) - 2, ( y0 >> 12 ) - 2, 4, 4 ) )
		{
			*x = x0 >> 8;
			*y = y0 >> 8;
			return true;
		}
		else
		{
			if ( i < N )
			{
				x0 += dx;
				y0 += dy;
			}
			else
			{
				*x = x0 >> 8;
				*y = y0 >> 8;
				return true;
			};
		};
	};
	return false;
};
bool FindCorrectTopPos1( int x0, int y0, int* x, int* y )
{
	int top0 = GetTopology( &x0, &y0 );
	if ( top0 == 0xFFFF )return false;
	x0 >>= 4;
	y0 >>= 4;
	for ( int r = 0; r < 20; r++ )
	{
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		for ( int j = 0; j < N; j++ )
		{
			int x1 = x0 + xi[j];
			int y1 = y0 + yi[j];
			int tpx1 = x1 >> 2;
			int tpy1 = y1 >> 2;
			if ( tpx1 > 0 && tpy1 > 0 && tpx1 < TopLx&&tpy1 < TopLx )
			{
				if ( !CheckBar( x1 - 1, y1 - 1, 3, 3 ) )
				{
					word top = TopRef[tpx1 + ( tpy1 << TopSH )];
					if ( top == top0 )
					{
						*x = ( x1 << 4 );
						*y = ( y1 << 4 );
						return true;
					};
				};
			};
		};
	};
	return false;
};
int FindSuperSmartBestPosition( OneObject* OB, int* cx, int* cy, int dx, int dy, word Top, byte LTP );
void OptimiseBrigadePosition( Brigade* BR )
{
	int MinX = 10000000;
	int MinY = 10000000;
	int MaxX = -10000000;
	int MaxY = -10000000;
	int N = BR->NMemb;
	word* Mem = BR->Memb;
	int* px = BR->posX;
	int* py = BR->posY;
	for ( int i = 0; i < N; i++ )
	{
		if ( Mem[i] != 0xFFFF )
		{
			int x = px[i];
			int y = py[i];
			if ( x > MaxX )MaxX = x;
			if ( x < MinX )MinX = x;
			if ( y > MaxY )MaxY = y;
			if ( y < MinY )MinY = y;
		};
	};
	int GMin = 64;
	int GMax = ( msx - 2 ) << 5;
	int Dx0 = 0;
	int Dy0 = 0;
	if ( MinX < GMin )Dx0 = -MinX + GMin;
	if ( MinY < GMin )Dy0 = -MinY + GMin;
	if ( MaxX > GMax )Dx0 = GMax - MaxX;
	if ( MaxY > GMax )Dy0 = GMax - MaxY;
	int x0 = ( ( MinX + MaxX ) >> 1 );
	int y0 = ( ( MinY + MaxY ) >> 1 );
	int Top0 = GetTopology( x0, y0 );
	//search for best place
	int MinFailed = 10000;
	int BestDx = 0;
	int BestDy = 0;
	bool UnFoundBP = true;
	for ( int r = 0; r < 4; r++ )
	{
		int Np = Rarr[r].N;
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		for ( int j = 0; j < Np; j++ )
		{
			int DX = ( int( xi[j] ) << 5 );
			int DY = ( int( yi[j] ) << 5 );
			int dx = Dx0 + DX;
			int dy = Dy0 + DY;
			int xt = ( x0 + DX ) >> 6;
			int yt = ( y0 + DY ) >> 6;
			int Top1 = 0xFFFF;
			if ( xt > 0 && xt < TopLx&&yt>0 && yt < TopLx )
			{
				Top1 = TopRef[xt + ( yt << TopSH )];
			};
			if ( Top1 == Top0 )
			{
				int nf = 0;
				for ( int k = 0; k < N; k++ )
				{
					if ( Mem[k] != 0xFFFF )
					{
						if ( CheckBar( ( ( px[k] + dx ) >> 4 ) - 1, ( ( py[k] + dy ) >> 4 ) - 1, 3, 3 ) )nf++;
					};
				};
				if ( !nf )
				{
					BestDx = dx;
					BestDy = dy;
					MinFailed = 0;
					goto ChangePos;
				};
				if ( nf < MinFailed )
				{
					BestDx = dx;
					BestDy = dy;
					MinFailed = nf;
				};
			};
		};
	};
ChangePos:
	if ( BestDx || BestDy )
	{
		for ( int i = 0; i < N; i++ )
		{
			if ( Mem[i] != 0xFFFF )
			{
				px[i] += BestDx;
				py[i] += BestDy;
			};
		};
	};
	if ( MinFailed )
	{
		//need to shift some positions
		int x0 = ( ( MinX + MaxX ) >> 1 ) + BestDx;
		int y0 = ( ( MinY + MaxY ) >> 1 ) + BestDy;
		for ( int i = 0; i < N; i++ )
		{
			if ( Mem[i] != 0xFFFF )
			{
				int x = px[i];
				int y = py[i];
				int xx = x0;
				int yy = y0;
				int v = FindSuperSmartBestPosition( NULL, &xx, &yy, x - xx, y - yy, 0, 0 );
				if ( v )
				{
					px[i] = xx;
					py[i] = yy;
					//assert(!CheckBar((xx>>4)-1,(yy>>4)-1,3,3));
				}
				else
				{
					FindCorrectTopPos1( x0, y0, &x, &y );
					px[i] = x;
					py[i] = y;
					//assert(!CheckBar((x>>4)-1,(y>>4)-1,3,3));
				};
			};
		};
		BR->Precise = false;
	}
	else
	{
		BR->Precise = true;
	};
	//BR->Precise=false;
};
class B_SmartSend :public BrigadeOrder
{
public:
	int x, y, NextX, NextY;
	short Dir;
	word NextTop;
};
bool CheckBDirectWay( int x0, int y0, int x1, int y1 )
{
	int bx = x0 << 10;
	int by = y0 << 10;
	int Lx = x1 - x0;
	int Ly = y1 - y0;
	int r = Norma( Lx, Ly );
	int N = ( r >> 6 ) + 1;
	Lx = ( Lx << 10 ) / N;
	Ly = ( Ly << 10 ) / N;
	for ( int i = 0; i < N; i++ )
	{
		bx += Lx;
		by += Ly;
		int xx = bx >> 14;
		int yy = by >> 14;
		if ( CheckPt( xx, yy ) )return false;
		if ( CheckPt( xx - 8, yy ) )return false;
		if ( CheckPt( xx + 8, yy ) )return false;
		if ( CheckPt( xx, yy - 8 ) )return false;
		if ( CheckPt( xx, yy + 8 ) )return false;
	};
	return true;
};
bool CalcCenter( Brigade* BR, int* x, int* y )
{
	if ( BR->NMemb < 3 )return false;
	int xx = 0;
	int yy = 0;
	int N = 0;
	int M = BR->NMemb;
	for ( int i = 2; i < M; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == BR->MembSN[i] )
			{
				xx += OB->RealX;
				yy += OB->RealY;
				N++;
			};
		};
	};
	if ( N )
	{
		*x = xx / N;
		*y = yy / N;
		return true;
	}
	else return false;
};
void B_HumanGlobalSendToLink( Brigade* BR )
{
	B_SmartSend* BS = (B_SmartSend*) BR->BOrder;
	int x = BS->x;
	int y = BS->y;
	int NextX = BS->NextX;
	int NextY = BS->NextY;
	int NextTop = BS->NextTop;
	byte prior = BS->Prio;
	short dir = BS->Dir;
	int xc, yc;
	if ( !CalcCenter( BR, &xc, &yc ) )
	{
		BR->DeleteBOrder();
		return;
	};
	if ( CheckBDirectWay( xc >> 4, yc >> 4, x, y ) )
	{
		BR->DeleteBOrder();
		BR->HumanLocalSendTo( x << 4, y << 4, dir, prior, 1 );
		return;
	};
	if ( NextTop == 0xFFFF )
	{
		NextTop = GetTopology( xc >> 4, yc >> 4 );
		if ( NextTop == 0xFFFF )
		{
			BR->DeleteBOrder();
			return;
		};
	};
	int FinalTop = GetTopology( &x, &y );
	if ( FinalTop == 0xFFFF )
	{
		BR->DeleteBOrder();
		return;
	};
	BS->x = x;
	BS->y = y;
	word NextNextTop = MotionLinks[FinalTop + NAreas*NextTop];
	if ( NextNextTop == FinalTop || FinalTop == NextTop )
	{
		BR->DeleteBOrder();
		BR->HumanLocalSendTo( x << 4, y << 4, dir, prior, 1 );
		return;
	};
	if ( NextNextTop != 0xFFFF )
	{
		//atttempt to optomise way
		int MaxPre = 3;
		int cox = xc >> 4;
		int coy = yc >> 4;
		do
		{
			int Next2 = MotionLinks[FinalTop + NAreas*NextNextTop];
			if ( Next2 != 0xFFFF )
			{
				Area* AR2 = TopMap + Next2;
				int nx = ( int( AR2->x ) << 6 ) + 32;
				int ny = ( int( AR2->y ) << 6 ) + 32;
				int rx = nx;
				int ry = ny;
				if ( CheckBDirectWay( cox, coy, rx, ry ) )
				{
					NextNextTop = Next2;
					MaxPre--;
				}
				else MaxPre = 0;
			}
			else MaxPre = 0;
		} while ( MaxPre );
		Area* NextAr = TopMap + NextNextTop;
		//------------------------
		int nx = ( int( NextAr->x ) << 6 ) + 32;
		int ny = ( int( NextAr->y ) << 6 ) + 32;
		BS->NextX = nx;
		BS->NextY = ny;
		BS->NextTop = NextNextTop;
		BR->HumanLocalSendTo( nx << 4, ny << 4, 512, prior, 1 );
		return;
	}
	else
	{
		BR->DeleteBOrder();
		return;
	};
};
char* HGST_Message = "[HumanGlobalSendTo]";
void Brigade::HumanGlobalSendTo( int x, int y, short Dir, byte Prio, byte OrdType )
{
	if ( LastOrderTime == tmtmt )return;
	AttEnm = false;
	LastOrderTime = tmtmt;
	word Top = GetTopology( &x, &y );
	if ( Top == 0xFFFF )return;
	B_SmartSend* OR1 = (B_SmartSend*) CreateOrder( OrdType, sizeof B_SmartSend );
	OR1->BLink = &B_HumanGlobalSendToLink;
	OR1->Prio = Prio;
	OR1->x = x;
	OR1->y = y;
	OR1->Dir = Dir;
	OR1->Size = sizeof B_SmartSend;
	OR1->NextX = 0xFFFF;
	OR1->NextY = 0xFFFF;
	OR1->NextTop = 0xFFFF;
	OR1->Message = HGST_Message;
};
void SearchArmy( OneObject* OB );
void EraseBrigade( Brigade* BR )
{
	if ( BR->WarType&&BR->ArmyID != 0xFFFF )
	{
		BR->WarType = 0;
		int nr = 0;
		for ( int i = 0; i < BR->NMemb; i++ )
		{
			if ( i < 2 )
			{
				word mid = BR->Memb[i];
				if ( mid != 0xFFFF )
				{
					OneObject* OB = Group[mid];
					if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] )
					{
						OB->BrigadeID = 0xFFFF;
						OB->BrIndex = 0xFFFF;
						OB->InArmy = 0;
						OB->ClearOrders();
						SearchArmy( OB );
					};
				};
			}
			else
			{
				word mid = BR->Memb[i];
				if ( mid != 0xFFFF )
				{
					OneObject* OB = Group[mid];
					if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] )
					{
						BR->Memb[nr] = OB->Index;
						OB->BrIndex = nr;
						OB->InArmy = 0;
						BR->MembSN[nr] = OB->Serial;
						nr++;
					};
				};
			};
		};
		BR->NMemb = nr;
		free( BR->posX );
		BR->posX = 0;
		free( BR->posY );
		BR->posY = 0;
		BR->PosCreated = 0;
		return;
	};
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB )
			{
				OB->BrigadeID = 0xFFFF;
				OB->BrIndex = 0xFFFF;
				OB->InArmy = false;
				OB->StandGround = false;
				OB->AddDamage = 0;
				OB->AddShield = 0;
				OB->GroundState = 0;
			};
		};
	};
	if ( BR->Memb )
	{
		free( BR->Memb );
		free( BR->MembSN );
		BR->Memb = NULL;
		BR->MembSN = NULL;
	};
	City* CT = BR->CT;
	int id = BR->ID;
	BR->DeleteAll();
	memset( BR, 0, sizeof Brigade );
	BR->ID = id;
	BR->CT = CT;
};
word CheckMotionThroughEnemyAbility( OneObject* OB, int px, int py )
{
	px >>= 11;
	py >>= 11;
	byte mmask = OB->NMask;
	byte mask = ~mmask;
	if ( px >= 0 && py >= 0 && px < VAL_MAXCX&&py < VAL_MAXCX )
	{
		int cell = px + ( py << VAL_SHFCX );
		if ( NPresence[cell] & mask )
		{
			//Enemy present!!!
			cell += VAL_MAXCX + 1;
			int NMon = MCount[cell];
			if ( !NMon )return 0xFFFF;
			int ofs1 = cell << SHFCELL;
			word MID;
			for ( int i = 0; i < NMon; i++ )
			{
				MID = GetNMSL( ofs1 + i );
				if ( MID != 0xFFFF )
				{
					OneObject* OBJ = Group[MID];
					if ( OBJ && ( !( OBJ->NMask&mmask || OBJ->newMons->Artilery ) ) && Norma( OB->RealX - OBJ->RealX, OB->RealY - OBJ->RealY ) < 512 )
					{
						OB->AttackObj( MID, 15 + 128, 0 );
						return MID;
					};
				};
			};
		};
		return 0xFFFF;
	}
	else return 0xFFFF;
};

//------------------------------Bitva------------------------
char* BBIT_Message = "[Brigade::Bitva]";
//#define maxen
class Brigade_Bitva :public BrigadeOrder
{
public:
	word StartTop;
	byte BitMask[1024];
	word Enm[512];
	word EnSN[512];
	byte NDang[512];
	int  NEn;
	int MinX;
	int MinY;
	int MaxX;
	int MaxY;
	void AddEnemXY( int x, int y, int MyTop, byte mask );
};
word GetTopFast( int x, int y );
void Brigade_Bitva::AddEnemXY( int x, int y, int MyTop, byte mask )
{
	if ( NEn >= 512 )return;
	int x0 = x << 1;
	int y0 = y << 1;

	int HimTop1 = GetTopFast( x0, y0 );
	int HimTop2 = GetTopFast( x0 + 1, y0 );
	int HimTop3 = GetTopFast( x0, y0 + 1 );
	int HimTop4 = GetTopFast( x0 + 1, y0 + 1 );
	if ( HimTop1 >= 0xFFFE && HimTop2 >= 0xFFFE && HimTop3 >= 0xFFFE && HimTop4 >= 0xFFFE )return;
	int NTP = MyTop*NAreas;
	if ( HimTop1 < 0xFFFE )
	{
		if ( HimTop1 != MyTop&&LinksDist[HimTop1 + NTP] > 30 )return;
	};
	if ( HimTop2 < 0xFFFE )
	{
		if ( HimTop2 != MyTop&&LinksDist[HimTop2 + NTP] > 30 )return;
	};
	if ( HimTop3 < 0xFFFE )
	{
		if ( HimTop3 != MyTop&&LinksDist[HimTop3 + NTP] > 30 )return;
	};
	if ( HimTop4 < 0xFFFE )
	{
		if ( HimTop4 != MyTop&&LinksDist[HimTop4 + NTP] > 30 )return;
	};
	int cell = x + 1 + ( ( y + 1 ) << VAL_SHFCX );
	if ( cell > 0 && cell < MAXCIOFS )
	{
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
				if ( OB && !( OB->NMask&mask || OB->Sdoxlo || OB->newMons->LockType ) )
				{
					int id = OB->Index;
					int ms = 1 << ( id & 7 );
					int of = id >> 3;
					if ( !( BitMask[of] & ms ) )
					{
						BitMask[of] |= ms;
						Enm[NEn] = id;
						EnSN[NEn] = OB->Serial;
						int xx = OB->RealX >> 10;
						int yy = OB->RealY >> 10;
						if ( xx > 0 && yy > 0 )
						{
							int off = xx + ( yy << TopSH );
							NDang[NEn] = 0;//((InflMap[off-1]&255)+(InflMap[off+1]&255)+(InflMap[off+TopLx]&255)+(InflMap[off-TopLx]&255))>>2;
						}
						else NDang[NEn] = 0;
						NEn++;
						if ( NEn >= 512 )return;
					};
				};
			};
		};
	};
};
word GetDir( int dx, int dy );
void AttackObjLink( OneObject* OBJ );
void B_BitvaLink( Brigade* BR )
{
	Brigade_Bitva* OR1 = (Brigade_Bitva*) BR->BOrder;
	byte Mask = 1 << BR->CT->NI;
	//1.Check range of battle
	if ( rando() < 1024 || OR1->StartTop == 0xFFFF )
	{
		word Top = 0xFFFF;
		int MinX = 10000000;
		int MinY = 10000000;
		int MaxX = 0;
		int MaxY = 0;
		int N = BR->NMemb;
		word* Mem = BR->Memb;
		word* MSN = BR->MembSN;
		for ( int j = 0; j < N; j++ )
		{
			word MID = Mem[j];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->Serial == MSN[j] )
				{
					if ( Top == 0xFFFF )
					{
						Top = GetTopology( OB->RealX >> 4, OB->RealY >> 4 );
						OR1->StartTop = Top;
					};
					int xx = OB->RealX >> 11;
					int yy = OB->RealY >> 11;
					if ( xx < MinX )MinX = xx;
					if ( yy < MinY )MinY = yy;
					if ( xx > MaxX )MaxX = xx;
					if ( yy > MaxY )MaxY = yy;
				};
			};
		};
		if ( MaxX >= MinX )
		{
			MinX -= 4;
			MaxX += 4;
			MinY -= 4;
			MaxY += 4;
			if ( MinX < 0 )MinX = 0;
			if ( MinY < 0 )MinY = 0;
			OR1->MinX = MinX;
			OR1->MaxX = MaxX;
			OR1->MinY = MinY;
			OR1->MaxY = MaxY;
		}
		else
		{
			BR->DeleteBOrder();
			return;
		};
	};
	//2.renew enemy list
	if ( rando() < 1024 )
	{
		int N = OR1->NEn;
		word* Mem = OR1->Enm;
		word* MSN = OR1->EnSN;
		byte* ATT = OR1->NDang;
		for ( int i = 0; i < N; i++ )
		{
			OneObject* OB = Group[Mem[i]];
			if ( ( !OB ) || ( OB && ( OB->Sdoxlo || OB->Serial != MSN[i] ) ) )
			{
				//delete enemy from list
				if ( i < N - 1 )
				{
					memcpy( Mem + i, Mem + i + 1, ( N - i - 1 ) << 1 );
					memcpy( MSN + i, MSN + i + 1, ( N - i - 1 ) << 1 );
					memcpy( ATT + i, ATT + i + 1, N - i - 1 );
				};
				i--;
				N--;
			};
		};
		OR1->NEn = N;
	};
	int MaxAttServ = 64;
	bool InBattle = false;
	//3.Adding new units to battle
	int MinX = OR1->MinX;
	int MinY = OR1->MinY;
	int Dx = OR1->MaxX - OR1->MinX + 1;
	int Dy = OR1->MaxY - OR1->MinY + 1;
	for ( int p = 0; p < 64; p++ )
	{
		int xx = ( ( int( rando() )*Dx ) >> 15 ) + MinX;
		int yy = ( ( int( rando() )*Dy ) >> 15 ) + MinY;
		OR1->AddEnemXY( xx, yy, OR1->StartTop, Mask );
	};
	//4.Attack service
	bool MorPresent = false;
	int N = BR->NMemb;
	word* Mem = BR->Memb;
	word* MSN = BR->MembSN;
	for ( int j = 0; j < N; j++ )
	{
		word MID = Mem[j];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MSN[j] && !OB->newMons->Artilery )
			{
				if ( ( !( OB->LocalOrder ) ) || ( OB->EnemyID == 0xFFFF ) )
				{
					//need to find enemy
					NewMonster* NM = OB->newMons;
					AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
					byte mms = NM->KillMask;
					int MinR = ADC->MinR_Attack;
					int MaxR = ADC->MaxR_Attack;
					int myx = OB->RealX;
					int myy = OB->RealY;
					if ( MaxR )
					{
						//search best enemy
						int NearDist = 1000000;
						int ReadyDist = 1000000;
						word NearMID = 0xFFFF;
						word ReadyMID = 0xFFFF;
						int NEn = OR1->NEn;
						word* Mem = OR1->Enm;
						word* MSN = OR1->EnSN;
						byte* DANG = OR1->NDang;
						char mdr = OB->RealDir;
						for ( int t = 0; t < NEn; t++ )
						{
							word MID = Mem[t];
							OneObject* EOB = Group[MID];
							int dan = DANG[t];
							NewMonster* ENM = NULL;
							if ( EOB )ENM = EOB->newMons;
							if ( EOB && ( ENM->MathMask&mms ) && ( !EOB->Sdoxlo ) && EOB->Serial == MSN[t] )
							{
								int Eusage = ENM->Usage;
								if ( Eusage == MortiraID )MorPresent = true;
								//nash paren!
								int R = Norma( myx - EOB->RealX, myy - EOB->RealY ) >> 4;
								if ( R > MinR )
								{
									if ( R < NearDist && ( dan < 7 || R < MaxR || MorPresent ) )
									{
										//checking for direction of motion
										if ( EOB->InMotion )
										{
											int dr1 = GetDir( EOB->RealX - myx, EOB->RealY - myy );
											char dr = EOB->RealDir - dr1;
											if ( abs( dr ) > 64 )
											{
												NearMID = MID;
												NearDist = R;
											};
										}
										else
										{
											NearMID = MID;
											NearDist = R;
										};
									};
									if ( R < MaxR&&R < ReadyDist )
									{
										ReadyMID = MID;
										ReadyDist = R;
									};
								};
							};
						};
						if ( ReadyMID == 0xFFFF )ReadyMID = NearMID;
						if ( ReadyMID != 0xFFFF )
						{
							OB->AttackObj( ReadyMID, 128 + 8 );
							InBattle = true;
						}
						else
						{

						};
					};
				}
				else InBattle = true;
			};
		};
	};
	if ( !InBattle )
	{
		BR->DeleteBOrder();
	};
};
void Brigade::Bitva()
{
	if ( BOrder&&BOrder->BLink == &B_BitvaLink )return;
	Brigade_Bitva* OR1 = (Brigade_Bitva*) CreateOrder( 0, sizeof Brigade_Bitva );
	SetStandState( this, 1 );
	OR1->StartTop = 0xFFFF;
	OR1->BLink = &B_BitvaLink;
	OR1->Size = sizeof Brigade_Bitva;
	OR1->Prio = 0;
	OR1->Message = BBIT_Message;
	OR1->NEn = 0;
	memset( OR1->BitMask, 0, sizeof OR1->BitMask );
	int MinX = 10000000;
	int MinY = 10000000;
	int MaxX = 0;
	int MaxY = 0;

	int N = NMemb;
	word* Mem = Memb;
	word* MSN = MembSN;
	word Top = 0xFFFF;
	for ( int j = 0; j < N; j++ )
	{
		word MID = Mem[j];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == MSN[j] )
			{
				if ( Top == 0xFFFF )
				{
					Top = GetTopology( OB->RealX >> 4, OB->RealY >> 4 );
				};
				int xx = OB->RealX >> 11;
				int yy = OB->RealY >> 11;
				if ( xx < MinX )MinX = xx;
				if ( yy < MinY )MinY = yy;
				if ( xx > MaxX )MaxX = xx;
				if ( yy > MaxY )MaxY = yy;
			};
		};
	};
	byte Mask = 1 << CT->NI;
	if ( MaxX >= MinX )
	{
		MinX -= 4;
		MaxX += 4;
		MinY -= 4;
		MaxY += 4;
		if ( MinX < 0 )MinX = 0;
		if ( MinY < 0 )MinY = 0;
		for ( int ix = MinX; ix <= MaxX; ix++ )
		{
			for ( int iy = MinY; iy <= MaxY; iy++ )
			{
				OR1->AddEnemXY( ix, iy, Top, Mask );
			};
		};
		OR1->MinX = MinX;
		OR1->MaxX = MaxX;
		OR1->MinY = MinY;
		OR1->MaxY = MaxY;
	}
	else
	{
		DeleteBOrder();
		return;
	};
};
extern City CITY[8];
void SelBrigade( byte NI, byte Type, int ID );
void ImSelBrigade( byte NI, byte Type, int ID );

void CorrectBrigadesSelection( byte NT )
{
	if ( sizeof( NSL ) <= NT )//BUGFIX: NT > 7 can cause access violation
	{//Call stack: ExecuteBuffer() -> RememSelection()
		return;
	}

	int BrigsID[512];
	byte BFlags[128];
	memset( BFlags, 0, sizeof BFlags );
	int NBR = 0;
	int N = NSL[NT];
	word* mon = Selm[NT];
	City* CT = CITY + NatRefTBL[NT];
	for ( int i = 0; i < N; i++ )
	{
		word MID = mon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				if ( OB->InArmy && OB->BrigadeID != 0xFFFF )
				{
					int idx = OB->BrigadeID;
					int xx = idx >> 3;
					int sms = 1 << ( idx & 7 );
					if ( !( BFlags[xx >> 3] & sms ) )
					{
						if ( NBR < 512 )
						{
							BrigsID[NBR] = idx;
							NBR++;
							BFlags[xx >> 3] |= sms;
						}
					}
				}
			}
		}
	}


	for ( int i = 0; i < NBR; i++ )
	{
		SelBrigade( NT, 1, BrigsID[i] );
	}
}

void CorrectImSelectionInGroups( byte NI );
void ImCorrectBrigadesSelection( byte NT )
{
	int BrigsID[512];
	byte BFlags[128];
	memset( BFlags, 0, sizeof BFlags );
	int NBR = 0;
	int N = ImNSL[NT];
	word* mon = ImSelm[NT];
	City* CT = CITY + NatRefTBL[NT];
	for ( int i = 0; i < N; i++ )
	{
		word MID = mon[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				if ( OB->InArmy&&OB->BrigadeID != 0xFFFF )
				{
					int idx = OB->BrigadeID;
					int xx = idx >> 3;
					int sms = 1 << ( idx & 7 );
					if ( !( BFlags[xx >> 3] & sms ) )
					{
						if ( NBR < 512 )
						{
							BrigsID[NBR] = idx;
							NBR++;
							BFlags[xx >> 3] |= sms;
						};
					};
				};
			};
		};
	};
	for ( int i = 0; i < NBR; i++ )
	{
		ImSelBrigade( NT, 1, BrigsID[i] );
	};
	CorrectImSelectionInGroups( NT );
};
//                                 //\\
//                               // /\ \\
//                             // / /\ \ \\
//----------------------------<< WATER AI >>--------------------------//
//                             \\ \ \/ / //
//                               \\ \/ //
//                                 \\//

//Human ships location system



//==========================RESEARCH ISLANDS===============================//
int NIslands = 0;
#define MaxIsl 64
int IslandX[MaxIsl];
int IslandY[MaxIsl];
byte IslPrs[MaxIsl];

word* TopIslands = NULL;
word* NearWater = NULL;
void CheckGP();
void ArrangeAreas( int ID, byte IsID, int Deep )
{
	TopIslands[ID] = IsID;
	if ( Deep < 100 )
	{
		Area* AR = TopMap + ID;
		for ( int i = 0; i < AR->NLinks; i++ )
		{
			int D1 = AR->Link[i + i];
			if ( TopIslands[D1] == 0xFFFF )ArrangeAreas( D1, IsID, Deep + 1 );
		};
	};
};
void ResearchIslands()
{
	if ( !NAreas )return;
	if ( TopIslands )
	{
		TopIslands = (word*) realloc( TopIslands, 2 * NAreas );
		NearWater = (word*) realloc( NearWater, 2 * NAreas );
	}
	else
	{
		TopIslands = new word[NAreas];
		NearWater = new word[NAreas];
	};
	memset( TopIslands, 0xFF, 2 * NAreas );
	NIslands = 0;
	for ( int j = 0; j < NAreas; j++ )if ( TopIslands[j] == 0xFFFF )
	{
		ArrangeAreas( j, NIslands, 0 );
		for ( int p = 0; p < NAreas; p++ )if ( TopIslands[p] == NIslands )ArrangeAreas( p, NIslands, 0 );
		NIslands++;
	}
	word NTIsl[MaxIsl];
	assert( NIslands < MaxIsl );
	//calculating center
	for ( int i = 0; i < NIslands; i++ )
	{
		IslandX[i] = 0;
		IslandY[i] = 0;
		NTIsl[i] = 0;
	};
	for ( int i = 0; i < NAreas; i++ )
	{
		Area* Ar = TopMap + i;
		int ISL = TopIslands[i];
		IslandX[ISL] += Ar->x;
		IslandY[ISL] += Ar->y;
		NTIsl[ISL]++;
	};
	for ( int i = 0; i < NIslands; i++ )
	{
		IslandX[i] /= NTIsl[i];
		IslandY[i] /= NTIsl[i];
		IslPrs[i] = 0;
	};
	//calculating the cost
	memset( NearWater, 0xFF, NAreas << 1 );
	int tpmax = msx >> 1;
	for ( int ix = 3; ix < tpmax - 3; ix++ )
		for ( int iy = 3; iy < tpmax - 3; iy++ )
		{
			int ctp = SafeTopRef( ix, iy );
			if ( ctp < 0xFFFE )
			{
				for ( int dx = -3; dx <= 3; dx++ )
					for ( int dy = -3; dy <= 3; dy++ )
					{
						int wtp = WTopRef[ix + dx + ( ( iy + dy ) << TopSH )];
						if ( wtp < 0xFFFE )
						{
							if ( NearWater[ctp] == 0xFFFF )
							{
								NearWater[ctp] = wtp;
							}
							else
							{
								Area* CAR = TopMap + ctp;
								Area* OLD = WTopMap + NearWater[ctp];
								Area* NEW = WTopMap + wtp;
								if ( Norma( CAR->x - OLD->x, CAR->y - OLD->y ) > Norma( CAR->x - NEW->x, CAR->y - NEW->y ) )
								{
									NearWater[ctp] = wtp;
								};
							};
						};
					};
			};
		};
};

//Some island AI logic
void ResearchCurrentIsland( byte Nat )
{
	if ( NATIONS[Nat].AI_Enabled && CITY[Nat].MyIsland != 0xFF )
	{
		if ( SCENINF.hLib && !SCENINF.StandartVictory )
			return;
	}

	if ( !TopIslands )
		return;

	int NInIsl[MaxIsl];

	memset( NInIsl, 0, sizeof NInIsl );
	memset( IslPrs, 0, sizeof IslPrs );

	int N = NtNUnits[Nat];
	word* Uni = NatList[Nat];
	if ( !N )
	{
		CITY[Nat].MyIsland = 0xFF;
		return;
	}

	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Uni[i]];
		if ( OB )
		{
			int tof = ( OB->RealX / 1024 ) + ( ( OB->RealY / 1024 ) << TopSH );
			if ( tof >= 0 && tof < TopLx*TopLx )
			{
				int tp = TopRef[tof];
				if ( tp < 0xFFFE )
				{
					NInIsl[TopIslands[tp]]++;
					if ( !( OB->NewBuilding || OB->Sdoxlo || OB->LockType ) )
					{
						IslPrs[TopIslands[tp]] |= 1 << OB->NNUM;
					}
				}
			}
		}
	}

	int NMax = 0;
	int CISL = 0xFF;
	for ( int i = 0; i < NIslands; i++ )
	{
		if ( NInIsl[i] > NMax )
		{
			NMax = NInIsl[i];
			CISL = i;
		}
	}
	CITY[Nat].MyIsland = CISL;
}

//======================RESEARCH BEST PORT TO FISH=========================//
void ResearchBestPortToFish( byte Nat )
{

	int N = NtNUnits[Nat];
	word* Uni = NatList[Nat];
	if ( !N )return;



	if ( CITY[0].MyIsland == 255 || !NATIONS[Nat].AI_Enabled )return;
	int x0 = IslandX[CITY[0].MyIsland] << 10;
	int y0 = IslandY[CITY[0].MyIsland] << 10;



	int MaxR = 0;
	int BestID = -1;
	int NP = 0;

	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Uni[i]];
		if ( OB && ( !OB->Sdoxlo ) && OB->newMons->Usage == PortID )
		{
			int R = Norma( OB->RealX - x0, OB->RealY - y0 );
			if ( R > MaxR )
			{
				MaxR = R;
				BestID = OB->Index;
			};
			NP++;
		};
	};

	if ( BestID != -1 )
	{
		for ( int i = 0; i < N; i++ )
		{
			OneObject* OB = Group[Uni[i]];
			if ( OB && ( !OB->Sdoxlo ) && OB->newMons->Usage == PortID )
			{
				if ( OB->Index == BestID )
				{

					if ( NP )
					{
						OB->StandGround = 1;

						OB->NoSearchVictim = 1;

					}
					else
					{
						OB->StandGround = 1;

						OB->NoSearchVictim = 0;

					};
				}
				else
				{

					if ( NP )
					{
						OB->StandGround = 0;

						OB->NoSearchVictim = 0;

					}
					else
					{
						OB->StandGround = 1;

						OB->NoSearchVictim = 0;

					};
				};
			};
		};
	};
};
void ProduceBattleShip( byte nat, word ID )
{
	int N = NtNUnits[nat];
	word* Uni = NatList[nat];
	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Uni[i]];
		if ( OB && ( !( OB->Sdoxlo || OB->LocalOrder ) ) && OB->newMons->Usage == PortID )
		{
			if ( !OB->NoSearchVictim )OB->Produce( ID );
		};
	};
};
void ProduceLodka( byte nat, word ID )
{
	int N = NtNUnits[nat];
	word* Uni = NatList[nat];
	for ( int i = 0; i < N; i++ )
	{
		OneObject* OB = Group[Uni[i]];
		if ( OB && ( !( OB->Sdoxlo || OB->LocalOrder ) ) && OB->newMons->Usage == PortID )
		{
			if ( OB->StandGround )OB->Produce( ID );
		};
	};
};
void ProduceByAI( byte nat, word Producer, word ID )
{
	byte Use = NATIONS[nat].Mon[ID]->newMons->Usage;
	switch ( Use )
	{
	case FisherID:
		ProduceLodka( nat, ID );
		break;
	case GaleraID:
	case LinkorID:
	case IaxtaID:
	case FregatID:
	case ShebekaID:
		ProduceBattleShip( nat, ID );
		break;
	default:
		Group[Producer]->Produce( ID );
	};
};
//====================Research human places for attack=====================//




//extern word TowsID[24];
//extern word TowsSN[24];
word SearchMineToDestroy( Brigade* BR )
{
	int NGalley = 0;
	word GalleyID[32];
	int GalleyX[32];
	int GalleyY[32];
	int GalleyR = 0;
	for ( int i = 0; i < BR->NMemb&&NGalley < 32; i++ )
	{
		word mid = BR->Memb[i];
		if ( mid != 0xFFFF )
		{
			OneObject* OB = Group[mid];
			if ( OB&&OB->newMons->Usage == GaleraID && !OB->Sdoxlo )
			{
				if ( !NGalley )
				{
					GalleyR = OB->Ref.General->MoreCharacter->MaxR_Attack << 4;
				};
				GalleyID[NGalley] = OB->Index;
				GalleyX[NGalley] = OB->RealX;
				GalleyY[NGalley] = OB->RealY;
				NGalley++;
			};
		};
	};
	if ( !NGalley )return 0xFFFF;

	int MinD = 1000000;
	word BestMINE = 0xFFFF;
	GalleyR -= 64 * 16;
	if ( BR->NMemb >= 4 )
	{
		for ( int MID = 0; MID < MAXOBJECT; MID++ )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				int mx = OB->RealX;
				int my = OB->RealY;
				byte Use = OB->newMons->Usage;
				if ( Use == MineID || Use == TowerID )
				{
					for ( int j = 0; j < NGalley; j++ )
					{
						int D = Norma( GalleyX[j] - mx, GalleyY[j] - my );
						if ( D < MinD&&D < GalleyR )
						{
							MinD = D;
							BestMINE = OB->Index;
						};
					};
				};
			};
		};
	}
	else
	{
		for ( int MID = 0; MID < MAXOBJECT; MID++ )
		{
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB&&OB->newMons->Usage == MineID && !OB->Sdoxlo )
				{
					int mx = OB->RealX;
					int my = OB->RealY;
					for ( int j = 0; j < NGalley; j++ )
					{
						int D = Norma( GalleyX[j] - mx, GalleyY[j] - my );
						if ( D < MinD&&D < GalleyR )
						{
							MinD = D;
							BestMINE = OB->Index;
						};
					};
				};
			};
		};
	};
	if ( BestMINE != 0xFFFF )
	{
		for ( int i = 0; i < NGalley; i++ )
		{
			Group[GalleyID[i]]->AttackObj( BestMINE, 128 + 16, 0, 0 );
		};
	};
	return BestMINE;
};
int GetShipForce( byte Usage );
word SearchEnemyToDestroy( Brigade* BR )
{
	int RMIN[128];
	int RMAX[128];
	int DANG[128];
	int XI[128];
	int YI[128];
	int XC = 0;
	int YC = 0;
	int DR = 0;
	int NS = 0;
	EnemyInfo* EIN = GNFO.EINF[BR->CT->NI];
	if ( !EIN )
	{
		return 0xFFFF;
	}
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( NS < 128 && OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] )
			{
				byte Usage = OB->newMons->Usage;
				AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
				if ( Usage == GaleraID )
				{
					RMIN[NS] = ADC->AttackRadius1[1];
					RMAX[NS] = ADC->AttackRadius2[1] - 64 - DR;
				}
				else
				{
					RMIN[NS] = ADC->AttackRadius1[0];
					RMAX[NS] = ADC->AttackRadius2[0] - 64 - DR;
				};

				RMIN[NS] <<= 4;
				RMAX[NS] <<= 4;

				DANG[NS] = GetShipForce( Usage );
				XI[NS] = OB->RealX;
				YI[NS] = OB->RealY;
				NS++;
				XC += OB->RealX;
				YC += OB->RealY;
			};
		};
	};
	if ( NS )
	{
		int MAXR = 0;
		XC /= NS;
		YC /= NS;
		int MAXF = 1000000;//-1;
		word BestID = 0xFFFF;
		for ( int j = 0; j < NS; j++ )
		{
			int R = Norma( XI[j] - XC, YI[j] - YC );
			if ( R + RMAX[j] > MAXR )MAXR = R + RMAX[j];
		};
		if ( BR->LastEnemyID != 0xFFFF )
		{
			OneObject* OB = Group[BR->LastEnemyID];
			if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->LastEnemySN&&OB->LockType )
			{
				if ( Norma( XC - OB->RealX, YC - OB->RealY ) < MAXR )BestID = BR->LastEnemyID;
			};
		};
		if ( BestID == 0xFFFF )
		{
			HumanShip* HS = EIN->SHIPS;
			for ( int i = 0; i < EIN->NHSHIPS; i++ )
			{
				int DR = Norma( XC - HS->x, YC - HS->y );
				if ( DR < MAXR )
				{
					if ( MAXF > DR )
					{
						OneObject* OB = Group[HS->ID];
						if ( OB && !OB->Sdoxlo )
						{
							MAXF = DR;
							BestID = HS->ID;
						}
					}
				}
				HS++;
			}
		}

		if ( BestID != 0xFFFF )
		{
			return BestID;
		}
		//search for tower
		int MinTowR = 100000;
		BestID = 0xFFFF;
		for ( int i = 0; i < 24; i++ )
		{
			if ( EIN->TowsID[i] != 0xFFFF )
			{
				OneObject* OB = Group[EIN->TowsID[i]];
				if ( OB && ( !OB->Sdoxlo ) && OB->Serial == EIN->TowsSN[i] )
				{
					int x = OB->RealX;
					int y = OB->RealY;
					int DR = Norma( x - XC, y - YC );
					if ( DR < MAXR&&DR < MinTowR )
					{
						MinTowR = DR;
						BestID = OB->Index;
					}
				}
			}
		}
		return BestID;
	}
	else
	{
		return 0xFFFF;
	}
}

void WaterAttackLink( OneObject* OBJ );
int TryToBeatByShips( Brigade* BR )
{
	word ENEMY = SearchEnemyToDestroy( BR );
	if ( ENEMY != 0xFFFF )
	{
		OneObject* EOB = Group[ENEMY];
		if ( EOB->LockType )
		{
			BR->LastEnemyID = ENEMY;
			BR->LastEnemySN = EOB->Serial;
			for ( int i = 0; i < BR->NMemb; i++ )
			{
				word MID = BR->Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] )
					{
						if ( OB->LocalOrder&&OB->LocalOrder->DoLink == WaterAttackLink )
						{
							if ( OB->EnemyID != ENEMY )
							{
								OB->DeleteLastOrder();
								OB->AttackObj( ENEMY, 128 + 16, 1, 0 );
								if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &WaterAttackLink&&OB->newMons->Usage == GaleraID )
								{
									OB->LocalOrder->info.BuildObj.AttMethod = 1;
								};
							};
						}
						else
						{
							OB->AttackObj( ENEMY, 128 + 16, 1, 0 );
							if ( OB->LocalOrder&&OB->LocalOrder->DoLink == &WaterAttackLink&&OB->newMons->Usage == GaleraID )
							{
								OB->LocalOrder->info.BuildObj.AttMethod = 1;
							};
						};
					};
				};
			};
		}
		else
		{
			int X0 = EOB->RealX;
			int Y0 = EOB->RealY;
			int NA = 0;
			for ( int i = 0; i < BR->NMemb; i++ )
			{
				word MID = BR->Memb[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] )
					{
						bool DoAttack = false;
						int R = Norma( OB->RealX - X0, OB->RealY - Y0 ) >> 4;
						AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
						byte Usage = OB->newMons->Usage;
						if ( Usage == GaleraID )
						{
							if ( R > ADC->AttackRadius1[1] + 64 && R < ADC->AttackRadius2[1] - 64 )DoAttack = true;
						}
						else
						{
							if ( R > ADC->AttackRadius1[0] + 64 && R < ADC->AttackRadius2[0] - 64 )DoAttack = true;
						};
						if ( DoAttack&&OB->EnemyID != EOB->Index )OB->AttackObj( EOB->Index, 128 + 16, 0, 10 );
						if ( OB->EnemyID == EOB->Index )NA++;
					};
				};
			};
			if ( NA )return 2;
			else return 0;
		};
		return 1;
	}
	else
	{
		for ( int i = 0; i < BR->NMemb; i++ )
		{
			word MID = BR->Memb[i];
			if ( MID != 0xFFFF )
			{
				OneObject* OB = Group[MID];
				if ( OB && ( !OB->Sdoxlo ) && OB->Serial == BR->MembSN[i] &&
					OB->LocalOrder&&OB->LocalOrder->DoLink == &WaterAttackLink&&
					OB->LocalOrder->info.BuildObj.SN == 0 )
				{
					OB->DeleteLastOrder();
				};
			};
		};
		return 0;
	};
};
int TryToBeatByShipsWhenStand( Brigade* BR )
{
	int RS = TryToBeatByShips( BR );
	if ( RS == 0 || RS == 2 )
	{

	};
	return 0;
};

class LSS_Order :public BrigadeOrder
{
public:
	int FirstTime;
	int NPos;
	int Position[30];
};
void LocalSendShipsLink( Brigade* BR )
{
	if ( TryToBeatByShips( BR ) )return;
	LSS_Order* OR = (LSS_Order*) BR->BOrder;
	int NReady = 0;
	int NTot = 0;
	for ( int i = 0; i < OR->NPos; i++ )
	{
		int i3 = i + i + i;
		word ID = OR->Position[i3 + 2] & 0xFFFF;
		word SN = DWORD( OR->Position[i3 + 2] ) >> 16;
		OneObject* OB = Group[ID];
		if ( OB && !OB->Sdoxlo )
		{
			int r = Norma( OB->RealX - OR->Position[i3], OB->RealY - OR->Position[i3 + 1] );
			if ( r < 150 * 16 )NReady++;
			else if ( !OB->LocalOrder )
			{
				OB->NewMonsterSmartSendTo( OR->Position[i3] >> 4, OR->Position[i3 + 1] >> 4, 0, 0, 18 + 16, 0 );
			};
			NTot++;
		};
	};
	if ( NReady == NTot )
	{
		BR->DeleteBOrder();
		return;
	};
	if ( OR->FirstTime == -1 )
	{
		if ( NReady )OR->FirstTime = tmtmt;
	}
	else
	{
		if ( tmtmt - OR->FirstTime > 350 )
			BR->DeleteBOrder();
	};
};
extern int RealLx;
extern int RealLy;
void CBar( int x, int y, int Lx, int Ly, byte c );
void ShowShipsDest( Brigade* BR )
{
	if ( BR->BOrder&&BR->BOrder->BLink == LocalSendShipsLink )
	{
		LSS_Order* LO = (LSS_Order*) BR->BOrder;
		for ( int i = 0; i < LO->NPos; i++ )
		{
			int x = LO->Position[i + i + i] >> 4;
			int y = LO->Position[i + i + i + 1] >> 4;
			int sx = x - ( mapx << 5 );
			int sy = ( y >> 1 ) - ( mapy << 4 ) - GetHeight( x, y );
			if ( sx > 0 && sy > 0 && sx < RealLx&&sy < RealLy )
			{
				CBar( sx - 4, sy - 4, 8, 8, 255 );
			};
		};
	};
};
void LocalSendShipsOld( Brigade* BR, int x, int y, byte Type )
{
	LSS_Order* OR = (LSS_Order*) BR->CreateOrder( Type, sizeof( LSS_Order ) - 120 + BR->NMemb * 12 );
	OR->Size = sizeof( LSS_Order ) - 120 + BR->NMemb * 12;
	OR->NPos = BR->NMemb;
	OR->Message = NULL;
	int N1 = 0;
	int MinX = 1000;
	int MaxX = -1000;
	int MinY = 1000;
	int MaxY = -1000;
	int Quant = 4 * 64 * 16;
	int px[200];
	int py[200];
	int Np = 0;
	int Maxp = BR->NMemb;
	for ( int r = 0; r < 10; r++ )
	{
		char* xi = Rarr[r].xi;
		char* yi = Rarr[r].yi;
		int N = Rarr[r].N;
		for ( int j = 0; j < N; j++ )
		{
			int dx = xi[j];
			int dy = yi[j];
			int xx = x + dx*Quant;
			int yy = y + dy*Quant;
			if ( !MFIELDS[1].CheckBar( ( xx >> 8 ) - 7, ( yy >> 8 ) - 7, 14, 14 ) )
			{
				px[Np] = dx;
				py[Np] = dy;
				Np++;
				if ( dx < MinX )MinX = dx;
				if ( dx > MaxX )MaxX = dx;
				if ( dy < MinY )MinY = dy;
				if ( dy > MaxY )MaxY = dy;
				if ( Np >= Maxp )goto pos1;
			};
		};
	};
pos1:
	SortClass SCL;
	//need to sort units
	UNISORT.CreateByY( BR->Memb, BR->NMemb, true );
	UNISORT.Sort();
	int NU = UNISORT.NUids;
	word Ids[200];
	word IdPos[200];
	UNISORT.Copy( Ids );
	word pxi[200];
	int pxx[200];
	int pxpos = 0;
	for ( int yp = MinY; yp <= MaxY; yp++ )
	{
		int npx = 0;
		for ( int j = 0; j < Np; j++ )if ( py[j] == yp )
		{
			pxi[npx] = j;
			pxx[npx] = px[j];
			npx++;
		};
		//sorting positions by x
		SCL.CheckSize( npx );
		memcpy( SCL.Uids, pxi, npx << 1 );
		memcpy( SCL.Parms, pxx, npx << 2 );
		SCL.NUids = npx;
		SCL.Sort();
		SCL.Copy( IdPos + pxpos );
		UNISORT.CreateByX( Ids + pxpos, npx, true );
		UNISORT.Sort();
		UNISORT.Copy( Ids + pxpos );
		pxpos += npx;
	};
	//sending to positions
	for ( int i = 0; i < pxpos; i++ )
	{
		OneObject* OB = Group[Ids[i]];
		int ps = IdPos[i];
		int xx = x + px[ps] * Quant;
		int yy = y + py[ps] * Quant;
		OB->NewMonsterSmartSendTo( xx >> 4, yy >> 4, 0, 0, 128 + 16, 0 );
		int i3 = i + i + i;
		OR->Position[i3] = xx;
		OR->Position[i3 + 1] = yy;
		DWORD PP = Ids[i] + ( DWORD( OB->Serial ) << 16 );
		OR->Position[i3 + 2] = PP;
	};
	OR->NPos = pxpos;
	OR->FirstTime = -1;
	OR->BLink = &LocalSendShipsLink;
};
byte FormSize[55 * 3] =
{ 1,1,2,//1
 2,1,3,//2
 3,1,4,//3
 4,1,5,//4
 5,1,6,//5
 3,2,7,//6
 4,2,9,//7
 4,2,9,//8
 5,2,11,//9
 5,2,11,//10
 6,2,13,//11
 6,2,13,//12
 5,3,16,//13
 5,3,16,//14
 5,3,16,//15
 6,3,19,//16
 6,3,19,//17
 6,3,19,//18
 7,3,22,//19
 7,3,22,//20
 7,3,22,//21
 8,3,25,//22
 8,3,25,//23
 8,3,25,//24
 7,4,29,//25
 7,4,29,//26
 7,4,29,//27
 7,4,29,//28
 8,4,33,//29
 8,4,33,//30
 8,4,33,//31
 8,4,33,//32
 9,4,37,//33
 9,4,37,//34
 9,4,37,//35
 9,4,37,//36
 10,4,41,//37
 10,4,41,//38
 10,4,41,//39
 10,4,41,//40
 9,5,46,//41
 9,5,46,//42
 9,5,46,//43
 9,5,46,//44
 9,5,46,//45
 10,5,51,//46
 10,5,51,//47
 10,5,51,//48
 10,5,51,//49
 10,5,51,//50
 11,5,56,//51
 11,5,56,//52
 11,5,56,//53
 11,5,56,//54
 11,5,56 };//55
int MTRLX;
int MTRLY;
byte* MTRPTR;
inline byte GetPX( int x, int y )
{
	if ( x >= 0 && y >= 0 && x < MTRLX&&y < MTRLY )return MTRPTR[x + y*MTRLX];
	else return 0;
};
inline void SetPX( int x, int y, byte Val )
{
	if ( x >= 0 && y >= 0 && x < MTRLX&&y < MTRLY )MTRPTR[x + y*MTRLX] = Val;
};
void LocalSendShips( Brigade* BR, int x, int y, byte Type )
{
	//getting the direction
	int xc = 0;
	int yc = 0;
	int Nc = 0;
	int N = BR->NMemb;
	for ( int i = 0; i < N; i++ )
	{
		word MID = BR->Memb[i];
		OneObject* OB = Group[MID];
		if ( OB )
		{
			Nc++;
			xc += OB->RealX;
			yc += OB->RealY;
		};
	};
	if ( Nc )
	{
		xc /= Nc;
		yc /= Nc;
	}
	else return;
	int dx = x - xc;
	int dy = y - yc;
	int Nr = Norma( dx, dy ) >> 4;
	if ( !Nr )return;
	dx = ( dx * 6 * 64 ) / Nr;
	dy = ( dy * 6 * 64 ) / Nr;
	int fdx = -dy;
	int fdy = dx;
	byte Matrix[8 * 32];
	MTRPTR = Matrix;
	//byte NInLine[16];
	//int NSLines;
	int NTot = Nc;
	int NP, Nx, Ny;

	do
	{
		if ( Nc > 55 )
		{
			LocalSendShipsOld( BR, x, y, Type );
			return;
		};
		Nx = FormSize[Nc + Nc + Nc - 3];
		Ny = FormSize[Nc + Nc + Nc - 2];
		MTRLX = Nx;
		MTRLY = Ny;
		int x0 = x - ( ( fdx*Nx - dx*Ny ) >> 1 );
		int y0 = y - ( ( fdy*Nx - dy*Ny ) >> 1 );
		int ofs = 0;
		int ix, iy;
		for ( ix = 0; ix < Nx; ix++ )
		{
			for ( iy = 0; iy < Ny; iy++ )
			{
				if ( MFIELDS[1].CheckBar( ( ( x0 + fdx*ix - dx*iy ) >> 8 ) - 8, ( ( y0 + fdy*ix - dy*iy ) >> 8 ) - 8, 16, 16 ) )Matrix[ofs] = 0;
				else Matrix[ofs] = 1;
				ofs++;
			};
		};
		int cpx = Nx >> 1;
		int cpy = Ny >> 1;
		bool Change = false;
		NP = 0;
		int r0 = 2;
		if ( GetPX( cpx, cpy ) )
		{
			SetPX( cpx, cpy, 2 );
			NP++;
		};
		do
		{
			Change = false;
			int rrr = r0 >> 1;
			char* xi = Rarr[rrr].xi;
			char* yi = Rarr[rrr].yi;
			int Ni = Rarr[rrr].N;
			for ( int j = 0; j < Ni; j++ )
			{
				int xx = cpx + xi[j];
				int yy = cpy + yi[j];
				if ( NP < NTot&&GetPX( xx, yy ) == 1 && ( GetPX( xx + 1, yy ) == 2 || GetPX( xx - 1, yy ) == 2 || GetPX( xx, yy - 1 ) == 2 || GetPX( xx, yy + 1 ) == 2 ) )
				{
					Change = true;
					NP++;
					SetPX( xx, yy, 2 );
				};
			};
			r0++;
		} while ( Change );
		if ( NP != NTot )
		{
			Nc = FormSize[Nc + Nc + Nc - 1];
		};
	} while ( NP != NTot );

	LSS_Order* OR = (LSS_Order*) BR->CreateOrder( Type, sizeof( LSS_Order ) - 120 + BR->NMemb * 12 );
	OR->Size = sizeof( LSS_Order ) - 120 + BR->NMemb * 12;
	OR->NPos = BR->NMemb;
	OR->Message = NULL;
	OR->FirstTime = -1;
	OR->BLink = &LocalSendShipsLink;

	UNISORT.CreateByLine( BR->Memb, BR->NMemb, fdx, fdy );
	UNISORT.Sort();
	word Ids[100];
	int upos = 0;
	int x0 = x - ( ( fdx*Nx - dx*Ny ) >> 1 );
	int y0 = y - ( ( fdy*Nx - dy*Ny ) >> 1 );
	UNISORT.Copy( Ids );
	int ppos = 0;
	for ( int ix = 0; ix < Nx; ix++ )
	{
		int NN = 0;
		int pos0 = ppos;
		for ( int iy = 0; iy < Ny; iy++ )
		{
			if ( GetPX( ix, iy ) == 2 )
			{
				OR->Position[ppos] = x0 + fdx*ix - dx*iy;
				OR->Position[ppos + 1] = y0 + fdy*ix - dy*iy;
				//assert(!MFIELDS[1].CheckBar((OR->Position[ppos]>>8)-8,(OR->Position[ppos+1]>>8)-8,16,16));
				NN++;
				ppos += 3;
			};
		};
		if ( NN )
		{
			UNISORT.CreateByLineForShips( Ids + upos, NN, -dx, -dy );
			UNISORT.Sort();
			UNISORT.Copy( Ids + upos );
			int j3 = 0;
			for ( int j = 0; j < NN; j++ )
			{
				OR->Position[pos0 + j3 + 2] = Ids[upos + j];
				word MID = Ids[upos + j];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB )
					{
						OB->NewMonsterSmartSendTo( OR->Position[pos0 + j3] >> 4, OR->Position[pos0 + j3 + 1] >> 4, 0, 0, 128 + 16, 0 );
					};
				};
				j3 += 3;
			};
			upos += NN;
		};
	};
};
class Ship_Battle :public BrigadeOrder
{
public:
	byte ComType;
	word CurTop;
	int Params[4];
};
word GetNearestBrig( City* CT, word Top )
{
	int D = 1000000;
	int BRB = -1;
	int TP1 = Top*WNAreas;
	for ( int i = 0; i < 128; i++ )
	{
		int N = CT->WaterBrigs[i];
		if ( N != 0xFFFF )
		{
			Brigade* BR = CT->Brigs + N;
			if ( BR->Enabled )
			{
				int TP = BR->LastTopology;
				if ( TP != 0xFFFF )
				{
					if ( TP == Top )
					{
						return N;
					}
					else
					{
						int R = WLinksDist[TP1 + TP];
						if ( R < D )
						{
							D = R;
							BRB = N;
						};
					};
				};
			};
		};
	};
	return BRB;
};
word GetNearestBrigToKillUnitsAndTowers( City* CT, word Top )
{
	int D = 1000000;
	int BRB = -1;
	int TP1 = Top*WNAreas;
	for ( int i = 0; i < 128; i++ )
	{
		int N = CT->WaterBrigs[i];
		if ( N != 0xFFFF )
		{
			Brigade* BR = CT->Brigs + N;
			if ( BR->Enabled&&BR->Direction == 0 )
			{
				int TP = BR->LastTopology;
				if ( TP != 0xFFFF )
				{
					if ( TP == Top )
					{
						return N;
					}
					else
					{
						int R = WLinksDist[TP1 + TP];
						if ( R < D )
						{
							D = R;
							BRB = N;
						};
					};
				};
			};
		};
	};
	return BRB;
};
word GetNearestBrigToKillMines( City* CT, word Top )
{
	int D = 1000000;
	int BRB = -1;
	int TP1 = Top*WNAreas;
	for ( int i = 0; i < 128; i++ )
	{
		int N = CT->WaterBrigs[i];
		if ( N != 0xFFFF )
		{
			Brigade* BR = CT->Brigs + N;
			if ( BR->Enabled&&BR->Direction == 1 )
			{
				int TP = BR->LastTopology;
				if ( TP != 0xFFFF )
				{
					if ( TP == Top )
					{
						return N;
					}
					else
					{
						int R = WLinksDist[TP1 + TP];
						if ( R < D )
						{
							D = R;
							BRB = N;
						};
					};
				};
			};
		};
	};
	return BRB;
};
word GetNearestBrigWithType( City* CT, word Top, byte Type )
{
	int D = 1000000;
	int BRB = -1;
	int TP1 = Top*WNAreas;
	for ( int i = 0; i < 128; i++ )
	{
		int N = CT->WaterBrigs[i];
		if ( N != 0xFFFF )
		{
			Brigade* BR = CT->Brigs + N;
			if ( BR->Enabled&&BR->Direction == Type )
			{
				int TP = BR->LastTopology;
				if ( TP != 0xFFFF )
				{
					if ( TP == Top )
					{
						return N;
					}
					else
					{
						int R = WLinksDist[TP1 + TP];
						if ( R < D )
						{
							D = R;
							BRB = N;
						};
					};
				};
			};
		};
	};
	return BRB;
};
void MakeShipBattle( Brigade* BR );
void MakeShipBattleLink( Brigade* BR )
{
	if ( PeaceTimeLeft )return;
	if ( TryToBeatByShips( BR ) )return;
	Ship_Battle* OR = (Ship_Battle*) BR->BOrder;
	if ( OR->Params[1] != -1 )
	{
		OR->CurTop = OR->Params[1];
		OR->Params[1] = -1;
	};
	if ( BR->NMemb == 0 )
	{
		BR->DeleteBOrder();
		return;
	};
	int CurTop = OR->CurTop;
	int NextTop = -1;
	int FinalTop = -1;
	//test for connection
	City* CT = BR->CT;
	word* WDst = GTOP[1].LinksDist;
	int NWA = GTOP[1].NAreas;
	int XC = int( CT->CenterX ) << ( 7 + 4 );
	int YC = int( CT->CenterY ) << ( 7 + 4 );
	EnemyInfo* EIN = GNFO.EINF[BR->CT->NI];

	int MaxShip[2] = { 100,100 };
	int SHIPFORCE = EIN->ShipsForce;
	if ( SHIPFORCE <= 15 )MaxShip[0] = 3;
	else if ( SHIPFORCE <= 25 )MaxShip[0] = 6;
	else if ( SHIPFORCE <= 40 )MaxShip[0] = 12;

	if ( BR->NMemb > MaxShip[BR->Direction] )
	{
		int ID1 = CT->GetFreeBrigade();
		if ( ID1 != -1 )
		{
			Brigade* BR1 = CT->Brigs + ID1;
			CT->RegisterWaterBrigade( ID1 );
			BR->RemoveObjects( BR->NMemb - MaxShip[BR->Direction], BR1 );
			BR1->Enabled = true;
			MakeShipBattle( BR1 );
			return;
		};
	}
	else
	{
		if ( CurTop != 0xFFFF )
		{
			BR->LastTopology = CurTop;
			for ( int i = 0; i < 128; i++ )
			{
				int ID = CT->WaterBrigs[i];
				if ( ID != 0xFFFF && ID != BR->ID )
				{
					Brigade* BRF = CT->Brigs + ID;
					if ( BR->Enabled&&BRF->Direction == BR->Direction&&BR->NMemb + BRF->NMemb <= MaxShip[BR->Direction] )
					{
						int FTop = BRF->LastTopology;
						if ( FTop != 0xFFFF )
						{
							int D = WDst[FTop*NWA + CurTop];
							if ( FTop == CurTop || D < 24 )
							{
								if ( BR->NMemb < BRF->NMemb )
								{
									BR->RemoveObjects( BR->NMemb, BRF );
									BR->DeleteAll();
									BR->Enabled = false;
									CT->UnRegisterWaterBrigade( BR->ID );
								}
								else
								{
									BRF->RemoveObjects( BRF->NMemb, BR );
									BRF->DeleteAll();
									BRF->Enabled = false;
									CT->UnRegisterWaterBrigade( BRF->ID );
								};
								return;
							};
						};
					};
				};
			};
		};
	};
	int MyForce = 0;
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			word SN = BR->MembSN[i];
			OneObject* OB = Group[MID];
			if ( OB&&OB->Serial == SN )MyForce += GetShipForce( OB->newMons->Usage );
		};
	};
	int MinF = MyForce >> 2;
	if ( MinF > 15 )MinF = 15;
	//switch(OR->ComType){
	//case 0://razvedka
	//case 1:
		/*
		if(BR->NMemb>1000){
			OR->Params[0]=-1;
			OR->ComType=2;
		}else{
			if(OR->Params[0]==-1){
				//need to search final zone
				int NTop=GTOP[1].NAreas;
				int NAtt=0;
				int FinalTop=-1;
				while(NAtt<50&&FinalTop==-1){
					NAtt++;
					int Topz=(int(rando())*NTop)>>15;
					int D=GTOP[1].LinksDist[Topz*NTop+CurTop];
					if(D>20&&D<50)FinalTop=Topz;
				};
				if(FinalTop!=-1){
					OR->Params[0]=FinalTop;
				};
			}else{
				if(OR->Params[0]==CurTop){
					OR->Params[0]=-1;
				}else{
					FinalTop=OR->Params[0];
				};
			};
		};
		if(MyForce>1)OR->ComType=1;
		*/
		//search for the friend
	{
		int MinD = 0xFFFF;
		for ( int i = 0; i < 128; i++ )
		{
			int ID = CT->WaterBrigs[i];
			if ( ID != 0xFFFF && ID != BR->ID )
			{
				Brigade* BRF = CT->Brigs + ID;
				if ( BR->Enabled&&BR->Direction == BRF->Direction&&BRF->NMemb >= BR->NMemb&&BRF->NMemb + BR->NMemb <= MaxShip[BR->Direction] )
				{
					int FTop = BRF->LastTopology;
					if ( FTop != 0xFFFF )
					{
						int D = WDst[FTop*NWA + CurTop];
						if ( D < MinD )
						{
							int F = EIN->GetMaxForceOnTheWay( CurTop, FTop );
							if ( MyForce >= ( F - ( F >> 2 ) ) )
							{
								MinD = D;
								FinalTop = FTop;
							};
						};
					};
				};
			};
		};
	};
	//	break;
	//case 1://go to strong enemy group
	{
		int FGrp = -1;
		int MinTopDst = 10000;
		HShipsGroup* SGR = EIN->SGRP;
		for ( int i = 0; i < EIN->NSGRP; i++ )if ( SGR->CTop != 0xFFFF )
		{
			int DT = 0;
			if ( SGR->CTop != CurTop )DT = WLinksDist[CurTop*WNAreas + SGR->CTop];
			int R0 = Norma( ( ( SGR->xL + SGR->xR ) >> 1 ) - XC, ( ( SGR->yL + SGR->yR ) >> 1 ) - YC );
			int F = EIN->GetMaxForceOnTheWay( CurTop, SGR->CTop );
			if ( CurTop == SGR->CTop )DT = 0;
			else DT = WLinksDist[CurTop*WNAreas + SGR->CTop];
			if ( DT < MinTopDst )
			{
				F -= F >> 1;
				if ( ( ( R0<1024 * 7 * 16 || MinTopDst>DT ) && MyForce >= F ) || DT < 20 )
				{
					if ( F <= 8 )
					{
						int NB = GetNearestBrig( CT, SGR->CTop );
						if ( NB == BR->ID )
						{
							MinTopDst = DT;
							FGrp = i;
						};
					}
					else
					{
						MinTopDst = DT;
						FGrp = i;
					};
				};
			};
			SGR++;
		};
		if ( FGrp != -1 )
		{
			FinalTop = EIN->SGRP[FGrp].CTop;
		}
		else OR->ComType = 0;
	};
	//attempt to erase human mines
	word MIN = SearchMineToDestroy( BR );
	if ( MIN == 0xFFFF && FinalTop == -1 && BR->Direction == 0 )
	{
		int BestTop = -1;
		int DT = 10000;
		for ( int i = 0; i < EIN->NWTopNear; i++ )
		{
			int top = EIN->WTopNear[i];
			int D = WLinksDist[top*WNAreas + CurTop];
			if ( top == CurTop )D = 0;
			if ( D < DT )
			{
				int F = EIN->GetMaxForceOnTheWay( CurTop, top );
				if ( MyForce >= ( F - ( F >> 2 ) ) )
				{
					if ( F <= 10 )
					{
						int N = GetNearestBrigToKillUnitsAndTowers( CT, top );
						if ( N == BR->ID )
						{
							DT = D;
							BestTop = top;
						};
					}
					else
					{
						DT = D;
						BestTop = top;
					};
				};
			};
		};
		if ( BestTop != -1 )
		{
			FinalTop = BestTop;
		}
	}

	if ( FinalTop == -1 && MIN == 0xFFFF )
	{
		int NTop = GTOP[1].NAreas;
		int NAtt = 0;
		while ( NAtt < 3 && FinalTop == -1 )
		{
			NAtt++;
			int Topz = ( int( rando() )*NTop ) >> 15;
			int N = GetNearestBrigWithType( CT, Topz, BR->Direction );
			if ( N == BR->ID )
			{
				FinalTop = Topz;
			}
		}

		if ( FinalTop != -1 )
		{
			OR->Params[0] = FinalTop;
		}
	}

	if ( FinalTop == CurTop )
	{
		FinalTop = -1;
	}

	if ( FinalTop != -1 && CurTop >= 0 && CurTop < 0xFFFE )
	{
		NextTop = GTOP[1].MotionLinks[FinalTop + CurTop*GTOP[1].NAreas];
		if ( NextTop < 0xFFFE )
		{
			OR->Params[1] = NextTop;
			Area* TA = GTOP[1].TopMap + NextTop;
			LocalSendShips( BR, int( TA->x ) << 10, int( TA->y ) << 10, 1 );
			BR->LastTopology = NextTop;
			return;
		}
	}
}

word DetermineWaterTopology( Brigade* BR )
{
	int NTops = 0;
	//int NInTop[64];
	int Tops[128];
	int xc = 0;
	int yc = 0;
	int Nu = 0;
	for ( int i = 0; i < BR->NMemb; i++ )
	{
		word MID = BR->Memb[i];
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && !OB->Sdoxlo )
			{
				int tx = OB->RealX >> 10;
				int ty = OB->RealY >> 10;
				xc += tx;
				yc += ty;
				Nu++;
				int Top = tx >= 0 && ty >= 0 && tx < TopLx&&ty <= TopLy ? GTOP[1].TopRef[tx + ( ty << TopSH )] : 0xFFFF;
				int j;
				for ( j = 0; j < NTops&&Tops[j] != Top; j++ );
				if ( j >= NTops )
				{
					Tops[NTops] = Top;
					//NInTop[NTops]=1;
					NTops++;
				}
			}
		}
	}
	if ( Nu )
	{
		xc /= Nu;
		yc /= Nu;
	};
	if ( NTops )
	{
		int BestTop = -1;
		int mind = 10000;
		for ( int p = 0; p < NTops; p++ )
		{
			int tp = Tops[p];
			if ( tp >= 0 && tp < 0xFFFE )
			{
				Area* TA = GTOP[1].TopMap + tp;
				int d = Norma( int( TA->x ) - xc, int( TA->y ) - yc );
				if ( mind > d )
				{
					mind = d;
					BestTop = Tops[p];
				};
			};
		};
		return BestTop;
	};
	return 0xFFFF;
};
void MakeShipBattle( Brigade* BR )
{
	word Top = DetermineWaterTopology( BR );
	if ( Top == 0xFFFF )return;
	Ship_Battle* OR = (Ship_Battle*) BR->CreateOrder( 0, sizeof Ship_Battle );
	OR->BLink = &MakeShipBattleLink;
	OR->Message = NULL;
	OR->Size = sizeof Ship_Battle;
	OR->ComType = 0;
	OR->Params[0] = -1;
	OR->Params[1] = -1;
	OR->CurTop = Top;
};
void City::RegisterWaterBrigade( int ID )
{
	for ( int i = 0; i < 128; i++ )
	{
		if ( WaterBrigs[i] == 0xFFFF )
		{
			WaterBrigs[i] = ID;
			return;
		};
	};
};
void City::UnRegisterWaterBrigade( int ID )
{
	for ( int i = 0; i < 128; i++ )
	{
		if ( WaterBrigs[i] == ID )
		{
			WaterBrigs[i] = 0xFFFF;
			return;
		};
	};

};
//=====================DESANT======================//
//=============1.Research cost spots===============//
CostPlace* COSTPL;
int NCost;
int MaxCost;
extern byte* WaterDeep;
extern int WLX;

void CreateCostPlaces()
{
	NCost = 0;
	MotionField* MFT = new MotionField;
	MFT->Allocate();
	MFT->ClearMaps();
	int maxx = (msx) -15;
	for ( int ix = 15; ix < maxx; ix++ )
	{
		for ( int iy = 15; iy < maxx; iy++ )
		{
			int ofs = ix + ( iy << ( 8 + ADDSH ) );
			int Z0 = WaterDeep[ofs];
			int Z1 = WaterDeep[ofs + 1];
			int Z2 = WaterDeep[ofs + WLX];
			int Z3 = WaterDeep[ofs + WLX + 1];
			byte x = 0;
			if ( Z0 > 128 )x |= 1; else x |= 2;
			if ( Z1 > 128 )x |= 1; else x |= 2;
			if ( Z2 > 128 )x |= 1; else x |= 2;
			if ( Z3 > 128 )x |= 1; else x |= 2;
			if ( x == 3 )
			{
				if ( !MFT->CheckBar( ix + ix - 10, iy + iy - 10, 20, 20 ) )
				{
					int DX = Z0 + Z2 - Z1 - Z3;
					int DY = Z0 + Z1 - Z2 - Z3;
					int N = Norma( DX, DY );
					if ( N )
					{
						DX = -( DX * 10 ) / N;
						DY = -( DY * 10 ) / N;
						if ( !( MFIELDS[1].CheckBar( ix + ix + DX - 6, iy + iy + DY - 6, 12, 12 ) ||
							CheckBar( ix + ix - DX - 6, iy + iy - DY - 6, 12, 12 ) ) )
						{
							int R0 = 6;
							while ( !CheckBar( ix + ix - DX - R0 - 1, iy + iy - DY - R0 - 1, R0 + R0 + 2, R0 + R0 + 2 ) )R0++;
							MFT->BSetBar( ix + ix - 10, iy + iy - 10, 20 );
							if ( NCost == MaxCost )
							{
								MaxCost += 100;
								COSTPL = (CostPlace*) realloc( COSTPL, MaxCost * sizeof CostPlace );
							};
							COSTPL[NCost].xc = ix + ix - DX;
							COSTPL[NCost].yc = iy + iy - DY;
							COSTPL[NCost].xw = ix + ix + DX;
							COSTPL[NCost].yw = iy + iy + DY;
							COSTPL[NCost].R = R0;
							int Top = SafeTopRef( ( COSTPL[NCost].xc >> 2 ), ( COSTPL[NCost].yc >> 2 ) );
							if ( Top < 0xFFFE )
							{
								COSTPL[NCost].Island = TopIslands[Top];
							};
							COSTPL[NCost].Access = true;
							NCost++;
						}
					}
				}
			}
		}
	}
	MFT->FreeAlloc();
	free( MFT );
}

int GetWarInCell( int cell )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )return NULL;
	int ofs1 = cell << SHFCELL;
	word MID;
	int NU = 0;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB&&OB->NNUM == 0 && !( OB->LockType || OB->newMons->Capture ) )
			{
				NU++;
			}
		}
	}
	return NU;
}

bool CheckUnitsNearPoint( int x, int y )
{
	int cell = ( ( y >> 3 ) << VAL_SHFCX ) + ( x >> 3 );
	int rx1 = 2;
	int NU = 0;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );
	byte* bpt = NPresence + stcell;
	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCIOFS )
			{
				if ( bpt[0] & 1 )
				{
					NU += GetWarInCell( stcell );
				}
			}
			stcell++;
			bpt++;
		}
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	}
	return NU > 6;
}

int GetTrInCell( int cell )
{
	cell += VAL_MAXCX + 1;
	int NMon = MCount[cell];
	if ( !NMon )
	{
		return 0;
	}

	int ofs1 = cell << SHFCELL;
	word MID;
	for ( int i = 0; i < NMon; i++ )
	{
		MID = GetNMSL( ofs1 + i );
		if ( MID != 0xFFFF )
		{
			OneObject* OB = Group[MID];
			if ( OB && ( !OB->Sdoxlo ) && OB->Transport )
			{
				return OB->Index;
			}
		}
	}
	return 0xFFFF;
}

word CheckTransportNearPoint( int x, int y )
{
	int cell = ( ( y >> 3 ) << VAL_SHFCX ) + ( x >> 3 );
	int rx1 = 2;
	int rx2 = rx1 + rx1 + 1;
	int stcell = cell - rx1 - ( rx1 << VAL_SHFCX );

	byte* bpt = NPresence + stcell;

	for ( int nx = 0; nx < rx2; nx++ )
	{
		for ( int ny = 0; ny < rx2; ny++ )
		{
			if ( stcell >= 0 && stcell < VAL_MAXCIOFS )
			{
				word T = GetTrInCell( stcell );
				if ( T )
				{
					return T;
				}
			}
			stcell++;
			bpt++;
		}
		stcell += VAL_MAXCX - rx2;
		bpt += VAL_MAXCX - rx2;
	}
	return 0xFFFF;
}

void ProcessCostPoints()
{
	if ( !NCost )
	{
		return;
	}

	int N = tmtmt % NCost;
	int R = COSTPL[N].R;

	COSTPL[N].Access = !CheckBar( COSTPL[N].xc - R, COSTPL[N].yc - R, R + R, R + R );
	if ( CheckUnitsNearPoint( COSTPL[N].xc, COSTPL[N].yc ) )
	{
		COSTPL[N].Access = false;
	}

	COSTPL[N].Transport = CheckTransportNearPoint( COSTPL[N].xw, COSTPL[N].yw );
}

void ClearCostPlaces()
{
	NCost = 0;
}

extern int RealLx;
extern int RealLy;
void DrawMetka( int x, int y, byte c )
{
	x <<= 4;
	y <<= 4;
	int xx = x - ( mapx << 5 );
	int yy = ( y >> 1 ) - ( mapy << 4 ) - GetHeight( x, y );
	if ( xx > -20 && yy > -20 && xx < RealLx + 20 && yy < RealLy + 20 )
	{
		DrawLine( xx - 20, yy - 10, xx + 20, yy + 10, c );
		DrawLine( xx - 20, yy + 10, xx + 20, yy - 10, c );
	};
};
void DrawNum( int x, int y, byte v )
{
	x <<= 4;
	y <<= 4;
	int xx = x - ( mapx << 5 );
	int yy = ( y >> 1 ) - ( mapy << 4 ) - GetHeight( x, y );
	if ( xx > -20 && yy > -20 && xx < RealLx + 20 && yy < RealLy + 20 )
	{
		char cc[16];
		sprintf( cc, "%d", v );
		ShowString( xx - 10, yy - 5, cc, &WhiteFont );
	};
};
void ShowCostPlaces()
{

};
//=============2.Desant============================//
class B_DiversionOrder :public BrigadeOrder
{
public:
	word IDX[80];
	word USN[80];
	int NU;
	int MaxU;
	int Phase;
	int LoadX;
	int LoadY;
	int CostPointIndex;
	int StartTime;
};
int FindCostPoint( int x, int y, byte Isl )
{
	if ( !NCost )return -1;
	int MinR = 100000;
	int BestPL = -1;
	for ( int i = 0; i < NCost; i++ )
	{
		int R = Norma( x - COSTPL[i].xw, y - COSTPL[i].yw );
		if ( R > 20 && R < MinR&&COSTPL[i].Access&&COSTPL[i].Island == Isl&&COSTPL[i].Transport == 0xFFFF )
		{
			MinR = R;
			BestPL = i;
		};
	};
	return BestPL;
};
int FindCostPointEx( int x, int y, byte Mask )
{
	if ( !NCost )return -1;
	int MinR = 100000;
	int BestPL = -1;
	byte ISLSET[32];
	memset( ISLSET, 0, 32 );
	for ( int i = 0; i < 8; i++ )if ( !( NATIONS[i].NMask&Mask ) )
	{
		int isl = CITY[i].MyIsland;
		if ( isl < 255 )
		{
			ISLSET[isl >> 3] |= 1 << ( isl & 7 );
		};
	};
	for ( int i = 0; i < NCost; i++ )
	{
		int R = Norma( x - COSTPL[i].xw, y - COSTPL[i].yw );
		if ( R > 20 && R < MinR&&COSTPL[i].Access&&COSTPL[i].Transport == 0xFFFF )
		{
			int ISL = COSTPL[i].Island;
			if ( ISLSET[ISL >> 3] & ( 1 << ( ISL & 7 ) ) )
			{
				MinR = R;
				BestPL = i;
			};
		};
	};
	return BestPL;
};
void LeaveAll( OneObject* OB );
void ArmyMakeBattleLink( AI_Army* ARM );
void MakeDiversionLink( Brigade* BR )
{
	if ( PeaceTimeLeft )return;
	City* CT = BR->CT;
	//CheckArmies(CT);
	B_DiversionOrder* DORD = (B_DiversionOrder*) BR->BOrder;
	int MaxInMy = 50;
	if ( IslPrs[CT->MyIsland] & 1 )MaxInMy = 150;
	if ( BR->NMemb )
	{
		OneObject* TRANS = Group[BR->Memb[0]];
		OneObject* CurTrans = NULL;
		if ( CT->TransportID != 0xFFFF )
		{
			CurTrans = Group[CT->TransportID];
			if ( !( CurTrans&&CurTrans->Serial == CT->TransportSN&&CurTrans->DstX > 0 ) )
			{
				CurTrans = NULL;
				CT->TransportID = 0xFFFF;
				CT->TransportSN = 0xFFFF;
			};
		};
		if ( TRANS && ( !TRANS->Sdoxlo ) && TRANS->Serial == BR->MembSN[0] )
		{
			if ( TRANS->StandTime > 200 && TRANS->DstX <= 0 )
			{
				TRANS->Die();
				return;
			};
			if ( TRANS->delay > 100 && TRANS->delay < 130 )
			{
				TRANS->Die();
				return;
			};
			if ( TRANS->DstX > 0 && DORD->Phase == 0 )
			{
				if ( DORD->NU < DORD->MaxU )
				{
					if ( CurTrans == TRANS )
					{
						int NAR = 0;
						for ( int i = 0; i < MaxArm; i++ )
						{
							AI_Army* AR = CT->ARMS + i;
							if ( AR->Enabled&&AR->AOrder&&AR->AOrder->ALink == &ArmyMakeBattleLink )
							{
								for ( int j = 0; j < AR->NExBrigs; j++ )
								{
									Brigade* BRG = AR->ExBrigs[j].Brig;
									NAR += BRG->NMemb;
								};
							};
						};
						OneObject* UNI = NULL;
						byte Unitype = 2;
						if ( NAR > MaxInMy )
						{
							do
							{
								for ( int i = 0; i < MaxArm; i++ )
								{
									AI_Army* AR = CT->ARMS + i;
									if ( AR->Enabled&&AR->AOrder&&AR->AOrder->ALink == &ArmyMakeBattleLink )
									{
										for ( int j = 0; j < AR->NExBrigs; j++ )
										{
											if ( AR->ExBrigs[j].BrigadeType == Unitype )
											{
												Brigade* BRG = AR->ExBrigs[j].Brig;
												if ( BRG->NMemb )
												{
													if ( BRG->Memb[0] != 0xFFFF )
													{
														OneObject* OB = Group[BRG->Memb[0]];
														if ( OB&&OB->Serial == BRG->MembSN[0] )
														{
															UNI = OB;
															goto UNIFOUND;
														};
													};
												};
											};
										};
									};
								};
								//0-Light infantry(short range)
								//1-infantry(strelok)
								//2-fasthorce
								//3-hardhorce
								//4-horse-strelok
								//5-mortira&pushka
								//6-grenader
								//7-peasant
								//8-weak units
								//9-supermortira
								switch ( Unitype )
								{
								case 2:
									Unitype = 3;
									break;
								case 3:
									Unitype = 5;
									break;
								case 5:
									Unitype = 0;
									break;
								case 0:
									Unitype = 1;
									break;
								case 1:
									Unitype = 4;
									break;
								case 4:
									Unitype = 6;
									break;
								case 6:
									Unitype = 0xFF;
									break;
								};
							} while ( Unitype != 0xFF );
						};
					UNIFOUND:
						if ( UNI )
						{
							CT->UnRegisterNewUnit( UNI );
							UNI->NewMonsterSmartSendTo( TRANS->DstX >> 4, TRANS->DstY >> 4, 0, 0, 128 + 16, 0 );
							DORD->IDX[DORD->NU] = UNI->Index;
							DORD->USN[DORD->NU] = UNI->Serial;
							DORD->NU++;
						};
					}
					else
					{
						if ( CurTrans == NULL )
						{
							CT->TransportID = TRANS->Index;
							CT->TransportSN = TRANS->Serial;
							CurTrans = TRANS;
						};
					};
				}
				else
				{
					if ( CurTrans == TRANS )
					{
						CT->TransportID = 0xFFFF;
						CT->TransportSN = 0xFFFF;
						CurTrans = NULL;
					};
					if ( TRANS->NInside >= DORD->MaxU )
					{
						DORD->Phase = 1;
						DORD->CostPointIndex = -1;
						TRANS->DstX = -1;
						TRANS->StandTime = 0;
					};
				};
				//conrol of loading
				int x0 = TRANS->DstX;
				int y0 = TRANS->DstY;
				for ( int q = 0; q < DORD->NU; q++ )
				{
					OneObject* OB = Group[DORD->IDX[q]];
					bool remove = false;
					if ( OB&&OB->Serial == DORD->USN[q] )
					{
						if ( !OB->Sdoxlo )
						{
							if ( !OB->LocalOrder )
							{
								if ( Norma( OB->RealX - x0, OB->RealY - y0 ) < 160 * 16 )
								{
									OB->GoToTransport( TRANS->Index, 128 + 16 );
								}
								else OB->NewMonsterSmartSendTo( x0 >> 4, y0 >> 4, 0, 0, 128 + 16, 0 );
							};
						};
						if ( OB->Sdoxlo && !OB->Hidden )remove = true;
					}
					else remove = true;
					if ( remove )
					{
						if ( q < DORD->NU - 1 )
						{
							memcpy( DORD->IDX + q, DORD->IDX + q + 1, ( DORD->NU - q - 1 ) << 1 );
							memcpy( DORD->USN + q, DORD->USN + q + 1, ( DORD->NU - q - 1 ) << 1 );
						};
						q--;
						DORD->NU--;
					};
				};
				if ( TRANS->NInside )
				{
					if ( DORD->StartTime > tmtmt )
					{
						DORD->StartTime = tmtmt;
					}
					else
					{
						if ( tmtmt - DORD->StartTime > 5000 )
						{
							for ( int q = 0; q < DORD->NU; q++ )
							{
								OneObject* OB = Group[DORD->IDX[q]];
								bool remove = false;
								if ( OB&&OB->Serial == DORD->USN[q] )
								{
									if ( !OB->Sdoxlo )
									{
										CT->RegisterNewUnit( OB );
										if ( q < DORD->NU - 1 )
										{
											memcpy( DORD->IDX + q, DORD->IDX + q + 1, ( DORD->NU - q - 1 ) << 1 );
											memcpy( DORD->USN + q, DORD->USN + q + 1, ( DORD->NU - q - 1 ) << 1 );
										};
										q--;
										DORD->NU--;
									};
								};
							};
							DORD->MaxU = DORD->NU;
							DORD->StartTime = tmtmt;
						};
					};
				};
			}
			else
			{
				if ( TRANS->DstX <= 0 && DORD->Phase == 0 )
				{
					//goto my cost
					if ( DORD->CostPointIndex == -1 )
					{
						DORD->CostPointIndex = FindCostPoint( TRANS->RealX >> 8, TRANS->RealY >> 8, CT->MyIsland );
						if ( DORD->CostPointIndex != -1 )
						{
							CostPlace* CP = COSTPL + DORD->CostPointIndex;
							TRANS->NewMonsterSmartSendTo( CP->xw << 4, CP->yw << 4, 0, 0, 128 + 16, 0 );
						};
					};
					if ( DORD->CostPointIndex != -1 )
					{
						CostPlace* CP = COSTPL + DORD->CostPointIndex;
						if ( ( !CP->Access ) || ( CP->Transport != 0xFFFF && CP->Transport != TRANS->Index ) )DORD->CostPointIndex = -1;
						else
						{
							if ( !TRANS->LocalOrder )
							{
								if ( Norma( ( TRANS->RealX >> 8 ) - CP->xw, ( TRANS->RealY >> 8 ) - CP->yw ) < 18 )
								{
									TRANS->NewMonsterSmartSendTo( CP->xc << 4, CP->yc << 4, 0, 0, 128 + 16, 0 );
								}
								else TRANS->NewMonsterSmartSendTo( CP->xw << 4, CP->yw << 4, 0, 0, 128 + 16, 0 );
							};

						};
					};
				};
				if ( DORD->Phase == 1 && TRANS->DstX <= 0 )
				{
					if ( rando() < 512 )DORD->CostPointIndex = -1;
					if ( DORD->CostPointIndex == -1 )
					{
						DORD->CostPointIndex = FindCostPointEx( TRANS->RealX >> 8, TRANS->RealY >> 8, 1 << CT->NI );
						if ( DORD->CostPointIndex != -1 )
						{
							CostPlace* CP = COSTPL + DORD->CostPointIndex;
							TRANS->NewMonsterSmartSendTo( CP->xw << 4, CP->yw << 4, 0, 0, 128 + 16, 0 );
						};
					};
					if ( DORD->CostPointIndex != -1 )
					{
						CostPlace* CP = COSTPL + DORD->CostPointIndex;
						if ( ( !CP->Access ) || ( CP->Transport != 0xFFFF && CP->Transport != TRANS->Index ) )DORD->CostPointIndex = -1;
						else
						{
							int NN = Norma( ( TRANS->RealX >> 8 ) - CP->xw, ( TRANS->RealY >> 8 ) - CP->yw );
							if ( NN < 40 || !TRANS->LocalOrder )
							{
								if ( NN < 40 )
								{
									if ( !TRANS->LocalOrder )TRANS->NewMonsterSmartSendTo( CP->xc << 4, CP->yc << 4, 0, 0, 128 + 16, 0 );
								}
								else TRANS->NewMonsterSmartSendTo( CP->xw << 4, CP->yw << 4, 0, 0, 128 + 16, 0 );
							};

						};
					};
				};
				if ( DORD->Phase == 1 && TRANS->DstX > 0 )
				{
					if ( TRANS->NInside && !TRANS->LocalOrder )
					{
						LeaveAll( TRANS );
					};
					//attempt to register units
					int NUN = 0;
					for ( int i = 0; i < DORD->NU; i++ )
					{
						OneObject* OB = Group[DORD->IDX[i]];
						if ( OB&&OB->Serial == DORD->USN[i] && OB->BrigadeID == 0xFFFF )
						{
							if ( OB->Hidden || OB->UnlimitedMotion )
							{
								NUN++;
							}
							else
							{
								//CheckArmies(CT);
								if ( !OB->Sdoxlo )CT->Settlers.AddObject( OB );
								//CheckArmies(CT);
							};
						};
					};
					if ( !( TRANS->NInside || NUN ) )
					{
						TRANS->DstX = -1;
						DORD->Phase = 0;
						DORD->CostPointIndex = -1;
						DORD->NU = 0;
						//CT->Brigs[TRANS->BrigadeID].DeleteAll();
						//CT->Brigs[TRANS->BrigadeID].Enabled=false;
						TRANS->delay = 350;
						TRANS->StandTime = 0;
					};
				};

			};
		}
		else
		{
			BR->NMemb = 0;
		};
	}
	else
	{
		BR->DeleteAll();
		BR->Enabled = false;
	};
};
void MakeDiversion( Brigade* BR )
{
	B_DiversionOrder* DOR = (B_DiversionOrder*) BR->CreateOrder( 0, sizeof B_DiversionOrder );
	BR->BOrder = DOR;
	DOR->Size = sizeof B_DiversionOrder;
	DOR->NU = 0;
	DOR->MaxU = 80;
	DOR->Phase = 0;
	DOR->BLink = &MakeDiversionLink;
	DOR->LoadX = -1;
	DOR->LoadY = -1;
	DOR->CostPointIndex = -1;
	DOR->StartTime = 10000000;
};
//-----------------------------------NEW!!! Formations for AI!!!------------------------//
void CalculateFreeUnits( AI_Army* AIR );

void SearchArmyLink( OneObject* OBJ )
{
	word ArmID = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	bool OFC = OBJ->Ref.General->OFCR != NULL;
	if ( ArmID == 0xFFFF )
	{
		int Aid = -1;
		//need to find nearest appropriate army

		int MyTop = GetTopology( OBJ->RealX >> 4, OBJ->RealY >> 4 );
		if ( MyTop >= 0xFFFE )return;
		int MMTop = MyTop*NAreas;
		City* CT = CITY + OBJ->NNUM;
		int MinR = 0xFFFE;
		int id = -1;
		for ( int i = 0; i < MaxArm; i++ )
		{
			AI_Army* ARM = CT->ARMS + i;
			if ( ARM->Enabled&&ARM->TopPos < 0xFFFE )
			{
				if ( ( OFC&&ARM->NFreeUnits / 36 > ARM->NCom&&ARM->NCom < 8 ) || ( ( !OFC ) && ARM->NFreeUnits / 36 > ARM->NBar&&ARM->NBar < 8 ) )
				{
					int topa = ARM->TopPos;
					int dr = 0;
					if ( topa != MyTop )dr = LinksDist[topa + MMTop];
					if ( dr < MinR )
					{
						MinR = dr;
						Aid = i;
					};
				};
			};
		};
		if ( Aid != -1 )
		{
			AI_Army* ARM = CT->ARMS + Aid;
			if ( OFC )
			{
				ARM->ComID[ARM->NCom] = OBJ->Index;
				ARM->ComSN[ARM->NCom] = OBJ->Serial;
				ARM->NCom++;
			}
			else
			{
				ARM->BarID[ARM->NBar] = OBJ->Index;
				ARM->BarSN[ARM->NBar] = OBJ->Serial;
				ARM->NBar++;
			};
			OBJ->LocalOrder->info.BuildObj.ObjIndex = Aid;
		};
		return;
	}
	else
	{
		AI_Army* ARM = CITY[OBJ->NNUM].ARMS + ArmID;
		if ( OFC )
		{
			bool pr = 1;
			for ( int i = 0; i < ARM->NCom&&pr; i++ )if ( ARM->ComID[i] == OBJ->Index )pr = 0;
			if ( pr )
			{
				OBJ->LocalOrder->info.BuildObj.ObjIndex = 0xFFFF;
				return;
			};
		}
		else
		{
			bool pr = 1;
			for ( int i = 0; i < ARM->NBar&&pr; i++ )if ( ARM->BarID[i] == OBJ->Index )pr = 0;
			if ( pr )
			{
				OBJ->LocalOrder->info.BuildObj.ObjIndex = 0xFFFF;
				return;
			};
		};
		if ( ( OFC&&ARM->NFreeUnits / 36 > ARM->NCom ) || ( ( !OFC ) && ARM->NFreeUnits / 36 > ARM->NBar ) )
		{
			word* CID;
			word* CSN;
			int CN;
			if ( OFC )
			{
				CID = ARM->ComID;
				CSN = ARM->ComSN;
				CN = ARM->NCom;
			}
			else
			{
				CID = ARM->BarID;
				CSN = ARM->BarSN;
				CN = ARM->NBar;
			};
			//search farest commander/bar..
			int maxr = -1;
			int comid = 0xFFFF;
			int cps = 0;
			int top0 = ARM->TopPos;
			if ( top0 >= 0xFFFE )return;
			for ( int i = 0; i < CN; i++ )
			{
				OneObject* OB = Group[CID[i]];
				if ( OB&&OB->Serial == CSN[i] && !OB->Sdoxlo )
				{
					int top1 = GetTopology( OB->RealX >> 4, OB->RealY >> 4 );
					int dd = 0xFFFF;
					if ( top1 < 0xFFFE )dd = top1 == top0 ? 0 : LinksDist[top1*NAreas + top0];
					if ( dd > maxr )
					{
						maxr = dd;
						comid = OB->Index;
						cps = i;
					};
				};
			};
			if ( comid == OBJ->Index )
			{
				//delete ofcr from army!
				CSN[cps]++;
				CalculateFreeUnits( ARM );
				OBJ->LocalOrder->info.BuildObj.ObjIndex = 0xFFFF;
				return;
			};
		}
		else
		{

		};
		//we need to go to the army
		int top0 = ARM->TopPos;
		if ( top0 >= 0xFFFE )return;
		int top1 = GetTopology( OBJ->RealX >> 4, OBJ->RealY >> 4 );
		if ( top1 >= 0xFFFE )return;
		if ( top1 != top0 )
		{
			top1 = MotionLinks[top0 + top1*NAreas];
			Area* AR = TopMap + top1;
			int x1 = int( AR->x ) << 6;
			int y1 = int( AR->y ) << 6;
			int x0 = OBJ->RealX >> 4;
			int y0 = OBJ->RealY >> 4;
			OBJ->NewState = 0;
			OBJ->GroundState = 0;
			if ( Norma( x0 - x1, y0 - y1 ) > 250 )
			{
				OBJ->CreatePath( x1 >> 4, y1 >> 4 );
			}
			else
			{
				OBJ->DestX = 0;
				OBJ->DestY = 0;
			};
		}
		else
		{
			//need create formation now!
			if ( OFC )
			{
				//need to check barabanschiks...
				word Bar = 0xFFFF;
				int bpos = -1;
				for ( int i = 0; i < ARM->NBar; i++ )
				{
					OneObject* BOB = Group[ARM->BarID[i]];
					if ( BOB&&BOB->Serial == ARM->BarSN[i] && !BOB->Sdoxlo )
					{
						int top1 = GetTopology( BOB->RealX >> 4, BOB->RealY >> 4 );
						if ( top1 == top0 )
						{
							bpos = i;
							Bar = BOB->Index;
						};
					};
				};
				if ( Bar != 0xFFFF )
				{
					word FormUID = NATIONS[ARM->CT->NI].FormUnitID;
					//now we are ready to create formation!
					//1.Search the biggest brigade
					int BID = -1;
					int MaxU = 0;
					int N = ARM->NExBrigs;
					for ( int i = 0; i < N; i++ )
					{
						Brigade* BR = ARM->ExBrigs[i].Brig;
						if ( !BR->WarType )
						{
							int Nu = BR->NMemb;
							word* mem = BR->Memb;
							word* msn = BR->MembSN;
							int NFU = 0;
							for ( int j = 0; j < Nu; j++ )
							{
								word mid = mem[j];
								if ( mid != 0xFFFF )
								{
									OneObject* OB = Group[mid];
									if ( OB&&OB->Serial == msn[j] && ( !OB->Sdoxlo ) && OB->NIndex == FormUID )NFU++;
								};
							};
							if ( NFU > MaxU )
							{
								MaxU = NFU;
								BID = i;
							};
						};
					};
					if ( BID != -1 )
					{
						Brigade* FBR = ARM->ExBrigs[BID].Brig;
						FBR->WarType = 0xFF;
						//let us exclude other members
						int N = FBR->NMemb;
						word* mem = FBR->Memb;
						word* msn = FBR->MembSN;
						word TrueMem[38];
						int  NTrue = 0;
						City* CT = ARM->CT;
						int id = CT->GetFreeBrigade();
						if ( id == -1 )
						{
							FBR->WarType = 0;
							return;
						};
						Brigade* OTB = CT->Brigs + id;
						OTB->Enabled = 1;
						for ( int i = 0; i < N; i++ )
						{
							word mid = mem[i];
							if ( mid != 0xFFFF )
							{
								OneObject* OB = Group[mid];
								if ( OB && ( OB->Serial == msn[i] ) && ( !OB->Sdoxlo ) )
								{
									if ( OB->NIndex != FormUID || NTrue >= 36 )
									{
										FBR->RemoveOne( i, OTB );
										i--;
										N--;
									}
									else
									{
										TrueMem[NTrue + 2] = OB->Index;
										NTrue++;
									};
								};
							};
						};
						if ( OTB->NMemb )
						{
							ARM->AddBrigade( OTB );
						};
						if ( NTrue < 36 )
						{
							//not enough units to create formaton.Need to collect
							int Need = 36 - NTrue;
							int NB = ARM->NExBrigs;
							for ( int i = 0; i < NB&&Need; i++ )
							{
								Brigade* MBR = ARM->ExBrigs[i].Brig;
								if ( !MBR->WarType )
								{
									word* mem = MBR->Memb;
									word* msn = MBR->MembSN;
									int nm = MBR->NMemb;
									for ( int j = 0; j < nm&&Need; j++ )
									{
										word mid = mem[j];
										if ( mid != 0xFFFF )
										{
											OneObject* OB = Group[mid];
											if ( OB && ( !OB->Sdoxlo ) && OB->Serial == msn[j] && OB->NIndex == FormUID )
											{
												MBR->RemoveOne( j, OTB );
												Need--;
												j--;
											};
										};
									};
								};
							};
							OTB->RemoveObjects( OTB->NMemb, FBR );
						};
						NTrue = 0;
						for ( int i = 0; i < FBR->NMemb; i++ )
						{
							word mid = FBR->Memb[i];
							if ( mid != 0xFFFF )
							{
								OneObject* OB = Group[mid];
								if ( OB && ( !OB->Sdoxlo ) && OB->Serial == FBR->MembSN[i] )
								{
									TrueMem[NTrue + 2] = OB->Index;
									NTrue++;
								};
							};
						};
						TrueMem[0] = OBJ->Index;
						TrueMem[1] = Bar;
						NTrue += 2;
						FBR->Memb = (word*) realloc( FBR->Memb, 38 * 2 );
						FBR->MembSN = (word*) realloc( FBR->MembSN, 38 * 2 );
						memset( FBR->Memb, 0xFF, 38 * 2 );
						memset( FBR->MembSN, 0xFF, 38 * 2 );
						FBR->NMemb = 38;
						FBR->MaxMemb = 38;
						int cidx = -1;
						for ( int p = 0; p < NEOrders&&cidx == -1; p++ )
						{
							if ( !strcmp( ElementaryOrders[p].ID, "#SQUARE36" ) )cidx = p;
						};
						if ( cidx == -1 )
						{
							MessageBox( NULL, "Could not find fomation: #SQUARE36", "ERROR!", 0 );
							//assert(cidx);
						};
						OrderDescription* ODS = ElementaryOrders + cidx;
						int AddShield = ODS->AddShield2;
						int AddDamage = ODS->AddDamage2;
						FBR->AddDamage = AddDamage;
						FBR->AddShield = AddShield;
						int ncr = 0;
						int xs = 0;
						int ys = 0;
						for ( int i = 0; i < NTrue; i++ )
						{
							OneObject* OB = Group[TrueMem[i]];
							FBR->Memb[i] = TrueMem[i];
							FBR->MembSN[i] = OB->Serial;
							OB->BrigadeID = FBR->ID;
							OB->BrIndex = i;
							OB->AddShield = AddShield;
							OB->AddDamage = AddDamage;
							OB->InArmy = 1;
							OB->Zombi = 1;
							OB->DoNotCall = 1;
							OB->NoBuilder = 1;
							if ( i > 1 )
							{
								xs += OB->RealX >> 4;
								ys += OB->RealY >> 4;
								ncr++;
							};
						};
						xs /= ncr;
						ys /= ncr;
						OBJ->ClearOrders();
						Group[Bar]->ClearOrders();
						FBR->NMemb = 38;
						FBR->ArmyID = ArmID;
						FBR->PosCreated = false;
						FBR->WarType = cidx + 1;
						FBR->MembID = FormUID;
						FBR->CreateOrderedPositions( xs << 4, ys << 4, 0 );
						FBR->KeepPositions( 0, 128 + 16 );
						OTB->DeleteAll();
						OTB->Enabled = false;
						for ( int i = 0; i < ARM->NCom; i++ )
						{
							if ( ARM->ComID[i] == OBJ->Index )ARM->ComSN[i]++;
						};
						for ( int i = 0; i < ARM->NBar; i++ )
						{
							if ( ARM->BarID[i] == Bar )ARM->BarSN[i]++;
						};
						CalculateFreeUnits( ARM );
						return;
					};
				};
			};
			Area* AR = TopMap + top1;
			int x1 = int( AR->x ) << 6;
			int y1 = int( AR->y ) << 6;
			int x0 = OBJ->RealX >> 4;
			int y0 = OBJ->RealY >> 4;
			OBJ->NewState = 0;
			OBJ->GroundState = 0;
			if ( Norma( x0 - x1, y0 - y1 ) > 250 )
			{
				OBJ->CreatePath( x1 >> 4, y1 >> 4 );
			}
			else
			{
				OBJ->DestX = -1;
				OBJ->DestY = -1;
			};
		};
	};
};
void SearchArmy( OneObject* OB )
{
	Order1* OR1 = OB->CreateOrder( 0 );
	OR1->info.BuildObj.ObjIndex = 0xFFFF;
	OR1->DoLink = &SearchArmyLink;
	if ( OB->newMons->Officer )
	{
		OB->PrioryLevel = 0;

		OB->NoSearchVictim = 1;

	}
	else
	{
		OB->PrioryLevel = 16;

		OB->NoSearchVictim = 1;

	};
	OR1->PrioryLevel = 0;
};
void CalculateFreeUnits( AI_Army* AIR )
{
	AIR->NFreeUnits = 0;
	word FormUID = NATIONS[AIR->CT->NI].FormUnitID;
	for ( int i = 0; i < AIR->NExBrigs; i++ )
	{
		Brigade* BR = AIR->ExBrigs[i].Brig;
		if ( ( !BR->WarType ) && BR->NMemb )
		{
			word* Mem = BR->Memb;
			word* SN = BR->MembSN;
			int N = BR->NMemb;
			for ( int i = 0; i < N; i++ )
			{
				word MID = Mem[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB&&OB->Serial == SN[i] && !OB->Sdoxlo )
					{
						if ( OB->NIndex == FormUID )AIR->NFreeUnits++;
					};
				};
			};
		};
	};
	//handle officers
	int id = AIR->ArmyID;
	for ( int i = 0; i < AIR->NCom; i++ )
	{
		OneObject* OB = Group[AIR->ComID[i]];
		bool remove = false;
		if ( OB )
		{
			if ( !( OB->Serial == AIR->ComSN[i] && !OB->Sdoxlo ) )remove = 1;
			else
			{
				Order1* OR1 = OB->LocalOrder;
				remove = true;
				while ( OR1 )
				{
					if ( OR1->DoLink == &SearchArmyLink )
					{
						if ( OR1->info.BuildObj.ObjIndex == id )remove = false;
						OR1 = NULL;
					}
					else OR1 = OR1->NextOrder;
				};
			};
		};
		if ( remove )
		{
			if ( i < AIR->NCom - 1 )
			{
				word NCp = ( AIR->NCom - i - 1 ) << 1;
				memcpy( AIR->ComID + i, AIR->ComID + i + 1, NCp );
				memcpy( AIR->ComSN + i, AIR->ComSN + i + 1, NCp );
			};
			i--;
			AIR->NCom--;
		};
	};
	for ( int i = 0; i < AIR->NBar; i++ )
	{
		OneObject* OB = Group[AIR->BarID[i]];
		bool remove = false;
		if ( OB )
		{
			if ( !( OB->Serial == AIR->BarSN[i] && !OB->Sdoxlo ) )remove = 1;
			else
			{
				Order1* OR1 = OB->LocalOrder;
				remove = true;
				while ( OR1 )
				{
					if ( OR1->DoLink == &SearchArmyLink )
					{
						if ( OR1->info.BuildObj.ObjIndex == id )remove = false;
						OR1 = NULL;
					}
					else OR1 = OR1->NextOrder;
				};
			};
		};
		if ( remove )
		{
			if ( i < AIR->NBar - 1 )
			{
				word NCp = ( AIR->NBar - i - 1 ) << 1;
				memcpy( AIR->BarID + i, AIR->BarID + i + 1, NCp );
				memcpy( AIR->BarSN + i, AIR->BarSN + i + 1, NCp );
			};
			i--;
			AIR->NBar--;
		};
	};
};
