// In this file you will find methods for changing time-domain signals
// cut/copy/paste functions are included in sampleedit.cpp

// I/O Functions such as loading/saving are in sampleio.cpp

//Here choose biggest prime factor to be tolerated before
//popping up a requester, when doing a fft
#define MAXPRIME 512 

#include <dlfcn.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "kwavesignal.h"
#include "parser.h"
#include "gsl_fft.h"
#include "windowfunction.h"
#include "globals.h"
#include "filter.h"

#include <kmsgbox.h>
#include "windowfunction.h"
#include "dynamicloader.h"

extern void *mmapalloc (int);
extern void mmapfree   (void *);
extern long mmap_threshold; //thresholf for using memory mapped to files
extern Global globals;
//**********************************************************
int *KwaveSignal::getNewMem (int size)
  //these are internal methods replacing new and delete if file-mapped
  //memory is desired  
{
  int *mem;
  if (size>(mmap_threshold<<20)) 
    {
      mapped=true;
      mem=(int *) mmapalloc(size*sizeof(int));
    }
  else mem=new int[size];
  return mem;
}
//**********************************************************
void KwaveSignal::getridof (int *mem)
{
  if (mem)
    if (mapped) mmapfree (mem);
    else delete [] mem;
}
//**********************************************************
void  KwaveSignal::getMaxMin (int &max,int &min,int begin,int len)
{
  int c;
  int *sample=&(this->sample[begin]);
  min=INT_MAX;
  max=INT_MIN;
  for (int i=0;i<len;i++)
    {
      c=sample[i];
      if (c>max) max=c;
      if (c<min) min=c;
    }
}
//**********************************************************
KwaveSignal::KwaveSignal (int *signal,int numsamples,int rate)
  // constructor for creating a "silence"-sample 
  // the other constructor which loads a file into memory is located in sampleio.cpp
{
  this->rate=rate;
  this->length=numsamples;
  mapped=false;
  sample=signal;
  locked=0;
  lmarker=0;
  rmarker=0;
}
//**********************************************************
KwaveSignal::KwaveSignal (int numsamples,int rate)
  // constructor for creating a "silence"-sample 
{
  this->rate=rate;
  this->length=numsamples;

  mapped=false;
  sample=getNewMem (length);

  if (sample)
    {
      //erase, because memory is not cleared by default
      for (int i=0;i<length;sample[i++]=0);

      //initialise attributes

      locked=0;
      lmarker=0;
      rmarker=0;
      
      
    }
  else noMemory ();
}
//**********************************************************
KwaveSignal::~KwaveSignal ()
{
  if (sample!=0)
    {
      getridof (sample);
      sample=0;
    }
}
//**********************************************************
//functions that return attribute to external callers
int     KwaveSignal::getLockState   () {return locked;}
int	KwaveSignal::getSingleSample (int offset) {return sample[offset];} 
//**********************************************************
void KwaveSignal::setMarkers (int l,int r )
  //this one sets the internal markers, most operations use
{
  lmarker=l;
  rmarker=r;
}
// now follow the various editing and effects functions
//**********************************************************
void KwaveSignal::fadeOut(const char *)
{
  int *sample=&(this->sample[lmarker]);
  int i=0;
  int curve=0;

  counter=0;

  if (curve==0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)(((long long) (sample[i]))*(len-i)/len);
	counter=i;
      }
  else if (curve<0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		log10(1+(-curve*((double)len-i)/len))/log10(1-curve));
	counter=i;
      }
  else 
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		(1-log10(1+(curve*((double)i)/len))/log10(1+curve)));
	counter=i;
      }
  counter=-1;   //signal progress that action is performed...
}
//**********************************************************
void KwaveSignal::fadeIn (const char *)
{
  int *sample=&(this->sample[lmarker]);
  int curve=0;

  int i=0;
  if (curve==0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)(((long long) (sample[i]))*i/len);
	counter=i;
      }
  else if (curve<0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		log10(1+(-curve*((double)i)/len))/log10(1-curve));
	counter=i;
      }
  else 
    {
      for (;i<len;i++)
	sample[i]=
	  (int)((double)sample[i]*
		(1-log10(1+(curve*((double)len-i)/len))/log10(1+curve)));
      counter=i;
    }
  counter=-1;   //signal progress that action is performed...
}
//**********************************************************
int KwaveSignal::getChannelMaximum ()
{
  int max=0;
  for (int i=0;i<length;i++)
    if (max<abs(sample[i])) max=abs(sample[i]);

  return max;
}
//**********************************************************
void KwaveSignal::delayRecursive (int delay,int ampl)
{
  int j;
  if (lmarker-delay<0) lmarker=delay;

  counter=0;
  for (int i=lmarker;i<rmarker;)
    {
      if (i<rmarker-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
      else j=rmarker;

      for (;i<j;i++)
	sample[i]=(sample[i]+(sample[i-delay]*ampl/100))/2;

      counter=i;
    }
  counter=-1;
}
//*********************************************************
void KwaveSignal::delay (int delay,int ampl)
{
  int j;

  counter=0;
  if (lmarker-delay<0) lmarker=delay;

  for (int i=rmarker-1;i>=lmarker;)
    {
      if (i>lmarker+PROGRESS_SIZE) j=i-PROGRESS_SIZE;
      else j=lmarker;

      for (;i>=j;i--)
	sample[i]=(sample[i]+(sample[i-delay]*ampl/100))/2;

      counter=len-i;
    }
  counter=-1;
}
//*********************************************************
void KwaveSignal::hullCurve (int time,int type)
{
  int max=0;

  int chunksize=rate*10/time;
  Interpolation interpolation (type);
  Curve *points=new Curve;

  if (chunksize<len)
    {
      for (int i=0;i<chunksize/2;i++)
	{
	  int act=sample[begin+i];
	  if (max<act) max=act;
	  if (max<-act) max=-act;
	}

      points->append (0,(double) max);

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
    	  points->append ((double) (pos-chunksize/2-begin)/len,(double) max);
	}

      max=0;
      for (int i=len-chunksize/2;i<len;i++)
	{
	  int act=sample[begin+i];
	  if (max<act) max=act;
	  if (max<-act) max=-act;
	}
      points->append (1,(double) max);

      double *y=interpolation.getInterpolation (points,len);
	      
      for (int i=0;i<len;i++) sample[begin+i]=(int)y[i];

    }
}
//*********************************************************
void KwaveSignal::changeRate (int newrate)
{
  rate=newrate;
}
//*********************************************************
void KwaveSignal::resample (const char *)
{
  int newrate=44100;
  Interpolation interpolation(1); //get Spline Interpolation
  Curve *points=new Curve;

  //  int oldmax=getChannelMaximum();

  int newlen=(int)((double)length*(double)newrate/rate);
  int *newsample=getNewMem (newlen);

  if (newsample&&points)
    {
      for (int i=0;i<length;i++)
	  points->append  (((double) i)/length,(double) sample[i]);

      double *y=interpolation.getInterpolation (points,newlen);
      if (y)
	{
	  delete points;
	  points=0;

	  getridof (sample);
      
	  sample=newsample;
	  for (int i=0;i<newlen;i++) newsample[i]=(int)y[i];

	  length=newlen;
	  begin=0;
	  len=length;
	  //	  amplifyChannelMax (oldmax);
	  rate=newrate;
	  delete y;
	}
      else getridof (newsample);      
    }
  else getridof (newsample);      
  if (points) delete points;
}
//*********************************************************
int getMaxPrimeFactor (int len)
{
  int max=1;
  int tst=len;

 //here follows the canonical slow prime factor search, but it does its job
 //with small numbers, greater ones should not occur within this program...

  if (((tst%2))==0)
    {
      max=2;
      tst/=2;
      while ((tst%2)==0) tst/=2;  //remove prime factor 2
    }
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
void KwaveSignal::fft (int windowtype,bool accurate)
{
  complex *data=0;

  if (!accurate)
    {
      int reduce=1;
      int max=getMaxPrimeFactor (len);  //get biggest prime factor

      if (max>MAXPRIME) 
	{
	  while ((len-reduce>MAXPRIME)&&(getMaxPrimeFactor(len-reduce)>MAXPRIME)) reduce++;
	  len-=reduce; //correct length of buffer to be transferred
	}
    }

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

    }
  else
    {
      if (data) delete data;
      KMsgBox::message
	(0,"Info","No Memory for FFT-buffers available !",2);
    }
}
//*********************************************************
void KwaveSignal::average (const char *)
{
  int taps=3;
  int *sample=&(this->sample[lmarker]);

  int b=taps/2;
  long int newsam;
  int i,j;

  int *sam=new int[len];
  if (sam)
    {
      for (i=b;i<len-b;i++)
	{
	  newsam=0;
	  for (j=-b;j<b;j++) newsam+=sample[i+j];
	  newsam/=taps;
	  sam[i]=newsam;
	  counter=i;
	}
      memcpy (&sample[b],&sam[b],(len-taps)*sizeof(int));
      delete sam;
    }
  counter=-1;
}
//*********************************************************
void KwaveSignal::sonagram (int points,int windowtype)
{
  int length=((len/(points/2))*points/2)+points/2; //round up length
  double *data=new double[length];
  int i;

  if (data)
    {
      for (i=0;i<len;i++)
	data[i]=((double)(sample[begin+i])/(1<<23));
      for (;i<length;i++) data[i]=0; //pad with zeros...

      //do attach data

     delete data;
    }
}
//*********************************************************
void KwaveSignal::averageFFT (int points,int windowtype)
{
  complex *data=new complex[points];
  complex *avgdata=new complex[points];
  WindowFunction func(windowtype);

  double *windowfunction=func.getFunction (points);

  int count=0;

  if (data&&avgdata&&windowfunction)
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
		data[i].real=(windowfunction[i]*(double)(sample[begin+i])/(1<<23));
		data[i].imag=0;
	      }
	  else 
	    {
	      int i=0;
	      for (;i<len-page;i++)
		{
		  data[i].real=(windowfunction[i]*(double)(sample[begin+i])/(1<<23));
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

      //create window for object
    }
  else
    {
      if (data) delete data;
      KMsgBox::message
	(0,"Info","No Memory for FFT-buffers available !",2);
    }
}
//*********************************************************
void KwaveSignal::filter (Filter *filter)
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
void KwaveSignal::movingFilter (Filter *filter,int tap, Curve *points,int low,int high)
{
  Interpolation interpolation (0);
  
  double *move=interpolation.getInterpolation (points,len);
  if (move)
    {
      for (int i=0;i<len;i++) move[i]=((double)low)/1000+(((double)(high-low))/1000*move[i]); //rescale range of filtermovement...

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
}
//*********************************************************
void KwaveSignal::replaceStutter (int len1,int len2)
{
  int *sample=&(this->sample[lmarker]);

  int j;
  int i=len2;
  while (i<len-len1)
    {
      for (j=0;j<len1;j++) sample[i+j]=0;
      i+=len1+len2;
      counter=i;
    }
  counter=-1;
}
//*********************************************************
void KwaveSignal::quantize (const char *)
{
  int bits=4;
  int *sample=&(this->sample[lmarker]);

  double a;
  for (int j=0;j<len;j++)
    {
      a=(double)(sample[j]+(1<<23))/(1<<24);
      a=floor(a*bits+.5);
      a/=bits;
      sample[j]=(int)((a-.5)*((1<<24)-1));   //24 because of double range (no more signed)
      counter=j;
    }
  counter=-1;   //signal progress that action is performed...
}
//*********************************************************
void KwaveSignal::lockRead ()
{
  locked++;
}
//*********************************************************
void KwaveSignal::unlockRead ()
{
  locked--;
}
//*********************************************************
void KwaveSignal::lockWrite ()
{
  locked=-1;
}
//*********************************************************
void KwaveSignal::unlockWrite ()
{
  locked=0;
}
//*********************************************************
void KwaveSignal::command (TimeOperation *operation)
{
  const char *command=operation->getCommand();

  printf ("signal : %s\n",command);

  if (lmarker<0) lmarker=0; //check markers for bounding
  if (rmarker>length) lmarker=length; 

  //the above is usually useless, or it introduces
  //new bugs, or even better makes old bug worse to debug... 
  //once the kwave seems to be bugfree, I will leave this out,
  //let's see what happens ;-)

  len=rmarker-lmarker;
  begin=lmarker;

  if (len==0) //if no part is selected select the whole signal
    {
      len=length;
      begin=0;
    }

  if (globals.timeplugins)
    {
      int cnt=0;
      bool done=false;
      KwaveParser parse (command);

      while ((globals.timeplugins[cnt])&&(!done))
	{
	  if (strcmp(parse.getCommand(),
		     globals.timeplugins[cnt]->getName()
		     )==0)
	    {
	      void *handle=dlopen(globals.timeplugins[cnt]->getFileName(),
				  RTLD_NOW);
	      if (handle)
		{
		  int (*modfunction)(TimeOperation *);      

		  modfunction=(int (*)(TimeOperation *))
		    dlsym(handle,"operation__FP13TimeOperation");
		  //currently assuming egcs mangling behaviour

		  if (modfunction) (*modfunction)(operation); 
		  else printf ("dlerror:%s\n",dlerror());

		  dlclose (handle);
		}
	      else printf ("dlerror:%s\n",dlerror());

	      done=true;
	    }
	  else cnt++;
	}
    }
  else printf ("no timeplugins known...\n");
}
