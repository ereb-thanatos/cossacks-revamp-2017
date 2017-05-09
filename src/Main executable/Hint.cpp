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
#include "Fonts.h"
#include "recorder.h"

//Duration in ms for various messages and hints
extern const int kChatMessageDisplayTime = 10000;
extern const int kImportantMessageDisplayTime = 5000;
extern const int kSystemMessageDisplayTime = 3000;
extern const int kMinorMessageDisplayTime = 1500;


extern bool RecordMode;
extern byte PlayGameMode;
extern byte PlayGameMode;
extern byte CHOPT;
extern char LASTCHATSTR[512];

//Ensures that ProcessHints() does not run more frequently than every ... ms
const unsigned int kMinimalProcessingInterval = 100;

unsigned long last_hint_processing_time = 0;

//Base coordinates for all calculations
int HintX;
int HintY;

//Primary UI hint field
char primary_hint[256];
char primary_hint_lo[256];
static unsigned int primary_hint_time;

//Maximum number of simoultaneous shown chat messages or hints
const int kMaxHintCount = 25;
//Additional secondary hints and chat messages
char additional_hints[kMaxHintCount][256];
static unsigned int  additional_hints_times[kMaxHintCount];

//Hint or chat messages design
//0: normal; 16-23: national color border; 32: red
byte HintOpt[kMaxHintCount];

//Calculates coordinates for primary hint field
void SetupHint()
{
	HintY = smapy + smaply * 32 - 20;
	HintX = smapx;
	primary_hint_time = 0;
}

//Shows active hints and chat messages
void ShowHints()
{
	if (PlayGameMode == 1)
	{
		return;
	}

	if (primary_hint_time)
	{
		ShowString( HintX + 2, HintY + 2, primary_hint, &BlackFont );
		ShowString( HintX, HintY, primary_hint, &WhiteFont );

		ShowString( HintX + 2, HintY + 20 + 2, primary_hint_lo, &BlackFont );
		ShowString( HintX, HintY + 20, primary_hint_lo, &WhiteFont );
	}

	int yy = HintY - 20;
	for (int i = 0; i < kMaxHintCount; i++)
	{
		if (additional_hints_times[i])
		{
			ShowString( HintX + 2, yy + 2, additional_hints[i], &BlackFont );
			byte opt = HintOpt[i];
			byte opp = opt >> 4;
			if (opp == 2)
			{//Red blinking hint
				int tt = ( GetTickCount() / 300 ) & 1;
				if (tt)
				{
					ShowString( HintX, yy, additional_hints[i], &WhiteFont );
				}
				else
				{
					ShowString( HintX, yy, additional_hints[i], &RedFont );
				}
			}
			else
			{
				if (opp == 1)
				{//National color border
					ShowString( HintX, yy, additional_hints[i], &WhiteFont );
					Xbar( HintX - 2, yy, GetRLCStrWidth( additional_hints[i], &WhiteFont ) + 5,
						GetRLCHeight( WhiteFont.RLC, 'y' ) + 1, 0xD0 + ( ( opt & 7 ) << 2 ) );
				}
				else
				{//Default hint
					ShowString( HintX, yy, additional_hints[i], &WhiteFont );
				}
			}
		}
		yy -= 20;
	}
}

//Clears all hint strinhs except primary hint
void ClearHints()
{
	for (int i = 0; i < kMaxHintCount; i++)
	{
		additional_hints[i][0] = 0;
	}
	primary_hint_lo[0] = 0;
}

//Sets primary UI hint text for given time in ms
void AssignHint( char* s, int time )
{
	strcpy( primary_hint, s );
	primary_hint_time = time;
	primary_hint_lo[0] = 0;
}

//Sets primary UI hint text for given time in ms
void AssignHintLo( char* s, int time )
{
	strcpy( primary_hint_lo, s );
	primary_hint_time = time;
}

//Adds secondary hint or chat message for [time] ms
void CreateTimedHint( char* s, int time )
{
	if (!strcmp( s, additional_hints[0] ))
	{//Same text already on display, reset timer and exit
		additional_hints_times[0] = time;
		HintOpt[0] = 0;
		return;
	}

	for (int i = kMaxHintCount - 1; i > 0; i--)
	{//Shift existing hints and make room for one more
		strcpy( additional_hints[i], additional_hints[i - 1] );
		additional_hints_times[i] = additional_hints_times[i - 1];
		HintOpt[i] = HintOpt[i - 1];
	}

	strcpy( additional_hints[0], s );
	additional_hints_times[0] = time;
	HintOpt[0] = 0;
}

//Adds secondary hint or chat message with special design option for [time] ms
void CreateTimedHintEx( char* s, int time, byte opt )
{
	if (opt >= 16)
	{//National color masking requested
		if (RecordMode && !PlayGameMode)
		{//Do not assign option bit when watching a replay
			strcpy( LASTCHATSTR, s );
			CHOPT = opt;
		}
	}

	if (!strcmp( s, additional_hints[0] ))
	{//Same text already on display, reset timer and exit
		additional_hints_times[0] = time;
		HintOpt[0] = opt;
		return;
	}

	for (int i = kMaxHintCount - 1; i > 0; i--)
	{//Shift existing hints and make room for one more
		strcpy( additional_hints[i], additional_hints[i - 1] );
		additional_hints_times[i] = additional_hints_times[i - 1];
		HintOpt[i] = HintOpt[i - 1];

	}

	strcpy( additional_hints[0], s );
	additional_hints_times[0] = time;
	HintOpt[0] = opt;
}

//Shows all hints and adjusts timestamps
//Call rate ~38 Hz
void ProcessHints()
{
	ShowHints();

	//BUGFIX: Replaced weird internal function call timer with proper timing in ms

	unsigned long current_time = GetTickCount();

	if (0 == last_hint_processing_time)
	{
		last_hint_processing_time = current_time;
	}

	const unsigned int delta = current_time - last_hint_processing_time;

	if (kMinimalProcessingInterval > delta)
	{//Don't overdo it with precision
		return;
	}

	if (delta < primary_hint_time)
	{
		primary_hint_time -= delta;
	}
	else
	{
		primary_hint_time = 0;
	}

	for (int i = 0; i < kMaxHintCount; i++)
	{
		if (delta < additional_hints_times[i])
		{
			additional_hints_times[i] -= delta;
		}
		else
		{
			additional_hints_times[i] = 0;
		}
	}

	last_hint_processing_time = current_time;
}

//Copies GO->Message to s
void GetChar( GeneralObject* GO, char* s )
{
	Visuals* VS = (Visuals*) GO;
	strcpy( s, VS->Message );
}
