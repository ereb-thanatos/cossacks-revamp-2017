class GFILE{
	byte Buf[16384];
	int NBytesRead;
	int BufPos;
	int GlobalPos;
	int Size;
	ResFile F;
public:
	FILE* rf;
	bool RealText;
	GFILE();
	~GFILE();
	bool Open(char* Name);
	void Close();
	int ReadByte();
	int CheckByte();
	//standart functions
	int Gscanf(char* Mask,va_list args);
	int Ggetch();
};

__declspec(dllexport) GFILE* Gopen(char* Name,char* Mode);
__declspec(dllexport) int Gscanf(GFILE* F,char* mask,...);
__declspec(dllexport) int Ggetch(GFILE* F);
__declspec(dllexport) void Gprintf(GFILE* F,const char *format,...);
__declspec(dllexport) void Gclose(GFILE* F);
