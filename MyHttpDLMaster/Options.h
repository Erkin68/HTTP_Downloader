#ifndef _Config_H__
#define _Config_H__

typedef signed __int64 S64;

namespace config
{
extern char userAgentStr[128];
extern char protocolStr[32];
extern wchar_t outDir[MAX_PATH*2];
extern int	iMaxMultipart,iMinMultipart,iRecBufSize,
			iMaxConnectTimeout,iMaxRecvTimeout,iMaxSendTimeout,iMinSizeForDividing;
extern bool GetFromDlg(HWND);
extern bool ReadConfig();
extern bool Save();
extern void SetDefault();
extern bool SetToDlg(HWND);
extern bool SaveDnldFileInfo(int);
extern bool ChangeTree(bool,int);
extern char GetMyFileType(wchar_t*);
}


extern bool InsertNewUrl(wchar_t*,wchar_t* path=NULL,int pc=0,S64* pSz=NULL,S64* pSzDld=NULL);
extern BOOL MyU64To(wchar_t*,int,unsigned __int64,int*);
extern unsigned __int64 MyAtoU64(wchar_t*);


#endif