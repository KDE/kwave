#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "sample.h"
#include <kmsgbox.h>
#include <linux/soundcard.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <endian.h>
#include <bytesex.h>

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define processid	0
#define stopprocess	1
#define samplepointer	2

int play16bit=false;
int bufbase=5;
extern MSignal *clipboard; 

const char *sounddevice={"/dev/dsp"};

int swapEndian (int s)	//yes you guessed it only for sizeof(int)==4 this
			//works as it should do ...
{
 return ((s&0xff)<<24)|((s&0xff00)<<8)|((s&0xff0000)>>8)|((s%0xff000000)>>24);
}

long swapEndian (long s)	//yes you guessed it only for sizeof(int)==4 this
			//works as it should do ...
{
 return ((s&0xff)<<24)|((s&0xff00)<<8)|((s&0xff0000)>>8)|((s%0xff000000)>>24);
}


//**********************************************************
short int swapEndian (short int s)	//sizeof (short int ) must be 2
{
 return ((s&0xff)<<8)|((s&0xff00)>>8);
}
//**********************************************************
void MSignal::stopplay ()
{
 if (msg[processid]>=0)		//if there is a process running
  {
	msg[stopprocess]=true;	//set flag for stopping
  }
}
//**********************************************************
int MSignal::setSoundParams (int audio,int bitspersample,int channels,int rate,int bufbase)
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
	      else printf ("unusable buffer size\n");
	    }
	  else printf ("unusable rate\n");
	}
      else printf ("wrong number of channels\n");
    }
  else printf ("number of bits per samples not supported\n");
  return 0;
}
//**********************************************************
void MSignal::play8 (int loop)
{
  int 	audio;			//file handle for /dev/dsp
  int   act;
  char	*buffer=0;
  int	bufsize;
  int **samples=new int* [channels];
  int j=0;
  MSignal *tmp=this;
  while (tmp!=0)
    {
      samples[j++]=tmp->getSample();
      tmp=tmp->getNext();
    }

  if (msg[processid]<=0)
    {

      msg[stopprocess]=false;
      int	proc=fork();

      if (proc==0)
	{
	  if ( (audio=open(sounddevice,O_WRONLY)) != -1 )
	    {
	      bufsize=setSoundParams(audio,8,0,rate,bufbase);
 
	      if (bufsize)
		{
      		  buffer=new char[bufsize];
		
		  if ((buffer)&&(samples))
		    {
		      int	&pointer=msg[samplepointer];
		      int	last=rmarker;
		      int	cnt=0;
		      pointer=lmarker;

		      if (loop)
			{
			  if (lmarker==rmarker) last=length;

			  while (msg[stopprocess]==false)
			    {
			      for (cnt=0;cnt<bufsize;cnt++)
				{
				  if (pointer>=last) pointer=lmarker;
				  act=samples[0][pointer];
				  for (int i=1;i<channels;i++) act+=samples[i][pointer];
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
			  if (last==pointer) last=length;

			  while ((last-pointer>bufsize)&&(msg[stopprocess]==false))
			    {
			      for (cnt=0;cnt<bufsize;cnt++)
				{
				  act=samples[0][pointer];
				  for (int i=1;i<channels;i++) act+=samples[i][pointer];
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
				  act=samples[0][pointer];
				  for (int i=1;i<channels;i++) act+=samples[i][pointer];
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
	      if (buffer) delete buffer;
	      msg[stopprocess]=false;
	      msg[samplepointer]=0;
	    }
	  msg[processid]=0;
	  exit (0);
	}
      else msg[processid]=proc;		//main process stores id ...

      if (proc==-1)
	KMsgBox::message (parent,"Info","Can not fork playing process!",2);
    }
}
//**********************************************************
void MSignal::play16 (int loop)
{
  int 	audio;			//file handle for /dev/dsp
  unsigned char *buffer=0;
  int	bufsize;

  int **samples=new int* [channels];  //read linear list of channels into array
  int j=0;
  MSignal *tmp=this;
  while (tmp!=0)
    {
      samples[j++]=tmp->getSample();
      tmp=tmp->getNext();
    }

  if (msg[processid]<=0)
    {
      msg[stopprocess]=false;
      int proc=fork();

      if (proc==0)
	{
	  if ( (audio=open(sounddevice,O_WRONLY)) != -1 )
	    {
	      bufsize=setSoundParams(audio,16,0,rate,bufbase);

	      buffer=new unsigned char[bufsize];
		
	      if (buffer)
		{
		  int	&pointer=msg[samplepointer];
		  int	last=rmarker;
		  int	act,cnt=0;
		  pointer=lmarker;
		  if (last==pointer) last=length;

		  if (loop)
		    {
		      while (msg[stopprocess]==false)
			{
			  for (cnt=0;cnt<bufsize;)
			    {
			      if (pointer>=last) pointer=lmarker;
			      act=samples[0][pointer];
			      for (int i=1;i<channels;i++) act+=samples[i][pointer];
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
			      act=samples[0][pointer];
			      for (int i=1;i<channels;i++) act+=samples[i][pointer];
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
			      act=samples[0][pointer];
			      for (int i=1;i<channels;i++) act+=samples[i][pointer];
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
	      if (buffer) delete buffer; 
	      msg[stopprocess]=false;
	      msg[samplepointer]=0;
	    }
	  msg[processid]=0;
	  exit (0);
	}
      else msg[processid]=proc;		//main process stores id ...

      if (proc==-1)
	KMsgBox::message (parent,"Info","Can not fork playing process!",2);
    }
}
//**********************************************************
MSignal::MSignal (QWidget *par,QString *name,int channel=1) :QObject ()
{
  selected=true;
  sample=0;
  parent=par;
  lmarker=0;
  rmarker=0;
  next=0;

  int key = ftok(".", 'S');
  int memid=-1;

  while (memid==-1) memid=shmget(key++,sizeof(int)*4,IPC_CREAT|IPC_EXCL|0660); //seems to be the only way to ensure to get a block of memory at all... 
  msg= (int*) shmat (memid,0,0);
  shmctl(memid, IPC_RMID, 0);  
  if (msg)
    {
      msg[processid]=0;
      msg[stopprocess]=false;
      msg[samplepointer]=0;
	
      QFile *sigin=new QFile (name->data());
      if (sigin->exists())
	{
	  union
	  {
	    char rheader[sizeof(struct wavheader)];
	    wavheader header;
	  };
	  union 
	  {
	    char rlength [4];
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

			  if ((header.channels!=2)||(header.bitspersample!=16))
			  if (header.channels>channel)
			      next=new MSignal (par,name,channel+1);

			  emit sampleChanged();
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
  else  KMsgBox::message (parent,"Info","Shared Memory could not be allocated !",2);
}
//**********************************************************
void	MSignal::save (QString *filename,int bit,int selection)
{
  int begin=0;
  int length=this->length;
  struct wavheader	header;
  int **samples=new int* [channels];
  int channels;
  int j=0;

  if (lmarker!=rmarker)
    {
      begin=lmarker;
      length=rmarker;
    }

  QFile	*sigout=new QFile (filename->data());
  MSignal *tmp=this;

  while (tmp!=0)
    {
      if ((!selection)||(tmp->isSelected()))
      samples[j++]=tmp->getSample();
      tmp=tmp->getNext();
    }
  channels=j;

  if (sigout&&samples)
    {
      if (sigout->exists())
	{
	  debug ("File Exists !\n");
	}

      sigout->open (IO_WriteOnly);
      sigout->at (0);

      strncpy (header.riffid,"RIFF",4);
      strncpy (header.wavid,"WAVE",4);
      strncpy (header.fmtid,"fmt ",4);
      header.fmtlength=16;
      header.filelength=(length*bit/8+sizeof(struct wavheader));
      header.mode=1;
      header.channels=channels;
      header.rate=rate;
      header.AvgBytesPerSec=rate*bit/8;
      header.BlockAlign=bit/8;
      header.bitspersample=bit;

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

      sigout->writeBlock ((char *) &header,sizeof (struct wavheader));
      sigout->writeBlock ("data",4);
      sigout->writeBlock ((char *)&datalen,4);

      switch (bit)
	{
	case 8:
	  writeData8Bit (sigout,begin,length,samples,channels);
	break;
	case 16:
	  writeData16Bit (sigout,begin,length,samples,channels);
	break;
	case 24:
	  writeData24Bit (sigout,begin,length,samples,channels);
	break;
	}
      sigout->close();
    }
  if (sigout) delete sigout;
}
//**********************************************************
void MSignal::writeData8Bit (QFile *sigout,int begin,int end,int **samples,int channels)
{
  char o;
  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{
	  o=128+(char)(samples[j][i]/65536);
	  sigout->putch (o);
	}
    }
}
//**********************************************************
void MSignal::writeData16Bit (QFile *sigout,int begin,int end,int **samples,int channels)
{
  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{

#if defined(IS_BIG_ENDIAN)
	  sigout->putch ((char)(samples[j][i]/65536));
	  sigout->putch ((char)(samples[j][i]/256));
#else
	  sigout->putch ((char)(samples[j][i]/256));
	  sigout->putch ((char)(samples[j][i]/65536));
#endif
	}
    }
}
//**********************************************************
void MSignal::writeData24Bit (QFile *sigout,int begin,int end,int **samples,int channels)
{
  for (int i=begin;i<end;i++)
    {
      for (int j=0;j<channels;j++)
	{

#if defined(IS_BIG_ENDIAN)
	  sigout->putch ((char)(samples[j][i]/65536));
	  sigout->putch ((char)(samples[j][i]/256));
	  sigout->putch ((char)(samples[j][i]));
#else
	  sigout->putch ((char)(samples[j][i]));
	  sigout->putch ((char)(samples[j][i]/256));
	  sigout->putch ((char)(samples[j][i]/65536));
#endif
	}
    }
}
//**********************************************************
void	MSignal::load24Bit (QFile *sigin,int offset,int interleave)
{
  unsigned char *loadbuffer = new unsigned char[PROGRESS_SIZE*3];
  sample = new int [length];
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
		  if (i<length-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
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
void	MSignal::load16Bit (QFile *sigin,int offset,int interleave)
{
  unsigned char *loadbuffer = new unsigned char[PROGRESS_SIZE*2];
  sample = new int [length];
  if ((sample)&&(loadbuffer))
    {
      ProgressDialog *dialog=new ProgressDialog (length,"Loading 16-Bit File :");
      dialog->show();

      if (dialog)
	{
	  int i,j,pos;
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
		      //		      sample[i]=(sigin->getch()<<8)+(sigin->getch()<<16);
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
		  sample[i]=(sigin->getch()<<8)+(sigin->getch()<<16);
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
void	MSignal::loadStereo16Bit (QFile *sigin)
{
  sample = new int [length];
  unsigned char *loadbuffer = new unsigned char[PROGRESS_SIZE*4];

  next=new MSignal (parent,length,rate,1);
  int *lsample = next->getSample();

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

	      //	      printf ("%d\n",
	      sigin->readBlock((char *)loadbuffer,(j-i)*4);
	      pos=0;

	      for (;i<j;i++)
		{
		  sample[i]=loadbuffer[pos++]<<8;
		  sample[i]+=loadbuffer[pos++]<<16;
		  if (sample[i]>=(1<<23)) sample[i]=(-(1<<24))+sample[i];
		  lsample[i]=loadbuffer[pos++]<<8;
		  lsample[i]+=loadbuffer[pos++]<<16;
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
void MSignal::load8Bit (QFile *sigin,int offset,int interleave)
{
  sample = new int [length];
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
void MSignal::findDatainFile (QFile *sigin)
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

















