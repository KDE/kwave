#include "clipboard.h"
#include "main.h"
#include <libkwave/globals.h>

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
void ClipBoard::appendChannel (KwaveSignal *channel)
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
