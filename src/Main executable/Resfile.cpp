/*      Work with the resource files
 *
 *  You must use this module for accesss to files this
 * routine allows you to read files from disk or from
 * the resource file, you even will not recognise where
 * the given file is.
 */
 //#include <afx.h>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
//#include "tntFileIO.h"
#include "unrar.h"
#include "Arc\GSCSet.h"
#include "assert.h"
void AText( char* str );
//typedef LPGSCfile ResFile;
typedef HANDLE ResFile;
bool InitDone = 0;
CGSCset GSFILES;
bool FilesInit();
//Opening the resource file
bool ProtectionMode = false;
ResFile RResetEx( LPCSTR lpFileName )
{
	bool Only = GSFILES.m_ArchList&&ProtectionMode;

	FilesInit();

	LPGSCfile lpf = GSFILES.gOpenFile( lpFileName, Only );
	if (lpf)
	{
		return ResFile( lpf );
	}
	else
	{
		return INVALID_HANDLE_VALUE;
	}
}

bool GetRarName( LPCSTR Name, char* Dest )
{
	int L = strlen( Name );
	strcpy( Dest, Name );
	if (L > 3)
	{
		if (Dest[L - 1] == '.')Dest[L - 1] = 0;
		else if (Dest[L - 2] == '.')Dest[L - 2] = 0;
		else if (Dest[L - 3] == '.')Dest[L - 3] = 0;
		else if (Dest[L - 4] == '.')Dest[L - 4] = 0;
		strcat( Dest, ".rar" );
		return 1;
	}
	else return 0;
};
char** FHNames = NULL;
ResFile* FHANDLES = NULL;
int NHNames = 0;
int MaxNames = 0;

void AddFHandle( ResFile F, char* Name )
{
	if (NHNames >= MaxNames)
	{
		MaxNames += 16;
		FHNames = (char**) realloc( FHNames, 4 * MaxNames );
		FHANDLES = (ResFile*) realloc( FHANDLES, 4 * MaxNames );
	}

	FHNames[NHNames] = new char[strlen( Name ) + 1];
	strcpy( FHNames[NHNames], Name );

	FHANDLES[NHNames] = F;

	NHNames++;
}

void EraseFName( ResFile F )
{
	for (int i = 0; i < NHNames; i++)if (FHANDLES[i] == F)
	{
		DeleteFile( FHNames[i] );
		free( FHNames[i] );
		if (i < NHNames - 1)
		{
			memcpy( FHNames + i, FHNames + i + 1, 4 * ( NHNames - i - 1 ) );
			memcpy( FHANDLES + i, FHANDLES + i + 1, 4 * ( NHNames - i - 1 ) );
		};
		NHNames--;
	};
}

void EraseAllFNames()
{
	for (int i = 0; i < NHNames; i++)DeleteFile( FHNames[i] );
}

void RCloseEx( ResFile hFile );
void ExtractArchive( char *ArcName, int Mode, char* Dest );

__declspec( dllexport ) ResFile RReset( LPCSTR lpFileName )
{
	SetLastError( 0 );

	ResFile F = RResetEx( lpFileName );

	if (F == INVALID_HANDLE_VALUE)
	{
		RCloseEx( F );
		int L = strlen( lpFileName );
		char ccc[200];
		if (GetRarName( lpFileName, ccc ))
		{
			F = RResetEx( ccc );
			if (F != INVALID_HANDLE_VALUE)
			{
				RCloseEx( F );

				ExtractArchive( ccc, 0, (char*) lpFileName );

				F = RResetEx( lpFileName );

				if (F != INVALID_HANDLE_VALUE)
				{
					AddFHandle( F, (char*) lpFileName );
				}
			}
		}
	}

	return F;
}

//Rewriting file
__declspec( dllexport ) ResFile RRewrite( LPCSTR lpFileName )
{
	FilesInit();
	//return CreateFile(lpFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
	//	                         CREATE_ALWAYS,0,NULL);
	LPGSCfile lpf = GSFILES.gWriteOpen( lpFileName );
	if ( lpf )
	{
		return ResFile( lpf );
	}
	else
	{
		return INVALID_HANDLE_VALUE;
	}
}

//Getting size of the resource file
__declspec( dllexport ) DWORD RFileSize( ResFile hFile )
{
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return 0;
	}
	LPGSCfile hf = LPGSCfile( hFile );
	return GSFILES.gFileSize( hf );//GetFileSize(hFile,NULL);
}

// Setting file position 
__declspec( dllexport ) DWORD RSeek( ResFile hFile, int pos )
{
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	LPGSCfile hf = LPGSCfile( hFile );
	GSFILES.gSeekFile( hf, pos );

	return 0;
}

//Reading the file
__declspec( dllexport ) DWORD RBlockRead( ResFile hFile, LPVOID lpBuffer, DWORD BytesToRead )
{
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	LPGSCfile hf = LPGSCfile( hFile );

	GSFILES.gReadFile( hf, LPBYTE( lpBuffer ), BytesToRead );

	return BytesToRead;
}

//Writing the file
__declspec( dllexport ) DWORD RBlockWrite( ResFile hFile, LPVOID lpBuffer, DWORD BytesToWrite )
{
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return 0;
	}

	LPGSCfile hf = LPGSCfile( hFile );
	GSFILES.gWriteFile( hf, LPBYTE( lpBuffer ), BytesToWrite );

	return BytesToWrite;
}

DWORD IOresult( void )
{
	return 0;//GetLastError();
}

void RCloseEx( ResFile hFile )
{
	if (hFile == INVALID_HANDLE_VALUE)return;
	//CloseHandle(hFile);
	LPGSCfile hf = LPGSCfile( hFile );
	GSFILES.gCloseFile( hf );
}

__declspec( dllexport ) void RClose( ResFile hFile )
{
	if (hFile == INVALID_HANDLE_VALUE)return;
	LPGSCfile hf = LPGSCfile( hFile );
	//CloseHandle(hFile);
	RCloseEx( hf );
	EraseFName( hFile );
}

//Loads unrar.dll and resource archives
bool FilesInit()
{
	if (InitDone)
	{
		return true;
	}

	char CDR[256];
	GetCurrentDirectory( 256, CDR );
	if (CDR[strlen( CDR ) - 1] == '\\')
	{
		CDR[strlen( CDR ) - 1] = 0;
	}

	char ccc[290];
	sprintf( ccc, "%s\\unrar.dll", CDR );
	FILE* F = fopen( ccc, "r" );
	if (F)
	{
		fclose( F );
	}
	else
	{
		int L = strlen( CDR );

		while (L && CDR[L] != '\\')
			L--;

		CDR[L] = 0;
		sprintf( ccc, "%s\\unrar.dll", CDR );
		F = fopen( ccc, "r" );
		if (F)
		{
			fclose( F );
			SetCurrentDirectory( CDR );
		}
	}

	InitDone = 1;

	bool retval = 0 != GSFILES.gOpen();

	return retval;
}

void CloseRARLib();

void FilesExit()
{
	CloseRARLib();
	EraseAllFNames();
	GSFILES.gClose();
}