//#include "stdafx.h"
#include "windows.h"
#include "TMixer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CMixer::ZeroAll()
{
	m_HMixer = NULL;
	m_iMixerControlID = 0;
	mmr = MMSYSERR_NOERROR;
	m_dwChannels = 0;
	m_bSuccess = FALSE;
}

CMixer::CMixer(DWORD DstType, DWORD SrcType, DWORD ControlType)
{
	ZeroAll();
	if(mixerGetNumDevs() < 1) return;
	mmr = mixerOpen(&m_HMixer, 0, 0, 0L, CALLBACK_NULL);
	if (mmr != MMSYSERR_NOERROR) return;
// get dwLineID
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
// DstType
	mxl.dwComponentType = DstType;
	if (mixerGetLineInfo((HMIXEROBJ)m_HMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)
	!= MMSYSERR_NOERROR) return;
// SrcType
	if( SrcType != NO_SOURCE )
	{
		UINT nconn = mxl.cConnections;
		DWORD DstIndex = mxl.dwDestination;
		for( UINT j = 0; j < nconn; j++ )
		{
			mxl.cbStruct = sizeof( MIXERLINE );
			mxl.dwSource = j;
			mxl.dwDestination = DstIndex;
			if(mixerGetLineInfo( ( HMIXEROBJ )m_HMixer,
			&mxl, MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR) return;
			if( mxl.dwComponentType == SrcType ) break;
		}
	}
// get dwControlID
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = ControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if (mixerGetLineControls((HMIXEROBJ)m_HMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return;
	m_iMixerControlID = mxc.dwControlID;
	m_dwChannels = mxl.cChannels;
	if (m_dwChannels > 0) m_dwChannels--;
	m_bSuccess = TRUE;
}

CMixer::CMixer(HWND hwnd, DWORD DstType, DWORD SrcType, DWORD ControlType)
{
	ZeroAll();
	if(mixerGetNumDevs() < 1) return;
	mmr = mixerOpen(&m_HMixer, 0, (DWORD)hwnd, 0L, CALLBACK_WINDOW);
	if (mmr != MMSYSERR_NOERROR) return;
// get dwLineID
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
// DstType
	mxl.dwComponentType = DstType;
	if (mixerGetLineInfo((HMIXEROBJ)m_HMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)
	!= MMSYSERR_NOERROR) return;
// SrcType
	if( SrcType != NO_SOURCE )
	{
		UINT nconn = mxl.cConnections;
		DWORD DstIndex = mxl.dwDestination;
		for( UINT j = 0; j < nconn; j++ )
		{
			mxl.cbStruct = sizeof( MIXERLINE );
			mxl.dwSource = j;
			mxl.dwDestination = DstIndex;
			if(mixerGetLineInfo( ( HMIXEROBJ )m_HMixer,
			&mxl, MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR) return;
			if( mxl.dwComponentType == SrcType ) break;
		}
	}
// get dwControlID
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = ControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if (mixerGetLineControls((HMIXEROBJ)m_HMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return;
	m_iMixerControlID = mxc.dwControlID;
	m_dwChannels = mxl.cChannels;
	if (m_dwChannels > 0) m_dwChannels--;
	m_bSuccess = TRUE;
}

CMixer::~CMixer()
{
	if (m_HMixer) mixerClose(m_HMixer);
}

////////////////////////////////////////

DWORD CMixer::GetControlValue()
{
	if (!m_bSuccess) return 0;
	m_bSuccess = FALSE;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_iMixerControlID;
	mxcd.cChannels = m_dwChannels;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(mxcd_u);
	mxcd.paDetails = &mxcd_u;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return 0;
	m_bSuccess = TRUE;
	return mxcd_u.dwValue;
}

BOOL CMixer::SetControlValue(DWORD dw)
{
	if (!m_bSuccess) return m_bSuccess;
	m_bSuccess = FALSE;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_iMixerControlID;
	mxcd.cChannels = m_dwChannels;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(mxcd_u);
	mxcd.paDetails = &mxcd_u;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return m_bSuccess;
	mxcd_u.dwValue  = dw;
	mmr = mixerSetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return m_bSuccess;
	m_bSuccess = TRUE;
	return m_bSuccess;
}

BOOL CMixer::On()
{
	return SetControlValue(0);
}

BOOL CMixer::Off()
{
	return SetControlValue(1);
}

