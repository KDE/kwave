#include <stdio.h>
#include <libkwave/Parser.h>
#include <libkwave/String.h>
#include <libkwave/Global.h>
#include <libkwave/MessagePort.h>
#include <kapp.h>
#include "Menu.h"
#include "MenuCommand.h"

extern struct Global globals;
static int unique_menu_id=0;
//*****************************************************************************
int Menu::getUniqueId ()
{
  int x=unique_menu_id;
  unique_menu_id++;

  return x;
}
//*****************************************************************************
int Menu::getIdRange (int num)
{
  int x=unique_menu_id;
  unique_menu_id+=num;

  return x;
}
//*****************************************************************************
Menu::Menu (const char *name,int id): QPopupMenu()
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
void Menu::setCommand (const char *com)
  //sets the command emitted when selecting the menu entry
{
  this->com=duplicateString (com);
}
//*****************************************************************************
void Menu::numberable ()
  //enables checking of menu entries
{
  numberItems=true;
}
//*****************************************************************************
void Menu::checkable ()
  //enables checking of menu entries
{
  checkItems=true;
  QObject::connect (this,SIGNAL(activated(int)),this,SLOT(check(int)));
  setCheckable (true);
  checked=-1; 
}
//*****************************************************************************
int Menu::insertEntry (const char *name,const char *command, int keycode)
{
  int id=getUniqueId();
  int key;
  commands.append (new MenuCommand (command,id));
  key=this->insertItem (name,id);
  this->setAccel (keycode,id);
  return key; // return the real id of the menu entry
}
//*****************************************************************************
void Menu::selected (int num)
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
void Menu::check ()
  //checks currently checked menu item
{
  if (checkItems)
    {
      if (checked<0) checked=idAt (0);
      setItemChecked (checked,true);  
    }
}
//*****************************************************************************
void Menu::check (int num)
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
void Menu::insertMenu (Menu *entry)
{
  children.append (entry);
  this->insertItem (klocale->translate(entry->getName()),entry);
}
//*****************************************************************************
void Menu::removeMenu (const char *name)
{
  Menu *menu=findMenu(name);
  if (menu)
    {
      int index=menu->getId();
      removeItem(index);
      children.removeRef (menu);
      delete menu;
    }
}
//*****************************************************************************
Menu *Menu::findMenu (const char *name)
{
  Menu *tmp=children.first();

  while (tmp)
    {
      if (strcmp (tmp->getName(),name)==0) return tmp;
      tmp=children.next();
    }
  return 0;
}
//*****************************************************************************
Menu::~Menu ()
{
  deleteString (name);
  deleteString (com);
}
//*****************************************************************************

