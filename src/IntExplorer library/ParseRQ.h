void* ie_malloc(int size,char* File,int Line);
void* ie_realloc(void* data,int size,char* File,int Line);
void ie_free(void* data);
/*
#define free ie_free
#define malloc(x) ie_malloc(x,__FILE__,__LINE__)
#define realloc(x,y) ie_realloc(x,y,__FILE__,__LINE__)
*/
struct OneComm{
	char ComID[32];
	int NParams;
	char** Params;
	int* ParamSize;
};
class ParsedRQ{
public:
	char DevName[16];
	OneComm* Comm;
	int NComm;

	void AddComm(char* Name);
	void AddParam(char* data,int size);
	void AddIntParam(int val);
	void DelComm(int pos);
	void AddParamToCom(int idx,char* data,int size);

	void Clear();
	void Parse(char* s);
	int  UnParse(char* dst,int size);
	int  Compact(char* dst,int size);
	void Extract(char* s,int size);
	ParsedRQ();
	~ParsedRQ();
};
