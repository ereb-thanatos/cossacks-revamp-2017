class OneObject;
class SimpleUTP{
public:
	char* Message;
	byte NLinks;
	byte Kind;
	int* Links;
	byte NAuto;
	int* AutoPerf;
	bool Enabled:1;
	bool OneTime:1;
	bool Done:1;
	bool Finished:1;
	word IFileID;
	word IFIndex;
	word Cost;
	word Wood;
	word Oil;
	word MonsterID;
	word Time;
	word Stage;
};

class UTP1:public SimpleUTP{
public:
	byte ResType;
	byte AddVal;
};
class UTP2:public SimpleUTP{
public:
	word MID;
	byte CharID;
	byte AddVal;
};
class UTP3:public SimpleUTP{
public:
	word FinalMID;
};
class Upgrade{
public:
	union{
		UTP1* utp1[1024];
		UTP2* utp2[1024];
		UTP3* utp3[1024];
		SimpleUTP* utp[1024];
	};
	int NUpgrades;
	Upgrade();
};
