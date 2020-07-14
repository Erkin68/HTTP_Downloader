// HANDLE f = CreateFile(L"pth.txt",GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
// DWORD d = SetFilePointer(f,5000,NULL,FILE_BEGIN);
// BOOL b = WriteFile(f,L"Assalom",8,0,NULL);
// CloseHandle(f);Hech narsa yozmasdan ham pointerini surish mumkin,faqat undan so'ng yozish amaliyoti bo'lishi shart.
// Hamma amallarimiz shuning asosida olib boriladi!!!
#include "myHttp.h"
#include "DownloadChunk.h"
#include "strsafe.h"
#include "Win7ProgressTaskbar.h"
#include "DMFile.h"
#include "config.h"


extern HWND hWnd;
extern bool MessagePump();


FileHeader apndFileHeader;

namespace myHttp
{
int iWorkingParts(0);


//Here firstSocket worked in blocking mode;
//In multipart writing, hole file with full size will be opened immediately.
//Every step we will write downloaded datas and append memFileHeader in the tile.
//Then file pointer will be shifted to back to the current pos.
//Remember thet first header will be queued with first socket!!!
//You must create other sockets...Undestand!!!
bool DownloadMultipartContext()
{bool r=false;
 ChnkHeader::WriteHeaders();
 memFileHeader.flag=2;

 fSpeed = 0.0f;fTimeToDownload = 0.0f;DWORD gtck = GetTickCount();
 do 
 {int iState1=0,iErr2=0;
  S64 lastTotDownloaded = memFileHeader.sizeDownloaded;
  for(int i=0; i<config::iMaxMultipart; i++)
  {if(memFileHeader.headers[i].f.state>1)
   {++iState1;
	int rBytes = memFileHeader.headers[i].Read();
    if(rBytes>0)
	{memFileHeader.headers[i].Write();
     memFileHeader.sizeDownloaded += rBytes;
	}else if(-2==rBytes)++iErr2;
	memFileHeader.headers[i].SplitIfDeprecated();
	ChnkHeader::WriteHeaders();
    MessagePump();
  }}
  if(iWorkingParts<config::iMinMultipart)
  {for(;myHttp::iMaxConnectServerAllowed>iWorkingParts && iWorkingParts<config::iMaxMultipart;)//In init iMaxConnectServerAllowed setted to max(100);
   {if(!ChnkHeader::IncrementPartitions())break;
    MessagePump();
  }}

/*#ifdef _DEBUG
  char s[128];sprintf(s,"\nTot.downloaded: %d",memFileHeader.sizeDownloaded);
  OutputDebugStringA(s);
#endif*/

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

   if(memFileHeader.sizeDownloaded == lastTotDownloaded)//Internet uzilgandur???
   {fSpeed = 0.0f;
    if(iState1==iErr2)//Fatal err,intrnt disconnected;
	{bForceClose=true;//Reconnect messages;
	 bResend=true;
   }}else if(delta>0)
     fSpeed = (float)((double)(memFileHeader.sizeDownloaded-lastTotDownloaded) / (double)delta);
   else fSpeed = 0.0f;

   Win7PrgrsTaskbar::Send();
   SendMessage(hWnd,WM_USER,0,5);
   MessagePump();
 }}
 while(memFileHeader.sizeDownloaded<memFileHeader.sizeFull && (!bForceClose));

 if(memFileHeader.sizeDownloaded==memFileHeader.sizeFull)
 {r=true;LONG l[2]={memFileHeader.sizeFull & 0xffffffff, memFileHeader.sizeFull>>32};
  SetFilePointer(DMFile::f,l[0],&l[1],FILE_BEGIN);
  SetEndOfFile(DMFile::f);
  DMFile::Close(true);
  SendMessage(hWnd,WM_USER,0,6);
  MessagePump();
 }
 r = true;
//End:
 if(!myHttp::bResend)
 {for(int i=1; i<config::iMaxMultipart; i++)
   memFileHeader.headers[i].Free();
  if(memFileHeader.headers)free(memFileHeader.headers);
 }
 return r;
}






}//end of namespace



namespace DMFile 
{

bool CheckForDMFileMultipartContext(HANDLE f,wchar_t *fName)
{DWORD readed=0;int ln;bool r=false;
 if(INVALID_SET_FILE_POINTER==SetFilePointer(f,-(int)sizeof(FileHeader),0,FILE_END))return false;
 if(!ReadFile(f,&apndFileHeader,sizeof(FileHeader),&readed,NULL))return false;
 if(!readed)return false;
 if(sizeof(FileHeader)!=readed)return false;
 apndFileHeader.URL=0;						  //for(int i=0;i<config::iMaxMultipart;i++)
 apndFileHeader.headers=0;					  //for(int i=0;i<config::iMaxMultipart;i++)
 if(apndFileHeader.iNumParts>100)return false;// WriteFile(DMFile::f,&memFileHeader.headers[i].f,sizeof(ChnkHeader::TF),&writed,NULL);
 if(apndFileHeader.URLSize>512)return false;  //WriteFile(DMFile::f,memFileHeader.URL,memFileHeader.URLSize*sizeof(wchar_t),&writed,NULL);
 ln=sizeof(wchar_t)*apndFileHeader.URLSize;   //WriteFile(DMFile::f,&memFileHeader,sizeof(memFileHeader),&writed,NULL);

 S64 fullSz=sizeof(FileHeader)+ln+apndFileHeader.iNumParts*sizeof(ChnkHeader::TF);
 if(apndFileHeader.sizeFull>0)fullSz+=apndFileHeader.sizeFull;
 DWORD sz=GetFileSize(f,&readed);
 S64 sz64 = (((S64)readed) << 32) | sz;
 if(sz64!=fullSz)return false;

 apndFileHeader.URL=(wchar_t*)malloc(ln+2);
 if(!apndFileHeader.URL)return false;
 if(INVALID_SET_FILE_POINTER==SetFilePointer(f,-(int)sizeof(FileHeader)-ln,0,FILE_END))goto Fail;
 readed=0;if(!ReadFile(f,&apndFileHeader.URL[0],ln,&readed,NULL))goto Fail;
 if(ln!=readed)goto Fail;
 apndFileHeader.URL[ln/2]=0;

 if(0==wcscmp(apndFileHeader.URL,memFileHeader.URL))
 {if(INVALID_SET_FILE_POINTER==SetFilePointer(f,-(int)sizeof(FileHeader)-ln-apndFileHeader.iNumParts*sizeof(ChnkHeader::TF),0,FILE_END))goto Fail;
  readed=0;apndFileHeader.headers=(ChnkHeader*)malloc(apndFileHeader.iNumParts*sizeof(ChnkHeader));
  if(!apndFileHeader.headers)goto Fail;
  for(int i=0; i<apndFileHeader.iNumParts; i++)
  {if(!ReadFile(f,&apndFileHeader.headers[i].f,sizeof(ChnkHeader::TF),&readed,NULL))goto Fail1;
   apndFileHeader.headers[i].f.lastDownloaded=0;
  }
  double pc = apndFileHeader.sizeFull>0?(100.0f*(double)apndFileHeader.sizeDownloaded/(double)apndFileHeader.sizeFull):0.0f;
  if(pc!=0.0f)
  {wchar_t s[128];StringCchPrintf(s,128,L"Founded unclosed file %s with %.2f downloaded part",fName,pc);
   if(IDYES==MessageBox(hWnd,s,L"Append existing file?",MB_YESNO))
    r=true;
 }}
Fail1:
 if(!r)if(apndFileHeader.headers){free(apndFileHeader.headers);apndFileHeader.headers=0;}
Fail:
 if(!r)if(apndFileHeader.URL){free(apndFileHeader.URL);apndFileHeader.URL=0;}
 return r;
}


}