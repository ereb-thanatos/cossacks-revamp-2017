#include "../Main executable/common.h"
#include "ParseRQ.h"
#include <crtdbg.h>
#include <assert.h>
ParsedRQ::ParsedRQ()
{
	memset( this, 0, sizeof *this );
};
ParsedRQ::~ParsedRQ()
{
	Clear();
};
void ParsedRQ::AddComm( char* Name )
{
	char NAME[18];
	strncpy( NAME, Name, 16 );
	NAME[15] = 0;
	Comm = (OneComm*) realloc( Comm, ( NComm + 1 ) * sizeof OneComm );
	strcpy( Comm[NComm].ComID, NAME );
	Comm[NComm].NParams = 0;
	Comm[NComm].Params = NULL;
	Comm[NComm].ParamSize = NULL;
	NComm++;
};
void ParsedRQ::AddParam( char* Data, int size )
{
	if( NComm )
	{
		OneComm* OC = Comm + NComm - 1;
		OC->Params = (char**) realloc( OC->Params, ( OC->NParams + 1 ) << 2 );
		OC->Params[OC->NParams] = (char*) malloc( size + 1 );
		memcpy( OC->Params[OC->NParams], Data, size );
		OC->Params[OC->NParams][size] = 0;
		OC->ParamSize = (int*) realloc( OC->ParamSize, ( OC->NParams + 1 ) << 2 );
		OC->ParamSize[OC->NParams] = size;
		OC->NParams++;
	};
};
void ParsedRQ::Clear()
{
	for( int i = 0; i < NComm; i++ )
	{
		for( int j = 0; j < Comm[i].NParams; j++ )
		{
			free( Comm[i].Params[j] );
		};
		if( Comm[i].Params )free( Comm[i].Params );
		if( Comm[i].ParamSize )free( Comm[i].ParamSize );
	};
	if( Comm )free( Comm );
	memset( this, 0, sizeof *this );
};
int GetHVal( char c )
{
	if( c >= '0'&&c <= '9' )
	{
		return c - '0';
	}
	else if( c >= 'A'&&c <= 'F' )
	{
		return c - 'A' + 10;
	}
	else return 0;
};
void ParsedRQ::Parse( char* s )
{
	Clear();
	int pos = 0;
	char c;
	while( ( c = s[pos] ) != '|'&&c&&pos < 16 && c != '&' )pos++;
	if( c != '|' )
	{
		pos = 0;
	}
	else
	{
		memcpy( DevName, s, pos );
		DevName[pos] = 0;
		pos++;
	};
	bool doit = 1;
	do
	{
		int p1 = pos;
		while( ( c = s[p1] ) != '&'&&c != 0 && c != '|' )p1++;
		char cc[18];
		int p = p1 - pos > 15 ? 15 : p1 - pos;
		memcpy( cc, s + pos, p );
		cc[p] = 0;
		AddComm( cc );
		p1++;
		pos = p1;
		if( c == '&' )
		{
			while( c == '&' )
			{
				p1 = pos;
				while( ( c = s[p1] ) != '&'&&c != '|'&&c != 0 )p1++;
				char* dst = (char*) malloc( p1 - pos + 1 );
				//if(c=='|')doit=0;
				int N = p1 - pos;
				int dpos = 0;
				for( int j = 0; j < N; j++ )
				{
					char cc1 = s[pos + j];
					if( cc1 == '\\' )
					{
						int v = ( GetHVal( s[pos + j + 1] ) << 4 ) + GetHVal( s[pos + j + 2] );
						j += 2;
						dst[dpos] = (char) v;
					}
					else dst[dpos] = cc1;
					dpos++;
				};
				AddParam( dst, dpos );
				free( dst );
				pos = p1 + 1;
			};
		};
		if( c == 0 )
		{
			doit = 0;
		}
	} while( doit );
}

int ParsedRQ::Compact( char* dst, int size )
{
	int pos = 0;

	//1.calculating size
	int sz = 2;
	for( int i = 0; i < NComm; i++ )
	{
		sz += strlen( Comm[i].ComID ) + 1 + 2 + Comm[i].NParams * 4;
		for( int j = 0; j < Comm[i].NParams; j++ )
			sz += Comm[i].ParamSize[j];
	}
	if( sz > size || !dst )
		return sz;

	//2.compactification
	( (short*) ( dst + pos ) )[0] = (short) NComm;
	pos += 2;
	for( int i = 0; i < NComm; i++ )
	{
		int L = strlen( Comm[i].ComID );
		dst[pos] = (char) L;
		pos++;
		memcpy( dst + pos, Comm[i].ComID, L );
		pos += L;
		int N = Comm[i].NParams;
		( (short*) ( dst + pos ) )[0] = (short) N;
		pos += 2;
		for( int j = 0; j < N; j++ )
		{
			int SZ = Comm[i].ParamSize[j];
			( (int*) ( dst + pos ) )[0] = SZ;
			pos += 4;
			memcpy( dst + pos, Comm[i].Params[j], SZ );
			pos += SZ;
		};
	};
	return pos;
};
void ParsedRQ::Extract( char* s, int size )
{
	try
	{
		Clear();
		int pos = 0;
		int N = ( (short*) ( s + pos ) )[0];
		pos += 2;
		for( int i = 0; i < N; i++ )
		{
			int nc = s[pos];
			pos++;
			char ss[18];
			int sz1 = nc > 15 ? 15 : nc;
			memcpy( ss, s + pos, sz1 );
			ss[sz1] = 0;
			pos += nc;
			nc = ( (short*) ( s + pos ) )[0];
			pos += 2;
			AddComm( ss );
			for( int j = 0; j < nc; j++ )
			{
				int csz = ( (int*) ( s + pos ) )[0];
				pos += 4;
				AddParam( s + pos, csz );
				pos += csz;
			};
		};
	}
	catch( ... )
	{
	};
};
char HXARR[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
int ParsedRQ::UnParse( char* dst, int size )
{
	//checking the size
	int sz = strlen( DevName );
	for( int i = 0; i < NComm; i++ )
	{
		sz += 1 + strlen( Comm[i].ComID );
		int np = Comm[i].NParams;
		for( int j = 0; j < np; j++ )
		{
			sz++;
			char* data = Comm[i].Params[j];
			int L = Comm[i].ParamSize[j];
			sz++;
			for( int q = 0; q < L; q++ )
			{
				unsigned char c = data[q];
				if( c < 32 || c == '|' || c == '&' || c == '{' || c == '}' || c == '\\' )sz += 3;
				else sz++;
			};
		};
	};
	if( size < sz && !dst )return sz + 1;
	strcpy( dst, DevName );
	sz = strlen( DevName );
	for( int i = 0; i < NComm; i++ )
	{
		strcat( dst, "|" );
		sz++;
		strcat( dst, Comm[i].ComID );
		sz += strlen( Comm[i].ComID );
		int np = Comm[i].NParams;
		for( int j = 0; j < np; j++ )
		{
			strcat( dst, "&" );
			sz++;
			char* data = Comm[i].Params[j];
			int L = Comm[i].ParamSize[j];
			int p = strlen( dst );
			int dd = 0;
			for( int q = 0; q < L; q++ )
			{
				unsigned char c = data[q];
				if( c < 32 || c == '|' || c == '&' || c == '{' || c == '}' || c == '\\' )
				{
					dst[q + p + dd] = '\\';
					dst[q + 1 + p + dd] = HXARR[c >> 4];
					dst[q + 2 + p + dd] = HXARR[c & 15];
					dst[q + 3 + p + dd] = 0;
					sz += 3;
					dd += 2;
				}
				else
				{
					dst[q + p + dd] = c;
					dst[q + 1 + p + dd] = 0;
					sz++;
				};
			};
		};
	};
	dst[sz] = 0;
	return sz;
};
void ParsedRQ::DelComm( int pos )
{
	if( pos < NComm )
	{
		for( int i = 0; i < Comm[pos].NParams; i++ )free( Comm[pos].Params[i] );
		if( Comm[pos].ParamSize )free( Comm[pos].ParamSize );
		if( Comm[pos].Params )free( Comm[pos].Params );
		if( pos < NComm - 1 )memmove( Comm + pos, Comm + pos + 1, ( NComm - pos - 1 ) * sizeof OneComm );
		NComm--;
	};
};
void ParsedRQ::AddParamToCom( int idx, char* data, int size )
{
	if( idx < NComm )
	{
		OneComm* OC = Comm + idx;
		OC->Params = (char**) realloc( OC->Params, ( OC->NParams + 1 ) << 2 );
		OC->Params[OC->NParams] = (char*) malloc( size + 1 );
		memcpy( OC->Params[OC->NParams], data, size );
		OC->Params[OC->NParams][size] = 0;
		OC->ParamSize = (int*) realloc( OC->ParamSize, ( OC->NParams + 1 ) << 2 );
		OC->ParamSize[OC->NParams] = size;
		OC->NParams++;
	};
};
void ParsedRQ::AddIntParam( int val )
{
	char cc[64];
	itoa( val, cc, 10 );
	AddParam( cc, strlen( cc ) + 1 );
};
/*
#undef malloc
#undef calloc
#undef free
#undef realloc
int NMEM=0;
struct OneLeak{
	char* File;
	int Line;
	void* ptr;
};
class LEAKS{
public:
	int NLK;
	int MAXLK;
	OneLeak* LK;
	LEAKS();
	~LEAKS();
	void AddLeak(char* file,int line,void* ptr);
	void ReplaceLeak(char* file,int line,void* ptr,void* prt1);
	void DeleteLeak(void* ptr);
};
LEAKS::LEAKS(){
	memset(this,0,sizeof *this);
};
LEAKS::~LEAKS(){
	if(LK)free(LK);
};
void LEAKS::AddLeak(char* file,int line,void* ptr){
	if(NLK>=MAXLK){
		MAXLK+=64;
		LK=(OneLeak*)realloc(LK,MAXLK*sizeof OneLeak);
	};
	LK[NLK].File=file;
	LK[NLK].Line=line;
	LK[NLK].ptr=ptr;
	NLK++;
};
LEAKS LK;
void LEAKS::DeleteLeak(void *ptr){
	for(int i=NLK-1;i>=0;i--)if(LK[i].ptr==ptr){
		memcpy(LK+i,LK+i+1,(NLK-i-1)*sizeof OneLeak);
		NLK--;
		return;
	};
};
void LEAKS::ReplaceLeak(char* file,int line,void* ptr,void* ptr1){
	for(int i=NLK-1;i>=0;i--)if(LK[i].ptr==ptr){
		LK[i].File=file;
		LK[i].Line=line;
		LK[i].ptr=ptr1;
	};
};
void* ie_malloc(int size,char* File,int Line){
	NMEM++;
	void* xx=malloc(size);
	LK.AddLeak(File,Line,xx);
	return xx;
};
void* ie_realloc(void* data,int size,char* File,int Line){
	if(!data){
		NMEM++;
		void* xx=malloc(size);
		LK.AddLeak(File,Line,xx);
		return xx;
	};
	void* xx=realloc(data,size);
	LK.ReplaceLeak(File,Line,data,xx);
	return xx;
};
void ie_free(void* data){
	NMEM--;
	LK.DeleteLeak(data);
	free(data);
};
*/