#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "MapDiscr.h"
#include "Megapolis.h"
#include "Dialogs.h"
#include "fonts.h"
#include "GP_Draw.h"
#include "DrawForm.h"

#define DOTRACE

#include "PlayerInfo.h"
extern PlayerInfo PINFO[8];

#pragma pack(1)
class PLAYERLOGIN
{
public:
	PLAYERLOGIN()
	{
	};

	~PLAYERLOGIN()
	{
	};
	bool Connected;
	bool Logged;
	bool Error;
	byte Rank;
	char Nick[64];
	char EMail[128];
	char Password[64];
	bool LogIn(char* Name, char* Password); //true-logged ok, false - profile not exsists
};

PLAYERLOGIN LOGIN;

extern "C" void xDOTRACE(char* file, int Line);


extern int ItemChoose;
extern bool KeyPressed;
extern int ShowGameScreen;

byte INSIDE4 = 0;

void CheckExistingSaves();

void ShowCentralMessage(char* Message, int GPIDX);
void ShowCentralMessage2(char* Message, int GPIDX);
void ShowCentralMessage3(char* Message, int GPIDX);
void ShowCentralMessage4(char* Message, int GPIDX, int DX);

char* GetTextByID(char* ID);

bool ReadWinString(GFILE* F, char* STR, int Max);
bool MMItemChoose(SimpleDialog* SD);
bool ProcessMessages();

void WaitWithError(char* ID, int GPID)
{
	char ccc[1256];
	strcpy(ccc, GetTextByID(ID));
	ShowCentralMessage3(GetTextByID(ID), GPID);
	FlipPages();
	KeyPressed = 0;
	do
	{
		ProcessMessages();
	}
	while (!KeyPressed);
}

void WaitWithError4(char* ID, int GPID)
{
	char ccc[4096];
	strcpy(ccc, GetTextByID(ID));
	ShowCentralMessage4(GetTextByID(ID), GPID, 90);
	FlipPages();
	KeyPressed = 0;
	do
	{
		ProcessMessages();
	}
	while (!KeyPressed);
}


char* LASTNICK;
extern int RealLx;
extern int RealLy;

void GetAllPar3(int LXMAX, int* N, int* LMaxX, char* Message)
{
	int NStr = 1;
	int pos = 0;
	int LM = 0;
	int L = strlen(Message);
	int L0 = 0;
	do
	{
		if (L0 > LM)LM = L0;
		if (Message[pos] == ' ' && L0 > LXMAX)
		{
			NStr++;
			L0 = 0;
			pos++;
		}
		else
		{
			int DX;
			L0 += GetRLCWidthUNICODE(YellowFont.RLC, (byte*)(Message + pos), &DX);
			pos += DX;
		};
		if (L0 > LM)LM = L0;
	}
	while (Message[pos]);
	*N = NStr;
	*LMaxX = LM;
};
extern int CurPalette;

void ShowCentralMessage4(char* Message, int GPIDX, int DX)
{
	int LXMAX = 400;
	char ccx[256];
	int L0 = 0;
	int NStr = 1;
	int pos = 0;
	int LM = 0;
	int L = strlen(Message);
	GetAllPar3(LXMAX, &NStr, &LM, Message);
	int N1, LM1;
	int DXXX = 32;
	do
	{
		GetAllPar3(LXMAX - DXXX, &N1, &LM1, Message);
		DXXX += 32;
	}
	while (N1 == NStr && N1 > 1);
	LXMAX -= DXXX - 64;
	GetAllPar3(LXMAX, &NStr, &LM, Message);
	ccx[0] = 0;
	pos = 0;
	int pos1 = 0;

	int DY = 25;
	if (CurPalette == 2)
	{
		DrawStdBar2(
			(RealLx - 64 - LM) / 2 - DX,
			(RealLy - 70) / 2 + 18 - DY - (NStr - 1) * 13,
			(RealLx + 64 + LM) / 2 - DX,
			(RealLy - 70) / 2 + 55 - DY + (NStr - 1) * 13,
			GPIDX
		);
	}
	else
	{
		DX = 0;
		DrawStdBar(
			(RealLx - 64 - LM) / 2 - DX,
			(RealLy - 70) / 2 + 18 - DY - (NStr - 1) * 13,
			(RealLx + 64 + LM) / 2 - DX,
			(RealLy - 70) / 2 + 55 - DY + (NStr - 1) * 13
		);
	}
	int y0 = (RealLy - GetRLCHeight(YellowFont.RLC, 'W')) / 2 + 1 - DY - (NStr - 1) * 13;
	L0 = 0;
	do
	{
		if ((Message[pos] == ' ' && L0 > LXMAX))
		{
			int LL = GetRLCStrWidth(ccx, &YellowFont);
			ShowString((RealLx - LL) / 2 + 1 - DX, y0 + 1, ccx, &BlackFont);
			ShowString((RealLx - LL) / 2 - DX, y0, ccx, &YellowFont);
			pos1 = 0;
			ccx[0] = 0;
			L0 = 0;
			pos++;
			y0 += 26;
		}
		else
		{
			int length;
			L0 += GetRLCWidthUNICODE(YellowFont.RLC, (byte*)(Message + pos), &length);
			memcpy(ccx + pos1, Message + pos, length);
			pos += length;
			pos1 += length;
			ccx[pos1] = 0;
		}
		if (!Message[pos])
		{
			int LL = GetRLCStrWidth(ccx, &YellowFont);
			ShowString((RealLx - LL) / 2 + 1 - DX, y0 + 1, ccx, &BlackFont);
			ShowString((RealLx - LL) / 2 - DX, y0, ccx, &YellowFont);
		}
	}
	while (Message[pos]);
}

void ShowCentralMessage3(char* Message, int GPIDX)
{
	ShowCentralMessage4(Message, GPIDX, 206);
}

bool CHKCLICK(SimpleDialog* SD)
{
	GP_Button* GB = (GP_Button*)SD;
	GB->ActiveFrame = 8;
	GB->PassiveFrame = 9;
	return true;
};

void GETPROFINFO(int ProfileID)
{
};

bool GPI_Answer = 0;

struct SRRESULT
{
	bool NOTUNICAL;
	bool FINAL;
	bool Error;
	char* email;
};

//-1 -back,0-cancel,1-use it ,2-use it like original
void MarkNameLikeUnical(int, bool);

void CheckOriginalNick(int ProfileID)
{
	if (!LOGIN.Logged)
	{
		return;
	}
}

//----------------------Personal user data--------------------//
struct OneRatedPlayer
{
	byte Rank;
	byte State;
	byte Mask;
	byte Color;
	byte NationID;
	int Profile;
	int Score;
	char Nick[32];
	word MaxScore;
	word MaxPopul;
	byte ScoreG[32];
	byte Popul[32];
};

struct SysTime
{
	word Year;
	word Month;
	word Day;
	word DayOfWeek;
	word Hour;
	word Min;
};

class CurrentGame
{
public:
	bool Active;
	char GameName[128];
	char MapName[128];
	int ProfileID;
	char Nick[64];
	char InGameNick[64];
	char password[64];
	char email[127];
	bool Broken;
	bool Finished;

	SysTime StartTime;
	SysTime LastLoadTime;
	SysTime CurTime;

	int PlayingTime;
	int NPL;
	byte GameType;
	OneRatedPlayer PLAYERS[8];
	//-----------------------------//
	void CreateGame(
		char* GameName,
		char* Map,
		char* Nick
	);
	void UpdateGame();
	void SaveGameToFile();
	void LoadGameFromFile();
	void DeleteThisGameFromFile();
	void AssignVictory(char* Nick);
	void AssignDefeat(char* Nick);
	void AssignKick(char* Nick);
	int GetAddScore();
};

extern word NPlayers;
extern City CITY[8];
extern DWORD MyDPID;
int SortPlayers(byte* Res, int* par);

void CurrentGame::CreateGame(char* pGameName, char* pMap, char* pNick)
{
	Active = false;
}

extern int GameTime;

void CurrentGame::UpdateGame()
{
	if (Active)
	{
		PlayingTime = GameTime;
		byte ord[8];
		int par[8];
		int no = SortPlayers(ord, par);
		byte prevms = 0;
		int CurTm = 0;
		SYSTEMTIME SYSTM;
		GetSystemTime(&SYSTM);
		CurTime.Year = SYSTM.wYear;
		CurTime.Month = SYSTM.wMonth;
		CurTime.Day = SYSTM.wDay;
		CurTime.Hour = SYSTM.wHour;
		CurTime.Min = SYSTM.wMinute;
		CurTime.DayOfWeek = SYSTM.wDayOfWeek;

		if (no)
		{
			for (int q = 0; q < no; q++)
			{
				byte ms = NATIONS[NatRefTBL[ord[q]]].NMask;
				if (NATIONS[NatRefTBL[ord[q]]].AI_Enabled)
				{
					Active = 0;
					DeleteThisGameFromFile();
					return;
				};
				byte c = NatRefTBL[ord[q]];
				int v = ord[q];
				if (!(ms & prevms))
				{
					CurTm++;
				};
				int PIDIDX = -1;
				for (int w = 0; w < NPlayers; w++)if (PINFO[w].ColorID == ord[q])PIDIDX = w;
				if (PIDIDX == -1)
				{
					if (NATIONS[c].AI_Enabled)
					{
						strcpy(PLAYERS[q].Nick, "Computer");
					}
					else
					{
						strcpy(PLAYERS[q].Nick, "???");
					};
					PLAYERS[q].Rank = 0;
				}
				else
				{
					strcpy(PLAYERS[q].Nick, PINFO[PIDIDX].name);
					PLAYERS[q].Rank = PINFO[PIDIDX].Rank;
					PLAYERS[q].Profile = PINFO[PIDIDX].ProfileID;
				};
				PLAYERS[q].Mask = NATIONS[c].NMask;
				PLAYERS[q].Color = c;
				PLAYERS[q].State = 3;
				if (CITY[c].Account >= 0)PLAYERS[q].Score = CITY[c].Account;
				else PLAYERS[q].Score = 0;

				//determining the nation
				int NID = 255;

				for (int j = 0; j < MAXOBJECT; j++)
				{
					OneObject* OB = Group[j];
					if (OB && OB->NNUM == c && OB->newMons->Peasant && !OB->Sdoxlo)
					{
						NID = OB->Ref.General->NatID;
					}
				}
				PLAYERS[q].NationID = NID;
				if (PINFO[q].PlayerID == MyDPID)
				{
					strcpy(InGameNick, PINFO[q].name);
				}

				memset(PLAYERS[q].Popul, 0, sizeof PLAYERS[q].Popul);
				memset(PLAYERS[q].ScoreG, 0, sizeof PLAYERS[q].ScoreG);

				if (NATIONS[c].NAccount > 1)
				{
					int NN = NATIONS[c].NAccount - 1;
					int MaxP = 1;
					for (int i = 0; i < NN; i++)
					{
						int v = NATIONS[c].Account[i];
						if (v > MaxP)MaxP = v;
					}
					for (int i = 0; i < 32; i++)
					{
						PLAYERS[q].ScoreG[i] = (NATIONS[c].Account[(NN * i) / 31] * 100) / MaxP;
					}
					PLAYERS[q].MaxScore = MaxP;
				}
				if (NATIONS[c].NPopul > 1)
				{
					int NN = NATIONS[c].NPopul - 1;
					int MaxP = 1;
					for (int i = 0; i < NN; i++)
					{
						int v = NATIONS[c].Popul[i];
						if (v > MaxP)MaxP = v;
					}
					for (int i = 0; i < 32; i++)
					{
						PLAYERS[q].Popul[i] = (NATIONS[c].Popul[(NN * i) / 31] * 100) / MaxP;
					}
					PLAYERS[q].MaxPopul = MaxP;
				}
			}
			NPL = no;
		}
		SaveGameToFile();
	}
}

void CurrentGame::AssignVictory(char* pNick)
{
	byte MyMask = 0;
	for (int i = 0; i < NPL; i++)
		if (!strcmp(PLAYERS[i].Nick, pNick))
		{
			PLAYERS[i].State = 0;
			MyMask = PLAYERS[i].Mask;
		}
	for (int i = 0; i < NPL; i++)
		if (PLAYERS[i].State != 1 && PLAYERS[i].Mask & MyMask)
		{
			PLAYERS[i].State = 0;
		}
		else
		{
			PLAYERS[i].State = 1;
		}
	if (!Broken)
	{
		Finished = 1;
	}
}

void CurrentGame::AssignDefeat(char* pNick)
{
	bool ALLMYLOOSE = 1;
	byte MyMask = 0;
	for (int i = 0; i < NPL; i++)
	{
		if (!strcmp(PLAYERS[i].Nick, pNick))
		{
			PLAYERS[i].State = 1;
			MyMask = PLAYERS[i].Mask;
		}
	}

	if (MyMask)
	{
		byte HimMask1 = 0;
		byte HimMask2 = 0;
		for (int i = 0; i < NPL; i++)
		{
			if (PLAYERS[i].Mask & MyMask)
			{
				if (PLAYERS[i].State != 1)
				{
					ALLMYLOOSE = 0;
				}
			}
			else
			{
				if (HimMask1)
				{
					if (!(PLAYERS[i].Mask & HimMask1))
					{
						HimMask2 = PLAYERS[i].Mask;
					}
				}
				else
				{
					HimMask1 = PLAYERS[i].Mask;
				}
			}
		}

		if (ALLMYLOOSE && !HimMask2)
		{
			for (int i = 0; i < NPL; i++)
			{
				if (PLAYERS[i].State != 1 && !(PLAYERS[i].Mask & MyMask))
				{
					PLAYERS[i].State = 0;
				}
			}
		}
	}

	if (!Broken)
	{
		Finished = 1;
	}
}

void CurrentGame::AssignKick(char* pNick)
{
	for (int i = 0; i < NPL; i++)
	{
		if (!strcmp(PLAYERS[i].Nick, pNick))
		{
			PLAYERS[i].State = 2;
		}
	}
}

void SetNormAttr();

void CurrentGame::SaveGameToFile()
{
	if (!Active)
	{
		return;
	}

	int t0 = (StartTime.Min + StartTime.Hour * 60) % 200;

	ResFile F1 = RReset("cew.dll");
	if (F1 == INVALID_HANDLE_VALUE)
	{
		return;
	}

	RSeek(F1, 20532 + t0);
	int v = 1;
	RBlockWrite(F1, &v, 1);
	RSeek(F1, 20532 + 200 + t0 * (sizeof( CurrentGame) + 4));

	CurrentGame XXX;
	XXX = *this;
	byte* DATA = (byte*)(&XXX);
	byte* Key = (byte*)randoma;
	for (int i = 0; i < sizeof XXX; i++)
	{
		DATA[i] ^= Key[i];
	}

	RBlockWrite(F1, &XXX, sizeof XXX);

	int S = 0;
	for (int i = 0; i < sizeof CurrentGame; i++)
	{
		S += DATA[i];
	}

	RBlockWrite(F1, &S, 4);
	RClose(F1);
	SetNormAttr();
}

void CurrentGame::LoadGameFromFile()
{
	ResFile F = RReset("cew.dll");
	if (F == INVALID_HANDLE_VALUE)
	{
		return;
	}

	RSeek(F, 20532);
	int cur = 0;
	int sz = 0;
	int t = GetTickCount() & 4096;

	for (int p = 0; p < t; p++)
	{
		rand();
	}

	int NMAX = 0;
	byte HEADR[200];
	RBlockRead(F, HEADR, 200);

NEXTFILE:
	bool SUCCESS = 0;
	do
	{
		cur = rand() % 200;
		SUCCESS = 0 != HEADR[cur];
		NMAX++;
	}
	while ((!SUCCESS) && NMAX < 5000);

	if (SUCCESS)
	{
		RSeek(F, 20532 + 200 + cur * (sizeof( CurrentGame) + 4));
		RBlockRead(F, this, sizeof CurrentGame);
		int S;
		RBlockRead(F, &S, 4);
		byte* Key = (byte*)randoma;
		byte* xx = (byte*)this;

		int S1 = 0;
		for (int i = 0; i < sizeof CurrentGame; i++)
		{
			S1 += xx[i];
		}
		if (S != S1)
		{
			Active = false;
		}
		else
		{
			for (int i = 0; i < sizeof CurrentGame; i++)
			{
				xx[i] ^= Key[i];
			}
		}

		if (!Finished)
		{
			//checking the start time
			SYSTEMTIME ST;
			GetSystemTime(&ST);
			FILETIME FT;
			SystemTimeToFileTime(&ST, &FT);
			SYSTEMTIME ST0;
			memset(&ST0, 0, sizeof ST0);
			ST0.wMinute = StartTime.Min;
			ST0.wHour = StartTime.Hour;
			ST0.wDay = StartTime.Day;
			ST0.wMonth = StartTime.Month;
			ST0.wYear = StartTime.Year;
			FILETIME FT0;
			SystemTimeToFileTime(&ST0, &FT0);
			LARGE_INTEGER LI0;
			LARGE_INTEGER LI;
			memcpy(&LI0, &FT0, 8);
			memcpy(&LI, &FT, 8);
			LI.QuadPart -= LI0.QuadPart;
			memcpy(&FT, &LI, 8);

			int dt = 10000000;
			if (FileTimeToSystemTime(&FT, &ST))
			{
				dt = (ST.wDay - 1) * 24 + ST.wHour;
			}

			if (dt < 24)
			{
				RClose(F);
				Active = 0;
				F = INVALID_HANDLE_VALUE;
				goto NEXTFILE;
			}
		}
	}
	else
	{
		Active = false;
	}
	RClose(F);
	SetNormAttr();
}

void CurrentGame::DeleteThisGameFromFile()
{
	int cur = 0;
	int sz = 0;

	ResFile F = RReset("cew.dll");
	if (F == INVALID_HANDLE_VALUE)
	{
		return;
	}

	int t0 = (StartTime.Min + StartTime.Hour * 60) % 200;
	RSeek(F, 20532 + t0);
	int v = 0;
	RBlockWrite(F, &v, 1);
	RClose(F);
	SetNormAttr();
};

struct OneIChunk
{
	byte Index;
	word size;
	word Summ;
	byte Data[64];
};

struct CurUpload
{
	bool Authenticated : 1;
	bool Started : 1;
	bool Finished : 1;
	bool Error : 1;
	bool DataSent : 1;
	int CurProfile;
	int CurPage;
};

struct CurDownload
{
	bool Started;
	bool Finished;
	bool Error;
	int CurProfile;
	int CurPage;
};

class InternetStatsChunks
{
public:
	int CurIndex;
	bool Ready;
	bool Error;
	int N;
	OneIChunk** Chunks;
	CurDownload CDOWN;
	CurUpload CUPLD;

	InternetStatsChunks();
	void Init();
	void Clear();
	int ReadChunkFrom(byte* Sourse);
	void WriteChunk(byte* Src, int L, byte Type);
	//----downloading process----//
	void StartDownload(int ProfileID, int Page);
	void ProcessDownload();
	void StartUpload(int ProfileID, char* Password, int Page);
	void ProcessUpload();
};

InternetStatsChunks::InternetStatsChunks()
{
	Init();
}

void InternetStatsChunks::Init()
{
	N = 0;
	CurIndex = 0;
	Chunks = NULL;
};

void InternetStatsChunks::Clear()
{
	for (int i = 0; i < N; i++)free(Chunks[i]);
	if (Chunks)free(Chunks);
	Chunks = NULL;
	N = 0;
};

void PersAuthCallback2(int localid, int profileid, int authenticated,
                       char* errmsg, void* instance)
{
	INSIDE4 = 17;
	DOTRACE;
	InternetStatsChunks* ISC = (InternetStatsChunks*)instance;
	if (authenticated)
	{
		ISC->CUPLD.Authenticated = 1;
	}
	else ISC->CUPLD.Error = 1;
	INSIDE4 = 18;
	DOTRACE;
};


void PersAuthCallback(int localid, int profileid, int authenticated,
                      char* errmsg, void* instance)
{
	INSIDE4 = 24;
	DOTRACE;
	int* res = (int*)instance;
	if (authenticated)
	{
		*res = 1;
	}
	else *res = 0;
	INSIDE4 = 25;
	DOTRACE;
};


void InternetStatsChunks::WriteChunk(byte* Src, int L, byte Type)
{
	if (!L)return;
	Chunks = (OneIChunk**)realloc(Chunks, 4 * N + 4);
	Chunks[N] = (OneIChunk*)malloc(5 + L);
	Chunks[N]->Index = Type;
	Chunks[N]->size = L;
	word S = 0;
	for (int i = 0; i < L; i++)
	{
		S += Src[i];
	};
	Chunks[N]->Summ = S;
	memcpy(Chunks[N]->Data, Src, L);
	N++;
};

#define WRDT(tp,val) (*((tp*)(BUFF+bps)))=val;bps+=sizeof(tp);
int GetRankByScore(int Score);

void EncodeGS_Password(char* pass, char* result)
{
	result[0] = (char)strlen(pass);
	char cc3[512];
	strcpy(cc3, pass);
	do
	{
		strcat(cc3, pass);
	}
	while (strlen(cc3) < 64);
	cc3[64] = 0;
	for (int i = 0; i < 1024; i++)
	{
		int v1 = randoma[i + i] & 511;
		int v2 = randoma[i + i + 1] & 511;
		int vo1 = v1 >> 3;
		int vo2 = v2 >> 3;
		int p1 = v1 & 7;
		int p2 = v2 & 7;
		byte b1 = cc3[vo1];
		byte b2 = cc3[vo2];
		byte mb1 = b1 & (1 << p1);
		byte mb2 = b2 & (1 << p2);
		b1 &= ~(1 << p1);
		b2 &= ~(1 << p2);
		if (mb1)b2 |= 1 << p2;
		if (mb2)b1 |= 1 << p1;
		cc3[vo1] = b1;
		cc3[vo2] = b2;
	};
	memcpy(result + 1, cc3, 65);
};

void DecodeGS_Password(char* pass, char* result)
{
	byte Len = result[0];
	char* cc3 = result + 1;
	for (int i = 1023; i >= 0; i--)
	{
		int v1 = randoma[i + i] & 511;
		int v2 = randoma[i + i + 1] & 511;
		int vo1 = v1 >> 3;
		int vo2 = v2 >> 3;
		int p1 = v1 & 7;
		int p2 = v2 & 7;
		byte b1 = cc3[vo1];
		byte b2 = cc3[vo2];
		byte mb1 = b1 & (1 << p1);
		byte mb2 = b2 & (1 << p2);
		b1 &= ~(1 << p1);
		b2 &= ~(1 << p2);
		if (mb1)b2 |= 1 << p2;
		if (mb2)b1 |= 1 << p1;
		cc3[vo1] = b1;
		cc3[vo2] = b2;
	};
	pass[Len] = 0;
	memcpy(pass, cc3, Len);
};


CurrentGame CURIGAME;


int PREVUPDATETIME = 0;

void UPDATEIGAME()
{
	if (CURIGAME.Active)
	{
		if ((!PREVUPDATETIME) || (GetTickCount() - PREVUPDATETIME > 60000))
		{
			CURIGAME.UpdateGame();
			PREVUPDATETIME = GetTickCount();
		};
	};
};
extern int ShowGameScreen;
void CheckExistingSaves();

void LOOSEIGAME(char* Nick)
{
	if (CURIGAME.Active)
	{
		CURIGAME.UpdateGame();
		CURIGAME.AssignDefeat(Nick);
		CURIGAME.SaveGameToFile();
		ShowGameScreen = 1;
		CheckExistingSaves();
		ShowGameScreen = 0;
		CURIGAME.Active = 0;
	}
}

void LOOSEANDEXITFAST()
{
	if (CURIGAME.Active)
	{
		for (int i = 0; i < NPlayers; i++)
		{
			if (PINFO[i].ColorID == MyNation)
			{
				CURIGAME.UpdateGame();
				CURIGAME.AssignDefeat(PINFO[i].name);
				CURIGAME.SaveGameToFile();
				CURIGAME.Active = 0;
			}
		}
	}
}

void WINIGAME(char* Nick)
{
	if (CURIGAME.Active)
	{
		CURIGAME.UpdateGame();
		CURIGAME.AssignVictory(Nick);
		CURIGAME.SaveGameToFile();
		ShowGameScreen = 1;
		CheckExistingSaves();
		ShowGameScreen = 0;
		CURIGAME.Active = 0;
	}
}

void UPDATEIGAME();
void LOOSEIGAME(char* Nick);
void WINIGAME(char* Nick);

char WinScoreTable[162] =
{
	5, -2, 10, -2, 20, -2, 30, -2, 40, -2, 50, -2, 60, -2, 70, -2, 80, -2,
	5, -2, 5, -2, 10, -2, 20, -2, 30, -2, 40, -2, 50, -2, 60, -2, 70, -2,
	5, -2, 5, -2, 5, -2, 10, -2, 20, -2, 30, -2, 40, -2, 50, -2, 60, -2,
	5, -2, 5, -2, 5, -2, 5, -2, 10, -2, 20, -2, 30, -2, 40, -2, 50, -2,
	5, -4, 5, -2, 5, -2, 5, -2, 5, -2, 10, -2, 20, -2, 30, -2, 40, -2,
	5, -6, 5, -4, 5, -2, 5, -2, 5, -2, 5, -2, 10, -2, 20, -2, 30, -2,
	5, -8, 5, -6, 5, -4, 5, -2, 5, -2, 5, -2, 5, -2, 10, -2, 20, -2,
	5, -16, 5, -8, 5, -6, 5, -4, 5, -2, 5, -2, 5, -2, 5, -2, 10, -2,
	5, -32, 5, -16, 5, -8, 5, -6, 5, -4, 5, -2, 5, -2, 5, -2, 5, -2
};

int CurrentGame::GetAddScore()
{
	byte MyMask = 0;
	int MyIndex = -1;
	for (int i = 0; i < NPL; i++)
	{
		if (!strcmp(this->PLAYERS[i].Nick, InGameNick))
		{
			MyMask = PLAYERS[i].Mask;
			MyIndex = i;
		}
	}
	if (MyIndex == -1)
	{
		return 0;
	}
	int NFriends = 0;
	int NOpp = 0;
	int MyRank = PLAYERS[MyIndex].Rank;
	int SummOpRank = 0;
	int SummFrRank = 0;
	int ADDSCO = 0;
	for (int i = 0; i < NPL; i++)
	{
		if (PLAYERS[i].Mask & MyMask)
		{
			NFriends++;
			SummFrRank += PLAYERS[i].Rank;
		}
		else
		{
			NOpp++;
			SummOpRank += PLAYERS[i].Rank;
			ADDSCO += WinScoreTable[(PLAYERS[i].Rank + MyRank * 9) * 2];
		}
	}
	if (!(NOpp && NFriends))
	{
		return 0;
	}
	SummOpRank /= NOpp;
	SummFrRank /= NFriends;

	if (PLAYERS[MyIndex].State == 0)
	{
		//Victory!
		int ADDS = ADDSCO / NFriends;
		if (ADDS < 5)
		{
			ADDS = 5;
		}

		return ADDS; //WinScoreTable[(SummFrRank*9+SummOpRank)*2];
	}
	else
	{
		return WinScoreTable[(SummFrRank * 9 + SummOpRank) * 2 + 1];
	}
}

int GetLogRank()
{
	if (LOGIN.Logged)
	{
		return LOGIN.Rank;
	}
	else
	{
		return 0;
	}
}

void CheckExistingSaves()
{
	if (!LOGIN.Logged)return;
	//CurrentGame CGM;
	CURIGAME.Active = 0;
	CURIGAME.LoadGameFromFile();
	CURIGAME.Active = 0;
}

__declspec( dllexport ) void ExplorerOpenRef(int Index, char* ref);

void DontMakeRaiting()
{
	if (CURIGAME.Active)
	{
		CURIGAME.DeleteThisGameFromFile();
		CURIGAME.Active = 0;
	}
}

void StopRaiting()
{
	if (CURIGAME.Active)
	{
		CURIGAME.Active = 0;
	}
}

int PBackTime = 0;
void DontMakeRaiting();

void ProcessUpdate()
{
	if (PBackTime == 0)
	{
		PBackTime = GetTickCount() - 100000;
	}

	if (GetTickCount() - PBackTime > 100000)
	{
		if (NPlayers <= 1)
			DontMakeRaiting();

		PBackTime = GetTickCount();

		if (CURIGAME.Active)
			CURIGAME.UpdateGame();
	}
}

//---------------------TOP100-------------------//

struct OneTopClient
{
	int ProfileID;
	int Score;
	int NBattl;
	int NVict;
	int NIncomp;
	int LastRequestTime;

	bool OLD_Profile : 1;
	bool OLD_PLINF : 1;
	bool OLD_PARAM : 1;
};


int TopProfile = 5358612;
// Nick:     T100CEW_3742B
// Password: AV7LxvTuQ01w
//email      top100@top100.com
bool FirstTop100 = 1;
void RewriteTop100(int ProfileID, int Score);

int LASTI = 0;

extern "C" int CALLBACK12 = 0;

int res3 = -1;

int SECCOD0 = 0;
int SECCOD1 = 0;
int SECCOD2 = 0;
int SECCOD3 = 0;

void AddStrToURL(char* url, char* var, char* value, DWORD* HASH)
{
	char VAL[512];
	strcpy(VAL, value);
	int CODE = 0;
	for (size_t i = 0; i < strlen(VAL); i++)
	{
		byte c = VAL[i];
		if (c <= 32 || c == 34 || c == '%' || c == '&' || c == '*'
			|| c == 39 || c == '+' || c == '\\' || c == '/'
			|| c == '<' || c == '>' || c == '?' || c == 0xA0 || c == 0xAD)
			c = '_';
		VAL[i] = c;
		CODE += int(c) * int(c) + SECCOD0;
		CODE &= 65535;
	}

	CODE *= SECCOD1;
	CODE &= 0xFFFF;
	*HASH += CODE * SECCOD2;
	*HASH &= 0xFFFFFF;
	*HASH ^= SECCOD3;
	sprintf(url + strlen(url), "%s=%s&", var, VAL);
}

void AddIntToURL(char* url, char* var, int value, DWORD* HASH)
{
	char cc2[32];
	sprintf(cc2, "%d", value);
	AddStrToURL(url, var, cc2, HASH);
}

void AddIntIdxToURL(char* url, char* var, int value, DWORD* HASH, int idx)
{
	char cc2[32];
	char cc3[32];
	sprintf(cc2, "%d", value);
	sprintf(cc3, "%s%d", var, idx);
	AddStrToURL(url, cc3, cc2, HASH);
}

void AddStrIdxToURL(char* url, char* var, char* value, DWORD* HASH, int idx)
{
	char cc3[32];
	sprintf(cc3, "%s%d", var, idx);
	AddStrToURL(url, cc3, value, HASH);
}

char HISET[100] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char GETCOD(int v)
{
	if (v < 0)
		v = 0;
	if (v > 99)
		v = 99;
	return HISET[v * 60 / 100];
}

bool TestHash1(char*);
void RunHTTPC();

void SendPHPString(CurrentGame* CGM)
{
	GFILE* F = Gopen("Internet\\alternative.dat", "r");
	if (!F)return;
	char ABBREV[256];
	char PHP[256];
	char MODE[128];
	bool r1, r2, r3;
	bool unfound = 1;
	DWORD HASH = 0;
	do
	{
		r1 = ReadWinString(F, ABBREV, 256);
		r2 = ReadWinString(F, MODE, 256);
		r3 = ReadWinString(F, PHP, 256);
		if (r1 && r2 && r3)
		{
			if (strstr(CGM->GameName, ABBREV))unfound = false;
		}
		else return;
	}
	while (unfound);
	SECCOD0 = 0;
	SECCOD1 = 0;
	SECCOD2 = 0;
	SECCOD3 = 0;
	int L = strlen(ABBREV);
	for (int i = 0; i < L; i++)
	{
		SECCOD0 += ABBREV[i];
		SECCOD1 += ABBREV[i] * 2;
		SECCOD2 += ABBREV[i] * 3;
		SECCOD3 += ABBREV[i] * 4;
	};
	for (int i = 0; i < L - 1; i++)
	{
		SECCOD0 += ABBREV[i] * ABBREV[i + 1] * 4;
		SECCOD1 += ABBREV[i] * ABBREV[i + 1] * 3;
		SECCOD2 += ABBREV[i] * ABBREV[i + 1] * 2;
		SECCOD3 += ABBREV[i] * ABBREV[i + 1];
	};
	for (int i = 0; i < L - 2; i++)
	{
		SECCOD0 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * 17;
		SECCOD1 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * 13;
		SECCOD2 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * 11;
		SECCOD3 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2];
	};
	for (int i = 0; i < L - 3; i++)
	{
		SECCOD0 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * ABBREV[i + 3] * 11;
		SECCOD1 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * ABBREV[i + 3] * 21;
		SECCOD2 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * ABBREV[i + 3] * 3;
		SECCOD3 += ABBREV[i] * ABBREV[i + 1] * ABBREV[i + 2] * ABBREV[i + 3];
	};
	SECCOD0 &= 32767;
	SECCOD1 &= 32767;
	SECCOD2 &= 32767;
	SECCOD3 &= 32767;

	Gclose(F);
	char ccc[4096];
	strcpy(ccc, PHP);
	char tmp[256];
	strcpy(tmp, CGM->GameName);
	char* sss = strstr(tmp, " #GUID:");
	if (sss)sss[0] = 0;
	if (tmp[0] == 127)AddStrToURL(ccc, "GNAME", tmp + 5, &HASH);
	else AddStrToURL(ccc, "GNAME", tmp, &HASH);
	//!!!!!!!
	if (sss)AddStrToURL(ccc, "GUID", sss + 7, &HASH);
	else AddStrToURL(ccc, "GUID", "00000000", &HASH);
	//!!!!!!!
	AddStrToURL(ccc, "MAP", CGM->MapName, &HASH);
	AddIntToURL(ccc, "GTIME", CGM->PlayingTime, &HASH);
	char cc5[32];
	sprintf(cc5, "%d,%d,%d,%d,%d", CGM->StartTime.Min, CGM->StartTime.Hour, CGM->StartTime.Day, CGM->StartTime.Month,
	        CGM->StartTime.Year);
	AddStrToURL(ccc, "GSTART", cc5, &HASH);
	AddIntToURL(ccc, "NPLR", CGM->NPL, &HASH);
	int NTM = 0;
	byte TML[8];
	for (int i = 0; i < CGM->NPL; i++)
	{
		AddStrIdxToURL(ccc, "PL", CGM->PLAYERS[i].Nick, &HASH, i);
		AddIntIdxToURL(ccc, "CL", CGM->PLAYERS[i].Color, &HASH, i);
		AddIntIdxToURL(ccc, "NT", CGM->PLAYERS[i].NationID, &HASH, i);
		switch (CGM->PLAYERS[i].State)
		{
		case 0:
			AddStrIdxToURL(ccc, "ST", "V", &HASH, i);
			break;
		case 1:
			AddStrIdxToURL(ccc, "ST", "D", &HASH, i);
			break;
		default:
			AddStrIdxToURL(ccc, "ST", "U", &HASH, i);
			break;
		};
		int mtm = -1;
		byte ms = CGM->PLAYERS[i].Mask;
		for (int j = 0; j < NTM && mtm == -1; j++)if (TML[j] & ms)mtm = j;
		if (mtm == -1)
		{
			TML[NTM] = ms;
			mtm = NTM;
			NTM++;
		};
		AddIntIdxToURL(ccc, "TM", mtm + 1, &HASH, i);
		AddIntIdxToURL(ccc, "score", CGM->PLAYERS[i].Score, &HASH, i);
		AddIntIdxToURL(ccc, "ID", CGM->PLAYERS[i].Profile, &HASH, i);
		if (!strcmp(MODE, "FULL"))
		{
			//population
			AddIntIdxToURL(ccc, "MXP", CGM->PLAYERS[i].MaxPopul, &HASH, i);
			char cc6[256];
			cc6[0] = 0;
			for (int p = 0; p < 32; p++)
			{
				cc6[p] = GETCOD(CGM->PLAYERS[i].Popul[p]);
			};
			cc6[32] = 0;
			AddStrIdxToURL(ccc, "STP", cc6, &HASH, i);
			//score
			AddIntIdxToURL(ccc, "MXS", CGM->PLAYERS[i].MaxScore, &HASH, i);
			cc6[0] = 0;
			for (int p = 0; p < 32; p++)
			{
				cc6[p] = GETCOD(CGM->PLAYERS[i].ScoreG[p]);
			};
			cc6[32] = 0;
			AddStrIdxToURL(ccc, "STS", cc6, &HASH, i);
		};
	};
	sprintf(ccc + strlen(ccc), "SEQ=%d", HASH);
	//TestHash1(ccc);
	FILE* F1 = fopen("Internet\\upload.dat", "a");
	if (F1)
	{
		fprintf(F1, "%s\n", ccc);
		fclose(F1);
		RunHTTPC();
	};
};
bool CheckGameTime();

bool IsGameActive()
{
	return CURIGAME.Active;
}

int GetCurGamePtr(byte** Ptr)
{
	*Ptr = (byte*)&CURIGAME;
	return sizeof CURIGAME;
}

bool CheckLogin()
{
	return LOGIN.Logged;
}

void UpdateCurGame()
{
	if (CURIGAME.Active)
	{
		SYSTEMTIME SYSTM;
		GetSystemTime(&SYSTM);
		CURIGAME.LastLoadTime.Year = SYSTM.wYear;
		CURIGAME.LastLoadTime.Month = SYSTM.wMonth;
		CURIGAME.LastLoadTime.Day = SYSTM.wDay;
		CURIGAME.LastLoadTime.Hour = SYSTM.wHour;
		CURIGAME.LastLoadTime.Min = SYSTM.wMinute;
		CURIGAME.LastLoadTime.DayOfWeek = SYSTM.wDayOfWeek;
		memcpy(&CURIGAME.CurTime, &CURIGAME.LastLoadTime, sizeof CURIGAME.LastLoadTime);
	}
}

bool CheckGameTime()
{
	if (CURIGAME.Active)
	{
		SYSTEMTIME ST;
		memset(&ST, 0, sizeof ST);
		ST.wMinute = CURIGAME.CurTime.Min;
		ST.wHour = CURIGAME.CurTime.Hour;
		ST.wDay = CURIGAME.CurTime.Day;
		ST.wMonth = CURIGAME.CurTime.Month;
		ST.wYear = CURIGAME.CurTime.Year;
		FILETIME FT;
		SystemTimeToFileTime(&ST, &FT);
		SYSTEMTIME ST0;
		memset(&ST0, 0, sizeof ST0);
		ST0.wMinute = CURIGAME.LastLoadTime.Min;
		ST0.wHour = CURIGAME.LastLoadTime.Hour;
		ST0.wDay = CURIGAME.LastLoadTime.Day;
		ST0.wMonth = CURIGAME.LastLoadTime.Month;
		ST0.wYear = CURIGAME.LastLoadTime.Year;
		FILETIME FT0;
		SystemTimeToFileTime(&ST0, &FT0);
		LARGE_INTEGER LI0;
		LARGE_INTEGER LI;
		memcpy(&LI0, &FT0, 8);
		memcpy(&LI, &FT, 8);
		LI.QuadPart -= LI0.QuadPart;
		memcpy(&FT, &LI, 8);

		int dt = 0;
		if (FileTimeToSystemTime(&FT, &ST))
		{
			dt = (ST.wDay - 1) * 24 * 60 + ST.wHour * 60 + ST.wMinute;
		}
		return dt >= 10;
	}
	else
	{
		return false;
	}
}

void SetNormAttr()
{
	HANDLE H = CreateFile("cew.dll",
	                      GENERIC_WRITE, FILE_SHARE_READ, NULL,
	                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (H == INVALID_HANDLE_VALUE)
	{
		return;
	}

	FILETIME F1, F2, F3;
	GetFileTime(H, &F1, &F2, &F3);
	F3 = F1;
	F2 = F2;
	SetFileTime(H, &F1, &F2, &F3);
	CloseHandle(H);
}

bool TestHash1(char* s);

bool TestHash1(char* s)
{
	char ccc[4096];
	strcpy(ccc, s);
	char* cc = strstr(ccc, "?");
	if (!cc)
	{
		return false;
	}

	cc++;
	char VAR[48];
	char VALUE[256];
	DWORD HASH = 0;
	char URL[4096] = "";
	bool FINAL = 0;
	DWORD FINHASH;
	do
	{
		char* cc3 = strstr(cc, "=");
		if (cc3)
		{
			int L = cc3 - cc;
			memcpy(VAR, cc, L);
			VAR[L] = 0;
			char* cc4 = strstr(cc3, "&");
			if (cc4)
			{
				L = cc4 - cc3 - 1;
				memcpy(VALUE, cc3 + 1, L);
				VALUE[L] = 0;
				AddStrToURL(URL, VAR, VALUE, &HASH);
				cc = cc4 + 1;
			}
			else
			{
				FINAL = 1;
				int z = sscanf(cc3 + 1, "%d", &FINHASH);
			}
		}
	}
	while (!FINAL);
	return true;
}

int GetGSC_Profile();

//Returns GetGSC_Profile()
int GetMyProfile()
{
	return GetGSC_Profile();
}

void SetBrokenState()
{
	if (CURIGAME.Active)
	{
		CURIGAME.Broken = 1;
	}
}

#include "http\HttpComm.h"

int T0 = 0;
char CLANR[32] = "";
char PLNICK[32] = "";

CHttpComm HTTPC;

DWORD SendHTTPRequest(char* URL)
{
	return HTTPC.AddRequest(URL);
}

bool CheckHTTPAnswer(DWORD Handle, int* size, byte* Data)
{
	*size = 0;
	try
	{
		HTTPC.ProcessRequests();
		*size = HTTPC.GetData(Handle, (void*)Data, *size);
	}
	catch (...)
	{
	}

	return (*size) != 0;
}

bool RejectThisPlayer = 0;
DWORD CHECK_HANDLE = 0;
DWORD SERV_HANDLES[6];
int NSERV = 0;

DWORD SendOneRequest(char* serv, char* param)
{
	char ccc[512];
	sprintf(ccc, "%s?%s", serv, param);
	try
	{
		return SendHTTPRequest(ccc);
	}
	catch (...)
	{
		return 0;
	}
}

int GetStrHASH1(char* s)
{
	int L = strlen(s);
	DWORD S = 0;
	for (int i = 0; i < L; i++)
	{
		S += s[i];
	}

	for (int i = 0; i < L - 1; i++)
	{
		S += int(s[i]) * int(s[i + 1]);
	}

	for (int i = 0; i < L - 2; i++)
	{
		S -= int(s[i]) * int(s[i + 1]) * int(s[i + 2]);
	}

	for (int i = 0; i < L - 3; i++)
	{
		S += int(s[i]) * int(s[i + 1]) * int(s[i + 2]) * int(s[i + 3]);
	}

	for (int i = 0; i < L - 4; i++)
	{
		S -= int(s[i]) * int(s[i + 1]) * int(s[i + 2]) * int(s[i + 3]) * int(s[i + 4]);
	}

	for (int i = 0; i < L - 5; i++)
	{
		S += int(s[i]) * int(s[i + 1]) * int(s[i + 2]) * int(s[i + 3]) * int(s[i + 4]) * int(s[i + 5]);
	}
	return S;
}

void SendAllRequests(char* clan, char* nick, char* mail)
{
	RejectThisPlayer = 0;
	char ccc[512];
	sprintf(ccc, "com=check&clan=%s&user=%s&mail=%s&code=%d", clan, nick, mail, GetTickCount() & 65535);
	NSERV = 0;

	GFILE* F = Gopen("Internet\\serv.dat", "r");
	if (F)
	{
		char cc[256];
		int z = 0;
		do
		{
			int hash = 0;
			z = Gscanf(F, "%s%d", cc, &hash);
			if (z == 2 && NSERV < 6)
			{
				if (hash == GetStrHASH1(cc))
				{
					SERV_HANDLES[NSERV] = SendOneRequest(cc, ccc);
					NSERV++;
				}
			}
		}
		while (z == 2 && NSERV < 6);
		Gclose(F);
	}

	if (!NSERV)
	{
		SERV_HANDLES[NSERV] = SendOneRequest("www.gsc-game.net/games/Cossacks-AW/scripts/checkuser.php", ccc);
		NSERV++;
	}

	CHECK_HANDLE = 1;
}

char CLANX[256];

void WaitWithMessage(char* Message);


bool CheckPlayerToExit()
{
	if (CHECK_HANDLE)
	{
		for (int i = 0; i < NSERV; i++)
		{
			char STR[4096];
			int sz = 4096;
			if (!T0)T0 = GetTickCount();
			if (CheckHTTPAnswer(SERV_HANDLES[i], &sz, (byte*)STR))
			{
				STR[255] = 0;
				int HC1 = 0;
				int HC2 = 0;
				int zz = sscanf(STR, "%d%d", &HC1, &HC2);
				if (zz == 2)
				{
					if ((HC1 + 37) * 13 == HC2)
					{
						LocalGP BOR2("Interface\\bor2");
						sprintf(STR, GetTextByID("NO_CLAN_MEMBER_[%s]"), CLANX);
						WaitWithError4(STR, BOR2.GPID);
						HTTPC.FreeData(SERV_HANDLES[i]);
						CHECK_HANDLE = 0;
						T0 = 0;
						RejectThisPlayer = 1;
						return true;
					}
					else
					{
						if ((HC1 + 97) * 17 == HC2)
						{
							HTTPC.FreeData(SERV_HANDLES[i]);
							CHECK_HANDLE = 0;
							T0 = 0;
							return false;
						}
					}
				}
			}
		}
	}
	return false;
}
