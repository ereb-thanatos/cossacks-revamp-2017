typedef void EventHandPro(void*);
int RegisterEventHandler(EventHandPro* pro,int Type,void*);
void CloseEventHandler(int i);