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
SignalManager::SignalManager (QWidget *par,const char *name,int type)
// constructor that loads a signal file to memory 
{
  initialize();
  parent=par;

  this->name=duplicateString (name);

  switch (type)
    {
    case WAV:
      loadWav();
      break;
    case ASCII:
      loadAscii();
      break;
    }
}
//**********************************************************
void SignalManager::loadAscii ()
  //import ascii files with one sample per line, everything unparseable
  //by strtod is ignored
{
  FILE *sigin=fopen(name,"r");

  if (sigin)
    {
      float value;

      int cnt=0;
      float max=0;
      float min=INT_MAX;

      //loop over all samples in file to get maximum and minimum
      if (fscanf (sigin,"%e\n",&value)!=0)
      {
	if (value>max) max=value;
	if (value<min) min=value;
	cnt++;
      }

      float amp=max;
      if (-min>max) amp=-min;

      fseek (sigin,0,SEEK_SET); //seek to beginning

      rate=10000;     //will be asked in requester
      channels=1;

      signal[0]=new KwaveSignal (cnt,rate);
      int *sample = signal[0]->getSample();
      if (sample)
      {
	cnt=0;

	if (fscanf (sigin,"%e\n",&value)!=0)
	  sample[cnt++]=(int)(value/amp*((1<<23)-1));
	}
      fclose (sigin);
    }
  else printf ("File does not exist !\n");
}
//**********************************************************
void SignalManager::loadWav ()
{
  FILE *sigfile=fopen (name,"r");

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
		      debug ("length is %d,res is %d",length,res);

		      length=(length/(header.bitspersample/8))/header.channels;
		      switch (header.bitspersample)
			{
			case 8:
			case 16:
			case 24:
			  loadWavChunk(sigfile,length, header.channels,header.bitspersample);
			  break;
			default:
			  KMsgBox::message
			    (parent,"Info","Sorry only 8/16/24 Bits per Sample are supported !",2);
			  break;	
			}
		      channels=header.channels;

		    }
		}
	      else KMsgBox::message (parent,"Info","File must be uncompressed (Mode 1) !",2);
	    }
	  else  KMsgBox::message (parent,"Info","File is no RIFF Wave File !",2);

	}
      else  KMsgBox::message (parent,"Info","File does not contain enough data !",2);
      fclose(sigfile);
    }
  else  KMsgBox::message (parent,"Info","File does not exist !",2);
}
//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
//**********************************************************
void SignalManager::exportAscii ()
{
  //  QString name=QFileDialog::getSaveFileName (0,"*.dat",parent);
  const char *name="test.dat";
  if (name)
    {
      char buf[64];
      int length=getLength();

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
void SignalManager::writeWavChunk(FILE *sigout,int begin,int length,int bits)
{
  unsigned int bufsize=16*1024*sizeof(int);
  unsigned char *savebuffer = 0;
  int **sample=0; // array of pointers to samples
  int bytes=bits/8;
  int bytes_per_sample=bytes * channels;
  bufsize -= bufsize % bytes_per_sample;

  // try to allocate memory for the save buffer
  // if failed, try again with the half buffer size as long
  // as "0" is not reached (then we are really out of memory)
  while (savebuffer==0)
    {
      if (bufsize<=1024)
	{
	  debug("SignalManager::writeWavSignal:not enough memory for buffer");
	  return;
	}
      savebuffer = new unsigned char[bufsize];
      if (!savebuffer)
	{
	  bufsize>>=1;
	  bufsize-=bufsize % bytes_per_sample;
	}
    }

  //prepare and show the progress dialog
  char progress_title[64];
  char str_channels[32];
  if (channels==1)
    strcpy((char *)&str_channels,"Mono");
  else if (channels==2)
    strcpy((char *)&str_channels,"Stereo");
  else
    sprintf((char *)&str_channels,"%d-channel", channels);

  sprintf((char *)&progress_title,"Saving %d-Bit-%s File :",
    bits, str_channels);

  char *title = duplicateString(klocale->translate(progress_title));
  ProgressDialog *dialog=new ProgressDialog (100,title);
  delete title;
  if (dialog) dialog->show();

  //prepare the store loop
  int percent_count = length/200;
  unsigned int shift=24-bits;

  sample=new (int*)[channels];
  for (int channel=0; channel<channels; channel++)
    sample[channel]=signal[channel]->getSample();

  //loop for writing data
  for (int pos=begin;pos<length;)
    {
      unsigned char *buf = savebuffer;
      unsigned int nsamples=0;

      while (pos<length && (nsamples<bufsize/bytes_per_sample))
	{
	  for (int channel=0;channel<channels;channel++)
	    {
	      int *smp=sample[channel] + pos;
	      int act=(*smp) >> shift;
	      for (register int byte=bytes; byte; byte--)
		{
		  *(buf++) = (char)(act&0xFF);
		  act >>= 8;
		}
	    }
	  nsamples++;
	  pos++;
	}

      int written_bytes = fwrite(savebuffer,
	bytes_per_sample,nsamples,sigout);

      percent_count -= written_bytes;
      if (dialog && (percent_count <= 0))
	{
	  percent_count=length/200;
	  float percent=(float)pos;
	  percent/=(float)length;
	  percent*=100.0;
	  dialog->setProgress (percent);
	}
    }

  if (sample) delete sample;
  if (dialog) delete dialog;
  if (savebuffer) delete savebuffer;
}
//**********************************************************
void  SignalManager::save (const char *filename,int bits,bool selection)
{
  printf ("saving %d Bit to %s ,%d\n",bits,filename,selection);
  int begin=0;
  int length=this->getLength();
  struct wavheader header;

  if (selection && (lmarker!=rmarker))
    {
      begin=lmarker;
      length=rmarker-lmarker+1;
    }

  FILE *sigout=fopen (filename,"w");
  if (name) deleteString (name);
  name=duplicateString (filename);

  if (sigout)
    {
      fseek (sigout,0,SEEK_SET);

      strncpy (header.riffid,"RIFF",4);
      strncpy (header.wavid,"WAVE",4);
      strncpy (header.fmtid,"fmt ",4);
      header.fmtlength=16;
      header.filelength=(length*bits/8*channels+sizeof(struct wavheader));
      header.mode=1;
      header.channels=channels;
      header.rate=rate;
      header.AvgBytesPerSec=rate*bits/8*channels;
      header.BlockAlign=bits*channels/8;
      header.bitspersample=bits;

      int datalen=length*header.channels*header.bitspersample/8;

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
      fwrite ("data",1,4,sigout);
      fwrite ((char *)&datalen,1,4,sigout);

      switch (bits)
	{
	case 8:
	case 16:
	case 24:
	  writeWavChunk(sigout,begin,length,bits);
	  break;
	default:
	  KMsgBox::message(parent,"Info",
	    "Sorry only 8/16/24 Bits per Sample are supported !",2);
	  break;
	}

      fclose(sigout);
    }
}
//**********************************************************
/**
 * Reads in the wav data chunk from a .wav-file. It creates
 * a new empty KwaveSignal for each channel and fills it with
 * data read from an opened file. The file's actual position
 * must already be set to the correct position.
 * <p>
 * \param sigfile FILE* pointer to the already opened file
 * \param channels int number of channels [1,2,...]
 * \param bits resolution in bits [8,16,24]
 */
void SignalManager::loadWavChunk(FILE *sigfile,int length,int channels,int bits)
{
//int bufsize=PROGRESS_SIZE*sizeof(int) << 1;
  unsigned int bufsize=16*1024*sizeof(int);
  unsigned char *loadbuffer = 0;
  int **sample=0; // array of pointers to samples

  // try to allocate memory for the load buffer
  // if failed, try again with the half buffer size as long
  // as "0" is not reached (then we are really out of memory)
  while (loadbuffer==0)
    {
      if (bufsize<=1024)
	{
	  debug("SignalManager::loadWavSignal:not enough memory for buffer");
	  return;
	}
      loadbuffer = new unsigned char[bufsize];
      if (!loadbuffer) bufsize>>=1;
    }

  sample=new (int*)[channels];
  for (int channel=0; channel<channels; channel++)
    {
      signal[channel]=new KwaveSignal (length,rate);
      signal[channel]->setBits(bits);
      selected[channel]=true;
      sample[channel]=signal[channel]->getSample();
    }

  //prepare and show the progress dialog
  char progress_title[64];
  char str_channels[32];
  if (channels==1)
    strcpy((char *)&str_channels,"Mono");
  else if (channels==2)
    strcpy((char *)&str_channels,"Stereo");
  else
    sprintf((char *)&str_channels,"%d-channel", channels);

  sprintf((char *)&progress_title,"Loading %d-Bit-%s File :",
    bits, str_channels);
  char *title = duplicateString(klocale->translate(progress_title));
  ProgressDialog *dialog=new ProgressDialog (100,title);
  delete title;

  if (dialog) dialog->show();

  //prepare the load loop
  int percent_count=length/1000;
  int bytes=bits/8;
  unsigned int sign=1 << (24-1);
  unsigned int negative=~(sign-1);
  unsigned int shift=24-bits;
  unsigned int bytes_per_sample=bytes*channels;
  unsigned int max_samples=bufsize/bytes_per_sample;
  long int start_offset=ftell(sigfile);

  // debug("sign=%08X, negative=%08X, shift=%d",sign,negative,shift);

  for (int pos=0;pos<length;)
    {
      int read_samples=fread((char *)loadbuffer,bytes_per_sample,
	max_samples,sigfile);
      percent_count -= read_samples;
      // debug("read %d samples", read_samples);
      if (read_samples<=0)
	{
	  debug("SignalManager::loadWavChunk:EOF reached?"\
	    " (at sample %ld, expected length=%d",
	    ftell(sigfile)/bytes_per_sample-start_offset, length);
	  break;
	}

      unsigned char *buffer=loadbuffer;
      while (read_samples--)
	{
	  for (register int channel=0; channel<channels; channel++)
	    {
	      register int *smp=sample[channel] + pos;

	      for (register int byte=0; byte<bytes; byte++)
		*smp|=*(buffer++) << ((byte<<3)+shift);

	      // sign correcture for negative values
	      if ((unsigned int)*smp & sign)
		*smp|=negative;
	    }
	  pos++;
	}

      if (dialog && (percent_count <= 0))
	{
	  percent_count=length/1000;
	  float percent=(float)pos;
	  percent/=(float)length;
	  percent*=100.0;
	  dialog->setProgress (percent);
	}
    }

  if (dialog) delete dialog;
  if (loadbuffer) delete loadbuffer;
  if (sample) delete sample;
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

















