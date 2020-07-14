#include "windows.h"
#include "strsafe.h"
#include "Options.h"


extern HWND hWnd;

typedef unsigned char U8;
typedef unsigned __int16 U16;
typedef __int64 S64;

__declspec(align(1)) class ChnkHeader
{public:
 static bool IncrementPartitions();
 static void WriteHeaders();

		ChnkHeader();
 void	Free();
 SOCKET CreateSocket();
 bool	FirstGet();
 int	GetSocketMaxRecvBuffer();
 bool   NextGet(bool);
 int	Read();
 bool	SocketIsReadyForRecv();
 bool	SocketIsReadyForSend();
 bool   SplitIfDeprecated();
 bool	TryConnectSocket();
 bool	Write();

struct TF//Earch time this struct(any of) will be written to the dml-file
 {S64 pos;
  S64 size;
  S64 downloaded;
  DWORD lastDownloaded;
  int state;//0-decremented(qisqarib ketgan),1-at the end,2-working.
 }f;
 SOCKET sock;
};

__declspec(align(1)) class FileHeader
{public:
		FileHeader();
	   ~FileHeader();

  bool SetURLFromCmndLine(wchar_t*);

 U8  flag,iNumParts;
 U16 URLSize;
 S64 sizeFull;
 S64 sizeDownloaded;
 S64 fromPos;
 S64 toPos;
 union
 {wchar_t *URL;
  char alignURL[8];
 };
 union
 {wchar_t *URLhostName;
  char alignURLhostName[8];//64 talikka sizeof(FileHeader)=80, shuni qo'ymasa 32talikda 64 bo'ladur;128 talikka o'zgaradur;
 };
 union
 {wchar_t *URLpath;
  char alignURLpath[8];
 };
 union
 {wchar_t *crFileName;
  char aligncrFileName[8];
 };
 ChnkHeader *headers;
};

FileHeader apndFileHeader;


FileHeader::FileHeader():URL(0),URLhostName(0),URLSize(0),URLpath(0),crFileName(0),
						 sizeFull(-1),sizeDownloaded(0),fromPos(0),toPos(0),flag(0){}
FileHeader::~FileHeader()
{/*if(URL)free(URL);*/URL=0;
 /*if(URLhostName)free(URLhostName);*/URLhostName=0;
 /*if(URLpath)free(URLpath);*/URLpath=0;
 /*if(crFileName)free(crFileName);*/crFileName=0; 
}//malloc process heap free dan keyin xato beradur;

bool CheckForDMFileMultipartContext(HANDLE,wchar_t*,int);


bool CheckForDMFile(wchar_t *pth)
{bool bMultPart=false;
 HANDLE f = CreateFile(pth,GENERIC_READ,FILE_SHARE_READ,NULL,
						   OPEN_EXISTING,						// existing file only
						   FILE_ATTRIBUTE_NORMAL,				// normal file
						   NULL);								// no attr. template
 if(INVALID_HANDLE_VALUE==f)return false;
 if(CheckForDMFileMultipartContext(f,pth,0))//type not means
  bMultPart=true;
 CloseHandle(f);
 return bMultPart;
}

bool FillDLFiles(wchar_t *initDir)
{bool r=false;wchar_t pth[512];WIN32_FIND_DATA fd;
 StringCchCopy(pth,512,initDir);StringCchCat(pth,512,L"*.*");
 HANDLE h=FindFirstFile(pth,&fd);
 if(INVALID_HANDLE_VALUE==h)return false;
 while(FindNextFile(h, &fd)!=0)
 {if('.'==fd.cFileName[0] && '.'==fd.cFileName[1] && 0==fd.cFileName[2])continue;
  wchar_t s[1024];StringCchCopy(s,1024,pth);
  int l=(int)wcslen(s);StringCchCopy(&s[l-3],1024-l-3,fd.cFileName);
  if(fd.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
  {StringCchCat(s,1024,L"\\");
   if(FillDLFiles(s))
   {r=true;
    break;
  }}else
  {r=CheckForDMFile(s);
   if(r)break;
 }}
 FindClose(h);
 return r;
}

bool CheckForDMFileMultipartContext(HANDLE f,wchar_t *fName,int type)
{DWORD readed=0;int ln;bool r=false;
 //int shdr = (int)sizeof(FileHeader);
 if(INVALID_SET_FILE_POINTER==SetFilePointer(f,-(int)sizeof(FileHeader),0,FILE_END))return false;
 if(!ReadFile(f,&apndFileHeader,sizeof(FileHeader),&readed,NULL))return false;
 if(!readed)return false;
 if(sizeof(FileHeader)!=readed)return false;
 apndFileHeader.URL=0;						  //for(int i=0;i<config::iMaxMultipart;i++)
 //apndFileHeader.headers=0;				  //for(int i=0;i<config::iMaxMultipart;i++)
 if(apndFileHeader.iNumParts>100)return false;// WriteFile(DMFile::f,&memFileHeader.headers[i].f,sizeof(ChnkHeader::TF),&writed,NULL);
 if(apndFileHeader.URLSize>512)return false;  //WriteFile(DMFile::f,memFileHeader.URL,memFileHeader.URLSize*sizeof(wchar_t),&writed,NULL);
 ln=sizeof(wchar_t)*apndFileHeader.URLSize;   //WriteFile(DMFile::f,&memFileHeader,sizeof(memFileHeader),&writed,NULL);

 S64 fullSz=sizeof(FileHeader)+ln+apndFileHeader.iNumParts*sizeof(ChnkHeader::TF);
 if(apndFileHeader.sizeFull>0)fullSz+=apndFileHeader.sizeFull;
 DWORD sz=GetFileSize(f,&readed);
 S64 sz64 = (((S64)readed) << 32) | sz;
 if(sz64!=fullSz)return false;

 if(apndFileHeader.URL)apndFileHeader.URL=(wchar_t*)realloc(apndFileHeader.URL,ln+2);
 else apndFileHeader.URL=(wchar_t*)malloc(ln+2);
 if(!apndFileHeader.URL)return false;

 if(INVALID_SET_FILE_POINTER==SetFilePointer(f,-(int)sizeof(FileHeader)-ln,0,FILE_END))goto Fail;
 readed=0;if(!ReadFile(f,&apndFileHeader.URL[0],ln,&readed,NULL))goto Fail;
 if(ln!=readed)goto Fail;
 apndFileHeader.URL[ln/2]=0;

 double pc = apndFileHeader.sizeFull>0?(100.0f*(double)apndFileHeader.sizeDownloaded/(double)apndFileHeader.sizeFull):0.0f;
 wchar_t* pt=wcsstr(fName,config::outDir);
 if(pt)
 {int l=(int)wcslen(config::outDir);
  if('\\'==fName[l-1] && !wcsrchr(&fName[l],'\\'))pt=NULL;
  else pt=fName;
 }
 else pt=fName;
 if(0==type || type==config::GetMyFileType(fName))
  InsertNewUrl(	apndFileHeader.URL,
				pt,
				(int)pc,
				apndFileHeader.sizeFull>0?(&apndFileHeader.sizeFull):NULL,
				&apndFileHeader.sizeDownloaded);
 r=true;
Fail:
 if(!r)if(apndFileHeader.URL){free(apndFileHeader.URL);apndFileHeader.URL=0;apndFileHeader.URLSize=0;}
 return r;
}

