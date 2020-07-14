#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#include "windows.h"
#include "stdio.h"
#include "config.h"
#include "strsafe.h"
#include "shlobj.h"


namespace config
{

char userAgentStr[128]="Mozilla/4.0 (compatible; MSIE 5.0; Windows 98)";
char protocolStr[32]=" HTTP/1.0";
wchar_t outDir[MAX_PATH*2]=L"";
//Bools bools(0);
int iMaxMultipart(10),iMinMultipart(4),iRecBufSize(1024*1024),
    iMaxConnectTimeout(1),iMaxRecvTimeout(1),iMaxSendTimeout(1),iMinSizeForDividing(32768);

void Read()
{
/*	DWORD al[]={1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10};
	DWORD al1[]={11,12,13,14,15,16,17,18,19,20,11,12,13,14,15,16,17,18,19,20,11,12,13,14,15,16,17,18,19,20,11,12,13,14,15,16,17,18,19,20};
	FILE *f=fopen("al.al:stream1","wb");
	fwrite(al,4,40,f);

	fpos_t t=4000;
	fsetpos(f,&t);
	fwrite(al,4,40,f);

	fclose(f);
	f=fopen("al.al:stream2","wb");
	fwrite(al1,4,40,f);

	fsetpos(f,&t);
	fwrite(al,4,40,f);

	fclose(f);

	DWORD al[40],al1[40];
	FILE *f=fopen("al.al:stream1","rb");
	fread(al,4,40,f);
	fclose(f);
	f=fopen("al.al:stream2","rb");
	fread(al1,4,40,f);
	fclose(f);*/

 wchar_t s[512]=L"";SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,s);
 if(!s[0])return;
 int ln=(int)wcslen(s);
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL");
 WIN32_FIND_DATA fd;HANDLE h = FindFirstFile(s,&fd);
 if(INVALID_HANDLE_VALUE==h || (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
  CreateDirectory(s,NULL);
 if(INVALID_HANDLE_VALUE!=h)FindClose(h);
	
 StringCchPrintf(&s[ln],512-ln,L"\\myHttpDL\\conf.bin");
 //GetModuleFileName(NULL,s,512);
 //wchar_t *p=wcsrchr(s,'\\');
 //if(p)StringCchCopy(p+1,9,L"conf.bin");
 //else StringCchCopy(s,9,L"conf.bin");
 FILE *f=0;_wfopen_s(&f,s,L"rb");
 if(!f)
 {SetDefault();
  //Save();
  return;
 }

 //fread(&bools,sizeof(Bools),1,f);
 if(!fread(&iMaxMultipart,sizeof(iMaxMultipart),1,f))//iMaxMultipart=10;
 {
Fail:fclose(f);
  SetDefault();
  return;
 }if(iMaxMultipart>64)goto Fail;
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
 if(0==outDir[0])
 {fread(outDir,sizeof(__int16),l,f);
  outDir[l]=0;
 }else fseek(f,l*sizeof(__int16),SEEK_CUR);

 fread(&l,sizeof(__int16),1,f);
 if(l<1 || l>128)goto Fail;
 fread(userAgentStr,sizeof(char),l,f);
 userAgentStr[l]=0;

 fread(&l,sizeof(__int16),1,f);
 if(l<1 || l>32)goto Fail;
 fread(protocolStr,sizeof(char),l,f);
 protocolStr[l]=0;
 
 fclose(f);
}

void Save()
{//wchar_t s[512];GetModuleFileName(NULL,s,512);
 //wchar_t *p=wcsrchr(s,'\\');
 //if(p)StringCchCopy(p+1,9,L"conf.bin");
 //else StringCchCOpy(s,9,L"conf.bin");
 wchar_t s[512]=L"";SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,s);
 if(!s[0])return;
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
}

void SetDefault()
{//bools=0;
 iMaxMultipart=8;
 iMinMultipart=4;
 iMaxConnectTimeout=1;
 iMaxRecvTimeout=1;
 iMaxSendTimeout=1;
 iMinSizeForDividing=32768;
 strcpy(userAgentStr,"Mozilla/4.0 (compatible; MSIE 5.0; Windows 98)");
 strcpy(protocolStr," HTTP/1.0");
 if(0==outDir[0])//if not,thet means path inputted via env.strs.
 {int l=GetTempPath(MAX_PATH*2,outDir);
  if(0==l)StringCchPrintf(outDir,4,L"C:\\");
}}




}