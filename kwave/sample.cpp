// In this file you will find methods for changing time-domain signals
// cut/copy/paste functions are included in sampleedit.cpp
// I/O Functions such as loading/saving are in sampleio.cpp

//Here choose biggest prime factor to be tolerated before
//popping up a requester, when doing a fft
#define MAXPRIME 512 
		
#include "sample.h"

#include <kmsgbox.h>
#include <kprogress.h>
#include <sys/shm.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include "fftview.h"
#include "sonagram.h"
#include "faderwidget.h"
#include "windowfunction.h"
#include "clipboard.h"

//some definitions of values follow

#define processid	0
#define stopprocess	1
#define samplepointer	2

//id's for functions...
#define KLOC 1000

#define FFT	        (KLOC+0)
#define SONAGRAM        (KLOC+1)
#define AVERAGEFFT      (KLOC+2)

#define NOISE		(KLOC+3)
#define HULLCURVE	(KLOC+4)
#define ADDSYNTH 	(KLOC+5)
#define PULSE	        (KLOC+6)
#define ZERO            (KLOC+7)

#define FLIP            (KLOC+8)
#define REVERSE 	(KLOC+9)
#define FADEIN 		(KLOC+10)
#define FADEOUT		(KLOC+11)
#define AMPLIFYMAX 	(KLOC+12)
#define DELAY           (KLOC+13)
#define AMPLIFY 	(KLOC+14)
#define AMPWITHCLIP 	(KLOC+15)
#define RATECHANGE 	(KLOC+16)
#define DISTORT 	(KLOC+17)
#define MOVINGAVERAGE 	(KLOC+18)
#define STUTTER 	(KLOC+19)
#define REQUANTISE      (KLOC+20)
#define FILTERCREATE 	(KLOC+21)
#define FILTER    	(KLOC+22)
#define CHANNELMIX    	(KLOC+23)
#define MIXPASTE	(KLOC+24)
#define CENTER		(KLOC+25)
#define RESAMPLE	(KLOC+26)

#define FILTERPRESET    (KLOC+100)   //leave space behind this

//the menu structure being allocated by this Object

KWaveMenuItem sample_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut

  {0              ,"&Calculate"         ,KMENU ,-1   ,KEXCLUSIVE},
  {ZERO           ,"&Silence"           ,KITEM ,-1   ,-1},
  {NOISE          ,"&Noise"             ,KITEM ,-1   ,-1},
  {ADDSYNTH       ,"&Additive Synthesis",KITEM ,-1   ,-1},
  {PULSE          ,"&Pulse Train"       ,KITEM ,-1   ,-1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {HULLCURVE      ,"&Envelope"          ,KITEM ,-1   ,-1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},

  {0              ,"&Frequencies"       ,KMENU ,-1   ,KEXCLUSIVE},
  {FFT            ,"&Spectrum"          ,KITEM ,-1   ,Key_F},
  {AVERAGEFFT     ,"&Average Spectrum"  ,KITEM ,-1   ,SHIFT+Key_F},
  {SONAGRAM       ,"S&onagram"          ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  //second menu 

  {0              ,"F&x"                ,KMENU ,-1   ,KEXCLUSIVE},
  {0              ,"&Amplify"           ,KMENU ,-1   ,KEXCLUSIVE},
  {DISTORT        ,"&Distort"           ,KITEM ,-1   ,-1},
  {FADEIN         ,"Fade &in"           ,KITEM ,-1   ,-1},
  {FADEOUT        ,"Fade &out"          ,KITEM ,-1   ,-1},
  {AMPLIFY        ,"&Free"              ,KITEM ,-1   ,-1},
  {AMPLIFYMAX     ,"to &Maximum"        ,KITEM ,-1   ,Key_M},
  {AMPWITHCLIP    ,"with &Clipboard"    ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},  
  {0              ,0                    ,KSEP  ,KSEP ,-1},  

  {0              ,"&Filter"            ,KMENU ,-1   ,KEXCLUSIVE},
  {MOVINGAVERAGE  ,"&Moving Average"    ,KITEM ,-1   ,-1},
  {FILTERCREATE   ,"&Create"            ,KITEM ,-1   ,-1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {0              ,"&Presets"           ,KMENU ,-1   ,KEXCLUSIVE},
  //reference to grouped menu items attached at runtime
  {FILTERPRESET   ,"FilterPresets"      ,KREF  ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},  
  {0              ,0                    ,KEND  ,KEND ,-1},  
  {0              ,0                    ,KSEP  ,KSEP ,-1},

  {DELAY          ,"&Delay"             ,KITEM ,-1   ,-1},
  {REVERSE        ,"&Reverse"           ,KITEM ,-1   ,-1},
  {STUTTER        ,"&Periodic Silence"  ,KITEM ,-1   ,-1},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {RATECHANGE     ,"&Change rate"       ,KITEM ,-1   ,-1},
  {REQUANTISE     ,"Re&Quantize"        ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  //third menu, only partially added to an alreay existent menu 

  {0              ,"&Edit"              ,KMENU ,-1   ,KSHARED},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {MIXPASTE       ,"&Mix & Paste"       ,KITEM ,-1   ,CTRL+SHIFT+Key_X},
  {CROP           ,"Tri&m"              ,KITEM ,-1   ,CTRL+Key_T},
  {DELETE         ,"&Delete"            ,KITEM ,-1   ,Key_Delete},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {FLIP           ,"&Flip Phase"        ,KITEM ,-1   ,-1},
  {CENTER         ,"&Center Signal"     ,KITEM ,-1   ,-1},
  {RESAMPLE       ,"&Resample"          ,KITEM ,-1   ,SHIFT+Key_R},

  {0              ,"&Channel"           ,KMENU ,-1   ,KSHARED},
  {CHANNELMIX     ,"&Mix"               ,KITEM ,-1   ,SHIFT+Key_M},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},

  {0,0,0,0,0} //Terminates
};

extern int play16bit;
extern int bufbase;
extern ClipBoard *clipboard; 
extern QDir *configDir;

static int len;
static int begin;
extern void *mmapalloc (int);
extern void mmapfree   (void *);
extern long mmap_threshold; //thresholf for using memory mapped to files
static const char *NOMEM={"Not enough Memory for Operation !"};
QDir        *filterDir=0;
QStrList    *filterNameList=0;
//**********************************************************
#ifndef THREADS

//use a wrapper to use functions also on Systems with no thread support...
//seems to work...
void pthread_create (pthread_t *,void *,void* (*jumpto)(void *),void *arg)
{
  *(jumpto) (arg);
}
#endif
//**********************************************************
int *MSignal::getNewMem (int size)
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
void MSignal::getridof (int *mem)
{
  if (mem)
    if (mapped) mmapfree (mem);
    else delete mem;
}
//**********************************************************
void MSignal::appendMenus ()
{
  if (manage)
    {
      if (manage->addNumberedMenu ("FilterPresets")) //append items only if the Menu is created from scratch
	{
	  filterDir=new QDir(configDir->path());

	  if (!filterDir->cd ("presets"))
	    {
	      filterDir->mkdir ("presets");
	      filterDir->cd ("presets");
	    }
	  if (!filterDir->cd ("filters"))
	    {
	      filterDir->mkdir ("filters");
	      filterDir->cd ("filters");
	    }

	  filterDir->setNameFilter ("*.filter");

	  filterNameList=(QStrList *)filterDir->entryList ();

	  for (char *tmp=filterNameList->first();tmp!=0;tmp=filterNameList->next())
	    {
	      char buf[strlen(tmp)-6];
	      strncpy (buf,tmp,strlen(tmp)-6);
	      buf[strlen(tmp)-7]=0;
	      manage->addNumberedMenuEntry ("FilterPresets",buf);
	    }
	}
      manage->appendMenus (sample_menus);
    }
}
//**********************************************************
void MSignal::deleteMenus ()
{
  if (manage) manage->deleteMenus (sample_menus);
}
//**********************************************************
void MSignal::getIDs ()
{
}
//**********************************************************
void MSignal::doRangeOp (int id)
  // this is the sample wrapper function 
  // all methods should called by giving an id to this function
  // this will improve independency of gui system should there be need of
  // doing a port to another system...
{
  stopplay();	//every operation cancels playing...

  if (lmarker<0) lmarker=0; //check markers for bounding
  if (rmarker>length) lmarker=length; 

  //usually useless, or it introduces
  //new bugs, or even better makes old bug worse to debug... 

  len=rmarker-lmarker;
  begin=lmarker;

  if (len==0) //if no part is selected select the whole signal
    {
      len=length;
      begin=0;
    }
  
  //decode dynamical allocated menu id's
  //into the ones used by the switch statement below

  if (manage) id=manage->translateId (sample_menus,id);

  switch (id)
    {
    case PLAY:
      if (play16bit) play16(false);
      else play8(false);
      break;
    case LOOP:
      if (play16bit) play16(true);
      else play8(true);
      break;
    }

  //chaos is about to come
  //check for ranges of id that have to be decoded into a parameter

  if ((id>=TOGGLECHANNEL)&&(id<TOGGLECHANNEL+10)) toggleChannel (0,id-TOGGLECHANNEL);
  if ((id>DELETECHANNEL)&&(id<DELETECHANNEL+256))
    {
      deleteChannel (0,id-DELETECHANNEL);
      channels--;
      emit sampleChanged();
      emit channelReset();
    }

  if ((id>=FILTERPRESET)&&(id<FILTERPRESET+MENUMAX)) movingFilter (id-FILTERPRESET);



  switch (id)
    {
      //add your wrapper here

    case EXPORTASCII:
      exportAscii ();
      break;
    case CHANNELMIX:
      mix ();
      break;
    case ALLCHANNEL:
      selected=true;
      break;
    case INVERTCHANNEL:
      selected=!selected;
      break;
    case COPY:
      copyRange();
      break;
    case FFT:
      fft();
      break;
    case ADDCHANNEL:
      addChannel();
      break;
    case PULSE:
      pulse();
      break;
    case SONAGRAM:
      sonagram ();
      break;
    case AVERAGEFFT:
      averagefft();
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
    case CUT:
      cutRange();
      break;
    case ZERO:
      zeroRange();
      break;
    case FLIP:
      flipRange();
      break;
    case STUTTER:
      stutter();
      break;
    case REQUANTISE:
      reQuantize();
      break;
    case FILTERCREATE:
      filterCreate ();
      break;
    case RESAMPLE:
      resample ();
      break;
    case MOVINGAVERAGE:
      movingAverage();
      break;
    case ADDSYNTH:
      addSynth();
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
    }
}
//**********************************************************
QString *MSignal::getName ()
{
  return name;
}
//**********************************************************
void MSignal::setParent (QWidget *par)
{
  parent=par;
}
//**********************************************************
MSignal::MSignal (QWidget *par,MenuManager *manage,int numsamples,int rate,int channels) :QObject ()
{
  this->manage=manage;
  getIDs ();
  appendMenus ();
  this->channels=channels; //store how many channels are linked to this
  this->rate=rate;
  this->length=numsamples;
  parent=par;

  initSignal ();
}
//**********************************************************
MSignal::MSignal (QWidget *par,int numsamples,int rate,int channels) :QObject ()
  // constructor for creating a "silence"-sample 
  // the other constructor which loads a file into memory is located in sampleio.cpp
{
  this->channels=channels; //store how many channels are linked to this
  this->rate=rate;
  this->length=numsamples;
  parent=par;
  this->manage=0;

  initSignal();
}
//**********************************************************
void MSignal::setMenuManager (MenuManager *manage)
{
  this->manage=manage;
  appendMenus ();
}
//**********************************************************
void MSignal::initSignal ()
{
  mapped=false;
  name=new QString ("Unnamed");
  sample=getNewMem (length);

  if (sample)
    {
      //erase, because not all memory is clearedy
      for (int i=0;i<length;sample[i++]=0);

      //get shared memory needed by playback, this will be changed, if 
      //threading is standard

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

      //initialise attributes

      locked=0;
      selected=true;
      lmarker=0;
      rmarker=0;
      
      next=0;

      //if there are more channels to be created create them recursively
      if (channels>1) next=new MSignal(parent,manage,length,rate,channels-1);
      emit sampleChanged();
    }
  else KMsgBox::message (parent,"Info",NOMEM,2);
}
//**********************************************************
void MSignal::detachChannels ()
// detach all channels linked to this one
{
  next=0;
}
//**********************************************************
MSignal::~MSignal ()
{
  if (manage) deleteMenus();
  if (sample!=0)
    {
      stopplay();
      getridof (sample);
      sample=0;
    }
  if (msg) shmdt ((char *)msg); 

  if (next!=0) delete next;
}
//**********************************************************
//functions that return attribute to external callers
int     MSignal::getLockState   () {return locked;}
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
  //this one sets the internal markers, most operations use
{
	lmarker=l;
	rmarker=r;

	if (next) next->setMarkers (l,r); //recursive call for all channels
}
//**********************************************************
void MSignal::insertZero (int ins)
{
  int *newsam=getNewMem (length+ins);

  if (newsam)
    {
      memcpy (newsam,sample,lmarker*sizeof(int));
      for (int i=0;i<ins;i++) newsam[lmarker+i]=0;
      memcpy (&newsam[lmarker+ins],&sample[rmarker],(length-rmarker)*sizeof(int));
      getridof (sample);

      sample=newsam;
      length+=ins;
      rmarker=lmarker+ins;
    }
  else KMsgBox::message (parent,"Info",NOMEM,2);
}
//**********************************************************
// now follow the various editing and effects functions
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
void fadeOutThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  int curve=(int)params->data[0];
  int i=0;

  if (curve==0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)(((long long) (sample[i]))*(len-i)/len);
	*counter=i;
      }
  else if (curve<0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		log10(1+(-curve*((double)len-i)/len))/log10(1-curve));
	*counter=i;
      }
  else 
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		(1-log10(1+(curve*((double)i)/len))/log10(1+curve)));
	*counter=i;
      }
  *counter=-1;   //signal progress that action is performed...
}
//**********************************************************
void MSignal::fadeChannelOut (int curve) 
{
  pthread_t thread;

  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Fading channel");
  if (dialog)
    {
      dialog->show ();
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));
      lockWrite ();
      connect (dialog,SIGNAL(done()),this,SLOT(unlockWrite()));

      pthread_create (&thread,0,(void *)fadeOutThread,
		      getFXParams((void *)curve));
    }
}
//**********************************************************
void MSignal::fadeOut () 
{
  if (rmarker!=lmarker)
    {
      FadeDialog dialog(parent,-1,(int)(((long)len)*1000/rate));

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
void fadeInThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  int curve=(int)params->data[0];
  int i=0;
  if (curve==0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)(((long long) (sample[i]))*i/len);
	*counter=i;
      }
  else if (curve<0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		log10(1+(-curve*((double)i)/len))/log10(1-curve));
	*counter=i;
      }
  else 
    {
      for (;i<len;i++)
	sample[i]=
	  (int)((double)sample[i]*
		(1-log10(1+(curve*((double)len-i)/len))/log10(1+curve)));
      *counter=i;
    }
  *counter=-1;   //signal progress that action is performed...
}
//**********************************************************
void MSignal::fadeChannelIn (int curve)
{
  pthread_t thread;

  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Fading channel");
  if (dialog)
    {
      dialog->show ();

      lockWrite ();
      connect (dialog,SIGNAL(done()),this,SLOT(unlockWrite()));
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));

      pthread_create (&thread,0,(void *)fadeInThread,
		      getFXParams((void *)curve));
    }
}
//**********************************************************
void MSignal::fadeIn ()
{
  if (rmarker!=lmarker)
    {
      FadeDialog dialog(parent,1,(int)(((long)len)*1000/rate));

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
  sample=getNewMem(numsamples);

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
/*void MSignal::newSignal ()
{
  NewSampleDialog *dialog=new NewSampleDialog (parent);
  if (dialog->exec())
    {
      int	rate=dialog->getRate();
      int	numsamples=dialog->getLength();

      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (!tmp->newChannel(numsamples,rate)) KMsgBox::message (parent,"Info",NOMEM,2);
	  tmp=tmp->getNext();
	}
      emit sampleChanged();
    }
}
*/
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
void noiseThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  for (int i=0;i<len;i++)
    {
      sample[i]=(int)((drand48()*(1<<24)-1)-(1<<23));
      *counter=i;
    }
  *counter=-1;
}
//*********************************************************
void MSignal::noiseRange()
{
  pthread_t thread;

  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Generating Noise");
  if (dialog)
    {
      dialog->show ();
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));

      pthread_create (&thread,0,(void *)noiseThread,
		      getFXParams());
    }
}
//*********************************************************
void MSignal::noiseInsert(int ins)
{
  int *newsam=getNewMem(length+ins);

  if (newsam)
    {
      memcpy (newsam,sample,lmarker*sizeof(int));
      for (int i=0;i<ins;i++)
	newsam[lmarker+i]=(int)((drand48()*(1<<24)-1)-(1<<23));
      memcpy (&newsam[lmarker+ins],&sample[rmarker],(length-rmarker)*sizeof(int));

      getridof (sample);

      sample=newsam;
      length+=ins;
      rmarker=lmarker+ins;
    }
}
//*k********************************************************
void MSignal::noise()
{
  int len=rmarker-lmarker;

  if (len==0)
    {
      TimeDialog dialog (parent,rate,klocale->translate("Insert Noise :"));
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
  HullCurveDialog dialog (parent,"Create Hullcurve from Signal:");
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
  int *newsample=getNewMem (newlen);

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

	  getridof (sample);
      
	  sample=newsample;
	  for (int i=0;i<newlen;i++) newsample[i]=(int)y[i];

	  length=newlen;
	  begin=0;
	  len=length;
	  amplifyChannelMax (oldmax);
	  rate=newrate;
	  delete y;
	}
      else getridof (newsample);      
    }
  else getridof (newsample);      
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
      MSignal *clip=clipboard->getSignal();
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->amplifyChannelwithClip (clip);
	  tmp=tmp->getNext();
	  clip=clip->getNext();
	  if (!clip) clip=clipboard->getSignal();
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
  AmplifyCurveDialog *dialog =new AmplifyCurveDialog (parent,(int)(((double)len)*1000/rate));
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

      Interpolation *interpolation=new Interpolation (dialog->getInterpolationType());
      int type=dialog->getSymmetryType ();

      if ((interpolation)&&(interpolation->prepareInterpolation (points)))
	{
	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->distortChannel (interpolation,type);
	      tmp=tmp->getNext();
	    }
	  //	  delete interpolation; have to find a way for doing this in future...
	}
      delete dialog; 
    }  
}
//*********************************************************
void distortThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  Interpolation *interpolation=(Interpolation *) params->data[0];
  int type=(int) params->data[1];

  int x;
  double oldy,y;
  interpolation->incUsage();


  switch (type)
    {
    case 0:
      for (int i=0;i<len;i++)
	{
	  x=sample[i];
	  oldy=((double) abs(x))/((1<<23)-1);
	  y=interpolation->getSingleInterpolation(oldy);

	  if (x>0)
	    sample[i]=(int) (y*((1<<23)-1));
	  else
	    sample[i]=-(int) (y*((1<<23)-1));
	  *counter=i;
	}
      break;
    case 1: //only upper half...
      for (int i=0;i<len;i++)
	{
	  x=sample[i];

	  if (x>0)
	    {
	      oldy=((double) abs(x))/((1<<23)-1);
	      y=interpolation->getSingleInterpolation(oldy);

	      sample[i]=(int) (y*((1<<23)-1));
	    }
	  *counter=i;
	}
      break;
    case 2: //only lower half...
      for (int i=0;i<len;i++)
	{
	  x=sample[i];

	  if (x<=0)
	    {
	      oldy=((double) abs(x))/((1<<23)-1);
	      y=interpolation->getSingleInterpolation(oldy);

	      sample[i]=-(int) (y*((1<<23)-1));
	    }
	  *counter=i;
	}
      break;

    }
  *counter=-1;
  interpolation->decUsage();
  if (interpolation->getUsage()<=0) delete interpolation;
}
//*********************************************************
void MSignal::distortChannel (Interpolation *interpolation,int type)
{
  pthread_t thread;
  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Distorting channel");
  if (dialog)
    {
      dialog->show ();
      lockWrite ();
      connect (dialog,SIGNAL(done()),this,SLOT(unlockWrite()));
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));

      pthread_create (&thread,0,(void *)distortThread,
		      getFXParams((void *)interpolation,(void *)type));
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
int getMaxPrimeFactor (int len)
{
  int max=1;
  int tst=len;

 //here follows the cannonical slow prime factor search, but it does its job
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
void MSignal::fft ()
{
  int max=getMaxPrimeFactor (len);
  int reduce=1;
  int res=1;

  if (max>MAXPRIME)
    {
      char buf[512];
      while ((len-reduce>MAXPRIME)&&(getMaxPrimeFactor(len-reduce)>MAXPRIME)) reduce++;


      sprintf (buf,"The selected number of samples, contains the large prime factor %d.\nThis may result in extremly long computing time\nIt is recommended to reduce the selected range by %d samples\nto gain lots of speed, but lose some accuracy !\nWhat do you want to go for ?",max,reduce);

      res=KMsgBox::yesNoCancel(parent,"Attention",buf,KMsgBox::QUESTION,"Accuracy","Speed","Cancel");
      if (res==2) len-=reduce;
    }

  if (res!=3) 
    {
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->fftChannel (0);
	  tmp=tmp->getNext();
	}
    }
  if (res==2) len+=reduce;
}
//*********************************************************
void MSignal::fftChannel (int windowtype)
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

      FFTWindow *win=new FFTWindow(name);
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
void MSignal::addSynth ()
{
  AddSynthDialog *dialog =new AddSynthDialog (parent,rate);
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
void MSignal::pulse ()
{
  PulseDialog *dialog =new PulseDialog (parent);
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
      int type=dialog->getType();
      MSignal *tmp=this;
      while (tmp!=0)
	{
	  if (tmp->isSelected()) tmp->averageChannel (average,type);
	  tmp=tmp->getNext();
	}
    }
  emit sampleChanged();
  delete dialog; 
}
//*********************************************************
void averageThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  int a=(int)params->data[0];
  int b=a/2;
  long int newsam;
  int i,j;

  int *sam=new int[len];
  if (sam)
    {
      for (i=b;i<len-b;i++)
	{
	  newsam=0;
	  for (j=-b;j<b;j++) newsam+=sample[i+j];
	  newsam/=a;
	  sam[i]=newsam;
	  *counter=i;
	}
      memcpy (&sample[b],&sam[b],(len-a)*sizeof(int));
      delete sam;
    }
  *counter=-1;
}
//*********************************************************
void MSignal::averageChannel (int a,int type)
{
  pthread_t thread;

  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Quantizing channel");
  if (dialog)
    {
      dialog->show ();
      lockWrite ();
      connect (dialog,SIGNAL(done()),this,SLOT(unlockWrite()));
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));

      pthread_create (&thread,0,(void *)averageThread,
		      getFXParams((void *)a));
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
	  int windowtype=dialog->getWindowType();

	  MSignal *tmp=this;
	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->sonagramChannel (points,windowtype);
	      tmp=tmp->getNext();
	    }
	  emit sampleChanged();
	}
      delete dialog; 
    }
}
//*********************************************************
void MSignal::sonagramChannel (int points,int windowtype)
{
  int length=((len/(points/2))*points/2)+points/2; //round up length
  double *data=new double[length];
  int i;

  if (data)
    {
      for (i=0;i<len;i++)
	data[i]=((double)(sample[begin+i])/(1<<23));
      for (;i<length;i++) data[i]=0; //pad with zeros...

      SonagramWindow *sonagramwindow=new SonagramWindow(name);
      if (sonagramwindow)
	{
	  sonagramwindow->show();
	  sonagramwindow->setSignal (data,length,points,windowtype,rate);
	} 
     delete data;
    }
}
//*********************************************************
void MSignal::averagefft ()
{
  AverageFFTDialog *dialog =new AverageFFTDialog (parent,len,rate);
  if (dialog)
    {
      if (dialog->exec())
	{
	  int points=dialog->getPoints();
	  int windowtype=dialog->getWindowType();

	  MSignal *tmp=this;

	  while (tmp!=0)
	    {
	      if (tmp->isSelected()) tmp->averagefftChannel (points,windowtype);
	      tmp=tmp->getNext();
	    }
	  delete dialog;
	}
    }
}
//*********************************************************
void MSignal::averagefftChannel (int points,int windowtype)
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

      FFTWindow *win=new FFTWindow(name);
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
  Filter *filter=new Filter();

  if (dialog&&filter&&filterDir)
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
void replaceStutterThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  int len1=(int)params->data[0];
  int len2=(int)params->data[1];

  int j;
  int i=len2;
  while (i<len-len1)
    {
      for (j=0;j<len1;j++) sample[i+j]=0;
      i+=len1+len2;
      *counter=i;
    }
  *counter=-1;
}
//*********************************************************
void MSignal::replaceStutterChannel (int len1,int len2)
{
  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Replacing signal with silence in channel");
  if (dialog)
    {
      pthread_t thread;

      dialog->show ();
      lockWrite ();
      connect (dialog,SIGNAL(done()),this,SLOT(unlockWrite()));
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));

      pthread_create (&thread,0,(void *)replaceStutterThread,
		      getFXParams((void *)len1,(void *)len2));
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
void quantizeThread (struct FXParams *params)
{
  int *sample=params->source;
  int len=params->len;
  int *counter=params->counter;
  int bits=(int)params->data[0];
  double a;
  for (int j=0;j<len;j++)
    {
      a=(double)(sample[j]+(1<<23))/(1<<24);
      a=floor(a*bits+.5);
      a/=bits;
      sample[j]=(int)((a-.5)*((1<<24)-1));   //24 because of double range (no more signed)
      *counter=j;
    }
  *counter=-1;   //signal progress that action is performed...
  delete params;
}
//*********************************************************
FXParams *MSignal::getFXParams (void *a,void *b,void *c,void *d,void *e,void *f)
{
  FXParams *fx=new FXParams;
  fx->source=&sample[begin];
  fx->len=len;
  fx->counter=&msg[2];
  fx->data[0]=a;
  fx->data[1]=b;
  fx->data[2]=c;
  fx->data[3]=d;
  fx->data[4]=e;
  fx->data[5]=f;
  return fx;
}
//*********************************************************
void MSignal::lockRead ()
{
  locked++;
}
//*********************************************************
void MSignal::unlockRead ()
{
  locked--;
}
//*********************************************************
void MSignal::lockWrite ()
{
  locked=-1;
}
//*********************************************************
void MSignal::unlockWrite ()
{
  locked=0;
}
//*********************************************************
void MSignal::quantizeChannel (int bits)
{
  bits--; //really complex calculation is needed, to bend this variable into usable form
  pthread_t thread;

  ProgressDialog *dialog=new ProgressDialog (len,&msg[2],"Quantizing channel");
  if (dialog)
    {
      dialog->show ();
      lockWrite ();
      connect (dialog,SIGNAL(done()),this,SLOT(unlockWrite()));
      connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));

      pthread_create (&thread,0,(void *)quantizeThread,
		      getFXParams((void *)bits));
    }
}
//**********************************************************************
