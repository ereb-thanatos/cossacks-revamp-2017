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
#include "Safety.h"
#include "3DGraph.h"
#include "Nature.h"
#include "ConstStr.h"

#define NTicks 100

char* Messages[NTicks];
int NTimers;

extern int RealLx;

int AddTime = 0;
int NeedAddTime = 0;
int PREVT = 0;

//returns GetTickCount()
int GetTicksEx()
{
	int T = GetTickCount();
	//T+=T>>10;
	return T;
}

int GetDADDT(int DT)
{
	DT >>= 7;
	if (DT < 50) return 1;
	if (DT < 100) return 2;
	if (DT < 150) return 3;
	if (DT < 220) return 4;
	if (DT < 350) return 5;
	if (DT < 400) return 6;
	if (DT < 600) return 7;
	if (DT < 1000) return 10;
	return 20;
}


//returns GetTickCount()
unsigned long GetRealTime()
{
	return GetTickCount();//REFACTORED: simplify GetRealTime()?
	/*
	//In single player AddTime always = 0
	//Overall this function returns pretty exact the GetTickCount values
	int tick_count = GetTickCount();
	if (PREVT)
	{
		if (tick_count - PREVT > 5)
		{
			if (NeedAddTime < AddTime)
			{
				AddTime -= GetDADDT(AddTime - NeedAddTime);
				if (AddTime < NeedAddTime)
				{
					AddTime = NeedAddTime;
				}
			}
			if (NeedAddTime > AddTime)
			{
				AddTime += GetDADDT(NeedAddTime - AddTime);
				if (AddTime > NeedAddTime)
				{
					AddTime = NeedAddTime;
				}
			}
			PREVT = tick_count;
		}
	}
	else
	{
		PREVT = tick_count;
	}
	int retval = tick_count + (AddTime >> 7);
	return retval;
	*/
}

//returns GetTickCount()
int GetAbsoluteRealTime()
{
	return GetTickCount();
}
