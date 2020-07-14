#ifndef _DM_FILE_H__
#define _DM_FILE_H__

#include "stdio.h"
#include "windows.h"


namespace DMFile
{

extern HANDLE f;
extern int bAppended;//0-not file opened,1-single,no size,2-single with size,3-multipart,const number,4-multipart dynamic number;
extern wchar_t *tempName;
extern wchar_t crFilePthAndName[512];
extern wchar_t foundedFileName[512];


extern bool Open();
extern void Close(bool rFinishHttp=false);
extern bool CheckDMsForExisting(wchar_t*,wchar_t*);
extern bool OpenForAppending(wchar_t*);


//extern bool CheckForDMFileSingleWithSize(HANDLE);
extern bool CheckForDMFileMultipartContext(HANDLE,wchar_t*);
extern bool ChangeFileName(char*);


}

#endif