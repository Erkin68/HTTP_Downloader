#pragma warning(disable:4996)

#include "windows.h"
#include "stdio.h"
#include "strsafe.h"
#include "shlobj.h"
#include <time.h>
#include "Options.h"
#include "resource.h"


extern HWND hWndTree;
bool CheckForDMFileMultipartContext(HANDLE,wchar_t*,int);


namespace config
{
char userAgentStr[128]="Mozilla/4.0 (compatible; MSIE 5.0; Windows 98)";
char protocolStr[32]=" HTTP/1.0";
wchar_t outDir[MAX_PATH*2]=L"";
int iMaxMultipart(8),iMinMultipart(4),iRecBufSize(1024*1024),
    iMaxConnectTimeout(1),iMaxRecvTimeout(1),iMaxSendTimeout(1),iMinSizeForDividing(32768);
bool GetFromDlg(HWND);
bool ReadConfig();
bool Save();
void SetDefault();
bool SetToDlg(HWND);
char GetMyFileType(wchar_t*);
}

using namespace config;

INT_PTR CALLBACK Options(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{UNREFERENCED_PARAMETER(lParam);
 switch (message)
 {case WM_INITDIALOG:
   CoInitializeEx(0,COINIT_APARTMENTTHREADED);
   ReadConfig();
   SetToDlg(hDlg);
   //wchar_t s[32];StringCchPrintf(s,19,L"iuuq;00qsphsbnn/v{"/*http://programm.uz*/);for(int i=0;i<18;i++)s[i]--;
   //SetDlgItemText(hDlg,IDC_STATIC3,s);
  return (INT_PTR)TRUE;


  case WM_COMMAND:
  switch(LOWORD(wParam))
  {case IDOK:
    if(GetFromDlg(hDlg))
	{if(Save())
      EndDialog(hDlg, LOWORD(wParam));
	}
   return (INT_PTR)TRUE;
   case IDCANCEL:
    EndDialog(hDlg, LOWORD(wParam));
   return (INT_PTR)TRUE;
   case IDC_EDIT_PROTOCOL_STRING:
    if(EN_CHANGE==HIWORD(wParam))
	{char s[64];int l;l=GetDlgItemTextA(hDlg,IDC_EDIT_PROTOCOL_STRING,s,64);
	 if(l>33)MessageBoxA(hDlg,"Long protocol string.Must be before 32 characters.",s,MB_OK);
	 if('H'!=s[0]||'T'!=s[1]||'T'!=s[2]||'P'!=s[3]||'/'!=s[4])
	  MessageBoxA(hDlg,"Protocol string must began from 'HTTP/' string.",s,MB_OK);
	}
   return (INT_PTR)TRUE;
   case IDC_EDIT_AGENT_STRING:
    if(EN_CHANGE==HIWORD(wParam))
	{char s[130];int l;l=GetDlgItemTextA(hDlg,IDC_EDIT_AGENT_STRING,s,64);
	 if(l>127)MessageBoxA(hDlg,"Long agent string.Must be before 128 characters.",s,MB_OK);
	}
   return (INT_PTR)TRUE;
   case IDC_BUTTON_BROWSE:wchar_t pt[MAX_PATH];
    BROWSEINFO bi;bi.hwndOwner = hDlg;
    bi.pidlRoot = 0;//pidlRoot;
	bi.pszDisplayName = &pt[0];
    bi.lpszTitle = L"Browse paths for saving downloaded files:";
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;
    PIDLIST_ABSOLUTE rt;rt=SHBrowseForFolder(&bi);
	if(rt)
	{SHGetPathFromIDList(rt, pt);
	 SetDlgItemText(hDlg,IDC_EDIT_OUT_DIR,pt);
	 CoTaskMemFree(rt);
	}
   return (INT_PTR)TRUE;
  }
  break;



  case WM_HSCROLL:int dwPos[2];char s[32];
   if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_MAX_PART))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
    dwPos[1] = (int)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_MIN_PART),TBM_GETPOS,0,0)+2;
	if(dwPos[1]-3>dwPos[0])
	{int n = dwPos[1]-3-dwPos[0];
	 SendMessage((HWND)lParam,TBM_SETPOS,TRUE,dwPos[0]+n);
	 dwPos[0] += n;
	}StringCchPrintfA(s,8,"%d",dwPos[0]+4);
	SetDlgItemTextA(hDlg,IDC_STATIC_1,s);
   }
   else if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_MIN_PART))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
    dwPos[1] = (int)SendMessage(GetDlgItem(hDlg,IDC_SLIDER_MAX_PART),TBM_GETPOS,0,0)+4;
	if(dwPos[0]+3>dwPos[1])
	{int n = dwPos[0]+3-dwPos[1];
	 SendMessage((HWND)lParam,TBM_SETPOS,TRUE,dwPos[0]-n);
	 dwPos[0] -= n;
	}StringCchPrintfA(s,8,"%d",dwPos[0]+2);
	SetDlgItemTextA(hDlg,IDC_STATIC_2,s);
   }
   else if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_MIN_PART_SIZE))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
	StringCchPrintfA(s,32,"%d B",(dwPos[0]+1)*4096);
	SetDlgItemTextA(hDlg,IDC_STATIC_3,s);
   }
   else if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_CONNECT_TIMEOUT))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
	StringCchPrintfA(s,8,"%d s",(dwPos[0]+1));
	SetDlgItemTextA(hDlg,IDC_STATIC_4,s);
   }
   else if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_SEND_TIMEOUT))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
	StringCchPrintfA(s,8,"%d s",(dwPos[0]+1));
	SetDlgItemTextA(hDlg,IDC_STATIC_5,s);
   }
   else if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_RECEIVE_TIMEOUT))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
	StringCchPrintfA(s,8,"%d s",(dwPos[0]+1));
	SetDlgItemTextA(hDlg,IDC_STATIC_6,s);
   }
   else if((HWND)lParam==GetDlgItem(hDlg,IDC_SLIDER_BUFFER_SIZE))
   {dwPos[0] = (int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
	StringCchPrintfA(s,32,"%d B",(dwPos[0]+1)*16384);
	SetDlgItemTextA(hDlg,IDC_STATIC_7,s);
   }   
  return 0;



  case WM_DESTROY:
   CoUninitialize();
  return 0;
 }
 return (INT_PTR)FALSE;
}

namespace config
{

bool ReadConfig()
{wchar_t s[512]=L"";SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,s);
 if(!s[0])return false;
 int ln=(int)wcslen(s);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL");
 WIN32_FIND_DATA fd;HANDLE h = FindFirstFile(s,&fd);
 if(INVALID_HANDLE_VALUE==h || (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
  CreateDirectory(s,NULL);
 if(INVALID_HANDLE_VALUE!=h)FindClose(h);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL\\conf.bin");
 FILE *f=0;_wfopen_s(&f,s,L"rb");
 if(!f)
 {
Fail:
  SetDefault();
  Save();
  return false;
 }

 if(!fread(&iMaxMultipart,sizeof(iMaxMultipart),1,f))goto Fail;
 if(iMaxMultipart>64)goto Fail;
 if(!fread(&iMinMultipart,sizeof(iMinMultipart),1,f))goto Fail;//iMinMultipart=10;
 if(iMinMultipart<2 || iMinMultipart>iMaxMultipart)goto Fail;
 if(!fread(&iMaxConnectTimeout,sizeof(iMaxConnectTimeout),1,f))goto Fail;
 if(iMaxConnectTimeout<1 || iMaxConnectTimeout>60)goto Fail;
 if(!fread(&iMaxRecvTimeout,sizeof(iMaxRecvTimeout),1,f))goto Fail;
 if(iMaxRecvTimeout<1 || iMaxRecvTimeout>60)goto Fail;
 if(!fread(&iMaxSendTimeout,sizeof(iMaxSendTimeout),1,f))goto Fail;
 if(iMaxSendTimeout<1 || iMaxSendTimeout>60)goto Fail;
 if(!fread(&iMinSizeForDividing,sizeof(iMinSizeForDividing),1,f))goto Fail;
 if(iMinSizeForDividing<1024 || iMinSizeForDividing>65535)goto Fail;
 
 __int16 l;fread(&l,sizeof(__int16),1,f);
 if(l<1 || l>520)goto Fail;
 fread(outDir,sizeof(__int16),l,f);
 outDir[l]=0;

 fread(&l,sizeof(__int16),1,f);
 if(l<1 || l>128)goto Fail;
 fread(userAgentStr,sizeof(char),l,f);
 userAgentStr[l]=0;

 fread(&l,sizeof(__int16),1,f);
 if(l<1 || l>32)goto Fail;
 fread(protocolStr,sizeof(char),l,f);
 protocolStr[l]=0;
 
 fclose(f);
 return true;
}

bool Save()
{wchar_t s[512]=L"";SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,s);
 if(!s[0])return false;
 int ln=(int)wcslen(s);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL");
 WIN32_FIND_DATA fd;HANDLE h = FindFirstFile(s,&fd);
 if(INVALID_HANDLE_VALUE==h || (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
  CreateDirectory(s,NULL);
 if(INVALID_HANDLE_VALUE!=h)FindClose(h);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL\\conf.bin");

 FILE *f=0;_wfopen_s(&f,s,L"wb");

 //fwrite(&bools,sizeof(Bools),1,f);
 fwrite(&iMaxMultipart,sizeof(iMaxMultipart),1,f);
 fwrite(&iMinMultipart,sizeof(iMinMultipart),1,f);
 fwrite(&iMaxConnectTimeout,sizeof(iMaxConnectTimeout),1,f);
 fwrite(&iMaxRecvTimeout,sizeof(iMaxRecvTimeout),1,f);
 fwrite(&iMaxSendTimeout,sizeof(iMaxSendTimeout),1,f);
 fwrite(&iMinSizeForDividing,sizeof(iMinSizeForDividing),1,f);

 __int16 l=(__int16)wcslen(outDir);
 fwrite(&l,sizeof(__int16),1,f);
 fwrite(outDir,sizeof(__int16),l,f);

 l=(__int16)strlen(userAgentStr);
 fwrite(&l,sizeof(__int16),1,f);
 fwrite(userAgentStr,sizeof(char),l,f);

 l=(__int16)strlen(protocolStr);
 fwrite(&l,sizeof(__int16),1,f);
 fwrite(protocolStr,sizeof(char),l,f);

 fclose(f);
 return true;
}

void SetDefault()
{iMaxMultipart=8;
 iMinMultipart=4;
 iMaxConnectTimeout=1;
 iMaxRecvTimeout=1;
 iMaxSendTimeout=1;
 iMinSizeForDividing=32768;
 StringCchPrintfA(userAgentStr,128,"Mozilla/4.0 (compatible; MSIE 5.0; Windows 98)");
 StringCchPrintfA(protocolStr,32," HTTP/1.0");
 int l=GetTempPath(MAX_PATH*2,outDir);
 if(0==l)StringCchPrintf(outDir,4,L"C:\\");
}

void SetTrackCtrl(HWND hTrck,int iMin,int iMax,int iPage,/*int iSelMin,int iSelMax,*/int iPos)
{SendMessage(hTrck,TBM_SETRANGE,(WPARAM)TRUE,(LPARAM)MAKELONG(iMin, iMax));
 SendMessage(hTrck,TBM_SETPAGESIZE,0,(LPARAM)iPage);
 //SendMessage(hTrck,TBM_SETSEL,(WPARAM)FALSE,(LPARAM)MAKELONG(iSelMin,iSelMax));
 SendMessage(hTrck,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)iPos);
}

bool SetToDlg(HWND hDlg)
{SetDlgItemTextA(hDlg,IDC_EDIT_PROTOCOL_STRING,&protocolStr[1]);
 SetDlgItemTextA(hDlg,IDC_EDIT_AGENT_STRING,userAgentStr);
 SetDlgItemText(hDlg,IDC_EDIT_OUT_DIR,outDir);
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_MAX_PART),0,28,1,iMaxMultipart-4);//4-32
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_MIN_PART),0,24,1,iMinMultipart-2);//2-28
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_MIN_PART_SIZE),0,30,1,iMinSizeForDividing/4096-1);//4096-126976
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_CONNECT_TIMEOUT),0,99,1,iMaxConnectTimeout-1);//1-100
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_SEND_TIMEOUT),0,99,1,iMaxSendTimeout-1);//1-100
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_RECEIVE_TIMEOUT),0,99,1,iMaxRecvTimeout-1);//1-100
 SetTrackCtrl(GetDlgItem(hDlg,IDC_SLIDER_BUFFER_SIZE),0,127,1,iRecBufSize/16384-1);//16384-2097152

 char s[32];StringCchPrintfA(s,8,"%d",iMaxMultipart);SetDlgItemTextA(hDlg,IDC_STATIC_1,s);
 StringCchPrintfA(s,8,"%d",iMinMultipart);SetDlgItemTextA(hDlg,IDC_STATIC_2,s);
 StringCchPrintfA(s,32,"%d B",iMinSizeForDividing);SetDlgItemTextA(hDlg,IDC_STATIC_3,s);
 StringCchPrintfA(s,32,"%d s",iMaxConnectTimeout);SetDlgItemTextA(hDlg,IDC_STATIC_4,s);
 StringCchPrintfA(s,32,"%d s",iMaxSendTimeout);SetDlgItemTextA(hDlg,IDC_STATIC_5,s);
 StringCchPrintfA(s,32,"%d s",iMaxRecvTimeout);SetDlgItemTextA(hDlg,IDC_STATIC_6,s);
 StringCchPrintfA(s,32,"%d B",iRecBufSize);SetDlgItemTextA(hDlg,IDC_STATIC_7,s);


 return true;
}

bool GetFromDlg(HWND hDlg)
{char s[128];int l=GetDlgItemTextA(hDlg,IDC_EDIT_PROTOCOL_STRING,s,64);
 if(0==l || l>32)
 {
Fail1:MessageBox(hDlg,L"Enter correct protocol string as 'HTTP/1.0'",L"Err.",MB_OK);
  return false;
 }if('H'!=s[0]||'T'!=s[1]||'T'!=s[2]||'P'!=s[3]||'/'!=s[4])goto Fail1;
 protocolStr[0]=' ';StringCchPrintfA(&protocolStr[1],31,s);


 l=GetDlgItemTextA(hDlg,IDC_EDIT_AGENT_STRING,s,128);
 if(0==l || l>127)
 {MessageBox(hDlg,L"Enter correct agent string.'",L"Err.",MB_OK);
  return false;
 }StringCchCopyA(userAgentStr,128,s);


 wchar_t ou[MAX_PATH*2];l=GetDlgItemText(hDlg,IDC_EDIT_OUT_DIR,ou,MAX_PATH*2);
 if(0==l)
 {MessageBox(hDlg,L"Enter correct path for saving files.'",L"Err.",MB_OK);
  return false;
 }if('\\'!=ou[l-1]){ou[l++]='\\';ou[l]=0;}
 ou[l++]='*';ou[l++]='.';ou[l++]='*';ou[l]=0;
 WIN32_FIND_DATA ff;HANDLE h=FindFirstFile(ou,&ff);
 if(INVALID_HANDLE_VALUE==h)
 {MessageBox(hDlg,L"Enter existing path for saving files.'",L"Err.",MB_OK);
  return false;
 }FindClose(h);ou[l-3]=0;
 StringCchCopy(outDir,MAX_PATH*2,ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_1,ou,MAX_PATH))return false;
 iMaxMultipart = _wtoi(ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_2,ou,MAX_PATH))return false;
 iMinMultipart = _wtoi(ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_3,ou,MAX_PATH))return false;
 iMinSizeForDividing = _wtoi(ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_4,ou,MAX_PATH))return false;
 iMaxConnectTimeout = _wtoi(ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_5,ou,MAX_PATH))return false;
 iMaxSendTimeout = _wtoi(ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_6,ou,MAX_PATH))return false;
 iMaxRecvTimeout = _wtoi(ou);

 if(!GetDlgItemText(hDlg,IDC_STATIC_7,ou,MAX_PATH))return false;
 iRecBufSize = _wtoi(ou);

 return true;
}

bool SaveDnldFileInfo(int iItemTree)
{wchar_t s[512]=L"";SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,s);
 if(!s[0])return false;
 int ln=(int)wcslen(s);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL\\history.bin");
 FILE *f=0;_wfopen_s(&f,s,L"ab");
 if(!f)return false;

 s[0]=0;ListView_GetItemText(hWndTree,iItemTree,0,s,512);
 if(0==s[0])goto End;

 char type=GetMyFileType(s);
 s[0]=0;ListView_GetItemText(hWndTree,iItemTree,7,s,512);//File name
 if(0!=s[0])
  type=GetMyFileType(s);
 fwrite(&type,1,1,f);

 __time64_t t;_time64(&t);
 fwrite(&t,sizeof(t),1,f);

 __int64 sz=0;
 s[0]=0;ListView_GetItemText(hWndTree,iItemTree,1,s,512);
 if(0!=s[0])sz = MyAtoU64(s);
 fwrite(&sz,sizeof(sz),1,f);

 s[0]=0;ListView_GetItemText(hWndTree,iItemTree,0,s,512);
 fwprintf(f,L"%s",&s[0]);
 __int16 endStr=0;fwrite(&endStr,2,1,f);

 s[0]=0;ListView_GetItemText(hWndTree,iItemTree,3,s,512);
 if(0==s[0])StringCchCopy(s,16,L"Unknown server");
 fwprintf(f,L"%s",&s[0]);
 fwrite(&endStr,2,1,f);

End:fclose(f);
 return true;
}

char GetMyFileType(wchar_t *fPathName)
{wchar_t *p=wcsrchr(fPathName,'.');
 if(!p)return 0x00;
 ++p;
 if(0==(*p))return  0x00;//unknown;
 if('m'== *p && 'd'== *(p+1) && 'l'== *(p+2) && 0==*(p+3))
 {wchar_t *pp=p-1;*pp=0;
  p=wcsrchr(fPathName,'.');
  *pp='.';
  if(!p)return 0x00;
  ++p;
  if(0==(*p))return  0x00;//unknown;
 }
 if(wcsstr(p,L"com"))return 0x01;//execution
 if(wcsstr(p,L"COM"))return 0x01;//execution
 if(wcsstr(p,L"exe"))return 0x01;//execution
 if(wcsstr(p,L"EXE"))return 0x01;//execution
 if(wcsstr(p,L"dll"))return 0x01;//execution
 if(wcsstr(p,L"DLL"))return 0x01;//execution
 if(wcsstr(p,L"cpl"))return 0x01;//execution
 if(wcsstr(p,L"CPL"))return 0x01;//execution
 if(wcsstr(p,L"dlu"))return 0x01;//execution
 if(wcsstr(p,L"DLU"))return 0x01;//execution
 if(wcsstr(p,L"dlt"))return 0x01;//execution
 if(wcsstr(p,L"DLT"))return 0x01;//execution
 if(wcsstr(p,L"bat"))return 0x01;//execution
 if(wcsstr(p,L"BAT"))return 0x01;//execution

 if(wcsstr(p,L"zip"))return 0x02;//archive
 if(wcsstr(p,L"ZIP"))return 0x02;//archive
 if(wcsstr(p,L"rar"))return 0x02;//archive
 if(wcsstr(p,L"RAR"))return 0x02;//archive
 if(wcsstr(p,L"tar"))return 0x02;//archive
 if(wcsstr(p,L"TAR"))return 0x02;//archive
 if(wcsstr(p,L"arj"))return 0x02;//archive
 if(wcsstr(p,L"ARJ"))return 0x02;//archive
 if(wcsstr(p,L"7z"))return 0x02;//archive
 if(wcsstr(p,L"7Z"))return 0x02;//archive
 if(wcsstr(p,L"cab"))return 0x02;//archive
 if(wcsstr(p,L"CAB"))return 0x02;//archive
 if(wcsstr(p,L"tgz"))return 0x02;//archive
 if(wcsstr(p,L"TGZ"))return 0x02;//archive
 if(wcsstr(p,L"lha"))return 0x02;//archive
 if(wcsstr(p,L"LHA"))return 0x02;//archive

 if(wcsstr(p,L"jpg"))return 0x03;//images
 if(wcsstr(p,L"JPG"))return 0x03;//images
 if(wcsstr(p,L"jpeg"))return 0x03;//images
 if(wcsstr(p,L"JPEG"))return 0x03;//images
 if(wcsstr(p,L"bmp"))return 0x03;//images
 if(wcsstr(p,L"BMP"))return 0x03;//images
 if(wcsstr(p,L"png"))return 0x03;//images
 if(wcsstr(p,L"PNG"))return 0x03;//images
 if(wcsstr(p,L"tiff"))return 0x03;//images
 if(wcsstr(p,L"TIFF"))return 0x03;//images
 if(wcsstr(p,L"tif"))return 0x03;//images
 if(wcsstr(p,L"TIF"))return 0x03;//images
 if(wcsstr(p,L"tga"))return 0x03;//images
 if(wcsstr(p,L"TGA"))return 0x03;//images
 if(wcsstr(p,L"gif"))return 0x03;//images
 if(wcsstr(p,L"GIF"))return 0x03;//images
 if(wcsstr(p,L"dib"))return 0x03;//images
 if(wcsstr(p,L"DIB"))return 0x03;//images
 if(wcsstr(p,L"pcx"))return 0x03;//images
 if(wcsstr(p,L"PCX"))return 0x03;//images
 if(wcsstr(p,L"ps"))return 0x03;//images
 if(wcsstr(p,L"PS"))return 0x03;//images
 if(wcsstr(p,L"pict"))return 0x03;//images
 if(wcsstr(p,L"PICT"))return 0x03;//images
 if(wcsstr(p,L"dds"))return 0x03;//images
 if(wcsstr(p,L"DDS"))return 0x03;//images
 if(wcsstr(p,L"max"))return 0x03;//images
 if(wcsstr(p,L"MAX"))return 0x03;//images

 if(wcsstr(p,L"mp3"))return 0x04;//sounds
 if(wcsstr(p,L"MP3"))return 0x04;//sounds
 if(wcsstr(p,L"wav"))return 0x04;//sounds
 if(wcsstr(p,L"WAV"))return 0x04;//sounds
 if(wcsstr(p,L"snd"))return 0x04;//sounds
 if(wcsstr(p,L"SND"))return 0x04;//sounds
 if(wcsstr(p,L"au"))return 0x04;//sounds
 if(wcsstr(p,L"AU"))return 0x04;//sounds
 if(wcsstr(p,L"aif"))return 0x04;//sounds
 if(wcsstr(p,L"AIF"))return 0x04;//sounds
 if(wcsstr(p,L"voc"))return 0x04;//sounds
 if(wcsstr(p,L"VOC"))return 0x04;//sounds
 if(wcsstr(p,L"svx"))return 0x04;//sounds
 if(wcsstr(p,L"SVX"))return 0x04;//sounds
 if(wcsstr(p,L"v8"))return 0x04;//sounds
 if(wcsstr(p,L"V8"))return 0x04;//sounds
 if(wcsstr(p,L"vox"))return 0x04;//sounds
 if(wcsstr(p,L"VOX"))return 0x04;//sounds

 if(wcsstr(p,L"mpg"))return 0x05;//video
 if(wcsstr(p,L"MPG"))return 0x05;//video
 if(wcsstr(p,L"avi"))return 0x05;//video
 if(wcsstr(p,L"AVI"))return 0x05;//video
 if(wcsstr(p,L"mp4"))return 0x05;//video
 if(wcsstr(p,L"MP4"))return 0x05;//video
 if(wcsstr(p,L"vob"))return 0x05;//video
 if(wcsstr(p,L"VOB"))return 0x05;//video
 if(wcsstr(p,L"vcd"))return 0x05;//video
 if(wcsstr(p,L"VCD"))return 0x05;//video
 if(wcsstr(p,L"dat"))return 0x05;//video
 if(wcsstr(p,L"dat"))return 0x05;//video

 return 0x00;
}

void ListExistingDMFiles(wchar_t *initDir,int type)
{wchar_t pth[512];WIN32_FIND_DATA fd;
 StringCchCopy(pth,512,initDir);StringCchCat(pth,512,L"*.*");
 HANDLE h=FindFirstFile(pth,&fd);
 if(INVALID_HANDLE_VALUE==h)return;
 while(FindNextFile(h, &fd)!=0)
 {if('.'==fd.cFileName[0] && '.'==fd.cFileName[1] && 0==fd.cFileName[2])continue;
  wchar_t s[1024];StringCchCopy(s,1024,pth);
  int l=(int)wcslen(s);StringCchCopy(&s[l-3],1024-l-3,fd.cFileName);
  if(fd.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
  {StringCchCat(s,1024,L"\\");
   ListExistingDMFiles(s,type);
  }else
  {HANDLE f = CreateFile(s,GENERIC_READ,FILE_SHARE_READ,NULL,
					     OPEN_EXISTING,						// existing file only
						 FILE_ATTRIBUTE_NORMAL,				// normal file
						 NULL);								// no attr. template
   if(INVALID_HANDLE_VALUE!=f)
   {CheckForDMFileMultipartContext(f,&s[0],type);
    CloseHandle(f);
 }}}
 FindClose(h);
}

bool ChangeTree(bool bTop,int type)
{BOOL b=ListView_DeleteAllItems(hWndTree);
 if(bTop)
 {ListExistingDMFiles(config::outDir,type);
  return true;
 }

 wchar_t s[512]=L"";SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,s);
 if(!s[0])return false;
 int ln=(int)wcslen(s);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL\\history.bin");
 FILE *f=0;_wfopen_s(&f,s,L"rb");
 if(!f)return false;

 for(;!feof(f);)
 {char chType;if(!fread(&chType,1,1,f))break;
  __time64_t t;if(!fread(&t,sizeof(t),1,f))break;
  __int64 sz=0;if(!fread(&sz,sizeof(sz),1,f))break;
  s[0]=0;if(!fwscanf(f,L"%s",&s[0]))break;//URL
  wchar_t servStr[128]=L"";if(!fwscanf(f,L"%s",&servStr[0]))break;//ServerStr
  if(0==type || type==chType)//GetMyFileType(s))
  {InsertNewUrl(&s[0],
				NULL,
				100,
				&sz,
				0);
   ListView_SetItemText(hWndTree,TreeView_GetCount(hWndTree)-1,7,servStr);
 }}
//End:
 fclose(f);
 return true;


 return true;
}




}

unsigned __int64 MyAtoU64(wchar_t *s)
{
register unsigned __int64 rt=0;
wchar_t *ps = s;
	while(*ps!=0)
	{	if(*ps < '0')
			return 0;
		if(*ps > '9')
			return 0;
		rt = rt*10 + (*ps - '0');
		ps++;
	}
	return rt;
}

BOOL MyU64To(wchar_t* st,int sLen,unsigned __int64 u,int *ln)
{
register int i;
register unsigned __int64 delit = u;
static wchar_t s[32];
	*ln=0;
	if(!u)
	{	st[0] = '0';
		st[1] = 0;
		return TRUE;
	}

	while(delit)
	{	s[(*ln)++] = delit % 10 + '0';
		delit /= 10;
	}
	for(i=0; i<(*ln); i++)
		st[i] = s[(*ln)-i-1];
	st[*ln] = 0;
	return TRUE;
}