class WIcon{
public:
	char* Message;
	word FileID;
	word Spr;
	byte Kind;
	word MagReq;
	//==0 - directory entry
	//==1 - uniq command, no parameters
	//==2 - uniq command, (x,y) requrired
	//==3 - upgrade
	//==4 - producind
	word Param1;
	word Param2;
	word* SubList;
};
