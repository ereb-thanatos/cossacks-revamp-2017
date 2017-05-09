/* Horisontal and vertical lines,rectangles*/
#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
void Hline(int x,int y,int xend,byte c){
	if(y<WindY||y>WindY1)return;
	int xr;
	int Lxr;
	if(xend>=x){
		if(x>WindX1)return;
		if(x<WindX)xr=WindX; else xr=x;
		if(xend>WindX1)Lxr=WindX1-xr+1;else Lxr=xend-xr+1;
	}else{
		if(xend>WindX1)return;
		if(xend<WindX)xr=WindX; else xr=xend;
		if(x>WindX1)Lxr=WindX1-xr+1;else Lxr=x-xr+1;
	};
	int scr=int(ScreenPtr)+xr+y*ScrWidth;
	if(Lxr<=0)return;
	__asm{
		mov		edi,scr
		mov		al,c
		mov		ecx,Lxr
		cld
		rep		stosb
	};
};
void Vline(int x,int y,int yend,byte c){
	if(x<WindX||x>WindX1)return;
	int yr;
	int Lyr;
	if(yend>=y){
		if(y>WindY1)return;
		if(y<WindY)yr=WindY; else yr=y;
		if(yend<WindY)return;
		if(yend>WindY1)Lyr=WindY1-yr+1;else Lyr=yend-yr+1;
	}else{
		if(yend>WindY1)return;
		if(yend<WindY)yr=WindY; else yr=yend;
		if(y>WindY1)Lyr=WindY1-yr+1;else Lyr=y-yr+1;
	};
	int scr=int(ScreenPtr)+x+yr*ScrWidth;
	if(Lyr<=0)return;
	__asm{
		mov		edi,scr
		mov		al,c
		mov		ecx,Lyr
		mov		edx,ScrWidth
		dec		edx
		cld
uuuuu:	stosb
		add		edi,edx
		loop	uuuuu
	};
};
void Xbar(int x,int y,int lx,int ly,byte c){
	Hline(x,y,x+lx-1,c);
	Hline(x,y+ly-1,x+lx-1,c);
	Vline(x,y,y+ly-1,c);
	Vline(x+lx-1,y,y+ly-1,c);
};
int GetXKind(int x){
	if(x<WindX)return 8;
	else if(x>WindX1)return 4;
	else return 0;
};
int GetYKind(int y){
	if(y<WindY)return 2;
	else if(y>WindY1)return 1;
	else return 0;
};
void DrawLine(int x,int y,int x1,int y1,byte c){
	if(x==x1&&y==y1){
		Hline(x,y,x,c);
		return;
	};
	int pp=GetXKind(x)+GetYKind(y);
	int pp1=GetXKind(x1)+GetYKind(y1);
	if(pp&pp1)return;
	if(x>x1){
		x1=x+x1;
		y1=y+y1;
		x=x1-x;
		y=y1-y;
		x1=x1-x;
		y1=y1-y;
	};
	if(x<WindX){
		y+=div((y1-y)*(WindX-x),x1-x).quot;
		x=WindX;
	};
	if(x1>WindX1){
		y1=y+div((y1-y)*(WindX1-x),x1-x).quot;
		x1=WindX1;
	};
	if(GetYKind(y1)&GetYKind(y))return;
	if(y<WindY){
		x+=div((x1-x)*(WindY-y),y1-y).quot;
		y=WindY;
	};
	if(y1<WindY){
		x1=x+div((x1-x)*(WindY-y),y1-y).quot;
		y1=WindY;
	};
	if(y>WindY1){
		x+=div((x1-x)*(WindY1-y),y1-y).quot;
		y=WindY1;
	};
	if(y1>WindY1){
		x1=x+div((x1-x)*(WindY1-y),y1-y).quot;
		y1=WindY1;
	};
	if(GetXKind(x)|GetXKind(x1)|GetYKind(y)|GetYKind(y1))return;
	int Lx=x1-x;
	int Ly;
	int ofst=int(ScreenPtr)+x+y*ScrWidth;
	if(y<y1){
		Ly=y1-y;
		if(Lx<Ly){
			__asm{
				push	edi
				mov		edi,ofst
				mov		edx,Lx
				mov		ebx,Ly
				sar		ebx,1
				mov		ecx,Ly
				inc     ecx
				mov		al,c
lpp1:			mov		[edi],al
				add		edi,ScrWidth
				sub		ebx,edx
				jg		lpp2
				add		ebx,Ly
				inc     edi
lpp2:			dec		ecx
				jnz		lpp1
				pop		edi
			};
		}else{
			__asm{
				push	edi
				mov		edi,ofst
				mov		edx,Ly
				mov		ebx,Lx
				sar		ebx,1
				mov		ecx,Lx
				inc     ecx
				mov		al,c
lpp1_1:			mov		[edi],al
				inc		edi
				sub		ebx,edx
				jg		lpp2_1
				add		ebx,Lx
				add		edi,ScrWidth
lpp2_1:			dec		ecx
				jnz		lpp1_1
				pop		edi
			};
		};
	}else{
		Ly=y-y1;
		if(Lx<Ly){
			__asm{
				push	edi
				mov		edi,ofst
				mov		edx,Lx
				mov		ebx,Ly
				sar		ebx,1
				mov		ecx,Ly
				inc     ecx
				mov		al,c
lpp1_2:			mov		[edi],al
				sub		edi,ScrWidth
				sub		ebx,edx
				jg		lpp2_2
				add		ebx,Ly
				inc     edi
lpp2_2:			dec		ecx
				jnz		lpp1_2
				pop		edi
			};
		}else{
			__asm{
				push	edi
				mov		edi,ofst
				mov		edx,Ly
				mov		ebx,Lx
				sar		ebx,1
				mov		ecx,Lx
				inc     ecx
				mov		al,c
lpp1_3:			mov		[edi],al
				inc		edi
				sub		ebx,edx
				jg		lpp2_3
				add		ebx,Lx
				sub		edi,ScrWidth
lpp2_3:			dec		ecx
				jnz		lpp1_3
				pop		edi
			};
		};
	};
};
