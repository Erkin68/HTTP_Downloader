#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

//#include "Winsock2.h"
#include "DownloadChunk.h"
#include "strsafe.h"
#include "config.h"

extern bool MessagePump();
extern BOOL MyU64ToA(char*,int,unsigned __int64,int*);


ChnkHeader::ChnkHeader():sock(INVALID_SOCKET)
{f.pos=0;
 f.state=0;
 f.downloaded=0;
 f.lastDownloaded=0;
}

void ChnkHeader::Free()
{f.state=0;
 if(INVALID_SOCKET!=sock)closesocket(sock);sock=INVALID_SOCKET; 
}

int ChnkHeader::Read()
{if(!SocketIsReadyForRecv())return 0;
 S64 unldSz = f.size - f.downloaded;
 int ret = myHttp::iBufferSize;
 if(unldSz < (S64)myHttp::iBufferSize)
  ret = (int)unldSz;
 ret = recv(sock, myHttp::buf, ret, 0);
 if(ret == -1)
 {if(WSAEWOULDBLOCK==WSAGetLastError())//would block
   return -1;//normally
  else return -2;//fatal;
  if(ret == 0)//the connection was shut down
   return 0;
 }
 f.lastDownloaded = ret;
 f.downloaded+=f.lastDownloaded;
 if(0==f.state)f.state=2;//working;
 return ret;
}

bool ChnkHeader::Write()
{if(f.lastDownloaded)
 {DWORD writed;S64 crntPos = f.pos+f.downloaded-f.lastDownloaded;
  LONG dwPos[2]={crntPos & 0xffffffff, crntPos>>32};
  writed=SetFilePointer(DMFile::f,dwPos[0],&dwPos[1],FILE_BEGIN);
  WriteFile(DMFile::f,myHttp::buf,f.lastDownloaded,&writed,NULL);//downloaded size kerak emas;
#ifdef _DEBUG
  //if(this==&memFileHeader.headers[0]){
  char s[128];sprintf(s,"\nWrite: %d, pos: %d wr: %d (%d) , to %d",(int)(this-&memFileHeader.headers[0]),dwPos[0],f.lastDownloaded,writed,dwPos[0]+f.lastDownloaded);
  OutputDebugStringA(s);
  sprintf(s," downloaded: %d, lastdownloaded %d",f.downloaded,f.lastDownloaded);
  OutputDebugStringA(s);//}
#endif
 }return true;
 return false;
}

SOCKET ChnkHeader::CreateSocket()
{sock = socket(AF_INET, SOCK_STREAM, 0);
 if(INVALID_SOCKET!=sock)
 {u_long status = 1;
  if(SOCKET_ERROR==ioctlsocket(sock, FIONBIO, &status))//put the socket in non-blocking mode.
  {closesocket(sock);
   sock= INVALID_SOCKET;
 }}
 return sock;
}

bool ChnkHeader::FirstGet()
{int ln = sprintf(myHttp::buf,"GET ");
 ln += WideCharToMultiByte(CP_ACP,0,memFileHeader.URLpath,-1,&myHttp::buf[ln],1023-ln,NULL,NULL)-1;
 ln += sprintf(&myHttp::buf[ln],config::protocolStr);//" HTTP/1.0"
 ln += sprintf(&myHttp::buf[ln],"\r\nHost: ");
 ln += WideCharToMultiByte(CP_ACP,0,memFileHeader.URLhostName,-1,&myHttp::buf[ln],1023-ln,NULL,NULL)-1;
 ln += sprintf(&myHttp::buf[ln],"\r\nAccept: */*");
 ln += sprintf(&myHttp::buf[ln],"\r\nUser-Agent: %s",config::userAgentStr);
 ln += sprintf(&myHttp::buf[ln],"\r\nReferer: ");
 int L = (int)wcslen(memFileHeader.URLhostName);
 int G = (int)(wcsstr(&memFileHeader.URL[0],memFileHeader.URLhostName) - &memFileHeader.URL[0]);
 ln += WideCharToMultiByte(CP_ACP,0,memFileHeader.URL,L+G,&myHttp::buf[ln],1023-ln,NULL,NULL);
 ln += sprintf(&myHttp::buf[ln],"\r\n\r\n");
 myHttp::buf[ln]=0;
 for(;!SocketIsReadyForSend();){MessagePump();}
 L = send(sock,myHttp::buf,(int)strlen(myHttp::buf),0);
 if(-1==L)
 {int err = WSAGetLastError();//10057
  if(WSAENOTCONN==err)
  {}return false;
 }
 for(;!SocketIsReadyForRecv();){MessagePump();}
 int bytes = recv(sock, myHttp::buf, myHttp::iBufferSize, 0);
 if(bytes<1)return false;

#ifdef _DEBUG
 OutputDebugStringA(myHttp::buf);
#endif

 char *pErr=strchr(myHttp::buf,' ');
 if(pErr)
 {int iErr=atoi(pErr+1);
  if(iErr)
  {switch(iErr)
   {case 200://Service Temporarily
     //if(strstr(pErr+4,"OK")
    break;//Normally
    case 503://Service Temporarily
    return false;
  }}
  else return false;
 }

 char *p=strstr(myHttp::buf,"Content-Range:");//Content-Range: bytes 88080384-160993791/160993792
 if(p)
 {p = strchr(p,'/');
  if(p)
  {myHttp::bGlobalSizeContext=true;
   memFileHeader.sizeFull = _atoi64(p+1);//p + 17
 }}
 if(strstr(myHttp::buf,"Accept-Ranges:"))
  myHttp::bPartialContext=true;
 if(!myHttp::bGlobalSizeContext)
 {p=strstr(myHttp::buf,"Content-Length:");//"Content-Length: 72913408
  if(p)
  {myHttp::bGlobalSizeContext=true;
   memFileHeader.sizeFull = _atoi64(p+16);
 }}

 p=strstr(myHttp::buf,"Server:");
 if(p)
 {char *pp = strchr(p,'\r');
  if(pp)
  {int l = (int)(pp-p);memcpy(myHttp::serverStr,p,l);
   StringCchPrintfA(&myHttp::serverStr[l],512-l," {%d.%d.%d.%d}",myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b1,//myHttp::serverStr[l]=0;
																 myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b2,
																 myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b3,
																 myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b4);
   //myHttp::SendToExtMaster(0);
 }}

 int headerSz = bytes;
 char *pb = strstr(myHttp::buf,"\r\n\r\n");
 if((!pb) && bytes<myHttp::iBufferSize-4)
  pb=&myHttp::buf[bytes];
 else
 {headerSz = (int)(pb-&myHttp::buf[0])+4;
  if(pb && headerSz<=bytes)
   pb+=4;
 }//else return false;

 char *fn = strstr(myHttp::buf,"filename=");
 if(fn)
 {fn+=9;
  char *pfn=strstr(fn,"\r\n");
  if(pfn)
  {int nl=(int)(pfn-fn);
   if('"'==*fn)
    {++fn;--nl;}
   if('"'==(*(fn+nl-1)))
	--nl;
   /*int err = */MultiByteToWideChar(CP_ACP,0,fn,nl,DMFile::foundedFileName,512);//memcpy(DMFile::foundedFileName,fn,nl);
   //if(!err)err=GetLastError();
   DMFile::foundedFileName[nl]=0;
 }}

 DWORD writed;int fileFrstChnkSz = bytes-headerSz; 
 //if(0==memFileHeader.fromPos)
 {SetFilePointer(DMFile::f,0,0,FILE_BEGIN);
  if(fileFrstChnkSz>0)
  {WriteFile(DMFile::f,pb,fileFrstChnkSz,&writed,NULL);
   memFileHeader.sizeDownloaded = fileFrstChnkSz;
#ifdef _DEBUG
  //if(this==&memFileHeader.headers[0]){
  char s[128];sprintf(s,"\nPos: 0 wr: %d (%d)",fileFrstChnkSz,writed);
  OutputDebugStringA(s);//}
#endif
  }}/*else if(memFileHeader.fromPos<fileFrstChnkSz)
 {__int64 offst = (__int64)fileFrstChnkSz-memFileHeader.fromPos;
  WriteFile(DMFile::f,pb+offst,fileFrstChnkSz-offst,&writed,NULL);
  memFileHeader.sizeDownloaded = fileFrstChnkSz-offst;
 }*/
 f.size = memFileHeader.sizeFull;
 f.downloaded = fileFrstChnkSz;
 if(fileFrstChnkSz<memFileHeader.sizeFull)
 {f.state = 2;
  ++myHttp::iWorkingParts;
 }
 return true;
}

int ChnkHeader::GetSocketMaxRecvBuffer()
{int optVal=0;
 int optLen = sizeof(int);
 if(getsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&optVal,&optLen)!=SOCKET_ERROR && optVal>1024)
  return optVal;
 return MAX_RECV_BUF_SZ;
}

bool ChnkHeader::IncrementPartitions()
{//First we must find first unworking part:
 int iNewPart=-1;
 for(int i=0; i<config::iMaxMultipart; i++)
 {if(0==memFileHeader.headers[i].f.state)
  {iNewPart=i;
   break;
 }}if(-1==iNewPart)return false;
 
 //Second we must find max-size working part:
 S64 maxSz=0;int iFromPart=-1;
 for(int i=0; i<config::iMaxMultipart; i++)
 {if(memFileHeader.headers[i].f.state>1)
  {S64 unld = memFileHeader.headers[i].f.size-memFileHeader.headers[i].f.downloaded;
   if(unld>config::iMinSizeForDividing)
   {if(maxSz<unld)
    {maxSz=unld;
     iFromPart=i;
 }}}}if(-1==iFromPart)return false;
 
 //Then try to create socket:
 if(INVALID_SOCKET==memFileHeader.headers[iNewPart].CreateSocket())return false;
 if(!memFileHeader.headers[iNewPart].TryConnectSocket())
 {memFileHeader.headers[iNewPart].Free();
  myHttp::iMaxConnectServerAllowed=myHttp::iWorkingParts;
  return false;
 }

 //Fill params for new partition:
 maxSz /= 2;
 memFileHeader.headers[iNewPart].f.pos =  memFileHeader.headers[iFromPart].f.pos +
										  memFileHeader.headers[iFromPart].f.downloaded +
										  maxSz;
 memFileHeader.headers[iNewPart].f.size = memFileHeader.headers[iFromPart].f.size -
										  memFileHeader.headers[iFromPart].f.downloaded -
										  maxSz;
 memFileHeader.headers[iNewPart].f.state = 2;
 memFileHeader.headers[iNewPart].f.downloaded = 0;
 memFileHeader.headers[iNewPart].f.lastDownloaded = 0;
 if(!memFileHeader.headers[iNewPart].NextGet(false))
 {memFileHeader.headers[iNewPart].f.state = 0;
  memFileHeader.headers[iNewPart].f.size = 0;
  memFileHeader.headers[iNewPart].f.pos = 0;
  closesocket(memFileHeader.headers[iNewPart].sock);
  memFileHeader.headers[iNewPart].sock=INVALID_SOCKET;
  return false;
 }

 memFileHeader.headers[iFromPart].f.size = memFileHeader.headers[iNewPart].f.pos -
										   memFileHeader.headers[iFromPart].f.pos;
 ++myHttp::iWorkingParts;

#ifdef _DEBUG
 char s[128];sprintf(s,"\nNew part: %d div.fr. %d, pos: %d , sz: %d",iNewPart,iFromPart,
				(int)memFileHeader.headers[iNewPart].f.pos,(int)memFileHeader.headers[iNewPart].f.size);
 OutputDebugStringA(s);
 sprintf(s,"\n   Div.part. pos: %d , sz: %d",(int)memFileHeader.headers[iFromPart].f.pos,(int)memFileHeader.headers[iFromPart].f.size);
 OutputDebugStringA(s);
#endif
 return true;
}

bool ChnkHeader::NextGet(bool bAppend)
{int ln = sprintf(myHttp::buf,"GET ");//Server must response 206 with "Content-Range",if ret 200 so this is full range;
 ln += WideCharToMultiByte(CP_ACP,0,memFileHeader.URLpath,-1,&myHttp::buf[ln],1023-ln,NULL,NULL)-1;
 ln += sprintf(&myHttp::buf[ln],config::protocolStr);//" HTTP/1.0"
 ln += sprintf(&myHttp::buf[ln],"\r\nHost: ");
 ln += WideCharToMultiByte(CP_ACP,0,memFileHeader.URLhostName,-1,&myHttp::buf[ln],1023-ln,NULL,NULL)-1;
 ln += sprintf(&myHttp::buf[ln],"\r\nAccept: */*");
 ln += sprintf(&myHttp::buf[ln],"\r\nUser-Agent: %s",config::userAgentStr);
 ln += sprintf(&myHttp::buf[ln],"\r\nRange: bytes=");
 int l;MyU64ToA(&myHttp::buf[ln],1024,
				bAppend?(f.pos+f.downloaded):f.pos,
				&l);ln += l;
 ln += sprintf(&myHttp::buf[ln],"-");
 MyU64ToA(&myHttp::buf[ln],1024,f.pos+f.size-1,&l);ln += l;
 ln += sprintf(&myHttp::buf[ln],"\r\nReferer: ");
 int L = (int)wcslen(memFileHeader.URLhostName);
 int G = (int)(wcsstr(&memFileHeader.URL[0],memFileHeader.URLhostName) - &memFileHeader.URL[0]);
 ln += WideCharToMultiByte(CP_ACP,0,memFileHeader.URL,L+G,&myHttp::buf[ln],1023-ln,NULL,NULL);
 ln += sprintf(&myHttp::buf[ln],"\r\n\r\n");
 myHttp::buf[ln]=0;
 for(;!SocketIsReadyForSend();){MessagePump();}
 ln = send(sock,myHttp::buf,(int)strlen(myHttp::buf),0);
 if(-1==ln)
 {int err = WSAGetLastError();//10057
  closesocket(sock);
  return false;
 }

/*#ifdef _DEBUG
 OutputDebugStringA(myHttp::buf);
#endif*/

 for(;!SocketIsReadyForRecv();){MessagePump();}
 int bytes = recv(sock, myHttp::buf, myHttp::iBufferSize, 0);
 if(bytes<1)
 {closesocket(sock);
  return false;
 }

 char *pErr=strchr(myHttp::buf,' ');
 if(pErr)
 {int iErr=atoi(pErr+1);
  if(iErr)
  {switch(iErr)
   {case 206://Service Temporarily
     //if(strstr(pErr+4,"OK")
    break;//Normally
    case 503://Service Temporarily
    return false;
  }}
  else return false;
 }

 char *p200=strstr(pErr," 206 Partial Content");
 char *pcontrng=strstr(myHttp::buf,"Content-Range: bytes ");
 if((!p200)&&(!pcontrng))
 {closesocket(sock);
  return false;
 }char *p_ = strchr(pcontrng+20,'-');
 if(!p_)
 {closesocket(sock);
  return false;
 }//*p_=0; ummuman mumkin emas, pastdagi pb ololmaydur;
 S64 rtFrom = _atoi64(pcontrng+20);
 S64 rtTo= _atoi64(p_+1);
 if((bAppend?(f.pos+f.downloaded):f.pos)!=rtFrom || f.pos+f.size-1!=rtTo)
 {closesocket(sock);
  return false;
 }

 char* p=strstr(myHttp::buf,"Server:");
 if(p)
 {char *pp = strchr(p,'\r');
  if(pp)
  {int l = (int)(pp-p);memcpy(myHttp::serverStr,p,l);
   StringCchPrintfA(&myHttp::serverStr[l],512-l," {%d.%d.%d.%d}",myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b1,//myHttp::serverStr[l]=0;
																 myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b2,
																 myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b3,
																 myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b4);
   //myHttp::SendToExtMaster(0);
 }}

 char *fn = strstr(myHttp::buf,"filename=");
 if(fn)
 {fn+=9;
  char *pfn=strstr(fn,"\r\n");
  if(pfn)
  {int nl=(int)(pfn-fn);
   if('"'==*fn)
    {++fn;--nl;}
   if('"'==(*(fn+nl-1)))
	--nl;
   /*int err = */MultiByteToWideChar(CP_ACP,0,fn,nl,DMFile::foundedFileName,512);//memcpy(DMFile::foundedFileName,fn,nl);
   //if(!err)err=GetLastError();
   DMFile::foundedFileName[nl]=0;
 }}

 char *pb = strstr(myHttp::buf,"\r\n\r\n");
 int headerSz = (int)(pb-&myHttp::buf[0])+4;
 if(pb && headerSz<=bytes)
  pb+=4;
 else
 {closesocket(sock);
  return false;
 }
 DWORD writed;int fileFrstChnkSz = bytes-headerSz;
 if(0!=fileFrstChnkSz)
 {LONG lng[2]={(bAppend?(f.downloaded+f.pos):f.pos) & 0xffffffff,(bAppend?(f.downloaded+f.pos):f.pos) >> 32};
  SetFilePointer(DMFile::f,lng[0],&lng[1],FILE_BEGIN);
  WriteFile(DMFile::f,pb,fileFrstChnkSz,&writed,NULL);
  memFileHeader.sizeDownloaded += fileFrstChnkSz;
  f.downloaded += fileFrstChnkSz;
 }//else if(memFileHeader.fromPos<fileFrstChnkSz)
/*#ifdef _DEBUG
  //if(this==&memFileHeader.headers[0]){
 char s[128];sprintf(s,"\nNext get chunk: %d, pos: %d crPos: %d to %d dwnldd: %d lastDwnld: %d",
						(int)(this-&memFileHeader.headers[0]),
						f.pos,
						bAppend?(f.pos+f.downloaded):f.pos,
						f.pos+f.size,
						f.downloaded,
						f.lastDownloaded);
  OutputDebugStringA(s);//}
#endif*/

 return true;
}

bool ChnkHeader::SocketIsReadyForRecv()
{timeval tv;fd_set fds;/* Wait for data. */ 
 tv.tv_sec = config::iMaxRecvTimeout;
 tv.tv_usec = 0;
 FD_ZERO(&fds);
 FD_SET(sock, &fds);//       readable  writable
 int ret = select((int)sock + 1, &fds, NULL, NULL, &tv);
 if(!ret)
  return false;//die("Timeout exceeded");
 else if(ret == -1)
  return false;//die("select() #2");
 return true;
}

bool ChnkHeader::SocketIsReadyForSend()
{timeval tv;fd_set fds;/* Wait for data. */ 
 tv.tv_sec = config::iMaxSendTimeout;
 tv.tv_usec = 0;
 FD_ZERO(&fds);
 FD_SET(sock, &fds);//       readable  writable
 int ret = select((int)sock + 1, NULL, &fds, NULL, &tv);
 if(!ret)
  return false;//die("Timeout exceeded");
 else if(ret == -1)
  return false;//die("select() #2");
 return true;
}

bool ChnkHeader::SplitIfDeprecated()//Tugagan,lekin keyingisi bilan birlashtirib qo'yish:
{if(f.downloaded<f.size)return false;
 S64 finPos = f.pos + f.size;
 if(finPos==memFileHeader.sizeFull)
 {closesocket(sock);
  sock=INVALID_SOCKET;
  f.lastDownloaded=0;
  f.state=1;//If part on the tail(end of file):
  --myHttp::iWorkingParts;
  return false;
 }//else:
 int iFindSplit=-1;
 for(int i=0; i<config::iMaxMultipart; i++)
 {if(this==&memFileHeader.headers[i])continue;
  if(memFileHeader.headers[i].f.state>0)
  {if(memFileHeader.headers[i].f.pos==finPos)
   {iFindSplit=i;
    break;
 }}}if(-1==iFindSplit)return false;//fatal;
 memFileHeader.headers[iFindSplit].f.pos=f.pos;
 memFileHeader.headers[iFindSplit].f.size+=f.size;
 memFileHeader.headers[iFindSplit].f.downloaded+=f.size;
 closesocket(sock);
 sock=INVALID_SOCKET;
 f.lastDownloaded=0;
 f.downloaded=0;
 f.size=0;
 f.state=0;
 --myHttp::iWorkingParts;
#ifdef _DEBUG
 char s[128];sprintf(s,"\n Split part %d to part: %d , ",(int)(this-&memFileHeader.headers[0]),iFindSplit);
 OutputDebugStringA(s);
#endif
 return true;
}

bool ChnkHeader::TryConnectSocket()
{/*myHttp::httpServerAddr.sin_family = AF_INET;
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b1 = 81;
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b2 = 95;
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b3 = 225;
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b4 = 134;
  myHttp::httpServerAddr.sin_port = htons(80);*/
 if(!myHttp::httpServerAddr.sin_port)//if(0==par->iThread)
 {struct hostent *hp=0;
  char hostAddr[260];if(!WideCharToMultiByte(CP_ACP,0,memFileHeader.URLhostName,-1,hostAddr,260,NULL,NULL))return false;
  hp = gethostbyname(hostAddr);
  if(!hp)return false;
  myHttp::httpServerAddr.sin_family = AF_INET;
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr[0];
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr[1];
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr[2];
  myHttp::httpServerAddr.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr[3];
  myHttp::httpServerAddr.sin_port = htons(80);
  for(int i=0; i<8; i++)myHttp::httpServerAddr.sin_zero[i]=0;
 }
 int iConnect = connect(sock, (sockaddr*)&myHttp::httpServerAddr, sizeof(myHttp::httpServerAddr));
 if(SOCKET_ERROR==iConnect)//-1==iConnect)
 {int iErr=WSAGetLastError();
  if(WSAEWOULDBLOCK!=iErr)//if(errno != 10036)//EINPROGRESS
   return false;
 }

 //Waiting:
 timeval tv;fd_set fds;
 tv.tv_sec = config::iMaxConnectTimeout;
 tv.tv_usec = 0;
 FD_ZERO(&fds);
 FD_SET(sock, &fds);
 int ret = select((int)sock+1, NULL, &fds, NULL, &tv);
 if(!ret)
  return false;//die("Timeout exceeded");
 else if(ret == -1)
  return false;//die("select() #1");
 int len = sizeof(ret);
 if(getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&ret, &len) == -1)
  return false;//die("getsockopt()");
 if(ret)
  return false;;//die("Can't connect to server.");

 return true;
}

void ChnkHeader::WriteHeaders()
{DWORD writed;LONG l[2]={memFileHeader.sizeFull & 0xffffffff,memFileHeader.sizeFull>>32};
 SetFilePointer(DMFile::f,l[0],&l[1],FILE_BEGIN);
 for(int i=0;i<config::iMaxMultipart;i++)
  WriteFile(DMFile::f,&memFileHeader.headers[i].f,sizeof(ChnkHeader::TF),&writed,NULL);
 WriteFile(DMFile::f,memFileHeader.URL,memFileHeader.URLSize*sizeof(wchar_t),&writed,NULL);
 WriteFile(DMFile::f,&memFileHeader,sizeof(memFileHeader),&writed,NULL);
}
