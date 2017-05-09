typedef unsigned char byte;
#pragma pack(1)
struct BMPformat{ // bmfh 
        WORD    bfType; 
        DWORD   bfSize; 
        WORD    bfReserved1; 
        WORD    bfReserved2; 
        DWORD   bfOffBits; 
		DWORD   biSize; 
		LONG    biWidth; 
		LONG    biHeight; 
		WORD    biPlanes; 
		WORD    biBitCount;
		DWORD   biCompression; 
		DWORD   biSizeImage; 
		LONG    biXPelsPerMeter; 
		LONG    biYPelsPerMeter; 
		DWORD   biClrUsed; 
		DWORD   biClrImportant; 
};
void SaveToBMP24(char* Name,int Lx,int Ly,byte* data);
bool ReadBMP24(char* Name,BMPformat* BM,byte** data);
bool ReadBMP8(char* Name,BMPformat* BM,byte** data);
bool ReadBMP8TOBPX(char* Name,byte** data);