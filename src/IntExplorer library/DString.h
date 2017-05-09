class DString;
class DString{
public:
	char* str;
	int L;
	int MaxL;
	DString();
	DString(char* s);
	~DString();
	void Allocate(int sz);
	void ReadFromFile(char* file);
	void WriteToFile (char* file);
	void Add(DString& Str);
	void Add(char* Str);
	void Replace(char* src,char* dst);
	void Replace(char* src,DString& dst);
	void Clear();
	void Free();
	void Assign(char* str);
	void Assign(DString& str);
	DString& operator = (char* s);
	DString& operator + (char* s);
	DString& operator + (DString& ds1);
	DString& operator += (char* s);
	DString& operator += (DString& ds);
	bool operator == (char* s);
	bool operator == (DString& ds);
	char operator [] (int index);
};