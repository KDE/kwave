//This file includes all methods of Class SignalManager that deal with I/O
//such as loading and saving. audio playback is now in signalplay.cpp
#include <unistd.h>
#include <fcntl.h>
#include <kmsgbox.h>
#include <endian.h>
#include <limits.h>
#include "dialog_progress.h"
#include "signalmanager.h"
#include <libkwave/kwavesignal.h>
#include <libkwave/fileformat.h>
#include <libkwave/globals.h>

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define processid	0
#define stopprocess	1
#define samplepointer	2

extern Global globals;
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
SignalManager::SignalManager (QWidget *par,const char *name,int channel,int type)
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
  //  QFile *sigin=new QFile (name);
  FILE *sigin=fopen(name,"r");

  if (sigin)
    {
      //      char buf[80];
      float value;

      int cnt=0;
      float max=0;
      float min=INT_MAX;

      //	  while (sigin->readLine(&buf[0],80)>0)
      //	    {

      //loop over all samples in file to get maximum and minimum
      if (fscanf (sigin,"%e\n",&value)!=0)
	//	      if (sscanf (buf,"%e",&value)!=0)
	{
	  if (value>max) max=value;
	  if (value<min) min=value;
	  cnt++;
	  //		}
	}

      float amp=max;
      if (-min>max) amp=-min;

      fseek (sigin,0,SEEK_SET);
      //      sigin->at(0); //seek to beginning

      rate=10000;	//will be asked in requester
      length=cnt;
      channels=1;

      signal[0]=new KwaveSignal (length,rate);
      int *sample = signal[0]->getSample();
      if (sample)
	{
	  cnt=0;

	  if (fscanf (sigin,"%e\n",&value)!=0)
	    sample[cnt++]=(int)(value/amp*((1<<23)-1));
	  //	  while (sigin->readLine(&buf[0],80)>0)
	  //	    {
	  //	      if (sscanf (buf,"%e",&value)!=0)

	  //	    }
	}
      fclose (sigin);
    }
  else printf ("File does not exist !\n");
}
//**********************************************************
void SignalManager::loadWav (int channel)
{
  FILE *sigfile=fopen (name,"r");
  //  QFile *sigin=new QFile (name);

  if (sigfile)
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

      int num=fread (rheader,1,sizeof(wavheader),sigfile);
      printf ("%d %d\n",num,sizeof(wavheader));

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

		  int res=findDatainFile (sigfile);

		  if (res==0) printf("File contains no data chunk!\n");
		  else
		    {
		      fseek (sigfile,res,SEEK_SET);           //seek after DATA
		      fread (rlength,1,sizeof (int),sigfile); //load length of data chunk
#if defined(IS_BIG_ENDIAN)
		      length=swapEndian (length);	
#endif
		      printf ("length is %d,res is %d\n",length,res);

		      this->length=(length/(header.bitspersample/8))/header.channels;
		      switch (header.bitspersample)
			{
			case 8:
			  load8Bit (sigfile,(channel-1)*(header.bitspersample/8),(header.channels-1)*(header.bitspersample/8));
			  break;
			case 16:
			  if (header.channels==2) loadStereo16Bit (sigfile);
			  else load16Bit (sigfile,(channel-1)*(header.bitspersample/8),(header.channels-1)*(header.bitspersample/8));
			  break;
			case 24:
			  load24Bit (sigfile,(channel-1)*(header.bitspersample/8),(header.channels-1)*(header.bitspersample/8));
			  break;
			default:
			  KMsgBox::message
			    (parent,"Info","Sorry only 8/16/24 Bits per Sample are supported !",2);
			  break;	
			}
		      channels=header.channels;

		      info ();
		      globals.port->putMessage ("refreshchannels()");
		    }
		}
	      else printf ("File must be uncompressed (Mode 1) !");
	    }
	  else  printf ("File is no RIFF Wave File !");
	}
      else  printf ("File does not contain enough data !");
      fclose (sigfile);
    }
  else  printf ("File does not exist !");
}
//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
void writeData8Bit (FILE *sigout,int begin,int end,SignalManager *manage)
{
  int channels=manage->getChannelCount();

  char o;
  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  o=128+(char)(manage->getSingleSample(j,i)/65536);
	  fputc (o,sigout);
	}
    }
}
//**********************************************************
void SignalManager::exportAscii ()
{
  //  QString name=QFileDialog::getSaveFileName (0,"*.dat",parent);
  const char *name="test.dat";
  if (name)
    {
      char buf[64];

      QFile sigout(name);
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
void writeData16Bit (FILE *sigout,int begin,int end,SignalManager *manage)
{
  int channels=manage->getChannelCount();
  int act;

  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  act=(manage->getSingleSample(j,i));
	  act+=1<<23;

#if defined(IS_BIG_ENDIAN)
	  fputc ((char)((act>>16)+128),sigout);
	  fputc ((char)(act>>8),sigout);

	  //	  sigout->putch ((char)((act>>16)+128));
	  //sigout->putch ((char)(act>>8));
#else
	  fputc ((char)(act>>8),sigout);
	  fputc ((char)((act>>16)+128),sigout);
	  //	  sigout->putch ((char)(act>>8));
	  // sigout->putch ((char)((act>>16)+128));
#endif
	}
    }
}
//**********************************************************
void writeData24Bit (FILE *sigout,int begin,int end,SignalManager *manage)
{
  int channels=manage->getChannelCount();
  int act;

  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  act=(manage->getSingleSample(j,i));

#if defined(IS_BIG_ENDIAN)
	  fputc ((char)(act/65536),sigout);
	  fputc ((char)(act/256),sigout);
	  fputc ((char)(act),sigout);

	  //	  sigout->putch ((char)(act/65536));
	  //	  sigout->putch ((char)(act/256));
	  //	  sigout->putch ((char)(act));
#else
	  fputc ((char)(act),sigout);
	  fputc ((char)(act/256),sigout);
	  fputc ((char)(act/65536),sigout);

	  //	  sigout->putch ((char)(act));
	  // sigout->putch ((char)(act/256));
	  //sigout->putch ((char)(act/65536));
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

  FILE	*sigout=fopen (filename,"w");
  if (name) deleteString (name);
  name=duplicateString (filename);

  for (int i=0;i<channels;i++)
    samples[i]=signal[i]->getSample();

  if (sigout&&samples)
    {
      //      if (sigout->exists())
      //	{
      //	  debug ("File Exists overwriting !\n");
      //	}

      //      sigout->open (IO_WriteOnly);
      //      sigout->at (0);
      fseek (sigout,0,SEEK_SET);

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

      fwrite ((char *) &header,1,sizeof (struct wavheader),sigout);
      //      sigout->writeBlock ((char *) &header,sizeof (struct wavheader));
      fwrite ("data",1,4,sigout);
      //    sigout->writeBlock ("data",4);
      fwrite ((char *)&datalen,1,4,sigout);
      // sigout->writeBlock ((char *)&datalen,4);

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
      fclose (sigout);
	//      sigout->close();
    }
  if (sigout) delete sigout;
}
//**********************************************************
void SignalManager::load24Bit (FILE *sigin,int offset,int interleave)
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
	      fseek (sigin,(ftell(sigin)+offset),SEEK_SET);
	      for (i=0;i<length;)
		{
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;//determine number of sample to fill buffer with...
		  else j=length;

		  fread ((char *)loadbuffer,1,(j-i)*3,sigin);
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
	      fseek (sigin,(ftell(sigin)+offset),SEEK_SET);
	      //	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=(fgetc(sigin)+fgetc(sigin)<<8)+(fgetc(sigin)<<16);
		  //		  sample[i]=(sigin->getch()+sigin->getch()<<8)+(sigin->getch()<<16);
		  if (sample[i]>(1<<23)) sample[i]=(-(1<<24))+sample[i];
	      fseek (sigin,(ftell(sigin)+interleave),SEEK_SET);
	      //		  sigin->at(sigin->at()+interleave);
		}
	    }
	  delete dialog;
	}
      if (loadbuffer) delete loadbuffer;
    }
}
//**********************************************************
void SignalManager::load16Bit (FILE *sigin,int offset,int interleave)
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
	      fseek (sigin,(ftell(sigin)+offset),SEEK_SET);
	      //	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;)
		{
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
		  else j=length;

		  fread ((char *)loadbuffer,1,(j-i)*2,sigin);
		  //		  sigin->readBlock((char *)loadbuffer,(j-i)*2);
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
	      fseek (sigin,(ftell(sigin)+offset),SEEK_SET);
	      //sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=loadbuffer[pos++]<<8;
		  sample[i]+=loadbuffer[pos++]<<16;
		  if (sample[i]>(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  fseek (sigin,(ftell(sigin)+interleave),SEEK_SET);
	      //		  sigin->at(sigin->at()+interleave);
		}
	    }
	  delete dialog;
	}
      if (loadbuffer) delete loadbuffer;
    }
}
//**********************************************************
void  SignalManager::loadStereo16Bit (FILE *sigin)
  //explicit routine for faster loading of 16 bit stereo files,
  //since they are widely-spread...
{
  //the current object should be the first channel....

  printf ("loadstereo16bit...\n");

  signal[0]=new KwaveSignal (length,rate);
  int *sample = signal[0]->getSample();

  printf ("second channel coming up\n");

  signal[1]=new KwaveSignal (length,rate);
  printf ("signal is %p\n",signal[1]);  
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

	      fread ((char *)loadbuffer,1,(j-i)*4,sigin);
	      //	      sigin->readBlock((char *)loadbuffer,(j-i)*4);
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
void SignalManager::load8Bit (FILE *sigin,int offset,int interleave)
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
	      fseek (sigin,(ftell(sigin)+offset),SEEK_SET);
	      //	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;)
		{
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
		  else j=length;

		  for (;i<j;i++)
		    {
		      sample[i]=(fgetc(sigin)-128)*(1<<16);
		      //sample[i]=(((int) sigin->getch())-128)*(1<<16);

		      if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		    }
		  dialog->setProgress (i);
		}
	    }
	  else
	    {	//non zero interleave for multichannel-files !
	      fseek (sigin,(ftell(sigin)+offset),SEEK_SET);
	      //	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=(((int) fgetc(sigin))-128)*(1<<16);
		  //		  sample[i]=(((int) sigin->getch())-128)*(1<<16);
		  if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  fseek (sigin,(ftell(sigin)+interleave),SEEK_SET);
		  //sigin->at(sigin->at()+interleave);
		}
	    }
	  delete dialog;
	}
    }
}
//**********************************************************
int SignalManager::findDatainFile (FILE *sigin)
  //help function for finding data chunk in wav files
{
  char	buffer[4096];
  int 	point=0,max=0;
  int	filecount=ftell(sigin);
  int   res=(filecount>=0)?0:-1;
 
  while ((point==max)&&(res==0))
    {
      max=fread (buffer,1,4096,sigin);
      //      max=sigin->readBlock (buffer,4096);
      point=0;
      while (point<max) if (strncmp (&buffer[point++],"data",4)==0) break;

      if (point==max)
	{
	  filecount+=point-4;	//rewind 4 Bytes;
	  res=fseek (sigin,filecount,SEEK_SET);
	}
      else return filecount+point+3;
    }

  return 0;
}


















