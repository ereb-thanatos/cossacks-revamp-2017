/*             Work with the resource files
 *   
 *  You must use this module for  accesss to files.This 
 * routine allows you to read  files from disk  or from 
 * the resource file, you even will not recognise where
 * the given file is.
 */
//#include "Arc\GSCSet.h"
//typedef LPGSCfile ResFile;
typedef HANDLE ResFile;
//Opening the resource file
ResFile RReset(LPCSTR lpFileName);
//Rewriting file
ResFile RRewrite(LPCSTR lpFileName);
//Getting size of the resource file
DWORD RFileSize(ResFile hFile);
// Setting file position 
DWORD RSeek(ResFile hFile,int pos);
//Reading the file
DWORD RBlockRead(ResFile hFile,LPVOID lpBuffer,DWORD BytesToRead);
//Writing the file
DWORD RBlockWrite(ResFile hFile,LPVOID lpBuffer,DWORD BytesToWrite);
//Returns last error
DWORD IOresult(void);
//Close the file
void RClose(ResFile hFile);