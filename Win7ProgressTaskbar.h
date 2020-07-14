#ifndef _WIN7PROGRESSTASKBAR_H__
#define _WIN7PROGRESSTASKBAR_H__


#include "windows.h"
#include "commctrl.h"
//#include "ShObjIdl.h"

#ifndef TSKINTH
#define TSKINTH
 
#define TBPF_NOPROGRESS     0
#define TBPF_INDETERMINATE  0x1
#define TBPF_NORMAL         0x2
#define TBPF_ERROR          0x4
#define TBPF_PAUSED         0x8
 
 
EXTERN_C const IID IID_ITaskbarList;
 
MIDL_INTERFACE("56FDF342-FD6D-11d0-958A-006097C9A090")
ITaskbarList : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE HrInit( void) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE AddTab(
        HWND hwnd) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE DeleteTab(
        HWND hwnd) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE ActivateTab(
        HWND hwnd) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE SetActiveAlt(
        HWND hwnd) = 0;
 
};
 
 
MIDL_INTERFACE("602D4995-B13A-429b-A66E-1935E44F4317")
ITaskbarList2 : public ITaskbarList
{
public:
    virtual HRESULT STDMETHODCALLTYPE MarkFullscreenWindow(
        HWND hwnd,
        BOOL fFullscreen) = 0;
 
};
 
typedef struct THUMBBUTTON {
 
    DWORD dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[ 260 ];
    DWORD dwFlags;
 
}   THUMBBUTTON;
 
typedef struct THUMBBUTTON *LPTHUMBBUTTON;
 
//extern "C" const IID IID_ITaskbarList3 =
// { 0x0EA1AFB91, 0x09E28, 0x04B86, { 0x090, 0x0E9, 0x09E, 0x09F, 0x08A, 0x05E, 0x0EF, 0x0AF } };
 
MIDL_INTERFACE("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")
ITaskbarList3 : public ITaskbarList2
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetProgressValue(
         HWND hwnd,
         ULONGLONG ullCompleted,
         ULONGLONG ullTotal);
 
    virtual HRESULT STDMETHODCALLTYPE SetProgressState(
         HWND hwnd,
         int tbpFlags);
 
    virtual HRESULT STDMETHODCALLTYPE RegisterTab(
        HWND hwndTab,
        HWND hwndMDI) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE UnregisterTab(
        HWND hwndTab) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE SetTabOrder(
        HWND hwndTab,
        HWND hwndInsertBefore) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE SetTabActive(
        HWND hwndTab,
        HWND hwndMDI,
        DWORD dwReserved) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons(
        HWND hwnd,
        UINT cButtons,
        LPTHUMBBUTTON pButton) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons(
        HWND hwnd,
        UINT cButtons,
        LPTHUMBBUTTON pButton) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList(
        HWND hwnd,
        HIMAGELIST himl) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon(
        HWND hwnd,
        HICON hIcon,
        LPCWSTR pszDescription) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip(
        HWND hwnd,
        LPCWSTR pszTip) = 0;
 
    virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip(
        HWND hwnd,
        RECT *prcClip) = 0;
    
};
 
EXTERN_C const CLSID CLSID_TaskbarList;
 
//extern "C" const CLSID CLSID_TaskbarList3 =
// { 0x56FDF344, 0xFD6D, 0x11D0, { 0x095, 0x08A, 0x000, 0x060, 0x097, 0x0C9, 0x0A0, 0x090 } };
 
class DECLSPEC_UUID("56FDF344-FD6D-11d0-958A-006097C9A090") TaskbarList;
 
EXTERN_C const CLSID CLSID_TaskbarList3;
 
#endif






namespace Win7PrgrsTaskbar
{
extern int perc;

extern bool Init();
extern void Close();
extern void Send(int,int type=TBPF_NORMAL);
extern void Send();


}// end of namespace

#endif