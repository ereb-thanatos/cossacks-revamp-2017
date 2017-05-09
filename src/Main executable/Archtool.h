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
bool SaveArch24(char* Name,byte* Data,int Lx,int Ly);
bool ReadArch24(char* Name,BMPformat* BM,byte** data);