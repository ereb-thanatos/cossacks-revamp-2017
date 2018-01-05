#include "../Main executable/common.h"
#include "IntExplorer.h"
#include <assert.h>
#include <crtdbg.h>
#include "GameOnMap.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "ParseRQ.h"

//#txt[%...](%...:[...position...],{...}...,"...{..}...")
//в тексте
//|pix.gp,dx,dy,idx|-картинка
//{текст}-активная текстовая ссылка
void Replace(char** str, char* src, char* dst, int& MaxL);
extern char HXARR[16];
void SendSmartRequest(sicExplorer* SXP, char* Str);
sicExplorer* REQSXP = nullptr;
char* GREQUEST = nullptr;

bool SendToServer(SimpleDialog* SD)
{
	sicExplorer* SXP = (sicExplorer*)SD->UserParam;
	GREQUEST = (char*)realloc(GREQUEST, strlen(SD->AllocPtr) + 1);
	strcpy(GREQUEST, SD->AllocPtr);
	REQSXP = SXP;
	return false;
}

void SendSmartRequest(sicExplorer* SXP, char* Str)
{
	char* request_string = (char*)malloc(strlen(Str) + 1);
	strcpy(request_string, Str);

	int request_string_length = strlen(request_string) + 1;
	char* request_substring = request_string;
	do
	{
		char* s4 = strstr(request_string, "<#WinH#>");
		if (s4)
		{
			char LY[16];
			sprintf(LY, "%d", SXP->y1 - SXP->y + 1);
			Replace(&request_string, "<#WinH#>", LY, request_string_length);
		}

		s4 = strstr(request_string, "<#WinW#>");
		if (s4)
		{
			char LY[16];
			sprintf(LY, "%d", SXP->x1 - SXP->x + 1);
			Replace(&request_string, "<#WinW#>", LY, request_string_length);
		}

		request_substring = strstr(request_substring, "<%");
		if (request_substring)
		{
			char VAR[64];
			char* s2 = strchr(request_substring, '>');
			if (s2)
			{
				int L = s2 - request_substring - 1;
				if (L < 40)
				{
					memcpy(VAR, request_substring + 1, L);
					VAR[L] = 0;
					bool repl = 0;
					for (int pp = SXP->CurWPosition; pp >= 0 && pp >= SXP->CurWPosition - 1; pp--)
					{
						OneSicWindow* OSW = SXP->Windows[pp];
						OSW->SaveCookies();
						for (int j = 0; j < OSW->NVARS; j++)
							if (!strcmp(OSW->VARS[j].Name, VAR))
							{
								char* s3 = (char*)_ExMalloc(strlen(OSW->VARS[j].Value) * 3 + 1);
								char* vars_value = OSW->VARS[j].Value;
								int L3 = strlen(vars_value);
								int pos = 0;
								for (int q = 0; q < L3; q++)
								{
									char c = vars_value[q];
									if (c < 32 || c == '&' || c == '|' || c == '\\')
									{
										s3[pos] = '\\';
										s3[pos + 1] = HXARR[byte(c) >> 4];
										s3[pos + 2] = HXARR[c & 15];
										pos += 3;
									}
									else
									{
										s3[pos] = c;
										pos++;
									};
								};
								s3[pos] = 0;
								char VAR1[64];
								sprintf(VAR1, "<%s>", VAR);
								Replace(&request_string, VAR1, s3, L3);
								repl = 1;
								request_substring = request_string;
								j = OSW->NVARS;
								_ExFree(s3);
							};
						if (!repl)request_substring++;
					};
				}
				else request_substring++;
			}
			else request_substring++;
		};
	}
	while (request_substring);
	request_substring = request_string;
	ParsedRQ PR;
	PR.Parse(request_string);
	bool change = 0;
	for (int i = 0; i < PR.NComm; i++)
	{
		int N = PR.Comm[i].NParams;
		for (int j = 0; j < N; j++)
		{
			char* st = PR.Comm[i].Params[j];
			if (st)
			{
				if (st[0] == '<' && st[1] == '@' && st[2] == '%')
				{
					char name[512];
					memcpy(name, st + 2, PR.Comm[i].ParamSize[j] - 2);
					name[PR.Comm[i].ParamSize[j] - 3] = 0;
					char* VAR = SXP->GetVar(name);
					if (VAR && VAR[0])
					{
						ResFile F = RReset(VAR);
						if (F != INVALID_HANDLE_VALUE)
						{
							int sz = RFileSize(F);
							if (sz < 16384)
							{
								PR.Comm[i].ParamSize[j] = sz;
								PR.Comm[i].Params[j] = (char*)realloc(PR.Comm[i].Params[j], sz);
								RBlockRead(F, PR.Comm[i].Params[j], sz);
								change = 1;
							};
							RClose(F);
						};
					};
				};
			};
		};
		if (change)
		{
			int sz = PR.UnParse(nullptr, 0);
			request_string = (char*)realloc(request_string, sz + 512);
			PR.UnParse(request_string, sz);
		};
	};
	SendGlobalRequest(SXP, request_string, 1);
	free(request_string);
};
int ReadNumber(char* s, int* L, int vmax);

bool ADI_Txt(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);
	RLCFont* MFONT = SXP->GetFontByName(SXP->FONT3);
	int align = 0;
	if (!strcmp(param, "c"))align = 1;
	if (!strcmp(param, "r"))align = 2;
	int fdy = GetRLCHeight(PFONT->RLC, 'y') + 1;
	if (NParam)
	{
		int y0 = *y;
		int x0 = *x;
		int Lx = (*x1) - (*x);
		char* s = Param[0];

		char c = 0;
		int pos = 0;

		int UsedActives = 0;
		bool curactive = 0;
		bool wasactive = 0;

		char curstr[512];
		int xcs0 = 0;
		int cslx = 0;
		curstr[0] = 0;
		int csmaxdy = fdy;
		bool eof = 0;
		SimpleDialog* SDS[1024];
		int NSDS = 0;
		int maxdy = fdy;
		do
		{
			//reading one word/picture/...
			int nsym = 0;
			int ntotsym = 0;

			//phrase info
			int gpdx = 0;
			int gpdy = 0;
			char OneWord[512];
			int GP_Idx = -1;
			int a_sp = 0;
			int p_sp = 0;
			int reallx = 0;
			bool doit = 1;
			bool eoln = 0;
			wasactive = curactive;
			do
			{
				c = s[pos];
				int L;
				int lx = GetRLCWidthUNICODE(PFONT->RLC, (byte*)(s + pos), &L);
				if ((c == ' ' && nsym) || ((c == '{' || c == '}' || c == '|' || c == 13 || c == 10 || c == 0) && ntotsym) || reallx
					+ lx > Lx)
				{
					//end of phrase
					doit = 0;
				}
				else if (!c)
				{
					doit = 0;
					eof = 1;
				}
				else if (c == 13 || c == 10)
				{
					eoln = 1;
					if (s[pos + 1] == 10 || s[pos + 1] == 13)pos++;
					pos++;
				}
				else if (c == '{' && !ntotsym)
				{
					curactive = 1;
					UsedActives++;
					pos++;
				}
				else if (c == '}' && !ntotsym)
				{
					curactive = 0;
					pos++;
				}
				else if (c == '|' && !ntotsym)
				{
					//picture
					pos++;
					char gp[64];
					int p1 = pos;
					while ((c = s[p1]) != ',' && c != '|' && c)p1++;
					if (c != ',' || p1 - pos > 60)return pos != 0;
					memcpy(gp, s + pos, p1 - pos);
					gp[p1 - pos] = 0;
					char* sgp = strstr(gp, ".gp");
					if (sgp)
					{
						sgp[0] = 0;
						GP_Idx = SXP->GetGPPictureIndex(gp);
						pos = p1 + 1;
						int L2;
						gpdx = ReadNumber(s + pos, &L2, (*x1) - (*x));
						pos += L2 + 1;
						gpdy = ReadNumber(s + pos, &L2, (*x1) - (*x));
						pos += L2 + 1;
						a_sp = ReadNumber(s + pos, &L2, (*x1) - (*x));
						pos += L2 + 1;
						p_sp = ReadNumber(s + pos, &L2, (*x1) - (*x));
						pos += L2 + 1;
						if (GP_Idx != -1)
						{
							GPS.LoadGP(GP_Idx);
							reallx = GPS.GetGPWidth(GP_Idx, p_sp) + gpdx;
							int ly = GPS.GetGPWidth(GP_Idx, p_sp) + gpdy;
							if (ly > maxdy)maxdy = ly;
						};
					};
					OneWord[0] = 0;
					doit = 0;
					//
				}
				else
				{
					//simply the text
					if (ntotsym + L < 500)
					{
						memcpy(OneWord + ntotsym, s + pos, L);
						OneWord[ntotsym + L] = 0;
						ntotsym += L;
						pos += L;
						reallx += lx;
						if (c != ' ')nsym++;
					};
				};
			}
			while (doit);
			if (wasactive != curactive || GP_Idx != -1 || xcs0 + cslx + reallx > Lx || eof || eoln)
			{
				TextButton* TB;
				if (wasactive)
				{
					//if(UsedActives<NActive){
					//	int L1=strlen(curstr);
					//	curstr[L1]=1;
					//	curstr[L1+1]=0;
					//	strcat(curstr,Active[UsedActives-1]);
					//}else{
					//	int L1=strlen(curstr);
					//	curstr[L1+1]=0;
					//};
					TB = DSS->addTextButton(nullptr, x0 + xcs0, y0, curstr, MFONT, AFONT, PFONT, 0);
					//char* s2=strchr(TB->Message,1);
					if (UsedActives <= NActive)
					{
						TB->OnUserClick = &SendToServer;
						TB->UserParam = int(SXP);
						TB->AllocPtr = new char[strlen(Active[UsedActives - 1]) + 1];
						strcpy(TB->AllocPtr, Active[UsedActives - 1]);
					};
				}
				else TB = DSS->addTextButton(nullptr, x0 + xcs0, y0, curstr, PFONT, PFONT, PFONT, 0);
				SDS[NSDS] = TB;
				NSDS++;
				//new string?
				if (xcs0 + cslx + reallx > Lx || eof || eoln)
				{
					y0 += maxdy;
					xcs0 = 0;
					curstr[0] = 0;
					maxdy = fdy;
					cslx = 0;
					if (align)
					{
						int xmin = 10000;
						int xmax = -10000;
						for (int i = 0; i < NSDS; i++)
						{
							if (SDS[i]->x < xmin)xmin = SDS[i]->x;
							if (SDS[i]->x1 > xmax)xmax = SDS[i]->x1;
						};
						if (xmax > xmin)
						{
							if (align == 1)
							{
								int dx = (Lx - xmax + xmin) >> 1;
								for (int i = 0; i < NSDS; i++)
								{
									SDS[i]->x += dx;
									SDS[i]->x1 += dx;
								};
							};
							if (align == 2)
							{
								int dx = (Lx - xmax + xmin);
								for (int i = 0; i < NSDS; i++)
								{
									SDS[i]->x += dx;
									SDS[i]->x1 += dx;
								};
							};
						};
					};
					NSDS = 0;
				}
				else
				{
					xcs0 += cslx;
					cslx = 0;
					curstr[0] = 0;
				};
			};
			if (OneWord[0])
			{
				if (maxdy > csmaxdy)csmaxdy = maxdy;
				int nsp = 0;
				if (!(xcs0 + cslx || eoln))
				{
					while (OneWord[nsp] == ' ')nsp++;
					int L;
					if (nsp)reallx -= GetRLCWidthUNICODE(PFONT->RLC, (byte*)OneWord, &L) * nsp;
				};
				strcat(curstr, OneWord + nsp);
				cslx += reallx;
			}
			else
			{
				if (GP_Idx != -1)
				{
					GP_Button* GPP = DSS->addGP_Button(nullptr, x0 + xcs0 + gpdx, y0 + gpdy, GP_Idx, a_sp, p_sp);
					if (curactive && UsedActives <= NActive)
					{
						GPP->OnUserClick = &SendToServer;
						GPP->AllocPtr = new char[strlen(Active[UsedActives - 1]) + 1];
						strcpy(GPP->AllocPtr, Active[UsedActives - 1]);
						GPP->UserParam = int(SXP);
					}
					xcs0 += reallx;
					cslx = 0;
					curstr[0] = 0;
					SDS[NSDS] = GPP;
					NSDS++;
				};
			};
		}
		while (!eof);
		*y1 = y0;
	};
	return true;
};
//--------------edit box---------------
//#edit[%...](%...[...],{%var_name})
bool ADI_Edit(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (!NActive)return false;
	if (Active[0][0] != '%')return false;
	char* VAR = SXP->CreateVar(Active[0], 512);
	if (NParam)
	{
		char* V = SXP->GetVar(Active[0]);
		if (V)strcpy(V, Param[0]);
	};
	DSS->addInputBox(nullptr, *x, *y, VAR, 500, (*x1) - (*x), (*y1) - (*y), SXP->GetFontByName(SXP->FONT1),
	                 SXP->GetFontByName(SXP->FONT3));
	return true;
};

struct PingST
{
	TextButton* TB;
	DWORD IP;
};

int GETPING(DWORD IP);
//#ping[%...](%...[...],IP_DWORD)
void ProcPing(sicExplorer* SXP, void* PData, int size)
{
	PingST* PING = (PingST*)PData;
	int P = GETPING(PING->IP);
	if (P > 0)
	{
		sprintf(PING->TB->Message, "%d", P);
	}
}

bool ADI_Ping(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (!NParam)
	{
		return false;
	}
	PingST* PING = (PingST*)SXP->CreateSXProcess(&ProcPing, nullptr, sizeof PingST);
	PING->IP = 0;
	sscanf(Param[0], "%u", &PING->IP);
	RLCFont* FNT = SXP->GetFontByName(SXP->FONT1);
	PING->TB = DSS->addTextButton(nullptr, *x, *y, "                               ", FNT, FNT, FNT, 0);
	return true;
}

//-------------combo box---------------
struct ADI_Combo_Data
{
	char VarName[48];
	char URL[512];
	ComboBox* CBB;
};

void CBB_Process(sicExplorer* SXP, void* Data, int size)
{
	ADI_Combo_Data* ADCD = (ADI_Combo_Data*)Data;
	char* var = SXP->GetVar(ADCD->VarName);
	if (var)
	{
		char CC[128];
		sprintf(CC, "%d", ADCD->CBB->CurLine);
		if (strcmp(CC, var))
		{
			sprintf(var, "%d", ADCD->CBB->CurLine);
			if (ADCD->URL[0])SendGlobalRequest(SXP, ADCD->URL, 1);
		};
	};
};
//#cbb[%...](%...[...],{&res_variable},"item1","item2",...)
bool ADI_Combo(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
               int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (SXP->CurWPosition > SXP->NWindows)return true;
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	int cbgp = SXP->GetGPPictureIndex(OSW->combo_gp_file);
	int vsgp = SXP->GetGPPictureIndex(OSW->scrl_gp_file);
	if (cbgp == -1)return true;
	if (!NActive)return false;
	if (Active[0][0] != '%')return false;
	char* VAR = SXP->CreateVar(Active[0], 512);
	ADI_Combo_Data* ADCD = (ADI_Combo_Data*)SXP->CreateSXProcess(&CBB_Process, nullptr, sizeof ADI_Combo_Data);
	strcpy(ADCD->VarName, Active[0]);
	ADCD->CBB = DSS->addGP_ComboBoxDLX(nullptr, *x, *y, (*x1) - (*x), cbgp, OSW->cmb_Idx, 9, 0,
	                                   SXP->GetFontByName(SXP->FONT2), SXP->GetFontByName(SXP->FONT1), nullptr);
	ADCD->CBB->FontDx = OSW->cmb_bdx + 1;
	ADCD->CBB->OneDx = OSW->cmb_bdx;
	ADCD->CBB->Center -= 24 - OSW->cmb_bdx;
	if (vsgp != -1)
	{
		VScrollBar* VSB;
		ADCD->CBB->AssignScroll(DSS, &VSB, vsgp, OSW->scr_Idx, 10);
		VSB->ScrDy = 100;
		VSB->OnesDy = 10;
		VSB->LineLy += OSW->scr_dscroll * 2;
		VSB->btnly -= (byte)OSW->scr_dscroll;
	};
	for (int i = 0; i < NParam - 1; i++)
	{
		ADCD->CBB->AddLine(Param[i]);
	};

	if (VAR[0])
	{
		ADCD->CBB->CurLine = atoi(VAR);
	}
	else
	{
		if (ADCD->CBB->NLines && NParam > 1)
		{
			ADCD->CBB->CurLine = atoi(Param[NParam - 1]);
		};
	};
	if (ADCD->CBB->CurLine >= ADCD->CBB->NLines)ADCD->CBB->CurLine = 0;
	ADCD->URL[0] = 0;
	if (NActive > 1)
	{
		strcpy(ADCD->URL, Active[1]);
	};
	return true;
};

struct ADI_Combo_Data2
{
	char VarName[48];
	char URL[512];
	ComboBox* CBB;
	char** IDS;
	int NIDS;
};

void CBB_Process2(sicExplorer* SXP, void* Data, int size)
{
	ADI_Combo_Data2* ADCD = (ADI_Combo_Data2*)Data;
	char* var = SXP->GetVar(ADCD->VarName);
	if (var)
	{
		if (strcmp(var, ADCD->IDS[ADCD->CBB->CurLine]))
		{
			strcpy(var, ADCD->IDS[ADCD->CBB->CurLine]);
			if (ADCD->URL[0])SendGlobalRequest(SXP, ADCD->URL, 1);
		};
	};
};

void CBB_Close2(sicExplorer* SXP, void* Data, int size)
{
	ADI_Combo_Data2* ADCD = (ADI_Combo_Data2*)Data;
	for (int i = 0; i < ADCD->NIDS; i++)free(ADCD->IDS[i]);
	if (ADCD->IDS)free(ADCD->IDS);
};
//#cbb2[%...](%...[...],{&res_variable},"item1","item2",...)
bool ADI_Combo2(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
                int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (SXP->CurWPosition > SXP->NWindows)return true;
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	int cbgp = SXP->GetGPPictureIndex(OSW->combo_gp_file);
	int vsgp = SXP->GetGPPictureIndex(OSW->scrl_gp_file);
	if (cbgp == -1)return true;
	if (!NActive)return false;
	if (Active[0][0] != '%')return false;
	char* VAR = SXP->CreateVar(Active[0], 512);
	ADI_Combo_Data2* ADCD = (ADI_Combo_Data2*)SXP->CreateSXProcess(&CBB_Process2, &CBB_Close2, sizeof ADI_Combo_Data2);
	strcpy(ADCD->VarName, Active[0]);
	ADCD->CBB = DSS->addGP_ComboBoxDLX(nullptr, *x, *y, (*x1) - (*x), cbgp, OSW->cmb_Idx, 9, 0,
	                                   SXP->GetFontByName(SXP->FONT2), SXP->GetFontByName(SXP->FONT1), nullptr);
	ADCD->CBB->FontDx = OSW->cmb_bdx + 1;
	ADCD->CBB->OneDx = OSW->cmb_bdx;
	if (vsgp != -1)
	{
		VScrollBar* VSB;
		ADCD->CBB->AssignScroll(DSS, &VSB, vsgp, OSW->scr_Idx, 10);
		VSB->ScrDy = 100;
		VSB->OnesDy = 10;
		VSB->LineLy += OSW->scr_dscroll * 2;
		VSB->btnly -= (byte)OSW->scr_dscroll;
	};
	int NN = (NParam - 1) >> 1;
	ADCD->NIDS = 0;
	ADCD->IDS = (char**)malloc(NN << 2);
	for (int i = 0; i < NN; i++)
	{
		ADCD->CBB->AddLine(Param[i + i]);
		ADCD->IDS[i] = (char*)malloc(strlen(Param[i + i + 1]) + 1);
		strcpy(ADCD->IDS[i], Param[i + i + 1]);
		ADCD->NIDS++;
	};

	if (VAR[0])
	{
		for (int i = 0; i < NN; i++)if (!strcmp(ADCD->IDS[i], VAR))ADCD->CBB->CurLine = i;
	}
	else
	{
		if (ADCD->CBB->NLines && NParam > 1)
		{
			ADCD->CBB->CurLine = atoi(Param[NParam - 1]);
		};
	};
	ADCD->URL[0] = 0;
	if (NActive > 1)
	{
		strcpy(ADCD->URL, Active[1]);
	};
	return true;
};
//---------------button----------------
//#btn[%...](%...[...],{ref},"text")
extern int IBOR0;
extern int IBOR2;

bool ADI_Btn(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1)return false;

	if (SXP->CurWPosition > SXP->NWindows)return true;
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	if (OSW->btn_style == 1)
	{
		if (NParam < 1)return false;
		RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
		RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

		GP_TextButton* GPB;
		if (CurPalette == 0)
		{
			GPB = DSS->addStdGP_TextButton(*x, *y, (*x1) - (*x) + 1, Param[0], IBOR0, 11, AFONT, PFONT);
		}
		else
		{
			GPB = DSS->addStdGP_TextButton(*x, *y, (*x1) - (*x) + 1, Param[0], IBOR2, 11, AFONT, PFONT);
		};
		*y1 = *y + GPS.GetGPHeight(IBOR2, 11) - 1;
		if (NActive)
		{
			GPB->AllocPtr = (char*)_ExMalloc(strlen(Active[0]) + 1);
			strcpy(GPB->AllocPtr, Active[0]);
			GPB->OnUserClick = &SendToServer;
			GPB->UserParam = int(SXP);
		};
		return true;
	}
	else
	{
		int sbgp = SXP->GetGPPictureIndex(OSW->stb_gp_file);
		if (sbgp == -1)return true;

		*y += OSW->stb_wholedy;
		*x += OSW->stb_wholedx;
		*y1 += OSW->stb_wholedy;
		*x1 += OSW->stb_wholedx;

		RLCFont* AFONT = SXP->GetFontByName("XXYF");
		RLCFont* MFONT = SXP->GetFontByName("XXWF");
		GP_TextButton* GPB;

		if (CurPalette == 0)
		{
			GPB = DSS->addGP_TextButtonLimited(nullptr, *x, *y,
			                                   Param[0], sbgp, 14, 17, 124, MFONT, AFONT);
			GPB->FontDy--;
		}
		else
		{
			GPB = DSS->addGP_TextButtonLimited(nullptr, *x, *y,
			                                   Param[0], sbgp, 14, 17, 124, MFONT, AFONT);
			GPB->FontDy--;
		};
		if (NActive)
		{
			GPB->AllocPtr = (char*)_ExMalloc(strlen(Active[0]) + 1);
			strcpy(GPB->AllocPtr, Active[0]);
			GPB->OnUserClick = &SendToServer;
			GPB->UserParam = int(SXP);
		};
		return true;
	};
};
//---------------standart button----------------
//#sbtn[%...](%...[...],{ref},"text")
bool ADI_SBtn(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1)return false;

	if (SXP->CurWPosition > SXP->NWindows)return true;
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	int sbgp = SXP->GetGPPictureIndex(OSW->sstb_gp_file);
	if (sbgp == -1)return true;

	*y += OSW->sstb_wholedy;
	*x += OSW->sstb_wholedx;
	*y1 += OSW->sstb_wholedy;
	*x1 += OSW->sstb_wholedx;

	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

	GP_TextButton* GPB;
	int x0 = ((*x) + (*x1)) / 2 - OSW->sstb_wholeLx / 2;
	int y0 = *y;

	DSS->addGPPicture(nullptr, x0, y0, sbgp, OSW->sstb_Index + 3);
	DSS->addGPPicture(nullptr, x0, y0, sbgp, OSW->sstb_Index);
	GPB = DSS->addGP_TextButton(nullptr, x0 + OSW->sstb_dx, y0 + OSW->sstb_dy, Param[0], sbgp, OSW->sstb_Index + 1, AFONT,
	                            PFONT);

	//*y1=*y+GPS.GetGPHeight(IBOR2,11)-1;
	if (NActive)
	{
		GPB->AllocPtr = (char*)_ExMalloc(strlen(Active[0]) + 1);
		strcpy(GPB->AllocPtr, Active[0]);
		GPB->OnUserClick = &SendToServer;
		GPB->UserParam = int(SXP);
	};
	return true;
};

//--------------check box--------------
//#chk[%...](%...[...],{%var},"text","val1","val2","start")
struct ADI_Chk_Data
{
	char VarName[48];
	char URL[512];
	char DEF_st0[256];
	char DEF_st1[256];
	CheckBox* CBB;
};

void CHK_Process(sicExplorer* SXP, void* Data, int size)
{
	ADI_Chk_Data* ADCD = (ADI_Chk_Data*)Data;
	char* var = SXP->GetVar(ADCD->VarName);
	if (var)
	{
		if (ADCD->CBB->State)
		{
			if (strcmp(var, ADCD->DEF_st1))
			{
				strcpy(var, ADCD->DEF_st1);
				if (ADCD->URL[0])SendGlobalRequest(SXP, ADCD->URL, 1);
			};
		}
		else
		{
			if (strcmp(var, ADCD->DEF_st0))
			{
				strcpy(var, ADCD->DEF_st0);
				if (ADCD->URL[0])SendGlobalRequest(SXP, ADCD->URL, 1);
			};
		};
	};
};

bool ADI_Chk(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 4)return false;

	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

	CheckBox* CB;
	char cc[512];
	strcpy(cc, " ");
	strcat(cc, Param[0]);
	if (CurPalette == 0)
	{
		CB = DSS->addGP_CheckBox(nullptr, *x, *y, cc, PFONT, AFONT, 0, 0, IBOR0, 30, 31, 29);
	}
	else
	{
		CB = DSS->addGP_CheckBox(nullptr, *x, *y, cc, PFONT, AFONT, 0, 0, IBOR2, 30, 31, 29);
	};
	CB->Sprite3 = 32;
	if (NActive && Active[0][0] == '%')
	{
		char* VAR = SXP->CreateVar(Active[0], 512);
		ADI_Chk_Data* ADCD = (ADI_Chk_Data*)SXP->CreateSXProcess(&CHK_Process, nullptr, sizeof ADI_Chk_Data);
		strcpy(ADCD->VarName, Active[0]);
		ADCD->CBB = CB;
		strcpy(ADCD->DEF_st0, Param[1]);
		strcpy(ADCD->DEF_st1, Param[2]);
		ADCD->URL[0] = 0;
		if (NActive > 1)
		{
			strcpy(ADCD->URL, Active[1]);
		};
		if (VAR[0] == 0)
		{
			if (!strcmp(Param[3], "0"))CB->State = 0;
			else CB->State = 1;
		}
		else
		{
			if (!strcmp(VAR, Param[1]))CB->State = 0;
			if (!strcmp(VAR, Param[2]))CB->State = 1;
		};
	};
	return 1;
};
//-------------radio button------------
//#rad[%...](%...[...],{%var},"text","grp","val1","val2","start")
bool ADI_Rad(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 5)
		return false;

	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

	CheckBox* CB;
	char cc[512];
	strcpy(cc, " ");
	strcat(cc, Param[0]);
	int grp = atoi(Param[1]);
	if (CurPalette == 0)
	{
		CB = DSS->addGP_CheckBox(nullptr, *x, *y, cc, PFONT, AFONT, grp, 0, IBOR0, 34, 35, 33);
	}
	else
	{
		CB = DSS->addGP_CheckBox(nullptr, *x, *y, cc, PFONT, AFONT, grp, 0, IBOR2, 34, 35, 33);
	};
	CB->Sprite3 = 36;
	if (NActive && Active[0][0] == '%')
	{
		char* VAR = SXP->CreateVar(Active[0], 512);
		ADI_Chk_Data* ADCD = (ADI_Chk_Data*)SXP->CreateSXProcess(&CHK_Process, nullptr, sizeof ADI_Chk_Data);
		strcpy(ADCD->VarName, Active[0]);
		ADCD->CBB = CB;
		strcpy(ADCD->DEF_st0, Param[2]);
		strcpy(ADCD->DEF_st1, Param[3]);
		if (VAR[0])
		{
			if (!strcmp(VAR, Param[2]))CB->State = 0;
			if (!strcmp(VAR, Param[3]))CB->State = 1;
		}
		else
		{
			if (!strcmp(Param[4], "0"))CB->State = 0;
			else CB->State = 1;
		};
		ADCD->URL[0] = 0;
		if (NActive > 1)
		{
			strcpy(ADCD->URL, Active[1]);
		};
	};
	return 1;
};
//------------------panel----------------
//#pan[%...](%...[...],"style")
bool ADI_Pan(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1 || SXP->CurWPosition >= SXP->NWindows)return false;
	int sty = atoi(Param[0]);
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	if (sty < 0 || sty > 15)return true;
	if (!OSW->PANELS[sty].panel_gp[0])return true;
	int GPF = SXP->GetGPPictureIndex(OSW->PANELS[sty].panel_gp);
	if (GPF == -1)return true;
	int PX[8];
	memcpy(PX, &OSW->PANELS[sty].LU, 8 * 4);
	DSS->addCustomBorder(*x, *y, *x1, *y1, GPF, PX, PX, OSW->PANELS[sty].C, OSW->PANELS[sty].C1);
	return true;
};
//------------------panel----------------
//#apan[%...](%...[...],"style","text")
bool ADI_APan(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1 || SXP->CurWPosition >= SXP->NWindows)return false;
	int sty = atoi(Param[0]);
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	if (sty < 0 || sty > 15)return true;
	if (!OSW->PANELS[sty].panel_gp[0])return true;
	int GPF = SXP->GetGPPictureIndex(OSW->PANELS[sty].panel_gp);
	if (GPF == -1)return true;
	int PX[8];
	memcpy(PX, &OSW->PANELS[sty].LU, 8 * 4);
	SimpleDialog* SD = DSS->addCustomBorder(*x, *y, *x1, *y1, GPF, PX, PX, OSW->PANELS[sty].C, OSW->PANELS[sty].C1);
	if (NParam > 1)
	{
		RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
		RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);
		RLCFont* MFONT = SXP->GetFontByName(SXP->FONT3);
		DSS->addTextButton(nullptr, (*x + *x1) / 2, ((*y + *y1 - GetRLCHeight(PFONT->RLC, 'W')) / 2) - 1, Param[1], AFONT,
		                   PFONT, MFONT, 1);
	};
	if (NActive)
	{
		SD->AllocPtr = (char*)_ExMalloc(strlen(Active[0]) + 1);
		strcpy(SD->AllocPtr, Active[0]);
		SD->OnUserClick = &SendToServer;
		SD->UserParam = int(SXP);
	};
	return true;
};

//--------horizontal radio button------
//sort[](%...,{%var},"text","grp","val1","val2","start")
struct ADI_Sort_Data
{
	char VarName[48];
	char DEF_st0[256];
	char DEF_st1[256];
	char DEF_st2[256];
	CheckBox* CBB;
};

void SOR_Process(sicExplorer* SXP, void* Data, int size)
{
	ADI_Sort_Data* ADCD = (ADI_Sort_Data*)Data;
	char* var = SXP->GetVar(ADCD->VarName);
	if (var)
	{
		if (ADCD->CBB->State == 1)strcpy(var, ADCD->DEF_st1);
		else if (ADCD->CBB->State > 1)strcpy(var, ADCD->DEF_st2);
		else strcpy(var, ADCD->DEF_st0);
	};
};

bool ADI_Sort(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 6)return false;
	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);
	CheckBox* CB;
	char cc[512];
	strcpy(cc, " ");
	strcat(cc, Param[0]);
	int grp = atoi(Param[1]);
	if (CurPalette == 0)
	{
		CB = DSS->addGP_CheckBox(nullptr, *x, *y, cc, AFONT, PFONT, grp, 0, IBOR0, 26, 23, 20);
	}
	else
	{
		CB = DSS->addGP_CheckBox(nullptr, *x, *y, cc, AFONT, PFONT, grp, 0, IBOR2, 26, 23, 20);
	};
	CB->Central = 1;
	CB->x1 = *x1;
	if (NActive && Active[0][0] == '%')
	{
		SXP->CreateVar(Active[0], 512);
		ADI_Sort_Data* ADCD = (ADI_Sort_Data*)SXP->CreateSXProcess(&SOR_Process, nullptr, sizeof ADI_Sort_Data);
		strcpy(ADCD->VarName, Active[0]);
		ADCD->CBB = CB;
		strcpy(ADCD->DEF_st0, Param[2]);
		strcpy(ADCD->DEF_st1, Param[3]);
		strcpy(ADCD->DEF_st2, Param[4]);
		if (!strcmp(Param[5], "0"))
		{
			CB->State = 0;
		}
		else
		{
			if (!strcmp(Param[5], "1"))
			{
				CB->State = 1;
			}
			else
			{
				CB->State = 2;
			}
		}
	}
	return 1;
}

//------------empty polygone-----------
//#poly[%...](%...[...],r,g,b,x0,y0,x1,y1,x2,y2,...)
bool ADI_Poly(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 7)return false;
	int r = atoi(Param[0]);
	int g = atoi(Param[1]);
	int b = atoi(Param[2]);
	byte c = GetPaletteColor(r, g, b);
	int N = (NParam - 5) / 2;
	int ymax = 0;
	Canvas* CNV = DSS->AddCanvas(*x, *y, (*x1) - (*x) + 1, (*y1) - (*y) + 1);
	int px = atoi(Param[3]);
	int py = atoi(Param[4]);
	for (int i = 0; i < N; i++)
	{
		int xx = atoi(Param[i * 2 + 5]);
		int yy = atoi(Param[i * 2 + 6]);
		CNV->AddLine(px, py, xx, yy, c);
		if (yy > ymax)ymax = yy;
		if (py > ymax)ymax = py;
		px = xx;
		py = yy;
	};
	*y1 = (*y) + ymax;
	CNV->y1 = *y1;
	return true;
};
//----------------table----------------
//#tbl[%...](%...[...],{%variable},n_columns,"hdr1",sort1,filter1,
//------------simple table-------------
//stbl[](%...,{...}...,nx,btype,align1,lx2,align2,...,Line_height1,"text1_line1",..,Line_height2,"text1_line2",...)
void LimitString(char* str, lpRLCFont FONT, int L)
{
	int L0;
	do
	{
		L0 = GetRLCStrWidth(str, FONT);
		if (L0 > L)
		{
			int LL = strlen(str);
			str[LL - 4] = '.';
			str[LL - 3] = '.';
			str[LL - 2] = '.';
			str[LL - 1] = 0;
		};
	}
	while (L0 > L);
};

bool ADI_Stbl(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1)return false;

	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	int sty = atoi(Param[1]);
	if (sty < 0 || sty > 15)return true;
	if (!OSW->PANELS[sty].panel_gp[0])return true;
	int GPF = SXP->GetGPPictureIndex(OSW->PANELS[sty].panel_gp);
	if (GPF == -1)return true;
	int PX[8];
	memcpy(PX, &OSW->PANELS[sty].LU, 8 * 4);


	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);
	RLCFont* MFONT = SXP->GetFontByName(SXP->FONT3);
	int curac = 0;
	int LXI[256];
	byte align[256];
	int nclm = atoi(Param[0]);

	if (NParam < 2 + 2 * nclm)
	{
		return false;
	}

	int TLX = (*x1) - (*x);
	for (int i = 0; i < nclm; i++)
	{
		LXI[i] = 0;
		LXI[i] = (atoi(Param[2 + i * 2]) * TLX) / 100;;
		align[i] = (byte)atoi(Param[3 + i * 2]);
	}

	int cpr = 2 + nclm * 2;
	int cury = *y;
	bool doit = 1;
	do
	{
		int thisLy = 0;
		if (cpr < NParam)thisLy = atoi(Param[cpr]);
		else doit = 0;
		cpr++;
		if (doit)
		{
			int curx = *x;
			for (int p = 0; p < nclm; p++)
			{
				int bx = curx;
				int bx1 = curx + LXI[p] + 1;
				int by = cury;
				int by1 = cury + thisLy + 1;
				DSS->addCustomBorder(bx, by, bx1, by1, GPF, PX, PX, OSW->PANELS[sty].C, OSW->PANELS[sty].C1);
				if (cpr < NParam)
				{
					char* str = Param[cpr];
					cpr++;
					char tmps[256];
					if (strlen(str) < 255)strcpy(tmps, str);
					else
					{
						memcpy(tmps, str, 255);
						tmps[255] = 0;
					}

					bool active = 0;
					if (tmps[0] == '{')
					{
						active = 1;
						strcpy(tmps, tmps + 1);
					}

					SimpleDialog* SD = nullptr;

					if (tmps[0] == '|')
					{
						//picture
						int p1 = 1;
						char c;
						while ((c = tmps[p1]) != ',' && c != 0)p1++;
						char gp[64];
						if (p1 < 60)
						{
							memcpy(gp, tmps + 1, p1 - 1);
							gp[p1 - 1] = 0;
							char* s2 = strstr(gp, ".gp");
							if (s2)
							{
								s2[0] = 0;
								int GP_Idx = SXP->GetGPPictureIndex(gp);
								p1++;
								int L;
								int gpdx = ReadNumber(tmps + p1, &L, 0);
								p1 += L + 1;
								int gpdy = ReadNumber(tmps + p1, &L, (*x1) - (*x));
								p1 += L + 1;
								int a_sp = ReadNumber(tmps + p1, &L, (*x1) - (*x));
								p1 += L + 1;
								int p_sp = ReadNumber(tmps + p1, &L, (*x1) - (*x));
								p1 += L + 1;
								if (GP_Idx != -1)
								{
									GPS.LoadGP(GP_Idx);
									int Glx = GPS.GetGPWidth(GP_Idx, p_sp);
									int Gly = GPS.GetGPWidth(GP_Idx, p_sp);
									GP_Button* GB = DSS->addGP_Button(nullptr, cury + gpdy + (thisLy - Gly) / 2, curx + gpdx + (LXI[p] - Glx) / 2,
									                                  GP_Idx, a_sp, p_sp);
									SD = GB;
								};
							};
						};
					}
					else
					{
						//text
						RLCFont* AF;
						RLCFont* PF;
						if (active)
						{
							AF = MFONT;
							PF = AFONT;
						}
						else
						{
							AF = PFONT;
							PF = PFONT;
						};
						LimitString(tmps, PF, LXI[p] - 6);
						int FLY = GetRLCHeight(PF->RLC, 'C');
						switch (align[p])
						{
						case 0:
							//left
							SD = DSS->addTextButton(nullptr, curx + 6, cury + (thisLy - FLY) / 2 - 1, tmps, AF, PF, PF, 0);
							break;
						case 1:
							//center
							SD = DSS->addTextButton(nullptr, curx + LXI[p] / 2, cury + (thisLy - FLY) / 2 - 1, tmps, AF, PF, PF, 1);
							break;
						case 2:
							//right
							SD = DSS->addTextButton(nullptr, curx + LXI[p] - 6, cury + (thisLy - FLY) / 2 - 1, tmps, AF, PF, PF, 2);
							break;
						};
					};
					if (active)
					{
						if (curac < NActive)
						{
							SD->OnUserClick = &SendToServer;
							SD->AllocPtr = (char*)_ExMalloc(strlen(Active[curac]) + 1);
							SD->UserParam = (int)SXP;
							strcpy(SD->AllocPtr, Active[curac]);
							curac++;
						};
					};
				}
				else doit = 0;
				curx += LXI[p];
			};
		};
		cury += thisLy;
	}
	while (doit);
	*y1 = cury;
	return true;
};

//-------------------------database table-----------------
//#DBTBL[%...](%...[...],{TABLE_ID}{%var},UpdateFrequency,Style,NColumns,Default_sort_column,Width1,Message1,VisType1,FilterType1,SortType1,ID1,...)
//Style:      TBL,TBL_S,TBL_F,TBL_SF
//VisType:    NON,STR,INT,PING,ENUM:VAL1|VAL2|VAL3...
//FilterType: NON
//            STR,
//            PING:ALL|<...|<...,
//            FIXED:ALL|VAL1|VAL2...,
//            INT:ALL|<...|>...
//SortType:   STR,INT,PING

struct OneDB_Column
{
	char Message[256];
	int Width;
	int x;
	int VisType;
	char VisParams[512];
	char FiltParam[512];
	int FiltType;
	int SortType;
	char ID[32];
	CheckBox* SORT;
	ComboBox* FILT;
};

struct ADI_DBTBL
{
	int NColumns;
	OneDB_Column COLM[16];
	int CurLine;
	int LastUpdateTime;
	int UpdateFreq;
	char DB_REQID[32];
	int CurSortColumn;
	byte Style;
	OneSXPTable* TBL;
	ComplexBox* CBX;
	VScrollBar* VS;
	sicExplorer* SXP;
};

int BORXC[8] = {0, 1, 2, 3, 7, 6, 5, 4};
int BORXR[8] = {0, 8, 2, 9, 7, 10, 5, 4};
int PXA[8] = {37, 38, 39, 40, 41, 42, 44, 43};
int PXP[8] = {37 + 11, 38 + 11, 39 + 11, 40 + 11, 41 + 11, 42 + 11, 44 + 11, 43 + 11};

__declspec( dllimport ) void ShowCharUNICODE(int x, int y, byte* strptr, lpRLCFont lpr);

void ShowPixString(sicExplorer* SXP, int x, int y, int x1, int Ly, RLCFont* F, char* s, bool Active)
{
	TempWindow TW;
	PushWindow(&TW);
	IntersectWindows(x, y, x1, y + Ly - 1);
	byte ch;
	int i = 0;
	do
	{
		ch = s[i];
		if (ch != 0)
		{
			if (ch == '\\' && s[i + 1] == 'p' && s[i + 2] == '{')
			{
				i += 2;
				int pp = i + 1;

				ch = s[pp];
				while (0 != ch && ', ' != ch)
				{
					ch = s[++pp];
				}

				if (pp - i < 64)
				{
					char ccc[68];
					memcpy(ccc, s + i + 1, pp - i - 1);
					ccc[pp - i - 1] = 0;
					pp++;
					int L;
					int gpdx = ReadNumber(s + pp, &L, 0);
					pp += L + 1;
					int gpdy = ReadNumber(s + pp, &L, 0);
					pp += L + 1;
					int a_sp = ReadNumber(s + pp, &L, 0);
					pp += L + 1;
					int p_sp = ReadNumber(s + pp, &L, 0);
					pp += L + 2;
					i = pp - 1;
					int Idx = SXP->GetGPPictureIndex(ccc);
					if (Idx != -1)
					{
						GPS.ShowGP(x + gpdx, y + gpdy, Idx, Active ? a_sp : p_sp, 0);
						x += GPS.GetGPWidth(Idx, Active ? a_sp : p_sp) + gpdx;
					}
				}
			}
			else
			{
				ShowCharUNICODE(x, y, (byte*)(s + i), F);
				int L = 1;
				x += GetRLCWidthUNICODE(F->RLC, (byte*)(s + i), &L);
				i += L - 1;
			}
		}
		i++;
	}
	while (ch);

	PopWindow(&TW);
}

int GETPING(DWORD IP);

void Draw_DBTBL(int x, int y, int Lx, int Ly, int Index, byte Active, int param)
{
	ADI_DBTBL* TBL = (ADI_DBTBL*)param;
	if (Index >= TBL->TBL->NRefs)return;
	int gpid = CurPalette ? IBOR2 : IBOR0;
	if (Active == 2)DrawFilledRect3(x, y, x + Lx - 1, y + Ly - 1, (Corners*)PXA, gpid, 45, 3);
	else if (Active == 1)DrawFilledRect3(x, y, x + Lx - 1, y + Ly - 1, (Corners*)PXP, gpid, 45 + 11, 3);
	int nc = TBL->NColumns;
	int p0 = TBL->TBL->Refs[Index] * nc;
	int sy = GetRLCHeight(YellowFont.RLC, 'C');
	for (int i = 0; i < TBL->TBL->NCol; i++)
	{
		int xcl = TBL->COLM[i].x;
		char* s = TBL->TBL->Lines[p0 + i];
		int vt = TBL->COLM[i].VisType;
		if (vt == 4)
		{
			int idx = atoi(s);
			if (idx >= 0)
			{
				char visparams_buf[512];
				char* visparams = TBL->COLM[i].VisParams;

				int pos = 0;
				while (idx)
				{
					idx--;
					while (visparams[pos] && visparams[pos] != '|')
					{
						pos++;
					}
				}
				if (visparams[pos] == '|')
				{
					pos++;
				}

				int p1 = pos;
				while (visparams[p1] && visparams[p1] != '|')
				{
					p1++;
				}
				if (p1 - pos < 500)
				{
					memcpy(visparams_buf, visparams + pos, p1 - pos);
					visparams_buf[p1 - pos] = 0;
				}
				ShowPixString(TBL->SXP,
				              x + 6 + xcl, y + ((Ly - sy) >> 1) - 1,
				              x + xcl + TBL->COLM[i].Width - 1, 256,
				              (Active == 2 ? &WhiteFont : &YellowFont),
				              visparams_buf, Active == 2);
			}
		}
		else if (vt == 3)
		{
			int idx = atoi(s);
			int p = GETPING(idx);
			if (p)
			{
				char v[16];
				sprintf(v, "%d", p);
				ShowPixString(TBL->SXP, x + 6 + xcl, y + ((Ly - sy) >> 1) - 1, x + xcl + TBL->COLM[i].Width - 1, 256,
				              Active == 2 ? &WhiteFont : &YellowFont, v, Active == 2);
			};
		}
		else if (vt)
		{
			ShowPixString(TBL->SXP, x + 6 + xcl, y + ((Ly - sy) >> 1) - 1, x + xcl + TBL->COLM[i].Width - 1, 256,
			              Active == 2 ? &WhiteFont : &YellowFont, s, Active == 2);
		};
	};
};

void Process_DBTBL(sicExplorer* SXP, void* PData, int size)
{
	ADI_DBTBL* TBL = (ADI_DBTBL*)PData;

	for (int i = 0; i < TBL->NColumns; i++)
	{
		if (TBL->COLM[i].SORT)
		{
			int st = TBL->COLM[i].SORT->State;
			if (st == 2)
			{
				TBL->CurSortColumn = -i - 1;
			}
			if (st == 1)
			{
				TBL->CurSortColumn = i + 1;
			}
		}
	}

	if (TBL->CurSortColumn)
	{
		//sorting
		bool change;
		do
		{
			change = 0;
			int NL = TBL->TBL->NLines;
			int ccl = TBL->CurSortColumn;
			int ncl = TBL->NColumns;
			int accl = abs(ccl) - 1;
			int cp = accl;
			int cp0 = 0;
			int srttp = TBL->COLM[accl].SortType;
			for (int i = 0; i < NL - 1; i++)
			{
				char* S1 = TBL->TBL->Lines[cp];
				char* S2 = TBL->TBL->Lines[cp + ncl];

				int r = 0;
				switch (srttp)
				{
				case 1: //STR
					r = stricmp(S1, S2);
					break;
				case 3: //PING-??
					r = GETPING(atoi(S1)) - GETPING(atoi(S2));
					break;
				case 2: //INT
					r = atoi(S1) - atoi(S1);
					break;
				}

				if ((r > 0 && cp > 0) || (r < 0 && cp < 0))
				{
					for (int p = 0; p < ncl; p++)
					{
						char* cc = TBL->TBL->Lines[cp0 + p];
						TBL->TBL->Lines[cp0 + p] = TBL->TBL->Lines[cp0 + ncl + p];
						TBL->TBL->Lines[cp0 + ncl + p] = cc;
					}
					change = 1;
				}
				cp += ncl;
				cp0 += ncl;
			}
		}
		while (change);
	}

	//filtering
	int nl = TBL->TBL->NLines;
	int ncl = TBL->NColumns;
	TBL->TBL->NRefs = 0;
	for (int i = 0; i < nl; i++)
	{
		bool good = 1;
		for (int j = 0; j < ncl && good; j++)
		{
			int fcl = 0;
			if (TBL->COLM[j].FILT)
			{
				fcl = TBL->COLM[j].FILT->CurLine;
			}
			if (fcl || (TBL->COLM[j].FiltParam[0] && !TBL->COLM[j].FILT))
			{
				int ft = TBL->COLM[j].FiltType;
				int vt = TBL->COLM[j].VisType;
				char sv1[512];
				strncpy(sv1, TBL->TBL->Lines[ncl * i + j], 511);
				sv1[511] = 0;
				if (vt == 4)
				{
					int idx = atoi(sv1);
					if (idx >= 0)
					{
						char* s = TBL->COLM[j].VisParams;
						int pos = 0;
						while (idx)
						{
							idx--;
							while (s[pos] && s[pos] != '|')pos++;
						}
						if (s[pos] == '|')
						{
							pos++;
						}
						int p1 = pos;
						while (s[p1] && s[p1] != '|')p1++;
						if (p1 - pos < 500)
						{
							memcpy(sv1, s + pos, p1 - pos);
							sv1[p1 - pos] = 0;
						}
					}
				}
				switch (ft)
				{
				case 1: //STR
					if (!strstr(sv1, TBL->COLM[j].FiltParam))
					{
						good = 0;
					}
					break;
				case 2: //PING
					sprintf(sv1, "%d", GETPING(atoi(sv1)));
				case 4: //INT
					if (fcl)
					{
						char* st = TBL->COLM[j].FILT->Lines[fcl];
						if (st[0] == '<')
						{
							good = atoi(sv1) < atoi(st + 1);
						}
						else if (st[0] == '>')
						{
							good = atoi(sv1) > atoi(st + 1);
						}
						else
						{
							if (st[0] == '=')
							{
								good = atoi(sv1) == atoi(st + 1);
							}
							else
							{
								good = atoi(sv1) == atoi(st);
							}
						}
					}
					break;
				case 3: //FIXED
					if (fcl)
					{
						char* st = TBL->COLM[j].FILT->Lines[fcl];
						good = !strcmp(st, sv1);
					}
					break;
				}
			}
		}
		if (good)
		{
			TBL->TBL->Refs[TBL->TBL->NRefs] = i;
			TBL->TBL->NRefs++;
		}
	}
	TBL->CBX->N = TBL->TBL->NRefs;
	int p = TBL->CBX->CurrentItem;
	if (p < TBL->TBL->NRefs && p >= 0)
	{
		for (int i = 0; i < ncl; i++)
		{
			char* var = SXP->GetVar(TBL->COLM[i].ID);
			if (var)
			{
				int rp = TBL->TBL->Refs[p];
				strncpy(var, TBL->TBL->Lines[rp * ncl + i], 511);
				var[511] = 0;
			}
		}
	}
	else
	{
		for (int i = 0; i < ncl; i++)
		{
			char* var = SXP->GetVar(TBL->COLM[i].ID);
			if (var)
			{
				var[0] = 0;
			}
		}
	}

	if (TBL->LastUpdateTime)
	{
		if (GetTickCount() - TBL->LastUpdateTime > (unsigned long)TBL->UpdateFreq)
		{
			SXP->SendTableRefresh(TBL->DB_REQID, "GW");
			TBL->LastUpdateTime = GetTickCount();
		}
	}
	else
	{
		SXP->SendTableRefresh(TBL->DB_REQID, "GW");
		TBL->LastUpdateTime = GetTickCount();
	}
}

//Needed for CreateSXProcess
void Close_DBTBL(sicExplorer* SXP, void* PData, int size)
{
}

bool ADI_AddDBTBL(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
                  int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NActive < 2)
	{
		return false;
	}

	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	int scidx = SXP->GetGPPictureIndex(OSW->scrl_gp_file);
	int comidx = SXP->GetGPPictureIndex(OSW->combo_gp_file);
	int borgp = SXP->GetGPPictureIndex(OSW->box_gp_file);
	int sborgp = SXP->GetGPPictureIndex(OSW->sbox_gp_file);
	if (borgp == -1 || sborgp == -1)return true;

	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

	void* data = SXP->CreateSXProcess(&Process_DBTBL, &Close_DBTBL, sizeof ADI_DBTBL);
	ADI_DBTBL* ADI = (ADI_DBTBL*)data;
	memset(ADI, 0, sizeof ADI_DBTBL);
	strncpy(ADI->DB_REQID, Active[0], 31);
	ADI->DB_REQID[31] = 0;
	if (NParam < 4)return false;
	ADI->UpdateFreq = -1;
	ADI->UpdateFreq = atoi(Param[0]);
	ADI->Style = 0;
	if (!strcmp(Param[1], "TBL_S"))ADI->Style = 1;
	else if (!strcmp(Param[1], "TBL_F"))ADI->Style = 2;
	else if (!strcmp(Param[1], "TBL_SF"))ADI->Style = 3;
	ADI->NColumns = 0;
	ADI->NColumns = atoi(Param[2]);
	if (ADI->NColumns > 16)ADI->NColumns = 16;
	ADI->CurSortColumn = 0;
	ADI->CurSortColumn = atoi(Param[3]);
	int cp = 4;
	//int WT=(*x1)-(*x)+1;
	int WT = (*x1) - (*x) + 1;
	int px = 0;
	int LastColumn = 0;
	for (int i = 0; i < ADI->NColumns; i++)
	{
		if (cp + 6 <= NParam)
		{
			int prc = atoi(Param[cp]);
			int ww = ((px + prc) * WT / 100) - (px * WT / 100);

			ADI->COLM[i].x = px * WT / 100;
			px += prc;
			ADI->COLM[i].Width = ww;
			strncpy(ADI->COLM[i].Message, Param[cp + 1], 255);
			ADI->COLM[i].Message[255] = 0;
			char* VT = Param[cp + 2];
			int p = 0;
			while (VT[p] && VT[p] != ':')p++;
			if (p < 16)
			{
				char VTS[16];
				strncpy(VTS, VT, p);
				VTS[p] = 0;
				if (!strcmp(VTS, "STR"))ADI->COLM[i].VisType = 1;
				else if (!strcmp(VTS, "INT"))ADI->COLM[i].VisType = 2;
				else if (!strcmp(VTS, "PING"))ADI->COLM[i].VisType = 3;
				else if (!strcmp(VTS, "ENUM"))
				{
					ADI->COLM[i].VisType = 4;
					if (VT[p] == ':')
					{
						strncpy(ADI->COLM[i].VisParams, VT + p + 1, 511);
						ADI->COLM[i].VisParams[511] = 0;
					};
				}
				else ADI->COLM[i].VisType = 0;
				if (ADI->COLM[i].VisType)LastColumn = i;
			}
			else return false;
			char* FT = Param[cp + 3];
			p = 0;
			while (FT[p] && FT[p] != ':')p++;
			if (p < 16)
			{
				char FTS[16];
				strncpy(FTS, FT, p);
				FTS[p] = 0;
				if (!strcmp(FTS, "STR"))ADI->COLM[i].FiltType = 1;
				else if (!strcmp(FTS, "PING"))
				{
					if (FT[p] == ':')
					{
						strncpy(ADI->COLM[i].FiltParam, FT + p + 1, 511);
						ADI->COLM[i].FiltParam[511] = 0;
						ADI->COLM[i].FiltType = 2;
					};
				}
				else if (!strcmp(FTS, "FIXED"))
				{
					if (FT[p] == ':')
					{
						strncpy(ADI->COLM[i].FiltParam, FT + p + 1, 511);
						ADI->COLM[i].FiltParam[511] = 0;
						ADI->COLM[i].FiltType = 3;
					};
				}
				else if (!strcmp(FTS, "INT"))
				{
					if (FT[p] == ':')
					{
						strncpy(ADI->COLM[i].FiltParam, FT + p + 1, 511);
						ADI->COLM[i].FiltParam[511] = 0;
						ADI->COLM[i].FiltType = 4;
					};
				}
				else ADI->COLM[i].FiltType = 0;
			};
			if (!strcmp(Param[cp + 4], "STR"))ADI->COLM[i].SortType = 1;
			else if (!strcmp(Param[cp + 4], "INT"))ADI->COLM[i].SortType = 2;
			else if (!strcmp(Param[cp + 4], "PING"))ADI->COLM[i].SortType = 3;
			strncpy(ADI->COLM[i].ID, Param[cp + 5], 31);
			ADI->COLM[i].ID[31] = 0;
			SXP->CreateVar(ADI->COLM[i].ID, 512);
			cp += 6;
		};
	};
	//creating the dialog
	int TY0 = *y;
	int TY1 = *y1;
	if (ADI->Style & 1)
	{
		TY0 += OSW->CTBL_UpperLY;
	};
	if (ADI->Style & 2)
	{
		TY1 -= OSW->CTBL_LowLY;
	};
	int NY = (TY1 - TY0) / OSW->CTBL_OLY;

	ComplexBox* CBX = DSS->addComplexBox(*x, *y + OSW->CTBL_UpperLY + (TY1 - TY0 - NY * OSW->CTBL_OLY) / 2, NY,
	                                     OSW->CTBL_OLY, &Draw_DBTBL, -1, 0);
	CBX->x1 = (*x1) - OSW->sb_DxR + OSW->b_DxR;
	CBX->N = 40;
	//CBX->VS=VS;
	for (int i = 0; i < ADI->NColumns; i++)
		if (ADI->COLM[i].Width)
		{
			int BORX[8];
			int dw = 0;
			if (LastColumn != i)
			{
				int BBR[8] = {OSW->b_LU, OSW->b_RU, OSW->b_LD, OSW->b_RD, OSW->b_L, OSW->b_R, OSW->b_U, OSW->b_D};
				memcpy(BORX, BBR, 8 * 4);
			}
			else
			{
				int BBR[8] = {OSW->sb_LU, OSW->sb_RU, OSW->sb_LD, OSW->sb_RD, OSW->sb_L, OSW->sb_R, OSW->sb_U, OSW->sb_D};
				memcpy(BORX, BBR, 8 * 4);
				dw = OSW->b_DxR + OSW->b_DxL - OSW->sb_DxR - OSW->sb_DxL;
			};
			int IBORP;
			if (CurPalette == 0)IBORP = IBOR0;
			else IBORP = IBOR2;

			int Ty0 = *y;
			int Ty1 = *y1;
			int xx = (*x) + ADI->COLM[i].x;
			int xx1 = (*x) + ADI->COLM[i].x + ADI->COLM[i].Width;
			if (ADI->Style & 1)
			{
				DSS->addCustomBorder(xx, *y, xx1, (*y) + OSW->CTBL_UpperLY, IBORP, BORX, BORX, -1, -1);
				Ty0 += OSW->CTBL_UpperLY;

				CheckBox* CB;
				CB = DSS->addGP_CheckBox(nullptr, xx + OSW->b_DxL, (*y) + OSW->b_DyL, ADI->COLM[i].Message, AFONT, PFONT, 1, 0,
				                         CurPalette ? IBOR2 : IBOR0, 26, 23, 20);
				CB->Central = 1;
				CB->x1 = xx1 - OSW->b_DxR + dw;

				ADI->COLM[i].SORT = CB;
			};
			if (ADI->Style & 2)
			{
				int ft = ADI->COLM[i].FiltType;
				DSS->addCustomBorder(xx, (*y1) - OSW->CTBL_LowLY, xx1, *y1, IBORP, BORX, BORX, -1, -1);
				Ty1 -= OSW->CTBL_LowLY;
				if (ft > 1 && ft <= 4)
				{
					ComboBox* CBB = DSS->addGP_ComboBoxDLX(nullptr, xx + OSW->b_DxL, (*y1) - OSW->CTBL_LowLY + 1,
					                                       xx1 - xx - OSW->b_DxR - OSW->b_DxL + dw/*+24-OSW->cmb_bdx*/, comidx,
					                                       OSW->cmb_Idx, 9, 0, AFONT, PFONT, ADI->COLM[i].FiltParam);
					CBB->FontDx = OSW->cmb_bdx + 1;
					CBB->OneDx = OSW->cmb_bdx;
					CBB->Center -= 24 - OSW->cmb_bdx;
					ADI->COLM[i].FILT = CBB;
				};
				if (ft == 1)
				{
					DSS->addInputBox(nullptr,
					                 xx + 3, (*y1) - OSW->CTBL_LowLY + 1,
					                 ADI->COLM[i].FiltParam, 256,
					                 xx1 - xx - 8, OSW->CTBL_LowLY - 2,
					                 PFONT, AFONT);
				}
			}
			DSS->addCustomBorder(xx, Ty0, xx1, Ty1, IBORP, BORX, BORX, -1, -1);
		};
	if (scidx != -1)
	{
		VScrollBar* VS = DSS->addNewGP_VScrollBar(nullptr, (*x1) - OSW->sb_DxR + OSW->sc_dx, TY0 + OSW->sb_DyL + OSW->sc_dy,
		                                          TY1 - TY0 + 1 + OSW->sc_dly - OSW->sb_DyL - OSW->sb_DyR, 100, 0, scidx,
		                                          OSW->scr_Idx);
		VS->ScrDy = 6 * 10;
		VS->OnesDy = 10;
		CBX->VS = VS;
		ADI->VS = VS;
		VS->LineLy += OSW->scr_dscroll * 2;
		VS->btnly -= (byte)OSW->scr_dscroll;
	};
	ADI->CBX = CBX;
	ADI->TBL = SXP->GetTable(ADI->DB_REQID);
	ADI->SXP = SXP;
	if (!ADI->TBL)ADI->TBL = SXP->CreateTable(ADI->DB_REQID);
	for (int i = 0; i < ADI->NColumns; i++)if (ADI->COLM[i].ID[1] == '%')ADI->TBL->COLMOPT[i] = 1;
	ADI->TBL->NCol = ADI->NColumns;
	CBX->param = int(data);
	return true;
};

//----------------------------------------------------------------------------------------
//#pix[%...](%...[...],{ref},filename,StartFrame,EndFrame,ActiveStartFrame,ActiveEndFrame)
struct PixProcess
{
	int PStart;
	int PEnd;
	int AStart;
	int AEnd;
	int GP_Idx;
	int time;
	GP_Button* GPB;
};

void ShowPixProcess(sicExplorer* SXP, void* PData, int size)
{
	PixProcess* PP = (PixProcess*)PData;
	PP->time++;
	if (PP->AStart != PP->AEnd)
	{
		if (PP->AStart > PP->AEnd)PP->GPB->ActiveFrame = PP->AStart + (PP->time % (PP->AEnd - PP->AStart));
		else PP->GPB->ActiveFrame = PP->AStart - (PP->time % (PP->AStart - PP->AEnd));
	};
	if (PP->PStart != PP->PEnd)
	{
		if (PP->PStart > PP->PEnd)PP->GPB->PassiveFrame = PP->PStart + (PP->time % (PP->PEnd - PP->PStart));
		else PP->GPB->PassiveFrame = PP->PStart - (PP->time % (PP->PStart - PP->PEnd));
	};
};

bool ADI_Pix(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	int gpid = SXP->GetGPPictureIndex(Param[0]);
	if (gpid == -1)return true;

	PixProcess* PP = (PixProcess*)SXP->CreateSXProcess(&ShowPixProcess, nullptr, sizeof PixProcess);
	PP->PStart = atoi(Param[1]);
	PP->PEnd = atoi(Param[2]);
	PP->AStart = atoi(Param[3]);
	PP->AEnd = atoi(Param[4]);
	PP->time = 0;
	if (!strcmp(param, "1"))
	{
		*x += (*x1 - *x - GPS.GetGPWidth(gpid, PP->AStart)) / 2;
	};
	PP->GPB = DSS->addGP_Button(nullptr, *x, *y, gpid, PP->AStart, PP->PStart);
	*y1 = *y + GPS.GetGPHeight(gpid, PP->AStart);
	return true;
};
//-------------------------------------------------------------------
//#ubtn[...](...,gp-file,background,dx,dy,active,passive,text)
bool ADI_Ubtn(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 7)return false;
	int gpid = SXP->GetGPPictureIndex(Param[0]);
	if (gpid == -1)return true;

	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

	int back = atoi(Param[1]);
	int dx = atoi(Param[2]);
	int dy = atoi(Param[3]);

	int actv = atoi(Param[4]);
	int pasv = atoi(Param[5]);

	if (back != -1)DSS->addGPPicture(nullptr, *x, *y, gpid, back);
	if (pasv != -1)DSS->addGPPicture(nullptr, (*x) + dx, (*y) + dy, gpid, pasv);
	GP_TextButton* GPB = DSS->addGP_TextButton(nullptr, (*x) + dx, (*y) + dy, Param[6], gpid, actv, AFONT, PFONT);
	if (NActive)
	{
		GPB->AllocPtr = (char*)_ExMalloc(strlen(Active[0]) + 1);
		strcpy(GPB->AllocPtr, Active[0]);
		GPB->OnUserClick = &SendToServer;
		GPB->UserParam = int(SXP);
	};
	return true;
};
//---------------standart button----------------
//#def_gp_btn(gp-file,Active,Passive,dx,dy)
//#gpbtn[%...](%...[...],{ref},"text")
bool ADI_gpBtn(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
               int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1)return false;

	if (SXP->CurWPosition > SXP->NWindows)return true;
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	int sbgp = SXP->GetGPPictureIndex(OSW->gpb_file);
	if (sbgp == -1)return true;
	int LX = GPS.GetGPWidth(sbgp, OSW->gpb_passive);
	int LY = GPS.GetGPHeight(sbgp, OSW->gpb_passive);

	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	RLCFont* AFONT = SXP->GetFontByName(SXP->FONT2);

	GP_TextButton* GPB;
	int x0 = ((*x) + (*x1)) / 2 - LX / 2;
	int y0 = ((*y) + (*y1)) / 2 - LY / 2;
	DSS->addGP_Button(nullptr, x0, y0, sbgp, OSW->gpb_passive, OSW->gpb_passive);
	GPB = DSS->addGP_TextButton(nullptr, x0, y0, Param[0], sbgp, OSW->gpb_active, PFONT, AFONT);
	GPB->FontDx += OSW->gpb_dx;
	GPB->FontDy += OSW->gpb_dy;

	if (NActive)
	{
		GPB->AllocPtr = (char*)_ExMalloc(strlen(Active[0]) + 1);
		strcpy(GPB->AllocPtr, Active[0]);
		GPB->OnUserClick = &SendToServer;
		GPB->UserParam = int(SXP);
	};
	return true;
};

bool ADI_def_gp_btn(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
                    int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1)return false;
	if (SXP->CurWPosition > SXP->NWindows)return true;
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	strncpy(OSW->gpb_file, Param[0], 127);
	OSW->gpb_file[127] = 0;
	if (NParam > 1)OSW->gpb_active = (short)atoi(Param[1]);
	if (NParam > 2)OSW->gpb_passive = (short)atoi(Param[2]);
	if (NParam > 3)OSW->gpb_dx = (short)atoi(Param[3]);
	if (NParam > 4)OSW->gpb_dy = (short)atoi(Param[4]);
	return true;
};
//--------------------Hint assignment---------------
//#hint(ID,"message")
bool ADI_Hint(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 2)
	{
		return false;
	}

	if (SXP->CurWPosition > SXP->NWindows)
	{
		return true;
	}

	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	char* ID = Param[0];
	char* Hint = Param[1];
	for (int i = 0; i < OSW->NBoxes; i++)
	{
		int NP = OSW->Boxes[i].NISPOS;
		for (int j = 0; j < NP; j++)
		{
			if (!strcmp(OSW->Boxes[i].ISPOS[j].ID, ID))
			{
				DialogsSystem* DSS_Boxes = &OSW->Boxes[i].DSS;
				if (DSS_Boxes)
				{
					int spos = OSW->Boxes[i].ISPOS[j].StartSD;
					int fpos = OSW->Boxes[i].ISPOS[j].FinSD;
					for (int q = spos; q < fpos; q++)
					{
						if (DSS_Boxes->DSS[q])
						{
							DSS_Boxes->DSS[q]->Hint = (char*)malloc(strlen(Hint) + 1);
							strcpy(DSS_Boxes->DSS[q]->Hint, Hint);
							DSS_Boxes->DSS[q]->AllocHint = 1;
						}
					}
				}
			}
		}
	}
	return true;
}

//--------------Hint showing params-------------------
//#hintparam(%...[...],font)
bool ADI_HintParam(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
                   int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 1)
	{
		return false;
	}

	RLCFont* PFONT = SXP->GetFontByName(Param[0]);
	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];

	for (int i = 0; i < OSW->NBoxes; i++)
	{
		int NP = OSW->Boxes[i].NISPOS;
		for (int j = 0; j < NP; j++)
		{
			DialogsSystem* DSS_Boxes = &OSW->Boxes[i].DSS;
			if (DSS_Boxes)
			{
				DSS_Boxes->HintX = *x;
				DSS_Boxes->HintY = *y;
				DSS_Boxes->HintFont = PFONT;
			}
		}
	}
	return true;
}

//#time(time-ms,"command",timerid)
bool ADI_Time(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 2)return false;
	if (NParam > 2)
	{
		int V = atoi(Param[2]);
		if (V >= 0 && V < NTIME)
		{
			OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
			OSW->TimeLimit[V] = GetTickCount() + atoi(Param[0]);
			strcpy(OSW->TimeLimitRequest[V], Param[1]);
		};
	}
	else
	{
		OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
		OSW->TimeLimit[0] = GetTickCount() + atoi(Param[0]);
		strcpy(OSW->TimeLimitRequest[0], Param[1]);
	};
	return true;
};
//-----------------------------MAP CONTROL------------------------//
//#map[...](%...[...],{MAPID}{sendcom})
void DrawMap(int x, int y, int Lx, int Ly, int Index, byte Active, int param)
{
	BIGMAP.WX = x;
	BIGMAP.WY = y;
	BIGMAP.WLX = Lx;
	BIGMAP.WLY = Ly;
	BIGMAP.ShowMapPart();
};

bool MapClick(SimpleDialog* SD)
{
	//search for arrow click
	POINT PT;
	GetCursorPos(&PT);
	for (int i = 0; i < BIGMAP.NMapPix; i++)
	{
		OneMapPicture* OMP = BIGMAP.MapPix + i;
		if (OMP->Drawn && CheckGP_Inside(OMP->LastGPID, OMP->Sprite, PT.x - OMP->LastX, PT.y - OMP->LastY) && OMP->URL[0])
		{
			SendSmartRequest((sicExplorer*)SD->UserParam, OMP->URL);
			return false;
		};
	};
	OneScale* CUR = BIGMAP.SCALE + BIGMAP.CurScale;
	int x = (PT.x - SD->x + BIGMAP.CurX) * BIGMAP.CountryLx / CUR->Lx;
	int y = (PT.y - SD->y + BIGMAP.CurY) * BIGMAP.CountryLy / CUR->Ly;
	char cc[256];
	sprintf(cc, SD->AllocPtr, BIGMAP.Title, x, y);
	SendSmartRequest((sicExplorer*)SD->UserParam, cc);
	return true;
};

bool ADI_Map(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
             int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NActive < 2)return false;
	if (strcmp(BIGMAP.Title, Active[0]))
	{
		BIGMAP.SetTitle(Active[0]);
		BIGMAP.LoadMapData(SXP);
		if (!BIGMAP.NScales)return false;
		BIGMAP.SetCenterCoor(BIGMAP.SCALE[BIGMAP.NScales - 1].Lx / 2, BIGMAP.SCALE[BIGMAP.NScales - 1].Ly / 2);
	};
	CustomBox* MAP = DSS->addCustomBox(*x, *y, *x1 - *x + 1, *y1 - *y + 1, &DrawMap);
	MAP->OnUserClick = &MapClick;
	MAP->AllocPtr = (char*)malloc(strlen(Active[1]) + 1);
	strcpy(MAP->AllocPtr, Active[1]);
	MAP->UserParam = int(SXP);
	return true;
};
//#murl(Index,URL)
bool ADI_murl(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 2)return false;
	int idx = atoi(Param[0]);
	if (idx >= 0 && idx < BIGMAP.NMapPix)
	{
		strcpy(BIGMAP.MapPix[idx].URL, Param[1]);
	};
	return true;
};
//#uurl(StartIndex,URL(%s),s1,s2,s3,...)
bool ADI_uurl(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 2)return false;
	char URL[512];
	int idx = atoi(Param[0]);
	for (int i = 2; i < NParam; i++)
	{
		sprintf(URL, Param[1], Param[i]);
		if (idx >= 0 && idx < BIGMAP.NMapPix)
		{
			strcpy(BIGMAP.MapPix[idx].URL, URL);
		};
		idx++;
	};
	return true;
};
//-------------------file downloader---------------------------
//#getf("server_ref")
extern bool NOXCFILE;

void sgetFile(sicExplorer* SXP, char* URL)
{
	char ccc[256];
	sprintf(ccc, "Internet/cash/%s", URL);
	struct _stat ST;
	_stat(ccc, &ST);
	sprintf(ccc, "GW|file&%s\\00&%u\\00", URL, (int)ST.st_mtime);
	SendSmartRequest(SXP, ccc);
};

bool ADI_getf(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NOXCFILE)return true;
	if (NParam < 1)return false;
	sgetFile(SXP, Param[0]);
	return true;
};
//---------------------text file viewer------------------//
//#ftxt[](%..[..],filenameincash,addspace)
bool ADI_ftxt(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
              int NActive, char** Active, int NParam, char** Param, char* param)
{
	RLCFont* PFONT = SXP->GetFontByName(SXP->FONT1);
	if (NParam < 1)return false;
	char ccc[256];
	sprintf(ccc, "Internet/Cash/%s", Param[0]);
	ResFile F = RReset(ccc);
	if (F != INVALID_HANDLE_VALUE)
	{
		RClose(F);
		TextViewer* TV = DSS->addTextViewer(nullptr, *x, *y, (*x1) - (*x) + 1, 100000, ccc, PFONT);
		word SSIZE = TV->SymSize;
		if (NParam > 1)
		{
			SSIZE += (word)atoi(Param[1]);
			TV->SymSize = SSIZE;
		};
		*y1 = *y + TV->NLines * SSIZE;
		TV->y1 = *y1;
		return true;
	}
	else
	{
		*y1 = *y;
		return false;
	};
};

//---------------------------file browser-----------------------------
//#fbrowse({%RESULTVAR},%ComboControl,TextFrameID,FileMask)
struct FBrowse
{
	sicExplorer* SXP;
	char VarName[32];
	char FileNameID[32];
	char ShortNameID[32];
	char FrameID[32];
	char FileMask[32];
	ComboBox* Disk;
	char* CurrPath;
	int LastDisk;
};

FBrowse* LastBrowse = nullptr;

void SortStrings(char** strs, int nstr)
{
	bool change = 0;
	do
	{
		change = 0;
		for (int i = 1; i < nstr; i++)
		{
			if (strcmp(strs[i], strs[i - 1]) < 0)
			{
				char* s = strs[i];
				strs[i] = strs[i - 1];
				strs[i - 1] = s;
				change = 1;
			};
		};
	}
	while (change);
};
extern sicExplorer* REQSXP;
extern char* GREQUEST;
void EnumFilesInDirectory(char* Dir, char** mask, int NMasks, char* FrameID, char* BoxID);

void FileClickProc(char** strs, int nstr)
{
	char* Dir = LastBrowse->SXP->GetVar(LastBrowse->VarName);
	if (!strcmp(strs[0], "updir"))
	{
		int L = strlen(Dir);
		while (L && Dir[L - 1] != '\\' && Dir[L - 1] != '/')L--;
		if (L)Dir[L - 1] = 0;
		char* s = LastBrowse->FileMask;
		EnumFilesInDirectory(Dir, &s, 1, LastBrowse->FrameID, LastBrowse->FrameID);
	}
	else if (!strcmp(strs[0], "chdir"))
	{
		strcat(Dir, "\\");
		strcat(Dir, strs[1]);
		char* s = LastBrowse->FileMask;
		EnumFilesInDirectory(Dir, &s, 1, LastBrowse->FrameID, LastBrowse->FrameID);
	}
	else if (!strcmp(strs[0], "file"))
	{
		char* FL = LastBrowse->SXP->GetVar(LastBrowse->FileNameID);
		strcpy(FL, Dir);
		strcat(FL, "\\");
		strcat(FL, strs[1]);
		int L = strlen(FL);
		for (int i = 0; i < L; i++)if (FL[i] == '\\')FL[i] = '/';
		FL = LastBrowse->SXP->GetVar(LastBrowse->ShortNameID);
		strcpy(FL, strs[1]);
	};
};
#define ADDFL(name,com
) if(cstr)fprintf(F,mask,cstr,BoxID,cstr-1,DWORD(FileClickProc),com,name);else fprintf(F,mask0,BoxID,DWORD(FileClickProc),com,name);

void EnumFilesInDirectory(char* Dir, char** Mask, int NMasks, char* FrameID, char* BoxID)
{
	char tmp[512];
	char way[512];
	strcpy(tmp, Dir);

	char** Files = nullptr;
	int NFiles = 0;
	int MaxFiles = 0;

	char** Dirs = nullptr;
	int NDirs = 0;
	int MaxDirs = 0;

	int L = strlen(Dir);
	if (tmp[L - 1] == '\\' || tmp[L - 1] == '/')tmp[L - 1] = 0;
	for (int i = 0; i < NMasks; i++)
	{
		sprintf(way, "%s\\%s", tmp, Mask[i]);
		WIN32_FIND_DATA FD;
		HANDLE H = FindFirstFile(way, &FD);
		if (H != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(strchr(FD.cFileName, '&') || strchr(FD.cFileName, '|')))
				{
					if (FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (strcmp(FD.cFileName, ".") && strcmp(FD.cFileName, ".."))
						{
							if (NDirs >= MaxDirs)
							{
								MaxDirs += 512;
								Dirs = (char**)realloc(Dirs, MaxDirs << 2);
							};
							Dirs[NDirs] = (char*)malloc(strlen(FD.cFileName) + 1);
							strcpy(Dirs[NDirs], FD.cFileName);
							NDirs++;
						};
					}
					else
					{
						if (NFiles >= MaxFiles)
						{
							MaxFiles += 512;
							Files = (char**)realloc(Files, MaxFiles << 2);
						};
						Files[NFiles] = (char*)malloc(strlen(FD.cFileName) + 1);
						strcpy(Files[NFiles], FD.cFileName);
						NFiles++;
					};
				};
			}
			while (FindNextFile(H, &FD));
		};
	};
	//sorting files&dirs
	if (NFiles > 400)NFiles = 400;
	if (NDirs > 200)NDirs = 200;
	SortStrings(Files, NFiles);
	SortStrings(Dirs, NDirs);
	FILE* F = fopen("Internet\\Cash\\tempcml_files.cml", "w");
	fprintf(F, "<%s>\n", FrameID);
	int cstr = 0;
	char* mask0 = "#txt[%%FDR0](%s[x:0,y:0,w:1000,h:1000],{LW_proc&%u&%s},\"{%s}\")\n";
	char* mask = "#txt[%%FDR%d](%s[x:0,y:%%FDR%d,w:1000,h:1000],{LW_proc&%u&%s},\"{%s}\")\n";
	fprintf(F, "#font(WF,YF,WF)\n");
	if (strlen(Dir) > 3)
	{
		ADDFL( "|Internet/Cash/folder.gp,0,0,1,1|..                                                                       ",
			"updir" );
		cstr++;
	};
	for (int i = 0; i < NDirs; i++)
	{
		char cd[256];
		sprintf(cd, "chdir&%s", Dirs[i]);
		char cc[256];
		sprintf(cc, "|Internet/Cash/folder.gp,0,0,0,0|%s", Dirs[i]);
		ADDFL( cc, cd );
		cstr++;
	};
	for (int i = 0; i < NFiles; i++)
	{
		char cd[256];
		sprintf(cd, "file&%s", Files[i]);
		ADDFL( Files[i], cd );
		cstr++;
	};
	fprintf(F, "<%s>\n", FrameID);
	fclose(F);
	char CCC[256];
	sprintf(CCC, "LW_file&Internet/Cash/tempcml_files.cml\00");
	GREQUEST = (char*)realloc(GREQUEST, strlen(CCC) + 1);
	strcpy(GREQUEST, CCC);
	REQSXP = LastBrowse->SXP;
};

void BrowseChDisk(sicExplorer* SXP, void* PData, int size)
{
	if (LastBrowse->LastDisk != LastBrowse->Disk->CurLine)
	{
		char* VAR = SXP->GetVar(LastBrowse->VarName);
		strcpy(VAR, LastBrowse->Disk->Lines[LastBrowse->Disk->CurLine]);
		VAR[2] = 0;
		char* s = LastBrowse->FileMask;
		EnumFilesInDirectory(VAR, &s, 1, LastBrowse->FrameID, LastBrowse->FrameID);
	};
};

extern bool InReparse;

bool ADI_FBrowse(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
                 int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NActive < 3 || NParam < 3)
	{
		return false;
	}

	if (SXP->CurWPosition > SXP->NWindows)
	{
		return true;
	}

	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];
	//search for combo box
	ComboBox* CBB = nullptr;
	for (int i = 0; i < OSW->NBoxes; i++)
	{
		OneBox* OB = OSW->Boxes + i;
		for (int j = 0; j < OB->NISPOS; j++)
		{
			if (!strcmp(OB->ISPOS[j].ID, Param[0]))
			{
				CBB = (ComboBox*)OB->DSS.DSS[OB->ISPOS[j].StartSD];
				j = OB->NISPOS;
				i = OSW->NBoxes;
			}
		}
	}
	if (CBB)
	{
		FBrowse* FB = (FBrowse*)SXP->CreateSXProcess(&BrowseChDisk, nullptr, sizeof FBrowse);
		FB->Disk = CBB;
		FB->SXP = SXP;
		LastBrowse = FB;
		strcpy(FB->FileMask, Param[2]);
		strcpy(FB->FrameID, Param[1]);
		strcpy(FB->VarName, Active[0]);
		//disk enum
		CBB->Clear();
		char CC[256];
		GetLogicalDriveStrings(255, CC);
		char* cc = CC;

		char* VAR = SXP->CreateVar(Active[0], 512);

		strcpy(FB->FileNameID, Active[1]);
		strcpy(FB->ShortNameID, Active[2]);
		char DSK = VAR[0];
		if (DSK >= 'a' && DSK <= 'z')DSK += 'A' - 'a';
		do
		{
			strupr(cc);
			if (cc[0] && cc[0] >= 'C')
			{
				CBB->AddLine(cc);
				if (DSK == cc[0])
				{
					CBB->CurLine = CBB->NLines - 1;
				}
			}
			if (cc[0])
			{
				cc += strlen(cc) + 1;
			}
		}
		while (cc[0]);
		FB->CurrPath = VAR;
		if (!VAR[0])
		{
			GetCurrentDirectory(511, VAR);
		}
		int L = strlen(VAR);
		if (L)
		{
			if (VAR[L - 1] == '/' || VAR[L - 1] == '\\')VAR[L - 1] = 0;
		}
		if (!InReparse)
		{
			EnumFilesInDirectory(VAR, &Param[2], 1, Param[1], Param[1]);
		}
		FB->LastDisk = CBB->CurLine;
	}
	return true;
}

//#nfont(FONTID,gp-file,color_index,shadow)
bool ADI_NewFont(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
                 int NActive, char** Active, int NParam, char** Param, char* param)
{
	if (NParam < 3)
	{
		return false;
	}
	if (SXP->CurWPosition > SXP->NWindows)
	{
		return true;
	}

	OneSicWindow* OSW = SXP->Windows[SXP->CurWPosition];

	int GPID = SXP->GetGPPictureIndex(Param[1]);
	if (GPID == -1)return true;
	if (!OSW->NAddFonts)OSW->ADFonts = (OneAddFont*)malloc(64 * sizeof OneAddFont);
	if (OSW->NAddFonts > 64)return false;
	strcpy(OSW->ADFonts[OSW->NAddFonts].FID, Param[0]);
	memset(&OSW->ADFonts[OSW->NAddFonts].FONT, 0, sizeof OSW->ADFonts[OSW->NAddFonts].FONT);
	OSW->ADFonts[OSW->NAddFonts].FONT.SetGPIndex(GPID);
	OSW->ADFonts[OSW->NAddFonts].FONT.SetColorTable(atoi(Param[2]));
	OSW->ADFonts[OSW->NAddFonts].sdx = 0;
	OSW->ADFonts[OSW->NAddFonts].sdy = 0;
	if (NParam == 6)
	{
		OSW->ADFonts[OSW->NAddFonts].sdx = atoi(Param[4]);
		OSW->ADFonts[OSW->NAddFonts].sdy = atoi(Param[5]);
	};
	if (!strcmp(Param[3], "std"))OSW->ADFonts[OSW->NAddFonts].FONT.SetStdShadow();
	if (strcmp(Param[3], "none"))
	{
		int SGPID = SXP->GetGPPictureIndex(Param[3]);
		if (SGPID != -1)
		{
			OSW->ADFonts[OSW->NAddFonts].FONT.SetShadowGP(SGPID, 0, 0);
			if (NParam == 6)
			{
				OSW->ADFonts[OSW->NAddFonts].FONT.SetShadowGP(SGPID, atoi(Param[4]), atoi(Param[5]));
			};
		};
	}
	else
	{
		OSW->ADFonts[OSW->NAddFonts].FONT.SetShadowGP(-1, 0, 0);
	};
	OSW->NAddFonts++;
	return true;
};

OneInterfaceFunction* IFNS;

int N_IFNS = 0;

void InitIFNS()
{
	N_IFNS = 32;
	IFNS = new OneInterfaceFunction[N_IFNS];
	strcpy(IFNS[0].Name, "#txt");
	IFNS[0].AddInterface = &ADI_Txt;
	IFNS[0].ReqActiv = 1;
	IFNS[0].ReqCoor = 1;
	IFNS[0].ReqName = 1;
	IFNS[0].ReqParam = 1;
	strcpy(IFNS[0].param, "l");

	strcpy(IFNS[1].Name, "#ctxt");
	IFNS[1].AddInterface = &ADI_Txt;
	IFNS[1].ReqActiv = 1;
	IFNS[1].ReqCoor = 1;
	IFNS[1].ReqName = 1;
	IFNS[1].ReqParam = 1;
	strcpy(IFNS[1].param, "c");

	strcpy(IFNS[2].Name, "#rtxt");
	IFNS[2].AddInterface = &ADI_Txt;
	IFNS[2].ReqActiv = 1;
	IFNS[2].ReqCoor = 1;
	IFNS[2].ReqName = 1;
	IFNS[2].ReqParam = 1;
	strcpy(IFNS[2].param, "r");

	strcpy(IFNS[3].Name, "#edit");
	IFNS[3].AddInterface = &ADI_Edit;
	IFNS[3].ReqActiv = 1;
	IFNS[3].ReqCoor = 1;
	IFNS[3].ReqName = 1;
	IFNS[3].ReqParam = 1;
	strcpy(IFNS[3].param, "");

	strcpy(IFNS[4].Name, "#cbb");
	IFNS[4].AddInterface = &ADI_Combo;
	IFNS[4].ReqActiv = 1;
	IFNS[4].ReqCoor = 1;
	IFNS[4].ReqName = 1;
	IFNS[4].ReqParam = 1;
	strcpy(IFNS[4].param, "");

	strcpy(IFNS[5].Name, "#poly");
	IFNS[5].AddInterface = &ADI_Poly;
	IFNS[5].ReqActiv = 0;
	IFNS[5].ReqCoor = 1;
	IFNS[5].ReqName = 1;
	IFNS[5].ReqParam = 1;
	strcpy(IFNS[5].param, "");

	strcpy(IFNS[6].Name, "#btn");
	IFNS[6].AddInterface = &ADI_Btn;
	IFNS[6].ReqActiv = 1;
	IFNS[6].ReqCoor = 1;
	IFNS[6].ReqName = 1;
	IFNS[6].ReqParam = 1;
	strcpy(IFNS[6].param, "");

	strcpy(IFNS[7].Name, "#chk");
	IFNS[7].AddInterface = &ADI_Chk;
	IFNS[7].ReqActiv = 1;
	IFNS[7].ReqCoor = 1;
	IFNS[7].ReqName = 1;
	IFNS[7].ReqParam = 1;
	strcpy(IFNS[7].param, "");

	strcpy(IFNS[8].Name, "#rad");
	IFNS[8].AddInterface = &ADI_Rad;
	IFNS[8].ReqActiv = 1;
	IFNS[8].ReqCoor = 1;
	IFNS[8].ReqName = 1;
	IFNS[8].ReqParam = 1;
	strcpy(IFNS[8].param, "");

	strcpy(IFNS[9].Name, "#pan");
	IFNS[9].AddInterface = &ADI_Pan;
	IFNS[9].ReqActiv = 0;
	IFNS[9].ReqCoor = 1;
	IFNS[9].ReqName = 1;
	IFNS[9].ReqParam = 1;
	strcpy(IFNS[9].param, "");

	strcpy(IFNS[10].Name, "#sort");
	IFNS[10].AddInterface = &ADI_Sort;
	IFNS[10].ReqActiv = 1;
	IFNS[10].ReqCoor = 1;
	IFNS[10].ReqName = 1;
	IFNS[10].ReqParam = 1;
	strcpy(IFNS[10].param, "");

	strcpy(IFNS[11].Name, "#stbl");
	IFNS[11].AddInterface = &ADI_Stbl;
	IFNS[11].ReqActiv = 1;
	IFNS[11].ReqCoor = 1;
	IFNS[11].ReqName = 1;
	IFNS[11].ReqParam = 1;
	strcpy(IFNS[11].param, "");

	strcpy(IFNS[12].Name, "#sbtn");
	IFNS[12].AddInterface = &ADI_SBtn;
	IFNS[12].ReqActiv = 1;
	IFNS[12].ReqCoor = 1;
	IFNS[12].ReqName = 1;
	IFNS[12].ReqParam = 1;
	strcpy(IFNS[12].param, "");

	strcpy(IFNS[13].Name, "#DBTBL");
	IFNS[13].AddInterface = &ADI_AddDBTBL;
	IFNS[13].ReqActiv = 1;
	IFNS[13].ReqCoor = 1;
	IFNS[13].ReqName = 1;
	IFNS[13].ReqParam = 1;

	strcpy(IFNS[14].Name, "#pix");
	IFNS[14].AddInterface = &ADI_Pix;
	IFNS[14].ReqActiv = 1;
	IFNS[14].ReqCoor = 1;
	IFNS[14].ReqName = 1;
	IFNS[14].ReqParam = 1;
	strcpy(IFNS[14].param, "");

	strcpy(IFNS[15].Name, "#ubtn");
	IFNS[15].AddInterface = &ADI_Ubtn;
	IFNS[15].ReqActiv = 1;
	IFNS[15].ReqCoor = 1;
	IFNS[15].ReqName = 1;
	IFNS[15].ReqParam = 1;
	strcpy(IFNS[15].param, "");

	strcpy(IFNS[16].Name, "#cbb2");
	IFNS[16].AddInterface = &ADI_Combo2;
	IFNS[16].ReqActiv = 1;
	IFNS[16].ReqCoor = 1;
	IFNS[16].ReqName = 1;
	IFNS[16].ReqParam = 1;
	strcpy(IFNS[16].param, "");

	strcpy(IFNS[17].Name, "#gpbtn");
	IFNS[17].AddInterface = &ADI_gpBtn;
	IFNS[17].ReqActiv = 1;
	IFNS[17].ReqCoor = 1;
	IFNS[17].ReqName = 1;
	IFNS[17].ReqParam = 1;
	strcpy(IFNS[17].param, "");

	strcpy(IFNS[18].Name, "#def_gp_btn");
	IFNS[18].AddInterface = &ADI_def_gp_btn;
	IFNS[18].ReqActiv = 0;
	IFNS[18].ReqCoor = 0;
	IFNS[18].ReqName = 0;
	IFNS[18].ReqParam = 1;
	strcpy(IFNS[18].param, "");

	strcpy(IFNS[19].Name, "#cpix");
	IFNS[19].AddInterface = &ADI_Pix;
	IFNS[19].ReqActiv = 1;
	IFNS[19].ReqCoor = 1;
	IFNS[19].ReqName = 1;
	IFNS[19].ReqParam = 1;
	strcpy(IFNS[19].param, "1");

	strcpy(IFNS[20].Name, "#apan");
	IFNS[20].AddInterface = &ADI_APan;
	IFNS[20].ReqActiv = 1;
	IFNS[20].ReqCoor = 1;
	IFNS[20].ReqName = 1;
	IFNS[20].ReqParam = 1;
	strcpy(IFNS[20].param, "");

	strcpy(IFNS[21].Name, "#hint");
	IFNS[21].AddInterface = &ADI_Hint;
	IFNS[21].ReqActiv = 0;
	IFNS[21].ReqCoor = 0;
	IFNS[21].ReqName = 0;
	IFNS[21].ReqParam = 1;
	strcpy(IFNS[21].param, "");

	strcpy(IFNS[22].Name, "#hintparam");
	IFNS[22].AddInterface = &ADI_HintParam;
	IFNS[22].ReqActiv = 0;
	IFNS[22].ReqCoor = 1;
	IFNS[22].ReqName = 0;
	IFNS[22].ReqParam = 1;
	strcpy(IFNS[22].param, "");

	strcpy(IFNS[23].Name, "#time");
	IFNS[23].AddInterface = &ADI_Time;
	IFNS[23].ReqActiv = 0;
	IFNS[23].ReqCoor = 0;
	IFNS[23].ReqName = 0;
	IFNS[23].ReqParam = 1;
	strcpy(IFNS[23].param, "");

	strcpy(IFNS[24].Name, "#map");
	IFNS[24].AddInterface = &ADI_Map;
	IFNS[24].ReqActiv = 1;
	IFNS[24].ReqCoor = 1;
	IFNS[24].ReqName = 1;
	IFNS[24].ReqParam = 0;
	strcpy(IFNS[24].param, "");

	strcpy(IFNS[25].Name, "#getf");
	IFNS[25].AddInterface = &ADI_getf;
	IFNS[25].ReqActiv = 0;
	IFNS[25].ReqCoor = 0;
	IFNS[25].ReqName = 0;
	IFNS[25].ReqParam = 1;
	strcpy(IFNS[25].param, "");

	strcpy(IFNS[26].Name, "#ftxt");
	IFNS[26].AddInterface = &ADI_ftxt;
	IFNS[26].ReqActiv = 0;
	IFNS[26].ReqCoor = 1;
	IFNS[26].ReqName = 1;
	IFNS[26].ReqParam = 1;
	strcpy(IFNS[26].param, "");

	strcpy(IFNS[27].Name, "#murl");
	IFNS[27].AddInterface = &ADI_murl;
	IFNS[27].ReqActiv = 0;
	IFNS[27].ReqCoor = 0;
	IFNS[27].ReqName = 0;
	IFNS[27].ReqParam = 1;
	strcpy(IFNS[27].param, "");

	strcpy(IFNS[28].Name, "#uurl");
	IFNS[28].AddInterface = &ADI_uurl;
	IFNS[28].ReqActiv = 0;
	IFNS[28].ReqCoor = 0;
	IFNS[28].ReqName = 0;
	IFNS[28].ReqParam = 1;
	strcpy(IFNS[28].param, "");

	strcpy(IFNS[29].Name, "#fbrowse");
	IFNS[29].AddInterface = &ADI_FBrowse;
	IFNS[29].ReqActiv = 1;
	IFNS[29].ReqCoor = 0;
	IFNS[29].ReqName = 0;
	IFNS[29].ReqParam = 1;
	strcpy(IFNS[29].param, "");

	strcpy(IFNS[30].Name, "#ping");
	IFNS[30].AddInterface = &ADI_Ping;
	IFNS[30].ReqActiv = 0;
	IFNS[30].ReqCoor = 1;
	IFNS[30].ReqName = 1;
	IFNS[30].ReqParam = 1;
	strcpy(IFNS[30].param, "");

	strcpy(IFNS[31].Name, "#nfont");
	IFNS[31].AddInterface = &ADI_NewFont;
	IFNS[31].ReqActiv = 0;
	IFNS[31].ReqCoor = 0;
	IFNS[31].ReqName = 0;
	IFNS[31].ReqParam = 1;
	strcpy(IFNS[31].param, "");
};
