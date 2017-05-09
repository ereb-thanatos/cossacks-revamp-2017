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
#include "NewUpgrade.h"
#include "ConstStr.h"

extern const int kSystemMessageDisplayTime;
extern const int kMinorMessageDisplayTime;

Upgrade::Upgrade()
{
	NUpgrades = 0;
};
void PerformUpgradeLink( OneObject* OBJ );

void OneObject::PerformUpgrade( word NewU, word MakerID )
{
	if (CheckOrderAbility())
		return;

	if (UnlimitedMotion)
		return;

	if (Nat->NUpgrades <= NewU || !Ready)
		return;

	word NUP = Ref.General->NUpgrades;
	word* UPL = Ref.General->Upg;
	bool canU = false;

	for (int pp = 0; pp < NUP; pp++)
	{
		if (UPL[pp] == NewU)
		{
			canU = true;
		}
	}

	if (!canU)
		return;

	NewUpgrade* NU = Nat->UPGRADE[NewU];

	if (!NU->Enabled)
		return;

	for (int i = 0; i < 8; i++)
	{
		if (XRESRC( NNUM, i ) < NU->Cost[i])
		{
			return;
		}
	}

	if (NU->Individual)
	{
		if (SingleUpgLevel != NU->Level)
		{
			return;
		}
	}

	int Use = newMons->Usage;
	for (int i = 0; i < 8; i++)
	{
		AddXRESRC( NNUM, i, -NU->Cost[i] );
		Nat->ControlProduce( NU->Branch, i, -NU->Cost[i] );
		if (Use == MineID)
		{
			Nat->ResOnMines[i] += NU->Cost[i];
		}
		Nat->ResOnUpgrade[i] += NU->Cost[i];
	}

	Order1* Or1 = GetOrdBlock();
	Or1->PrioryLevel = 96;
	Or1->NextOrder = LocalOrder;
	Or1->OrderType = 75;//Upgrade
	Or1->OrderTime = 0;
	Or1->DoLink = &PerformUpgradeLink;
	Or1->info.PUpgrade.NewUpgrade = NewU;
	Or1->info.PUpgrade.Stage = 1;
	Or1->info.PUpgrade.NStages = NU->NStages;
	Or1->info.PUpgrade.OldUpgrade = MakerID;

	Order1* LOR = LocalOrder;
	LocalOrder = Or1;

	if (!( NU->Individual || NU->StageUp ))
	{
		NU->Enabled = false;
		NU->PermanentEnabled = false;
		NU->IsDoing = true;
	}
	Ustage = 0;
	NUstages = NU->NStages;
	Ready = false;
}

void CreateTimedHint( char* s, int time );
extern int tmtmt;
extern int StartTmtmt;
void PerformUpgradeLink( OneObject* OBJ )
{
	word OI = OBJ->LocalOrder->info.PUpgrade.NewUpgrade;
	Nation* NT = OBJ->Nat;
	byte NI = NT->NNUM;
	NewUpgrade* NUP = NT->UPGRADE[OI];
	OBJ->LocalOrder->info.PUpgrade.Stage += FrmDec;
	OBJ->Ustage = OBJ->LocalOrder->info.PUpgrade.Stage;
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;

	if (OBJ->LocalOrder->info.PUpgrade.Stage >= OBJ->LocalOrder->info.PUpgrade.NStages)
	{
		OBJ->Ready = true;
		OneObject* OBU = NULL;
		word MID = OBJ->LocalOrder->info.PUpgrade.OldUpgrade;
		if (MID != 0xFFFF)
		{
			OBU = Group[MID];
		};
		if (!( NUP->Individual || NUP->StageUp ))NUP->Done = true;
		PerformNewUpgrade( &NATIONS[OBJ->NNUM], OI, OBU );
		OBJ->Nat->AddUpgrade( OI, tmtmt + StartTmtmt );
		if (NI == MyNation)
		{
			char ccc[200];
			sprintf( ccc, UPGMADE, NUP->Message );
			CreateTimedHint( ccc, kMinorMessageDisplayTime );//Improvement complete: %s
			LastActionX = OBJ->RealX >> 4;
			LastActionY = OBJ->RealY >> 4;
		};
		OBJ->DeleteLastOrder();
		OBJ->SingleUpgLevel++;

	};
};
