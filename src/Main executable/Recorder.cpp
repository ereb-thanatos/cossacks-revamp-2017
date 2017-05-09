#include "ddini.h"
#include <stdlib.h>
#include "ResFile.h"
#include "Fastdraw.h"
#include "MapDiscr.h"
#include "mouse.h"
#include "mode.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <crtdbg.h>
#include <math.h>

#include "Megapolis.h"

#include "fonts.h"
#include "WeaponID.h"

#include "3DSurf.h"
#include "GSound.h"
#include <assert.h>
#include "3DMapEd.h"
#include "MapSprites.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "ZBuffer.h"
#include "NewAI.h"
#include "TopoGraf.h"
#include "StrategyResearch.h"
#include "Curve.h"
#include "NewMon.h"
#include "ActiveScenary.h"
#include "ConstStr.h"
#include "DrawForm.h"
#include "IconTool.h"
#include "Dialogs.h"
#include "Path.h"
#include "Sort.h"
#include "Recorder.h"


#include "PlayerInfo.h"
extern PlayerInfo PINFO[8];

extern const int kChatMessageDisplayTime;

//--------------------inserted from DIPLOMACY.H---------------//
typedef void tpStartDownloadInternetFile( char* Name, char* Server, char* DestName );
typedef void tpProcessDownloadInternetFiles();
typedef void tpSendRecBuffer( byte* Data, int size, bool Final );

extern tpStartDownloadInternetFile* StartDownloadInternetFile;
extern tpProcessDownloadInternetFiles* ProcessDownloadInternetFiles;
extern tpSendRecBuffer* SendRecBuffer;
//-----------------------------------------------------------//
//#include "diplomacy.h"
byte RecFormatID = 0;

RecordGame RGAME;
//db command
//dw mapx
//dw mapy
//dw size
//db execute data...

extern byte ExBuf[8192];
extern int EBPos;

RecordGame::RecordGame()
{
	Stream = nullptr;
	Size = 0;
	Pos = 0;
	MaxSize = 0;
	MapName[0] = 0;
	RecName[0] = 0;
}

RecordGame::~RecordGame()
{
	if ( Stream )
	{
		free( Stream );
	}
}

void RecordGame::AddByte( byte v )
{
	if ( Size >= MaxSize )
	{
		MaxSize += 65536;
		Stream = (byte*) realloc( Stream, MaxSize );
	}
	Stream[Size] = v;
	Size++;
}

void RecordGame::AddShort( short v )
{
	if ( Size >= MaxSize - 1 )
	{
		MaxSize += 65536;
		Stream = (byte*) realloc( Stream, MaxSize );
	}
	*( (short*) ( Stream + Size ) ) = v;
	Size += 2;
}

void RecordGame::AddBuf( byte* Buf, int sz )
{
	if ( Size >= MaxSize - sz )
	{
		MaxSize += 65536;
		Stream = (byte*) realloc( Stream, MaxSize );
	}
	memcpy( Stream + Size, Buf, sz );
	Size += sz;
}

char LASTCHATSTR[512];
byte CHOPT = 0;
void RecordGame::AddRecord()
{
	/*
		AddByte(0);
		AddShort(mapx+(smaplx>>1));
		AddShort(mapy+(smaply>>1));
		AddShort(EBPos);
		AddBuf(ExBuf,EBPos);
	*/

	if ( LASTCHATSTR[0] )
	{
		AddByte( 6 );
	}
	else
	{
		AddByte( 1 );
	}
	AddShort( rpos );
	AddShort( mapx + ( smaplx / 2 ) );
	AddShort( mapy + ( smaply / 2 ) );
	AddShort( EBPos );
	AddBuf( ExBuf, EBPos );
	if ( LASTCHATSTR[0] )
	{
		AddByte( 5 );
		AddByte( (byte) strlen( LASTCHATSTR ) );
		AddByte( CHOPT );
		AddBuf( (byte*) LASTCHATSTR, strlen( LASTCHATSTR ) + 1 );
		LASTCHATSTR[0] = 0;
	}
}

void RecordGame::AddEmptyRecord()
{
	AddByte( 0 );
	AddShort( mapx + ( smaplx >> 1 ) );
	AddShort( mapy + ( smaply >> 1 ) );
	AddShort( 0 );
}

extern byte PlayGameMode;
int LastScrollTime = 0;
int CurrentAnswer = 0;
bool AUTOSCROLL = 1;
extern int SeqErrorsCount;
extern bool RecordMode;
bool RecordGame::Extract()
{
	if ( PlayGameMode && RecordMode )
	{
		RecordMode = 0;
	}
	AUTOSCROLL = 1;
	if ( PlayGameMode == 2 && LastScrollTime && GetTickCount() - LastScrollTime < 10000 )
	{
		AUTOSCROLL = 0;
	}
	if ( AUTOSCROLL )
	{
		for ( int i = 0; i < 8; i++ )
		{
			if ( ImNSL[i] )
			{
				int N = ImNSL[i];
				for ( int j = 0; j < N; j++ )
				{
					word MID = ImSelm[i][j];
					if ( MID != 0xFFFF )
					{
						OneObject* OB = Group[MID];
						if ( OB )
						{
							OB->ImSelected = 0;
						}
					}
				}
			}

			if ( NSL[i] )
			{
				ImNSL[i] = NSL[i];
				ImSelm[i] = (word*) realloc( ImSelm[i], NSL[i] * 2 );
				ImSerN[i] = (word*) realloc( ImSerN[i], NSL[i] * 2 );
				memcpy( ImSelm[i], Selm[i], NSL[i] * 2 );
				memcpy( ImSerN[i], SerN[i], NSL[i] * 2 );
			}
			else
			{
				ImNSL[i] = 0;
				if ( ImSelm[i] )
				{
					free( ImSelm[i] );
				}
				if ( ImSerN[i] )
				{
					free( ImSerN[i] );
				}
				ImSelm[i] = NULL;
				ImSerN[i] = NULL;
			}
			int N = ImNSL[i];
			for ( int j = 0; j < N; j++ )
			{
				word MID = ImSelm[i][j];
				if ( MID != 0xFFFF )
				{
					OneObject* OB = Group[MID];
					if ( OB )
					{
						OB->ImSelected = OB->Selected;
					}
				}
			}
		}
	}

	byte RBUF[16384];
	int sz = 0;
	int szz = -1;
	if ( Pos >= Size - 2048 )
	{
		do
		{
			szz = STREAM.ReadSomething( RBUF + sz, 4096, 1, 1 );
			if ( szz != -1 )
			{
				sz += szz;
			}
			//if(szz==-1)return false;
		} while ( szz != -1 && sz < 4096 && !STREAM.Error() );

		Stream = (byte*) realloc( Stream, Size + sz );
		memcpy( Stream + Size, RBUF, sz );
		Size += sz;
	}

	if ( Pos < Size )
	{
		if ( Stream[Pos] == 0 )
		{
			//return false;
			Pos++;
			if ( AUTOSCROLL )
			{
				mapx = *( (short*) ( Stream + Pos ) ) - ( smaplx / 2 );
			}
			Pos += 2;
			if ( AUTOSCROLL )
			{
				mapy = *( (short*) ( Stream + Pos ) ) - ( smaply / 2 );
			}
			Pos += 2;
			if ( mapx <= 0 )
			{
				mapx = 1;
			}
			if ( mapy <= 0 )
			{
				mapy = 1;
			}
			if ( mapx + smaplx > msx + 1 )
			{
				mapx = msx - smaplx + 1;
			}
			if ( mapy + smaply > msy + 1 )
			{
				mapy = msy - smaply + 1;
			}
			EBPos = *( (short*) ( Stream + Pos ) );
			if ( EBPos < 0 || EBPos > 1024 )
			{
				return false;
			}
			Pos += 2;
			memcpy( ExBuf, Stream + Pos, EBPos );
			Pos += EBPos;
			return true;
		}
		else
		{
			if ( Stream[Pos] == 1 || Stream[Pos] == 6 )
			{
				bool CHAT = Stream[Pos] == 6;
				Pos++;
				if ( Pos >= Size )
				{
					return false;
				}

				short rr = *( (short*) ( Stream + Pos ) );
				if ( rr != rpos && PlayGameMode == 2 )
				{
					SeqErrorsCount = 1;
				}

				Pos += 2;
				if ( Pos >= Size )
				{
					return false;
				}
				if ( AUTOSCROLL )
				{
					mapx = *( (short*) ( Stream + Pos ) ) - ( smaplx / 2 );
				}
				Pos += 2;
				if ( Pos >= Size )
				{
					return false;
				}
				if ( AUTOSCROLL )
				{
					mapy = *( (short*) ( Stream + Pos ) ) - ( smaply / 2 );
				}
				Pos += 2;
				if ( Pos >= Size )
				{
					return false;
				}
				if ( mapx <= 0 )
				{
					mapx = 1;
				}
				if ( mapy <= 0 )
				{
					mapy = 1;
				}
				if ( mapx + smaplx > msx + 1 )
				{
					mapx = msx - smaplx + 1;
				}
				if ( mapy + smaply > msy + 1 )
				{
					mapy = msy - smaply + 1;
				}
				if ( Pos >= Size )
				{
					return false;
				}

				EBPos = *( (short*) ( Stream + Pos ) );
				assert( EBPos < 2048 );
				if ( EBPos < 0 || EBPos > 4096 )
				{
					return false;
				}

				Pos += 2;
				if ( Pos + EBPos > Size )
				{
					return false;
				}

				//Write commands from rec stream to the execution buffer
				memcpy( ExBuf, Stream + Pos, EBPos );
				Pos += EBPos;

				if ( CHAT )
				{
					int L = Stream[Pos + 1];
					byte c = Stream[Pos + 2];
					Pos += 3;
					CreateTimedHintEx( (char*) ( Stream + Pos ), kChatMessageDisplayTime, c );//Chat message
					Pos += L + 1;
				}
				return true;
			}
			else
			{
				if ( Stream[Pos] == 3 )
				{
					CurrentAnswer = Stream[Pos + 1];
					Pos += 2;
				}
				else
				{
					if ( Stream[Pos] == 2 )
					{
						SetMyNation( Stream[Pos + 1] );
						memcpy( NatRefTBL, Stream + Pos + 2, 8 );
						Pos += 10;
						return true;
					}
					else
					{
						return false;
					}
				}
			}
		}
	}
	else
	{
		STREAM.Close();
		return false;
	}
	return false;
}

extern int PitchTicks;
void WritePitchTicks()
{
	RGAME.AddByte( PitchTicks );
}

void ReadPichTicks()
{
	if ( RecFormatID == 1 )
	{
		if ( RGAME.Pos >= RGAME.Size )
		{
			byte RBUF[4096];
			int sz = 0;
			int szz = -1;
			do
			{
				szz = RGAME.STREAM.ReadSomething( RBUF, 4096, 1, 1 );
				if ( szz != -1 )
				{
					sz += szz;
				}
			} while ( szz != -1 && sz < 4096 && !RGAME.STREAM.Error() );

			RGAME.Stream = (byte*) realloc( RGAME.Stream, RGAME.Size + sz );
			memcpy( RGAME.Stream + RGAME.Size, RBUF, sz );
			RGAME.Size += sz;
		}
		PitchTicks = RGAME.Stream[RGAME.Pos];
		RGAME.Pos++;
	}
	else
	{
		PitchTicks = 0;
	}
}

int RunMethod = 0;
extern char RECFILE[128];

void RecordGame::StartRecord( char* Map )
{
	strcpy( MapName, Map );
	if ( RECFILE[0] )
	{
		strcpy( RecName, RECFILE );
	}
	else
	{
		strcpy( RecName, Map );
	}

	char* s = strstr( RecName, ".m3d" );
	if ( s )
	{
		strcpy( s, ".rec" );
	}
	else
	{
		strcat( RecName, ".rec" );
	}

	Pos = 0;
	Size = 0;
}

bool RecordMode = 0;
extern char CurrentMap[64];
void CreateNationalMaskForMap( char* Name );
void ShowLoading();
void PrepareGameMedia( byte myid, bool );
void InitGame();
extern City CITY[8];
void UnPress();
void CreateNationalMaskForRandomMap( char* );
extern char MapScenaryDLL[200];
int DetermineNationAI( byte Nat );
char* GetTextByID( char* ID );
void LoadAIFromDLL( byte Nat, char* Name );
extern byte PlayGameMode;
extern int tmtmt;
extern int REALTIME;
extern int Inform;
extern int RunDataSize;
extern byte RunData[2048];
extern word COMPSTART[8];
extern int RM_LandType;
extern int RM_Resstart;
extern int RM_Restot;

void RecordGame::ReadStream( char* Name )
{
	Inform = 0;
	SetMyNation( 0 );
	PINFO[0].VictCond = 0;
	NATIONS[0].VictState = 0;
	SCENINF.LooseGame = 0;
	SCENINF.Victory = 0;
	FrmDec = 2;
	SpeedSh = 1;
	Size = 0;
	Pos = 0;
	rpos = 0;
	tmtmt = 0;
	REALTIME = 0;

	if ( STREAM.Open( Name ) )
	{
		//int sz=RFileSize(F);
		byte BUFF[16384];
		int BFSZ = 0;
		int BFPOS = 0;

		do
		{
			int sz = STREAM.ReadSomething( BUFF + BFSZ, 4096, 0, 1 );
			if ( sz != -1 )
			{
				BFSZ += sz;
			}
		} while ( BFSZ < 4 || STREAM.Error() );

		int SIGN;
		//RBlockRead(F,&SIGN,4);
		memcpy( &SIGN, BUFF, 4 );
		BFPOS += 4;
		if ( SIGN != '[CER' || STREAM.Error() )
		{
			if ( SIGN == ']CER' )
			{
				RecFormatID = 1;
			}
			else
			{
				//RClose(F);
				STREAM.Close();
				return;
			}
		}
		else
		{
			RecFormatID = 0;
		}

		do
		{
			int sz = STREAM.ReadSomething( BUFF + BFSZ, 4096, 0, 1 );
			if ( sz != -1 )
			{
				BFSZ += sz;
			}
		} while ( BFSZ < 4 + 64 + 1 + 4 + 4 || STREAM.Error() );

		if ( STREAM.Error() )
		{
			STREAM.Close();
			return;
		}

		//RBlockRead(F,CurrentMap,64);
		memcpy( CurrentMap, BUFF + BFPOS, 64 );
		BFPOS += 64;
		strcpy( MapName, CurrentMap );
		//RBlockRead(F,&SIGN,1);
		memcpy( &SIGN, BUFF + BFPOS, 1 );
		BFPOS++;
		//RBlockRead(F,&RunMethod,4);
		memcpy( &RunMethod, BUFF + BFPOS, 4 );
		BFPOS += 4;
		if ( RunMethod == 0xFFFFFFFF )
		{
			//RBlockRead(F,&RunDataSize,4);
			memcpy( &RunDataSize, BUFF + BFPOS, 4 );
			BFPOS += 4;
			while ( BFSZ < BFPOS + RunDataSize || STREAM.Error() )
			{
				int sz = STREAM.ReadSomething( BUFF + BFSZ, 4096, 0, 1 );
				if ( sz != -1 )
				{
					BFSZ += sz;
				}
			}
			if ( STREAM.Error() )
			{
				STREAM.Close();
				return;
			}
			//RBlockRead(F,&RunData,RunDataSize);
			memcpy( &RunData, BUFF + BFPOS, RunDataSize );
			BFPOS += RunDataSize;
		}

		if ( RunMethod == 0xFFFFFFFE )
		{
			//RBlockRead(F,&RunDataSize,4);
			memcpy( &RunDataSize, BUFF + BFPOS, 4 );
			BFPOS += 4;
			while ( BFSZ < BFPOS + RunDataSize || STREAM.Error() )
			{
				int sz = STREAM.ReadSomething( BUFF + BFSZ, 4096, 0, 1 );
				if ( sz != -1 )
				{
					BFSZ += sz;
				}
			}
			if ( STREAM.Error() )
			{
				STREAM.Close();
				return;
			}
			//RBlockRead(F,&RunData,RunDataSize);
			memcpy( &RunData, BUFF + BFPOS, RunDataSize );
			BFPOS += RunDataSize;
		}

		//sz-=73;
		//if(sz>MaxSize){
		//	MaxSize=sz;
		//	Stream=(byte*)realloc(Stream,MaxSize);
		//};
		//BFSZ-=73;
		//RBlockRead(F,Stream,sz);
		Stream = (byte*) realloc( Stream, BFSZ - BFPOS );
		memcpy( Stream, BUFF + BFPOS, BFSZ - BFPOS );
		Size = BFSZ - BFPOS;//sz;
		Pos = 0;
		//RClose(F);
		ShowLoading();
		int RM = RunMethod >> 24;
		PlayGameMode = 1;
		RecordMode = 0;
		if ( RunMethod == 0xFFFFFFFF )
		{
			//RunData[0]=100;//version of the run data
			//RunData[1]=dwVersion;//version of the game
			int rdVer = RunData[0];
			SetMyNation( RunData[2] );
			RunDataSize = 3;
			memcpy( PINFO, RunData + RunDataSize, sizeof PINFO );
			RunDataSize += sizeof PINFO;
			memcpy( COMPSTART, RunData + RunDataSize, sizeof COMPSTART );
			RunDataSize += sizeof COMPSTART;
			memcpy( &RM_LandType, RunData + RunDataSize, 4 );
			RunDataSize += 4;
			memcpy( &RM_Resstart, RunData + RunDataSize, 4 );
			RunDataSize += 4;
			memcpy( &RM_Restot, RunData + RunDataSize, 4 );
			RunDataSize += 4;
			CreateNationalMaskForMap( CurrentMap );
			PrepareGameMedia( 0, 0 );
			InitGame();
		}
		else
		{
			if ( RunMethod == 0xFFFFFFFE )
			{
				//RunData[0]=100;//version of the run data
				//RunData[1]=dwVersion;//version of the game
				int rdVer = RunData[0];
				SetMyNation( RunData[2] );
				RunDataSize = 3;
				memcpy( PINFO, RunData + RunDataSize, sizeof PINFO );
				RunDataSize += sizeof PINFO;
				memcpy( COMPSTART, RunData + RunDataSize, sizeof COMPSTART );
				RunDataSize += sizeof COMPSTART;
				memcpy( &RM_LandType, RunData + RunDataSize, 4 );
				RunDataSize += 4;
				memcpy( &RM_Resstart, RunData + RunDataSize, 4 );
				RunDataSize += 4;
				memcpy( &RM_Restot, RunData + RunDataSize, 4 );
				RunDataSize += 4;
				memcpy( NatRefTBL, RunData + RunDataSize, 8 );
				RunDataSize += 8;
				CreateNationalMaskForMap( CurrentMap );
				byte MNAT = MyNation;
				PrepareGameMedia( 0, 1 );
				InitGame();
				SetMyNation( MNAT );
			}
			else
			{
				if ( RM == 0x12 )
				{
					int n = RunMethod & 0xFF;
					int Diff = ( RunMethod & 0xFFFF ) >> 8;
					ShowLoading();
					strcpy( CurrentMap, MISSLIST.MISS[n].MapName );
					CreateNationalMaskForMap( CurrentMap );
					PrepareGameMedia( 0, 0 );
					InitGame();
					//InitExplosions();
					for ( int i = 0; i < 7; i++ )
					{
						CITY[i].Difficulty = Diff;
						PINFO[i].ColorID = i;
						if ( i )
						{
							strcpy( PINFO[i].name, "???" );
						}
					}
					//strcpy(PINFO[0].name,PlName);
					PINFO[0].GameTime = 0;
					PINFO[0].VictCond = 0;
					MISSLIST.CurrentMission = n;
					SCENINF.Load( MISSLIST.MISS[n].DLLPath, MISSLIST.MISS[n].Description );
					UnPress();
				}
				else
				{
					if ( RM == 0x23 )
					{
						int RM_Resstart = ( RunMethod & 0xF );
						int	RM_Restot = ( RunMethod >> 4 ) & 0xF;
						int CurLine = ( RunMethod >> 8 ) & 0xF;
						int AI_Style = ( RunMethod >> 12 ) & 0xF;
						ShowLoading();
						bool RunAI = true;
						if ( strstr( CurrentMap, "RND " ) )
						{
							CreateNationalMaskForRandomMap( CurrentMap );
						}
						else
						{
							CreateNationalMaskForMap( CurrentMap );
						}
						CurrentCampagin = -1;
						CurrentMission = -1;
						PrepareGameMedia( 0, 0 );
						if ( MapScenaryDLL[0] )
						{
							RunAI = 0;
						}
						InitGame();
						//InitExplosions();
						//PrepareToGame();
						for ( int k = 1; k < 7; k++ )
						{
							int nidx = DetermineNationAI( k );
							if ( nidx != -1 )
							{//strcmp(PINFO[k].name,NoPlayer)){
								char ccx[64];
								Nation* NT = NATIONS + k;
								int MaxA = -1;
								char* NtID = GlobalAI.Ai[nidx/*MNATION[k]->CurLine*/].NationID;
								int Land = CurLine;
								for ( int w = 0; w < 10; w++ )
								{
									sprintf( ccx, "%s.%d", NtID, w );
									char* AIN = GetTextByID( ccx );
									if ( AIN != ccx )MaxA = w;
								}
								if ( MaxA == -1 )
								{
									sprintf( ccx, "Undefined AI identifier (see miss_AI.txt) : %s.%d", NtID, 0 );
									ErrM( ccx );
								}
								if ( RunAI )
								{
									sprintf( ccx, "%s.%d", NtID, ( ( MaxA + 1 )*int( rando() ) ) >> 15 );
									LoadAIFromDLL( k, GetTextByID( ccx ) );
								}
								strcpy( NT->CITY->AIID, ccx );
								NT->CITY->MAX_WORKERS = NT->DEFAULT_MAX_WORKERS;
								NT->CITY->StartRes = RM_Resstart;//CMGRP1[2]->CurLine;
								NT->CITY->ResOnMap = RM_Restot;//CMGRP1[3]->CurLine;
								NT->CITY->Difficulty = CurLine;
								NT->CITY->LandType = AI_Style;//RMP.STY[CMGRP1[0]->CurLine].AI_Style;
								if ( RunAI )
								{
									NT->AI_Enabled = true;
									NT->NMask = 0x7E;
									for ( int i = 0; i < MAXOBJECT; i++ )
									{
										OneObject* OB = Group[i];
										if ( OB&&OB->NNUM == k&&OB->BrigadeID == 0xFFFF )
										{
											NT->CITY->RegisterNewUnit( OB );
											OB->NMask = 0x7E;
										}
									}
								}
							}
						}
					}
					else
					{
						ShowLoading();
						CreateNationalMaskForMap( CurrentMap );
						PrepareGameMedia( 0, 0 );
						InitGame();
						//InitExplosions();
					}
				}
			}
		}
	}
}

int RunSize0 = 0;
int RunDataSize = 0;
byte RunData[2048];
void RecordGame::Save()
{
	if ( RecName[0] )
	{
		RGAME.TryToFlushNetworkStream( 1 );
		ResFile F = RRewrite( RecName );
		if ( F != INVALID_HANDLE_VALUE )
		{
			int SIGN = ']CER';
			RBlockWrite( F, &SIGN, 4 );
			RBlockWrite( F, MapName, 64 );
			SIGN = ']';
			RBlockWrite( F, &SIGN, 1 );
			RBlockWrite( F, &RunMethod, 4 );
			if ( RunMethod == 0xFFFFFFFF )
			{
				RBlockWrite( F, &RunDataSize, 4 );
				RBlockWrite( F, &RunData, RunDataSize );
			}
			if ( RunMethod == 0xFFFFFFFE )
			{
				RBlockWrite( F, &RunDataSize, 4 );
				RBlockWrite( F, &RunData, RunDataSize );
			}
			if ( Size )
			{
				RBlockWrite( F, Stream, Size );
			}
			RClose( F );
		}
		RGAME.STREAM.Close();
		if ( RGAME.Stream )
		{
			free( RGAME.Stream );
		}
		RGAME.Stream = nullptr;
		RGAME.Size = 0;
		RGAME.MaxSize = 0;
		RGAME.Pos = 0;
		RGAME.LastSentPos = 0;
		RGAME.RecName[0] = 0;
		RGAME.MapName[0] = 0;
	}
}

void RecordGame::TryToFlushNetworkStream( bool Final )
{
	if ( !RecordMode )
	{
		return;
	}

	if ( Size - LastSentPos > 500 || Final )
	{
		if ( SendRecBuffer )
		{
			if ( !LastSentPos )
			{
				const int buf_size = 16384;// 4096; //BUGFIX: increase buffer size to prevent overflows
				byte TMPB[buf_size];
				int SIGN = ']CER';
				memcpy( TMPB, &SIGN, 4 );
				memcpy( TMPB + 4, MapName, 64 );
				SIGN = ']';
				memcpy( TMPB + 4 + 64, &SIGN, 1 );
				memcpy( TMPB + 4 + 64 + 1, &RunMethod, 4 );
				memcpy( TMPB + 4 + 64 + 1 + 4, &RunDataSize, 4 );
				memcpy( TMPB + 4 + 64 + 1 + 4 + 4, &RunData, RunDataSize );
				RunSize0 = 4 + 64 + 1 + 4 + 4 + RunDataSize;
				if ( Size >= buf_size - RunSize0 )
				{
					Size = buf_size - RunSize0 - 1;//BUGFIX: prevent buffer overflow and cut the stream short
				}
				memcpy( TMPB + RunSize0, Stream, Size );
				SendRecBuffer( TMPB, Size + RunSize0, Final );
			}
			else
			{
				SendRecBuffer( Stream + LastSentPos, Size - LastSentPos, Final );
			}
		}
		LastSentPos = Size;
	}
}

void PlayGame();
void RecordGame::RunRecordedGame( char* Name )
{
	RGAME.ReadStream( Name );
	PlayGameMode = 2;
	PlayGame();
}

void GetFHashName( char* pass, char* Dst )
{
	int L = strlen( pass );
	DWORD S = 0;
	for ( int i = 0; i < L; i++ )S += pass[i];
	for ( int i = 0; i < L - 1; i++ )S += pass[i] * pass[i + 1];
	for ( int i = 0; i < L - 2; i++ )S += pass[i] * pass[i + 1] * pass[i + 2];
	for ( int i = 0; i < L - 3; i++ )S += pass[i] * pass[i + 1] * pass[i + 2] * pass[i + 3];
	for ( int i = 0; i < L - 4; i++ )S += pass[i] * pass[i + 1] * pass[i + 2] * pass[i + 3] * pass[i + 4];
	for ( int i = 0; i < L - 5; i++ )S += pass[i] * pass[i + 1] * pass[i + 2] * pass[i + 3] * pass[i + 4] * pass[i + 5];
	sprintf( Dst, "%X", S );
};

bool InternetStream::Open( char* Name )
{
	if ( Name[2] == '|' )
	{
		memcpy( Server, Name, 2 );
		Server[2] = 0;
		strcpy( StreamName, Name + 3 );
		GetFHashName( Name, HashName );
		if ( StartDownloadInternetFile )StartDownloadInternetFile( StreamName, Server, HashName );
		CurPosition = 0;
		LocalFile = 0;
		return true;
	}
	else
	{
		ResFile F = RReset( Name );
		if ( F != INVALID_HANDLE_VALUE )
		{
			strcpy( StreamName, Name );
			LocalFile = 1;
			CurPosition = 0;
			RClose( F );
			return true;
		};
		return false;
	};
};
void ProcessScreen();
bool ProcessMessages();
void GSYSDRAW();
char* DOWNLOADING = NULL;
void ShowCentralMessage4( char* Message, int GPIDX, int DX );
extern int LastKey;
extern int CurPalette;
extern int HISPEED;
extern bool SHOWSLIDE;
int InternetStream::ReadSomething( byte* Buf, int MaxLen, bool Scroll, bool Blocking )
{
	if ( LocalFile )
	{
		ResFile F = RReset( StreamName );
		if ( F != INVALID_HANDLE_VALUE )
		{
			RSeek( F, CurPosition );
			int sz = RFileSize( F );
			int max = sz - CurPosition;
			if ( max > MaxLen )max = MaxLen;
			RBlockRead( F, Buf, max );
			RClose( F );
			CurPosition += max;
			if ( max )return max;
			else return -1;
		}
		else return -1;
	};
	if ( !HashName[0] )return -1;
	bool somethingdone = 0;
	bool ready = 0;
	int bytesread = 0;
	if ( !DOWNLOADING )DOWNLOADING = GetTextByID( "DOWNLOADING" );
	int T0 = GetTickCount();
	do
	{
		char HNM[128];
		sprintf( HNM, "Internet\\dnl\\%s.open", HashName );
		ResFile F = INVALID_HANDLE_VALUE;
		F = RReset( HNM );
		if ( F == INVALID_HANDLE_VALUE )
		{
			sprintf( HNM, "Internet\\dnl\\%s.ready", HashName );
			F = RReset( HNM );
			if ( F != INVALID_HANDLE_VALUE )ready = 1;
		};
		if ( F != INVALID_HANDLE_VALUE )
		{
			int sz = RFileSize( F );
			if ( sz > CurPosition )
			{
				RSeek( F, CurPosition );
				if ( sz - CurPosition > MaxLen )
				{
					RBlockRead( F, Buf, MaxLen );
					bytesread = MaxLen;
					CurPosition += MaxLen;
					ready = 0;
				}
				else
				{
					RBlockRead( F, Buf, sz - CurPosition );
					bytesread = sz - CurPosition;
					CurPosition += bytesread;
					if ( !bytesread )
					{
						RClose( F );
						return 0;
					};
				};
			}
			else
			{
				if ( ready )
				{
					RClose( F );
					return -1;
				};
			};
		};
		RClose( F );
		if ( Blocking && !somethingdone )
		{
			if ( Scroll )
			{
				int SPD = HISPEED;
				HISPEED = 0;
				SHOWSLIDE = true;
				ProcessScreen();
				int TT = GetTickCount();
				if ( TT - T0 > 2000 )
				{
					if ( CurPalette == 2 )
					{
						LocalGP BOR2( "Interface\\bor2" );
						ShowCentralMessage4( DOWNLOADING, BOR2.GPID, 0 );
					}
					else ShowCentralMessage4( DOWNLOADING, 0, 0 );
					if ( LastKey == 27 || TT - T0 > 60000 )
					{
						return -1;
					};
				};
				ProcessMessages();
				GSYSDRAW();
				HISPEED = SPD;
			}
			else
			{
				ProcessMessages();
				int SPD = HISPEED;
				HISPEED = 0;
				GSYSDRAW();
				HISPEED = SPD;
			};
		};
	} while ( ( !( bytesread || ready ) ) && Blocking );

	return bytesread;
}

void InternetStream::Close()
{
	memset( this, 0, sizeof *this );
}

//Returns 0
int InternetStream::Error()
{
	return 0;
}

void UnLoading();
extern word PlayerMenuMode;
extern int ItemChoose;
extern bool use_gsc_network_protocol;

__declspec( dllexport ) void PlayRecfile( char* RecFile )
{
	int LOG = use_gsc_network_protocol;
	ShowLoading();
	FlipPages();
	RGAME.RunRecordedGame( RecFile );
	LoadPalette( "2\\agew_1.pal" );
	LoadFog( 2 );
	UnLoading();
	ItemChoose = -1;
	PlayerMenuMode = 1;
	use_gsc_network_protocol = ( 0 != LOG );
}