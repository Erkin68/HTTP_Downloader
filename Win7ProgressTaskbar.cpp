#include "Win7ProgressTaskbar.h"
#include "downloadchunk.h"
#include "strsafe.h"


extern HWND hWnd;


extern "C" const IID IID_ITaskbarList3 =
 { 0x0EA1AFB91, 0x09E28, 0x04B86, { 0x090, 0x0E9, 0x09E, 0x09F, 0x08A, 0x05E, 0x0EF, 0x0AF } };
extern "C" const CLSID CLSID_TaskbarList3 =
 { 0x56FDF344, 0xFD6D, 0x11D0, { 0x095, 0x08A, 0x000, 0x060, 0x097, 0x0C9, 0x0A0, 0x090 } };



namespace Win7PrgrsTaskbar
{

int perc=0;
wchar_t Caption[260]=L"";
bool bInited=false;
ITaskbarList*  tbList=0;
ITaskbarList3* tbList3=0;


bool Init()
{HRESULT hr = CoInitialize(NULL);
 if(FAILED(hr))return false;
 tbList = NULL;
 tbList3 = NULL;
 hr = CoCreateInstance(CLSID_TaskbarList,NULL,CLSCTX_INPROC_SERVER,IID_ITaskbarList,(void**)&(tbList));
 if(FAILED(hr))
 {tbList = NULL;
  return false;
 }
 hr = tbList->QueryInterface(IID_ITaskbarList3, (void**) &(tbList3));
 if(FAILED(hr))
 {tbList3 = NULL;
  tbList->Release();
  tbList = NULL;
  return false;
 }
 StringCchPrintf(Caption,260,L"OK");
 bInited=true;

 return true;	 
}

void Close()
{if(bInited)
 {if(tbList3)
  {tbList3->Release();
   tbList3 = NULL;
  }
  if(tbList)
  {tbList->Release();
   tbList = NULL;
  }
  CoUninitialize();
}}

void Send(int Perc, int type)
{if(!bInited)return;
 tbList3->SetProgressState(hWnd, type);//TBPF_NORMAL TBPF_PAUSED TBPF_ERROR TBPF_NORMAL TBPF_INDETERMINATE TBPF_NOPROGRESS
 tbList3->SetProgressValue(hWnd, Perc, 100);
}

void Send()
{double dPerc = (double)memFileHeader.sizeDownloaded / (double)memFileHeader.sizeFull;
 float fPerc = (float)dPerc * 100.0f;
 int iPerc = (int)fPerc;
 if(iPerc!=perc)
 {if(bInited)
  {tbList3->SetProgressState(hWnd, TBPF_NORMAL);
   tbList3->SetProgressValue(hWnd, iPerc, 100);
  }perc=iPerc;
}}

}//end of namespace;