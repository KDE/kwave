#include <unistd.h>
#include "sample.h"
#include <kmsgbox.h>
#include <kprogress.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <limits.h>
#include <math.h>
#include "fftview.h"

#define processid	0
#define stopprocess	1
#define samplepointer	2


extern int play16bit;
extern MSignal *clipboard; 
//**********************************************************
void MSignal::doRangeOp (int num)
{
	stopplay();	//no id needed, since every operation should cancel playing...
	
	switch (num)
	 {
		case NEW:
			newSignal();
		break;
		case PLAY:
		  if (play16bit) play16();
		  else play8();
		break;
		case LOOP:
		  if (play16bit) loop16();
			else loop8();
		break;
		case DELETE:
			deleteRange();
		break;
		case PASTE:
			pasteRange();
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
		case FADEINLINEAR:
			fadeInLinear ();
		break;
		case FADEINLOG:
			fadeInLogarithmic ();
		break;
		case FADEOUTLOG:
			fadeOutLogarithmic ();
		break;
		case FADEOUTLINEAR:
			fadeOutLinear ();
		break;
		case AMPLIFYMAX:
			amplifyMax ();
		break;
		case NOISE:
			noise ();
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
	 }
}
//**********************************************************
MSignal::MSignal (QWidget *par,int numsamples,int rate=48000) :QObject ()
{
 sample=new int[numsamples];

 if (sample)
  {
	parent=par;
	this->rate=rate;
	this->length=numsamples;
	memid=shmget(IPC_PRIVATE,sizeof(int)*4,IPC_CREAT+(6<<6)+(6<<3));
	msg= (int*) shmat (memid,0,0);
	msg[processid]=0;
	msg[stopprocess]=false;
	msg[samplepointer]=0;

	emit sampleChanged();
  }
 else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
}
//**********************************************************
MSignal::~MSignal ()
{
	if (sample!=0)
	 {
		stopplay();
		delete sample;
		sample=0;
	 }
}
//**********************************************************
int	MSignal::getRate	() {return rate;}
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
}
//**********************************************************
void MSignal::copyRange ()
{
 MSignal *temp; 
 int	 *tsample;

 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	temp=new MSignal (parent,rmarker-lmarker,rate);
	if (temp)
	 {
		int j=0;
		tsample=temp->getSample();

		for (int i=lmarker;i<rmarker;tsample[j++]=sample[i++]);

		if (clipboard) delete clipboard;
		clipboard=temp;

		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::pasteRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(clipboard))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	int 	*clipsam	=clipboard->getSample();
	int	cliplength	=clipboard->getLength();

	newsam=new int[length+cliplength-(rmarker-lmarker)];
	if (newsam)
	 {
		memcpy (newsam,sample,lmarker*sizeof(int));
		memcpy (&newsam[lmarker],clipsam,(cliplength)*sizeof(int));
		memcpy (&newsam[lmarker+cliplength],&sample[rmarker],(length-rmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length+=cliplength-(rmarker-lmarker);
		rmarker=lmarker+cliplength;
		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
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
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	newsam=new int[length-(rmarker-lmarker)];
	if (newsam)
	 {
		memcpy (newsam,sample,lmarker*sizeof(int));
		memcpy (&newsam[lmarker],&sample[rmarker],(length-rmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length=length-(rmarker-lmarker);
		rmarker=lmarker;
		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::cropRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int *newsam;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;
	
	newsam=new int[rmarker-lmarker];
	if (newsam)
	 {
		memcpy (newsam,&sample[lmarker],(rmarker-lmarker)*sizeof(int));
		delete sample;
		sample=newsam;
		length=rmarker-lmarker;
		rmarker=0;
		lmarker=0;

		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::zeroRange ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;

	for (int i=lmarker;i<rmarker;sample[i++]=0);
	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::flipRange ()
{
 if ((lmarker<length)&&(rmarker>=0)&&(rmarker!=lmarker))
  {
	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;

	for (int i=lmarker;i<rmarker;sample[i]=-sample[i++]);
	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::center ()
{
 if ((lmarker<length)&&(rmarker>=0))
  {
    int begin,len;
    long long addup=0;
    int dif;

	if (lmarker<0) lmarker=0;
	if (rmarker>length) rmarker=length;

	begin=lmarker;
	len=rmarker-lmarker;
	if (len==0) 
	  {
	    begin=0;
	    len=length;
	  }

	for (int i=begin;i<begin+len;addup+=sample[i++]); 
	dif=addup/len;  //generate mean value

	if (div!=0)
	  for (int i=begin;i<begin+len;sample[i++]-=dif);  //and subtract it from all samples ...

	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::reverseRange ()
{
 if ((lmarker<length)&&(rmarker>=0))
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
	emit sampleChanged();
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
	emit sampleChanged();
 }
}
//**********************************************************
void MSignal::fadeOutLinear() 
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int len=rmarker-lmarker;
	int i=0;

	for (;i<len;i++)
		sample[lmarker+i]=(int)(((long long)(sample[lmarker+i]))*(len-i)/len);

	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::fadeInLinear ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int len=rmarker-lmarker;
	int i=0;

	for (;i<len;i++)
		sample[lmarker+i]=(int)(((long long) (sample[lmarker+i]))*i/len);

	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::fadeInLogarithmic ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int len=rmarker-lmarker;
	int i=0;

	for (;i<len;i++)
		sample[lmarker+i]=(int)(log10(1+(99*((double)i)/len))*sample[lmarker+i])/2;

	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::fadeOutLogarithmic ()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int len=rmarker-lmarker;
	int i=0;

	for (;i<len;i++)
	sample[lmarker+i]=(int)(log10(1+(99*(1-((double)i)/len)))*sample[lmarker+i])/2;

	emit sampleChanged();
  }
}
//**********************************************************
void MSignal::newSignal ()
{
 NewSampleDialog *dialog=new NewSampleDialog (parent);
 if (dialog->exec())
  {
	int	rate=dialog->getRate();
	int	numsamples=(int) (((long long)(dialog->getLength()))*rate/1000);

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

		emit sampleChanged();
	 }
	else KMsgBox::message (parent,"Info","Not enough Memory for Operation !",2);
  }
}
//**********************************************************
void MSignal::amplifyMax ()
{
  if ((lmarker<length)&&(rmarker>=0))
    {
      int len=rmarker-lmarker;
      int begin=lmarker;

      if (len==0)
	{
	  len=length;
	  begin=0;
	}
      int max=0;

      ProgressDialog *dialog=new ProgressDialog (len*2,"Amplifying to Maximum :");
      if (dialog)
	{
	  int j;
	  dialog->show();

	  for (int i=0;i<len;)
	    {
	      if (i<len-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
	      else j=len;

	      for (;i<j;i++)
		if (max<abs(sample[begin+i])) max=abs(sample[begin+i]);

	      dialog->setProgress (i);
	    }

	  if (max<(1<<23)-1)
	    {
	      double prop=((double)((1<<23)-1))/max;
	      
	      for (int i=0;i<len;)
		{
		  if (i<len-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
		  else j=len;

		  for (;i<j;i++)
		    sample[begin+i]=(int) (sample[begin+i]*prop);	    

	      	  dialog->setProgress (i+len);
		}
	    }
	  emit sampleChanged();
	}
	  delete dialog;
    }
}
//*********************************************************
void MSignal::noise()
{
 if ((lmarker<length)&&(rmarker>0)&&(rmarker!=lmarker))
  {
	int len=rmarker-lmarker;
	int i=0;

	srandom (0);

	for (;i<len;i++)
		sample[lmarker+i]=(int)((drand48()*(1<<24)-1)-(1<<23));

	emit sampleChanged();
  }
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
      emit sampleChanged();

      delete dialog;
    }
}
//*********************************************************
void MSignal::rateChange ()
{
      RateDialog dialog (parent);
      if (dialog.exec())
	{
	  rate=dialog.getRate();

	  emit sampleChanged();
	}  
}
//*********************************************************
void MSignal::delay ()
{
  if ((lmarker<length)&&(rmarker>=0))
    {
      DelayDialog dialog (parent);
      if (dialog.exec())
	{
	  int delay=dialog.getDelay()*rate/10000;
	  int ampl=dialog.getAmpl();

	  int len=rmarker-lmarker;
	  int begin=lmarker;

	  if (len==0)
	    {
	      len=length;
	      begin=0;
	    }
	
	  if  (dialog.isRecursive())
	    delayRecursive (delay,ampl,begin,begin+len);
	  else delayOnce (delay,ampl,begin,begin+len);				       }
    }
}
//*********************************************************
#include <time.h>

void MSignal::fft ()
{
  if ((lmarker<length)&&(rmarker>=0))
    {
      int len=rmarker-lmarker;
      int begin=lmarker;

      if (len==0)
	{
	  len=length;
	  begin=0;
	}

      complex *data=0;
      ProgressDialog *prog=0;


      prog =new ProgressDialog (100,"Doing mixed radix fft");
      if (prog)
	{
	  prog->exec();
	}

      data=new complex[len];

      if ((data)&&(prog))
	{
	  double rea,ima,max=0;

	  prog->setProgress (1);

	  for (int i=0;i<len;i++)
	    {
	      data[i].real=((double)(sample[begin+i])/(1<<23));
	      data[i].imag=0;
	    }

	  gsl_fft_complex_wavetable table;

	  prog->setProgress (2);

	  gsl_fft_complex_wavetable_alloc (len,&table);

	  prog->setProgress (3);

	  gsl_fft_complex_init (len,&table);

	  prog->setProgress (4);

	  gsl_fft_complex_forward	(data,len,&table);
	  gsl_fft_complex_wavetable_free	(&table);

	  prog->setProgress (5);

	  for (int i=0;i<len;i++)
	    {
	      rea=data[i].real;
	      ima=data[i].imag;
	      data[i].real=sqrt(rea*rea+ima*ima);	//get amplitude
	      data[i].imag=(atan (ima/rea)+PI/2)/PI;	//get phase and	scale to 1.0
	      if (max<data[i].real) max=data[i].real;
	    }

	  prog->setProgress (6);

	  for (int i=0;i<len;i++) data[i].real/=max;	//scale amplitude to 1.0
	  delete prog;

	  FFTWindow *win=new FFTWindow();
	  if (win)
	    {
	      win ->show();
	      win->setSignal (data,len,rate);
	    }
	  else
	    KMsgBox::message
	      (parent,"Info","Could not open Freqency-Window!",2);
	}
      else
	{
	  if (data) delete data;
	  if (prog) delete prog;
	  KMsgBox::message
	    (parent,"Info","No Memory for FFT-buffers available !",2);
	}
    }
}
//*********************************************************
void MSignal::playBackOptions ()
{
      PlayBackDialog dialog (parent,play16bit);
      if (dialog.exec())
	{
	  play16bit=dialog.getResolution();
	 
	}  
}






















