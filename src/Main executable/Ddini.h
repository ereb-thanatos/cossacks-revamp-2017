/***********************************************************************
 *Direct Draw initialisation module
 *
 * This module creates the Direct Draw object with the primary surface
 * and a backbuffer and sets 800x600x8 display mode.
 *
 ***********************************************************************/
#ifndef __DDINI_H_
#define __DDINI_H_

#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdlib.h>
#include <stdarg.h>
#include "resource.h"
#include "lines.h"

#define free _ExFree
#define malloc _ExMalloc
void* _ExMalloc(int Size);
void _ExFree(void* Ptr);

inline void* _cdecl operator new(size_t size)
{
	return malloc(size);
}

inline void __cdecl operator delete(void *ptr)
{
	free(ptr);
}

#define MAKE_PTC

//#ifndef __ddini_cpp_
extern LPDIRECTDRAW            lpDD;			   // DirectDraw object
extern LPDIRECTDRAWSURFACE     lpDDSPrimary;   // DirectDraw primary surface
extern LPDIRECTDRAWSURFACE     lpDDSBack;      // DirectDraw back surface
extern BOOL                    bActive;        // is application active?
extern BOOL                    CurrentSurface; //=FALSE if backbuffer
											   // is active (Primary surface is visible)
											   //=TRUE if  primary surface is active
											   // (but backbuffer is visible)
extern BOOL                    DDError;        //=FALSE if Direct Draw works normally 
extern DDSURFACEDESC   ddsd;
extern HWND hwnd;
extern bool window_mode;
/*  Create Direct Draw object
 *
 * This procedure creadtes Direct Draw object with primary surface and
 * one backbuffer and sets 800x600x8 display mode.
 * This procedure enables usage of all other procedure in thes module.
 */
bool CreateDDObjects(HWND hwnd);
/*     Closing all Direct Draw objects
 *
 * This procedure must be called before the program terminates,
 * otherwise Windows can occur some problems.
 */
void FreeDDObjects(void);
/*
 * Flipping Pages
 */
__declspec(dllexport)
void FlipPages(void);
/*
 * Getting Screen Pointer
 *
 * You will get the pointer to the invisible area of the screen
 * i.e, if primary surface is visible, then you will obtain the
 * pointer to the backbuffer.
 * You must call UnlockSurface() to allow Windows draw on the screen
 */
void LockSurface(void);
/*
 *  Unlocking the surface
 *
 *  You must unlock the Video memory for Windows to work properly
 */
void UnlockSurface(void);
/*
 * Getting the DC of the active (invisible) area of the screen
 */
 //HDC GetSDC(void);//OBSOLETE
 //Sttting the palette
void LoadPalette(LPCSTR lpFileName);
void SetDebugMode();
void NoDebugMode();

// Define own DirectDrawCreate helper function which loads mdraw.dll at runtime
HRESULT DirectDrawCreate_wrapper(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);

//#endif
#endif //__DDINI_H_
