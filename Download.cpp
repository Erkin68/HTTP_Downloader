#include "DownloadChunk.h"
#include "DMFile.h"
#include <shlwapi.h>
#include "config.h"
#include "myHttp.h"
#include "Win7ProgressTaskbar.h"
#include "strsafe.h"
#include "myTime.h"


extern HWND hWnd,hWndResetBtn;
extern FileHeader apndFileHeader;
extern bool MessagePump();
FileHeader memFileHeader;


FileHeader::FileHeader():URL(0),URLhostName(0),URLSize(0),URLpath(0),crFileName(0),
						 sizeFull(-1),sizeDownloaded(0),fromPos(0),toPos(0),flag(0){}
FileHeader::~FileHeader()
{/*if(URL)free(URL);*/URL=0;
 /*if(URLhostName)free(URLhostName);*/URLhostName=0;
 /*if(URLpath)free(URLpath);*/URLpath=0;
 /*if(crFileName)free(crFileName);*/crFileName=0; 
}//malloc process heap free dan keyin xato beradur;

//class FileHeader:
//class FileHeader:
//class FileHeader:
//class FileHeader:
bool FileHeader::SetURLFromCmndLine(wchar_t* cmdLn)
{URLSize = (U16)wcslen(cmdLn)+1;
 if(URLSize<6)return false;
 URL = (wchar_t*)malloc(URLSize*sizeof(U16));
 StringCchCopy((wchar_t*)URL,URLSize,cmdLn);
 //MessageBox(NULL,URL,URL,MB_OK);

 //cracking URL:
 DWORD dwUrlLen = 0;
 ZeroMemory(&myHttp::urlComp, sizeof(myHttp::urlComp));
 myHttp::urlComp.dwStructSize = sizeof(myHttp::urlComp);
 myHttp::urlComp.dwSchemeLength=myHttp::urlComp.dwHostNameLength=myHttp::urlComp.dwUrlPathLength=myHttp::urlComp.dwExtraInfoLength=-1;

 //wchar_t s[512];StringCchPrintf(s,512,L"%s ln: %d",URL,memFileHeader.URLSize);
 //MessageBox(NULL,s,L"WinHttpCrackUrl failed",MB_OK);

 if(!WinHttpCrackUrl(URL,memFileHeader.URLSize, 0, &myHttp::urlComp))
 {//MessageBox(NULL,URL,L"WinHttpCrackUrl failed",MB_OK);
  return false;
 }

 if((DWORD)(-1)==myHttp::urlComp.dwHostNameLength)return false;
 if((DWORD)(-1)==myHttp::urlComp.dwUrlPathLength)return false;
 if((DWORD)(-1)==myHttp::urlComp.dwSchemeLength)return false;
 if((DWORD)(-1)==myHttp::urlComp.dwExtraInfoLength)return false;

 URLhostName = (wchar_t*)malloc((myHttp::urlComp.dwHostNameLength+1)*sizeof(wchar_t));
 memcpy(URLhostName,myHttp::urlComp.lpszHostName,myHttp::urlComp.dwHostNameLength*sizeof(wchar_t));
 URLhostName[myHttp::urlComp.dwHostNameLength]=0;

 int totURLpathLn=myHttp::urlComp.dwUrlPathLength+myHttp::urlComp.dwExtraInfoLength;
 URLpath = (wchar_t*)malloc((totURLpathLn+1)*sizeof(wchar_t));
 memcpy(URLpath,myHttp::urlComp.lpszUrlPath,totURLpathLn*sizeof(wchar_t));
 URLpath[totURLpathLn]=0;

 //Find file name from command line arguments php - > GET:
 int iAsterPos=0;//,iArgs=0;
 for(int i=0; i<totURLpathLn; i++)
 {if('?'==URLpath[i])
  {if(iAsterPos<0)
    iAsterPos=i;
  }//else if('&'==URLpath[i])
   // iArgs++;
  else if('='==URLpath[i] && i<totURLpathLn && iAsterPos>-1)
  {if(!crFileName)
   {crFileName=(wchar_t*)malloc((totURLpathLn-i+1)*sizeof(wchar_t));
    StringCchPrintf(crFileName,totURLpathLn-i+1,L"%s",&URLpath[i+1]);
	wchar_t *p=wcschr(crFileName,'&');
	if(p)*p=0;
	p=wcschr(crFileName,'=');
	if(p)*p=0;
	break;
 }}}
 if(!crFileName)
 {crFileName=URLpath;//(wchar_t*)malloc((totURLpathLn)*sizeof(wchar_t));
  //wcscpy(crFileName,URLpath);
 }

 LPWCH envStrs = GetEnvironmentStringsW();
 //MessageBox(NULL,envStrs,envStrs,MB_OK);
 wchar_t *p = wcsstr(envStrs,L"path=");//For example:   Range=2000-5400
 if(p)StringCchCopy(config::outDir,520,p+5);

 /*wchar_t *p = wcsstr(envStrs,L"Range=");//For example:   Range=2000-5400
 if(p)
 {wchar_t *pp=wcschr(p,'-');
  if(pp)
  {*pp=0;
   int iFrom = _wtoi(p+6);
   int iTo = _wtoi(pp+1);
   if(!iTo)iFrom=0;
   memFileHeader.fromPos=iFrom;
   memFileHeader.toPos=iTo;
 }}*/
return true;
}

bool Downloading()
{bool r=false;
 SendMessage(hWnd,WM_USER,0,1);

 memFileHeader.flag=2;memFileHeader.iNumParts=(U8)config::iMaxMultipart;
 memFileHeader.headers = (ChnkHeader*)malloc(config::iMaxMultipart*sizeof(ChnkHeader));
 if(!memFileHeader.headers)goto End;
  for(int i=0; i<config::iMaxMultipart; i++)memFileHeader.headers[i].ChnkHeader::ChnkHeader();

RestSingle:
 if(INVALID_SOCKET==memFileHeader.headers[0].CreateSocket())
 {MessageBox(hWnd,L"Error creating socket...",L"Quitng...",MB_OK);
  return false;
 }

 myHttp::iBufferSize = memFileHeader.headers[0].GetSocketMaxRecvBuffer();
 if(!myHttp::CreateRecvBuffer())
 {MessageBox(hWnd,L"Error creating input memory buffer...",L"Quitng...",MB_OK);
  goto End;
 }

 for(;!memFileHeader.headers[0].ChnkHeader::TryConnectSocket();)
 {SendMessage(hWnd,WM_USER,0,1);
  if((!MessagePump()) || myHttp::bForceClose)goto End;
  if(myHttp::bPaused)
  {for(;myHttp::bPaused && (!myHttp::bForceClose);)
   {MessagePump();
    Sleep(500);
  }}else Sleep(100);
 }

 SendMessage(hWnd,WM_USER,0,2);
 myTime::Init();
 for(;!memFileHeader.headers[0].ChnkHeader::FirstGet();)
 {if((!MessagePump()) || myHttp::bForceClose)goto End;
  if(myHttp::bPaused)
  {for(;myHttp::bPaused && (!myHttp::bForceClose);)
   {MessagePump();
    Sleep(500);
  }}else Sleep(100);
 }Win7PrgrsTaskbar::Send();

 if((!myHttp::bGlobalSizeContext) || (-1==memFileHeader.sizeFull))
 {SendMessage(hWnd,WM_USER,0,3);
  r=myHttp::DownloadSingleUnknownSize();
  if(!r)
  {myHttp::Close(false);
   goto RestSingle;
 }}else if(!myHttp::bPartialContext)
 {SendMessage(hWnd,WM_USER,0,4);MessagePump();
  r=myHttp::DownloadSingleWithSize();
  if(!r)
  {myHttp::Close(false);
   goto RestSingle;
 }}else
 {SendMessage(hWnd,WM_USER,0,5);
  EnableWindow(hWndResetBtn,TRUE);
  r=myHttp::DownloadMultipartContext();
 }

 if(!myHttp::bForceClose)
  SendMessage(hWnd,WM_CLOSE,0,0);

End:
 return true;
#undef CheckPause
}

bool DownloadingAppend()//sockets not inited,from starting program.
{bool r=false;
 SendMessage(hWnd,WM_USER,0,1);

 config::iMaxMultipart=apndFileHeader.iNumParts;
 memFileHeader.flag=3;memFileHeader.iNumParts=(U8)config::iMaxMultipart;
 memFileHeader.sizeFull=apndFileHeader.sizeFull;
 memFileHeader.sizeDownloaded=apndFileHeader.sizeDownloaded;

 memFileHeader.headers = (ChnkHeader*)malloc(config::iMaxMultipart*sizeof(ChnkHeader));
 if(!memFileHeader.headers)goto End;
 for(int i=0; i<config::iMaxMultipart; i++)
  memFileHeader.headers[i]=apndFileHeader.headers[i];
 //free(apnFileHeader.URL);free(apnFileHeader.headers);  malloc is in the app mem space, destroy when quiting...

 SendMessage(hWnd,WM_USER,0,2);
 myTime::Init();
 myHttp::bGlobalSizeContext=true;
 myHttp::bPartialContext=true;

 for(int i=0; i<config::iMaxMultipart; i++)
 {if(memFileHeader.headers[i].f.state>1)
  {if(INVALID_SOCKET==memFileHeader.headers[i].CreateSocket())
   {MessageBox(hWnd,L"Error creating socket...",L"Quitng...",MB_OK);
    return false;
 }}}

 myHttp::iBufferSize = memFileHeader.headers[0].GetSocketMaxRecvBuffer();
 if(!myHttp::CreateRecvBuffer())
 {MessageBox(hWnd,L"Error creating input memory buffer...",L"Quitng...",MB_OK);
  goto End;
 }

 for(int i=0; i<config::iMaxMultipart; i++)
 {if(2==memFileHeader.headers[i].f.state)
  {for(;!memFileHeader.headers[i].ChnkHeader::TryConnectSocket();)
   {SendMessage(hWnd,WM_USER,0,1);
    if((!MessagePump()) || myHttp::bForceClose)goto End;
    if(myHttp::bPaused)
    {for(;myHttp::bPaused && (!myHttp::bForceClose);)
     {MessagePump();
      Sleep(500);
    }}else Sleep(100);
 }}}

 myHttp::iWorkingParts=0;
 for(int i=0; i<config::iMaxMultipart; i++)
 {if(2==memFileHeader.headers[i].f.state)
  {if(!memFileHeader.headers[i].NextGet(true))
   {MessageBox(hWnd,L"Error receiving first chunk for append file part...",L"Quitng...",MB_OK);
    return false;
   }++myHttp::iWorkingParts;
 }}

 SendMessage(hWnd,WM_USER,0,5);
 r=myHttp::DownloadMultipartContext();

 if(!myHttp::bForceClose)
  SendMessage(hWnd,WM_CLOSE,0,0);

End:
 return true;
#undef CheckPause
}

bool DownloadingResend()//sockets inited,we must destroy first,create new sockets.Process is multipart;
{bool r=false;

 myHttp::bForceClose = false;
 myHttp::bResend = false;//Aynan DownloadMultipartContext da yana qayta true berishi mumkin?!!!

 SendMessage(hWnd,WM_USER,0,1);

 memFileHeader.flag=4;

 for(int i=0; i<config::iMaxMultipart; i++)
 {if(INVALID_SOCKET!=memFileHeader.headers[i].sock)
   closesocket(memFileHeader.headers[i].sock);
  memFileHeader.headers[i].sock=INVALID_SOCKET;
 }

 for(int i=0; i<config::iMaxMultipart; i++)
 {if(memFileHeader.headers[i].f.state>1)
  {if(INVALID_SOCKET==memFileHeader.headers[i].CreateSocket())
   {MessageBox(hWnd,L"Error creating socket...",L"Quitng...",MB_OK);
    return false;
 }}}

 for(int i=0; i<config::iMaxMultipart; i++)
 {for(;!memFileHeader.headers[i].ChnkHeader::TryConnectSocket();)
  {SendMessage(hWnd,WM_USER,0,1);
   if((!MessagePump()) || myHttp::bForceClose)goto End;
   if(myHttp::bPaused)
   {for(;myHttp::bPaused && (!myHttp::bForceClose);)
    {MessagePump();
     Sleep(500);
   }}else Sleep(100);
 }}

 myHttp::iWorkingParts=0;
 for(int i=0; i<config::iMaxMultipart; i++)
 {if(!memFileHeader.headers[i].NextGet(true))
  {MessageBox(hWnd,L"Error receiving first chunk for append file part...",L"Quitng...",MB_OK);
   return false;
  }++myHttp::iWorkingParts;
 }

 SendMessage(hWnd,WM_USER,0,5);
 r=myHttp::DownloadMultipartContext();

 if(!myHttp::bForceClose)
  SendMessage(hWnd,WM_CLOSE,0,0);

End:
 return true;
#undef CheckPause
}
