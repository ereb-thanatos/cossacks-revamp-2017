#include <mmsystem.h>
typedef void VOIDFN();
class CDeviceCD
{
// Construction
public:
	CDeviceCD();

// Attributes
public:

// Operations
public:

// Implementation
public:
	bool Play(DWORD Track);
	bool SetVolume(DWORD Volume);
	DWORD GetVolume();
	bool Stop();
	bool Resume();
	bool Pause();
	bool Close();
	bool Open();
	virtual ~CDeviceCD();

	// Generated message map functions
protected:
	bool FOpened;
	MCIDEVICEID FDeviceID;
	MCIERROR FError;
	LRESULT MCINotify(WPARAM wFlags, LONG lDevId);
};
