struct OneIChunk{
	byte Index;
	word size;
	word Summ;
	byte Data[64];
};
struct CurUpload{
	bool Authenticated:1;
	bool Started:1;
	bool Finished:1;
	bool Error:1;
	bool DataSent:1;
	int  CurProfile;
	int  CurPage;
};
struct CurDownload{
	bool Started;
	bool Finished;
	bool Error;
	int  CurProfile;
	int  CurPage;
};
class InternetStatsChunks{
public:
	int CurIndex;
	bool Ready;
	bool Error;
	int N;
	OneIChunk** Chunks;
	CurDownload CDOWN;
	CurUpload CUPLD;

	InternetStatsChunks();
	void Init();
	void Clear();
	int ReadChunkFrom(byte* Sourse);
	void WriteChunk(byte* Src,int L,byte Type);
	bool Upload(int ProfileID,char* Password,int Page);
	bool Download(int ProfileID,int Page);
	//----downloading process----//
	void StartDownload(int ProfileID,int Page);
	void ProcessDownload();
	void StartUpload(int ProfileID,char* Password,int Page);
	void ProcessUpload();
};

void STARTIGAME(char* GameName,char* Map,char* Nick,DWORD G_GUID);
void UPDATEIGAME();
void LOOSEIGAME(char* Nick);
void WINIGAME(char* Nick);

struct OneTopClient{
	int ProfileID;
	int Score;
	int NBattl;
	int NVict;
	int NIncomp;
	int LastRequestTime;

	bool OLD_Profile:1;
	bool OLD_PLINF:1;
	bool OLD_PARAM:1;
};
