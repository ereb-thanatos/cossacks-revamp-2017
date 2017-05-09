class OneSicWindow;
typedef DWORD fnSendRequest(char* request, int size);
typedef DWORD fnGetRequestResult(DWORD Handle, char** Result, int* Size);
typedef void fnCloseRequest(DWORD Handle);
typedef void fnProcess();
typedef void fnCloseAll();
struct OneVariable {
	char Name[48];
	char* Value;
	int MaxLen;
};
class sicExplorer;
typedef void fnExpProcess(sicExplorer* SXP, void* PData, int size);
struct OneSXProcess {
	fnExpProcess* Process;
	fnExpProcess* Close;
	void* Data;
	int size;
};
struct OneSXPTable {
	char ID[32];
	byte COLMOPT[32];
	int NLines;
	int NRefs;
	int NCol;
	int MaxLines;
	char** Lines;
	int* Refs;
};
struct FileDownloadProcess {
	char  URL[256];
	char  Name[256];
	DWORD Handle;
	byte  ReqMask[128];
	bool  Ready;
};
class sicExplorer {
public:
	char HomeAddress[512];
	char ACCESSKEY[16];
	DWORD ID;

	OneVariable* GVARS;
	int NGVars;
	int MaxGVars;

	OneSXPTable* Tables;
	int NTables;

	FileDownloadProcess* DOWNL;
	int NDownl;
	int MaxDownl;

	int LastReparseTime;

	int GetGPPictureIndex(char* Name);

	//server information
	HMODULE ServerDLL;
	fnSendRequest* SendRequest;
	fnGetRequestResult* GetRequestResult;
	fnCloseRequest* CloseRequest;
	fnProcess* Process;
	fnCloseAll* CloseAll;

	//geometry
	int x, y, x1, y1;
	char FONT1[16];
	char FONT2[16];
	char FONT3[16];
	HMODULE H;

	OneSicWindow** Windows;
	int NWindows;
	int MaxWindow;
	int CurWPosition;
	bool BaseDialogActive;

	RLCFont* GetFontByName(char* fnt);

	//bool RegisterServer(char* DLL_path,char* home);
	void RegisterOutput(int x, int y, int x1, int y1);
	void ChangeOutput(int x, int y, int x1, int y1);
	//void OpenWindow(char* res,int size);
	void NewWindow(char* ref, char* WinID);
	//void OpenRef(char* request);
	void StepBack();
	void StepForward();
	void Refresh();
	void EraseAllDialogs();

	char* CreateVar(char* Name, int Len);
	char* GetVar(char* Name);

	OneSXPTable* CreateTable(char* Name);
	OneSXPTable* GetTable(char* Name);
	void SendTableRefresh(char* Name, char* server);

	char* CreateGVar(char* Name, int Len);
	char* GetGVar(char* Name);

	void* CreateSXProcess(fnExpProcess* Process, fnExpProcess* Close, int size);

	~sicExplorer();
};
struct InterfaceElementPosition {
	char ID[16];
	int x, y, x1, y1;
	int StartSD;
	int FinSD;
};
class OneBox {
public:
	DialogsSystem DSS;
	char Name[16];
	int x, y, x1, y1;    //external coordinates
	int xi, yi, xi1, yi1;//internal coordinates
	bool Scroll : 1;    //have scroller?
	bool Box : 1;       //have box?
	bool Active : 1;
	bool WasActive : 1;
	short StartScrollIndex;
	int LastScrollPos;
	VScrollBar* VS;

	InterfaceElementPosition* ISPOS;
	int NISPOS;
	int MaxISPOS;

	OneBox();
	~OneBox();
};
class OneInterfaceElement {
public:
	char ID[16];
	int x, y, x1, y1;
	char** ActiveRefs;
	int NActiveRefs;
	char** Params;
	int NParams;
	OneInterfaceElement();
	void ClearActive();
	void ClearParams();
	void Clear();
	~OneInterfaceElement() {
		Clear();
	};
};
struct PanelParam {
	char panel_gp[128];
	int LU, RU, LD, RD, L, R, U, D, C, C1;
};
struct OneAddFont {
	char FID[32];
	RLCFont FONT;
	int sdx, sdy;
};
#define NTIME 8
class OneSicWindow {
public:
	char WinID[16];
	sicExplorer* EXP;
	char* REF;
	DWORD GHANDLE;
	DWORD UPHANDLE;
	bool Ready : 1;
	bool Parsed : 1;
	bool Error : 1;
	bool RefsSent : 1;
	char* Result;
	int MaxL;

	OneAddFont* ADFonts;
	int NAddFonts;

	int TimeLimit[NTIME];
	char TimeLimitRequest[NTIME][160];

	char CurrTempCml[64];
	char* ResizeComm;

	//variables
	OneVariable* VARS;
	int NVARS;
	//processes
	OneSXProcess* PRC;
	int NPRC;
	OneSXPTable* Tables;
	int NTables;

	OneBox* Boxes;
	int NBoxes;
	int MaxBoxes;
	//pre-defined panels
	PanelParam PANELS[16];
	//#def_panel(Index,gp-file,LU,RU,LD,RD,L,R,U,D,C,C1)
	//--------------REDEFINED-STANDART-PARAMETERS-------------//
	//1.  #box  -  #def_box
	//#def_box(gpfile,LU,RU,LD,RD,L,R,U,D,DxL,DyL,DxR,DyR)
	//bool ReDefBox;
	char box_gp_file[128];
	int b_LU, b_RU, b_LD, b_RD, b_L, b_R, b_U, b_D, b_DxL, b_DyL, b_DxR, b_DyR;
	//2.  #sbox -  #def_sbox
	//#def_box(gpfile,LU,RU,LD,RD,L,R,U,D,DxL,DyL,DxR,DyR,sc_dx,sc_dy,sc_dly)
	//bool ReDefSBox;
	char sbox_gp_file[128];
	int sb_LU, sb_RU, sb_LD, sb_RD, sb_L, sb_R, sb_U, sb_D, sb_DxL, sb_DyL, sb_DxR, sb_DyR, sc_dx, sc_dy, sc_dly;
	//3. scroller
	//#def_scroll(gpfile,index,dscroll)
	//bool ReDefScroll;

	char scrl_gp_file[128];
	int scr_Idx, scr_dscroll;

	//4. combo box
	//#def_combo(gpfile,index,dx)
	char combo_gp_file[128];
	int cmb_Idx, cmb_bdx;
	//5. table
	//#def_tbl(MainBorderID,ScrollBorderID,SelBorderID,MouseBorderID,UpperLy,LowLy,OneLineLy,
	//MarginL,MarginR,EditMarginL,EditMarginR,FilterMarginL,FilterMarginR,FilterMarginU)
	int CTBL_UpperLY;
	int CTBL_LowLY;
	int CTBL_OLY;
	int CTBL_MainBorderID;
	int CTBL_ScrollBorderID;
	int CTBL_SelBorderID;
	int CTBL_MouseBorderID;
	//margins
	int CTBL_MarginL;
	int CTBL_MarginR;
	int CTBL_EditMarginL;
	int CTBL_EditMarginR;
	int CTBL_FilterMarginL;
	int CTBL_FilterMarginR;
	int CTBL_FilterMarginU;
	//panels
	//6. #def_panels(gpfile)
	char panel_gp_file[128];
	//7. Standart button
	//#def_btn(gp-file,index,dx,dy,wholedx,wholedy,whole_Lx)
	char  stb_gp_file[128];
	int stb_Index;
	int stb_dx;
	int stb_dy;
	int stb_wholedx;
	int stb_wholedy;
	int stb_wholeLx;
	//8. ТОЖЕ Standart button
	//#def_sbtn(gp-file,index,dx,dy,wholedx,wholedy,whole_Lx)
	char  sstb_gp_file[128];
	int sstb_Index;
	int sstb_dx;
	int sstb_dy;
	int sstb_wholedx;
	int sstb_wholedy;
	int sstb_wholeLx;

	int btn_style;
	//#def_gp_btn(gp-file,Active,Passive,dx,dy)
	char gpb_file[128];
	int gpb_active;
	int gpb_passive;
	int gpb_dx;
	int gpb_dy;
	//----------------------END-BLOCK-------------------------//
	//void Open(char* ref);
	int ParseTheWholeText();
	void DownloadRefs();
	OneSicWindow(sicExplorer* EXPL);
	void Process();
	void Erase();
	char* CreateVar(char* Name, int Len);
	char* GetVar(char* Name);
	void* CreateSXProcess(fnExpProcess* Process, fnExpProcess* Close, int size);
	void SaveCookies();
	void LoadCookies();
	void ReParse();
	void LoadDefaultSettings();
	OneSicWindow();
	~OneSicWindow();
};
typedef bool tpAddInterface(sicExplorer* SXP, DialogsSystem* DSS, int* x, int* y, int* x1, int* y1,
	int NActive, char** Active, int NParam, char** Param, char* param);
class OneInterfaceFunction {
public:
	char Name[16];
	bool ReqName : 1;
	bool ReqCoor : 1;
	bool ReqActiv : 1;
	bool ReqParam : 1;
	char param[32];
	tpAddInterface* AddInterface;
};
extern OneInterfaceFunction* IFNS;
extern int N_IFNS;
extern int SCROLL2;
extern int SCROLL0;
extern int CBB_2;
extern int CBB_0;
__declspec(dllimport)
byte GetPaletteColor(int r, int g, int b);
DWORD SendGlobalRequest(sicExplorer* SXP, char* data, bool allow);
struct Corners {
	int CLU, CRU, CLD, CRD;
	int LL, LR, LU, LD;
};
__declspec(dllimport)
void DrawFilledRect3(int x0, int y0, int x1, int y1, Corners* CR, int GP_File, int StartFill, int NFill);
__declspec(dllimport)
void DrawRect3(int x0, int y0, int x1, int y1, Corners* CR, int GP_File);
//DWORD SendGlobalRequest(ParsedRQ* RQ);
__declspec(dllimport)
void _ExFree(void* ptr);
__declspec(dllimport)
void* _ExMalloc(int Size);
__declspec(dllimport) void SetExComm(char* Data, int size);
__declspec(dllimport) bool CheckGP_Inside(int FileIndex, int SprIndex, int dx, int dy);
