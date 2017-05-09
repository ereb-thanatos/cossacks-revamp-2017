#include "stdio.h"
#include "windows.h"
#include "ResFile.h"
#include "math.h"
#include "bmptool.h"

void SaveToBMP24(char* Name, int Lx, int Ly, byte* data)
{
	ResFile f1 = RRewrite(Name);
	BMPformat BM;
	BM.bfType = 'MB';
	int rlx = Lx * 3;
	if (rlx & 3)rlx = (rlx | 3) + 1;
	BM.bfSize = (sizeof BMPformat) + rlx*Ly;
	BM.bfReserved1 = 0;
	BM.bfReserved2 = 0;
	BM.bfOffBits = (sizeof BMPformat);
	BM.biSize = 40;
	BM.biWidth = Lx;
	BM.biHeight = Ly;
	BM.biPlanes = 1;
	BM.biBitCount = 24;
	BM.biCompression = BI_RGB;
	BM.biSizeImage = 0;
	BM.biXPelsPerMeter = 0;
	BM.biYPelsPerMeter = 0;
	BM.biClrUsed = 0;
	BM.biClrImportant = 0;
	RBlockWrite(f1, &BM, sizeof BM);
	for (int j = 0; j < Ly; j++) {
		RBlockWrite(f1, &data[3 * Lx*(Ly - j - 1)], rlx);
	};
	RClose(f1);
};
bool ReadBMP24(char* Name, BMPformat* BM, byte** data) {
	ResFile f1 = RReset(Name);
	if (f1 != INVALID_HANDLE_VALUE) {
		RBlockRead(f1, BM, sizeof BMPformat);
		if (IOresult() || BM->bfType != 'MB')return false;
		if (BM->biBitCount != 24)return false;
		*data = new byte[BM->biWidth*BM->biHeight * 3];
		int wid = BM->biWidth * 3;
		int rwid = wid;
		if (wid & 3)rwid = (wid | 3) + 1;
		for (int i = 0; i < BM->biHeight; i++) {
			RSeek(f1, (sizeof BMPformat) + (BM->biHeight - i - 1)*rwid);
			RBlockRead(f1, &((*data)[i*wid]), wid);
		};
		RClose(f1);
		return true;
	}
	else return false;
};
bool ReadBMP8(char* Name, BMPformat* BM, byte** data) {
	ResFile f1 = RReset(Name);
	if (f1 != INVALID_HANDLE_VALUE) {
		RBlockRead(f1, BM, sizeof BMPformat);
		if (IOresult() || BM->bfType != 'MB')return false;
		if (BM->biBitCount != 8)return false;
		*data = new byte[BM->biWidth*BM->biHeight];
		int wid = BM->biWidth;
		int rwid = wid;
		if (wid & 3)rwid = (wid | 3) + 1;
		for (int i = 0; i < BM->biHeight; i++) {
			RSeek(f1, (sizeof BMPformat) + 1024 + (BM->biHeight - i - 1)*rwid);
			RBlockRead(f1, &((*data)[i*wid]), wid);
		};
		RClose(f1);
		return true;
	}
	else return false;
};
bool ReadBMP8TOBPX(char* Name, byte** data) {
	BMPformat BM;
	ResFile f1 = RReset(Name);
	if (f1 != INVALID_HANDLE_VALUE) {
		RBlockRead(f1, &BM, sizeof BMPformat);
		if (IOresult() || BM.bfType != 'MB')return false;
		if (BM.biBitCount != 8)return false;
		*data = new byte[BM.biWidth*BM.biHeight + 4];
		int wid = BM.biWidth;
		int rwid = wid;
		if (wid & 3)rwid = (wid | 3) + 1;
		((short*)*data)[0] = BM.biWidth;
		((short*)*data)[1] = BM.biHeight;
		for (int i = 0; i < BM.biHeight; i++) {
			RSeek(f1,/*(sizeof BMPformat)+1024*/BM.bfOffBits + (BM.biHeight - i - 1)*rwid);
			RBlockRead(f1, &((*data)[i*wid + 4]), wid);
		};
		RClose(f1);
		return true;
	}
	else return false;
};
bool LoadBitmapLikeGrayscale(char* Name, int* Lx, int* Ly, byte** res) {
	byte* data;
	BMPformat BM;
	if (ReadBMP24(Name, &BM, &data)) {
		int LX = BM.biWidth;
		int LY = BM.biHeight;
		*res = new byte[LX*LY];
		for (int ix = 0; ix < LX; ix++)
			for (int iy = 0; iy < LY; iy++) {
				int ofs = (ix + iy*LX);
				int ofs3 = ofs * 3;
				int v = int(data[ofs3] + data[ofs3 + 1] + data[ofs3 + 2]) / 3;
				(*res)[ofs] = v;
			};

		free(data);
		*Lx = BM.biWidth;
		*Ly = BM.biHeight;
		return true;
	};
	return false;
};
int GetResVal(byte* res, int LX, int LY, int RLX, int RLY, int x, int y) {
	int vx = (x*LX) / RLX;
	int vy = (y*LY) / RLY;
	int rx = (vx*RLX) / LX;
	int ry = (vy*RLX) / LY;
	int rx1 = ((vx + 1)*RLX) / LX;
	int ry1 = ((vy + 1)*RLY) / LY;
	int vx1 = vx + 1;
	int vy1 = vy + 1;
	if (vx < 0)vx = 0;
	if (vy < 0)vy = 0;
	if (vx1 < 0)vx1 = 0;
	if (vy1 < 0)vy1 = 0;
	if (vx >= LX)vx = LX - 1;
	if (vy >= LY)vy = LY - 1;
	if (vx1 >= LX)vx1 = LX - 1;
	if (vy1 >= LY)vy1 = LY - 1;
	int z1 = res[vx + LX*vy];
	int z2 = res[vx1 + LX*vy];
	int z3 = res[vx + LX*vy1];
	int z4 = res[vx1 + LX*vy1];
	return z1 + ((x - rx)*(z2 - z1)) / (rx1 - rx) + ((y - ry)*(z3 - z1)) / (ry1 - ry) - (((z2 + z3 - z1 - z4)*(x - rx)*(y - ry)) / (rx1 - rx) / (ry1 - ry));
};
byte* DATA;
DWORD GetSumm(char* Name) {
	ResFile F = RReset(Name);
	if (F != INVALID_HANDLE_VALUE) {
		int sz = RFileSize(F);

		RBlockRead(F, DATA, sz);
		RClose(F);
		DWORD SZZ = 0;
		for (int i = 0; i < sz; i++)SZZ += DATA[i];
		SZZ &= 65535;
		return SZZ;
	}
	else return 0;
}

extern HWND hwnd;

#undef free
#undef malloc
int TotalSize = 0;

__declspec(dllexport) void _ExFree(void* ptr) 
{
	DWORD* Ptr = (DWORD*)ptr;
	try 
	{
		if (Ptr&&Ptr[0] != 0xcdcdcdcd) 
		{
			free(ptr);
			TotalSize--;
		}
	}
	catch (...) {}
}

__declspec(dllexport) void* _ExMalloc(int Size) 
{
	TotalSize++;
	return calloc(Size, 1);
}
