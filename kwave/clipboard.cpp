#include "clipboard.h"
#include "main.h"
#include "menumanager.h"

#define FLUSHCLIP 1
#define TOWINDOW  2

KWaveMenuItem clip_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut
  {0              ,"&Edit"              ,KMENU ,-1   ,KSHARED},
  {0              ,0                    ,KSEP  ,KSEP ,-1},
  {0              ,"&Clipboard"         ,KMENU ,-1   ,KEXCLUSIVE},
  {FLUSHCLIP      ,"&Flush"             ,KITEM ,-1   ,-1},
  {TOWINDOW       ,"&to new Window"     ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0}
};

extern QList<TopWidget>*topwidget; 
extern KApplication *app;
//*****************************************************************************
ClipBoard::ClipBoard (MSignal *signal)
{
  printf ("filling clipboard\n");
  this->signal=signal;
}
//*****************************************************************************
void ClipBoard::appendChannel (MSignal *channel)
{
  signal->appendChannel (channel);
}
//*****************************************************************************
MSignal *ClipBoard::getSignal ()
{
  return signal;
}
//*****************************************************************************
int ClipBoard::getLength ()
{
  return signal->getLength();
}
//*****************************************************************************
void ClipBoard::toWindow ()
{
  if (signal)
   {
    TopWidget *tnew=new TopWidget(app);
    topwidget->append (tnew);
    tnew->setSignal (signal);

    tnew->show();
    tnew->setCaption (klocale->translate("Clipboard"));
    delete this;
   }
}
//*****************************************************************************
ClipBoard::~ClipBoard ()
{
  delete signal;
}
//*****************************************************************************
