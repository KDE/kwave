#include "clipboard.h"
#include "main.h"
#include "../libgui/menumanager.h"
#include <libkwave/globals.h>

#define FLUSHCLIP 1
#define TOWINDOW  2

KwaveMenuItem clip_menus[]=
{
  //internalID    ,name                 ,type  ,id  ,shortcut
  {0            ,0             ,"&Edit"              ,KMENU ,-1   ,KSHARED},
  {0            ,0             ,0                    ,KSEP  ,KSEP ,-1},
  {FLUSHCLIP    ,"flush ()"    ,"&Flush Clipboard"   ,KITEM ,-1   ,-1},
  {0            ,0             ,"&Clipboard"         ,KMENU ,-1   ,KEXCLUSIVE},
  {TOWINDOW     ,"cliptonew ()","&to new Window"     ,KITEM ,-1   ,-1},
  {0            ,0             ,0                    ,KEND  ,KEND ,-1},
  {0            ,0             ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0,0}
};

QList<MenuManager> clipmenulist;    //list of all registered Menus
                                    //used for appending clipboard menus
extern Global globals;
//*****************************************************************************
ClipBoard::ClipBoard () {signal=0,hasmenu=false;}
//*****************************************************************************
ClipBoard::ClipBoard (SignalManager *signal)
{
  this->signal=signal;
  
  MenuManager *tmp=clipmenulist.first();
  while (tmp)
    {
      tmp->appendMenus (clip_menus);
      tmp=clipmenulist.next();
    }
  hasmenu=true;
}
//*****************************************************************************
void ClipBoard::registerMenu (MenuManager *manage)
{
  if (clipmenulist.findRef(manage)<0)
    {
      clipmenulist.append (manage);
      if (globals.clipboard) manage->appendMenus (clip_menus);
    }
}
//*****************************************************************************
void ClipBoard::unregisterMenu (MenuManager *manage)
{
  clipmenulist.removeRef (manage);
}
//*****************************************************************************
void ClipBoard::appendChannel (KwaveSignal *channel)
{
  if (signal) signal->appendChannel (channel);
  else
    signal=new SignalManager (channel);
}
//*****************************************************************************
SignalManager *ClipBoard::getSignal ()
{
  return signal;
}
//*****************************************************************************
int ClipBoard::getLength ()
{
  return signal->getLength();
}
//*****************************************************************************
void ClipBoard::setOp (const char *)
{
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
    signal=0; //detach signal
    delete this;
   }
}
//*****************************************************************************
ClipBoard::~ClipBoard ()
{
  if (signal) delete signal;

  if (hasmenu) //delete menu from all Topwidgets, that are supposed to have one
    {
      MenuManager *tmp=clipmenulist.first();

      while (tmp)
	{
	  tmp->deleteMenus (clip_menus);
	  tmp=clipmenulist.next();
	}
    }
  globals.clipboard=0;
}
//*****************************************************************************
