
#include <libkwave/Global.h>

#include "kapp.h"
#include "SignalManager.h"
#include "TopWidget.h"
#include "ClipBoard.h"

extern Global globals;
//*****************************************************************************
ClipBoard::ClipBoard ()
{
  signal=0;
}
//*****************************************************************************
ClipBoard::ClipBoard (SignalManager *signal)
{
  this->signal=signal;
}
//*****************************************************************************
SignalManager *ClipBoard::getSignal ()
{
  return signal;
}
//*****************************************************************************
int ClipBoard::getLength ()
{
  return (signal) ? signal->getLength() : 0;
}
//*****************************************************************************
void ClipBoard::appendChannel (Signal *channel)
{
  if (signal) signal->appendChannel (channel);
  else
    signal=new SignalManager (channel);
}
//*****************************************************************************
void ClipBoard::toWindow ()
{
  if (signal)
   {
    TopWidget *tnew=new TopWidget();

    tnew->setSignal (signal);
    tnew->show();
    tnew->setCaption (klocale->translate("Clipboard"));
    signal=0; //detach signal from this object
    delete this;
   }
}
//*****************************************************************************
ClipBoard::~ClipBoard ()
{
  if (signal) delete signal;

  globals.clipboard=0;
}
//*****************************************************************************
