#include <unistd.h>
#include <kmsgbox.h>
#include <kprogress.h>
#include <limits.h>
#include "kwavesignal.h"

//**********************************************************
void KwaveSignal::noMemory ()
{
  KMsgBox::message (0,"Info","Not enough Memory for Operation !",2);
}
//**********************************************************
KwaveSignal *KwaveSignal::copyRange ()
{
  if (rmarker!=lmarker) //this function makes no sense if there is no Range is selected 
    {
      int len=rmarker-lmarker;
      int *newsam=new int [len];

      if (newsam)
	{
	  memcpy (newsam,&sample[lmarker],len*sizeof(int));
	  return new KwaveSignal (newsam,len,rate);
	}
      else noMemory ();
    }
  return 0;
}
//**********************************************************
void KwaveSignal::insertPaste (KwaveSignal *signal)
{
  int pastelength=signal->getLength ();
  int *paste=signal->getSample();

  if (paste)
    {
      int *newsam=new int [length+pastelength];
      if (newsam)
	{
	  memcpy (newsam,sample,lmarker*sizeof(int));
	  memcpy (&newsam[lmarker],paste,pastelength*sizeof(int));
	  memcpy (&newsam[pastelength+lmarker],&sample[rmarker],(length-rmarker)*sizeof(int));

	  delete []sample;
	  sample=newsam;
	  length=length+pastelength;
	}
      else noMemory ();
    }
  else KMsgBox::message (0,"Info","signal is empty !",2);
}
//**********************************************************
void KwaveSignal::overwritePaste (KwaveSignal *signal)
{
  int pastelength=signal->getLength ();
  int *paste=signal->getSample();
  int len=rmarker-lmarker;
  if (len==0) len=length-rmarker;
  if (len>pastelength) len=pastelength;

  memcpy (&sample[lmarker],paste,len*sizeof(int));
}
//**********************************************************
void KwaveSignal::mixPaste (KwaveSignal *signal)
{
  int pastelength=signal->getLength ();
  int *paste=signal->getSample();
  int len=rmarker-lmarker;
  if (len==0) len=length-rmarker;
  if (len>pastelength) len=pastelength;

  for (int i=0;i<len;i++) sample[i+lmarker]=(sample[i+lmarker]+paste[i])/2;
}
//**********************************************************
KwaveSignal *KwaveSignal::cutRange ()
//fine example for reusal of code, my prof would say, don't show him the rest
// of my code
{
  KwaveSignal *tmp=copyRange ();
  if (tmp) deleteRange ();
  return tmp;
}
//**********************************************************
void KwaveSignal::deleteRange ()
{
  int *newsam=new int [length-(rmarker-lmarker)];
  if (newsam)
    {
      memcpy (newsam,sample,lmarker*sizeof(int));
      memcpy (&newsam[lmarker],&sample[rmarker],(length-rmarker)*sizeof(int));

      delete []sample;
      sample=newsam;
      length-=(rmarker-lmarker);
    }
  else noMemory ();
}
//**********************************************************
void KwaveSignal::cropRange ()
{
  if (rmarker!=lmarker)
    {
      int *newsam;

      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
	
      newsam=getNewMem(rmarker-lmarker);
      if (newsam)
	{
	  memcpy (newsam,&sample[lmarker],(rmarker-lmarker)*sizeof(int));
	  getridof (sample);
	  sample=newsam;
	  length=rmarker-lmarker;
	  rmarker=0;
	  lmarker=0;
	}
      else noMemory ();
    }
}
//**********************************************************
