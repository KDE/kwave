#include "clipboard.h"

#include <libkwave/kwavesignal.h>
#include <libkwave/dynamicloader.h>
#include <libkwave/timeoperation.h>
#include <libkwave/globals.h>
#include <libkwave/dialogoperation.h>
#include <libkwave/parser.h>

extern int play16bit;
extern int bufbase;

extern Global globals;
extern void *createProgressDialog (TimeOperation *operation,const char *caption);
//**********************************************************
void SignalManager::getMaxMin (int channel,int&max, int &min,int begin,int len)
{
  if (channel<channels) signal[channel]->getMaxMin (max,min,begin,len);
}
//**********************************************************
void SignalManager::deleteChannel (int channel)
{
  KwaveSignal *todelete=signal[channel];

  for (int cnt=channel;cnt<channels;cnt++)
    {
      signal[cnt]=signal[cnt+1];
      selected[cnt]=selected[cnt+1];
    }
  delete (todelete);
  channels--;              //decrease number of channels...

  globals.port->putMessage ("refreshchannels()");
  info ();                 //and let everybody know about it

}
//**********************************************************
void SignalManager::addChannel ()
  //adds a channel with silence
{
  signal[channels]=new KwaveSignal (length,rate);
  selected[channels]=true; //enable channel
  channels++;              //increase number of channels...
  info ();                 //and let everybody know about it
}
//**********************************************************
void SignalManager::appendChannel (KwaveSignal *newsig)
{
  signal[channels]=newsig;
  channels++;
  info ();
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
}
//**********************************************************
void threadStub (TimeOperation *obj)
{
  (obj->getSignal())->command (obj);
}
//**********************************************************
int SignalManager::doCommand (const char *str)
{
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
	info ();
      }
    else
      if (matchCommand(str,"deletechannel"))
	{
	  KwaveParser parser (str);
	  int i=atoi(parser.getFirstParam());
	  deleteChannel (i);
	}
      else
      if (matchCommand(str,"delete"))
	{
	  for (int i=0;i<channels;i++) signal[i]->cutRange();
	  info ();
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
			signal[i]->insertPaste (toinsert->getSignal (sourcechan));
			sourcechan++;
			if (sourcechan<clipchan) sourcechan=0;
		      }
		  }
		info ();
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
      if ((signal[i])&&(selected[i]))  //that exist and are selected
	{
	  int len=rmarker-lmarker;
	  int begin=lmarker;
	  if (len==0) begin=0,len=signal[i]->getLength ();

	  char buf[32];
	  sprintf (buf,"%d",i+1);
	  char *caption=catString (command," on Channel ",buf);

	  //create a nice little Object that should contain everything important
	  TimeOperation *operation=
	    new TimeOperation (signal[i],command,begin,len);

	  if (operation)
	    {
	      //create a new progress dialog, that watches an memory address
	      //that is updated by the modules

	      void *dialog=createProgressDialog(operation,caption);
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
	    }
	  else debug ("out of memory: could not allocate TimeOperation\n");
	}
    }
  if (i<channels) return false;
  //could not promote command to modules or an error occured
  else return true;
}
//**********************************************************
void SignalManager::info ()
{
  globals.port->putMessage ("refreshchannels()");
  globals.port->putMessage ("refresh()");
}
//**********************************************************
SignalManager::SignalManager (KwaveSignal *sample)
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
SignalManager::SignalManager (QWidget *parent,int numsamples,int rate,int channels)
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
void SignalManager::setRange (int l,int r )
  //this one sets the internal markers and promotes them to all channels
{
  lmarker=l;
  rmarker=r;
  for (int i=0;i<channels;i++) signal[i]->setMarkers (l,r);
}
//**********************************************************
