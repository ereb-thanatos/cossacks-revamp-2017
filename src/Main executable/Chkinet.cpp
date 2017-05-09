#include "ddini.h"
bool CheckInternet(){
	return gethostbyname("peerchat.gamespy.com")!=NULL;
};