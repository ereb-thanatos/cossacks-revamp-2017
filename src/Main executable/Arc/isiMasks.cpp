#pragma warning (disable : 4035)

#include <windows.h>

void isiDecryptMem(LPBYTE lpbBuffer, DWORD dwSize, BYTE dbKey)
{
_asm
	{
		mov	ecx,dwSize
		mov	ebx,lpbBuffer
		mov	ah,dbKey

next_byte:

		mov	al,[ebx]
		not	al
		xor	al,ah
		mov	[ebx],al
		inc	ebx

		loop next_byte
	}
}

void isiEncryptMem(LPBYTE lpbBuffer, DWORD dwSize, BYTE dbKey)
{
_asm
	{
		mov	ecx,dwSize
		mov	ebx,lpbBuffer
		mov	ah,dbKey
		not	ah

next_byte:

		mov	al,[ebx]
		xor	al,ah
		not	al
		mov	[ebx],al
		inc	ebx

		loop next_byte
	}
}

DWORD isiCalcHash(LPSTR lpszFileName)
{
	char	szFileName[64];

	ZeroMemory(szFileName,64);
	strcpy(szFileName,_strupr(lpszFileName));

_asm
	{
		mov	edx,0
		mov	ecx,16
		lea ebx,szFileName

new_dword:

		mov	eax,[ebx]
		xchg ah,al
		rol	eax,16
		xchg ah,al

		add	edx,eax

		add	ebx,4

		loop new_dword

		mov	eax,edx
	}
}


BOOL isiMatchesMask(LPSTR lpszFile, LPSTR lpszMask)
{
	char	szFile[255];
	char	szMask[255];

	strcpy(szFile,_strupr(lpszFile));
	strcpy(szMask,_strupr(lpszMask));

_asm
	{
		lea esi,szMask
		lea edi,szFile

	next_char:

		mov ah,byte ptr [esi]	// Mask
		mov al,byte ptr [edi]	// File

		cmp ax,0
		jnz short cont_compare
		mov eax,1
		jmp short exit_comp

	cont_compare:

		cmp ah,'?'				// Mask
		jz	skip_one_char

		cmp ah,al				// Mask & File
		jz	skip_one_char

		cmp ah,'*'				// Mask
		jz	skip_multiple

		mov eax,0
		jmp short exit_comp

	skip_multiple:

		inc esi

		mov ah,byte ptr [esi]	// Mask

	next:

		mov al,byte ptr [edi]	// File

		cmp al,ah
		jz	next_char
		inc edi
		jmp short next

	skip_one_char:

		inc esi
		inc edi
		jmp short next_char

	exit_comp:

	}
}

BOOL isiFileExists(LPSTR lpszFileName)
{
	WIN32_FIND_DATA	FindData;
	HANDLE			hFindFile;
	
	if((hFindFile=FindFirstFile(lpszFileName,&FindData))!=INVALID_HANDLE_VALUE)
		{
			FindClose(hFindFile);
			return TRUE;
		}
	else
		return FALSE;
}

#pragma warning (default : 4035)