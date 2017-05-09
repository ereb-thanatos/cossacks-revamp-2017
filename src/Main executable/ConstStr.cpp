#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma pack(1)
#include "ResFile.h"
#include "gFile.h"
#define LOADSCX
char* GetTextByID(char* ID);
char* HiEdMode;//"Редактирование высот."
void LOADSC(char* ID,char** str){
	(*str)=GetTextByID(ID);
	if(!strcmp(*str,ID)){
		char cc[128];
		sprintf(cc,"Unknown string: %s (see COMMENT.TXT)",ID);
		MessageBox(NULL,cc,"String not found...",0);
	};
};
#undef LoadSC
#define LoadSC(z) char* ##z##=NULL;
#include "ConstStr.h"
#undef LoadSC
#define LoadSC(x) LOADSC(#x,&##x);
void LoadConstStr(){
#include "ConstStr.h"
};