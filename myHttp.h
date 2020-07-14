#ifndef _myHTTP_H__
#define _myHTTP_H__

#include "windows.h"
#include "winhttp.h"

#define MAX_RECV_BUF_SZ 20*1024

typedef unsigned __int64 U64;
typedef signed __int64 S64;

namespace myHttp
{
extern struct sockaddr_in httpServerAddr;
extern char serverStr[512];
extern char *buf;

extern int iMaxConnectServerAllowed,iBufferSize,iWorkingParts;
extern float fSpeed,fTimeToDownload;
extern bool bPartialContext,//Partitional downloading allowed from server;
			bGlobalSizeContext,//Global size determination allowed from server;
			bPaused,//Button pause was presssed;
			bForceClose,//Aplcn must be closed;
			bResend,
			bFileNameSended;
extern URL_COMPONENTS urlComp;


extern bool CreateRecvBuffer();
extern void Close(bool);


extern bool DownloadSingleUnknownSize();
extern bool DownloadSingleWithSize();
extern bool IncrementPartitions();

extern bool DownloadMultipartContext();

extern BOOL SendToExtMaster(char);

}

#endif
