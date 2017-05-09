typedef void fnc(void);
struct AZone{
	int x,y,x1,y1;
};
class Menu{
public:
	Menu(void);
	bool	Active:1;
	bool	MakeShift:1;
	bool	ChangeFont:1;
	bool	EraseOnChoose:1;
	LPCSTR	items;
	int		MenuLy;
	int		ItemLy;
	int		NItems;
	int		StartY;
	fnc*	RedrawBackground;
	AZone	Zone[16];
	int		NZones;
	int		choose;;
	lpRLCFont font;
	lpRLCFont font1;
	int		evHandler;
	void	CreateMenu(LPCSTR s,lpRLCFont f,lpRLCFont f1,fnc* ff,int style);
	void	Show(int n);
	int		GetAmount();
	LPCSTR  GetNstr(int n);
	int		GetLen(LPCSTR s);
	void	ShowModal();
	void	Close();
};
#define MCP_MAKESHIFT 2
#define MCP_CHANGEFONT 4
#define MCP_ERASEONCHOOSE 8
