// GSCset.h: interface for the CGSCset class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GSCSET_H__E81AB1CB_A7B5_4DFE_B67D_9C1AC503EAD2__INCLUDED_)
#define AFX_GSCSET_H__E81AB1CB_A7B5_4DFE_B67D_9C1AC503EAD2__INCLUDED_

#include "GSCtypes.h"	// Added by ClassView
#include "GSCarch.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class GFILE_API CGSCset  
{
public:
	LPGSCfile gWriteOpen(LPCSTR lpcsFileName);
	BOOL gWriteFile(LPGSCfile gFile, LPBYTE lpbBuffer, DWORD dwSize);
	LPBYTE gMapFile(LPGSCfile gFile);
	DWORD gFileSize(LPGSCfile gFile);
	VOID gClose();
	BOOL gOpen();
	BOOL gFindNext(LPGSCFindInfo gFindInfo);
	LPGSCFindInfo gFindFirst(LPCSTR lpcsMask);
	DWORD gFilePos(LPGSCfile gFile);
	VOID gSeekFile(LPGSCfile gFile, DWORD dwPosition);
	VOID gReadFile(LPGSCfile gFile, LPBYTE lpbBuffer, DWORD dwSize);
	VOID gCloseFile(LPGSCfile gFile);
	LPGSCfile gOpenFile(LPCSTR lpcsFileName,bool Only);
	CGSCset();
	virtual ~CGSCset();

//private:
	LPGSCArchList m_ArchList;
};

#endif // !defined(AFX_GSCSET_H__E81AB1CB_A7B5_4DFE_B67D_9C1AC503EAD2__INCLUDED_)
