#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <kprogress.h>
#include <limits.h>
#include "clipboard.h"
#include "dialogs.h"

extern ClipBoard *clipboard; 
extern int **summonChannels (MSignal *tmp,int channels);
//**********************************************************
void mixChannels (int **sigs,int *mixto,double *amps)
{
  
}
//**********************************************************
void MSignal::mix ()
{
  if (channels>1)
    {
      ChannelMixDialog *dialog=new ChannelMixDialog (parent,channels);
      if (dialog)
	{
	  if (dialog->exec())
	    {
	      int    channel=dialog->getChannel();
	      double *amps=dialog->getAmpl ();
	      int    *mixto=0;
	      int    **sigs=summonChannels ();

	      if (sigs&&amps)
		{
		  if (channel) mixto=sigs[channel-1];
		  else 
		    {
		      MSignal *ins=new MSignal (parent,length,rate,1);
		      this->appendChannel (ins);
		      mixto=ins->getSample();
		    }
		  if (mixto) mixChannels (sigs,mixto,amps);

		  delete [] sigs;
		}
	      info ();
	    }
	  delete dialog;
	}
    }
  else KMsgBox::message (parent,"Info","Mixing makes no sense with less than two channels !",2);
}
//**********************************************************
//toggles selection for channel # b with recursive calls
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
//deletes channel # b with recursive calls until channel # b-1
//this one detaches channel #b from the chain, and calls its destructor 
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
  this->appendChannel (ins);
  info ();
}
//**********************************************************
//sets number of channels, only used if first channel is deleted...
void MSignal::setChannels (int num)
{
  channels=num;
}
//**********************************************************
//finds last channel and appends this new signal to it
void MSignal::appendChannel (MSignal *ins)
{
  MSignal *tmp=this;
  while (tmp->getNext()) tmp=tmp->getNext();  //find last channel
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
  if (rmarker!=lmarker) //this function makes no sense if there is no Range is selected 
    {
      if (clipboard) //kill current clipboard if it exists...
	{
	  delete clipboard;
	  clipboard=0;
	}
      MSignal *tmp=this;
      while (tmp)
	{
	  if (tmp->isSelected()) tmp->copyChannelRange ();
	  tmp=tmp->getNext();
	}  
    }
}
//**********************************************************
void MSignal::copyChannelRange ()
{
  MSignal *temp; 
  int	 *tsample;

  if (lmarker<0) lmarker=0;
  if (rmarker>length) rmarker=length;
  temp=new MSignal (parent,rmarker-lmarker,rate);
  if (temp)
    {
      //int j=0;
      tsample=temp->getSample();

      //for (int i=lmarker;i<rmarker;tsample[j++]=sample[i++]);
      //memcopy should be more optimized
      memcpy (tsample,&sample[lmarker],sizeof(int)*(rmarker-lmarker));

      if (!clipboard) clipboard=new ClipBoard (temp);
      else clipboard->appendChannel(temp);

      info ();
    }
  else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
}
//**********************************************************
void MSignal::pasteRange ()
{
  if (clipboard)
    {
      MSignal *clip=clipboard->getSignal();
      MSignal *tmp=this;
      int all=true;

      while (tmp) //check if all Channels are selected ....
	{
	  if (!tmp->isSelected()) all=false;
	  tmp=tmp->getNext();
	}  

      tmp=this;

      if (all)
	{
	  while (tmp)
	    {
	      tmp->pasteChannelRange (clip);

	      clip=clip->getNext();
	      if (!clip) clip=clipboard->getSignal();
	      tmp=tmp->getNext();
	    }
	}
      else
	{
	  while (tmp)
	    {
	      if (tmp->isSelected())
	      tmp->overwriteChannelRange (clip);

	      clip=clip->getNext();
	      if (!clip) clip=clipboard->getSignal();
	      tmp=tmp->getNext();
	    }
	}
      info ();
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

  newsam=getNewMem(length+cliplength-(rmarker-lmarker));
  if (newsam)
    {
      int r=rmarker;
      int l=lmarker;
      memcpy (newsam,sample,lmarker*sizeof(int));
      memcpy (&newsam[lmarker],clipsam,(cliplength)*sizeof(int));
      memcpy (&newsam[lmarker+cliplength],&sample[rmarker],(length-rmarker)*sizeof(int));
      getridof (sample);
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
      int len=rmarker-lmarker;
      int *newsam;

      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
      len=rmarker-lmarker;
	
      int *clipsam	=(clipboard->getSignal())->getSample();
      int  cliplength	=clipboard->getLength();

      if (len>0)
	{
	  for (int i=0;(i<len)&&(i<cliplength);i++)  // mix range with clipboard...
	    sample[lmarker+i]=(sample[lmarker+i]+clipsam[i])/2;
	  info ();
	}
      else
	{
	  if (lmarker<length-cliplength)
	    {
	      for (int i=0;i<cliplength;i++)  // mix clipboard to signal
		sample[lmarker+i]=(sample[lmarker+i]+clipsam[i])/2;
	      info ();
	    }
	  else 
	    { //not that simple, need to append ...
	      newsam=getNewMem(cliplength+lmarker);
	      if (newsam)
		{
		  memcpy (newsam,sample,lmarker*sizeof(int));
		  for (int i=lmarker;i<length;i++)  
		    newsam[i]=(sample[i]+clipsam[i-lmarker])/2;
		  for (int i=length;i<cliplength+lmarker;i++)  
		    newsam[i]=clipsam[i-lmarker]/2;

		  getridof (sample);
		  sample=newsam;
		  length=cliplength+lmarker;
		  rmarker=lmarker+cliplength;
		  info ();
		}
	      else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
	    }
	}
    }
}
//**********************************************************
void MSignal::cutRange ()
//fine example for reusal of code, my prof would say, don't show him the rest
// of my code
{
 copyRange();
 deleteRange();
}
//**********************************************************
void MSignal::deleteRange ()
{
  MSignal *tmp=this;
  int all=true;

  while (tmp)
    {
      if (!tmp->isSelected()) all=false;
      tmp=tmp->getNext();
    }  

  tmp=this;
  if (all)
    {
      int r=rmarker;
      int l=lmarker;

      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
	
      while (tmp)
	{
	  tmp->deleteChannelRange();
	  tmp=tmp->getNext();
	}  
      length=length-(rmarker-lmarker);
      rmarker=lmarker;
      emit signaldeleted (l,r-l);
    }
  else
    {
      while (tmp)
	{
	  if (tmp->isSelected()) tmp->zeroChannelRange();
	  tmp=tmp->getNext();
	}  
    }
  info ();
}
//**********************************************************
void MSignal::deleteChannelRange ()
{
 if (rmarker!=lmarker)
   {
     int *newsam;

     newsam=getNewMem (length-(rmarker-lmarker));
     if (newsam)
       {
	 memcpy (newsam,sample,lmarker*sizeof(int));
	 memcpy (&newsam[lmarker],&sample[rmarker],(length-rmarker)*sizeof(int));
	 getridof (sample);
	 sample=newsam;
       }
     else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
   }
}
//**********************************************************
void MSignal::cropRange ()
{
  MSignal *tmp=this;
  while (tmp)
    {
      tmp->cropChannelRange ();
      tmp=tmp->getNext();
    }  
  info ();
}
//**********************************************************
void MSignal::cropChannelRange ()
{
  if (rmarker!=lmarker)
    {
      int *newsam;

      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;
	
      newsam=getNewMem(rmarker-lmarker);
      if (newsam)
	{
	  int r=rmarker;
	  int l=lmarker;
	  int le=length;

	  memcpy (newsam,&sample[lmarker],(rmarker-lmarker)*sizeof(int));
	  getridof (sample);
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
