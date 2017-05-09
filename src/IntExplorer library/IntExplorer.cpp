// IntExplorer.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
void EraseSXP();
extern bool LOGMODE;
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	FILE* F=fopen("dolog.dat","r");
	if(F){
		LOGMODE=1;
		fclose(F);
	};
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			EraseSXP();
			break;
	};
    return TRUE;
};

