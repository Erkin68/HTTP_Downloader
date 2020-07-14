#ifndef _CONFIG_H_
#define _CONFIG_H_




namespace config
{

//typedef struct TBools
//{TBools(int i){bConstMultipart=i;}
// __int16 bConstMultipart:1;
//}Bools;
//extern Bools bools;
 extern int iMaxMultipart,iMinMultipart,iRecBufSize,iMaxConnectTimeout,iMaxRecvTimeout,iMaxSendTimeout,iMinSizeForDividing;

 extern char userAgentStr[128];
 extern char protocolStr[32];
 extern wchar_t outDir[MAX_PATH*2];
 extern void Read();
 extern void Save();
 extern void SetDefault();
}


#endif
