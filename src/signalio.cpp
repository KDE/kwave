//This file includes all methods of Class SignalManager that deal with I/O
//such as loading and saving. audio playback is now in signalplay.cpp
#include <unistd.h>
#include <fcntl.h>
#include <kmsgbox.h>
#include <endian.h>
#include <limits.h>
#include <qfiledialog.h>
#include "dialog_progress.h"
#include "signalmanager.h"
#include <libkwave/kwavesignal.h>
#include <libkwave/fileformat.h>

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define processid	0
#define stopprocess	1
#define samplepointer	2

//**********************************************************
// now some helper functions... somewhere in c-lib there should be something
// compareable. But as long the reinvented wheel works there is no need for
// change 
static inline unsigned int swapEndian (unsigned int s)
  //yes you guessed it only for sizeof(int)==4 this
  //works as it should do ...
{
 return ((s&0xff)<<24)|((s&0xff00)<<8)|((s&0xff0000)>>8)|((s%0xff000000)>>24);
}

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
SignalManager::SignalManager (QWidget *par,const char *name,int channel,int type) :QObject ()
// constructor that loads a signal file to memory 
{
  parent=par;
  lmarker=0;
  rmarker=0;

  this->name=duplicateString (name);

  switch (type)
    {
    case WAV:
      loadWav (channel);
      break;
    case ASCII:
      loadAscii(channel);
      break;
    }
}
//**********************************************************
void SignalManager::loadAscii (int channel)
  //import ascii files with one sample per line, everything unparseable
  //by strtod is ignored
{
  QFile *sigin=new QFile (name);
  if (sigin->exists())
    {
      if (sigin->open(IO_ReadWrite))
	{
	  char buf[80];
	  float value;

	  int cnt=0;
	  float max=0;
	  float min=INT_MAX;

	  while (sigin->readLine(&buf[0],80)>0)
	    {
	      if (sscanf (buf,"%e",&value)!=0)
		{
		  if (value>max) max=value;
		  if (value<min) min=value;
		  cnt++;
		}
	    }

	  float amp=max;
	  if (-min>max) amp=-min;

	  sigin->at(0); //seek to beginning

	  rate=10000;	//will be asked in requester 
	  length=cnt;
	  channels=1;

	  signal[0]=new KwaveSignal (length,rate);
	  int *sample = signal[0]->getSample();
	  if (sample)
	    {
	      cnt=0;
	      while (sigin->readLine(&buf[0],80)>0)
		{
		  if (sscanf (buf,"%e",&value)!=0)
		    sample[cnt++]=(int)(value/amp*((1<<23)-1));
		}
	    }
	}
    }
  else  KMsgBox::message (parent,"Info","File does not exist !",2);
}
//**********************************************************
void SignalManager::loadWav (int channel)
{
  QFile *sigin=new QFile (name);
  if (sigin->exists())
    {
      union
      {
	char      rheader[sizeof(struct wavheader)];
	wavheader header;
      };
      union 
      {
	char     rlength [4];
	int	 length;
      };

      sigin->open(IO_ReadOnly);
      int num=sigin->readBlock (rheader,sizeof(wavheader));
      if (num==sizeof(struct wavheader))
	{
	  if ((strncmp("RIFF",header.riffid,4)==0)&&
	      (strncmp("WAVE",header.wavid,4)==0)&&(strncmp("fmt ",header.fmtid,4)==0))
	    {
#if defined(IS_BIG_ENDIAN)
	      header.mode 	= swapEndian (header.mode);
	      header.rate 	= swapEndian (header.rate);
	      header.channels = swapEndian (header.channels);
	      header.bitspersample = swapEndian (header.bitspersample);
#endif
	      if (header.mode==1)
		{
		  rate=header.rate;
		  findDatainFile (sigin);
		  if (sigin->atEnd()) KMsgBox::message (parent,"Info","File contains no data chunk!",2);
		  else
		    {
		      sigin->readBlock (rlength,4);
#if defined(IS_BIG_ENDIAN)
		      length=swapEndian (length);	
#endif

		      this->length=(length/(header.bitspersample/8))/header.channels;
		      switch (header.bitspersample)
			{
			case 8:
			  load8Bit (sigin,(channel-1)*(header.bitspersample/8),(header.channels-1)*(header.bitspersample/8));
			  break;
			case 16:
			  if (header.channels==2) loadStereo16Bit (sigin);
			  else load16Bit (sigin,(channel-1)*(header.bitspersample/8),(header.channels-1)*(header.bitspersample/8));
			  break;
			case 24:
			  load24Bit (sigin,(channel-1)*(header.bitspersample/8),(header.channels-1)*(header.bitspersample/8));
			  break;
			default:
			  KMsgBox::message
			    (parent,"Info","Sorry only 8/16/24 Bits per Sample are supported !",2);
			  break;	
			}
		      channels=header.channels;

		      info ();
		      emit channelReset();
		    }
		}
	      else KMsgBox::message (parent,"Info","File must be uncompressed (Mode 1) !",2);
	    }
	  else  KMsgBox::message (parent,"Info","File is no RIFF Wave File !",2);

	}
      else  KMsgBox::message (parent,"Info","File does not contain enough data !",2);
    }
  else  KMsgBox::message (parent,"Info","File does not exist !",2);
}
//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
void writeData8Bit (QFile *sigout,int begin,int end,SignalManager *manage)
{
  int channels=manage->getChannels();

  char o;
  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  o=128+(char)(manage->getSingleSample(j,i)/65536);
	  sigout->putch (o);
	}
    }
}
//**********************************************************
void SignalManager::exportAscii ()
{
  QString name=QFileDialog::getSaveFileName (0,"*.dat",parent);
  if (!name.isNull())
    {
      char buf[64];

      QFile sigout(name.data());
      sigout.open (IO_WriteOnly);

      sprintf (buf,"%d\n",length);
      sigout.writeBlock (&buf[0],strlen(buf));

      sigout.writeBlock ("1.00\n",5);

      int *sample = signal[0]->getSample();
      if (sample)
	{
	  for (int i=0;i<length;i++)
	    {
	      sprintf (buf,"%d\n",sample[i]/(1<<8));
	      sigout.writeBlock (&buf[0],strlen(buf));
	    }
	}
    }
}
//**********************************************************
void writeData16Bit (QFile *sigout,int begin,int end,SignalManager *manage)
{
  int channels=manage->getChannels();
  int act;

  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  act=(manage->getSingleSample(j,i));
	  act+=1<<23;

#if defined(IS_BIG_ENDIAN)
	  sigout->putch ((char)((act>>16)+128));
	  sigout->putch ((char)(act>>8));
#else
	  sigout->putch ((char)(act>>8));
	  sigout->putch ((char)((act>>16)+128));
#endif
	}
    }
}
//**********************************************************
void writeData24Bit (QFile *sigout,int begin,int end,SignalManager *manage)
{
  int channels=manage->getChannels();
  int act;

  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  act=(manage->getSingleSample(j,i));

#if defined(IS_BIG_ENDIAN)
	  sigout->putch ((char)(act/65536));
	  sigout->putch ((char)(act/256));
	  sigout->putch ((char)(act));
#else
	  sigout->putch ((char)(act));
	  sigout->putch ((char)(act/256));
	  sigout->putch ((char)(act/65536));
#endif
	}
    }
}
//**********************************************************
void  SignalManager::save (const char *filename,int bit,int selection)
{
  printf ("saving %d Bit to %s ,%d\n",bit,filename,selection);
  int begin=0;
  int endp=this->length;
  struct wavheader header;
  int **samples=new int* [channels];

  if (selection)
    if (lmarker!=rmarker)
      {
	begin=lmarker;
	endp=rmarker;
      }

  QFile	*sigout=new QFile (filename);
  if (name) deleteString (name);
  name=duplicateString (filename);

  for (int i=0;i<channels;i++)
    samples[i]=signal[i]->getSample();

  if (sigout&&samples)
    {
      if (sigout->exists())
	{
	  debug ("File Exists overwriting !\n");
	}

      sigout->open (IO_WriteOnly);
      sigout->at (0);

      strncpy (header.riffid,"RIFF",4);
      strncpy (header.wavid,"WAVE",4);
      strncpy (header.fmtid,"fmt ",4);
      header.fmtlength=16;
      header.filelength=((endp-begin)*bit/8*channels+sizeof(struct wavheader));
      header.mode=1;
      header.channels=channels;
      header.rate=rate;
      header.AvgBytesPerSec=rate*bit/8*channels;
      header.BlockAlign=bit*channels/8;
      header.bitspersample=bit;

      int datalen=(endp-begin)*header.channels*header.bitspersample/8;

#if defined(IS_BIG_ENDIAN)
      header.mode 		= swapEndian (header.mode);
      header.rate 		= swapEndian (header.rate);
      header.channels		= swapEndian (header.channels);
      header.bitspersample	= swapEndian (header.bitspersample);
      header.AvgBytesPerSec	= swapEndian (header.AvgBytesPerSec);
      header.fmtlength	= swapEndian (header.fmtlength);
      header.filelength	= swapEndian (header.filelength);
      datalen= swapEndian (datalen);
#endif

      sigout->writeBlock ((char *) &header,sizeof (struct wavheader));
      sigout->writeBlock ("data",4);
      sigout->writeBlock ((char *)&datalen,4);

      switch (bit)
	{
	case 8:
	  writeData8Bit (sigout,begin,endp,this);
	  break;
	case 16:
	  writeData16Bit (sigout,begin,endp,this);
	  break;
	case 24:
	  writeData24Bit (sigout,begin,endp,this);
	  break;
	}
      delete []samples;
      sigout->close();
    }
  if (sigout) delete sigout;
}
//**********************************************************
void SignalManager::load24Bit (QFile *sigin,int offset,int interleave)
{
  unsigned char *loadbuffer = new unsigned char[PROGRESS_SIZE*3];

  signal[0]=new KwaveSignal (length,rate);
  int *sample = signal[0]->getSample();
  if (sample)

  if ((sample)&&(loadbuffer))
    {
      ProgressDialog *dialog=new ProgressDialog (length,"Loading 24-Bit File :");

      if (dialog)
	{
	  dialog->show();
	  int i,j,pos;
	  if (interleave==0)
	    {
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;)
		{
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;//determine number of sample to fill buffer with...
		  else j=length;

		  sigin->readBlock((char *)loadbuffer,(j-i)*3);
		  pos=0;

		  for (;i<j;i++)
		    {
		      //		      sample[i]=(sigin->getch()+sigin->getch()<<8)+(sigin->getch()<<16);
		      sample[i]=loadbuffer[pos++];
		      sample[i]+=loadbuffer[pos++]<<8;
		      sample[i]+=loadbuffer[pos++]<<16;
		      
		      if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		    }
		  dialog->setProgress (i);
		}
	    }
	  else
	    {	//non zero interleave for multichannel-files !
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=(sigin->getch()+sigin->getch()<<8)+(sigin->getch()<<16);
		  if (sample[i]>(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  sigin->at(sigin->at()+interleave);
		}
	    }
	  delete dialog;
	}
      if (loadbuffer) delete loadbuffer;
    }
}
//**********************************************************
void SignalManager::load16Bit (QFile *sigin,int offset,int interleave)
{
  unsigned char *loadbuffer = new unsigned char[PROGRESS_SIZE*2];

  signal[0]=new KwaveSignal (length,rate);
  int *sample = signal[0]->getSample();

  if ((sample)&&(loadbuffer))
    {
      ProgressDialog *dialog=new ProgressDialog (length,"Loading 16-Bit File :");
      dialog->show();

      if (dialog)
	{
	  int i,j,pos=0;

	  if (interleave==0)
	    {
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;)
		{
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
		  else j=length;

		  sigin->readBlock((char *)loadbuffer,(j-i)*2);
		  pos=0;

		  for (;i<j;i++)
		    {
		      sample[i]=loadbuffer[pos++]<<8;
		      sample[i]+=loadbuffer[pos++]<<16;

		      //convert to signed int
		      if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		    }
		  dialog->setProgress (i);
		}
	    }
	  else
	    {	//non zero interleave for multichannel-files !
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=loadbuffer[pos++]<<8;
		  sample[i]+=loadbuffer[pos++]<<16;
		  if (sample[i]>(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  sigin->at(sigin->at()+interleave);
		}
	    }
	  delete dialog;
	}
      if (loadbuffer) delete loadbuffer;
    }
}
//**********************************************************
void  SignalManager::loadStereo16Bit (QFile *sigin)
  //explicit routine for faster loading of 16 bit stereo files,
  //since they are widely-spread...
{
  //the current object should be the first channel....

  signal[0]=new KwaveSignal (length,rate);
  int *sample = signal[0]->getSample();

  signal[1]=new KwaveSignal (length,rate);
  int *lsample = signal[1]->getSample();

  unsigned char *loadbuffer = new unsigned char[PROGRESS_SIZE*4];

  if (lsample&&sample&&loadbuffer)
    {
      ProgressDialog *dialog=new ProgressDialog (length,"Loading Stereo 16-Bit File :");
      dialog->show();

      if (dialog)
	{
	  int i,j,pos;
	  for (i=0;i<length;)
	    {
	      if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
	      else j=length;

	      sigin->readBlock((char *)loadbuffer,(j-i)*4);
	      pos=0;

	      for (;i<j;i++)
		{
		  sample[i]=loadbuffer[pos++]<<8;
		  sample[i]+=loadbuffer[pos++]<<16;
		  lsample[i]=loadbuffer[pos++]<<8;
		  lsample[i]+=loadbuffer[pos++]<<16;

		  if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  if (lsample[i]>=(1<<23)) lsample[i]=(-(1<<24))+lsample[i];
		}
	      dialog->setProgress (i);
	    }
	  delete dialog;
	}
    }
  if (loadbuffer) delete loadbuffer;
}
//**********************************************************
void SignalManager::load8Bit (QFile *sigin,int offset,int interleave)
{
  signal[0]=new KwaveSignal (length,rate);
  int *sample = signal[0]->getSample();

  if (sample)
    {
      ProgressDialog *dialog=new ProgressDialog (length,"Loading 8-Bit File :");
      dialog->show();

      if (dialog)
	{
	  int i,j;

	  if (interleave==0)
	    {
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;)
		{
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
		  else j=length;

		  for (;i<j;i++)
		    {
		      sample[i]=(((int) sigin->getch())-128)*(1<<16);
		      if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		    }
		  dialog->setProgress (i);
		}
	    }
	  else
	    {	//non zero interleave for multichannel-files !
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=(((int) sigin->getch())-128)*(1<<16);
		  if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  sigin->at(sigin->at()+interleave);
		}
	    }
	  delete dialog;
	}
    }
}
//**********************************************************
void SignalManager::findDatainFile (QFile *sigin)
  //help function for finding data chunk in wav files
{
  char	buffer[4096];
  int 	point=0,max=0;
  int	filecount=sigin->at();
  while ((point==max)&&(!sigin->atEnd()))
    {
      max=sigin->readBlock (buffer,4096);
      point=0;
      while (point<max) if (strncmp (&buffer[point++],"data",4)==0) break;

      if (point==max)
	{
	  filecount+=point-4;	//rewind 4 Bytes;
	  sigin->at(filecount);
	}
      else filecount+=point;
    }
  if (point!=max)
    {
      sigin->at(filecount+3);
    }
}


















