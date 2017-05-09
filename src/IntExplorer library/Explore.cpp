#include "../Main executable/common.h"
#include "IntExplorer.h"
#include "ParseRQ.h"
#include <crtdbg.h>
#include <assert.h>

extern int IBOR2;
extern int IBOR0;

/* communication with server
DWORD SendRequest(char* request);
DWORD GetRequestResult(DWORD Handle,char* Result,int* Size);
0-no answer
1-timeout
2-invalid request
3-server failure
128-success
void CloseRequest(DWORD Handle);
*/

#define xnew(t,n) new t[n]
/*
void OneSicWindow::Open(char* ref){
	REF=xnew(char,strlen(ref)+1);
	strcpy(REF,ref);
	ParsedRQ P1;
	P1.Parse(ref);
	int sz=P1.Compact(nullptr,0);
	char* data=(char*)malloc(sz+1);
	P1.Compact(data,sz);
	GHANDLE=EXP->SendRequest(data,sz);
	free(data);
	Ready=0;
	Parsed=0;
	Result=nullptr;
	UPHANDLE=0;
};
*/
/*
void OneSicWindow::Open(char* ref,int size){
	REF=xnew(char,strlen(ref)+1);
	strcpy(REF,ref);
	ParsedRQ P1;
	P1.Parse(ref);
	int sz=P1.Compact(nullptr,0);
	char* data=(char*)malloc(sz+1);
	P1.Compact(data,sz);
	GHANDLE=EXP->SendRequest(data,sz);
	free(data);
	Ready=0;
	Parsed=0;
	Result=nullptr;
	UPHANDLE=0;
};
*/
void SendSmartRequest( sicExplorer* SXP, char* Str );
void OneSicWindow::Process()
{
	//EXP->Process();
	if (Error)
	{
		return;
	}
	int TT = GetTickCount();
	for (int V = 0; V < NTIME; V++)
	{
		if (TimeLimit[V] && TT - TimeLimit[V] > 0)
		{
			if (TimeLimitRequest[V][0] == 'c'&&
				TimeLimitRequest[V][1] == 'o'&&
				TimeLimitRequest[V][2] == 'm'&&
				TimeLimitRequest[V][3] == ':')
			{
				TimeLimit[V] = 0;
				SendSmartRequest( EXP, TimeLimitRequest[V] + 4 );
			}
			else
				if (TimeLimitRequest[V][0] == 'o'&&
					TimeLimitRequest[V][1] == 'p'&&
					TimeLimitRequest[V][2] == 'e'&&
					TimeLimitRequest[V][3] == 'n'&&
					TimeLimitRequest[V][4] == ':')
				{
					TimeLimit[V] = 0;
					ShellExecute( nullptr, "open", TimeLimitRequest[V] + 5, nullptr, nullptr, SW_MAXIMIZE );
				}
				else
				{
					TimeLimit[V] = 0;
					char CC[256];
					sprintf( CC, "Internet\\Cash\\%s", TimeLimitRequest[V] );
					ParsedRQ P1;
					strcpy( P1.DevName, "LF" );
					P1.AddComm( "LW_file" );
					P1.AddParam( CC, strlen( CC ) + 1 );
					char dat[512];
					P1.UnParse( dat, 511 );
					SendGlobalRequest( EXP, dat, 1 );
				};
		};
	};
	for (int q = 0; q < NPRC; q++)if (PRC[q].Process)
		PRC[q].Process( EXP, PRC[q].Data, PRC[q].size );
	if (!Ready)
	{
		if (GHANDLE)
		{
			char* res;
			int sz;
			DWORD r = EXP->GetRequestResult( GHANDLE, &res, &sz );
			MaxL = sz;
			if (r == 128)
			{
				GHANDLE = 0;
				Result = (char*) malloc( strlen( res ) + 1 );
				strcpy( Result, res );
				EXP->CloseRequest( GHANDLE );
			}
			else
			{
				if (r != 0)
				{
					Error = 1;
				};
			};
		};
		if (Result)
		{
			if (!UPHANDLE)
			{
				if (RefsSent || ( !strchr( Result, '~' ) ))Ready = 1;
				else
				{
					DownloadRefs();
				};
			};
		};
	}
	else
	{
		//now we are ready to parse the whole text
		if (!Parsed)
		{
			int pos = ParseTheWholeText();
			LoadCookies();
			if (Result&&pos != -1)
			{
				ResFile F = RRewrite( "Internet\\error.log" );
				if (F != INVALID_HANDLE_VALUE)
				{
					int L = strlen( Result );
					if (L >= pos)
					{
						RBlockWrite( F, Result, pos );
						char* ccc = "[!!!ERROR!!!]";
						RBlockWrite( F, ccc, strlen( ccc ) );
						RBlockWrite( F, Result + pos, L - pos );
						RClose( F );
					}
					else
					{
						RBlockWrite( F, Result, L );
						RClose( F );
					};
				};
			};
		};
		Parsed = 1;
	};
};
void Replace( char** str, char* src, char* dst, int& MaxL )
{
	int L = strlen( *str );
	char* s1 = nullptr;
	int L0 = strlen( src );
	int L1 = strlen( dst );
	int N = 0;
	do
	{
		s1 = strstr( *str, src );
		if (s1)
		{
			if (L0 != L1)
			{
				if (L + L1 - L0 >= MaxL - 32)
				{
					MaxL += L + L1 - L0 + 512;
					*str = (char*) realloc( *str, MaxL );
					s1 = strstr( *str, src );
				};
				int LL = L - L0 - ( s1 - *str ) + 1;
				memmove( s1 + L1, s1 + L0, LL );
				L += L1 - L0;
			};
			memcpy( s1, dst, L1 );
		};
		N++;
	} while (s1&&N < 10000);
};
void OneSicWindow::DownloadRefs()
{
	char req[16384];
	if (!REF)return;
	strcpy( req, REF );
	char* ss = strchr( req, '|' );
	if (!ss)return;
	ss[1] = 0;
	strcat( req, "get" );
	int L = strlen( Result );
	int N = 0;
	char temp[256];
	char temp1[256];
	for (int i = 0; i < L; i++)
	{
		char c = Result[i];
		if (c == '~')
		{
			char* cc1 = Result + i + 1;
			int j = 0;
			while (cc1[j] != '~'&&cc1[j] && j < 128)j++;
			memcpy( temp, Result + i, j + 2 );
			temp[j + 2] = 0;
			i += j + 1;
			bool FoundInCash = 0;
			sprintf( temp1, "Internet\\cash\\%s.txt", temp );
			ResFile F = RReset( temp1 );
			if (F != INVALID_HANDLE_VALUE)
			{
				int sz = RFileSize( F );
				char* s2 = xnew( char, sz + 1 );
				RBlockRead( F, s2, sz );
				s2[sz] = 0;
				Replace( &Result, temp, s2, MaxL );
				i += strlen( s2 ) - 1;
				L = strlen( Result );
				free( s2 );
				RClose( F );
				FoundInCash = 1;
			}
			else
			{
				sprintf( temp1, "Internet\\cash\\%s", temp );
				temp1[strlen( temp1 ) - 1] = 0;
				ResFile F_local = RReset( temp1 );
				if (F_local != INVALID_HANDLE_VALUE)
				{
					strcpy( temp1, temp + 1 );
					temp1[strlen( temp1 ) - 1] = 0;
					Replace( &Result, temp, temp1, MaxL );
					i += strlen( temp1 ) - 1;
					L = strlen( Result );
					RClose( F_local );
					FoundInCash = 1;
				};
			};
			if (!FoundInCash)
			{
				N++;
				strcat( req, "&" );
				strcat( req, temp );
				strcat( req, "&" );
				strcat( req, temp );
				strcat( req, ".txt" );
			};
		};
	};
	if (N)
	{
		UPHANDLE = SendGlobalRequest( EXP, req, 0 );
		RefsSent = 1;
	};
};
int ReadNumber( char* s, int* L, int vmax )
{
	int v = 0;
	char c;
	int p = 0;
	bool doit = 1;
	int sign = 0;
	do
	{
		c = s[p];
		if (c >= '0'&&c <= '9')
		{
			v = v * 10 + c - '0';
			p++;
		}
		else if (c == '%')
		{
			v = ( v*vmax ) / 100;
			doit = 0;
			p++;
		}
		else
			if (c == '-' && !p)
			{
				p++;
				sign = 1;
			}
			else doit = 0;
	} while (doit);
	*L = p;
	if (sign)return -v;
	else return v;
};
int CReadX( char** sx, int x0, int x1 )
{
	int xx = 0;
	int com = 0;
	char* s = *sx;
	while (s[0] != ','&&s[0] != ')'&&s[0] != ']'&&s[0])
	{
		if (s[0] == 'R')xx = x1;
		else if (s[0] == 'L')xx = x0;
		else if (s[0] == 'C')xx = ( x1 + x0 ) >> 1;
		else if (s[0] == '+')com = 1;
		else if (s[0] == '-')com = -1;
		else
		{
			int L = 0;
			int v = ReadNumber( s, &L, x1 - x0 );
			if (L)
			{
				if (com)xx += com*v;
				else xx = x0 + v;
			};
			s += L - 1;
		};
		s++;
	};
	*sx = s;
	return xx;
};
int CReadY( char** sx, int x0, int x1, OneBox* BOX )
{
	int xx = 0;
	int com = 0;
	char* s = *sx;
	while (s[0] != ','&&s[0] != ')'&&s[0] != ']'&&s[0])
	{
		if (s[0] == '%')
		{
			int p1 = 0;
			char c;
			while (( c = s[p1] ) != 0 && c != ','&&c != ']'&&c != ')'&&c != '+'&&c != '-'&&c != '['&&c != '('&&p1 < 14)p1++;
			if (p1 < 14 && BOX)
			{
				char name[16];
				memcpy( name, s, p1 );
				name[p1] = 0;
				for (int i = 0; i < BOX->NISPOS; i++)if (!strcmp( BOX->ISPOS[i].ID, name ))xx = BOX->ISPOS[i].y1;
			};
			s += p1 - 1;
		}
		else
			if (s[0] == 'D')xx = x1;
			else if (s[0] == 'U')xx = x0;
			else if (s[0] == 'C')xx = ( x1 + x0 ) >> 1;
			else if (s[0] == '+')com = 1;
			else if (s[0] == '-')com = -1;
			else
			{
				int L = 0;
				int v = ReadNumber( s, &L, x1 - x0 );
				if (L)
				{
					if (com)xx += com*v;
					else xx = x0 + v;
				};
				s += L - 1;
			};
		s++;
	};
	*sx = s;
	return xx;
};
void ReadCoordinates( char** src, int& x, int& y, int& xR, int& yR, OneBox* BOX )
{
	char c;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	int xc = 0;
	int yc = 0;
	int w = 0;
	int h = 0;
	bool vx0 = 0;
	bool vy0 = 0;
	bool vx1 = 0;
	bool vy1 = 0;
	bool vxc = 0;
	bool vyc = 0;
	bool vw = 0;
	bool vh = 0;
	do
	{
		c = ( *src )[0];
		if (c == 'x' && ( *src )[1] == ':')
		{
			( *src ) += 2;
			x0 = CReadX( src, x, xR );
			vx0 = 1;
		}
		else
			if (c == 'x' && ( *src )[1] == '1' && ( *src )[2] == ':')
			{
				( *src ) += 3;
				x1 = CReadX( src, x, xR );
				vx1 = 1;
			}
			else
				if (c == 'x' && ( *src )[1] == 'c' && ( *src )[2] == ':')
				{
					( *src ) += 3;
					xc = CReadX( src, x, xR );
					vxc = 1;
				}
				else
					if (c == 'y' && ( *src )[1] == ':')
					{
						( *src ) += 2;
						y0 = CReadY( src, y, yR, BOX );
						vy0 = 1;
					}
					else
						if (c == 'y' && ( *src )[1] == '1' && ( *src )[2] == ':')
						{
							( *src ) += 3;
							y1 = CReadY( src, y, yR, BOX );
							vy1 = 1;
						}
						else
							if (c == 'y' && ( *src )[1] == 'c' && ( *src )[2] == ':')
							{
								( *src ) += 3;
								yc = CReadY( src, y, yR, BOX );
								vyc = 1;
							}
							else
								if (c == 'w' && ( *src )[1] == ':')
								{
									( *src ) += 2;
									w = CReadX( src, 0, xR - x );
									vw = 1;
								}
								else
									if (c == 'h' && ( *src )[1] == ':')
									{
										( *src ) += 2;
										h = CReadY( src, 0, yR - y, BOX );
										vh = 1;
									}
									else ( *src )++;
	} while (c != ')'&&c != ']'&&c != '}');
	( *src )--;
	if (vx0&&vx1)
	{
		x = x0;
		xR = x1;
	};
	if (vx0&&vw)
	{
		x = x0;
		xR = x0 + w;
	};
	if (vxc&&vw)
	{
		x = xc - ( w >> 1 );
		xR = x + w;
	};
	if (vx0&&vxc)
	{
		x = x0;
		xR = xc + xc - x0;
	};
	if (vx1&&vxc)
	{
		x = xc + xc - x1;
		xR = x1;
	};
	if (vy0&&vh)
	{
		y = y0;
		yR = y0 + h;
	};
	if (vy0&&vy1)
	{
		y = y0;
		yR = y1;
	};
	if (vyc&&vh)
	{
		y = yc - ( h >> 1 );
		yR = y + h;
	};
	if (vy0&&vyc)
	{
		y = y0;
		yR = yc + yc - y0;
	};
	if (vy1&&vyc)
	{
		y = yc + yc - y1;
		yR = y1;
	};
}

RLCFont* sicExplorer::GetFontByName( char* fnt )
{
	if (CurWPosition < NWindows)
	{
		OneSicWindow* OSW = Windows[CurWPosition];
		for (int j = 0; j < OSW->NAddFonts; j++)
			if (!strcmp( fnt, OSW->ADFonts[j].FID ))
				return &OSW->ADFonts[j].FONT;
	}

	if (!strcmp( fnt, "F10" ))return &fn10;
	else if (!strcmp( fnt, "F8" ))return &fn8;

	else if (!strcmp( fnt, "WF" ))return &WhiteFont;
	else if (!strcmp( fnt, "YF" ))return &YellowFont;
	else if (!strcmp( fnt, "RF" ))return &RedFont;
	else if (!strcmp( fnt, "BF" ))return &BlackFont;

	else if (!strcmp( fnt, "SWF" ))return &SmallWhiteFont;
	else if (!strcmp( fnt, "SYF" ))return &SmallYellowFont;
	else if (!strcmp( fnt, "SRF" ))return &SmallRedFont;
	else if (!strcmp( fnt, "SBF" ))return &SmallBlackFont;

	else if (!strcmp( fnt, "BWF" ))return &BigWhiteFont;
	else if (!strcmp( fnt, "BYF" ))return &BigYellowFont;
	else if (!strcmp( fnt, "BRF" ))return &BigRedFont;
	else if (!strcmp( fnt, "BBF" ))return &BigBlackFont;

	else if (!strcmp( fnt, "SPWF" ))return &BigWhiteFont;
	else if (!strcmp( fnt, "SPYF" ))return &BigYellowFont;
	else if (!strcmp( fnt, "SPRF" ))return &BigRedFont;
	else if (!strcmp( fnt, "SPBF" ))return &BigBlackFont;

	else if (!strcmp( fnt, "XXWF" ))return &SpecialWhiteFont;
	else if (!strcmp( fnt, "XXYF" ))return &SpecialYellowFont;
	else if (!strcmp( fnt, "XXRF" ))return &SpecialRedFont;
	else if (!strcmp( fnt, "XXBF" ))return &SpecialBlackFont;

	else
	{
		return &YellowFont;
	}
}

int GETHX( char c )
{
	if (c >= '0'&&c <= '9')
	{
		return c - '0';
	}
	if (c >= 'A'&&c <= 'F')
	{
		return c - 'A' + 10;
	}
	return 0;
}

__declspec( dllimport ) void SetChatWState( int ID, int State );

int OneSicWindow::ParseTheWholeText()
{
	SetChatWState( 0, 0 );
	SetChatWState( 1, 0 );
	SetChatWState( 2, 0 );
	int pos = 0;
	do
	{
		char c = Result[pos];
		if (c == '#')
		{
			int p1 = pos;
			while (( c = Result[p1] ) != '('&&c != '['&&c != 0 && c != 10 && c != 13)p1++;
			char com[128];
			if (p1 - pos > 32)
			{
				return pos;
			}

			memcpy( com, Result + pos, p1 - pos );
			com[p1 - pos] = 0;
			pos = p1;
			bool B1 = !strcmp( com, "#box" );
			bool B2 = !strcmp( com, "#sbox" );
			bool B3 = !strcmp( com, "#ebox" );
			if (B1 || B2 || B3)
			{
				if (Result[pos] != '[')
				{
					return pos;
				}

				pos++;

				if (Result[pos] == '%')
				{
					int pos_2 = pos;
					while (( c = Result[pos_2] ) != ']')pos_2++;
					if (pos_2 - pos > 14)return pos;
					memcpy( com, Result + pos, pos_2 - pos );
					com[pos_2 - pos] = 0;
					pos = pos_2 + 2;
					if (NBoxes >= MaxBoxes)
					{
						MaxBoxes += 16;
						Boxes = (OneBox*) realloc( Boxes, MaxBoxes * sizeof OneBox );
					};
					int X0 = EXP->x;
					int Y0 = EXP->y;
					int X1 = EXP->x1;
					int Y1 = EXP->y1;
					char* src = Result + pos;
					ReadCoordinates( &src, X0, Y0, X1, Y1, nullptr );
					memset( Boxes + NBoxes, 0, sizeof OneBox );
					OneBox* OBX = Boxes + NBoxes;
					pos = src - Result + 1;
					OBX->Scroll = B2;
					OBX->Box = B1;

					OBX->x = X0;
					OBX->y = Y0;
					OBX->x1 = X1;
					OBX->y1 = Y1;

					OBX->xi = X0;
					OBX->yi = Y0;
					OBX->xi1 = X1;
					OBX->yi1 = Y1;
					if (B2)
					{
						OBX->xi += sb_DxL;
						OBX->yi += sb_DyL;
						OBX->xi1 -= sb_DxR;
						OBX->yi1 -= sb_DyR;
						int GPIDX = EXP->GetGPPictureIndex( sbox_gp_file );
						OBX->StartScrollIndex = 0;
						if (GPIDX != -1)
						{
							int BORX[8] = { sb_LU,sb_RU,sb_LD,sb_RD,sb_L,sb_R,sb_U,sb_D };
							OBX->DSS.addCustomBorder( OBX->x, OBX->y, OBX->x1, OBX->y1, GPIDX, BORX, BORX, -1, -1 );
							OBX->DSS.addClipper( 0, 0, RealLx - 1, RealLy - 1 );
							OBX->StartScrollIndex += 2;
							int gpid = EXP->GetGPPictureIndex( scrl_gp_file );
							if (gpid != -1)
							{
								OBX->VS = OBX->DSS.addNewGP_VScrollBar( nullptr,
									OBX->xi1 + sc_dx, OBX->yi + sc_dy, OBX->yi1 - OBX->yi + 1 + sc_dly, 10, 0, gpid, scr_Idx );
								OBX->StartScrollIndex++;
								OBX->VS->btnly -= scr_dscroll;
								OBX->VS->LineLy += scr_dscroll * 2;
								OBX->VS->SPos = 0;
								OBX->VS->SMaxPos = 0;
								OBX->LastScrollPos = OBX->VS->SPos;
							};
						}
						else
						{
							OBX->StartScrollIndex = 0;
							OBX->Scroll = 0;
						};
						OBX->DSS.addClipper( OBX->xi, OBX->yi, OBX->xi1, OBX->yi1 );
						OBX->StartScrollIndex++;
					}
					else if (B1)
					{
						OBX->xi += b_DxL;
						OBX->yi += b_DyL;
						OBX->xi1 -= b_DxR;
						OBX->yi1 -= b_DyR;
						//OBX->DSS.addClipper(OBX->x,OBX->y,OBX->x1,OBX->y1);
						int BORX[8] = { b_LU,b_RU,b_LD,b_RD,b_L,b_R,b_U,b_D };
						int gpid = EXP->GetGPPictureIndex( box_gp_file );
						if (gpid != -1)
						{
							OBX->DSS.addCustomBorder( OBX->x, OBX->y, OBX->x1, OBX->y1, gpid, BORX, BORX, -1, -1 );
						};
						OBX->DSS.addClipper( OBX->xi, OBX->yi, OBX->xi1, OBX->yi1 );

						OBX->StartScrollIndex = 0;
						OBX->VS = nullptr;
					}
					else
					{
						OBX->StartScrollIndex = 0;
						OBX->VS = nullptr;
					};
					strcpy( Boxes[NBoxes].Name, com );
					NBoxes++;
				}
				else return pos;
			}
			else
				if (!strcmp( com, "#def_panel" ))
				{
					pos++;
					int L = 0;
					int PIDX = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					if (PIDX < 16)
					{
						int p1 = pos;
						while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
						if (p1 - pos > 120)return pos;
						memcpy( PANELS[PIDX].panel_gp, Result + pos, p1 - pos );
						PANELS[PIDX].panel_gp[p1 - pos] = 0;
						p1++;
						pos = p1;
					};
					PANELS[PIDX].LU = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].RU = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].LD = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].RD = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].L = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].R = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].U = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].D = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].C = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
					PANELS[PIDX].C1 = ReadNumber( Result + pos, &L, 0 );
					pos += L + 1;
				}
				else
					if (!strcmp( com, "#def_sbtn" ))
					{
						pos++;
						int L = 0;
						int p1 = pos;
						while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
						if (p1 - pos > 120)return pos;
						memcpy( sstb_gp_file, Result + pos, p1 - pos );
						sstb_gp_file[p1 - pos] = 0;
						p1++;
						pos = p1;
						sstb_Index = ReadNumber( Result + pos, &L, 0 );
						pos += L + 1;
						sstb_dx = ReadNumber( Result + pos, &L, 0 );
						pos += L + 1;
						sstb_dy = ReadNumber( Result + pos, &L, 0 );
						pos += L + 1;
						sstb_wholedx = ReadNumber( Result + pos, &L, 0 );
						pos += L + 1;
						sstb_wholedy = ReadNumber( Result + pos, &L, 0 );
						pos += L + 1;
						sstb_wholeLx = ReadNumber( Result + pos, &L, 0 );
						pos += L + 1;
					}
					else
						if (!strcmp( com, "#def_btn" ))
						{
							pos++;
							int L = 0;
							int p1 = pos;
							while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
							if (p1 - pos > 120)return pos;
							memcpy( stb_gp_file, Result + pos, p1 - pos );
							stb_gp_file[p1 - pos] = 0;
							p1++;
							pos = p1;
							stb_Index = ReadNumber( Result + pos, &L, 0 );
							pos += L + 1;
							stb_dx = ReadNumber( Result + pos, &L, 0 );
							pos += L + 1;
							stb_dy = ReadNumber( Result + pos, &L, 0 );
							pos += L + 1;
							stb_wholedx = ReadNumber( Result + pos, &L, 0 );
							pos += L + 1;
							stb_wholedy = ReadNumber( Result + pos, &L, 0 );
							pos += L + 1;
							stb_wholeLx = ReadNumber( Result + pos, &L, 0 );
							pos += L + 1;
						}
						else
							if (!strcmp( com, "#btn_style" ))
							{
								pos++;
								int L = 0;
								btn_style = ReadNumber( Result + pos, &L, 0 );
								pos += L + 1;
							}
							else
								if (!strcmp( com, "#def_box" ))
								{
									if (Result[pos] != '(')return pos;
									pos++;
									int p1 = pos;
									while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
									if (p1 - pos > 120)return pos;
									memcpy( box_gp_file, Result + pos, p1 - pos );
									box_gp_file[p1 - pos] = 0;
									p1++;
									pos = p1;
									int L = 0;
									b_LU = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_RU = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_LD = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_RD = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_L = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_R = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_U = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_D = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;

									b_DxL = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_DyL = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_DxR = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;
									b_DyR = ReadNumber( Result + pos, &L, 0 );
									pos += L + 1;

								}
								else
									if (!strcmp( com, "#def_sbox" ))
									{
										if (Result[pos] != '(')return pos;
										pos++;
										int p1 = pos;
										while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
										if (p1 - pos > 120)return pos;
										memcpy( sbox_gp_file, Result + pos, p1 - pos );
										box_gp_file[p1 - pos] = 0;
										p1++;
										pos = p1;
										int L = 0;
										sb_LU = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_RU = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_LD = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_RD = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_L = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_R = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_U = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_D = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;

										sb_DxL = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_DyL = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_DxR = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sb_DyR = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;

										sc_dx = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sc_dy = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
										sc_dly = ReadNumber( Result + pos, &L, 0 );
										pos += L + 1;
									}
									else
										if (!strcmp( com, "#def_scroll" ))
										{
											if (Result[pos] != '(')return pos;
											pos++;
											int p1 = pos;
											while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
											if (p1 - pos > 120)return pos;
											memcpy( scrl_gp_file, Result + pos, p1 - pos );
											scrl_gp_file[p1 - pos] = 0;
											p1++;
											pos = p1;
											int L = 0;
											scr_Idx = ReadNumber( Result + pos, &L, 0 );
											pos += L + 1;
											scr_dscroll = ReadNumber( Result + pos, &L, 0 );
											pos += L + 1;
										}
										else
											if (!strcmp( com, "#def_combo" ))
											{
												if (Result[pos] != '(')return pos;
												pos++;
												int p1 = pos;
												while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
												if (p1 - pos > 120)return pos;
												memcpy( combo_gp_file, Result + pos, p1 - pos );
												combo_gp_file[p1 - pos] = 0;
												p1++;
												pos = p1;
												int L = 0;
												cmb_Idx = ReadNumber( Result + pos, &L, 0 );
												pos += L + 1;
												cmb_bdx = ReadNumber( Result + pos, &L, 0 );
												pos += L + 1;
											}
											else
												if (!strcmp( com, "#def_tbl" ))
												{
													if (Result[pos] != '(')return pos;
													pos++;
													int L = 0;
													CTBL_MainBorderID = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_ScrollBorderID = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_SelBorderID = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_MouseBorderID = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_UpperLY = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_LowLY = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_OLY = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_MarginL = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_MarginR = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_EditMarginL = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_EditMarginR = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_FilterMarginL = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_FilterMarginR = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
													CTBL_FilterMarginU = ReadNumber( Result + pos, &L, 0 );
													pos += L + 1;
												}
												else
													if (!strcmp( com, "#def_panel" ))
													{
														if (Result[pos] != '(')return pos;
														pos++;
														int p1 = pos;
														while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
														if (p1 - pos > 120)return pos;
														memcpy( panel_gp_file, Result + pos, p1 - pos );
														panel_gp_file[p1 - pos] = 0;
														p1++;
														pos = p1;
													}
													else
														if (!strcmp( com, "#exec" ))
														{
															if (Result[pos] != '(')return pos;
															char FL[512];
															pos++;
															int p1 = pos;
															while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
															if (p1 - pos > 510)return pos;
															memcpy( FL, Result + pos, p1 - pos );
															FL[p1 - pos] = 0;
															p1++;
															pos = p1;
															SendSmartRequest( EXP, FL );
														}
														else
															if (!strcmp( com, "#large" ))SetChatWState( 1, 1 );
															else
																if (!strcmp( com, "#small" ))SetChatWState( 0, 1 );
																else
																	if (!strcmp( com, "#lockresize" ))SetChatWState( 2, 1 );
																	else
																		if (!strcmp( com, "#block" ))
																		{
																			if (Result[pos] != '(')return pos;
																			char FL[128];
																			char ID[64];
																			pos++;
																			int p1 = pos;
																			while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																			if (c != ',')return p1;
																			if (p1 - pos > 64)return pos;
																			memcpy( FL, Result + pos, p1 - pos );
																			FL[p1 - pos] = 0;
																			p1++;
																			pos = p1;
																			while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																			if (p1 - pos > 16)return pos;
																			memcpy( ID, Result + pos, p1 - pos );
																			ID[p1 - pos] = 0;
																			p1++;
																			pos = p1;
																			char FNAME[160];
																			char EID[64];
																			sprintf( FNAME, "Internet\\Cash\\%s", FL );
																			sprintf( EID, "#end(%s)", ID );
																			char* S1 = strstr( Result + pos, EID );
																			if (S1)
																			{
																				ResFile F = RRewrite( FNAME );
																				if (F != INVALID_HANDLE_VALUE)
																				{
																					RBlockWrite( F, Result + pos, S1 - Result - pos );
																					RClose( F );
																				};
																				pos = S1 - Result + strlen( EID );
																			};
																		}
																		else
																			if (!strcmp( com, "#hblock" ))
																			{
																				if (Result[pos] != '(')return pos;
																				char FL[128];
																				char ID[64];
																				pos++;
																				int p1 = pos;
																				while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																				if (c != ',')return p1;
																				if (p1 - pos > 64)return pos;
																				memcpy( FL, Result + pos, p1 - pos );
																				FL[p1 - pos] = 0;
																				p1++;
																				pos = p1;
																				while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																				if (p1 - pos > 16)return pos;
																				memcpy( ID, Result + pos, p1 - pos );
																				ID[p1 - pos] = 0;
																				p1++;
																				pos = p1;
																				char FNAME[160];
																				char EID[64];
																				sprintf( FNAME, "Internet\\Cash\\%s", FL );
																				sprintf( EID, "#end(%s)", ID );
																				char* S1 = strstr( Result + pos, EID );
																				if (S1)
																				{
																					int N = S1 - Result - pos;
																					byte* buf = (byte*) malloc( N / 2 + 10 );
																					char* src = Result + pos;
																					int bpos = 0;
																					for (int i = 0; i < N;)
																					{
																						char c;
																						do
																						{
																							c = src[i];
																							i++;
																						} while (i < N && !( ( c >= '0'&&c <= '9' ) || ( c >= 'A'&&c <= 'F' ) ));
																						char c1;
																						do
																						{
																							c1 = src[i];
																							i++;
																						} while (i < N && !( ( c1 >= '0'&&c1 <= '9' ) || ( c1 >= 'A'&&c1 <= 'F' ) ));
																						if (i < N)
																						{
																							buf[bpos] = (byte) ( ( GETHX( c ) << 4 ) + GETHX( c1 ) );
																							bpos++;
																						};
																					};
																					ResFile F = RRewrite( FNAME );
																					if (F != INVALID_HANDLE_VALUE)
																					{
																						RBlockWrite( F, buf, bpos );
																						RClose( F );
																					};
																					free( buf );
																					pos = S1 - Result + strlen( EID );
																				};
																			}
																			else
																				if (!strcmp( com, "#save" ))
																				{
																					if (Result[pos] != '(')return pos;
																					char FL[128];
																					char ID[64];
																					pos++;
																					int p1 = pos;
																					while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																					if (c != ',')return p1;
																					if (p1 - pos > 64)return pos;
																					memcpy( FL, Result + pos, p1 - pos );
																					FL[p1 - pos] = 0;
																					p1++;
																					pos = p1;
																					while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																					if (p1 - pos > 16)return pos;
																					memcpy( ID, Result + pos, p1 - pos );
																					ID[p1 - pos] = 0;
																					p1++;
																					pos = p1;
																					char FNAME[160];
																					char EID[64];
																					sprintf( FNAME, "Internet\\Cash\\%s", FL );

																					sprintf( EID, "<%s>", ID );
																					char* S1 = strstr( Result, EID );
																					if (S1)
																					{
																						char* S2 = strstr( S1 + 1, EID );
																						if (S2)
																						{
																							ResFile F = RRewrite( FNAME );
																							if (F != INVALID_HANDLE_VALUE)
																							{
																								RBlockWrite( F, S1, S2 - S1 + strlen( EID ) );
																								RClose( F );
																							};
																						};
																					};
																				}
																				else
																					if (!strcmp( com, "#font" ))
																					{
																						if (Result[pos] != '(')return pos;
																						pos++;
																						int p1 = pos;
																						while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																						if (c != ',')return p1;
																						if (p1 - pos > 12)return pos;
																						memcpy( EXP->FONT1, Result + pos, p1 - pos );
																						EXP->FONT1[p1 - pos] = 0;

																						p1++;

																						pos = p1;
																						while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																						if (p1 - pos > 12)return pos;
																						if (c != ',')return p1;
																						memcpy( EXP->FONT2, Result + pos, p1 - pos );
																						EXP->FONT2[p1 - pos] = 0;

																						p1++;

																						pos = p1;
																						while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																						if (p1 - pos > 12)return pos;
																						if (c != ')')return p1;
																						memcpy( EXP->FONT3, Result + pos, p1 - pos );
																						EXP->FONT3[p1 - pos] = 0;
																						pos = p1 + 1;
																					}
																					else
																						if (!strcmp( com, "#resize" ))
																						{
																							if (Result[pos] != '(')return pos;
																							pos++;
																							int p1 = pos;
																							while (( c = Result[p1] ) != ')'&&c)p1++;
																							ResizeComm = (char*) malloc( p1 - pos + 1 );
																							memcpy( ResizeComm, Result + pos, p1 - pos );
																							ResizeComm[p1 - pos] = 0;
																							pos = p1 + 1;
																						}
																						else
																						{
																							//more complex commands
																							OneInterfaceElement IFS;
																							memset( &IFS, 0, sizeof IFS );
																							DialogsSystem* DSS = nullptr;
																							for (int i = 0; i < N_IFNS; i++)
																							{
																								if (!strcmp( IFNS[i].Name, com ))
																								{
																									if (IFNS[i].ReqName&&Result[pos] == '[')
																									{
																										if (Result[pos + 1] != '%')return pos;
																										pos++;
																										int p1 = pos;
																										while (( c = Result[p1] ) != ']'&&c != 0)p1++;
																										if (p1 - pos > 14)return p1;
																										memcpy( IFS.ID, Result + pos, p1 - pos );
																										IFS.ID[p1 - pos] = 0;
																										pos = p1 + 1;
																									};
																									if (Result[pos] != '(')return pos;
																									pos++;
																									OneBox* CURBOX = nullptr;
																									if (IFNS[i].ReqCoor)
																									{
																										if (Result[pos] != '%')return pos;
																										int p1 = pos;
																										char ss[16];
																										while (( c = Result[p1] ) != '['&&c != 0)p1++;
																										if (p1 - pos > 14)return pos;
																										memcpy( ss, Result + pos, p1 - pos );
																										ss[p1 - pos] = 0;
																										pos = p1;
																										int j;
																										for (j = 0; j < NBoxes; j++)if (!strcmp( Boxes[j].Name, ss ))
																										{
																											CURBOX = &Boxes[j];
																											DSS = &Boxes[j].DSS;
																											IFS.x = Boxes[j].xi;
																											IFS.y = Boxes[j].yi;
																											IFS.x1 = Boxes[j].xi1;
																											IFS.y1 = Boxes[j].yi1;
																											j = NBoxes + 100;
																										};
																										if (j == NBoxes)return pos;
																										pos++;
																										char* ss1 = Result + pos;
																										ReadCoordinates( &ss1, IFS.x, IFS.y, IFS.x1, IFS.y1, CURBOX );
																										pos = ss1 - Result;
																										//if(Result[pos]!=']')return pos;
																										pos++;
																										if (( IFNS[i].ReqActiv || IFNS[i].ReqParam ) && Result[pos] != ',')return pos;
																										pos++;
																									};
																									if (IFNS[i].ReqActiv)
																									{
																										//IFS.NActiveRefs=0;
																										//IFS.ActiveRefs=nullptr;
																										IFS.ClearActive();
																										do
																										{
																											if (( c = Result[pos] ) != '{'&&c != ','&&c != ')')return pos;
																											pos++;
																											int p1 = pos;
																											while (( c = Result[p1] ) != '}'&&c)p1++;
																											if (Result[p1] != '}')return pos;
																											IFS.ActiveRefs = (char**) realloc( IFS.ActiveRefs, ( IFS.NActiveRefs + 1 ) << 2 );
																											IFS.ActiveRefs[IFS.NActiveRefs] = (char*) malloc( p1 - pos + 1 );
																											memcpy( IFS.ActiveRefs[IFS.NActiveRefs], Result + pos, p1 - pos );
																											IFS.ActiveRefs[IFS.NActiveRefs][p1 - pos] = 0;
																											pos = p1 + 1;
																											IFS.NActiveRefs++;
																											c = Result[pos];
																										} while (c != ','&&c != ')');
																										//if(IFNS[i].ReqParam&&(c!=','||c!=')'))return pos;
																									};
																									if (IFNS[i].ReqParam)
																									{
																										IFS.ClearParams();
																										bool doit = 1;
																										do
																										{
																											if (Result[pos] == ',')pos++;
																											if (Result[pos] == ')')doit = 0;
																											c = Result[pos];
																											int p1;
																											int p2;
																											if (c == '"')
																											{
																												pos++;
																												p1 = pos;
																												while (( c = Result[p1] ) != '"'&&c)p1++;
																												p2 = p1;
																												p1 = pos;
																												pos = p2;
																												if (Result[pos] != '"')return pos;
																												pos++;
																											}
																											else
																											{
																												p1 = pos;
																												while (( c = Result[p1] ) != ','&&c != ')'&&c)p1++;
																												p2 = p1;
																												p1 = pos;
																												pos = p2;
																											};
																											IFS.Params = (char**) realloc( IFS.Params, ( IFS.NParams + 1 ) << 2 );
																											char* ss = IFS.Params[IFS.NParams] = (char*) malloc( p2 - p1 + 1 );
																											memcpy( ss, Result + p1, p2 - p1 );
																											ss[p2 - p1] = 0;
																											IFS.NParams++;
																											doit = Result[pos] == ',';
																											if (!Result[pos])return p1;
																											//IFS.NParams++;
																										} while (doit);
																									};
																									int NBefore = 0;
																									if (DSS)
																									{
																										while (DSS->DSS[NBefore])NBefore++;
																									};
																									IFNS[i].AddInterface( EXP, DSS, &IFS.x, &IFS.y, &IFS.x1, &IFS.y1, IFS.NActiveRefs, IFS.ActiveRefs, IFS.NParams, IFS.Params, IFNS[i].param );
																									int NAfter = 0;
																									if (DSS)
																									{
																										while (DSS->DSS[NAfter])NAfter++;
																									};
																									if (CURBOX)
																									{
																										if (CURBOX->NISPOS >= CURBOX->MaxISPOS)
																										{
																											CURBOX->MaxISPOS += 32;
																											CURBOX->ISPOS = (InterfaceElementPosition*) realloc( CURBOX->ISPOS, CURBOX->MaxISPOS * sizeof InterfaceElementPosition );
																										};
																										InterfaceElementPosition* IEP = CURBOX->ISPOS + CURBOX->NISPOS;
																										CURBOX->NISPOS++;
																										strcpy( IEP->ID, IFS.ID );
																										IEP->x = IFS.x;
																										IEP->y = IFS.y;
																										IEP->x1 = IFS.x1;
																										IEP->y1 = IFS.y1;
																										IEP->StartSD = NBefore;
																										IEP->FinSD = NAfter;
																									};
																									//for(int q=0;q<IFS.NParams;q++)free(IFS.Params[q]);
																									//for(q=0;q<IFS.NActiveRefs;q++)free(IFS.ActiveRefs[q]);
																									//DON'T FORGET IFS!!!
																									IFS.Clear();
																									i = N_IFNS + 100;
																								};
																								if (i == N_IFNS)return pos;
																							};
																						};
		}
		else
			if (c == '/'&&Result[pos + 1] == '/')
			{
				while (( c = Result[pos] ) != 13 && c != 0 && c != 10)pos++;
				if (c == 10 && Result[pos + 1] == 13)pos += 2;
				else if (c == 13 && Result[pos + 1] == 10)pos += 2;
				else if (c == 13 || c == 10)pos++;
			}
			else pos++;
	} while (Result[pos]);
	return -1;
}

OneSicWindow::OneSicWindow()
{
	memset( this, 0, sizeof *this );
}

OneSicWindow::~OneSicWindow()
{
	Erase();
	if (Result)
	{
		free( Result );//bug is there!!
	}
}

OneBox::OneBox()
{
	memset( this, 0, sizeof *this );
}

OneBox::~OneBox()
{}

void OneSicWindow::Erase()
{
	SaveCookies();
	for (int i = 0; i < NBoxes; i++)
	{
		Boxes[i].DSS.CloseDialogs();
		if (Boxes[i].ISPOS)free( Boxes[i].ISPOS );
		Boxes[i].ISPOS = nullptr;
		Boxes[i].NISPOS = 0;
		Boxes[i].MaxISPOS = 0;
	};
	//delete(Boxes);
	for (int i = 0; i < NVARS; i++)
	{
		if (VARS[i].Value)free( VARS[i].Value );
	};
	for (int i = 0; i < NPRC; i++)
	{
		if (PRC[i].Close)PRC[i].Close( EXP, PRC[i].Data, PRC[i].size );
		if (PRC[i].Data)free( PRC[i].Data );
	};
	if (PRC)free( PRC );
	if (VARS)free( VARS );
	NBoxes = 0;
	MaxBoxes = 0;
	if (Boxes)free( Boxes );
	Boxes = nullptr;
	if (Result)free( Result );
	if (REF)free( REF );
	if (ResizeComm)
	{
		free( ResizeComm );
		ResizeComm = nullptr;
	};
	if (ADFonts)free( ADFonts );
	ADFonts = nullptr;
	memset( this, 0, sizeof *this );
	LoadDefaultSettings();
}

void sicExplorer::EraseAllDialogs()
{
	for (int i = 0; i < NWindows; i++)
	{
		Windows[i]->Erase();
		//delete(Windows[i]);
		Windows[i] = nullptr;
	};
	if (Windows)free( Windows );
	Windows = nullptr;
	NWindows = 0;
	MaxWindow = 0;
	for (int i = 0; i < NGVars; i++)
	{
		if (GVARS[i].Value)free( GVARS[i].Value );
	};
	if (GVARS)free( GVARS );
	if (Tables)
	{
		for (int i = 0; i < NTables; i++)
		{
			OneSXPTable* ST = Tables + i;
			int ne = ST->NLines*ST->NCol;
			for (int j = 0; j < ne; j++)free( ST->Lines[j] );
			if (ST->Lines)free( ST->Lines );
			if (ST->Refs)free( ST->Refs );
		};
		if (Tables)free( Tables );
	};
	memset( this, 0, sizeof *this );
}

sicExplorer::~sicExplorer()
{
	EraseAllDialogs();
}

sicExplorer SXP[8];

void InitIFNS();
int SCROLL2;
int SCROLL0;
int CBB_2;
int CBB_0;
int IBOR2;
int IBOR0;
void InitDevs();

char* OneSicWindow::CreateVar( char* Name, int Len )
{
	char* GV = GetVar( Name );
	if (GV)
		return GV;

	VARS = (OneVariable*) realloc( VARS, ( NVARS + 1 ) * sizeof OneVariable );
	VARS[NVARS].Value = new char[Len];
	VARS[NVARS].MaxLen = Len;
	VARS[NVARS].Value[0] = 0;
	strcpy( VARS[NVARS].Name, Name );
	NVARS++;

	if (Name[0] == '%' && Name[1] == 'G' && Name[2] == 'V' && Name[3] == '_')
	{
		LoadCookies();
	}
	return VARS[NVARS - 1].Value;
}

char* OneSicWindow::GetVar( char* Name )
{
	for (int i = 0; i < NVARS; i++)if (!strcmp( VARS[i].Name, Name ))return VARS[i].Value;
	return nullptr;
};
char* sicExplorer::CreateGVar( char* Name, int Len )
{
	GVARS = (OneVariable*) realloc( GVARS, ( NGVars + 1 ) * sizeof OneVariable );
	GVARS[NGVars].Value = new char[Len];
	GVARS[NGVars].MaxLen = Len;
	GVARS[NGVars].Value[0] = 0;
	strcpy( GVARS[NGVars].Name, Name );
	NGVars++;
	return GVARS[NGVars - 1].Value;
};
char* sicExplorer::GetGVar( char* Name )
{
	for (int i = 0; i < NGVars; i++)if (!strcmp( GVARS[i].Name, Name ))return GVARS[i].Value;
	return nullptr;
}

char* sicExplorer::CreateVar( char* Name, int Len )
{
	if (NWindows)
	{
		return Windows[CurWPosition]->CreateVar( Name, Len );
	}
	return nullptr;
}

char* sicExplorer::GetVar( char* Name )
{
	if (NWindows)return Windows[CurWPosition]->GetVar( Name );
	return nullptr;
};
void* OneSicWindow::CreateSXProcess( fnExpProcess* Process, fnExpProcess* Close, int size )
{
	PRC = (OneSXProcess*) realloc( PRC, ( NPRC + 1 ) * sizeof OneSXProcess );
	PRC[NPRC].Process = Process;
	PRC[NPRC].Close = Close;
	PRC[NPRC].Data = malloc( size );
	PRC[NPRC].size = size;
	NPRC++;
	return PRC[NPRC - 1].Data;
};
void* sicExplorer::CreateSXProcess( fnExpProcess* Process, fnExpProcess* Close, int size )
{
	if (NWindows)return Windows[CurWPosition]->CreateSXProcess( Process, Close, size );
	return nullptr;
};
void InstallSXP_Device( char* DLL, char* DevName )
{

};

void sicExplorer::RegisterOutput( int wx, int wy, int wx1, int wy1 )
{
	x = wx;
	y = wy;
	x1 = wx1;
	y1 = wy1;
};

void sicExplorer::ChangeOutput( int x, int y, int x1, int y1 )
{
	RegisterOutput( x, y, x1, y1 );
	for (int j = 0; j < NWindows; j++)
	{
		OneSicWindow* OW = Windows[j];
		OW->SaveCookies();
		for (int i = 0; i < OW->NBoxes; i++)
		{
			OW->Boxes[i].DSS.CloseDialogs();
			if (OW->Boxes[i].ISPOS)free( OW->Boxes[i].ISPOS );
			OW->Boxes[i].ISPOS = nullptr;
			OW->Boxes[i].NISPOS = 0;
			OW->Boxes[i].MaxISPOS = 0;
		};

		for (int i = 0; i < OW->NPRC; i++)
		{
			if (OW->PRC[i].Close)OW->PRC[i].Close( this, OW->PRC[i].Data, OW->PRC[i].size );
			if (OW->PRC[i].Data)free( OW->PRC[i].Data );
		}

		if (OW->PRC)free( OW->PRC );
		OW->PRC = nullptr;
		OW->NPRC = 0;
		OW->NBoxes = 0;
		OW->MaxBoxes = 0;
		if (OW->Boxes)free( OW->Boxes );
		OW->Boxes = nullptr;
		if (OW->NAddFonts&&OW->ADFonts)
		{
			free( OW->ADFonts );
		};
		OW->ADFonts = nullptr;
		OW->NAddFonts = 0;
		if (OW->Ready&&j == CurWPosition&&OW->Parsed)
		{
			if (OW->ResizeComm)
			{
				char* RESZ = OW->ResizeComm;
				OW->ResizeComm = nullptr;
				SendSmartRequest( this, RESZ );
				if (RESZ)free( RESZ );
				//OW->Erase();
			}
			else
			{
				OW->ParseTheWholeText();
				OW->LoadCookies();
			};
		}
		else
		{
			OW->Parsed = 0;
		}
	}
}

void sicExplorer::NewWindow( char* request, char* WinID )
{
	if (CurWPosition < NWindows - 1)
	{
		//erase higher windows
		for (int i = CurWPosition + 1; i < NWindows; i++)Windows[i]->Erase();
		NWindows = CurWPosition + 1;
	}

	if (NWindows >= MaxWindow)
	{
		MaxWindow += 8;
		Windows = (OneSicWindow**) realloc( Windows, MaxWindow * 4 );
	}

	Windows[NWindows] = new OneSicWindow;
	strcpy( Windows[NWindows]->WinID, WinID );
	Windows[NWindows]->REF = (char*) malloc( strlen( request ) + 1 );
	strcpy( Windows[NWindows]->REF, request );
	Windows[NWindows]->EXP = this;
	Windows[NWindows]->LoadDefaultSettings();
	NWindows++;
}

void sicExplorer::StepBack()
{
	if (CurWPosition > 0)
		CurWPosition--;
}

void sicExplorer::StepForward()
{
	if (CurWPosition < NWindows - 1)
		CurWPosition++;
}

void sicExplorer::Refresh()
{
	if (CurWPosition < NWindows)
	{
		if (Windows[CurWPosition] && Windows[CurWPosition]->REF)
		{
			char* ref = (char*) malloc( strlen( Windows[CurWPosition]->REF ) + 1 );
			strcpy( ref, Windows[CurWPosition]->REF );
			char WID[64];
			strcpy( WID, Windows[CurWPosition]->WinID );
			sicExplorer* SXP_local = Windows[CurWPosition]->EXP;
			Windows[CurWPosition]->Erase();
			Windows[CurWPosition]->EXP = SXP_local;
			strcpy( Windows[CurWPosition]->WinID, WID );
			Windows[CurWPosition]->REF = ref;
			SendGlobalRequest( this, ref, 0 );
		}
	}
}

extern "C" __declspec( dllexport ) void SXP_StepBack( int Index )
{
	SXP[Index].StepBack();
}

extern "C" __declspec( dllexport ) void SXP_StepForw( int Index )
{
	SXP[Index].StepForward();
}

extern "C" __declspec( dllexport ) void SXP_Refresh( int Index )
{
	SXP[Index].Refresh();
}

void EraseTempFiles();

extern "C" __declspec( dllexport ) void InitSXP()
{
	EraseTempFiles();
	SCROLL2 = GPS.PreLoadGPImage( "Interface\\_Slider" );
	SCROLL0 = GPS.PreLoadGPImage( "Interface\\GScroll" );
	CBB_2 = GPS.PreLoadGPImage( "Interface\\LongBox" );
	CBB_0 = GPS.PreLoadGPImage( "Interface\\LongBox0" );

	IBOR2 = GPS.PreLoadGPImage( "Internet\\pix\\i_bor2" );
	IBOR0 = GPS.PreLoadGPImage( "Internet\\pix\\i_bor0" );

	InitIFNS();
	InitDevs();
	for (int i = 0; i < 8; i++)
	{
		SXP[i].ID = i;
	}
}

extern "C" __declspec( dllexport ) void RunSXP( int Index, char* home, int x, int y, int x1, int y1 )
{
	if (Index > 8)
	{
		return;
	}

	if (Index == 0 && SXP->NWindows)
	{
		return;
	}

	SXP[Index].ID = Index;
	SXP[Index].RegisterOutput( x, y, x1, y1 );

	SendGlobalRequest( SXP + Index, home, 1 );

	if (SXP[Index].CurWPosition < SXP[Index].NWindows)
	{
		SXP[Index].Windows[SXP[Index].CurWPosition]->Process();
		SXP[Index].Windows[SXP[Index].CurWPosition]->Process();
	}
}

extern "C" __declspec( dllexport ) void OpenRef( int Index, char* home )
{
	if (Index > 8)
	{
		return;
	}

	SendGlobalRequest( SXP + Index, home, 1 );

	if (SXP[Index].CurWPosition < SXP[Index].NWindows)
	{
		SXP[Index].Windows[SXP[Index].CurWPosition]->Process();
		SXP[Index].Windows[SXP[Index].CurWPosition]->Process();
	}
}

extern "C" __declspec( dllexport ) void ResizeSXP( int Index, int x, int y, int x1, int y1 )
{
	if (Index > 8)
		return;

	SXP[Index].ChangeOutput( x, y, x1, y1 );
}

extern "C" __declspec( dllexport ) void CloseSXP( int Index )
{
	if (Index > 8)
		return;

	SXP[Index].EraseAllDialogs();
}

void ProcessDataXchange();
extern sicExplorer* REQSXP;
extern char* GREQUEST;

extern "C" __declspec( dllexport ) void ProcessSXP( int Index, DialogsSystem* DSS )
{
	ProcessMessages();

	//Parse LW_* commands
	ProcessDataXchange();

	if (Index > 8)
	{
		return;
	}

	sicExplorer* SXPR = SXP + Index;

	if (!SXPR->LastReparseTime)
	{
		SXPR->LastReparseTime = GetTickCount();
	}

	if (GetTickCount() - SXPR->LastReparseTime > 3000)
	{
		SXPR->LastReparseTime = GetTickCount();
		for (int i = 0; i < SXPR->NDownl; i++)
		{
			byte* DNMASK = SXPR->DOWNL[i].ReqMask;
			for (int j = 0; j < 128; j++)
			{
				if (DNMASK[j])
				{
					byte B = DNMASK[j];
					int w = j << 3;
					for (int h = 0; h < 8; h++)
					{
						if (B&( 1 << h ))
						{
							if (w < SXPR->NWindows && SXPR->Windows[w]->Parsed)
							{
								SXPR->Windows[w]->ReParse();
							}
							w++;
						}
					}
				}
			}
		}
	}

	int i = SXPR->CurWPosition;
	if (i >= SXPR->NWindows)
	{
		return;
	}

	SXPR->Windows[i]->Process();
	for (int j = 0; j < SXPR->Windows[i]->NBoxes; j++)
	{
		OneBox* OBX = &SXPR->Windows[i]->Boxes[j];
		if (OBX->VS)
		{
			DialogsSystem* DSS_Boxes = &SXPR->Windows[i]->Boxes[j].DSS;
			int st = SXPR->Windows[i]->Boxes[j].StartScrollIndex;
			int p0 = SXPR->Windows[i]->Boxes[j].LastScrollPos;
			int ymax = 0;
			for (int q = st; q < MAXDLG; q++)
			{
				if (DSS_Boxes->DSS[q])
				{
					int y = p0 + DSS_Boxes->DSS[q]->y1;
					if (y > ymax)
						ymax = y;
				}
			}
			if (ymax > OBX->yi1)
			{
				OBX->VS->SMaxPos = ymax - OBX->yi1 + 10;
				if (OBX->VS->SPos > OBX->VS->SMaxPos)
					OBX->VS->SPos = OBX->VS->SMaxPos;
				OBX->VS->Visible = 1;
			}
			else
			{
				OBX->VS->Visible = 0;
				OBX->VS->SPos = 0;
			}
			if (OBX->VS->SPos != OBX->LastScrollPos)
			{
				int dd = OBX->LastScrollPos - OBX->VS->SPos;
				for (int q = st; q < MAXDLG; q++)
				{
					if (DSS_Boxes->DSS[q])
					{
						DSS_Boxes->DSS[q]->y += dd;
						DSS_Boxes->DSS[q]->y1 += dd;
					}
				}
				OBX->LastScrollPos = OBX->VS->SPos;
			};
			OBX->VS->OnesDy = 8;
			OBX->VS->ScrDy = OBX->y1 - OBX->y - 24;
		};
		OBX->DSS.ProcessDialogs();
		WindX = 0;
		WindX1 = RealLx - 1;
		WindY = 0;
		WindY1 = RealLy - 1;
		WindLx = RealLx;
		WindLy = RealLy;
	}

	ProcessMessages();

	bool BASEACTIVE = 0;
	if (DSS)
	{
		for (int i = 0; i < MAXDLG; i++)
		{
			if (DSS->DSS[i] && DSS->DSS[i]->Active)
			{
				BASEACTIVE = 1;
			}
		}
	}

	if (SXPR->CurWPosition < SXPR->NWindows)
	{
		if (BASEACTIVE && !SXPR->BaseDialogActive)
		{
			OneSicWindow* OSW = SXPR->Windows[SXPR->CurWPosition];
			for (int j = 0; j < OSW->NBoxes; j++)
			{
				DialogsSystem* DSS_Boxes = &OSW->Boxes[j].DSS;
				if (DSS_Boxes)
				{
					for (int p = 0; p < MAXDLG; p++)
					{
						if (DSS_Boxes->DSS[p])DSS_Boxes->DSS[p]->Active = 0;
					}
				}
				OSW->Boxes[j].Active = 0;
				OSW->Boxes[j].WasActive = 0;
			}
			SXPR->BaseDialogActive = 1;
		}
		else
		{
			OneSicWindow* OSW = SXPR->Windows[SXPR->CurWPosition];
			int ActiveIndex = -1;
			for (int j = 0; j < OSW->NBoxes; j++)
			{
				DialogsSystem* DSS_Boxes = &OSW->Boxes[j].DSS;
				if (DSS_Boxes)
				{
					for (int p = 0; p < MAXDLG; p++)
					{
						if (DSS_Boxes->DSS[p] && DSS_Boxes->DSS[p]->Active)
						{
							if (!OSW->Boxes[j].WasActive)
							{
								ActiveIndex = j;
							}
						}
					}
				}
			}

			if (ActiveIndex != -1)
			{
				for (int j = 0; j < OSW->NBoxes; j++)
				{
					if (j != ActiveIndex)
					{
						DialogsSystem* DSS_Boxes = &OSW->Boxes[j].DSS;
						if (DSS_Boxes)
						{
							for (int p = 0; p < MAXDLG; p++)
							{
								if (DSS_Boxes->DSS[p])
								{
									DSS_Boxes->DSS[p]->Active = 0;
								}
							}
						}
						OSW->Boxes[j].Active = 0;
						OSW->Boxes[j].WasActive = 0;
					}
					else
					{
						OSW->Boxes[j].WasActive = 1;
						OSW->Boxes[j].Active = 0;
					}
				}

				if (DSS)
				{
					for (int p = 0; p < MAXDLG; p++)
					{
						if (DSS->DSS[p])
						{
							DSS->DSS[p]->Active = 0;
						}
					}
					SXPR->BaseDialogActive = 0;
				}
			}
		}
	}

	if (GREQUEST)
	{
		char* REQ = GREQUEST;
		sicExplorer* RSXP = REQSXP;
		REQSXP = 0;
		GREQUEST = nullptr;

		SendSmartRequest( RSXP, REQ );

		free( REQ );
	}
}

void EraseSXP()
{
	for (int i = 0; i < 8; i++)
	{
		SXP[i].EraseAllDialogs();
		SXP[i].ID = i;
	};
};

__declspec( dllexport ) char* GetAccessKey( int Index )
{
	return SXP[Index].ACCESSKEY;
};

__declspec( dllexport ) void SetAccessKey( int Index, char* Accesskey )
{
	strncpy( SXP[Index].ACCESSKEY, Accesskey, 15 );
	SXP[Index].ACCESSKEY[15] = 0;
};

extern "C" __declspec( dllexport ) void SXP_SetVar( int Index, char* Name, char* value )
{
	sicExplorer* EXP = SXP + Index;
	for (int i = 0; i < EXP->NGVars; i++)if (!strcmp( EXP->GVARS[i].Name, Name ))
	{
		strncpy( EXP->GVARS[i].Value, value, 511 );
		return;
	};
	if (SXP[Index].CurWPosition < SXP[Index].NWindows)
	{
		OneSicWindow* OSW = SXP[Index].Windows[SXP[Index].CurWPosition];
		char* v = OSW->GetVar( Name );
		if (v)
		{
			strncpy( v, value, 511 );
			v[511] = 0;
		}
		else
		{
			v = OSW->CreateVar( Name, 512 );
			if (v)
			{
				strncpy( v, value, 511 );
				v[511] = 0;
			};
		};
	};
};

extern "C" __declspec( dllexport ) char* SXP_GetVar( int Index, char* Name )
{
	sicExplorer* EXP = SXP + Index;
	for (int i = 0; i < EXP->NGVars; i++)if (!strcmp( EXP->GVARS[i].Name, Name ))return EXP->GVARS[i].Value;
	if (SXP[Index].CurWPosition < SXP[Index].NWindows)
	{
		OneSicWindow* OSW = SXP[Index].Windows[SXP[Index].CurWPosition];
		return OSW->GetVar( Name );
	}
	else return nullptr;
};

void OneSicWindow::SaveCookies()
{
	for (int i = 0; i < NVARS; i++)
	{
		char* var = VARS[i].Name;
		if (var[0] == '%'&&var[1] == 'G'&&var[2] == 'V'&&var[3] == '_')
		{
			char ccc[256];
			sprintf( ccc, "Internet\\Cash\\Cookies\\%s", var );
			ResFile F = RRewrite( ccc );
			if (F != INVALID_HANDLE_VALUE)
			{
				RBlockWrite( F, VARS[i].Value, VARS[i].MaxLen );
				RClose( F );
			};
		};
	};
};

void OneSicWindow::LoadCookies()
{
	for (int i = 0; i < NVARS; i++)
	{
		char* var = VARS[i].Name;
		if (var[0] == '%'&&var[1] == 'G'&&var[2] == 'V'&&var[3] == '_')
		{
			char ccc[256];
			sprintf( ccc, "Internet\\Cash\\Cookies\\%s", var );
			ResFile F = RReset( ccc );
			if (F != INVALID_HANDLE_VALUE)
			{
				RBlockRead( F, VARS[i].Value, VARS[i].MaxLen );
				RClose( F );
			};
		};
	};
};

OneSXPTable* sicExplorer::CreateTable( char* Name )
{
	Tables = (OneSXPTable*) realloc( Tables, ( NTables + 1 ) * sizeof OneSXPTable );
	memset( Tables + NTables, 0, sizeof OneSXPTable );
	strncpy( Tables[NTables].ID, Name, 15 );
	Tables[NTables].ID[15] = 0;
	NTables++;
	return Tables + NTables - 1;
};

OneSXPTable* sicExplorer::GetTable( char* Name )
{
	for (int i = 0; i < NTables; i++)if (!strcmp( Tables[i].ID, Name ))return Tables + i;
	return nullptr;
};

#define BASE 65521L
#define NMAX 5552

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

DWORD GetHVAL( char* buf )
{
	u_int len = strlen( buf );
	const u_long adler = 1L;
	u_long s1 = adler & 0xffff;
	u_long s2 = ( adler >> 16 ) & 0xffff;
	int k;

	if (buf == nullptr) return 1L;

	while (len > 0)
	{
		k = len < NMAX ? len : NMAX;
		len -= k;
		while (k >= 16)
		{
			DO16( buf );
			buf += 16;
			k -= 16;
		}
		if (k != 0) do
		{
			s1 += *buf++;
			s2 += s1;
		} while (--k);
		s1 %= BASE;
		s2 %= BASE;
	}
	return ( s2 << 16 ) | s1;
};

DWORD GetTableHash( OneSXPTable* TB, int Line )
{
	char CC[2048];
	CC[0] = 0;
	if (Line < TB->NLines)
	{
		int nc = TB->NCol;
		for (int j = 0; j < nc; j++)if (TB->COLMOPT[j] == 0)strcat( CC, TB->Lines[Line*nc + j] );
		return GetHVAL( CC );
	}
	else
	{
		return 0;
	}
}

extern __declspec( dllimport ) bool GameInProgress;

void sicExplorer::SendTableRefresh( char* Name, char* server )
{
	if (GameInProgress)return;
	for (int i = 0; i < NTables; i++)if (!strcmp( Tables[i].ID, Name ))
	{
		OneSXPTable* TB = Tables + i;
		ParsedRQ P1;
		strncpy( P1.DevName, server, 15 );
		P1.DevName[15] = 0;
		P1.AddComm( "GETTBL" );
		P1.AddParam( TB->ID, strlen( TB->ID ) + 1 );
		char ccc[512];
		sprintf( ccc, "%d", TB->NLines );
		P1.AddParam( ccc, strlen( ccc ) + 1 );
		DWORD* HSET = (DWORD*) malloc( ( TB->NLines + 1 ) << 2 );
		for (int j = 0; j < TB->NLines; j++)HSET[j] = GetTableHash( TB, j );
		P1.AddParam( (char*) HSET, TB->NLines << 2 );
		int sz = P1.UnParse( nullptr, 0 );
		char* dst = (char*) malloc( sz + 4 );
		sz = P1.UnParse( dst, sz );
		SendGlobalRequest( this, dst, 0 );
		free( HSET );
		free( dst );
	};
};

int sicExplorer::GetGPPictureIndex( char* NAME0 )
{
	char NAME[512];
	sprintf( NAME, NAME0, CurPalette );
	char Name[256];
	strcpy( Name, NAME );
	if (!( strstr( NAME, ".gp" ) || strstr( NAME, ".GP" ) ))strcat( Name, ".gp" );
	if (Name[2] != '*')
	{
		ResFile F = RReset( Name );
		if (F != INVALID_HANDLE_VALUE)
		{
			RClose( F );
			char cc[256];
			strcpy( cc, Name );
			char* ss = strstr( cc, ".gp" );
			if (ss)ss[0] = 0;
			return GPS.PreLoadGPImage( cc );
		};
		char CCX[512];
		sprintf( CCX, "Internet\\Cash\\%s", Name );
		F = RReset( CCX );
		if (F != INVALID_HANDLE_VALUE)
		{
			RClose( F );
			char cc[256];
			strcpy( cc, CCX );
			char* ss = strstr( cc, ".gp" );
			if (ss)ss[0] = 0;
			return GPS.PreLoadGPImage( cc );
		};
		return -1;
	};
	char CCX[512];
	sprintf( CCX, "Internet\\Cash\\%s", Name + 3 );
	ResFile F = RReset( CCX );
	if (F != INVALID_HANDLE_VALUE)
	{
		RClose( F );
		char* ss = strstr( CCX, ".gp" );
		if (ss)ss[0] = 0;
		for (int i = 0; i < NDownl; i++)if (!strcmp( DOWNL[i].URL, Name ))
		{
			memset( DOWNL[i].ReqMask, 0, 128 );
		};
		return GPS.PreLoadGPImage( CCX );
	};
	bool Found = 0;
	for (int i = 0; i < NDownl; i++)if (!strcmp( DOWNL[i].URL, Name ))
	{
		Found = 1;
		if (( !DOWNL[i].Ready ) && CurWPosition < 128 * 8)
		{
			DOWNL[i].ReqMask[CurWPosition >> 3] |= 1 << ( CurWPosition & 7 );
		};
	};
	if (!Found)
	{
		if (NDownl >= MaxDownl)
		{
			MaxDownl += 32;
			DOWNL = (FileDownloadProcess*) realloc( DOWNL, MaxDownl * sizeof FileDownloadProcess );
		};
		//parsing request
		strcpy( DOWNL[NDownl].URL, Name );
		DOWNL[NDownl].Ready = 0;
		strcpy( DOWNL[NDownl].Name, Name + 3 );
		ParsedRQ P1;

		P1.DevName[0] = Name[0];
		P1.DevName[1] = Name[1];
		P1.DevName[2] = 0;
		P1.AddComm( "get" );
		P1.AddParam( DOWNL[NDownl].Name, strlen( DOWNL[NDownl].Name ) + 1 );
		P1.AddParam( DOWNL[NDownl].Name, strlen( DOWNL[NDownl].Name ) + 1 );
		char CCC[512];
		P1.UnParse( CCC, 512 );
		DOWNL[NDownl].Handle = SendGlobalRequest( this, CCC, 0 );
		NDownl++;
	};
	return -1;
};

void OneSicWindow::ReParse()
{
	char** BOXES = nullptr;
	int*   SCRS = nullptr;
	int*   SCMAX = nullptr;
	int    NBOX = 0;
	int    MAXBOX = 0;
	int    NV = NVARS;
	SaveCookies();
	NVARS = 0;
	OneVariable* OV = VARS;
	VARS = nullptr;
	for (int i = 0; i < NBoxes; i++)
	{
		if (NBOX >= MAXBOX)
		{
			MAXBOX += 32;
			BOXES = (char**) realloc( BOXES, MAXBOX << 2 );
			SCRS = (int*) realloc( SCRS, MAXBOX << 2 );
			SCMAX = (int*) realloc( SCMAX, MAXBOX << 2 );
		};
		BOXES[i] = (char*) malloc( strlen( Boxes[i].Name ) + 1 );
		strcpy( BOXES[i], Boxes[i].Name );
		if (Boxes[i].VS)
		{
			SCRS[i] = Boxes[i].VS->SPos;
			SCMAX[i] = Boxes[i].VS->SMaxPos;
		}
		else SCRS[i] = 0;
		NBOX++;
	};
	if (ResizeComm)
	{
		free( ResizeComm );
		ResizeComm = nullptr;
	};
	char* RES = Result;
	char* REQ = REF;
	char WID[32];
	char TMPC[64];
	strcpy( WID, WinID );
	Result = nullptr;
	REF = nullptr;
	sicExplorer* exp = EXP;
	memcpy( TMPC, CurrTempCml, 64 );
	Erase();
	NVARS = NV;
	VARS = OV;
	memcpy( CurrTempCml, TMPC, 64 );
	Result = RES;
	REF = REQ;
	EXP = exp;
	strcpy( WinID, WID );
	ParseTheWholeText();
	LoadCookies();
	Ready = 1;
	Parsed = 1;

	for (int i = 0; i < NBOX; i++)
	{
		for (int j = 0; j < NBoxes; j++)if (!strcmp( Boxes[j].Name, BOXES[i] ))
		{
			if (Boxes[j].VS)
			{
				Boxes[j].VS->SPos = SCRS[i];
				if (Boxes[j].VS->SPos > Boxes[j].VS->SMaxPos)
				{
					Boxes[j].VS->SPos = SCRS[j];
					Boxes[j].VS->SMaxPos = SCMAX[j];
				};
			};
		};
		free( BOXES[i] );
	};
	if (BOXES)
	{
		free( BOXES );
		free( SCRS );
		free( SCMAX );
	};
};

void OneSicWindow::LoadDefaultSettings()
{
	if (CurPalette == 2)
	{
		strcpy( box_gp_file, "Internet\\pix\\i_bor2" );
		strcpy( sbox_gp_file, "Internet\\pix\\i_bor2" );
		strcpy( panel_gp_file, "Internet\\pix\\i_bor2" );
		strcpy( scrl_gp_file, "Interface\\_Slider" );
		strcpy( combo_gp_file, "Interface\\LongBox" );
		strcpy( stb_gp_file, "Internet\\pix\\i_bor2" );
		strcpy( sstb_gp_file, "Internet\\pix\\i_bor2" );
		strcpy( gpb_file, "Internet\\pix\\i_bor2" );
	}
	else if (CurPalette == 1)
	{
		strcpy( box_gp_file, "Internet\\pix\\i_bor1" );
		strcpy( sbox_gp_file, "Internet\\pix\\i_bor1" );
		strcpy( panel_gp_file, "Internet\\pix\\i_bor1" );
		strcpy( scrl_gp_file, "Interface\\_Slider1" );
		strcpy( combo_gp_file, "Interface\\LongBox1" );
		strcpy( stb_gp_file, "Internet\\pix\\i_bor1" );
		strcpy( sstb_gp_file, "Internet\\pix\\i_bor1" );
		strcpy( gpb_file, "Internet\\pix\\i_bor1" );
	}
	else
	{
		strcpy( box_gp_file, "Internet\\pix\\i_bor0" );
		strcpy( sbox_gp_file, "Internet\\pix\\i_bor0" );
		strcpy( panel_gp_file, "Internet\\pix\\i_bor0" );
		strcpy( scrl_gp_file, "Interface\\_Slider" );
		strcpy( combo_gp_file, "Interface\\LongBox0" );
		strcpy( stb_gp_file, "Internet\\pix\\i_bor0" );
		strcpy( sstb_gp_file, "Internet\\pix\\i_bor0" );
		strcpy( gpb_file, "Internet\\pix\\i_bor0" );
	};
	b_LU = 0;
	b_RU = 1;
	b_LD = 2;
	b_RD = 3;
	b_L = 7;
	b_R = 6;
	b_U = 5;
	b_D = 4;

	b_DxL = 1;
	b_DyL = 1;
	b_DxR = 2;//3;
	b_DyR = 2;

	sb_LU = 0;
	sb_RU = 8;
	sb_LD = 2;
	sb_RD = 9;
	sb_L = 7;
	sb_R = 10;
	sb_U = 5;
	sb_D = 4;

	sb_DxL = 1;
	sb_DyL = 1;
	sb_DxR = 16;//15;
	sb_DyR = 2;//3;

	sc_dx = 1;
	sc_dy = -1;
	sc_dly = 3;

	scr_Idx = 0;
	scr_dscroll = 0;
	cmb_Idx = 0;
	cmb_bdx = 24;

	CTBL_UpperLY = 24;
	CTBL_LowLY = 25;
	CTBL_OLY = 26;

	stb_Index = 14;
	stb_dx = 9;
	stb_dy = 20;
	stb_wholedx = 0;
	stb_wholedy = 0;
	stb_wholeLx = 154;

	sstb_Index = 81;
	sstb_dx = 9;
	sstb_dy = 20;
	sstb_wholedx = 0;
	sstb_wholedy = 0;
	sstb_wholeLx = 154;

	btn_style = 0;

	gpb_active = 11;
	gpb_passive = 17;
	gpb_dx = 0;
	gpb_dy = -1;
};

OneInterfaceElement::OneInterfaceElement()
{
	memset( this, 0, sizeof *this );
};

void OneInterfaceElement::ClearActive()
{
	for (int i = 0; i < NActiveRefs; i++)if (ActiveRefs[i])free( ActiveRefs[i] );
	if (ActiveRefs)free( ActiveRefs );
	NActiveRefs = 0;
	ActiveRefs = nullptr;
};

void OneInterfaceElement::ClearParams()
{
	for (int i = 0; i < NParams; i++)if (Params[i])free( Params[i] );
	if (Params)free( Params );
	Params = nullptr;
	NParams = 0;
};

void OneInterfaceElement::Clear()
{
	for (int i = 0; i < NActiveRefs; i++)if (ActiveRefs[i])free( ActiveRefs[i] );
	for (int i = 0; i < NParams; i++)if (Params[i])free( Params[i] );
	if (ActiveRefs)free( ActiveRefs );
	if (Params)free( Params );
	memset( this, 0, sizeof *this );
};