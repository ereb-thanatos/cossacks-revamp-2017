#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "fog.h"
#include "Megapolis.h"

#include <assert.h>
#include "walls.h"
#include "mode.h"
#include "GSound.h"
#include "MapSprites.h"
#include "NewMon.h"
#include "Nature.h"
char* WPK[7] = { "Удар мечем","Удар стрелой","Удар пикой","Выстрел ядром","Выстрел","Выстрел картечью","Поражение от гранаты" };
char* SHK[7] = { "меча","стрелы","пики","ядра","выстрела","картечи","гранаты" };
void sprintAttack( AdvCharacter* ADC, char* cc )
{
	cc[0] = 0;
	int Na = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i])Na++;
	int Nn = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i])
	{
		Nn++;
		if (Nn != Na)sprintf( cc, "%s:%d (радиус действия %d-%d), ", WPK[ADC->WeaponKind[i]], ADC->MaxDamage[i], ADC->AttackRadius1[i], ADC->AttackRadius2[i] );
		else sprintf( cc, "%s:%d (радиус действия %d-%d)", WPK[ADC->WeaponKind[i]], ADC->MaxDamage[i], ADC->AttackRadius1[i], ADC->AttackRadius2[i] );
		cc += strlen( cc );
	};
};
void sprintAttackInComparison( AdvCharacter* ADC, AdvCharacter* MODIF, char* cc )
{
	cc[0] = 0;
	int Na = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i] != MODIF->MaxDamage[i])Na++;
	int Nn = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i] != MODIF->MaxDamage[i])
	{
		Nn++;
		if (Nn != Na)sprintf( cc, "%s:%d, ", WPK[MODIF->WeaponKind[i]], MODIF->MaxDamage[i] );
		else sprintf( cc, "%s:%d", WPK[MODIF->WeaponKind[i]], MODIF->MaxDamage[i] );
		cc += strlen( cc );
	};
};
void sprintShield( AdvCharacter* ADC, char* cc )
{
	int Ns = 0;
	for (int i = 0; i < 7; i++)if (ADC->Protection[i])Ns++;
	int Nn = 0;
	if (Ns)sprintf( cc, "Общая защита:%d, защита ", ADC->Shield );
	else sprintf( cc, "Общая защита:%d", ADC->Shield );
	cc += strlen( cc );
	for (int i = 0; i < 7; i++)if (ADC->Protection[i])
	{
		Nn++;
		if (Nn != Ns)sprintf( cc, "от %s:%d, ", SHK[i], ADC->Protection[i] );
		else sprintf( cc, "от %s:%d", SHK[i], ADC->Protection[i] );
		cc += strlen( cc );
	};
};
void sprintShieldInComp( AdvCharacter* ADC, AdvCharacter* MODIF, char* cc )
{
	cc[0] = 0;
	int Ns = 0;
	for (int i = 0; i < 7; i++)if (ADC->Protection[i] != MODIF->Protection[i])Ns++;
	int Nn = 0;
	if (ADC->Shield != MODIF->Shield)
	{
		if (Ns)sprintf( cc, "Общая защита:%d\t Защита ", MODIF->Shield );
		else sprintf( cc, "Общая защита:%d", MODIF->Shield );
	}
	else
	{
		if (Ns)sprintf( cc, "Защита " );
	};
	cc += strlen( cc );
	for (int i = 0; i < 7; i++)if (ADC->Protection[i] != MODIF->Protection[i])
	{
		Nn++;
		if (Nn != Ns)sprintf( cc, "от %s:%d, ", SHK[i], MODIF->Protection[i] );
		else sprintf( cc, "от %s:%d", SHK[i], MODIF->Protection[i] );
		cc += strlen( cc );
	};
};
void PerformNewUpgrade( Nation* NT, int UIndex, OneObject* OB );
void PrintAttackUpgrades( Nation* NT, int NIndex, AdvCharacter* START, GFILE* f )
{
	int N = 0;
	char ccc[128];
	bool Present = false;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 12 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex&&NU->CtgGroup == NULL)
		{//Upgrade on attack
			Present = true;
		};
	};
	if (Present)
	{
		Gprintf( f, "\nУлучшение атаки:\n" );
	}
	else return;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 12 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex&&NU->CtgGroup == NULL)
		{//Upgrade on attack
			Gprintf( f, "%s +%d, Цена:", WPK[NU->CtgValue], NU->Value );
			int Nr = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])Nr++;
			int Nn = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])
			{
				Nn++;
				if (Nn < Nr)Gprintf( f, " %s:%d, ", RDS[j].Name, NU->Cost[j] );
				else Gprintf( f, " %s:%d\n", RDS[j].Name, NU->Cost[j] );
			};
			PerformNewUpgrade( NT, i, NULL );
			sprintAttackInComparison( START, NT->Mon[NIndex]->MoreCharacter, ccc );
			if (ccc[0])
			{
				Gprintf( f, "После тренировки:%s\n\n", ccc );
			};
		};
	};
};
void PrintShieldUpgrades( Nation* NT, int NIndex, AdvCharacter* START, GFILE* f )
{
	int N = 0;
	char ccc[128];
	bool Present = false;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 2 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex)
		{//Upgrade on protection
			Present = true;
		};
	};
	if (Present)
	{
		Gprintf( f, "Улучшение защиты oт меча,стелы,пики:\n" );
	}
	else return;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 2 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex)
		{//Upgrade on attack
			Gprintf( f, "Защита +%d, Цена:", NU->Value );
			int Nr = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])Nr++;
			int Nn = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])
			{
				Nn++;
				if (Nn < Nr)Gprintf( f, " %s:%d, ", RDS[j].Name, NU->Cost[j] );
				else Gprintf( f, " %s:%d\n", RDS[j].Name, NU->Cost[j] );
			};
			PerformNewUpgrade( NT, i, NULL );
			sprintShieldInComp( START, NT->Mon[NIndex]->MoreCharacter, ccc );
			if (ccc[0])
			{
				Gprintf( f, "После тренировки:%s\n\n", ccc );
			};
		};
	};
};
void CreateAdvCharacter( AdvCharacter* AC, NewMonster* NM );
void MakeReportOnUnit( Nation* NT, int NIndex, GFILE* f )
{
	GeneralObject* GO = NT->Mon[NIndex];
	Gprintf( f, "%s\nЖизнь:%d\nВремя строительства:%d\nЦена:", GO->Message, GO->MoreCharacter->Life, GO->MoreCharacter->ProduceStages );
	NewMonster* NM = GO->newMons;
	AdvCharacter* OLD = new AdvCharacter;
	AdvCharacter* START = new AdvCharacter;
	AdvCharacter* NEW = GO->MoreCharacter;
	memcpy( OLD, NEW, sizeof AdvCharacter );
	CreateAdvCharacter( NEW, NM );
	memcpy( START, NEW, sizeof AdvCharacter );

	int Nr = 0;
	for (int i = 0; i < 8; i++)if (NM->NeedRes[i])Nr++;
	int Nn = 0;
	for (int i = 0; i < 8; i++)if (NM->NeedRes[i])
	{
		Nn++;
		if (Nn < Nr)Gprintf( f, " %s:%d, ", RDS[i].Name, NM->NeedRes[i] );
		else Gprintf( f, " %s:%d\n", RDS[i].Name, NM->NeedRes[i] );
	};
	if (NM->ResConsumer)
	{
		Gprintf( f, "Потребление золота:" );
		Gprintf( f, "%d/100\n", ( int( NM->ResConsumer ) * 100 ) / 400 );
	};
	char ccc[128];
	sprintAttack( NEW, ccc );
	if (ccc[0])Gprintf( f, "%s\n", ccc );
	sprintShield( NEW, ccc );
	Gprintf( f, "%s\n", ccc );
	PrintAttackUpgrades( NT, NIndex, START, f );
	PrintShieldUpgrades( NT, NIndex, START, f );
	memcpy( NEW, OLD, sizeof AdvCharacter );
	Gprintf( f, "\n" );
	free( OLD );
	free( START );
};
//-----------------------------------------------------------------------
char* RSN[6] = { "W","G","S","F","I","C" };
char* WKN[8] = { "M","S","P","N","V","K","G","H" };
void sprintBriefAttack( AdvCharacter* ADC, char* cc )
{
	cc[0] = 0;
	int Na = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i])Na++;
	int Nn = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i])
	{
		Nn++;
		if (Nn != Na)sprintf( cc, "%s:%d;", WKN[ADC->WeaponKind[i]], ADC->MaxDamage[i] );
		else sprintf( cc, "%s:%d", WKN[ADC->WeaponKind[i]], ADC->MaxDamage[i] );
		cc += strlen( cc );
	};
	if (!Na)
	{
		cc[0] = '0';
		cc[1] = 0;
	};
};
void sprintBriefAttackInComparison( AdvCharacter* ADC, AdvCharacter* MODIF, char* cc )
{
	cc[0] = 0;
	int Na = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i] != MODIF->MaxDamage[i])Na++;
	int Nn = 0;
	for (int i = 0; i < 4; i++)if (ADC->MaxDamage[i] != MODIF->MaxDamage[i])
	{
		Nn++;
		if (Nn != Na)sprintf( cc, "%s:%d,", WKN[MODIF->WeaponKind[i]], MODIF->MaxDamage[i] );
		else sprintf( cc, "%s:%d", WKN[MODIF->WeaponKind[i]], MODIF->MaxDamage[i] );
		cc += strlen( cc );
	};
};
void sprintBriefShield( AdvCharacter* ADC, char* cc )
{
	int Ns = 0;
	for (int i = 0; i < 7; i++)if (ADC->Protection[i])Ns++;
	int Nn = 0;
	if (Ns)sprintf( cc, "%d;", ADC->Shield );
	else sprintf( cc, "%d", ADC->Shield );
	cc += strlen( cc );
	for (int i = 0; i < 7; i++)if (ADC->Protection[i])
	{
		Nn++;
		if (Nn != Ns)sprintf( cc, "%s:%d;", WKN[i], ADC->Protection[i] );
		else sprintf( cc, "%s:%d", WKN[i], ADC->Protection[i] );
		cc += strlen( cc );
	};
};
void sprintBriefShieldInComp( AdvCharacter* ADC, AdvCharacter* MODIF, char* cc )
{
	cc[0] = 0;
	if (ADC->Shield + ADC->Protection[0] != MODIF->Shield + MODIF->Protection[0])
	{
		sprintf( cc, "%d", MODIF->Shield + MODIF->Protection[0] );
	};
	cc += strlen( cc );
};
void PerformNewUpgrade( Nation* NT, int UIndex, OneObject* OB );
void PrintBriefAttackUpgrades( Nation* NT, int NIndex, AdvCharacter* START, GFILE* f )
{
	int N = 0;
	char ccc[128];
	bool Present = false;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 12 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex&&NU->CtgGroup == NULL)
		{//Upgrade on attack
			Present = true;
		};
	};
	int NUP = 7;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 12 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex&&NU->CtgGroup == NULL)
		{//Upgrade on attack
			Gprintf( f, " +%d;", NU->Value );
			int Nr = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])Nr++;
			int Nn = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])
			{
				Nn++;
				if (Nn < Nr)Gprintf( f, "%s:%d,", RSN[j], NU->Cost[j] );
				else Gprintf( f, "%s:%d", RSN[j], NU->Cost[j] );
			};
			PerformNewUpgrade( NT, i, NULL );
			sprintBriefAttackInComparison( START, NT->Mon[NIndex]->MoreCharacter, ccc );
			if (ccc[0])
			{
				Gprintf( f, "(%s) ", ccc );
			};
			NUP--;
		};
	};
	for (int j = 0; j < NUP; j++)Gprintf( f, " ---/--- " );
};
void PrintBriefShieldUpgrades( Nation* NT, int NIndex, AdvCharacter* START, GFILE* f )
{
	int N = 0;
	char ccc[128];
	bool Present = false;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 2 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex)
		{//Upgrade on protection
			Present = true;
		};
	};
	int NUP = 7;
	for (int i = 0; i < NT->NUpgrades; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU->CtgUpgrade == 2 && NU->UnitGroup == NULL&&NU->UnitType == 0 && NU->UnitValue == NIndex)
		{//Upgrade on attack
			Gprintf( f, " +%d;", NU->Value );
			int Nr = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])Nr++;
			int Nn = 0;
			for (int j = 0; j < 8; j++)if (NU->Cost[j])
			{
				Nn++;
				if (Nn < Nr)Gprintf( f, "%s:%d,", RSN[j], NU->Cost[j] );
				else Gprintf( f, "%s:%d", RSN[j], NU->Cost[j] );
			};
			PerformNewUpgrade( NT, i, NULL );
			sprintBriefShieldInComp( START, NT->Mon[NIndex]->MoreCharacter, ccc );
			if (ccc[0])
			{
				Gprintf( f, "(%s) ", ccc );
			};
			NUP--;
		};
	};
	for (int j = 0; j < NUP; j++)Gprintf( f, " ---/--- " );
};
void MakeBriefReport( Nation* NT, int NIndex, GFILE* f )
{
	GeneralObject* GO = NT->Mon[NIndex];
	char ccc[128];
	strcpy( ccc, GO->Message );
	for (size_t j = 0; j < strlen( ccc ); j++)
	{
		if (ccc[j] == ' ')
		{
			ccc[j] = '_';
		}
	}
	Gprintf( f, "%s %d %d ", ccc, GO->MoreCharacter->Life, GO->MoreCharacter->ProduceStages );
	NewMonster* NM = GO->newMons;
	AdvCharacter* OLD = new AdvCharacter;
	AdvCharacter* START = new AdvCharacter;
	AdvCharacter* NEW = GO->MoreCharacter;
	memcpy( OLD, NEW, sizeof AdvCharacter );
	CreateAdvCharacter( NEW, NM );
	memcpy( START, NEW, sizeof AdvCharacter );

	int Nr = 0;
	for (int i = 0; i < 8; i++)if (NM->NeedRes[i])Nr++;
	int Nn = 0;
	for (int i = 0; i < 8; i++)if (NM->NeedRes[i])
	{
		Nn++;
		if (Nn < Nr)Gprintf( f, "%s:%d,", RSN[i], NM->NeedRes[i] );
		else Gprintf( f, "%s:%d ", RSN[i], NM->NeedRes[i] );
	};
	if (NM->ResConsID == GoldID)Gprintf( f, " %d ", ( int( NM->ResConsumer ) * 100 ) / 400 );
	else Gprintf( f, " 0 " );
	sprintBriefAttack( NEW, ccc );
	if (ccc[0])Gprintf( f, " %s ", ccc );
	sprintBriefShield( NEW, ccc );
	Gprintf( f, " %s ", ccc );
	PrintBriefAttackUpgrades( NT, NIndex, START, f );
	PrintBriefShieldUpgrades( NT, NIndex, START, f );
	Gprintf( f, "\n", ccc );
	memcpy( NEW, OLD, sizeof AdvCharacter );
	free( OLD );
	free( START );

};
extern int NNations;
extern char** NatNames;
