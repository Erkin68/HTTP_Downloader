#pragma comment(lib, "Ws2_32.lib")
#ifndef _DWNLD_CHNK__H__


#include "myHttp.h"



typedef __int16 S16;
typedef __int32 S32;
typedef unsigned __int16 U16;
typedef unsigned __int32 U32;
typedef char S8;
typedef unsigned char U8;
typedef float F32;
typedef double F64;
typedef __int64 S64;
typedef unsigned __int64 U64;


#include "DMFile.h"



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

extern FileHeader memFileHeader;


#endif//_DWNLD_CHNK__H__