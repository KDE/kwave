#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <endian.h>
#include <limits.h>
#include <math.h>

#include <kmsgbox.h>

#include <libkwave/Signal.h>
#include <libkwave/DynamicLoader.h>
#include <libkwave/TimeOperation.h>
#include <libkwave/Global.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/MessagePort.h>
#include <libkwave/Parser.h>
#include <libkwave/FileFormat.h>

#include "sampleop.h"

#include "ClipBoard.h"
#include "ProgressDialog.h"

#include "SignalManager.h"
#include "SignalWidget.h"

extern int play16bit;
extern int bufbase;
extern Global globals;

//**********************************************************
void SignalManager::getMaxMin (int channel,int&max, int &min,int begin,int len)
{
  int sig_len;

  if (channel>=channels)
    {
      debug("SignalManager::getMaxMin:warning: channel %d >= %d",
	channel, channels);
      return;
    }
  if (signal[channel]==0)
    {
      debug("SignalManager::getMaxMin:warning: signal[%d]==0", channel);
      return;
    }

  sig_len = signal[channel]->getLength();
  if (begin+len >= sig_len) len=sig_len-begin;
  signal[channel]->getMaxMin (max,min,begin,len);
}

//**********************************************************
int SignalManager::getLength()
{
    return signal[0] ? signal[0]->getLength() : 0;
}

//**********************************************************
int SignalManager::getSingleSample(int channel,int offset)
{
  return signal[channel] ?
    signal[channel]->getSingleSample(offset) : 0;
}

//**********************************************************
int SignalManager::getBitsPerSample()
{
  int max_bps=0;
  for (int i=0;i<channels;i++)
    {
      int bps = signal[i]->getBits();
      if (bps>max_bps) max_bps=bps;
    }
  return max_bps;
}

//**********************************************************
void SignalManager::deleteChannel (int channel)
{
  Signal *todelete=signal[channel];

  for (int cnt=channel;cnt<channels;cnt++)
    {
      signal[cnt]=signal[cnt+1];
      selected[cnt]=selected[cnt+1];
    }
  delete (todelete);
  channels--;              //decrease number of channels...

  globals.port->putMessage ("refreshchannels()");
  refresh ();              //and let everybody know about it
}

//**********************************************************
void SignalManager::addChannel ()
  //adds a channel with silence
{
  signal[channels]=new Signal (getLength(),rate);
  selected[channels]=true; //enable channel
  channels++;              //increase number of channels...
  refresh ();              //and let everybody know about it
}

//**********************************************************
void SignalManager::appendChannel (Signal *newsig)
{
  signal[channels]=newsig;
  selected[channels]=true; //enable channel
  channels++;
  refresh ();
}

//**********************************************************
void SignalManager::setOp (int id)
{
  stopplay();	//every operation cancels playing...

  //decode dynamical allocated menu id's
  //into the ones used by the switch statement below

  int loop=false;
  switch (id)
    {
      case LOOP:
      loop=true;
      case PLAY:
	play (loop);
      break;
    }

  //chaos is about to come
  //check for ranges of id that have to be decoded into a parameter

  if ((id>=TOGGLECHANNEL)&&(id<TOGGLECHANNEL+10)) toggleChannel (id-TOGGLECHANNEL);
}
//**********************************************************
void threadStub (TimeOperation *obj)
{
  (obj->getSignal())->command (obj);
}
//**********************************************************
int SignalManager::doCommand (const char *str)
{
  debug("SignalManager::doCommand(%s)\n", str); // ###

  if (matchCommand(str,"copy"))
    {
      if (globals.clipboard) delete globals.clipboard;
      globals.clipboard=new ClipBoard ();
      if (globals.clipboard)
	for (int i=0;i<channels;i++) globals.clipboard->appendChannel (signal[i]->copyRange());
    }
  else
    if (matchCommand(str,"cut"))
      {
	if (globals.clipboard) delete globals.clipboard;
	globals.clipboard=new ClipBoard ();
	if (globals.clipboard)
	  for (int i=0;i<channels;i++) globals.clipboard->appendChannel (signal[i]->cutRange());
	refresh ();
      }
    else
    if (matchCommand(str,"crop"))
      {
	if (globals.clipboard) delete globals.clipboard;
	globals.clipboard=new ClipBoard ();
	if (globals.clipboard)
	  for (int i=0;i<channels;i++) (signal[i]->cropRange());
	refresh ();
      }
    else
      if (matchCommand(str,"deletechannel"))
	{
	  Parser parser (str);
	  int i=atoi(parser.getFirstParam());
	  deleteChannel (i);
	}
      else
      if (matchCommand(str,"delete"))
	{
	  for (int i=0;i<channels;i++) signal[i]->deleteRange();
	  refresh ();
	}
      else
	if (matchCommand(str,"paste"))
	  {
	    if (globals.clipboard)
	      {
		SignalManager *toinsert=globals.clipboard->getSignal();
		if (toinsert)
		  {
		    int clipchan=toinsert->getChannelCount();
		    int sourcechan=0;

		    for (int i=0;i<channels;i++)
		      {
			signal[i]->insertPaste(
			  toinsert->getSignal(sourcechan++)
			);
			sourcechan %= clipchan;
		      }
		  }
		refresh ();
	      }
	  }
	if (matchCommand(str,"mixpaste"))
	  {
	    if (globals.clipboard)
	      {
		SignalManager *toinsert=globals.clipboard->getSignal();
		if (toinsert)
		  {
		    int clipchan=toinsert->getChannelCount();
		    int sourcechan=0;

		    for (int i=0;i<channels;i++)
		      {
			signal[i]->mixPaste(
			  toinsert->getSignal(sourcechan++)
			);
			sourcechan %= clipchan;
		      }
		  }
		refresh ();
	      }
	  }
	else
	  if (matchCommand (str,"selectchannels"))
	    for (int i=0;i<channels;i++) selected[i]=true;
	  else
	    if (matchCommand (str,"invertchannels"))
	      for (int i=0;i<channels;i++) selected[i]=!selected[i];
	    else
	      if (matchCommand(str,"addchannel")) addChannel ();
	      else
		return promoteCommand (str);

  return true;
}
//**********************************************************
bool SignalManager::promoteCommand (const char *command)
{
  int i;
  for (i=0;i<channels;i++)       //for all channels
    {
      if (!signal[i])   debug("signal[%d]==0\n", i);   // ###
      if (!selected[i]) debug("selected[%d]==0\n", i); // ###
      if ((signal[i])&&(selected[i]))  //that exist and are selected
	{
	  int begin, len;
	  if (lmarker!=rmarker)
	    {
	      begin=lmarker;
	      len=rmarker-lmarker+1;
	    }
	  else
	    {
	      begin=0;
	      len=signal[i]->getLength();
	    }

	  char buf[32];
	  sprintf (buf,"%d",i+1);
	  char *caption=catString (command," on channel ",buf);

	  //create a nice little Object that should contain everything important
	  TimeOperation *operation=
	    new TimeOperation (signal[i],command,begin,len);

	  if (operation)
	    {
#ifndef DISABLE_THREADS
	      //create a new progress dialog, that watches an memory address
	      //that is updated by the modules

	      ProgressDialog *dialog=createProgressDialog(operation,caption);
		if (dialog)
		{
		  pthread_t thread;
	  
		  //create the new thread 
		  if (pthread_create (&thread,
				      0,
				      (void *(*)(void *))(threadStub),
				      (void *)operation)!=0)
		    {
		      debug ("thread creation failed\n");
		      delete dialog;
		      return false;
		    }
		}
	      else debug ("out of memory: could not allocate ProgressDialog\n");
#else /* DISABLE_THREADS */
	      signal[i]->command(operation);
#endif /* DISABLE_THREADS */
	    }
	  else debug ("out of memory: could not allocate TimeOperation\n");
	}
    }

  if (i<channels) return false;
  //could not promote command to modules or an error occured
  else return true;
}
//**********************************************************
void SignalManager::refresh ()
{
  globals.port->putMessage ("refreshchannels()");
  globals.port->putMessage ("refresh()");
}
//**********************************************************
void SignalManager::initialize()
{
  name=0;
  parent=0;
  lmarker=0;
  rmarker=0;
  channels=0;
  rate=0;
  for (unsigned int i=0;i<sizeof(msg)/sizeof(msg[0]); i++) msg[i]=0;
  for (int i=0;i<MAXCHANNELS;i++)
    {
      signal[i]=0;
      selected[i]=false;
    }
}
//**********************************************************
SignalManager::SignalManager (Signal *sample)
{
  initialize();
  if (sample)
    {
      this->channels=1;
      this->rate=sample->getRate();

      signal[0]=sample;
      selected[0]=true;
    }
}
//**********************************************************
SignalManager::SignalManager (QWidget *parent,int numsamples,int rate,int channels)
{
  initialize();
  if (channels>MAXCHANNELS) channels=MAXCHANNELS;

  this->channels=channels; //store how many channels are linked to this
  this->rate=rate;
  this->parent=parent;

  for (int i=0;i<channels;i++)
    {
      signal[i]=new Signal (numsamples,rate);
      selected[i]=true;
    }
}
//**********************************************************
SignalManager::~SignalManager ()
{
  for (int channel=0; channel<MAXCHANNELS;channel++)
    {
      selected[channel]=false;
      if (signal[channel]) delete signal[channel];
      signal[channel]=0;
    }
  if (name) delete name;
}
//**********************************************************
void SignalManager::setRange (int l,int r )
  //this one sets the internal markers and promotes them to all channels
{
  for (int i=0;i<channels;i++)
    {
      if (signal[i])
	signal[i]->setMarkers (l,r);
      else
	warning("WARNING: channel[%d] is null", i);
    }
  lmarker=signal[0]->getLMarker();
  rmarker=signal[0]->getRMarker();
}
//**********************************************************
//below are all methods of Class SignalManager that deal with I/O
//such as loading and saving. audio playback is now in signalplay.cpp
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
void SignalManager::loadAscii()
{
    float value;
    int cnt=0;
    float max=0;
    float amp;
    int *sample;

    FILE *sigin=fopen(name,"r");
    if (!sigin) {
	printf ("File does not exist !\n");
	return;
    }

    //loop over all samples in file to get maximum and minimum
    while (!feof(sigin)) {
	if (fscanf (sigin,"%e\n",&value) == 1) {
	    if ( value > max) max= value;
	    if (-value > max) max=-value;
	    cnt++;
	}
    }
    debug("reading ascii file with %d samples", cnt); // ###

    // get the maximum and the scale
    amp = ((1<<23)-1) / max;
    rate=10000;     //will be asked in requester
    channels=1;

    signal[0]=new Signal(cnt, rate);
    signal[0]->setBits(24);
    sample = signal[0]->getSample();
    if (sample) {
	fseek (sigin,0,SEEK_SET); //seek to beginning
	cnt=0;
	while (!feof(sigin)) {
	    if (fscanf(sigin,"%e\n",&value) == 1) {
		sample[cnt++]=(int)(value*amp);
	    }
	}
    }

    fclose (sigin);
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
			    (parent,"Info",
			    i18n("Sorry only 8/16/24 Bits per Sample are supported !"),2);
			  break;	
			}
		      channels=header.channels;

		    }
		}
	      else KMsgBox::message (parent,"Info",i18n("File must be uncompressed (Mode 1) !"),2);
	    }
	  else  KMsgBox::message (parent,"Info",i18n("File is no RIFF Wave File !"),2);

	}
      else  KMsgBox::message (parent,"Info",i18n("File does not contain enough data !"),2);
      fclose(sigfile);
    }
  else  KMsgBox::message (parent,"Info",i18n("File does not exist !"),2);
}
//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
//**********************************************************
void SignalManager::exportAscii(const char *name)
{
    if (!name) return;

    int length=getLength();
    FILE *sigout=fopen (name,"w");

    int *sample = signal[0]->getSample();
    if (sample) {
	for (int i=0; i<length ;i++) {
	    fprintf(sigout,"%0.8e\n", (double)sample[i]/(double)((1<<23)-1));
	}
    }

    fclose(sigout);
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
  // as <1kB is not reached (then we are really out of memory)
  while (savebuffer==0)
    {
      if (bufsize<1024)
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

  char *title = duplicateString(i18n(progress_title));
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
void  SignalManager::save(const char *filename, int bits, bool selection)
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
	    i18n("Sorry only 8/16/24 Bits per Sample are supported !"),2);
	  break;
	}

      fclose(sigout);
    }
}
//**********************************************************
/**
 * Reads in the wav data chunk from a .wav-file. It creates
 * a new empty Signal for each channel and fills it with
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
  // as <1kB is not reached (then we are really out of memory)
  while (loadbuffer==0)
    {
      if (bufsize<1024)
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
      signal[channel]=new Signal (length,rate);
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
  char *title = duplicateString(i18n(progress_title));
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
/***************************************************************************/
//below  all methods of Class SignalManager that deal with sound playback

#define processid	0
#define stopprocess	1
#define samplepointer	2

int play16bit=false;
int bufbase=5;
const char *sounddevice={"/dev/dsp"};
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
			  act=0;
			  for (int i=0;i<channels;i++)
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

			  act=0;
			  for (int i=0;i<channels;i++)
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
			  act=0;
			  for (int i=0;i<channels;i++)
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

		      act=0;
		      for (int i=0;i<channels;i++)
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
		      act=0;
		      for (int i=0;i<channels;i++)
			act+=signal[i]->getSingleSample(pointer);

		      act/=channels;
		      act+=1<<23;
		      buffer[cnt++]=act>>8;
		      buffer[cnt++]=(act>>16)+128;
		      pointer++;
		    }      
		  write (audio,buffer,bufsize);
		}
	      // playing not aborted and still something left, so play the rest...
	      if (msg[stopprocess]==false)  
		{
		  for (cnt=0;cnt<last-pointer;cnt++)
		    {
		      act=0;
		      for (int i=0;i<channels;i++)
			act+=signal[i]->getSingleSample(pointer);

		      act/=channels;
		      act+=1<<23;
		      buffer[cnt++]=act>>8;
		      buffer[cnt++]=(act>>16)+128;
		      pointer++;
		    }      

		  while (cnt<bufsize)
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
