#include "myTime.h"
#include "Win7ProgressTaskbar.h"
#include <time.h>
#include <strsafe.h>


namespace myTime
{
bool bPaused(false);
__time64_t initTime(0),pauseTime(0),pauseStrtTime(0);


void Init()
{//_tzset();
 _time64(&initTime);
}

bool GetTimeStr(int t,wchar_t *out)
{struct tm newTime;int err=0;
 switch(t)
 {case 0:
   if(!initTime)return false;
   err = _localtime64_s(&newTime, &initTime);
   if(err) return false;
   StringCchPrintf(out,18,L"%02d.%02d.%02d %02d:%02d:%02d",
	 newTime.tm_mday,
	 1+newTime.tm_mon,
	 newTime.tm_year-100,//_wasctime(newTime)
	 newTime.tm_hour,
	 newTime.tm_min,
	 newTime.tm_sec);
  return true;
  case 1://Interrupted time:
   if(!pauseTime)return false;
   StringCchPrintf(out,18,L"%02d:%02d:%02d",pauseTime/3600,(pauseTime%3600)/60,pauseTime%60);
  return true;
  case 2://Working time:
   if(!initTime)return false;
   __time64_t t,ps;_time64(&t);
   if(bPaused)
   {ps = pauseTime + (t-pauseStrtTime);
    ps = (t-initTime) - ps;
   }else
     ps = (t-initTime) - pauseTime;
   StringCchPrintf(out,18,L"%02d:%02d:%02d",ps/3600,(ps%3600)/60,ps%60);
  return true;
  case 3://Prognoze to finishing:
   if(!initTime)return false;
   _time64(&t);
   if(bPaused)
   {ps = pauseTime + (t-pauseStrtTime);
    ps = (t-initTime) - ps;
   }else
     ps = (t-initTime) - pauseTime;
   if(!Win7PrgrsTaskbar::perc)return false;
   ps = (__time64_t)(((double)ps * (100-Win7PrgrsTaskbar::perc))/(double)Win7PrgrsTaskbar::perc);
   if(ps/3600>0)
    StringCchPrintf(out,18,L"%02d:%02d:%02d",ps/3600,(ps%3600)/60,ps%60);
   else if((ps%3600)/60>0)
    StringCchPrintf(out,18,L"%02d:%02d",(ps%3600)/60,ps%60);
   else// if(ps%60>0)
    StringCchPrintf(out,18,L"%02d s.    ",ps%60);
  return true;
 }
 return false;
}

void Pause(bool bPaused)
{if(bPaused)
 {_time64(&pauseStrtTime);
  bPaused = true;
 }else
 {__time64_t t;
  _time64(&t);
  pauseTime += t-pauseStrtTime;
  bPaused = false;
}}



}//end of namespace;