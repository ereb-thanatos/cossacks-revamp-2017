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
#include "StringHash.h"

void NLine( GFILE* f );
char* GetTextByID( char* ID );
int GetUnitKind( char* Name );

//UPGRADE_NAME Icon_ID Level Message Cost 
//      CATEGORY UNIT VALUE
//-------------------------------
//Level: 1..9
//Message:StringValue||NONE
//Cost: NResources Resoirce1_ID Resource1_Amount ... ResoirceN_ID ResourceN_Amount
//-------------------------------
//CATEGORY:
//CATEGORY_NAME [WEAPONKIND,ATTINDEX,ALL]
//Example:
//DAMAGE ALL
//DAMAGE WEAPONKIND FIRE
//DAMAGE WEAPONKIND GRP 2 FIRE SWORD
//DAMAGE ATTINDEX 2
//DAMAGE ATTINDEX GRP 2 1 2
//-------------------------------
//Categiries:
//SPEED
//SHIELD
//PROTECTION  [WEAPONKIND||ALL]
//ATTPAUSE    [WEAPONKIND||ATTINDEX||ALL]
//WEAPON      [WEAPONKIND||ATTINDEX||ALL]
//BUILD
//LIFE
//PRECISE     [WEAPONKIND||ATTINDEX||ALL]
//ATTRANGE    [WEAPONKIND||ATTINDEX||ALL]
//INSIDE      
//COST
//ENABLE      [UNIT||UPGRADE||UNIT&UPGRADE]
//DAMAGE      [WEAPONKIND||ATTINDEX||ALL]
//--------------------------------
//UNIT:
//UNIT_NAME||UNITKIND KIND_ID||ALL
//Example:
//Grenader
//UNITKIND SimpleBuilding
//UNITKIND Tower
//UNITKIND Wall
//--------------------------------
//Value:
//ID||IntegerValue
//Example of IntegerValue:
//+10 -10 100 +10% -10% 110%
int GetIconByName( char* Name );
int GetWeaponType( char* Name );
char* Sect;
void ErrM( char* s );
void IncPar( char* name, int line, char* Sect );
void GetWeapons( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{

};
int GetNTUnit( Nation* NT, char* Name )
{
	int nn = NT->NMon;
	GeneralObject** GO = NT->Mon;
	for (int i = 0; i < nn; i++)
	{
		if (!strcmp( Name, GO[i]->MonsterID ))return i;
	};
	return -1;
};
int GetNTUpgrade( Nation* NT, char* Name )
{
	int nn = NT->NUpgrades;
	for (int i = 0; i < nn; i++)
	{
		NewUpgrade* NU = NT->UPGRADE[i];
		if (NU && !strcmp( Name, NU->Name ))return i;
	};
	return -1;
};
int GetNTElement( Nation* NT, char* Name, bool UseUnits, bool UseUpgrades )
{
	int zz = -1;
	if (UseUnits)zz = GetNTUnit( NT, Name );
	if (zz == -1 && UseUpgrades)
	{
		zz = GetNTUpgrade( NT, Name );
		if (zz >= 0)zz += 8192;
	};
	return zz;
};
//CtgTypes:
//0-  no category
//1-  CtgValue=i if AttType[i]==CtgValue   //WEAPONKIND
//2-  ATTINDEX
//3-  0<=CtgValue<=NAttTypes
//4-  0..31
void GetWeaponKind( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	int z = Gscanf( f, "%s", gx );
	NU->CtgType = 1;
	if (z != 1)IncPar( Name, Line, Sect );
	if (!strcmp( gx, "GRP" ))
	{
		int ngrp;
		z = Gscanf( f, "%d", &ngrp );
		if (z != 1)
		{
			sprintf( gx, "%s,Line %d :%s:Invalid GRP directive format for the list of weapon.", Name, Line, Sect );
			ErrM( gx );
		};
		NU->CtgGroup = new word[ngrp];
		NU->NCtg = ngrp;
		for (int i = 0; i < ngrp; i++)
		{
			z = Gscanf( f, "%s", gx );
			if (z != 1)IncPar( Name, Line, Sect );
			int zz1 = GetWeaponType( gx );
			if (zz1 == -1)
			{
				sprintf( gy, "%s,Line %d :%s: Unknown weapon type:%s", Name, Line, Sect, gx );
				ErrM( gy );
			};
			NU->CtgGroup[i] = zz1;
		};
	}
	else
	{
		int zz1 = GetWeaponType( gx );
		if (zz1 == -1)
		{
			sprintf( gy, "%s,Line %d :%s: Unknown Monster ID:%s", Name, Line, Sect, gx );
			ErrM( gy );
		};
		NU->CtgValue = zz1;
		NU->CtgGroup = NULL;
	};
};
void GetAttIndex( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	int z = Gscanf( f, "%s", gx );
	int p1, p2;
	if (z != 1)IncPar( Name, Line, Sect );
	NU->CtgType = 2;
	if (!strcmp( gx, "GRP" ))
	{
		z = Gscanf( f, "%d", &p1 );
		if (z != 1)IncPar( Name, Line, Sect );
		NU->CtgGroup = new word[p1];
		NU->NCtg = p1;
		for (int i = 0; i < p1; i++)
		{
			z = Gscanf( f, "%d", &p2 );
			if (z != 1)IncPar( Name, Line, Sect );
			NU->CtgGroup[i] = p2;
		};
	}
	else
	{
		if (!strcmp( gx, "ALL" ))
		{
			NU->CtgGroup = new word[NAttTypes];
			for (int i = 0; i < NAttTypes; i++)
			{
				NU->CtgGroup[i] = i;
			};
			NU->NCtg = NAttTypes;
		}
		else
		{
			z = sscanf( gx, "%d", &p1 );
			if (z != 1)IncPar( Name, Line, Sect );
			NU->CtgValue = p1;
			NU->CtgGroup = NULL;
		};
	};
};
void GetCtgParam01( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	int z = Gscanf( f, "%s", gx );
	if (!strcmp( gx, "ATTINDEX" ))GetAttIndex( f, NU, NT, Name, Line );
	else if (!strcmp( gx, "WEAPONKIND" ))GetWeaponKind( f, NU, NT, Name, Line );
	else if (!strcmp( gx, "ALL" ))
	{
		NU->CtgType = 3;
		NU->CtgGroup = NULL;
		NU->CtgValue = 0;
	}
	else
	{
		sprintf( gy, "%s, Line %d :%s: ATTINDEX or WEAPONKIND or ALL expected instead of %s", Name, Line, Sect, gx );
		ErrM( gy );
	};
};
void GetUnits( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	int z = Gscanf( f, "%s", gx );
	if (z != 1)IncPar( Name, Line, Sect );
	if (!strcmp( gx, "GRP" ))
	{
		int ngrp;
		z = Gscanf( f, "%d", &ngrp );
		if (z != 1)
		{
			sprintf( gx, "%s,Line %d :%s:Invalid GRP directive format for the list of units.", Name, Line, Sect );
			ErrM( gx );
		};
		NU->UnitGroup = new word[ngrp];
		NU->NUnits = ngrp;
		NU->UnitType = 0;
		for (int i = 0; i < ngrp; i++)
		{
			z = Gscanf( f, "%s", gx );
			if (z != 1)IncPar( Name, Line, Sect );
			int zz1 = GetNTUnit( NT, gx );
			if (zz1 == -1)
			{
				sprintf( gy, "%s,Line %d :%s: Unknown Monster ID:%s", Name, Line, Sect, gx );
				ErrM( gy );
			};
			NU->UnitGroup[i] = zz1;
		};
	}
	else
		if (!strcmp( gx, "UNITKIND" ))
		{
			z = Gscanf( f, "%s", gx );
			if (z != 1)IncPar( Name, Line, Sect );
			if (!strcmp( gx, "GRP" ))
			{
				int ngrp;
				z = Gscanf( f, "%d", &ngrp );
				if (z != 1)
				{
					sprintf( gx, "%s,Line %d :%s:Invalid GRP directive format for UNITKIND.", Name, Line, Sect );
					ErrM( gx );
				};
				NU->UnitGroup = new word[ngrp];
				NU->NUnits = ngrp;
				NU->UnitType = 1;
				for (int i = 0; i < ngrp; i++)
				{
					z = Gscanf( f, "%s", gx );
					if (z != 1)IncPar( Name, Line, Sect );
					int zz1 = GetUnitKind( gx );
					if (zz1 == -1)
					{
						sprintf( gy, "%s,Line %d :%s: Unknown unit kind:%s", Name, Line, Sect, gx );
						ErrM( gy );
					};
					NU->UnitGroup[i] = zz1;
				};
			}
			else
			{
				int zz1 = GetUnitKind( gx );
				if (zz1 == -1)
				{
					sprintf( gy, "%s,Line %d :%s: Unknown unit kind:%s", Name, Line, Sect, gx );
					ErrM( gy );
				};
				NU->UnitValue = zz1;
				NU->UnitGroup = NULL;
				NU->UnitType = 1;
			};
		}
		else
		{
			int zz1 = GetNTUnit( NT, gx );
			if (zz1 == -1)
			{
				sprintf( gy, "%s,Line %d :%s: Unknown Monster ID:%s", Name, Line, Sect, gx );
				ErrM( gy );
			};
			NU->UnitValue = zz1;
			NU->UnitGroup = NULL;
			NU->UnitType = 0;
		};
};
void GetUnitsAndUpgrades( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line, bool UseUnits, bool UseUpgrades )
{
	NU->CtgType = 0;
	char gx[128];
	char gy[128];
	int z = Gscanf( f, "%s", gx );
	if (z != 1)IncPar( Name, Line, Sect );
	if (!strcmp( gx, "GRP" ))
	{
		int ngrp;
		z = Gscanf( f, "%d", &ngrp );
		if (z != 1)
		{
			sprintf( gx, "%s,Line %d :%s:Invalid GRP directive format for the list of units.", Name, Line, Sect );
			ErrM( gx );
		};
		NU->UnitGroup = new word[ngrp];
		NU->NUnits = ngrp;
		for (int i = 0; i < ngrp; i++)
		{
			z = Gscanf( f, "%s", gx );
			if (z != 1)IncPar( Name, Line, Sect );
			int zz1 = GetNTElement( NT, gx, UseUnits, UseUpgrades );
			if (zz1 == -1)
			{
				sprintf( gy, "%s,Line %d :%s: Unknown Monster/Upgrade ID:%s", Name, Line, Sect, gx );
				ErrM( gy );
			};
			NU->UnitGroup[i] = zz1;
		};
	}
	else
	{
		int zz1 = GetNTElement( NT, gx, UseUnits, UseUpgrades );
		if (zz1 == -1)
		{
			sprintf( gy, "%s,Line %d :%s: Unknown Monster/Upgrade ID:%s", Name, Line, Sect, gx );
			ErrM( gy );
		};
		NU->UnitValue = zz1;
		NU->UnitGroup = NULL;
	};
}

bool ReadValue( GFILE* f, int* result, int* ValType )
{
	char gx[128];

	int z = Gscanf( f, "%s", gx );
	if (z != 1)
	{
		return false;
	}

	if (gx[0] == '+')
	{
		*ValType = 1;
	}
	else
	{
		if (gx[0] == '-')
		{
			*ValType = 2;
		}
		else
		{
			*ValType = 0;
		}
	}

	if (*ValType)
	{
		gx[0] = ' ';
	}

	if (gx[strlen( gx ) - 1] == '%')
	{
		*ValType |= 4;
		gx[strlen( gx ) - 1] = 0;
	}

	int value;
	z = sscanf( gx, "%d", &value );
	if (z != 1)
	{
		return false;
	}

	*result = value;

	return true;
}

void GetValue( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	int value, ValType;
	char gy[128];

	if (ReadValue( f, &value, &ValType ))
	{
		NU->Value = value;
		NU->ValGroup = nullptr;
		NU->ValueType = ValType;
	}
	else
	{
		sprintf( gy, "%s, Line %d :%s: Invalid value", Name, Line, Sect );
		ErrM( gy );
	}
}

int GetResID( char* Name );

void GetCostValue( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	int p1, p2;
	int z = Gscanf( f, "%s", gx );
	if (z != 1)IncPar( Name, Line, Sect );
	if (!strcmp( gx, "GRP" ))
	{
		z = Gscanf( f, "%d", &p1 );
		if (z != 1)IncPar( Name, Line, Sect );
		NU->ValGroup = new int[p1];
		NU->NValues = p1;
		for (int i = 0; i < p1; i++)
		{
			z = Gscanf( f, "%s", gx );
			if (z != 1)IncPar( Name, Line, Sect );
			p2 = GetResID( gx );
			if (p2 == -1)
			{
				sprintf( gy, "%s, Line %d :%s: Invalid resource ID:%s", Name, Line, Sect, gx );
				ErrM( gy );
			};
			int value, ValType;
			if (!ReadValue( f, &value, &ValType ))
			{
				sprintf( gy, "%s, Line %d :%s: Invalid amount of resource %s.", Name, Line, Sect, gx );
				ErrM( gy );
			};
			NU->ValGroup[i] = ( ( ValType + ( p2 << 3 ) ) << 26 ) + value;
		};
	}
	else
	{
		p2 = GetResID( gx );
		if (p2 == -1)
		{
			sprintf( gy, "%s, Line %d :%s: Invalid resource ID:%s", Name, Line, Sect, gx );
			ErrM( gy );
		};
		int value, ValType;
		if (!ReadValue( f, &value, &ValType ))
		{
			sprintf( gy, "%s, Line %d :%s: Invalid amount of resource %s.", Name, Line, Sect, gx );
			ErrM( gy );
		};
		NU->Value = ( ( ValType + ( p2 << 3 ) ) << 26 ) + value;
		NU->ValGroup = NULL;
	};
};
void GetUpgradeCost( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	int p1, p2, p3;
	int z = Gscanf( f, "%s", gy );
	if (strcmp( gy, "#COST" ))
	{
		sprintf( gx, "%s, Line %d :%s: #COST expected but %s is found.", Name, Line, Sect, gy );
		ErrM( gx );
	};
	memset( NU->Cost, 0, sizeof NU->Cost );
	z = Gscanf( f, "%d", &p1 );
	if (z != 1)IncPar( Name, Line, Sect );
	for (int i = 0; i < p1; i++)
	{
		z = Gscanf( f, "%s", gx );
		if (z != 1)IncPar( Name, Line, Sect );
		p2 = GetResID( gx );
		if (p2 == -1)
		{
			sprintf( gy, "%s, Line %d :%s: Invalid resource ID:%s", Name, Line, Sect, gx );
			ErrM( gy );
		};
		z = Gscanf( f, "%d", &p3 );
		if (z != 1)
		{
			sprintf( gy, "%s, Line %d :%s: Invalid amount of resource %s.", Name, Line, Sect, gx );
			ErrM( gy );
		};
		NU->Cost[p2] = p3;
	};
};
void GetUpgradeTime( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	int p1;
	int z = Gscanf( f, "%s", gy );
	if (strcmp( gy, "#TIME" ))
	{
		sprintf( gx, "%s, Line %d :%s: #TIME expected but %s is found.", Name, Line, Sect, gy );
		ErrM( gx );
	};
	z = Gscanf( f, "%d", &p1 );
	if (z != 1)
	{
		sprintf( gx, "%s, Line %d :%s: Invalid #TIME value.", Name, Line, Sect );
		ErrM( gx );
	};
	NU->NStages = p1;
};
int GetWeaponIndex( char* str );
void GetWeapon( GFILE* f, NewUpgrade* NU, Nation* NT, char* Name, int Line )
{
	char gx[128];
	char gy[128];
	char gz[128];
	int z = Gscanf( f, "%s%s", gx, gz );
	int zzk = GetWeaponType( gx );
	if (zzk == -1)
	{
		sprintf( gy, "%s, Line %d :%s: Unknown weapon kind ID: %s", Name, Line, Sect, gx );
		ErrM( gy );
	};
	int zz = GetWeaponIndex( gz );
	if (zz == -1)
	{
		sprintf( gy, "%s, Line %d :%s: Unknown weapon ID: %s", Name, Line, Sect, gz );
		ErrM( gy );
	};
	NU->ValueType = 0;
	NU->ValGroup = nullptr;
	NU->Value = zz + ( zzk << 16 );
}

StrHash UPGS;

bool UnderstandUpgrade( GFILE* f, char* UpgName, Nation* NT, char* name, int* lpLine, byte NID )
{
	if (NT->NNUM == 0 && NT->NUpgrades == 0)
	{
		UPGS.Clear();
	}

	Sect = UpgName;
	int line = *lpLine;
	int z;
	char gx[128];
	char gy[128];
	char gz[128];
	char gg[128];
	char g1[128];
	char g2[128];
	int p1, p4, icx, icy;
	z = Gscanf( f, "%s%s%d%d%s%d%s", gg, g1, &icx, &icy, g2, &p1, gy );
	if (z != 7)
	{
		IncPar( name, line, Sect );
	}

	if (strcmp( g1, "#POSITION" ))
	{
		sprintf( gz, "%s, Line %d :%s: #POSITION expected  but %s is found.", name, line, Sect, g1 );
		ErrM( gz );
	}

	if (strcmp( g2, "#LEVEL" ))
	{
		sprintf( gz, "%s, Line %d :%s: #LEVEL or #STAGELEVEL expected  but %s is found.", name, line, Sect, g2 );
		ErrM( gz );
	}

	NewUpgrade* NewUpg = new NewUpgrade;
	memset( NewUpg, 0, sizeof NewUpgrade );
	int NUPG = NT->NUpgrades;
	NT->UPGRADE[NUPG] = NewUpg;
	NT->NUpgrades++;

	if (NT->NNUM == 0)
	{
		UPGS.AddString( UpgName );
	}

	GetUpgradeCost( f, NewUpg, NT, name, line + 1 );
	GetUpgradeTime( f, NewUpg, NT, name, line + 1 );
	z = Gscanf( f, "%s", gz );

	if (!strcmp( gg, "NONE" ))
	{
		p4 = -1;
	}
	else
	{
		p4 = GetIconByName( gg );
		if (p4 == -1)
		{
			sprintf( gy, "%s,Line %d :%s: Invalid icon ID:%s", name, line, Sect, gx );
			ErrM( gy );
		}
	}

	if (!( ( icx >= 0 && icx < 12 && icy >= 0 && icy < 3 ) || ( icx == -1 && icy == -1 ) ))
	{
		sprintf( gy, "%s,Line %d :%s: Invalid icon coordinates:(%d,%d)", name, line, Sect, icx, icy );
		ErrM( gy );
	}

	NewUpg->IconFileID = 0;
	NewUpg->Options = 0;

	if (p4 == -1)
	{
		NewUpg->IconFileID = 0xFFFF;
	}

	NewUpg->IconSpriteID = p4;
	NewUpg->Name = new char[strlen( UpgName ) + 1];
	char* UpgMess = GetTextByID( gy );
	NewUpg->Message = new char[strlen( UpgMess ) + 1];
	strcpy( NewUpg->Name, UpgName );
	strcpy( NewUpg->Message, UpgMess );
	NewUpg->Level = p1;
	NewUpg->CtgType = 0;
	NewUpg->Enabled = false;
	NewUpg->PermanentEnabled = false;
	NewUpg->Done = false;
	NewUpg->IsDoing = false;
	NewUpg->Gray = false;
	NewUpg->CurStage = 0;
	NewUpg->AutoEnable = nullptr;
	NewUpg->AutoPerform = nullptr;
	NewUpg->NAutoEnable = 0;
	NewUpg->NAutoPerform = 0;
	NewUpg->IconPosition = icx + icy * 12;
	NewUpg->Individual = false;
	NewUpg->StageUp = false;
	NewUpg->Branch = 0xFF;
	NewUpg->NatID = NID;
	NewUpg->ManualDisable = 0;

	if (!strcmp( gz, "SPEED" ))
	{
		NewUpg->CtgUpgrade = 0;
		GetUnits( f, NewUpg, NT, name, line + 1 );
		GetValue( f, NewUpg, NT, name, line + 1 );
		line += 2;
	}
	else
		if (!strcmp( gz, "SHIELD" ))
		{
			NewUpg->CtgUpgrade = 1;
			GetUnits( f, NewUpg, NT, name, line + 1 );
			GetValue( f, NewUpg, NT, name, line + 1 );
			line += 2;
		}
		else
			if (!strcmp( gz, "PROTECTION" ))
			{
				NewUpg->CtgUpgrade = 2;
				GetWeaponKind( f, NewUpg, NT, name, line + 1 );
				GetUnits( f, NewUpg, NT, name, line + 1 );
				GetValue( f, NewUpg, NT, name, line + 1 );
				NewUpg->CtgType = 4;
				line += 2;
			}
			else
				if (!strcmp( gz, "ATTPAUSE" ))
				{
					NewUpg->CtgUpgrade = 3;
					GetCtgParam01( f, NewUpg, NT, name, line + 1 );
					GetUnits( f, NewUpg, NT, name, line + 1 );
					GetValue( f, NewUpg, NT, name, line + 1 );
					line += 2;
				}
				else
					if (!strcmp( gz, "WEAPON" ))
					{
						NewUpg->CtgUpgrade = 4;
						GetCtgParam01( f, NewUpg, NT, name, line + 1 );
						GetUnits( f, NewUpg, NT, name, line + 1 );
						GetWeapon( f, NewUpg, NT, name, line + 1 );
						line += 2;
					}
					else
						if (!strcmp( gz, "BUILD" ))
						{
							NewUpg->CtgUpgrade = 5;
							GetUnits( f, NewUpg, NT, name, line + 1 );
							GetValue( f, NewUpg, NT, name, line + 1 );
							line += 2;
						}
						else
							if (!strcmp( gz, "LIFE" ))
							{
								NewUpg->CtgUpgrade = 6;
								GetUnits( f, NewUpg, NT, name, line + 1 );
								GetValue( f, NewUpg, NT, name, line + 1 );
								line += 2;
							}
							else
								if (!strcmp( gz, "PRECISE" ))
								{
									NewUpg->CtgUpgrade = 7;
									GetCtgParam01( f, NewUpg, NT, name, line + 1 );
									GetUnits( f, NewUpg, NT, name, line + 1 );
									GetValue( f, NewUpg, NT, name, line + 1 );
									line += 2;
								}
								else
									if (!strcmp( gz, "ATTRANGE" ))
									{
										NewUpg->CtgUpgrade = 8;
										GetCtgParam01( f, NewUpg, NT, name, line + 1 );
										GetUnits( f, NewUpg, NT, name, line + 1 );
										GetValue( f, NewUpg, NT, name, line + 1 );
										line += 2;
									}
									else
										if (!strcmp( gz, "INSIDE" ))
										{
											NewUpg->CtgUpgrade = 9;
											GetUnits( f, NewUpg, NT, name, line + 1 );
											GetValue( f, NewUpg, NT, name, line + 1 );
											line += 2;
										}
										else
											if (!strcmp( gz, "COST" ))
											{
												NewUpg->CtgUpgrade = 10;
												GetUnits( f, NewUpg, NT, name, line + 1 );
												GetCostValue( f, NewUpg, NT, name, line + 1 );
												line += 2;
											}
											else
												if (!strcmp( gz, "ENABLE" ))
												{
													NewUpg->CtgUpgrade = 11;
													z = Gscanf( f, "%s", gg );
													if (!strcmp( gg, "UNITS" ))
													{
														GetUnitsAndUpgrades( f, NewUpg, NT, name, line + 1, true, false );
													}
													else if (!strcmp( gg, "UPGRADES" ))
													{
														GetUnitsAndUpgrades( f, NewUpg, NT, name, line + 1, false, true );
													}
													else if (!strcmp( gg, "UNITS&UPGRADES" ))
													{
														GetUnitsAndUpgrades( f, NewUpg, NT, name, line + 1, true, true );
													}
													else
													{
														sprintf( gy, "%s, Line %d :%s: UNITS or UPGRADES or UNITS&UPGRADES expected after ENABLED", name, line + 1, Sect );
														ErrM( gy );
													};
													NewUpg->ValGroup = NULL;
													line += 2;
												}
												else
													if (!strcmp( gz, "DAMAGE" ))
													{
														NewUpg->CtgUpgrade = 12;
														GetCtgParam01( f, NewUpg, NT, name, line + 1 );
														GetUnits( f, NewUpg, NT, name, line + 1 );
														GetValue( f, NewUpg, NT, name, line + 1 );
														line += 2;
													}
													else
														if (!strcmp( gz, "GETRES" ))
														{
															NewUpg->CtgUpgrade = 13;
															GetUnits( f, NewUpg, NT, name, line + 1 );
															GetValue( f, NewUpg, NT, name, line + 1 );
															line += 2;
														}
														else
															if (!strcmp( gz, "SINGLE_INSIDE" ))
															{
																NewUpg->CtgUpgrade = 14;
																GetUnits( f, NewUpg, NT, name, line + 1 );
																GetValue( f, NewUpg, NT, name, line + 1 );
																line += 2;
															}
															else
																if (!strcmp( gz, "SINGLE_ATTPAUSE" ))
																{
																	NewUpg->CtgUpgrade = 15;
																	GetCtgParam01( f, NewUpg, NT, name, line + 1 );
																	GetUnits( f, NewUpg, NT, name, line + 1 );
																	GetValue( f, NewUpg, NT, name, line + 1 );
																	line += 2;
																}
																else
																	if (!strcmp( gz, "FISHING" ))
																	{
																		NewUpg->CtgUpgrade = 16;
																		GetUnits( f, NewUpg, NT, name, line + 1 );
																		GetValue( f, NewUpg, NT, name, line + 1 );
																		line += 2;
																	}
																	else
																		if (!strcmp( gz, "RAZBROS" ))
																		{
																			NewUpg->CtgUpgrade = 17;
																			GetUnits( f, NewUpg, NT, name, line + 1 );
																			GetValue( f, NewUpg, NT, name, line + 1 );
																			line += 2;
																		}
																		else
																			if (!strcmp( gz, "SHAR" ))
																			{
																				NewUpg->CtgUpgrade = 18;
																				GetUnits( f, NewUpg, NT, name, line + 1 );
																				NewUpg->ValGroup = NULL;
																				line += 2;
																			}
																			else
																				if (!strcmp( gz, "EFFECT_FOOD" ))
																				{
																					NewUpg->CtgUpgrade = 19;
																					GetValue( f, NewUpg, NT, name, line + 1 );
																					line += 2;
																				}
																				else
																					if (!strcmp( gz, "EFFECT_WOOD" ))
																					{
																						NewUpg->CtgUpgrade = 20;
																						GetValue( f, NewUpg, NT, name, line + 1 );
																						line += 2;
																					}
																					else
																						if (!strcmp( gz, "EFFECT_STONE" ))
																						{
																							NewUpg->CtgUpgrade = 21;
																							GetValue( f, NewUpg, NT, name, line + 1 );
																							line += 2;
																						}
																						else
																							if (!strcmp( gz, "GEOLOGY" ))
																							{
																								NewUpg->CtgUpgrade = 22;
																								line += 2;
																							}
																							else
																								if (!strcmp( gz, "HEALING" ))
																								{
																									NewUpg->CtgUpgrade = 23;
																									GetUnits( f, NewUpg, NT, name, line + 1 );
																									line += 2;
																								}
																								else
																									if (!strcmp( gz, "UPSTAGE0" ))
																									{
																										NewUpg->CtgUpgrade = 24;
																										line += 2;
																									}
																									else
																										if (!strcmp( gz, "UPSTAGE1" ))
																										{
																											NewUpg->CtgUpgrade = 25;
																											line += 2;
																										}
																										else
																											if (!strcmp( gz, "UPSTAGE2" ))
																											{
																												NewUpg->CtgUpgrade = 26;
																												line += 2;
																											}
																											else
																												if (!strcmp( gz, "UPSTAGE3" ))
																												{
																													NewUpg->CtgUpgrade = 27;
																													line += 2;
																												}
																												else
																													if (!strcmp( gz, "UPSTAGE4" ))
																													{
																														NewUpg->CtgUpgrade = 28;
																														line += 2;
																													}
																													else
																													{
																														sprintf( gy, "%s, Line %d :%s: Unknown category of upgrade:%s", name, line + 1, Sect, gz );
																														ErrM( gy );
																													};
	*lpLine = line;
	for (int cp = 1; cp < 8; cp++)
	{
		NewUpgrade* NewUpg1 = new NewUpgrade;
		int NUPG = NT[cp].NUpgrades;
		NT[cp].UPGRADE[NUPG] = NewUpg1;
		NT[cp].NUpgrades++;
		memcpy( NewUpg1, NewUpg, sizeof NewUpgrade );
	};
	return true;
}

//performs upgrade immediately
void UseValue( int *Value, byte Type, int NewValue )
{
	switch (Type)
	{
	case 0://XX
		*Value = NewValue;
		break;
	case 1://+XX
		*Value += NewValue;
		break;
	case 2://-XX
		*Value -= NewValue;
		break;
	case 4://XX%
		*Value = div( ( *Value )*NewValue, 100 ).quot;
		break;
	case 5://+XX%
		*Value += div( ( *Value )*NewValue, 100 ).quot;
		break;
	case 6:
		*Value -= div( ( *Value )*NewValue, 100 ).quot;
		break;
	}
}

typedef void UpgradeFN( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB );
void UpgradeSpeed( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	for (int i = 0; i < NAttTypes; i++)
	{
		int val = AC->Rate[i];
		UseValue( &val, ValueType, Value );
		AC->Rate[i] = val;
	};
	AC->Changed = true;
};
void UpgradeShield( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->Shield;
	UseValue( &val, ValueType, Value );
	AC->Shield = val;
	AC->Changed = true;
};
void UpgradeProtection( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->Protection[CtgValue];
	UseValue( &val, ValueType, Value );
	AC->Protection[CtgValue] = val;
	AC->Changed = true;
};
void UpgradeAttPause( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->AttackPause[CtgValue];
	UseValue( &val, ValueType, Value );
	AC->AttackPause[CtgValue] = val;
	AC->Changed = true;
};
void UpgradeWeapon( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	//AC->DamWeap[CtgValue]=WPLIST[Value&65535];
	//AC->WeaponKind[CtgValue]=Value>>16;
	AC->Changed = true;
};
void UpgradeBuild( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* param_object )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->ProduceStages;
	int oldval = val;
	UseValue( &val, ValueType, Value );
	AC->ProduceStages = val;
	AC->Changed = true;
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB && OB->Ref.General == GO)
		{
			if (OB->Stage < oldval)
			{
				if (OB->Stage >= val)
				{
					OB->Stage = val - 1;
					OB->NextStage();
				}
				else
				{
					if (OB->Stage)
					{
						OB->Stage--;
						OB->NextStage();
					}
				}
			}
			else
			{
				OB->Stage = val;
			}
		}
	}
}
void UpgradeLife( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* param_object )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->Life;
	UseValue( &val, ValueType, Value );
	if (val > 65535)val = 65535;
	AC->Life = val;
	for (int i = 0; i < MAXOBJECT; i++)
	{
		OneObject* OB = Group[i];
		if (OB && OB->Ref.General == GO)
		{
			OB->MaxLife = val;
			OB->Life = val;
		}
	}
	AC->Changed = true;
};
void UpgradeAttRange( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->AttackRadius2[CtgValue];
	UseValue( &val, ValueType, Value );
	AC->AttackRadius2[CtgValue] = val;
	val = AC->DetRadius2[CtgValue];
	UseValue( &val, ValueType, Value );
	AC->DetRadius2[CtgValue] = val;
	if (CtgValue + 1 < NAttTypes)
	{
		CtgValue++;
		val = AC->AttackRadius1[CtgValue];
		UseValue( &val, ValueType, Value );
		AC->AttackRadius1[CtgValue] = val;
		val = AC->DetRadius1[CtgValue];
		UseValue( &val, ValueType, Value );
		AC->AttackRadius1[CtgValue] = val;
	};
	AC->Changed = true;
	UpdateAttackR( AC );
};
void InviteAI_Peasants( OneObject* Mine );
void UpgradeInside( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->MaxInside;
	UseValue( &val, ValueType, Value );
	AC->MaxInside = val;
	AC->Changed = true;
};
void UpgradeCost( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	byte VT = ( Value >> 26 ) & 7;
	byte ResID = byte( DWORD( Value ) >> 29 );
	Value &= 0xFFFFFF;
	int val = AC->NeedRes[ResID];
	UseValue( &val, VT, Value );
	AC->NeedRes[ResID] = val;
	AC->Changed = true;
};
extern City CITY[8];

void UpgradeEnable( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	GO->Enabled = true;
	GO->CondEnabled = true;
	for (int i = 0; i < 8; i++)
	{
		CITY[i].EnumUnits();
		CITY[i].RefreshAbility();
	};
	GO->MoreCharacter->Changed = true;
}

void UpgradeDamage( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->MaxDamage[CtgValue];
	if (!val)return;
	UseValue( &val, ValueType, Value );
	AC->MaxDamage[CtgValue] = val;
	AC->Changed = true;
	UpdateAttackR( AC );
};
void UpgradeGetRes( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->ResEff;
	UseValue( &val, ValueType, Value );
	AC->ResEff = val;
	AC->Changed = true;
};
void ApplyToOneUnitAndOneCategory( GeneralObject* GO, word Category, NewUpgrade* NU, UpgradeFN* UFN, OneObject* OB )
{
	if (NU->ValGroup)
	{
		for (int i = 0; i < NU->NValues; i++)
			UFN( GO, Category, NU->ValueType, NU->ValGroup[i], OB );
	}
	else
	{
		UFN( GO, Category, NU->ValueType, NU->Value, OB );
	};
};
void UpgradeSingleAttPause( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	if (!OB)return;
	int val = OB->PersonalDelay;
	UseValue( &val, ValueType, Value );
	OB->PersonalDelay = val;
};
void UpgradeSingleInside( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	if (!OB)return;
	int val = OB->AddInside;
	UseValue( &val, ValueType, Value );
	OB->AddInside = val;
	if (OB&&OB->newMons->Usage == MineID&&OB->Nat->AI_Enabled)
	{
		InviteAI_Peasants( OB );
	};
};
void UpgradeFishing( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->FishAmount;
	UseValue( &val, ValueType, Value );
	AC->FishAmount = val;
	val = AC->FishSpeed;
	UseValue( &val, ValueType, Value );
	AC->FishSpeed = val;
	AC->Changed = true;
};
void UpgradeRazbros( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	AdvCharacter* AC = GO->MoreCharacter;
	int val = AC->Razbros;
	UseValue( &val, ValueType, Value );
	AC->Razbros = val;
	AC->Changed = true;
}

void UpgradeShar( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	NATIONS[OB->NNUM].SharAllowed = true;
	NATIONS[OB->NNUM].SharX = OB->RealX;
	NATIONS[OB->NNUM].SharY = OB->RealY;
	NATIONS[OB->NNUM].SearchRadius = 180;
}

void HealUnits( GeneralObject* GO, word CtgValue, byte ValueType, int Value, OneObject* OB )
{
	int N = NtNUnits[OB->NNUM];
	word* units = NatList[OB->NNUM];
	for (int i = 0; i < N; i++)
	{
		word MID = units[i];
		if (MID != 0xFFFF)
		{
			OneObject* OBJ = Group[MID];
			if (OBJ&&OBJ->Ref.General == GO)
			{
				int L = int( OBJ->Life ) + OBJ->MaxLife / 10;
				if (L > OBJ->MaxLife)L = OBJ->MaxLife;
				OBJ->Life = L;
			};
		};
	};
};
void ApplyToOneUnit( GeneralObject* GO, NewUpgrade* NU, UpgradeFN* UFN, OneObject* OB )
{
	NewMonster* NM = GO->newMons;
	switch (NU->CtgType)
	{
	case 0://no categoties
		ApplyToOneUnitAndOneCategory( GO, 0, NU, UFN, OB );
		break;
	case 1://WEAPONKIND
		if (NU->CtgGroup)
		{
			for (int j = 0; j < NU->NCtg; j++)
			{
				int Ctgval = NU->CtgGroup[j];
				for (int i = 0; i < NAttTypes; i++)
				{
					if (NM->WeaponKind[i] == Ctgval)
						ApplyToOneUnitAndOneCategory( GO, i, NU, UFN, OB );
				};
			};
		}
		else
		{
			for (int i = 0; i < NAttTypes; i++)
			{
				if (NM->WeaponKind[i] == NU->CtgValue)
					ApplyToOneUnitAndOneCategory( GO, i, NU, UFN, OB );
			};
		};
		break;
	case 2://ATTINDEX
		if (NU->CtgGroup)
		{
			for (int j = 0; j < NU->NCtg; j++)
			{
				int Ctgval = NU->CtgGroup[j];
				//assert(Ctgval>=0&&Ctgval<NAttTypes);
				ApplyToOneUnitAndOneCategory( GO, Ctgval, NU, UFN, OB );
			};
		}
		else
		{
			//assert(NU->CtgValue>=0&&NU->CtgValue<NAttTypes);
			ApplyToOneUnitAndOneCategory( GO, NU->CtgValue, NU, UFN, OB );
		};
		break;
	case 3://ALL
	{
		for (int i = 0; i < NAttTypes; i++)
			ApplyToOneUnitAndOneCategory( GO, i, NU, UFN, OB );
	};
	break;
	case 4://PROTECTION
		if (NU->CtgGroup)
		{
			for (int j = 0; j < NU->NCtg; j++)
			{
				int Ctgval = NU->CtgGroup[j];
				//assert(Ctgval>=0&&Ctgval<32);
				ApplyToOneUnitAndOneCategory( GO, Ctgval, NU, UFN, OB );
			};
		}
		else
		{
			//assert(NU->CtgValue>=0&&NU->CtgValue<32);
			ApplyToOneUnitAndOneCategory( GO, NU->CtgValue, NU, UFN, OB );
		};
		break;
	};
};
void ApplyToUnits( Nation* NT, NewUpgrade* NU, UpgradeFN* UFN, OneObject* OB )
{
	if (NU->UnitGroup)
	{
		if (NU->UnitType == 1)
		{
			for (int j = 0; j < NU->NUnits; j++)
			{
				int UnitValue = NU->UnitGroup[j];
				for (int i = 0; i < NT->NMon; i++)
				{
					GeneralObject* GO = NT->Mon[i];
					if (GO->newMons->Kind == UnitValue)
						ApplyToOneUnit( GO, NU, UFN, OB );
				};
			};
		}
		else
		{
			for (int i = 0; i < NU->NUnits; i++)
			{
				word val = NU->UnitGroup[i];
				//assert(val<NT->NMon);
				ApplyToOneUnit( NT->Mon[NU->UnitGroup[i]], NU, UFN, OB );
			};
		};
	}
	else
	{
		if (NU->UnitType == 1)
		{//UNITKIND
			int UnitValue = NU->UnitValue;
			for (int i = 0; i < NT->NMon; i++)
			{
				GeneralObject* GO = NT->Mon[i];
				if (GO->newMons->Kind == UnitValue)
					ApplyToOneUnit( GO, NU, UFN, OB );
			};
		}
		else
		{
			//assert(NU->UnitValue<NT->NMon);
			ApplyToOneUnit( NT->Mon[NU->UnitValue], NU, UFN, OB );
		};
	};
};

void EraseUnitsInPoint( int x, int y )
{
	if (x > 0 && y > 0 && x < VAL_MAPSX&&y < VAL_MAPSX)
	{
		int xc = x >> 3;
		int yc = y >> 3;
		int cell = ( ( yc >> 7 ) << VAL_SHFCX ) + ( xc >> 7 ) + VAL_MAXCX + 1;
		int NMon = MCount[cell];
		int ofs1 = cell << SHFCELL;
		word MID;
		for (int i = 0; i < NMon; i++)
		{
			MID = GetNMSL( ofs1 + i );
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB->x == x && OB->y == y)
				{
					OB->Die();
				}
			};
		};
	};
}

void TurnFogOff();
void PerformNewUpgrade( Nation* NT, int UIndex, OneObject* OB )
{
	NewUpgrade* NU = NT->UPGRADE[UIndex];
	switch (NU->CtgUpgrade)
	{
	case 0://SPEED
		ApplyToUnits( NT, NU, &UpgradeSpeed, OB );
		break;
	case 1://SHIELD
		ApplyToUnits( NT, NU, &UpgradeShield, OB );
		break;
	case 2://PROTECTION
		ApplyToUnits( NT, NU, &UpgradeProtection, OB );
		break;
	case 3://ATTPAUSE
		ApplyToUnits( NT, NU, &UpgradeAttPause, OB );
		break;
	case 4://WEAPON
		ApplyToUnits( NT, NU, &UpgradeWeapon, OB );
		break;
	case 5://BUILD
		ApplyToUnits( NT, NU, &UpgradeBuild, OB );
		break;
	case 6://LIFE
		ApplyToUnits( NT, NU, &UpgradeLife, OB );
		break;
	case 7://PRECISE
		break;
	case 8://ATTRANGE
		ApplyToUnits( NT, NU, &UpgradeAttRange, OB );
		break;
	case 9://INSIDE
		ApplyToUnits( NT, NU, &UpgradeInside, OB );
		break;
	case 10://COST
		ApplyToUnits( NT, NU, &UpgradeCost, OB );
		break;
	case 11://ENABLE
		ApplyToUnits( NT, NU, &UpgradeEnable, OB );
		break;
	case 12://DAMAGE
		ApplyToUnits( NT, NU, &UpgradeDamage, OB );
		break;
	case 13://GETRES
		ApplyToUnits( NT, NU, &UpgradeGetRes, OB );
		break;
	case 14://SINGLE_INSIDE
		ApplyToUnits( NT, NU, &UpgradeSingleInside, OB );
		break;
	case 15://SINGLE_ATTPAUSE
		ApplyToUnits( NT, NU, &UpgradeSingleAttPause, OB );
		break;
	case 16://FISHING
		ApplyToUnits( NT, NU, &UpgradeFishing, OB );
		break;
	case 17://RAZBROS
		ApplyToUnits( NT, NU, &UpgradeRazbros, OB );
		break;
	case 18://SHAR
	{
		if (MyNation == NT->CITY->NI)
		{//Prevent from firing when other players do their upgrades
			TurnFogOff();//BUGFIX: Set FogMode = 0 as soon as the update is through
		}
		ApplyToUnits( NT, NU, &UpgradeShar, OB );
	}
	break;
	case 19://EFFECT_FOOD
	{
		int val = NT->FoodEff;
		UseValue( &val, NU->ValueType, NU->Value );
		NT->FoodEff = val;
	}
	break;
	case 20://EFFECT_WOOD
	{
		int val = NT->WoodEff;
		UseValue( &val, NU->ValueType, NU->Value );
		if (200 < val)
		{
			val = 200;//just in case
		}
		NT->WoodEff = val;
	}
	break;
	case 21://EFFECT_STONE
	{
		int val = NT->StoneEff;
		UseValue( &val, NU->ValueType, NU->Value );
		if (400 < val)
		{
			val = 400;//just in case
		}
		NT->StoneEff = val;
	}
	break;
	case 22://GEOLOGY
		NT->Geology = true;
		break;
	case 23:
		ApplyToUnits( NT, NU, &HealUnits, OB );
		break;
	}

	if (NU->AutoEnable)
	{
		for (int i = 0; i < NU->NAutoEnable; i++)
		{
			NewUpgrade* NU1 = NT->UPGRADE[NU->AutoEnable[i]];
			NU1->Enabled = true;
			NU1->PermanentEnabled = true;
			NU1->Done = false;
		}
	}

	if (NU->AutoPerform)
	{
		for (int i = 0; i < NU->NAutoPerform; i++)
		{
			PerformNewUpgrade( NT, NU->AutoPerform[i], OB );
		}
	}
}

void CreateAdvCharacter( AdvCharacter* AC, NewMonster* NM )
{
	for (int i = 0; i < NAttTypes; i++)
	{
		AC->AttackPause[i] = NM->AttackPause[i];
		AC->AttackRadius1[i] = NM->AttackRadius1[i];
		AC->AttackRadius2[i] = NM->AttackRadius2[i];
		//AC->DamWeap[i]=NM->DamWeap[i];
		AC->DetRadius1[i] = NM->DetRadius1[i];
		AC->DetRadius2[i] = NM->DetRadius2[i];
		AC->MaxDamage[i] = NM->MaxDamage[i];
		AC->Rate[i] = NM->Rate[i];
		AC->WeaponKind[i] = NM->WeaponKind[i];
		AC->MaxInside = NM->MaxInside;
		AC->ResEff = NM->ResEff;
		AC->FishSpeed = NM->FishSpeed;
		AC->FishAmount = NM->FishAmount;
		AC->Razbros = NM->Razbros;
	};
	for (int i = 0; i < 32; i++)AC->Protection[i] = byte( NM->Protection[i] );
	for (int i = 0; i < 8; i++)AC->NeedRes[i] = NM->NeedRes[i];
	AC->Life = NM->Life;
	AC->ProduceStages = NM->ProduceStages;
	AC->Shield = NM->Shield;
	AC->Changed = false;
	UpdateAttackR( AC );
};
int GetUpgradeID( Nation* NT, char* Name )
{
	return UPGS.SearchString( Name );
};
