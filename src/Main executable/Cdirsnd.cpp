///////////////////////////////////////////////////////////
// CDIRSND.CPP -- Implementation file for the CDirSound
//                class.
///////////////////////////////////////////////////////////

//#include <stdafx.h>
#include "cdirsnd.h"
///////////////////////////////////////////////////////////
// CDirSound::CDirSound()
//
// This is the class's constructor.
///////////////////////////////////////////////////////////
void CDirSound::CreateDirSound(HWND hWnd)
{
	// Initialize class data members.
	m_hWindow = hWnd;
	m_pDirectSoundObj = NULL;
	m_currentBufferNum = 0;

	for (UINT x = 0; x < MAXSND1; ++x)
	{
		m_bufferPointers[x] = NULL;
		m_bufferSizes[x] = 0;
	}

	// Create the main DirectSound object.
	HRESULT result =
		DirectSoundCreate(NULL, &m_pDirectSoundObj, NULL);
	if (result == DS_OK)
	{
		// Set the priority level.
		result = m_pDirectSoundObj->
			SetCooperativeLevel(m_hWindow, DSSCL_EXCLUSIVE);//NORMAL);
		if (result != DS_OK)
			m_pDirectSoundObj = NULL;
		// Set up DSBUFFERDESC structure. 

		DSBUFFERDESC dsbdesc;
		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out. 
		dsbdesc.dwSize = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
		// Buffer size is determined by sound hardware. 
		dsbdesc.dwBufferBytes = 0;
		dsbdesc.lpwfxFormat = NULL; // Must be NULL for primary buffers. 
		LPDIRECTSOUNDBUFFER lpDsb = NULL;
		HRESULT hr = m_pDirectSoundObj->CreateSoundBuffer(&dsbdesc, &lpDsb, NULL);
		/*
		int v;
		switch(hr){
		case DSERR_ALLOCATED:
			v=0;
			break;
		case DSERR_BADFORMAT:
			v=1;
			break;
		case DSERR_INVALIDPARAM:
			v=2;
			break;
		case DSERR_NOAGGREGATION:
			v=3;
			break;
		case DSERR_OUTOFMEMORY:
			v=4;
			break;
		case DSERR_UNINITIALIZED:
			v=5;
			break;
		case DSERR_UNSUPPORTED:
			v=6;
			break;
		case DS_OK:
			v=7;
			break;
		};
		*/
		WAVEFORMATEX FR;
		DWORD rd;
		hr = lpDsb->GetFormat(&FR, sizeof FR, &rd);
		FR.wBitsPerSample = 16;
		FR.nAvgBytesPerSec = 88200;
		FR.nBlockAlign = 4;
		FR.nChannels = 2;
		FR.nSamplesPerSec = 22050;
		hr = lpDsb->SetFormat(&FR);
		memset(BufIsRun, 0, sizeof BufIsRun);

	}

}

///////////////////////////////////////////////////////////
// CDirSound::~CDirSound()
//
// This is the class's destructor.
///////////////////////////////////////////////////////////
CDirSound::~CDirSound()
{
	if (m_pDirectSoundObj != NULL)
		ReleaseAll();
}

///////////////////////////////////////////////////////////
// CDirSound::ReleaseAll()
//
// This member function releases all the DirectSound
// objects created in the class.
///////////////////////////////////////////////////////////
void CDirSound::ReleaseAll()
{
	// Release all sound buffers.
	for (UINT x = 1; x <= m_currentBufferNum; ++x)
		m_bufferPointers[x]->Release();

	// Release the DirectSound object.
	if (m_pDirectSoundObj != NULL)
		m_pDirectSoundObj->Release();
	memset(BufIsRun, 0, sizeof BufIsRun);
}

///////////////////////////////////////////////////////////
// CDirSound::CreateSoundBuffer()
//
// This member function creates secondary sound buffers.
// The class can accommodate up to MAXSND such buffers.
///////////////////////////////////////////////////////////
UINT CDirSound::CreateSoundBuffer(CWave* pWave)
{
	DSBUFFERDESC dsBufferDesc;
	HRESULT hResult;

	// Make sure there's room for another buffer.
	if (m_currentBufferNum == MAXSND)
		return 0;

	// Calculate the next available buffer number.
	++m_currentBufferNum;

	// Initialize the DSBUFFERDESC structure.
	LPWAVEFORMATEX pWaveFormat = pWave->GetWaveFormatPtr();
	memset(&dsBufferDesc, 0, sizeof(DSBUFFERDESC));
	dsBufferDesc.dwSize = sizeof(DSBUFFERDESC);
	dsBufferDesc.dwFlags = DSBCAPS_CTRLDEFAULT;
	dsBufferDesc.dwBufferBytes = pWave->GetWaveSize();
	dsBufferDesc.lpwfxFormat = (LPWAVEFORMATEX)pWaveFormat;

	// Create the secondary sound buffer.
	hResult = m_pDirectSoundObj->CreateSoundBuffer(&dsBufferDesc,
		&m_bufferPointers[m_currentBufferNum], NULL);
	if (hResult != DS_OK)
		return 0;

	// Save the buffer size.
	m_bufferSizes[m_currentBufferNum] = dsBufferDesc.dwBufferBytes;

	return m_currentBufferNum;
}
///////////////////////////////////////////////////////////
// CDirSound::DuplicateSoundBuffer()
//
// This member function creates secondary sound buffers.
// The class can accommodate up to MAXSND such buffers.
///////////////////////////////////////////////////////////
UINT CDirSound::DuplicateSoundBuffer(UINT bufferNum)
{
	HRESULT hResult;

	// Make sure there's room for another buffer.
	if (m_currentBufferNum == MAXSND)
		return 0;

	// Calculate the next available buffer number.
	++m_currentBufferNum;
	hResult = m_pDirectSoundObj->DuplicateSoundBuffer(m_bufferPointers[bufferNum], &m_bufferPointers[m_currentBufferNum]);
	if (hResult != DS_OK)
		return 0;
	m_bufferSizes[m_currentBufferNum] = m_bufferSizes[bufferNum];

	return m_currentBufferNum;
}
///////////////////////////////////////////////////////////
// CDirSound::CopyWaveToBuffer()
//
// This function copies wave data to a DirectSound buffer.
///////////////////////////////////////////////////////////
BOOL CDirSound::CopyWaveToBuffer(CWave* pWave, UINT bufferNum)
{
	LPVOID pSoundBlock1;
	LPVOID pSoundBlock2;
	DWORD bytesSoundBlock1;
	DWORD bytesSoundBlock2;
	HRESULT result;

	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return FALSE;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer =
		m_bufferPointers[bufferNum];

	// Lock the buffer.
	result = pSoundBuffer->Lock(0, m_bufferSizes[bufferNum],
		&pSoundBlock1, &bytesSoundBlock1,
		&pSoundBlock2, &bytesSoundBlock2, 0);
	if (result != DS_OK)
		return FALSE;

	// Copy the data into the buffer.
	char* pWaveData = pWave->GetWaveDataPtr();
	DWORD waveSize = pWave->GetWaveSize();
	memcpy((void*)pSoundBlock1, pWaveData, waveSize);

	// Unlock the buffer.
	pSoundBuffer->Unlock(pSoundBlock1, bytesSoundBlock1,
		pSoundBlock2, bytesSoundBlock2);

	return TRUE;
}

///////////////////////////////////////////////////////////
// CDirSound::DirectSoundOK()
//
// This member function returns TRUE if the DirectSound
// object was created and initialized okay. Otherwise, it
// returns FALSE.
///////////////////////////////////////////////////////////
BOOL CDirSound::DirectSoundOK()
{
	if (m_pDirectSoundObj == NULL)
		return FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////
// CDirSound::PlaySound()
//
// This member function plays the sound stored in the given
// buffer.
///////////////////////////////////////////////////////////
BOOL CDirSound::PlaySound(UINT bufferNum)
{
	HRESULT result;

	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return FALSE;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	result = pSoundBuffer->SetCurrentPosition(0);
	if (result != DS_OK)
		return FALSE;

	// Play the sound.
	result = pSoundBuffer->Play(0, 0, 0);
	if (result != DS_OK)
		return FALSE;
	BufIsRun[bufferNum] = 0;
	return TRUE;
}
BOOL CDirSound::PlayCoorSound(UINT bufferNum, int x, int vx)
{
	HRESULT result;

	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return FALSE;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	result = pSoundBuffer->SetCurrentPosition(0);
	if (result != DS_OK)
		return FALSE;

	// Play the sound.
	result = pSoundBuffer->Play(0, 0, 0);
	if (result != DS_OK)
		return FALSE;
	BufIsRun[bufferNum] = 1;
	SrcX[bufferNum] = x;
	SrcY[bufferNum] = vx;
	return TRUE;
}

extern int CenterX;

void CDirSound::ControlPan(UINT bufferNum) 
{
	if (BufIsRun[bufferNum]) 
	{
		SrcX[bufferNum] += SrcY[bufferNum];
		int pan = (SrcX[bufferNum] - CenterX) << 1;

		if (pan < -4000)
		{
			pan = -4000;
		}

		if (pan > 4000)
		{
			pan = 4000;
		}

		SetPan(bufferNum, pan);

		if (rand() < 350)
		{
			IsPlaying(bufferNum);
		}
	}
}

void CDirSound::SetVolume(UINT bufferNum, int vol) {
	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	pSoundBuffer->SetVolume(vol);
};
void CDirSound::SetPan(UINT bufferNum, int pan) {
	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	pSoundBuffer->SetPan(pan);
}

void CDirSound::ProcessSoundSystem()
{
	for (int i = 0; i < MAXSND1; i++)
	{
		if (BufIsRun[i])
		{
			ControlPan(i);
		}
	}
}

///////////////////////////////////////////////////////////
// CDirSound::StopSound()
//
// This member function stops the sound stored in the given
// buffer.
///////////////////////////////////////////////////////////
BOOL CDirSound::StopSound(UINT bufferNum)
{
	HRESULT result;

	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return FALSE;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	result = pSoundBuffer->Stop();
	if (result != DS_OK)
		return FALSE;

	return TRUE;
}
int CDirSound::GetPos(UINT bufferNum)
{
	HRESULT result;

	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
		return 0;

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	DWORD pos;
	result = pSoundBuffer->GetCurrentPosition(&pos, NULL);
	return pos;

}

bool CDirSound::IsPlaying(UINT bufferNum)
{
	HRESULT result;

	// Check for a valid buffer number.
	if ((bufferNum > m_currentBufferNum) || (bufferNum == 0))
	{
		return 0;
	}

	// Get a pointer to the requested buffer.
	LPDIRECTSOUNDBUFFER pSoundBuffer = m_bufferPointers[bufferNum];

	// Make sure the buffer is set to play from the beginning.
	DWORD pos;
	result = pSoundBuffer->GetStatus(&pos);
	bool play = pos & DSBSTATUS_PLAYING;

	if (!play)
	{
		BufIsRun[bufferNum] = 0;
	}

	return play;
}

