//This file includes all methods of Class SignalManager that deal with
//sound playback
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <kmsgbox.h>
#include <linux/soundcard.h>
#include <endian.h>
#include <limits.h>
#include "signalmanager.h"
#include <libkwave/kwavesignal.h>

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define processid	0
#define stopprocess	1
#define samplepointer	2

int play16bit=false;
int bufbase=5;
const char *sounddevice={"/dev/dsp"};
//**********************************************************
// now some helper functions... somewhere in c-lib there should be something
// compareable. But as long the reinvented wheel works there is no need for
// change 
static inline int swapEndian (int s)
  //yes you guessed it only for sizeof(int)==4 this
  //works as it should do ...
{
 return ((s&0xff)<<24)|((s&0xff00)<<8)|((s&0xff0000)>>8)|((s%0xff000000)>>24);
}
//**********************************************************
static inline long swapEndian (long s)
{
 return ((s&0xff)<<24)|((s&0xff00)<<8)|((s&0xff0000)>>8)|((s%0xff000000)>>24);
}
//**********************************************************
static inline short int swapEndian (short int s)
  //sizeof (short int ) must be 2
{
 return ((s&0xff)<<8)|((s&0xff00)>>8);
}
//**********************************************************
//following are the playback routines 
struct Play
{
  SignalManager *manage;
  bool          loop;
};
//**********************************************************
void playThread (struct Play *par)
{
  if (play16bit) par->manage->play16 (par->loop);
  else par->manage->play8 (par->loop);
  
  delete par;
}
//**********************************************************
void SignalManager::play (bool loop)
{
  
  Play *par=new Play;
  pthread_t thread;
  
  par->manage=this;
  par->loop=loop;

  pthread_create (&thread,0,(void *  (*) (void *))playThread,par);
}
//**********************************************************
void SignalManager::stopplay ()
{
  if (msg[processid]>=0)	//if there is a process running
    msg[stopprocess]=true;	//set flag for stopping
}
//**********************************************************
int SignalManager::setSoundParams (int audio,int bitspersample,int channels,int rate,int bufbase)
{
  if ( ioctl(audio,SNDCTL_DSP_SAMPLESIZE,&bitspersample)!=-1)
    {
      if ( ioctl(audio,SNDCTL_DSP_STEREO,&channels)!=-1)
	{
	  if (ioctl(audio,SNDCTL_DSP_SPEED,&rate)!=-1) 
	    {
	      if (ioctl(audio,SNDCTL_DSP_SETFRAGMENT,&bufbase)!=-1) 
		{
		  int size;
		  ioctl(audio,SNDCTL_DSP_GETBLKSIZE,&size);
		  return size;
		}
	      else debug ("unusable buffer size\n");
	    }
	  else debug ("unusable rate\n");
	}
      else debug ("wrong number of channels\n");
    }
  else debug ("number of bits per samples not supported\n");
  return 0;
}
//**********************************************************
void SignalManager::play8 (bool loop)
{
  debug("SignalManager::play8 (%d)", loop); // ###

  int 	audio;			//file handle for /dev/dsp
  int   act;
  char	*buffer=0;
  int	bufsize;

  msg[stopprocess]=false;
  if ((audio=open(sounddevice,O_WRONLY)) != -1 )
    {
      bufsize=setSoundParams(audio,8,0,rate,bufbase);
 
      if (bufsize)
	{
	  buffer=new char[bufsize];
	  memset(buffer, 0, bufsize);
		
	  if (buffer)
	    {
	      int &pointer=msg[samplepointer];
	      int last=rmarker;
	      int cnt=0;
	      pointer=lmarker;

	      if (loop)
		{
		  if (lmarker==rmarker) last=getLength();

		  while (msg[stopprocess]==false)
		    {
		      for (cnt=0;cnt<bufsize;cnt++)
			{
			  if (pointer>=last) pointer=lmarker;
			  act=signal[0]->getSingleSample(pointer);
			  for (int i=1;i<channels;i++)
			    act+=signal[i]->getSingleSample(pointer);
			  act/=channels;
			  act+=1<<23;
			  buffer[cnt]=act>>16;
			  pointer++;
			}
		      write (audio,buffer,bufsize);
		    }
		}
	      else
		{
		  if (last==pointer) last=getLength();

		  while ((last-pointer>bufsize)&&(msg[stopprocess]==false))
		    {
		      for (cnt=0;cnt<bufsize;cnt++)
			{

			  act=signal[0]->getSingleSample(pointer);
			  for (int i=1;i<channels;i++)
			    act+=signal[i]->getSingleSample(pointer);

			  act/=channels;
			  act+=1<<23;
			  buffer[cnt]=act>>16;
			  pointer++;
			}

		      write (audio,buffer,bufsize);
		    }
		  if (msg[stopprocess]==false)
		    {
		      for (cnt=0;cnt<last-pointer;cnt++)
			{
			  act=signal[0]->getSingleSample(pointer);
			  for (int i=1;i<channels;i++)
			    act+=signal[i]->getSingleSample(pointer);

			  act/=channels;
			  act+=1<<23;
			  buffer[cnt]=act>>16;
			  pointer++;
			}
		      for (;cnt<bufsize;cnt++) buffer[cnt]=128; // fill up last buffer
		      write (audio,buffer,bufsize);
		    }
		}
	    }
	}

      close (audio);
      if (buffer) delete [] buffer;
      msg[stopprocess]=false;
      msg[samplepointer]=0;
    }
}
//**********************************************************
void SignalManager::play16 (bool loop)
{
  debug("SignalManager::play16 (%d)", loop); // ###
  int 	        audio;			//file handle for /dev/dsp
  unsigned char *buffer=0;
  int	        bufsize;

  msg[stopprocess]=false;
  if ( (audio=open(sounddevice,O_WRONLY)) != -1 )
    {
      bufsize=setSoundParams(audio,16,0,rate,bufbase);

      buffer=new unsigned char[bufsize];
      memset(buffer, 0, bufsize);

      if (buffer)
	{
	  int	&pointer=msg[samplepointer];
	  int	last=rmarker;
	  int	act,cnt=0;
	  pointer=lmarker;
	  if (last==pointer) last=getLength();

	  if (loop)
	    {
	      while (msg[stopprocess]==false)
		{
		  for (cnt=0;cnt<bufsize;)
		    {
		      if (pointer>=last) pointer=lmarker;

		      act=signal[0]->getSingleSample(pointer);
		      for (int i=1;i<channels;i++)
			act+=signal[i]->getSingleSample(pointer);

		      act/=channels;
		      act+=1<<23;
		      buffer[cnt++]=act>>8;
		      buffer[cnt++]=(act>>16)+128;
		      pointer++;
		    }
		  write (audio,buffer,bufsize);
		}
	    }
	  else
	    {
	      while ((last-pointer>bufsize)&&(msg[stopprocess]==false))
		{
		  for (cnt=0;cnt<bufsize;)
		    {
		      act=signal[0]->getSingleSample(pointer);
		      for (int i=1;i<channels;i++)
			act+=signal[i]->getSingleSample(pointer);

		      act/=channels;
		      act+=1<<23;
		      buffer[cnt++]=act>>8;
		      buffer[cnt++]=(act>>16)+128;
		      pointer++;
		    }      
		  write (audio,buffer,bufsize);
		}
	      if (msg[stopprocess]==false)  // playing not aborted and still something left, so play the rest...
		{
		  for (cnt=0;cnt<last-pointer;cnt++)
		    {
		      act=signal[0]->getSingleSample(pointer);
		      for (int i=1;i<channels;i++)
			act+=signal[i]->getSingleSample(pointer);

		      act/=channels;
		      act+=1<<23;
		      buffer[cnt++]=act>>8;
		      buffer[cnt++]=(act>>16)+128;
		      pointer++;
		    }      

		  for (;cnt<bufsize;)
		    {
		      buffer[cnt++]=0;
		      buffer[cnt++]=0;
		    }
		  write (audio,buffer,bufsize);
		}
	    }
	}
      close (audio);
      if (buffer) delete [] buffer; 
      msg[stopprocess]=false;
      msg[samplepointer]=0;
    }
}
//**********************************************************






















