#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <kprogress.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <limits.h>
#include <time.h>

#include <math.h>
#include "fftview.h"
#include "sonagram.h"

#define processid	0
#define stopprocess	1
#define samplepointer	2

#define MAXPRIME 512   //choose biggest prime factor to be tolerated before popping up a requester

extern int play16bit;
extern int bufbase;
extern MSignal *clipboard; 
extern QStrList *filterNameList;
extern QDir *filterDir;

static int len;
static int begin;

static const char *NOMEM={"Not enough Memory for Operation !"};
//**********************************************************
void MSignal::doRangeOp (int num)
{
  stopplay();	//no id needed, since every operation should cancel playing...
	
  if (lmarker<0) lmarker=0;           //some safety against bad coding
  if (rmarker>length) lmarker=length; //usually useless, or it introduces new bugs, or even better makes old bug worse to debug... 

  len=rmarker-lmarker;
  begin=lmarker;
  if (len==0)
    {
      len=length;
      begin=0;
    }

  if ((num>=TOGGLECHANNEL)&&(num<TOGGLECHANNEL+10)) toggleChannel (0,num-TOGGLECHANNEL);
  else
    if ((num>DELETECHANNEL)&&(num<DELETECHANNEL+999))
      {
	deleteChannel (0,num-DELETECHANNEL);
	channels--;
	emit sampleChanged();
	emit channelReset();
      }
    else
      if ((num>=FILTER)&&(num<FILTEREND)) movingFilter (num-FILTER);
      else
	switch (num)
	  {
	  case ALLCHANNEL:
	    selected=true;
	    if (next) next->doRangeOp (ALLCHANNEL);
	    break;
	  case INVERTCHANNEL:
	    selected=!selected;
	    if (next) next->doRangeOp (INVERTCHANNEL);
	    break;
	  case NEW:
	    newSignal();
	    break;
	  case PLAY:
	    if (play16bit) play16(false);
	    else play8(false);
	    break;
	  case LOOP:
	    if (play16bit) play16(true);
	    else play8(true);
	    break;
	  case DELETE:
	    deleteRange();
	    break;
	  case PASTE:
	    pasteRange();
	    break;
	  case MIXPASTE:
	    mixpasteRange();
	    break;
	  case CROP:
	    cropRange();
	    break;
	  case COPY:
	    copyRange();
	    break;
	  case CUT:
	    cutRange();
	    break;
	  case ZERO:
	    zeroRange();
	    break;
	  case FLIP:
	    flipRange();
	    break;
	  case CENTER:
	    center();
	    break;
	  case REVERSE:
	    reverseRange();
	    break;
	  case FADEIN:
	    fadeIn ();
	    break;
	  case FADEOUT:
	    fadeOut ();
	    break;
	  case AMPLIFYMAX:
	    amplifyMax ();
	    break;
	  case AMPLIFY:
	    amplify ();
	    break;
	  case DISTORT:
	    distort ();
	    break;
	  case AMPWITHCLIP:
	    amplifywithClip ();
	    break;
	  case NOISE:
	    noise ();
	    break;
	  case HULLCURVE:
	    hullCurve ();
	    break;
	  case DELAY:
	    delay();
	    break;
	  case RATECHANGE:
	    rateChange();
	    break;
	  case FFT:
	    fft();
	    break;
	  case PLAYBACKOPTIONS:
	    playBackOptions();
	    break;
	  case ADDCHANNEL:
	    addChannel();
	    break;
	  case ADDSYNTH:
	    addSynth();
	    break;
	  case MOVINGAVERAGE:
	    movingAverage();
	    break;
	  case SONAGRAM:
	    sonagram ();
	    break;
	  case RESAMPLE:
	    resample ();
	    break;
	  case FILTERCREATE:
	    filterCreate ();
	    break;
	  case AVERAGEFFT:
	    averagefft();
	    break;
	  case STUTTER:
	    stutter();
	  break;
	  case REQUANTISE:
	    reQuantize();
	  break;
	  }
}
//**********************************************************
void MSignal::setParent (QWidget *par)
{
  parent=par;
}
//**********************************************************
MSignal::MSignal (QWidget *par,int numsamples,int rate,int channels) :QObject ()
{
  sample=new int[numsamples];

  if (sample)
    {
      int key = ftok(".", 'S');
      int memid =-1;

      while (memid==-1) memid=shmget(key++,sizeof(int)*4,IPC_CREAT|IPC_EXCL|0660); //seems to be the only way to ensure to get a block of memory at all... 

      msg= (int*) shmat (memid,0,0);
      shmctl(memid, IPC_RMID, 0);  
    
      if (msg)
	{
	  msg[processid]=0;
	  msg[stopprocess]=false;
	  msg[samplepointer]=0;
	}
      else KMsgBox::message (parent,"Info","Could not get shared memory\n",2);

      locked=samplenotlocked;
      parent=par;
      this->channels=channels;
      this->selected=true;
      this->rate=rate;
      this->length=numsamples;
      lmarker=0;
      rmarker=0;
      
      next=0;
	  
      if (channels>1) next=new MSignal(par,numsamples,rate,channels-1);
      emit sampleChanged();
    }
  else KMsgBox::message (parent,"Info",NOMEM,2);
}
//**********************************************************
void MSignal::detachChannels () {next=0;}
int  MSignal::getLockState   () {return locked;}
//**********************************************************
MSignal::~MSignal ()
{
  if (sample!=0)
    {
      stopplay();
      delete sample;
      
      sample=0;
    }
  if (msg) shmdt ((char *)msg); 

  if (next!=0) delete next;
}
//**********************************************************
int	MSignal::getRate	() {return rate;}
int	MSignal::isSelected	() {return selected;}
int	MSignal::getChannels	() {return channels;}
MSignal *MSignal::getNext       () {return next;};
int* 	MSignal::getSample	() {return sample;}
int	MSignal::getLength	() {return length;} 
int	MSignal::getLMarker	() {return lmarker;} 
int	MSignal::getRMarker	() {return rmarker;} 
int	MSignal::getPlayPosition() {return msg[samplepointer];} 
//**********************************************************
void	MSignal::setMarkers (int l,int r )
{
	lmarker=l;
	rmarker=r;

	if (next) next->setMarkers (l,r); //recursive call for all channels
}
//**********************************************************
void MSignal::insertZero (int ins)
{
  int *newsam=new int[length+ins];
  if (newsam)
    {
      memcpy (newsam,sample,lmarker*sizeof(int));
      for (int i=0;i<ins;i++) newsam[lmarker+i]=0;
      memcpy (&newsam[lmarker+ins],&sample[rmarker],(length-rmarker)*sizeof(int));
      delete sample;
      sample=newsam;
      length+=ins;
      rmarker=lmarker+ins;
    }
  else KMsgBox::message (parent,"Info",NOMEM,2);
}
//**********************************************************
void MSignal::zeroChannelRange ()
{
 for (int i=lmarker;i<rmarker;sample[i++]=0);
}
//**********************************************************
void MSignal::zeroRange ()
{
  int len=rmarker-lmarker;

  if (len==0)
    {
      TimeDialog dialog (parent,rate,"Insert Silence :");
      if (dialog.exec())
	{
	  int ins=dialog.getLength();

	  MSignal *tmp=this;

	  while (tmp!=0)
	    {
	      tmp->insertZero (ins);
	      tmp=tmp->getNext();
	    }
	}
    }
  else
    {
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->zeroChannelRange ();
	  tmp=tmp->getNext();
	}
    }
  emit sampleChanged();
}
//**********************************************************
void MSignal::flipRange ()
{
  MSignal *tmp=this;
  while (tmp!=0)
    {
      tmp->flipChannelRange ();
      tmp=tmp->getNext();
    }
  emit sampleChanged();
}
//**********************************************************
void MSignal::flipChannelRange ()
{
  if (rmarker!=lmarker)
    {
      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;

      for (int i=lmarker;i<rmarker;sample[i]=-sample[i++]);
    }
}
//**********************************************************
void MSignal::center ()
{
  MSignal *tmp=this;
  while (tmp!=0)
    {
      if (tmp->isSelected()) tmp->centerChannel ();
      tmp=tmp->getNext();
    }
  emit sampleChanged();
}
//**********************************************************
void MSignal::centerChannel ()
{
  long long addup=0;
  int dif;

  for (int i=begin;i<begin+len;addup+=sample[i++]); 
  dif=addup/len;  //generate mean value

  if (div!=0)
    for (int i=begin;i<begin+len;sample[i++]-=dif);  //and subtract it from all samples ...
}
//**********************************************************
void MSignal::reverseRange ()
{
  MSignal *tmp=this;
  while (tmp!=0)
    {
      if (tmp->isSelected()) tmp->reverseChannelRange ();
      tmp=tmp->getNext();
    }
  emit sampleChanged();
}
//**********************************************************
void MSignal::reverseChannelRange ()
{
  if (rmarker!=lmarker)
    {
      int i,x;
      if (lmarker<0) lmarker=0;
      if (rmarker>length) rmarker=length;

      for (i=0;i<(rmarker-lmarker)/2;i++)
	{
	  x=sample[lmarker+i];
	  sample[lmarker+i]=sample[rmarker-i];
	  sample[rmarker-i]=x;
	}
    }
  else 
    {
      int i,x;

      for (i=0;i<length/2;i++)
	{
	  x=sample[i];
	  sample[i]=sample[length-i];
	  sample[length-i]=x;
	}
    }
}
//**********************************************************
void MSignal::fadeChannelOut (int curve) 
{
  int len=rmarker-lmarker;
  int i=0;

  if (curve==0)
    for (;i<len;i++)
      sample[lmarker+i]=
	(int)(((long long) (sample[lmarker+i]))*(len-i)/len);
  else if (curve<0)
    for (;i<len;i++)
      sample[lmarker+i]=
	(int)((double)sample[lmarker+i]*
	      log10(1+(-curve*((double)len-i)/len))/log10(1-curve));
  else 
    for (;i<len;i++)
      sample[lmarker+i]=
	(int)((double)sample[lmarker+i]*
	      (1-log10(1+(curve*((double)i)/len))/log10(1+curve)));
}
//**********************************************************
void MSignal::fadeOut () 
{
  if (rmarker!=lmarker)
    {
      FadeDialog dialog(parent,-1);

      if (dialog.exec ())
	{
	  int curve=dialog.getCurve();
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->fadeChannelOut (curve);
	      tmp=tmp->getNext();
	    }
	  emit sampleChanged();
	}
    }
}
//**********************************************************
void MSignal::fadeChannelIn (int curve)
{
  int len=rmarker-lmarker;
  int i=0;

  if (curve==0)
    for (;i<len;i++)
      sample[lmarker+i]=
	(int)(((long long) (sample[lmarker+i]))*i/len);
  else if (curve<0)
    for (;i<len;i++)
      sample[lmarker+i]=
	(int)((double)sample[lmarker+i]*
	      log10(1+(-curve*((double)i)/len))/log10(1-curve));
  else 
    for (;i<len;i++)
      sample[lmarker+i]=
	(int)((double)sample[lmarker+i]*
	      (1-log10(1+(curve*((double)len-i)/len))/log10(1+curve)));

}
//**********************************************************
void MSignal::fadeIn ()
{
  if (rmarker!=lmarker)
    {
      FadeDialog dialog(parent,1);

      if (dialog.exec ())
	{
	  int curve=dialog.getCurve();
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->fadeChannelIn (curve);
	      tmp=tmp->getNext();
	    }
	  emit sampleChanged();
	}
    }
}
//**********************************************************
int MSignal::newChannel (int numsamples, int rate)
{
      if (sample) delete sample;	//get rid of old sample
      sample=new int[numsamples];

      if (sample)
	{
	  for (int i=0;i<numsamples;sample[i++]=0);

	  this->rate=rate;
	  this->length=numsamples;
	  msg[processid]=0;
	  msg[stopprocess]=false;
	  msg[samplepointer]=0;
	  lmarker=0;
	  rmarker=0;

	  return true;
	}
      return false;
}
//**********************************************************
void MSignal::newSignal ()
{
  NewSampleDialog *dialog=new NewSampleDialog (parent);
  if (dialog->exec())
    {
      int	rate=dialog->getRate();
      int	numsamples=(int) (((long long)(dialog->getLength()))*rate/1000);

      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (!tmp->newChannel(numsamples,rate)) KMsgBox::message (parent,"Info",NOMEM,2);
	  tmp=tmp->getNext();
	}
      emit sampleChanged();
    }
}
//**********************************************************
void MSignal::amplifyMax ()
{
  int max=0;
  MSignal *tmp=this;
  while (tmp!=0)
    {
      int j;
      if (tmp->isSelected())
	{
	  j=tmp->getRangeMaximum();
	  if (j>max) max=j;
	}
      tmp=tmp->getNext();
    }
  tmp=this;
  while (tmp!=0)
    {
      if (tmp->isSelected()) tmp->amplifyChannelMax(max);
      tmp=tmp->getNext();
    }
  emit sampleChanged();
}
//**********************************************************
int MSignal::getChannelMaximum ()
{
  int max=0;
  for (int i=0;i<length;i++)
    if (max<abs(sample[begin+i])) max=abs(sample[begin+i]);
  return max;
}
//**********************************************************
int MSignal::getRangeMaximum ()
{
  int max=0;
  for (int i=0;i<len;i++)
    if (max<abs(sample[begin+i])) max=abs(sample[begin+i]);
  return max;
}
//**********************************************************
void MSignal::amplifyChannelMax (int max)
{
  ProgressDialog *dialog=new ProgressDialog (len*2,"Amplifying to Maximum :");
  if (dialog)
    {
      int j;
      dialog->show();

      double prop=((double)((1<<23)-1))/max;
	      
      for (int i=0;i<len;)
	{
	  if (i<len-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
	  else j=len;

	  for (;i<j;i++)
	    sample[begin+i]=(int) (sample[begin+i]*prop);	    

	  dialog->setProgress (i+len);
	}
      delete dialog;
    }
}
//*********************************************************
void MSignal::noiseRange()
{
  int len=rmarker-lmarker;
    for (int i=0;i<len;i++)
      sample[lmarker+i]=(int)((drand48()*(1<<24)-1)-(1<<23));
}
//*********************************************************
void MSignal::noiseInsert(int ins)
{
  int *newsam=new int[length+ins];
  if (newsam)
    {
      memcpy (newsam,sample,lmarker*sizeof(int));
      for (int i=0;i<ins;i++)
	newsam[lmarker+i]=(int)((drand48()*(1<<24)-1)-(1<<23));
      memcpy (&newsam[lmarker+ins],&sample[rmarker],(length-rmarker)*sizeof(int));
      delete sample;
      sample=newsam;
      length+=ins;
      rmarker=lmarker+ins;
    }
}
//*k********************************************************
void MSignal::noise()
{
  int len=rmarker-lmarker;
  srandom (0);

  if (len==0)
    {
      TimeDialog dialog (parent,rate,"Insert Silence :");
      if (dialog.exec())
	{
	  int ins=dialog.getLength();
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      tmp->noiseInsert (ins);
	      tmp=tmp->getNext();
	    }
	}
    }
  else
    {
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->noiseRange();
	  tmp=tmp->getNext();
	}
    }
  emit sampleChanged();
}
//*********************************************************
void MSignal::delayRecursive (int delay,int ampl,int start,int stop)
{
  int len=stop-start;
  ProgressDialog *dialog=new ProgressDialog (len,"Recursive delaying :");

  if (dialog)
    {
      dialog->show();
      int j;
      if (start-delay<0) start=delay;

      for (int i=start;i<stop;)
	{
	  if (i<stop-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
	  else j=stop;

	  for (;i<j;i++)
	    sample[i]=(sample[i]+(sample[i-delay]*ampl/100))*100/(ampl+100);

	  dialog->setProgress (i);
	}
      emit sampleChanged();

      delete dialog;
    }
}
//*********************************************************
void MSignal::delayOnce (int delay,int ampl,int start,int stop)
{
  int len=stop-start;
  ProgressDialog *dialog=new ProgressDialog (len,"Delaying :");

  if (dialog)
    {
      dialog->show();
      int j;
      if (start-delay<0) start=delay;

      for (int i=stop-1;i>=start;)
	{
	  if (i>start+PROGRESS_SIZE) j=i-PROGRESS_SIZE;
	  else j=start;

	  for (;i>=j;i--)
       	    sample[i]=(sample[i]+(sample[i-delay]*ampl/100))*200/(ampl+100);

	  dialog->setProgress (len-i);
      	}

      delete dialog;
    }
}
//*********************************************************
void MSignal::hullCurve ()
{
  HullCurveDialog dialog (parent);
  if (dialog.exec())
    {
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->hullCurveChannel (dialog.getTime(),dialog.getType());
	  tmp=tmp->getNext();
	}

      emit sampleChanged();
    }  
  else KMsgBox::message (parent,"Info","sampling intervall is to big for this signal",2);
}
//*********************************************************
void MSignal::hullCurveChannel (int time,int type)
{
  CPoint *nep;
  int max=0;

  int chunksize=rate*10/time;
  Interpolation interpolation(type);
  QList<CPoint> *points=new QList<CPoint>;

  if (chunksize<len)
    {
      for (int i=0;i<chunksize/2;i++)
	{
	  int act=sample[begin+i];
	  if (max<act) max=act;
	  if (max<-act) max=-act;
	}
      nep=new CPoint;
      nep->x=0;
      nep->y=(double) max;
      points->append (nep);

      int pos=begin;
      while (pos<begin+len-chunksize)
	{
	  max=0;

	  for (int i=0;i<chunksize;i++)
	    {
	      int act=sample[pos++]; 
	      if (max<act) max=act;
	      if (max<-act) max=-act;
	    }

	  nep=new CPoint;
	  nep->x=(double) (pos-chunksize/2-begin)/len;
	  nep->y=(double) max;
	  points->append (nep);
	}

      max=0;
      for (int i=len-chunksize/2;i<len;i++)
	{
	  int act=sample[begin+i];
	  if (max<act) max=act;
	  if (max<-act) max=-act;
	}
      nep=new CPoint;
      nep->x=1;
      nep->y=(double) max;
      points->append (nep);

      double *y=interpolation.getInterpolation (points,len);
	      
      for (int i=0;i<len;i++) sample[begin+i]=(int)y[i];

    }
}
//*********************************************************
void MSignal::rateChange ()
{
  RateDialog dialog (parent);
  if (dialog.exec())
    {
      rate=dialog.getRate();

      MSignal *tmp=this;
      while (tmp!=0)
	{
	  tmp->changeChannelRate (rate);
	  tmp=tmp->getNext();
	}
      emit sampleChanged();
    }  
}
//*********************************************************
void MSignal::changeChannelRate (int newrate)
{
  rate=newrate;
}
//*********************************************************
void MSignal::resampleChannel (int newrate)
{
  Interpolation interpolation(1); //get Spline Interpolation
  struct CPoint *nep;
  QList<CPoint> *points=new QList<CPoint>;

  int oldmax=getChannelMaximum();

  int newlen=(int)((double)length*(double)newrate/rate);
  int *newsample=new int[newlen];

  if (newsample&&points)
    {
      for (int i=0;i<length;i++)
	{
	  nep=new CPoint;
	  nep->x=(double) i/length;
	  nep->y=(double) sample[i];
	  points->append (nep);
	}
      double *y=interpolation.getInterpolation (points,newlen);
      if (y)
	{
	  delete points;
	  points=0;
	  delete sample;
	  sample=newsample;
	  for (int i=0;i<newlen;i++) newsample[i]=(int)y[i];

	  length=newlen;
	  begin=0;
	  len=length;
	  amplifyChannelMax (oldmax);
	  rate=newrate;
	  delete y;
	}
      else if (newsample) delete newsample;      
    }
  else if (newsample) delete newsample;
  if (points) delete points;
}
//*********************************************************
void MSignal::resample ()
{
  RateDialog dialog (parent);
  if (dialog.exec())
    {
      int rate=dialog.getRate();

      MSignal *tmp=this;
      while (tmp!=0)
	{
	  tmp->resampleChannel (rate);
	  tmp=tmp->getNext();
	}

      emit sampleChanged();
    }  
}
//*********************************************************
void MSignal::amplifywithClip ()
{
  if (clipboard)
    {
      MSignal *tmp=this;
      MSignal *clip=clipboard;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->amplifyChannelwithClip (clip);
	  tmp=tmp->getNext();
	  clip=clip->getNext();
	  if (!clip) clip=clipboard;
	}
      emit sampleChanged();
    }
}
//*********************************************************
void MSignal::amplifyChannelwithClip (MSignal *clip)
{
  int 	*clipsam	=clip->getSample();
  int	cliplength	=clip->getLength();

  for (int i=0;i<len;i++) 
    sample[begin+i]=(int)
      ((double)sample[begin+i]*clipsam[i%cliplength]/(1<<23));
}
//*********************************************************
void MSignal::amplifyChannel (double *y)
{
  for (int i=0;i<len;i++) sample[begin+i]=(int) (sample[begin+i]*y[i]);
}
//*********************************************************
void MSignal::amplify ()
{
  AmplifyCurveDialog *dialog =new AmplifyCurveDialog (parent);
  if (dialog->exec())
    {
      QList<CPoint> *points=dialog->getPoints ();

      Interpolation interpolation (dialog->getType());

      double *y=interpolation.getInterpolation (points,len);

      if (y)
	{
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->amplifyChannel (y);
	      tmp=tmp->getNext();
	    }
	}

      emit sampleChanged();

      delete dialog; 
    }  
}
//*********************************************************
void MSignal::distort ()
{
  DistortDialog *dialog =new DistortDialog (parent);
  if (dialog->exec())
    {
      QList<CPoint> *points=dialog->getPoints ();

      Interpolation *interpolation=new Interpolation (dialog->getType());

      if ((interpolation)&&(interpolation->prepareInterpolation (points)))
	{
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->distortChannel (interpolation);
	      tmp=tmp->getNext();
	    }
	  delete interpolation;
	}
      emit sampleChanged();
      delete dialog; 
    }  
}
//*********************************************************
void MSignal::distortChannel (Interpolation *interpolation)
{
  int x;
  double oldy,y;

  for (int i=0;i<len;i++)
    {
      x=sample[begin+i];
      oldy=((double) abs(x))/((1<<23)-1);
      y=interpolation->getSingleInterpolation(oldy);

      if (x>0)
	sample[begin+i]=(int) (y*((1<<23)-1));
      else 
	sample[begin+i]=-(int) (y*((1<<23)-1));
    }
}
//*********************************************************
void MSignal::delay ()
{
  DelayDialog dialog (parent,rate);
  if (dialog.exec())
    {
      int delay=dialog.getDelay();
      int ampl=dialog.getAmpl();

      int len=rmarker-lmarker;
      int begin=lmarker;

      if (len==0)
	{
	  len=length;
	  begin=0;
	}
	
      if  (dialog.isRecursive())
	{
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->delayRecursive (delay,ampl,begin,begin+len);
	      tmp=tmp->getNext();
	    }
	}
      else
	{
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->delayOnce (delay,ampl,begin,begin+len);
	      tmp=tmp->getNext();
	    }
	}
      emit sampleChanged();
    }
}
//*********************************************************
int getMaxPrime (int len)
{
  int max=0;
  int tst=len;

 //here follows the cannonical slow prime factor search, but it does its job
 //with small numbers, that should occur within this program...

  while ((tst%2)==0) tst/=2;  //remove prime factor 2

  for (int i=3;i<=sqrt(tst);i+=2)
	if ((tst%i)==0)
	  {
	    if (i>max) max=i;
	    while ((tst%i)==0) tst/=i;  //divide the current prime factor until it is not present any more
	  }
  if (tst>max) max=tst;

  return max;
}
//*********************************************************
void MSignal::fft ()
{
  int max=getMaxPrime (len);
  int reduce=1;
  int res=1;

  if (max>MAXPRIME)
    {
      char buf[512];
      while ((len-reduce>MAXPRIME)&&(getMaxPrime(len-reduce)>MAXPRIME)) reduce++;


      sprintf (buf,"The selected number of samples, contains the large prime factor %d.\nThis may result in extremly long computing time\nIt is recommended to reduce the selected range by %d samples\nto gain lots of speed, but lose some accuracy !\nWhat do you want to go for ?",max,reduce);
      res=KMsgBox::yesNoCancel(parent,"Attention",buf,KMsgBox::QUESTION,"Accuracy","Speed","Cancel");
      if (res==2) len-=reduce;
    }

  if (res!=3) 
    {
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->fftChannel ();
	  tmp=tmp->getNext();
	}
    }
  if (res==2) len+=reduce;
}
//*********************************************************
void MSignal::fftChannel ()
{
  complex *data=0;

  data=new complex[len];

  if (data)
    {
      double rea,ima,max=0;

      for (int i=0;i<len;i++)
	{
	  data[i].real=((double)(sample[begin+i])/(1<<23));
	  data[i].imag=0;
	}

      gsl_fft_complex_wavetable table;

      gsl_fft_complex_wavetable_alloc (len,&table);

      gsl_fft_complex_init (len,&table);

      gsl_fft_complex_forward	(data,len,&table);
      gsl_fft_complex_wavetable_free	(&table);

      for (int i=0;i<len;i++)
	{
	  rea=data[i].real;
	  ima=data[i].imag;
	  rea=sqrt(rea*rea+ima*ima);	        //get amplitude
	  if (max<rea) max=rea;
	}

      FFTWindow *win=new FFTWindow();
      if (win)
	{
	  win->show();
	  win->setSignal (data,max,len,rate);
	}
      else
	KMsgBox::message
	  (parent,"Info","Could not open Freqency-Window!",2);
    }
  else
    {
      if (data) delete data;
      KMsgBox::message
	(parent,"Info","No Memory for FFT-buffers available !",2);
    }
}
//*********************************************************
void MSignal::playBackOptions ()
{
      PlayBackDialog dialog (parent,play16bit,bufbase);
      if (dialog.exec())
	{
	  play16bit=dialog.getResolution();
	  bufbase=dialog.getBufferSize();
	}  
}
//*********************************************************
void MSignal::addSynth ()
{
  AddSynthDialog *dialog =new AddSynthDialog (parent);
  if (dialog->exec())
    {
      MSignal *add=dialog->getSignal ();
      if (add)
	{
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->pasteChannelRange (add);
	      else tmp->insertZero ((int)(len));

	      tmp=tmp->getNext();
	    }

	  emit sampleChanged();
	  delete dialog; 
	  delete add;
	}
    }
}
//*********************************************************
void MSignal::movingAverage ()
{
  AverageDialog *dialog =new AverageDialog (parent,"Moving Average:");
  if (dialog->exec())
    {
      int average=dialog->getTaps();
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->averageChannel (average);
	  tmp=tmp->getNext();
	}
    }
  emit sampleChanged();
  delete dialog; 
}
//*********************************************************
void MSignal::averageChannel (int a)
{
  int b=a/2;
  int newsam;
  int i,j;

  int *sam=new int [len];
  if (sam)
    {
      for (i=begin+b;i<begin+len-b;i++)
	{
	  newsam=0;
	  for (j=-b;j<b;j++) newsam+=sample[i+j];
	  newsam/=a;
	  sam[i-begin]=newsam;
	}
      memcpy (&sample[begin+b],&sam[b],(len-a)*sizeof(int));
      delete sam;
    }
}
//*********************************************************
void MSignal::sonagram ()
{
  SonagramDialog *dialog =new SonagramDialog (parent,len,rate,"Sonagram:");
  if (dialog)
    {
      if (dialog->exec())
	{
	  int points=dialog->getPoints();

	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->sonagramChannel (points);
	      tmp=tmp->getNext();
	    }
	}
      emit sampleChanged();
      delete dialog; 
    }
}
//*********************************************************
void MSignal::sonagramChannel (int points)
{
  int length=((len/(points/2))*points/2)+points/2; //round up length
  double *data=new double[length];
  int i;

  if (data)
    {
      for (i=0;i<len;i++)
	data[i]=((double)(sample[begin+i])/(1<<23));
      for (;i<length;i++) data[i]=0; //pad with zeros...

      SonagramWindow *sonagramwindow=new SonagramWindow();
      if (sonagramwindow)
	{
	  sonagramwindow->show();
	  sonagramwindow->setSignal (data,length,points,rate);
	} 
     delete data;
    }
}
//*********************************************************
void MSignal::averagefft ()
{
  MSignal *tmp=this;
  while (tmp!=0)
    {
      if (tmp->isSelected()) tmp->averagefftChannel (1024);
      tmp=tmp->getNext();
    }
}
//*********************************************************
void MSignal::averagefftChannel (int points)
{
  complex *data=new complex[points];
  complex *avgdata=new complex[points];
  int count=0;

  if (data&&avgdata)
    {
      gsl_fft_complex_wavetable table;
      gsl_fft_complex_wavetable_alloc (points,&table);
      gsl_fft_complex_init (points,&table);


      for (int i=0;i<points;i++)
	{
	  avgdata[i].real=0;
	  avgdata[i].imag=0;
	}

      double rea,ima,max=0;
      int page=0;

      while (page<len)
	{
	  if (page+points<len)
	    for (int i=0;i<points;i++)
	      {
		data[i].real=((double)(sample[begin+i])/(1<<23));
		data[i].imag=0;
	      }
	  else 
	    {
	      int i=0;
	      for (;i<len-page;i++)
		{
		  data[i].real=((double)(sample[begin+i])/(1<<23));
		  data[i].imag=0;
		}
	      for (;i<points;i++)
		{
		  data[i].real=0;
		  data[i].imag=0;
		}
	    }

	  page+=points;
	  count++;
	  gsl_fft_complex_forward	(data,points,&table);

	  for (int i=0;i<points;i++)
	    {
	      rea=data[i].real;
	      ima=data[i].imag;
	      avgdata[i].real+=sqrt(rea*rea+ima*ima);
	    }
	}

      if (data) delete data;
      gsl_fft_complex_wavetable_free (&table);

      for (int i=0;i<points;i++)        //find maximum
	{
	  avgdata[i].real/=count;
	  rea=avgdata[i].real;
	  if (max<rea) max=rea;
	}

      FFTWindow *win=new FFTWindow();
      if (win)
	{
	  win ->show();
	  win->setSignal (avgdata,max,points,rate);
	}
      else
	KMsgBox::message
	  (parent,"Info","Could not open Freqency-Window!",2);
    }
  else
    {
      if (data) delete data;
      KMsgBox::message
	(parent,"Info","No Memory for FFT-buffers available !",2);
    }
}
//*********************************************************
void MSignal::filterCreate ()
{
  FilterDialog dialog (parent,rate);
  if (dialog.exec ())
    {
      Filter *filter=dialog.getFilter ();
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->filterChannel (filter);
	  tmp=tmp->getNext();
	}
      emit sampleChanged();
    }
}
//*********************************************************
void MSignal::filterChannel (Filter *filter)
{
  double val;
  double addup=0;
  int max=0;
  for (int j=0;j<filter->num;j++)
    {
      addup+=fabs(filter->mult[j]);
      if (max<filter->offset[j]) max=filter->offset[j]; //find maximum offset
    }

  if (filter->fir)
    {
      for (int i=begin+len-1;i>=begin+max;i--)
	{
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
      for (int i=begin+max-1;i>=begin;i--)  //slower routine because of check, needed only in this range...
	{
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    if (i-filter->offset[j]>0) val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
    }
  else //basically the same,but the loops go viceversa
    {
      for (int i=begin;i<begin+max;i++)  //slower routine because of check, needed only in this range...
	{
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    if (i-filter->offset[j]>0) val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
      for (int i=begin+max;i<begin+len;i++)
	{
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
    }
}
//*********************************************************
void MSignal::movingFilterChannel (Filter *filter,int tap,double *move)
{
  double val;
  double addup=0;
  int max=0;
  for (int j=0;j<filter->num;j++)
    {
      addup+=fabs(filter->mult[j]);
      if (max<filter->offset[j]) max=filter->offset[j]; //find maximum offset
    }

  if (filter->fir)
    {
      for (int i=begin+len-1;i>=begin+max;i--)
	{
	  filter->mult[tap]=(move[i-begin]);
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
      for (int i=begin+max-1;i>=begin;i--)  //slower routine because of check, needed only in this range...
	{
	  filter->mult[tap]=(move[i-begin]);
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    if (i-filter->offset[j]>0) val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
    }
  else //basically the same,but the loops go viceversa
    {
      for (int i=begin;i<begin+max;i++)  //slower routine because of check, needed only in this range...
	{
	  filter->mult[tap]=(move[i-begin]);
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    if (i-filter->offset[j]>0) val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
      for (int i=begin+max;i<begin+len;i++)
	{
	  filter->mult[tap]=(move[i-begin]);
	  val=filter->mult[0]*sample[i];
	  for (int j=1;j<filter->num;j++)
	    val+=filter->mult[j]*sample[i-filter->offset[j]];
	  sample[i]=(int)(val/addup);    //renormalize
	}
    }
}
//*********************************************************
void MSignal::movingFilter (int number)
{
  QString name=filterNameList->at(number);

  MovingFilterDialog *dialog =new MovingFilterDialog (parent,1);
  Filter *filter=new Filter;

  if (dialog&&filter)
    {
      QString fullname=filterDir->filePath(name);
      filter->load (&fullname);

      if (dialog->exec())
	{
	  if (dialog->getState()) //moving filter
	    {
	      QList<CPoint> *points=dialog->getPoints ();
	      Interpolation interpolation (dialog->getType());
	      int low=dialog->getLow ();
	      int high=dialog->getHigh ();
	      int tap=dialog->getTap();

	      double *y=interpolation.getInterpolation (points,len);
	      if (y)
		{
		  for (int i=0;i<len;i++) y[i]=((double)low)/1000+(((double)(high-low))/1000*y[i]); //rescale range of filtermovement...
		  MSignal *tmp=this;
		  while (tmp!=0)
		    {
		      if (tmp->isSelected()) tmp->movingFilterChannel (filter,tap,y);
		      tmp=tmp->getNext();
		    }
		  emit sampleChanged();
		  delete y;
		}
	    }
	  else  //use normal filtering
	    {
	      MSignal *tmp=this;
	      while (tmp!=0)
		{
		  if (tmp->isSelected()) tmp->filterChannel (filter);
		  tmp=tmp->getNext();
		}
	      emit sampleChanged();
	    }
	}
      delete dialog;
      delete filter;
    }
}
//*********************************************************
void MSignal::stutter ()
{
  StutterDialog *dialog =new StutterDialog (parent,rate);
  if (dialog->exec())
    {
      MSignal *tmp=this;
      int len1=dialog->getLen1();
      int len2=dialog->getLen2();

      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->replaceStutterChannel (len1,len2);
	  tmp=tmp->getNext();
	}
      emit sampleChanged();
      delete dialog; 
    }  
}
//*********************************************************
void MSignal::replaceStutterChannel (int len1,int len2)
{
  int j;
  int i=len2;
  while (i<len-len1)
    {
      for (j=0;j<len1;j++) sample[begin+i+j]=0;
      i+=len1+len2;
    }
}
//*********************************************************
void MSignal::reQuantize ()
{
  QuantiseDialog *dialog =new QuantiseDialog (parent);
  if (dialog->exec())
    {
      MSignal *tmp=this;
      int bit=dialog->getBits();

      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->quantizeChannel (bit);
	  tmp=tmp->getNext();
	}
      emit sampleChanged();
      delete dialog; 
    }  
}
//*********************************************************
void MSignal::quantizeChannel (int bits)
{
  double a;
  bits--;
      for (int j=0;j<len;j++)
	{
	  a=(double)(sample[begin+j]+(1<<23))/(1<<24);
	  a=floor(a*bits+.5);
	  a/=bits;
	  sample[begin+j]=(int)((a-.5)*((1<<24)-1));   //24 because of souble range
	}
}


