#include <math.h>
#include <stdlib.h>

#define MLX 1024
#define MMSK 1023
#define MSHF 10

short fMap[MLX*MLX];
int mrand();

int GetRand(int amp) 
{
	return ((mrand()*amp) >> 14) - amp;
}

int GetfMap(int x, int y) 
{
	x &= MMSK;
	y &= MMSK;
	return fMap[x + (y << MSHF)];
}

void SetfMap(int x, int y, int val) 
{
	if (x < 0 || y < 0 || x >= MLX || y >= MLX)
	{
		return;
	}

	fMap[x + (y << MSHF)] = short(val);
}

void Generate(int N) 
{
	int LX = MLX >> N;
	short A0 = 2048;
	int x, y;

	for (x = 0; x < MLX; x += LX) 
	{
		for (y = 0; y < MLX; y += LX) 
		{
			SetfMap(x, y, GetRand(A0));
		}
	}

	while (LX > 1) 
	{
		A0 >>= 1;
		for (x = 0; x < MLX; x += LX) 
		{
			for (y = 0; y < MLX; y += LX) 
			{
				int L1 = LX >> 1;
				SetfMap(x + L1, y, GetRand(A0) + ((GetfMap(x, y) + GetfMap(x + LX, y)) >> 1));
				SetfMap(x + L1, y + LX, GetRand(A0) + ((GetfMap(x, y + LX) + GetfMap(x + LX, y + LX)) >> 1));
				SetfMap(x, y + L1, GetRand(A0) + ((GetfMap(x, y) + GetfMap(x, y + LX)) >> 1));
				SetfMap(x + LX, y + L1, GetRand(A0) + ((GetfMap(x + LX, y) + GetfMap(x + LX, y + LX)) >> 1));
				SetfMap(x + L1, y + L1, GetRand(A0) + ((GetfMap(x + L1, y) + GetfMap(x + L1, y + LX) + GetfMap(x, y + L1) + GetfMap(x + LX, y + L1)) >> 2));
			}
		}
		LX >>= 1;
	}

	int fmin = 100000;
	int fmax = -100000;

	for (x = 0; x < MLX; x += LX) 
	{
		for (y = 0; y < MLX; y += LX) 
		{
			int f = GetfMap(x, y);
			if (f < fmin)fmin = f;
			if (f > fmax)fmax = f;
		}
	}

	int dx = fmax - fmin;

	for (x = 0; x < MLX; x += LX) 
	{
		for (y = 0; y < MLX; y += LX) 
		{
			int f = (int(GetfMap(x, y) - fmin) << 9) / dx;
			SetfMap(x, y, (int(GetfMap(x, y) - fmin) << 9) / dx);
		}
	}

	for (x = 0; x < MLX; x += LX) 
	{
		for (y = 0; y < MLX; y += LX) 
		{
			SetfMap(x, y, (GetfMap(x + 1, y) + GetfMap(x - 1, y) + GetfMap(x, y + 1) + GetfMap(x, y - 1) + GetfMap(x, y)) / 5);
		}
	}
}

#define scale 32

int GetFractalVal(int x, int y) 
{
	int x0 = (x / scale)&MMSK;
	int y0 = (y / scale)&MMSK;
	int x1 = (x0++)&MMSK;
	int y1 = (y0++)&MMSK;
	int dx = x%scale;
	int dy = y%scale;
	int v0 = GetfMap(x0, y0);
	int v1 = GetfMap(x1, y0);
	int v2 = GetfMap(x0, y1);
	int v3 = GetfMap(x1, y1);
	return v0 + (v1 - v0)*dx / scale + (v2 - v0)*dy / scale + ((v3 + v0 - v1 - v2)*dx / scale)*dy / scale;
}

void CreateFractal() 
{
	Generate(6);
}