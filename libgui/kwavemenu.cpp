#include <stdio.h>
#include "kwavemenu.h"
#include <libkwave/parser.h>
#include <kapp.h>

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
void KwavePopMenu::insertEntry (const char *name,const char *command, int keycode)
{
  int id=getUniqueId();
  commands.append (new MenuCommand (command,id));
  this->insertItem (name,id);
  this->setAccel (keycode,id);
}
//*****************************************************************************
void KwavePopMenu::selected (int num)
{
  printf ("clicked %d\n",num);
  MenuCommand *tmp=commands.first();

  while (tmp)
    {
      if (tmp->getId()==num) emit command (tmp->getCommand());
      tmp=commands.next();
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
  if (name) deleteString (name);
}
//*****************************************************************************
