
#include "config.h"
#include "kapp.h"
#include "SignalManager.h"
#include "ClipBoard.h"

//***************************************************************************
ClipBoard::ClipBoard()
{
    signal = 0;
}

//***************************************************************************
ClipBoard::ClipBoard(SignalManager *signal)
{
    this->signal = signal;
}

//***************************************************************************
SignalManager *ClipBoard::getSignal()
{
    return signal;
}

//***************************************************************************
unsigned int ClipBoard::getLength()
{
    return (signal) ? signal->length() : 0;
}

//***************************************************************************
void ClipBoard::appendChannel(Signal *channel)
{
// ###
//    ASSERT(channel);
//    if (!channel) return;
//
//    if (signal) {
//	signal->appendChannel(channel);
//    } else {
//	signal=new SignalManager(channel);
//	ASSERT(signal);
//    }
}

//***************************************************************************
void ClipBoard::toWindow()
{
    /* ###
      if (signal)
       {
	TopWidget *tnew=new TopWidget(? ? ?);
     
	tnew->setSignal (signal);
	tnew->show();
	tnew->setCaption (i18n("Clipboard"));
	signal=0; //detach signal from this object
	delete this;
       }
    ### */
}

//***************************************************************************
ClipBoard::~ClipBoard()
{
    if (signal) delete signal;
}

//***************************************************************************
