class ActiveZone{
public:
	int x,y,R;
	char* Name;
	byte Dir;
//--------------------//
	void Draw();
};
class ActiveGroup{
public:
	word* Units;
	word* Serials;
	int N;
	char* Name;
	int MinX,MinY,MaxX,MaxY;
	int AminX,AminY,AmaxX,AmaxY;
	bool Visible;
	void Draw(byte c);
};
extern ActiveGroup* AGroups;
extern int NAGroups;
extern int MaxAGroups;

extern ActiveZone* AZones;
extern int NAZones;
extern int MaxAZones;
void ClearAllZones();
void RefreshAGroups();