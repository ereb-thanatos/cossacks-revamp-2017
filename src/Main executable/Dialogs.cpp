#include "Cdirsnd.h"
#include "ddini.h"
#include "ResFile.h"
#include "fastdraw.h"
#include "mapdiscr.h"
#include "mouse.h"
#include "Gsound.h"
#include "Dialogs.h"
#include "fonts.h"
#include <assert.h>
#include <crtdbg.h>
#include "GP_Draw.h"
#include "bmptool.h"
#include "DrawForm.h"
int ROLLU2 = 1;
extern int UNI_LINEDLY1;
extern int UNI_LINEDY1;
extern int UNI_LINEDLY2;
extern int UNI_LINEDY2;
extern int UNI_HINTDY1;
extern int UNI_HINTDLY1;
extern int UNI_HINTDY2;
extern int UNI_HINTDLY2;

extern bool realLpressed;
extern bool KeyPressed;
extern int LastKey;
extern bool MUSTDRAW;
extern int RealLx;
extern int RealLy;
extern char* SoundID[MaxSnd];
extern byte LastAsciiKey;
void ShowCharUNICODE(int x, int y, byte* strptr, lpRLCFont lpr);
void ShowChar(int x, int y, char c, lpRLCFont lpf);
void ClearKeyStack();
int ReadKey();
void AddKey(byte Key, byte Ascii);
extern int CurPalette;

void ErrD(LPCSTR s)
{
	char pal[128];
	sprintf(pal, "%d\\agew_1.pal", CurPalette);
	LoadPalette(pal);
	MessageBox(hwnd, s, "Loading failed...", MB_ICONWARNING | MB_OK);
	assert( false );
}

void ShowString(int x, int y, LPCSTR lps, lpRLCFont lpf);

extern int curptr;

__declspec( dllexport ) void SetCurPtr(int v)
{
	curptr = v;
}

__declspec( dllexport ) int GetSound(char* Name)
{
	if (!NSounds)return -1;
	int ID = SearchStr(SoundID, Name, NSounds);
	if (ID == -1)
	{
		char cc[128];
		sprintf(cc, "Unknown sound ID: %s", Name);
		MessageBox(nullptr, cc, "ERROR!", 0);
		ErrM(cc);
		return -1;
	};
	return ID;
}

DialogsSystem::DialogsSystem(int x, int y)
{
	memset(this, 0, sizeof *this);
	BaseX = x;
	BaseY = y;
	HintY = RealLy - 80;
	OkSound = GetSound("STANDART");
	CancelSound = GetSound("STANDART");
	UserClickSound = GetSound("STANDART");
};

DialogsSystem::DialogsSystem()
{
	memset(this, 0, sizeof *this);
	HintY = RealLy - 80;
	OkSound = GetSound("STANDART");
	CancelSound = GetSound("STANDART");
	UserClickSound = GetSound("STANDART");
};

void DialogsSystem::SetFonts(RLCFont* fActive,
                             RLCFont* fPassive,
                             RLCFont* fDisabled,
                             RLCFont* fMessage)
{
	Active = fActive;
	Passive = fPassive;
	Disabled = fDisabled;
	Message = fMessage;
};
//-----------------class Picture
bool Picture_OnMouseOver(SimpleDialog* SD)
{
	Picture* Pic = (Picture*)SD;
	if (Pic->ActivePicture != Pic->PassivePicture)
		SD->NeedToDraw = true;
	return false;
};

bool Picture_OnDraw(SimpleDialog* SD)
{
	if (!SD)return false;
	if (!SD->Visible)return false;
	Picture* pic = (Picture*)SD;
	if (!pic->Enabled && pic->DisabledPicture)
	{
		pic->DisabledPicture->Draw(pic->x, pic->y);
		return true;
	};
	if (pic->MouseOver && pic->ActivePicture)
	{
		pic->ActivePicture->Draw(pic->x, pic->y);
		return true;
	};
	if (pic->PassivePicture)
	{
		pic->PassivePicture->Draw(pic->x, pic->y);
		return true;
	};
	return false;
};

Picture* DialogsSystem::addPicture(
	SimpleDialog* Parent,
	int px, int py,
	SQPicture* pActive,
	SQPicture* pPassive,
	SQPicture* pDisabled
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);

	if (i < MAXDLG)
	{
		Picture* pic = new Picture;
		DSS[i] = pic;
		if (pActive != pPassive)
		{
			pic->MouseOverActive = true;
		}
		pic->NeedToDraw = true;
		pic->Parent = Parent;
		pic->x = BaseX + px;
		pic->y = BaseY + py;
		pic->x1 = pic->x + pPassive->PicPtr[0] - 1;
		pic->y1 = pic->y + pPassive->PicPtr[1] - 1;
		pic->ActivePicture = pActive;
		pic->PassivePicture = pPassive;
		pic->DisabledPicture = pDisabled;
		pic->OnDraw = &Picture_OnDraw;
		pic->OnMouseOver = &Picture_OnMouseOver;
		pic->Enabled = true;
		pic->Active = false;
		return pic;
	};
	return nullptr;
};
//-----------------class Canvas----------//
bool CanvasDestroy(SimpleDialog* SD)
{
	Canvas* CAN = (Canvas*)SD;
	if (CAN->DrawData)
	{
		free(CAN->DrawData);
	};
	return true;
};
#define CV_INT(x) (*(int*)(CAN->DrawData+pos+x))
#define CV_BYTE(x) CAN->DrawData[pos+x]
__declspec( dllexport ) void CBar(int x, int y, int Lx, int Ly, byte c);
void xLine(int x, int y, int x1, int y1, byte c);

bool CanvasDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	TempWindow TW;
	PushWindow(&TW);
	Canvas* CAN = (Canvas*)SD;
	if (!(CAN->L))return true;
	IntersectWindows(SD->x, SD->y, SD->x1, SD->y1);
	int ymin = SD->y1 - 500;
	int ymax = SD->y1 + 500;
	int pos = 0;
	int x0 = CAN->x;
	int y0 = CAN->y;
	if (CAN->VS)
	{
		if (CAN->BottomY > CAN->y1 - CAN->y)
		{
			CAN->VS->Visible = 1;
			CAN->VS->Enabled = 1;
			CAN->VS->SMaxPos = CAN->BottomY - CAN->y1 + CAN->y + 1;
			if (CAN->VS->SPos > CAN->VS->SMaxPos)CAN->VS->SPos = CAN->VS->SMaxPos;
			y0 -= CAN->VS->SPos;
		}
		else
		{
			CAN->VS->Visible = 0;
			CAN->VS->Enabled = 0;
		};
	};
	byte* data = CAN->DrawData;
	int YY;
	do
	{
		switch (data[pos])
		{
		case 0: //bar
			YY = y0 + CV_INT( 5 );
			if (YY > ymin)CBar(x0 + CV_INT( 1 ), YY, CV_INT( 9 ), CV_INT( 13 ), CV_BYTE( 17 ));
			if (YY > ymax)goto XXZ;
			pos += 18;
			break;
		case 1: //Line
			YY = y0 + CV_INT( 5 );
			if (YY > ymin)xLine(x0 + CV_INT( 1 ), YY, x0 + CV_INT( 9 ), y0 + CV_INT( 13 ), CV_BYTE( 17 ));
			if (YY > ymax)goto XXZ;
			pos += 18;
			break;
		case 2: //Rect
			YY = y0 + CV_INT( 5 );
			if (YY > ymin)Xbar(x0 + CV_INT( 1 ), YY, CV_INT( 9 ), CV_INT( 13 ), CV_BYTE( 17 ));
			if (YY > ymax)goto XXZ;
			pos += 18;
			break;
		case 3: //sprite
			YY = y0 + CV_INT( 5 );
			if (YY > ymin)GPS.ShowGP(x0 + CV_INT( 1 ), YY, CV_INT( 9 ), CV_INT( 13 ), 0);
			if (YY > ymax)goto XXZ;
			pos += 17;
			break;
		case 4: //text
			YY = y0 + CV_INT( 5 );
			if (YY > ymin)ShowString(x0 + CV_INT( 1 ), YY, (char*)(CAN->DrawData + pos + 13),
			                         *((RLCFont**)(CAN->DrawData + pos + 9)));
			if (YY > ymax)goto XXZ;
			pos += strlen((char*)(CAN->DrawData + pos + 13)) + 14;
			break;
		case 5: //centered text
			{
				YY = y0 + CV_INT( 5 );
				char* str = (char*)(CAN->DrawData + pos + 13);
				if (YY > ymin)
				{
					RLCFont* RF = *((RLCFont**)(CAN->DrawData + pos + 9));
					int L = GetRLCStrWidth(str, RF) >> 1;
					ShowString(x0 + CV_INT( 1 ) - L, y0 + CV_INT( 5 ), str, RF);
					if (y0 + CV_INT( 5 ) > ymax)goto XXZ;
				};
				pos += strlen(str) + 14;
			};
			break;
		default:
			return false;
		};
	}
	while (pos < CAN->L);
XXZ:;
	PopWindow(&TW);
	return true;
};

void Canvas::CheckSize(int sz)
{
	if (sz > MaxL - 100)
	{
		MaxL += 1024;
		DrawData = (byte*)realloc(DrawData, MaxL);
	};
};
#define CN_INT(x) (*(int*)(DrawData+L))=x;L+=4;
#define CN_SHORT(x) (*(int*)(DrawData+L))=x;L+=2;
#define CN_BYTE(x) DrawData[L]=x;L++;

void Canvas::AddBar(int px, int py, int Lx, int Ly, byte c)
{
	CheckSize(L + 100);
	CN_BYTE( 0 );
	CN_INT( px );
	CN_INT( py );
	CN_INT( Lx );
	CN_INT( Ly );
	CN_BYTE( c );
};

void Canvas::AddLine(int px, int py, int px1, int py1, byte c)
{
	CheckSize(L + 100);
	CN_BYTE( 1 );
	CN_INT( px );
	CN_INT( py );
	CN_INT( px1 );
	CN_INT( py1 );
	CN_BYTE( c );
};

void Canvas::AddRect(int px, int py, int Lx, int Ly, byte c)
{
	CheckSize(L + 100);
	CN_BYTE( 2 );
	CN_INT( px );
	CN_INT( py );
	CN_INT( Lx );
	CN_INT( Ly );
	CN_BYTE( c );
};

void Canvas::AddSprite(int px, int py, int GPIDX, int Sprite)
{
	CheckSize(L + 100);
	CN_BYTE( 3 );
	CN_INT( px );
	CN_INT( py );
	CN_INT( GPIDX );
	CN_INT( Sprite );
};

void Canvas::AddText(int px, int py, char* Text, RLCFont* Font)
{
	CheckSize(L + 100);
	CN_BYTE( 4 );
	CN_INT( px );
	CN_INT( py );
	int V = int(Font);
	CN_INT( V );
	strcpy((char*)(DrawData + L), Text);
	L += 1 + strlen(Text);
};

void Canvas::AddCText(int px, int py, char* Text, RLCFont* Font)
{
	CheckSize(13 + 1 + strlen(Text));
	CN_BYTE( 5 );
	CN_INT( px );
	CN_INT( py );
	int V = int(Font);
	CN_INT( V );
	strcpy((char*)(DrawData + L), Text);
	L += 1 + strlen(Text);
};

Canvas* DialogsSystem::AddCanvas(int x, int y, int Lx, int Ly)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		Canvas* CAN = new Canvas;
		DSS[i] = CAN;
		CAN->x = x;
		CAN->y = y;
		CAN->x1 = x + Lx - 1;
		CAN->y1 = y + Ly - 1;
		CAN->Active = 0;
		CAN->Parent = nullptr;
		CAN->Enabled = 1;
		CAN->DrawData = nullptr;
		CAN->L = 0;
		CAN->MaxL = 0;
		CAN->Destroy = &CanvasDestroy;
		CAN->OnDraw = &CanvasDraw;
		CAN->VS = nullptr;
		return CAN;
	};
	return 0;
};
//-----------------class GPPicture
bool GPPicture_OnMouseOver(SimpleDialog* SD)
{
	return false;
}

bool GPPicture_OnDraw(SimpleDialog* SD)
{
	if (!SD)
		return false;

	if (!SD->Visible)
		return false;

	GPPicture* pic = (GPPicture*)SD;
	GPS.ShowGP(pic->x, pic->y, pic->FileID, pic->SpriteID, pic->Nation);

	return false;
}

GPPicture* DialogsSystem::addGPPicture(
	SimpleDialog* Parent,
	int dx, int dy, int FileID, int SpriteID)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		GPPicture* pic = new GPPicture;
		DSS[i] = pic;

		if (Active != Passive)
		{
			pic->MouseOverActive = true;
		}
		pic->NeedToDraw = true;
		pic->Parent = Parent;
		pic->x = BaseX + dx;
		pic->y = BaseY + dy;
		pic->x1 = pic->x + GPS.GetGPWidth(FileID, SpriteID & 4095) - 1;
		pic->y1 = pic->y + GPS.GetGPHeight(FileID, SpriteID & 4095) - 1;
		pic->FileID = FileID;
		pic->SpriteID = SpriteID;
		pic->OnDraw = &GPPicture_OnDraw;
		pic->OnMouseOver = nullptr;
		pic->Enabled = true;
		pic->Active = false;
		pic->Nation = 0;
		return pic;
	}
	return nullptr;
}

//-----------------class RLCPicture
bool RLCPicture_OnDraw(SimpleDialog* SD)
{
	if (!SD)
		return false;

	if (!SD->Visible)
		return false;

	RLCPicture* pic = (RLCPicture*)SD;
	if (!pic->Enabled && pic->DisabledPicture)
	{
		ShowRLCItem(pic->x, pic->y, pic->DisabledPicture, pic->dpic, 0);
		return true;
	}

	if (pic->MouseOver && pic->ActivePicture)
	{
		ShowRLCItem(pic->x, pic->y, pic->ActivePicture, pic->apic, 0);
		return true;
	}

	if (pic->PassivePicture)
	{
		ShowRLCItem(pic->x, pic->y, pic->PassivePicture, pic->dpic, 0);
		return true;
	}
	return false;
}

RLCPicture* DialogsSystem::addRLCPicture(
	SimpleDialog* Parent, int x, int y,
	RLCTable* pActive, byte apic,
	RLCTable* pPassive, byte ppic,
	RLCTable* pDisabled, byte dpic
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);

	if (i < MAXDLG)
	{
		RLCPicture* pic = new RLCPicture;
		DSS[i] = pic;
		if (pActive != pPassive)
		{
			pic->MouseOverActive = true;
		}
		pic->NeedToDraw = true;
		pic->Parent = Parent;
		pic->x = BaseX + x;
		pic->y = BaseY + y;
		pic->x1 = pic->x + GetRLCWidth(*pPassive, ppic);
		pic->y1 = pic->y + GetRLCHeight(*pPassive, ppic);
		pic->ActivePicture = pActive;
		pic->apic = apic;
		pic->PassivePicture = pPassive;
		pic->ppic = ppic;
		pic->DisabledPicture = pDisabled;
		pic->dpic = dpic;
		pic->OnDraw = &Picture_OnDraw;
		pic->OnMouseOver = &Picture_OnMouseOver;
		pic->Enabled = true;
		pic->Active = false;
		return pic;
	}
	return nullptr;
}

//------------end of the class Picture


//------------class TextButton
int GetRLen(char* s, RLCFont* font)
{
	if (!int(s))return 0;
	int x = 0;
	int L;
	for (int i = 0; s[i] != 0;)
	{
		x += GetRLCWidthUNICODE(font->RLC, (byte*)(s + i), &L);
		i += L;
	};
	return x;
};

void DrawString(int x, int y, char* str, RLCFont* fnt, byte Align)
{
	int xx;
	if (!str)return;
	switch (Align)
	{
	case 0: ShowString(x, y, str, fnt);
		return;
	case 1: //center align
		xx = x - (GetRLen(str, fnt) >> 1);
		ShowString(xx, y, str, fnt);
		return;
	case 2: //right align
		xx = x - GetRLen(str, fnt);
		ShowString(xx, y, str, fnt);
	};
};

bool TextButton_OnDraw(SimpleDialog* SD)
{
	if (!SD)return false;
	if (!SD->Visible)return false;
	TextButton* tb = (TextButton*)SD;
	RLCFont* CFN = nullptr;
	if (!tb->Enabled && tb->DisabledFont)CFN = tb->DisabledFont;
	else if (tb->MouseOver && tb->ActiveFont)CFN = tb->ActiveFont;
	else
		if (tb->PassiveFont)CFN = tb->PassiveFont;
	if (CFN)
	{
		int rlen = GetRLen(tb->Message, CFN);
		switch (tb->Align)
		{
		case 1: tb->x = tb->xc - (rlen >> 1);
			break;
		case 2: tb->x = tb->yc - rlen;
		};
		tb->x1 = tb->x + rlen - 1;
		tb->y1 = tb->y + GetRLCHeight(CFN->RLC, 'W');
		ShowString(tb->x, tb->y, tb->Message, CFN);
		return true;
	};
	return false;
};

bool TextButton_OnMouseOver(SimpleDialog* SD)
{
	TextButton* tb = (TextButton*)SD;
	if (tb->PassiveFont != tb->ActiveFont)
		SD->NeedToDraw = true;
	return false;
};

bool TextButton_Destroy(SimpleDialog* SD)
{
	TextButton* tb = (TextButton*)SD;
	if (tb->Message)free(tb->Message);
	return true;
};

TextButton* DialogsSystem::addTextButton(
	SimpleDialog* Parent,
	int px, int py, char* str,
	RLCFont* pActive,
	RLCFont* pPassive,
	RLCFont* pDisabled,
	byte Align)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);

	if (i < MAXDLG)
	{
		TextButton* tb = new TextButton;
		DSS[i] = tb;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + px;
		tb->y = BaseY + py;
		tb->xc = BaseX + px;
		tb->yc = BaseY + py;
		int rlen = GetRLen(str, pPassive);

		switch (Align)
		{
		case 1: tb->x -= rlen >> 1;
			break;
		case 2: tb->x -= rlen;
		}

		tb->x1 = tb->x + rlen - 1;
		tb->y1 = tb->y + GetRLCHeight(pPassive->RLC, 'W');
		tb->ActiveFont = pActive;
		tb->PassiveFont = pPassive;
		tb->DisabledFont = pDisabled;
		tb->ActiveDX = 0;
		tb->ActiveDY = 0;
		tb->Align = Align;
		tb->Message = new char[strlen(str) + 1];
		strcpy(tb->Message, str);
		tb->OnDraw = &TextButton_OnDraw;
		tb->OnMouseOver = &TextButton_OnMouseOver;
		tb->Destroy = &TextButton_Destroy;
		tb->Enabled = true;
		tb->Active = false;
		return tb;
	}
	return nullptr;
}

//------------end of the class TextButton
bool GP_TextButton_OnDraw(SimpleDialog* SD)
{
	if (!SD)return false;
	if (!SD->Visible)return false;
	GP_TextButton* tb = (GP_TextButton*)SD;
	//drawing button
	TempWindow TW;
	PushWindow(&TW);
	IntersectWindows(tb->x, tb->y, tb->x1 + 1, tb->y1 + 10);
	tb->xc = tb->x + tb->FontDx;
	tb->yc = ((tb->y + tb->y1) >> 1) - (GetRLCHeight(tb->ActiveFont->RLC, 'W') >> 1) + tb->FontDy;
	if (tb->Enabled && tb->MouseOver)
	{
		for (int q = 0; q < tb->Nx; q++)GPS.ShowGP(tb->x + q * tb->OneLx, tb->y, tb->GP_File, tb->Sprite, 0);
	}
	else
	{
		if (tb->Sprite1 != -1)for (int q = 0; q < tb->Nx; q++)GPS.ShowGP(tb->x + q * tb->OneLx, tb->y, tb->GP_File,
		                                                                 tb->Sprite1, 0);
	};
	//drawing text
	RLCFont* CFN = nullptr;
	if (!tb->Enabled && tb->DisabledFont)CFN = tb->DisabledFont;
	else if (tb->MouseOver && tb->ActiveFont)CFN = tb->ActiveFont;
	else
		if (tb->PassiveFont)CFN = tb->PassiveFont;
	if (CFN)
	{
		int rlen = 0;
		if (tb->Center)rlen = GetRLCStrWidth(tb->Message, CFN) + tb->x - tb->x1;
		ShowString(tb->xc - (rlen >> 1), tb->yc, tb->Message, CFN);
		PopWindow(&TW);
		return true;
	};
	PopWindow(&TW);
	return false;
};

bool GP_TextButton_Destroy(SimpleDialog* SD)
{
	GP_TextButton* tb = (GP_TextButton*)SD;
	if (tb->Message)free(tb->Message);
	return true;
};

GP_TextButton* DialogsSystem::addGP_TextButton(
	SimpleDialog* Parent, int x, int y, char* str,
	int GP_File, int Sprite,
	RLCFont* pActive,
	RLCFont* pPassive
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		GP_TextButton* tb = new GP_TextButton;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + GPS.GetGPWidth(GP_File, Sprite) - 1;
		tb->y1 = tb->y + GPS.GetGPHeight(GP_File, Sprite) - 1;
		DSS[i] = tb;
		tb->GP_File = GP_File;
		tb->Sprite = Sprite;
		tb->Sprite1 = -1;
		tb->ActiveFont = pActive;
		tb->PassiveFont = pPassive;
		tb->DisabledFont = pPassive;
		tb->FontDy = -1;
		tb->FontDx = 0;
		tb->Message = new char[strlen(str) + 1];
		strcpy(tb->Message, str);
		tb->OnDraw = &GP_TextButton_OnDraw;
		tb->Destroy = &GP_TextButton_Destroy;
		tb->Enabled = true;
		tb->Active = false;
		tb->Center = 1;
		tb->Nx = 1;
		tb->OneLx = 0;
		//tb->FontDx=4;
		return tb;
	};
	return nullptr;
};

GP_TextButton* DialogsSystem::addGP_TextButtonLimited(
	SimpleDialog* Parent, int x, int y, char* str,
	int GP_File, int SpriteActive, int SpritePassive, int Lx,
	RLCFont* pActive, RLCFont* pPassive
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);

	if (i < MAXDLG)
	{
		GP_TextButton* tb = new GP_TextButton;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + Lx - 1;
		tb->y1 = tb->y + GPS.GetGPHeight(GP_File, SpriteActive) - 1;
		DSS[i] = tb;
		tb->GP_File = GP_File;
		tb->Sprite = SpriteActive;
		tb->Sprite1 = SpritePassive;
		tb->ActiveFont = pActive;
		tb->PassiveFont = pPassive;
		tb->DisabledFont = pPassive;
		tb->FontDy = -1;
		tb->FontDx = 0;
		tb->Message = new char[strlen(str) + 1];
		strcpy(tb->Message, str);
		tb->OnDraw = &GP_TextButton_OnDraw;
		tb->Destroy = &GP_TextButton_Destroy;
		tb->Enabled = true;
		tb->Active = false;
		tb->Center = 1;
		int LXX = GPS.GetGPWidth(GP_File, SpriteActive);
		tb->Nx = (Lx / LXX) + 1;
		tb->OneLx = LXX;
		//tb->FontDx=4;
		return tb;
	}
	return nullptr;
}

bool GP_StdTextButton_OnDraw(SimpleDialog* SD)
{
	if (!SD)return false;
	if (!SD->Visible)return false;
	GP_TextButton* tb = (GP_TextButton*)SD;
	//drawing button
	TempWindow TW;
	PushWindow(&TW);
	int spr;
	RLCFont* FNT;
	if (tb->Enabled)
	{
		if (tb->MouseOver)
		{
			spr = tb->Sprite;
			FNT = tb->ActiveFont;
		}
		else
		{
			spr = tb->Sprite + 3;
			FNT = tb->PassiveFont;
		};
	}
	else
	{
		spr = tb->Sprite + 6;
		FNT = tb->DisabledFont;
	};
	int LL = GPS.GetGPWidth(tb->GP_File, spr + 1);
	int LR = GPS.GetGPWidth(tb->GP_File, spr + 2);
	GPS.ShowGP(tb->x, tb->y, tb->GP_File, spr + 1, 0);
	GPS.ShowGP(tb->x1 - LR, tb->y, tb->GP_File, spr + 2, 0);
	IntersectWindows(tb->x + LL, tb->y, tb->x1 - LR, tb->y1 + 10);
	for (int i = 0; i < tb->Nx; i++)GPS.ShowGP(tb->x + LL + i * tb->OneLx, tb->y, tb->GP_File, spr, 0);
	PopWindow(&TW);

	int LY = GetRLCHeight(FNT->RLC, 'C');
	int LX = GetRLCStrWidth(tb->Message, FNT);
	ShowString((tb->x + tb->x1 - LX) / 2, (tb->y + tb->y1 - LY) / 2 - 1, tb->Message, FNT);
	return false;
}

GP_TextButton* DialogsSystem::addStdGP_TextButton(int x, int y, int Lx, char* str,
                                                  int GP_File, int Sprite,
                                                  RLCFont* pActive, RLCFont* pPassive
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);

	if (i < MAXDLG)
	{
		GP_TextButton* tb = new GP_TextButton;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = nullptr;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + Lx - 1;
		tb->y1 = tb->y + GPS.GetGPHeight(GP_File, Sprite);
		DSS[i] = tb;
		tb->GP_File = GP_File;
		tb->Sprite = Sprite;
		tb->Sprite1 = 0;
		tb->ActiveFont = pActive;
		tb->PassiveFont = pPassive;
		tb->DisabledFont = pPassive;
		tb->FontDy = 0;
		tb->FontDx = 0;
		tb->Message = new char[strlen(str) + 1];
		strcpy(tb->Message, str);
		tb->OnDraw = &GP_StdTextButton_OnDraw;
		tb->Destroy = &GP_TextButton_Destroy;
		tb->Enabled = true;
		tb->Active = false;
		tb->Center = 1;
		int LXX = GPS.GetGPWidth(GP_File, Sprite);
		tb->Nx = (Lx / LXX) + 1;
		tb->OneLx = LXX;
		//tb->FontDx=4;
		return tb;
	}
	return nullptr;
}

bool UniversalButton_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)
		return false;

	UniversalButton* UB = (UniversalButton*)SD;
	int StartIDX = 0;
	RLCFont* FONT;
	if (UB->ActiveFont == UB->PassiveFont)
	{
		if (UB->State)FONT = UB->SelectedFont;
		else FONT = UB->PassiveFont;
	}
	else
	{
		if (UB->MouseOver)FONT = UB->ActiveFont;
		else if (UB->State)FONT = UB->SelectedFont;
		else FONT = UB->PassiveFont;
	};
	if (UB->MouseOver)
	{
		StartIDX = 3 * 5;
	};
	StartIDX = StartIDX + UB->State * 5;
	TempWindow TW;
	PushWindow(&TW);
	int x0 = UB->x;
	int x1 = UB->x1;
	if (!UB->Tiling)
	{
		int sp = UB->SpritesSet[StartIDX];
		if (sp != -1)x0 += GPS.GetGPWidth(UB->GP_File, sp);
		sp = UB->SpritesSet[StartIDX + 4];
		if (sp != -1)x1 -= GPS.GetGPWidth(UB->GP_File, sp);
	};
	IntersectWindows(x0, 0, x1, RealLy - 1);
	int pp0 = x0 * 7 + UB->y * 17;
	int NN = 0;
	while (x0 < x1 && NN < 1000)
	{
		int sp = UB->SpritesSet[StartIDX + 1 + randoma[pp0 & 8191] % 3];
		NN++;
		if (sp != -1)
		{
			GPS.ShowGP(x0, UB->y, UB->GP_File, sp, MyNation);
			x0 += GPS.GetGPWidth(UB->GP_File, sp);
		};
		pp0++;
	};
	PopWindow(&TW);
	IntersectWindows(0, 0, (UB->x + UB->x1) / 2, RealLy - 1);
	int sp = UB->SpritesSet[StartIDX];
	if (sp != -1)GPS.ShowGP(UB->x, UB->y, UB->GP_File, sp, MyNation);
	PopWindow(&TW);
	IntersectWindows((UB->x + UB->x1) / 2 + 1, 0, RealLx - 1, RealLy - 1);
	sp = UB->SpritesSet[StartIDX + 4];
	if (sp != -1)GPS.ShowGP(UB->x1 - GPS.GetGPWidth(UB->GP_File, sp), UB->y, UB->GP_File, sp, MyNation);
	PopWindow(&TW);
	//text
	if (FONT)
	{
		int x;
		int y = (UB->y + UB->y1 - GetRLCHeight(FONT->RLC, 'W')) / 2 - 1;
		if (UB->Center)
		{
			x = (UB->x + UB->x1 - GetRLCStrWidth(UB->Message, FONT)) / 2;
		}
		else
		{
			x = x0;
		};
		x += UB->FontDx;
		y += UB->FontDy;
		ShowString(x, y, UB->Message, FONT);
	};
	return true;
};

bool UniversalButton_Destroy(SimpleDialog* SD)
{
	UniversalButton* UB = (UniversalButton*)SD;
	free(UB->Message);
	return true;
};

bool UniversalButton_OnKlick(SimpleDialog* SD)
{
	UniversalButton* UB = (UniversalButton*)SD;
	DialogsSystem* DSS = UB->ParentDS;
	if (!DSS)return false;
	if (UB->Group != -1)
	{
		if (!UB->State)UB->State = 1;
		else if (UB->State == 1)UB->State = 2;
		else UB->State = 1;
		for (int i = 0; i < MAXDLG; i++)
		{
			if (DSS->DSS[i] && DSS->DSS[i]->OnDraw == &UniversalButton_OnDraw)
			{
				UniversalButton* UB1 = (UniversalButton*)DSS->DSS[i];
				if (UB1 != UB && UB1->Group == UB->Group)
				{
					UB1->State = 0;
				};
			};
		};
	}
	else
	{
		if (!UB->State)UB->State = 1;
		else UB->State = 0;
	};
	return true;
};

UniversalButton* DialogsSystem::addUniversalButton(
	int x, int y, int Lx, char* str, int GP_File,
	int* SprSet, int Group, int NowSelected, bool tiling,
	RLCFont* pActive, RLCFont* pPassive, RLCFont* Selected
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		UniversalButton* tb = new UniversalButton;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = nullptr;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + Lx - 1;
		tb->y1 = tb->y + GPS.GetGPHeight(GP_File, SprSet[0]);
		DSS[i] = tb;
		tb->GP_File = GP_File;
		memcpy(tb->SpritesSet, SprSet, 30 * 4);

		tb->Group = Group;
		tb->State = NowSelected;

		tb->ActiveFont = pActive;
		tb->PassiveFont = pPassive;
		tb->DisabledFont = pPassive;
		tb->SelectedFont = Selected;

		tb->FontDy = 0;
		tb->FontDx = 0;
		tb->Message = new char[strlen(str) + 1];
		strcpy(tb->Message, str);
		tb->OnDraw = &UniversalButton_OnDraw;
		tb->Destroy = &UniversalButton_Destroy;
		tb->OnClick = &UniversalButton_OnKlick;
		tb->Enabled = true;
		tb->Active = false;
		tb->Center = 1;
		tb->ParentDS = this;
		//tb->FontDx=4;
		return tb;
	};
	return nullptr;
};

UniversalButton* DialogsSystem::addTextureStrip(int x, int y, int Lx, int GP_File,
                                                int L, int C1, int C2, int C3, int R, bool Tiling)
{
	int PATTERN[30] = {
		L, C1, C2, C3, R, L, C1, C2, C3, R, L, C1, C2, C3, R, L, C1, C2, C3, R, L, C1, C2, C3, R, L, C1, C2, C3, R
	};
	return addUniversalButton(x, y, Lx, "", GP_File, PATTERN, -1, 0, Tiling, nullptr, nullptr, nullptr);
};
//-------class : button with text-----------
bool BpxTextButton_OnDraw(SimpleDialog* SD)
{
	if (!SD)return false;
	if (!SD->Visible)return false;
	BpxTextButton* tb = (BpxTextButton*)SD;
	//drawing button
	SQPicture* SP;
	tb->xc = (tb->x + tb->x1) >> 1;
	tb->yc = (tb->y + tb->y1) >> 1;
	if (tb->Enabled)
	{
		if (tb->MouseOver && realLpressed)SP = tb->ActivePicture;
		else SP = tb->PassivePicture;
	}
	else SP = tb->DisabledPicture;
	SP->Draw(tb->x, tb->y);
	//drawing text
	RLCFont* CFN = nullptr;
	if (!tb->Enabled && tb->DisabledFont)CFN = tb->DisabledFont;
	else if (tb->MouseOver && tb->ActiveFont)CFN = tb->ActiveFont;
	else
		if (tb->PassiveFont)CFN = tb->PassiveFont;
	if (CFN)
	{
		int rlen = GetRLen(tb->Message, CFN);
		ShowString(tb->xc - (rlen >> 1), tb->yc - (GetRLCHeight(CFN->RLC, 'W') >> 1), tb->Message, CFN);
		return true;
	};
	return false;
};

bool BpxTextButton_OnMouseOver(SimpleDialog* SD)
{
	if (SD->MouseOver)SD->NeedToDraw = true;
	return false;
};

BpxTextButton* DialogsSystem::addBpxTextButton(SimpleDialog* Parent,
                                               int px, int py, char* str,
                                               RLCFont* pActiveFont,
                                               RLCFont* pPassiveFont,
                                               RLCFont* pDisabledFont,
                                               SQPicture* pActivePicture,
                                               SQPicture* pPassivePicture,
                                               SQPicture* pDisabledPicture)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		BpxTextButton* tb = new BpxTextButton;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + px;
		tb->y = BaseY + py;
		tb->x1 = tb->x + pPassivePicture->PicPtr[0] - 1;
		tb->y1 = tb->y + pPassivePicture->PicPtr[1] - 1;
		tb->ActivePicture = pActivePicture;
		tb->PassivePicture = pPassivePicture;
		tb->DisabledPicture = pDisabledPicture;
		DSS[i] = tb;
		tb->ActiveFont = pActiveFont;
		tb->PassiveFont = pPassiveFont;
		tb->DisabledFont = pDisabledFont;
		tb->ActiveDX = 0;
		tb->ActiveDY = 0;
		tb->Message = new char[strlen(str) + 1];
		strcpy(tb->Message, str);
		tb->OnDraw = &BpxTextButton_OnDraw;
		tb->Destroy = &TextButton_Destroy;
		tb->Enabled = true;
		tb->Active = false;
		tb->OnMouseOver = &BpxTextButton_OnMouseOver;
		return tb;
	};
	return nullptr;
};
//--------end of button with text----------//
bool Video_OnDraw(SimpleDialog* SD)
{
	VideoButton* tb = (VideoButton*)SD;
	if (!SD->Visible)return 0;
	int dt = GetTickCount() - tb->LastTime;
	if (tb->Stage == 2)
	{
		GPS.ShowGP(tb->x, tb->y, tb->GP_off, tb->CurSprite, 0);
		if (tb->CurSprite < tb->N_off - 1)
		{
			if (dt > 40)
			{
				tb->CurSprite++;
				tb->LastTime = GetTickCount();
			};
		}
		else
		{
			tb->Stage = 0;
			tb->CurSprite = 0;
		};
	};
	if (tb->Stage == 1)
	{
		GPS.ShowGP(tb->x, tb->y, tb->GP_on, tb->CurSprite, 0);
		if (tb->CurSprite < tb->N_on - 1)
		{
			if (dt > 40)
			{
				tb->CurSprite++;
				tb->LastTime = GetTickCount();
			};
		}
		else
		{
			if (!tb->MouseOver)
			{
				tb->Stage = 2;
				tb->CurSprite = 0;
			};
		};
	};
	if (tb->Stage == 0)
	{
		GPS.ShowGP(tb->x, tb->y, tb->GP_on, 0, 0);
		if (tb->MouseOver)
		{
			tb->Stage = 1;
			tb->CurSprite = 0;
		};
	};
	return false;
}

VideoButton* DialogsSystem::addVideoButton(SimpleDialog* Parent, int x, int y, int GP1, int GP2)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		VideoButton* tb = new VideoButton;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + GPS.GetGPWidth(GP1, 0) - 1;
		tb->y1 = tb->y + GPS.GetGPHeight(GP1, 0) - 1;
		tb->GP_on = GP1;
		tb->GP_off = GP2;
		tb->N_on = GPS.GPNFrames[GP1];
		tb->N_off = GPS.GPNFrames[GP2];
		tb->Stage = 0;
		tb->CurSprite = 0;
		DSS[i] = tb;
		tb->OnDraw = &Video_OnDraw;
		tb->Destroy = nullptr;
		tb->Enabled = true;
		tb->Active = false;
		tb->LastTime = GetTickCount();
		return tb;
	};
	return nullptr;
}

//---------------GP_Button-----------------//
bool GP_Button_OnDraw(SimpleDialog* SD)
{
	GP_Button* tb = (GP_Button*)SD;

	if (!tb->Visible)
	{
		return false;
	}

	/*
	//TESTING: menu colors
	for (int i = 0, c = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++, c++)
		{
			//if (i * 16 + j > 160) continue;
			CBar( i * 60, j * 45, 60, 45, c );
		}
	}
	*/

	int checkers_color = 127; //light brown

	if (tb->MouseOver)
	{
		GPS.ShowGP(tb->x, tb->y, tb->GP_File, tb->ActiveFrame, 0);
		checkers_color = 232; //white
	}
	else
	{
		GPS.ShowGP(tb->x, tb->y, tb->GP_File, tb->PassiveFrame, 0);
	}

	if ((1005 == tb->GP_File || 1004 == tb->GP_File) && 15 == tb->ActiveFrame)
	{
		//Showing "Internet Game" button with checkers. WAAAAGH!
		int x = tb->x + 100;
		int y = tb->y;
		CBar(x, y, 104, 24, 134); //Dark brown background rectangle
		for (int i = 0; i < 78; i++) //26 columns with 3 squares in each
		{
			//Checkers, baby!
			int div3 = i / 3;
			int mod3 = i % 3;
			int xs = x + 4 * div3;
			int ys = y + 8 * mod3;
			if (0 == div3 % 2)
			{
				//Alternating vertical offset for every 2nd column
				ys += 4;
			}
			CBar(xs, ys, 4, 4, checkers_color);
		}
	}

	return false;
}

GP_Button* DialogsSystem::addGP_Button(
	SimpleDialog* Parent, int x, int y,
	int GP_File, int pActive, int pPassive)
{
	int i;

	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++)
	{
	}

	if (i < MAXDLG)
	{
		GP_Button* tb = new GP_Button;
		tb->MouseOverActive = true;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + GPS.GetGPWidth(GP_File, pPassive) - 1;
		tb->y1 = tb->y + GPS.GetGPHeight(GP_File, pPassive) - 1;
		tb->GP_File = GP_File;
		tb->ActiveFrame = pActive;
		tb->PassiveFrame = pPassive;

		DSS[i] = tb;

		tb->OnDraw = &GP_Button_OnDraw;
		tb->Destroy = nullptr;
		tb->Enabled = true;
		tb->Active = false;

		return tb;
	}

	return nullptr;
}

//------class : Vertical scroll bar--------//
bool MouseIns(int x, int y, int x1, int y1)
{
	if (mouseX >= x && mouseY >= y && mouseX <= x1 && mouseY <= y1)return true;
	else return false;
};

bool MouseInsL(int x, int y, int lx, int ly)
{
	return MouseIns(x, y, x + lx - 1, y + ly - 1);
};

bool VScrollBar_Draw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	VScrollBar* tb = (VScrollBar*)SD;
	if (tb->GetMaxPos() > 0)
	{
		tb->sbar0->DrawTransparent(tb->x, tb->sby + 2);
		if (MouseInsL(tb->x, tb->y, tb->sblx, tb->btnly) && realLpressed)
			tb->btn_up1p->DrawTransparent(tb->x, tb->y);
		else tb->btn_up1->DrawTransparent(tb->x, tb->y);
		if (MouseInsL(tb->x, tb->sby + tb->sbly, tb->sblx, tb->btnly) && realLpressed)
			tb->btn_dn1p->DrawTransparent(tb->x, tb->sby + tb->sbly);
		else tb->btn_dn1->DrawTransparent(tb->x, tb->sby + tb->sbly);
		tb->marker->DrawTransparent(
			tb->x + 1, tb->y + tb->btnly + div( int( tb->sbly - tb->mkly + 2 )*tb->SPos, tb->SMaxPos ).quot);
	}
	else
	{
		tb->sbar1->DrawTransparent(tb->x, tb->sby + 2);
		tb->btn_up0->DrawTransparent(tb->x, tb->y);
		tb->btn_dn0->DrawTransparent(tb->x, tb->sby + tb->sbly);
	};
	return true;
};

bool VScrollBar_MouseOver(SimpleDialog* SD)
{
	VScrollBar* tb = (VScrollBar*)SD;
	int dt = GetTickCount() - tb->LastTime;
	if (tb->SMaxPos > 0)
	{
		if (!realLpressed)tb->Zaxvat = false;
		int my = tb->y + tb->btnly + div( int( tb->sbly - tb->mkly + 2 )*tb->SPos, tb->SMaxPos ).quot;
		if (!tb->Zaxvat)
		{
			if (MouseInsL(tb->x, tb->y, tb->sblx, tb->btnly) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SPos > 0)tb->SPos--;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseInsL(tb->x, tb->sby + tb->sbly, tb->sblx, tb->btnly) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SPos < tb->SMaxPos)tb->SPos++;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseIns(tb->x, tb->y + tb->btnly, tb->x + tb->sblx - 1, my) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SPos > 6)tb->SPos -= 6;
				else tb->SPos = 0;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseIns(tb->x, my + tb->mkly, tb->x + tb->sblx - 1, tb->y + tb->sbly + tb->btnly - 1) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SMaxPos - tb->SPos > 6)tb->SPos += 6;
				else tb->SPos = tb->SMaxPos;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseInsL(tb->x, my, tb->sblx, tb->mkly) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				tb->Zaxvat = true;
			};
		}
		else
		{
			int txx = tb->SPos;
			tb->SPos = div( ( mouseY - tb->sby - ( tb->mkly >> 1 ) )*tb->SMaxPos, tb->sbly - tb->mkly ).quot;
			if (tb->SPos < 0)tb->SPos = 0;
			if (tb->SPos > tb->SMaxPos)tb->SPos = tb->SMaxPos;
			if (tb->SPos != txx)tb->NeedToDraw = true;
		};
	};
	return true;
};

VScrollBar* DialogsSystem::addVScrollBar(SimpleDialog* Parent,
                                         int x, int y, int MaxPos, int Pos,
                                         SQPicture* btn_up0,
                                         SQPicture* btn_up1,
                                         SQPicture* btn_up1p,
                                         SQPicture* btn_dn0,
                                         SQPicture* btn_dn1,
                                         SQPicture* btn_dn1p,
                                         SQPicture* sbar0,
                                         SQPicture* sbar1,
                                         SQPicture* marker)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		VScrollBar* tb = new VScrollBar;
		DSS[i] = tb;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->sblx = sbar0->PicPtr[0];
		tb->sbly = sbar0->PicPtr[1];
		tb->mkly = byte(marker->PicPtr[1]);
		tb->btnly = byte(btn_dn1->PicPtr[1]);
		tb->y1 = tb->y + tb->sbly + (tb->btnly << 1) - 1;
		tb->x1 = tb->x + tb->sblx - 1;
		tb->sby = tb->y + tb->btnly;
		tb->SMaxPos = MaxPos;
		tb->SPos = Pos;
		tb->btn_up0 = btn_up0;
		tb->btn_up1 = btn_up1;
		tb->btn_up1p = btn_up1p;
		tb->btn_dn0 = btn_dn0;
		tb->btn_dn1 = btn_dn1;
		tb->btn_dn1p = btn_dn1p;
		tb->sbar0 = sbar0;
		tb->sbar1 = sbar1;
		tb->marker = marker;
		tb->Zaxvat = false;
		tb->OnDraw = &VScrollBar_Draw;
		tb->OnMouseOver = &VScrollBar_MouseOver;
		tb->Enabled = true;
		tb->Active = false;
		tb->GP_File = -1;
		tb->StartGP_Spr = -1;
		tb->OnesDy = 1;
		return tb;
	};
	return nullptr;
};
//-------end of vertical Scroll bar--------//
//------class : Horizontal scroll bar--------//
bool HScrollBar_Draw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	VScrollBar* tb = (VScrollBar*)SD;
	if (tb->SMaxPos > 0)
	{
		tb->sbar0->DrawTransparent(tb->sby + 2, tb->y);
		//left click
		if (MouseInsL(tb->x, tb->y, tb->btnly, tb->sblx) && realLpressed)
			tb->btn_up1p->DrawTransparent(tb->x, tb->y);
		else tb->btn_up1->DrawTransparent(tb->x, tb->y);
		//right click
		if (MouseInsL(tb->sby + tb->sbly, tb->y, tb->btnly, tb->sblx) && realLpressed)
			tb->btn_dn1p->DrawTransparent(tb->sby + tb->sbly, tb->y);
		else tb->btn_dn1->DrawTransparent(tb->sby + tb->sbly, tb->y);
		tb->marker->DrawTransparent(tb->x + tb->btnly + div( int( tb->sbly - tb->mkly + 2 )*tb->SPos, tb->SMaxPos ).quot,
		                            tb->y + 1);
	}
	else
	{
		tb->sbar1->DrawTransparent(tb->sby + 2, tb->y);
		tb->btn_up0->DrawTransparent(tb->x, tb->y);
		tb->btn_dn0->DrawTransparent(tb->sby + tb->sbly, tb->x);
	};
	return true;
};

bool HScrollBar_MouseOver(SimpleDialog* SD)
{
	VScrollBar* tb = (VScrollBar*)SD;
	int dt = GetTickCount() - tb->LastTime;
	if (tb->SMaxPos > 0)
	{
		if (!realLpressed)tb->Zaxvat = false;
		int my = tb->x + tb->btnly + div( int( tb->sbly - tb->mkly + 2 )*tb->SPos, tb->SMaxPos ).quot;
		if (!tb->Zaxvat)
		{
			if (MouseInsL(tb->x, tb->y, tb->btnly, tb->sblx) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SPos > 0)tb->SPos--;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseInsL(tb->sby + tb->sbly, tb->y, tb->btnly, tb->sblx) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SPos < tb->SMaxPos)tb->SPos++;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseIns(tb->x + tb->btnly, tb->y, my, tb->y + tb->sblx - 1) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SPos > 6)tb->SPos -= 6;
				else tb->SPos = 0;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseIns(my + tb->mkly, tb->y, tb->x + tb->sbly + tb->btnly - 1, tb->y + tb->sblx - 1) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				int txx = tb->SPos;
				if (tb->SMaxPos - tb->SPos > 6)tb->SPos += 6;
				else tb->SPos = tb->SMaxPos;
				if (tb->SPos != txx)tb->NeedToDraw = true;
			};
			if (MouseInsL(my, tb->y, tb->mkly, tb->sblx) && Lpressed && dt > 150)
			{
				tb->LastTime = GetTickCount();
				//Lpressed=false;
				tb->Zaxvat = true;
			};
		}
		else
		{
			int txx = tb->SPos;
			tb->SPos = div( ( mouseX - tb->sby - ( tb->mkly >> 1 ) )*tb->SMaxPos, tb->sbly - tb->mkly ).quot;
			if (tb->SPos < 0)tb->SPos = 0;
			if (tb->SPos > tb->SMaxPos)tb->SPos = tb->SMaxPos;
			if (tb->SPos != txx)tb->NeedToDraw = true;
		};
	};
	return true;
};

VScrollBar* DialogsSystem::addHScrollBar(SimpleDialog* Parent,
                                         int x, int y, int MaxPos, int Pos,
                                         SQPicture* btn_up0,
                                         SQPicture* btn_up1,
                                         SQPicture* btn_up1p,
                                         SQPicture* btn_dn0,
                                         SQPicture* btn_dn1,
                                         SQPicture* btn_dn1p,
                                         SQPicture* sbar0,
                                         SQPicture* sbar1,
                                         SQPicture* marker)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		VScrollBar* tb = new VScrollBar;
		DSS[i] = tb;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->sblx = sbar0->PicPtr[1];
		tb->sbly = sbar0->PicPtr[0];
		tb->mkly = byte(marker->PicPtr[0]);
		tb->btnly = btn_dn1->PicPtr[0] - 2;
		tb->x1 = tb->x + tb->sbly + (tb->btnly << 1) - 1;
		tb->y1 = tb->y + tb->sblx - 1;
		tb->sby = tb->x + tb->btnly;
		tb->SMaxPos = MaxPos;
		tb->SPos = Pos;
		tb->btn_up0 = btn_up0;
		tb->btn_up1 = btn_up1;
		tb->btn_up1p = btn_up1p;
		tb->btn_dn0 = btn_dn0;
		tb->btn_dn1 = btn_dn1;
		tb->btn_dn1p = btn_dn1p;
		tb->sbar0 = sbar0;
		tb->sbar1 = sbar1;
		tb->marker = marker;
		tb->Zaxvat = false;
		tb->OnDraw = &HScrollBar_Draw;
		tb->OnMouseOver = &HScrollBar_MouseOver;
		tb->Enabled = true;
		tb->Active = false;
		tb->GP_File = -1;
		tb->StartGP_Spr = -1;
		tb->OnesDy = 1;
		return tb;
	};
	return nullptr;
};
//-------end of horizontal Scroll bar--------//
//--------------GP_Scrollbar-----------------//
bool GP_ScrollBar_OnDraw(SimpleDialog* SD)
{
	VScrollBar* SB = (VScrollBar*)SD;
	if (!(SB->Enabled && SB->Visible && SB->SMaxPos > 0))
	{
		SB->Zaxvat = false;
		return false;
	};
	if (!SB->SMaxPos)return false;
	if (SB->LineIndex >= 0)GPS.ShowGP(SB->x, SB->y, SB->GP_File, SB->LineIndex, 0);
	//setting new value for scroller
	int scx, scy;
	if (SB->LineLx > SB->LineLy)
	{
		scx = SB->x + (SB->SPos * (SB->LineLx - SB->ScrLx)) / SB->SMaxPos + SB->ScrDx;
		scy = SB->y + SB->ScrDy;
	}
	else
	{
		scx = SB->x + SB->ScrDx;
		scy = SB->y + SB->ScrDy + (SB->SPos * (SB->LineLy - SB->ScrLy)) / SB->SMaxPos;
	};
	if (!realLpressed)SB->Zaxvat = false;
	if (!SB->Zaxvat)
	{
		if (Lpressed && mouseX > scx && mouseY > scy && mouseX < scx + SB->ScrLx - 1 && mouseY < scy + SB->ScrLy - 1)
		{
			SB->Zaxvat = true;
			SB->sbx = mouseX;
			SB->sby = mouseY;
			SB->sblx = SB->SPos;
			Lpressed = false;
		};
	};
	if (SB->Zaxvat)
	{
		int dx = mouseX - SB->sbx;
		int dy = mouseY - SB->sby;
		if (SB->LineLx > SB->LineLy)
		{
			SB->SPos = SB->sblx + dx * (SB->SMaxPos + 1) / (SB->LineLx - SB->ScrLx);
		}
		else
		{
			SB->SPos = SB->sblx + dy * (SB->SMaxPos + 1) / (SB->LineLy - SB->ScrLy);
		};
		if (SB->SPos < 0)SB->SPos = 0;
		if (SB->SPos > SB->SMaxPos)SB->SPos = SB->SMaxPos;
		if (SB->LineLx > SB->LineLy)
		{
			scx = SB->x + (SB->SPos * (SB->LineLx - SB->ScrLx)) / SB->SMaxPos + SB->ScrDx;
			scy = SB->y + SB->ScrDy;
		}
		else
		{
			scx = SB->x + SB->ScrDx;
			scy = SB->y + SB->ScrDy + (SB->SPos * (SB->LineLy - SB->ScrLy)) / SB->SMaxPos;
		};
	};
	//-----------------------------
	if (SB->LineLx > SB->LineLy)
	{
		GPS.ShowGP(scx, scy, SB->GP_File, SB->ScrollIndex, 0);
	}
	else
	{
		GPS.ShowGP(scx, scy, SB->GP_File, SB->ScrollIndex, 0);
	};
	return false;
};

VScrollBar* DialogsSystem::addGP_ScrollBar(SimpleDialog* Parent, int x, int y, int MaxPos, int Pos, int GP_File,
                                           int ScrIndex, int LineIndex, int ScrDx, int ScrDy)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		VScrollBar* tb = new VScrollBar;
		DSS[i] = tb;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + GPS.GetGPWidth(GP_File, LineIndex) - 1;
		tb->y1 = tb->y1 + GPS.GetGPHeight(GP_File, LineIndex) - 1;
		tb->LineLx = GPS.GetGPWidth(GP_File, LineIndex);
		tb->LineLy = GPS.GetGPHeight(GP_File, LineIndex);
		tb->ScrLx = GPS.GetGPWidth(GP_File, ScrIndex);
		tb->ScrLy = GPS.GetGPHeight(GP_File, ScrIndex);
		tb->ScrDx = ScrDx;
		tb->ScrDy = ScrDy;
		if (tb->LineLx > tb->LineLy)tb->ScrDy -= tb->ScrLy >> 1;
		else tb->ScrDx -= tb->ScrLx >> 1;
		tb->GP_File = GP_File;
		tb->ScrollIndex = ScrIndex;
		tb->LineIndex = LineIndex;
		tb->SPos = Pos;
		tb->SMaxPos = MaxPos;
		tb->Active = false;
		tb->Zaxvat = false;
		tb->Enabled = true;
		tb->OnDraw = &GP_ScrollBar_OnDraw;
		tb->StartGP_Spr = -1;
		tb->OnesDy = 1;
		return tb;
	};
	return nullptr;
};

bool NewGP_VScrollBar_OnDraw(SimpleDialog* SD)
{
	VScrollBar* SB = (VScrollBar*)SD;
	if (!(SB->Enabled && SB->Visible && SB->SMaxPos > 0))
	{
		//SB->Zaxvat=false;
		return false;
	};
	if (!SB->SMaxPos)return false;
	int upid = 0;
	int dnid = 2;
	if (mouseX > SB->x && mouseX < SB->x1 && mouseY > SB->y && mouseY < SB->y + SB->btnly)upid++;
	if (mouseX > SB->x && mouseX < SB->x1 && mouseY < SB->y1 && mouseY > SB->y1 - SB->btnly)dnid++;
	DrawVScroller(SB->x, SB->y, SB->y1 - SB->y + 1, SB->GP_File, SB->StartGP_Spr + upid, SB->StartGP_Spr + dnid,
	              SB->StartGP_Spr + 5, 3);

	//if(SB->LineIndex>=0)GPS.ShowGP(SB->x,SB->y,SB->GP_File,SB->LineIndex,0);
	//setting new value for scroller
	int scx, scy;
	scx = SB->x;
	scy = SB->y + SB->btnly + (SB->SPos * (SB->LineLy - SB->ScrLy)) / SB->SMaxPos;
	if (!realLpressed)SB->Zaxvat = false;
	if (!SB->Zaxvat)
	{
		if (Lpressed && mouseX > scx && mouseY > scy && mouseX < scx + SB->ScrLx - 1 && mouseY < scy + SB->ScrLy - 1)
		{
			SB->Zaxvat = true;
			SB->sbx = mouseX;
			SB->sby = mouseY;
			SB->sblx = SB->SPos;
			Lpressed = false;
		};
	};
	if (SB->Zaxvat)
	{
		int dx = mouseX - SB->sbx;
		int dy = mouseY - SB->sby;
		SB->SPos = SB->sblx + dy * (SB->SMaxPos + 1) / (SB->LineLy - SB->ScrLy);
		if (SB->SPos < 0)SB->SPos = 0;
		if (SB->SPos > SB->SMaxPos)SB->SPos = SB->SMaxPos;
		scx = SB->x;
		scy = SB->y + SB->btnly + (SB->SPos * (SB->LineLy - SB->ScrLy)) / SB->SMaxPos;
	}
	else
	{
		if (GetTickCount() - SB->LastTime > 100)
		{
			if (upid == 1 && Lpressed)
			{
				SB->SPos -= SB->OnesDy;
				if (SB->SPos < 0)SB->SPos = 0;
				SB->LastTime = GetTickCount();
			};
			if (dnid == 3 && Lpressed)
			{
				SB->SPos += SB->OnesDy;
				if (SB->SPos > SB->SMaxPos)SB->SPos = SB->SMaxPos;
				SB->LastTime = GetTickCount();
			};
			if (Lpressed && SB->MouseOver && upid == 0 && dnid == 2)
			{
				if (mouseY < scy)
				{
					SB->SPos -= SB->ScrDy;
					if (SB->SPos < 0)SB->SPos = 0;
					Lpressed = false;
				}
				else
				{
					SB->SPos += SB->ScrDy;
					if (SB->SPos > SB->SMaxPos)SB->SPos = SB->SMaxPos;
					Lpressed = false;
				};
			};
		};
	};
	//-----------------------------
	GPS.ShowGP(scx, scy, SB->GP_File, SB->StartGP_Spr + 4, 0);
	return false;
};

VScrollBar* DialogsSystem::addNewGP_VScrollBar(SimpleDialog* Parent, int x, int y, int Ly,
                                               int MaxPos, int Pos, int GP_File, int Sprite)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		VScrollBar* tb = new VScrollBar;
		DSS[i] = tb;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + GPS.GetGPWidth(GP_File, Sprite) - 1;
		tb->y1 = tb->y + Ly - 1;
		tb->LineLx = GPS.GetGPWidth(GP_File, Sprite);
		tb->LineLy = Ly - 2 * GPS.GetGPHeight(GP_File, Sprite);
		tb->ScrLx = GPS.GetGPWidth(GP_File, Sprite);
		tb->ScrLy = GPS.GetGPHeight(GP_File, Sprite + 4);
		tb->ScrDx = 0;
		tb->ScrDy = 0;
		//if(tb->LineLx>tb->LineLy)tb->ScrDy-=tb->ScrLy>>1;
		//else tb->ScrDx-=tb->ScrLx>>1;
		tb->GP_File = GP_File;
		tb->StartGP_Spr = Sprite;
		tb->LineIndex = -1;
		tb->SPos = Pos;
		tb->SMaxPos = MaxPos;
		tb->Active = false;
		tb->Zaxvat = false;
		tb->Enabled = true;
		tb->btnly = GPS.GetGPHeight(GP_File, Sprite);
		tb->OnDraw = &NewGP_VScrollBar_OnDraw;
		tb->LastTime = GetTickCount();
		tb->OnesDy = 1;
		return tb;
	};
	return nullptr;
};

VScrollBar* DialogsSystem::addGP_ScrollBarL(SimpleDialog* Parent, int x, int y,
                                            int MaxPos, int Pos, int GP_File,
                                            int ScrIndex, int LineLx, int LineLy, int ScrDx, int ScrDy)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		VScrollBar* tb = new VScrollBar;
		DSS[i] = tb;
		tb->NeedToDraw = true;
		tb->Parent = Parent;
		tb->x = BaseX + x;
		tb->y = BaseY + y;
		tb->x1 = tb->x + LineLx - 1;
		tb->y1 = tb->y1 + LineLy - 1;
		tb->LineLx = LineLx;
		tb->LineLy = LineLy;
		tb->ScrLx = GPS.GetGPWidth(GP_File, ScrIndex);
		tb->ScrLy = GPS.GetGPHeight(GP_File, ScrIndex);
		tb->ScrDx = ScrDx;
		tb->ScrDy = ScrDy;
		if (tb->LineLx > tb->LineLy)tb->ScrDy -= tb->ScrLy >> 1;
		else tb->ScrDx -= tb->ScrLx >> 1;
		tb->GP_File = GP_File;
		tb->ScrollIndex = ScrIndex;
		tb->LineIndex = -1;
		tb->SPos = Pos;
		tb->SMaxPos = MaxPos;
		tb->Active = false;
		tb->Zaxvat = false;
		tb->Enabled = true;
		tb->OnDraw = &GP_ScrollBar_OnDraw;
		tb->StartGP_Spr = -1;
		tb->OnesDy = 1;
		return tb;
	};
	return nullptr;
};
//-----------end of GP_ScrollBar-------------//

//-------------class : ComplexBox------------//
bool ComplexBox_OnMouseOver(SimpleDialog* SD)
{
	//if(!Lpressed)return false;
	ComplexBox* LB = (ComplexBox*)SD;
	SD->NeedToDraw = true;
	int ni = (mouseY - LB->y) / LB->OneLy;
	int curitem;
	if (ni < LB->NOnScr && LB->FirstVisItem + ni < LB->N)
	{
		curitem = LB->FirstVisItem + ni;
		if (curitem >= 0)
		{
			LB->M_OvrItem = curitem;
			if (Lpressed)LB->CurrentItem = curitem;
		};
	}
	else LB->M_OvrItem = -1;
	return true;
};

bool ComplexBox_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	ComplexBox* LB = (ComplexBox*)SD;
	if (!LB->MouseOver)LB->M_OvrItem = -1;
	if (LB->VS)
	{
		int mxp = (LB->N - LB->NOnScr) * 10;
		if (mxp < 0)
		{
			LB->VS->SMaxPos = 1;
			LB->VS->SPos = 0;
			LB->VS->Visible = 0;
			LB->VS->Enabled = 0;
		}
		else
		{
			LB->VS->SMaxPos = mxp;
			if (LB->VS->SPos > LB->VS->SMaxPos)LB->VS->SPos = LB->VS->SMaxPos;
			LB->VS->Visible = 1;
			LB->VS->Enabled = 1;
		};
		LB->FirstVisItem = LB->VS->SPos / 10;
	};
	for (int i = 0; i < LB->NOnScr; i++)
	{
		int cp = i + LB->FirstVisItem;
		if (cp < LB->N)
		{
			int xx = LB->x;
			int yy = LB->y + LB->OneLy * i;
			byte state = 0;
			if (LB->M_OvrItem == cp)
			{
				if (LB->GP_Index != -1)GPS.ShowGP(xx, yy, LB->GP_Index, LB->StSprite + 3 + (cp % 3), 0);
				state = 2;
			}
			else if (LB->CurrentItem == cp)
			{
				if (LB->GP_Index != -1)GPS.ShowGP(xx, yy, LB->GP_Index, LB->StSprite + (cp % 3), 0);
				state = 1;
			};
			LB->DRAWPROC(xx, yy, LB->x1 - LB->x, LB->OneLy, cp, state, LB->param);
		};
	};
	return true;
}

ComplexBox* DialogsSystem::addComplexBox(int x, int y, int Ny, int OneLy,
                                         procDrawBoxElement* PDRAW, int GP_File, int spr)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ComplexBox* LB = new ComplexBox;
		DSS[i] = LB;
		LB->NeedToDraw = true;
		LB->Parent = nullptr;
		LB->x = x + BaseX;
		LB->y = y + BaseY;
		LB->OneLy = OneLy;
		LB->N = 0;
		LB->x1 = x + GPS.GetGPWidth(GP_File, spr);
		LB->y1 = y + Ny * OneLy;
		LB->OneLy = OneLy;
		LB->DRAWPROC = PDRAW;
		LB->N = 0;
		LB->GP_Index = GP_File;
		LB->StSprite = spr;
		LB->VS = nullptr;
		LB->CurrentItem = 0;
		LB->M_OvrItem = -1;
		LB->Enabled = 1;
		LB->OnMouseOver = &ComplexBox_OnMouseOver;
		LB->OnDraw = &ComplexBox_OnDraw;
		LB->VS = nullptr;
		LB->NOnScr = Ny;
		LB->param = 0;
		return LB;
	}
	return nullptr;
}

//-------------end of ComplexBox-------------//

//-------------class : CustomBox------------//
bool CustomBox_OnDraw(SimpleDialog* SD)
{
	CustomBox* LB = (CustomBox*)SD;
	if (LB->Visible)
		LB->DRAWPROC(LB->x, LB->y, LB->x1 - LB->x + 1, LB->y1 - LB->y + 1, 0, LB->MouseOver, LB->param);
	return true;
};

CustomBox* DialogsSystem::addCustomBox(int x, int y, int Lx, int Ly, procDrawBoxElement* PDRAW)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		CustomBox* LB = new CustomBox;
		DSS[i] = LB;
		LB->NeedToDraw = true;
		LB->Parent = nullptr;
		LB->x = x + BaseX;
		LB->y = y + BaseY;
		LB->x1 = x + Lx - 1;
		LB->y1 = y + Ly - 1;
		LB->DRAWPROC = PDRAW;
		LB->Enabled = 1;
		LB->OnDraw = &CustomBox_OnDraw;
		LB->param = 0;
		return LB;
	};
	return nullptr;
};
//-------------end of ComplexBox-------------//
//-------------class : ListBox---------------//
ListBoxItem* ListBox::GetItem(int i)
{
	ListBoxItem* LB = FirstItem;
	for (int j = 0; j < i; j++)
	{
		if (LB)LB = LB->NextItem;
	};
	return LB;
};
bool LB_SORT_ITEM = 0;

void ListBox::AddItem(char* str, int n)
{
	NeedToDraw = true;
	ListBoxItem* LB = new ListBoxItem;
	if (LastItem)
	{
		if (LB_SORT_ITEM)
		{
			ListBoxItem* LBF = FirstItem;
			ListBoxItem** PPTR = &FirstItem;
			bool NeedIns = 1;
			do
			{
				if (_stricmp(LBF->Message, str) > 0)
				{
					*PPTR = LB;
					LB->NextItem = LBF;
					NeedIns = 0;
				}
				else
				{
					if (!LBF->NextItem)
					{
						LBF->NextItem = LB;
						LB->NextItem = 0;
						NeedIns = 0;
					}
					else
					{
						PPTR = &LBF->NextItem;
						LBF = LBF->NextItem;
					};
				};
			}
			while (NeedIns);
		}
		else
		{
			LastItem->NextItem = LB;
			LB->NextItem = nullptr;
		};
	}
	else
	{
		FirstItem = LB;
		LastItem = LB;
		LB->NextItem = nullptr;
	};
	LastItem = LB;
	NItems++;
	LB->Message = new char[strlen(str) + 1];
	strcpy(LB->Message, str);
	LB->Param1 = n;
	LB->Flags = 0;
};

void ListBox::AddStaticItem(char* str, int n)
{
	NeedToDraw = true;
	ListBoxItem* LB = new ListBoxItem;
	if (LastItem)
	{
		LastItem->NextItem = LB;
		LB->NextItem = nullptr;
	}
	else
	{
		FirstItem = LB;
		LastItem = LB;
		LB->NextItem = nullptr;
	};
	LastItem = LB;
	NItems++;
	LB->Message = new char[strlen(str) + 1];
	strcpy(LB->Message, str);
	LB->Param1 = n;
	LB->Flags = 1;
};

void ListBox::ClearItems()
{
	NeedToDraw = true;
	ListBoxItem *LB1, *LB = FirstItem;
	while (LB)
	{
		free(LB->Message);
		LB1 = LB->NextItem;
		free(LB);
		LB = LB1;
	};
	NItems = 0;
	FirstItem = nullptr;
	LastItem = nullptr;
	NItems = 0;
	CurItem = 0;
	FLItem = 0;
};

void ListBox::SetFirstPos(int n)
{
	if (n != FLItem)NeedToDraw = true;
	FLItem = n;
};

void ListBox::SetCurrentItem(int n)
{
	if (n != CurItem)NeedToDraw = true;
	CurItem = n;
	if (CurItem >= NItems)CurItem = NItems - 1;
	if (!NItems)CurItem = -1;
	if (CurItem < FLItem)FLItem = CurItem;
};

bool ListBox_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	ListBox* LB = (ListBox*)SD;
	if (LB)
	{
		if (!LB->MouseOver)LB->CurMouseOver = -1;
		if (KeyPressed && LB->NItems && LB->Active)
		{
			if (LastKey == 40)LB->CurItem++;
			if (LastKey == 38)LB->CurItem--;
			if (LastKey == 34)LB->CurItem += 3;
			if (LastKey == 33)LB->CurItem -= 3;
			if (LB->CurItem < 0)LB->CurItem = 0;
			if (LB->CurItem > LB->NItems - 1)LB->CurItem = LB->NItems - 1;
			if (LastKey == 40 || LastKey == 38 || LastKey == 34 || LastKey == 33)
			{
				if (LB->CurItem < LB->FLItem)LB->FLItem = LB->CurItem;
				if (LB->CurItem - LB->FLItem >= LB->ny)LB->FLItem = LB->CurItem - LB->ny + 1;
				if (LB->VS && LB->VS->Visible)LB->VS->SPos = LB->FLItem * 32;
			};
			KeyPressed = false;
		};
		if (LB->GP_File == -1)
		{
			int y = LB->y;
			for (int i = 0; i < LB->ny; i++)
			{
				LB->ItemPic->Draw(LB->x, y);
				int Iind = LB->FLItem + i;
				ListBoxItem* str = LB->GetItem(Iind);
				if (str)
				{
					if (str->Flags)
					{
						ShowString(LB->x + 8 + LB->FontDx, y + 8 + LB->FontDy, str->Message, LB->DFont);
					}
					else
					{
						if (Iind == LB->CurItem)
							ShowString(LB->x + 8 + LB->FontDx, y + 8 + LB->FontDy, str->Message, LB->AFont);
						else ShowString(LB->x + 8 + LB->FontDx, y + 8 + LB->FontDy, str->Message, LB->PFont);
					};
				};
				y += LB->oneLy - 1;
			};
		}
		else
		{
			int y = LB->y;
			int fdy = ((LB->oneLy - GetRLCHeight(LB->AFont->RLC, 'W')) >> 1) + LB->FontDy;
			int fdx = LB->FontDx;
			for (int i = 0; i < LB->ny; i++)
			{
				LB->ItemPic->Draw(LB->x, y);
				int Iind = LB->FLItem + i;
				ListBoxItem* str = LB->GetItem(Iind);
				if (str)
				{
					if (LB->M_Over != -1)
					{
						if (Iind == LB->CurMouseOver)
						{
							GPS.ShowGP(LB->x, y, LB->GP_File, LB->M_Over + (i % 3), 0);
							ShowString(LB->x + fdx, y + fdy, str->Message, LB->AFont);
						}
						else
						{
							if (Iind == LB->CurItem)GPS.ShowGP(LB->x, y, LB->GP_File, LB->Sprite + (i % 3), 0);
							ShowString(LB->x + fdx, y + fdy, str->Message, LB->PFont);
						};
					}
					else
					{
						if (str->Flags)
						{
							ShowString(LB->x + fdx, y + fdy, str->Message, LB->DFont);
						}
						else
						{
							if (Iind == LB->CurItem)
							{
								GPS.ShowGP(LB->x, y, LB->GP_File, LB->Sprite + (i % 3), 0);
								ShowString(LB->x + fdx, y + fdy, str->Message, LB->AFont);
							}
							else ShowString(LB->x + fdx, y + fdy, str->Message, LB->PFont);
						};
					};
				};
				y += LB->oneLy;
			};
		};
	};
	return true;
};

bool ListBox_OnMouseOver(SimpleDialog* SD)
{
	if (!Lpressed)return false;
	ListBox* LB = (ListBox*)SD;
	SD->NeedToDraw = true;
	int ni = div( mouseY - LB->y - 8 - LB->FontDy, LB->oneLy - 1 ).quot;
	int curitem;
	if (ni < LB->ny)
	{
		if (LB->FLItem + ni < LB->NItems)
			curitem = LB->FLItem + ni;
		else curitem = LB->NItems - 1;
		if (curitem >= 0)
		{
			ListBoxItem* LBI = LB->GetItem(curitem);
			if (!LBI->Flags)LB->CurItem = curitem;
		};
	};
	return true;
};

bool GPListBox_OnMouseOver(SimpleDialog* SD)
{
	//if(!Lpressed)return false;
	ListBox* LB = (ListBox*)SD;
	LB->CurMouseOver = -1;
	SD->NeedToDraw = true;
	int ni = div( mouseY - LB->y, LB->oneLy ).quot;
	int curitem;
	if (ni < LB->ny)
	{
		if (LB->FLItem + ni < LB->NItems)
			curitem = LB->FLItem + ni;
		else curitem = -1;
		if (curitem == -1)return 1;
		if (Lpressed)
		{
			if (curitem >= 0)
			{
				ListBoxItem* LBI = LB->GetItem(curitem);
				if (!LBI->Flags)LB->CurItem = curitem;
			};
		};
		LB->CurMouseOver = curitem;
	};
	return true;
};

bool ListBox_Refresh(SimpleDialog* SD)
{
	ListBox* LB = (ListBox*)SD;
	if (LB->VS)
	{
		VScrollBar* VS = LB->VS;
		VS->SetMaxPos((LB->NItems - LB->ny) * 32);
		VS->OnesDy = 32;
		if (VS->GP_File != -1)
		{
			VS->ScrDy = (LB->ny - 1) * 32;
		};
		if (VS->GetMaxPos() > 0)LB->FLItem = (VS->GetPos() + 16) / 32;
	};
	return true;
};

ListBox* DialogsSystem::addListBox(SimpleDialog* Parent,
                                   int x, int y, int Ny,
                                   SQPicture* ItemPic,
                                   RLCFont* AFont,
                                   RLCFont* PFont,
                                   VScrollBar* VS)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ListBox* LB = new ListBox;
		DSS[i] = LB;
		LB->NeedToDraw = true;
		LB->Parent = Parent;
		LB->AFont = AFont;
		LB->PFont = PFont;
		LB->DFont = AFont;
		LB->ItemPic = ItemPic;
		LB->x = x + BaseX;
		LB->y = y + BaseY;
		LB->oneLy = byte(ItemPic->PicPtr[1]);
		LB->oneLx = ItemPic->PicPtr[0];
		LB->NItems = 0;
		LB->FirstItem = nullptr;
		LB->LastItem = nullptr;
		LB->y1 = LB->y + Ny * int(LB->oneLy);
		LB->x1 = LB->x + LB->oneLx;
		LB->OnDraw = &ListBox_OnDraw;
		LB->OnMouseOver = &ListBox_OnMouseOver;
		LB->Refresh = &ListBox_Refresh;
		LB->FLItem = 0;
		LB->CurItem = 0;
		LB->ny = Ny;
		LB->VS = VS;
		LB->GP_File = -1;
		LB->FontDy = 0;
		LB->FontDx = 0;
		LB->M_Over = -1;
		LB->CurMouseOver = -1;
		LB->FontDx = 0;
		LB->FontDy = 0;
		return LB;
	};
	return nullptr;
};

ListBox* DialogsSystem::addListBox(SimpleDialog* Parent,
                                   int x, int y, int Ny, int Lx, int Ly,
                                   RLCFont* AFont,
                                   RLCFont* PFont,
                                   VScrollBar* VS)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ListBox* LB = new ListBox;
		DSS[i] = LB;
		LB->NeedToDraw = true;
		LB->Parent = Parent;
		LB->AFont = AFont;
		LB->PFont = PFont;
		LB->DFont = AFont;
		LB->ItemPic = nullptr;
		LB->x = x + BaseX;
		LB->y = y + BaseY;
		LB->oneLy = Ly;
		LB->oneLx = Lx;
		LB->NItems = 0;
		LB->FirstItem = nullptr;
		LB->LastItem = nullptr;
		LB->y1 = LB->y + Ny * int(LB->oneLy);
		LB->x1 = LB->x + LB->oneLx;
		LB->OnDraw = &ListBox_OnDraw;
		LB->OnMouseOver = &ListBox_OnMouseOver;
		LB->Refresh = &ListBox_Refresh;
		LB->FLItem = 0;
		LB->CurItem = 0;
		LB->ny = Ny;
		LB->VS = VS;
		LB->GP_File = -1;
		LB->FontDy = 0;
		LB->FontDx = 0;
		LB->M_Over = -1;
		return LB;
	};
	return nullptr;
};

ListBox* DialogsSystem::addGP_ListBox(SimpleDialog* Parent, int x, int y, int Ny,
                                      int GP_File, int Sprite, int Ly,
                                      RLCFont* AFont,
                                      RLCFont* PFont,
                                      VScrollBar* VS)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ListBox* LB = new ListBox;
		DSS[i] = LB;
		LB->NeedToDraw = true;
		LB->Parent = Parent;
		LB->AFont = AFont;
		LB->PFont = PFont;
		LB->DFont = PFont;
		LB->ItemPic = nullptr;
		LB->FontDy = 0;
		LB->x = x + BaseX;
		LB->y = y + BaseY;
		LB->oneLy = Ly;
		LB->oneLx = GPS.GetGPWidth(GP_File, Sprite);
		LB->NItems = 0;
		LB->FirstItem = nullptr;
		LB->LastItem = nullptr;
		LB->y1 = LB->y + Ny * int(LB->oneLy);
		LB->x1 = LB->x + LB->oneLx;
		LB->OnDraw = &ListBox_OnDraw;
		LB->OnMouseOver = &GPListBox_OnMouseOver;
		LB->Refresh = &ListBox_Refresh;
		LB->FLItem = 0;
		LB->CurItem = 0;
		LB->ny = Ny;
		LB->VS = VS;
		LB->FontDx = 24;
		LB->FontDy = 0;
		LB->Sprite = Sprite;
		LB->GP_File = GP_File;
		LB->M_Over = -1;
		return LB;
	};
	return nullptr;
};
//-------------end of List Box---------------//
//-----------class : InputBox----------------//
void CutStringToLength(char* str, RLCFont* Fnt, int Lx)
{
	int L;
	do
	{
		L = GetRLCStrWidth(str, Fnt);
		if (L > Lx)str[strlen(str) - 1] = 0;
	}
	while (L > Lx);
};

bool InputBox_OnDraw(SimpleDialog* SD)
{
	InputBox* IB = (InputBox*)SD;
	if (!SD->Visible)return false;
	if (SD)
	{
		//CutStringToLength(IB->Str,IB->AFont,IB->x1-IB->x);
		int LL = IB->x1 - IB->x;
		char base[2048];
		strcpy(base, IB->Str);
		if (IB->Anonim)
		{
			int L = strlen(IB->Str);
			for (int i = 0; i < L; i++)base[i] = '*';
		};
		int xx = 0;
		if (IB->CursPos > strlen(IB->Str))
		{
			IB->CursPos = strlen(IB->Str);
		}
		for (size_t j = 0; j < IB->CursPos; j++)
		{
			xx += GetRLCWidth(IB->Font->RLC, base[j]);
		};
		if (xx < IB->totdx)
		{
			IB->totdx -= 64;
			if (IB->totdx < 0)IB->totdx = 0;
		};
		if (xx - IB->totdx > LL - 32)
		{
			IB->totdx = xx - LL + 32;
		};
		TempWindow TW;
		PushWindow(&TW);
		IntersectWindows(IB->x, IB->y, IB->x1, IB->y1);

		//if(IB->Active&&(GetTickCount()/250)&1)strcat(base,"|");
		if (IB->Pict)IB->Pict->Draw(IB->x, IB->y);
		if (IB->Active)
		{
			int dx = 0;
			if (IB->Centering)dx = (IB->x1 - IB->x - GetRLCStrWidth(base, IB->AFont)) >> 1;
			ShowString(IB->x + 8 + dx - IB->totdx, IB->y + 4, base, IB->AFont);
		}
		else
		{
			int dx = 0;
			if (IB->Centering)dx = (IB->x1 - IB->totdx - IB->x - GetRLCStrWidth(base, IB->Font)) >> 1;
			ShowString(IB->x + 8 + dx, IB->y + 4, base, IB->Font);
		};
		base[0] = '|';
		base[1] = 0;
		if (IB->Active && (GetTickCount() / 250) & 1)ShowString(IB->x + 8 + xx - IB->totdx, IB->y + 4, base, IB->Font);
		PopWindow(&TW);
	};
	return true;
};

bool InputBox_OnClick(SimpleDialog* SD)
{
	if (!SD->Active)SD->NeedToDraw = true;
	SD->Active = true;
	InputBox* IB = (InputBox*)SD;
	if (IB->MouseOver && Lpressed)
	{
		int xxx = mouseX - IB->x + IB->totdx - 8;
		int xx1 = 0;
		size_t L = strlen(IB->Str);
		size_t j;
		for (j = 0; j < L; j++)
		{
			if (IB->Anonim)xx1 += GetRLCWidth(IB->Font->RLC, '*');
			else xx1 += GetRLCWidth(IB->Font->RLC, IB->Str[j]);
			if (xx1 >= xxx)
			{
				if (j >= 0)IB->CursPos = j;
				if (IB->CursPos < 0)IB->CursPos = 0;
				j = L + 1;
			};
		};
		if (j == L)IB->CursPos = L;
		Lpressed = 0;
	};
	return true;
};
extern int NKeys;

bool InputBox_OnKeyDown(SimpleDialog* SD)
{
	SD->NeedToDraw = true;
	InputBox* IB = (InputBox*)SD;
	if (LastKey == VK_LEFT)
	{
		if (IB->CursPos > 0)IB->CursPos--;
		KeyPressed = 0;
		LastKey = 0;
		return true;
	}
	else if (LastKey == VK_RIGHT)
	{
		if (IB->CursPos < strlen(IB->Str))IB->CursPos++;
		KeyPressed = 0;
		LastKey = 0;
		return true;
	}
	else if (LastKey == VK_END)
	{
		IB->CursPos = strlen(IB->Str);
		KeyPressed = 0;
		LastKey = 0;
		return true;
	}
	else if (LastKey == VK_HOME)
	{
		IB->CursPos = 0;
		KeyPressed = 0;
		LastKey = 0;
		return true;
	}
	else if (LastKey == VK_BACK)
	{
		if (IB->CursPos > 0)
		{
			strcpy(IB->Str + IB->CursPos - 1, IB->Str + IB->CursPos);
			IB->CursPos--;
		};
		//if(strlen(IB->Str)>0)IB->Str[strlen(IB->Str)-1]=0;
		return true;
	}
	else if (LastKey == 46)
	{
		//VK_DEL
		if (IB->CursPos < strlen(IB->Str))
		{
			strcpy(IB->Str + IB->CursPos, IB->Str + IB->CursPos + 1);
		}
		//if(strlen(IB->Str)>0)IB->Str[strlen(IB->Str)-1]=0;
		return true;
	}
	else
	{
		if (LastAsciiKey && LastAsciiKey >= 32)
		{
			LastKey = LastAsciiKey;
			char xx[2];
			xx[1] = 0;
			xx[0] = char(LastKey);
			if (strlen(IB->Str) + 1 < DWORD(IB->StrMaxLen))
			{
				char ccc[2048];
				strcpy(ccc, IB->Str);
				ccc[IB->CursPos] = 0;
				strcat(ccc, xx);
				strcat(ccc, IB->Str + IB->CursPos);
				strcpy(IB->Str, ccc);
				IB->CursPos++;
			};
			return true;
		};
	};
	return false;
};

InputBox* DialogsSystem::addInputBox(SimpleDialog* Parent,
                                     int x, int y, char* str,
                                     int Len,
                                     SQPicture* Panel,
                                     RLCFont* RFont,
                                     RLCFont* AFont)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ClearKeyStack();
		InputBox* IB = new InputBox;
		DSS[i] = IB;
		IB->NeedToDraw = true;
		IB->Parent = Parent;
		IB->x = x + BaseX;
		IB->y = y + BaseY;
		IB->Pict = Panel;
		IB->Font = RFont;
		IB->AFont = AFont;
		IB->StrMaxLen = Len;
		IB->Str = str;
		IB->x1 = IB->x + Panel->PicPtr[0] - 1;
		IB->y1 = IB->y + Panel->PicPtr[1] - 1;
		IB->OnDraw = &InputBox_OnDraw;
		IB->OnClick = &InputBox_OnClick;
		IB->OnKeyDown = &InputBox_OnKeyDown;
		IB->Centering = false;
		IB->CursPos = strlen(str);
		IB->totdx = 0;
		IB->Anonim = 0;
		return IB;
	};
	return nullptr;
};

InputBox* DialogsSystem::addInputBox(SimpleDialog* Parent,
                                     int x, int y, char* str,
                                     int Len,
                                     int Lx, int Ly,
                                     RLCFont* RFont,
                                     RLCFont* AFont,
                                     bool Centering)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		InputBox* IB = new InputBox;
		DSS[i] = IB;
		IB->NeedToDraw = true;
		IB->Parent = Parent;
		IB->x = x + BaseX;
		if (Centering)IB->x -= Lx >> 1;
		IB->y = y + BaseY;
		IB->Pict = nullptr;
		IB->Font = RFont;
		IB->AFont = AFont;
		IB->StrMaxLen = Len;
		IB->Str = str;
		IB->x1 = IB->x + Lx;
		IB->y1 = IB->y + Ly;
		IB->OnDraw = &InputBox_OnDraw;
		IB->OnClick = &InputBox_OnClick;
		IB->OnKeyDown = &InputBox_OnKeyDown;
		IB->Centering = Centering;
		IB->CursPos = strlen(str);
		IB->totdx = 0;
		IB->Anonim = 0;
		return IB;
	};
	return nullptr;
};

InputBox* DialogsSystem::addInputBox(SimpleDialog* Parent,
                                     int x, int y, char* str,
                                     int Len,
                                     int Lx, int Ly,
                                     RLCFont* RFont,
                                     RLCFont* AFont)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		InputBox* IB = new InputBox;
		DSS[i] = IB;
		IB->NeedToDraw = true;
		IB->Parent = Parent;
		IB->x = x + BaseX;
		IB->y = y + BaseY;
		IB->Pict = nullptr;
		IB->Font = RFont;
		IB->AFont = AFont;
		IB->StrMaxLen = Len;
		IB->Str = str;
		IB->x1 = IB->x + Lx - 1;
		IB->y1 = IB->y + Ly - 1;
		IB->OnDraw = &InputBox_OnDraw;
		IB->OnClick = &InputBox_OnClick;
		IB->OnKeyDown = &InputBox_OnKeyDown;
		IB->Centering = false;
		IB->CursPos = strlen(str);
		IB->totdx = 0;
		IB->Anonim = 0;
		return IB;
	};
	return nullptr;
};
//------end of class InputBox----------------//
//----------class : CheckBox-----------------//
bool CheckBox_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	CheckBox* CB = (CheckBox*)SD;
	if (CB->GP_File != -1)
	{
		if (CB->Central)
		{
			int Lx = 0;
			int Ly = 0;
			int spr;
			if (CB->MouseOver)spr = CB->Sprite0;
			else
			{
				if (CB->State)spr = CB->Sprite1;
				else spr = CB->Sprite2;
			};
			GPS.ShowGP(CB->x, CB->y, CB->GP_File, spr + 1, 0);
			GPS.ShowGP(CB->x1 - GPS.GetGPWidth(CB->GP_File, spr + 2), CB->y, CB->GP_File, spr + 2, 0);
			TempWindow TW;
			PushWindow(&TW);
			int x0 = CB->x + GPS.GetGPWidth(CB->GP_File, spr + 1);
			int x1 = CB->x1 - GPS.GetGPWidth(CB->GP_File, spr + 2);
			IntersectWindows(x0, 0, x1, RealLx);
			int LL = GPS.GetGPWidth(CB->GP_File, spr);
			if (!LL)LL = 1;
			int N = (x1 - x0) / LL + 1;
			for (int i = 0; i < N; i++)
			{
				GPS.ShowGP(x0 + i * LL, CB->y, CB->GP_File, spr, 0);
			};
			if (CB->MouseOver)
			{
				int Ty = GetRLCHeight(CB->AFont->RLC, 'C');
				int Tx = GetRLCStrWidth(CB->Message, CB->AFont);
				ShowString((CB->x + CB->x1 - Tx) >> 1, (CB->y + CB->y1 - Ty) >> 1, CB->Message, CB->AFont);
			}
			else
			{
				int Ty = GetRLCHeight(CB->AFont->RLC, 'C');
				int Tx = GetRLCStrWidth(CB->Message, CB->AFont);
				ShowString((CB->x + CB->x1 - Tx) >> 1, (CB->y + CB->y1 - Ty) >> 1, CB->Message, CB->Font);
			};
			PopWindow(&TW);
		}
		else
		{
			if (CB->State)
			{
				if (CB->MouseOver && CB->Sprite3 != -1)GPS.ShowGP(CB->x, CB->y, CB->GP_File, CB->Sprite3, 0);
				GPS.ShowGP(CB->x, CB->y, CB->GP_File, CB->Sprite1, 0);
			}
			else
			{
				if (CB->MouseOver)GPS.ShowGP(CB->x, CB->y, CB->GP_File, CB->Sprite0, 0);
				else GPS.ShowGP(CB->x, CB->y, CB->GP_File, CB->Sprite2, 0);
			};
			if (CB->MouseOver)ShowString(CB->x + GPS.GetGPWidth(CB->GP_File, CB->Sprite0), CB->y + 4, CB->Message, CB->Font);
			else ShowString(CB->x + GPS.GetGPWidth(CB->GP_File, CB->Sprite0), CB->y + 4, CB->Message, CB->AFont);
		};
	}
	else
	{
		if (CB->Transparent)
		{
			if (CB->State)CB->OnPic->DrawTransparent(CB->x, CB->y);
			else CB->OffPic->DrawTransparent(CB->x, CB->y);
		}
		else
		{
			if (CB->State)CB->OnPic->Draw(CB->x, CB->y);
			else CB->OffPic->Draw(CB->x, CB->y);
		};
		if (CB->MouseOver)ShowString(CB->x + CB->OnPic->PicPtr[0], CB->y + 4, CB->Message, CB->Font);
		else ShowString(CB->x + CB->OnPic->PicPtr[0], CB->y + 4, CB->Message, CB->AFont);
	};
	return true;
}

bool CheckBox_OnClick(SimpleDialog* SD)
{
	SD->NeedToDraw = true;
	CheckBox* CB = (CheckBox*)SD;
	if (CB->GroupIndex)
	{
		DialogsSystem* DSM = CB->DS;
		int st = CB->State;
		for (int i = 0; i < MAXDLG; i++)
		{
			CheckBox* SDD = (CheckBox*)DSM->DSS[i];
			if (SDD && SDD->OnClick == &CheckBox_OnClick && SDD->GroupIndex == CB->GroupIndex)
			{
				SDD->State = false;
				SDD->NeedToDraw = true;
			}
		}
		if (st == 1)
		{
			CB->State = 2;
		}
		else
		{
			CB->State = 1;
		}
	}
	else
	{
		CB->State = !CB->State;
	}
	return false;
}

bool CheckBox_Destroy(SimpleDialog* SD)
{
	CheckBox* CB = (CheckBox*)SD;
	free(CB->Message);
	return true;
}

bool CheckBox_OnMouseOver(SimpleDialog* SD)
{
	SD->NeedToDraw = true;
	return false;
}

CheckBox* DialogsSystem::addCheckBox(SimpleDialog* Parent,
                                     int x, int y, char* message,
                                     int group, bool State,
                                     SQPicture* OnPict,
                                     SQPicture* OffPict,
                                     RLCFont* Font,
                                     RLCFont* AFont)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		CheckBox* CB = new CheckBox;
		DSS[i] = CB;
		CB->MouseOverActive = true;
		CB->NeedToDraw = true;
		CB->Parent = Parent;
		CB->x = x + BaseX;
		CB->y = y + BaseY;
		CB->GroupIndex = group;
		CB->State = State;
		CB->OnPic = OnPict;
		CB->OffPic = OffPict;
		CB->x1 = CB->x + OnPict->PicPtr[0] + GetRLen(message, Font) - 1 + 2;
		CB->y1 = CB->y + OnPict->PicPtr[1] - 1 + 2;
		CB->DS = this;
		CB->OnClick = &CheckBox_OnClick;
		CB->Destroy = &CheckBox_Destroy;
		CB->OnDraw = &CheckBox_OnDraw;
		CB->Font = Font;
		CB->AFont = AFont;
		CB->Message = new char[strlen(message) + 1];
		CB->Transparent = true;
		CB->GP_File = -1;
		CB->Central = 0;
		CB->Sprite3 = -1;
		strcpy(CB->Message, message);
		return CB;
	};
	return nullptr;
};

CheckBox* DialogsSystem::addGP_CheckBox(SimpleDialog* Parent,
                                        int x, int y, char* message, RLCFont* a_font, RLCFont* p_font,
                                        int group, bool State,
                                        int GP, int active, int passive, int mouseover)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		CheckBox* CB = new CheckBox;
		DSS[i] = CB;
		CB->MouseOverActive = true;
		CB->NeedToDraw = true;
		CB->Parent = Parent;
		CB->x = x + BaseX;
		CB->y = y + BaseY;
		CB->GroupIndex = group;
		CB->State = State;
		CB->GP_File = GP;
		CB->Sprite0 = mouseover;
		CB->Sprite1 = active;
		CB->Sprite2 = passive;
		CB->Sprite3 = -1;
		CB->x1 = CB->x + GPS.GetGPWidth(GP, active) + GetRLen(message, a_font);
		CB->y1 = CB->y + GPS.GetGPHeight(GP, active);
		CB->DS = this;
		CB->OnClick = &CheckBox_OnClick;
		CB->Destroy = &CheckBox_Destroy;
		CB->OnDraw = &CheckBox_OnDraw;
		CB->Font = p_font;
		CB->AFont = a_font;
		CB->Message = new char[strlen(message) + 1];
		CB->Transparent = true;
		strcpy(CB->Message, message);
		CB->Central = 0;
		CB->Sprite3 = -1;
		return CB;
	};
	return nullptr;
};
//--------end of CheckBox--------------------//

//-----------colored bar---------------------//
__declspec( dllexport ) void CBar(int x0, int y0, int Lx0, int Ly0, byte c)
{
	if (Lx0 <= 0 || Ly0 <= 0 || x0 < 0 || y0 < 0 || x0 + Lx0 > ScrWidth || y0 + Ly0 > SCRSizeY)
	{
		return;
	}

	//clipping
	int x = x0;
	int y = y0;
	int Lx = Lx0;
	int Ly = Ly0;
	if (x < WindX)
	{
		Lx -= WindX - x;
		x = WindX;
	};
	if (y < WindY)
	{
		Ly -= WindY - y;
		y = WindY;
	};
	if (Lx <= 0 || Ly <= 0)return;
	if (x + Lx > WindX1)
	{
		Lx = WindX1 - x + 1;
		if (Lx <= 0)return;
	};
	if (y + Ly > WindY1)
	{
		Ly = WindY1 - y + 1;
		if (Ly <= 0)return;
	};
	int ofst = int(ScreenPtr) + x + y * ScrWidth;
	int adds = ScrWidth - Lx;
	int Lx4 = Lx >> 2;
	int Lx1 = Lx & 3;
	__asm
	{
		push edi
		mov edi, ofst
		mov edx, Ly
		cld
		mov al, c
		mov ah, al
		shl eax, 16
		mov al, c
		mov ah, al
		mov ebx, Lx1
		qwr : mov ecx, Lx4
		rep stosd
		mov ecx, ebx
		rep stosb
		add edi, adds
		dec edx
		jnz qwr
		pop edi
	}
}

bool ColoredBar_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	ColoredBar* PIC = (ColoredBar*)SD;
	if (PIC->Style)Xbar(SD->x, SD->y, SD->x1 - SD->x + 1, SD->y1 - SD->y + 1, PIC->color);
	else CBar(SD->x, SD->y, SD->x1 - SD->x + 1, SD->y1 - SD->y + 1, PIC->color);
	return true;
};

ColoredBar* DialogsSystem::addColoredBar(int x, int y, int Lx, int Ly, byte c)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ColoredBar* PIC = new ColoredBar;
		DSS[i] = PIC;
		PIC->color = c;
		PIC->x = x + BaseX;
		PIC->y = y + BaseY;
		PIC->x1 = PIC->x + Lx;
		PIC->y1 = PIC->y + Ly;
		PIC->Parent = nullptr;
		PIC->OnDraw = &ColoredBar_OnDraw;
		PIC->Style = 0;
		return PIC;
	};
	return nullptr;
};

bool ColoredBar_OnDraw2(SimpleDialog* SD)
{
	if (!(SD->Visible && SD->MouseOver))return false;
	ColoredBar* PIC = (ColoredBar*)SD;
	Xbar(SD->x, SD->y, SD->x1 - SD->x + 1, SD->y1 - SD->y + 1, PIC->color);
	return true;
};

ColoredBar* DialogsSystem::addViewPort2(int x, int y, int Lx, int Ly, byte c)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ColoredBar* PIC = new ColoredBar;
		DSS[i] = PIC;
		PIC->color = c;
		PIC->x = x + BaseX;
		PIC->y = y + BaseY;
		PIC->x1 = PIC->x + Lx;
		PIC->y1 = PIC->y + Ly;
		PIC->Parent = nullptr;
		PIC->OnDraw = &ColoredBar_OnDraw2;
		PIC->Style = 0;
		return PIC;
	};
	return nullptr;
};
//---------------Text Viewer-----------------//
//---------Service functions---------
void TextViewer::AssignScroll(VScrollBar* SB)
{
	VS = SB;
	SB->SMaxPos = -1;
	SB->SPos = 0;
}

void TextViewer::GetNextLine(LineInfo* inf)
{
	int offs = inf->Offset;
	word LS = 0;
	word LW = 0;
	word NS = 0;
	//char tms[128];
	char c;
	int tmsp = 0;
	int ofs;
	int wid;
	int spw = GetRLCWidth(Font->RLC, ' ');
	int nlsp = 0; //кол-во пробелов перед словом
	while (offs < TextSize)
	{
		//char c=TextPtr[offs];
		//if(c==' '){
		//reading next word
		tmsp = 0; //length of string
		nlsp = 0; //amount of spaces
		wid = 0;
		while (TextPtr[offs + nlsp] == ' ' && offs + nlsp < TextSize)
		{
			nlsp++;
			wid += spw;
		};
		do
		{
			ofs = offs + nlsp + tmsp;
			c = 0;
			if (ofs < TextSize)
			{
				c = TextPtr[ofs];
				if (c != ' ' && c != 0x0D)
				{
					//tms[tmsp]=c;
					//tmsp++;
					int L = 1;
					if (c != '~')wid += GetRLCWidthUNICODE(Font->RLC, (byte*)(TextPtr + ofs), &L);
					tmsp += L;
				};
			};
		}
		while (ofs < TextSize && c != ' ' && c != 0x0D);
		if (LW + wid < Lx && c == 0x0D)
		{
			inf->NeedFormat = false;
			inf->NextOffset = offs + tmsp + nlsp + 2;
			inf->NSpaces = NS;
			inf->LineWidth = LW;
			inf->LineSize = offs - inf->Offset + tmsp + nlsp;
			if (inf->NextOffset > TextSize)inf->NextOffset = TextSize;
			return;
		};
		if (LW + wid < Lx)
		{
			//still current line
			LW += wid;
			offs += nlsp + tmsp;
			NS += nlsp;
		}
		else
		{
			if (inf->NextOffset == offs + nlsp)
			{
				//need to break line
				tmsp = 0; //length of string
				nlsp = 0; //amount of spaces
				wid = 0;
				while (TextPtr[offs + nlsp] == ' ' && offs + nlsp < TextSize)
				{
					nlsp++;
					wid += spw;
				};
				int prevpos = 0;
				int prevwid = 0;
				do
				{
					prevpos = tmsp;
					prevwid = wid;
					ofs = offs + nlsp + tmsp;
					c = 0;
					if (ofs < TextSize)
					{
						c = TextPtr[ofs];
						if (c != ' ' && c != 0x0D)
						{
							int L = 1;
							if (c != '~')wid += GetRLCWidthUNICODE(Font->RLC, (byte*)(TextPtr + ofs), &L);
							tmsp += L;
						};
					};
				}
				while (ofs < TextSize && c != ' ' && c != 0x0D && LW + wid < Lx);
				if (LW + wid >= Lx)
				{
					wid = prevwid;
					tmsp = prevpos;
				};
				inf->NeedFormat = true;
				inf->NextOffset = offs + nlsp + tmsp;
				inf->NSpaces = NS + nlsp;
				inf->LineWidth = LW + wid;
				inf->LineSize = offs + nlsp + tmsp - inf->Offset;
				if (inf->NextOffset > TextSize)inf->NextOffset = TextSize;
				return;
			}
			else
			{
				inf->NeedFormat = true;
				inf->NextOffset = offs + nlsp;
				inf->NSpaces = NS;
				inf->LineWidth = LW;
				inf->LineSize = offs - inf->Offset;
				if (inf->NextOffset > TextSize)inf->NextOffset = TextSize;
				return;
			};
			//};
		};
		//};
	};
	inf->NeedFormat = false;
	inf->NextOffset = TextSize;
	inf->NSpaces = NS;
	inf->LineWidth = LW;
	inf->LineSize = offs - inf->Offset;
}

void TextViewer::CreateLinesList()
{
	if (!TextPtr)return;
	NLines = 0;
	LineInfo LINF;
	LINF.Offset = 0;
	do
	{
		GetNextLine(&LINF);
		if (LINF.NextOffset != -1)NLines++;
		LINF.Offset = LINF.NextOffset;
	}
	while (LINF.NextOffset < TextSize);
	LinePtrs = new char*[NLines];
	LineSize = new word[NLines];
	LineWidth = new word[NLines];
	NeedFormat = new bool[NLines];
	NSpaces = new word[NLines];
	LINF.Offset = 0;
	NLines = 0;
	do
	{
		GetNextLine(&LINF);
		if (LINF.LineSize)
		{
			LinePtrs[NLines] = TextPtr + LINF.Offset;
			LineSize[NLines] = LINF.LineSize;
			LineWidth[NLines] = LINF.LineWidth;
			NeedFormat[NLines] = LINF.NeedFormat;
			NSpaces[NLines] = LINF.NSpaces;
			NLines++;
		};
		LINF.Offset = LINF.NextOffset;
	}
	while (LINF.NextOffset < TextSize);
};

bool TextViewer_OnDestroy(SimpleDialog* SD)
{
	TextViewer* TV = (TextViewer*)SD;
	if (TV->TextPtr)
	{
		free(TV->TextPtr);
		free(TV->LinePtrs);
		free(TV->LineSize);
		free(TV->LineWidth);
		free(TV->NeedFormat);
		free(TV->NSpaces);
		TV->TextPtr = nullptr;
	};
	return true;
};

bool TextViewer_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	TextViewer* TV = (TextViewer*)SD;
	char c;
	if (!TV->TextPtr)return false;
	if (TV->Line > TV->NLines)TV->Line = TV->NLines - TV->PageSize + 1;
	if (TV->Line < 0)TV->Line = 0;
	if (TV->VS)
	{
		TV->VS->SMaxPos = (TV->NLines - TV->PageSize) * 32;
		TV->VS->ScrDy = (TV->PageSize - 1) * 32;
		TV->VS->OnesDy = 32;
		if (TV->VS->SMaxPos > 0)
		{
			if (TV->VS->SPos > TV->VS->SMaxPos)TV->VS->SPos = TV->VS->SMaxPos;
			TV->Line = (TV->VS->SPos + 16) / 32;
			TV->VS->Visible = 1;
		}
		else
		{
			TV->VS->Visible = 0;
		};
	};
	bool Cross = false;
	for (int i = 0; i < TV->PageSize; i++)
	{
		int lin = TV->Line + i;
		int yy = TV->y + TV->SymSize * i;
		if (lin < TV->NLines)
		{
			char* off = TV->LinePtrs[lin];
			int x = 0;
			int LS = TV->LineSize[lin];
			int LW = TV->LineWidth[lin];
			int NS = TV->NSpaces[lin];
			int addsp = 0;
			if (NS > 0)addsp = div( TV->Lx - LW, NS ).quot;
			if (addsp > 5)addsp = 5;
			if (TV->NeedFormat[lin])
			{
				for (int j = 0; j < LS; j++)
				{
					c = off[j];
					if (c != '~')
					{
						int L = 1;
						ShowCharUNICODE(TV->x + x, yy, (byte*)(off + j), TV->Font);
						int Lx = GetRLCWidthUNICODE(TV->Font->RLC, (byte*)(off + j), &L);
						j += L - 1;
						if (Cross)
						{
							Hline(TV->x + x, yy + 8, TV->x + x + Lx, 0);
							Hline(TV->x + x, yy + 9, TV->x + x + Lx, 0);
						};
						x += Lx;
						if (c == ' ')x += addsp;
					}
					else Cross = !Cross;
				};
			}
			else
			{
				for (int j = 0; j < LS; j++)
				{
					c = off[j];
					if (c != '~')
					{
						ShowCharUNICODE(TV->x + x, yy, (byte*)(off + j), TV->Font);
						int L = 1;
						int Lx = GetRLCWidthUNICODE(TV->Font->RLC, (byte*)(off + j), &L);
						j += L - 1;
						if (Cross)
						{
							Hline(TV->x + x, yy + 8, TV->x + x + Lx, 0);
							Hline(TV->x + x, yy + 9, TV->x + x + Lx, 0);
						};
						x += Lx;
					}
					else Cross = !Cross;
				};
			};
		};
	};
	return true;
};

bool TextView_OnKeyDown(SimpleDialog* SD)
{
	TextViewer* TV = (TextViewer*)SD;
	if (!TV->TextPtr)return false;
	if (KeyPressed)
	{
		switch (LastKey)
		{
		case 38:
			TV->Line--;
			KeyPressed = false;
			break;
		case 40:
			TV->Line++;
			KeyPressed = false;
			break;
		case 33:
			TV->Line -= TV->PageSize - 1;
			KeyPressed = false;
			break;
		case 34:
			TV->Line += TV->PageSize - 1;
			KeyPressed = false;
			break;
		};
		if (TV->Line > TV->NLines - TV->PageSize)TV->Line = TV->NLines - TV->PageSize;
		if (TV->Line < 0)TV->Line = 0;
		if (TV->VS && TV->NLines > TV->PageSize)
		{
			TV->VS->SPos = TV->Line * 32;
		};
	};
	return false;
};

bool TextView_OnClick(SimpleDialog* SD)
{
	if (!SD->Active)SD->NeedToDraw = true;
	SD->Active = true;
	return true;
};

void TextViewer::LoadFile(char* TextFile)
{
	if (TextPtr)
	{
		free(TextPtr);
		free(LinePtrs);
		free(LineSize);
		free(LineWidth);
		free(NeedFormat);
		free(NSpaces);
		TextPtr = nullptr;
	};
	ResFile f1 = RReset(TextFile);
	if (f1 == INVALID_HANDLE_VALUE)
	{
		TextPtr = nullptr;
		NLines = 0;
		return;
	};
	TextSize = RFileSize(f1);
	TextPtr = new char[TextSize + 32];
	RBlockRead(f1, TextPtr, TextSize);
	RClose(f1);
	CreateLinesList();
	Line = 0;
	if (VS)VS->SPos = 0;
};

TextViewer* DialogsSystem::addTextViewer(
	SimpleDialog* Parent,
	int x, int y, int Lx, int Ly, char* TextFile,
	RLCFont* Font)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		TextViewer* TV = new TextViewer;
		DSS[i] = TV;
		TV->NeedToDraw = true;
		TV->Parent = Parent;
		TV->x = x + BaseX;
		TV->y = y + BaseY;
		TV->x1 = TV->x + Lx - 1;
		TV->y1 = TV->y + Ly - 1;
		TV->Lx = Lx;
		TV->OnDraw = &TextViewer_OnDraw;
		TV->Destroy = &TextViewer_OnDestroy;
		TV->Font = Font;
		TV->Line = 0;
		TV->SymSize = GetRLCHeight(Font->RLC, 'W');
		TV->SymSize += UNI_LINEDLY2;
		TV->PageSize = div( Ly, TV->SymSize ).quot;
		ResFile f1 = RReset(TextFile);
		if (f1 == INVALID_HANDLE_VALUE)
		{
			TV->TextPtr = nullptr;
			TV->NLines = 0;
			return TV;
		};
		TV->TextSize = RFileSize(f1);
		TV->TextPtr = new char[TV->TextSize + 32];
		TV->OnClick = &TextView_OnClick;
		RBlockRead(f1, TV->TextPtr, TV->TextSize);
		RClose(f1);
		TV->CreateLinesList();
		TV->OnKeyDown = &TextView_OnKeyDown;
		TV->Enabled = true;
		TV->Active = true;
		TV->VS = nullptr;
		return TV;
	};
	return nullptr;
}

__declspec( dllexport ) char LASTCLICKCHATNAME[128] = "";

bool ChatViewer_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)
	{
		return false;
	}
	ChatViewer* CV = (ChatViewer*)SD;
	int NCHATS = *CV->NChats;
	int DYL = CV->ChatDY;
	int CLY = CV->OneLy;
	int NCL = CV->ScrNy;
	int x0 = CV->x;
	int y0 = CV->y;

	char* PNAME = "";
	int NCLINES = 0;
	char** ChatMess = *CV->Mess;
	char** ChatSender = *CV->Names;
	for (int i = DYL; i < DYL + NCL; i++)
	{
		char cc3[256];
		if (i < NCHATS)
		{
			if (ChatSender[i][0])
			{
				sprintf(cc3, "%s: ", ChatSender[i]);

				int Y0 = y0 + (i - DYL) * CLY;
				int x1 = x0 + GetRLCStrWidth(cc3, &WhiteFont);
				int y1 = Y0 + GetRLCHeight(WhiteFont.RLC, 'y');
				if (mouseX >= x0 && mouseX <= x1 && mouseY >= Y0 && mouseY < y1)
				{
					if ((GetTickCount() / 500) & 1)ShowString(x0, y0 + (i - DYL) * CLY, cc3, &WhiteFont);
					else ShowString(x0, y0 + (i - DYL) * CLY, cc3, &YellowFont);
					if (Lpressed)
					{
						strcpy(LASTCLICKCHATNAME, ChatSender[i]);
						char* ss = strstr(LASTCLICKCHATNAME, "-->");
						if (ss)
						{
							strcpy(LASTCLICKCHATNAME, ss + 3);
						}
						ss = strstr(LASTCLICKCHATNAME, GetTextByID("ONLYOU"));
						if (ss)
						{
							ss[0] = 0;
						}
					}
				}
				else
				{
					ShowString(x0, y0 + (i - DYL) * CLY, cc3, &WhiteFont);
				}
				ShowString(x1, y0 + (i - DYL) * CLY, ChatMess[i], &YellowFont);
			}
			else
			{
				ShowString(x0 + 32, y0 + (i - DYL) * CLY, ChatMess[i], &YellowFont);
			}
		}
	}
	return 0;
};

ChatViewer* DialogsSystem::addChatViewer(SimpleDialog* Parent, int x, int y, int Ny, int OneLy, int OneLx,
                                         char*** Mess, char*** Name, int* Nchats)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ChatViewer* TV = new ChatViewer;
		DSS[i] = TV;
		TV->NeedToDraw = true;
		TV->Parent = Parent;
		TV->x = x + BaseX;
		TV->y = y + BaseY;
		TV->x1 = TV->x + OneLx - 1;
		TV->y1 = TV->y + OneLy * Ny - 1;
		TV->MaxLx = OneLx;
		TV->OnDraw = &ChatViewer_OnDraw;
		TV->ChatDY = 0;
		TV->ScrNy = Ny;
		TV->OneLy = OneLy;
		TV->Mess = Mess;
		TV->Names = Name;
		TV->NChats = Nchats;
		TV->Visible = 1;
		return TV;
	};
	return nullptr;
};
//-----------------Bit Pictures Viewer-------------
bool BPXView_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)
	{
		return false;
	}
	BPXView* BV = (BPXView*)SD;
	if (BV->VSC)
	{
		BV->PosY = BV->VSC->SPos;
	}
	if (BV->PosY > BV->RealNy - BV->Ny)BV->PosY = BV->RealNy - BV->Ny;
	if (BV->PosY < 0)BV->PosY = 0;
	int ps = int(BV->Ptr) + BV->PosY * BV->OneLx * BV->Nx * BV->OneLy;
	int sofst = int(ScreenPtr) + BV->x + BV->y * SCRSizeX;
	int Ly = BV->Ny * BV->OneLy;
	int Lx = (BV->Nx * BV->OneLx) >> 2;
	if (BV->DECLX)
	{
		for (int iy = 0; iy < BV->Ny; iy++)
		{
			for (int ix = 0; ix < BV->Nx; ix++)
			{
				Lx = BV->OneLx >> 2;
				Ly = BV->OneLy;
				int addof = SCRSizeX - (Lx << 2);
				ps = int(BV->Ptr) + (BV->PosY + iy) * (BV->OneLx + BV->DECLX) * BV->Nx * (BV->OneLy + BV->DECLX) + ix * (BV->OneLx +
					BV->DECLX);
				int addB = (BV->DECLX + BV->OneLx) * BV->Nx - BV->OneLx;
				int sofst = int(ScreenPtr) + BV->x + BV->OneLx * ix + (BV->y + BV->OneLy * iy) * SCRSizeX;
				__asm
				{
					push esi
					push edi
					pushf
					cld
					mov esi, ps
					mov edi, sofst
					mov edx, Ly
					ppp1 : mov ecx, Lx
					rep movsd
					add edi, addof
					add esi, addB
					dec edx
					jnz ppp1
					popf
					pop edi
					pop esi

				}
			}
		}
	}
	else
	{
		int addof = SCRSizeX - (Lx << 2);
		__asm
		{
			push esi
			push edi
			pushf
			cld
			mov esi, ps
			mov edi, sofst
			mov edx, Ly
			ppp : mov ecx, Lx
			rep movsd
			add edi, addof
			dec edx
			jnz ppp
			popf
			pop edi
			pop esi
		}
	}
	bool SSTAT = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	for (int ix = 0; ix < BV->Nx; ix++)
	{
		for (int iy = 0; iy < BV->Ny; iy++)
		{
			if (BV->Choosed[ix + (iy + BV->PosY) * BV->Nx])
			{
				Xbar(BV->x + ix * BV->OneLx, BV->y + iy * BV->OneLy, BV->OneLx - 1, BV->OneLy - 1, (byte)254);
				char ccc[16];
				if (BV->Nx > 1)
				{
					sprintf(ccc, "%d", BV->Choosed[ix + (iy + BV->PosY) * BV->Nx]);
					ShowString(BV->x + ix * BV->OneLx + 4, BV->y + iy * BV->OneLy + 4, ccc, &fn10);
				}
			}
			if (SSTAT)
			{
				char cc[32];
				if (BV->Nx > 1)
				{
					sprintf(cc, "%d", ix + (iy + BV->PosY) * BV->Nx);
					ShowString(BV->x + ix * BV->OneLx + 4, BV->y + (iy + 1) * BV->OneLy - 16, cc, &fn10);
				}
			}
		}
	}
	return true;
}

bool BPXView_OnKeyDown(SimpleDialog* SD)
{
	return false;
}

bool BPXView_OnMouseOver(SimpleDialog* SD)
{
	BPXView* BV = (BPXView*)SD;
	if (!BV->EnableSelection)return false;
	if (Lpressed || Rpressed)
	{
		int bx = div( mouseX - BV->x, BV->OneLx ).quot;
		int by = div( mouseY - BV->y, BV->OneLy ).quot;
		BV->ChoosedPos = bx + (by + BV->PosY) * BV->Nx;
		if (!(GetKeyState(VK_CONTROL) & 0x8000))
		{
			if (Lpressed)
			{
				memset(BV->Choosed, 0, BV->Nx * BV->RealNy);
				BV->Choosed[BV->ChoosedPos] = 1;
			};
		}
		else
		{
			if (Lpressed)
			{
				BV->Choosed[BV->ChoosedPos]++;
				BV->ChoosedPos = -1;
			}
			else
			{
				if (BV->Choosed[BV->ChoosedPos])
					BV->Choosed[BV->ChoosedPos]--;
				BV->ChoosedPos = -1;
			};
		};
		Lpressed = false;
		return true;
	};
	return false;
};

bool BPXView_OnDestroy(SimpleDialog* SD)
{
	BPXView* BV = (BPXView*)SD;
	free(BV->Choosed);
	return true;
};

BPXView* DialogsSystem::addBPXView(
	SimpleDialog* Parent,
	int x, int y, int OneLx, int OneLy, int Nx, int Ny, int RealNy,
	byte* Ptr, VScrollBar* VSC
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		BPXView* BV = new BPXView;
		DSS[i] = BV;
		BV->NeedToDraw = true;
		BV->Parent = Parent;
		BV->x = x + BaseX;
		BV->y = y + BaseY;
		BV->x1 = BV->x + OneLx * Nx - 1;
		BV->y1 = BV->y + OneLy * Ny - 1;
		BV->OnDraw = &BPXView_OnDraw;
		BV->OnKeyDown = &BPXView_OnKeyDown;
		BV->OnMouseOver = &BPXView_OnMouseOver;
		BV->Destroy = &BPXView_OnDestroy;
		BV->Enabled = true;
		BV->Active = Nx > 1;
		BV->OneLx = OneLx;
		BV->OneLy = OneLy;
		BV->Nx = Nx;
		BV->Ny = Ny;
		BV->Choosed = new byte[Nx * RealNy];
		memset(BV->Choosed, 0, Nx * RealNy);
		BV->Done = false;
		BV->RealNy = RealNy;
		BV->PosY = 0;
		BV->ChoosedPos = -1;
		BV->Ptr = Ptr;
		BV->VSC = VSC;
		BV->EnableSelection = Nx > 1;
		BV->DECLX = 0;
		if (VSC)
		{
			VSC->SPos = 0;
			VSC->SMaxPos = RealNy - Ny;
		};
		return BV;
	};
	return nullptr;
};
//---------------end of BPXView--------------//
//-----------------RLCListBox----------------//
bool RLCListBox_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	RLCListBox* RB = (RLCListBox*)SD;
	int WX = WindX;
	int WY = WindX;
	int WLX = WindLx;
	int WLY = WindLy;
	SetRLCWindow(RB->x, RB->y, RB->x1 - RB->x + 1, RB->y1 - RB->y + 1, ScrWidth);
	int curx = RB->x - RB->XPos;
	CBar(RB->x, RB->y, RB->x1 - RB->x + 1, RB->y1 - RB->y + 1, RB->BackColor);
	for (int i = 0; i < RB->NItems; i++)
	{
		int Lx = GPS.GetGPWidth(RB->GPIndex, i);
		if (curx <= RB->x1 && curx + Lx >= RB->x)
		{
			//ShowRLCItem(curx,RB->y,&RB->Items,i,0);
			GPS.ShowGP(curx, RB->y, RB->GPIndex, i, 0);
			if (RB->Choosed[i])
				Xbar(curx, RB->y, Lx - 1, RB->y1 - RB->y + 1, RB->SelColor);
		};
		curx += Lx;
	};
	//restoring the window
	SetRLCWindow(WX, WY, WLX, WLY, ScrWidth);
	return true;
};

bool RLCListBox_OnKeyDown(SimpleDialog* SD)
{
	RLCListBox* RB = (RLCListBox*)SD;
	return false;
};

bool RLCListBox_OnMouseOver(SimpleDialog* SD)
{
	RLCListBox* RB = (RLCListBox*)SD;
	if (!RB->NItems)return false;
	if (Lpressed)
	{
		Lpressed = false;
		//let us determine position
		int pos = 0;
		int mox = mouseX - RB->x + RB->XPos;
		int Lx;
		int i = 0;
		do
		{
			Lx = GPS.GetGPWidth(RB->GPIndex, i); //GetRLCWidth(RB->Items,i);
			mox -= Lx;
			i++;
		}
		while (i < RB->NItems && mox > 0);
		if (mox < 0)
		{
			pos = i - 1;
			//item is choosed;
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				RB->Choosed[pos] ^= 1;
			}
			else
			{
				memset(RB->Choosed, 0, RB->NItems);
				RB->Choosed[pos] = 1;
				RB->Done = true;
			};
		};
	};
	return true;
};

bool RLCListBox_OnDestroy(SimpleDialog* SD)
{
	RLCListBox* RB = (RLCListBox*)SD;
	if (RB->Choosed)free(RB->Choosed);
	return true;
};

RLCListBox* DialogsSystem::addRLCListBox(SimpleDialog* Parent,
                                         int x, int y, int Lx, int Ly,
                                         int GPIndex, byte BGColor, byte SelColor)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		RLCListBox* RB = new RLCListBox;
		DSS[i] = RB;
		RB->NeedToDraw = true;
		RB->Parent = Parent;
		RB->x = x + BaseX;
		RB->y = y + BaseY;
		RB->x1 = RB->x + Lx;
		RB->y1 = RB->y + Ly;
		RB->OnDraw = &RLCListBox_OnDraw;
		RB->OnKeyDown = &RLCListBox_OnKeyDown;
		RB->OnMouseOver = &RLCListBox_OnMouseOver;
		RB->Destroy = &RLCListBox_OnDestroy;
		RB->Enabled = true;
		RB->Active = true;
		//RB->Items=Pictures;
		RB->GPIndex = GPIndex;
		RB->NItems = GPS.GPNFrames[GPIndex]; //Pictures->SCount&65535;
		RB->SelColor = SelColor;
		RB->BackColor = BGColor;
		RB->Done = false;
		//MaxPos calculating
		int MAXX = 0;
		for (int i = 0; i < RB->NItems; i++)
		{
			MAXX += GPS.GetGPWidth(RB->GPIndex, i);
			//GetRLCWidth(Pictures,i);
		};
		RB->MaxXpos = MAXX - Lx;
		RB->Choosed = new byte[RB->NItems];
		memset(RB->Choosed, 0, RB->NItems);
		RB->XPos = 0;
		return RB;
	};
	return nullptr;
};
//--------------end of RLCListBox------------//
//-------------------ComboBox----------------//
bool ComboBox_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	ComboBox* CB = (ComboBox*)SD;
	if (!CB->NLines)return false;
	int center = 0;
	int XI = (CB->x + CB->x1) >> 1;
	if (CB->GP_File != -1 && CB->BackColor)center = 1;
	if (CB->GP_File != -1)
	{
		if (CB->Enabled)
		{
			if (CB->DLX)
			{
				TempWindow TW;
				PushWindow(&TW);
				IntersectWindows(0, 0, XI, RealLy);
				GPS.ShowGP(CB->x, CB->y, CB->GP_File, CB->UpPart + 1 - (CB->MouseOver || CB->IsActive), 0);
				PopWindow(&TW);
				IntersectWindows(XI, 0, RealLx, RealLy);
				GPS.ShowGP(CB->x - CB->DLX, CB->y, CB->GP_File, CB->UpPart + 1 - (CB->MouseOver || CB->IsActive), 0);
				PopWindow(&TW);
			}
			else GPS.ShowGP(CB->x, CB->y, CB->GP_File, CB->UpPart + 1 - (CB->MouseOver || CB->IsActive), 0);
		};
	};
	if (CB->IsActive && CB->Enabled)
	{
		//CBar(CB->x,CB->y,CB->x1-CB->x+1,CB->y1-CB->y+1,CB->BackColor);
		if (CB->rulermode)
		{
			char ccc[32];
			sprintf(ccc, "%d->%d", CB->MinDeal + CB->CurLine, CB->MinDeal + CB->LightIndex);
			ShowString(CB->x + CB->OneDx + center * ((CB->x1 - CB->x - GetRLCStrWidth(ccc, CB->ActiveFont)) >> 1),
			           CB->y + CB->OneDy, ccc, CB->ActiveFont);
		}
		else ShowString(
			CB->x + CB->OneDx + center * ((CB->x1 - CB->x - GetRLCStrWidth(CB->Lines[CB->CurLine], CB->ActiveFont)) >> 1),
			CB->y + CB->OneDy, CB->Lines[CB->CurLine], CB->ActiveFont);
	}
	else
	{
		//CBar(CB->x,CB->y,CB->x1-CB->x+1,CB->y1-CB->y+1,CB->BackColor);
		if (CB->rulermode)
		{
			char ccc[32];
			sprintf(ccc, "%d", CB->MinDeal + CB->CurLine);
			ShowString(CB->x + CB->OneDx + center * ((CB->x1 - CB->x - GetRLCStrWidth(ccc, CB->PassiveFont)) >> 1),
			           CB->y + CB->OneDy, ccc, CB->PassiveFont);
		}
		else ShowString(
			CB->x + CB->OneDx + center * ((CB->x1 - CB->x - GetRLCStrWidth(CB->Lines[CB->CurLine], CB->PassiveFont)) >> 1),
			CB->y + CB->OneDy, CB->Lines[CB->CurLine], CB->PassiveFont);
	};
	return true;
};
extern short randoma[8192];
#define PTPERDEAL 2
#define STARTOTS 12

bool ComboBox_OnDrawActiveRuler(SimpleDialog* SD)
{
	ComboBox* CB = (ComboBox*)SD;
	if (!CB->NLines)
	{
		return false;
	}
	int center = 0;
	if (CB->GP_File != -1 && CB->BackColor)
	{
		center = 1;
	}
	int NeedPoints = CB->NLines * PTPERDEAL + STARTOTS * 2;
	int NeedLines = (NeedPoints / CB->OneLy) + 1;
	int realots = STARTOTS;
	int NLINES = NeedLines;
	if (CB->VS && NLINES > CB->MaxLY)
	{
		NLINES = CB->MaxLY;
	}
	int DealsOnScreen = (NLINES * CB->OneLy + 12 + 4) / PTPERDEAL;
	int MaxScroll = (CB->NLines - DealsOnScreen) + 3 * (realots / PTPERDEAL);
	int StartDeal = CB->MinDeal;
	if (CB->VS)
	{
		StartDeal = CB->YPos + CB->MinDeal - 20;
	}
	int FinDeal = CB->MinDeal + CB->YPos + (NLINES * CB->OneLy) / PTPERDEAL + 40;
	int MinDeal = CB->MinDeal;
	int MaxDeal = CB->MinDeal + CB->NLines;
	int XI = (CB->x + CB->x1) >> 1;
	if (CB->IsActive)
	{
		if (CB->DLX)
		{
			TempWindow TW;
			PushWindow(&TW);
			IntersectWindows(0, 0, XI, RealLy);
			GPS.ShowGP(CB->DropX, CB->DropY, CB->GP_File, CB->UpPart + 2, 0);
			PopWindow(&TW);
			IntersectWindows(XI, 0, RealLx, RealLy);
			GPS.ShowGP(CB->DropX - CB->DLX, CB->DropY, CB->GP_File, CB->UpPart + 2, 0);
			PopWindow(&TW);
		}
		else
		{
			GPS.ShowGP(CB->DropX, CB->DropY, CB->GP_File, CB->UpPart + 2, 0);
		}
		int y0 = CB->DropY + CB->UpLy;
		for (int i = 0; i < NLINES; i++)
		{
			if (CB->DLX)
			{
				TempWindow TW;
				PushWindow(&TW);
				IntersectWindows(0, 0, XI, RealLy);
				GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 5, 0);
				PopWindow(&TW);
				IntersectWindows(XI, 0, RealLx, RealLy);
				GPS.ShowGP(CB->DropX - CB->DLX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 5, 0);
				PopWindow(&TW);
			}
			else
			{
				GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 5, 0);
			}
			y0 += CB->OneLy;
		}
		if (CB->DLX)
		{
			TempWindow TW;
			PushWindow(&TW);
			IntersectWindows(0, 0, XI, RealLy);
			GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + 7, 0);
			PopWindow(&TW);
			IntersectWindows(XI, 0, RealLx, RealLy);
			GPS.ShowGP(CB->DropX - CB->DLX, y0, CB->GP_File, CB->UpPart + 7, 0);
			PopWindow(&TW);
		}
		else
		{
			GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + 7, 0);
		}
		TempWindow TW;
		PushWindow(&TW);
		IntersectWindows(CB->DropX, CB->DropY + CB->UpLy - 4, CB->DropX + 164, CB->DropY + CB->OneLy * CB->MaxLY + 12);
		int x0 = CB->DropX + 10;
		char cc[128];
		for (int i = StartDeal; i < FinDeal; i++)
		{
			if (i >= MinDeal && i <= MaxDeal)
			{
				int yy = (i - CB->YPos - CB->MinDeal) * PTPERDEAL + CB->DropY + CB->UpLy + realots;
				if ((i % 50) == 0)
				{
					xLine(x0, yy, x0 + 32, yy, 0x4C);
					sprintf(cc, "%d", i);
					ShowString(x0 + 40, yy - 10, cc, &YellowFont);
				}
				else
				{
					if ((i % 10) == 0)
					{
						xLine(x0, yy, x0 + 24, yy, 0x4C);
						sprintf(cc, "%d", i);
						ShowString(x0 + 40, yy - 10, cc, &YellowFont);
					}
					else
					{
						xLine(x0, yy, x0 + 20, yy, 0x4C);
					}
					if (i == CB->LightIndex + CB->MinDeal)
					{
						xLine(x0, yy, x0 + 32, yy, 0xFF);
					}
				}
			}
		}

		PopWindow(&TW);
		if (CB->VS && NeedLines > CB->MaxLY)
		{
			CB->VS->SMaxPos = MaxScroll;
			CB->VS->Visible = 1;
			CB->VS->Enabled = 1;
			CB->VS->OnDraw(CB->VS);
			CB->VS->Visible = 0;
			CB->VS->Enabled = 0;
			CB->YPos = CB->VS->SPos;
			if (CB->YPos > CB->VS->SMaxPos)CB->YPos = CB->VS->SMaxPos;
		};
	};
	return true;
};

bool ComboBox_MouseOverActiveRuler(SimpleDialog* SD, int x, int y, int ID)
{
	if (!SD->Enabled)return false;
	ComboBox* CB = (ComboBox*)SD;
	//---------------
	int NeedPoints = CB->NLines * PTPERDEAL + STARTOTS * 2;
	int NeedLines = (NeedPoints / CB->OneLy) + 1;
	int realots = STARTOTS;
	int NLINES = NeedLines;
	if (CB->VS && NLINES > CB->MaxLY)NLINES = CB->MaxLY;
	int DealsOnScreen = (NLINES * CB->OneLy + 12 + 4) / PTPERDEAL;
	int MaxScroll = (CB->NLines - DealsOnScreen) + 3 * (realots / PTPERDEAL);
	int StartDeal = CB->MinDeal;
	if (CB->VS)
	{
		StartDeal = CB->YPos + CB->MinDeal - 20;
	};
	int FinDeal = CB->MinDeal + CB->YPos + (NLINES * CB->OneLy) / PTPERDEAL + 40;
	int MinDeal = CB->MinDeal;
	int MaxDeal = CB->MinDeal + CB->NLines;
	//---------------

	if (!CB->NLines)return false;
	if (y < CB->OneDy)return false;
	bool DOIT = 1;
	if (CB->VS && CB->NLines > CB->MaxLY)
	{
		if (mouseX >= CB->VS->x && mouseY >= CB->VS->y && mouseX < CB->VS->x1 && mouseY <= CB->VS->y1)
		{
			//CB->VS->OnMouseOver(CB->VS);
			CB->YPos = CB->VS->SPos;
			if (CB->YPos > CB->VS->SMaxPos)CB->YPos = CB->VS->SMaxPos;
			DOIT = 0;
		};
	};
	if (DOIT)
	{
		int y0 = CB->UpLy - 4;
		int y00 = CB->UpLy + realots;

		if (y >= y0)CB->LightIndex = (y - y00) / PTPERDEAL;
		if (CB->VS)CB->LightIndex += CB->YPos;
		if (CB->LightIndex > CB->NLines)CB->LightIndex = CB->NLines;
		if (CB->LightIndex < 0)CB->LightIndex = 0;
		if (Lpressed)
		{
			if (DOIT)
			{
				CB->CurLine = CB->LightIndex;
				CB->NeedRedrawAll = true;
				CB->IsActive = false;
				DialogsSystem* DIS = CB->ParentDS;
				DIS->ActiveParent = nullptr;
				Lpressed = false;
			};
		};
	};
	return true;
};

bool ComboBox_OnDrawActive(SimpleDialog* SD)
{
	ComboBox* CB = (ComboBox*)SD;
	if (CB->rulermode)return ComboBox_OnDrawActiveRuler(SD);
	if (!CB->NLines)return false;
	int center = 0;
	if (CB->GP_File != -1 && CB->BackColor)center = 1;
	int FL = 0;
	int LL = CB->NLines;
	if (CB->VS)
	{
		FL = CB->YPos;
		LL = FL + CB->MaxLY;
		if (LL > CB->NLines)LL = CB->NLines;
	};
	int XI = (CB->x + CB->x1) >> 1;
	if (CB->IsActive)
	{
		if (CB->GP_File == -1)
		{
			CBar(CB->DropX, CB->DropY, CB->OneLx, CB->OneLy * CB->NLines + CB->OneDy * 2, CB->BackColor);
			Xbar(CB->DropX, CB->DropY, CB->OneLx - 1, CB->OneLy * CB->NLines + CB->OneDy * 2 - 1, CB->BorderColor);
		}
		else
		{
			if (CB->DLX)
			{
				TempWindow TW;
				PushWindow(&TW);
				IntersectWindows(0, 0, XI, RealLy);
				GPS.ShowGP(CB->DropX, CB->DropY, CB->GP_File, CB->UpPart + 2, 0);
				PopWindow(&TW);
				IntersectWindows(XI, 0, RealLx, RealLy);
				GPS.ShowGP(CB->DropX - CB->DLX, CB->DropY, CB->GP_File, CB->UpPart + 2, 0);
				PopWindow(&TW);
			}
			else GPS.ShowGP(CB->DropX, CB->DropY, CB->GP_File, CB->UpPart + 2, 0);
			int y0 = CB->DropY + CB->UpLy;
			for (int i = FL; i < LL; i++)
			{
				if (CB->DLX)
				{
					TempWindow TW;
					PushWindow(&TW);
					IntersectWindows(0, 0, XI, RealLy);
					GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 5, 0);
					PopWindow(&TW);
					IntersectWindows(XI, 0, RealLx, RealLy);
					GPS.ShowGP(CB->DropX - CB->DLX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 5, 0);
					PopWindow(&TW);
				}
				else GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 5, 0);
				y0 += CB->OneLy;
			};
			if (CB->DLX)
			{
				TempWindow TW;
				PushWindow(&TW);
				IntersectWindows(0, 0, XI, RealLy);
				GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + 7, 0);
				PopWindow(&TW);
				IntersectWindows(XI, 0, RealLx, RealLy);
				GPS.ShowGP(CB->DropX - CB->DLX, y0, CB->GP_File, CB->UpPart + 7, 0);
				PopWindow(&TW);
			}
			else GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + 7, 0);
		};
		int y0 = CB->DropY + CB->UpLy;
		for (int i = FL; i < LL; i++)
		{
			if (i == CB->LightIndex)
			{
				if (CB->GP_File != -1)
				{
					if (CB->DLX)
					{
						TempWindow TW;
						PushWindow(&TW);
						IntersectWindows(0, 0, XI, RealLy);
						GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 3, 0);
						PopWindow(&TW);
						IntersectWindows(XI, 0, RealLx, RealLy);
						GPS.ShowGP(CB->DropX - CB->DLX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 3, 0);
						PopWindow(&TW);
					}
					else GPS.ShowGP(CB->DropX, y0, CB->GP_File, CB->UpPart + (randoma[i * 17] & 1) + 3, 0);
				};
				ShowString(CB->x + CB->FontDx + center * ((CB->x1 - CB->x - GetRLCStrWidth(CB->Lines[i], CB->ActiveFont)) >> 1),
				           CB->DropY + CB->FontDy + (i - FL) * CB->OneLy + CB->UpLy, CB->Lines[i], CB->ActiveFont);
			}
			else ShowString(
				CB->x + CB->FontDx + center * ((CB->x1 - CB->x - GetRLCStrWidth(CB->Lines[i], CB->PassiveFont)) >> 1),
				CB->DropY + CB->FontDy + (i - FL) * CB->OneLy + CB->UpLy, CB->Lines[i], CB->PassiveFont);
			y0 += CB->OneLy;
		};
		if (CB->VS && CB->NLines > CB->MaxLY)
		{
			CB->VS->Visible = 1;
			CB->VS->Enabled = 1;
			CB->VS->OnDraw(CB->VS);
			CB->VS->Visible = 0;
			CB->VS->Enabled = 0;
			CB->YPos = (CB->VS->SPos + 5) / 10;
			if (CB->YPos + CB->MaxLY > CB->NLines)CB->YPos = CB->NLines - CB->MaxLY;
		};
	};
	return true;
};

bool ComboBox_OnMouseOver(SimpleDialog* SD)
{
	if (!(SD->Enabled && SD->Visible))return false;
	ComboBox* CB = (ComboBox*)SD;
	if (Lpressed && CB->NLines)
	{
		if (CB->IsActive)
		{
			CB->IsActive = false;
			CB->NeedRedrawAll = true;
			DialogsSystem* DIS = CB->ParentDS;
			DIS->ActiveParent = nullptr;
		}
		else
		{
			if (CB->NLines)
			{
				CB->IsActive = true;
				CB->DropX = CB->x + CB->Center;
				CB->DropY = CB->y1;
				CB->LightIndex = CB->CurLine;
				//if(CB->DropY+CB->NLines*CB->OneLy+CB->OneDy*2>740)
				//	CB->DropY-=CB->NLines*CB->OneLy+CB->OneDy*2-CB->y+CB->y1+6;
				DialogsSystem* DIS = CB->ParentDS;
				DIS->ActiveParent = SD;
				DIS->ActiveX = CB->DropX;
				DIS->ActiveY = CB->DropY;
				DIS->ActiveX1 = CB->DropX + CB->OneLx - 1;
				DIS->ActiveY1 = CB->DropY + CB->OneLy * CB->NLines + CB->FontDy + CB->UpLy;
				if (CB->VS)
				{
					DIS->ActiveX1 = CB->VS->x1 - 1;
					DIS->ActiveY1 = CB->VS->y1;
				};
			};
		};
		Lpressed = false;
	};
	return true;
};

bool ComboBox_MouseOverActiveZone(SimpleDialog* SD, int x, int y, int ID)
{
	if (!SD->Enabled)return false;
	ComboBox* CB = (ComboBox*)SD;
	if (CB->rulermode)return ComboBox_MouseOverActiveRuler(SD, x, y, ID);
	if (CB->VS)
	{
		if (CB->NLines > CB->MaxLY)
		{
			CB->VS->SMaxPos = (CB->NLines - CB->MaxLY) * 10;
			if (CB->VS->SPos > CB->VS->SMaxPos)CB->VS->SPos = CB->VS->SMaxPos;
			CB->YPos = (CB->VS->SPos + 5) / 10;
		};
	};

	if (!CB->NLines)return false;
	if (y < CB->OneDy)return false;
	bool DOIT = 1;
	if (CB->VS && CB->NLines > CB->MaxLY)
	{
		if (mouseX >= CB->VS->x && mouseY >= CB->VS->y && mouseX < CB->VS->x1 && mouseY <= CB->VS->y1)
		{
			//CB->VS->OnMouseOver(CB->VS);
			CB->YPos = (CB->VS->SPos + 5) / 10;
			if (CB->YPos + CB->MaxLY > CB->NLines)CB->YPos = CB->NLines - CB->MaxLY;
			DOIT = 0;
		};
	};
	if (DOIT)
	{
		int y0 = CB->OneDy + CB->FontDy + CB->UpLy;
		if (y >= y0)CB->LightIndex = div( y - y0, CB->OneLy ).quot;
		if (CB->VS)CB->LightIndex += CB->YPos;
		if (CB->LightIndex >= CB->NLines)CB->LightIndex = CB->NLines - 1;
		if (Lpressed)
		{
			if (DOIT)
			{
				CB->CurLine = CB->LightIndex;
				if (CB->CurLine < 0)CB->CurLine = 0;
				if (CB->CurLine >= CB->NLines && CB->NLines)CB->CurLine = CB->NLines - 1;
				CB->NeedRedrawAll = true;
				CB->IsActive = false;
				DialogsSystem* DIS = CB->ParentDS;
				DIS->ActiveParent = nullptr;
				Lpressed = false;
			};
		};
	};
	return true;
};

bool ComboBox_OnDestroy(SimpleDialog* SD)
{
	ComboBox* CB = (ComboBox*)SD;
	CB->Clear();
	return true;
};

void ComboBox::AssignScroll(DialogsSystem* DSS, VScrollBar** SCR, int GPFile, int Sprite, int MaxLy)
{
	*SCR = DSS->addNewGP_VScrollBar(nullptr, x1 - DSS->BaseX - GPS.GetGPWidth(GPFile, Sprite) + 1, y1 + 5 - DSS->BaseY,
	                                OneLy * MaxLy + 9, 1, 0, GPFile, Sprite);
	VS = *SCR;
	VS->Visible = 0;
	VS->Enabled = 0;
	VS->SPos = 0;
	VS->SMaxPos = 1;
	YPos = 0;
	MaxLY = MaxLy;
};

void ComboBox::CreateRuler(int Min, int NDeals)
{
	Clear();
	NLines = NDeals;
	MinDeal = Min;
	rulermode = 1;
};

ComboBox* DialogsSystem::addComboBox(SimpleDialog* Parent, int x, int y, int Lx, int Ly, int LineLy,
                                     byte BackColor, byte BorderColor,
                                     RLCFont* ActiveFont, RLCFont* PassiveFont,
                                     char* Contence)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ComboBox* CB = new ComboBox;
		CB->NeedToDraw = true;
		CB->Parent = Parent;
		CB->x = x + BaseX;
		CB->y = y + BaseY;
		CB->x1 = CB->x + Lx - 1;
		CB->y1 = CB->y + Ly - 1;
		CB->ActiveFont = ActiveFont;
		CB->PassiveFont = PassiveFont;
		CB->Enabled = true;
		CB->BackColor = BackColor;
		CB->BorderColor = BorderColor;
		CB->CurLine = 0;
		CB->FontDx = 2;
		CB->FontDy = 0;
		CB->OneDx = 1;
		CB->OneDy = 1;
		CB->OneLx = Lx;
		CB->OneLy = LineLy;
		CB->NLines = 0;
		CB->Lines = nullptr;
		CB->UpLy = 0;
		CB->LightIndex = -1;
		CB->UpLy = 0;
		CB->OnDraw = ComboBox_OnDraw;
		CB->OnDrawActive = ComboBox_OnDrawActive;
		CB->Destroy = ComboBox_OnDestroy;
		CB->OnMouseOver = ComboBox_OnMouseOver;
		CB->MouseOverActiveZone = ComboBox_MouseOverActiveZone;
		CB->GP_File = -1;
		DSS[i] = CB;
		CB->ParentDS = this;
		CB->Enabled = true;
		CB->Center = 0;
		if (Contence)CB->AddComplexLine(Contence);
		CB->rulermode = 0;
		CB->VS = nullptr;
		//assert(_CrtCheckMemory());
		return CB;
	}

	return nullptr;
}

ComboBox* DialogsSystem::addGP_ComboBox(SimpleDialog* Parent, int x, int y, int GP_File,
                                        int UpPart, int Center, int DownPart,
                                        RLCFont* ActiveFont, RLCFont* PassiveFont,
                                        char* Contence)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ComboBox* CB = new ComboBox;
		CB->NeedToDraw = true;
		CB->Parent = Parent;
		CB->x = x + BaseX;
		CB->y = y + BaseY;
		CB->OneLx = GPS.GetGPWidth(GP_File, UpPart);
		CB->OneLy = GPS.GetGPHeight(GP_File, UpPart + 5);
		int Lx = CB->OneLx;
		int Ly = GPS.GetGPHeight(GP_File, UpPart) + 1;
		CB->x1 = CB->x + Lx - 1;
		CB->y1 = CB->y + Ly - 1;
		CB->ActiveFont = ActiveFont;
		CB->PassiveFont = PassiveFont;
		CB->Enabled = true;
		CB->BackColor = 0;
		CB->BorderColor = 0;
		CB->CurLine = 0;
		CB->FontDx = 25;
		CB->FontDy = 2;
		CB->OneDx = 24;
		CB->OneDy = 3;
		CB->UpLy = GPS.GetGPHeight(GP_File, UpPart + 2);
		CB->GP_File = GP_File;
		CB->UpPart = UpPart;
		CB->Center = Center;
		CB->DownPart = DownPart;
		CB->NLines = 0;
		CB->Lines = nullptr;
		CB->LightIndex = -1;
		CB->OnDraw = ComboBox_OnDraw;
		CB->OnDrawActive = ComboBox_OnDrawActive;
		CB->Destroy = ComboBox_OnDestroy;
		CB->OnMouseOver = ComboBox_OnMouseOver;
		CB->MouseOverActiveZone = ComboBox_MouseOverActiveZone;
		DSS[i] = CB;
		CB->ParentDS = this;
		CB->Enabled = true;
		CB->VS = nullptr;
		CB->DLX = 0;
		if (Contence)CB->AddComplexLine(Contence);
		CB->rulermode = 0;
		return CB;
	}
	else
	{
		return nullptr;
	}
}

ComboBox* DialogsSystem::addGP_ComboBoxDLX(SimpleDialog* Parent, int x, int y, int LX, int GP_File,
                                           int UpPart, int Center, int DownPart,
                                           RLCFont* ActiveFont, RLCFont* PassiveFont,
                                           char* Contence)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		ComboBox* CB = new ComboBox;
		CB->NeedToDraw = true;
		CB->Parent = Parent;
		CB->x = x + BaseX;
		CB->y = y + BaseY;
		CB->OneLx = GPS.GetGPWidth(GP_File, UpPart);
		CB->DLX = CB->OneLx - LX;
		CB->OneLx = LX;
		CB->OneLy = GPS.GetGPHeight(GP_File, UpPart + 5);
		int Lx = CB->OneLx;
		int Ly = GPS.GetGPHeight(GP_File, UpPart) + 1;
		CB->x1 = CB->x + Lx - 1;
		CB->y1 = CB->y + Ly - 1;
		CB->ActiveFont = ActiveFont;
		CB->PassiveFont = PassiveFont;
		CB->Enabled = true;
		CB->BackColor = 0;
		CB->BorderColor = 0;
		CB->CurLine = 0;
		CB->FontDx = 25;
		CB->FontDy = 2;
		CB->OneDx = 24;
		CB->OneDy = 3;
		CB->UpLy = GPS.GetGPHeight(GP_File, UpPart + 2);
		CB->GP_File = GP_File;
		CB->UpPart = UpPart;
		CB->Center = Center;
		CB->DownPart = DownPart;
		CB->NLines = 0;
		CB->Lines = nullptr;
		CB->LightIndex = -1;
		CB->OnDraw = ComboBox_OnDraw;
		CB->OnDrawActive = ComboBox_OnDrawActive;
		CB->Destroy = ComboBox_OnDestroy;
		CB->OnMouseOver = ComboBox_OnMouseOver;
		CB->MouseOverActiveZone = ComboBox_MouseOverActiveZone;
		DSS[i] = CB;
		CB->ParentDS = this;
		CB->Enabled = true;
		CB->VS = nullptr;
		if (Contence)
		{
			CB->AddComplexLine(Contence);
		}
		CB->rulermode = 0;
		return CB;
	}
	else
	{
		return nullptr;
	}
}

void ComboBox::Clear()
{
	if (Lines)
	{
		for (int i = 0; i < NLines; i++)
			free(Lines[i]);
		free(Lines);
		Lines = nullptr;
		NLines = 0;
		CurLine = 0;
	}
	rulermode = 0;
}

void ComboBox::AddLine(char* Text)
{
	if (Lines)
	{
		Lines = (char**)realloc(Lines, (NLines + 1) << 2);
		Lines[NLines] = new char[strlen(Text) + 1];
		strcpy(Lines[NLines], Text);
		NLines++;
	}
	else
	{
		Lines = (char**)malloc(4);
		Lines[0] = new char[strlen(Text) + 1];
		strcpy(Lines[0], Text);
		NLines++;
	}
}

void ComboBox::AddComplexLine(char* Text)
{
	char* Tmp = new char[strlen(Text) + 1];
	strcpy(Tmp, Text);
	int pos = 0;
	int Nextpos = 0;
	bool All = false;
	do
	{
		while (Tmp[Nextpos] != '|' && Tmp[Nextpos] != 0)Nextpos++;
		if (Tmp[Nextpos])Tmp[Nextpos] = 0;
		else All = true;
		AddLine(Tmp + pos);
		pos = Nextpos + 1;
		Nextpos = pos;
	}
	while (!All);
	free(Tmp);
}

//---------------end of ComboBox-------------//
//-----------------WinComboBox---------------//
bool WinComboBox_OnDraw(SimpleDialog* SD)
{
	WinComboBox* CB = (WinComboBox*)SD;
	if (!CB->Visible)return false;
	if (!CB->NLines)return false;
	bool CanChoose = GetTickCount() - CB->LastChooseTime > 500;
	if (!CB->CurBox)
	{
		int MouseOvList = mouseX >= CB->ListX0 && mouseX <= CB->ListX1 && mouseY >= CB->ListY0 && mouseY <= CB->ListY1;
		if (!CB->Enabled)MouseOvList = false;
		int Cur = -1;
		int Ly = CB->y1 - CB->y;

		if (CB->Enabled)
		{
			if (CanChoose || MouseOvList)
			{
				for (int i = 0; i < CB->NBox; i++)
				{
					if (CB->Group[i]->MouseOver)Cur = i;
				};
			}
			else
			{
				for (int i = 0; i < CB->NBox; i++)
				{
					if (CB->Group[i]->ActiveLine != -1)
						Cur = i;
				};
			};
		};
		if (Cur == -1)
		{
			if (!MouseOvList)
			{
				if (CanChoose)for (int i = 0; i < CB->NBox; i++)CB->Group[i]->ActiveLine = -1;
			}
			else
			{
				for (int i = 0; i < CB->NBox; i++)if (CB->Group[i]->ActiveLine != -1)Cur = i;
				for (int i = 0; i < CB->NBox; i++)CB->Group[i]->ActiveLine = -1;
				if (Cur != -1)
				{
					int yy = (mouseY - CB->y) / Ly;
					CB->Group[Cur]->ActiveLine = yy;
					if (Lpressed && yy < CB->Group[Cur]->NLines)
					{
						CB->Group[Cur]->CurLine = yy;
						CB->Group[Cur]->ActiveLine = -1;
					};
				};
			};
		}
		else
		{
			if (CanChoose)
			{
				for (int i = 0; i < CB->NBox; i++)CB->Group[i]->ActiveLine = -1;
				CB->Group[Cur]->ActiveLine = CB->Group[Cur]->CurLine;
				CB->LastChooseTime = GetTickCount();
			};
		};
		for (int i = 0; i < CB->NBox; i++)
		{
			WinComboBox* WCB = CB->Group[i];
			if (WCB->MouseOver)
			{
				ShowString(WCB->x, WCB->y, WCB->Message, WCB->ActiveFont);
				if (Cur == -1)
				{
					ShowString(WCB->ListX0, WCB->y, WCB->Lines[WCB->CurLine], WCB->ActiveFont);
				};
			}
			else
			{
				if (WCB->ActiveLine != -1 && MouseOvList)ShowString(WCB->x, WCB->y, WCB->Message, WCB->ActiveFont);
				else ShowString(WCB->x, WCB->y, WCB->Message, WCB->PassiveFont);
				if (Cur == -1)
				{
					ShowString(WCB->ListX0, WCB->y, WCB->Lines[WCB->CurLine], WCB->ActiveFont);
				};
			};
			if (Cur == i)
			{
				for (int j = 0; j < WCB->NLines; j++)
				{
					if (j == WCB->ActiveLine)
					{
						ShowString(WCB->ListX0, WCB->ListY0 + j * Ly, WCB->Lines[j], WCB->ActiveFont);
					}
					else
					{
						ShowString(WCB->ListX0, WCB->ListY0 + j * Ly, WCB->Lines[j], WCB->PassiveFont);
					};
				};
			};
		};
	};
	return false;
}

bool WinComboBox_OnDestroy(SimpleDialog* SD)
{
	WinComboBox* CB = (WinComboBox*)SD;
	CB->Clear();
	return true;
}

WinComboBox* DialogsSystem::addWinComboBox(
	SimpleDialog* Parent, char* pMessage, int x, int y, int Lx, int Ly,
	int ListX, int ListY, int ListLx, int ListLy,
	RLCFont* ActiveFont, RLCFont* PassiveFont,
	WinComboBox** Group, int NInGroup, int CurBox
)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);

	if (i < MAXDLG)
	{
		WinComboBox* CB = new WinComboBox;
		CB->NeedToDraw = true;
		CB->Parent = Parent;
		CB->x = x + BaseX;
		CB->y = y + BaseY;
		CB->x1 = CB->x + Lx - 1;
		CB->y1 = CB->y + Ly - 1;
		CB->ListX0 = ListX;
		CB->ListY0 = ListY;
		CB->ListX1 = ListX + ListLx - 1;
		CB->ListY1 = ListY + ListLy - 1;
		CB->ActiveFont = ActiveFont;
		CB->PassiveFont = PassiveFont;
		CB->Group = Group;
		CB->NBox = NInGroup;
		CB->CurLine = 0;
		CB->ActiveLine = -1;
		CB->Lines = nullptr;
		CB->NLines = 0;
		CB->Message = pMessage;
		CB->CurBox = CurBox;
		CB->OnDraw = &WinComboBox_OnDraw;
		CB->Destroy = &WinComboBox_OnDestroy;
		CB->Active = false;
		CB->Enabled = true;
		CB->Visible = true;
		CB->LastChooseTime = 0;
		DSS[i] = CB;
		return CB;
	}
	return nullptr;
}

void WinComboBox::AddLine(char* Text)
{
	if (Lines)
	{
		Lines = (char**)realloc(Lines, (NLines + 1) << 2);
		Lines[NLines] = new char[strlen(Text) + 1];
		strcpy(Lines[NLines], Text);
		NLines++;
	}
	else
	{
		Lines = (char**)malloc(4);
		Lines[0] = new char[strlen(Text) + 1];
		strcpy(Lines[0], Text);
		NLines++;
	}
}

void WinComboBox::Clear()
{
	if (Lines)
	{
		for (int i = 0; i < NLines; i++)free(Lines[i]);
		free(Lines);
		Lines = nullptr;
		NLines = 0;
		CurLine = 0;
	}
}

//--------------end of WinComboBox-----------//

//-----------------PageControl---------------//
bool GP_PageControl_OnDraw(SimpleDialog* SD)
{
	GP_PageControl* PC = (GP_PageControl*)SD;
	if (PC->MouseOver && Lpressed)
	{
		for (int i = 0; i < PC->NPages; i++)
		{
			if (mouseX >= PC->Pages[i].x
				&& mouseX <= PC->Pages[i].x1
				&& mouseY >= PC->Pages[i].y
				&& mouseY <= PC->Pages[i].y1)
			{
				PC->CurPage = i;
			}
		}
	}
	GPS.ShowGP(PC->x, PC->y, PC->GP_File, PC->Pages[PC->CurPage].Index, 0);
	return false;
}

GP_PageControl* DialogsSystem::addPageControl(SimpleDialog* Parent, int x, int y, int GP_File, int FirstIndex)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++)
	{
	}

	if (i < MAXDLG)
	{
		GP_PageControl* PC = new GP_PageControl;
		PC->NeedToDraw = true;
		PC->Parent = Parent;
		PC->x = x + BaseX;
		PC->y = y + BaseY;
		PC->x1 = PC->x + GPS.GetGPWidth(GP_File, FirstIndex);
		PC->y1 = PC->y + GPS.GetGPHeight(GP_File, FirstIndex);
		PC->GP_File = GP_File;
		PC->CurPage = 0;
		PC->NPages = 0;
		PC->Pages = nullptr;
		PC->Active = false;
		PC->OnDraw = &GP_PageControl_OnDraw;
		PC->Enabled = true;
		PC->Visible = true;
		DSS[i] = PC;
		return PC;
	}

	return nullptr;
}

void GP_PageControl::AddPage(int x0, int y0, int px1, int py1, int Index)
{
	Pages = (OnePage*)realloc(Pages, (NPages + 1) * sizeof OnePage);
	Pages[NPages].Index = Index;
	Pages[NPages].x = x0;
	Pages[NPages].y = y0;
	Pages[NPages].x1 = px1;
	Pages[NPages].y1 = py1;
	NPages++;
}

//-------------end of PageControl------------//
extern int BordGP2;
void DrawHdrTable(int x0, int y0, int x1, int y1);
void DrawScrollDoubleTableF(int x0, int y0, int x1, int y1, int ym);

bool BorderEx_OnDraw(SimpleDialog* SD)
{
	BorderEx* BR = (BorderEx*)SD;
	switch (BR->Style)
	{
	case 0: //simple
		DrawStdBar(BR->x, BR->y, BR->x1, BR->y1);
		break;
	case 1: //simple with header
		DrawHdrTable(BR->x, BR->y, BR->x1, BR->y1);
		break;
	case 2: //double with scroller in bottom
		DrawScrollDoubleTable(BR->x, BR->y, BR->x1, BR->y1, BR->ymid);
		break;
	case 3: //double with scroller in bottom(filled)
		DrawScrollDoubleTableF(BR->x, BR->y, BR->x1, BR->y1, BR->ymid);
		break;
	case 4: //simple with header and scroller
		DrawHdrTable(BR->x, BR->y, BR->x1, BR->y1);
		break;
	case 5:
		DrawStdRect2(BR->x, BR->y, BR->x1, BR->y1, BordGP2);
		break;
	};
	return true;
};

BorderEx* DialogsSystem::addBorder(int x, int y, int x1, int y1, int Ymid, byte Style)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		BorderEx* BR = new BorderEx;
		DSS[i] = BR;
		BR->x = x + BaseX;
		BR->y = y + BaseY;
		BR->x1 = x1 + BaseX;
		BR->y1 = y1 + BaseY;
		BR->ymid = Ymid + BaseY;
		BR->Style = Style;
		BR->Enabled = 1;
		BR->Visible = 1;
		BR->OnDraw = &BorderEx_OnDraw;
		BR->Active = 0;
		return BR;
	}
	else return nullptr;
};

bool CustomBorder_OnDraw(SimpleDialog* SD)
{
	if (!SD->Visible)return false;
	CustomBorder* BR = (CustomBorder*)SD;
	Corners CR;
	int gp = BR->GP;
	int FIL = -1;
	if (BR->MouseOver && BR->Enabled)
	{
		memcpy(&CR, BR->BOR_A, 32);
		FIL = BR->FILL_A;
	}
	else
	{
		memcpy(&CR, BR->BOR_N, 32);
		FIL = BR->FILL_N;
	};
	if (gp != -1)
	{
		if (FIL == -1)
		{
			DrawRect3(BR->x, BR->y, BR->x1, BR->y1, &CR, gp);
		}
		else
		{
			DrawFilledRect3(BR->x, BR->y, BR->x1, BR->y1, &CR, gp, FIL, 3);
		};
	};
	return true;
}

CustomBorder* DialogsSystem::addCustomBorder(int x, int y, int x1, int y1,
                                             int gp, int* bn, int* ba,
                                             int fill_n, int fill_a)
{
	int i;
	for (i = 0; i < MAXDLG && DSS[i] != nullptr; i++);
	if (i < MAXDLG)
	{
		CustomBorder* BR = new CustomBorder;
		DSS[i] = BR;
		BR->x = x + BaseX;
		BR->y = y + BaseY;
		BR->x1 = x1 + BaseX;
		BR->y1 = y1 + BaseY;
		BR->Enabled = 1;
		BR->Visible = 1;
		BR->OnDraw = &CustomBorder_OnDraw;
		memcpy(BR->BOR_N, bn, 32);
		memcpy(BR->BOR_A, ba, 32);
		BR->FILL_N = fill_n;
		BR->FILL_A = fill_a;
		BR->GP = gp;
		BR->Active = 0;
		return BR;
	}
	else return nullptr;
}

//-------------------------------------------//
SimpleDialog::SimpleDialog()
{
	Enabled = false;
	x = 0;
	y = 0;
	x1 = 0;
	y1 = 0;
	UserParam = 0;
	OnClick = nullptr;
	OnUserClick = nullptr;
	OnDraw = nullptr;
	OnActivate = nullptr;
	OnKeyDown = nullptr;
	OnLeave = nullptr;
	OnMouseOver = nullptr;
	MouseOverActive = nullptr;
	Destroy = nullptr;
	Refresh = nullptr;
	Parent = nullptr;
	Child = nullptr;
	OnDrawActive = nullptr;
	OnNewClick = false;
	MouseOver = false;
	MouseOverActive = false;
	Active = false;
	IsActive = false;
	Visible = true;
	Enabled = true;
	NeedRedrawAll = false;
	Hint = nullptr;
	MouseSound = -1;
	ClickSound = -1;
	MouseOver = 0;
	ParentSB = nullptr;
	AllocPtr = nullptr;
}

void SimpleDialog::AssignSound(int ID, int Usage)
{
	switch (Usage)
	{
	case MOUSE_SOUND:
		MouseSound = ID;
		break;
	case CLICK_SOUND:
		ClickSound = ID;
		break;
	};
};

void SimpleDialog::AssignSound(char* Name, int Usage)
{
	int ID = SearchStr(SoundID, Name, NSounds);
	if (ID == -1)
	{
		char cc[128];
		sprintf(cc, "Unknown sound ID: %s", Name);
		ErrM(cc);
		return;
	};
	AssignSound(ID, Usage);
};

//----copy rectangle to screen----//
void CopyToScreen(int zx, int zy, int zLx, int zLy)
{
	if (!bActive)
	{
		return;
	}

	int x = zx;
	int y = zy;
	int Lx = zLx;
	int Ly = zLy;
	if (x < 0)
	{
		Lx += x;
		x = 0;
	}
	if (y < 0)
	{
		Ly += y;
		y = 0;
	}

	if (x + Lx > RealLx)
		Lx = RealLx - x;

	if (y + Ly > RealLy)
		Ly = RealLy - y;

	if (Lx < 0 || Ly < 0)
		return;

	int scof = int(ScreenPtr) + x + y * SCRSizeX;
	int reof = int(RealScreenPtr) + x + y * RSCRSizeX;
	int sadd = SCRSizeX - Lx;
	int radd = RSCRSizeX - Lx;
	int Lx4 = Lx >> 2;
	int Lx1 = Lx & 3;
	__asm
	{
		push esi
		push edi
		mov edx, Ly
		or edx, edx
		jz lpp4
		mov esi, scof
		mov edi, reof
		cld
		lpp1 : mov ecx, Lx4
		jcxz lpp2
		//		cli
		rep movsd
		lpp2 : mov ecx, Lx1
		jcxz lpp3
		rep movsb
		lpp3 : //sti
		add esi, sadd
		add edi, radd
		dec edx
		jnz lpp1
		lpp4 : pop edi
		pop esi
	}
}

void CopyToOffScreen(int zx, int zy,
                     int srLx, int srLy,
                     byte* data)
{
	if (!bActive)return;
	int x = zx;
	int y = zy;
	int Lx = srLx;
	int Ly = srLy;
	//if(zLx<=0||zLy<=0||zx<0||zy<0||zx+zLx>RealLx||zy+zLy>RealLy)return;
	//if(zLx<=0||zLy<=0||zx<0||zy<0)return;
	if (x < 0)
	{
		Lx += x;
		x = 0;
	};
	if (y < 0)
	{
		Ly += y;
		y = 0;
	};
	if (x + Lx > RealLx)Lx = RealLx - x;
	if (y + Ly > RealLy)Ly = RealLy - y;
	if (Lx < 0 || Ly < 0)
		return;
	int scof = int(data);
	int reof = int(ScreenPtr) + x + y * SCRSizeX;
	int sadd = srLx - Lx;
	int radd = SCRSizeX - Lx;
	int Lx4 = Lx >> 2;
	int Lx1 = Lx & 3;
	__asm {
		push esi
		push edi
		mov edx, Ly
		or edx, edx
		jz lpp4
		mov esi, scof
		mov edi, reof
		cld
		lpp1 : mov ecx, Lx4
		jcxz lpp2
		//		cli
		rep movsd
		lpp2 : mov ecx, Lx1
		jcxz lpp3
		rep movsb
		lpp3 : //sti
		add esi, sadd
		add edi, radd
		dec edx
		jnz lpp1
		lpp4 : pop edi
		pop esi
	};
};
void CopyToRealScreenMMX(int zx, int zy,
                         int srLx, int srLy,
                         byte* data);

void CopyToRealScreen(int zx, int zy,
                      int srLx, int srLy,
                      byte* data)
{
	if (!bActive)return;
	//CopyToRealScreenMMX(zx,zy,srLx,srLy,data);
	//return;
	int x = zx;
	int y = zy;
	int Lx = srLx;
	int Ly = srLy;
	//if(zLx<=0||zLy<=0||zx<0||zy<0||zx+zLx>RealLx||zy+zLy>RealLy)return;
	//if(zLx<=0||zLy<=0||zx<0||zy<0)return;
	if (x < 0)
	{
		Lx += x;
		x = 0;
	};
	if (y < 0)
	{
		Ly += y;
		y = 0;
	};
	if (x + Lx > RealLx)Lx = RealLx - x;
	if (y + Ly > RealLy)Ly = RealLy - y;
	if (Lx < 0 || Ly < 0)
		return;
	int scof = int(data);
	int reof = int(RealScreenPtr) + x + y * RSCRSizeX;
	int sadd = srLx - Lx;
	int radd = RSCRSizeX - Lx;
	int Lx4 = Lx >> 2;
	int Lx1 = Lx & 3;
	__asm {
		push esi
		push edi
		mov edx, Ly
		or edx, edx
		jz lpp4
		mov esi, scof
		mov edi, reof
		cld
		lpp1 : mov ecx, Lx4
		jcxz lpp2
		//		cli
		rep movsd
		lpp2 : mov ecx, Lx1
		jcxz lpp3
		rep movsb
		lpp3 : //sti
		add esi, sadd
		add edi, radd
		dec edx
		jnz lpp1
		lpp4 : pop edi
		pop esi
	};
};

void CopyToRealScreenMMX(int zx, int zy,
                         int srLx, int srLy,
                         byte* data)
{
	int x = zx;
	int y = zy;
	int Lx = srLx;
	int Ly = srLy;
	//if(zLx<=0||zLy<=0||zx<0||zy<0||zx+zLx>RealLx||zy+zLy>RealLy)return;
	//if(zLx<=0||zLy<=0||zx<0||zy<0)return;
	if (x < 0)
	{
		Lx += x;
		x = 0;
	};
	if (y < 0)
	{
		Ly += y;
		y = 0;
	};
	if (x + Lx > RealLx)Lx = RealLx - x;
	if (y + Ly > RealLy)Ly = RealLy - y;
	if (Lx < 0 || Ly < 0)
		return;
	int scof = int(data);
	int reof = int(RealScreenPtr) + x + y * RSCRSizeX;
	int sadd = srLx - Lx;
	int radd = RSCRSizeX - Lx;
	int Lx4 = Lx >> 3;
	int Lx1 = Lx & 7;
	__asm {
		push esi
		push edi
		mov edx, Ly
		or edx, edx
		jz lpp4
		mov esi, scof
		mov edi, reof
		cld
		lpp1 : mov ecx, Lx4
		jcxz lpp2
		//		cli
		lppm1 : movq mm0, [esi]
		add esi, 8
		movq[edi], mm0
		add edi, 8
		dec ecx
		jnz lppm1
		lpp2 : mov ecx, Lx1
		jcxz lpp3
		rep movsb
		lpp3 : //sti
		add esi, sadd
		add edi, radd
		dec edx
		jnz lpp1
		lpp4 : pop edi
		pop esi
		emms
	};
};
//--------Pictures methods--------//
void SQPicture::Draw(int x, int y)
{
	if (!bActive)return;
	if (!this)return;
	if (!PicPtr)return;
	int sofst = int(ScreenPtr) + x + y * ScrWidth; //SizeX;
	int pofst = int(PicPtr + 2);
	int lx = int(PicPtr[0]);
	int ly = int(PicPtr[1]);
	if (ly <= 0 || lx <= 0 || lx > RealLx || ly > RealLy)return;
	int lx4 = lx >> 2;
	int lx1 = lx & 3;
	int scradd = ScrWidth - lx; //SizeX-lx;
	__asm {
		push esi
		push edi
		mov esi, pofst
		mov edi, sofst
		mov edx, ly
		or edx, edx
		jz uu3
		cld
		lpp1 : mov ecx, lx4
		jcxz uu1
		rep movsd
		uu1 : mov ecx, lx1
		jcxz uu2
		rep movsb
		uu2 : add edi, scradd
		dec edx
		jnz lpp1
		uu3 : pop edi
		pop esi
	};
};

void SQPicture::DrawTransparent(int x, int y)
{
	if (!PicPtr)return;
	int sofst = int(ScreenPtr) + x + y * SCRSizeX;
	int pofst = int(PicPtr + 2);
	int lx = int(PicPtr[0]);
	int ly = int(PicPtr[1]);
	if (ly <= 0 || lx <= 0 || lx > RealLx || ly > RealLy)return;
	int lx4 = lx >> 2;
	int lx1 = lx & 3;
	int scradd = SCRSizeX - lx;
	__asm {
		push esi
		push edi
		mov esi, pofst
		mov edi, sofst
		mov edx, ly
		or edx, edx
		jz uu3
		cld
		lpp0 : mov ecx, lx
		lpp1 : lodsb
		or al, al
		jnz lpp1x
		//inc		esi
		inc edi
		dec ecx
		jnz lpp1
		jmp uu2
		lpp1x : stosb
		dec ecx
		jnz lpp1
		uu2 : add edi, scradd
		dec edx
		jnz lpp0
		uu3 : pop edi
		pop esi
	};
};
bool SafeLoad = 0;

void SQPicture::LoadPicture(char* name)
{
	if (this->PicPtr)
	{
		free(PicPtr);
		PicPtr = nullptr;
	};

	if (strstr(name, ".bmp") || strstr(name, ".BMP"))
	{
		if (!ReadBMP8TOBPX(name, (byte**)(&this->PicPtr)))
		{
			PicPtr = nullptr;
			if (!SafeLoad)
			{
				char gg[128];
				sprintf(gg, "Could not load picture: %s", name);
				ErrD(gg);
			};
		};
		return;
	};
	ResFile ff1 = RReset(name);
	if (ff1 != INVALID_HANDLE_VALUE)
	{
		int Lx = 0;
		int Ly = 0;
		RBlockRead(ff1, &Lx, 2);
		RBlockRead(ff1, &Ly, 2);
		if (Lx > 0 && Ly > 0)
		{
			PicPtr = (word*)(new char[Lx * Ly + 4]);
			RBlockRead(ff1, PicPtr + 2, Lx * Ly);
			PicPtr[0] = Lx;
			PicPtr[1] = Ly;
			int pptr = int(PicPtr + 2);
			int Len = Lx * Ly;
			__asm {
				push esi
				mov ecx, Len
				mov esi, pptr
				lll : cmp byte ptr[esi], 0xFF
				jne uuu
				mov byte ptr[esi], 0xF6
				uuu : inc esi
				dec ecx
				jnz lll
				pop esi
			};
		}
		else PicPtr = nullptr;
		RClose(ff1);
	}
	else
	{
		PicPtr = nullptr;
		char gg[128];
		sprintf(gg, "Could not load picture: %s", name);
		ErrD(gg);
	};
};

SQPicture::~SQPicture()
{
	if (PicPtr)free(PicPtr);
	PicPtr = nullptr;
};

SQPicture::SQPicture()
{
	PicPtr = nullptr;
};

SQPicture::SQPicture(char* Name)
{
	PicPtr = nullptr;
	LoadPicture(Name);
};
//-----end of pictures
//--------------Handling dialog system----------------//
void MFix();
void RedrawGameBackground();

void DrawSD(SimpleDialog* SD)
{
	int DY = 0;
	if (SD->ParentSB && SD->ParentSB->Visible)DY = SD->ParentSB->SPos;
	SD->y -= DY;
	SD->y1 -= DY;
	SD->OnDraw(SD);
	SD->y += DY;
	SD->y1 += DY;
};

void NRFUNC();
typedef void vfn();
int mrand();

extern int menu_x_off;

void DialogsSystem::ProcessDialogs()
{
	if (!bActive)
	{
		return;
	}

	if (MUSTDRAW)
	{
		MUSTDRAW = false;

		RedrawGameBackground();

		MarkToDraw();
	}

	bool UseMouse = true;
	if (ActiveParent && mouseX >= ActiveX && mouseX <= ActiveX1 && mouseY >= ActiveY && mouseY <= ActiveY1)
	{
		ActiveParent->MouseOverActiveZone(ActiveParent, mouseX - ActiveX, mouseY - ActiveY, ActiveID);
		UseMouse = false;
	}
	else
	{
		if (ActiveParent && Lpressed && (ActiveParent->x > mouseX || ActiveParent->x1 < mouseX || ActiveParent->y > mouseY ||
			ActiveParent->y1 < mouseY))
		{
			ActiveParent->IsActive = false;
			for (int i = 0; i < MAXDLG; i++)
			{
				SimpleDialog* SD = DSS[i];
				if (SD)
				{
					SD->NeedToDraw = true;
				}
			}
			ActiveParent = nullptr;
		}
	}

	if (ActiveParent)
	{
		ActiveParent->NeedToDraw = true;
	}

	SimpleDialog* SD_Hinted = nullptr;

	for (int i = 0; i < MAXDLG; i++)
	{
		SimpleDialog* SD = DSS[i];
		if (SD && SD->Enabled)
		{
			int dd = 0;
			if (SD->ParentSB && SD->ParentSB->Visible)
			{
				dd = SD->ParentSB->SPos;
			}
			SD->y -= dd;
			SD->y1 -= dd;

			if (SD->Refresh)
			{
				SD->Refresh(SD);
			}
			if (SD->Active)
			{
				if (SD->OnKeyDown)
				{
					if (KeyPressed && SD->OnKeyDown(SD))
						KeyPressed = false;
				}
			}

			if (UseMouse && mouseX >= SD->x && mouseX <= SD->x1 && mouseY >= SD->y && mouseY <= SD->y1 &&
				mouseX >= WindX && mouseY >= WindY && mouseX <= WindX1 && mouseY <= WindY1)
			{
				if (SD->OnMouseOver)
					SD->OnMouseOver(SD);
				if (SD->MouseOverActive)
					SD->NeedToDraw = true;
				SD->MouseOver = true;
				if (Lpressed)
				{
					if (SD->OnUserClick)
					{
						bool played = false;
						if (SD->UserParam == mcmOk && OkSound != -1)
						{
							PlayEffect(OkSound, 0, 0);
							played = true;
						}
						if (SD->UserParam == mcmCancel && CancelSound != -1)
						{
							PlayEffect(CancelSound, 0, 0);
							played = true;
						}
						if (UserClickSound != -1 && !played)
						{
							PlayEffect(UserClickSound, 0, 0);
						}
						SD->OnUserClick(SD);
						Lpressed = false;
						try
						{
							if (int(Hint) == 0xdddddddd)
								return;
						}
						catch (...)
						{
							return;
						}
						Lpressed = false;
					}

					if (SD->OnClick)
					{
						if (SD->OnClick(SD))
						{
							for (int j = 0; j < MAXDLG; j++)
							{
								SimpleDialog* SD1 = DSS[j];
								if (SD1 && SD1 != SD)
								{
									SD1->Active = false;
								}
							}
						}
						Lpressed = false;
					}
				}
			}
			else
			{
				if (SD->MouseOver && SD->MouseOverActive)
				{
					SD->NeedToDraw = true;
				}
				SD->MouseOver = false;
			}
			SD->y += dd;
			SD->y1 += dd;
		}
		else
		{
			if (SD)
				SD->MouseOver = 0;
		}
		if (SD && SD->OnDraw)
		{
			DrawSD(SD);
		}

		if (SD && SD->NeedRedrawAll)
		{
			for (int j = 0; j < MAXDLG; j++)
			{
				if (DSS[j])DSS[j]->NeedToDraw = true;
			}
			SD->NeedRedrawAll = false;
		}
	}

	Hint = nullptr;
	for (int i = 0; i < MAXDLG; i++)
	{
		SimpleDialog* SD = DSS[i];
		if (SD)
		{
			if (SD->OnDrawActive)
			{
				TempWindow TW;
				PushWindow(&TW);
				WindX = 0;
				WindX1 = RealLx - 1;
				WindY = 0;
				WindY1 = RealLy - 1;
				WindLx = RealLx;
				WindLy = RealLy;
				SD->OnDrawActive(SD);
				PopWindow(&TW);
			}
			if (SD->MouseOver && SD->Visible && SD->Hint)
			{
				Hint = SD->Hint;
				SD_Hinted = SD;
			}
			if (SD->MouseOver && !SD->PrevMouseOver)
			{
				if (SD->MouseSound != -1)
				{
					PlayEffect(SD->MouseSound, 0, 0);
				}
			}
			SD->PrevMouseOver = SD->MouseOver;
		}
	}

	if (SD_Hinted && SD_Hinted->Child)
	{
		while (SD_Hinted)
		{
			if (SD_Hinted->OnDraw && SD_Hinted->Visible)
			{
				DrawSD(SD_Hinted);
			}
			SD_Hinted = SD_Hinted->Child;
		}
	}

	TempWindow TW;
	PushWindow(&TW);
	WindX = 0;
	WindY = 0;
	WindX1 = RealLx - 1;
	WindY1 = RealLy - 1;
	WindLx = RealLx;
	WindLy = RealLy;

	if (!Hint && DefaultHint[0])
	{
		Hint = DefaultHint;
	}

	if (Hint && HintFont)
	{
		if (HintX == -1)
		{
			//Dynamic hints in stats screen
			int MaxLx = 80;
			int Ly = 0;
			int Ny = 1;

			CheckFontColor(HintFont);

			int Dy = GetRLCHeight(HintFont->RLC, 'y');
			Dy += UNI_HINTDLY2;

			byte c = ' ';
			int x1 = 0;
			int pos = 0;
			do
			{
				c = Hint[pos];
				if (c != 0 && c != '/' && c != '\\')
				{
					//ShowRLCItem(x1,y0,&HintFont->RLC,c,0);
					if (c == ' ' && x1 > HintY)
					{
						pos++;
						if (x1 > MaxLx)
						{
							MaxLx = x1;
						}
						x1 = 0;
						Ny++;
					}
					else
					{
						int L = 1;
						x1 += GetRLCWidthUNICODE(HintFont->RLC, (byte*)(Hint + pos), &L);
						pos += L;
					};
				}
				else
				{
					if (c == '/' || c == '\\')
					{
						pos++;
						if (x1 > MaxLx)
						{
							MaxLx = x1;
						}
						x1 = 0;
						Ny++;
					};
				};
			}
			while (c);

			if (x1 > MaxLx)
			{
				MaxLx = x1;
			}

			Ly = Ny * Dy;
			int x0 = mouseX - MaxLx + 20;
			int y0 = mouseY + 32;

			if (x0 < 10 + menu_x_off)
			{
				x0 = 10 + menu_x_off;
			}
			if (x0 + MaxLx + 20 > RealLx)
			{
				x0 = RealLx - MaxLx - 20;
			}
			if (y0 + Ly + 16 > RealLy)
			{
				y0 = RealLy - Ly - 16;
			}

			DrawHintPattern(x0, y0, x0 + MaxLx + 14, y0 + Ly + 12);
			x0 += 7;
			y0 += 5;
			pos = 0;
			x1 = x0;
			do
			{
				c = Hint[pos];
				if (c != 0 && c != '/' && c != '\\')
				{
					if (c == ' ' && x1 > x0 + HintY)
					{
						pos++;
						x1 = x0;
						y0 += Dy;
						Ny++;
					}
					else
					{
						ShowCharUNICODE(x1, y0, (byte*)(Hint + pos), HintFont);
						int L = 1;
						x1 += GetRLCWidthUNICODE(HintFont->RLC, (byte*)(Hint + pos), &L);
						pos += L;
					};
				}
				else
				{
					if (c == '/' || c == '\\')
					{
						x1 = x0;
						y0 += Dy;
						pos++;
					};
				};
			}
			while (c);
		}
		else
		{
			//Ordinary hints
			int x0 = HintX;
			int y0 = HintY + UNI_HINTDY1;
			int pos = 0;
			int x1 = HintX;
			CheckFontColor(HintFont);
			int Dy = GetRLCHeight(HintFont->RLC, 'y') + UNI_HINTDLY1;
			byte c = ' ';
			do
			{
				c = Hint[pos];
				if (c != 0 && c != '/' && c != '\\')
				{
					ShowCharUNICODE(x1, y0, (byte*)(Hint + pos), HintFont);
					int L = 1;
					x1 += GetRLCWidthUNICODE(HintFont->RLC, (byte*)(Hint + pos), &L);
					pos += L;
				}
				else
				{
					if (c == '/' || c == '\\')
					{
						x1 = x0;
						y0 += Dy;
						pos++;
					};
				};
			}
			while (c);
		};
	};

	PopWindow(&TW);

	MFix();
}

void RedrawOffScreenMouse();
void OnMouseMoveRedraw();
void PostRedrawMouse();
void SERROR();
void SERROR1();
void SERROR2();
extern bool InGame;
extern bool RecordMode;

extern bool InMainMenuLoop;
extern bool RUNMAPEDITOR;
extern bool RUNUSERMISSION;
extern int ItemChoose;

void DialogsSystem::RefreshView()
{
	if (!InMainMenuLoop)
	{
		if (RUNUSERMISSION || RUNMAPEDITOR)
		{
			ItemChoose = mcmCancel;
		}
	}

	if (!bActive)
	{
		return;
	}

	if (window_mode)
	{
		RSCRSizeX = SCRSizeX;
		RedrawOffScreenMouse();
		PostRedrawMouse();
		FlipPages();
	}
	else
	{
		RedrawOffScreenMouse();
		CopyToScreen(0, 0, RealLx, RSCRSizeY);
		PostRedrawMouse();
	}
}

void DialogsSystem::CloseDialogs()
{
	for (int i = 0; i < MAXDLG; i++)
	{
		SimpleDialog* SD = DSS[i];
		if (SD)
		{
			if (SD->AllocHint && SD->Hint)
			{
				free(Hint);
			}
			if (SD->AllocPtr)
			{
				free(SD->AllocPtr);
			}
			SD->AllocPtr = nullptr;
			if (SD->Destroy)
			{
				SD->Destroy(SD);
			}
			free(SD);
			DSS[i] = nullptr;
		}
	}
}

void DialogsSystem::MarkToDraw()
{
	for (int i = 0; i < MAXDLG; i++)
	{
		if (DSS[i])
		{
			DSS[i]->NeedToDraw = true;
		}
	}
}

void UnPress();
//----------end of handler of the dialogsystem--------//
DialogsSystem::~DialogsSystem()
{
	CloseDialogs();
	UnPress();
};
//transparency effect

byte* TransPtr = nullptr;
int TransLx = 0;
int TransLy = 0;

void MakeTranspSnapshot()
{
	if (RealLx != TransLx || RealLy != TransLy)
	{
		TransPtr = (byte*)realloc(TransPtr, RealLx * RealLy);
		TransLx = RealLx;
		TransLy = RealLy;
	};
	for (int i = 0; i < RealLy; i++)
	{
		memcpy(TransPtr + i * RealLx, (byte*)ScreenPtr + i * ScrWidth, RealLx);
	};
};

void FreeTransBuffer()
{
	if (TransPtr)free(TransPtr);
	TransPtr = nullptr;
	TransLx = 0;
	TransLy = 0;
};

void EncodeLine(byte* src, byte* dst, byte* scr, byte* tbl, byte* oddtbl)
{
	int N = TransLx >> 1;
	__asm {
		push esi
		push edi
		push ebx
		mov esi, tbl
		mov edi, oddtbl
		mov ebx, scr
		mov edx, src
		mov ecx, dst
		LPP1 :
		mov al, [edx]
		mov ah, [ecx]
		mov al, [esi + eax]
		mov[ebx], al
		inc edx
		inc ecx
		inc ebx
		mov al, [edx]
		mov ah, [ecx]
		mov al, [edi + eax]
		mov[ebx], al
		inc edx
		inc ecx
		inc ebx
		dec N
		jnz LPP1

		pop ebx
		pop edi
		pop esi
	};
};

void EncodeLine1(byte* src, byte* dst, byte* scr, byte* tbl)
{
	int N = TransLx >> 1;
	__asm {
		push esi
		push edi
		push ebx
		mov esi, tbl
		mov edi, tbl
		mov ebx, scr
		mov edx, src
		mov ecx, dst
		LPP1 :
		mov al, [edx]
		mov ah, [ecx]
		mov al, [esi + eax]
		mov[ebx], al
		inc edx
		inc ecx
		inc ebx
		mov ah, [ecx]
		mov[ebx], ah
		inc edx
		inc ecx
		inc ebx
		dec N
		jnz LPP1

		pop ebx
		pop edi
		pop esi
	};
};

void EncodeLine2(byte* src, byte* dst, byte* scr, byte* tbl)
{
	int N = TransLx >> 1;
	__asm {
		push esi
		push edi
		push ebx
		mov esi, tbl
		mov edi, tbl
		mov ebx, scr
		mov edx, src
		mov ecx, dst
		LPP1 :
		mov ah, [ecx]
		mov[ebx], ah
		inc edx
		inc ecx
		inc ebx
		mov al, [edx]
		mov ah, [ecx]
		mov al, [edi + eax]
		mov[ebx], al
		inc edx
		inc ecx
		inc ebx
		dec N
		jnz LPP1

		pop ebx
		pop edi
		pop esi
	};
};
extern byte trans4[65536];
extern byte trans8[65536];

void PerformTransMix(int degree)
{
	int N = TransLy >> 1;
	switch (degree)
	{
	case 0:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine1(transp, screen, screen, trans4);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine2(transp, screen, screen, trans4);
			};
		};
		break;
	case 1:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine(transp, screen, screen, trans4, trans4);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine(transp, screen, screen, trans4, trans4);
			};
		};
		break;
	case 2:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine(transp, screen, screen, trans8, trans4);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine(transp, screen, screen, trans4, trans8);
			};
		};
		break;
	case 3:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine(transp, screen, screen, trans8, trans8);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine(transp, screen, screen, trans8, trans8);
			};
		};
		break;
	case 4:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine(screen, transp, screen, trans4, trans8);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine(screen, transp, screen, trans8, trans4);
			};
		};
		break;
	case 5:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine(screen, transp, screen, trans4, trans4);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine(screen, transp, screen, trans4, trans4);
			};
		};
		break;
	case 6:
		{
			for (int i = 0; i < N; i++)
			{
				byte* screen = (byte*)ScreenPtr + i * 2 * ScrWidth;
				byte* transp = (byte*)TransPtr + i * 2 * TransLx;
				EncodeLine1(screen, transp, screen, trans4);
				screen += ScrWidth;
				transp += TransLx;
				EncodeLine2(screen, transp, screen, trans4);
			};
		};
		break;
	};
};
int Time0 = 0;

void StartMixing()
{
	Time0 = GetTickCount();
	MakeTranspSnapshot();
};

void ProcessMixing()
{
	int idx = (GetTickCount() - Time0) / 50;
	if (idx <= 8)
	{
		idx--;
		if (idx < 0)idx = 0;
		if (idx > 6)idx = 6;
		if (TransPtr)
		{
			PerformTransMix(6 - idx);
		};
	}
	else
	{
		FreeTransBuffer();
	};
};
