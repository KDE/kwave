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
  {FLUSHCLIP      ,"&Flush Clipboard"   ,KITEM ,-1   ,-1},
  {0              ,"&Clipboard"         ,KMENU ,-1   ,KEXCLUSIVE},
  {TOWINDOW       ,"&to new Window"     ,KITEM ,-1   ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0              ,0                    ,KEND  ,KEND ,-1},
  {0,0,0,0,0}
};

QList<MenuManager> clipmenulist;    //list of all registered Menus
                                    //used for appending clipboard menus
extern QList<TopWidget>*topwidget; 
extern KApplication    *app;
extern ClipBoard       *clipboard;
//*****************************************************************************
ClipBoard::ClipBoard () {signal=0,hasmenu=false;}
//*****************************************************************************
ClipBoard::ClipBoard (MSignal *signal)
{
  this->signal=signal;
  
  if (signal) signal->setMenuManager (0); //detach from previous menu context

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
      if (clipboard) manage->appendMenus (clip_menus);
    }
}
//*****************************************************************************
void ClipBoard::unregisterMenu (MenuManager *manage)
{
  clipmenulist.removeRef (manage);
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
void ClipBoard::setOp (int op)
{
  MenuManager *manage=clipmenulist.first();

  if (manage) op=manage->translateId (clip_menus,op);
  switch (op)
    {
    case TOWINDOW:
      toWindow ();
      break;
    case FLUSHCLIP:
      delete this;
      break;
    }
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
    signal=0; //detach signal
    delete this;
   }
}
//*****************************************************************************
ClipBoard::~ClipBoard ()
{
  if (signal) delete signal;

  if (hasmenu)
    {
      MenuManager *tmp=clipmenulist.first();

      printf ("removing from %p\n",tmp);
      while (tmp)
	{
	  tmp->deleteMenus (clip_menus);
	  tmp=clipmenulist.next();
	}
    }
}
//*****************************************************************************
