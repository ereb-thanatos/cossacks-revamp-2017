#include "../Main executable/common.h"
#include "IntExplorer.h"
#include "ParseRQ.h"
#include <crtdbg.h>
#include <assert.h>

class DString{
public:
	char* str;
	int L;
	int MaxL;
	DString();
	DString(char* s);
	~DString();
	void Allocate(int sz);
	void ReadFromFile(char* file);
	void WriteToFile (char* file);
	void Add(DString& Str);
	void Add(char* Str);
	void Replace(char* src,char* dst);
	void Replace(char* src,DString& dst);
	void Clear();
	void Free();
	void Assign(char* str);
	void Assign(DString& str);
	DString& operator = (char* s);
	DString& operator + (char* s);
	DString& operator + (DString& ds1);
	DString& operator += (char* s);
	DString& operator += (DString& ds);
	bool operator == (char* s);
	bool operator == (DString& ds);
	char operator [] (int index);
};
//   OPERATORS
DString& operator + (char*s,DString& ds){
	DString DS(s);
	DS.Add(ds);
	ds.Assign(DS);
	return ds;
};

DString& DString::operator = (char* s){
	Assign(s);
	return *this;
};
DString& DString::operator + (char* s){
	Add(s);
	return *this;
};
DString& DString::operator + (DString& ds1){
	Add(ds1);
	return *this;
};
DString& DString::operator += (char* s){
	Add(s);
	return *this;
};
DString& DString::operator += (DString& ds){
	Add(ds);
	return *this;
};
char DString::operator [] (int index){
	if(str&&index>L)return str[index];
	else return 0;
};
bool DString::operator == (char* s){
	if(s&&str){
		return !strcmp(s,str);
	}else return false;
};
bool DString::operator == (DString& ds){
	if(ds.str&&str){
		return !strcmp(ds.str,str);
	}else return false;
};
//---------------
DString::DString(){
	str=NULL;
	L=0;
	MaxL=0;
};
DString::DString(char* s){
	str=NULL;
	L=0;
	MaxL=0;
	Assign(s);
};
DString::~DString(){
	if(str)free(str);
	str=NULL;
	L=0;
	MaxL=0;
};
void DString::ReadFromFile(char* file){
	Clear();
	ResFile F=RReset(file);
	if(F!=INVALID_HANDLE_VALUE){
		int sz=RFileSize(F);
		if(sz){
			Allocate(sz+1);
			RBlockRead(F,str,sz);
			str[sz]=0;
			L=strlen(str);
		};
		RClose(F);
	};
};
void DString::WriteToFile(char* file){
	ResFile F=RRewrite(file);
	if(F!=INVALID_HANDLE_VALUE){
		if(str)RBlockWrite(F,str,L);
		RClose(F);
	};
};
void DString::Add(DString& Str){
	if(!Str.str)return;
	Allocate(L+Str.L+1);
	strcat(str,Str.str);
	L=strlen(str);
};
void DString::Add(char* Str){
	Allocate(L+strlen(Str)+1);
	strcat(str,Str);
	L=strlen(str);
};
void DString::Replace(char* src,char* dst){
	if(!str)return;
	char* s=str;
	int L0=strlen(src);
	int L1=strlen(dst);
	do{
		s=strstr(s,src);
		if(s){
			int pos=s-str;
			Allocate(L+L1-L0+1);
			memmove(str+pos+L1,str+pos+L0,L-pos-L0+1);
			memcpy(str+pos,dst,L1);
			L=strlen(str);
			s=str+pos+L1;
		};
	}while(s);
};
void DString::Replace(char* src,DString& dst){
	if(!dst.str)return;
	Replace(src,dst.str);
};
void DString::Clear(){
	L=0;
	if(str)str[0]=0;
};
void DString::Assign(char* Str){
	Clear();
	L=strlen(Str);
	Allocate(L+1);
	strcpy(str,Str);
};
void DString::Assign(DString& Str){
	if(Str.str)Assign(Str.str);
	else Clear();
};
void DString::Free(){
	if(str)free(str);
	str=NULL;
	L=0;
	MaxL=0;
};
void DString::Allocate(int LN){
	if(MaxL<LN){
		MaxL=((LN+256)&0xFFFF00);
		str=(char*)realloc(str,MaxL);
		str[L]=0;
	};
};
