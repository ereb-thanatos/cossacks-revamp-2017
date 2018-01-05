#include "ddini.h"
#include "ResFile.h"
#include "mode.h"
#include "fastdraw.h"
#include "Lines.h"
#include <stdio.h>
#include <assert.h>
#include <CrtDbg.h>
#include "CTables.h"
#include "GP_Draw.h"
#include "mapdiscr.h"
#include "RealWater.h"
#include <math.h>
#include "ActiveScenary.h"
#include "GSINC.h"
bool NewGPImage;
#define INTV(x) (((int*)(x))[0])

//What are you?
extern int XShift[512];

extern word RLCNSpr[MaxGPIdx];
void ShowRLCItemPal(int x, int y, lpRLCTable lprt, int n, byte* Pal);
void ShowRLCItemGrad(int x, int y, lpRLCTable lprt, int n, byte* Pal);
word GP_L_IDXS[MaxGPIdx];
int LOADED = 0;

extern int COUNTER;
typedef short* lpShort;
typedef DWORD* lpDWORD;

GP_System::GP_System()
{
	CashSize = 4200000;
	PackCash = new byte[CashSize + 4];
	PackCash[CashSize] = 0x37;
	PackCash[CashSize + 1] = 0x42;
	INTV( PackCash ) = 0;
	INTV( PackCash + 4 ) = CashSize;
	CashPos = 0;
	NGP = 0;
	NGPReady = MaxGPIdx;
	GPH = new lpGP_GlobalHeader[NGPReady];
	GPNames = new LPCHAR[NGPReady];
	GPSize = new int[NGPReady];
	GPLastTime = new int[NGPReady];
	RLCImage = new RLCTable[NGPReady];
	memset(RLCImage, 0, (sizeof RLCImage) * NGPReady);
	RLCShadow = new RLCTable[NGPReady];
	memset(RLCShadow, 0, (sizeof RLCShadow) * NGPReady);
	GPNFrames = new word[NGPReady];
	ImageType = new byte[NGPReady];
	UNITBL = (UNICODETABLE**)malloc(NGPReady << 2);
	memset(ImageType, 0, NGPReady);
	memset(UNITBL, 0, NGPReady << 2);
	ImLx = new lpShort[NGPReady];
	ImLy = new lpShort[NGPReady];
	ItDX = (char**)malloc(4 * NGPReady);
	ItLX = (char**)malloc(4 * NGPReady);
	memset(ImLx, 0, 4 * NGPReady);
	memset(ImLy, 0, 4 * NGPReady);
	memset(ItDX, 0, 4 * NGPReady);
	memset(ItLX, 0, 4 * NGPReady);
	CASHREF = new lpDWORD[NGPReady];
	Mapping = new byte[NGPReady];
	memset(CASHREF, 0, NGPReady << 2);
	memset(Mapping, 0, NGPReady);
	//PreLoadGPImage("gets2");
	memset(GP_L_IDXS, 0, sizeof GP_L_IDXS);
};

GP_System::~GP_System()
{
	free(PackCash);
	for (int i = 0; i < NGP; i++)
	{
		if (GPH[i] && !Mapping[i])free(GPH[i]);
		if (RLCImage[i])free(RLCImage[i]);
		if (RLCShadow[i])free(RLCShadow[i]);
		free(GPNames[i]);
		if (CASHREF[i])free(CASHREF[i]);
	};
	free(GPH);
	free(GPNames);
	free(RLCImage);
	free(RLCShadow);
	free(GPNFrames);
	free(ImageType);
	free(CASHREF);
	free(Mapping);
	free(UNITBL);
};

int GP_System::GetGPWidth(int i, int n)
{
	if (LoadGP(i))
	{
		if (n >= GPS.GPNFrames[i])
			return 0;
		if (ItLX[i])
			return ItLX[i][n];
		else
			return ImLx[i][n];
	}
	else return 0;
};

int GP_System::GetGPShift(int i, int n)
{
	if (LoadGP(i))
	{
		if (n >= GPS.GPNFrames[i])return 0;
		if (ItDX[i])return ItDX[i][n];
		else return 0;
	}
	else return 0;
}

int GP_System::GetGPHeight(int i, int n)
{
	if (LoadGP(i))
	{
		if (n >= GPS.GPNFrames[i])
		{
			return 0;
		}
		return ImLy[i][n];
	}
	else
	{
		return 0;
	}
}

bool GP_System::GetGPSize(int i, int n, int* Lx, int* Ly)
{
	*Lx = 0;
	*Ly = 0;
	if (LoadGP(i))
	{
		if (n >= GPS.GPNFrames[i])return false;
		*Lx = ImLx[i][n];
		*Ly = ImLy[i][n];
		return true;
	}
	else return false;
};

byte* GP_System::GetCash(int Size)
{
	int UsedSize = -8;
	int NSeg = 0;
	int tpos = CashPos;
	int cas = CashSize;
	//COUNTER=div(CashPos*100,CashSize).quot;
	byte* Cash = PackCash;
	//assert(Cash[CashSize]==0x37&&Cash[CashSize+1]==0x42);
	while (tpos < cas && UsedSize < Size)
	{
		int sz = INTV( Cash + tpos + 4 );
		UsedSize += sz;
		tpos += sz;
		NSeg++;
	};
	if (UsedSize < Size)
	{
		tpos = 0;
		UsedSize = -8;
		NSeg = 0;
		CashPos = 0;
		while (UsedSize < Size)
		{
			int sz = INTV( Cash + tpos + 4 );
			UsedSize += sz;
			tpos += sz;
			NSeg++;
		};
	};
	//free refs
	tpos = CashPos;
	for (int i = 0; i < NSeg; i++)
	{
		int sz = INTV( Cash + tpos + 4 );
		int* ptr = (int*)INTV( Cash + tpos );
		if (ptr)
		{
			ptr[0] = 0xFFFFFFFF;
			INTV( Cash + tpos ) = NULL;
		};
		tpos += sz;
	};
	if (UsedSize - Size > 32)
	{
		INTV( Cash + CashPos + Size + 8 ) = 0;
		INTV( Cash + CashPos + Size + 12 ) = UsedSize - Size;
		UsedSize = Size;
	};
	INTV( Cash + CashPos ) = 0;
	INTV( Cash + CashPos + 4 ) = UsedSize + 8;
	byte* cps = Cash + CashPos;
	CashPos += UsedSize + 8;
	return cps;
};

int GP_System::PreLoadGPImage(char* Name, bool Shadow)
{
	//search for existing name
	return PreLoadGPImage(Name);
	/*
	NewGPImage=false;
	for(int i=0;i<NGP;i++){
		if(!strcmp(Name,GPNames[i])){
			return i;
		};
	};
	UNITBL[i]=UFONTS.FindFont(Name);
	NewGPImage=true;
	//determining image type
	//1. GP
	char FName[128];
	sprintf(FName,"%s.gp",Name);
	ResFile f=RReset(FName);
	ImageType[NGP]=0;
	if(f!=INVALID_HANDLE_VALUE){
		RSeek(f,4);
		RBlockRead(f,&GPNFrames[NGP],2);
		RClose(f);
		AText(FName);
		ImageType[NGP]=1;
	}else{
		//2. RLC?
		sprintf(FName,"%s.rlc",Name);
		f=RReset(FName);
		if(f!=INVALID_HANDLE_VALUE){
			AText(FName);
			RSeek(f,4);
			RBlockRead(f,&GPNFrames[NGP],2);
			RClose(f);
			ImageType[NGP]=2;
			if(Shadow){
				sprintf(FName,"%ss.rlc",Name);
				f=RReset(FName);
				if(f!=INVALID_HANDLE_VALUE){
					RClose(f);
					ImageType[NGP]=3;
				};
			};
		}else{
			sprintf(FName,"Could not load %s.(gp|rlc)",Name);
			ErrM(FName);
		};
	};
	RLCNSpr[NGP]=GPNFrames[NGP];
	GPNames[NGP]=new char[strlen(Name)+1];
	strcpy(GPNames[NGP],Name);
	GPH[NGP]=NULL;
	GPLastTime[NGP]=0;
	GPSize[NGP]=0;
	NGP++;
	return NGP-1;
	*/
};

void GP_System::SetWhiteFont(int n)
{
	if (n >= NGP)return;
	ImageType[n] = (ImageType[n] & 7) | 16;
};

void GP_System::SetRedFont(int n)
{
	if (n >= NGP)return;
	ImageType[n] = (ImageType[n] & 7) | 31;
};

void GP_System::SetBlackFont(int n)
{
	if (n >= NGP)return;
	ImageType[n] = (ImageType[n] & 7);
};

void GP_System::SetOptionalColor(int n, int c)
{
	if (n >= NGP)return;
	ImageType[n] = (ImageType[n] & 7) | (c << 4);
};

void GP_System::UnLoadGP(int i)
{
	if (i >= NGP)return;
	UNITBL[i] = NULL;
	switch (ImageType[i] & 7)
	{
	case 1:
		if (GPSize[i])
		{
			LOADED -= GPSize[i];
			GPSize[i] = 0;
			FreeRefs(i);
			if (!Mapping[i])free(GPH[i]);
			GPH[i] = NULL;
			Mapping[i] = 0;
			free(ImLx[i]);
			free(ImLy[i]);
			ImLx[i] = NULL;
			ImLy[i] = NULL;
			free(CASHREF[i]);
			CASHREF[i] = NULL;
			if (ItDX[i])
			{
				free(ItDX[i]);
				free(ItLX[i]);
				ItDX[i] = NULL;
				ItLX[i] = NULL;
			};
		};
		GPNames[i][0] = '*';
		break;
	case 0:
	case 2:
		assert( false );
	};
	for (int j = NGP - 1; j >= 0 && GPNames[j][0] == '*'; j--)
	{
		free(GPNames[j]);
		NGP--;
	};
};

int GP_Header::GetLx()
{
	GP_Header* GPH = this;
	GP_Header* GPS = this;
	int DIFF = -1;
	int LxMax = 0;
	do
	{
		DIFF = GPH->NextPict;
		int Lxx = GPH->dx + GPH->Lx;
		if (Lxx > LxMax)LxMax = Lxx;
		__asm {
			mov eax, GPS
			add eax, DIFF
			mov GPH, eax
		};
	}
	while (DIFF != -1);
	return LxMax;
};

int GP_Header::GetLy()
{
	GP_Header* GPH = this;
	GP_Header* GPS = this;
	int DIFF = -1;
	int LyMax = 0;
	do
	{
		DIFF = GPH->NextPict;
		int Lyy = GPH->dy + GPH->Ly;
		if (Lyy > LyMax)LyMax = Lyy;
		__asm {
			mov eax, GPS
			add eax, DIFF
			mov GPH, eax
		};
	}
	while (DIFF != -1);
	return LyMax;
};

int GP_Header::GetDx()
{
	GP_Header* GPH = this;
	GP_Header* GPS = this;
	int DIFF = -1;
	int LxMax = 10000;
	do
	{
		DIFF = GPH->NextPict;
		int Lxx = GPH->dx;
		if (Lxx < LxMax)LxMax = Lxx;
		__asm {
			mov eax, GPS
			add eax, DIFF
			mov GPH, eax
		};
	}
	while (DIFF != -1);
	return LxMax;
};

int GP_Header::GetDy()
{
	GP_Header* GPH = this;
	GP_Header* GPS = this;
	int DIFF = -1;
	int LxMax = 10000;
	do
	{
		DIFF = GPH->NextPict;
		int Lxx = GPH->dy;
		if (Lxx < LxMax)LxMax = Lxx;
		__asm {
			mov eax, GPS
			add eax, DIFF
			mov GPH, eax
		};
	}
	while (DIFF != -1);
	return LxMax;
}

extern bool ProtectionMode;

int GP_System::PreLoadGPImage(char* Name)
{
	NewGPImage = false;

	int fidx = -1;

	//Search for existing name
	for (int i = 0; i < NGP; i++)
	{
		if (GPNames[i][0] == '*')
		{
			fidx = i;
		}

		if (!strcmp(Name, GPNames[i]))
		{
			//File already loaded
			return i;
		}
	}

	if (fidx == -1)
	{
		fidx = NGP;
	}

	UNITBL[fidx] = UFONTS.FindFont(Name);

	NewGPImage = true;

	ImageType[fidx] = 0;

	//Determine image type
	char FName[128];
	//GP?
	sprintf(FName, "%s.gp", Name);
	ResFile f = RReset(FName);
	if (f != INVALID_HANDLE_VALUE)
	{
		RSeek(f, 4);
		RBlockRead(f, &GPNFrames[fidx], 2);
		RClose(f);
		ImageType[fidx] = 1; //GP
	}
	else
	{
		//RLC?
		sprintf(FName, "%s.rlc", Name);
		f = RReset(FName);
		if (f != INVALID_HANDLE_VALUE)
		{
			RSeek(f, 4);
			RBlockRead(f, &GPNFrames[fidx], 2);
			RClose(f);
			ImageType[fidx] = 2; //RLC

			//RLC + Shadow?
			sprintf(FName, "%ss.rlc", Name);
			f = RReset(FName);
			if (f != INVALID_HANDLE_VALUE)
			{
				RClose(f);
				ImageType[fidx] = 3; //RLC + Shadow
			}
		}
		else
		{
			if (strstr(Name, "L_Mode\\"))
			{
				return 0;
			}
			sprintf(FName, "Could not load %s.(gp|rlc)", Name);
			ErrM(FName);
		}
	}

	RLCNSpr[fidx] = GPNFrames[fidx];

	GPNames[fidx] = new char[strlen(Name) + 1];
	strcpy(GPNames[fidx], Name);

	GPH[fidx] = nullptr;
	GPLastTime[fidx] = 0;
	GPSize[fidx] = 0;

	if (fidx == NGP)
	{
		NGP++;
	}

	char cc3[256];
	if (!strstr(Name, "\\"))
	{
		sprintf(cc3, "L_Mode\\%s", Name);
		GP_L_IDXS[fidx] = PreLoadGPImage(cc3);
	}

	return fidx;
}

#define GPX(x,y) ((GP_Header*)(int(x)+x##->##y))

bool GP_System::LoadGP(int i)
{
	char Name[128];
	if (i > NGP)
	{
		return false;
	}
	ResFile f;
	byte* mpptr = 0;
	switch (ImageType[i] & 7)
	{
	case 0:
		return false;
	case 1:
		if (GPH[i])return true;
		sprintf(Name, "%s.gp", GPNames[i]);
		//ProtectionMode=1;
		f = RReset(Name);
		//ProtectionMode=0;
		//assert(f!=INVALID_HANDLE_VALUE);
		if (f != INVALID_HANDLE_VALUE)
		{
			GPSize[i] = RFileSize(f);
			mpptr = GSFILES.gMapFile((LPGSCfile)f);
			lpGP_GlobalHeader lpGPH;
			if (mpptr)
			{
				GPH[i] = (lpGP_GlobalHeader)mpptr;
				lpGPH = GPH[i];
				Mapping[i] = 1;
			}
			else
			{
				LOADED += GPSize[i];
				GPH[i] = lpGP_GlobalHeader(new byte[GPSize[i]]);
				lpGPH = GPH[i];
				RBlockRead(f, GPH[i], GPSize[i]);
				Mapping[i] = 0;
			};
			//int* lpint=(int*)(&lpGPH->LGPH);
			ImLx[i] = new short[lpGPH->NPictures];
			ImLy[i] = new short[lpGPH->NPictures];
			int np = lpGPH->NPictures;
			for (int n = 0; n < np; n++)
			{
				//lpint[n]+=int(lpGPH);         //----------!new!----------//
				ImLx[i][n] = GPX( lpGPH, LGPH[n] )->GetLx();
				ImLy[i][n] = GPX( lpGPH, LGPH[n] )->GetLy();
			};
			RClose(f);
			sprintf(Name, "%s.gpi", GPNames[i]);
			f = RReset(Name);
			if (f != INVALID_HANDLE_VALUE)
			{
				ItDX[i] = (char*)malloc(lpGPH->NPictures);
				ItLX[i] = (char*)malloc(lpGPH->NPictures);
				RBlockRead(f, ItDX[i], lpGPH->NPictures);
				RBlockRead(f, ItLX[i], lpGPH->NPictures);
				RClose(f);
			};
			//Creating cash references
			int csz = np;
			for (int n = 0; n < np; n++)
			{
				GP_Header* LGP = GPX( lpGPH, LGPH[n] );
				GP_Header* LGP0 = LGP;
				int DIFF = -1;
				int LxMax = 0;
				do
				{
					DIFF = LGP->NextPict;
					csz++;
					__asm {
						mov eax, LGP0
						add eax, DIFF
						mov LGP, eax
					};
				}
				while (DIFF != -1);
			};
			CASHREF[i] = new DWORD[csz + 1];
			DWORD* CREF = CASHREF[i];
			memset(CREF, 0xFF, (csz + 1) << 2);
			csz = np;
			for (int n = 0; n < np; n++)
			{
				GP_Header* LGP = GPX( lpGPH, LGPH[n] );
				GP_Header* LGP0 = LGP;
				int DIFF = -1;
				int LxMax = 0;
				CREF[n] = csz;
				do
				{
					DIFF = LGP->NextPict;
					csz++;
					__asm {
						mov eax, LGP0;
						add eax, DIFF
						mov LGP, eax
					};
				}
				while (DIFF != -1);
			};
			return true;
		}
		else return false;
	case 2:
		{
			if (RLCImage[i])
			{
				return true;
			}
			sprintf(Name, "%s.rlc", GPNames[i]);
			LoadRLC(Name, RLCImage + i);
			ImLx[i] = new short[RLCImage[i]->SCount];
			ImLy[i] = new short[RLCImage[i]->SCount];
			for (int n = 0; n < RLCImage[i]->SCount; n++)
			{
				ImLx[i][n] = GetRLCWidth(RLCImage[i], n);
				ImLy[i][n] = GetRLCWidth(RLCImage[i], n);
			};
		};
		return true;
	case 3:
		{
			if (RLCImage[i])return true;
			sprintf(Name, "%s.rlc", GPNames[i]);
			LoadRLC(Name, RLCImage + i);
			ImLx[i] = new short[RLCImage[i]->SCount];
			ImLy[i] = new short[RLCImage[i]->SCount];
			for (int n = 0; n < RLCImage[i]->SCount; n++)
			{
				ImLx[i][n] = GetRLCWidth(RLCImage[i], n);
				ImLy[i][n] = GetRLCWidth(RLCImage[i], n);
			};
			sprintf(Name, "%ss.rlc", GPNames[i]);
			LoadRLC(Name, RLCShadow + i);
		};
		return true;
	default:
		return false;
	};
}

//cache format:
//DWORD Pack reference offset(PRefOfs)[=NULL if not assigned]
//DWORD Unpacked data size+8(UDataSize)

//Draw units in shadows and menu effects
void GP_ShowMaskedPict(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;

	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)
	{
		return;
	}

	//vertical clipping
	//top clipper
	int CDPOS = int(CData);

	if (y < WindY)
	{
		__asm
		{
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		}
	}
	//bottom clipper

	if (y + NLines > WindY1)
	{
		NLines = WindY1 - y + 1;
	}

	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;

	if (x >= WindX && x1 <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm
		{
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			mov al, cl
			shr cl, 2
			and al, 3
			rep movsd
			mov cl, al
			rep movsb
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			mov al, cl
			shr cl, 2
			and al, 3
			rep movsd
			mov cl, al
			rep movsb
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		}
	}
	else
	{
		//image must be clipped
		if (x < WindX)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm
			{
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				mov al, cl
				shr cl, 2
				and al, 3
				rep movsd
				mov cl, al
				rep movsb
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				mov al, cl
				shr cl, 2
				and al, 3
				rep movsd
				mov cl, al
				rep movsb
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			}
		}
		else
		{
			if (x1 > WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm
				{
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					mov al, cl
					shr cl, 2
					and al, 3
					rep movsd
					mov cl, al
					rep movsb
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					mov al, cl
					shr cl, 2
					and al, 3
					rep movsd
					mov cl, al
					rep movsb
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				}
			}
		}
	}
}

void GP_ShowMaskedPictInv(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX && x <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz LP0_1
			LP0_0 : movsb
			sub edi, 2
			dec cl
			jnz LP0_0
			LP0_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz LP1_1
			LP1_0 : movsb
			sub edi, 2
			dec cl
			jnz LP1_0
			LP1_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz LP2_1
				LP2_0 : movsb
				sub edi, 2
				dec cl
				jnz LP2_0
				LP2_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz LP3_1
				LP3_0 : movsb
				sub edi, 2
				dec cl
				jnz LP3_0
				LP3_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz LP4_1
					LP4_0 : movsb
					sub edi, 2
					dec cl
					jnz LP4_0
					LP4_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz LP5_1
					LP5_0 : movsb
					sub edi, 2
					dec cl
					jnz LP5_0
					LP5_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};
//key word: SHADRAW
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
//****																		****//
//****																		****//
//****				       S H A D O W   D R A W I N G						****//
//****																		****//
//****																		****//
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
void GP_ShowMaskedPictShadow(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x >= WindX && x1 <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz SH0_1
			xor eax, eax
			SH0_0 : mov al, [edi]
			mov al, [ebx + eax]
			mov[edi], al
			inc edi
			dec cl
			jnz SH0_0
			SH0_1 :
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz SH1_1
			xor eax, eax
			SH1_0 : mov al, [edi]
			mov al, [ebx + eax]
			mov[edi], al
			inc edi
			dec cl
			jnz SH1_0
			SH1_1 :
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x < WindX)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz SH2_1
				xor eax, eax
				SH2_0 : mov al, [edi]
				mov al, [ebx + eax]
				mov[edi], al
				inc edi
				dec cl
				jnz SH2_0
				SH2_1 :
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz SH3_1
				xor eax, eax
				SH3_0 : mov al, [edi]
				mov al, [ebx + eax]
				mov[edi], al
				inc edi
				dec cl
				jnz SH3_0
				SH3_1 :
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 > WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz SH4_1
					xor eax, eax
					SH4_0 : mov al, [edi]
					mov al, [ebx + eax]
					mov[edi], al
					inc edi
					dec cl
					jnz SH4_0
					SH4_1 :
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz SH5_1
					xor eax, eax
					SH5_0 : mov al, [edi]
					mov al, [ebx + eax]
					mov[edi], al
					inc edi
					dec cl
					jnz SH5_0
					SH5_1 :
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

void GP_ShowMaskedPictShadowInv(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX && x <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz SH0_1
			xor eax, eax
			SH0_0 : mov al, [edi]
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz SH0_0
			SH0_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz SH1_1
			xor eax, eax
			SH1_0 : mov al, [edi]
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz SH1_0
			SH1_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz SH2_1
				xor eax, eax
				SH2_0 : mov al, [edi]
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz SH2_0
				SH2_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz SH3_1
				xor eax, eax
				SH3_0 : mov al, [edi]
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz SH3_0
				SH3_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz SH4_1
					xor eax, eax
					SH4_0 : mov al, [edi]
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz SH4_0
					SH4_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz SH5_1
					xor eax, eax
					SH5_0 : mov al, [edi]
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz SH5_0
					SH5_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};
//key word: DARK_OVERPOINT
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
//****																		****//
//****																		****//
//****				       OVERPOINT   D R A W I N G						****//
//****																		****//
//****																		****//
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
void GP_ShowMaskedPictOverpoint(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	int OCNTR = y;
	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
		OCNTR = WindY;
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x >= WindX && x1 <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz SH0_1
			mov eax, OCNTR
			SH0_0 : add eax, edi
			test al, 1
			jz SH0_0A
			mov byte ptr[edi], 0
			SH0_0A : sub eax, edi
			inc edi
			dec cl
			jnz SH0_0
			SH0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			inc OCNTR
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz SH1_1
			mov eax, OCNTR
			SH1_0 : add eax, edi
			test al, 1
			jz SH1_0A
			mov byte ptr[edi], 0
			SH1_0A : sub eax, edi
			inc edi
			dec cl
			jnz SH1_0
			SH1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			inc OCNTR
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x < WindX)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz SH2_1
				mov eax, OCNTR
				SH2_0 : add eax, edi
				test al, 1
				jz SH2_0A
				mov byte ptr[edi], 0
				SH2_0A : sub eax, edi
				inc edi
				dec cl
				jnz SH2_0
				SH2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				inc OCNTR
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz SH3_1
				mov eax, OCNTR
				SH3_0 : add eax, edi
				test al, 1
				jz SH3_0A
				mov[edi], 0
				SH3_0A : sub eax, edi
				inc edi
				dec cl
				jnz SH3_0
				SH3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				inc OCNTR
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 > WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz SH4_1
					mov eax, OCNTR
					SH4_0 : add eax, edi
					test al, 1
					jz SH4_0A
					mov byte ptr[edi], 0
					SH4_0A : sub eax, edi
					inc edi
					dec cl
					jnz SH4_0
					SH4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					inc OCNTR
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz SH5_1
					mov eax, OCNTR
					SH5_0 : add eax, edi
					test al, 1
					jz SH5_0A
					mov byte ptr[edi], 0
					SH5_0A : sub eax, edi
					inc edi
					dec cl
					jnz SH5_0
					SH5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					inc OCNTR
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

void GP_ShowMaskedPictOverpointInv(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX && x <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz SH0_1
			xor eax, eax
			SH0_0 : mov al, [edi]
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz SH0_0
			SH0_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz SH1_1
			xor eax, eax
			SH1_0 : mov al, [edi]
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz SH1_0
			SH1_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz SH2_1
				xor eax, eax
				SH2_0 : mov al, [edi]
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz SH2_0
				SH2_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz SH3_1
				xor eax, eax
				SH3_0 : mov al, [edi]
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz SH3_0
				SH3_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz SH4_1
					xor eax, eax
					SH4_0 : mov al, [edi]
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz SH4_0
					SH4_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz SH5_1
					xor eax, eax
					SH5_0 : mov al, [edi]
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz SH5_0
					SH5_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

//key word: PALDRAW
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
//****																		****//
//****																		****//
//****						PALETTED IMAGE  DRAWING							****//
//****																		****//
//****																		****//
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
void GP_ShowMaskedPalPict(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x >= WindX && x1 <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz PAL0_1
			xor eax, eax
			PAL0_0 : lodsb
			mov al, [ebx + eax]
			stosb
			dec cl
			jnz PAL0_0
			PAL0_1 :
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz PAL1_1
			xor eax, eax
			PAL1_0 : lodsb
			mov al, [ebx + eax]
			stosb
			dec cl
			jnz PAL1_0
			PAL1_1 :
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x < WindX)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz PAL2_1
				xor eax, eax
				PAL2_0 : lodsb
				mov al, [ebx + eax]
				stosb
				dec cl
				jnz PAL2_0
				PAL2_1 :
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz PAL3_1
				xor eax, eax
				PAL3_0 : lodsb
				mov al, [ebx + eax]
				stosb
				dec cl
				jnz PAL3_0
				PAL3_1 :
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 > WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz PAL4_1
					xor eax, eax
					PAL4_0 : lodsb
					mov al, [ebx + eax]
					stosb
					dec cl
					jnz PAL4_0
					PAL4_1 :
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz PAL5_1
					xor eax, eax
					PAL5_0 : lodsb
					mov al, [ebx + eax]
					stosb
					dec cl
					jnz PAL5_0
					PAL5_1 :
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

void GP_ShowMaskedPalPictInv(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX && x <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz PAL0_1
			xor eax, eax
			PAL0_0 : lodsb
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz PAL0_0
			PAL0_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz PAL1_1
			xor eax, eax
			PAL1_0 : lodsb
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz PAL1_0
			PAL1_1 :
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz PAL2_1
				xor eax, eax
				PAL2_0 : lodsb
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz PAL2_0
				PAL2_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz PAL3_1
				xor eax, eax
				PAL3_0 : lodsb
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz PAL3_0
				PAL3_1 :
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz PAL4_1
					xor eax, eax
					PAL4_0 : lodsb
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz PAL4_0
					PAL4_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz PAL5_1
					xor eax, eax
					PAL5_0 : lodsb
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz PAL5_0
					PAL5_1 :
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};
//key word: MULTIDRAW
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
//****																		****//
//****																		****//
//****				      COMPOSED WITH BACKGROUND PICTURE					****//
//****																		****//
//****																		****//
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
void GP_ShowMaskedMultiPalPict(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x >= WindX && x1 <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz MPL0_1
			xor eax, eax
			MPL0_0 : mov al, [edi]
			mov ah, [esi]
			mov al, [ebx + eax]
			inc esi
			stosb
			dec cl
			jnz MPL0_0
			MPL0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz MPL1_1
			xor eax, eax
			MPL1_0 : mov al, [edi]
			mov ah, [esi]
			mov al, [ebx + eax]
			inc esi
			stosb
			dec cl
			jnz MPL1_0
			MPL1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x < WindX)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz MPL2_1
				xor eax, eax
				MPL2_0 : mov al, [edi]
				mov ah, [esi]
				mov al, [ebx + eax]
				inc esi
				stosb
				dec cl
				jnz MPL2_0
				MPL2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz MPL3_1
				xor eax, eax
				MPL3_0 : mov al, [edi]
				mov ah, [esi]
				mov al, [ebx + eax]
				inc esi
				stosb
				dec cl
				jnz MPL3_0
				MPL3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 > WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz MPL4_1
					xor eax, eax
					MPL4_0 : mov al, [edi]
					mov ah, [esi]
					mov al, [ebx + eax]
					inc esi
					stosb
					dec cl
					jnz MPL4_0
					MPL4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz MPL5_1
					xor eax, eax
					MPL5_0 : mov al, [edi]
					mov ah, [esi]
					mov al, [ebx + eax]
					inc esi
					stosb
					dec cl
					jnz MPL5_0
					MPL5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

void GP_ShowMaskedMultiPalPictInv(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX && x <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz MPL0_1
			xor eax, eax
			MPL0_0 : mov al, [edi]
			mov ah, [esi]
			mov al, [ebx + eax]
			inc esi
			mov[edi], al
			dec edi
			dec cl
			jnz MPL0_0
			MPL0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz MPL1_1
			xor eax, eax
			MPL1_0 : mov al, [edi]
			mov ah, [esi]
			mov al, [ebx + eax]
			inc esi
			mov[edi], al
			dec edi
			dec cl
			jnz MPL1_0
			MPL1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz MPL2_1
				xor eax, eax
				MPL2_0 : mov al, [edi]
				mov ah, [esi]
				mov al, [ebx + eax]
				inc esi
				mov[edi], al
				dec edi
				dec cl
				jnz MPL2_0
				MPL2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz MPL3_1
				xor eax, eax
				MPL3_0 : mov al, [edi]
				mov ah, [esi]
				mov al, [ebx + eax]
				inc esi
				mov[edi], al
				dec edi
				dec cl
				jnz MPL3_0
				MPL3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz MPL4_1
					xor eax, eax
					MPL4_0 : mov al, [edi]
					mov ah, [esi]
					mov al, [ebx + eax]
					inc esi
					mov[edi], al
					dec edi
					dec cl
					jnz MPL4_0
					MPL4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz MPL5_1
					xor eax, eax
					MPL5_0 : mov al, [edi]
					mov ah, [esi]
					mov al, [ebx + eax]
					inc esi
					mov[edi], al
					dec edi
					dec cl
					jnz MPL5_0
					MPL5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};
//key word: MULTIDRAW
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
//****																		****//
//****																		****//
//****			      COMPOSED WITH BACKGROUND PICTURE(TRANSPOSED)			****//
//****																		****//
//****																		****//
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
void GP_ShowMaskedMultiPalTPict(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x >= WindX && x1 <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz MPL0_1
			xor eax, eax
			MPL0_0 : mov ah, [edi]
			lodsb
			mov al, [ebx + eax]
			stosb
			dec cl
			jnz MPL0_0
			MPL0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz MPL1_1
			xor eax, eax
			MPL1_0 : mov ah, [edi]
			lodsb
			mov al, [ebx + eax]
			stosb
			dec cl
			jnz MPL1_0
			MPL1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x < WindX)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz MPL2_1
				xor eax, eax
				MPL2_0 : mov ah, [edi]
				lodsb
				mov al, [ebx + eax]
				stosb
				dec cl
				jnz MPL2_0
				MPL2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz MPL3_1
				xor eax, eax
				MPL3_0 : mov ah, [edi]
				lodsb
				mov al, [ebx + eax]
				stosb
				dec cl
				jnz MPL3_0
				MPL3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 > WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz MPL4_1
					xor eax, eax
					MPL4_0 : mov ah, [edi]
					lodsb
					mov al, [ebx + eax]
					stosb
					dec cl
					jnz MPL4_0
					MPL4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz MPL5_1
					xor eax, eax
					MPL5_0 : mov ah, [edi]
					lodsb
					mov al, [ebx + eax]
					stosb
					dec cl
					jnz MPL5_0
					MPL5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

void GP_ShowMaskedMultiPalTPictInv(int x, int y, GP_Header* Pic, byte* CData, byte* Encoder)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX && x <= WindX1)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, Encoder //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE : test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz MPL0_1
			xor eax, eax
			MPL0_0 : mov ah, [edi]
			lodsb
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz MPL0_0
			MPL0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz MPL1_1
			xor eax, eax
			MPL1_0 : mov ah, [edi]
			lodsb
			mov al, [ebx + eax]
			mov[edi], al
			dec edi
			dec cl
			jnz MPL1_0
			MPL1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, Encoder //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				xchg CLIP, edx
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz MPL2_1
				xor eax, eax
				MPL2_0 : mov ah, [edi]
				lodsb
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz MPL2_0
				MPL2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz MPL3_1
				xor eax, eax
				MPL3_0 : mov ah, [edi]
				lodsb
				mov al, [ebx + eax]
				mov[edi], al
				dec edi
				dec cl
				jnz MPL3_0
				MPL3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, Encoder //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					xchg CLIP, edx
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz MPL4_1
					xor eax, eax
					MPL4_0 : mov ah, [edi]
					lodsb
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz MPL4_0
					MPL4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz MPL5_1
					xor eax, eax
					MPL5_0 : mov ah, [edi]
					lodsb
					mov al, [ebx + eax]
					mov[edi], al
					dec edi
					dec cl
					jnz MPL5_0
					MPL5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};
//key word: WMIRROR
extern byte refl[3072];
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
//****																		****//
//****																		****//
//****									MIRROR								****//
//****																		****//
//****																		****//
//******************************************************************************//
//******************************************************************************//
//******************************************************************************//
void GP_ShowMaskedMirrorPict(int x, int y, GP_Header* Pic, byte* CData, int* WSHIFT)
{
	x += Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;

	if (y + NLines <= WindY || x + Lx <= WindX || x > WindX1 || y > WindY1)
		return;

	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm
		{
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x + Lx - 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x >= WindX + 10 && x1 <= WindX1 - 10)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, WSHIFT //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, [ebx]
			add ebx, 4
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE :
			test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			add edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz MPL0_1
			xor eax, eax
			MPL0_0 : mov al, [edi]
			sub al, 0xB0
			cmp al, 0xB
			jae MPL0_0A
			mov ah, al
			mov al, [esi]
			mov al, [refl + eax]
			mov[edi], al
			MPL0_0A : inc edi
			inc esi
			dec cl
			jnz MPL0_0
			MPL0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			add edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//
			//*****************************************//
			or cl, cl
			jz MPL1_1
			xor eax, eax

			MPL1_0 : mov al, [edi]
			sub al, 0xB0
			cmp al, 0xB
			jae MPL1_0A
			mov ah, al
			mov al, [esi]
			mov al, [refl + eax]
			mov[edi], al
			MPL1_0A : inc edi
			inc esi
			dec cl
			jnz MPL1_0

			MPL1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x < WindX + 10)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping LEFT edge						**//
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = WindX - x;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, WSHIFT //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				add edi, [ebx]
				xchg CLIP, edx
				add edi, ScrWidth
				sub edx, [ebx]
				add ebx, 4
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE : test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				add edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz MPL2_1
				xor eax, eax

				MPL2_0 : mov al, [edi]
				sub al, 0xB0
				cmp al, 12
				jae MPL2_0A
				mov ah, al
				mov al, [esi]
				mov al, [refl + eax]
				mov[edi], al
				MPL2_0A : inc edi
				inc esi
				dec cl
				jnz MPL2_0
				MPL2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				add edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				add edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				add edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//
				//*****************************************//
				or cl, cl
				jz MPL3_1
				xor eax, eax

				MPL3_0 : mov al, [edi]
				sub al, 0xB0
				cmp al, 12
				jae MPL3_0A
				mov ah, al
				mov al, [esi]
				mov al, [refl + eax]
				mov[edi], al
				MPL3_0A : inc edi
				inc esi
				dec cl
				jnz MPL3_0

				MPL3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 > WindX1 - 10)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   	clipping RIGHT edge			**************//
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = WindX1 - x + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, WSHIFT //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					mov ADDESI, 0
					add edi, [ebx]
					xchg CLIP, edx
					sub edx, [ebx]
					add ebx, 4
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					add edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz MPL4_1
					xor eax, eax

					MPL4_0 : mov al, [edi]
					sub al, 0xB0
					cmp al, 12
					jae MPL4_0A
					mov ah, al
					mov al, [esi]
					mov al, [refl + eax]
					mov[edi], al
					MPL4_0A : inc edi
					inc esi
					dec cl
					jnz MPL4_0

					MPL4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					add edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//
					//*****************************************//
					or cl, cl
					jz MPL5_1
					xor eax, eax

					MPL5_0 : mov al, [edi]
					sub al, 0xB0
					cmp al, 12
					jae MPL5_0A
					mov ah, al
					mov al, [esi]
					mov al, [refl + eax]
					mov[edi], al
					MPL5_0A : inc edi
					inc esi
					dec cl
					jnz MPL5_0

					jnz MPL5_0
					MPL5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

void GP_ShowMaskedMirrorPictInv(int x, int y, GP_Header* Pic, byte* CData, int* WSHIFT)
{
	x -= Pic->dx;
	y += Pic->dy;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	int ofst = int(Pic) + 23;
	if (y + NLines <= WindY || x - Lx >= WindX1 || x < WindX || y > WindY1)return;
	//vertical clipping
	//top clipper
	int CDPOS = int(CData);
	if (y < WindY)
	{
		__asm {
			mov ecx, WindY
			sub ecx, y
			sub NLines, ecx
			add y, ecx
			mov ebx, ofst
			xor eax, eax
			xor edx, edx
			NLINE : mov al, [ebx]
			test al, 128
			jz SIMPLE_LINE
			inc ebx
			mov ah, al
			and al, 31
			or al, al
			jz COMPLINE_1
			and ah, 32
			shr ah, 1
			COMPLINE_LOOP1:
			mov dl, [ebx]
			shr dl, 4
			or dl, ah
			add CDPOS, edx
			inc ebx
			dec al
			jnz COMPLINE_LOOP1
			COMPLINE_1 :
			xor eax, eax
			dec ecx
			jnz NLINE
			jmp END_VCLIP
			SIMPLE_LINE : inc ebx
			or eax, eax
			jz SIMPLINE_1
			SIMPLINE_LOOP1 :
			mov dl, [ebx + 1]
			add CDPOS, edx
			add ebx, 2
			dec al
			jnz SIMPLINE_LOOP1
			SIMPLINE_1 : dec ecx
			jnz NLINE
			END_VCLIP : mov ofst, ebx
		};
	};
	//bottom clipper
	if (y + NLines > WindY1)NLines = WindY1 - y + 1;
	//horisontal clipper
	int x1 = x - Lx + 1;
	int scrofs = int(ScreenPtr) + x + y * ScrWidth;
	int TEMP1;
	int LineStart;
	int CLIP;
	int CURCLIP;
	byte SPACE_MASK;
	byte PIX_MASK;
	if (x1 >= WindX + 10 && x <= WindX1 - 10)
	{
		//***********************************************************//
		//******************(((((((((((())))))))))))*****************//
		//**                      NO CLIPPING                      **//INVERTED
		//******************(((((((((((())))))))))))*****************//
		//***********************************************************//
		//no clipping
		__asm {
			pushf
			push esi
			push edi
			//initialisation
			mov edi, scrofs //edi-screen pointer
			mov esi, CDPOS //esi-points array
			mov ebx, WSHIFT //ebx-encoding pointer
			mov edx, ofst //edx-mask offset
			xor eax, eax
			xor ecx, ecx
			cld
			sub edi, ScrWidth
			mov LineStart, edi
			START_SCANLINE :
			mov edi, LineStart
			mov al, [edx]
			inc edx
			add edi, [ebx]
			add ebx, 4
			add edi, ScrWidth
			or al, al
			mov LineStart, edi
			jnz DRAW_LINE
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_LINE :
			test al, 128
			jnz DRAW_COMPLEX_LINE
			//drawing simple line
			or al, al
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_SIMPLE_SEGMENT :
			mov al, [edx] //empty space
			sub edi, eax
			mov cl, [edx + 1]
			add edx, 2
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz MPL0_1
			xor eax, eax
			MPL0_0 : mov al, [edi]
			sub al, 0xB0
			cmp al, 0x0B
			ja MPL0_0A
			mov ah, al
			mov al, [esi]
			mov al, [refl + eax]
			mov[edi], al
			MPL0_0A : inc esi
			dec edi
			dec cl
			jnz MPL0_0
			MPL0_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_SIMPLE_SEGMENT
			NEXT_SEGMENT :
			dec NLines
			jnz START_SCANLINE
			jmp END_DRAW_PICTURE
			DRAW_COMPLEX_LINE :
			//complex packed line
			mov SPACE_MASK, 0
			mov PIX_MASK, 0
			test al, 64
			jz DCL1
			mov SPACE_MASK, 16
			DCL1: test al, 32
			jz DCL2
			mov PIX_MASK, 16
			DCL2 : and al, 31
			jz NEXT_SEGMENT
			mov TEMP1, eax
			START_COMPLEX_SEGMENT :
			mov al, [edx]
			mov cl, al
			and al, 15
			or al, SPACE_MASK
			sub edi, eax
			shr cl, 4
			or cl, PIX_MASK
			inc edx
			//*****************************************//
			//put various code with encoding&loop there//INVERTED
			//*****************************************//
			or cl, cl
			jz MPL1_1
			xor eax, eax

			MPL1_0 : mov al, [edi]
			sub al, 0xB0
			cmp al, 0x0B
			ja MPL1_0A
			mov ah, al
			mov al, [esi]
			mov al, [refl + eax]
			mov[edi], al
			MPL1_0A : inc esi
			dec edi
			dec cl
			jnz MPL1_0

			MPL1_1 : xor eax, eax
			//*****************************************//
			//          end of variation zone          //INVERTED
			//*****************************************//
			dec TEMP1
			jnz START_COMPLEX_SEGMENT
			dec NLines
			jnz START_SCANLINE
			END_DRAW_PICTURE :
			//end code
			pop edi
			pop esi
			popf
		};
	}
	else
	{
		//image must be clipped
		if (x > WindX1 - 10)
		{
			//****************************************************************//
			//****************************************************************//
			//**                                                            **//
			//**					clipping RIGT(old-LEFT) edge						**//INVERTED
			//**                                                            **//
			//****************************************************************//
			//****************************************************************//
			CLIP = x - WindX1;
			__asm {
				pushf
				push esi
				push edi
				//initialisation
				mov edi, scrofs //edi-screen pointer
				mov esi, CDPOS //esi-points array
				mov ebx, WSHIFT //ebx-encoding pointer
				mov edx, ofst //edx-mask offset
				xor eax, eax
				xor ecx, ecx
				cld
				sub edi, ScrWidth
				mov LineStart, edi
				CLIPLEFT_START_SCANLINE :
				mov edi, LineStart
				mov al, [edx]
				inc edx
				add edi, [ebx]
				xchg CLIP, edx
				add edx, [ebx]
				add ebx, 4
				add edi, ScrWidth
				mov CURCLIP, edx
				or al, al
				xchg CLIP, edx
				mov LineStart, edi
				jnz CLIPLEFT_DRAW_LINE
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_LINE :
				test al, 128
				jnz CLIPLEFT_DRAW_COMPLEX_LINE
				//drawing simple line
				or al, al
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_SIMPLE_SEGMENT :
				mov al, [edx] //empty space
				sub edi, eax
				mov cl, [edx + 1]
				add edx, 2
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_SIMPLE_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_SIMPLE
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_SIMPLE :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_SIMPLE_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz MPL2_1
				xor eax, eax
				MPL2_0 : mov al, [edi]
				sub al, 0xB0
				cmp al, 0x0B
				ja MPL2_0A
				mov ah, al
				mov al, [esi]
				mov al, [refl + eax]
				mov[edi], al
				MPL2_0A : inc esi
				dec edi
				dec cl
				jnz MPL2_0
				MPL2_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_SIMPLE_SEGMENT
				CLIPLEFT_NEXT_SEGMENT :
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_DRAW_COMPLEX_LINE :
				//complex packed line
				mov SPACE_MASK, 0
				mov PIX_MASK, 0
				test al, 64
				jz CLIPLEFT_DCL1
				mov SPACE_MASK, 16
				CLIPLEFT_DCL1: test al, 32
				jz CLIPLEFT_DCL2
				mov PIX_MASK, 16
				CLIPLEFT_DCL2 : and al, 31
				jz CLIPLEFT_NEXT_SEGMENT
				mov TEMP1, eax
				CLIPLEFT_START_COMPLEX_SEGMENT :
				mov al, [edx]
				mov cl, al
				and al, 15
				or al, SPACE_MASK
				sub edi, eax
				shr cl, 4
				or cl, PIX_MASK
				inc edx
				sub CURCLIP, eax
				jle CLIPLEFT_DRAW_COMPLEX_SEGMENT
				cmp CURCLIP, ecx
				jl CLIPLEFT_PARTIAL_COMPLEX
				//full  clipping
				sub CURCLIP, ecx
				add esi, ecx
				sub edi, ecx
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				jmp CLIPLEFT_END_DRAW_PICTURE
				CLIPLEFT_PARTIAL_COMPLEX :
				//partial clipping
				add esi, CURCLIP
				sub edi, CURCLIP
				sub ecx, CURCLIP
				mov CURCLIP, -1
				CLIPLEFT_DRAW_COMPLEX_SEGMENT :
				//*****************************************//
				//put various code with encoding&loop there//INVERTED
				//*****************************************//
				or cl, cl
				jz MPL3_1
				xor eax, eax
				MPL3_0 : mov al, [edi]
				sub al, 0xB0
				cmp al, 0x0B
				ja MPL3_0A
				mov ah, al
				mov al, [esi]
				mov al, [refl + eax]
				mov[edi], al
				MPL3_0A : inc esi
				dec edi
				dec cl
				jnz MPL3_0
				MPL3_1 : xor eax, eax
				//*****************************************//
				//          end of variation zone          //INVERTED
				//*****************************************//
				dec TEMP1
				jnz CLIPLEFT_START_COMPLEX_SEGMENT
				dec NLines
				jnz CLIPLEFT_START_SCANLINE
				CLIPLEFT_END_DRAW_PICTURE :
				//end code
				pop edi
				pop esi
				popf
			};
		}
		else
		{
			if (x1 < WindX1 + 10)
			{
				//****************************************************************//
				//****************************************************************//
				//**      **     **                                **    **     **//
				//****************   clipping LEFT(old-RIGHT) edge  **************//INVERTED
				//**      **     **                                **    **     **//
				//****************************************************************//
				//****************************************************************//
				CLIP = x - WindX + 1;
				int ADDESI;
				__asm {
					pushf
					push esi
					push edi
					//initialisation
					mov edi, scrofs //edi-screen pointer
					mov esi, CDPOS //esi-points array
					mov ebx, WSHIFT //ebx-encoding pointer
					mov edx, ofst //edx-mask offset
					xor eax, eax
					xor ecx, ecx
					cld
					sub edi, ScrWidth
					mov LineStart, edi
					CLIPRIGHT_START_SCANLINE :
					mov edi, LineStart
					mov al, [edx]
					inc edx
					add edi, [ebx]
					mov ADDESI, 0
					xchg CLIP, edx
					add edx, [ebx]
					add ebx, 4
					add edi, ScrWidth
					mov CURCLIP, edx
					or al, al
					xchg CLIP, edx
					mov LineStart, edi
					jnz CLIPRIGHT_DRAW_LINE
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_LINE :
					test al, 128
					jnz CLIPRIGHT_DRAW_COMPLEX_LINE
					//drawing simple line
					or al, al
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_SIMPLE_SEGMENT :
					mov al, [edx] //empty space
					sub edi, eax
					mov cl, [edx + 1]
					add edx, 2
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_SIMPLE
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_SIMPLE_SEGMENT
					CLIPRIGHT_PARTIAL_SIMPLE :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_SIMPLE_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz MPL4_1
					xor eax, eax
					MPL4_0 : mov al, [edi]
					sub al, 0xB0
					cmp al, 0x0B
					ja MPL4_0A
					mov ah, al
					mov al, [esi]
					mov al, [refl + eax]
					mov[edi], al
					MPL4_0A : inc esi
					dec edi
					dec cl
					jnz MPL4_0
					MPL4_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_SIMPLE_SEGMENT
					CLIPRIGHT_NEXT_SEGMENT :
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_DRAW_COMPLEX_LINE :
					//complex packed line
					mov SPACE_MASK, 0
					mov PIX_MASK, 0
					test al, 64
					jz CLIPRIGHT_DCL1
					mov SPACE_MASK, 16
					CLIPRIGHT_DCL1: test al, 32
					jz CLIPRIGHT_DCL2
					mov PIX_MASK, 16
					CLIPRIGHT_DCL2 : and al, 31
					jz CLIPRIGHT_NEXT_SEGMENT
					mov TEMP1, eax
					CLIPRIGHT_START_COMPLEX_SEGMENT :
					mov al, [edx]
					mov cl, al
					and al, 15
					or al, SPACE_MASK
					sub edi, eax
					shr cl, 4
					or cl, PIX_MASK
					inc edx
					sub CURCLIP, eax
					jg CLIPRIGHT_TRY_TO_CLIP_COMPLEX
					//full clipping until the end of line
					add esi, ecx
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					jmp CLIPRIGHT_END_DRAW_PICTURE
					CLIPRIGHT_TRY_TO_CLIP_COMPLEX :
					cmp CURCLIP, ecx
					jl CLIPRIGHT_PARTIAL_COMPLEX
					sub CURCLIP, ecx
					jmp CLIPRIGHT_DRAW_COMPLEX_SEGMENT
					CLIPRIGHT_PARTIAL_COMPLEX :
					//partial clipping
					sub ecx, CURCLIP
					mov ADDESI, ecx
					mov ecx, CURCLIP
					mov CURCLIP, -1
					CLIPRIGHT_DRAW_COMPLEX_SEGMENT :
					//*****************************************//
					//put various code with encoding&loop there//INVERTED
					//*****************************************//
					or cl, cl
					jz MPL5_1
					xor eax, eax
					MPL5_0 : mov al, [edi]
					sub al, 0xB0
					cmp al, 0x0B
					ja MPL5_0A
					mov ah, al
					mov al, [esi]
					mov al, [refl + eax]
					mov[edi], al
					MPL5_0A : inc esi
					dec edi
					dec cl
					jnz MPL5_0
					MPL5_1 : xor eax, eax
					//*****************************************//
					//          end of variation zone          //INVERTED
					//*****************************************//
					add esi, ADDESI
					dec TEMP1
					jnz CLIPRIGHT_START_COMPLEX_SEGMENT
					dec NLines
					jnz CLIPRIGHT_START_SCANLINE
					CLIPRIGHT_END_DRAW_PICTURE :
					//end code
					pop edi
					pop esi
					popf
				};
			};
		};
	};
};

inline void NatUnpack(byte* Dest, byte* Src, int Len)
{
	__asm {
		push esi
		push edi
		pushf
		mov ecx, Len
		shr ecx, 2
		mov esi, Src
		mov edi, Dest
		cld
		jcxz NTZERO
		NTUNP1 : lodsb
		mov bl, al
		mov bh, al
		and bl, 00110000b
		and bh, 11000000b
		shr bl, 4
		shr bh, 6
		shl ebx, 16
		mov bl, al
		mov bh, al
		and bh, 00001100b
		and bl, 00000011b
		shr bh, 2
		mov eax, ebx
		stosd
		dec ecx
		jnz NTUNP1
		NTZERO : popf
		pop edi
		pop esi
	};
};

inline void GreyUnpack(byte* Dest, byte* Src, int Len)
{
	Len >>= 1;
	__asm {
		push esi
		push edi
		pushf
		mov ecx, Len
		shr ecx, 1
		mov esi, Src
		mov edi, Dest
		cld
		jcxz NTZERO
		NTUNP1 : lodsb
		mov bl, al
		mov bh, al
		and bl, 00001111b
		shl bl, 1
		and bh, 11110000b
		shr bh, 3
		mov ax, bx
		stosw
		dec ecx
		jnz NTUNP1
		NTZERO : popf
		pop edi
		pop esi
	};
};

inline void StdUnpack(byte* Dest, byte* Src, int Len, byte* Voc)
{
	//COUNTER++;
	byte Calc;
	__asm {
		push esi
		push edi
		pushf
		cld
		mov ebx, Src
		mov edi, Dest
		mov edx, Len
		xor eax, eax
		xor ecx, ecx
		Unpack_Loop_Start_Octant :
		mov ah, [ebx]
		mov Calc, 8
		inc ebx
		Unpack_Local_Start :
		test ah, 128
		jz O1UL_1
		mov cx, [ebx]
		mov esi, ecx
		shr ecx, 12
		and esi, 0xFFF
		add ecx, 3
		add esi, Voc
		add ebx, 2
		sub edx, ecx
		rep movsb
		jle Unpack_End
		shl ah, 1
		dec Calc
		jnz Unpack_Local_Start
		jmp Unpack_Loop_Start_Octant
		O1UL_1 : mov al, [ebx]
		mov[edi], al
		inc edi
		dec edx
		jz Unpack_End
		inc ebx
		shl ah, 1
		dec Calc
		jnz Unpack_Local_Start
		jmp Unpack_Loop_Start_Octant
		Unpack_End :
		popf
		pop edi
		pop esi
	};
};

inline void LZUnpack(byte* Dest, byte* Src, int Len)
{
	byte Calc;
	__asm {
		push esi
		push edi
		pushf
		cld
		mov ebx, Src
		mov edi, Dest
		mov edx, Len
		xor eax, eax
		xor ecx, ecx
		Unpack_Loop_Start_Octant :
		mov ah, [ebx]
		mov Calc, 8
		inc ebx
		Unpack_Local_Start :
		test ah, 1
		jz O1UL_1
		mov cx, [ebx]
		mov esi, ecx
		and esi, 0x1FFF
		neg esi
		shr ecx, 13
		add ecx, 3
		add esi, edi
		dec esi
		add ebx, 2
		sub edx, ecx
		rep movsb
		jle Unpack_End
		shr ah, 1
		dec Calc
		jnz Unpack_Local_Start
		jmp Unpack_Loop_Start_Octant
		O1UL_1 : mov al, [ebx]
		mov[edi], al
		inc edi
		dec edx
		jz Unpack_End
		inc ebx
		shr ah, 1
		dec Calc
		jnz Unpack_Local_Start
		jmp Unpack_Loop_Start_Octant
		Unpack_End :
		popf
		pop edi
		pop esi
	};
};
extern byte Bright[8192];


//Used for color masking of interface elements. Color offset = Nation byte * 4
byte NatPal[64] =
{
	0xD0, //red
	0xD1,
	0xD2,
	0xD3,
	0xD4, //blue
	0xD5,
	0xD6,
	0xD7,
	0xD8, //cyan
	0xD9,
	0xDA,
	0xDB,
	0xDC, //purple
	0xDD,
	0xDE,
	0xDF,
	0xE0, //orange
	0xE1,
	0xE2,
	0xE3,
	0xE4, //black
	0xE5,
	0xE6,
	0xE7,
	0xE8, //white
	0xE9,
	0xEA,
	0xEB,
	0xEC, //mercs
	0xED,
	0xEE,
	0xEF
};

extern byte Optional1[8192];
extern byte Optional2[8192];
extern byte Optional3[8192];

int startTrans = 0;

void GP_System::ShowGP( //IMPORTANT: show sprite with color masking
	int x, int y, int FileIndex, int SprIndex,
	byte Nation
)
{
	if (!(FileIndex < NGP && (SprIndex & 4095) < GPNFrames[FileIndex]))
	{
		return;
	}

	int imt = ImageType[FileIndex] >> 4;
	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		}

		if (RLCImage[FileIndex])
		{
			ShowRLCItem(x, y, RLCImage + FileIndex, SprIndex, Nation);
		}

		return;
	}

	if (!GPH[FileIndex])
	{
		LoadGP(FileIndex);
	}

	//TODO: Fix access violation (lpGH points to nothin', but is itself not NULL)
	GP_GlobalHeader* lpGH = GPH[FileIndex];

	/*
	if (nullptr == &lpGH->NPictures)
	{//A pitiful attempt to tackle the crash cause
		return;
	}
	*/

	int RSprIndex = SprIndex;
	if (SprIndex >= lpGH->NPictures)
	{
		RSprIndex &= 4095;
	}

	GP_Header* lpGP = GPX( lpGH, LGPH[RSprIndex] );
	GP_Header* lpGPCUR = lpGP;


	DWORD* PAK = CASHREF[FileIndex];
	PAK += PAK[RSprIndex];

	if (ItDX[FileIndex])
	{
		x -= ItDX[FileIndex][RSprIndex];
	}

	int DIFF = -1;
	int UnpackLen = lpGP->CData >> 14;
	int CDOffs = lpGP->CData & 16383;
	byte Optx = lpGP->Options;

	if (Optx & 64)
	{
		CDOffs += 16384;
	}

	if (Optx & 128)
	{
		CDOffs += 32768;
	}

	byte* PACKOFS = (byte*)(*PAK); //lpGP->Pack;

	if ((Optx & 63) == 43)
	{
		UnpackLen += 262144;
	}

	if ((Optx & 63) == 44)
	{
		UnpackLen += 262144 * 2;
	}

	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 18);
				INTV( PACKOFS ) = (int)(PAK); //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				StdUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen, ((byte*)lpGH) + lpGH->VocOffset);
			}

			if (SprIndex >= 4096)
				GP_ShowMaskedPictInv(x, y, lpGPCUR, PACKOFS, NULL);
			else
				GP_ShowMaskedPict(x, y, lpGPCUR, PACKOFS, NULL);

			//GP_ShowMaskedMirrorPict(x,y,lpGPCUR,PACKOFS,XShift);
			break;
		case 1: //National mask
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 8);
				INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				NatUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			}


			if (SprIndex >= 4096)
			{
				GP_ShowMaskedPalPictInv(
					x, y, lpGPCUR, PACKOFS,
					(byte*)(NatPal + (Nation << 2)) //IMPORTANT: color masking for interface
				);
			}
			else
			{
				GP_ShowMaskedPalPict(
					x, y, lpGPCUR, PACKOFS,
					(byte*)(NatPal + (Nation << 2)) //IMPORTANT: color masking for interface
				);
			}

			break;
		case 2: //transparent 1/4
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			break;
		case 3: //transparent 1/2
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			break;
		case 4: //transparent 3/4
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			break;
		case 5: //Shadow
			switch (imt)
			{
			case 1: //white
				GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, Bright + 7936);
				break;
			case 2: //red
				GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, yfog + 7936);
				break;
			case 3:
				GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, Optional1 + 7936);
				break;
			case 4:
				GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, Optional2 + 7936);
				break;
			case 5:
				GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, Optional3 + 7936);
				break;

			default:
				if (SprIndex >= 4096)
					GP_ShowMaskedPictShadowInv(x, y, lpGPCUR, NULL, fog + 4096);
				else
					GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, fog + 4096);
			};
			break;
		case 6: //AlphaRY
			if (SprIndex >= 4096)
				GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaR);
			else
				GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaR);
			break;
		case 7: //AlphaWB
			if (SprIndex >= 4096)
				GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaW);
			else
				GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaW);
			break;
		case 38:
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 8);
				INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				GreyUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			};
			switch (imt)
			{
			case 1: //white
				if (SprIndex < 4096 || RSprIndex == SprIndex)
					GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Bright + startTrans);
				else
					GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Bright + startTrans);
				break;
			case 2: //red
				if (SprIndex < 4096 || RSprIndex == SprIndex)
					GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, yfog + startTrans);
				else
					GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, yfog + startTrans);
				break;
			case 3:
				if (SprIndex < 4096 || RSprIndex == SprIndex)
					GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional1 + startTrans);
				else
					GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Optional1 + startTrans);
				break;
			case 4:
				if (SprIndex < 4096 || RSprIndex == SprIndex)
					GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional2 + startTrans);
				else
					GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Optional2);
				break;
			case 5:
				if (SprIndex < 4096 || RSprIndex == SprIndex)
					GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional3);
				else
					GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Optional3);
				break;

			default:
				if (SprIndex < 4096 || RSprIndex == SprIndex)
				{
					GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, fog + 1024);
				}
				else
				{
					GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, fog + 1024);
				}
				break;
			}
			break;

		case 39:
			switch (imt)
			{
			case 1: //white
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Bright + startTrans);
				else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog);
				break;
			case 2: //red
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog + startTrans);
				else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog);
				break;
			case 3:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1 + startTrans);
				else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1);
				break;
			case 4:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2 + startTrans);
				else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2);
				break;
			case 5:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
				else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
				break;
			case 6:
				//if(SprIndex<4096)GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
				//else GP_ShowMaskedMultiPalPictInv(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
				break;
			default:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
				else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
				break;
			};
			break;
		case 41:
			if (SprIndex >= 4096)GP_ShowMaskedPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, NULL);
			else GP_ShowMaskedPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, NULL);
			break;
		case 43:
		case 44:
		case 42:
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 18);
				INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				LZUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			};
			if (SprIndex >= 4096)GP_ShowMaskedPictInv(x, y, lpGPCUR, PACKOFS, NULL);
			else GP_ShowMaskedPict(x, y, lpGPCUR, PACKOFS, NULL);
			break;
		};
		DIFF = lpGPCUR->NextPict;
		__asm {
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		};
		UnpackLen = lpGPCUR->CData >> 14;
		CDOffs = lpGPCUR->CData & 16383;
		byte Optx = lpGPCUR->Options;
		if ((Optx & 63) == 43)UnpackLen += 262144;
		if ((Optx & 63) == 44)UnpackLen += 262144 * 2;
		if (Optx & 64)CDOffs += 16384;
		if (Optx & 128)CDOffs += 32768;
		PAK++;
		PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
	}
	while (DIFF != -1);
}

void GP_System::ShowGPLayers( //IMPORTANT: color masking for units and buildings
	int x, int y, int FileIndex, int SprIndex,
	byte Nation, int mask)
{
	if (!(FileIndex < NGP && (SprIndex & 4095) < GPNFrames[FileIndex]))
	{
		return;
	}

	int imt = ImageType[FileIndex] >> 4;
	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		}
		if (RLCImage[FileIndex])
		{
			ShowRLCItem(x, y, RLCImage + FileIndex, SprIndex, Nation);
		}
		return;
	}

	if (!GPH[FileIndex])
	{
		LoadGP(FileIndex);
	}

	GP_GlobalHeader* lpGH = GPH[FileIndex];
	GP_Header* lpGP = GPX( lpGH, LGPH[SprIndex & 4095] );
	GP_Header* lpGPCUR = lpGP;
	DWORD* PAK = CASHREF[FileIndex];
	PAK += PAK[SprIndex & 4095];
	int DIFF = -1;
	int UnpackLen = lpGP->CData >> 14;
	int CDOffs = lpGP->CData & 16383;
	byte Optx = lpGP->Options;
	if (Optx & 64)
	{
		CDOffs += 16384;
	}
	if (Optx & 128)
	{
		CDOffs += 32768;
	}
	byte* PACKOFS = (byte*)(*PAK); //lpGP->Pack;
	if ((Optx & 63) == 43)
	{
		UnpackLen += 262144;
	}
	if ((Optx & 63) == 44)
	{
		UnpackLen += 262144 * 2;
	}
	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
			if (mask & 1)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 18);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					StdUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen, ((byte*)lpGH) + lpGH->VocOffset);
				};
				if (mask & 512)
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictInv(x, y, lpGPCUR, PACKOFS, NULL);
					else GP_ShowMaskedPict(x, y, lpGPCUR, PACKOFS, NULL);
				}
				else
				{
					if (SprIndex >= 4096)GP_ShowMaskedMirrorPictInv(x, y, lpGPCUR, PACKOFS, XShift);
					else GP_ShowMaskedMirrorPict(x, y, lpGPCUR, PACKOFS, XShift);
				};
			};
			break;
		case 1: //National mask
			if (mask & 2)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 8);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					NatUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				if (SprIndex >= 4096)GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
				else GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
			};
			break;
		case 2: //transparent 1/4
			if (mask & 4)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
				else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			};
			break;
		case 3: //transparent 1/2
			if (mask & 8)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
				else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			};
			break;
		case 4: //transparent 3/4
			if (mask & 16)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			}
			else if (mask & 2048)
			{
				if (SprIndex >= 4096)GP_ShowMaskedPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, NULL);
				else GP_ShowMaskedPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, NULL);
			};
			break;
		case 5: //Shadow
			if (mask & 32)
			{
				if (mask & 64)
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictShadowInv(x, y, lpGPCUR, NULL, fog + 4096);
					else GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, fog + 4096);
				}
				else
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictOverpointInv(x, y, lpGPCUR, NULL, fog + 4096);
					else GP_ShowMaskedPictOverpoint(x, y, lpGPCUR, NULL, fog + 4096);
				};
			};
			break;
		case 6: //AlphaRY
			if (mask & 128)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaR);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaR);
			};
			break;
		case 7: //AlphaWB
			if (mask & 256)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaW);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, AlphaW);
			};
			break;
		case 38:
			if (mask & 1024)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 8);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					GreyUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				switch (imt)
				{
				case 1: //white
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Bright + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Bright);
					break;
				case 2: //red
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, yfog + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, yfog);
					break;
				case 3:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional1 + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Optional1);
					break;
				case 4:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional2 + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Optional2);
					break;
				case 5:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional3);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, Optional3);
					break;
				case 6:
					//if(SprIndex<4096)GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,PACKOFS,Optional4);
					//else GP_ShowMaskedMultiPalPictInv(x,y,lpGPCUR,PACKOFS,Optional4);
					break;
				default:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, fog + 1024);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, fog + 1024);
					break;
				};
				break;
			};
		case 39:
			if (mask & 1024)
			{
				switch (imt)
				{
				case 1: //white
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Bright + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog);
					break;
				case 2: //red
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog);
					break;
				case 3:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1 + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1);
					break;
				case 4:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2 + startTrans);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2);
					break;
				case 5:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
					break;
				case 6:
					//if(SprIndex<4096)GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
					//else GP_ShowMaskedMultiPalPictInv(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
					break;
				default:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
					else GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
					break;
				};
			};
			break;
		case 41:
			if (mask & 512)
			{
				if (SprIndex >= 4096)GP_ShowMaskedPictInv(x, y, lpGPCUR, PACKOFS, NULL);
				else GP_ShowMaskedPict(x, y, lpGPCUR, PACKOFS, NULL);
			}
			else
			{
				if (SprIndex >= 4096)
					GP_ShowMaskedMirrorPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, XShift);
				else
					GP_ShowMaskedMirrorPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, XShift);
			};
			break;
		case 43:
		case 44:
		case 42:
			if (mask & 1)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 18);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					LZUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				if (mask & 512)
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictInv(x, y, lpGPCUR, PACKOFS, NULL);
					else GP_ShowMaskedPict(x, y, lpGPCUR, PACKOFS, NULL);
				}
				else
				{
					if (SprIndex >= 4096)GP_ShowMaskedMirrorPictInv(x, y, lpGPCUR, PACKOFS, XShift);
					else GP_ShowMaskedMirrorPict(x, y, lpGPCUR, PACKOFS, XShift);
				};
			};
			break;
		};
		DIFF = lpGPCUR->NextPict;
		__asm {
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		};
		UnpackLen = lpGPCUR->CData >> 14;
		CDOffs = lpGPCUR->CData & 16383;
		byte Optx = lpGPCUR->Options;
		if ((Optx & 63) == 43)UnpackLen += 262144;
		if ((Optx & 63) == 44)UnpackLen += 262144 * 2;
		if (Optx & 64)CDOffs += 16384;
		if (Optx & 128)CDOffs += 32768;
		PAK++;
		PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
	}
	while (DIFF != -1);
};

void GP_System::ShowGPTransparent(int x, int y, int FileIndex, int SprIndex, byte Nation)
{
	if (!(FileIndex < NGP && (SprIndex & 4095) < GPNFrames[FileIndex]))
	{
		//		assert(FileIndex<NGP);
		//		assert((SprIndex&4095)<GPNFrames[FileIndex]);
		return;
	};
	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		};
		if (RLCImage[FileIndex])ShowRLCItem(x, y, RLCImage + FileIndex, SprIndex, Nation);

		return;
	};
	if (!GPH[FileIndex])
	{
		LoadGP(FileIndex);
	};
	GP_GlobalHeader* lpGH = GPH[FileIndex];
	GP_Header* lpGP = GPX( lpGH, LGPH[SprIndex & 4095] );
	GP_Header* lpGPCUR = lpGP;
	DWORD* PAK = CASHREF[FileIndex];
	PAK += PAK[SprIndex & 4095];
	int DIFF = -1;
	int UnpackLen = lpGP->CData >> 14;
	int CDOffs = lpGP->CData & 16383;
	byte Optx = lpGP->Options;
	if (Optx & 64)CDOffs += 16384;
	if (Optx & 128)CDOffs += 32768;
	byte* PACKOFS = (byte*)(*PAK); //lpGP->Pack;
	if ((Optx & 63) == 43)UnpackLen += 262144;
	if ((Optx & 63) == 44)UnpackLen += 262144 * 2;
	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 18);
				INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				StdUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen, ((byte*)lpGH) + lpGH->VocOffset);
			};
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, trans8);
			else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, trans8);
			break;
		case 1: //National mask
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 8);
				INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				NatUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			};
			if (SprIndex >= 4096)GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
			else GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
			break;
		case 3: //transparent 1/2
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			break;
		case 4: //transparent 3/4
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			break;
		case 5: //Shadow
			if (SprIndex >= 4096)GP_ShowMaskedPictShadowInv(x, y, lpGPCUR, NULL, fog + 4096);
			else GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, fog + 4096);
			break;
		case 41:
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
		case 43:
		case 44:
		case 42:
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 18);
				INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
				PACKOFS += 8;
				//lpGPCUR->Pack=PACKOFS;
				*PAK = (DWORD)PACKOFS;
				LZUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			};
			if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, trans8);
			else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, trans8);
			break;
		};
		DIFF = lpGPCUR->NextPict;
		__asm {
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		};
		UnpackLen = lpGPCUR->CData >> 14;
		CDOffs = lpGPCUR->CData & 16383;
		byte Optx = lpGPCUR->Options;

		if ((Optx & 63) == 43)UnpackLen += 262144;
		if ((Optx & 63) == 44)UnpackLen += 262144 * 2;

		if (Optx & 64)CDOffs += 16384;
		if (Optx & 128)CDOffs += 32768;
		PAK++;
		PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
	}
	while (DIFF != -1);
};

void GP_System::ShowGPTransparentLayers(int x, int y, int FileIndex, int SprIndex, byte Nation, int mask)
{
	if (!(FileIndex < NGP && (SprIndex & 4095) < GPNFrames[FileIndex]))
	{
		//		assert(FileIndex<NGP);
		//		assert((SprIndex&4095)<GPNFrames[FileIndex]);
		return;
	};
	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		};
		if (RLCImage[FileIndex])ShowRLCItem(x, y, RLCImage + FileIndex, SprIndex, Nation);

		return;
	};
	if (!GPH[FileIndex])
	{
		LoadGP(FileIndex);
	};
	GP_GlobalHeader* lpGH = GPH[FileIndex];
	GP_Header* lpGP = GPX( lpGH, LGPH[SprIndex & 4095] );
	GP_Header* lpGPCUR = lpGP;
	DWORD* PAK = CASHREF[FileIndex];
	PAK += PAK[SprIndex & 4095];
	int DIFF = -1;
	int UnpackLen = lpGP->CData >> 14;
	int CDOffs = lpGP->CData & 16383;
	byte Optx = lpGP->Options;
	if (Optx & 64)CDOffs += 16384;
	if (Optx & 128)CDOffs += 32768;
	byte* PACKOFS = (byte*)(*PAK); //lpGP->Pack;
	if ((Optx & 63) == 43)UnpackLen += 262144;
	if ((Optx & 63) == 44)UnpackLen += 262144 * 2;
	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
			if (mask & 1)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 18);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					StdUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen, ((byte*)lpGH) + lpGH->VocOffset);
				};
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, trans8);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, trans8);
			};
			break;
		case 1: //National mask
			if (mask & 2)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 8);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					NatUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				}

				if (SprIndex >= 4096)
				{
					GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
				}
				else
				{
					//GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
					GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
				}
			};
			break;
		case 3: //transparent 1/2
			if (mask & 8)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
				else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			};
			break;
		case 4: //transparent 3/4
			if (mask & 16)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			};
			break;
		case 5: //Shadow
			if (mask & 32)
			{
				if (mask & 64)
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictShadowInv(x, y, lpGPCUR, NULL, fog + 4096);
					else GP_ShowMaskedPictShadow(x, y, lpGPCUR, NULL, fog + 4096);
				}
				else
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictOverpointInv(x, y, lpGPCUR, NULL, fog + 4096);
					else GP_ShowMaskedPictOverpoint(x, y, lpGPCUR, NULL, fog + 4096);
				};
			};
			break;
		case 41: //storing
			if (mask & 1)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			};
			break;
		case 43:
		case 44:
		case 42:
			if (mask & 1)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 18);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					LZUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, PACKOFS, trans8);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, trans8);
			};
			break;
		};
		DIFF = lpGPCUR->NextPict;
		__asm {
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		};
		UnpackLen = lpGPCUR->CData >> 14;
		CDOffs = lpGPCUR->CData & 16383;
		byte Optx = lpGPCUR->Options;

		if ((Optx & 63) == 43)UnpackLen += 262144;
		if ((Optx & 63) == 44)UnpackLen += 262144 * 2;

		if (Optx & 64)CDOffs += 16384;
		if (Optx & 128)CDOffs += 32768;
		PAK++;
		PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
	}
	while (DIFF != -1);
};

void GP_System::FreeRefs(int FileIndex)
{
	//	assert(FileIndex<NGP);
	if ((ImageType[FileIndex] & 7) > 1 || !GPH[FileIndex])
	{
		return;
	}

	GP_GlobalHeader* lpGH = GPH[FileIndex];
	int NFR = GPNFrames[FileIndex];

	for (int SprIndex = 0; SprIndex < NFR; SprIndex++)
	{
		GP_Header* lpGP = GPX( lpGH, LGPH[SprIndex & 4095] );
		GP_Header* lpGPCUR = lpGP;
		DWORD* PAK = CASHREF[FileIndex];
		PAK += PAK[SprIndex & 4095];

		int DIFF = -1;

		int UnpackLen = lpGP->CData >> 14;
		int CDOffs = lpGP->CData & 16383;

		byte Optx = lpGP->Options;
		if (Optx & 64)
			CDOffs += 16384;
		if (Optx & 128)
			CDOffs += 32768;

		byte* PACKOFS = (byte*)(*PAK); //lpGP->Pack;

		do
		{
			if ((PACKOFS) && PACKOFS != NO_PACK)
			{
				INTV( PACKOFS - 8 ) = 0;
			}

			//lpGPCUR->Pack=NULL;

			*PAK = 0xFFFFFFFF;
			DIFF = lpGPCUR->NextPict;

			__asm
			{
				mov eax, lpGP
				add eax, DIFF
				mov lpGPCUR, eax
			}

			UnpackLen = lpGPCUR->CData >> 14;
			CDOffs = lpGPCUR->CData & 16383;

			byte Optx = lpGPCUR->Options;
			if (Optx & 64)
				CDOffs += 16384;
			if (Optx & 128)
				CDOffs += 32768;

			PAK++;
			PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
		}
		while (DIFF != -1);
	}
}

void GP_System::ShowGPPal( //IMPORTANT: color masking for buildings (only in placement mode) and ???
	int x, int y, int FileIndex, int SprIndex,
	byte Nation, byte* Table
)
{
	if ((SprIndex & 4095) >= GPNFrames[FileIndex])
		return;

	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		}
		if (RLCImage[FileIndex])
			ShowRLCItemPal(x, y, RLCImage + FileIndex, SprIndex, Table);


		return;
	}

	if (!GPH[FileIndex])
	{
		LoadGP(FileIndex);
	}

	GP_GlobalHeader* lpGH = GPH[FileIndex];
	GP_Header* lpGP = GPX( lpGH, LGPH[SprIndex & 4095] );
	GP_Header* lpGPCUR = lpGP;
	DWORD* PAK = CASHREF[FileIndex];
	PAK += PAK[SprIndex & 4095];
	int imt = ImageType[FileIndex] >> 4;
	int DIFF = -1;
	int UnpackLen = lpGP->CData >> 14;
	int CDOffs = lpGP->CData & 16383;
	byte Optx = lpGP->Options;
	if (Optx & 64)CDOffs += 16384;
	if (Optx & 128)CDOffs += 32768;
	byte* PACKOFS = (byte*)(*PAK);
	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 18);
				INTV( PACKOFS ) = (int)PAK;
				PACKOFS += 8;
				*PAK = (DWORD)PACKOFS;
				StdUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen, ((byte*)lpGH) + lpGH->VocOffset);
			}

			if (SprIndex >= 4096)
				GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, Table);
			else
				GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, Table);

			break;
		case 1: //National mask
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 8);
				INTV( PACKOFS ) = (int)PAK;
				PACKOFS += 8;
				*PAK = (DWORD)PACKOFS;
				NatUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			}

			if (SprIndex >= 4096)
			{
				GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
			}
			else
			{
				GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
			}

			break;
		case 3: //transparent 1/2
			if (SprIndex >= 4096)
				GP_ShowMaskedPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			else
				GP_ShowMaskedPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			break;
		case 4: //transparent 3/4
			if (SprIndex >= 4096)
				GP_ShowMaskedPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			else
				GP_ShowMaskedPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			break;
		case 5: //Shadow
			if (SprIndex >= 4096)
				GP_ShowMaskedPictShadowInv(x, y, lpGPCUR, nullptr, fog + 4096);
			else
				GP_ShowMaskedPictShadow(x, y, lpGPCUR, nullptr, fog + 4096);
			break;
		case 41:
			if (SprIndex >= 4096)
				GP_ShowMaskedPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			else
				GP_ShowMaskedPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			break;
		case 43:
		case 44:
		case 42:
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 18);
				INTV( PACKOFS ) = (int)PAK;
				PACKOFS += 8;
				*PAK = (DWORD)PACKOFS;
				LZUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			}

			if (SprIndex >= 4096)
			{
				GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, Table);
			}
			else
			{
				GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, Table);
			}
			break;
		case 38:
			if (PACKOFS == NO_PACK)
			{
				PACKOFS = GetCash(UnpackLen + 8);
				INTV( PACKOFS ) = (int)PAK;
				PACKOFS += 8;
				*PAK = (DWORD)PACKOFS;
				GreyUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
			}

			switch (imt)
			{
			case 1: //white
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Bright + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Bright);
				break;
			case 2: //red
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, yfog + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, yfog);
				break;
			case 3:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional1 + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional1);
				break;
			case 4:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional2 + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional2);
				break;
			case 5:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional3);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional3);
				break;
			default:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, fog + 1024);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, fog + 1024);
				break;
			};
			break;
		case 39:
			switch (imt)
			{
			case 1: //white
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Bright + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog);
				break;
			case 2: //red
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog);
				break;
			case 3:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1 + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1);
				break;
			case 4:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2 + startTrans);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2);
				break;
			case 5:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
				break;
			default:
				if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
				break;
			};
			break;
		};
		DIFF = lpGPCUR->NextPict;
		__asm {
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		};
		UnpackLen = lpGPCUR->CData >> 14;
		CDOffs = lpGPCUR->CData & 16383;
		byte Optx = lpGPCUR->Options;

		if ((Optx & 63) == 43)UnpackLen += 262144;
		if ((Optx & 63) == 44)UnpackLen += 262144 * 2;

		if (Optx & 64)CDOffs += 16384;
		if (Optx & 128)CDOffs += 32768;
		PAK++;
		PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
	}
	while (DIFF != -1);
}

void GP_System::ShowGPPalLayers( //IMPORTANT: color masking for buildings (only when selected) and ???
	int x, int y, int FileIndex, int SprIndex,
	byte Nation, byte* Table, int mask
)
{
	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		}
		if (RLCImage[FileIndex])ShowRLCItemPal(x, y, RLCImage + FileIndex, SprIndex, Table);

		return;
	}

	if (!GPH[FileIndex])
	{
		LoadGP(FileIndex);
	}

	int imt = ImageType[FileIndex] >> 4;
	GP_GlobalHeader* lpGH = GPH[FileIndex];
	GP_Header* lpGP = GPX( lpGH, LGPH[SprIndex & 4095] );
	GP_Header* lpGPCUR = lpGP;
	DWORD* PAK = CASHREF[FileIndex];
	PAK += PAK[SprIndex & 4095];
	int DIFF = -1;
	int UnpackLen = lpGP->CData >> 14;
	int CDOffs = lpGP->CData & 16383;
	byte Optx = lpGP->Options;
	if (Optx & 64)CDOffs += 16384;
	if (Optx & 128)CDOffs += 32768;
	byte* PACKOFS = (byte*)(*PAK); //lpGP->Pack;
	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
			if (mask & 1)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 18);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					StdUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen, ((byte*)lpGH) + lpGH->VocOffset);
				};
				if (SprIndex >= 4096)GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, Table);
				else GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, Table);
			};
			break;
		case 1: //National mask
			if (mask & 2)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 8);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					NatUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				if (SprIndex >= 4096)GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
				else GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, (byte*)(NatPal + (Nation << 2)));
			};
			break;


		case 2: //transparent 1/4
			if (mask & 4)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
				else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			};
			break;
		case 3: //transparent 1/2
			if (mask & 8)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalTPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
				else GP_ShowMaskedMultiPalTPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans8);
			};
			break;
		case 4: //transparent 3/4
			if (mask & 16)
			{
				if (SprIndex >= 4096)GP_ShowMaskedMultiPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
				else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, trans4);
			};
			break;

		case 5: //Shadow
			if (mask & 32)
			{
				if (mask & 64)
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictShadowInv(x, y, lpGPCUR, nullptr, fog + 4096);
					else GP_ShowMaskedPictShadow(x, y, lpGPCUR, nullptr, fog + 4096);
				}
				else
				{
					if (SprIndex >= 4096)GP_ShowMaskedPictOverpointInv(x, y, lpGPCUR, nullptr, fog + 4096);
					else GP_ShowMaskedPictOverpoint(x, y, lpGPCUR, nullptr, fog + 4096);
				};
			};
			break;
		case 41:
			if (SprIndex >= 4096)GP_ShowMaskedPalPictInv(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			else GP_ShowMaskedPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Table);
			break;
		case 43:
		case 44:
		case 42:
			if (mask & 1)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 18);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					LZUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				if (SprIndex >= 4096)GP_ShowMaskedPalPictInv(x, y, lpGPCUR, PACKOFS, Table);
				else GP_ShowMaskedPalPict(x, y, lpGPCUR, PACKOFS, Table);
			};
			break;
		case 38:
			if (mask & 1024)
			{
				if (PACKOFS == NO_PACK)
				{
					PACKOFS = GetCash(UnpackLen + 8);
					INTV( PACKOFS ) = (int)PAK; //&lpGPCUR->Pack);
					PACKOFS += 8;
					//lpGPCUR->Pack=PACKOFS;
					*PAK = (DWORD)PACKOFS;
					GreyUnpack(PACKOFS, ((byte*)lpGPCUR) + CDOffs, UnpackLen);
				};
				switch (imt)
				{
				case 1: //white
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Bright + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Bright);
					break;
				case 2: //red
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, yfog + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, yfog);
					break;
				case 3:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional1 + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional1);
					break;
				case 4:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional2 + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional2);
					break;
				case 5:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional3);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, Optional3);
					break;
				case 6:
					//if(SprIndex<4096)GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
					//else GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
					break;
				default:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, fog + 1024);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, PACKOFS, fog + 1024);
					break;
				};
				break;
			};
		case 39:
			if (mask & 1024)
			{
				switch (imt)
				{
				case 1: //white
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Bright + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog);
					break;
				case 2: //red
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, yfog);
					break;
				case 3:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1 + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional1);
					break;
				case 4:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2 + startTrans);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional2);
					break;
				case 5:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, Optional3);
					break;
				case 6:
					//if(SprIndex<4096)GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
					//else GP_ShowMaskedMultiPalPict(x,y,lpGPCUR,((byte*)lpGPCUR)+CDOffs,Optional4);
					break;
				default:
					if (SprIndex < 4096)GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
					else GP_ShowMaskedMultiPalPict(x, y, lpGPCUR, ((byte*)lpGPCUR) + CDOffs, fog + 1024);
					break;
				};
			};
			break;
		};
		DIFF = lpGPCUR->NextPict;
		__asm {
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		};
		UnpackLen = lpGPCUR->CData >> 14;
		CDOffs = lpGPCUR->CData & 16383;
		byte Optx = lpGPCUR->Options;

		if ((Optx & 63) == 43)UnpackLen += 262144;
		if ((Optx & 63) == 44)UnpackLen += 262144 * 2;

		if (Optx & 64)CDOffs += 16384;
		if (Optx & 128)CDOffs += 32768;
		PAK++;
		PACKOFS = (byte*)(*PAK); //lpGPCUR->Pack;
	}
	while (DIFF != -1);
};

void GP_System::ShowGPRedN(int x, int y, int FileIndex, int SprIndex, byte Nation, int N)
{
	ShowGPPal(x, y, FileIndex, SprIndex, Nation, yfog + (N * 256));
}

void GP_System::ShowGPDarkN(int x, int y, int FileIndex, int SprIndex, byte Nation, int N)
{
	ShowGPPal(x, y, FileIndex, SprIndex, Nation, wfog + (N << 8));
};

void GP_System::ShowGPDark(int x, int y, int FileIndex, int SprIndex, byte Nation)
{
	//	assert(FileIndex<NGP);
	ShowGPGrad(x, y, FileIndex, SprIndex, Nation, fog + 1024);
};

void GP_System::ShowGPFired(int x, int y, int FileIndex, int SprIndex, byte Nation)
{
	//	assert(FileIndex<NGP);
	ShowGPGrad(x, y, FileIndex, SprIndex, Nation, yfog + 1024);
};

void GP_System::ShowGPMutno(int x, int y, int FileIndex, int SprIndex, byte Nation)
{
	ShowGPGrad(x, y, FileIndex, SprIndex, Nation, wfog + 1024);
};

void GP_System::ShowGPGrad(int x, int y, int FileIndex, int SprIndex, byte Nation, byte* Table)
{
	//assert(FileIndex<NGP);
	if ((ImageType[FileIndex] & 7) > 1)
	{
		//RLC file
		if (!RLCImage[FileIndex])
		{
			LoadGP(FileIndex);
		};
		if (RLCImage[FileIndex])ShowRLCItemGrad(x, y, RLCImage + FileIndex, SprIndex, Table);

		return;
	};
	ShowGP(x, y, FileIndex, SprIndex, Nation);
};

GP_System GPS;

extern int mapx;
extern int mapy;
static int npp = 0;

void OvpBar1(int x, int y, int Lx, int Ly, byte c)
{
	int ofst = int(ScreenPtr) + x + y * ScrWidth;
	__asm {
		push edi
		mov eax, ScrWidth
		mov bl, c
		mov edi, ofst
		lppy : mov ecx, Lx
		push edi
		lppx : mov[edi], bl
		mov[edi + eax + 1], bl
		add edi, 2
		dec ecx
		jnz lppx
		pop edi
		add edi, eax
		add edi, eax
		dec Ly
		jnz lppy
		pop edi
	};
};

void OvpBar2(int x, int y, int Lx, int Ly, byte c)
{
	int ofst = int(ScreenPtr) + x + y * ScrWidth;
	__asm {
		push edi
		mov eax, ScrWidth
		mov bl, c
		mov edi, ofst
		lppy : mov ecx, Lx
		push edi
		lppx : mov[edi + 1], bl
		mov[edi + eax], bl
		add edi, 2
		dec ecx
		jnz lppx
		pop edi
		add edi, eax
		add edi, eax
		dec Ly
		jnz lppy
		pop edi
	};
};
extern byte WaterCost[65536];
//Waves
void ShowGradPicture(int x, int y, int Lx, int Ly,
                     int z1, int z2, int z3, int z4,
                     byte* Bitmap, int BMLx)
{
	BMLx -= Lx;
	int a = z1 << 16;
	int b = div( ( z2 - z1 ) << 16, Lx ).quot;
	int c = div( ( z3 - z1 ) << 16, Ly ).quot;
	int d = div( ( z4 + z1 - z2 - z3 ) << 16, Lx*Ly ).quot;
	int ofst = int(ScreenPtr) + x + y * ScrWidth;
	int addo = ScrWidth - Lx;

	__asm
	{
		push esi
		push edi
		pushf
		mov esi, Bitmap
		mov edi, ofst
		mov ebx, a
		mov edx, b
		lpp0 : mov cl, byte ptr Lx
		push ebx
		lpp1 : mov eax, ebx
		shr eax, 13
		mov ah, [edi]
		and al, 11111000b
		or al, [esi]
		add ebx, edx
		inc esi
		mov al, [WaterCost + eax]
		mov[edi], al
		inc edi
		dec cl
		jnz lpp1
		pop ebx
		add edx, d
		add ebx, c
		add esi, BMLx
		add edi, addo
		dec Ly
		jnz lpp0
		popf
		pop edi
		pop esi
	}
}

class WaterSpot
{
public:
	byte* Vertex;
	byte* Altitude;
	int Lx, Ly;

	WaterSpot();

	void WSpot(int x, int y, int r, int dr);
	void Process();
	void Draw(int x, int y);
};

WaterSpot::WaterSpot()
{
	Lx = 20;
	Ly = 20;
	int nn = Lx * Ly;

	Vertex = new byte[nn];
	Altitude = new byte[nn];

	memset(Vertex, 0, nn);
	memset(Altitude, 0, nn);

	for (int i = 0; i < nn; i++)
	{
		Altitude[i] = rand() & 7;

		if (rand() < 16384)
		{
			Altitude[i] |= 128;
		}
	}

	WSpot(32 * 8, 32 * 8, 10, 256);
}

void WaterSpot::WSpot(int xx, int yy, int r, int dr)
{
	int i = 0;
	for (int y = 0; y < Ly; y++)
	{
		for (int x = 0; x < Lx; x++)
		{
			int x1 = x * 32;
			int y1 = y * 32;
			int dd = int(sqrt((xx - x1) * (xx - x1) + (yy - y1) * (yy - y1)));
			int dp = 0;

			if (dd < r)
			{
				dp = 32;
			}

			if (dd > r + dr)
			{
				dp = 0;
			}

			if (dd > r && dd < r + dr)
			{
				dp = div( ( r + dr - dd ) * 32, dr ).quot;
			}

			Vertex[i] += dp;

			if (Vertex[i] > 30)
			{
				Vertex[i] = 30;
			}

			i++;
		}
	}
}

extern int tmtmt;

void WaterSpot::Process()
{
	int nn = Lx * Ly;
	for (int i = 0; i < nn; i++)
	{
		byte alt = Altitude[i];
		if (alt & 128)
		{
			if (alt < 128 + 4)alt++;
			else alt &= 127;
		}
		else
		{
			if (alt)
			{
				if (tmtmt & 1)alt--;
			}
			else alt |= 128;
		};
		Altitude[i] = alt;
	};
};
byte ttt[4096];

void WaterSpot::Draw(int xx, int yy)
{
	int i = 0;
	for (int y = 0; y < Ly - 1; y++)
	{
		for (int x = 0; x < Lx - 1; x++)
		{
			int z1 = int(Vertex[i]) + (Altitude[i] & 127) - 5;
			if (z1 < 0)z1 = 0;
			if (z1 > 30)z1 = 30;
			int z2 = int(Vertex[i + 1]) + (Altitude[i + 1] & 127) - 3;
			if (z2 < 0)z2 = 0;
			if (z2 > 30)z2 = 30;
			int z3 = int(Vertex[i + Lx]) + (Altitude[i + Lx] & 127) - 3;
			if (z3 < 0)z3 = 0;
			if (z3 > 30)z3 = 30;
			int z4 = int(Vertex[i + Lx + 1]) + (Altitude[i + Lx + 1] & 127) - 3;
			if (z4 < 0)z4 = 0;
			if (z4 > 30)z4 = 30;
			ShowGradPicture(xx + (x << 5), yy + (y << 4), 32, 16, z1, z2, z3, z4, ttt, 64);
			i++;
		};
		i++;
	};
};

WaterSpot WSPOT;

//Calls HandleWater()
void TestGP()
{
	HandleWater();
}

int XShift[512];

void InitXShift()
{
	int ttt = tmtmt;
	int ii;
	int T1 = 48 + 20;
	int T2 = 84 + 20;
	int T3 = 68 + 20;
	int T4 = 56 + 20;

	for (int i = 0; i < 64; i++)
	{
		ii = i << 2;
		XShift[i] = ((TSin[(((ii - ttt) * T1) / 16) & 255]
			+ TSin[(((ii - ttt) * T2) / 16) & 255]
			+ TSin[(((ii + ttt) * T3) / 16) & 255]
			+ TCos[(((ii + ttt) * T4) / 16) & 255]) * 4) >> 6;
	}

	memcpy(XShift + 64, XShift, 256);
	memcpy(XShift + 128, XShift + 64, 256);
	memcpy(XShift + 192, XShift + 128, 256);
	memcpy(XShift + 256, XShift + 192, 256);
	memcpy(XShift + 256 + 64, XShift + 256, 256);
	memcpy(XShift + 256 + 128, XShift + 256 + 64, 256);
	memcpy(XShift + 256 + 192, XShift + 256 + 128, 256);

	XShift[0] = 0;

	for (int i = 0; i < 512; i++)
	{
		XShift[i] = (XShift[i] * (i + 20)) >> 10;
	}

	for (int i = 511; i > 0; i--)
	{
		XShift[i] -= XShift[i - 1];
	}
}

LocalGP::LocalGP()
{
	GPID = -1;
	Uniq = false;
}

__declspec( dllexport ) LocalGP::LocalGP(char* Name)
{
	GPID = -1;
	Load(Name);
}

__declspec( dllexport ) LocalGP::~LocalGP()
{
	UnLoad();
}

void LocalGP::Load(char* Name)
{
	if (GPID != -1)
	{
		UnLoad();
	}
	GPID = GPS.PreLoadGPImage(Name);
	Uniq = NewGPImage;
}

void LocalGP::UnLoad()
{
	if (GPID != -1)
	{
		if (Uniq)
			GPS.UnLoadGP(GPID);
		GPID = -1;
		Uniq = 0;
	};
};
UNIFONTS UFONTS;

UNIFONTS::UNIFONTS()
{
	NFonts = 0;
	UFONTS = NULL;
};

UNIFONTS::~UNIFONTS()
{
	if (UFONTS)free(UFONTS);
	NFonts = 0;
	UFONTS = NULL;
};

void FONERR()
{
	MessageBox(NULL, "Invalid Unicode.dat", "ERROR!", 0);
	assert( 0 );
};

int UNI_LINEDLY1 = 0;
int UNI_LINEDY1 = 0;
int UNI_LINEDLY2 = 0;
int UNI_LINEDY2 = 0;
int UNI_HINTDY1 = 0;
int UNI_HINTDLY1 = 0;
int UNI_HINTDY2 = 0;
int UNI_HINTDLY2 = 0;

void UNIFONTS::LoadFonts()
{
	GFILE* F = Gopen("Unicode.dat", "r");
	if (!F)
	{
		return;
	}
	int N;
	int z = Gscanf(F, "%d", &N);
	if (1 == z)
	{
		for (int i = 0; i < N; i++)
		{
			char ccc[128];
			int z = Gscanf(F, "%s", ccc);
			GPS.PreLoadGPImage(ccc);
		}
	}
	z = Gscanf(F, "%d", &N);
	if (z == 1)
	{
		NFonts = 0;
		UFONTS = new OneUniFont[N];
		for (int i = 0; i < N; i++)
		{
			int q;
			int z = Gscanf(F, "%s%d", UFONTS[i].FontName, &q);
			if (z != 2)
			{
				FONERR();
			}
			if (q > 4)
			{
				FONERR();
			}
			UFONTS[i].UTBL.NTables = q;
			for (int p = 0; p < q; p++)
			{
				char FNM[64];
				int uc = 0;
				int z = Gscanf(F, "%d%d%d%d%d%d%s", &UFONTS[i].UTBL.USET[p].DX,
				               &UFONTS[i].UTBL.USET[p].DY,
				               &UFONTS[i].UTBL.USET[p].Start,
				               &UFONTS[i].UTBL.USET[p].NSymbols,
				               &UFONTS[i].UTBL.USET[p].GP_Start, &uc,
				               FNM);
				UFONTS[i].UTBL.USET[p].UseColor = uc;
				if (z != 7)
				{
					FONERR();
				}
				UFONTS[i].UTBL.USET[p].GPID = GPS.PreLoadGPImage(FNM);
			}
		}
		NFonts = N;
		z = Gscanf(F, "%d", &N);
		int v;
		char ccc[64];
		for (int i = 0; i < N; i++)
		{
			z = Gscanf(F, "%s%d", ccc, &v);
			if (z == 2)
			{
				if (!strcmp(ccc, "UNI_LINEDLY1"))
				{
					UNI_LINEDLY1 = v;
				}
				else if (!strcmp(ccc, "UNI_LINEDLY2"))
				{
					UNI_LINEDLY2 = v;
				}
				else if (!strcmp(ccc, "UNI_LINEDY1"))
				{
					UNI_LINEDY1 = v;
				}
				else if (!strcmp(ccc, "UNI_LINEDY2"))
				{
					UNI_LINEDY2 = v;
				}
				else if (!strcmp(ccc, "UNI_HINTDY1"))
				{
					UNI_HINTDY1 = v;
				}
				else if (!strcmp(ccc, "UNI_HINTDLY1"))
				{
					UNI_HINTDLY1 = v;
				}
				else if (!strcmp(ccc, "UNI_HINTDY2"))
				{
					UNI_HINTDY2 = v;
				}
				else if (!strcmp(ccc, "UNI_HINTDLY2"))
				{
					UNI_HINTDLY2 = v;
				};
			};
		};
	};
	Gclose(F);
}

UNICODETABLE* UNIFONTS::FindFont(char* Name)
{
	for (int i = 0; i < NFonts; i++)
	{
		if (!_stricmp(UFONTS[i].FontName, Name))
		{
			return &UFONTS[i].UTBL;
		}
	}
	return nullptr;
}

bool CheckInsideMask(GP_Header* Pic, int x, int y)
{
	x -= Pic->dx;
	y -= Pic->dy;
	if (x < 0 || y < 0)return false;
	int Lx = Pic->Lx;
	int NLines = Pic->NLines;
	if (x >= Pic->Lx)return false;
	if (y >= Pic->Ly)return false;
	int ofst = int(Pic) + 23;
	//skipping lines
	if (y > 0)
	{
		__asm
		{
			mov ecx, y
			mov ebx, ofst
			xor eax, eax
			START_SKIP_1 :
			mov al, [ebx]
			inc ebx
			test al, 128
			jz SIMPLE_LINE
			//complex line
			and al, 31
			add ebx, eax
			dec ecx
			jnz START_SKIP_1
			jmp END_SKIP
			SIMPLE_LINE :
			shl eax, 1
			add ebx, eax
			dec ecx
			jnz START_SKIP_1
			END_SKIP :
			mov ofst, ebx
		}
	}

	int SPACE_MASK = 0;
	int DATA_MASK = 0;

	__asm
	{
		mov ebx, ofst
		mov edx, x
		xor eax, eax

		mov al, [ebx]
		test al, 128
		jz SIMP_L_1
		mov SPACE_MASK, 0
		mov DATA_MASK, 0
		test al, 64
		jz MSK1
		mov SPACE_MASK, 16
		MSK1: test al, 32
		jz MSK2
		mov DATA_MASK, 16
		MSK2 : and al, 31
		inc ebx
		CHK_1 :
		mov cl, [ebx]
		and ecx, 15
		or ecx, SPACE_MASK
		sub edx, ecx
		jl NOT_INSIDE
		mov cl, [ebx]
		shr ecx, 4
		or ecx, DATA_MASK
		sub edx, ecx
		jl IS_INSIDE
		inc ebx
		dec al
		jnz CHK_1
		NOT_INSIDE :
		mov eax, 0
		jmp DO_END
		SIMP_L_1 :
		or al, al
		jz NOT_INSIDE
		inc ebx
		CHK_2 :
		mov cl, [ebx]
		inc ebx
		sub edx, ecx
		jl NOT_INSIDE
		mov cl, [ebx]
		inc ebx
		sub edx, ecx
		jl IS_INSIDE
		dec al
		jnz CHK_2
		mov eax, 0
		jmp DO_END
		IS_INSIDE :
		mov eax, 1
		DO_END :
	}
}

__declspec( dllexport ) bool CheckGP_Inside(int FileIndex, int SprIndex, int dx, int dy)
{
	if (!(FileIndex < GPS.NGP && (SprIndex & 4095) < GPS.GPNFrames[FileIndex]))
		return false;

	int imt = GPS.ImageType[FileIndex] >> 4;

	if ((GPS.ImageType[FileIndex] & 7) > 1)
		return false;

	if (!GPS.GPH[FileIndex])
	{
		GPS.LoadGP(FileIndex);
	}
	GP_GlobalHeader* lpGH = GPS.GPH[FileIndex];
	int RSprIndex = SprIndex;
	if (SprIndex >= lpGH->NPictures)RSprIndex &= 4095;
	GP_Header* lpGP = GPX( lpGH, LGPH[RSprIndex] );
	GP_Header* lpGPCUR = lpGP;
	int DIFF = -1;

	byte Optx = lpGP->Options;


	do
	{
		byte Opt = lpGPCUR->Options & 63;
		switch (Opt)
		{
		case 0: //standart packing
		case 1: //National mask
		case 3: //transparent 1/2
		case 4: //transparent 3/4
		case 43:
		case 44:
		case 42:
			if (SprIndex >= 4096)
			{
				if (CheckInsideMask(lpGPCUR, -dx, dy))
					return true;
			}
			else
			{
				if (CheckInsideMask(lpGPCUR, dx, dy))
					return true;
			}
			break;
		}

		DIFF = lpGPCUR->NextPict;
		__asm
		{
			mov eax, lpGP
			add eax, DIFF
			mov lpGPCUR, eax
		}
	}
	while (DIFF != -1);
	return false;
}

extern int mouseX;
extern int mouseY;

//Registring GP-frames
int N_GP_Reg = 0;
int Max_GP_Reg = 0;
struct GP_IMG;
GP_IMG* GP_Reg = nullptr;

struct GP_IMG
{
	word Index;
	word GPID;
	word Spr;
	short x;
	short y;
};

void Init_GP_IMG()
{
	N_GP_Reg = 0;
	Max_GP_Reg = 1024;
	GP_Reg = new GP_IMG[Max_GP_Reg];
}

void Clean_GP_IMG()
{
	N_GP_Reg = 0;
}

void RegisterVisibleGP(word Index, int FileIndex, int SprIndex, int x, int y)
{
	if (N_GP_Reg >= Max_GP_Reg)
	{
		Max_GP_Reg += 512;
		GP_Reg = (GP_IMG*)realloc(GP_Reg, Max_GP_Reg * sizeof GP_IMG);
	}

	GP_Reg[N_GP_Reg].GPID = FileIndex;
	GP_Reg[N_GP_Reg].Spr = SprIndex;
	GP_Reg[N_GP_Reg].x = x;
	GP_Reg[N_GP_Reg].y = y;
	GP_Reg[N_GP_Reg].Index = Index;
	N_GP_Reg++;
}

word CheckCoorInGP(int x, int y)
{
	for (int i = N_GP_Reg - 1; i >= 0; i--)
	{
		if (CheckGP_Inside(GP_Reg[i].GPID, GP_Reg[i].Spr, x - GP_Reg[i].x, y - GP_Reg[i].y))
		{
			return GP_Reg[i].Index;
		}
	}
	return 0xFFFF;
}

void GetRealGPSize(int File, int spr, int* dx, int* dy)
{
	*dx = 0;
	*dy = 0;

	if (!GPS.GPH[File])
	{
		return;
	}

	if (spr >= GPS.GPH[File]->NPictures)
	{
		return;
	}

	lpGP_GlobalHeader lpGPH = GPS.GPH[File];
	*dx = GPX( lpGPH, LGPH[spr] )->GetDx();
	*dy = GPX( lpGPH, LGPH[spr] )->GetDy();
}
