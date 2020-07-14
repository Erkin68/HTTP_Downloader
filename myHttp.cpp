#include "myHttp.h"
#include "DownloadChunk.h"
#include "strsafe.h"
#include "config.h"
#include "Win7ProgressTaskbar.h"

extern HWND hWnd;
extern bool MessagePump();

namespace myHttp
{
struct sockaddr_in httpServerAddr;
char serverStr[512]="";
char *buf(0);

URL_COMPONENTS urlComp;
int iMaxConnectServerAllowed(100),iBufferSize(MAX_RECV_BUF_SZ);
float fSpeed(0.0f),fTimeToDownload(0.0f);
bool bPartialContext(false),bGlobalSizeContext(false),bPaused(false),bForceClose(false),bResend(false);


void Close(bool bFull)
{if(memFileHeader.headers)
 {for(int i=0; i<config::iMaxMultipart; i++)
  {if(INVALID_SOCKET!=memFileHeader.headers[i].sock)
    closesocket(memFileHeader.headers[i].sock);
   memFileHeader.headers[i].sock=INVALID_SOCKET;
 }}
 if(bFull)if(buf)free(buf);buf=0;
 myHttp::iWorkingParts=0;
}

bool CreateRecvBuffer()
{if(buf)buf=(char*)realloc(buf,iBufferSize);
 else buf=(char*)malloc(iBufferSize);
 if(!buf)return false;
 return true;
}

#define firstSocket memFileHeader.headers[0].sock
bool DownloadSingleUnknownSize()
{bool r=false;
 memFileHeader.flag=1;

 fSpeed = 0.0f;
 fTimeToDownload = 0.0f;
 DWORD gtck = GetTickCount();

 DWORD dwDownloaded=0,writed[2];
 int percent=0;
 do 
 {dwDownloaded=-1;
  if(memFileHeader.headers[0].SocketIsReadyForRecv())
   dwDownloaded = recv(firstSocket, buf, iBufferSize, 0);
  if(((DWORD)-1)==dwDownloaded)
  {MessageBox(hWnd,L"Server disconnected!",L"You must restart process",MB_OK);
   DMFile::Close(false);
   memFileHeader.sizeDownloaded=0;
   return false;
  }
  WriteFile(DMFile::f,buf,dwDownloaded,&writed[0],NULL);//dwonloaded size kerak emas;
  memFileHeader.sizeDownloaded+=(int)dwDownloaded;
  WriteFile(DMFile::f,memFileHeader.URL,memFileHeader.URLSize*sizeof(wchar_t),&writed[0],NULL);
  WriteFile(DMFile::f,&memFileHeader,sizeof(memFileHeader),&writed[1],NULL);
  LONG li = ((LONG)writed[0])+((LONG)writed[1]);
  SetFilePointer(DMFile::f,-li,NULL,FILE_CURRENT);

  DWORD gtcke = GetTickCount();

  if(bPaused)
  {for(;bPaused && (!bForceClose);)
   {MessagePump();
    Sleep(500);
   }gtck = GetTickCount();
   fSpeed = 0.0f;
  }
  else
  {DWORD delta = gtcke>gtck?(gtcke-gtck):0;
   fTimeToDownload += delta;
   gtck = gtcke;
   if(delta>0)
    fSpeed = (float)dwDownloaded / (float)delta;
   memFileHeader.sizeDownloaded += dwDownloaded;

   SendMessage(hWnd,WM_USER,0,3);
   MessagePump();
 }}
 while(dwDownloaded>0 && (!bForceClose));

 if(0==dwDownloaded || memFileHeader.sizeDownloaded==memFileHeader.sizeFull)
 {r=true;
  SetEndOfFile(DMFile::f);
  DMFile::Close(true);
  myHttp::bForceClose=true;
 }
 return true;//r;
}

bool DownloadSingleWithSize()
{bool r=false;
 memFileHeader.flag=1;

 fSpeed = 0.0f;
 fTimeToDownload = 0.0f;
 DWORD gtck = GetTickCount();

 DWORD dwDownloaded=0,writed[2];
 int percent=0;
 if(memFileHeader.sizeDownloaded<memFileHeader.sizeFull)
 {do 
  {dwDownloaded=-1;
   if(memFileHeader.headers[0].SocketIsReadyForRecv())
    dwDownloaded = recv(firstSocket, buf, iBufferSize, 0);
   if(!dwDownloaded)break;
   if((DWORD)-1==dwDownloaded)
   {MessageBox(hWnd,L"Server disconnected!",L"You must restart process",MB_OK);
	DMFile::Close(false);
    memFileHeader.sizeDownloaded=0;
	return false;
   }
   WriteFile(DMFile::f,buf,dwDownloaded,&writed[0],NULL);//dwonloaded size kerak emas;
   memFileHeader.sizeDownloaded+=(int)dwDownloaded;
   WriteFile(DMFile::f,memFileHeader.URL,memFileHeader.URLSize*sizeof(wchar_t),&writed[0],NULL);
   WriteFile(DMFile::f,&memFileHeader,sizeof(memFileHeader),&writed[1],NULL);
   LONG li = ((LONG)writed[0])+((LONG)writed[1]);
   SetFilePointer(DMFile::f,-li,NULL,FILE_CURRENT);

   DWORD gtcke = GetTickCount();

   if(bPaused)
   {for(;bPaused && (!bForceClose);)
    {MessagePump();
     Sleep(500);
    }gtcke = gtck = GetTickCount();
    fSpeed = 0.0f;
   }
   else
   {DWORD delta = gtcke>gtck?(gtcke-gtck):0;
    fTimeToDownload += delta;
    gtck = gtcke;
    if(delta>0 && dwDownloaded && memFileHeader.sizeDownloaded != dwDownloaded)
     fSpeed = (float)dwDownloaded / (float)delta;
    
	Win7PrgrsTaskbar::Send();
    SendMessage(hWnd,WM_USER,0,4);
	MessagePump();
  }}
  while(dwDownloaded>0 && (!bForceClose));
 }

 if(0==dwDownloaded || memFileHeader.sizeDownloaded==memFileHeader.sizeFull)
 {r=true;
  SetEndOfFile(DMFile::f);
  DMFile::Close(true);
 }
 return r;
}
#undef firstSocket

bool bFileNameSended=false;
BOOL SendToExtMaster(char type)//wchar_t* urlFileName,int crntDwnldPercent,__int64 szTot)
{static int perc = 0;
 if(perc!=Win7PrgrsTaskbar::perc)
 perc=Win7PrgrsTaskbar::perc;
 static HWND findWnd=0;
 if(0==findWnd)findWnd=FindWindow(L"My HHTP downloader master class",L"My HTTP downloader");
 if(!findWnd)return FALSE;

 typedef struct TCopyData
 {int pc;
  F32 fSpeed;
  S64 size;
  S64 downloaded;
  char typeOfMsg;
  char bGlobalSizeContext;
  char bPartialContext;
  wchar_t URL[512];
  char serverStr[128];
 }CopyData;

 static CopyData cd={	0,//Win7PrgrsTaskbar::perc,
						0.0f,//fSpeed,
						0,//memFileHeader.sizeFull,
						0,//memFileHeader.sizeDownloaded,
						0,
						0,//bGlobalSizeContext?1:0,
						0,//bPartialContext?1:0,
						L"",
						""};

 cd.pc=Win7PrgrsTaskbar::perc;
 cd.fSpeed=fSpeed;
 cd.size=memFileHeader.sizeFull;
 cd.downloaded=memFileHeader.sizeDownloaded;
 cd.typeOfMsg=type;
 cd.bGlobalSizeContext=bGlobalSizeContext?1:0;
 cd.bPartialContext=bPartialContext?1:0;

 //switch(type)
 if(!cd.serverStr[0])StringCchCopyA((STRSAFE_LPSTR)&cd.serverStr[0],128,&myHttp::serverStr[0]);
 else if((!bFileNameSended) && DMFile::foundedFileName[0])
 {StringCchCopy((STRSAFE_LPWSTR)&cd.serverStr[0],64,&DMFile::foundedFileName[0]);
  if(type!=2)cd.typeOfMsg=1;
  bFileNameSended=true;
 }
 if(!cd.URL[0])StringCchCopy(&cd.URL[0],512,memFileHeader.URL);

 COPYDATASTRUCT cs={0,sizeof(CopyData),&cd};
 BOOL r=(BOOL)SendMessage(findWnd,WM_COPYDATA,(WPARAM)hWnd,(LPARAM)&cs);
 if(!r)
 {findWnd=FindWindow(L"My HHTP downloader master class",L"My HTTP downloader");
  r=(BOOL)SendMessage(findWnd,WM_COPYDATA,(WPARAM)hWnd,(LPARAM)&cs);
 }
 return r;
}


}//end of namespace