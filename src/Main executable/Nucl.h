class HugeExplosion{
public:
	bool Enabled;
	int x;
	int y;
	int Radius1;
	int Radius2;
	int r1;
	int r2;
	int vr1;
	int vr2;
	int NR1;
	int NR2;
	void CreateExpl(int x,int y,int r1,int r2,int time);
	void HandleExpl();
};
void AddHugExp(int ex,int ey,int er1,int er2,int time);
void HandleHExpls();