#include "stdio.h"
#include "windows.h"
//#include "stdafx.h"
#include "ResFile.h"
#include "math.h"
#include "bmptool.h"
//arch24 format:
//dd 'AR24'
//dd Lx
//dd Ly
//db KeyRed
//db KeyGreen
//db KeyBlue
//db Reserved (0)
//data:
//db Key
//Key: XY?????? %%%%%%%%
// if(X==0&&Y==0){
//    ??????-length of pix. data
// }else
// if(X==0&&Y==1){
//    ?????? - Hi part of data length
//    %%%%%%%% - Lo part of data length
// }else
// if(X==1&&Y==0){
//    ?????? - KeyColor Length
// }else
// if(X==1&&Y==1){
//    ?????? - Hi part of KeyColor
//    %%%%%%%% - Lo part of KeyColor
// };
//
bool SaveArch24(char* Name,byte* Data,int Lx,int Ly){
	int KeyR=Data[0];
	int KeyG=Data[1];
	int KeyB=Data[2];
	byte* NData=new byte[Lx*Ly*3+256];
	byte* TMP=new byte[Lx*Ly*3+256];
	bool Type=false;
	int Len=Lx*Ly*3;
	int pos=0;
	int NPos=0;
	int DLen=0;
	int ps=0;
	while(pos<Len){
		int r=Data[pos];
		int g=Data[pos+1];
		int b=Data[pos+2];
		pos+=3;
		if(abs(r-KeyR)+abs(g-KeyG)+abs(b-KeyB)<=6){
			//Key color
			if(!Type)DLen++;
			else{
				if(DLen>63){
					NData[NPos]=0x40+(DLen>>8);
					NData[NPos+1]=DLen&0xFF;
					memcpy(NData+NPos+2,TMP,DLen*3);
					NPos+=DLen*3+2;
					DLen=1;
					Type=false;
				}else{
					NData[NPos]=DLen;
					memcpy(NData+NPos+1,TMP,DLen*3);
					NPos+=DLen*3+1;
					DLen=1;
					Type=false;
				};
			};
		}else{
			//Normal color
			if(Type){
				TMP[ps]=r;
				TMP[ps+1]=g;
				TMP[ps+2]=b;
				ps+=3;
				DLen++;
			}else{
				if(DLen>63){
					NData[NPos]=0xC0+(DLen>>8);
					NData[NPos+1]=DLen&0xFF;
					NPos+=2;
					TMP[0]=r;
					TMP[1]=g;
					TMP[2]=b;
					ps=3;
					DLen=1;
					Type=true;
				}else{
					NData[NPos]=0x80+DLen;
					NPos++;
					TMP[0]=r;
					TMP[1]=g;
					TMP[2]=b;
					ps=3;
					DLen=1;
					Type=true;
				};
			};
		};
		if(DLen==16383){
			if(Type){
				//pix. data
				NData[NPos]=0x40+(DLen>>8);
				NData[NPos+1]=DLen&0xFF;
				memcpy(NData+2,TMP,DLen*3);
				NPos+=DLen*3+2;
				DLen=0;
				ps=0;
			}else{
				//key color
				NData[NPos]=0xC0+(DLen>>8);
				NData[NPos+1]=DLen&0xFF;
				NPos+=2;
				DLen=0;
			};
		};
	};
	if(DLen){
		if(Type){
			//pix. data
			if(DLen>63){
				NData[NPos]=0x40+(DLen>>8);
				NData[NPos+1]=DLen&0xFF;
				memcpy(NData+NPos+2,TMP,DLen*3);
				NPos+=DLen*3+2;
				DLen=1;
				Type=false;
			}else{
				NData[NPos]=DLen;
				memcpy(NData+NPos+1,TMP,DLen*3);
				NPos+=DLen*3+1;
				DLen=1;
				Type=false;
			};
		}else{
			//key color
			if(DLen>63){
				NData[NPos]=0xC0+(DLen>>8);
				NData[NPos+1]=DLen&0xFF;
				NPos+=2;
			}else{
				NData[NPos]=0x80+DLen;
				NPos++;
			};
		};
	};
	char str[128];
	sprintf(str,"%s.A24",Name);
	ResFile f1=RRewrite(str);
	if(f1==INVALID_HANDLE_VALUE)return false;
	DLen='42RA';
	RBlockWrite(f1,&DLen,4);
	RBlockWrite(f1,&Lx,4);
	RBlockWrite(f1,&Ly,4);
	RBlockWrite(f1,&KeyR,1);
	RBlockWrite(f1,&KeyG,1);
	RBlockWrite(f1,&KeyB,1);
	KeyB=0;
	RBlockWrite(f1,&KeyB,1);
	RBlockWrite(f1,NData,NPos);
	RClose(f1);
	free(NData);
	free(TMP);
	return true;
};
bool ReadArch24(char* Name,BMPformat* BM,byte** data){
	ResFile f1=RReset(Name);
	if(IOresult())return false;
	int Lx,Ly;
	byte KeyR,KeyG,KeyB,zz;
	RBlockRead(f1,&Lx,4);
	if(Lx!='42RA'){
		RClose(f1);
		return false;
	};
	RBlockRead(f1,&Lx,4);
	RBlockRead(f1,&Ly,4);
	RBlockRead(f1,&KeyR,1);
	RBlockRead(f1,&KeyG,1);
	RBlockRead(f1,&KeyB,1);
	RBlockRead(f1,&zz,1);
	int DSize=RFileSize(f1)-16;
	byte* NData=new byte[DSize];
	RBlockRead(f1,NData,DSize);
	int DstLen=Lx*Ly*3;
	*data=new byte[Lx*Ly*3];
	byte* Data=*data;
	int APos=0;
	int NPos=0;
	int Cmd,Len,i;
	while(APos<DSize&&NPos<DstLen){
		Cmd=NData[APos];
		if(!(Cmd&0x80)){
			//pix. data
			if(Cmd&0x40){
				//Long data
				Cmd=((Cmd&0x3F)<<8)+NData[APos+1];
				APos+=2;
				Len=Cmd*3;
				memcpy(Data+NPos,NData+APos,Len);
				APos+=Len;
				NPos+=Len;
			}else{
				APos++;
				Len=(Cmd&0x7F)*3;
				memcpy(Data+NPos,NData+APos,Len);
				APos+=Len;
				NPos+=Len;
			};
		}else{
			//key color
			if(Cmd&0x40){
				//Long data
				Cmd=((Cmd&0x3F)<<8)+NData[APos+1];
				APos+=2;
				Len=Cmd;
				for(i=0;i<Len;i++){
					Data[NPos]=KeyR;
					Data[NPos+1]=KeyG;
					Data[NPos+2]=KeyB;
					NPos+=3;
				};
			}else{
				APos++;
				Len=Cmd&0x3F;
				for(i=0;i<Len;i++){
					Data[NPos]=KeyR;
					Data[NPos+1]=KeyG;
					Data[NPos+2]=KeyB;
					NPos+=3;
				};
			};
		};
	};
	RClose(f1);
	BM->biWidth=Lx;
	BM->biHeight=Ly;
	return true;
};