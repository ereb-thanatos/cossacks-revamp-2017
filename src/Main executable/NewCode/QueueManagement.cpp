#include <windows.h>

/*
	Determine increment / decrement amount for production queues.
*/
int GetQueueMultiplier()
{
	if (GetKeyState(VK_F5) & 0x8000) return 250;
	if (GetKeyState(VK_TAB) & 0x8000) return 50;
	if (GetKeyState(VK_F2) & 0x8000) return 36;
	if (GetKeyState(VK_MENU) & 0x8000) return 20; // Alt key
	if (GetKeyState(VK_F1) & 0x8000) return 15;
	if (GetKeyState(VK_SHIFT) & 0x8000) return 5;
	return 1;
}