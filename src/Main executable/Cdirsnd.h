///////////////////////////////////////////////////////////
// CDIRSND.H -- Header file for the CDirSound class.
///////////////////////////////////////////////////////////

#ifndef __CDIRSND_H
#define __CDIRSND_H

//#include <afxwin.h>
#include <windows.h>
#include "cwave.h"
#include "dsound.h"
#include <mmsystem.h>

#define MAXSND 600
#define MAXSND1 601
class CDirSound
{
protected:
    LPDIRECTSOUND m_pDirectSoundObj;
    HWND m_hWindow;
    LPDIRECTSOUNDBUFFER m_bufferPointers[MAXSND1];
    DWORD m_bufferSizes[MAXSND1];

public:
	short Volume[MAXSND1];
	short SrcX[MAXSND1];
	short SrcY[MAXSND1];
	byte  BufIsRun[MAXSND1];
	UINT m_currentBufferNum;
	void CDirSound::CreateDirSound(HWND hWnd);
    ~CDirSound();
    UINT CreateSoundBuffer(CWave* pWave);
	UINT DuplicateSoundBuffer(UINT bufferNum);
    BOOL DirectSoundOK();
	void SetLastVolume(short Vol){
		Volume[m_currentBufferNum]=Vol;
	};
    BOOL CopyWaveToBuffer(CWave* pWave, UINT bufferNum);
	void SetVolume(UINT bufferNum,int vol);
	void SetPan(UINT bufferNum,int pan);
    BOOL PlaySound(UINT bufferNum);
    BOOL StopSound(UINT bufferNum);
	BOOL PlayCoorSound(UINT bufferNum,int x,int vx);
	void ControlPan(UINT bufferNum);
	bool IsPlaying(UINT bufferNum);
	int GetPos(UINT bufferNum);
	void ProcessSoundSystem();
protected:
    void ReleaseAll();
};

#endif

#define MaxSnd 1024