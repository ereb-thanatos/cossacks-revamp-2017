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

int prevrtime = 0;
FILE* FX = nullptr;
extern bool RecordMode;
extern byte PlayGameMode;
char CURLOG[32];

void WriteRec( char* s )
{
	if ( !( PlayGameMode || RecordMode ) )
	{
		return;
	}

	if ( GetTickCount() - prevrtime > 10000 )
	{
		if ( FX )
		{
			fclose( FX );
			if ( PlayGameMode )
			{
				FX = fopen( "lox1.log", "a" );
				strcpy( CURLOG, "lox1.log" );
			}
			else
			{
				FX = fopen( "lox0.log", "a" );
				strcpy( CURLOG, "lox0.log" );
			}
		}
		prevrtime = GetTickCount();
	}
	if ( !FX )
	{
		if ( PlayGameMode )
		{
			FX = fopen( "lox1.log", "a" );
			strcpy( CURLOG, "lox1.log" );
		}
		else
		{
			FX = fopen( "lox0.log", "a" );
			strcpy( CURLOG, "lox0.log" );
		}
	}
	fprintf( FX, s );
}

extern int tmtmt;
extern DWORD RealTime;

void Syncro::Copy( Syncro* Syn )
{
	memcpy( Syn->RSL, RSL, NSlots * sizeof RandSlot );
	Syn->NSlots = NSlots;
	NSlots = 0;
}

Syncro SYN;
Syncro SYN1;
extern short randoma[8192];
extern char LASTFILE[128];
extern int LastLine;
char* LastFile;

int RandNew( char* File, int Line )
{
	LastFile = File;
	LastLine = Line;
	int r = randoma[rpos];
	rpos = ( rpos + 1 ) & 8191;
	if ( SYN.NSlots < maxUline )
	{
		RandSlot* RS = SYN.RSL + SYN.NSlots;
		RS->FileName = File;
		RS->Line = Line;
		RS->Param = 0;
		RS->rpos = rpos;
		RS->Type = -1;
		SYN.NSlots++;
	}

	return r;
}
