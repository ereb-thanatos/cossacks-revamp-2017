#define ERAR_END_ARCHIVE     10
#define ERAR_NO_MEMORY       11
#define ERAR_BAD_DATA        12
#define ERAR_BAD_ARCHIVE     13
#define ERAR_UNKNOWN_FORMAT  14
#define ERAR_EOPEN           15
#define ERAR_ECREATE         16
#define ERAR_ECLOSE          17
#define ERAR_EREAD           18
#define ERAR_EWRITE          19
#define ERAR_SMALL_BUF       20

#define RAR_OM_LIST           0
#define RAR_OM_EXTRACT        1

#define RAR_SKIP              0
#define RAR_TEST              1
#define RAR_EXTRACT           2

#define RAR_VOL_ASK           0
#define RAR_VOL_NOTIFY        1

struct RARHeaderData
{
  char ArcName[260];
  char FileName[260];
  UINT Flags;
  UINT PackSize;
  UINT UnpSize;
  UINT HostOS;
  UINT FileCRC;
  UINT FileTime;
  UINT UnpVer;
  UINT Method;
  UINT FileAttr;
  char *CmtBuf;
  UINT CmtBufSize;
  UINT CmtSize;
  UINT CmtState;
};


struct RAROpenArchiveData
{
  char *ArcName;
  UINT OpenMode;
  UINT OpenResult;
  char *CmtBuf;
  UINT CmtBufSize;
  UINT CmtSize;
  UINT CmtState;
};


#ifdef __cplusplus
extern "C" {
#endif
typedef HANDLE PASCAL tpRAROpenArchive(struct RAROpenArchiveData *ArchiveData);
extern tpRAROpenArchive* lpRAROpenArchive;
HANDLE PASCAL RAROpenArchive(struct RAROpenArchiveData *ArchiveData);
typedef int PASCAL tpRARCloseArchive(HANDLE hArcData);
extern tpRARCloseArchive* lpRARCloseArchive;
int PASCAL RARCloseArchive(HANDLE hArcData);
typedef int PASCAL tpRARReadHeader(HANDLE hArcData,struct RARHeaderData *HeaderData);
extern tpRARReadHeader* lpRARReadHeader;
int PASCAL RARReadHeader(HANDLE hArcData,struct RARHeaderData *HeaderData);
typedef int PASCAL tpRARProcessFile(HANDLE hArcData,int Operation,char *DestPath,char *DestName);
extern tpRARProcessFile* lpRARProcessFile;
int PASCAL RARProcessFile(HANDLE hArcData,int Operation,char *DestPath,char *DestName);
typedef void PASCAL tpRARSetChangeVolProc(HANDLE hArcData,int (*ChangeVolProc)(char *ArcName,int Mode));
extern tpRARSetChangeVolProc* lpRARSetChangeVolProc;
void PASCAL RARSetChangeVolProc(HANDLE hArcData,int (*ChangeVolProc)(char *ArcName,int Mode));
typedef void PASCAL tpRARSetProcessDataProc(HANDLE hArcData,int (*ProcessDataProc)(unsigned char *Addr,int Size));
extern tpRARSetProcessDataProc* lpRARSetProcessDataProc;
void PASCAL RARSetProcessDataProc(HANDLE hArcData,int (*ProcessDataProc)(unsigned char *Addr,int Size));
typedef void PASCAL tpRARSetPassword(HANDLE hArcData,char *Password);
extern tpRARSetPassword* lpRARSetPassword;
void PASCAL RARSetPassword(HANDLE hArcData,char *Password);

#ifdef __cplusplus
}
#endif
