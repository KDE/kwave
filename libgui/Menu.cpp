#include <stdio.h>
#include "kwavemenu.h"
#include <libkwave/parser.h>
#include <libkwave/kwavestring.h>
#include <libkwave/globals.h>
#include <libkwave/messageport.h>
#include <kapp.h>

extern struct Global globals;
static int unique_menu_id=0;
//*****************************************************************************
MenuCommand::MenuCommand (const char *command,int id)
{
  this->command=duplicateString(command);
  this->id=id;
};
//*****************************************************************************
MenuCommand::~MenuCommand ()
{
  if (command) deleteString (command);
}
//*****************************************************************************
int KwavePopMenu::getUniqueId ()
{
  int x=unique_menu_id;
  unique_menu_id++;

  return x;
}
//*****************************************************************************
int KwavePopMenu::getIdRange (int num)
{
  int x=unique_menu_id;
  unique_menu_id+=num;

  return x;
}
//*****************************************************************************
KwavePopMenu::KwavePopMenu (const char *name,int id): QPopupMenu()
{
  this->name=duplicateString (name);
  this->id=id;
  this->memberId=0;
  QObject::connect (this,SIGNAL(activated(int)),this,SLOT(selected(int)));
  checkItems=false;
  numberItems=false;
  com=0;
}
//*****************************************************************************
void KwavePopMenu::setCommand (const char *com)
  //enables checking of menu entries
{
  this->com=duplicateString (com);
}
//*****************************************************************************
void KwavePopMenu::numberable ()
  //enables checking of menu entries
{
  numberItems=true;
}
//*****************************************************************************
void KwavePopMenu::checkable ()
  //enables checking of menu entries
{
  checkItems=true;
  QObject::connect (this,SIGNAL(activated(int)),this,SLOT(check(int)));
  setCheckable (true);
  checked=-1; 
}
//*****************************************************************************
int KwavePopMenu::insertEntry (const char *name,const char *command, int keycode)
{
  int id=getUniqueId();
  int key;
  commands.append (new MenuCommand (command,id));
  key=this->insertItem (name,id);
  this->setAccel (keycode,id);
  return key; // return the real id of the menu entry
}
//*****************************************************************************
void KwavePopMenu::selected (int num)
{
  if (numberItems)
    {
      char buf[512];
      char *tmp=duplicateString (this->com);
      int cnt=strlen (tmp);
      while ((cnt)&&(tmp[cnt]!=')')) cnt--;
      tmp[cnt]=0;
      sprintf (buf,"%d",num);

      char *com=catString (tmp,buf,")");

      globals.port->putMessage (com);

      deleteString (com);
      deleteString (tmp);
    }
  else
    {
      MenuCommand *tmp=commands.first();

      while (tmp)
	{
	  if (tmp->getId()==num)
	    globals.port->putMessage (tmp->getCommand());

	  tmp=commands.next();
	}
    }
}
//*****************************************************************************
void KwavePopMenu::check ()
  //checks currently checked menu item
{
  if (checkItems)
    {
      if (checked<0) checked=idAt (0);
      setItemChecked (checked,true);  
    }
}
//*****************************************************************************
void KwavePopMenu::check (int num)
  //checkmarks a new item, removes check from old item
{
  if (checkItems)
    {
      setItemChecked (checked,false);
      checked=num;
      check ();
    }
}
//*****************************************************************************
void KwavePopMenu::insertMenu (KwavePopMenu *entry)
{
  children.append (entry);
  this->insertItem (klocale->translate(entry->getName()),entry);
}
//*****************************************************************************
void KwavePopMenu::removeMenu (const char *name)
{
  KwavePopMenu *menu=findMenu(name);
  if (menu)
    {
      int index=menu->getId();
      removeItem(index);
      children.removeRef (menu);
      delete menu;
    }
}
//*****************************************************************************
KwavePopMenu *KwavePopMenu::findMenu (const char *name)
{
  KwavePopMenu *tmp=children.first();

  while (tmp)
    {
      if (strcmp (tmp->getName(),name)==0) return tmp;
      tmp=children.next();
    }
  return 0;
}
//*****************************************************************************
KwavePopMenu::~KwavePopMenu ()
{
  deleteString (name);
  deleteString (com);
}
//*****************************************************************************

