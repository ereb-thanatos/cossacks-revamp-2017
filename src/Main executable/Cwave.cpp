///////////////////////////////////////////////////////////
// CWAVE.CPP: Implementation file for the WAVE class.
///////////////////////////////////////////////////////////

//#include <stdafx.h>
//#include <afx.h>
#include <windows.h>
#include "windowsx.h"
//#include "stdafx.h"
#include "cwave.h"

///////////////////////////////////////////////////////////
// CWave::CWave()
///////////////////////////////////////////////////////////
CWave::CWave(char* fileName)
{
    // Initialize the class's members.
    m_waveSize = 0;
    m_waveOK = FALSE;
    m_pWave= NULL;

    // Load the wave file.
    m_waveOK = LoadWaveFile(fileName);
}

///////////////////////////////////////////////////////////
// CWave::~CWave()
///////////////////////////////////////////////////////////
CWave::~CWave()
{
    // Free the memory assigned to the wave data.
    GlobalFreePtr(m_pWave);
}

///////////////////////////////////////////////////////////
// CWave::LoadWaveFile()
//
// This function loads a wave from disk into memory. It
// also initializes various class data members.
///////////////////////////////////////////////////////////
BOOL CWave::LoadWaveFile(char* fileName)
{
    MMCKINFO mmCkInfoRIFF;
    MMCKINFO mmCkInfoChunk;
    MMRESULT result;
    HMMIO hMMIO;
    long bytesRead;

    // Open the wave file.
    hMMIO = mmioOpen(fileName, NULL, MMIO_READ | MMIO_ALLOCBUF);
    if (hMMIO == NULL)
        return FALSE;

    // Descend into the RIFF chunk.
    mmCkInfoRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    result = mmioDescend(hMMIO, &mmCkInfoRIFF, NULL, MMIO_FINDRIFF);
    if (result != MMSYSERR_NOERROR)
        return FALSE;

    // Descend into the format chunk.
    mmCkInfoChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    result = mmioDescend(hMMIO, &mmCkInfoChunk,
        &mmCkInfoRIFF, MMIO_FINDCHUNK);
    if (result != MMSYSERR_NOERROR)
        return FALSE;

    // Read the format information into the WAVEFORMATEX structure.
    bytesRead = mmioRead(hMMIO, (char*)&m_waveFormatEx,
        sizeof(WAVEFORMATEX));
    if (bytesRead == -1)
        return FALSE;

    // Ascend out of the format chunk.
    result = mmioAscend(hMMIO, &mmCkInfoChunk, 0);
    if (result != MMSYSERR_NOERROR)
        return FALSE;

    // Descend into the data chunk.
    mmCkInfoChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    result = mmioDescend(hMMIO, &mmCkInfoChunk,
        &mmCkInfoRIFF, MMIO_FINDCHUNK);
    if (result != MMSYSERR_NOERROR)
        return FALSE;

    // Save the size of the wave data.
    m_waveSize = mmCkInfoChunk.cksize;

    // Allocate a buffer for the wave data.
    m_pWave = (char*)GlobalAllocPtr(GMEM_MOVEABLE, m_waveSize);
    if (m_pWave == NULL)
        return FALSE;

    // Read the wave data into the buffer.
    bytesRead = mmioRead(hMMIO, (char*)m_pWave, m_waveSize);
    if (bytesRead == -1)
        return FALSE;
    mmioClose(hMMIO, 0);

    return TRUE;
}

///////////////////////////////////////////////////////////
// CWave::GetWaveSize()
//
// This returns the size in bytes of the wave data.
///////////////////////////////////////////////////////////
DWORD CWave::GetWaveSize()
{
    return m_waveSize;
}

///////////////////////////////////////////////////////////
// CWave::GetWaveFormatPtr()
//
// This function returns a pointer to the wave file's
// WAVEFORMATEX structure.
///////////////////////////////////////////////////////////
LPWAVEFORMATEX CWave::GetWaveFormatPtr()
{
    return &m_waveFormatEx;
}

///////////////////////////////////////////////////////////
// CWave::GetWaveDataPtr()
//
// This function returns a pointer to the wave's
// actual sound data.
///////////////////////////////////////////////////////////
char* CWave::GetWaveDataPtr()
{
    return m_pWave;
}

///////////////////////////////////////////////////////////
// CWave::WaveOK()
//
// This function returns a Boolean value indicating whether
// the wave file object was set up successfully.
///////////////////////////////////////////////////////////
BOOL CWave::WaveOK()
{
    return m_waveOK;
}
