// GSCarch.cpp: implementation of the CGSCarch class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
//#include "GSCarch.h"

#include <stdio.h>
#include "GSCtypes.h"
#include "GSCarch.h"
#include "GSCset.h"
#include "isiMasks.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSCarch::CGSCarch()
{}

GFILE_API CGSCarch::~CGSCarch()
{}

void GSC_OpenError()
{
	MessageBox( NULL, "Unable to map files into memory.", "Loading error...", MB_ICONERROR );
}

BOOL CGSCarch::Open( LPCSTR lpcsArchFileName )
{
	strcpy( m_ArchName, lpcsArchFileName );

	m_hMapFile = CreateFile( m_ArchName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
	if (INVALID_HANDLE_VALUE == m_hMapFile)
	{
		GSC_OpenError();
		return FALSE;
	}

	m_hMapping = CreateFileMapping( m_hMapFile, NULL, PAGE_READONLY, 0, 0, NULL );
	if (!m_hMapping)
	{
		GSC_OpenError();
		return FALSE;
	}


	m_pViewOfFile = (LPBYTE) MapViewOfFile( m_hMapping, FILE_MAP_READ, 0, 0, 0 );
	if (!m_pViewOfFile)
	{
		GSC_OpenError();
		return FALSE;
	}

	m_Header = (TGSCarchHDR*) ( m_pViewOfFile );
	m_FAT = (TGSCarchFAT*) ( LPBYTE( m_pViewOfFile ) + sizeof( TGSCarchHDR ) );
	m_Data = LPBYTE( m_pViewOfFile ) + sizeof( TGSCarchHDR )
		+ ( m_Header->m_Entries * sizeof( TGSCarchFAT ) );

	return TRUE;
}

BOOL CGSCarch::Close()
{
	if (m_pViewOfFile)
	{
		UnmapViewOfFile( m_pViewOfFile );
	}

	if (m_hMapFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle( m_hMapFile );
	}

	return TRUE;
}

LPGSCfile CGSCarch::GetFileHandle( LPCSTR lpcsFileName )
{
	DWORD			i = 0;
	LPGSCfile		lpFileHandle = NULL;
	LPGSCarchFAT	pFAT = NULL;
	CHAR			sUpFileName[64];

	ZeroMemory( sUpFileName, 64 );
	strcpy( sUpFileName, lpcsFileName );
	_strupr( sUpFileName );

	DWORD HASH = isiCalcHash( sUpFileName );

	for (i = 0; i <= m_Header->m_Entries - 1; i++)
	{
		pFAT = (TGSCarchFAT*) ( LPBYTE( m_FAT ) + i * sizeof( TGSCarchFAT ) );
		if (pFAT->m_Hash == HASH)
			if (!strcmp( LPCSTR( pFAT->m_FileName ), sUpFileName ))
			{
				lpFileHandle = new TGSCfile;
				lpFileHandle->m_FileHandle = i;
				lpFileHandle->m_Flags = 1;	// archieve file
				lpFileHandle->m_Position = 0;
				return lpFileHandle;
			}
	}

	return NULL;
}

VOID CGSCarch::CloseFileHandle( LPGSCfile lpFileHandle )
{
	if (lpFileHandle)
		delete lpFileHandle;
}

VOID CGSCarch::MemDecrypt( LPBYTE lpbDestination, DWORD dwSize )
{
	BYTE Key = (BYTE) ~( HIBYTE( _CRYPT_KEY_ ) );

	isiDecryptMem( lpbDestination, dwSize, Key );
}

DWORD CGSCarch::GetFileSize( LPGSCfile lpFileHandle )
{
	LPGSCarchFAT pFAT;

	pFAT = (TGSCarchFAT*) ( LPBYTE( m_FAT ) + lpFileHandle->m_FileHandle * sizeof( TGSCarchFAT ) );

	return pFAT->m_Size;
}


DWORD CGSCarch::GetFilePos( LPGSCfile lpFileHandle )
{
	return lpFileHandle->m_Position;
}

VOID CGSCarch::SetFilePos( LPGSCfile lpFileHandle, DWORD dwPosition )
{
	lpFileHandle->m_Position = dwPosition;
}

VOID CGSCarch::ReadFile( LPGSCfile lpFileHandle, LPBYTE lpbBuffer, DWORD dwSize )
{
	LPGSCarchFAT pFAT;

	pFAT = (TGSCarchFAT*) ( LPBYTE( m_FAT ) + lpFileHandle->m_FileHandle * sizeof( TGSCarchFAT ) );

	memcpy( lpbBuffer, ( m_Data + ~pFAT->m_Offset + lpFileHandle->m_Position ), dwSize );

	if (pFAT->m_Flags)
		MemDecrypt( lpbBuffer, dwSize );

	lpFileHandle->m_Position += dwSize;
}

LPBYTE CGSCarch::GetFileData( LPGSCfile lpFileHandle )
{
	LPGSCarchFAT pFAT;

	pFAT = (TGSCarchFAT*) ( LPBYTE( m_FAT ) + lpFileHandle->m_FileHandle * sizeof( TGSCarchFAT ) );

	return m_Data + ~pFAT->m_Offset;
}

/*
DWORD CGSCarch::CalcHash(LPCSTR lpcsFileName)
{
	DWORD HASH=0;
	DWORD D=0;

	CHAR sUpFileName[64];

	ZeroMemory(sUpFileName,64);

	strcpy(sUpFileName,lpcsFileName);
	_strupr(sUpFileName);

	for(int k=0;k<=15;k++)
	{
		D=(DWORD(CHAR(sUpFileName[k*4]))   << 24)+
		  (DWORD(CHAR(sUpFileName[k*4+1])) << 16)+
		  (DWORD(CHAR(sUpFileName[k*4+2])) << 8 )+
		  (DWORD(CHAR(sUpFileName[k*4+3]))      );

		HASH+=D;
	};

	return HASH;
}
*/

LPGSCFindData CGSCarch::FindFile( LPCSTR lpcsMask )
{
	LPGSCarchFAT pFAT;
	LPGSCFindData pFindData;
	LPSTR	lpsDelim;
	CHAR	sUpMask[64];

	ZeroMemory( sUpMask, 64 );
	strcpy( sUpMask, lpcsMask );
	_strupr( sUpMask );

	DWORD i;

	pFindData = new TGSCFindData;

	strcpy( pFindData->m_Mask, sUpMask );
	pFindData->m_Found = -1;

	for (i = 0; i <= m_Header->m_Entries - 1; i++)
	{
		pFAT = (TGSCarchFAT*) ( LPBYTE( m_FAT ) + i * sizeof( TGSCarchFAT ) );
		if (isiMatchesMask( LPSTR( pFAT->m_FileName ), sUpMask ))
		{
			lpsDelim = strrchr( LPSTR( pFAT->m_FileName ), '\\' );
			if (lpsDelim)
				strcpy( LPSTR( pFindData->m_FileName ), LPSTR( lpsDelim + 1 ) );
			else
				strcpy( LPSTR( pFindData->m_FileName ), LPSTR( pFAT->m_FileName ) );

			pFindData->m_Found = i;
			return pFindData;
		};
	};

	delete pFindData;
	return NULL;
};

BOOL CGSCarch::NextFile( LPGSCFindData gFindData )
{
	LPGSCarchFAT pFAT;
	LPSTR lpsDelim;

	DWORD i;

	for (i = gFindData->m_Found + 1; i <= m_Header->m_Entries - 1; i++)
	{
		pFAT = (TGSCarchFAT*) ( LPBYTE( m_FAT ) + i * sizeof( TGSCarchFAT ) );
		if (isiMatchesMask( LPSTR( pFAT->m_FileName ), LPSTR( gFindData->m_Mask ) ))
		{
			lpsDelim = strrchr( LPSTR( pFAT->m_FileName ), '\\' );
			if (lpsDelim)
				strcpy( LPSTR( gFindData->m_FileName ), LPSTR( lpsDelim + 1 ) );
			else
				strcpy( LPSTR( gFindData->m_FileName ), LPSTR( pFAT->m_FileName ) );
			gFindData->m_Found = i;
			return TRUE;
		};
	};

	delete gFindData;
	return FALSE;
}
