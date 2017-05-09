void LoadPointer(LPCSTR s);
void SetMPtr(int x,int y,int key);
//void ShowMPtr();
//void ShowMPtra();
extern int mouseX;
extern int mouseY;
extern bool Lpressed;
extern bool Rpressed;
struct MouseStack{
	int x,y;
	bool Lpressed:1;
	bool Rpressed:1;
	bool rLpressed:1;
	bool rRpressed:1;
	bool Control:1;
	bool Shift:1;
};
MouseStack* ReadMEvent();
void ClearMStack();