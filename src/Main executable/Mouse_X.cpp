#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mode.h"
#include "GP_Draw.h"

#define MaxMX 32
#define MsizeX 32

extern int CurrentCursorGP;
extern int SCRSizeX;
extern int SCRSizeY;
extern int RSCRSizeX;
extern int RSCRSizeY;
extern int COPYSizeX;
bool realLpressed;
bool realRpressed;

//Current cursor image
//0: Default arrow cursor
//1: Sword
//2: Shackles
//3: Hammer
//4: Enter mine
//5: Pickaxe
//6: Axe
//7: Scythe
//8: Gathering point
//9: Artillery barrage
//10: Guard (shield)
//11: Enter transport
//12: Guard (highlighted shield)
//13: Patrol (shield and sword)
int curptr;

int mouseX;
int	mouseY;
bool Lpressed;
bool Rpressed;
static char buf1[1024];
static char buf2[1024];
static char buf1o[1024];
static char buf2o[1024];
static int OldMX;
static int OldMY;
static int MX;
static int MY;

static bool LockMouse;

int curdx;
int curdy;
bool MNotRemoved;
typedef unsigned short word;
int GetF( word k );

extern int mapx;
extern int mapy;
extern int smapx;
extern int smapy;

//retreives data from the screen buffer to field 32x32
void GetMData( void* dest, void* src, int x, int y, int SSizeX, int SSizeY )
{
	if (!bActive)
		return;

	int Lx = 32;
	int Ly = 32;
	int x1 = x;
	int y1 = y;
	int bx = 0;//x-coord. on bitbap 32x32
	int by = 0;
	if (x1 < 0)
	{
		bx = -x1;
		Lx += x1;
		x1 = 0;
	}
	if (y1 < 0)
	{
		by = -y1;
		Ly += y1;
		y1 = 0;
	}
	if (x1 + 32 > SSizeX)Lx = SSizeX - x1;
	if (y1 + 32 > SSizeY)Ly = SSizeY - y1;
	if (Lx <= 0 || Ly <= 0)return;
	int sofs = int( src ) + x1 + y1*SSizeX;
	int dofs = int( dest ) + bx + ( by << 5 );
	int Lx4 = Lx >> 2;
	int Lx1 = Lx & 3;
	int adds = SSizeX - Lx;
	int addd = 32 - Lx;

	__asm
	{
		push	esi
		push	edi
		mov		edx, Ly
		mov		esi, sofs
		mov		edi, dofs
		cld
		lpp1 : mov		ecx, Lx4
			   jcxz	lpp2
			   rep		movsd
			   lpp2 : mov		ecx, Lx1
					  jcxz	lpp3
					  rep		movsb
					  lpp3 : add		esi, adds
							 add		edi, addd
							 dec		edx
							 jnz		lpp1
							 pop		edi
							 pop		esi
	}

}

bool CmpMData( void* dest, void* src, int x, int y, int SSizeX, int SSizeY )
{
	int Lx = 32;
	int Ly = 32;
	int x1 = x;
	int y1 = y;
	int bx = 0;//x-coord. on bitbap 32x32
	int by = 0;
	if (x1 < 0)
	{
		bx = -x1;
		Lx += x1;
		x1 = 0;
	}
	if (y1 < 0)
	{
		by = -y1;
		Ly += y1;
		y1 = 0;
	}
	if (x1 + 32 > SSizeX)Lx = SSizeX - x1;
	if (y1 + 32 > SSizeY)Ly = SSizeY - y1;
	if (Lx <= 0 || Ly <= 0)return false;
	int sofs = int( src ) + x1 + y1*SSizeX;
	int dofs = int( dest ) + bx + ( by << 5 );
	int Lx4 = Lx >> 2;
	int Lx1 = Lx & 3;
	int adds = SSizeX - Lx;
	int addd = 32 - Lx;
	bool notequal = false;
	__asm
	{
		push	esi
		push	edi
		mov		edx, Ly
		mov		esi, sofs
		mov		edi, dofs
		cld
		lpp1 : mov		ecx, Lx4
			   jcxz	lpp2
			   repe	cmpsd
			   jne		noteq
			   lpp2 : mov		ecx, Lx1
					  jcxz	lpp3
					  repe	cmpsb
					  jne		noteq
					  lpp3 : add		esi, adds
							 add		edi, addd
							 dec		edx
							 jnz		lpp1
							 jmp		lpp4
							 noteq : mov		notequal, 1
									 lpp4 : pop		edi
											pop		esi
	}
	return notequal;
}

void RestoreMData( void* scrn, void* buf, void* comp, int x, int y, int SSizeX, int SSizeY )
{
	if (!bActive)
	{
		return;
	}

	int Lx = 32;
	int Ly = 32;
	int x1 = x;
	int y1 = y;
	int bx = 0;//x-coord. on bitbap 32x32
	int by = 0;

	if (x1 < 0)
	{
		bx = -x1;
		Lx += x1;
		x1 = 0;
	}
	if (y1 < 0)
	{
		by = -y1;
		Ly += y1;
		y1 = 0;
	}

	if (x1 + 32 > SSizeX)
	{
		Lx = SSizeX - x1;
	}
	if (y1 + 32 > SSizeY)
	{
		Ly = SSizeY - y1;
	}

	if (Lx <= 0 || Ly <= 0)
	{
		return;
	}

	int src1 = int( buf ) + bx + ( by << 5 );
	int srcom = int( comp ) + bx + ( by << 5 );
	int scrof = int( scrn ) + x1 + y1*SSizeX;
	int addscr = SSizeX - Lx;
	int add32 = 32 - Lx;

	__asm
	{
		push	esi
		push	edi
		mov		edx, Ly
		mov		esi, src1
		mov		ebx, srcom
		mov		edi, scrof
		cld
		lpp0 : mov		ecx, Lx
			   lpp1 : lodsb
					  mov		ah, [edi]
					  cmp		ah, [ebx]
					  jnz		lpp2
					  mov[edi], al
					  lpp2 : inc		edi
							 inc		ebx
							 dec		ecx
							 jnz		lpp1
							 add		edi, addscr
							 add		ebx, add32
							 add		esi, add32
							 dec		edx
							 jnz		lpp0
							 pop		edi
							 pop		esi
	}
}

//Sets mouse[X|Y] & real[L|R]pressed variables according to mouse state
void SetMPtr( int x, int y, int key )
{
	if (x > RSCRSizeX - 1)
	{
		x = RSCRSizeX - 1;
	}

	if (( x != mouseX ) | ( y != mouseY ))
	{
		mouseX = x;
		mouseY = y;
		realLpressed = ( ( key & MK_LBUTTON ) != 0 );
		realRpressed = ( ( key & MK_RBUTTON ) != 0 );
	}
}

//Redraws mouse in the offscreen buffer
//and prepares data for onscreen transferring 
void RedrawOffScreenMouse()
{
	if (!bActive)
	{
		return;
	}

	if (curptr == 8)
	{
		curdx = 16;
		curdy = 17;
	}
	else
	{
		curdx = 5;
		curdy = 5;
	}

	LockMouse = true;
	MX = mouseX - curdx;
	MY = mouseY - curdy;

	RestoreMData( ScreenPtr, (void*) buf1, (void*) buf2, OldMX, OldMY, SCRSizeX, SCRSizeY );

	GetMData( (void*) buf1, ScreenPtr, MX, MY, SCRSizeX, SCRSizeY );

	GPS.ShowGP( MX, MY, CurrentCursorGP, curptr, 0 );

	GetMData( (void*) buf2, ScreenPtr, MX, MY, SCRSizeX, SCRSizeY );
}

void RedrawScreenMouse()
{
	if (!bActive || window_mode)//BUGFIX: Cursor shadow trail while showing ingame menues
	{
		return;
	}

	RestoreMData( RealScreenPtr, (void*) buf1o, (void*) buf2o, OldMX, OldMY, RSCRSizeX, RSCRSizeY );
	GetMData( (void*) buf1o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
	void* osp = ScreenPtr;
	int osx = SCRSizeX;
	int sw = ScrWidth;
	ScrWidth = RSCRSizeX;
	ScreenPtr = RealScreenPtr;
	SCRSizeX = RSCRSizeX;
	GPS.ShowGP( MX, MY, CurrentCursorGP, curptr, 0 );
	SCRSizeX = osx;
	ScreenPtr = osp;
	ScrWidth = sw;
	GetMData( (void*) buf2o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
}

void OnMouseMoveRedraw()
{
	if (LockMouse)
	{
		return;
	}

	RedrawOffScreenMouse();
	RedrawScreenMouse();
	LockMouse = false;
	OldMX = MX;
	OldMY = MY;
}

void PostRedrawMouse()
{
	bool need = true;
	if (MX == OldMX && MY == OldMY)
	{
		need = CmpMData( (void*) buf2o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
	}

	if (need)
	{
		RestoreMData( RealScreenPtr, (void*) buf1, (void*) buf2, MX, MY, RSCRSizeX, RSCRSizeY );
		RestoreMData( RealScreenPtr, (void*) buf1o, (void*) buf2o, OldMX, OldMY, RSCRSizeX, RSCRSizeY );
		GetMData( (void*) buf1o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
		void* osp = ScreenPtr;
		int osx = SCRSizeX;
		int sw = ScrWidth;
		ScrWidth = RSCRSizeX;
		ScreenPtr = RealScreenPtr;
		SCRSizeX = RSCRSizeX;
		GPS.ShowGP( MX, MY, CurrentCursorGP, curptr, 0 );
		SCRSizeX = osx;
		ScreenPtr = osp;
		ScrWidth = sw;
		GetMData( (void*) buf2o, RealScreenPtr, MX, MY, RSCRSizeX, RSCRSizeY );
		OldMX = MX;
		OldMY = MY;
	}

	LockMouse = false;
}