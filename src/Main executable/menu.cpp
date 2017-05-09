#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "mode.h"
#include "event.h"
extern int SCRSizeX;
extern int SCRSizeY;
extern int RSCRSizeX;
extern int RSCRSizeY;
extern int COPYSizeX;
typedef void fnc(void);
struct AZone {
	int x, y, x1, y1;
};
class Menu {
public:
	Menu(void);
	bool	Active : 1;
	bool	MakeShift : 1;
	bool	ChangeFont : 1;
	bool	EraseOnChoose : 1;
	LPCSTR  items;
	int		MenuLy;
	int		ItemLy;
	int		NItems;
	int		StartY;
	fnc*	RedrawBackground;
	AZone	Zone[16];
	int		NZones;
	int		choose;;
	lpRLCFont font;
	lpRLCFont font1;
	int		evHandler;
	void	CreateMenu(LPCSTR s, lpRLCFont f, lpRLCFont f1, fnc* ff, int style);
	void	Show(int n);
	int		GetAmount();
	LPCSTR  GetNstr(int n);
	int		GetLen(LPCSTR s);
	void	ShowModal();
	void	Close();

};
Menu::Menu() {
	items = NULL;
	NZones = 0;
};

void MouseHandle(void* par) {
	int j = -1;
	Menu* mn = (Menu*)par;
	for (int i = 0; i < (*mn).NZones; i++)
		if (mouseX >= (*mn).Zone[i].x && mouseY >= (*mn).Zone[i].y &&
			mouseX < (*mn).Zone[i].x1 && mouseY < (*mn).Zone[i].y1) j = i;
	(*mn).Show(j);
	if (Lpressed && j != -1) {
		if (mn->EraseOnChoose) {
			CloseEventHandler((*mn).evHandler);
			mn->Active = false;
		};
		(*mn).choose = j;
	}
	else (*mn).choose = -1;
	return;
};
//"Item1|Item2|Item3"
char tmpstr[1000];
int Menu::GetAmount() {
	int j = 0;
	int	i;
	if (!int(items)) return 0;
	for (i = 0; items[i] != 0; i++)if (items[i] == '|')j++;
	return j + 1;
};
LPCSTR Menu::GetNstr(int n) {
	int j = 0;
	int i;
	tmpstr[0] = 0;
	if (!int(items))return LPCSTR(&tmpstr);
	for (i = 0; j < n&&items[i] != 0; i++)
	{
		if (items[i] == '|')j++;
	};
	if (j != n)return LPCSTR(&tmpstr);
	int k;
	for (k = 0; items[i] != 0 && items[i] != '|'; i++, k++)
	{
		tmpstr[k] = items[i];
	}
	tmpstr[k] = 0;
	return LPCSTR(&tmpstr);
};
int Menu::GetLen(LPCSTR s) {
	if (!int(s))return 0;
	int x = 0;
	for (int i = 0; s[i] != 0; i++) {
		int L = 1;
		x += GetRLCWidthUNICODE(font->RLC, (byte*)(s + i), &L);
		i += L - 1;
	};
	return x;
};
void Menu::CreateMenu(LPCSTR s, lpRLCFont f, lpRLCFont f1, fnc* ff, int style) {
	if (!int(items))delete((void*)items);
	items = new(char[strlen(s) + 1]);
	memcpy((void*)items, s, strlen(s) + 1);
	NItems = GetAmount();
	font = f;
	font1 = f1;
	NZones = NItems;
	ItemLy = GetRLCHeight(font->RLC, font->FirstSymbol + 'W');
	MenuLy = ItemLy*NItems;
	StartY = (RSCRSizeY - MenuLy) / 2;
	int Ly = 0;
	for (int i = 0; i < NItems; i++) {
		LPCSTR ss = GetNstr(i);
		int Lx = GetLen(ss);
		Zone[i].x = (COPYSizeX - Lx) / 2 + 100;
		Zone[i].y = StartY + i*ItemLy;
		Zone[i].x1 = Zone[i].x + Lx;
		Zone[i].y1 = Zone[i].y + ItemLy;
	};

	RedrawBackground = ff;
	Active = style & 1 ? true : false;
	MakeShift = style & 2 ? true : false;
	ChangeFont = style & 4 ? true : false;
	EraseOnChoose = style & 8 ? true : false;
};
void Menu::Show(int n) {
	if (!int(items)) return;
	if (int(RedrawBackground))(*RedrawBackground)();
	for (int i = 0; i < NItems; i++) {
		LPCSTR s = GetNstr(i);
		int x = Zone[i].x;
		int y = Zone[i].y;
		lpRLCFont ff = font;
		if (i == n) {
			if (MakeShift) { x += 2; y += 2; }
			if (ChangeFont)ff = font1;
		};
		ShowString(x, y, s, ff);
	};
};

void Menu::ShowModal() {
	if (!Active) {
		evHandler = RegisterEventHandler(&MouseHandle, 21, this);
		choose = -1;
		Active = true;
	};
};
void Menu::Close() {
	CloseEventHandler(evHandler);
	Active = false;
};

