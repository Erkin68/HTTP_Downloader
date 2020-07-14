#include "windows.h"


unsigned __int64 MyAtoU64A(char*);
unsigned __int64 MyAtoU64(wchar_t*);
BOOL MyU64ToA(char*,int,unsigned __int64,int*);
BOOL MyU64To(wchar_t*,int,unsigned __int64);
char* MyStringAddModulePathA(char*);
wchar_t* MyStringAddModulePath(wchar_t*);
