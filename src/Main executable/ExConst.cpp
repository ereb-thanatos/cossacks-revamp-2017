#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#pragma pack(1)
#include "ResFile.h"
#include "gFile.h"
void NLine(GFILE*);
void ErrM(char*);
void normstr(char* str);
class GameConstants {
	char* names[1024];
	int nnames;
	int Values[1024];
	char* Snames[512];
	int nsnames;
	char* Strings[512];
public:
	GameConstants();
	~GameConstants();
	int GetValue(char* name);
	char* GetString(char* Name);
	bool CheckDef(char* Name);
};

GameConstants::GameConstants() {
	GFILE* f1 = Gopen("Const.lst", "r");
	
	nnames = 0;
	if (f1) {
		int z, value;
		char gx[128];
		char gy[128];
		char gz[128];
		do {
			z = Gscanf(f1, "%s", gx);
			if (z > 0) {
				if (gx[0] != '/'&&gx[0] != 0) {
					if (gx[0] == '@') {
						z = Gscanf(f1, "%s", &gy);
						if (z != 1) {
							sprintf(gz, "Const.lst: Invalid string %s", gx);
							ErrM(gz);
						};
						normstr(gy);
						Strings[nsnames] = new char[strlen(gy) + 1];
						strcpy(Strings[nsnames], gy);
						Snames[nsnames] = new char[strlen(gx) + 1];
						strcpy(Snames[nsnames], gx);
						
						
						nsnames++;
					}
					else {
						z = Gscanf(f1, "%d", &value);
						if (z != 1) {
							sprintf(gz, "Const.lst: Invalid integer value of %s", gx);
							ErrM(gz);
						};
						Values[nnames] = value;
						names[nnames] = new char[strlen(gx) + 1];
						strcpy(names[nnames], gx);
						nnames++;
					};
				}
				else {
					NLine(f1);
				};
			}
			else {
				Gclose(f1);
			};
		} while (z > 0);
	}
	else {
		ErrM("Could not load game constants : Const.lst");
	};
};
int GameConstants::GetValue(char* Name) {
	for (int i = 0; i < nnames; i++) {
		if (!strcmp(names[i], Name))return Values[i];
	};
	char gx[128];
	sprintf(gx, "Unknown external integer value ID: %s \n(It does not present in Const.lst)", Name);
	ErrM(gx);
	return 0;
};
bool GameConstants::CheckDef(char* Name)
{
	for (int i = 0; i < nnames; i++)
	{
		if (!strcmp(names[i], Name))
		{
			return true;
		}
	}
	for (int i = 0; i < nsnames; i++)
	{
		if (!strcmp(Snames[i], Name))
		{
			return true;
		}
	}
	return false;
};
char* GameConstants::GetString(char* Name) {
	for (int i = 0; i < nsnames; i++) {
		if (!strcmp(Snames[i], Name))return Strings[i];
	};
	char gx[128];
	sprintf(gx, "Unknown external string value ID: %s \n(It does not present in Const.lst)", Name);
	ErrM(gx);
	return NULL;
}

GameConstants::~GameConstants()
{
	for (int i = 0; i < nnames; i++)
	{
		free(names[i]);
	}
	nnames = 0;
	for (int i = 0; i < nsnames; i++)
	{
		free(Snames[i]);
		free(Strings[i]);
	}
	nsnames = 0;
}

GameConstants GConstant;
int GETV(char* Name) {
	return GConstant.GetValue(Name);
};
char* GETS(char* Name) {
	return GConstant.GetString(Name);
};
bool CHKV(char* Name) {
	return GConstant.CheckDef(Name);
};