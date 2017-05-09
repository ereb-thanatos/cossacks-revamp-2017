class SaveBuf{
public:
	byte* Buf;
	int Pos;
	int Size;
	int RealSize;
	SaveBuf();
	void Init();
	~SaveBuf();
	void Clear();
	void SaveToFile(ResFile f1);
	void LoadFromFile(ResFile f1);
};
void PreSaveGame(SaveBuf* SB,char* Messtr,int ID);
void SFLB_PreLoadGame(SaveBuf* SB,bool LoadNation);
void SFLB_LoadGame(char* fnm,bool LoadNation);
void SaveGame(char* Name,char* Messtr,int ID);
void xBlockRead(SaveBuf* SB,void* Data,int Size);
void xBlockWrite(SaveBuf* SB,void* Data,int Size);