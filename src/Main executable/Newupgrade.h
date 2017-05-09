bool UnderstandUpgrade(GFILE* f,char* UpgName,Nation* NT,char* name,int* lpLine,byte NID);
void PerformNewUpgrade(Nation* NT,int UIndex,OneObject* OB);
void CreateAdvCharacter(AdvCharacter* AC,NewMonster* NM);
int GetUpgradeID(Nation* NT,char* Name);