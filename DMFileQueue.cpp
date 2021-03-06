#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#include "windows.h"
#include "config.h"
#include "DownloadChunk.h"
#include "DMFile.h"
#include "myHttp.h"
#include "strsafe.h"
#include "resource.h"
#include "Win7ProgressTaskbar.h"


typedef unsigned __int32 U32;
extern HINSTANCE hInst;
extern HWND hWnd,hWndURL,hWndResetBtn;
extern FileHeader apndFileHeader;


namespace DMFile
{

HANDLE f=0;
int bAppended=0;
wchar_t *tempName=0;
wchar_t crFilePthAndName[512]=L"";
wchar_t foundedFileName[512]=L"";

void ChangeFileNameToAllowed(wchar_t *name)
{for(;*name;name++)
 {if(0 >= *name && 31 <= *name)
   *name += '1';//'@';
  else if('<' == *name)
   *name = '2';
  else if('>' == *name)
   *name = '3';
  //else if(':' == *name)*name = 'Â';
  else if('"' == *name)
   *name = '4';
  //else if('/' == *name)*name = 'Ä';
  //else if('\\'== *name)*name = 'Å';
  else if('|' == *name)
   *name = '5';
  else if('?' == *name)
   *name = '6';//'Ç';
  else if('*' == *name)
   *name = '7';
}}

BOOL CALLBACK CrFileDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{int w,h,wch,hch;RECT r,rch;HWND hw;
 switch (message) 
 {case WM_INITDIALOG:
   SetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,DMFile::crFilePthAndName);
   SetTimer(hwndDlg,0,4000,NULL);
   SendMessage(GetDlgItem(hwndDlg,IDC_CHECK_TIMER),BM_SETCHECK,BST_CHECKED,0);
   hw = GetParent(hwndDlg);
   if(hw)
   {GetWindowRect(hw,&r);
    GetWindowRect(hwndDlg,&rch);
	w=r.right-r.left;
	h=r.bottom-r.top;
	wch=rch.right-rch.left;
	hch=rch.bottom-rch.top;
	MoveWindow(hwndDlg,r.left+(w-wch)/2,20+r.top+(h-hch)/2,wch,hch,TRUE);
   }
  return TRUE;
  case WM_TIMER:
   if(!GetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,crFilePthAndName,512))
    MessageBox(hwndDlg,L"Entered name is long...",L"Please,reenter.",MB_OK);
   else EndDialog(hwndDlg, wParam);
  return TRUE; 
  case WM_DESTROY:
   KillTimer(hwndDlg,0);
   return 0;
  case WM_COMMAND:
   switch (LOWORD(wParam))
   {case IDC_CHECK_TIMER:
     if(BST_CHECKED!=SendMessage(GetDlgItem(hwndDlg,IDC_CHECK_TIMER),BM_GETCHECK,0,0))
      KillTimer(hwndDlg,0);
    return TRUE;
    case IDOK:
    case IDCANCEL:
     if(!GetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,crFilePthAndName,512))
	  MessageBox(hwndDlg,L"Entered name is long...",L"Please,reenter.",MB_OK);
	 else EndDialog(hwndDlg, wParam);
    return TRUE; 
	case IDC_BUTTON_BROWSE:
	 OPENFILENAME ofn;       // common dialog box structure
	 wchar_t szFile[260];    // buffer for file name

	 // Initialize OPENFILENAME
	 ZeroMemory(&ofn, sizeof(ofn));
	 ofn.lStructSize = sizeof(ofn);
	 ofn.hwndOwner = hwndDlg;
	 ofn.lpstrFile = szFile;
	 //
	 // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	 // use the contents of szFile to initialize itself.
	 //
	 ofn.lpstrFile[0] = '\0';
	 ofn.nMaxFile = sizeof(szFile);
	 ofn.lpstrFilter = L"All\0*.*\0Dml\0*.dml\0";
	 ofn.nFilterIndex = 1;
	 ofn.lpstrFileTitle = NULL;
	 ofn.nMaxFileTitle = 0;
	 ofn.lpstrInitialDir = NULL;
	 ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	 // Display the Open dialog box.

	 if(GetOpenFileName(&ofn)==TRUE)
      SetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,ofn.lpstrFile);//StringCchPrintf(crFilePthAndName,wcslen(crFilePthAndName),ofn.lpstrFile);
     return TRUE;
 }}
 return FALSE; 
}

BOOL CALLBACK OvFileDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{int w,h,wch,hch;RECT r,rch;HWND hw;
 switch (message) 
 {case WM_INITDIALOG:
   SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
   SetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,(LPCWSTR)lParam);
   hw = GetParent(hwndDlg);
   if(hw)
   {GetWindowRect(hw,&r);
    GetWindowRect(hwndDlg,&rch);
	w=r.right-r.left;
	h=r.bottom-r.top;
	wch=rch.right-rch.left;
	hch=rch.bottom-rch.top;
	MoveWindow(hwndDlg,r.left+(w-wch)/2,20+r.top+(h-hch)/2,wch,hch,TRUE);
   }
  return TRUE;
  case WM_COMMAND:
   switch (LOWORD(wParam))
   {case IDOK:
	 EndDialog(hwndDlg, 1);
    return TRUE;
    case IDCANCEL:
	 EndDialog(hwndDlg, 2);
    return TRUE;
	case ID_WRITE_NEW:
	 wchar_t *pName;pName=(wchar_t*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
     GetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,pName,2*(int)wcslen(pName)-4);
	 EndDialog(hwndDlg, 1);
     return TRUE;
	case IDC_BUTTON_BROWSE:
	 OPENFILENAME ofn;       // common dialog box structure
	 wchar_t szFile[260];    // buffer for file name

	 // Initialize OPENFILENAME
	 ZeroMemory(&ofn, sizeof(ofn));
	 ofn.lStructSize = sizeof(ofn);
	 ofn.hwndOwner = hwndDlg;
	 ofn.lpstrFile = szFile;
	 //
	 // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	 // use the contents of szFile to initialize itself.
	 //
	 ofn.lpstrFile[0] = '\0';
	 ofn.nMaxFile = sizeof(szFile);
	 ofn.lpstrFilter = L"All\0*.*\0Dml\0*.dml\0";
	 ofn.nFilterIndex = 1;
	 ofn.lpstrFileTitle = NULL;
	 ofn.nMaxFileTitle = 0;
	 ofn.lpstrInitialDir = NULL;
	 ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	 // Display the Open dialog box.

	 if(GetOpenFileName(&ofn)==TRUE)
      SetDlgItemText(hwndDlg,IDC_EDIT_FILE_NAME,ofn.lpstrFile);//StringCchPrintf(crFilePthAndName,wcslen(crFilePthAndName),ofn.lpstrFile);
     return TRUE;
 }}
 return FALSE; 
}

bool Open()
{wchar_t foundedFilePathAndName[512];
 if(CheckDMsForExisting(&foundedFilePathAndName[0],config::outDir))
 {if(OpenForAppending(foundedFilePathAndName))
  {wcscpy(crFilePthAndName,foundedFilePathAndName);
   int ln=(int)wcslen(crFilePthAndName);
   tempName = (wchar_t*)malloc((ln+1)*sizeof(wchar_t));
   StringCchPrintf(tempName,ln+1,L"%s",crFilePthAndName);
   SetWindowText(hWndURL,apndFileHeader.URL);
   EnableWindow(hWndResetBtn,TRUE);
   Win7PrgrsTaskbar::perc=(int)(100.0f*(double)apndFileHeader.sizeDownloaded/apndFileHeader.sizeFull);
   return true;
 }}
 int ln=(int)wcslen(config::outDir)+memFileHeader.URLSize+6;
 StringCchPrintf(crFilePthAndName,512,L"%s",config::outDir);
 wchar_t *p=wcsrchr(memFileHeader.crFileName,'/');//memFileHeader.URL
 //URL_COMPONENTS uc;uc.dwStructSize=sizeof(uc);uc.dwSchemeLength= 1; uc.dwHostNameLength  = 1; uc.dwUserNameLength  = 1; uc.dwPasswordLength  = 1; uc.dwUrlPathLength   = 1; uc.dwExtraInfoLength = 260; wchar_t s[260]; uc.lpszScheme     = NULL;//"http" uc.lpszHostName   = NULL;//"dl.mp3.uz" uc.lpszUserName   = NULL;//""; uc.lpszPassword   = NULL; uc.lpszUrlPath    = NULL; uc.lpszExtraInfo  = s; InternetCrackUrl(memFileHeader.URL,memFileHeader.URLSize-1,ICU_DECODE,&uc);
 StringCchCat(crFilePthAndName,512,p?(p+1):memFileHeader.crFileName);//memFileHeader.URL);
 StringCchCat(crFilePthAndName,512,L".mdl");
 int allNameNum=0;
 DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG_FILE_NAME),hWnd,(DLGPROC)CrFileDlgProc);
TrCrF:
 f=CreateFile(crFilePthAndName,FILE_WRITE_DATA,FILE_SHARE_WRITE,NULL,//default security _wfopen(pth,L"wb"); GENERIC_WRITE
                  CREATE_ALWAYS,						// new file
                  FILE_ATTRIBUTE_NORMAL,				// normal file
                  NULL);								// no attr. template
 if(f==INVALID_HANDLE_VALUE)
 {if(ERROR_INVALID_NAME==GetLastError())//&&(0==allNameNum++))
  {ChangeFileNameToAllowed(crFilePthAndName);
   goto TrCrF;
 }}
 ln=(int)wcslen(crFilePthAndName);
 tempName = (wchar_t*)malloc((ln+1)*sizeof(wchar_t));
 StringCchPrintf(tempName,ln+1,L"%s",crFilePthAndName);
 if(f==INVALID_HANDLE_VALUE)
 {f=0;return false;}
 DWORD writed;WriteFile(f,memFileHeader.URL,memFileHeader.URLSize*sizeof(wchar_t),&writed,NULL);
 WriteFile(f,&memFileHeader,sizeof(memFileHeader),&writed,NULL);
 return true;
}

bool ChangeFileName(char* newName)
{
 return true;
}

void Close(bool rFinishHttp)
{if(f)
 {CloseHandle(f);f=0;
  if(rFinishHttp)
  {int ln = (int)wcslen(tempName)+1;
   wchar_t *newName = new wchar_t[2*ln];
   StringCchPrintf(newName,ln,L"%s",tempName);
   if(foundedFileName[0])
   {wchar_t *p=wcsrchr(newName,'\\');
    if(!p)p=&newName[0];else ++p;
    wcscpy(p,foundedFileName);
   }else
   {wchar_t *p=wcsrchr(newName,'.');
    if(p)*p=0;
   }
   WIN32_FIND_DATA ff;bool r=true;
   HANDLE h=FindFirstFile(newName,&ff);
   if(INVALID_HANDLE_VALUE!=h)
   {S64 sz = (((S64)ff.nFileSizeHigh)<<32) | ff.nFileSizeLow;
    if(2==DialogBoxParam(hInst,MAKEINTRESOURCE(sz==memFileHeader.sizeFull?IDD_DIALOG_FILE_OVERWRITE:IDD_DIALOG_FILE_OVERWRITE1),
									hWnd,(DLGPROC)OvFileDlgProc,(LPARAM)&newName[0]))
     r=false;
    FindClose(h);
   }if(r)
   MoveFileEx(tempName,newName,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);

   myHttp::bFileNameSended=false;
   wchar_t *p=wcsrchr(newName,'\\');if(!p)p=&newName[0];
   wcscpy(foundedFileName,p);
   myHttp::SendToExtMaster(2);

  //DeleteFile(tempName);
  }
  if(tempName)free(tempName);tempName=0;
}}

bool CheckForDMFile(wchar_t *pth)
{HANDLE f = CreateFile(pth,GENERIC_READ,FILE_SHARE_READ,NULL,
						   OPEN_EXISTING,						// existing file only
						   FILE_ATTRIBUTE_NORMAL,				// normal file
						   NULL);								// no attr. template
 if(INVALID_HANDLE_VALUE==f)return false;
 if(CheckForDMFileMultipartContext(f,pth))
  bAppended=1;
 CloseHandle(f);
 return bAppended>0?true:false;
}

bool CheckDMsForExisting(wchar_t *fFind,wchar_t *initDir)
{bool r=false;wchar_t pth[512];WIN32_FIND_DATA fd;
 StringCchCopy(pth,512,initDir);StringCchCat(pth,512,L"*.*");fFind[0]=0;
 HANDLE h=FindFirstFile(pth,&fd);
 if(INVALID_HANDLE_VALUE==h)return false;
 while(FindNextFile(h, &fd)!=0)
 {if('.'==fd.cFileName[0] && '.'==fd.cFileName[1] && 0==fd.cFileName[2])continue;
  wchar_t s[1024];StringCchCopy(s,1024,pth);
  int l=(int)wcslen(s);StringCchCopy(&s[l-3],1024-l-3,fd.cFileName);
  if(fd.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
  {StringCchCat(s,1024,L"\\");
   if(CheckDMsForExisting(fFind,s))
   {r=true;
    break;
  }}else
  {r=CheckForDMFile(s);
   if(r)
   {wcscpy(fFind,s);
	break;
 }}}
 FindClose(h);
 return r;
}

bool OpenForAppending(wchar_t *pth)
{f=CreateFile(pth,FILE_WRITE_DATA,FILE_SHARE_WRITE,NULL,//default security _wfopen(pth,L"wb"); GENERIC_WRITE
                  OPEN_EXISTING,						// existing file only
                  FILE_ATTRIBUTE_NORMAL,				// normal file
                  NULL);								// no attr. template
 if(f==INVALID_HANDLE_VALUE)return false;
 return true;
}

}//end of namespace;