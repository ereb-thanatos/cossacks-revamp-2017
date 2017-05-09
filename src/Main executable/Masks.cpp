//Masking 2 or 3 textures in triangle
#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "multipl.h"
#include "fog.h"
#include "walls.h"
#include "Nature.h"
#include <time.h>
#include "Nucl.h"

#include "Megapolis.h"
#include "dialogs.h"
#include <assert.h>
#include <math.h>
#include "Masks.h"
byte ResultMask[MaskLx*256];
extern byte trans4[65536];
extern byte trans8[65536];
void CopyMaskedBitmap64(byte* Bits,int x,int y,void* Mask){
	__asm{
		push	esi
		push	edi
		pushf
		mov		esi,Mask
		mov		edi,Bits
		xor		ebx,ebx
		mov		bl,byte ptr x
		mov		bh,byte ptr y
		mov		cl,[esi+2]    //Number of strings
		add		esi,4
		xor		edx,edx     //ResulMask offset
rlclop1:
		mov		ch,[esi]    //number of RLE segments
		inc		esi
		or		ch,ch
		jz		endrlclop
rllpstrt:
		mov		ax,[esi]
		add		bl,al
		add		dl,al
lpp1:	and		bx,0011111100111111b
		mov		al,[edi+ebx]
		inc		ebx
		mov		[ResultMask+edx],al
		inc		edx
		dec		ah
		jnz		lpp1
		add		esi,2
		dec		ch
		jnz		rllpstrt
endrlclop:
		xor		dl,dl
		inc		bh
		inc		dh
		mov		bl,byte ptr x
		dec		cl
		jnz		rlclop1
		popf
		pop		edi
		pop		esi
	};
};
void CopyMaskedTransparentBitmap_8(byte* Bits,int x,int y,void* Mask){
	byte nstr;
	__asm{
		push	esi
		push	edi
		pushf
		mov		esi,Mask
		mov		edi,Bits
		xor		ebx,ebx
		mov		bl,byte ptr x
		mov		bh,byte ptr y
		mov		cl,[esi+2]    //Number of strings
		mov		nstr,cl
		add		esi,4
		xor		eax,eax
		xor		edx,edx     //ResulMask offset
rlclop1:
		mov		ch,[esi]    //number of RLE segments
		inc		esi
		or		ch,ch
		jz		endrlclop
rllpstrt:
		mov		ax,[esi]
		add		bl,al
		add		dl,al
		mov		cl,ah
lpp1:	and		bx,0011111100111111b
		mov		ah,[edi+ebx]
		mov		al,[ResultMask+edx]
		mov		al,[trans8+eax]
		inc		ebx
		mov		[ResultMask+edx],al
		inc		edx
		dec		cl
		jnz		lpp1
		add		esi,2
		dec		ch
		jnz		rllpstrt
endrlclop:
		xor		dl,dl
		inc		bh
		inc		dh
		mov		bl,byte ptr x
		dec		nstr
		jnz		rlclop1
		popf
		pop		edi
		pop		esi
	};
};
void CopyMaskedTransparentBitmap_4(byte* Bits,int x,int y,void* Mask){
	byte nstr;
	__asm{
		push	esi
		push	edi
		pushf
		mov		esi,Mask
		mov		edi,Bits
		xor		ebx,ebx
		mov		bl,byte ptr x
		mov		bh,byte ptr y
		mov		cl,[esi+2]    //Number of strings
		mov		nstr,cl
		add		esi,4
		xor		eax,eax
		xor		edx,edx     //ResulMask offset
rlclop1:
		mov		ch,[esi]    //number of RLE segments
		inc		esi
		or		ch,ch
		jz		endrlclop
rllpstrt:
		mov		ax,[esi]
		add		bl,al
		add		dl,al
		mov		cl,ah
lpp1:	and		bx,0011111100111111b
		mov		ah,[edi+ebx]
		mov		al,[ResultMask+edx]
		mov		al,[trans4+eax]
		inc		ebx
		mov		[ResultMask+edx],al
		inc		edx
		dec		cl
		jnz		lpp1
		add		esi,2
		dec		ch
		jnz		rllpstrt
endrlclop:
		xor		dl,dl
		inc		bh
		inc		dh
		mov		bl,byte ptr x
		dec		nstr
		jnz		rlclop1
		popf
		pop		edi
		pop		esi
	};
};
void CopyMaskedTransparentBitmap_12(byte* Bits,int x,int y,void* Mask){
	byte nstr;
	__asm{
		push	esi
		push	edi
		pushf
		mov		esi,Mask
		mov		edi,Bits
		xor		ebx,ebx
		mov		bl,byte ptr x
		mov		bh,byte ptr y
		mov		cl,[esi+2]    //Number of strings
		mov		nstr,cl
		add		esi,4
		xor		eax,eax
		xor		edx,edx     //ResulMask offset
rlclop1:
		mov		ch,[esi]    //number of RLE segments
		inc		esi
		or		ch,ch
		jz		endrlclop
rllpstrt:
		mov		ax,[esi]
		add		bl,al
		add		dl,al
		mov		cl,ah
lpp1:	and		bx,0011111100111111b
		mov		al,[edi+ebx]
		mov		ah,[ResultMask+edx]
		mov		al,[trans4+eax]
		inc		ebx
		mov		[ResultMask+edx],al
		inc		edx
		dec		cl
		jnz		lpp1
		add		esi,2
		dec		ch
		jnz		rllpstrt
endrlclop:
		xor		dl,dl
		inc		bh
		inc		dh
		mov		bl,byte ptr x
		dec		nstr
		jnz		rlclop1
		popf
		pop		edi
		pop		esi
	};
};
extern RLCTable SimpleMaskA;
extern RLCTable SimpleMaskB;
extern RLCTable SimpleMaskC;
extern RLCTable SimpleMaskD;
void CopyMaskedBitmap(byte* Bits,int x,int y,int MaskID){
	CopyMaskedBitmap64(Bits,x,y,   (void*)(SimpleMaskA->OfsTable[MaskID]));
	CopyMaskedTransparentBitmap_4(Bits,x,y,(void*)(SimpleMaskB->OfsTable[MaskID]));
	CopyMaskedTransparentBitmap_8(Bits,x,y, (void*)(SimpleMaskC->OfsTable[MaskID]));
	CopyMaskedTransparentBitmap_12(Bits,x,y, (void*)(SimpleMaskD->OfsTable[MaskID]));
};
//  0
//	|\
//	|  \
//	|    \
//	|    /
//	|  /
//	|/ 
//Creates triangle (Type1) with bitmap
void FastCreateMaskedBitmap64_1(byte* Bits,int x,int y){
	int tmedi;
	__asm{
		push	esi
		push	edi
		pushf
		mov		al,64
		xor		ebx,ebx
		mov		bl,byte ptr x
		mov		bh,byte ptr y
		and		ebx,0011111100111111b
		sub		al,bl       //Max x in trapezoid
		xor		ecx,ecx
		mov		ah,16//32
		mov		edi,offset ResultMask
		mov		dl,2
		cld
		mov		tmedi,edi
StartTrapezoid1:
		and     ebx,0011111100111111b
		mov		esi,Bits
		add		esi,ebx
		cmp		dl,al
		ja		RightPart1
		mov		cl,dl
		shr		cl,2
		rep		movsd
		mov		cl,dl
		and		cl,3
		rep     movsb
		add		tmedi,256
		mov		edi,tmedi
		add		dl,2
		inc		bh
		dec		ah
		jnz		StartTrapezoid1
		jmp		Trap2
RightPart1:
		mov		cl,al
		shr		cl,2
		rep		movsd
		mov		cl,al
		and		cl,3
		rep		movsb
		sub		esi,64
		mov		cl,dl
		sub		cl,al
		mov		dh,cl
		shr		cl,2
		rep		movsd
		mov		cl,dh
		and		cl,3
		rep		movsb
		add		dl,2
		add		tmedi,256
		mov		edi,tmedi
		inc		bh
		dec		ah
		jnz		StartTrapezoid1
Trap2:  sub		dl,2
		mov		ah,16
DoTrapezoid2:
		and     ebx,0011111100111111b
		mov		esi,Bits
		add		esi,ebx
		cmp		dl,al
		ja		RightPart2
		mov		cl,dl
		shr		cl,2
		rep		movsd
		mov		cl,dl
		and		cl,3
		rep     movsb
		add		tmedi,256
		mov		edi,tmedi
		sub		dl,2
		inc		bh
		dec		ah
		jnz		DoTrapezoid2
		jmp		EndTrapezoid
RightPart2:
		mov		cl,al
		shr		cl,2
		rep		movsd
		mov		cl,al
		and		cl,3
		rep		movsb
		sub		esi,64
		mov		cl,dl
		sub		cl,al
		mov		dh,cl
		shr		cl,2
		rep		movsd
		mov		cl,dh
		and		cl,3
		rep		movsb
		sub		dl,2
		add		tmedi,256
		mov		edi,tmedi
		inc		bh
		dec		ah
		jnz		DoTrapezoid2
EndTrapezoid:
		popf
		pop		edi
		pop		esi
	};
};
//      /|
//    /  |
// 0/    |
//  \    |
//    \  |
//      \|
void FastCreateMaskedBitmap64_2(byte* Bits,int x,int y){
	int tmedi;
	__asm{
		push	esi
		push	edi
		pushf
		xor		ebx,ebx
		mov		bl,byte ptr x
		mov		bh,byte ptr y
		add		bl,32//64
		sub		bh,16//31
		and		ebx,0011111100111111b
		mov		al,bl       //Max x in trapezoid
		xor		ecx,ecx
		mov		ah,16//32
		mov		edi,offset ResultMask
		add		edi,32//64
		mov		dl,2
		std
		mov		tmedi,edi
StartTrapezoid1:
		and     ebx,0011111100111111b
		mov		esi,Bits
		add		esi,ebx
		cmp		dl,al
		ja		RightPart1
		mov		cl,dl
		dec		esi
		dec		edi
		cmp		cl,4
		jb		uue1
		shr		cl,2
		sub		esi,3
		sub		edi,3
		rep		movsd
		add		esi,3
		add		edi,3
uue1:
		mov		cl,dl
		and		cl,3
		rep     movsb
		add		tmedi,256
		mov		edi,tmedi
		inc		bh
		add		dl,2
		dec		ah
		jnz		StartTrapezoid1
		jmp		Trap2
RightPart1:
		mov		cl,al
		dec		esi
		dec		edi
		cmp		cl,4
		jb		uu3
		sub		esi,3
		sub		edi,3
		shr		cl,2
		rep		movsd
		add		esi,3
		add		edi,3
uu3:
		mov		cl,al
		and		cl,3
		rep		movsb
		add		esi,64
		mov		cl,dl
		sub		cl,al
		mov		dh,cl
		cmp		cl,4
		jb		uu31
		shr		cl,2
		sub		esi,3
		sub		edi,3
		rep		movsd
		add		esi,3
		add		edi,3
uu31:
		mov		cl,dh
		and		cl,3
		rep		movsb
		add		dl,2
		add		tmedi,256
		mov		edi,tmedi
		inc		bh
		dec		ah
		jnz		StartTrapezoid1
Trap2:	sub		dl,2
		mov		ah,16
DoTrapezoid2:
		and     ebx,0011111100111111b
		mov		esi,Bits
		add		esi,ebx
		cmp		dl,al
		ja		RightPart2
		mov		cl,dl
		dec		esi
		dec		edi
		cmp		cl,4
		jb		uue1x
		shr		cl,2
		sub		esi,3
		sub		edi,3
		rep		movsd
		add		esi,3
		add		edi,3
uue1x:
		mov		cl,dl
		and		cl,3
		rep     movsb
		add		tmedi,256
		mov		edi,tmedi
		inc		bh
		sub		dl,2
		dec		ah
		jnz		DoTrapezoid2
		jmp		EndTrapezoid
RightPart2:
		mov		cl,al
		dec		esi
		dec		edi
		cmp		cl,4
		jb		uu3x
		sub		esi,3
		sub		edi,3
		shr		cl,2
		rep		movsd
		add		esi,3
		add		edi,3
uu3x:
		mov		cl,al
		and		cl,3
		rep		movsb
		add		esi,64
		mov		cl,dl
		sub		cl,al
		mov		dh,cl
		cmp		cl,4
		jb		uu31x
		shr		cl,2
		sub		esi,3
		sub		edi,3
		rep		movsd
		add		esi,3
		add		edi,3
uu31x:
		mov		cl,dh
		and		cl,3
		rep		movsb
		sub		dl,2
		add		tmedi,256
		mov		edi,tmedi
		inc		bh
		dec		ah
		jnz		DoTrapezoid2
EndTrapezoid:
		popf
		pop		edi
		pop		esi
	};
};
int GetBmOfst(int i){
	return ((i&3)<<6)+((i>>2)<<(8+6));
};
// 1(x0,y0)
//  |\
//  |  \ s1
//s3|    \ 2
//  |    /
//  |  / s2
//  |/
// 3
//
// bm1,bm2,bm3-numbers of bitmaps in BitmapArray
// if bm1<bm2 then bm2 is over bm1
// s1 (0..2) - section of 1-2 
// s2 (0..2) - section of 2-3 
// s3 (0..2) - section of 3-1 
/*void PrepareIntersection1(int bm1,int bm2,int bm3,
						  int x0,int y0,
						  int s1,int s2,int s3,
						  RLCTable Masks,byte* BitmapsArray){
	if(bm1==bm2){
		if(bm3<bm2){
			//1,2 over 3 - inverse mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm1),x0,y0);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[27+(2-s2)*3+s3]));
			return;
		}else{
			//3 over 1,2 - normal mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm1),x0,y0);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[18+(2-s2)*3+s3]));
			return;
		};
	};
	if(bm2==bm3){
		if(bm1<bm3){
			//2,3 over 1 - inverse mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
				(void*)(Masks->OfsTable[45+(2-s3)*3+s1]));
			return;
		}else{
			//1 over 2,3 - normal mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
				(void*)(Masks->OfsTable[36+(2-s3)*3+s1]));
			return;
		};
	};
	if(bm1==bm3){
		if(bm2<bm3){
			//1,3 over 2 - inverse mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[9+(2-s1)*3+s2]));
			return;
		}else{
			//2 over 1,3 - normal mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[(2-s1)*3+s2]));
			return;
		};
	};
	//Triple intersections
	if(bm1<bm2&&bm1<bm3){
		FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm1),x0,y0);
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[(2-s1)*3+s2]));
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[18+(2-s2)*3+s3]));
		return;
	};
	if(bm2<bm1&&bm2<bm3){
		FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm2),x0,y0);
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
				(void*)(Masks->OfsTable[36+(2-s3)*3+s1]));
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[18+(2-s2)*3+s3]));
		return;
	};
	FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
	CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
			(void*)(Masks->OfsTable[36+(2-s3)*3+s1]));
	CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
			(void*)(Masks->OfsTable[(2-s1)*3+s2]));
};
//
//                1
//         s1   /|
//            /  |
//        2 /    | s3
// (x01,y0) \    |
//            \  |
//         s2   \|
//                3
void PrepareIntersection2(int bm1,int bm2,int bm3,
						  int x0,int y01,
						  int s1,int s2,int s3,
						  RLCTable Masks,byte* BitmapsArray){
	int y0=y01-31;
	if(bm1==bm2){
		if(bm3<bm2){
			//1,2 over 3 - inverse mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm1),x0,y01);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[54+27+(2-s2)*3+s3]));
			return;
		}else{
			//3 over 1,2 - normal mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm1),x0,y01);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[54+18+(2-s2)*3+s3]));
			return;
		};
	};
	if(bm2==bm3){
		if(bm1<bm3){
			//2,3 over 1 - inverse mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
				(void*)(Masks->OfsTable[54+45+(2-s3)*3+s1]));
			return;
		}else{
			//1 over 2,3 - normal mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
				(void*)(Masks->OfsTable[54+36+(2-s3)*3+s1]));
			return;
		};
	};
	if(bm1==bm3){
		if(bm2<bm3){
			//1,3 over 2 - inverse mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[54+9+(2-s1)*3+s2]));
			return;
		}else{
			//2 over 1,3 - normal mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[54+(2-s1)*3+s2]));
			return;
		};
	};
	//Triple intersections
	if(bm1<bm2&&bm1<bm3){
		FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm1),x0,y01);
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[54+(2-s1)*3+s2]));
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[54+18+(2-s2)*3+s3]));
		return;
	};
	if(bm2<bm1&&bm2<bm3){
		FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm2),x0,y01);
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
				(void*)(Masks->OfsTable[54+36+(2-s3)*3+s1]));
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm3),x0,y0,
				(void*)(Masks->OfsTable[54+18+(2-s2)*3+s3]));
		return;
	};
	FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
	CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm1),x0,y0,
			(void*)(Masks->OfsTable[54+36+(2-s3)*3+s1]));
	CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
			(void*)(Masks->OfsTable[54+(2-s1)*3+s2]));	
};*/
// 1(x0,y0)
//  |\
//  |  \ s1
//s3|    \ 2
//  |    /
//  |  / s2
//  |/
// 3
//
// bm1,bm2,bm3-numbers of bitmaps in BitmapArray
// if bm1<bm2 then bm2 is over bm1
// s1 (0..2) - section of 1-2 
// s2 (0..2) - section of 2-3 
// s3 (0..2) - section of 3-1 
void PrepareIntersection1(int bm1,int bm2,int bm3,
						  int x0,int y0,
						  int s1,int s2,int s3,
						  RLCTable Masks,byte* BitmapsArray){
	if(bm1==bm2){
		if(bm3<bm2){
			//1,2 over 3 - inverse mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm1),x0,y0);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,27+(2-s2)*3+s3);
			return;
		}else{
			//3 over 1,2 - normal mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm1),x0,y0);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,18+(2-s2)*3+s3);
			return;
		};
	};
	if(bm2==bm3){
		if(bm1<bm3){
			//2,3 over 1 - inverse mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,45+(2-s3)*3+s1);
			return;
		}else{
			//1 over 2,3 - normal mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,36+(2-s3)*3+s1);
			return;
		};
	};
	if(bm1==bm3){
		if(bm2<bm3){
			//1,3 over 2 - inverse mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,9+(2-s1)*3+s2);
			return;
		}else{
			//2 over 1,3 - normal mask
			FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,(2-s1)*3+s2);
			return;
		};
	};
	//Triple intersections
	if(bm1<bm2&&bm1<bm3){
		FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm1),x0,y0);
		CopyMaskedBitmap64(BitmapsArray+GetBmOfst(bm2),x0,y0,
				(void*)(Masks->OfsTable[(2-s1)*3+s2]));
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,18+(2-s2)*3+s3);
		return;
	};
	if(bm2<bm1&&bm2<bm3){
		FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm2),x0,y0);
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,36+(2-s3)*3+s1);
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,18+(2-s2)*3+s3);
		return;
	};
	FastCreateMaskedBitmap64_1(BitmapsArray+GetBmOfst(bm3),x0,y0);
	CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,36+(2-s3)*3+s1);
	CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,(2-s1)*3+s2);
};
//
//                1
//         s1   /|
//            /  |
//        2 /    | s3
// (x01,y0) \    |
//            \  |
//         s2   \|
//                3
void PrepareIntersection2(int bm1,int bm2,int bm3,
						  int x0,int y01,
						  int s1,int s2,int s3,
						  RLCTable Masks,byte* BitmapsArray){
	int y0=y01-31;
	if(bm1==bm2){
		if(bm3<bm2){
			//1,2 over 3 - inverse mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm1),x0,y01);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,54+27+(2-s2)*3+s3);
			return;
		}else{
			//3 over 1,2 - normal mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm1),x0,y01);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,54+18+(2-s2)*3+s3);
			return;
		};
	};
	if(bm2==bm3){
		if(bm1<bm3){
			//2,3 over 1 - inverse mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,54+45+(2-s3)*3+s1);
			return;
		}else{
			//1 over 2,3 - normal mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,54+36+(2-s3)*3+s1);
			return;
		};
	};
	if(bm1==bm3){
		if(bm2<bm3){
			//1,3 over 2 - inverse mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,54+9+(2-s1)*3+s2);
			return;
		}else{
			//2 over 1,3 - normal mask
			FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
			CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,54+(2-s1)*3+s2);
			return;
		};
	};
	//Triple intersections
	if(bm1<bm2&&bm1<bm3){
		FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm1),x0,y01);
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,54+(2-s1)*3+s2);
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,54+18+(2-s2)*3+s3);
		return;
	};
	if(bm2<bm1&&bm2<bm3){
		FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm2),x0,y01);
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,54+36+(2-s3)*3+s1);
		CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm3),x0,y0,54+18+(2-s2)*3+s3);
		return;
	};
	FastCreateMaskedBitmap64_2(BitmapsArray+GetBmOfst(bm3),x0,y01);
	CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm1),x0,y0,54+36+(2-s3)*3+s1);
	CopyMaskedBitmap(BitmapsArray+GetBmOfst(bm2),x0,y0,54+(2-s1)*3+s2);	
};
void ClearIntersectionBuffer(){
	memset(ResultMask,0,sizeof ResultMask);
};
void ShowIntersectionBuffer(){
	if(!bActive)return;
	int SCROF=int(ScreenPtr)+256*ScrWidth+256;
	__asm{
		push	esi
		push	edi
		pushf
		mov		esi,offset ResultMask
		mov		edi,SCROF
		mov		edx,64
		cld
lpp1:	mov		ecx,64
		rep		movsd
		add		edi,ScrWidth
		sub		edi,256
		dec		edx
		jnz		lpp1
		popf
		pop		edi
		pop		esi
	};
	//memset(ResultMask,0,sizeof ResultMask);
};