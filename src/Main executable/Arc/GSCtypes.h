#ifndef _INC_GSC_TYPES
#define _INC_GSC_TYPES

#include <windows.h>

#ifndef GFILE_USER
#define GFILE_API __declspec(dllexport)
#else
#define GFILE_API __declspec(dllimport)
#endif

#pragma pack(1)

struct TGSCFindData
{
 CHAR	m_Mask[64];
 CHAR	m_FileName[64];
 int	m_Found;
};

struct TGSCFileList
{
 CHAR			m_FileName[64];
 TGSCFileList*	m_NextEntry;
};

struct TGSCFindInfo
{
 CHAR			m_FileName[64];
 TGSCFileList*	m_Pos;
 TGSCFileList*	m_FileList;
};

struct TGSCarchHDR
{
 BYTE		m_Descriptor[6];
 WORD		m_Version;
 WORD		m_Key;
 DWORD		m_Entries;
};

struct TGSCarchFAT
{
 DWORD		m_Hash;
 BYTE		m_FileName[64];
 DWORD		m_Offset;
 DWORD		m_Size;
 DWORD		m_Reserved;
 BYTE		m_Flags;
};

typedef TGSCFindData* LPGSCFindData;

typedef TGSCarchFAT* LPGSCarchFAT;

typedef TGSCFileList* LPGSCFileList;

typedef TGSCFindInfo* LPGSCFindInfo;

#endif // _INC_GSC_TYPES