#include <unistd.h>
#include <kmsgbox.h>
#include <kprogress.h>
#include <limits.h>
#include "kwavesignal.h"
#include <stdio.h>
#include <stdlib.h>

//**********************************************************
void KwaveSignal::noMemory ()
{
  KMsgBox::message (0,"Info","Not enough Memory for Operation !",2);
}
//**********************************************************
KwaveSignal *KwaveSignal::copyRange ()
{
  if (rmarker!=lmarker) //this function makes no sense if no Range is selected
    {
      int len=rmarker-lmarker+1;
      int *newsam=new int [len];

      if (newsam)
	{
	  memmove(newsam,sample+lmarker,len*sizeof(int));
	  return new KwaveSignal (newsam,len,rate);
	}
      else noMemory ();
    }
  return 0;
}
//**********************************************************
// shouldn't this be "insert()" instead of "insertPaste()" ?
void KwaveSignal::insertPaste (KwaveSignal *signal)
{
  int insertlength=signal->getLength ();
  int *insert=signal->getSample();

  if (insert && insertlength)
    {
      int newlength=length+insertlength;
      int *newsam = (int *)realloc(sample, newlength*sizeof(int));
      if (newsam)
	{
	  memmove (newsam+lmarker+insertlength,newsam+lmarker,
	    (length-lmarker)*sizeof(int));
	  memmove (newsam+lmarker,insert,insertlength*sizeof(int));

	  sample=newsam;
	  length=newlength;
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
  int marked = (lmarker!=rmarker) ? rmarker-lmarker+1 : length;
  if (pastelength>marked) pastelength=marked;
  if (lmarker+pastelength > length) pastelength=length-lmarker;

  memmove (sample+lmarker,paste,pastelength*sizeof(int));
}
//**********************************************************
void KwaveSignal::mixPaste (KwaveSignal *signal)
{
  int pastelength=signal->getLength ();
  int *paste=signal->getSample();
  int marked = (lmarker!=rmarker) ? rmarker-lmarker+1 : length;
  if (pastelength>marked) pastelength=marked;
  if (lmarker+pastelength > length) pastelength=length-lmarker;

  int *sample=this->sample+lmarker;
  while (pastelength--)
    {
      *sample=(*paste+*sample)>>1;
      sample++;
      paste++;
    }
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
  // example:
  // sample = [01(23)45678], length = 9
  // lmarker = 2, rmarker = 3
  // -> selected = 3-2+1 = 2
  // -> rest = 9-2 = 7
  // -> left = [0..1] (length=2)
  // -> right = [4..8] (length=9-3-1=5)
  int selected = rmarker-lmarker+1; // selected samples
  int rest = length - selected;     // rest after delete

  if (rest && length && sample)
    {
      if (rmarker < length-1)
	{
	  // copy data after the right marker
	  memmove(sample+lmarker, sample+rmarker+1,
	    (length-rmarker-1)*sizeof(int));
	}
      int *newsam = (int *)realloc(sample, rest*sizeof(int));

      if (newsam != 0)
 	{
	    sample = newsam;
	    length=rest;
	}
      else
	{
	  // oops, not enough memory !
	  noMemory();
	}
    }
  else
    {
      // delete everything
      if (sample != 0) delete []sample;
      sample = 0;
      length = 0;
    }

  // correct the right marker	
  setMarkers(lmarker-1, lmarker-1);
}
//**********************************************************
void KwaveSignal::cropRange ()
{
  if (rmarker!=lmarker)
    {
      int *newsam;
      int newlength=rmarker-lmarker+1;

      memmove(sample, sample+lmarker, newlength*sizeof(int));
      length=newlength;

      newsam = (int *)realloc(sample, newlength*sizeof(int));
      if (newsam != 0) sample=newsam;
      // NOTE: if the realloc failed, the old memory
      // will remain allocated and only length will be
      // reduced - so the user won't see that.
      // The (dead) memory will be freed on the next operation
      // that calls "delete sample".
    }

    // correct the markers
    setMarkers(lmarker, rmarker);
}
//**********************************************************

