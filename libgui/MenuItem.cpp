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
  debug("Menu::getIdRange(%d", num);
  int x=unique_menu_id;
  unique_menu_id+=num;

  return x;
}
//*****************************************************************************
Menu::Menu (const char *name,int id, bool toplevel): QPopupMenu()
{
  this->parent_menu=0;
  this->name=duplicateString (name);
  this->id=id;
  this->toplevel=toplevel;
  this->toplevelEnabled=true;
  this->enabled=true;
  QObject::connect (this,SIGNAL(activated(int)),this,SLOT(selected(int)));
  QObject::connect (this,SIGNAL(highlighted(int)),this,SLOT(hilight(int)));
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
void Menu::setEnabled (const bool enable)
{
  this->enabled=enable;

//  debug("Menu::setEnabled(), menu='%s', id=%d", getName(), getId());

  Menu *parentMenu = this->getParent();
  if (!this->isTopLevel() && parentMenu)
    {
      parentMenu->setItemEnabled(this->getId(),
	(enable && toplevelEnabled));
    }
  else
    {
      Menu *child=children.first();
      while (child)
	{
	  child->setTopLevelEnabled(enable);
	  child=children.next();
	}
    }
}
//*****************************************************************************
/**
 * Sets/resets an internal flag of the menu that indicates that it is a
 * toplevel menu. This setting affects the behaviour of the setEnabled()
 * function.
 * <p>
 * @param enable true=toplevel, false if not
 * @see Menu::setEnabled(bool)
 */
void Menu::setTopLevelEnabled (const bool enable)
{
  toplevelEnabled=enable;
  setEnabled(this->enabled);
}
//*****************************************************************************
/**
 * Checks a menu item. The checkmark of a currently checked menu item
 * will be removed, so that this method could be used for 1-of-n
 * selections. The currently selected item id will be set to the new
 * id.
 * <p>
 * @param id unique id of the menu or menu entry
 */
void Menu::checkEntry(int id)
{
//  if (! checkItems) return;

  debug("Menu::checkEntry(%d): checked=%d", id, checked);
  debug("(this=%p, name='%s', id=%d)", this,
    this->getName(), this->getId());

  if (checked >= 0) this->checkEntry(checked, false);
  checked = id;
  this->checkEntry(id, true);
}
//*****************************************************************************
/**
 * Checks or unchecks a menu item. The checkmark of a currently
 * checked menu item will not be removed, so that this method
 * could be used for multiple-choice selections. The currently selected
 * item id will not be changed.
 * <p>
 * @param id unique id of the menu or menu entry
 * @param checked true=checkmark on, false=checkmark off
 */
void Menu::checkEntry(const int id, const bool check)
{
  debug("Menu::checkEntry(%d,%d): checked=%d",
    id, check, checked);
  setItemChecked(id, check);
}

void Menu::check(int id)
{
/*  Menu *parent = getParent();
  if (parent) parent->*/
  checkEntry(id);
}

void Menu::hilight(int id)
{
/*
  // (this is useful for debugging menu ids)
  debug("Menu::hilight(%d)", id);
  Menu *parent = getParent();
  setCheckable(true);
  if (parent) {
      for (int i=0; i < 300; i++) {
          parent->setItemChecked(i, false);
      }
      parent->setItemChecked(id, true);
  }
*/
}
//*****************************************************************************
int Menu::insertEntry (const char *name,const char *command, int keycode, int id)
{
  int key;
  Menu *dummy;

  dummy = new Menu(name, id, false);
  commands.append (new MenuCommand (command,id));
  children.append (dummy);

  key=this->insertItem (klocale->translate(name),id);
  this->setAccel (keycode,id);

  dummy->setId(key);
  dummy->setParent(this);
  return key; // return the real id of the menu entry
}
//*****************************************************************************
int Menu::insertMenu (Menu *entry)
{
  int key;
  children.append (entry);
  key = this->insertItem(entry->getName(), entry, entry->getId());
  entry->setId(key);
  entry->setParent(this);
  return key;
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

