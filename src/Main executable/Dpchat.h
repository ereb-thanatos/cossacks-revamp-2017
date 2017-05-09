/*==========================================================================
 *
 *  Copyright (C) 1996-1997 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dpchat.h
 *  Content:	Header file for dpchat.cpp
 *
 ***************************************************************************/

#define IDIRECTPLAY2_OR_GREATER
#include "dplay.h"

// constants
const DWORD MAXPLAYERS = 7;			// max no. players in the session

// structure used to store DirectPlay information
typedef struct {
	LPDIRECTPLAY3A	lpDirectPlay3A;		// IDirectPlay3A interface pointer
	HANDLE			hPlayerEvent;		// player event to use
	DPID			dpidPlayer;			// ID of player created
	BOOL			bIsHost;			// TRUE if we are hosting the session
} DPLAYINFO, *LPDPLAYINFO;

// guid for this application
// {5BFDB060-06A4-11d0-9C4F-00A0C705475f}
DEFINE_GUID(DPCHAT_GUID, 
0x5bfdb060, 0x6a4, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc7, 0x5, 0x47, 0x5f);

// prototypes
//extern HRESULT	ConnectUsingLobby(LPDPLAYINFO lpDPInfo);
//extern HRESULT	ConnectUsingDialog(HINSTANCE hInstance, LPDPLAYINFO lpDPInfo);
//extern void		ErrorBox(LPSTR lpszErrorStr, HRESULT hr);

