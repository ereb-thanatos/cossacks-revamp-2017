#if !defined(_MASKS_H_INCLUDED_)
#define _MASKS_H_INCLUDED_

BOOL	isiMatchesMask(LPSTR lpszFile, LPSTR lpszMask);

DWORD	isiCalcHash(LPSTR lpszFileName);

void	isiDecryptMem(LPBYTE lpbBuffer, DWORD dwSize, BYTE dbKey);

void	isiEncryptMem(LPBYTE lpbBuffer, DWORD dwSize, BYTE dbKey);

BOOL	isiFileExists(LPSTR lpszFileName);

#endif //_MASKS_H_INCLUDED_