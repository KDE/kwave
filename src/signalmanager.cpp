#include <dlfcn.h>
#include <kmsgbox.h>
#include "fftview.h"
#include "sonagram.h"

#include "clipboard.h"
#include "dialog_progress.h"

#include "../lib/kwavesignal.h"
#include "../lib/dynamicloader.h"
#include "../lib/timeoperation.h"
#include "../lib/globals.h"
#include "../lib/dialogoperation.h"
#include "../libgui/kwavedialog.h"

//some definitions of values follow

//id's for functions...
#define KLOC 1000

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

extern int play16bit;
extern int bufbase;

extern Global globals;
static QStrList filterNameList;
//**********************************************************
//append items only if the Menu is created from scratch
//  if (manage->addNumberedMenu ("FilterPresets"))
//  {
//    QDir filterDir (globals.filterDir);
//    filterDir.setNameFilter ("*.filter");
//    filterNameList=(QStrList *)filterDir.entryList ();
//
//      for (char *tmp=filterNameList.first();tmp!=0;tmp=filterNameList.next())
//	{
//	  char buf[strlen(tmp)-6];
//	  strncpy (buf,tmp,strlen(tmp)-6);
//	  buf[strlen(tmp)-7]=0;
//	  manage->addNumberedMenuEntry ("FilterPresets",buf);
//	}
//  }
//**********************************************************
void SignalManager::getMaxMin (int channel,int&max, int &min,int begin,int len)
{
  if (channel<channels) signal[channel]->getMaxMin (max,min,begin,len);
}
//**********************************************************
void SignalManager::deleteChannel (int)
{
}
//**********************************************************
void SignalManager::addChannel ()
  //adds a channel with zero samples
{
  signal[channels]=new KwaveSignal (length,rate);
  selected[channels]=true;
  channels++;
  info ();
}
//**********************************************************
void SignalManager::appendChannel (KwaveSignal *newsig)
{
  signal[channels]=newsig;
  channels++;
  info ();
}
//**********************************************************
void SignalManager::showDialog (const char *name)
{
  DialogOperation *operation=new DialogOperation (&globals,length,rate,channels);
  if (operation)
    {
      KwaveDialog *dialog=DynamicLoader::getDialog (name,operation);
      if (dialog)
	{
	  connect (dialog,SIGNAL(command(const char*)),
		   this,SLOT(promoteCommand(const char *)));
	  dialog->show();
	}
      else printf ("error: could get dialog !\n");

      delete operation;
    }
}
//**********************************************************
void SignalManager::checkRange ()
//usually useless, or it introduces
//new bugs, or even better makes old bug worse to debug... 
{
  if (lmarker<0) lmarker=0; //check markers for bounding
  if (rmarker>length) lmarker=length; 

  len=rmarker-lmarker;
  begin=lmarker;

  if (len==0) //if no part is selected select the whole signal
    {
      len=length;
      begin=0;
    }
}
//**********************************************************
void SignalManager::setOp (int id)
  // this is the sample wrapper function 
  // all methods should called by giving an id to this function
  // this will improve independency of gui system should there be need of
  // doing a port to another system...
{
  stopplay();	//every operation cancels playing...

  checkRange ();

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
  if ((id>DELETECHANNEL)&&(id<DELETECHANNEL+256))
    {
      deleteChannel (id-DELETECHANNEL);
      channels--;
      info ();
      emit channelReset();
    }

  switch (id)
    {
      //add your wrapper here
    case COPY:
      if (globals.clipboard) delete globals.clipboard;
      globals.clipboard=new ClipBoard ();
      if (globals.clipboard)
      for (int i=0;i<channels;i++) globals.clipboard->appendChannel (signal[i]->copyRange());
      break;
    case CUT:
      if (globals.clipboard) delete globals.clipboard;
      globals.clipboard=new ClipBoard ();
      if (globals.clipboard)
      for (int i=0;i<channels;i++) globals.clipboard->appendChannel (signal[i]->cutRange());
      info ();
      break;
    case PASTE:
      if (globals.clipboard)
	{
	  debug ("not done yet\n");
	}
      info ();
      break;
    case ALLCHANNEL:
      for (int i=0;i<channels;i++) selected[i]=true;
      break;
    case INVERTCHANNEL:
      for (int i=0;i<channels;i++) selected[i]=!selected[i];
      break;
    case ADDCHANNEL:
      addChannel();
      break;
    case NOISE:
      promoteCommand ("noise");
      break;
    case ZERO:
      promoteCommand ("zero");
      break;
    case CENTER:
      promoteCommand ("center");
      break;
    case REVERSE:
      promoteCommand ("reverse");
      break;
    }
}
//**********************************************************
void threadStub (TimeOperation *obj)
{
  (obj->getSignal())->command (obj);
}
//**********************************************************
void SignalManager::promoteCommand (const char *command)
{
  for (int i=0;i<channels;i++)
    if ((signal[i])&&(selected[i]))
      {
	int len=rmarker-lmarker;
	int begin=lmarker;
	if (len==0) begin=0,len=signal[i]->getLength ();

	QString header(command);
	QString cnt;
	header=header+" on Channel "+cnt.setNum(i+1);

	TimeOperation *operation=new TimeOperation (signal[i],command,begin,len);
	if (operation)
	  {
	    ProgressDialog *dialog=new ProgressDialog (operation,header.data());
	    if (dialog)
	      {
		pthread_t thread;

		dialog->show ();
	  
		connect (dialog,SIGNAL(done()),parent,SLOT(refresh()));
		pthread_create (&thread,0,(void *(*)(void *))(threadStub),(void *)operation);
	      }
	  }
      }
}
//**********************************************************
void SignalManager::toggleChannel (int c)
{
  selected[c]=!selected[c];
}
//**********************************************************
void SignalManager::info ()
  //notifies attached slots about important events
  //does a little bit to much, but nothing should hurt the performance
{
  length=signal[0]->getLength();
  emit sampleChanged ();
  emit rateInfo (rate);
  emit channelInfo (channels);
  emit selectedTimeInfo ((int)(((long long)(lmarker-rmarker))*10000/rate));
  emit lengthInfo (length);
  emit timeInfo ((int)((((long long)length)*10000)/rate));
}
//**********************************************************
const char *SignalManager::getName ()
{
  return name->data();
}
//**********************************************************
SignalManager::SignalManager (KwaveSignal *sample):QObject ()
{
  if (sample)
    {
      this->channels=1;
      this->rate=sample->getRate();
      this->length=sample->getLength();

      signal[0]=sample;

      for (int i=1;i<MAXCHANNELS;i++) signal[i]=0;
      for (int i=0;i<channels;i++) selected[i]=true;
    }
}
//**********************************************************
SignalManager::SignalManager (QWidget *parent,int numsamples,int rate,int channels) :QObject ()
{
  if (channels>MAXCHANNELS) channels=MAXCHANNELS;

  this->channels=channels; //store how many channels are linked to this
  this->rate=rate;
  this->length=numsamples;
  this->parent=parent;

  for (int i=0;i<channels;i++) selected[i]=true;

  for (int i=0;i<channels;i++)
    signal[i]=new KwaveSignal (numsamples,rate);

  prepareChannels ();
}
//**********************************************************
void SignalManager::prepareChannels ()
{
  for (int i=channels;i<MAXCHANNELS;i++) signal[i]=0;
}
//**********************************************************
SignalManager::~SignalManager ()
{
}
//**********************************************************
void SignalManager::setMarkers (int l,int r )
  //this one sets the internal markers and promotes them to all channels
{
  lmarker=l;
  rmarker=r;
  emit selectedTimeInfo((int)(((long long)(r-l))*10000/rate));
  for (int i=0;i<channels;i++) signal[i]->setMarkers (l,r);
}
//**********************************************************
