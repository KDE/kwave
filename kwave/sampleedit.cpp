#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <kprogress.h>
#include <limits.h>

extern MSignal *clipboard; 

void MSignal::toggleChannel (int a,int b)
{
  if (a<b)
    {
      if (next) next->toggleChannel (a+1,b);
    }
  else
    {
      selected=!selected;
    }
}
//**********************************************************
void MSignal::deleteChannel (int a,int b)
{
  if (a<b)
    {
      if (next)
	{
	  if (a==b-1)
	    {
	      MSignal *todump=next;
	      next=next->getNext();
	      todump->detachChannels();
	      delete  todump;
	    }
	  else 
	    deleteChannel (a+1,b);
	}
    }
}
//**********************************************************
void MSignal::addChannel ()
{
  MSignal *ins=new MSignal (parent,length,rate,1);
  appendChannel (ins);
  emit sampleChanged();
}
//**********************************************************
void MSignal::setChannels (int num)
{
  channels=num;
}
//**********************************************************
void MSignal::appendChannel (MSignal *ins)
{
  MSignal *tmp=this;
  while (tmp->getNext()!=0) tmp=tmp->getNext();  //find last channel
  tmp->appendChanneltoThis (ins);
  channels++;
}
//**********************************************************
void MSignal::appendChanneltoThis (MSignal *ins)
{
  next=ins;
}
//**********************************************************
void MSignal::copyRange ()
{
  if (clipboard)
    {
      delete clipboard;
      clipboard=0;
    }
  MSignal *tmp=this;
  while (tmp!=0)
    {
      if (tmp->isSelected()) tmp->copyChannelRange ();
      tmp=tmp->getNext();
    }  
}
//**********************************************************
void MSignal::copyChannelRange ()
{
  MSignal *temp; 
  int	 *tsample;

  if (rmarker!=lmarker) //this function makes no sense if there is no Range is selected 
    {
      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
      temp=new MSignal (parent,rmarker-lmarker,rate);
      if (temp)
	{
	  int j=0;
	  tsample=temp->getSample();

	  for (int i=lmarker;i<rmarker;tsample[j++]=sample[i++]);

	  if (!clipboard) clipboard=temp;
	  else clipboard->appendChannel(temp);

	  emit sampleChanged();
	}
      else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
    }
}
//**********************************************************
void MSignal::pasteRange ()
{
  if (clipboard)
    {
      MSignal *clip=clipboard;
      MSignal *tmp=this;
      int all=true;

      while (tmp!=0) //check if all Channels are selected ....
	{
	  if (!tmp->isSelected()) all=false;
	  tmp=tmp->getNext();
	}  

      tmp=this;

      if (all)
	{
	  while (tmp!=0)
	    {
	      tmp->pasteChannelRange (clip);

	      clip=clip->getNext();
	      if (!clip) clip=clipboard;
	      tmp=tmp->getNext();
	    }
	}
      else
	{
	  while (tmp!=0)
	    {
	      if (tmp->isSelected())
	      tmp->overwriteChannelRange (clip);

	      clip=clip->getNext();
	      if (!clip) clip=clipboard;
	      tmp=tmp->getNext();
	    }
	}
  
      emit sampleChanged();
    }
}
//**********************************************************
void MSignal::overwriteChannelRange (MSignal *clipboard)
{
  if (lmarker<0) lmarker=0;
  if (rmarker>length) rmarker=length;
	
  int 	*clipsam	=clipboard->getSample();
  int	cliplength	=clipboard->getLength();

  if (cliplength>length-lmarker) cliplength=length-lmarker;

  memcpy (&sample[lmarker],clipsam,(cliplength)*sizeof(int));

  rmarker=lmarker+cliplength;
}
//**********************************************************
void MSignal::pasteChannelRange (MSignal *clipboard)
{
  int *newsam;

  if (lmarker<0) lmarker=0;
  if (rmarker>length) rmarker=length;
	
  int 	*clipsam	=clipboard->getSample();
  int	cliplength	=clipboard->getLength();

  newsam=new int[length+cliplength-(rmarker-lmarker)];
  if (newsam)
    {
      int r=rmarker;
      int l=lmarker;
      memcpy (newsam,sample,lmarker*sizeof(int));
      memcpy (&newsam[lmarker],clipsam,(cliplength)*sizeof(int));
      memcpy (&newsam[lmarker+cliplength],&sample[rmarker],(length-rmarker)*sizeof(int));
      delete sample;
      sample=newsam;
      length+=cliplength-(rmarker-lmarker);
      rmarker=lmarker+cliplength;

      if (cliplength-(r-l)>0)
      emit signalinserted (l,cliplength-(r-l));
      else 
      emit signaldeleted (l,-(cliplength-(r-l)));
    }
  else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
}
//**********************************************************
void MSignal::mixpasteRange ()
{
  if (clipboard)
    {
      int *newsam;
      int len;

      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
      len=rmarker-lmarker;
	
      int 	*clipsam	=clipboard->getSample();
      int	cliplength	=clipboard->getLength();

      if (len>0)
	{
	  for (int i=0;(i<len)&&(i<cliplength);i++)  // mix range with clipboard...
	    sample[lmarker+i]=(sample[lmarker+i]+clipsam[i])/2;
	  emit sampleChanged();
	}
      else
	{
	  if (lmarker<length-cliplength)
	    {
	      for (int i=0;i<cliplength;i++)  // mix clipboard to signal
		sample[lmarker+i]=(sample[lmarker+i]+clipsam[i])/2;
	      emit sampleChanged();
	    }
	  else 
	    { //not so simple, need to append ...
	      newsam=new int[cliplength+lmarker];
	      if (newsam)
		{
		  memcpy (newsam,sample,lmarker*sizeof(int));
		  for (int i=lmarker;i<length;i++)  
		    newsam[i]=(sample[i]+clipsam[i-lmarker])/2;
		  for (int i=length;i<cliplength+lmarker;i++)  
		    newsam[i]=clipsam[i-lmarker]/2;

		  delete sample;
		  sample=newsam;
		  length=cliplength+lmarker;
		  rmarker=lmarker+cliplength;
		  emit sampleChanged();
		}
	      else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
	    }
	}
    }
}
//**********************************************************
void MSignal::cutRange ()
{
 copyRange();
 deleteRange();
}
//**********************************************************
void MSignal::deleteRange ()
{
  MSignal *tmp=this;
  int all=true;

  while (tmp!=0)
    {
      if (!tmp->isSelected()) all=false;
      tmp=tmp->getNext();
    }  

  tmp=this;
  if (all)
    {
      while (tmp!=0)
	{
	  tmp->deleteChannelRange();
	  tmp=tmp->getNext();
	}  
    }
  else
    {
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->zeroChannelRange();
	  tmp=tmp->getNext();
	}  
    }
  emit sampleChanged();
}
//**********************************************************
void MSignal::deleteChannelRange ()
{
 if (rmarker!=lmarker)
   {
     int *newsam;

     if (lmarker<0) lmarker=0;
     if (rmarker>length) rmarker=length;
	
     newsam=new int[length-(rmarker-lmarker)];
     if (newsam)
       {
	 int r=rmarker;
	 int l=lmarker;
	 memcpy (newsam,sample,lmarker*sizeof(int));
	 memcpy (&newsam[lmarker],&sample[rmarker],(length-rmarker)*sizeof(int));
	 delete sample;
	 sample=newsam;
	 length=length-(rmarker-lmarker);
	 rmarker=lmarker;
	 emit signaldeleted (l,r-l);
       }
     else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
   }
}
//**********************************************************
void MSignal::cropRange ()
{
  MSignal *tmp=this;
  while (tmp!=0)
    {
      tmp->cropChannelRange ();
      tmp=tmp->getNext();
    }  
  emit sampleChanged();
}
//**********************************************************
void MSignal::cropChannelRange ()
{
  if (rmarker!=lmarker)
    {
      int *newsam;

      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
	
      newsam=new int[rmarker-lmarker];
      if (newsam)
	{
	  int r=rmarker;
	  int l=lmarker;
	  int le=length;

	  memcpy (newsam,&sample[lmarker],(rmarker-lmarker)*sizeof(int));
	  delete sample;
	  sample=newsam;
	  length=rmarker-lmarker;
	  rmarker=0;
	  lmarker=0;

	  emit signaldeleted (0,r);
	  emit signaldeleted (l,le-r);
	}
      else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
    }
}
//**********************************************************
