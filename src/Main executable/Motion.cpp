#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "fog.h"
#include "Megapolis.h"

#include <assert.h>
#include "walls.h"
#include "mode.h"
#include "GSound.h"
#include "MapSprites.h"
#include "NewMon.h"
#include "Math.h"
#include "GP_Draw.h"
#include "RealWater.h"
#include "NewUpgrade.h"
#include "ZBuffer.h"
#include "Path.h"
#include "TopoGraf.h"
MotionField UnitsField;
typedef bool PhysicsFn(OneObject* OB);
extern short TSin[257];
extern short TCos[257];
word GetDir(int dx,int dy);
void RunLeftVeslo(OneObject* OB,bool State);
void RunRightVeslo(OneObject* OB,bool State);
bool CalculateMotion2(OneObject* OB){
	NewMonster* NM=OB->newMons;
	int MaxV=NM->MotionDist;
	GeneralObject* GO=OB->Ref.General;
	AdvCharacter* ADC=GO->MoreCharacter;
	MaxV=(MaxV*ADC->Rate[0])>>(4-SpeedSh);
	int MinV=MaxV>>1;
	if(NM->NVesel)MaxV+=16;
	if(OB->DestX>0){
		int DIS=Norma(OB->DestX-OB->RealX,OB->DestY-OB->RealY);
		if(DIS>64){
			if(OB->Speed<MaxV){
				OB->Speed+=6;
			}else OB->Speed=MaxV;
			char DIR1=char(GetDir(OB->DestX-OB->RealX,OB->DestY-OB->RealY));
			char ddir=DIR1-OB->RealDir;
			if(abs(ddir)<4){
				OB->RealDir=DIR1;
				OB->RealVx=div((OB->DestX-OB->RealX)*OB->Speed,DIS).quot;
				OB->RealVy=div((OB->DestY-OB->RealY)*OB->Speed,DIS).quot;
				OB->GraphDir=OB->RealDir;
				RunLeftVeslo(OB,true);
				RunRightVeslo(OB,true);
				if(NM->NVesel){
					NewAnimation* VES=NM->Veslo;
					if(OB->LeftVeslo<128){
						if(OB->LeftVeslo<128)OB->LeftVeslo++;
						if(OB->LeftVeslo>=VES->NFrames)OB->LeftVeslo=0;
					}else{
						if(OB->LeftVeslo!=128+4)OB->LeftVeslo++;
						if(OB->LeftVeslo>=128+VES->NFrames)OB->LeftVeslo=128;
					};
					if(OB->RightVeslo<128){
						if(OB->RightVeslo<128)OB->RightVeslo++;
						if(OB->RightVeslo>=VES->NFrames)OB->RightVeslo=0;
					}else{
						if(OB->RightVeslo!=128+4)OB->RightVeslo++;
						if(OB->RightVeslo>=128+VES->NFrames)OB->RightVeslo=128;
					};
					if(OB->LeftVeslo!=OB->RightVeslo&&OB->LeftVeslo<128&&OB->RightVeslo<128){
						if(OB->LeftVeslo<OB->RightVeslo)OB->RightVeslo--;
						else OB->LeftVeslo--;
					}
					if(OB->LeftVeslo<NM->VesStart||OB->LeftVeslo>NM->VesFin)if(OB->Speed>MinV)OB->Speed-=9;
				};
				return true;
			}else{
				if(ddir>0){
					OB->RealDir+=2;
					RunLeftVeslo(OB,true);
					RunRightVeslo(OB,false);
				}else{ 
					OB->RealDir-=2;
					RunLeftVeslo(OB,false);
					RunRightVeslo(OB,true);
				};
				OB->RealVx=(OB->Speed*TCos[OB->RealDir])>>8;
				OB->RealVy=(OB->Speed*TSin[OB->RealDir])>>8;
			};
		}else{
			OB->DestX=-1;
		};
		OB->GraphDir=OB->RealDir;
		
	}else{
		int V=OB->Speed;
		if(V<=8){
			OB->RealVx=0;
			OB->RealVy=0;
			OB->Speed=0;
		}else{
			OB->RealVx-=OB->RealVx>>2;
			OB->RealVy-=OB->RealVy>>2;
			OB->Speed-=OB->Speed>>2;
		};
	};
	
	return false;
};
//				   0   1   2   3   4   5   6   7   0   1
const int DX8[10]={0  ,0  ,1  ,0  ,0  ,-1 ,-1 ,-1 ,0  ,0};
const int DY8[10]={-1 ,-1 ,0  ,0  ,1  ,0  ,0  ,-1 ,-1 ,-1};
const int DLX[10]={0  ,1  ,0  ,1  ,0  ,1  ,0  ,1  ,0  ,1};
const int DLY[10]={0  ,1  ,0  ,1  ,0  ,1  ,0  ,1  ,0  ,1};
bool TryDir(OneObject* OB,char Dir){
	Dir=(Dir+8)&7;
	int ULX=int(OB->Lx)<<7;
	int NewX=((OB->RealX-ULX)>>8)+DX8[Dir];
	int NewY=((OB->RealY-ULX)>>8)+DY8[Dir];
	MotionField* MFI=MFIELDS+OB->LockType;
	return !(MFI->CheckBar(NewX,NewY,OB->Lx+(Dir&1),OB->Lx+(Dir&1))||
		UnitsField.CheckBar(NewX,NewY,OB->Lx+(Dir&1),OB->Lx+(Dir&1)));
};
bool CalculateMotion2Limited(OneObject* OB){
	char Dir=char(GetDir(OB->RealVx,OB->RealVy));
	char dir8=(Dir+80)>>5;
	dir8=(dir8+16)&7;
	int Dir0=(dir8<<5)-64;
	char DDD=Dir-Dir0;
	char DirNew=Dir;
	if(!TryDir(OB,dir8)){
		if(DDD>0){
			if(!TryDir(OB,dir8+1)){
				if(!TryDir(OB,dir8-1)){
					if(!TryDir(OB,dir8+2))return false;
					else DirNew=Dir0+64;
				}else DirNew=Dir0-32;
			}else DirNew=Dir0+32;
		}else{
			if(!TryDir(OB,dir8-1)){
				if(!TryDir(OB,dir8+1)){
					if(!TryDir(OB,dir8-2))return false;
					else DirNew=Dir0-64;
				}else DirNew=Dir0+32;
			}else DirNew=Dir0-32;
		};
	}else DirNew=Dir0;
	//new direction is found!
	char ddir=DirNew-OB->RealDir;
	if(abs(ddir)<4){
		OB->RealDir=DirNew;
		OB->GraphDir=OB->RealDir;
		RunLeftVeslo(OB,true);
		RunRightVeslo(OB,true);
	}else{
		if(ddir>0){
			OB->RealDir+=4;
			RunLeftVeslo(OB,true);
			RunRightVeslo(OB,false);
		}else{
			OB->RealDir-=4;
			RunLeftVeslo(OB,false);
			RunRightVeslo(OB,true);
		};
	};
	OB->RealVx=(OB->Speed*TCos[OB->RealDir])>>8;
	OB->RealVy=(OB->Speed*TSin[OB->RealDir])>>8;
	return true;
};
bool CalculateMotion2LimitedOld(OneObject* OB){
	char Dir=char(GetDir(OB->RealVx,OB->RealVy));
	char dir8=(Dir+80)>>5;
	dir8=(dir8+16)&7;
	int Dir0=(dir8<<5)-64;
	char DDD=Dir-Dir0;
	char DirNew=Dir;
	if(!TryDir(OB,dir8)){
		if(DDD>0){
			if(!TryDir(OB,dir8+1)){
				if(!TryDir(OB,dir8-1)){
					if(!TryDir(OB,dir8+2))return false;
					else DirNew=Dir0+64;
				}else DirNew=Dir0-32;
			}else DirNew=Dir0+32;
		}else{
			if(!TryDir(OB,dir8-1)){
				if(!TryDir(OB,dir8+1)){
					if(!TryDir(OB,dir8-2))return false;
					else DirNew=Dir0+64;
				}else DirNew=Dir0+32;
			}else DirNew=Dir0-32;
		};
	}else DirNew=Dir0;
	//new direction is found!
	char ddir=DirNew-OB->RealDir;
	if(abs(ddir)<4){
		OB->RealDir=DirNew;
		OB->GraphDir=OB->RealDir;
	}else{
		if(ddir>0){
			OB->RealDir+=4;
			RunLeftVeslo(OB,true);
			RunRightVeslo(OB,false);
		}else{
			OB->RealDir-=4;
			RunLeftVeslo(OB,false);
			RunRightVeslo(OB,true);
		};
	};
	OB->RealVx=(OB->Speed*TCos[OB->RealDir])>>8;
	OB->RealVy=(OB->Speed*TSin[OB->RealDir])>>8;
	return true;
};
void MoveObject(OneObject* OB){
	int vx=OB->RealVx;
	int vy=OB->RealVy;
	OB->RealX+=vx;
	OB->RealY+=vy;
	//if(vx||vy)OB->StandTime=0;
	//else OB->StandTime++;
	NewMonster* NM=OB->newMons;
	OB->NewAnm=&NM->MotionL;
	OB->NewCurSprite=0;
	rando();
};
void StopObject(OneObject* OB){
	TryToStand(OB,false);
	OB->Speed=0;
	OB->RealVx=0;
	OB->RealVy=0; 
	//OB->StandTime++;
};
void EraseLodkiInCell(int cell,int x,int y,int Lx,int Ly){
	cell+=VAL_MAXCX+1;
    int NMon=MCount[cell];
	if(!NMon)return;
	int ofs1=cell<<SHFCELL;
	word MID;
	for(int i=0;i<NMon;i++){
		MID=GetNMSL(ofs1+i);
		if(MID!=0xFFFF){
			OneObject* OB=Group[MID];
			if(OB&&OB->newMons->Usage==FisherID){
				int mx=x+x+Lx;
				int my=y+y+Ly;
				int eLx=OB->Lx;
				int ex=(OB->x<<1)+eLx;
				int ey=(OB->y<<1)+eLx;
				int dx=abs(mx-ex);
				int dy=abs(my-ey);
				int r=dx<dy?dy:dx;
				if(r<Lx+eLx)
					OB->Die();
			};
		};
	};
};
int GetShipDangerInCell(int cell,int x,int y){
	int DNG=0;
	cell+=VAL_MAXCX+1;
    int NMon=MCount[cell];
	if(!NMon)return 0;
	int ofs1=cell<<SHFCELL;
	word MID;
	for(int i=0;i<NMon;i++){
		MID=GetNMSL(ofs1+i);
		if(MID!=0xFFFF){
			OneObject* OB=Group[MID];
			if(OB&&OB->LockType&&OB->newMons->Usage!=FisherID){
				int rx=(OB->RealX>>4)-x;
				int ry=(OB->RealY>>4)-y;
				int vx=OB->RealVx<<2;
				int vy=OB->RealVy<<2;
				int r0=Norma(rx,ry);
				int VX=vx;int VY=vy;
				for(int p=0;p<10;p++){
					int r=Norma(rx+VX,ry+VY);
					if(r<r0)r0=r;
					VX+=vx;
					VY+=vy;
				};
				if(r0<80)r0=80;
				DNG+=10000/r0;
			};
		};
	};
	return DNG;
};
int GetShipDanger(int x,int y){
	int x0=x>>7;
	int y0=y>>7;
	int maxx=msx>>2;
	int maxy=msy>>2;
	int DNG=0;
	for(int r=0;r<5;r++){
		char* xi=Rarr[r].xi;
		char* yi=Rarr[r].yi;
		int N=Rarr[r].N;
		for(int i=0;i<N;i++){
			int xx=x0+xi[i];
			int yy=y0+yi[i];
			if(xx>=0&&yy>=0&&xx<maxx&&yy<maxy){
				DNG+=GetShipDangerInCell(xx+(yy<<VAL_SHFCX),x,y);
			};
		};
	};
	return DNG;
};
int GetShipDanger1(int x,int y){
	int x0=x>>7;
	int y0=y>>7;
	int maxx=msx>>2;
	int maxy=msy>>2;
	int DNG=0;
	for(int r=0;r<7;r++){
		char* xi=Rarr[r].xi;
		char* yi=Rarr[r].yi;
		int N=Rarr[r].N;
		for(int i=0;i<N;i++){
			int xx=x0+xi[i];
			int yy=y0+yi[i];
			if(xx>=0&&yy>=0&&xx<maxx&&yy<maxy){
				DNG+=GetShipDangerInCell(xx+(yy<<VAL_SHFCX),x,y);
			};
		};
	};
	return DNG;
};
void EraseLodki(int x,int y,int Lx,int Ly){
	int x0=x>>3;
	int y0=y>>3;
	int maxx=msx>>1;
	int maxy=msy>>1;
	for(int r=0;r<3;r++){
		char* xi=Rarr[r].xi;
		char* yi=Rarr[r].yi;
		int N=Rarr[r].N;
		for(int i=0;i<N;i++){
			int xx=x0+xi[i];
			int yy=y0+yi[i];
			if(xx>=0&&yy>=0&&xx<maxx&&yy<maxy){
				EraseLodkiInCell(xx+(yy<<VAL_SHFCX),x,y,Lx,Ly);
			};
		};
	};
};
void AddTrace(int x,int y,byte Dir);
void CreateTrace(OneObject* OB);
bool RemoveShipsInCell(int cell,int x,int y,int Lx,OneObject* MyObj){
	cell+=VAL_MAXCX+1;
	if(cell<0||cell>=VAL_MAXCX*MAXCY)return false;
	word MyMid=MyObj->Index;
	int mx1=x+Lx-1;
	int my1=y+Lx-1;
    int NMon=MCount[cell];
	if(!NMon)return false;
	int ofs1=cell<<SHFCELL;
	word MID;
	int cx=(x<<1)+Lx;
	int cy=(y<<1)+Lx;
	for(int i=0;i<NMon;i++){
		MID=GetNMSL(ofs1+i);
		if(MID!=0xFFFF){
			OneObject* OB=Group[MID];
			if(OB&&(!OB->Sdoxlo)&&OB->Index!=MyMid&&OB->LockType&&!(OB->Transport&&OB->DstX>0)){
				int olx=OB->Lx;
				int ox=((OB->x)<<1)+olx;
				int oy=((OB->y)<<1)+olx;
				int dx=abs(ox-cx);
				int dy=abs(oy-cy);
				if(!(dx>=(olx+Lx)||dy>=(olx+Lx))){
					//intersection
					if(OB->Speed<(100<<SpeedSh)){
						OB->Speed=(100<<SpeedSh);
						OB->RealVx=(OB->Speed*TCos[OB->RealDir])>>8;
						OB->RealVy=(OB->Speed*TSin[OB->RealDir])>>8;
					};
				};
			};
		};
	};
	return false;
};
void TryToPushShipsBy(OneObject* MyMid,int x,int y,int Lx){
	int cell=((x+6)>>3)+(((y+6)>>3)<<VAL_SHFCX);
	byte D=MyMid->RealDir;
	int DX=0;
	int DY=0;
	if(D==0)x++;
	else if(D==64)DY=1;
	else if(D==128)DX=-1;
	else if(D==192)DY=-1;
	else if(D>0&&D<64){DX=1;DY=1;}
	else if(D>64&&D<128){DX=-1;DY=1;}
	else if(D>128&&D<192){DX=-1;DY=-1;}
	else {DX=1;DY=-1;};
	if(DX){
		for(int dx=-2;dx<=2;dx++){
			for(int dy=-2;dy<=2;dy++){
				RemoveShipsInCell(cell+dx+dy*VAL_MAXCX,x+DX,y,Lx,MyMid);
			};
		};
	};
	if(DY){
		for(int dx=-2;dx<=2;dx++){
			for(int dy=-2;dy<=2;dy++){
				RemoveShipsInCell(cell+dx+dy*VAL_MAXCX,x,y+DY,Lx,MyMid);
			};
		};
	};
};
void PerformMotion2(OneObject* OB){
	//if(OB->delay)OB->delay--;
	if(OB->BackDelay>=FrmDec)OB->BackDelay-=FrmDec;
	else OB->BackDelay=0;
    //OB->SearchVictim();
	bool FirstTime=true;
	bool CanCrashLimit=false;
	bool LODKA=OB->newMons->Usage==FisherID;
	if(OB->Sdoxlo)return;
	if(OB->NewCurSprite>=OB->NewAnm->NFrames-FrmDec){
		NewMonster* NMN=OB->newMons;
		AdvCharacter* ADC=OB->Ref.General->MoreCharacter;
		NMN->AdvChar=ADC;
		byte OLDDIR=OB->RealDir;
		bool MRes=CalculateMotion2(OB);
CalcMotion:
		if(OB->Speed){
			int NSp=NMN->MotionL.NFrames;
			int NewX;
			int NewY;
			if(OB->RealVx||OB->RealVy){
				CreateTrace(OB);
				//OB->StandTime=0;
			};
			if(NSp>1){
				NewX=OB->RealX+OB->RealVx*NSp;
				NewY=OB->RealY+OB->RealVy*NSp;
			}else{
				NewX=OB->RealX+OB->RealVx;
				NewY=OB->RealY+OB->RealVy;
			};
			int LX=OB->Lx;
			int ULX=(LX)<<7;
			int NewLockX=(NewX-ULX)>>8;
			int NewLockY=(NewY-ULX)>>8;
			int OldLockX=OB->x;
			int OldLockY=OB->y;
			MotionField* MFI=MFIELDS+OB->LockType;
			//assert(!MFI->CheckBar(OldLockX,OldLockY,LX,LX));
			if(OB->GLock)MFI->BClrBar(OldLockX,OldLockY,LX);
			if(OB->LLock)UnitsField.BClrBar(OldLockX,OldLockY,LX);
			bool AllowGMotion=!MFI->CheckBar(NewLockX,NewLockY,LX,LX);
			bool AllowLMotion=!UnitsField.CheckBar(NewLockX,NewLockY,LX,LX);
			if(!LODKA){
				EraseLodki(NewLockX,NewLockY,LX,LX);
			};
			if(AllowGMotion&&AllowLMotion){
				MoveObject(OB);
				OB->GLock=false;
				UnitsField.BSetBar(NewLockX,NewLockY,LX);
				OB->x=NewLockX;
				OB->y=NewLockY;
				OB->StandTime=0;
			}else{
				//OB->StandTime++;
				MRes=true;
				//can't move at once, G||L locking present
				if(MRes){
					//it moves in right direction, but can't
					//Who is the cause of the problem ?
					if(AllowGMotion){
						//unknown moving unit is the cause of the problem;
						if(FirstTime){
							CanCrashLimit=CalculateMotion2Limited(OB);
							if(CanCrashLimit){
								FirstTime=false;
								goto CalcMotion;
							}else OB->StandTime++;
						}else OB->StandTime++;
						if(!CanCrashLimit){
							StopObject(OB);
							if(OB->StandTime>15)TryToPushShipsBy(OB,OB->x,OB->y,LX);
							//if(OB->StandTime<8){
							//	OB->StandTime++;
							//}else{
							//	OB->GLock=true;
							//	MFI->BSetBar(OldLockX,OldLockY,LX);
							//	if(OB->StandTime<100)OB->StandTime++;
							//	if(OB->StandTime>12)OB->DeletePath();
							//};
						};
						UnitsField.BSetBar(OldLockX,OldLockY,LX);
						OB->LLock=true;
					}else{
						if(FirstTime){
							CanCrashLimit=CalculateMotion2Limited(OB);
							if(CanCrashLimit){
								FirstTime=false;
								goto CalcMotion;
							};
						};
						if(!CanCrashLimit){
							//OB->GLock=true;
							//MFI->BSetBar(OldLockX,OldLockY,LX);
							//if(OB->StandTime<100)OB->StandTime++;
							if(OB->StandTime>15)
								TryToPushShipsBy(OB,NewLockX,NewLockY,LX);
							OB->StandTime++;
							OB->DeletePath();
						};
						UnitsField.BSetBar(OldLockX,OldLockY,LX);
						OB->LLock=true;
					};
				}else{
					//we can rotate object without moving
					StopObject(OB);
					if(OB->StandTime<8){
						OB->StandTime++;
					}else{
						OB->GLock=true;
						MFI->BSetBar(OldLockX,OldLockY,LX);
						if(OB->StandTime<100)OB->StandTime++;
					};
					UnitsField.BSetBar(OldLockX,OldLockY,LX);
					OB->LLock=true;
				};
			};
		}else{
			RunLeftVeslo(OB,false);
			RunRightVeslo(OB,false);
			OB->StandTime++;
		};
	};
	OB->RZ=0;//GetHeight(OB->RealX>>4,OB->RealY>>4);
};