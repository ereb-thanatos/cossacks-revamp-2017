#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>

typedef unsigned short word;


#define GP_USER
#define DIALOGS_USER
#define FASTDRAW_USER
#define FONTS_USER
#define GFILE_USER

#include "fastdraw.h"
#pragma pack(1)
#include "gp_draw.h"
#include "dialogs.h"
#include "fonts.h"
#include "arc\gscarch.h"
#include "resfile.h"
#include "gfile.h"

extern __declspec(dllimport) int RealLx;
extern __declspec(dllimport) int RealLy;
extern __declspec(dllimport) int SCRSizeX;
extern __declspec(dllimport) int SCRSizeY;
extern __declspec(dllimport) int RSCRSizeX;
extern __declspec(dllimport) int RSCRSizeY;
extern __declspec(dllimport) int COPYSizeX;
extern __declspec(dllimport) int Pitch;

extern __declspec(dllimport) char BuildVersion[32];

extern __declspec(dllimport) int FPSTime;
extern __declspec(dllimport) int ScrollSpeed;
extern __declspec(dllimport) int WarSound;
extern __declspec(dllimport) int WorkSound;
extern __declspec(dllimport) int OrderSound;
extern __declspec(dllimport) int MidiSound;
extern __declspec(dllimport) void StopPlayCD();
extern __declspec(dllimport) int GetCDVolume();
extern __declspec(dllimport) void SetCDVolume(int Vol);
extern __declspec(dllimport) char RECFILE[128];
extern __declspec(dllimport) int ModeLX[32];
extern __declspec(dllimport) int ModeLY[32];
extern __declspec(dllimport) int NModes;
extern __declspec(dllimport) char PlName[64];
extern __declspec(dllimport) bool KeyPressed;
extern __declspec(dllimport) int NameChoose;
extern __declspec(dllimport) int ItemChoose;
extern __declspec(dllimport) char IPADDR[128];
extern __declspec(dllimport) int selected_network_protocol;
extern __declspec(dllimport) bool TOTALEXIT;
extern __declspec(dllimport) int LastKey;

__declspec(dllimport) void CBar(int x0,int y0,int Lx0,int Ly0,byte c);
__declspec(dllimport) bool MMItemChoose(SimpleDialog* SD);
__declspec(dllimport) int GETV(char* Name);
__declspec(dllimport) char* GETS(char* Name);
__declspec(dllimport) void LoadFog(int set);
__declspec(dllimport) bool MMChooseName(SimpleDialog* SD);
__declspec(dllimport) bool ProcessMessages();
__declspec(dllimport) void StdKeys();
__declspec(dllimport) void SlowLoadPalette(LPCSTR lpFileName);
__declspec(dllimport) void SlowUnLoadPalette(LPCSTR lpFileName);
__declspec(dllimport) void SavePlayerData();
__declspec(dllimport) void LoadPlayerData();
__declspec(dllimport) int CurPalette;
