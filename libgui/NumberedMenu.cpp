#include "NumberedMenu.h"
#include <libkwave/String.h>
#include "Menu.h"

NumberedMenu::NumberedMenu (const char *name)
{
  objname=duplicateString (name);
}
//*****************************************************************************
void NumberedMenu::clear ()
{
  entries.clear ();
  refresh ();
}
//*****************************************************************************
void NumberedMenu::notifyMenu (Menu *menu)
{
  notifymenus.append (menu);
}
//*****************************************************************************
void NumberedMenu::addEntry (const char *entry)
{
  entries.append (duplicateString (entry));
  refresh ();
}
//*****************************************************************************
void NumberedMenu::refresh ()
{
  //refresh of menus already in menu tree
  Menu *tmp=notifymenus.first();

  while (tmp)
    {
      tmp->clear();  //clear menu
      int itemcnt=1;
      int id=tmp->getId();

      const char *entry=entries.first();  //and insert all entries from scratch

      while ((entry)&&(itemcnt<MENUMAX))
	{
	  tmp->insertItem (entry,id+itemcnt);
	  entry=entries.next();
	  itemcnt++;
	}

      //call just in case an update of the checkmark is needed
      tmp->checkEntry(tmp->getCheckedId());

      tmp=notifymenus.next();
    }
}
//*****************************************************************************
const char *NumberedMenu::name ()
{
  return objname;
}
//*****************************************************************************
NumberedMenu::~NumberedMenu ()
{
  entries.setAutoDelete(true);
  entries.clear();

  notifymenus.setAutoDelete(false);
  notifymenus.clear();
  delete objname;
}
