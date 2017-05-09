#define STRICT
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include "unrar.h"

enum { EXTRACT, TEST, PRINT };

void ExtractArchive(char *ArcName, int Mode);
void ListArchive(char *ArcName);
void ShowComment(char *CmtBuf);
void OutHelp(void);
void OutOpenArchiveError(int Error, char *ArcName);
void OutProcessFileError(int Error);
int ChangeVolProc(char *ArcName, int Mode);
int ProcessDataProc(unsigned char *Addr, int Size);

//Handle to unrar.dll
HMODULE hLib;
void* LoadF(char* Name)
{
	void* fn = GetProcAddress(hLib, Name);
	if (!fn) 
	{
		char ccc[256];
		sprintf(ccc, "UNRAR.DLL does not contain function: %s", Name);
		MessageBox(NULL, ccc, "DLL init error.", 0);
	}
	return fn;
}

#define LOADF(x) lp##x=(tp##x*)LoadF(#x)
void LoadRARLib() 
{
	hLib = LoadLibrary("unrar.dll");
	if (!hLib) 
	{
		MessageBox(NULL, "Could not load unrar.dll", "ERROR!", 0);
		return;
	}
	else 
	{
		LOADF(RAROpenArchive);
		LOADF(RARCloseArchive);
		LOADF(RARReadHeader);
		LOADF(RARProcessFile);
		LOADF(RARSetChangeVolProc);
		LOADF(RARSetProcessDataProc);
		LOADF(RARSetPassword);
	}
}

void CloseRARLib()
{
	if (hLib)
	{
		FreeLibrary(hLib);
	}
}

void ExtractArchive(char *ArcName, int Mode, char* Dest)
{
	if (!hLib)LoadRARLib();
	HANDLE hArcData;
	int RHCode, PFCode;
	char CmtBuf[16384];
	struct RARHeaderData HeaderData;
	struct RAROpenArchiveData OpenArchiveData;

	OpenArchiveData.ArcName = ArcName;
	OpenArchiveData.CmtBuf = CmtBuf;
	OpenArchiveData.CmtBufSize = sizeof(CmtBuf);
	OpenArchiveData.OpenMode = RAR_OM_EXTRACT;
	hArcData = RAROpenArchive(&OpenArchiveData);

	if (OpenArchiveData.OpenResult != 0)
	{
		OutOpenArchiveError(OpenArchiveData.OpenResult, ArcName);
		return;
	}

	if (OpenArchiveData.CmtState == 1)
		ShowComment(CmtBuf);

	RARSetChangeVolProc(hArcData, ChangeVolProc);

	if (Mode == PRINT)
		RARSetProcessDataProc(hArcData, ProcessDataProc);

	HeaderData.CmtBuf = NULL;

	while ((RHCode = RARReadHeader(hArcData, &HeaderData)) == 0)
	{
		PFCode = RARProcessFile(hArcData, (Mode == EXTRACT) ? RAR_EXTRACT : RAR_TEST,
			NULL, Dest);
		if (PFCode != 0)
		{
			OutProcessFileError(PFCode);
			break;
		}
	}

	if (RHCode == ERAR_BAD_DATA)
		//printf("\nFile header broken");

		RARCloseArchive(hArcData);
}


void ListArchive(char *ArcName)
{
	HANDLE hArcData;
	int RHCode, PFCode;
	char CmtBuf[16384];
	struct RARHeaderData HeaderData;
	struct RAROpenArchiveData OpenArchiveData;

	OpenArchiveData.ArcName = ArcName;
	OpenArchiveData.CmtBuf = CmtBuf;
	OpenArchiveData.CmtBufSize = sizeof(CmtBuf);
	OpenArchiveData.OpenMode = RAR_OM_LIST;
	hArcData = RAROpenArchive(&OpenArchiveData);

	if (OpenArchiveData.OpenResult != 0)
	{
		OutOpenArchiveError(OpenArchiveData.OpenResult, ArcName);
		return;
	}

	if (OpenArchiveData.CmtState == 1)
		ShowComment(CmtBuf);

	RARSetChangeVolProc(hArcData, ChangeVolProc);

	HeaderData.CmtBuf = CmtBuf;
	HeaderData.CmtBufSize = sizeof(CmtBuf);

	printf("\nFile                       Size");
	printf("\n-------------------------------");
	while ((RHCode = RARReadHeader(hArcData, &HeaderData)) == 0)
	{
		printf("\n%-20s %10d ", HeaderData.FileName, HeaderData.UnpSize);
		if (HeaderData.CmtState == 1)
			ShowComment(CmtBuf);
		if ((PFCode = RARProcessFile(hArcData, RAR_SKIP, NULL, NULL)) != 0)
		{
			OutProcessFileError(PFCode);
			break;
		}
	}

	if (RHCode == ERAR_BAD_DATA)
		printf("\nFile header broken");

	RARCloseArchive(hArcData);
}


void ShowComment(char *CmtBuf)
{
	printf("\nComment:\n%s\n", CmtBuf);
}


void OutHelp(void)
{
	printf("\nUNRDLL.   This is a simple example of UNRAR.DLL usage\n");
	printf("\nSyntax:\n");
	printf("\nUNRDLL X <Archive>     extract archive contents");
	printf("\nUNRDLL T <Archive>     test archive contents");
	printf("\nUNRDLL P <Archive>     print archive contents to stdout");
	printf("\nUNRDLL L <Archive>     view archive contents\n");
}


void OutOpenArchiveError(int Error, char *ArcName)
{
	switch (Error)
	{
	case ERAR_NO_MEMORY:
		printf("\nNot enough memory");
		break;
	case ERAR_EOPEN:
		printf("\nCannot open %s", ArcName);
		break;
	case ERAR_BAD_ARCHIVE:
		printf("\n%s is not RAR archive", ArcName);
		break;
	case ERAR_BAD_DATA:
		printf("\n%s: archive header broken", ArcName);
		break;
	}
}

void OutProcessFileError(int Error)
{
	switch (Error)
	{
	case ERAR_UNKNOWN_FORMAT:
		printf("Unknown archive format");
		break;
	case ERAR_BAD_ARCHIVE:
		printf("Bad volume");
		break;
	case ERAR_ECREATE:
		printf("File create error");
		break;
	case ERAR_EOPEN:
		printf("Volume open error");
		break;
	case ERAR_ECLOSE:
		printf("File close error");
		break;
	case ERAR_EREAD:
		printf("Read error");
		break;
	case ERAR_EWRITE:
		printf("Write error");
		break;
	case ERAR_BAD_DATA:
		printf("CRC error");
		break;
	}
}


int ChangeVolProc(char *ArcName, int Mode)
{
	int Ch;
	if (Mode == RAR_VOL_ASK)
	{
		printf("\nInsert disk with %s and press 'Enter' or enter 'Q' to exit ", ArcName);
		Ch = getchar();
		return(toupper(Ch) != 'Q');
	}
	return(1);
}


int ProcessDataProc(unsigned char *Addr, int Size)
{
	fflush(stdout);
	fwrite(Addr, 1, Size, stdout);
	fflush(stdout);
	return(1);
}
tpRAROpenArchive* lpRAROpenArchive = NULL;
tpRARCloseArchive* lpRARCloseArchive = NULL;
tpRARReadHeader* lpRARReadHeader = NULL;
tpRARProcessFile* lpRARProcessFile = NULL;
tpRARSetChangeVolProc* lpRARSetChangeVolProc = NULL;
tpRARSetProcessDataProc* lpRARSetProcessDataProc = NULL;
tpRARSetPassword* lpRARSetPassword = NULL;
HANDLE PASCAL RAROpenArchive(struct RAROpenArchiveData *ArchiveData) {
	return lpRAROpenArchive(ArchiveData);
};
int PASCAL RARCloseArchive(HANDLE hArcData) {
	return lpRARCloseArchive(hArcData);
};
int PASCAL RARReadHeader(HANDLE hArcData, struct RARHeaderData *HeaderData) {
	return lpRARReadHeader(hArcData, HeaderData);
};
int PASCAL RARProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName) {
	return lpRARProcessFile(hArcData, Operation, DestPath, DestName);
};
void PASCAL RARSetChangeVolProc(HANDLE hArcData, int(*ChangeVolProc)(char *ArcName, int Mode)) {
	lpRARSetChangeVolProc(hArcData, ChangeVolProc);
};
void PASCAL RARSetProcessDataProc(HANDLE hArcData, int(*ProcessDataProc)(unsigned char *Addr, int Size)) {
	lpRARSetProcessDataProc(hArcData, ProcessDataProc);
};
void PASCAL RARSetPassword(HANDLE hArcData, char *Password) {
	lpRARSetPassword(hArcData, Password);
};