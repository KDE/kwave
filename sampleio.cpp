#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "sample.h"
#include <kmsgbox.h>
#include <linux/soundcard.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define processid	0
#define stopprocess	1
#define samplepointer	2


int play16bit=false;
extern MSignal *clipboard; 

int swapEndian (int s)	//yes you guessed it only for sizeof(int)==4 this
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
//**********************************************************
void MSignal::loop8 ()
{
 int 	audio;			//file handle for /dev/dsp
 int	bitspersample=8;
 char	*buffer;
 int	channels=0;
 int	size,base=5;

 if (msg[processid]<=0)
  {
   msg[stopprocess]=false;
   int	proc=fork();

   if (proc==0)
    {
     if ( (audio=open("/dev/dsp",O_WRONLY)) != -1 )
      {
	if ( ioctl(audio,SNDCTL_DSP_SAMPLESIZE,&bitspersample)!=-1)
	 {
	  if ( ioctl(audio,SNDCTL_DSP_STEREO,&channels)!=-1)
	   {
	    if (ioctl(audio,SNDCTL_DSP_SPEED,&rate)!=-1) 
	     {
	      if (ioctl(audio,SNDCTL_DSP_SETFRAGMENT,&base)!=-1) 
	       {
		ioctl(audio,SNDCTL_DSP_GETBLKSIZE,&size);
		buffer=new char[size];
		
		if (buffer)
		 {
			int	&pointer=msg[samplepointer];
			int	last=rmarker;
			int	cnt=0;
			pointer=lmarker;

			if (lmarker==rmarker) last=length;

			while (msg[stopprocess]==false)
			 {
				for (cnt=0;cnt<size;cnt++)
				 {
					if (pointer>=last) pointer=lmarker;
					buffer[cnt]=(sample[pointer++]+(1<<23))>>16;
				 }
				write (audio,buffer,size);
			 }
		 }
	       }
	     }
	   }
	 }
	close (audio);
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
void MSignal::play8 ()
{
  int 	audio;			//file handle for /dev/dsp
  int	bitspersample=8;
  char	*buffer;
  int	channels=0;
  int	size,base=5;

  if (msg[processid]<=0)
    {
      msg[stopprocess]=false;
      int	proc=fork();

      if (proc==0)
	{
	  if ( (audio=open("/dev/dsp",O_WRONLY)) != -1 )
	    {
	      if ( ioctl(audio,SNDCTL_DSP_SAMPLESIZE,&bitspersample)!=-1)
		{
		  if ( ioctl(audio,SNDCTL_DSP_STEREO,&channels)!=-1)
		    {
		      if (ioctl(audio,SNDCTL_DSP_SPEED,&rate)!=-1) 
			{
			  if (ioctl(audio,SNDCTL_DSP_SETFRAGMENT,&base)!=-1) 
			    {
			      ioctl(audio,SNDCTL_DSP_GETBLKSIZE,&size);
			      buffer=new char[size];
		
			      if (buffer)
				{
				  int	&pointer=msg[samplepointer];
				  int	last=rmarker;
				  int	cnt=0;
				  pointer=lmarker;

				  if (last==pointer) last=length;

				  while ((last-pointer>size)&&(msg[stopprocess]==false))
				    {
				      for (cnt=0;cnt<size;cnt++)
					buffer[cnt]=(sample[pointer++]+(1<<23))>>16;	//convert to 8Bit

				      write (audio,buffer,size);
				    }
				  if (msg[stopprocess]==false)
				    {
				      for (cnt=0;cnt<last-pointer;cnt++)
					buffer[cnt]=(sample[pointer++]+(1<<23))>>16;	//convert to 8Bit

				      for (;cnt<size;cnt++) buffer[cnt]=128; // fill up last buffer
				      write (audio,buffer,size);
				    }
				}
			    }
			}
		    }
		}
	      close (audio);
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
void MSignal::loop16 ()
{
 int 	audio;			//file handle for /dev/dsp
 int	bitspersample=16;
 short  int *buffer;
 int	channels=0;
 int	size,base=5;

 if (msg[processid]<=0)
  {
   msg[stopprocess]=false;
   int	proc=fork();

   if (proc==0)
    {
     if ( (audio=open("/dev/dsp",O_WRONLY)) != -1 )
      {
	if ( ioctl(audio,SNDCTL_DSP_SAMPLESIZE,&bitspersample)!=-1)
	 {
	  if ( ioctl(audio,SNDCTL_DSP_STEREO,&channels)!=-1)
	   {
	    if (ioctl(audio,SNDCTL_DSP_SPEED,&rate)!=-1) 
	     {
	      if (ioctl(audio,SNDCTL_DSP_SETFRAGMENT,&base)!=-1) 
	       {
		ioctl(audio,SNDCTL_DSP_GETBLKSIZE,&size);
		buffer=new short int[size];
		
		if (buffer)
		 {
			int	&pointer=msg[samplepointer];
			int	last=rmarker;
			int	cnt=0;
			pointer=lmarker;

			if (lmarker==rmarker) last=length;

			while (msg[stopprocess]==false)
			 {
				for (cnt=0;cnt<size;cnt++)
				 {
					if (pointer>=last) pointer=lmarker;
					buffer[cnt]=(short int)(sample[pointer++]/256);
				 }
				write (audio,buffer,size);
			 }
		 }
	       }
	     }
	   }
	 }
	close (audio);
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
void MSignal::play16 ()
{
  int 	audio;			//file handle for /dev/dsp
  int	bitspersample=16;
  short int *buffer;
  int	channels=0;
  int	size,base=5;

  if (msg[processid]<=0)
    {
      msg[stopprocess]=false;
      int	proc=fork();

      if (proc==0)
	{
	  if ( (audio=open("/dev/dsp",O_WRONLY)) != -1 )
	    {
	      if ( ioctl(audio,SNDCTL_DSP_SAMPLESIZE,&bitspersample)!=-1)
		{
		  if ( ioctl(audio,SNDCTL_DSP_STEREO,&channels)!=-1)
		    {
		      if (ioctl(audio,SNDCTL_DSP_SPEED,&rate)!=-1) 
			{
			  if (ioctl(audio,SNDCTL_DSP_SETFRAGMENT,&base)!=-1) 
			    {
			      ioctl(audio,SNDCTL_DSP_GETBLKSIZE,&size);
			      buffer=new short int[size];
		
			      if (buffer)
				{
				  int	&pointer=msg[samplepointer];
				  int	last=rmarker;
				  int	cnt=0;
				  pointer=lmarker;

				  if (last==pointer) last=length;

				  while ((last-pointer>size)&&(msg[stopprocess]==false))
				    {
				      for (cnt=0;cnt<size;cnt++)
					buffer[cnt]=(short int)(sample[pointer++]/256);
				      
				      write (audio,buffer,size);
				    }
				  if (msg[stopprocess]==false)
				    {
				      for (cnt=0;cnt<last-pointer;cnt++)
					buffer[cnt]=(short int)(sample[pointer++]/256);			

				      for (;cnt<size;cnt++) buffer[cnt]=0;
				      write (audio,buffer,size);
				    }
				}
			    }
			}
		    }
		}
	      close (audio);
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
MSignal::MSignal (QWidget *par,QString *name) :QObject ()
{
  sample=0;
  parent=par;
  lmarker=0;
  rmarker=0;

  memid=shmget(IPC_PRIVATE,sizeof(int)*4,IPC_CREAT+(6<<6)+(6<<3));
  msg= (int*) shmat (memid,0,0);
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
	    char	rlength [4];
	    int	length;
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

			      load8Bit (sigin,0,(header.channels-1)*(header.bitspersample/8));
			      break;
			    case 16:
			      load16Bit (sigin,0,(header.channels-1)*(header.bitspersample/8));
			      break;
			    default:
			      KMsgBox::message
				(parent,"Info","Sorry only 8&16 Bits per Sample are supported !",2);
			      break;	
			    }
			  emit sampleChanged();
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
void	MSignal::save16Bit (QString *filename)
{
  struct wavheader	header;

  printf ("%s\n",filename->data());
  QFile		*sigout=new QFile (filename->data());

  if (sigout)
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
      header.filelength=(length*2+sizeof(struct wavheader));
      header.mode=1;
      header.channels=1;
      header.rate=rate;
      header.AvgBytesPerSec=rate*2;
      header.BlockAlign=2;
      header.bitspersample=16;

      int datalen=length*header.channels*header.bitspersample/8;

#if defined(IS_BIG_ENDIAN)
      header.mode 		= swapEndian (header.mode);
      header.rate 		= swapEndian (header.rate);
      header.channels		= swapEndian (header.channels);
      header.bitspersample	= swapEndian (header.bitspersample);
      header.AvgBytesPerSec	= swapEndian (header.AvgBytesPerSec);
      header.filelength	= swapEndian (header.filelength);
      datalen= swapendian (datalen);
#endif



      sigout->writeBlock ((char *) &header,sizeof (struct wavheader));
      sigout->writeBlock ("data",4);
      sigout->writeBlock ((char *)&datalen,4);

      for (int i=0;i<length;i++)
	{
#if defined(IS_BIG_ENDIAN)
	  sigout->putch ((char)(sample[i]/65536));
	  sigout->putch ((char)(sample[i]/256));
#else
	  sigout->putch ((char)(sample[i]/256));
	  sigout->putch ((char)(sample[i]/65536));
#endif
	}
      sigout->close();
    }
}
//**********************************************************
void	MSignal::load16Bit (QFile *sigin,int offset,int interleave)
{
  sample = new int [length];
  if (sample)
    {
      ProgressDialog *dialog=new ProgressDialog (length,"Loading 16-Bit File :");
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
		      sample[i]=(sigin->getch()<<8)+(sigin->getch()<<16);
		      if (sample[i]>(1<<23)) sample[i]=(-(1<<24))+sample[i];
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
    }
}
//**********************************************************
void	MSignal::load8Bit (QFile *sigin,int offset,int interleave)
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
		      if (sample[i]>(1<<23)) sample[i]=(-(1<<24))+sample[i];
		    }
		  dialog->setProgress (i);
		}
	    }
	  else
	    {	//non zero interleave for multichannel-files !
	      sigin->at(sigin->at()+offset);
	      for (i=0;i<length;i++)
		{
		  sample[i]=(int) (sigin->getch()*1<<15);
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

















