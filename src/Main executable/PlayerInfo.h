#pragma once
#include <Windows.h>

typedef DWORD DPID1, FAR *LPDPID;
typedef unsigned short word;

#pragma pack(1)

//Beware: Sensitive to pragma pack(1) and exact structure size
//Can cause bugs. Fixed one in StartIGame()
//Original size: 132 bytes
//Used in MPlayer.cpp & CommCore with offset/size calculations
//e.g. memcpy(x, &PINFO[i].NationID, sizeof(PlayerInfo)-36)
struct PlayerInfo
{
	char name[32];
	DPID1 PlayerID;
	byte NationID;
	byte ColorID; //7 player colors coded as 0x00 - 0x06
	byte GroupID;
	char MapName[36 + 8]; //60-16-1-7-4 (?)
	int ProfileID;
	DWORD Game_GUID;
	byte UserParam[7]; //Values of additional game options
	byte Rank;
	word COMPINFO[8];
	int CHKSUMM;
	byte MapStyle;
	byte HillType;
	byte StartRes;
	byte ResOnMap;
	byte Ready;
	byte Host;
	byte Page;
	byte CD;
	word Version;
	byte VictCond;
	word GameTime;

	//New elements added after this point

	//3 elements for additional options
	//Didn't want to resize UserParam because 'word Version' would be moved.
	//This way you can recognize and be recognized by old 1.35 as new version.
	byte UserParam2[3];

	int speed_mode;
};
