#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"

void AssignHintLo(char* s, int time);

class MouseZone 
{
public:
	int x, y, x1, y1, Index, MoveIndex;
	byte ScanCode;
	byte KeyState;
	byte Pressed;
	HandlePro* Pro;
	HandlePro* RPro;
	HandlePro* MoveOver;
	char* Hint;
	char* HintLo;
	MouseZone();
};

extern bool LockMouse;
MouseZone::MouseZone() 
{
	Index = -1;
	Hint = NULL;
}

#define NZones 64
MouseZone Zones[NZones];
void InitZones() 
{
	for (int i = 0; i < NZones; i++) 
	{
		Zones[i].Index = -1;
	}
}

int CreateRZone(int x, int y, int lx, int ly, HandlePro* HPro, HandlePro* RHPro, int Index, char* Hint) 
{
	int i;
	for (i = 0; i < NZones; i++) 
	{
		if (Zones[i].Index == -1)
		{
			break;
		}
	}
	if (i < NZones) 
	{
		MouseZone* Z = &(Zones[i]);
		Z->x = x;
		Z->y = y;
		Z->x1 = x + lx - 1;
		Z->y1 = y + ly - 1;
		Z->Pro = HPro;
		Z->RPro = RHPro;
		Z->MoveOver = NULL;
		Z->Index = Index;
		Z->Pressed = false;
		if (int(Z->Hint))
			free(Z->Hint);
		Z->Hint = new char[strlen(Hint) + 1];
		strcpy(Z->Hint, Hint);
		Z->KeyState = 0;
		Z->ScanCode = 0xFF;
		return i;
	}
	return -1;
}

int CreateRZone(
	int x, int y, int lx, int ly,
	HandlePro* HPro, HandlePro* RHPro,
	int Index, char* Hint, char* HintLo
) 
{
	int i;
	for (i = 0; i < NZones; i++) 
	{
		if (Zones[i].Index == -1)
			break;
	}
	if (i < NZones) 
	{
		MouseZone* Z = &(Zones[i]);

		Z->x = x;
		Z->y = y;
		Z->x1 = x + lx - 1;
		Z->y1 = y + ly - 1;
		Z->Pro = HPro;
		Z->RPro = RHPro;
		Z->MoveOver = NULL;
		Z->Index = Index;
		Z->Pressed = false;
		Z->KeyState = 0;
		Z->ScanCode = 0xFF;

		if (Z->Hint) 
		{
			free(Z->Hint);
			Z->Hint = NULL;
		}
		if (Z->HintLo) 
		{
			free(Z->HintLo);
			Z->HintLo = NULL;
		}
		if (Hint) 
		{
			Z->Hint = new char[strlen(Hint) + 1];
			strcpy(Z->Hint, Hint);
		}
		if (HintLo) 
		{
			Z->HintLo = new char[strlen(HintLo) + 1];
			strcpy(Z->Hint, Hint);
		}
		return i;
	}
	return -1;
}

int CreateZone(int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint) 
{
	int i;
	for (i = 0; i < NZones; i++) 
	{
		if (Zones[i].Index == -1)break;
	}
	if (i < NZones) 
	{
		MouseZone* Z = &(Zones[i]);
		Z->x = x;
		Z->y = y;
		Z->x1 = x + lx - 1;
		Z->y1 = y + ly - 1;
		Z->Pro = HPro;
		Z->RPro = NULL;
		Z->Index = Index;
		Z->Pressed = false;
		Z->KeyState = 0;
		Z->ScanCode = 0xFF;
		if (int(Z->Hint))
			free(Z->Hint);
		Z->Hint = new char[strlen(Hint) + 1];
		strcpy(Z->Hint, Hint);
		return i;
	}
	return -1;
}

void AssignMovePro(int i, HandlePro* HPro, int id) 
{
	if (i != -1) 
	{
		Zones[i].MoveOver = HPro;
		Zones[i].MoveIndex = id;
	}
}

void AssignKeys(int i, byte Scan, byte State) 
{
	if (i != -1) 
	{
		Zones[i].ScanCode = Scan;
		Zones[i].KeyState = State;
	}
}

int CreateZone(int x, int y, int lx, int ly, HandlePro* HPro, int Index, char* Hint, char* HintLo) {
	int i;
	for (i = 0; i < NZones; i++) {
		if (Zones[i].Index == -1)break;
	};
	if (i < NZones) {
		MouseZone* Z = &(Zones[i]);
		Z->x = x;
		Z->y = y;
		Z->x1 = x + lx - 1;
		Z->y1 = y + ly - 1;
		Z->Pro = HPro;
		Z->RPro = NULL;
		Z->MoveOver = NULL;
		Z->Index = Index;
		Z->Pressed = false;
		Z->KeyState = 0;
		Z->ScanCode = 0xFF;
		if (int(Z->Hint)) {
			free(Z->Hint);
			Z->Hint = NULL;
		};
		if (int(Z->HintLo)) {
			free(Z->HintLo);
			Z->HintLo = NULL;
		};
		if (Hint) {
			Z->Hint = new char[strlen(Hint) + 1];
			strcpy(Z->Hint, Hint);
		};
		if (HintLo) {
			Z->HintLo = new char[strlen(HintLo) + 1];
			strcpy(Z->HintLo, HintLo);
		};
		return i;
	};
	return -1;
};
bool MouseOverZone = 0;
extern byte SpecCmd;
byte LastPressedCodes[8] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
extern bool EnterChatMode;
extern bool EditMapMode;
byte ScanPressed[256];
int CheckZonePressed(int i) {
	if (EnterChatMode || EditMapMode)return false;
	for (int j = 0; j < 8; j++)if (LastPressedCodes[j] != 0xFF) {
		if (!(GetKeyState(LastPressedCodes[j]) & 0x8000))LastPressedCodes[j] = 0xFF;
	};
	if (i < NZones&&Zones[i].Index != -1) {
		if (Zones[i].ScanCode != 0xFF) {
			if ((GetKeyState(Zones[i].ScanCode) & 0x8000) || ScanPressed[Zones[i].ScanCode]) {
				byte State = Zones[i].KeyState;
				byte Scan = Zones[i].ScanCode;

				if (State & 1) {
					if (!(GetKeyState(VK_CONTROL) & 0x8000))return false;
				}
				else if (GetKeyState(VK_CONTROL) & 0x8000)return false;
				if (State & 2) {
					if (!(GetKeyState(VK_MENU) & 0x8000))return false;
				}
				else if (GetKeyState(VK_MENU) & 0x8000)return false;
				if (State & 4) {
					if (!(GetKeyState(VK_SHIFT) & 0x8000))return false;
				}
				else if (GetKeyState(VK_SHIFT) & 0x8000)return false;

				for (int j = 0; j < 8; j++)if (LastPressedCodes[j] == Scan)return 1;
				for (int j = 0; j < 8; j++)if (LastPressedCodes[j] == 0xFF) {
					LastPressedCodes[j] = Scan;
					return 2;
				};
				return 2;
			}
			else return false;
		}
		else return false;
	}
	else return false;
};
extern byte KeyCodes[512][2];
#define NKEYS 61
extern byte ScanKeys[NKEYS];
bool CheckSpritePressed(int sp) {
	if (sp < 0 || sp >= 512 || EnterChatMode || EditMapMode)return false;
	if (KeyCodes[sp][0]) {
		if ((GetKeyState(ScanKeys[KeyCodes[sp][0]]) & 0x8000) || ScanPressed[ScanKeys[KeyCodes[sp][0]]]) {
			byte State = KeyCodes[sp][1];
			byte Scan = ScanKeys[KeyCodes[sp][0]];
			if (State & 1) {
				if (!(GetKeyState(VK_CONTROL) & 0x8000))return false;
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000)return false;
			if (State & 2) {
				if (!(GetKeyState(VK_MENU) & 0x8000))return false;
			}
			else if (GetKeyState(VK_MENU) & 0x8000)return false;
			if (State & 4) {
				if (!(GetKeyState(VK_SHIFT) & 0x8000))return false;
			}
			else if (GetKeyState(VK_SHIFT) & 0x8000)return false;
			return 1;
		};
	};
	return false;
};

void ControlZones()
{
	MouseOverZone = 0;
	if (LockMouse)
	{
		return;
	}
	int i;
	MouseZone* Z = nullptr;
	if (!Lpressed)
	{
		for (i = 0; i < NZones; i++)
		{
			Zones[i].Pressed = CheckZonePressed(i);
		}
	}
	for (i = 0; i < NZones; i++)
	{
		Z = &(Zones[i]);
		if ((Z->Index != -1 && mouseX >= Z->x&&mouseX <= Z->x1
			&& mouseY >= Z->y&&mouseY <= Z->y1)
			|| Z->Pressed)
		{
			break;
		}
	}
	if (i < NZones)
	{
		MouseOverZone = 1;
		if (SpecCmd == 241)
		{
			SpecCmd = 0;
		}
		if (CheckZonePressed(i) == 2 || Z->Pressed != 1)
		{
			if (Lpressed)
			{
				Z->Pressed = true;
			}
			if (int(Z->Hint))
			{
				AssignHint(Z->Hint, 3);
			}
			if (Z->HintLo)
			{
				AssignHintLo(Z->HintLo, 3);
			}
			if ((Lpressed || Z->Pressed == 2) && int(Z->Pro))
			{//Handle mouse clicks?
				(*Z->Pro)(Z->Index);
			}
			Lpressed = false;
			if (Rpressed&&Z->RPro)
			{
				(*Z->RPro)(Z->Index);
			}
			if (Z->MoveOver)
			{
				Z->MoveOver(Z->MoveIndex);
			}
			Rpressed = false;
			if (Z->Pressed == 2)
			{
				Z->Pressed = 0;
			}
		}
	}
	for (int i = 0; i < NZones; i++)
	{
		Zones[i].Pressed = 0;
	}
	memset(ScanPressed, 0, 256);
}

void DeleteZone(int i)
{
	if (i < NZones&&i >= 0)
	{
		Zones[i].Index = -1;
	}
}
