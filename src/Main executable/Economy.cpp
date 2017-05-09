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
#include "Dialogs.h"
#include "fonts.h"
#include "dpchat.h"
#include "dplobby.h"
#include "GSound.h"
#include "3DGraph.h"
#include "3DMapEd.h"
#include "MapSprites.h"
#include <assert.h>
#include <math.h>
#include "NewMon.h"
#include "IconTool.h"
#include "GP_Draw.h"
#include "3DRandMap.h"
#include <crtdbg.h>
#include "ConstStr.h"
#include "DrawForm.h"

class Economy
{
public:
	int EcResID[6];
	int EcResIcn[6];
	int ChertaIcn;
	int BuyIcn;
	int ClearIcn;
	int P10Icn;
	int P100Icn;
	int P1000Icn;
	int P10000Icn;
	int M10Icn;
	int M100Icn;
	int M1000Icn;
	int M10000Icn;

	//Market exchange rates
	//Beware: Synchronization critical!
	double ResCostBuy[8][6];//Current cost
	double ResCostSell[8][6];//Current cost
	double ResCostBuy0[8][6];//Default cost?
	double ResCostSell0[8][6];//Default cost?
	double ResCostBuy1[8][6];//Nominal cost
	double ResCostSell1[8][6];//Nominal cost
	double ResCostBuy2[8][6];//Default cost?
	double ResCostSell2[8][6];//Default cost?

	//Save default market exchange rates for dllexport function
	double buy_rates_for_bots[6];
	double sell_rates_for_bots[6];

	//Local exchange rates for 'Independenet Rates' option
	double local_ResCostBuy[6];
	double local_ResCostSell[6];

	double ExpBUY[6];
	double ExpSELL[6];
	double TimeBUY[6];
	double TimeSELL[6];

	int NeedToBuy;
	int NeedToSell;
	int ResSell;
	int ResBuy;

	void Load();
	void InitInterface();
	void CreateInterface( IconSet* ISET );
	void PerformOperation( byte Nation, byte SellRes, byte BuyRes, int SellAmount );
	void Process();
};

void EERR()
{
	ErrM( "ECONOMY.TXT is corrupted." );
}

int GetIconByName( char* Name );

int GetResID( char* );

void Economy::Load()
{
	GFILE* F = Gopen( "economy.txt", "r" );
	if (F)
	{
		char cc[128];
		int z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		ClearIcn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		ChertaIcn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		BuyIcn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		P10Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		M10Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		P100Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		M100Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		P1000Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}
		M1000Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		P10000Icn = GetIconByName( cc );
		z = Gscanf( F, "%s", cc );
		if (z != 1)
		{
			EERR();
		}

		M10000Icn = GetIconByName( cc );
		for (int i = 0; i < 6; i++)
		{//Iterate through each resource
			z = Gscanf( F, "%s", cc );
			if (z != 1)EERR();
			float v1, v2;
			EcResID[i] = GetResID( cc );
			EcResIcn[i] = GetIconByName( cc );
			float x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
			z = Gscanf( F, "%s%g%g%g%s%g%g%s%g%s%g%g%g%s%g%g%s%g",
				cc, &x1, &x2, &x3,
				cc, &v1, &x4, cc, &x5,
				cc, &x6, &x7, &x8,
				cc, &v2, &x9, cc, &x10 );

			if (z != 18)
			{
				EERR();
			}


			buy_rates_for_bots[i] = x2;
			sell_rates_for_bots[i] = x7;

			for (int j = 0; j < 8; j++)
			{//Iterate through each nation array
				ResCostBuy0[j][i] = x1;
				ResCostBuy1[j][i] = x2;
				ResCostBuy2[j][i] = x3;
				ResCostSell0[j][i] = x6;
				ResCostSell1[j][i] = x7;
				ResCostSell2[j][i] = x8;
				ResCostBuy[j][i] = ResCostBuy1[j][i];
				ResCostSell[j][i] = ResCostSell1[j][i];
			}
			ExpBUY[i] = x4;
			TimeBUY[i] = x5;
			ExpSELL[i] = x9;
			TimeSELL[i] = x10;
			ExpBUY[i] /= v1 * 100;
			ExpSELL[i] /= v2 * 100;
		}
		Gclose( F );
	}
	else
	{
		ErrM( "Could not open ECONOMY.TXT" );
	}

	InitInterface();
}

Economy ECO;

void ReloadECO()
{
	ECO.Load();
}

void Economy::InitInterface()
{
	NeedToBuy = 0;
	NeedToSell = 0;
	ResSell = -1;
	ResBuy = -1;
}

void SelectBUY( int i )
{
	ECO.ResBuy = i;
}

void CmdTorg( byte NI, byte SellRes, byte BuyRes, int SellAmount );

void SelectBUYBUTTON( int i )
{
	if (ECO.ResSell != -1 && ECO.ResBuy != -1)
	{
		CmdTorg( NatRefTBL[MyNation], ECO.ResSell, ECO.ResBuy, ECO.NeedToSell );
	}
}

void SelectSELL( int i )
{
	ECO.ResSell = i;
}

void SelectCLEAR( int i )
{
	ECO.NeedToBuy = 0;
	ECO.NeedToSell = 0;
}

void SelectCHANGE( int i )
{
	if (GetKeyState( VK_SHIFT ) & 0x8000)
	{
		ECO.NeedToSell += i * 100;
	}
	else
	{
		ECO.NeedToSell += i;
	}

	if (ECO.NeedToSell < 0)
	{
		ECO.NeedToSell = 0;
	}
}

extern int PrpX;
extern int PrpY;
extern int IconLx;
extern int IconLy;
extern bool ECOSHOW;

extern byte MarketState;

//Composes and shows the UI for market exchanges depending on MarketState
void Economy::CreateInterface( IconSet* ISET )
{
	if (1 == MarketState)
	{//Market Option is set to 'No Exchange'
		return;
	}

	for (int i = 0; i < 6; i++)
	{
		int ix = i % 2;
		int iy = i >> 1;

		OneIcon* IC = ISET->AddIconFixed( 0, EcResIcn[i], ix + iy * 12 );

		if (ResSell == i)
		{
			IC->SelectIcon();
		}

		IC->AssignLeft( &SelectSELL, i );
		IC->CreateHint( RDS[EcResID[i]].Name );
		IC = ISET->AddIconFixed( 0, EcResIcn[i], ix + 7 + iy * 12 );

		if (ResBuy == i)
		{
			IC->SelectIcon();
		}

		IC->AssignLeft( &SelectBUY, i );
		IC->CreateHint( RDS[EcResID[i]].Name );
	}

	OneIcon* M10 = ISET->AddIconFixed( 0, M10Icn, 2 );
	M10->AssignLeft( &SelectCHANGE, -10 );
	M10->CreateHint( ECONOM1 );//"-10 units"

	OneIcon* P10 = ISET->AddIconFixed( 0, P10Icn, 3 );
	P10->AssignLeft( &SelectCHANGE, 10 );
	P10->CreateHint( ECONOM2 );//"+10 units"

	OneIcon* M1000 = ISET->AddIconFixed( 0, M1000Icn, 5 );
	M1000->AssignLeft( &SelectCHANGE, -1000 );
	M1000->CreateHint( ECONOM3 );//"-1000 units"

	OneIcon* P1000 = ISET->AddIconFixed( 0, P1000Icn, 6 );
	P1000->AssignLeft( &SelectCHANGE, 1000 );
	P1000->CreateHint( ECONOM4 );//"+1000 units"

	OneIcon* BUY = ISET->AddIconFixed( 0, BuyIcn, 4 );
	BUY->AssignLeft( &SelectBUYBUTTON, 0 );
	BUY->CreateHint( ECONOM5 );//"Buy resource"

	OneIcon* M100 = ISET->AddIconFixed( 0, M100Icn, 2 + 24 );
	M100->AssignLeft( &SelectCHANGE, -100 );
	M100->CreateHint( ECONOM6 );//"-100 units"

	OneIcon* P100 = ISET->AddIconFixed( 0, P100Icn, 3 + 24 );
	P100->AssignLeft( &SelectCHANGE, 100 );
	P100->CreateHint( ECONOM7 );//"+100 units"

	OneIcon* CLEAR = ISET->AddIconFixed( 0, ClearIcn, 4 + 24 );
	CLEAR->AssignLeft( &SelectCLEAR, 0 );
	CLEAR->CreateHint( ECONOM8 );//"Clear"

	OneIcon* M10000 = ISET->AddIconFixed( 0, M10000Icn, 5 + 24 );
	M10000->AssignLeft( &SelectCHANGE, -10000 );
	M10000->CreateHint( ECONOM9 );//"-10000 units"

	OneIcon* P10000 = ISET->AddIconFixed( 0, P10000Icn, 6 + 24 );
	P10000->AssignLeft( &SelectCHANGE, 10000 );
	P10000->CreateHint( ECONOMA );//"+10000 units"

	char cc[128];
	if (ResSell != -1)
	{
		sprintf( cc, "%d", NeedToSell );
		int L = GetRLCStrWidth( cc, &WhiteFont );
		ShowString( PrpX + ( IconLx + 2 ) * 3 - ( L >> 1 ), PrpY + IconLy + 16, cc, &WhiteFont );
	}

	if (ResSell != -1 && ResBuy != -1)
	{
		NeedToBuy = int( double( NeedToSell )*ResCostSell[MyNation][ResSell] / ResCostBuy[MyNation][ResBuy] );
		sprintf( cc, "%d", NeedToBuy );
		int L = GetRLCStrWidth( cc, &WhiteFont );
		ShowString( PrpX + ( IconLx + 2 ) * 6 - ( L >> 1 ), PrpY + IconLy + 16, cc, &WhiteFont );
	}

	ECOSHOW = 1;
}

void EconomyInterface( IconSet* ISET )
{
	ECO.CreateInterface( ISET );
}

void EconomePostDraw()
{
	GPS.ShowGP( PrpX - 12, PrpY - 31, BordGP, 88, 0 );
	GPS.ShowGP( PrpX + 283, PrpY - 31, BordGP, 89, 0 );

	int L = GetRLCStrWidth( ECOSELL, &SmallYellowFont );//"SELL"
	ShowString( PrpX + ( IconLx + 2 ) - ( L >> 1 ), PrpY - 21, ECOSELL, &SmallYellowFont );//"SELL"

	L = GetRLCStrWidth( ECOBUY, &SmallYellowFont );//"BUY"
	ShowString( PrpX + ( IconLx + 2 ) * 8 - ( L >> 1 ), PrpY - 21, ECOBUY, &SmallYellowFont );//"BUY"
}

void ClearEconomy()
{
	ECO.InitInterface();
}

void LoadEconomy()
{
	ECO.Load();
}

//Exchange resources using players own exchange rate
//Reassign other players exchange rates, if the market is global
void Economy::PerformOperation( byte Nation, byte SellRes, byte BuyRes, int SellAmount )
{
	if (5 < SellRes || 5 < BuyRes || 0 > SellAmount)
	{//Sanity check
		return;
	}

	int BuyID = EcResID[BuyRes];
	int SellID = EcResID[SellRes];

	if (XRESRC( Nation, SellID ) < SellAmount)
	{
		SellAmount = XRESRC( Nation, SellID );
	}

	int BuyAmount = int( double( SellAmount )*ResCostSell[Nation][SellRes] / ResCostBuy[Nation][BuyRes] );

	AddXRESRC( Nation, SellID, -SellAmount );
	AddXRESRC( Nation, BuyID, BuyAmount );

	NATIONS[Nation].ResBuy[BuyID] += BuyAmount;
	NATIONS[Nation].ResSell[SellID] += SellAmount;

	double Ves = double( BuyAmount ) * ExpBUY[BuyRes];
	ResCostBuy[Nation][BuyRes] = ( ResCostBuy[Nation][BuyRes] + ResCostBuy2[Nation][BuyRes] * Ves ) / ( 1 + Ves );
	ResCostSell[Nation][BuyRes] = ( ResCostSell[Nation][BuyRes] + ResCostSell2[Nation][BuyRes] * Ves ) / ( 1 + Ves );
	Ves = double( SellAmount ) * ExpSELL[SellRes];
	ResCostSell[Nation][SellRes] = ( ResCostSell[Nation][SellRes] + ResCostSell0[Nation][SellRes] * Ves ) / ( 1 + Ves );
	ResCostBuy[Nation][SellRes] = ( ResCostBuy[Nation][SellRes] + ResCostBuy0[Nation][SellRes] * Ves ) / ( 1 + Ves );

	if (2 != MarketState)
	{//Market Option 'Independent Exchange Rates' NOT selected
		for (int i = 0; i < 8; i++)
		{//Assign calculated exchange rates to all players
			ResCostBuy[i][BuyRes] = ResCostBuy[Nation][BuyRes];
			ResCostSell[i][BuyRes] = ResCostSell[Nation][BuyRes];
			ResCostSell[i][SellRes] = ResCostSell[Nation][SellRes];
			ResCostBuy[i][SellRes] = ResCostBuy[Nation][SellRes];
		}
	}
}

//Calculates exchange result for dllexport (bots AI dll)
//Always use player #0 local exchange rates for transactions
//Else there will be unsync when bots exchange
int GetTorgResultEx( byte SellRes, byte BuyRes, int SellAmount )
{
	int BuyID = ECO.EcResID[BuyRes];
	int SellID = ECO.EcResID[SellRes];

	return int( double( SellAmount )*ECO.sell_rates_for_bots[SellRes] / ECO.buy_rates_for_bots[BuyRes] );
}

void PerformTorg( byte Nation, byte SellRes, byte BuyRes, int SellAmount )
{
	ECO.PerformOperation( Nation, SellRes, BuyRes, SellAmount );
}

//Balances out market exchange rates over time
void Economy::Process()
{
	for (int i = 0; i < 8; i++)
	{//For each nation's exchange array
		for (int j = 0; j < 6; j++)
		{//Each resource in array
			if (ResCostSell[i][j] > ResCostSell1[i][j])
			{
				ResCostSell[i][j] -= TimeSELL[j];
			}
			if (ResCostBuy[i][j] > ResCostBuy1[i][j])
			{
				ResCostBuy[i][j] -= TimeBUY[j];
			}
			if (ResCostSell[i][j] < ResCostSell1[i][j])
			{
				ResCostSell[i][j] += TimeSELL[j];
			}
			if (ResCostBuy[i][j] < ResCostBuy1[i][j])
			{
				ResCostBuy[i][j] += TimeBUY[j];
			}
		}
	}
}

//Balances out market exchange rates over time
void ProcessEconomy()
{
	ECO.Process();
}

extern byte INVECO[6];//={2,5,4,0,1,3};

int GetFinPower( int* Fin, int Nation )
{
	int FP = 0;
	for (int i = 0; i < 6; i++)
	{
		FP += int( ECO.ResCostSell1[Nation][i] * Fin[INVECO[i]] );
	}
	return FP;
}

//Calculates exchange rate for the game market information popup
int GetCurrentCost( byte Buy, byte Sell, int Amount )
{
	return int( ( Amount*ECO.ResCostSell[MyNation][Sell] ) / ECO.ResCostBuy[MyNation][Buy] );
}

//Calculates exchange rate for the game market information popup
int GetNominalCost( byte Buy, byte Sell, int Amount )
{
	return int( ( Amount*ECO.ResCostSell1[MyNation][Sell] ) / ECO.ResCostBuy1[MyNation][Buy] );
}

int GetEconomyData( byte** EC )
{
	*EC = (byte*) &ECO;
	return sizeof ECO;
}