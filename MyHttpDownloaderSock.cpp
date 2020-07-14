#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#pragma comment(lib, "Htmlhelp.Lib")
// MyHttpDownloaderSock.cpp : Defines the entry point for the application.
// Men bu yerda asosiy quyidagi funk.larni ishlatmoqchi edim:
// InternetOpenURL InternetReadFile InternetQueryDataAvailable InternetOpen InternetSetFilePointer

#include "windows.h"
#include "targetver.h"
#include "resource.h"
#include "commctrl.h"
#include "DownloadChunk.h"
#include "DMFile.h"
#include "config.h"
#include "myHttp.h"
#include "Win7ProgressTaskbar.h"
#include "strsafe.h"
#include "myTime.h"
//#include "HtmlHelp.h"


#define MAX_LOADSTRING		100
#define MIN_WIN_WIDTH		460
#define MIN_WIN_HALF_WIDTH	230
#define MIN_WIN_HEIGHT		300

int scrnWidth(0);
int scrnHeight(0);
void DrawProgress(HDC,RECT*,int);

extern int MyStringCpy(wchar_t*,int,wchar_t*);
extern BOOL MyU64To(wchar_t*,int,unsigned __int64,int*);

// Global Variables:
HINSTANCE hInst;								// current instance
HWND  hWnd,hWndURL,hWndPauseBtn,hWndResetBtn;
HBRUSH blueBrsh(0),prgrsBckBrsh(0),prgrsLineBrsh(0),prgrsBrsh(0);HFONT hFnt(0);HPEN prgrsLinePen(0);
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
bool				MessagePump();
void				OnPaint(HDC,int state=0);
extern bool			Downloading();
extern bool			DownloadingAppend();
extern bool			DownloadingResend();

//http://developer.android.com/sdk/index.html
//http://developer.android.com/sdk/installing/index.html?pkg=tools
//http://www.uff.uz/images/files/mour7771.jpg
//http://soft.snet.uz/default.html
//http://muz.uz/engine/modules/mservice/download.php
//http://soft.snet.uz/internet/Downloaders/downloadMaster/dmaster.exe
//http://dl.mp3.uz/dl/video/6875/Hilary_Duff_-_Tattoo.mp4
//L"http://dl.mp3.uz/dl/single/417537/DJ_Antoin_-_Holidaye_Feat_Akon.mp3",//www.mp3.uz
//http://programm.uz/getfile.php?fname=Shaxmat.zip&key=288061432
//http://programm.uz/dwnldFiles/Shaxmat.zip
//http://programm.uz/getfile.php?fname=SINO/Sino.msi&key=288062668 dan 'Sino.msi' ni chiqarayapti;
//http://okay.uz/data/image/045689.jpg
//http://okay.uz/front/home/music_download/3853/1441250671
//http://okay.uz/index.php/front/home/clip_download/1793/1/1441250869
//http://soft.snet.uz/internet/Downloaders/BitTorrent/BitTorrent.exe
//http://soft.snet.uz/user/Punto_Swticher/punto%20switcher%203.2.8.94.exe 1.2Mb
//http://soft.snet.uz/multimedia/video-converter/Format%20Factory/format-factory.exe  0.3Mb
//http://soft.snet.uz/network/BWMeter/BMSetup.exe 0.7Mb
//http://soft.snet.uz/multimedia/video-converter/Freemake%20Video%20Converter/FreemakeVideoConverterSetup.exe 0.9 Mb
//http://soft.snet.uz/user/Readers/stduviewer.exe 1.5 Mb
//http://soft.snet.uz/utilities/shell/putty.exe 0.4 Mb
//http://soft.snet.uz/system/Tuning/PowerStrip/psbeta.exe 0.9 Mb
//http://soft.snet.uz/security/antispyware/SpywareTerminator/SpywareTerminatorSetup.exe 0.6 Mb
//http://soft.snet.uz/multimedia/Players/Media%20Player%20Classic/mplayerc_6491_15.02.2009_rus.7z 1.5 Mb
//http://soft.snet.uz/system/diagnostics/cpuz_149.zip 0.6 Mb    81.95.225.134
//URL to lpCmdLine;
LPSTR GetWinNotifyText(DWORD);
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{UNREFERENCED_PARAMETER(hPrevInstance);
 UNREFERENCED_PARAMETER(lpCmdLine);

 //MessageBox(NULL,lpCmdLine,lpCmdLine,MB_OK);

 if(!memFileHeader.SetURLFromCmndLine(lpCmdLine))
 {MessageBox(NULL,memFileHeader.URL,L"Error,undefined URL string...",MB_OK);
  return 0;
 }//MessageBox(NULL,(LPWSTR)memFileHeader.URL,L"URL:",MB_OK);if(' ' == memFileHeader.URL[0])MessageBox(NULL,(LPWSTR)memFileHeader.URL,L"Err:",MB_OK);

 // Initialize global strings
 LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
 LoadString(hInstance, IDC_MYDM, szWindowClass, MAX_LOADSTRING);
 MyRegisterClass(hInstance);

 config::Read();

 // Perform application initialization:
 if(!InitInstance (hInstance, nCmdShow))
  return FALSE;

 WSADATA wsaData;
 int iWSAInited = WSAStartup(MAKEWORD(2,2), &wsaData);
 if(iWSAInited != NO_ERROR)
  return false;

 if(!DMFile::Open())
 {MessageBox(hWnd,L"Error creating save file...",L"Quiting...",MB_OK);
  return false;
 }

 if(DMFile::bAppended)
  DownloadingAppend();
 else
  Downloading();

 for(;myHttp::bForceClose && myHttp::bResend;)
  DownloadingResend();

 WSACleanup();

 MessagePump();//Oxirgi messagelarni tozalasun!!!

 //config::Save();
 return 0;
}

bool MessagePump()
{MSG msg;bool r=true;
 while(PeekMessage(&msg, 0,  0, 0, PM_REMOVE))
 {TranslateMessage(&msg);
  DispatchMessage(&msg);
  if(WM_QUIT==msg.message)
   r=false;
  //OutputDebugStringA("\n");
  //OutputDebugStringA(GetWinNotifyText(msg.message));
 }
 return r;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{WNDCLASSEX wcex;
 wcex.cbSize		= sizeof(WNDCLASSEX);
 wcex.style			= CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc	= WndProc;
 wcex.cbClsExtra	= 0;
 wcex.cbWndExtra	= 0;
 wcex.hInstance		= hInstance;
 wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYDM));
 wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
 wcex.hbrBackground	= CreateSolidBrush(RGB(201,225,248));//(1,45,78));//(HBRUSH)GetStockObject(GRAY_BRUSH);
 wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_MYDM);
 wcex.lpszClassName	= szWindowClass;
 wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MYDM_SMALL));
 return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{hInst = hInstance; // Store instance handle in our global variable
 //wchar_t s[512];StringCchPrintf(s,512,L"%s% URL: %s",szTitle,memFileHeader.URL);
 HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, MIN_WIN_WIDTH, MIN_WIN_HEIGHT, NULL, NULL, hInstance, NULL);
 if(!hWnd)return FALSE;
 ShowWindow(hWnd, nCmdShow);
 UpdateWindow(hWnd);
 return TRUE;
}

int OnCreate(HWND wnd)
{hWnd = wnd;
//MessageBox(NULL,L"sdsd",L"sdsd",MB_OK);
 InitCommonControls();
 blueBrsh = CreateSolidBrush(RGB(136,189,238));
 prgrsBckBrsh = CreatePatternBrush(LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP_PRGRS_BRUSH1)));//CreateSolidBrush(RGB(128,0,255));
 prgrsLineBrsh = CreateSolidBrush(RGB(41,26,189));//27,119,188));//18,232,237));
 prgrsBrsh = CreatePatternBrush(LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP_PRGRS_BRUSH)));//prgrsBrsh = CreateSolidBrush(RGB(11,200,26));
 prgrsLinePen = CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND, 2, RGB(18,232,237));

 LOGFONT logFont = {0};
 logFont.lfHeight = -14;
 logFont.lfItalic = TRUE;
 logFont.lfWeight = FW_ULTRABOLD;
 logFont.lfCharSet = DEFAULT_CHARSET;
 StringCchPrintf(logFont.lfFaceName, 32, L"Verdana");
 hFnt=CreateFontIndirect(&logFont);

 //MessageBox(NULL,memFileHeader.URL,memFileHeader.URL,MB_OK);

 hWndURL = CreateWindow(L"EDIT",				// predefined class 
                        memFileHeader.URL,      //L"http://dl.mp3.uz/dl/single/417537/DJ_Antoin_-_Holidaye_Feat_Akon.mp3",//www.mp3.uz  http://soft.snet.uz/internet/Downloaders/downloadMaster/dmaster.exe
						WS_CHILD | WS_VISIBLE | // | WS_HSCROLL |
                        ES_LEFT | ES_AUTOHSCROLL, 
                        36, 5, 150, 18,			// set size in WM_SIZE message 
                        wnd,					// parent window 
                        (HMENU) 1,				// edit control ID 
                        hInst,
                        NULL);					// pointer not needed 
 if(!hWndURL)return -1;
 SendMessage(hWndURL,WM_SETFONT,(WPARAM)hFnt,TRUE);
 hWndPauseBtn = CreateWindow(L"BUTTON",			// predefined class 
                        L"Pause",
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        2, 90, 96, 28,			// set size in WM_SIZE message 
                        wnd,					// parent window 
                        (HMENU)2,				// edit control ID 
                        hInst,
                        NULL);					// pointer not needed 
 if(!hWndPauseBtn)return -1;
 SendMessage(hWndPauseBtn,WM_SETFONT,(WPARAM)hFnt,TRUE);
 hWndResetBtn = CreateWindow(L"BUTTON",			// predefined class 
                        L"Resend packets",
						WS_DISABLED | WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        100, 90, 152, 28,		// set size in WM_SIZE message 
                        wnd,					// parent window 
                        (HMENU)3,				// edit control ID 
                        hInst,
                        NULL);					// pointer not needed 
 if(!hWndResetBtn)return -1;
 SendMessage(hWndResetBtn,WM_SETFONT,(WPARAM)hFnt,TRUE);
 Win7PrgrsTaskbar::Init();
 return 0;
}

VOID OnDestroy()
{if(blueBrsh)DeleteObject(blueBrsh);blueBrsh=0;
 if(prgrsBckBrsh)DeleteObject(prgrsBckBrsh);prgrsBckBrsh=0;
 if(prgrsLinePen)DeleteObject(prgrsLinePen);prgrsLinePen=0;
 if(prgrsLineBrsh)DeleteObject(prgrsLineBrsh);prgrsLineBrsh=0;
 if(prgrsBrsh)DeleteObject(prgrsBrsh);prgrsBrsh=0;
 if(hFnt)DeleteObject(hFnt);hFnt=0;
 DMFile::Close();
 myHttp::Close(true);
 Win7PrgrsTaskbar::Close();
}

BOOL OnResize(LPARAM lParam)
{static BOOL bIn=FALSE;
 if(bIn)return FALSE;
 bIn=TRUE;
 scrnWidth = LOWORD(lParam);
 scrnHeight = HIWORD(lParam);
 BOOL bRet=TRUE;
 if(scrnWidth<MIN_WIN_WIDTH || scrnHeight<MIN_WIN_HEIGHT)//Shundan past bo'lsa,o'zgartirmasun;
 {if(scrnWidth<MIN_WIN_WIDTH)scrnWidth=MIN_WIN_WIDTH;
  if(scrnHeight<MIN_WIN_HEIGHT)scrnHeight=MIN_WIN_HEIGHT;
  //lParam=MAKELPARAM(dx,dy);
  RECT r;GetWindowRect(hWnd,&r);
  MoveWindow(hWnd,r.left,r.top,scrnWidth,scrnHeight,TRUE);
  bRet = FALSE;
 }MoveWindow(hWndURL,36,5,scrnWidth-45,18,TRUE);
  //MoveWindow(hWndURL,36,24,150,32,TRUE);
 bIn=FALSE;
 return bRet;
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{int wmId, wmEvent;
 PAINTSTRUCT ps;
 HDC hdc;

 switch (message)
 {case WM_CREATE:
  return OnCreate(hwnd);
  case WM_SIZE:
   OnResize(lParam);
  return 0;
  case WM_COMMAND:
   wmId    = LOWORD(wParam);
   wmEvent = HIWORD(wParam);
   // Parse the menu selections:
   switch (wmId)
   {//case IDM_ABOUT:
	//HtmlHelp(hwnd,L"myHttpDL.chm",0,NULL);
	//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
	//	break;
	//case IDM_EXIT:
	//SendMessage(hwnd,WM_CLOSE,0,0);
	//	break;
	case 2://Pause:
	 myHttp::bPaused = !myHttp::bPaused;
	 SetWindowText(hWndPauseBtn,myHttp::bPaused?L"Continue":L"Pause");
	 myTime::Pause(myHttp::bPaused);
	break;
	case 3://Resend:
	 myHttp::bForceClose=true;
	 myHttp::bResend=true;
	break;
	default:
	 return DefWindowProc(hwnd, message, wParam, lParam);
   }
  break;
  case WM_CTLCOLOREDIT:
   SetTextColor((HDC)wParam,RGB(0,0,0));
   SetBkColor((HDC)wParam,RGB(136,189,238));
  return (LRESULT)blueBrsh;
  //case WM_CTLCOLORBTN:
  //return (LRESULT)blueBrsh;
  case WM_PAINT:
   hdc = BeginPaint(hwnd, &ps);
   OnPaint(ps.hdc);
   EndPaint(hwnd, &ps);
  return 0;
  case WM_USER:
   //int i;i=GetCurrentThreadId();
   switch(wParam)
   {case 0://Paint:
     OnPaint(0,(int)lParam);
    break;
   }
  return 0;
  case WM_CLOSE:
   if(!myHttp::bForceClose)
   {if(memFileHeader.sizeDownloaded<memFileHeader.sizeFull || -1==memFileHeader.sizeFull)
    {if(IDYES==MessageBox(hwnd,L"Downloading process is in the working state.",L"Are you want terminate this process?",MB_YESNO))
      myHttp::bForceClose=true;
   }}else DestroyWindow(hwnd);
  return 0;
  case WM_DESTROY:
   OnDestroy();
   PostQuitMessage(0);
  return 0;;
  default:
   return DefWindowProc(hwnd, message, wParam, lParam);
 }
 return 0;
}

// Message handler for about box.
/*INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{UNREFERENCED_PARAMETER(lParam);
 switch (message)
 {case WM_INITDIALOG:
  return (INT_PTR)TRUE;
  case WM_COMMAND:
   if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
   {EndDialog(hDlg, LOWORD(wParam));
	return (INT_PTR)TRUE;
   }
  break;
 }return (INT_PTR)FALSE;
}*/

int paintState=0;
void OnPaint(HDC DC,int state)
{HDC dc=DC?DC:GetDC(hWnd);
 if(state>0)paintState=state;
 SetBkColor(dc,RGB(201,225,248));//SetBkMode(dc,TRANSPARENT);
 SetTextColor(dc,RGB(4,27,16));
 RECT rWnd;GetClientRect(hWnd,&rWnd);
 TextOut(dc,1,5,L"URL:",4);
 wchar_t s[260];StringCchPrintf(s,260,L"Wr.to: %s",DMFile::crFilePthAndName);//memFileHeader.crFileName);
 TextOut(dc,1,30,s,(int)wcslen(s));
 //TextOutA(dc,1,50,"Server:",7); O'zi serstr da bor ekan:
 TextOutA(dc,1,50,myHttp::serverStr,(int)strlen(myHttp::serverStr));
 TextOutA(dc,1,70,"Agent:",6);TextOutA(dc,46,70,config::userAgentStr,(int)strlen(config::userAgentStr));

 RECT r ={1,120,MIN_WIN_HALF_WIDTH,140};
 static DWORD spdOldTick=0;static F32 speed=0;
 DWORD tc = GetTickCount();
 if(tc-spdOldTick>250)
 {speed = myHttp::fSpeed;
  spdOldTick=tc;
  myHttp::SendToExtMaster(0);
 }StringCchPrintf(s,260,L"                                                                                                                                                                                                                                ");
 DrawText(dc,s,-1,&r,DT_LEFT);
 StringCchPrintf(s,260,L"Speed : %.2f KB/s.",speed);
 DrawText(dc,s,-1,&r,DT_LEFT);
 r.left=MIN_WIN_HALF_WIDTH;r.right=MIN_WIN_WIDTH;
 StringCchPrintf(s,260,L"P/c: %2d %c",Win7PrgrsTaskbar::perc,'%');
 DrawText(dc,s,-1,&r,DT_LEFT);

 int l= 0;
 r.left=1;r.top=140;r.bottom=160;r.right=MIN_WIN_HALF_WIDTH;
 wcscpy(s,L"Size    : 0");if(memFileHeader.sizeFull>-1)MyU64To(&s[10],240,memFileHeader.sizeFull,&l);else l=1;
 l+=10;s[l++]=' ';s[l++]='B';s[l++]='.';s[l]=0;
 DrawText(dc,s,-1,&r,DT_LEFT);

 r.top=160;r.bottom=180;
 wcscpy(s,L"Start time: ");myTime::GetTimeStr(0,&s[12]);
 DrawText(dc,s,-1,&r,DT_LEFT);

 r.top=180;r.bottom=200;
 wcscpy(s,L"Will finish after about: ");myTime::GetTimeStr(3,&s[25]);
 DrawText(dc,s,-1,&r,DT_LEFT);

 r.top=200;r.bottom=220;r.right=MIN_WIN_WIDTH;
 s[0]=0;if(DMFile::foundedFileName[0])
  StringCchPrintf(s,260,L"File: %s",DMFile::foundedFileName);
 else if(DMFile::crFilePthAndName[0])
 {wchar_t *p=wcsrchr(DMFile::crFilePthAndName,'\\');
  if(!p)p=&DMFile::crFilePthAndName[0];
  StringCchPrintf(s,(int)wcslen(p+1)+3,L"File: %s",p+1);
 }if(s[0])DrawText(dc,s,-1,&r,DT_LEFT);

 r.left=MIN_WIN_HALF_WIDTH;r.right=MIN_WIN_WIDTH;r.top=140;r.bottom=160;l=0;
 wcscpy(s,L"Downloaded : 0");if(memFileHeader.sizeDownloaded>-1)MyU64To(&s[13],240,memFileHeader.sizeDownloaded,&l);else l=1;
 l+=13;s[l++]=' ';s[l++]='B';s[l++]='.';s[l]=0;
 DrawText(dc,s,-1,&r,DT_LEFT);

 r.top=160;r.bottom=180;
 wcscpy(s,L"Working time    : ");myTime::GetTimeStr(2,&s[18]);
 DrawText(dc,s,-1,&r,DT_LEFT);

 r.top=180;r.bottom=200;
 wcscpy(s,L"Interrupted time: ");myTime::GetTimeStr(1,&s[18]);
 DrawText(dc,s,-1,&r,DT_LEFT);

 StringCchPrintf(s,18,L"Vdkbnld-Uhrhssn9");for(int i=0; i<17; i++)++s[i];//wcscpy(s,L"Welcome. Visit to:");for(int i=0; i<wcslen(s); i++)--s[i];
 r.top=88;r.bottom=104;r.left=254;r.right=386;
 DrawText(dc,s,17,&r,DT_LEFT);//Welcome.Visit to: ++
 r.top=104;r.bottom=120;

 StringCchPrintf(s,21,L"&gsso9..oqnfq`ll-ty&");for(int i=0; i<20; i++)++s[i];
 DrawText(dc,s,20,&r,DT_LEFT);//'http://programm.uz' ++

 SetBkColor(dc,RGB(136,189,238));
 r.left=1;r.top=rWnd.bottom-16;r.right=rWnd.right-1;r.bottom=rWnd.bottom-1;
 FillRect(dc,&r,blueBrsh);
 switch(paintState)
 {case 1://DownloadThread beginning:
   DrawText(dc,L"Starting downloading, try connect to the URL server...",-1,&r,DT_LEFT);
   DrawProgress(dc,&rWnd,1);
  break;
  case 2://DMFile::Opening wait:
   DrawText(dc,L"Try get URL resource parameters...",-1,&r,DT_LEFT);
   DrawProgress(dc,&rWnd,1);
  break;
  case 3:
   wcscpy(s,L"Downloading,choice type: single. Size:unknown.");
   DrawText(dc,s,-1,&r,DT_LEFT);
   DrawProgress(dc,&rWnd,2);
  break;
  case 4:
   wcscpy(s,L"Downloading,choice type:single.");
   DrawText(dc,s,-1,&r,DT_LEFT);
   DrawProgress(dc,&rWnd,3);
  break;
  case 5:
   wcscpy(s,L"Downloading,choice type:multipart.");
   DrawText(dc,s,-1,&r,DT_LEFT);
   DrawProgress(dc,&rWnd,4);
  break;
  case 6://Must draw full,but now checking!!!
   wcscpy(s,L"Downloaded,finishing.");
   DrawText(dc,s,-1,&r,DT_LEFT);
   DrawProgress(dc,&rWnd,5);
  break;
 }

 if(!DC)ReleaseDC(hWnd,dc);
}

void DrawProgress(HDC dc,RECT* rWnd,int type)
{static int xShft=0,iCnt=0;static bool dir=true;static DWORD oldTc=0;DWORD tc;
 RECT r1,r={rWnd->left+1,rWnd->bottom-35,rWnd->right-1,rWnd->bottom-17};
 int w=r.right-r.left,h=r.bottom-r.top;
 HGDIOBJ original = NULL;
 FillRect(dc,&r,prgrsBckBrsh);
 switch(type)
 {case 0://starting any process:
   r1.left=r.left+w/2-15;r1.top=r.top+h/3;r1.right=r1.left+5;r1.bottom=r1.top+5;
   FillRect(dc,&r1,prgrsLineBrsh);
   r1.left+=15;r1.right+=15;
   FillRect(dc,&r1,prgrsLineBrsh);
   r1.left+=15;r1.right+=15;
   FillRect(dc,&r1,prgrsLineBrsh);
  break;
  case 1://waiting for any process, progress to right, after to left, and so:
   if(dir)
   {xShft+=5;
	if(xShft>w-40)
    {xShft-=5;dir=!dir;
   }}else
   {xShft-=5;
	if(xShft<1)
    {xShft=0;dir=!dir;
   }}
   r1.left=xShft+r.left+5;r1.top=r.top+h/3;r1.right=r1.left+5;r1.bottom=r1.top+5;
   FillRect(dc,&r1,prgrsLineBrsh);
   r1.left+=15;r1.right+=15;
   FillRect(dc,&r1,prgrsLineBrsh);
   r1.left+=15;r1.right+=15;
   FillRect(dc,&r1,prgrsLineBrsh);
  break;
  case 2://downloading single with unknown size:
   tc = GetTickCount();
   if(tc-oldTc>250)
   {if(dir){if(++xShft>14){xShft=0;if(++iCnt>2){dir=false;xShft=14;iCnt=0;}}}
    else {if(--xShft<1){xShft=14;if(++iCnt>2){dir=true;xShft=0;iCnt=0;}}}
	oldTc=tc;
   }
   original = SelectObject(dc,prgrsLinePen);
   for(int x=xShft-12; x<w; x+=15)
   {int xx = r.left+x;int yy=r.top;int xxx=r.left+18+x;int yyy=yy+18;
    if(xx<1)
	{yy+=-xx+1;
	 xx=1;
	}else if(xxx>w+4)
	{yyy-=xxx-w-4;
	 xxx=w+4;
	}
	MoveToEx(dc,xx,yy,NULL);
    LineTo(dc,xxx,yyy);
   }
   SelectObject(dc,original);
  break;
  case 3://downloading single with size:
   double f;f=0.0f;if(memFileHeader.sizeFull>0)f=(double)memFileHeader.sizeDownloaded/(double)memFileHeader.sizeFull;
   r1.left=r.left;r1.top=r.top;r1.right=r1.left+(int)(f * w);r1.bottom=r.bottom;
   FillRect(dc,&r1,prgrsBrsh);
  break;
  case 4://downloading multipart constant part,with size:
   r1.top=r.top;r1.bottom=r.bottom;
   for(int i=0; i<config::iMaxMultipart; i++)
   {if(0==memFileHeader.headers[i].f.state)continue;//1:working,2-downed,but at the end:
    r1.left = 1+(int)(w * ((double)memFileHeader.headers[i].f.pos / (double)memFileHeader.sizeFull));
	r1.right = 1+(int)(w * ((double)(memFileHeader.headers[i].f.pos + memFileHeader.headers[i].f.downloaded) / (double)memFileHeader.sizeFull));
    FillRect(dc,&r1,prgrsBrsh);
#ifdef _DEBUG
	if(Win7PrgrsTaskbar::perc>98)
	{char s[128];sprintf(s,"\n%d : left: %d right: %d sz: %d dw: %d",i,r1.left,r1.right,memFileHeader.headers[i].f.size,memFileHeader.headers[i].f.downloaded);
	 OutputDebugStringA(s);
    }
#endif
   }
  break;
  case 5://downloaded, finishing
   r1.top=r.top;r1.bottom=r.bottom;r1.left = 5;r1.right = 5+(int)w;
   FillRect(dc,&r1,prgrsBrsh);
  break;
}}