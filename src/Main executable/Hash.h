struct LocalHashItem{
	char* ID;
	char* Message;
	DWORD Value;
};
class LocalHash{
public:
	LocalHashItem* Items;
	int NItems;
	int MaxItems;

	LocalHash();
	~LocalHash();
	void Clear();
	void AddToHash(LocalHashItem* Item);
	void Delete(char* ID);

};
class GlobalHash{
public:
	LocalHash Hash[256];
	LocalHash GlobalList;
	void AddToHash(char* ID,char* Message,DWORD Value);
	bool GetHashItem(char* ID,LocalHashItem** Item);
	int FindItem(char* ID_Start,int StartSearchPos,LocalHashItem** Item);//returns next search index
	void Clear();
	GlobalHash();
	~GlobalHash();
};