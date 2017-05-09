extern word NNewMon;
extern NewMonster NewMon[512];
extern char* MonNames[512];
int CheckPt(int x, int y);
bool CheckBar(int x, int y, int Lx, int Ly);
#pragma warning(disable : 4035)

//returns (2|x| + |y|) / 2 if x > y
//returns (2|y| + |x|) / 2 if y > x
inline int Norma(int x, int y)
{
	__asm
	{
		mov		eax, x
		cmp		eax, 0
		jge		ggg1
		neg		eax
		ggg1 : mov		ebx, y
			   cmp		ebx, 0
			   jge		ggg2
			   neg		ebx
			   ggg2 : mov		ecx, eax
					  cmp		eax, ebx
					  ja		ggg3
					  mov		ecx, ebx
					  ggg3 : add		ecx, eax
							 add		ecx, ebx
							 shr		ecx, 1
							 mov		eax, ecx
	}
}
#pragma warning(default : 4035)

void TryToStand(OneObject* OB, bool rest);
void BSetPt(int x, int y);
void BClrPt(int x, int y);
void BSetBar(int x, int y, int Lx);
void BClrBar(int x, int y, int Lx);
//FindPoint flags:
#define FP_NEAREST_POINT  1
#define FP_UNLOCKED_POINT 2
#define FP_FIND_WORKPOINT 4
#define FP_FIND_DAMPOINT  8
#define FP_CONCENTRATION  16
void MakeRoundDamage(int x, int y, int r, word Damage, OneObject* Sender, word Attr);
void AddToVisual(short uy, short x, short y, OneObject* OB, word FileID, word Sprite, int Param1, int Param2);
void AddToVisual(short uy, short x, short y, OneObject* OB, word FileID, word Sprite, int Options);
//Adding to visual options:
#define AV_NORMAL      0x00000000
#define AV_PULSING     0x00000001
#define AV_TRANSPARENT 0x00000002
#define AV_PALETTE     0x00000003
#define AV_GRADIENT    0x00000004
//---
#define AV_RED         0x00000010
#define AV_WHITE       0x00000020
#define AV_DARK        0x00000030
//---
#define AV_SHADOWONLY    ((0x20<<9)+256)
#define AV_WITHOUTSHADOW (((0xFFFF-0x20)<<9)+256)
//bool CheckCostNM(byte NI,NewMonster* NM);
//bool ApplyCostNM(byte NI,NewMonster* NM);
//bool CheckCost(byte NI,GeneralObject* GO);
//bool ApplyCost(byte NI,GeneralObject* GO);
bool CheckCost(byte NI, word NIndex);
bool ApplyCost(byte NI, word NIndex);
#define TEX_ALWAYS_LAND_LOCK		1
#define TEX_ALWAYS_LAND_UNLOCK		2
#define TEX_ALWAYS_WATER_UNLOCK		4
#define TEX_PLAIN                   8
#define TEX_HARD                    16
#define TEX_HARDLIGHT               32
#define TEX_NOLIGHT                 64
#define TEX_NORMALPUT               128
extern word TexFlags[256];