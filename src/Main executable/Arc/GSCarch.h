// GSCarch.h: interface for the CGSCarch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GSCARCH_H__96E13529_35D1_407C_8155_20FD14236E7C__INCLUDED_)
#define AFX_GSCARCH_H__96E13529_35D1_407C_8155_20FD14236E7C__INCLUDED_

#include "GSCtypes.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#define RUSSIAN //BUGFIX: russian _CRYPT_KEY_ crashes when used with english ALL.GSC//IMPORTANT

#define _CRYPT_KEY_ 0x78CD

#ifdef RUSSIAN
#undef _CRYPT_KEY_
#define _CRYPT_KEY_ 0x4EBA
#endif

class CGSCarch;

typedef CGSCarch* LPGSCarch;

struct TGSCfile
{
 DWORD	m_FileHandle;
 BYTE	m_Flags;
 DWORD	m_Position;
 LPGSCarch	m_Arch;
};

typedef TGSCfile* LPGSCfile;

class GFILE_API CGSCarch  
{
public:
	BOOL NextFile(LPGSCFindData gFindData);
	LPGSCFindData FindFile(LPCSTR lpcsMask);
	LPBYTE GetFileData(LPGSCfile lpFileHandle);
	
	BOOL Open(LPCSTR lpcsArchFileName);
	BOOL Close();

	VOID ReadFile(LPGSCfile lpFileHandle, LPBYTE lpbBuffer, DWORD dwSize);
	
	VOID SetFilePos(LPGSCfile lpFileHandle, DWORD dwPosition);
	DWORD GetFilePos(LPGSCfile lpFileHandle);
	
	DWORD GetFileSize(LPGSCfile lpFileHandle);
	
	LPGSCfile GetFileHandle(LPCSTR lpcsFileName);
	VOID CloseFileHandle(LPGSCfile lpFileHandle);
	
	CGSCarch();
	virtual ~CGSCarch();

private:
	LPBYTE m_pViewOfFile;
	HANDLE m_hMapping;
	HANDLE m_hMapFile;
	CHAR m_ArchName[128];
	
	
	VOID MemDecrypt(LPBYTE lpbDestination, DWORD dwSize);
	
	LPBYTE m_Data;
	TGSCarchFAT* m_FAT;
	TGSCarchHDR* m_Header;
};

struct TGSCArchList
{
 CGSCarch* m_Arch;
 TGSCArchList* m_NextArch;
};

typedef TGSCArchList* LPGSCArchList;

#endif // !defined(AFX_GSCARCH_H__96E13529_35D1_407C_8155_20FD14236E7C__INCLUDED_)
