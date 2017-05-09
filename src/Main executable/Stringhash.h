struct StrHashItem{
	int NStr;
	int MaxStr;
	char** Str;
	int* Value;
};
class StrHash{
public:
	StrHashItem SHI[256];
	int LastIndex;
	void AddString(char* str);
	int SearchString(char* str);
	void Clear();
	StrHash();
	~StrHash();
};